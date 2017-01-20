//------------------------------------------------------------------
//
// Movie class
//
//Notes:
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MC2MOVIE_H
#define MC2MOVIE_H

//--------------------------------------------------------------------------
#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include"platform_windows.h"

//--------------------------------------------------------------------------
const DWORD MAX_TEXTURES_NEEDED = 6;

//--------------------------------------------------------------------------
class MC2Movie
{
	public:
		MC2Movie (void)
		{
			MC2Surface = NULL;

			for (long i=0;i<MAX_TEXTURES_NEEDED;i++)
			{
				mc2TextureNodeIndex[i] = 0xffffffff;
			}

			MC2Rect.bottom = MC2Rect.top = MC2Rect.left = MC2Rect.right = 0;

			numWide = numHigh = 0;
			totalTexturesUsed = 0;

			forceStop = false;
			stillPlaying = false;

			separateWAVE = false;
			soundStarted = false;

			waveName = NULL;
			m_MC2Name = NULL;
		}

		~MC2Movie (void)
		{
			if (MC2Surface)
			{
				systemHeap->Free(MC2Surface);
				MC2Surface = NULL;
			}

			if (waveName)
			{
				delete [] waveName;
				waveName = NULL;
			}

			if (m_MC2Name)
			{
				delete [] m_MC2Name;
				m_MC2Name = NULL;
			}
		}

		//Movie name assumes path is correct.
		// Sets up the MC2 to be played.
		void init (const char *MC2Name, RECT mRect, bool useWaveFile);

		//Handles tickling MC2 to make sure we keep playing back
		// Returns true when MC2 is DONE playing!!
		bool update (void);

		//Actually draws the MC2 texture using gos_DrawTriangle.
		void render (void);

		//Immediately stops playback of MC2.
		void stop (void)
		{
			forceStop = true;
		}

		//Pause video playback.
		void pause (bool pauseState)
		{
			if (stillPlaying)
			{
			}

			forceStop = false;
		}

		//Restarts MC2 from beginning.  Can be called anytime.
		void restart (void)
		{
			stillPlaying = true;
			forceStop = false;
		}

		//Changes rect.  If resize, calls malloc which will be QUITE painful during a MC2 playback
		// If just move, its awfully fast.
		void setRect (RECT vRect);

		bool isPlaying (void)
		{
			return stillPlaying;
		}

		char *getMovieName (void)
		{
			return m_MC2Name;
		}
		
	protected:
	
		DWORD*		MC2Surface;									//Extra surface used if MC2 Movie is larger then 256x256
		DWORD		mc2TextureNodeIndex[MAX_TEXTURES_NEEDED];		//Handles to textures for MC2 movie data.
		RECT		MC2Rect;										//Physical Location on screen for MC2 movie.
					
		DWORD 		numWide;										//Number of textures wide display is
		DWORD		numHigh;										//Number of textures high the display is
		DWORD		totalTexturesUsed;                              //total Number of texture used to display 
					
		bool		forceStop;										//Should MC2 movie end now?
					
		DWORD		singleTextureSize;								//Size of the single texture.  Fit it to smallest texture we can use.
		bool		stillPlaying;									//Is MC2 movie over?

		bool		separateWAVE;									//Tells us if this MC2 movie has a separate soundtrack.
		bool		soundStarted;									//If this MC2 movie has a separate soundtrack, this tells us when to start it.
		char		*waveName;										//Name of the wavefile.
		char		*m_MC2Name;										//Name of the Movie.

		void BltMovieFrame (void);								//Actually moves frame data from MC2 movie to surface and/or texture(s)
};

typedef MC2Movie *MC2MoviePtr;
//--------------------------------------------------------------------------
#endif
