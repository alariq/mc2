#define NOMINMAX
#include "mp4player.h"
#include "gameos.hpp"
#include <iostream>
#include <algorithm>
#include <GL/gl.h>

// =========================================================================
// Audio callback — drains PCM queue, updates master clock
// =========================================================================
void MP4Player::audioCallback(void* userdata, Uint8* stream, int len) {
    auto* p = static_cast<MP4Player*>(userdata);
    std::lock_guard<std::mutex> lock(p->audioMutex);

    int written = 0;
    SDL_memset(stream, 0, len);

    while (written < len && !p->audPcmQueue.empty()) {
        AudioChunk& chunk = p->audPcmQueue.front();
        int available = chunk.size - chunk.offset;
        int toCopy = std::min(len - written, available);

        SDL_memcpy(stream + written, chunk.data + chunk.offset, toCopy);
        chunk.offset += toCopy;
        written += toCopy;

        if (chunk.pts >= 0 && p->sampleRate > 0) {
            double bytesPerSec = p->sampleRate * p->channels * 2.0;
            p->audioClock.store(chunk.pts + chunk.offset / bytesPerSec);
        }

        if (chunk.offset >= chunk.size) {
            delete[] chunk.data;
            p->audPcmQueue.pop();
        }
    }
}

// =========================================================================
// Constructor / Destructor
// =========================================================================
MP4Player::MP4Player(const std::string& path, SDL_Window* window, SDL_GLContext context)
    : fmtCtx(nullptr), vidCtx(nullptr), audCtx(nullptr),
      decFrame(nullptr), rgbFrame(nullptr), rgbBuf(nullptr), rgbBufSize(0),
      swsCtx(nullptr), swrCtx(nullptr), vidIdx(-1), audIdx(-1),
      vidTimeBase(0), audTimeBase(0), frameDur(1.0/24.0), vidW(0), vidH(0),
      audFrame(nullptr), audioDevice(0), sampleRate(0), channels(0),
      hasAudio(false), audioStarted(false),
      textureID(0), textureReady(false), sdlWindow(window), glContext(context),
      pendingData(nullptr), pendingPts(-1), hasPending(false),
      displayRect{}, moviePath(path), playing(false), paused(false),
      looped(false), demuxEof(false), clockRunning(false),
      frameCount(0), frameRate(0), perfFreq(0), startCounter(0)
{
    perfFreq = SDL_GetPerformanceFrequency();
    openFile(path);
}

MP4Player::~MP4Player() {
    cleanup();
}

// =========================================================================
// Open file, find streams, init decoders
// =========================================================================
void MP4Player::openFile(const std::string& path) {
    moviePath = path;

    if (avformat_open_input(&fmtCtx, path.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "[MP4Player] Cannot open: " << path << "\n";
        return;
    }
    avformat_find_stream_info(fmtCtx, nullptr);

    // Video stream
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vidIdx = (int)i;
            break;
        }
    }
    if (vidIdx >= 0) {
        const AVCodec* codec = avcodec_find_decoder(fmtCtx->streams[vidIdx]->codecpar->codec_id);
        vidCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(vidCtx, fmtCtx->streams[vidIdx]->codecpar);
        avcodec_open2(vidCtx, codec, nullptr);

        vidTimeBase = av_q2d(fmtCtx->streams[vidIdx]->time_base);
        frameRate = av_q2d(fmtCtx->streams[vidIdx]->r_frame_rate);
        frameDur = frameRate > 0 ? 1.0 / frameRate : 1.0 / 24.0;
        vidW = vidCtx->width;
        vidH = vidCtx->height;

        swsCtx = sws_getContext(vidW, vidH, vidCtx->pix_fmt,
                                vidW, vidH, AV_PIX_FMT_RGBA,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);

        decFrame = av_frame_alloc();
        rgbFrame = av_frame_alloc();
        // sws_scale with SWS_BILINEAR uses SIMD that overruns the last row's
        // tail by up to ~32 bytes on SIMD-unfriendly widths (e.g. 76-px pilot
        // portrait clips). Keep align=1 so linesize is packed (downstream
        // memcpy and glTexSubImage2D assume packed rows), but allocate a tail
        // guard so the overrun lands in harmless memory. Pre-fix, this bug
        // manifested as STATUS_HEAP_CORRUPTION detected on the next heap op
        // (often inside SDL_GL_SwapWindow).
        const int SWS_TAIL_GUARD = 64;
        rgbBufSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, vidW, vidH, 1);
        rgbBuf = new uint8_t[rgbBufSize + SWS_TAIL_GUARD];
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, rgbBuf,
                             AV_PIX_FMT_RGBA, vidW, vidH, 1);
        pendingData = new uint8_t[rgbBufSize + SWS_TAIL_GUARD];

        std::cout << "[MP4Player] Video: " << vidW << "x" << vidH
                  << " @ " << frameRate << " FPS\n";
    }

    // Audio stream
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

        audTimeBase = av_q2d(fmtCtx->streams[audIdx]->time_base);
        sampleRate = audCtx->sample_rate;
        channels = audCtx->ch_layout.nb_channels;

        swrCtx = swr_alloc();
        av_opt_set_chlayout(swrCtx, "in_chlayout", &audCtx->ch_layout, 0);
        av_opt_set_chlayout(swrCtx, "out_chlayout", &audCtx->ch_layout, 0);
        av_opt_set_int(swrCtx, "in_sample_rate", sampleRate, 0);
        av_opt_set_int(swrCtx, "out_sample_rate", sampleRate, 0);
        av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", audCtx->sample_fmt, 0);
        av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        swr_init(swrCtx);

        audFrame = av_frame_alloc();

        SDL_AudioSpec want{};
        want.freq = sampleRate;
        want.format = AUDIO_S16SYS;
        want.channels = (Uint8)channels;
        want.samples = 2048;
        want.callback = audioCallback;
        want.userdata = this;

        audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &audioSpec, 0);
        if (audioDevice > 0) {
            hasAudio = true;
            std::cout << "[MP4Player] Audio: " << sampleRate << " Hz, "
                      << channels << " ch\n";
        }
    }
}

// =========================================================================
// init — called by game code to set display rect and start playback
// =========================================================================
void MP4Player::init(const char* path, RECT rect, bool loop,
                     SDL_Window* window, SDL_GLContext context) {
    displayRect = rect;
    looped = loop;
    if (window) sdlWindow = window;
    if (context) glContext = context;

    // Reopen if different file
    if (path && moviePath != path && !fmtCtx) {
        openFile(path);
    }

    initTexture();
    restart();
}

void MP4Player::initTexture() {
    if (!sdlWindow || !glContext) return;
    if (vidW == 0 || vidH == 0) return;

    SDL_GL_MakeCurrent(sdlWindow, glContext);

    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vidW, vidH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    textureReady = true;
}

// =========================================================================
// cleanup
// =========================================================================
void MP4Player::cleanup() {
    if (audioDevice) {
        SDL_PauseAudioDevice(audioDevice, 1);
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        while (!audPcmQueue.empty()) {
            delete[] audPcmQueue.front().data;
            audPcmQueue.pop();
        }
    }
    while (!vidPktQueue.empty()) {
        av_packet_unref(vidPktQueue.front());
        av_packet_free(&vidPktQueue.front());
        vidPktQueue.pop();
    }

    delete[] pendingData; pendingData = nullptr;
    delete[] rgbBuf; rgbBuf = nullptr;

    if (textureID) { glDeleteTextures(1, &textureID); textureID = 0; }
    if (swsCtx) { sws_freeContext(swsCtx); swsCtx = nullptr; }
    if (swrCtx) { swr_free(&swrCtx); swrCtx = nullptr; }
    if (decFrame) { av_frame_free(&decFrame); }
    if (rgbFrame) { av_frame_free(&rgbFrame); }
    if (audFrame) { av_frame_free(&audFrame); }
    if (vidCtx) { avcodec_free_context(&vidCtx); }
    if (audCtx) { avcodec_free_context(&audCtx); }
    if (fmtCtx) { avformat_close_input(&fmtCtx); }

    playing = false;
}

// =========================================================================
// getClock — audio master, wall clock fallback
// =========================================================================
double MP4Player::getClock() {
    if (hasAudio && audioStarted) return audioClock.load();
    if (clockRunning) return (double)(SDL_GetPerformanceCounter() - startCounter) / (double)perfFreq;
    return 0.0;
}

// =========================================================================
// demux — read packets from file, route to queues
// =========================================================================
void MP4Player::demux() {
    if (demuxEof) return;

    int audioPcmSize;
    { std::lock_guard<std::mutex> lock(audioMutex); audioPcmSize = (int)audPcmQueue.size(); }

    bool audioHungry = audioPcmSize < 15;
    bool videoHungry = (int)vidPktQueue.size() < 30;
    if (!audioHungry && !videoHungry) return;

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
            vidPktQueue.push(pkt);
        } else if (pkt->stream_index == audIdx && audCtx) {
            if (avcodec_send_packet(audCtx, pkt) == 0) {
                while (avcodec_receive_frame(audCtx, audFrame) == 0) {
                    int outSamples = swr_get_out_samples(swrCtx, audFrame->nb_samples);
                    if (outSamples <= 0) continue;
                    int bufSize = outSamples * channels * 2;
                    uint8_t* buf = new uint8_t[bufSize];
                    int converted = swr_convert(swrCtx, &buf, outSamples,
                        (const uint8_t**)audFrame->data, audFrame->nb_samples);
                    if (converted > 0) {
                        double aPts = pkt->pts * audTimeBase;
                        std::lock_guard<std::mutex> lock(audioMutex);
                        audPcmQueue.push({buf, converted * channels * 2, 0, aPts});
                    } else {
                        delete[] buf;
                    }
                }
            }
            av_packet_unref(pkt);
            av_packet_free(&pkt);
        } else {
            av_packet_unref(pkt);
            av_packet_free(&pkt);
        }

        { std::lock_guard<std::mutex> lock(audioMutex); audioHungry = (int)audPcmQueue.size() < 15; }
        videoHungry = (int)vidPktQueue.size() < 30;
    }
}

// =========================================================================
// decodeVideo — pop from video packet queue, decode until we have a frame
// =========================================================================
void MP4Player::decodeVideo() {
    static bool firstFrameLogged = false;
    while (!hasPending && !vidPktQueue.empty()) {
        AVPacket* pkt = vidPktQueue.front();
        vidPktQueue.pop();

        if (avcodec_send_packet(vidCtx, pkt) == 0) {
            while (avcodec_receive_frame(vidCtx, decFrame) == 0) {
                sws_scale(swsCtx, decFrame->data, decFrame->linesize, 0, vidH,
                          rgbFrame->data, rgbFrame->linesize);
                int64_t pts = decFrame->best_effort_timestamp;
                if (pts == AV_NOPTS_VALUE) pts = 0;
                double framePts = pts * vidTimeBase;

                double clock = getClock();

                if (!clockRunning || framePts <= clock + 0.005) {
                    // Due now — upload to texture
                    if (textureReady) {
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
                        glBindTexture(GL_TEXTURE_2D, textureID);
                        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vidW, vidH,
                                        GL_RGBA, GL_UNSIGNED_BYTE, rgbFrame->data[0]);
                        glBindTexture(GL_TEXTURE_2D, 0);
                    }
                    frameCount++;
                    if (!firstFrameLogged) {
                        firstFrameLogged = true;
                        std::cout << "[MP4Player] First frame decoded: PTS=" << framePts
                                  << " texReady=" << textureReady << " texID=" << textureID
                                  << " " << vidW << "x" << vidH << "\n";
                    }

                    // Frame drop if behind
                    if (clockRunning && framePts < clock - frameDur) continue;
                } else {
                    // Future frame — buffer
                    memcpy(pendingData, rgbFrame->data[0], rgbBufSize);
                    pendingPts = framePts;
                    hasPending = true;
                }
                break;
            }
        }

        av_packet_unref(pkt);
        av_packet_free(&pkt);
        if (hasPending) break;
    }
}

// =========================================================================
// update — main per-frame call from game loop
// =========================================================================
void MP4Player::update() {
    if (!playing || paused) return;
    if (sdlWindow && glContext) SDL_GL_MakeCurrent(sdlWindow, glContext);

    // Step 1: Demux packets into queues
    demux();

    // Step 2: Start audio once primed
    if (hasAudio && !audioStarted) {
        int qsz;
        { std::lock_guard<std::mutex> lock(audioMutex); qsz = (int)audPcmQueue.size(); }
        if (qsz >= 8 || demuxEof) {
            audioClock.store(0.0);
            SDL_PauseAudioDevice(audioDevice, 0);
            audioStarted = true;
            clockRunning = true;
            std::cout << "[MP4Player] Audio started (queue: " << qsz
                      << ", vidPkts: " << vidPktQueue.size() << ")\n";
        }
    }
    if (!hasAudio && !clockRunning && vidPktQueue.size() > 0) {
        startCounter = SDL_GetPerformanceCounter();
        clockRunning = true;
    }

    // Step 3: Check pending video frame
    double clock = getClock();
    if (hasPending && pendingPts <= clock + 0.005) {
        if (textureReady) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vidW, vidH,
                            GL_RGBA, GL_UNSIGNED_BYTE, pendingData);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        frameCount++;
        hasPending = false;
    }

    // Step 4: Decode video from packet queue
    decodeVideo();

    // Step 5: Handle EOF
    if (demuxEof && vidPktQueue.empty() && !hasPending) {
        int qsz;
        { std::lock_guard<std::mutex> lock(audioMutex); qsz = (int)audPcmQueue.size(); }
        if (qsz == 0) {
            if (looped) {
                // Seek back to start
                av_seek_frame(fmtCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
                if (vidCtx) avcodec_flush_buffers(vidCtx);
                if (audCtx) avcodec_flush_buffers(audCtx);
                demuxEof = false;
                audioClock.store(0.0);
                startCounter = SDL_GetPerformanceCounter();
                frameCount = 0;
            } else {
                playing = false;
            }
        }
    }
}

// =========================================================================
// render — draw current texture to displayRect
// =========================================================================
void MP4Player::render() {
    if (!textureReady || !textureID || vidW == 0) return;

    // Game uses logical coordinates (e.g. 800x600) but window may be larger.
    // Map display rect from game coords to physical window pixels.
    int windowW, windowH;
    SDL_GL_GetDrawableSize(sdlWindow, &windowW, &windowH);

    int logicalW = Environment.screenWidth > 0 ? Environment.screenWidth : windowW;
    int logicalH = Environment.screenHeight > 0 ? Environment.screenHeight : windowH;
    float scaleX = (float)windowW / logicalW;
    float scaleY = (float)windowH / logicalH;

    float physLeft   = displayRect.left   * scaleX;
    float physTop    = displayRect.top    * scaleY;
    float physRight  = displayRect.right  * scaleX;
    float physBottom = displayRect.bottom * scaleY;
    float physW = physRight - physLeft;
    float physH = physBottom - physTop;

    // Save GL state
    GLint savedViewport[4];
    glGetIntegerv(GL_VIEWPORT, savedViewport);
    GLboolean savedBlend = glIsEnabled(GL_BLEND);
    GLboolean savedTex2d = glIsEnabled(GL_TEXTURE_2D);
    GLboolean savedDepth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean savedScissor = glIsEnabled(GL_SCISSOR_TEST);
    GLint savedMatrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &savedMatrixMode);
    GLint savedActiveTex;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &savedActiveTex);
    GLint savedTexEnvMode;
    glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &savedTexEnvMode);
    GLint savedUnpackAlign, savedUnpackRowLen;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &savedUnpackAlign);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &savedUnpackRowLen);

    // Set up clean GL state for video rendering
    glViewport(0, 0, windowW, windowH);
    glActiveTexture(GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // Scissor to clip to physical rect (GL uses bottom-left origin)
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)physLeft, windowH - (int)physBottom, (int)physW, (int)physH);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Aspect-preserving fit within physical rect
    float vidAspect = (float)vidW / vidH;
    float rectAspect = physW / physH;
    float scaledW, scaledH;
    float offX = physLeft, offY = physTop;

    if (vidAspect > rectAspect) {
        scaledW = physW;
        scaledH = physW / vidAspect;
        offY += (physH - scaledH) / 2;
    } else {
        scaledH = physH;
        scaledW = physH * vidAspect;
        offX += (physW - scaledW) / 2;
    }

    // Top-left origin projection in physical window pixels
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowW, windowH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw video quad
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(offX, offY);
    glTexCoord2f(1, 0); glVertex2f(offX + scaledW, offY);
    glTexCoord2f(1, 1); glVertex2f(offX + scaledW, offY + scaledH);
    glTexCoord2f(0, 1); glVertex2f(offX, offY + scaledH);
    glEnd();

    // Restore all GL state
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_SCISSOR_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, savedTexEnvMode);
    glActiveTexture(savedActiveTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, savedUnpackAlign);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, savedUnpackRowLen);
    if (savedBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (savedTex2d) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
    if (savedDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (savedScissor) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
    glMatrixMode(savedMatrixMode);
}

// =========================================================================
// Playback control
// =========================================================================
void MP4Player::restart() {
    if (!fmtCtx || !vidCtx) return;

    playing = true;
    paused = false;
    demuxEof = false;
    audioStarted = false;
    clockRunning = false;
    hasPending = false;
    frameCount = 0;
    audioClock.store(0.0);

    av_seek_frame(fmtCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
    if (vidCtx) avcodec_flush_buffers(vidCtx);
    if (audCtx) avcodec_flush_buffers(audCtx);

    // Clear queues
    while (!vidPktQueue.empty()) {
        av_packet_unref(vidPktQueue.front());
        av_packet_free(&vidPktQueue.front());
        vidPktQueue.pop();
    }
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        while (!audPcmQueue.empty()) {
            delete[] audPcmQueue.front().data;
            audPcmQueue.pop();
        }
    }

    // Audio stays paused until primed in update()
    if (audioDevice) SDL_PauseAudioDevice(audioDevice, 1);
}

void MP4Player::stop() {
    playing = false;
    paused = false;
    if (audioDevice) SDL_PauseAudioDevice(audioDevice, 1);
}

void MP4Player::pause() {
    if (!playing) return;
    paused = !paused;
    if (audioDevice) SDL_PauseAudioDevice(audioDevice, paused ? 1 : 0);
}

bool MP4Player::isPlaying() {
    return playing && !paused;
}

bool MP4Player::isDone() {
    return !playing;
}

void MP4Player::setVolume(float volume) {
    // SDL2 doesn't have per-device volume; would need to scale in callback
}

void MP4Player::setRect(RECT rect) {
    displayRect = rect;
}

std::string MP4Player::getMovieName() const {
    return moviePath;
}

int MP4Player::getFrameCount() const {
    return frameCount;
}
