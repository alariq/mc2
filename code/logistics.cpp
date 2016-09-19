//******************************************************************************************
// logistics.cpp - This file contains the logistics class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------------------------
// Include Files
#ifndef LOGISTICS_H
#include"logistics.h"
#endif

#ifndef TEST_SHELL
#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

#ifndef MISSIONGUI_H
#include"missiongui.h"
#endif

#ifndef MECH_H
#include"mech.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef PREFS_H
#include"prefs.h"
#endif
extern CPrefs prefs;

#ifndef GAMECAM_H
#include"gamecam.h"
#endif

#endif /*TEST_SHELL*/

#ifndef SOUNDS_H
#include"sounds.h"
#endif

#ifndef MISSIONBEGIN_H
#include"missionbegin.h"
#endif

#ifndef MECHICON_H
#include"mechicon.h"
#endif

#include"missionresults.h"
#include"paths.h"

extern bool quitGame;
extern bool justStartMission;
extern long renderer;
extern bool useUnlimitedAmmo;
extern float loadProgress;

extern bool aborted;

#include "..\resource.h"
void DEBUGWINS_print (char* s, long window = 0);


//----------------------------------------------------------------------------------
//class Logistics
Logistics *logistics = NULL;


		
//----------------------------------------------------------------------------------
void Logistics::destroy (void)
{
	delete missionBegin;
	missionBegin = NULL;

	delete missionResults;
	missionResults = NULL;
}	
		
//----------------------------------------------------------------------------------
void Logistics::start (long startMode)
{
	bMissionLoaded  = 0;
	userInput->setMouseCursor( mState_LOGISTICS );
//	userInput->mouseOn();

	DWORD localRenderer = prefs.renderer;
	if (prefs.renderer != 0 && prefs.renderer != 3)
		localRenderer = 0;

   	bool localFullScreen = prefs.fullScreen;
   	bool localWindow = !prefs.fullScreen;
   	if (Environment.fullScreen && prefs.fullScreen)
   		localFullScreen = false;

	switch (startMode)
	{
		case log_RESULTS: // pull out results later...
			active = true;
			//Heading back to logistics now.  Change screen back to 800x600
			if (prefs.renderer == 3)
				gos_SetScreenMode(800,600,16,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
			else
				gos_SetScreenMode(800,600,16,prefs.renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);

			lastMissionResult = scenarioResult;
			if ( !missionResults )
			{
				LogisticsData::instance->init();
				missionResults = new MissionResults;
				missionResults->init();
			}

			if ( scenarioResult == mis_PLAYER_WIN_BIG || MPlayer )
			{
				LogisticsData::instance->removeMechsInForceGroup();
				if ( !MPlayer )
					LogisticsData::instance->setMissionCompleted( );

				// if mission is over, play video then quit when  done
				if ( LogisticsData::instance->campaignOver() && !MPlayer && !LogisticsData::instance->isSingleMission() )
				{
					mission->destroy();
					playFullScreenVideo( LogisticsData::instance->getFinalVideo() );
					setLogisticsState(log_SPLASH);
					missionBegin->beginSplash();
					userInput->mouseOn();
					char tmp[256];
					cLoadString( IDS_FINAL_MISSION, tmp, 255 );
					FullPathFileName path;
					path.init( savePath, tmp, ".fit" );
					FitIniFile file;
					file.create( path );
					LogisticsData::instance->save(file);
					active = true;
				}
				else if ( LogisticsData::instance->isSingleMission() && !MPlayer )
				{
					missionBegin->beginSplash();
					logisticsState = log_SPLASH;
				}
				else
				{
			
					missionResults->begin();
					logisticsState = log_RESULTS;
				}

			}
			else if ( !justStartMission )
			{
				missionResults->bDone = true;
				logisticsState = log_RESULTS;
			}
			else // end the game
				quitGame = true;

			break;
		case log_SPLASH:
		{
			if (aborted) {
				if (prefs.renderer == 3)
					gos_SetScreenMode(800,600,16,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(800,600,16,prefs.renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);

				if ( missionBegin )
					missionBegin->beginSplash();

				if ( MPlayer )
				{
					delete MPlayer;
					MPlayer = NULL;
				}
			}
			
			initializeLogData();
			
			bool bTestScript = false;
			if ( bTestScript )
			{
				FitIniFile loadFile;
				loadFile.open( "data\\missions\\save.fit" );
				LogisticsData::instance->load( loadFile );
				FitIniFile saveFile;
				saveFile.open("data\\missions\\save.fit", CREATE);
				LogisticsData::instance->save( saveFile );
		
			}

			active = TRUE;
			setLogisticsState(log_SPLASH);

			if ( !missionBegin )
			{
				missionBegin = new MissionBegin;
				missionBegin->beginSplash();
				missionBegin->init();
			}

		}
		break;

		case log_ZONE:
		{
			logisticsData.init();
			if ( !missionBegin )
			{
				missionBegin = new MissionBegin;
				missionBegin->init();
			}

			missionBegin->beginZone();
			active = TRUE;
			setLogisticsState( log_SPLASH );
			userInput->mouseOn();
			userInput->setMouseCursor( mState_NORMAL );

		}
		break;
	}
}	

//----------------------------------------------------------------------------------
void Logistics::stop (void)
{
	switch (prevState)
	{
		case log_SPLASH:
			break;
			
		case log_RESULTS:
			break;
	}
		
	active = FALSE;
}	

//----------------------------------------------------------------------------------
long Logistics::update (void)
{
	//MUST do this every frame.  The movie movies will play wrong, otherwise!!

	if ( bMovie )
	{
		userInput->mouseOff();
		if (userInput->getKeyDown(KEY_SPACE) || userInput->getKeyDown(KEY_ESCAPE) || userInput->getKeyDown(KEY_LMOUSE))
		{
			bMovie->stop();
		}

		bool result = bMovie->update();
		if (result)
		{
			if ( LogisticsData::instance->campaignOver() && (strnicmp(bMovie->getMovieName(),"credits",7) == 0) )
			{
				missionBegin->beginSplash();
			}

			//Check if we are cinema5.  If we are, spool up the credits and play those.
			// Otherwise, same as before Mr. Sulu.  Stay with him...
			if (strnicmp(bMovie->getMovieName(),"cinema5",7) == 0)
			{
				//OLD Movie's Over.
				//Whack it.
				delete bMovie;
				bMovie = NULL;

				FullPathFileName path;
				path.init( moviePath, "credits", ".bik" );

				RECT movieRect;
				movieRect.top = 0;
				movieRect.left = 0;
				movieRect.right = Environment.screenWidth;
				movieRect.bottom = 600;

				bMovie = new MC2Movie;
				bMovie->init(path,movieRect,true);
				soundSystem->playDigitalMusic(33);
			}
			else
			{
				//Movie's Over.
				//Whack it.
				delete bMovie;
				bMovie = NULL;

				soundSystem->playDigitalMusic(LogisticsData::instance->getCurrentMissionTune());
				userInput->mouseOn();	
			}
		}

		return logisticsState;
	}

	//Don't keep setting the mouse cursor to logistics cursor during mission!!
	if (!mission->isActive())
		userInput->setMouseCursor( mState_LOGISTICS );
	//Stop doing this every frame.  ONLY turn it on when we need to!!

	if ( logisticsState == log_RESULTS )
	{
		if ( missionResults )
			missionResults->update();

		if ( missionResults->isDone() )
		{
			missionResults->end();
			if ( !justStartMission && !MPlayer )
			{
					logisticsState = log_SPLASH;
					if ( !missionBegin )
					{
						missionBegin = new MissionBegin();
						missionBegin->init( );
					}
					
					missionBegin->begin();

 					if ( lastMissionResult == mis_PLAYER_LOST_BIG && !MPlayer )
					{
						if ( LogisticsData::instance->skipLogistics()  )
						{
							if ( LogisticsData::instance->showChooseMission() )
								missionBegin->begin();

							else
								missionBegin->beginSplash();
						}
						else
							missionBegin->setToMissionBriefing();
					}
					else
					{
						const char* videoName = LogisticsData::instance->getCurrentBigVideo();

						if ( videoName )
						{
							playFullScreenVideo( videoName );
						}
					}
				
			}
			else if ( MPlayer )
			{
				logisticsState = log_SPLASH;
				if ( !missionBegin )
				{
					missionBegin = new MissionBegin();
					missionBegin->init( );
				}

				if ( MPlayer->hostLeft || MPlayer->commanderID < 0 )
				{
					missionBegin->beginSplash( NULL );
					MPlayer->closeSession();
				}
				else
					missionBegin->restartMPlayer(NULL);


				
			}
			else
				quitGame = true;
		}
	}
	//Used to start mission from command line, bypassing logistics
	else if (logisticsState == log_STARTMISSIONFROMCMDLINE)
	{
		
	}
	else if ( logisticsState != log_DONE )
	{
		if ( missionBegin )
		{
			const char* pVid =  missionBegin->update();
			if ( pVid && (0 != strcmp("", pVid)) )
			{
				playFullScreenVideo( pVid );
				return logisticsState;
			}
			if ( missionBegin->readyToLoad() && !bMissionLoaded )
			{
				missionBegin->end();	//Called to clean up ABL!!

				lastMissionResult = 0;

				if (!beginMission(0, 0, 0)) {
					missionBegin->beginAtConnectionScreen();
					return(logisticsState);
				}
				bMissionLoaded = true;
			}
			if ( missionBegin->isDone() )
			{				
				logisticsState = log_DONE;
			}
		}
	}

	return(logisticsState);
}	
		
//----------------------------------------------------------------------------------
void Logistics::render (void)
{
	if (bMovie)
	{
		bMovie->render();
	}
	else if ( active )
	{
		if ( logisticsState == log_RESULTS )
			missionResults->render();
		else if ( logisticsState == log_SPLASH && missionBegin )
			missionBegin->render();
	}
}

//-----------------------------------------------------------------------------

extern unsigned long MultiPlayTeamId;
extern unsigned long MultiPlayCommanderId;

int _stdcall Logistics::beginMission(void*, int, void*[])
{

	if (MPlayer)
		MPlayer->setMode(MULTIPLAYER_MODE_LOADING);

	char commandersToLoad[MAX_MC_PLAYERS][3] = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}, {5, 5, 5}, {6, 6, 6}, {7, 7, 7}};
	long missionLoadType = LogisticsData::instance->skipLogistics() ? 
							MISSION_LOAD_SP_QUICKSTART : MISSION_LOAD_SP_LOGISTICS;
	if (MPlayer) {
		//---------------------------
		// Calc valid commanderIDs...
		long curCommanderID = 0;
		for (long CID = 0; CID < MAX_MC_PLAYERS; CID++) {
			MPlayer->availableCIDs[CID] = true;
			if (MPlayer->playerInfo[CID].player && (MPlayer->playerInfo[CID].commanderID > -1)) {
				if (CID != curCommanderID) {
					long oldCommanderID = CID;
					MPlayer->playerInfo[CID].commanderID = curCommanderID;
					memcpy(&MPlayer->playerInfo[curCommanderID], &MPlayer->playerInfo[CID], sizeof(MC2Player));
					MPlayer->playerInfo[CID].player = NULL;
					MPlayer->playerInfo[CID].commanderID = -1;
					for (long j = 0; j < MAX_MC_PLAYERS; j++)
						if (MPlayer->playerList[j].player == MPlayer->playerInfo[curCommanderID].player)
							MPlayer->playerList[j].commanderID = curCommanderID;
					if (oldCommanderID == MPlayer->commanderID)
						MPlayer->commanderID = curCommanderID;
				}
				MPlayer->availableCIDs[curCommanderID] = false;
				curCommanderID++;;
			}
		}
		//----------------------
		// Calc valid teamIDs...
		long curTeamID = 0;
		for (long teamID = 0; teamID < MAX_MC_PLAYERS; teamID++) {
			bool teamFound = false;
			for (long i = 0; i < MAX_MC_PLAYERS; i++)
				if (MPlayer->playerInfo[i].player && (MPlayer->playerInfo[i].team == teamID)) {
					MPlayer->playerInfo[i].team = curTeamID;
					teamFound = true;
				}
			if (teamFound)
				curTeamID++;
		}
		if (MPlayer->isHost()) {
			// Determine drop zone order here...
			char dropZoneList[8];
			char hqs[MAX_TEAMS];
			if (MPlayer->missionSettings.missionType == MISSION_TYPE_OTHER) {
				bool goodToLoad = mission->calcComplexDropZones((char*)(const char*)LogisticsData::instance->getCurrentMission(), dropZoneList);
				if (!goodToLoad)
					STOP(("Logisitics.beginMission: teams do not match up for complex mission"));
				for (long i = 0; i < MAX_TEAMS; i++)
					hqs[i] = i;
				}
			else
				MPlayer->calcDropZones(dropZoneList, hqs);
			if (MPlayer->missionSettings.quickStart)
				for (long i = 0; i < MAX_MC_PLAYERS; i++) {
					MPlayer->commandersToLoad[i][0] = (long)dropZoneList[i]; //MPlayer->playerInfo[i].commanderID;
					MPlayer->commandersToLoad[i][1] = (long)(dropZoneList[i] > -1) ? MPlayer->playerInfo[dropZoneList[i]].team : 0;
					MPlayer->commandersToLoad[i][2] = hqs[i];
				}
			else
				for (long i = 0; i < MAX_MC_PLAYERS; i++) {
					MPlayer->commandersToLoad[i][0] = dropZoneList[i]; //-1;
					MPlayer->commandersToLoad[i][1] = (dropZoneList[i] > -1) ? MPlayer->playerInfo[dropZoneList[i]].team : 0; //-1;
					MPlayer->commandersToLoad[i][2] = hqs[i];
				}
			MPlayer->sendMissionSetup(0, 0, NULL);
		}
		if (!MPlayer->waitTillStartLoading()) {
			// SERVER DROPPED
			return(0);
		}
		if (MPlayer->commandersToLoad[0][0] < -1)
			PAUSE(("Logistics.beginMission: bad commandersToLoad"));
		for (long i = 0; i < MAX_MC_PLAYERS; i++) {
			commandersToLoad[i][0] = MPlayer->commandersToLoad[i][0];
			commandersToLoad[i][1] = MPlayer->commandersToLoad[i][1];
			commandersToLoad[i][2] = MPlayer->commandersToLoad[i][2];
		}
		if (MPlayer->missionSettings.quickStart) {
			MultiPlayTeamId = MPlayer->playerInfo[MPlayer->commanderID].team;
			if (MultiPlayTeamId < 0)
				STOP(("Bad commanderID"));
			MultiPlayCommanderId = MPlayer->commanderID;
			if (MultiPlayCommanderId < 0)
				STOP(("Bad commanderID"));
			missionLoadType = MISSION_LOAD_MP_QUICKSTART;
			}
		else {
			MultiPlayTeamId = MPlayer->playerInfo[MPlayer->commanderID].team;
			MultiPlayCommanderId = MPlayer->commanderID;
			missionLoadType = MISSION_LOAD_MP_LOGISTICS;
		}
		long maxTeam = -1;
		for (i = 0; i < MAX_MC_PLAYERS; i++)
			if (MPlayer->playerInfo[i].team > maxTeam)
				maxTeam = MPlayer->playerInfo[i].team;
		MPlayer->numTeams = maxTeam + 1;
		}
	else if (missionLoadType == MISSION_LOAD_SP_LOGISTICS) {
		commandersToLoad[0][0] = -1;
		commandersToLoad[0][1] = -1;
		commandersToLoad[0][2] = -1;
	}
	else {
		commandersToLoad[0][0] = 0;
		commandersToLoad[0][1] = 0;
		commandersToLoad[0][2] = -1;
	}

	if (mission)
		mission->destroy();

	long numPlayers = 1;
	if ( MPlayer )
		MPlayer->getPlayers(numPlayers);
	long numMoversPerCommander[MAX_MC_PLAYERS] = {12, 12, 12, 9, 7, 6, 5, 4};
	Stuff::Vector3D dropZoneList[255]; // ubsurdly large, but sometimes we overrun this.
	long dropZoneID = 0;
	if (MPlayer) {
		//dropZoneID = MPlayer->commanderID;
		for (long i = 0; i < MAX_MC_PLAYERS; i++)
			if (commandersToLoad[i][0] == MPlayer->commanderID) {
				dropZoneID = i;
				break;
			}
		useUnlimitedAmmo = MPlayer->missionSettings.unlimitedAmmo;
	}

	mission->init((char*)(const char*)LogisticsData::instance->getCurrentMission(), missionLoadType, dropZoneID, dropZoneList, commandersToLoad, numMoversPerCommander[numPlayers - 1]);

	LogisticsData::instance->rpJustAdded = 0;

	if (MPlayer) {
		if (missionLoadType == MISSION_LOAD_MP_LOGISTICS) {
			EList< LogisticsMech*, LogisticsMech* > list;
			LogisticsData::instance->getForceGroup(list);

			long dropZoneIndex = 0;
			long numMechs = 0;
			for (EList< LogisticsMech*, LogisticsMech* >::EIterator iter = list.Begin(); !iter.IsDone(); iter++) {
				numMechs++;
				if ( !(*iter)->getPilot() )
					continue;
				CompressedMech mechData;
				mechData.lastMech = (list.Count() == numMechs);
				mechData.objNumber =  (*iter)->getFitID();
				mechData.commanderID = MPlayer->commanderID;
				mechData.baseColor = MPlayer->colors[MPlayer->playerInfo[MPlayer->commanderID].baseColor[BASECOLOR_TEAM]];
				mechData.highlightColor1 = MPlayer->colors[MPlayer->playerInfo[MPlayer->commanderID].stripeColor];
				mechData.highlightColor2 = MPlayer->colors[MPlayer->playerInfo[MPlayer->commanderID].stripeColor];
				strcpy(mechData.pilotFile, (*iter)->getPilot()->getFileName());
				strcpy(mechData.mechFile, (*iter)->getFileName());
				strcpy(mechData.variantName, (*iter)->getName());
				mechData.variantNum = (*iter)->getVariant()->getFileID();
				mechData.cBills = (*iter)->getVariant()->getCost();
				mechData.pos[0] = dropZoneList[dropZoneIndex].x;
				mechData.pos[1] = dropZoneList[dropZoneIndex++].y;
				mechData.designerMech = (*iter)->getVariant()->isDesignerMech();
				mechData.numComponents = (*iter)->getComponentCount();
				if (mechData.numComponents)	{
					long* componentList = (long*)systemHeap->Malloc(sizeof(long) * mechData.numComponents);
					long otherCount = mechData.numComponents;
					(*iter)->getComponents(otherCount, componentList);
					if (otherCount != mechData.numComponents)
						STOP(("Heidi's getComponentCount does not agree with count returned from getComponents"));
					for (long i = 0; i < mechData.numComponents; i++)
						mechData.components[i] = (unsigned char)componentList[i];
				}
				MPlayer->sendMissionSetup(0, 1, &mechData);
			}

			if (!MPlayer->waitTillMechDataReceived()) {
				// SERVER DROPPED
				mission->destroy();
				return(0);
			}
	
			ObjectManager->numMechs = 0;
			ObjectManager->numVehicles = 0;
			for (long i = 0; i < MAX_MC_PLAYERS; i++) {
				if (MPlayer->mechDataReceived[i]) {
					for (long j = 0; j < 12; j++) {
						if (MPlayer->mechData[i][j].objNumber > -1) {
							MoverInitData data;
							memset(&data, 0, sizeof(MoverInitData));
							data.objNumber = MPlayer->mechData[i][j].objNumber;
							data.rosterIndex = 255;
							data.controlType = 2;
							data.controlDataType = 1;
							data.position.x = MPlayer->mechData[i][j].pos[0];
							data.position.y = MPlayer->mechData[i][j].pos[1];
							data.position.z = 0.0;
							data.rotation = 0;
							data.teamID = MPlayer->playerInfo[MPlayer->mechData[i][j].commanderID].team;
							data.commanderID = MPlayer->mechData[i][j].commanderID;
							data.baseColor = MPlayer->mechData[i][j].baseColor;
							data.highlightColor1 = MPlayer->mechData[i][j].highlightColor1;
							data.highlightColor2 = MPlayer->mechData[i][j].highlightColor2;
							data.gestureID = 2;
							data.active = 1;
							data.exists = 1;
							data.capturable = 0;
							data.icon = 0;
							
							strcpy(data.pilotFileName, MPlayer->mechData[i][j].pilotFile);
							strcpy(data.brainFileName, "pbrain");
							strcpy(data.csvFileName, MPlayer->mechData[i][j].mechFile);
							data.numComponents = MPlayer->mechData[i][j].numComponents;
							for (long k = 0; k < MPlayer->mechData[i][j].numComponents; k++)
								data.components[k] = MPlayer->mechData[i][j].components[k];
							long moverHandle = mission->addMover(&data);
							if (moverHandle < 1)
								STOP(("Logistics.beginMission: unable to addMover"));
							MoverPtr mover = (MoverPtr)ObjectManager->get(moverHandle);
							if (!mover)
								STOP(("Logistics.beginMission: NULL mover"));
							if (mover->getObjectClass() != BATTLEMECH)
								STOP(("Logistics.beginMission: not a mech"));
							((BattleMech*)mover)->cBills = MPlayer->mechData[i][j].cBills;
							strcpy(((BattleMech*)mover)->variantName, MPlayer->mechData[i][j].variantName);
							data.variant = MPlayer->mechData[i][j].variantNum;
						}
					}
				}
			}
		}
		else // gotta update pilot availability
		{
			long count = 256;
			LogisticsPilot* pilots[256];
			LogisticsData::instance->getPilots( pilots, count );

			for ( int i = 0; i < count; i++ )
			{
				pilots[i]->setUsed(0);
			}

			Team* pTeam = Team::home;

			if ( pTeam )
			{
				for ( i = pTeam->getRosterSize() - 1; i > -1; i-- )
				{
					Mover* pMover = (Mover*)pTeam->getMover( i );
					if ( pMover && pMover->getCommander()->getId() == Commander::home->getId() )
					{
						LogisticsPilot* pPilot = LogisticsData::instance->getPilot( pMover->getPilot()->getName() );
						if ( pPilot )
							pPilot->setUsed( true );
					}
				}
			}
					

		}
		
		CompressedMech mechData;
		mechData.commanderID = MPlayer->commanderID;
		MPlayer->sendMissionSetup(0, 2, &mechData);
		if (!MPlayer->waitTillMissionLoaded()) {
			// SERVER DROPPED
			mission->destroy();
			return(0);
		}

		}
	else if (missionLoadType == MISSION_LOAD_SP_LOGISTICS) {
		EList< LogisticsMech*, LogisticsMech* > list;
		LogisticsData::instance->getForceGroup( list );

		float rotation = list.Count() ? 360.f/list.Count() : 0.f;

		MoverInitData data;
		memset( &data, 0, sizeof( data ) );
		data.rosterIndex = 255;
		data.controlType = 2;
		data.controlDataType = 1;
		data.position.Zero(); // need to get the drop zone location
		data.rotation = 0;
		data.teamID = Team::home->getId();
		data.commanderID = Team::home->getId();
		data.active = 1;
		data.exists = 1;
		data.capturable = 0;
		data.baseColor = prefs.baseColor;
		data.highlightColor1 = prefs.highlightColor;
		data.highlightColor2 = prefs.highlightColor;
		

		strcpy( data.pilotFileName, "pmw00031" );
		strcpy( data.brainFileName, "pbrain" );
	
		Stuff::Vector3D vector;
		vector.x = 0.f;
		vector.y = 1.f;
		vector.z = 0.f;
		Stuff::Vector3D scaled;
		for ( EList< LogisticsMech*, LogisticsMech* >::EIterator iter = list.Begin(); !iter.IsDone(); iter++) {
			scaled = vector;
			scaled *= 128.;
			data.position.Add(dropZoneList[0], scaled);
			//data.position = dropZoneList[dropZoneIndex++];
			data.objNumber = (*iter)->getFitID();
			strcpy( data.csvFileName, (*iter)->getFileName() );
			if ( !(*iter)->getPilot() )
				continue;
			strcpy( data.pilotFileName, (*iter)->getPilot()->getFileName() );
			data.overrideLoadedPilot = true;
			data.gunnerySkill = (*iter)->getPilot()->getGunnery();
			data.pilotingSkill = (*iter)->getPilot()->getPiloting();

			memcpy(data.specialtySkills,(*iter)->getPilot()->getSpecialtySkills(),sizeof(bool) * NUM_SPECIALTY_SKILLS);
			if ( !(*iter)->getVariant()->isDesignerMech())
			{		
				data.variant = 0;
				mission->addMover( &data, (*iter));
			}
			else
			{
				data.variant = (*iter)->getVariant()->getFileID();
				mission->addMover( &data );
			}
			Rotate( vector, rotation );
		}
	}

/*	if (MPlayer) {
		if (MPlayer->isHost())
			MPlayer->serverCID = MPlayer->commanderID;
		else {
			for (long i = 0; i < MAX_MC_PLAYERS; i++)
				if (MPlayer->serverPlayer && (MPlayer->playerList[i].player == MPlayer->serverPlayer))
					MPlayer->serverCID = MPlayer->playerList[i].commanderID;
			if (MPlayer->serverCID == -1)
				STOP(("Logistics.beginMission: bad serverCID"));
		}
	}
*/
	mission->missionInterface->initMechs();

	eye->activate();
	eye->update();
	mission->start();
	mission->update(); // force this, so we don't render first

	if (MPlayer) {
		CompressedMech mechData;
		mechData.commanderID = MPlayer->commanderID;
		MPlayer->sendMissionSetup(0, 4, &mechData);
		if (!MPlayer->waitTillMissionSetup()) {
			// SERVER DROPPED
				mission->destroy();
				return(0);
		}
		MPlayer->setMode(MULTIPLAYER_MODE_MISSION);
	}

	if (MPlayer && MPlayer->hostLeft) {
		mission->destroy();
		return(0);
	}

	return(1);
}

int Logistics::DoBeginMission()
{

#ifndef TEST_SHELL
//---------------------------
// Networking startup code...
#if 0
	/*MPlayer->launchedFromLobby =*/ gos_NetCheckLobby();
	Environment.NetworkGame = TRUE;
	if (!MPlayer->launchedFromLobby) {
		if (!gos_NetStartGame(0)) {
			//gos_TerminateApplication();
			return;
		}
	}
#endif				

if (!MPlayer) {
	logistics->setLogisticsState(log_DONE);
	soundSystem->playBettySample(BETTY_NEW_CAMPAIGN);
}

#endif	//TEST_SHELL

	return 0;

}

void Logistics::initializeLogData()
{
	LogisticsData::instance->removeMechsInForceGroup();

	LogisticsData::instance->init();
	Team* pTeam = Team::home;

	if ( pTeam )
	{
		for ( int i = pTeam->getRosterSize() - 1; i > -1; i-- )
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			LogisticsPilot* pPilot = LogisticsData::instance->getPilot(pMover->getPilot()->getName());

			unsigned long base, highlight1, highlight2;
			((Mech3DAppearance*)pMover->getAppearance())->getPaintScheme( highlight1, 
				highlight2, base );
			
			
			if ( pMover->getObjectType()->getObjectTypeClass() == BATTLEMECH_TYPE )
			{
				LogisticsVariant* pVar = LogisticsData::instance->getVariant( ((BattleMech*)pMover)->variantName );
				LogisticsData::instance->addMechToInventory( 
					pVar, 1, pPilot,
					base, highlight1, highlight2 );
			}
		}
	}
}

void Logistics::playFullScreenVideo( const char* fileName )
{
	if ( !fileName || !fileName[0])
		return;

	FullPathFileName path;
	path.init( moviePath, fileName, ".bik" );

	RECT movieRect;
	movieRect.top = 100;
	movieRect.left = 0;
	movieRect.right = Environment.screenWidth;
	movieRect.bottom = 500;

	bMovie = new MC2Movie;
	bMovie->init(path,movieRect,true);

	soundSystem->stopDigitalMusic();
}

void Logistics::setResultsHostLeftDlg( const char* pName )
{
	if ( missionResults && logisticsState == log_RESULTS )
	{
		missionResults->setHostLeftDlg( pName );
	}
}
//----------------------------------------------------------------------------------
