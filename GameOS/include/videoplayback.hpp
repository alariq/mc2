#pragma once
//===========================================================================//
// File:	 VideoPlayback.hpp												 //
// Contents: Video Playback routines										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

typedef struct gos_Video
{
	public:
		char *						m_lpPath;
		
		static HGOSHEAP m_videoHeap;
		IMultiMediaStream*			m_pMMStream;
		IMediaStream*				m_pPrimaryVidStream;   
		IBasicAudio*			   	m_pBasicAudio;
		IDirectDrawMediaStream*   	m_pDDStream;
		IDirectDrawSurface*			m_pSurface;
		IDirectDrawStreamSample*  	m_pSample;
		IDirectDrawSurface*			m_hDestSurf;
		IDirectDrawSurface*			m_hSrcSurf;
//
// Surface that is used to decompress the video to (DirectX 1 surface) and it's description
//
		IDirectDrawSurface*			m_pMMStreamSurface;
		IDirectDrawSurface7*		m_pMMStreamSurface7;
		DDSURFACEDESC2				m_pMMStreamSurfaceDesc;

		gosVideo_PlayMode			m_videoStatus, m_videoPlayMode;
		RECT						m_videoSrcRect, m_videoRect;

		int							m_videoLocX, m_videoLocY;

		float						m_scaleX, m_scaleY;
		float						m_volume, m_panning;
		
		DWORD						m_texture;

		STREAM_TIME					m_duration;
		STREAM_TIME					m_lastKnownTime;
		STREAM_TIME					m_nextFrameTime;

	public:
		gos_Video(char * path, bool texture);
		~gos_Video();
		bool
			Update();
		void
			Pause();
		void
			Continue();
		void 
			Stop();
		void 
			FF(double time);
		void
			Restore();
		void
			Release();
		void 
			SetLocation(DWORD, DWORD);
		void
			OpenMMStream(const char * pszFileName, IDirectDraw *pDD, IMultiMediaStream **ppMMStream);
}gos_Video;			

void VideoManagerInstall();

void VideoManagerPause();
void VideoManagerContinue();

void VideoManagerRelease();
void VideoManagerRestore();

void VideoManagerUninstall();

void VideoManagerUpdate();

void VideoManagerFF(double sec);

void OpenMMStream( const char * pszFileName, IDirectDraw *pDD, IMultiMediaStream **ppMMStream, IBasicAudio ** ppBasicAudio );



