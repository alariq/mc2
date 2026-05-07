//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MC2MOVIE_H
#include"mc2movie.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef PREFS_H
#include"prefs.h"
#endif

#include "../resource.h"
#include"gameos.hpp"

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg/pl_mpeg.h"

#include <SDL2/SDL.h>

struct MoviePlayerImpl {
	plm_t *plm;
};

//-----------------------------------------------------------------------
const DWORD MAX_TEXTURE_WIDTH 	= 256;
const DWORD MAX_TEXTURE_HEIGHT 	= 256;
const DWORD MAX_MOVIE_WIDTH 	= 640;
const DWORD MAX_MOVIE_HEIGHT 	= 480;
const float TEXTURE_ADJUST_MIN	= (0.4f / MAX_TEXTURE_WIDTH);
const float TEXTURE_ADJUST_MAX	= (1.0f - TEXTURE_ADJUST_MIN);

float averageFrameRate = 0.0f;
long currentFrameNum = 0;
float last30Frames[30] = {
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

extern char CDInstallPath[];
void EnterWindowMode();
void EnterFullScreenMode();
void __stdcall ExitGameOS();

//-----------------------------------------------------------------------

void app_on_video(plm_t *mpeg, plm_frame_t *frame, void *user) {
    MC2Movie *self = (MC2Movie*)user;

    // Hand the decoded data over to OpenGL. For the RGB texture mode, the
    // YCrCb->RGB conversion is done on the CPU.

    if(self->sizeY != vec2(frame->y.width, frame->y.height)) {
        gos_DestroyTexture(self->texY);
        DWORD wh = (frame->y.height << 16) | frame->y.width;
        self->sizeY = vec2(frame->y.width, frame->y.height);
        self->texY = gos_NewEmptyTexture(gos_Texture_Luminance,"MovieY", wh, gosHint_DisableMipmap);
    }
	gos_UpdateTexture(self->texY, frame->y.data, frame->y.width * frame->y.height);

    if(self->sizeCB != vec2(frame->cb.width, frame->cb.height)) {
        gos_DestroyTexture(self->texCB);
        DWORD wh = (frame->cb.height << 16) | frame->cb.width;
        self->sizeCB = vec2(frame->cb.width, frame->cb.height);
        self->texCB = gos_NewEmptyTexture(gos_Texture_Luminance,"MovieCB", wh, gosHint_DisableMipmap);
    }
	gos_UpdateTexture(self->texCB, frame->cb.data, frame->cb.width * frame->cb.height);

    if(self->sizeCR != vec2(frame->cr.width, frame->cr.height)) {
        gos_DestroyTexture(self->texCR);
        DWORD wh = (frame->cr.height << 16) | frame->cr.width;
        self->sizeCR = vec2(frame->cr.width, frame->cr.height);
        self->texCR = gos_NewEmptyTexture(gos_Texture_Luminance,"MovieCR", wh, gosHint_DisableMipmap);
    }
	gos_UpdateTexture(self->texCR, frame->cr.data, frame->cr.width * frame->cr.height);

    // The dimensions of the planes are always rounded up to the next
    // multiple of 16. We don't want to display these extra pixels, so
    // calculate the crop w/h and hand it over to the shader program.
    float cw = (float)frame->width / (float)frame->y.width;
    float ch = (float)frame->height / (float)frame->y.height;
    self->texture_crop_size_ = vec4(cw, ch, 0, 0);

	self->b_got_any_frame = true;
}

void app_on_audio(plm_t *mpeg, plm_samples_t *samples, void *user) {
	MC2Movie *self = (MC2Movie*)user;
    (void)self;

	int size = sizeof(float) * samples->count * 2;
    //printf("decoded: %d bytes\n", size);
    gosAudio_EnqueueSamples(self->audio_res_, samples->interleaved, size);
	//SDL_QueueAudio(3, samples->interleaved, size);
}

// Class MC2Movie
void MC2Movie::init (const char *MC2Name, RECT mRect, bool useWaveFile)
{
		char MOVIEName[1024];
		_splitpath(MC2Name,NULL,NULL,MOVIEName,NULL);

		m_MC2Name = new char [strlen(MOVIEName)+1];
		memset(m_MC2Name,0,strlen(MOVIEName)+1);
		strcpy(m_MC2Name,MOVIEName);

		SPEW(("MOVIE","playing movie: %s\n", MOVIEName));

	//Set the volume based on master system volume.
	// ONLY if we want silence!!!
	if (useWaveFile && (prefs.DigitalMasterVolume != 0.0f))
	{
		separateWAVE = true;
		soundStarted = false;
		char MOVIEName[1024];
		_splitpath(MC2Name,NULL,NULL,MOVIEName,NULL);

		waveName = new char [strlen(MOVIEName)+1];
		memset(waveName,0,strlen(MOVIEName)+1);
		strcpy(waveName,MOVIEName);
	}

    numHigh = 1;

    totalTexturesUsed = numWide * numHigh;

    MC2Rect = mRect;

    vec2 quad[4];
    quad[0].x = 0;
    quad[0].y = 0;

    quad[1].x = 0;
    quad[1].y = 1;

    quad[2].x = 1;
    quad[2].y = 1;

    quad[3].x = 1;
    quad[3].y = 0;

    uint16_t ib[] = {0,2,1,0,3,2};


    pimpl = (MoviePlayerImpl*)gos_Malloc(sizeof(MoviePlayerImpl));
    char buf[1024];
    sprintf(buf, "./data/movies/%s.mpg", MOVIEName);
    plm_t* plm = plm_create_with_filename(buf);

    if(!plm) {
        SPEW(("[MOVIE]", "Could not create movie: %s\n", buf));
        return;
    }

    pimpl->plm = plm;

    if (!plm_probe(pimpl->plm, 5000 * 1024)) {
        STOP(("No MPEG video or audio streams found in %s", MOVIEName));
    }


    SPEW((
        "Opened %s - framerate: %f, samplerate: %d, duration: %f",
        MOVIEName, 
        plm_get_framerate(pimpl->plm),
        plm_get_samplerate(pimpl->plm),
        plm_get_duration(pimpl->plm)
    ));

    plm_set_video_decode_callback(pimpl->plm, app_on_video, this);
    plm_set_audio_decode_callback(pimpl->plm, app_on_audio, this);

    plm_set_loop(pimpl->plm, FALSE);
    plm_set_audio_enabled(pimpl->plm, TRUE);
    plm_set_audio_stream(pimpl->plm, 0);

    // we create audio stream always if it is present
    // even if separate wav file requested, in case wav file will not be found
	if (plm_get_num_audio_streams(pimpl->plm) > 0) {
        gosAudio_Format fmt;
        fmt.nSamplesPerSec = plm_get_samplerate(pimpl->plm);
        fmt.nChannels = 2;
        fmt.wBitsPerSample = 32;
        audio_res_ = gosAudio_CreateStreamedResource(&fmt);

		// Adjust the audio lead time according to the audio_spec buffer size
        const int audio_dev_sample_frame_buffer_size = 4096; // TODO: get from audio engine
		plm_set_audio_lead_time(pimpl->plm, 
            (double)audio_dev_sample_frame_buffer_size/ (double)fmt.nSamplesPerSec);
    }

    stillPlaying = true;
    cur_movie_time_ = 0;
    bPaused = false;
    b_decode_audio_from_movie_ = !useWaveFile;
    plm_set_audio_enabled(pimpl->plm, b_decode_audio_from_movie_);
                        //
	quad_ib_ = gos_CreateBuffer(
            gosBUFFER_TYPE::INDEX, gosBUFFER_USAGE::STATIC_DRAW, sizeof(uint16_t), 6, ib);
	quad_vb_ = gos_CreateBuffer(
            gosBUFFER_TYPE::VERTEX, gosBUFFER_USAGE::STATIC_DRAW, sizeof(vec2), 4, quad);

	gosVERTEX_FORMAT_RECORD vdecl[] = { 
		{0, 2, false, sizeof(vec2), 0, gosVERTEX_ATTRIB_TYPE::FLOAT },
	};
	vdecl_ = gos_CreateVertexDeclaration(vdecl, sizeof(vdecl) / sizeof(gosVERTEX_FORMAT_RECORD));

}

void MC2Movie::destroy_stuff(struct MoviePlayerImpl* pimpl) {
	plm_destroy(pimpl->plm);
    gos_Free(pimpl);
    pimpl = 0;

    gos_DestroyVertexDeclaration(vdecl_);
    gos_DestroyBuffer(quad_vb_);
    gos_DestroyBuffer(quad_ib_);

    gos_DestroyTexture(texY);
    gos_DestroyTexture(texCB);
    gos_DestroyTexture(texCR);

    if(audio_res_) {
        gosAudio_DestroyStreamedResource(&audio_res_);
    }

    if (separateWAVE)
        soundSystem->stopSupportSample();
}

//-----------------------------------------------------------------------
//Changes rect.  If resize, calls malloc which will be QUITE painful during a MC2 playback
// If just move, its awfully fast.
void MC2Movie::setRect (RECT vRect)
{
#if 0
	if (((vRect.right - vRect.left) != (MC2Rect.right - MC2Rect.left)) ||
		((vRect.bottom - vRect.top) != (MC2Rect.bottom - MC2Rect.top)))
	{
		//Size changed.  STOP for now to tell people this is bad!
		// May be impossible to do when MC2 is running because MC2 counts on previous frame's contents not changing
		//STOP(("Tried to change MC2 Movie Rect size to different one from starting value"));
	}
	else
#endif
	{
		//Otherwise, just update the MC2Rect.
		MC2Rect = vRect;
	}
}

//-----------------------------------------------------------------------
//Handles tickling MC2 to make sure we keep playing back
bool MC2Movie::update (void)
{
	if (!soundStarted && separateWAVE)
	{
		soundStarted = true;
		if(-1 == soundSystem->playDigitalStream(waveName)) {
            // if could not load wav file use audio stream from video
            // !NB: because game relies on digital stream playing, it waits for it to 
            // finish to start another audio, in case we are playing audio stream from video
            // it will be paused if we go to e.g. options screen, and resumed if we return
            // but game could already start playing another audio stream through soundSystem
            // because it is not paused if we go to e.g. options screen
            // also if we switch to options while movie is playing, as it will be paused and 
            // digital stream not - it means when we return video will resume but audio may be already
            // finished. Should we also update video nevertheless to be in sync or try to pause digital
            // stream? or not use digital stream but use audio stream from video? but then we need to let
            // sound system know that it is still playing
            // NOTE: maybe just move video decoding to a differnt thread, this way it will be decoded even
            // if we go to options screen and will be stayed in sync with audio.
            if(audio_res_) { 
                b_decode_audio_from_movie_ = true;
                plm_set_audio_enabled(pimpl->plm, b_decode_audio_from_movie_);
            }
        }
	}


	if (
		stillPlaying)
	{
		if (forceStop)
		{
			stillPlaying = false;

			if (separateWAVE)
				soundSystem->stopSupportSample();

            // TODO: clear audio queue

			return true;
		}

        if(!bPaused) {
            plm_decode(pimpl->plm, frameLength);
            cur_movie_time_ += frameLength;
            stillPlaying = !plm_has_ended(pimpl->plm);
            SPEW(("[MOVIE]", "Playing movie: time: %f\n", (float)cur_movie_time_));
        }
	
		return false;
	}

	return true;
}

//Pause video playback.
void MC2Movie::pause (bool pauseState)
{
    //TODO: sebi: What if sound sample is separate? how do I pauseSupportSample?

    if (stillPlaying)
    {
    }

    bPaused = pauseState;

    forceStop = false;
}

//Restarts MC2 from beginning.  Can be called anytime.
void MC2Movie::restart (void)
{
    stillPlaying = true;
    forceStop = false;
    if(pimpl->plm) {
        plm_rewind(pimpl->plm);
    }

    //TODO: sebi: What if sound sample is separate? stop / start DigitalSample?
    // have not seen such movies yet

}


//-----------------------------------------------------------------------
//Actually moves frame data from MC2 to surface and/or texture(s) 
void MC2Movie::BltMovieFrame (void)
{
}

//-----------------------------------------------------------------------
//Actually draws the MC2 texture using gos_DrawTriangle.
void MC2Movie::render (void)
{
	if (!stillPlaying)
		return;

    if(!b_got_any_frame)
        return;

    gos_SetRenderState(gos_State_ZCompare, 0);
    gos_SetRenderState(gos_State_ZWrite, 1);
    //gos_SetRenderState(	gos_State_Culling, gos_Cull_);
    gos_SetRenderState(gos_State_Texture, texY);
    gos_SetRenderState(gos_State_Texture2, texCB);
    gos_SetRenderState(gos_State_Texture3, texCR);
    //gos_SetRenderViewport(viewport_[2], viewport_[3], viewport_[0], viewport_[1]);

    HGOSRENDERMATERIAL mat = gos_getRenderMaterial("gos_YCbCr");
    gos_SetRenderMaterialParameterMat4(mat, "projection_", gos_GetProjectionAsMatrix());
    gos_SetRenderMaterialParameterFloat4(mat, "texture_crop_size_", texture_crop_size_);
    vec4 scale_offset(MC2Rect.left, MC2Rect.top, MC2Rect.right - MC2Rect.left, MC2Rect.bottom - MC2Rect.top);
    gos_SetRenderMaterialParameterFloat4(mat, "scale_offset", scale_offset);
    gos_ApplyRenderMaterial(mat);
    gos_RenderIndexedArray(quad_ib_, quad_vb_, vdecl_);

    gos_SetRenderState(gos_State_Texture, 0);
    gos_SetRenderState(gos_State_Texture2, 0);
    gos_SetRenderState(gos_State_Texture3, 0);
}

//--
