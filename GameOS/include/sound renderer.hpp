#pragma once
//===========================================================================//
// File:	 Sound Renderer.hpp												 //
// Contents: Direct Sound manager											 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"linkedlist.hpp"
#include"directx.hpp"
#include "Sound Resource.hpp"
#include "Sound DS3DChannel.hpp"
#include "Sound DS3DMixer.hpp"


//
// This structure is used to initialize sound channels
//
// When GameOS is started, the init structure passed to GameOS by the application will contain a master frequency (11025,22050 or 44100), a number of channels (int) and a pointer to an array, a byte per channel containing flags listed above.
//
// Place to hold information about the sound renderer itself
//


//
// Used for sound device enumeration - Maximum of 8 devices
//
typedef struct
{
	GUID				Guid;
	char				Description[128];
	char				Driver[128];
	char				Version[256];
	DSCAPS				Caps;

} SoundDeviceInfo;

#define MAX_SOUNDCHANNELS 64


typedef struct _srdata
{
	int m_numChannels;
	DS3DSoundChannel *
		m_Channel[MAX_SOUNDCHANNELS];
	DS3DSoundMixer *
		m_Mixer;
	LONG 
		m_RendererTimer;
	int
		m_isInitialized;
    UINT 
		m_Timer;
	UINT
		m_TimerRefCount;
	LONG 
		m_TimerServicing;             // reentrancy semaphore
	char *
		m_workBuffer;

	SoundDeviceInfo m_DeviceArray[8];
	DWORD m_numDevices;
	DWORD m_PreferredDevice;
	bool m_reset;
	bool m_allPaused;
#ifdef LAB_ONLY
	char m_playHistory[32][128];
	bool m_verboseDebugger;
	DWORD m_playHistoryItr;
#endif

} SoundRendererData;

extern HGOSHEAP SoundHeap;

void SoundRendererInstall(int);

void SoundRendererStartFrame();

void SoundRendererEndFrame();

DWORD WINAPI SoundRendererUpdate( LPVOID ThreadParam );

void SoundRendererUninstall();

void SoundRendererCreateTimer();

void SoundRendererDestroyTimer();

void SoundRendererPause();

void SoundRendererContinue();

void SoundRendererFF(double sec);

bool IsValidSoundResource(HGOSAUDIO gosAudio);


void CALLBACK 
	TimeFunc
( 
	UINT uTimerID, 
	UINT uMsg, 
	DWORD dwUser,
	DWORD dw1, 
	DWORD dw2 
);

void SoundRendererNotify();

