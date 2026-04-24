// clean_mp4_player.cpp — Standalone MP4 player
// Zero game engine dependencies. FFmpeg + SDL2 + OpenGL only.
//
// Architecture (same as ffplay/VLC):
//   Demuxer reads packets and routes to separate audio/video queues.
//   Audio queue: decoded immediately, PCM fed to SDL callback.
//   Video queue: decoded on demand, one frame at a time, PTS-gated.
//   Audio callback is the master clock. Wall-clock fallback for no-audio.

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

// ---------------------------------------------------------------------------
// Audio PCM queue — fed by main thread, drained by SDL callback
// ---------------------------------------------------------------------------
struct AudioChunk {
    uint8_t* data;
    int size;
    int offset;
    double pts;
};

struct AudioState {
    std::mutex mutex;
    std::queue<AudioChunk> queue;
    SDL_AudioDeviceID device = 0;
    SDL_AudioSpec spec{};
    std::atomic<double> clock{0.0};
    int sampleRate = 0;
    int channels = 0;
};

static void audioCallback(void* userdata, Uint8* stream, int len) {
    auto* audio = static_cast<AudioState*>(userdata);
    std::lock_guard<std::mutex> lock(audio->mutex);

    int written = 0;
    SDL_memset(stream, 0, len);

    while (written < len && !audio->queue.empty()) {
        AudioChunk& chunk = audio->queue.front();
        int available = chunk.size - chunk.offset;
        int toCopy = std::min(len - written, available);

        SDL_memcpy(stream + written, chunk.data + chunk.offset, toCopy);
        chunk.offset += toCopy;
        written += toCopy;

        if (chunk.pts >= 0 && audio->sampleRate > 0) {
            double bytesPerSec = audio->sampleRate * audio->channels * 2.0;
            audio->clock.store(chunk.pts + chunk.offset / bytesPerSec);
        }

        if (chunk.offset >= chunk.size) {
            delete[] chunk.data;
            audio->queue.pop();
        }
    }
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: clean_mp4_player <video.mp4> [widthxheight]\n";
        return 1;
    }

    std::string videoPath = argv[1];
    int winW = 1280, winH = 720;
    if (argc >= 3) {
        std::string dim = argv[2];
        auto xpos = dim.find('x');
        if (xpos != std::string::npos) {
            winW = std::atoi(dim.substr(0, xpos).c_str());
            winH = std::atoi(dim.substr(xpos + 1).c_str());
        } else {
            winW = std::atoi(argv[2]);
            if (argc >= 4) winH = std::atoi(argv[3]);
        }
    }

    // -- SDL init --
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(
        "Clean MP4 Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        winW, winH, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) { std::cerr << "Window: " << SDL_GetError() << "\n"; return 1; }

    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    if (!glCtx) { std::cerr << "GL ctx: " << SDL_GetError() << "\n"; return 1; }
    SDL_GL_MakeCurrent(window, glCtx);
    SDL_GL_SetSwapInterval(1);

    if (glewInit() != GLEW_OK) { std::cerr << "GLEW init failed\n"; return 1; }

    // -- FFmpeg: open file, find streams --
    AVFormatContext* fmtCtx = nullptr;
    if (avformat_open_input(&fmtCtx, videoPath.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Cannot open: " << videoPath << "\n";
        return 1;
    }
    avformat_find_stream_info(fmtCtx, nullptr);

    // Video stream
    int vidIdx = -1;
    AVCodecContext* vidCtx = nullptr;
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vidIdx = (int)i;
            break;
        }
    }
    if (vidIdx < 0) { std::cerr << "No video stream\n"; return 1; }
    {
        const AVCodec* codec = avcodec_find_decoder(fmtCtx->streams[vidIdx]->codecpar->codec_id);
        vidCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(vidCtx, fmtCtx->streams[vidIdx]->codecpar);
        avcodec_open2(vidCtx, codec, nullptr);
    }
    double vidTimeBase = av_q2d(fmtCtx->streams[vidIdx]->time_base);
    double fps = av_q2d(fmtCtx->streams[vidIdx]->r_frame_rate);
    double frameDur = fps > 0 ? 1.0 / fps : 1.0 / 24.0;
    int vidW = vidCtx->width, vidH = vidCtx->height;
    std::cout << "Video: " << vidW << "x" << vidH << " @ " << fps << " FPS\n";

    SwsContext* swsCtx = sws_getContext(
        vidW, vidH, vidCtx->pix_fmt,
        vidW, vidH, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    AVFrame* decFrame = av_frame_alloc();
    AVFrame* rgbFrame = av_frame_alloc();

    // IMPORTANT: sws_scale with SWS_BILINEAR uses SIMD that can overrun the
    // last row's tail by up to ~32 bytes on widths that aren't SIMD-friendly
    // (e.g. 76-pixel pilot-portrait clips). We keep align=1 so FFmpeg uses
    // packed row strides (downstream memcpy/GL assume that), but allocate with
    // a tail guard so the overrun lands in harmless memory instead of
    // corrupting the heap.
    const int SWS_TAIL_GUARD = 64;
    int rgbBufSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, vidW, vidH, 1);
    uint8_t* rgbBuf = new uint8_t[rgbBufSize + SWS_TAIL_GUARD];
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, rgbBuf, AV_PIX_FMT_RGBA, vidW, vidH, 1);

    // Audio stream
    int audIdx = -1;
    AVCodecContext* audCtx = nullptr;
    SwrContext* swrCtx = nullptr;
    AudioState audio;
    bool hasAudio = false;
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audIdx = (int)i;
            break;
        }
    }
    if (audIdx >= 0) {
        const AVCodec* codec = avcodec_find_decoder(fmtCtx->streams[audIdx]->codecpar->codec_id);
        audCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(audCtx, fmtCtx->streams[audIdx]->codecpar);
        avcodec_open2(audCtx, codec, nullptr);

        audio.sampleRate = audCtx->sample_rate;
        audio.channels = audCtx->ch_layout.nb_channels;

        swrCtx = swr_alloc();
        av_opt_set_chlayout(swrCtx, "in_chlayout", &audCtx->ch_layout, 0);
        av_opt_set_chlayout(swrCtx, "out_chlayout", &audCtx->ch_layout, 0);
        av_opt_set_int(swrCtx, "in_sample_rate", audCtx->sample_rate, 0);
        av_opt_set_int(swrCtx, "out_sample_rate", audCtx->sample_rate, 0);
        av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", audCtx->sample_fmt, 0);
        av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        swr_init(swrCtx);

        SDL_AudioSpec want{};
        want.freq = audCtx->sample_rate;
        want.format = AUDIO_S16SYS;
        want.channels = (Uint8)audio.channels;
        want.samples = 2048;
        want.callback = audioCallback;
        want.userdata = &audio;

        audio.device = SDL_OpenAudioDevice(nullptr, 0, &want, &audio.spec, 0);
        if (audio.device > 0) {
            hasAudio = true;
            std::cout << "Audio: " << audCtx->sample_rate << " Hz, "
                      << audio.channels << " ch\n";
        } else {
            std::cerr << "Audio device failed: " << SDL_GetError() << "\n";
        }
    }
    double audTimeBase = (audIdx >= 0) ? av_q2d(fmtCtx->streams[audIdx]->time_base) : 0;
    AVFrame* audFrame = av_frame_alloc();

    // -- GL texture --
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vidW, vidH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // =====================================================================
    // PACKET QUEUES — the key architectural difference from before.
    // Demuxer feeds both queues. Audio and video consume independently.
    // =====================================================================
    std::queue<AVPacket*> videoPacketQueue;
    // (Audio PCM queue is inside AudioState)

    // -- Playback state --
    Uint64 perfFreq = SDL_GetPerformanceFrequency();
    Uint64 startCounter = 0;
    bool audioStarted = false;
    bool clockRunning = false;

    uint8_t* pendingData = new uint8_t[rgbBufSize + SWS_TAIL_GUARD];
    double pendingPts = -1.0;
    bool hasPending = false;

    auto getClock = [&]() -> double {
        if (hasAudio && audioStarted) return audio.clock.load();
        if (clockRunning) return (double)(SDL_GetPerformanceCounter() - startCounter) / (double)perfFreq;
        return 0.0;
    };

    bool running = true;
    bool demuxEof = false;
    int frameCount = 0;
    int lastLoggedFrame = -1;
    const int AUDIO_PRIME_CHUNKS = 8;

    while (running) {
        // -- Events --
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE || ev.key.keysym.sym == SDLK_q)
                    running = false;
            }
        }
        if (!running) break;

        double clock = getClock();

        // =================================================================
        // STEP 1: Demux — read packets and route to queues
        // =================================================================
        if (!demuxEof) {
            int audioPcmSize;
            { std::lock_guard<std::mutex> lock(audio.mutex); audioPcmSize = (int)audio.queue.size(); }

            // Keep reading until both queues have enough, or we've read enough packets
            bool audioHungry = audioPcmSize < 15;
            bool videoHungry = (int)videoPacketQueue.size() < 30;

            int packetsRead = 0;
            while (packetsRead < 80 && (audioHungry || videoHungry)) {
                AVPacket* pkt = av_packet_alloc();
                if (av_read_frame(fmtCtx, pkt) < 0) {
                    demuxEof = true;
                    av_packet_free(&pkt);
                    break;
                }
                packetsRead++;

                if (pkt->stream_index == vidIdx) {
                    videoPacketQueue.push(pkt); // ownership transferred to queue
                } else if (pkt->stream_index == audIdx && audCtx) {
                    // Decode audio immediately and queue PCM
                    if (avcodec_send_packet(audCtx, pkt) == 0) {
                        while (avcodec_receive_frame(audCtx, audFrame) == 0) {
                            int outSamples = swr_get_out_samples(swrCtx, audFrame->nb_samples);
                            if (outSamples <= 0) continue;
                            int bufSize = outSamples * audio.channels * 2;
                            uint8_t* buf = new uint8_t[bufSize];
                            int converted = swr_convert(swrCtx, &buf, outSamples,
                                (const uint8_t**)audFrame->data, audFrame->nb_samples);
                            if (converted > 0) {
                                double aPts = pkt->pts * audTimeBase;
                                std::lock_guard<std::mutex> lock(audio.mutex);
                                audio.queue.push({buf, converted * audio.channels * 2, 0, aPts});
                            } else {
                                delete[] buf;
                            }
                        }
                    }
                    av_packet_unref(pkt);
                    av_packet_free(&pkt);
                } else {
                    // Other streams — discard
                    av_packet_unref(pkt);
                    av_packet_free(&pkt);
                }

                // Re-check hunger
                { std::lock_guard<std::mutex> lock(audio.mutex); audioHungry = (int)audio.queue.size() < 15; }
                videoHungry = (int)videoPacketQueue.size() < 30;
            }
        }

        // =================================================================
        // STEP 2: Start audio once primed
        // =================================================================
        if (hasAudio && !audioStarted) {
            int qsz;
            { std::lock_guard<std::mutex> lock(audio.mutex); qsz = (int)audio.queue.size(); }
            if (qsz >= AUDIO_PRIME_CHUNKS || demuxEof) {
                audio.clock.store(0.0);
                SDL_PauseAudioDevice(audio.device, 0);
                audioStarted = true;
                clockRunning = true;
                std::cout << "Audio started (queue: " << qsz << ")\n";
            }
        }
        if (!hasAudio && !clockRunning && !videoPacketQueue.empty()) {
            startCounter = SDL_GetPerformanceCounter();
            clockRunning = true;
        }

        // =================================================================
        // STEP 3: Check pending video frame
        // =================================================================
        clock = getClock();
        if (hasPending && pendingPts <= clock + 0.005) {
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vidW, vidH, GL_RGBA, GL_UNSIGNED_BYTE, pendingData);
            glBindTexture(GL_TEXTURE_2D, 0);
            frameCount++;
            hasPending = false;
        }

        // =================================================================
        // STEP 4: Decode video from packet queue (one frame at a time)
        // =================================================================
        while (!hasPending && !videoPacketQueue.empty()) {
            AVPacket* pkt = videoPacketQueue.front();
            videoPacketQueue.pop();

            if (avcodec_send_packet(vidCtx, pkt) == 0) {
                while (avcodec_receive_frame(vidCtx, decFrame) == 0) {
                    sws_scale(swsCtx, decFrame->data, decFrame->linesize, 0, vidH,
                              rgbFrame->data, rgbFrame->linesize);
                    int64_t pts = decFrame->best_effort_timestamp;
                    if (pts == AV_NOPTS_VALUE) pts = 0;
                    double framePts = pts * vidTimeBase;

                    clock = getClock();

                    if (!clockRunning || framePts <= clock + 0.005) {
                        // Due now — display
                        glBindTexture(GL_TEXTURE_2D, tex);
                        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vidW, vidH,
                            GL_RGBA, GL_UNSIGNED_BYTE, rgbFrame->data[0]);
                        glBindTexture(GL_TEXTURE_2D, 0);
                        frameCount++;

                        // Frame drop if behind
                        if (clockRunning && framePts < clock - frameDur) continue;
                    } else {
                        // Future frame — buffer as pending
                        memcpy(pendingData, rgbFrame->data[0], rgbBufSize);
                        pendingPts = framePts;
                        hasPending = true;
                    }
                    break; // one frame per packet typically
                }
            }

            av_packet_unref(pkt);
            av_packet_free(&pkt);

            if (hasPending) break; // got our next frame, stop decoding
        }

        // =================================================================
        // STEP 5: EOF — done when both queues empty
        // =================================================================
        if (demuxEof && videoPacketQueue.empty() && !hasPending) {
            int qsz;
            { std::lock_guard<std::mutex> lock(audio.mutex); qsz = (int)audio.queue.size(); }
            if (qsz == 0) {
                SDL_Delay(200);
                running = false;
                continue;
            }
        }

        // =================================================================
        // STEP 6: Render
        // =================================================================
        int drawW, drawH;
        SDL_GL_GetDrawableSize(window, &drawW, &drawH);
        glViewport(0, 0, drawW, drawH);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        float vidAspect = (float)vidW / vidH;
        float winAspect = (float)drawW / drawH;
        float qw, qh;
        if (vidAspect > winAspect) {
            qw = 1.0f;
            qh = winAspect / vidAspect;
        } else {
            qh = 1.0f;
            qw = vidAspect / winAspect;
        }

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor4f(1, 1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-qw,  qh);
        glTexCoord2f(1, 0); glVertex2f( qw,  qh);
        glTexCoord2f(1, 1); glVertex2f( qw, -qh);
        glTexCoord2f(0, 1); glVertex2f(-qw, -qh);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        SDL_GL_SwapWindow(window);

        // Log every 60 frames
        if (frameCount > 0 && frameCount % 60 == 0 && frameCount != lastLoggedFrame) {
            lastLoggedFrame = frameCount;
            int aq, vq;
            { std::lock_guard<std::mutex> lock(audio.mutex); aq = (int)audio.queue.size(); }
            vq = (int)videoPacketQueue.size();
            std::cout << "Frame " << frameCount << " | clock: " << clock
                      << "s | audio_q: " << aq << " | video_q: " << vq << "\n";
        }
    }

    // -- Stats --
    double finalClock = getClock();
    double expectedFrames = finalClock * fps;
    std::cout << "\nDone. " << frameCount << " frames displayed in "
              << finalClock << "s (expected ~" << (int)expectedFrames
              << " at " << fps << " FPS)\n";

    // -- Cleanup --
    if (audio.device) {
        SDL_PauseAudioDevice(audio.device, 1);
        SDL_CloseAudioDevice(audio.device);
    }
    { std::lock_guard<std::mutex> lock(audio.mutex);
      while (!audio.queue.empty()) { delete[] audio.queue.front().data; audio.queue.pop(); } }
    while (!videoPacketQueue.empty()) {
        av_packet_unref(videoPacketQueue.front());
        av_packet_free(&videoPacketQueue.front());
        videoPacketQueue.pop();
    }

    delete[] pendingData;
    glDeleteTextures(1, &tex);
    delete[] rgbBuf;
    av_frame_free(&decFrame);
    av_frame_free(&rgbFrame);
    av_frame_free(&audFrame);
    sws_freeContext(swsCtx);
    if (swrCtx) swr_free(&swrCtx);
    if (audCtx) avcodec_free_context(&audCtx);
    avcodec_free_context(&vidCtx);
    avformat_close_input(&fmtCtx);

    SDL_GL_DeleteContext(glCtx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
