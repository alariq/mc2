#pragma once
//===========================================================================//
// File:	 Music.hpp														 //
// Contents: Music Playback routines										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// ******************************	MUSIC  API	  ***************************** //
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// An enumeration of the various commands to the Music playback API. 
enum gosMusic_Command
{
	gosMusic_SeekTime = 1, 			// Seek to a frame or timestamp in hMusic
	gosMusic_Volume = 2,			// Set the volume of the multiMusic
	gosMusic_Panning = 4			// Set the pan of the multiMusic
};

//////////////////////////////////////////////////////////////////////////////////
// An enumeration of the various states the video playback can be in.
enum gosMusic_PlayMode
{
	gosMusic_PlayOnce,				// Play the channel's resource once and then end
	gosMusic_Loop,					// Continually play the channel's resource, looping after each iteration
	gosMusic_Pause,					// Pause the sound, a Continue will resume the sound from where it was paused
	gosMusic_Continue,				// SET ONLY: continue a sound that was paused.
	gosMusic_Stop,					// Silence the channel and stop processing the resource
};

//////////////////////////////////////////////////////////////////////////////////
// This structure is used to send and receive information about a particular stream music 
// resource. Each command (above) requires specific info to be initialized in the 
// structure below in order to properly carry out that command. For this data, 
// consult the comments listed under the appropriate command (listed above).
typedef struct _gosMusic_Info
{
	char *					lpstrPath;		// string specified path to data
	gosMusic_PlayMode		ePlayMode;		// the play mode (see above)
	gosMusic_PlayMode		ePlayStatus;	// the play mode (see above)
	float					fDurationSec;	// read-only duration of video (hundredth of a second)
	float					fSoFarSec;		// current play position (hundredth of a second)
	float					fVolume;		// current volume from 0 to 1
	float					fPanning;		// current volume from -1 to 1
} gosMusic_ResourceInfo;

//////////////////////////////////////////////////////////////////////////////////
// Send a command to an existing music resource or create a new video resource.
// Consult the structures/enums listed above for more information.
//
void __stdcall gosMusic_CreateResource( HGOSMUSIC* Handle, char* filename );
void __stdcall gosMusic_DestroyResource( HGOSMUSIC* Handle );
void __stdcall gosMusic_GetResourceInfo( HGOSMUSIC Handle, gosMusic_ResourceInfo* gmi );

void __stdcall gosMusic_SetPlayMode( HGOSMUSIC Handle, enum gosMusic_PlayMode gmpm );
gosMusic_PlayMode __stdcall gosMusic_GetPlayMode( HGOSMUSIC Handle );
void __stdcall gosMusic_Command( HGOSMUSIC Handle, enum gosMusic_Command gmc, float x, float y = 0.0f );


struct gos_Music
{
	public:
		static bool gos_Music::m_hasDirectShow;

		IMultiMediaStream *	m_pMMStream;
		IMediaStream * m_pPrimaryAudStream;
		IBasicAudio * m_pBasicAudio;

		gosMusic_PlayMode m_musicStatus, m_musicPlayMode;

		char * 	m_lpPath;
		_int64	m_lastKnownTime;
		float m_fVolume, m_fPanning;

		_int64 m_fDurationSec;
		_int64 m_fSoFarSec;
		gos_Music * m_pNext;
		HANDLE phEOS;

	public:
		gos_Music(char * path);
		~gos_Music();
		void OpenMMStream();
		bool Update();
		void Pause();
		void Continue();
		void Stop();
		void Seek(float time);
		void Restore();
		void FF(double time);
};			







void MusicManagerInstall();

void MusicManagerRestore();

void MusicManagerUninstall();

void MusicManagerUpdate();

void OpenMMStream( const char * pszFileName, IDirectDraw *pDD, IMultiMediaStream **ppMMStream, IBasicAudio ** ppBasicAudio );



