//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"ezvector.hpp"

class SoundMixer
{
	public:
		WAVEFORMATEX m_waveFormatEx;

		float m_dopplerFactor, m_rolloffFactor, m_distanceFactor;
		float m_fVolume;
		float m_reverbFactor, m_decayFactor;
		DWORD m_properties;
		bool m_isCertified;
		DWORD m_freeHW3DChannels;
		DWORD m_totalHW3DChannels;
		DWORD m_freeHWMemory;
		DWORD m_totalHWMemory;

		EZVector m_position3D;
		EZVector m_velocity3D;
		EZVector m_orientation3DTop;
		EZVector m_orientation3DFront;

		bool m_changed;


	public:
		SoundMixer();
		virtual ~SoundMixer();
		virtual void GetHardwareData();
		virtual void Pause();
		virtual void Continue();
		virtual void Clear();
		virtual void Flush();
		virtual void Reset();
		virtual void SetVolume( float f );
		virtual void SetPosition( float x, float y, float z );
		virtual void SetVelocity( float x, float y, float z );
		virtual void SetFrontOrientation( float x, float y, float z );
		virtual void SetTopOrientation( float x, float y, float z );
		virtual void SetDopplerFactor( float df );
		virtual void SetRolloffFactor( float rf );
		virtual void SetDistanceFactor( float df );
		virtual void SetReverbFactor( float rf );
		virtual void SetDecayFactor( float df );
		virtual void SetSpeakerConfig( DWORD ) = 0;
		virtual void GetSpeakerConfig( DWORD* ) = 0;
};

 
