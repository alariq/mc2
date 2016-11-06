//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include<io.h> // for _chmod()

#include"prefs.h"
#include"gamesound.h"
#include"missiongui.h"

#include"soundsys.h"
#include"../resource.h"

extern SoundSystem *sndSystem;

extern long DigitalMasterVolume;
extern long MusicVolume;
extern long sfxVolume;
extern long RadioVolume;
extern long BettyVolume;
extern bool useShadows;
extern bool useLocalShadows;
extern bool useWaterInterestTexture;

extern bool useUnlimitedAmmo;
extern long GameDifficulty;
extern long renderer;
extern long resolution;
extern long gammaLevel;
extern bool useLeftRightMouseProfile;
extern bool useNonWeaponEffects;
extern bool useHighObjectDetail;

extern long GameVisibleVertices;

extern volatile bool mc2UseAsyncMouse;		//Should mouse draw and update in separate thread?


extern DWORD gEnableDetailTexture;

CPrefs::CPrefs() {
	DigitalMasterVolume = 255;
	MusicVolume = 64;
	sfxVolume = 64;
	RadioVolume = 64;
	BettyVolume = 64;

	useShadows = true;
	useWaterInterestTexture = true;
	useHighObjectDetail = true;

	GameDifficulty = 0;
	useUnlimitedAmmo = true;

	renderer = 0;
	resolution = 1;
	fullScreen = false;
	gammaLevel = 0;
	useLeftRightMouseProfile = true; // if false, use old style commands
	for ( int i = 0; i < 10; i++ )
		playerName[i][0] = 0;

	for (int i = 0; i < 10; i++ )
		ipAddresses[i][0] = 0;


	baseColor = 0xffff7e00;
	highlightColor = 0xffff7e00;
	faction = 0;
	insigniaFile[0] = 0;
	for (int i = 0; i < 10; i++ )
		unitName[i][0] = 0;

#if 0
	FilterState = gos_FilterNone;
	TERRAIN_TXM_SIZE = 64;
	ObjectTextureSize = 128;
	useRealLOS = true;
	doubleClickThreshold = 0.2f;
	dragThreshold = 10;
	BaseVertexColor = 0x00000000;		//This color is applied to all vertices in game as Brightness correction.
#endif
}

int CPrefs::load( const char* pFileName ) {
	long result = 0;

	FullPathFileName prefsPathname;
	prefsPathname.init("./",pFileName,".cfg");
	FitIniFilePtr prefsFile = new FitIniFile;

#ifdef _DEBUG
	long prefsOpenResult = 
#endif
	prefsFile->open(prefsPathname);

	gosASSERT (prefsOpenResult == NO_ERR);
	{
#ifdef _DEBUG
		long prefsBlockResult = 
#endif
			prefsFile->seekBlock("MechCommander2");
		gosASSERT(prefsBlockResult == NO_ERR);
		{
			// store volume settings in global variable since soundsystem 
			// does not exist yet.  These will be set in SoundSystem::init()
			result = prefsFile->readIdLong("DigitalMasterVolume",DigitalMasterVolume);
			if (result != NO_ERR)
				DigitalMasterVolume = 255;

			result = prefsFile->readIdLong("MusicVolume",MusicVolume);
			if (result != NO_ERR)
				MusicVolume = 64;

			result = prefsFile->readIdLong("RadioVolume",RadioVolume);
			if (result != NO_ERR)
				RadioVolume = 64;

			result = prefsFile->readIdLong("SFXVolume",sfxVolume);
			if (result != NO_ERR)
				sfxVolume = 64;

			result = prefsFile->readIdLong("BettyVolume",BettyVolume);
			if (result != NO_ERR)
				BettyVolume = 64;

			result = prefsFile->readIdBoolean( "Shadows", useShadows);
			if (result != NO_ERR)
				useShadows = true;

			result = prefsFile->readIdBoolean( "DetailTexture", useWaterInterestTexture);
			if (result != NO_ERR)
				useWaterInterestTexture = true;

			result = prefsFile->readIdBoolean( "HighObjectDetail", useHighObjectDetail );
			if ( result != NO_ERR )
				useHighObjectDetail = true;

			result = prefsFile->readIdLong("Difficulty",GameDifficulty);
			if (result != NO_ERR)
				GameDifficulty = 1;

			result = prefsFile->readIdBoolean("UnlimitedAmmo",useUnlimitedAmmo);
			if (result != NO_ERR)
				useUnlimitedAmmo = true;

			result = prefsFile->readIdLong("Rasterizer",renderer);
			if (result != NO_ERR)
				renderer = 0;

			if ((renderer < 0) || (renderer > 3))
				renderer = 0;

			//Force use of video card which is above min spec.
			// Used ONLY if they are using a below minSpec Primary with an above minSpec secondary.
			if ((renderer >= 0) && (renderer < 3) && 
				(gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, renderer) +
				gos_GetMachineInformation(gos_Info_GetDeviceAGPMemory, renderer)) < 6291456)
			{
				long testRender = 0;
				while (testRender < 3)
				{
					if ((gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, testRender) +
						gos_GetMachineInformation(gos_Info_GetDeviceAGPMemory, testRender)) >= 6291456)
						break;

					testRender++;
				}

				if (testRender == 3)
				{
					//No video card in the machine above MinSpec.  Just set it to the first one.
					renderer = 0;
				}
			}
				
			result = prefsFile->readIdLong("Resolution",resolution);
			if (result != NO_ERR)
				resolution = 1;

			result = prefsFile->readIdBoolean("FullScreen",fullScreen);
			if (result != NO_ERR)
				fullScreen = false;

			result = prefsFile->readIdLong("Brightness",gammaLevel);
			if (result != NO_ERR)
				gammaLevel = 0;

			result = prefsFile->readIdBoolean( "useLeftRightMouseProfile", useLeftRightMouseProfile );
			if ( result != NO_ERR )
				useLeftRightMouseProfile = true;

			char blockName[64];
			for ( int i = 0; i < 10; i++ )
			{	
				sprintf( blockName, "PlayerName%ld", i );
				result = prefsFile->readIdString( blockName, &playerName[i][0], 255 );

				if ( result != NO_ERR && i == 0 )
				{
					result = prefsFile->readIdString( "PlayerName", &playerName[0][0], 255 );
					result = prefsFile->readIdString( "UnitName", &unitName[0][0], 255 );
					break;
				}

				sprintf( blockName, "IPAddress%ld", i );
				result = prefsFile->readIdString( blockName, &ipAddresses[i][0], 255 );


				sprintf( blockName, "UnitName%ld", i );
				result = prefsFile->readIdString( blockName, &unitName[i][0], 255 );

			}

		
			result = prefsFile->readIdLong( "BaseColor", baseColor );
			if ( result != NO_ERR )	
				baseColor = 0xffff7e00;
			result = prefsFile->readIdLong( "Highlightcolor", highlightColor );
			if ( result != NO_ERR )	
				highlightColor = 0xffff7e00;
			
			result = prefsFile->readIdLong( "faction", faction );
			result = prefsFile->readIdString( "InsigniaFile", insigniaFile, 255 );

			result = prefsFile->readIdBoolean( "PilotVideos",	pilotVideos );
			if ( result != NO_ERR )
			{
				pilotVideos = true;
			}
			result = prefsFile->readIdBoolean( "UseLocalShadows",	useLocalShadows );
			if ( result != NO_ERR )
			{
				useLocalShadows = true;
			}
			result = prefsFile->readIdBoolean( "UseNonWeaponEffects",	useNonWeaponEffects );
			if ( result != NO_ERR )
			{
				useNonWeaponEffects = 0;
			}
			result = prefsFile->readIdBoolean( "AsyncMouse",	asyncMouse );
			if ( result != NO_ERR )
				asyncMouse = 0;

			result = prefsFile->readIdLong( "FogPos",	fogPos );
			if ( result != NO_ERR )
			{
				fogPos = 50;
			}
			result = prefsFile->readIdChar( "BitDepth",	bitDepth );
			if ( result != NO_ERR )
				bitDepth = 0;

			if (bitDepth && (renderer == 3))
				bitDepth = 0;

			result = prefsFile->readIdBoolean( "SaveTranscripts", saveTranscripts );
			result = prefsFile->readIdBoolean( "Tutorials", tutorials );
			if ( result != NO_ERR )
				tutorials = true;

		}

	}
#ifndef VIEWER
	MissionInterfaceManager::loadHotKeys( *prefsFile);
#endif
	prefsFile->close();
	
	delete prefsFile;
	prefsFile = NULL;

	return 0;
}

int CPrefs::save() {
	char backupPath[256];
	char originalPath[256];
	
	strcpy( originalPath, "options.cfg" );
	strcpy( backupPath, originalPath );
	strcat( backupPath,".old" );
	_chmod(originalPath, _S_IWRITE);
	_chmod(backupPath, _S_IWRITE);
	remove(backupPath);
	rename(originalPath, backupPath);
	//MoveFileEx(originalPath, backupPath, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

	FitIniFilePtr prefsFile = new FitIniFile;
	int result = prefsFile->create( (char*)originalPath );
	if (result != NO_ERR)
	{
		gosASSERT( false );
		return -1;
	}

	{
#ifdef _DEBUG
		long prefsBlockResult = 
#endif
		prefsFile->writeBlock("MechCommander2");
		gosASSERT(prefsBlockResult == strlen("\r\n[") + strlen("MechCommander2") + strlen("]\r\n"));
		{
			result = prefsFile->writeIdLong("DigitalMasterVolume",DigitalMasterVolume);
			result = prefsFile->writeIdLong("MusicVolume",MusicVolume);
			result = prefsFile->writeIdLong("RadioVolume",RadioVolume);
			result = prefsFile->writeIdLong("SFXVolume",sfxVolume);
			result = prefsFile->writeIdLong("BettyVolume",BettyVolume);

			result = prefsFile->writeIdBoolean( "Shadows", useShadows );
			result = prefsFile->writeIdBoolean( "DetailTexture", useWaterInterestTexture );
			result = prefsFile->writeIdBoolean( "HighObjectDetail", useHighObjectDetail );

			result = prefsFile->writeIdLong("Difficulty",GameDifficulty);
			result = prefsFile->writeIdBoolean("UnlimitedAmmo",useUnlimitedAmmo);

			result = prefsFile->writeIdLong("Rasterizer",renderer);
			result = prefsFile->writeIdLong("Resolution",resolution);
			result = prefsFile->writeIdBoolean("FullScreen",fullScreen);
			result = prefsFile->writeIdLong("Brightness",gammaLevel);
			result = prefsFile->writeIdBoolean( "useLeftRightMouseProfile", useLeftRightMouseProfile );
			char blockName[64];
			for ( int i = 0; i < 10; i++ )
			{	
				
				sprintf( blockName, "PlayerName%ld", i );
				result = prefsFile->writeIdString( blockName, &playerName[i][0] );

				sprintf( blockName, "UnitName%ld", i );
				result = prefsFile->writeIdString( blockName, &unitName[i][0] );

				sprintf( blockName, "IPAddress%ld", i );
				result = prefsFile->writeIdString( blockName, &ipAddresses[i][0] );

			}

			result = prefsFile->writeIdLong( "BaseColor", baseColor );
			result = prefsFile->writeIdLong( "Highlightcolor", highlightColor );
			result = prefsFile->writeIdLong( "faction", faction );
			result = prefsFile->writeIdString( "InsigniaFile", insigniaFile );

			result = prefsFile->writeIdBoolean( "PilotVideos",	pilotVideos );
			result = prefsFile->writeIdBoolean( "UseLocalShadows",	useLocalShadows );
			result = prefsFile->writeIdBoolean( "UseNonWeaponEffects",	useNonWeaponEffects );
			result = prefsFile->writeIdBoolean( "AsyncMouse",	asyncMouse );
			result = prefsFile->writeIdLong( "FogPos",	fogPos );
			result = prefsFile->writeIdChar( "BitDepth",	bitDepth );

			result = prefsFile->writeIdBoolean( "SaveTranscripts", saveTranscripts );
			result = prefsFile->writeIdBoolean( "Tutorials", tutorials );

		}
	}
#ifndef VIEWER	
	MissionInterfaceManager::saveHotKeys( *prefsFile);
#endif
	prefsFile->close();
	
	
	delete prefsFile;
	prefsFile = NULL;

	return 0;
}

int CPrefs::applyPrefs(bool applyRes) {
	if (soundSystem) {
		/*soundSystem doesn't seem to do anything*/
		soundSystem->setDigitalMasterVolume(this->DigitalMasterVolume);
		soundSystem->setSFXVolume(this->sfxVolume);
		soundSystem->setRadioVolume(this->RadioVolume);
		soundSystem->setMusicVolume(this->MusicVolume);
		soundSystem->setBettyVolume(this->BettyVolume);
	}

	if (sndSystem) {
		sndSystem->setDigitalMasterVolume(this->DigitalMasterVolume);
		sndSystem->setSFXVolume(this->sfxVolume);
		sndSystem->setRadioVolume(this->RadioVolume);
		sndSystem->setMusicVolume(this->MusicVolume);
		sndSystem->setBettyVolume(this->BettyVolume);
	}

	::useShadows = this->useShadows;
	::useLocalShadows = this->useLocalShadows;
	::useWaterInterestTexture = this->useWaterInterestTexture;

	::GameDifficulty = this->GameDifficulty;
	::useUnlimitedAmmo = this->useUnlimitedAmmo;

	::renderer = this->renderer;
	::resolution = this->resolution;
	::gammaLevel = this->gammaLevel;
	mc2UseAsyncMouse = this->asyncMouse;
	::useLeftRightMouseProfile = this->useLeftRightMouseProfile;
	::useNonWeaponEffects = this->useNonWeaponEffects;
	::useHighObjectDetail = this->useHighObjectDetail;

	int bitDepth = this->bitDepth ? 32 : 16;

	//Play with the fog distance.
	float fogPercent = float(fogPos) / 100.0f;
	Camera::MaxClipDistance = 3000.0f + (2000.0f * fogPercent);
	Camera::MinHazeDistance	= 2000.0f + (2000.0f * fogPercent);

	::GameVisibleVertices = 30 + (30 * fogPercent);

	if (land)
	{
		land->resetVisibleVertices(::GameVisibleVertices);
		land->update();
	}

	if ( applyRes )
	{
		long localRenderer = renderer;
		if ((renderer != 0) && (renderer != 3))
			localRenderer = 0;

		bool localFullScreen = fullScreen;
		bool localWindow = !fullScreen;
		if (Environment.fullScreen && fullScreen)
			localFullScreen = false;

		switch (resolution)
		{
			case 0:			//640by480
				if ((gos_GetMachineInformation( gos_Info_ValidMode, renderer, 640, 480, bitDepth) == 0) && (bitDepth == 32))
					bitDepth = 16;

				if (renderer == 3)
					gos_SetScreenMode(640,480,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(640,480,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 1:			//800by600
				if ((gos_GetMachineInformation( gos_Info_ValidMode, renderer, 800, 600, bitDepth) == 0) && (bitDepth == 32))
					bitDepth = 16;

				if (renderer == 3)
					gos_SetScreenMode(800,600,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(800,600,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 2:			//1024by768
				if ((gos_GetMachineInformation( gos_Info_ValidMode, renderer, 1024, 768, bitDepth) == 0) && (bitDepth == 32))
					bitDepth = 16;

				if (renderer == 3)
					gos_SetScreenMode(1024,768,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(1024,768,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 3:			//1280by1024
				if ((gos_GetMachineInformation( gos_Info_ValidMode, renderer, 1280, 1024, bitDepth) == 0) && (bitDepth == 32))
					bitDepth = 16;

				if (renderer == 3)
					gos_SetScreenMode(1280,1024,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(1280,1024,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 4:			//1600by1200
				if ((gos_GetMachineInformation( gos_Info_ValidMode, renderer, 1600, 1200, bitDepth) == 0) && (bitDepth == 32))
					bitDepth = 16;

				if (renderer == 3)
					gos_SetScreenMode(1600,1200,16,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(1600,1200,16,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;
		}
	}

	return 0;
}

void CPrefs::setNewName( const char* pNewName )
{
	if ( !pNewName )
		return;
	// check and see if this name is already in here
    int i = 0;
	for ( ; i < 10; i++ )
	{
		if ( !stricmp( pNewName, playerName[i] ) )
		{
			// found the same one so now we just shuffle
			for ( int j = i; j < 9; j++ )
			{
				strcpy( playerName[j], playerName[j+1] );
			}

			memmove( playerName[1], playerName[0], sizeof( char ) * 9 * 256 );
			strcpy( playerName[0], pNewName );

			break;
		}
		else if ( !strlen( playerName[i] ) )
		{
			// found the last one...

			memmove( playerName[1], playerName[0], sizeof( char ) * 9 * 256 );
			strcpy( playerName[0], pNewName );

			break;
		}
	}

	if ( i == 10 )
	{
		memmove( playerName[1], playerName[0], sizeof( char ) * 9 * 256 );
		strcpy( playerName[0], pNewName );
	}
}

void CPrefs::setNewIP( const char* pNewIP )
{
	if ( !pNewIP )
		return;
	// check and see if this name is already in here
	for ( int i = 0; i < 10; i++ )
	{
		if ( !stricmp( pNewIP, ipAddresses[i] ) )
		{
			// found the same one so now we just shuffle
			for ( int j = i; j < 9; j++ )
			{
				strcpy( ipAddresses[j], ipAddresses[j+1] );
			}

			memmove( ipAddresses[1], ipAddresses[0], sizeof( char ) * 9 * 24 );
			strcpy( ipAddresses[0], pNewIP );

			break;
		}
		else if ( !strlen( ipAddresses[i] ) )
		{
			// found the last one...

			memmove( ipAddresses[1], ipAddresses[0], sizeof( char ) * 9 * 24 );
			strcpy( ipAddresses[0], pNewIP );

			break;
		}
	}
}

void CPrefs::setNewUnit( const char* pNewUnit )
{
	if ( !pNewUnit )
		return;
// check and see if this name is already in here
    int i = 0;
	for ( ; i < 10; i++ )
	{
		if ( !stricmp( pNewUnit, unitName[i] ) )
		{
			// found the same one so now we just shuffle
			for ( int j = i; j < 9; j++ )
			{
				strcpy( unitName[j], unitName[j+1] );
			}

			memmove( unitName[1], unitName[0], sizeof( char ) * 9 * 256 );
			strcpy( unitName[0], pNewUnit );

			break;
		}
		else if ( !strlen( unitName[i] ) )
		{
			// found the last one...

			memmove( unitName[1], unitName[0], sizeof( char ) * 9 * 256 );
			strcpy( unitName[0], pNewUnit );

			break;
		}
	}

	if ( i == 10 )
	{
		memmove( unitName[1], unitName[0], sizeof( char ) * 9 * 256 );
		strcpy( unitName[0], pNewUnit );
	}
}



