#ifndef MP4PLAYER_H
#define MP4PLAYER_H

// MP4Player — Video playback using separate packet queues (ffplay/VLC architecture)
// Audio callback is master clock. Video decoded on demand, PTS-gated.

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <windows.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

class MP4Player {
public:
    MP4Player(const std::string& moviePath, SDL_Window* window, SDL_GLContext context);
    ~MP4Player();

    void init(const char* path, RECT rect, bool looped, SDL_Window* window, SDL_GLContext context);
    void update();   // demux + decode audio + decode video (one frame)
    void render();   // draw current frame to displayRect
    void stop();
    void pause();
    void restart();
    bool isPlaying();
    bool isDone();
    void setVolume(float volume);
    void setRect(RECT rect);
    std::string getMovieName() const;
    int getFrameCount() const;

    double frameRate;

private:
    void openFile(const std::string& path);
    void cleanup();
    void demux();            // read packets, route to queues
    void decodeVideo();      // pop video packets, decode until next frame ready
    void initTexture();

    static void audioCallback(void* userdata, Uint8* stream, int len);
    double getClock();

    // FFmpeg demux/decode
    AVFormatContext* fmtCtx;
    AVCodecContext* vidCtx;
    AVCodecContext* audCtx;
    AVFrame* decFrame;
    AVFrame* rgbFrame;
    uint8_t* rgbBuf;
    int rgbBufSize;
    SwsContext* swsCtx;
    SwrContext* swrCtx;
    int vidIdx, audIdx;
    double vidTimeBase, audTimeBase;
    double frameDur;
    int vidW, vidH;

    // Video packet queue
    std::queue<AVPacket*> vidPktQueue;

    // Audio PCM queue
    struct AudioChunk {
        uint8_t* data;
        int size;
        int offset;
        double pts;
    };
    std::queue<AudioChunk> audPcmQueue;
    std::mutex audioMutex;
    AVFrame* audFrame;

    // SDL audio
    SDL_AudioDeviceID audioDevice;
    SDL_AudioSpec audioSpec;
    std::atomic<double> audioClock{0.0};
    int sampleRate, channels;
    bool hasAudio;
    bool audioStarted;

    // GL rendering
    GLuint textureID;
    bool textureReady;
    SDL_Window* sdlWindow;
    SDL_GLContext glContext;

    // Pending video frame (decoded, waiting for PTS)
    uint8_t* pendingData;
    double pendingPts;
    bool hasPending;

    // Playback state
    RECT displayRect;
    std::string moviePath;
    bool playing;
    bool paused;
    bool looped;
    bool demuxEof;
    bool clockRunning;
    int frameCount;

    // Wall clock fallback
    Uint64 perfFreq, startCounter;
};

#endif
