//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

struct enumDevice
{
	GUID deviceGUID;
	DWORD iter;
};

class DS3DSoundMixer
{
	public:
		LPDIRECTSOUND3DLISTENER	m_lpListener;
		LPDIRECTSOUNDBUFFER m_lpPrimaryBuffer;
		LPDIRECTSOUND3DBUFFER m_lpEffectsBuffer;
		LPDIRECTSOUND m_lpDirectSound;
		DSCAPS m_dscaps;
		DSBCAPS m_mixerCaps;
		DS3DLISTENER m_Attributes3D;

		WAVEFORMATEX m_waveFormatEx;

		float m_fVolume;
		float m_reverbFactor, m_decayFactor;
		DWORD m_properties;
		bool m_changed;



	public:
		DS3DSoundMixer();
		~DS3DSoundMixer();
		void Reset();
		void Flush();
		void SetVolume( float f );
		void SetPosition( float x, float y, float z );
		void SetVelocity( float x, float y, float z );
		void SetFrontOrientation( float x, float y, float z );
		void SetTopOrientation( float x, float y, float z );
		void SetDopplerFactor( float df );
		void SetRolloffFactor( float rf );
		void SetDistanceFactor( float df );
		void SetReverbFactor( float rf );
		void SetDecayFactor( float df );
		void GetCaps();
		void SetSpeakerConfig( DWORD );
		void GetSpeakerConfig( DWORD* );
		void ReturnFormattedDebugData( char * dst, DWORD maxlen );
		void SetAllParameters();

};

