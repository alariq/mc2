#include "gameos.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <algorithm>
#include "utils/vec.h"

static const DWORD INVALID_AUDIO_ID = -1;
static const DWORD MAX_VOLUME = 254;

class gosAudio {

    public:

        static gosAudio* makeAudio(SDL_AudioFormat fmt, int channels, int freq, uint8_t* pdata, uint32_t size) {
            gosAudio* audio = new gosAudio(fmt, channels, freq, pdata, size);
            audio->mix_chunk_.allocated = 0;
            audio->mix_chunk_.abuf = pdata;
            audio->mix_chunk_.alen = size;
            audio->mix_chunk_.volume = 50; // just something default 
            return audio;
        }

        static void destroyAudio(gosAudio* paudio) {
            delete paudio;
        }

    private:

        gosAudio(SDL_AudioFormat fmt, int channels, int freq, uint8_t* pdata, uint32_t size):
            fmt_(fmt),
            channels_(channels),
            freq_(freq),
            pdata_(pdata),
            len_(size)
            {
            }

        ~gosAudio() {
            delete[] pdata_;
        }

        SDL_AudioFormat fmt_;
        int channels_;
        int freq_;
        uint8_t* pdata_;
        uint32_t len_;

    public:
        Mix_Chunk mix_chunk_;

};

class SoundEngine {

    public:
        SoundEngine():frequency_(0), format_(0), channels_(0), chunksize_(0), is_initialized_(false) {} 
        bool init(int frequency, bool b_fmt_16_bit, bool b_fmt_signed, bool b_stereo);
        void destroy();

        size_t addAudio(gosAudio* audio) {
            gosASSERT(audio);
            audioList_.push_back(audio);
            return audioList_.size()-1;
        }

        gosAudio* getAudio(DWORD audio_id) {
            // TODO: return default audio
            if(audio_id == INVALID_AUDIO_ID) {
                return NULL;
            }
            gosASSERT(audioList_.size() > audio_id);
            gosASSERT(audioList_[audio_id] != 0);
            return audioList_[audio_id];
        }

        void deleteAudio(gosAudio* audio) {
            gosAudio::destroyAudio(audio);
            std::vector<gosAudio*>::iterator it = std::remove(audioList_.begin(), audioList_.end(), audio);
            audioList_.erase(it, audioList_.end());
        }

        SDL_AudioFormat getFormat() const { return format_; }
        int getNumChannels() const { return channels_; }
        int getFrequency() const { return frequency_; }

        gosAudio_ChannelInfo* getChannelsInfo() { return channel_info_; }
        const gosAudio_ChannelInfo* getChannelsInfo() const { return channel_info_; }

        gosAudio_ChannelInfo* getChannel(int i) {
            gosASSERT(i<NUM_CHANNELS); return &channel_info_[i];
        }
        const gosAudio_ChannelInfo* getChannel(int i) const {
            gosASSERT(i<NUM_CHANNELS); return &channel_info_[i];
        }

        bool needsConversion(int format, int channels, int freq) const {
            return format != format_ || channels != channels_ || freq != frequency_;
        }

        static const int NUM_CHANNELS = 32;
    private:
        std::vector<gosAudio*> audioList_;
        int frequency_; // Hz
        SDL_AudioFormat format_; // see SDL_audio.h
        int channels_; // mono/stereo
        int chunksize_;

        bool is_initialized_;

        gosAudio_ChannelInfo channel_info_[NUM_CHANNELS];

};


bool SoundEngine::init(int frequency, bool b_fmt_16_bit, bool b_fmt_signed, bool b_stereo) {

    if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        STOP(("Failed to initialize SoundEngine: MSDL_InitSubSystem: %s\n", SDL_GetError()));
    }

    int initted = Mix_Init(0);
    (void)initted;

    frequency_ = frequency;
    if(b_fmt_16_bit) {
        format_ = b_fmt_signed ? AUDIO_S16LSB : AUDIO_U16LSB;
    } else {
        format_ = b_fmt_signed ? AUDIO_S8 : AUDIO_U8;
    }
    channels_ = b_stereo ? 2 : 1;
    chunksize_ = 1024;

    int rv = Mix_OpenAudio(frequency_, format_, channels_, chunksize_);
    if(rv == -1) {
        PAUSE(("Failed to initialize SoundEngine: Mix_OpenAudio: %s\n", Mix_GetError()));
    }


    {
        int audio_rate;
        Uint16 audio_format;
        int audio_channels;
        int audio_buffers = chunksize_;

        Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);             
        printf("Opened audio at %d Hz %d bit %s (%s), %d bytes audio buffer\n", audio_rate,
                (audio_format&0xFF),                                                
                (audio_channels > 2) ? "surround" : (audio_channels > 1) ? "stereo" : "mono",
                (audio_format&0x1000) ? "Big-Endian" : "Little-Endian",                                
                audio_buffers);  
    }

    int num_allocated_chanels = Mix_AllocateChannels(NUM_CHANNELS);
    gosASSERT(NUM_CHANNELS == num_allocated_chanels);

#if 0
    HGOSAUDIO hres;
    gosAudio_CreateResource(&hres, gosAudio_StreamedFile, "./data/sound/mc2_01.wav");
    // play sample on first free unreserved channel
    // play it exactly once through
    // Mix_Chunk *sample; //previously loaded
    if(Mix_PlayChannel(-1, &hres->mix_chunk, 0)==-1) {
        printf("Mix_PlayChannel: %s\n",Mix_GetError());
            // may be critical error, or maybe just no channels were free.
                // you could allocated another channel in that case...
    }
#endif

    memset(&channel_info_, 0, sizeof(channel_info_));
    for(int i=0;i<NUM_CHANNELS;++i) {
        channel_info_[i].ePlayMode = gosAudio_Stop;
        //Mix_Volume(i, 127);
    }

    return rv == -1 ? false : true;
}

void SoundEngine::destroy() {

    for(int i=0; i<NUM_CHANNELS;++i) {
        Mix_HaltChannel(i);
    }

    std::vector<gosAudio*>::iterator it = audioList_.begin();
    for(;it!=audioList_.end();++it) {
            gosAudio::destroyAudio(*it);
    }
    audioList_.clear();

    Mix_CloseAudio();
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    is_initialized_ = false;
}

SoundEngine* g_sound_engine = NULL;

////////////////////////////////////////////////////////////////////////////////
// possibly pass parameters
bool gos_CreateAudio() {
    if(!g_sound_engine) {
        g_sound_engine = new SoundEngine();
        return g_sound_engine->init(44100/2, true, true, true);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void gos_DestroyAudio() {
    if(g_sound_engine) {
        g_sound_engine->destroy();
        delete g_sound_engine;
        g_sound_engine = NULL;
    }
}

#define USE_MIX_LOAD

//////////////////////////////////////////////////////////////////////////////////
// Creates a resource to be played later
//
void __stdcall gosAudio_CreateResource( HGOSAUDIO* hgosaudio, enum gosAudio_ResourceType res_type, const char* file_name, gosAudio_Format* ga_wf, void* data, int size, bool only2D)
{
    gosASSERT(hgosaudio);
    gosASSERT((res_type == gosAudio_UserMemory && data && ga_wf) || (res_type == gosAudio_StreamedFile && file_name));

	//WORD  wFormatTag;				// Waveform-audio format type. 1=PCM, 2=Microsoft ADPCM.
	//WORD  nChannels; 				// 1=Mono, 2=Stereo.
    //DWORD nSamplesPerSec;			// Sample rate, 11025Hz, 22050Hz or 44100Hz.
	//DWORD nAvgBytesPerSec;          // Normally, nBlockAlign * nSamplesPerSec
	//WORD  nBlockAlign; 				// Normally, wBitsPerSample / 8 * nChannels
    //WORD  wBitsPerSample;			// Bits per sample for the wFormatTag format type. If wFormatTag is 1 (PCM), then wBitsPerSample should be equal to 8 or 16.
	//WORD  cbSize;					// Size, in bytes, of extra format information appended to the end of the WAVEFORMATEX structure. For PCM's, this should be set to 0. For ADPCM, this should be set to 32.

    uint8_t* databuf = NULL;
    uint32_t src_datasize = 0;

    SDL_AudioFormat src_fmt = 0;
    int src_channels = 0;
    int src_freq = 0;
    bool from_file = false;

#ifdef USE_MIX_LOAD
    Mix_Chunk* chunk = NULL;
#endif

    const SDL_AudioFormat dst_fmt = g_sound_engine->getFormat();
    const int dst_channels = g_sound_engine->getNumChannels();
    const int dst_freq = g_sound_engine->getFrequency();

    if(res_type == gosAudio_StreamedFile) {

        from_file = true;

#ifndef USE_MIX_LOAD
        SDL_AudioSpec spec;
        SDL_AudioSpec* pspec;
        pspec = SDL_LoadWAV(file_name, &spec, &databuf, &src_datasize);
        if(pspec == NULL) {
            PAUSE(("gosAudio_CreateResource: Failed to load %s, %s\n", file_name, SDL_GetError()));
            *hgosaudio = NULL;
            return;
        }

        src_fmt = spec.format;
        src_freq = spec.freq;
        src_channels = spec.channels;

        switch(spec.format)
        {
            case AUDIO_U8: printf("audio_u8\n"); break;
            case AUDIO_S8: printf("audio_s8\n"); break;
            case AUDIO_S16LSB: printf("audio_u16lsb\n"); break;
            case AUDIO_U16LSB: printf("AUDIO_s16lsb\n"); break;
            default: STOP(("Unsupported audio format\n"));
        }
#else
        chunk = Mix_LoadWAV(file_name);
        if(NULL == chunk) {
            SPEW(("AUDIO", "gosAudio_CreateResource: Failed to load %s, %s\n", file_name, Mix_GetError()));
            *hgosaudio = NULL;
            return;
        }
        databuf = chunk->abuf;
        src_datasize = chunk->alen;
        // Mix library always loads data in our output format
        src_freq = dst_freq;
        src_fmt = dst_fmt;
        src_channels = dst_channels;
#endif

    } else {
        gosASSERT(res_type == gosAudio_UserMemory);
        src_freq = ga_wf->nSamplesPerSec;
        src_fmt = ga_wf->wBitsPerSample==8 ? AUDIO_S8 : AUDIO_S16LSB;
        src_channels = ga_wf->nChannels;
        databuf = (uint8_t*)data;
        src_datasize = size;
    }

    uint32_t num_datapoints = src_datasize /(src_channels * ((SDL_AUDIO_MASK_BITSIZE & src_fmt) / 8));
    uint32_t audio_data_size = num_datapoints * dst_channels * ((SDL_AUDIO_MASK_BITSIZE & dst_fmt) / 8);
    double mul_k = (double)dst_freq / (double)src_freq;
    audio_data_size =(uint32_t)(audio_data_size * mul_k);

    uint8_t* audio_data = new uint8_t[audio_data_size];
    memset(audio_data, 0, audio_data_size);
    uint8_t* tmp_req_buf = NULL;

    const bool needs_conversion = g_sound_engine->needsConversion(src_fmt, src_channels, src_freq);
    if(needs_conversion) {

		SDL_AudioCVT cvt = { 0 };

        if(0 > SDL_BuildAudioCVT(&cvt,
                    src_fmt, src_channels, src_freq,
                    dst_fmt, dst_channels, dst_freq)) {

            PAUSE(("SDL_BuildAudioCVT: Failed to build conversion structure : %s\n", SDL_GetError()));
            delete[] audio_data;
            if(from_file)
                SDL_FreeWAV(databuf);
            *hgosaudio = NULL;
            return;
        }

        cvt.len = src_datasize;

        gosASSERT(cvt.needed);
        const uint32_t tmp_req_buf_size = cvt.len * cvt.len_mult;

        if(src_datasize >= tmp_req_buf_size) {
            cvt.buf = databuf;
        } else {
            tmp_req_buf = new uint8_t[tmp_req_buf_size];
            memcpy(tmp_req_buf, databuf, src_datasize);
            cvt.buf = tmp_req_buf;
        }

        // conversion is in-place
        if(0 > SDL_ConvertAudio(&cvt)) {
            PAUSE(("Failed to convert audio: %s\n", SDL_GetError()));
            delete[] audio_data;
            if(from_file)
                SDL_FreeWAV(databuf);
            *hgosaudio = NULL;
            delete[] tmp_req_buf;
            return;
        }

		// for some reason len_cvt may be less than required :-/
        gosASSERT(cvt.len_cvt <= (int)audio_data_size);
		if(cvt.len_cvt < (int)audio_data_size)
            SPEW(("AUDIO", "cvt.len_cvt != audio_data_len %d != %d\n", cvt.len_cvt, audio_data_size));
		if (cvt.len_cvt < (int)audio_data_size)
			audio_data_size = cvt.len_cvt;

        // if conversion took place in original buffer
        if(audio_data != cvt.buf) {
            memcpy(audio_data, cvt.buf, audio_data_size);
        }

    } else {
        memcpy(audio_data, databuf, audio_data_size);
    }

    delete[] tmp_req_buf;

#ifndef USE_MIX_LOAD
    if(from_file)
        SDL_FreeWAV(databuf);
#else
    if(from_file)
        Mix_FreeChunk(chunk);
#endif

    gosAudio* paudio = gosAudio::makeAudio(dst_fmt, dst_channels, dst_freq, audio_data, audio_data_size);
    if(!paudio) {
        PAUSE(("makeAudio: Failed to create audio resource\n"));
        return;
    }
    g_sound_engine->addAudio(paudio);
    *hgosaudio = paudio;
}

//////////////////////////////////////////////////////////////////////////////////
// Destroy a resource; any sounds currently playing using the ResourceID will be
//  stopped.
//  Any memory the SoundAPI associated with the resource will be freed.
//
void __stdcall gosAudio_DestroyResource( HGOSAUDIO* hgosaudio )
{
    gosASSERT(g_sound_engine && hgosaudio);

    gosAudio* audio = (gosAudio*)*hgosaudio;
    gosAudio_ChannelInfo* pci = g_sound_engine->getChannelsInfo();
    if(!audio)
        return;

    *hgosaudio = NULL;

    for(int i=0; i<g_sound_engine->NUM_CHANNELS;++i) {
        if(pci[i].hAudio == audio) {
            Mix_HaltChannel(i);
            pci[i].hAudio = NULL;
            pci[i].ePlayMode = gosAudio_Stop;

            g_sound_engine->deleteAudio(audio);
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
// This prepares the channel for a specific type of sound playback. Optimally,
// allocate only the properties that will need modification. Use a bitwise'd group
// of gosAudio_Properties to set what is needed.
//
void __stdcall gosAudio_AllocateChannelSliders( int Channel, DWORD properties)
{
    gosASSERT(g_sound_engine);
    gosASSERT(g_sound_engine->NUM_CHANNELS > Channel);
    gosAudio_ChannelInfo* ci = g_sound_engine->getChannel(Channel);
    if(!ci)
        return;

    ci->dwProperties = properties;
    // do some real work here?
}

//////////////////////////////////////////////////////////////////////////////////
// Prepare a channel to play a resource of any type.
//
void __stdcall gosAudio_AssignResourceToChannel( int Channel, HGOSAUDIO hgosaudio)
{
    gosASSERT(g_sound_engine);
    gosASSERT(g_sound_engine->NUM_CHANNELS > Channel);
    gosAudio_ChannelInfo* ci = g_sound_engine->getChannel(Channel);
    if(!ci)
        return;

    // this should only assign resource to channel but not play it
    gosAudio* audio = (gosAudio*)hgosaudio;
    ci->hAudio = audio;
}

//////////////////////////////////////////////////////////////////////////////////
// Get and Set functions only operate if a channel has the property enabled
//  Channel number -1 used in SetVolume and SetPanning will alter the windows master
//  volume and balance
void __stdcall gosAudio_SetChannelSlider( int Channel, enum gosAudio_Properties prop, float value1, float value2, float value3)
{
    gosASSERT(g_sound_engine);
    gosASSERT(g_sound_engine->NUM_CHANNELS > Channel);
    gosAudio_ChannelInfo* ci = g_sound_engine->getChannel(Channel);
    if(!ci)
        return;

    switch(prop) {
        case gosAudio_Common: break; //?
        case gosAudio_Volume:
        {
            ci->fVolume = saturate(value1);
            Mix_Volume(Channel, (DWORD)(ci->fVolume*MAX_VOLUME));
            //printf("Mix_Volume(%d, %d)\n", Channel, (DWORD)(ci->fVolume*MAX_VOLUME));
            break;
        }
        case gosAudio_Panning:
        {
            ci->fPanning = value1; // -1 ... +1
            // TODO: looks like panning sets level of volume in channels (like max. posssible)
            // so if we set Panning(0,0) and then changes volume, we will not hear anyting!
            // probably we need to uodate panning each time we set volume...  
            // like check dwProperties if panning property allocated and then do our dirty stuff
            /*
            uint8_t cur_volume = (uint8_t)(2*ci->fVolume * MAX_VOLUME);
            float t = (0.5f * ci->fPanning + 0.5f);
            t = t < 0.0f ? 0.0f : t;
            t = t > 1.0f ? 1.0f : t;
            uint8_t right = (uint8_t)(t*cur_volume);
            Mix_SetPanning(Channel, cur_volume - right, right);
            printf("Mix_SetPanning(%d, %d, %d)\n", Channel, cur_volume-right, right);
            */
            break;
        }
        case gosAudio_Frequency:
        {
            PAUSE(("Cannot change frequency on a specific channel\n"));
            break;
        }
        default:
            PAUSE(("Slider not supported yet\n"));
    }
}

void __stdcall gosAudio_GetChannelSlider( int Channel, enum gosAudio_Properties prop, float* value1, float* value2, float* value3)
{
    gosASSERT(g_sound_engine);
    gosASSERT(g_sound_engine->NUM_CHANNELS > Channel);
    gosAudio_ChannelInfo* ci = g_sound_engine->getChannel(Channel);
    if(!ci)
        return;

    gosASSERT(value1);

    switch(prop) {
        case gosAudio_Common: break; //?
        case gosAudio_Volume: *value1 = ci->fVolume; break;
        case gosAudio_Panning: *value1 = ci->fPanning; // -1 ... +1
        case gosAudio_Frequency:
        {
            PAUSE(("Cannot change frequency on a specific channel\n"));
            break;
        }
        default:
            PAUSE(("Slider not supported yet\n"));
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Play, Loop, Stop, Pause, or Continue a particular channel
//
void __stdcall gosAudio_SetChannelPlayMode( int Channel, enum gosAudio_PlayMode ga_pm )
{
    gosASSERT(g_sound_engine);
    gosASSERT(g_sound_engine->NUM_CHANNELS > Channel);
    gosAudio_ChannelInfo* ci = g_sound_engine->getChannel(Channel);
    if(!ci)
        return;

    ci->ePlayMode = ga_pm;

    switch(ga_pm) {
        case gosAudio_PlayOnce:
        case gosAudio_Loop:
        {
            gosAudio* audio = (gosAudio*)ci->hAudio;
            if(audio) {
                int loops = ci->ePlayMode == gosAudio_Loop  ? -1 : 0;
                Mix_PlayChannel(Channel, &audio->mix_chunk_, loops);
                printf("Mix_PlayChannel(%d)\n", Channel);
            }
            break;
        }
        case gosAudio_Pause:
        {
            Mix_Pause(Channel);
            break;
        }
        case gosAudio_Continue:
        {
            Mix_Resume(Channel);
            break;
        }
        case gosAudio_Stop:
        {
            Mix_HaltChannel(Channel);
            break;
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////
// Determine the current play mode of a channel
//
gosAudio_PlayMode __stdcall gosAudio_GetChannelPlayMode( int Channel )
{
    gosASSERT(g_sound_engine);
    gosASSERT(g_sound_engine->NUM_CHANNELS > Channel);
    gosAudio_ChannelInfo* ci = g_sound_engine->getChannel(Channel);
    if(!ci) {
        return gosAudio_PlayOnce;
    }
    int playing = Mix_Playing(Channel);
    return playing ? ci->ePlayMode : gosAudio_Stop;
}

