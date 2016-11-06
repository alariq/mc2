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
// Class MC2Movie
void MC2Movie::init (const char *MC2Name, RECT mRect, bool useWaveFile)
{
		char MOVIEName[1024];
		_splitpath(MC2Name,NULL,NULL,MOVIEName,NULL);

		m_MC2Name = new char [strlen(MOVIEName)+1];
		memset(m_MC2Name,0,strlen(MOVIEName)+1);
		strcpy(m_MC2Name,MOVIEName);

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
}

//-----------------------------------------------------------------------
//Changes rect.  If resize, calls malloc which will be QUITE painful during a MC2 playback
// If just move, its awfully fast.
void MC2Movie::setRect (RECT vRect)
{
	if (((vRect.right - vRect.left) != (MC2Rect.right - MC2Rect.left)) ||
		((vRect.bottom - vRect.top) != (MC2Rect.bottom - MC2Rect.top)))
	{
		//Size changed.  STOP for now to tell people this is bad!
		// May be impossible to do when MC2 is running because MC2 counts on previous frame's contents not changing
		STOP(("Tried to change MC2 Movie Rect size to different one from starting value"));
	}
	else
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
		soundSystem->playDigitalStream(waveName);
	}

	if (
		stillPlaying)
	{
		if (forceStop)
		{
			stillPlaying = false;

			if (separateWAVE)
				soundSystem->stopSupportSample();

			return true;
		}
	
		return false;
	}

	return true;
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

}

//--
