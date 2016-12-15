//******************************************************************************************
//	saveload.cpp - This file contains the mission Save Load class code
//		Missions are what scenarios were in MechCommander 1.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------------------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef MOVEMGR_H
#include"movemgr.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef SOUNDS_H
#include"sounds.h"
#endif

#ifndef COLLSN_H
#include"collsn.h"
#endif

#ifndef CMPONENT_H
#include"cmponent.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

#ifndef MECH_H
#include"mech.h"
#endif

#ifndef GVEHICL_H
#include"gvehicl.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

#ifndef GROUP_H
#include"group.h"
#endif

#ifndef GAMECAM_H
#include"gamecam.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
#endif

#ifndef WEATHER_H
#include"weather.h"
#endif

#ifndef GATE_H
#include"gate.h"
#endif

#ifndef LOGISTICSDATA_H
#include"logisticsdata.h"
#endif

#ifndef LOGISTICSPILOT_H
#include"logisticspilot.h"
#endif

#ifndef GAMELOG_H
#include"gamelog.h"
#endif

#ifndef CELLIP_H
#include"cellip.h"
#endif

#ifndef LOADSCREEN_H
#include"loadscreen.h"
#endif

#include "../resource.h"
#include<gameos.hpp>
#ifndef LINUX_BUILD
#include<ddraw.h>
#else
struct DDSURFACEDESC2 {
};
#endif

//----------------------------------------------------------------------------------------------------
// Globals
#define DEFAULT_SKY			1
extern float loadProgress;

void MouseTimerInit();
void MouseTimerKill();
void ProgressTimer(	RECT& WinRect,DDSURFACEDESC2& mouseSurfaceDesc );

extern volatile bool mc2IsInDisplayBackBuffer;
extern volatile bool mc2UseAsyncMouse;
extern volatile bool mc2IsInMouseTimer;
extern void (*AsynFunc)(RECT& WinRect,DDSURFACEDESC2& mouseSurfaceDesc );

#define USE_ABL_LOAD

void initABL (void);
void closeABL (void);

extern float WeaponRanges[NUM_WEAPON_RANGE_TYPES][2];
extern float OptimalRangePoints[NUM_WEAPON_RANGE_TYPES];
extern bool OptimalRangePointInRange[NUM_WEAPON_RANGE_TYPES][3];

extern unsigned long missionHeapSize;

//--------------------------------------------------
// Game System Constants -- Declarations here.
extern float maxVisualRange;
extern float MaxVisualRadius;
extern float fireVisualRange;
extern float WeaponRange[];		   
extern float DefaultAttackRange;   
extern float baseSensorRange;
extern long visualRangeTable[];

extern unsigned char godMode;	   
extern unsigned char revealTacMap; 
extern unsigned char footPrints;   
extern bool CantTouchThis;

extern long tonnageDivisor;		   
extern long resourcesPerTonDivided;

extern GameObjectPtr DebugGameObject[];

extern float MineDamage;		   
extern float MineSplashDamage;	   
extern float MineSplashRange;	   
extern long MineExplosion;		   
extern long MineLayThrottle;	   
extern long MineSweepThrottle;	   
extern float StrikeWaitTime;	   
extern float StrikeTimeToImpact;   
extern float MineWaitTime;		   

extern long totalSmokeSpheres;	   
extern long totalSmokeShapeSize;   

extern long maxFiresBurning;	   
extern float maxFireBurnTime;	   

extern bool force64MB;			   
extern bool force32Mb;			   

extern float InfluenceRadius;	   
extern float InfluenceTime;		   

extern float BaseHeadShotElevation;

#ifndef FINAL
extern float CheatHitDamage;
#endif

void InitDifficultySettings (FitIniFile *gameSystemFile);

extern long GameVisibleVertices;

void GetBlockedDoorCells (int moveLevel, int door, char* openCells);
void PlaceStationaryMovers (MoveMap* map);
void PlaceMovers (void);

extern GameObjectFootPrint* tempSpecialAreaFootPrints;
extern long tempNumSpecialAreas;
extern GameLog* CombatLog;

bool IsGateDisabled (int objectWID);
bool IsGateOpen (int objectWID);

DWORD elfHash (char *name);

extern char versionStamp[];
//----------------------------------------------------------------------------------------------------
// Save.
void Part::Save (FitIniFilePtr file, long partNum)
{
	char partId[1024];
	sprintf(partId,"Part%d", (int)partNum+1);

	file->writeBlock(partId);

	file->writeIdLong("ObjectWID",objectWID);
	file->writeIdULong("ObjectNumber", objNumber);
	file->writeIdULong("BaseColor",baseColor);
	file->writeIdULong("HighlightColor1",highlightColor1);
	file->writeIdULong("HighlightColor2",highlightColor2);
	file->writeIdLong("Active",active);
	file->writeIdLong("Exists",exists);
	file->writeIdBoolean("Destroyed",destroyed);
	file->writeIdFloat("PositionX",position.x);
	file->writeIdFloat("PositionY",position.y);
	file->writeIdFloat("PositionZ",position.z);
	file->writeIdFloat("Velocity",velocity);
	file->writeIdFloat("Rotation",rotation);
	file->writeIdLong("GestureId",gestureId);
	file->writeIdChar("Alignment",alignment);
	file->writeIdChar("TeamId",teamId);
	file->writeIdLong("CommanderId",commanderID);
	file->writeIdLong("SquadId",squadId);
	file->writeIdChar("MyIcon",myIcon);
	file->writeIdULong("ControlType",controlType);
	file->writeIdULong("ControlDataType",controlDataType);
	file->writeIdString("ObjectProfile",profileName);
	file->writeIdULong("Pilot",pilot);
	file->writeIdBoolean("Captureable",captureable);
	file->writeIdULong("VariantNum",variantNum);
}

void Mission::save (const char *saveFileName)
{
	//Check for sufficient hard Drive space on drive game is running from
	char currentPath[1024];
	gos_GetCurrentPath( currentPath, 1023 );
	__int64 driveSpace = gos_GetDriveFreeSpace(currentPath);
	if (driveSpace < (5 * 1024 * 1024))
	{
		soundSystem->playDigitalSample(INVALID_GUI);
		return;
	}

	userInput->mouseOff();

	DWORD oldXLoc = LoadScreen::xProgressLoc;
	DWORD oldYLoc = LoadScreen::yProgressLoc;

	LoadScreen::xProgressLoc = 0;
	LoadScreen::yProgressLoc = 0;

	bool turnOffAsyncMouse = mc2UseAsyncMouse;
	if ( !mc2UseAsyncMouse )
		MouseTimerInit();
	mc2UseAsyncMouse = true;			
	AsynFunc = ProgressTimer;

	//Get the campaign name for the campaign we are currently playing.
	EString campaignName = LogisticsData::instance->getCampaignName();
	const char *cmpName = campaignName.Data();

	//Open the Save File.
	// Assume path is correct when we get here.
	PacketFile saveFile;
	long result = saveFile.create((char *)saveFileName);

	loadProgress = 1.0f;

	//A thousand packets ought to get it!!
	saveFile.reserve(20000);
	DWORD currentPacket = 0;

	saveFile.writePacket(currentPacket,(MemoryPtr)versionStamp,strlen(versionStamp)+1,STORAGE_TYPE_RAW);
	currentPacket++;

	saveFile.writePacket(currentPacket,(MemoryPtr)cmpName,strlen(cmpName)+1, STORAGE_TYPE_RAW);
	currentPacket++;

	loadProgress = 3.0f;

	//Mission Settings
	// Create a temp fit file and shove it into Packet 0.
	// COmpress the Wee-wee out of it!!
	char tmpString[2048];
	sprintf(tmpString,"%d",timeGetTime());

	FullPathFileName tmpName;
	tmpName.init(savePath,tmpString,".fti");

	FitIniFile tempFile;
	result = tempFile.create(tmpName);

	if (eye)
		eye->save( &tempFile );

	loadProgress = 5.0f;

	if (land)
		land->save( &tempFile );

	loadProgress = 25.0f;

	tempFile.writeBlock( "MissionSettings" );
	tempFile.writeIdString( "MissionName"						, missionFileName);
	tempFile.writeIdFloat( "TimeLimit"							, m_timeLimit );
	tempFile.writeIdLong( "ResourcePoints"						, LogisticsData::instance->getResourcePoints() );
	tempFile.writeIdUChar( "scenarioTuneNum"					, missionTuneNum );

	tempFile.writeIdLong("OperationId"							, operationId);
	tempFile.writeIdLong("MissionId"							, missionId);
	tempFile.writeIdLong("ABLScriptHandle"						, missionBrain->getRealId());
	tempFile.writeIdBoolean("Active"							, active);
	tempFile.writeIdFloat("ActualTime"							, actualTime);
	tempFile.writeIdFloat("RunningTime"							, runningTime);
	tempFile.writeIdFloat("ScenarioTime"						, scenarioTime);
	tempFile.writeIdFloat("DropZoneX"							, dropZone.x);
	tempFile.writeIdFloat("DropZoneY"							, dropZone.y);
	tempFile.writeIdFloat("DropZoneZ"							, dropZone.z);

	tempFile.writeBlock("Parts");
	tempFile.writeIdULong("NumParts",numParts);

	//Must save all of the part data.  Not sure if its ever used after INIT!  Better safe then sorry.
	for (long i=0;i<numParts;i++)
		parts[i].Save(&tempFile,i);

	loadProgress = 35.0f;

	//Save Support Palette data at least from here.
	// VTOL current State, IF Needed.
	// Karnov current State, IF Needed.
	missionInterface->Save( &tempFile );

	loadProgress = 40.0f;

	if (weather)
		weather->save( &tempFile );

	loadProgress = 43.0f;

	tempFile.writeBlock("TheSky");
	tempFile.writeIdLong("SkyNumber"							, theSkyNumber);
	
	Team::home->objectives.Save( &tempFile );

	loadProgress = 50.0f;

	tempFile.writeBlock( "Script" );
	tempFile.writeIdString( "ScenarioScript"					, missionScriptName );

	tempFile.close();

	File dmyFile;
	result = dmyFile.open(tmpName);
	DWORD fileSz = dmyFile.fileSize();
	MemoryPtr tmpRAM = (MemoryPtr)malloc(fileSz);
	dmyFile.read(tmpRAM,fileSz);

	saveFile.writePacket(currentPacket,tmpRAM,fileSz,STORAGE_TYPE_ZLIB);
	currentPacket++;

	dmyFile.close();
	DeleteFile(tmpName);
	free(tmpRAM);
	//---------------------------

	//Terrain - NOT needed, the terrain does not change.
//	land->save( &saveFile, currentPacket, true );
//	currentPacket++;
	//---------------------------

	//Move Data - Needed, changes ALOT
	currentPacket += MOVE_saveData( &saveFile, currentPacket );
	//---------------------------

	loadProgress = 75.0f;

	//ABL
	sprintf(tmpString,"%d",timeGetTime());

	tmpName.init(savePath,tmpString,".abl");

	ABLFile ablFile;
	ablFile.create(tmpName);

	ABLi_saveEnvironment( &ablFile );

	ablFile.close();

	//Create a dummy file to stick in the packet file
	result = dmyFile.open(tmpName);
	fileSz = dmyFile.fileSize();
	tmpRAM = (MemoryPtr)malloc(fileSz);
	dmyFile.read(tmpRAM,fileSz);

	saveFile.writePacket(currentPacket,tmpRAM,fileSz,STORAGE_TYPE_ZLIB);
	currentPacket++;

	dmyFile.close();
	DeleteFile(tmpName);
	free(tmpRAM);
	//---------------------------

	loadProgress = 80.0f;

	//MechWarrior Manager
	// Save Number of Warriors.
	// Call Save for each Warrior on list.
	currentPacket = MechWarrior::Save( &saveFile, currentPacket );

	loadProgress = 85.0f;

	//Team Manager
	// Save Number of Teams
	// Save which number is HOME
	// Call Save for each team.
	currentPacket = Team::Save( &saveFile, currentPacket );

	//Object Manager
	// Save numbers of objects in all object lists in game
	// Save ClassID of objects IN the list.
	// For each object in each list:
	//		If NULL, save a magic Number to tell me this entry is NULL.
	//		If NOT NULL, 
	// 			Call Save DOWN the heirarchy.
	//
	currentPacket = ObjectManager->Save( &saveFile, currentPacket );

	loadProgress = 95.0f;

	//Commander Manager
	// Save Number of Commanders.
	// Save which number is HOME.
	// Call save for each commander.
	currentPacket = Commander::Save( &saveFile, currentPacket );

	//Logistics Manager
	// Save the Logistics Data
	FitIniFile tmpSaveFile;
	tmpSaveFile.create(tmpName);

	LogisticsData::instance->save(tmpSaveFile);

	tmpSaveFile.close();

	loadProgress = 99.0f;

	//Create a dummy file to stick in the packet file
	result = dmyFile.open(tmpName);
	fileSz = dmyFile.fileSize();
	tmpRAM = (MemoryPtr)malloc(fileSz);
	dmyFile.read(tmpRAM,fileSz);

	saveFile.writePacket(currentPacket,tmpRAM,fileSz,STORAGE_TYPE_ZLIB);
	currentPacket++;

	dmyFile.close();

	DeleteFile(tmpName);
	free(tmpRAM);
	//---------------------------

	//-------------------------------------------------------------------------------------------
	// Save the CurrentMission number in logistics cause Heidi don't save it with logisticsData.
	long currentMissionNum = LogisticsData::instance->getCurrentMissionNum();
	saveFile.writePacket(currentPacket,(MemoryPtr)&currentMissionNum,4,STORAGE_TYPE_RAW);
	currentPacket++;

	//-------------------------------------------------------------------------------------------
	saveFile.writePacket(currentPacket,(MemoryPtr)"END",4,STORAGE_TYPE_RAW);
	currentPacket++;

	saveFile.close();

	//YIKES!!  We could be checking the if before the null and executing after!!  Block the thread!
	//Wait for thread to finish.
	while (mc2IsInMouseTimer)
		;

	//ONLY set the mouse BLT data at the end of each update.  NO MORE FLICKERING THEN!!!
	// BLOCK THREAD WHILE THIS IS HAPPENING
	mc2IsInDisplayBackBuffer = true;

	AsynFunc = NULL;
	mc2UseAsyncMouse = turnOffAsyncMouse;
	if ( !mc2UseAsyncMouse)
		MouseTimerKill();

	mc2IsInDisplayBackBuffer = false;

	LoadScreen::xProgressLoc = oldXLoc;
	LoadScreen::yProgressLoc = oldYLoc;

	userInput->mouseOn();
}

//----------------------------------------------------------------------------------------------------
// Load.
void Part::Load (FitIniFilePtr file, long partNum)
{
	char partId[1024];
	sprintf(partId,"Part%d", (int)partNum);

	long result = file->seekBlock(partId);
	if (result != NO_ERR)
		STOP(("Part Number %d missing from In-Mission Save",partNum));

	result = file->readIdInt("ObjectWID", objectWID);
	if (result != NO_ERR)
		STOP(("Part %d ObjectWID missing from In-Mission Save",partNum));

	result = file->readIdULong("ObjectNumber", objNumber);
	if (result != NO_ERR)
		STOP(("Part %d ObjectNumber missing from In-Mission Save",partNum));

	result = file->readIdULong("BaseColor",baseColor);
	if (result != NO_ERR)
		STOP(("Part %d BaseColor missing from In-Mission Save",partNum));

	result = file->readIdULong("HighlightColor1",highlightColor1);
	if (result != NO_ERR)
		STOP(("Part %d HighlightColor1 missing from In-Mission Save",partNum));

	result = file->readIdULong("HighlightColor2",highlightColor2);
	if (result != NO_ERR)
		STOP(("Part %d HighlightColor2 missing from In-Mission Save",partNum));

	result = file->readIdLong("Active",active);
	if (result != NO_ERR)
		STOP(("Part %d Active missing from In-Mission Save",partNum));

	result = file->readIdLong("Exists",exists);
	if (result != NO_ERR)
		STOP(("Part %d Exists missing from In-Mission Save",partNum));

	result = file->readIdBoolean("Destroyed",destroyed);
	if (result != NO_ERR)
		STOP(("Part %d Destroyed missing from In-Mission Save",partNum));

	result = file->readIdFloat("PositionX",position.x);
	if (result != NO_ERR)
		STOP(("Part %d PositionX missing from In-Mission Save",partNum));

	result = file->readIdFloat("PositionY",position.y);
	if (result != NO_ERR)
		STOP(("Part %d PositionY missing from In-Mission Save",partNum));

	result = file->readIdFloat("PositionZ",position.z);
	if (result != NO_ERR)
		STOP(("Part %d PositionZ missing from In-Mission Save",partNum));

	result = file->readIdFloat("Velocity",velocity);
	if (result != NO_ERR)
		STOP(("Part %d Velocity missing from In-Mission Save",partNum));

	result = file->readIdFloat("Rotation",rotation);
	if (result != NO_ERR)
		STOP(("Part %d Rotation missing from In-Mission Save",partNum));

	result = file->readIdULong("GestureId",gestureId);
	if (result != NO_ERR)
		STOP(("Part %d Rotation missing from In-Mission Save",partNum));

	result = file->readIdChar("Alignment",alignment);
	if (result != NO_ERR)
		STOP(("Part %d Alignment missing from In-Mission Save",partNum));

	result = file->readIdChar("TeamId",teamId);
	if (result != NO_ERR)
		STOP(("Part %d TeamId missing from In-Mission Save",partNum));

	result = file->readIdLong("CommanderId",commanderID);
	if (result != NO_ERR)
		STOP(("Part %d CommanderId missing from In-Mission Save",partNum));

	result = file->readIdChar("SquadId",squadId);
	if (result != NO_ERR)
		STOP(("Part %d SquadId missing from In-Mission Save",partNum));

	result = file->readIdChar("MyIcon",myIcon);
	if (result != NO_ERR)
		STOP(("Part %d MyIcon missing from In-Mission Save",partNum));

	result = file->readIdULong("ControlType",controlType);
	if (result != NO_ERR)
		STOP(("Part %d ControlType missing from In-Mission Save",partNum));

	result = file->readIdULong("ControlDataType",controlDataType);
	if (result != NO_ERR)
		STOP(("Part %d ControlDataType missing from In-Mission Save",partNum));

	result = file->readIdString("ProfileName",profileName,8);
	if (result != NO_ERR)
		STOP(("Part %d ProfileName missing from In-Mission Save",partNum));

	result = file->readIdULong("Pilot",pilot);
	if (result != NO_ERR)
		STOP(("Part %d Pilot missing from In-Mission Save",partNum));

	result = file->readIdBoolean("Captureable",captureable);
	if (result != NO_ERR)
		STOP(("Part %d Captureable missing from In-Mission Save",partNum));

	result = file->readIdULong("VariantNum",variantNum);
	if (result != NO_ERR)
		STOP(("Part %d VeriantNum missing from In-Mission Save",partNum));

}

void Mission::load (const char *loadFileName)
{
	userInput->mouseOff();
	loadProgress = 0.0f;

	DWORD oldXLoc = LoadScreen::xProgressLoc;
	DWORD oldYLoc = LoadScreen::yProgressLoc;

	LoadScreen::xProgressLoc = 0;
	LoadScreen::yProgressLoc = 0;

	bool turnOffAsyncMouse = mc2UseAsyncMouse;
	if ( !mc2UseAsyncMouse )
		MouseTimerInit();
	mc2UseAsyncMouse = true;			
	AsynFunc = ProgressTimer;

	//Open the In-Mission Save packet file.
	PacketFile loadFile;
	long result = loadFile.open((char *)loadFileName);
	if (result != NO_ERR)
		return;		//Can't load.  No File.  Dialog?  Probably not.

	DWORD currentPacket = 0;

	char versionCheck[1024];
	loadFile.readPacket(currentPacket,(MemoryPtr)versionCheck);
	currentPacket++;

	if (strcmp(versionCheck,versionStamp) != 0)
	{
		char msg[2048];
		cLoadString(IDS_QUICKSAVE_VERSION_WRONG,msg,2047);
		PAUSE((msg));
		return;
	}

	loadFile.seekPacket(currentPacket);
	char *campName = new char [loadFile.getPacketSize()+1];
	loadFile.readPacket(currentPacket,(MemoryPtr)(campName));
	currentPacket++;

	//Get the campaign name for the campaign we are currently playing.
	EString campaignName = LogisticsData::instance->getCampaignName();
	const char *cmpName = campaignName.Data();

	/*
	if (stricmp(campName,cmpName) != 0)
	{
		char msg[2048];
		cLoadString(IDS_QUICKSAVE_CAMPAIGN_WRONG,msg,2047);
		PAUSE((msg,campName,cmpName));
		return;
	}
	*/

	delete [] campName;
	campName = NULL;

	//Used to call this before the version check.  Wow.
	destroy();

	loadProgress = 1.0f;

	char tmpString[2048];
	sprintf(tmpString,"%d",timeGetTime());

	//-------------------------------------------
	//Relationships saved with TEAMs!
				
	//-------------------------------------------
	// Always reset turn at scenario start
	turn = 0;
	terminationCounterStarted = 0;

	//-----------------------
	// Init the ABL system...
	initABL();

	initBareMinimum();
	loadProgress = 4.0f;
	initTGLForMission();
	
	//--------------------------------------------------------------
	// Start the Mission Heap
	missionHeap = new UserHeap;
	gosASSERT(missionHeap != NULL);
	
	missionHeap->init(missionHeapSize,"MISSION");
	
	//--------------------------
	// Load Game System stuff...
	FullPathFileName fullGameSystemName;
	fullGameSystemName.init(missionPath, "gamesys", ".fit");
	
	FitIniFile* gameSystemFile = new FitIniFile;
	gosASSERT(gameSystemFile != NULL);
		
	result = gameSystemFile->open(fullGameSystemName);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->seekBlock("WeaponRanges");
	gosASSERT(result == NO_ERR);

	float span[2];
	result = gameSystemFile->readIdFloatArray("Short", span, 2);
	gosASSERT(result == NO_ERR);
	WeaponRanges[WEAPON_RANGE_SHORT][0] = span[0];
	WeaponRanges[WEAPON_RANGE_SHORT][1] = span[1];

	result = gameSystemFile->readIdFloatArray("Medium", span, 2);
	gosASSERT(result == NO_ERR);
	WeaponRanges[WEAPON_RANGE_MEDIUM][0] = span[0];
	WeaponRanges[WEAPON_RANGE_MEDIUM][1] = span[1];

	result = gameSystemFile->readIdFloatArray("Long", span, 2);
	gosASSERT(result == NO_ERR);
	WeaponRanges[WEAPON_RANGE_LONG][0] = span[0];
	WeaponRanges[WEAPON_RANGE_LONG][1] = span[1];

	result = gameSystemFile->readIdFloatArray("OptimalRangePoints", OptimalRangePoints, 5);
	gosASSERT(result == NO_ERR);

	for (long i = 0; i < 5; i++)
		for (long j = 0; j < 3; j++) {
			OptimalRangePointInRange[i][j] = false;
			if (OptimalRangePoints[i] > WeaponRanges[j][0])
				if (OptimalRangePoints[i] <= WeaponRanges[j][1])
					OptimalRangePointInRange[i][j] = true;
		}

	result = gameSystemFile->seekBlock("General");
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdFloat("MaxVisualRange",maxVisualRange);
	gosASSERT(result == NO_ERR);
	MaxVisualRadius = maxVisualRange * 1.4142;

	result = gameSystemFile->readIdFloat("FireVisualRange",fireVisualRange);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdFloatArray("WeaponRange", WeaponRange, NUM_FIRERANGES);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdFloat("DefaultAttackRange", DefaultAttackRange);
	if (result != NO_ERR)
		DefaultAttackRange = 75.0;

	result = gameSystemFile->readIdFloat("BaseSensorRange",baseSensorRange);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdLongArray("VisualRangeTable",visualRangeTable,256);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdFloat("BaseHeadShotElevation",BaseHeadShotElevation);
	if (result != NO_ERR)
		BaseHeadShotElevation = 1.0f;

	long forestMoveCost;
	result = gameSystemFile->readIdLong("ForestMoveCost", forestMoveCost);
	gosASSERT(result == NO_ERR);
		
	//----------------------------------------------------------------------
	// Now that we have some base values, load the master component table...
	if (!MasterComponent::masterList) {
		FullPathFileName compFileName;
		compFileName.init(objectPath,"compbas",".csv");
#ifdef _DEBUG
		long loadErr = 
#endif
			MasterComponent::loadMasterList(compFileName, 255, baseSensorRange);
		gosASSERT(loadErr == NO_ERR);
	}

	result = gameSystemFile->readIdUChar("GodMode", godMode);
	if (result != NO_ERR)
		godMode = 0;

	unsigned char revealTacMap;
	result = gameSystemFile->readIdUChar("RevealTacMap", revealTacMap);
	if (result != NO_ERR)
		revealTacMap = 0;
		
	result = gameSystemFile->readIdUChar("FootPrints", footPrints);
	if (result != NO_ERR)
		footPrints = 1;

	result = gameSystemFile->readIdLong("BonusTonnageDivisor",tonnageDivisor);
	gosASSERT(result == NO_ERR);
	
	result = gameSystemFile->readIdLong("BonusPointsPerTon",resourcesPerTonDivided);
	gosASSERT(result == NO_ERR);
	
#ifndef FINAL
 	result = gameSystemFile->readIdFloat("CheatHitDamage",CheatHitDamage);
	if (result != NO_ERR)
		CheatHitDamage = 5.0f;
#endif
	
	//---------------------------------------
	// Read in difficulty here if it exits.
	InitDifficultySettings(gameSystemFile);

	result = Mover::loadGameSystem(gameSystemFile, maxVisualRange);
	gosASSERT(result == NO_ERR);

	result = BattleMech::loadGameSystem(gameSystemFile);
	gosASSERT(result == NO_ERR);

	//--------------------------------------------------------------------
	result = GroundVehicle::loadGameSystem(gameSystemFile);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->seekBlock("Mine");
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdFloat("BaseDamage", MineDamage);
	gosASSERT(result == NO_ERR);
		
	result = gameSystemFile->readIdFloat("SplashDamage", MineSplashDamage);
	gosASSERT(result == NO_ERR);
		
	result = gameSystemFile->readIdFloat("SplashRange", MineSplashRange);
	gosASSERT(result == NO_ERR);
		
	result = gameSystemFile->readIdLong("Explosion", MineExplosion);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdLong("MineLayThrottle", MineLayThrottle);
	if (result != NO_ERR)
		MineLayThrottle = 50;

	result = gameSystemFile->readIdLong("MineSweepThrottle", MineSweepThrottle);
	if (result != NO_ERR)
		MineSweepThrottle = 50;

	result = gameSystemFile->readIdFloat("MineWaitTime", MineWaitTime);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdFloat("StrikeWaitTime", StrikeWaitTime);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdFloat("StrikeTimeToImpact", StrikeTimeToImpact);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->seekBlock("Smoke");
	gosASSERT(result == NO_ERR);
	
	result = gameSystemFile->readIdLong("MaxSmokeSpheres",totalSmokeSpheres);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->readIdLong("TotalSmokeShapeSize",totalSmokeShapeSize);
	gosASSERT(result == NO_ERR);

	result = gameSystemFile->seekBlock("Fire");
	gosASSERT(result == NO_ERR);
	
	result = gameSystemFile->readIdLong("MaxFiresBurning", maxFiresBurning);
	gosASSERT(result == NO_ERR);
	
	result = gameSystemFile->readIdFloat("MaxFireBurnTime", maxFireBurnTime);
	gosASSERT(result == NO_ERR);

	//---------------------------------------------------------------------------------
	// OK, load up the in_mission Save mission data
	if (loadFile.seekPacket(currentPacket) != NO_ERR)
		STOP(("Mission Data missing from IN-Mission Save"));

	duration = 60;
	
	FullPathFileName tmpName;
	tmpName.init(savePath,tmpString,".fti");
	{
		//Create a dummy file to read the packet into.
		File dmyFile;
		result = dmyFile.create(tmpName);
		DWORD fileSz = loadFile.getPacketSize();
		MemoryPtr tmpRAM = (MemoryPtr)malloc(fileSz);
		loadFile.readPacket(currentPacket,tmpRAM);
		dmyFile.write(tmpRAM,fileSz);
	
		currentPacket++;
	
		dmyFile.close();
		free(tmpRAM);
		//---------------------------
	}

	FitIniFile missionFile;
	result = missionFile.open(tmpName);
	if (result != NO_ERR)
		STOP(("Mission Data missing from IN-Mission Save 2"));

	//--------------------------------
	//Load up the mission Settings.
	if (missionFile.seekBlock( "MissionSettings" ) != NO_ERR)
		STOP(("MissionSettings missing from IN-Mission Save"));

	result = missionFile.readIdString( "MissionName"					, missionFileName,79);
	if (result != NO_ERR)
		STOP(("Mission Name missing from IN-Mission Save"));

	result = missionFile.readIdFloat( "TimeLimit"						, m_timeLimit );
	if (result != NO_ERR)
		STOP(("Time Limit  missing from IN-Mission Save"));

	long rps;
	result = missionFile.readIdLong( "ResourcePoints"					, rps );
	if (result != NO_ERR)
		STOP(("Resource Points missing from IN-Mission Save"));

	result = missionFile.readIdUChar( "scenarioTuneNum"					, missionTuneNum );
	if (result != NO_ERR)
		STOP(("Scenario Tune Num missing from IN-Mission Save"));

	result = missionFile.readIdLong("OperationId"						, operationId);
	if (result != NO_ERR)
		STOP(("Operation ID missing from IN-Mission Save"));

	result = missionFile.readIdLong("MissionId"							, missionId);
	if (result != NO_ERR)
		STOP(("MissionId missing from IN-Mission Save"));

	result = missionFile.readIdLong("ABLScriptHandle"					, missionScriptHandle);
	if (result != NO_ERR)
		STOP(("ABL Script Handle missing from IN-Mission Save"));

	result = missionFile.readIdBoolean("Active"							, active);
	if (result != NO_ERR)
		STOP(("Active flag missing from IN-Mission Save"));

	result = missionFile.readIdFloat("ActualTime"						, actualTime);
	if (result != NO_ERR)
		STOP(("Actual Time missing from IN-Mission Save"));

	result = missionFile.readIdFloat("RunningTime"						, runningTime);
	if (result != NO_ERR)
		STOP(("Running Time missing from IN-Mission Save"));

	result = missionFile.readIdFloat("ScenarioTime"						, scenarioTime);
	if (result != NO_ERR)
		STOP(("ScenarioTime missing from IN-Mission Save"));

	result = missionFile.readIdFloat("DropZoneX"						, dropZone.x);
	if (result != NO_ERR)
		STOP(("DropZoneX missing from IN-Mission Save"));

	result = missionFile.readIdFloat("DropZoneY"						, dropZone.y);
	if (result != NO_ERR)
		STOP(("DropZoneY missing from IN-Mission Save"));

	result = missionFile.readIdFloat("DropZoneZ"						, dropZone.z);
	if (result != NO_ERR)
		STOP(("DropZoneZ missing from IN-Mission Save"));

	//-----------------------------------
	// Find the SKY Number and save it.
	// If no number, i.e. an old mission file,
	// simply make it 1 until its written out
	// in the magical editor (tm)
	result = missionFile.seekBlock("TheSky");
	if (result != NO_ERR)
		theSkyNumber = DEFAULT_SKY;
	else
	{
		result = missionFile.readIdLong("SkyNumber",theSkyNumber);
		if (result != NO_ERR)
			theSkyNumber = DEFAULT_SKY;
			
		if ((theSkyNumber < 1) || (theSkyNumber > 20))
			theSkyNumber = DEFAULT_SKY;
	}
		
	result = missionFile.seekBlock( "Script" );
	if (result != NO_ERR)
		STOP(("Mission ABL Script missing from In-Mission Save"));
	else
	{
		result = missionFile.readIdString( "ScenarioScript"					, missionScriptName,79 );
		if (result != NO_ERR)
			missionScriptName[0] = 0;
	}

#ifndef USE_ABL_LOAD
	//Start the scenario script the old fashion way for now.
	//----------------------
	// Load ABL Libraries...
	long numErrors, numLinesProcessed;
	FullPathFileName libraryFileName;
	libraryFileName.init(missionPath, "orders", ".abx");
	long err = ABLi_loadLibrary(libraryFileName, &numErrors, &numLinesProcessed);
	gosASSERT(err == NO_ERR);

	libraryFileName.init(missionPath, "miscfunc", ".abx");
	err = ABLi_loadLibrary(libraryFileName, &numErrors, &numLinesProcessed);

	libraryFileName.init(missionPath, "corebrain", ".abx");
	err = ABLi_loadLibrary(libraryFileName, &numErrors, &numLinesProcessed);

	FullPathFileName brainFileName;
	brainFileName.init(missionPath, missionScriptName, ".abl");
	
	missionScriptHandle = ABLi_preProcess(brainFileName, &numErrors, &numLinesProcessed);
	gosASSERT(missionScriptHandle >= 0);
	
	missionBrain = new ABLModule;
	gosASSERT(missionBrain != NULL);
		
	missionBrain->init(missionScriptHandle);

	missionBrain->setName("Mission");
	//MissionBrain->setStep(TRUE);

	missionBrainParams = new ABLParam;
	gosASSERT(missionBrainParams != NULL);

	missionBrainCallback = missionBrain->findFunction("handlemessage", TRUE);
#endif

	//-----------------------------
	// Init Trigger Area Manager...
	Mover::triggerAreaMgr = new TriggerAreaManager;
	if (Mover::triggerAreaMgr == NULL)
		STOP(("Mission.Load: unable to init triggerAreaMgr "));

	//-----------------------------------------
	// Startup the Crater Manager.
	craterManager = (CraterManagerPtr)missionHeap->Malloc(sizeof(CraterManager));
	gosASSERT(craterManager != NULL);
		
	result = craterManager->init(1000,20479,"feet");
	gosASSERT(result == NO_ERR);
	
	//-----------------------------------------------------------------
	// Start the object system next.	
	ObjectManager = new GameObjectManager;
	if (!ObjectManager)
		Fatal(0, " Mission.init: unable to create ObjectManager ");
	ObjectManager->init("object2", 716799, 3072000);
	gosASSERT(result == NO_ERR);

	//-----------------------------------------------------------------
	// Start the collision detection system. -- Doesn't need objects?
	ObjectManager->initCollisionSystem(&missionFile);

	//------------------------------------------------------------
	// Start the Terrain System
	FullPathFileName terrainFileName;
	terrainFileName.init( missionPath, missionFileName, ".pak" ); 

	PacketFile pakFile;
	result = pakFile.open( terrainFileName );
	gosASSERT( result == NO_ERR );

	land = new Terrain;
	gosASSERT(land != NULL);

	loadProgress = 15.0f;

	long terrainInitResult = land->init(&pakFile, 0, GameVisibleVertices, loadProgress, 15.0 );

	if (terrainInitResult != NO_ERR)
	{
		STOP(("Could not load terrain.  Probably size was wrong!"));
	}

	loadProgress = 35.0f;

	land->load( &missionFile );

	loadProgress = 38.0f;

	//----------------------------------------------------
	// Start GameMap for Movement System
	Assert(SimpleMovePathRange > 20, SimpleMovePathRange, " Simple MovePath Range too small ");
	MOVE_init(SimpleMovePathRange);
	if (loadFile.seekPacket(currentPacket) == NO_ERR)
	{
		if (loadFile.getPacketSize() != 0)
		{
			currentPacket += MOVE_readData(&loadFile, currentPacket);
			if (GlobalMoveMap[0]->badLoad)
				STOP((" Mission.Load: old version of move data In-Mission Save Obsolete."));

			GameMap->placeMoversCallback = PlaceMovers;
			GlobalMoveMap[0]->isGateDisabledCallback = IsGateDisabled;
			GlobalMoveMap[1]->isGateDisabledCallback = IsGateDisabled;
			GlobalMoveMap[2]->isGateDisabledCallback = IsGateDisabled;
			GlobalMoveMap[0]->isGateOpenCallback = IsGateOpen;
			GlobalMoveMap[1]->isGateOpenCallback = IsGateOpen;
			GlobalMoveMap[2]->isGateOpenCallback = IsGateOpen;
		}
		else
			STOP(("Mission has no movement Data.  QuickSaved Map"));
	}

	PathFindMap[SECTOR_PATHMAP]->blockedDoorCallback = GetBlockedDoorCells;
	PathFindMap[SECTOR_PATHMAP]->placeStationaryMoversCallback = PlaceStationaryMovers;
	PathFindMap[SIMPLE_PATHMAP]->placeStationaryMoversCallback = PlaceStationaryMovers;
	PathFindMap[SECTOR_PATHMAP]->forestCost = forestMoveCost;
	PathFindMap[SIMPLE_PATHMAP]->forestCost = forestMoveCost;
	PathManager = new MovePathManager;

	loadProgress = 40.0f;

	//----------------------
	// Load ABL Libraries...
	// ASS U ME that these are loaded when I call ABLi_Load
	
	//---------------------------
	// Load the mission script...
	//Create a dummy file to read the packet into.
	FullPathFileName dmpName;
	dmpName.init(savePath,tmpString,".abl");

	if (loadFile.seekPacket(currentPacket) != NO_ERR)
		STOP(("ABL SaveData missing from IN-Mission Save"));

	{
		//Create a dummy file to read the packet into.
		File dmyFile;
		result = dmyFile.create(dmpName);
		DWORD fileSz = loadFile.getPacketSize();
		MemoryPtr tmpRAM = (MemoryPtr)malloc(fileSz);
		loadFile.readPacket(currentPacket,tmpRAM);
		dmyFile.write(tmpRAM,fileSz);
	
		currentPacket++;
	
		dmyFile.close();
		free(tmpRAM);
		//---------------------------
	}

	ABLFile ablFile;
	ablFile.open(dmpName);

#ifdef USE_ABL_LOAD
	ABLi_loadEnvironment(&ablFile,true);
#endif
		
	ablFile.close();
	DeleteFile(dmpName);

#ifdef USE_ABL_LOAD
	missionBrain = ABLi_getModule(missionScriptHandle);
	if (!missionBrain)
		STOP(("Could not find mission brain in In-Mission Save"));

	missionBrain->setName("Mission");

	missionBrainParams = new ABLParam;
	gosASSERT(missionBrainParams != NULL);

	missionBrainCallback = missionBrain->findFunction("handlemessage", TRUE);
#endif

	loadProgress = 42.0f;

	//-------------------------------------------
	// Load all MechWarriors for this mission...
	MechWarrior::setup();

	currentPacket = MechWarrior::Load( &loadFile, currentPacket );

	loadProgress = 45.0f;

	//-----------------------------------------------------------------
  	// All systems are GO if we reach this point.  Now we need to
	// parse the scenario file for the Objects we need for this scenario
	// We then create each object and place it in the world at the 
	// position we read in with the frame we read in.
	result = missionFile.seekBlock("Parts");
	gosASSERT(result == NO_ERR);
		
	result = missionFile.readIdULong("NumParts",numParts);
	gosASSERT(result == NO_ERR);

	//--------------------------------------------------------------------------------
	// IMPORTANT NOTE: mission parts should always start with Part 1.
	// Part 0 is reserved as a "NULL" id for routines that reference the mission
	// parts. AI routines, Brain keywords, etc. use PART ID 0 as an "object not found"
	// error code. DO NOT USE PART 0!!!!!!! Start with Part 1...

	loadProgress = 50.0f;

	if (numParts)
	{
		//-----------------------------------------------------
		// Since we leave part 0 unused, malloc numParts + 1...
		parts = (PartPtr)missionHeap->Malloc(sizeof(Part) * (numParts + 1));
		gosASSERT(parts != NULL);
		
		memset(parts,0,sizeof(Part) * (numParts + 1));

		int i;

		for (i = 1; i < numParts + 1; i++)
		{
			char partName[12];
			sprintf(partName,"Part%d", i);
			
			//------------------------------------------------------------------
			// Find the object to load
			result = missionFile.seekBlock(partName);
			gosASSERT(result == NO_ERR);

			//------------------------------------------------------------------
			// Find out what kind of object this is.
			result = missionFile.readIdULong("ObjectNumber",parts[i].objNumber);
			gosASSERT(result == NO_ERR);

			//-------------------------------------------------
			// Read in the data needed to control the object...
			result = missionFile.readIdULong("ControlType", parts[i].controlType);
			gosASSERT(result == NO_ERR);

			result = missionFile.readIdULong("ControlDataType", parts[i].controlDataType);
			gosASSERT(result == NO_ERR);

			result = missionFile.readIdString("ObjectProfile", parts[i].profileName, 9);
			gosASSERT(result == NO_ERR);
				
			result = missionFile.readIdULong("ObjectVariant", parts[i].variantNum);
			if (result != NO_ERR)
				parts[i].variantNum = 0;		//FOR NOW!!!!!!!!!!!!!!!!
												//MAKE a REAL error when Heidi fixes editor.
												//-fs 12/7/99

			result = missionFile.readIdULong("Pilot", parts[i].pilot);
			gosASSERT(result == NO_ERR);
			
			//------------------------------------------------------------------
			// Read the object's position, initial velocity and rotation.
			result = missionFile.readIdFloat("PositionX",parts[i].position.x);
			gosASSERT(result == NO_ERR);
				
			result = missionFile.readIdFloat("PositionY",parts[i].position.y);
			gosASSERT(result == NO_ERR);
				
			parts[i].position.z = -1.0;

			result = missionFile.readIdFloat("Rotation",parts[i].rotation);
			gosASSERT(result == NO_ERR);
				
			result = missionFile.readIdChar("TeamId",parts[i].teamId);
			gosASSERT(result == NO_ERR);

			char cmdId = 0;
			result = missionFile.readIdChar("CommanderId", cmdId);
			if (result != NO_ERR)
			{
				result = missionFile.readIdLong("CommanderId", parts[i].commanderID);
				gosASSERT(result == NO_ERR);
			}
			else
			{
				parts[i].commanderID = cmdId;
			}

			parts[i].gestureId = 2; // this has never changed
	
			result = missionFile.readIdULong("BaseColor",parts[i].baseColor);
			if (result != NO_ERR)
				parts[i].baseColor = 0xffffffff;
				
			result = missionFile.readIdULong("HighlightColor1",parts[i].highlightColor1);
			if (result != NO_ERR)
				parts[i].highlightColor1 = 0xffffffff;
				
			result = missionFile.readIdULong("HighlightColor2",parts[i].highlightColor2);
			if (result != NO_ERR)
				parts[i].highlightColor2 = 0xffffffff;
				
  			parts[i].velocity = 0;
			
			result = missionFile.readIdLong("Active",parts[i].active);
			gosASSERT(result == NO_ERR);

			result = missionFile.readIdLong("Exists",parts[i].exists);
			gosASSERT(result == NO_ERR);

			float fDamage = 0.0f;
			result = missionFile.readIdFloat("Damage",fDamage);
			if (result == NO_ERR) {
				if (fDamage >= 1.0) {
					parts[i].destroyed = true;
				};
			}

			result = missionFile.readIdChar("MyIcon", parts[i].myIcon);
			gosASSERT(result == NO_ERR);

			result = missionFile.readIdBoolean("Captureable", parts[i].captureable);
			if (result != NO_ERR)
				parts[i].captureable = FALSE;
		}
	}

	loadProgress = 55.0f;

	//----------------------------------------
	//Team Manager
	// Load Number of Teams
	// Load which number is HOME
	// Call Load for each team.
	currentPacket = Team::Load( &loadFile, currentPacket );

	//-----------------------------------
	// Setup the Sensor System Manager...
	SensorManager = new SensorSystemManager;
	Assert(SensorManager != NULL, 0, " Unable to init sensor system manager ");
	result = SensorManager->init(true);
	Assert(result == NO_ERR, result, " could not start Sensor System Manager ");

	//-----------------------------------------
	//Object Manager
	// Save numbers of objects in all object lists in game
	// Save ClassID of objects IN the list.
	// For each object in each list:
	//		If NULL, save a magic Number to tell me this entry is NULL.
	//		If NOT NULL, 
	// 			Call Save DOWN the heirarchy.
	//
	currentPacket = ObjectManager->Load( &loadFile, currentPacket );

	ObjectManager->buildMoverLists();

	//----------------------------------------------
	// Read in the Objectives.  Safe to have none.
	{
		numObjectives = 0; // this refers to the number of *old* objectives
		objectives = 0;

		warning1 = FALSE;
		warning2 = FALSE;

		Team::home->objectives.Clear();
		Team::home->objectives.Read(&missionFile);

		Team::home->numPrimaryObjectives = 0;
		CObjectives::EIterator it = Team::home->objectives.Begin();
		while (!it.IsDone())
		{
			if (1 == (*it)->Priority())
			{
				Team::home->numPrimaryObjectives += 1;
			}

			it++;
		}
	}

	//----------------------------
	// Read in Nav Markers
	// Not needed?  I saved EVERY Objective including the NavMarkers!

	//----------------------------
	//Commander Manager
	// Save Number of Commanders.
	// Save which number is HOME.
	// Call save for each commander.
	currentPacket = Commander::Load( &loadFile, currentPacket );

	//---------------------------------------------------------------------------------
	// If we're not playing multiplayer, make sure all home commander movers have their
	// localMoverId set to 0, so the iface can at least check if a mover is player
	// controlled...
	//
	// Should already be done but no harm in doing again?
	if (!MPlayer)
		Commander::home->setLocalMoverId(0);

	//-----------------------------------------------------
	// This tracks time since scenario started in seconds.
	// ALREADY LOADED above.

	gameSystemFile->close();
	delete gameSystemFile;
	gameSystemFile = NULL;

	//----------------------------
	// Create and load the Weather
	loadProgress = 92.f;
	weather = new Weather;
	weather->init(&missionFile);
	
	loadProgress = 95.0;
 	//---------------------------------------------------------------
	// Start the Camera and Lists
	eye = new GameCamera;
	eye->init();
	gosASSERT(eye != NULL);

	result = eye->init(&missionFile);
	gosASSERT(result == NO_ERR);

	loadProgress = 96.0;

	//----------------------------------------------------------------------------
	// Start the Mission GUI

	//MUST set this back to NULL to insure that the new gosHandle gets loaded for it!!
	// ITS been flushed up top.  NO leakage!
	PilotIcon::s_pilotTextureHandle = 0;

	missionInterface = new MissionInterfaceManager;
	gosASSERT(missionInterface != NULL);
	
	missionInterface->initTacMap( &pakFile, 2 );
	
	FullPathFileName missionScreenName;
	missionScreenName.init(artPath,"missionScrn",".fit");
	
	FitIniFile missionLoader;
	result = missionLoader.open(missionScreenName);
	gosASSERT(result == NO_ERR);
	
	missionInterface->init(&missionLoader);
	missionInterface->initMechs();
	missionInterface->Load(&missionFile);
	missionLoader.close();
	
	//----------------------------------------------------------------------------
	userInput->setMouseCursor(mState_NORMAL);

	loadProgress = 98.0;
	
	MechWarrior::initGoalManager(200);

	if (tempSpecialAreaFootPrints) 
	{
		systemHeap->Free(tempSpecialAreaFootPrints);
		tempSpecialAreaFootPrints = NULL;
		tempNumSpecialAreas = 0;
	}

	Mover::initOptimalCells(32);

	if (CombatLog)
		MechWarrior::logPilots(CombatLog);

	missionFile.close();
	DeleteFile(tmpName);

	if (loadFile.seekPacket(currentPacket) != NO_ERR)
		STOP(("Logistics Data missing from IN-Mission Save"));

	{
		//Create a dummy file to read the packet into.
		File dmyFile;
		result = dmyFile.create(tmpName);
		DWORD fileSz = loadFile.getPacketSize();
		MemoryPtr tmpRAM = (MemoryPtr)malloc(fileSz);
		loadFile.readPacket(currentPacket,tmpRAM);
		dmyFile.write(tmpRAM,fileSz);
	
		currentPacket++;
	
		dmyFile.close();
		free(tmpRAM);
		//---------------------------
	}

	FitIniFile logisticsFile;
	result = logisticsFile.open(tmpName);
	if (result != NO_ERR)
		STOP(("Logistics Data missing from IN-Mission Save 2"));

	LogisticsData::instance->load(logisticsFile);
	LogisticsData::instance->setResourcePoints(rps);

	logisticsFile.close();

	DeleteFile(tmpName);

	//-------------------------------------------------------------------------------------------
	// Load the CurrentMission number in logistics cause Heidi don't save it with logisticsData.
	long currentMissionNum = 0;
	loadFile.readPacket(currentPacket,(MemoryPtr)&currentMissionNum);

	LogisticsData::instance->setCurrentMissionNum(currentMissionNum);
	currentPacket++;
	//-------------------------------------------------------------------------------------------
	loadFile.close();

	eye->activate();
	eye->update();
	loadProgress = 100.f;

	DebugGameObject[0] = DebugGameObject[1] = DebugGameObject[2] = NULL;

	//YIKES!!  We could be checking the if before the null and executing after!!  Block the thread!
	//Wait for thread to finish.
	while (mc2IsInMouseTimer)
		;

	//ONLY set the mouse BLT data at the end of each update.  NO MORE FLICKERING THEN!!!
	// BLOCK THREAD WHILE THIS IS HAPPENING
	mc2IsInDisplayBackBuffer = true;

	AsynFunc = NULL;
	mc2UseAsyncMouse = turnOffAsyncMouse;
	if ( !mc2UseAsyncMouse)
		MouseTimerKill();

	mc2IsInDisplayBackBuffer = false;

	LoadScreen::xProgressLoc = oldXLoc;
	LoadScreen::yProgressLoc = oldYLoc;

	userInput->mouseOn();

	//Reset so the LastTimeGetTime is RIGHT NOW!!!
	//  So quickLoad works!!
	DWORD currentTimeGetTime = timeGetTime();
	LastTimeGetTime = currentTimeGetTime;
}

//----------------------------------------------------------------------------------------------------

