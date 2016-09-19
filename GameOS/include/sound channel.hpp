#pragma once
//===========================================================================//
// File:	 Sound Channel.cpp												 //
// Contents: Direct Sound manager											 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"ezvector.hpp"
//
// Maximum number of available channels;
//
#define MAX_SOUNDCHANNELS 32
class SoundChannel
{
	public:
		float m_fVolume, m_fPanning, m_fFrequency;
		DWORD m_currentProperties;
		DWORD m_oldProperties;
		SoundResource * m_currentSoundResource;
		bool m_true3D;
		bool m_emulatedByGameOS;
		bool m_changed;

		EZVector m_position3D;
		EZVector m_velocity3D;
		EZVector m_orientation3D;

		struct
		{
			float inner;
			float outer;
		} m_innerOuterAngles;

		float m_distanceMin, m_distanceMax;
		bool m_bLoopFlag;
	    bool m_fCued;                  // semaphore (stream cued)
		bool m_fPlaying;               // semaphore (stream playing)
		bool m_fPaused;               // semaphore (stream playing)
	    UINT m_cbBufOffset;            // last write position
	    UINT m_nBufLength;             // length of sound buffer in msec
	    UINT m_cbBufSize;              // size of sound buffer in bytes
	    UINT m_nDuration;              // duration of wave file
	    UINT m_nTimeStarted;           // time (in system time) playback started
	    UINT m_nTimeElapsed;           // elapsed time in msec since playback started
		DWORD m_myEnum;

		gosAudio_ResourceType m_resourceType;
		char m_resourceName[128];

	public:
		SoundChannel();
		virtual ~SoundChannel();
		virtual void Reset(DWORD chan);
		virtual void CreateAndLoadBuffer( SoundResource * ) = 0;
		virtual void CreateStreamBuffer( SoundResource * ) = 0;
		virtual void CreateBuffer() = 0;
		virtual void Play() = 0;
		virtual void Pause() = 0;
		virtual void Stop() = 0;
		virtual void ClearAndFree() = 0;
		virtual void PlayStream() = 0;
		virtual void PauseStream() = 0;
		virtual void StopStream() = 0;
		virtual void Cue() = 0;
		virtual void WriteWaveData(UINT size) = 0;
		virtual DWORD GetMaxWriteSize() = 0;
		virtual BOOL ServiceBuffer() = 0;

		virtual void SetVolume(float f);
		virtual void SetPan(float f);
		virtual void SetFrequency(float f);
		virtual void SetPosition(float x, float y, float z);
		virtual void SetVelocity(float x, float y, float z);
		virtual void SetDistanceMinMax(float min, float max);
		virtual void SetConeAngles( float min, float max );
		virtual void SetConeOrientation( float x, float y, float z );
		virtual bool IsPlaying() = 0;
		virtual void Fake3D() = 0;
		virtual DWORD GetBytesPlayed() = 0;
};

 
