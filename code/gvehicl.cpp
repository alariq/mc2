//******************************************************************************************
//	gvehicl.cpp - This file contains the GroundVehicle Class Code
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

#ifndef COLLSN_H
#include"collsn.h"
#endif

#ifndef GVEHICL_H
#include"gvehicl.h"
#endif

#ifndef MECH_H
#include"mech.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#ifdef MISSION_H
#include"mission.h"
#endif

#ifndef CMPONENT_H
#include"cmponent.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef WEAPONBOLT_H
#include"weaponbolt.h"
#endif

#ifndef SOUNDS_H
#include"sounds.h"
#endif

#ifdef USE_ELEMENTALS
#ifndef ELEMNTL_H
#include<elemntl.h>
#endif
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef GROUP_H
#include"group.h"
#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef GAMELOG_H
#include"gamelog.h"
#endif

#ifndef ARTLRY_H
#include"artlry.h"
#endif

#ifndef LOGISTICSDATA_H
#include"logisticsdata.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

#include "../resource.h"

extern unsigned long		NextIdNumber;

#define TURN_THRESHOLD		20.0
#define BLIP_FRAME_RATE		0.067

//extern ObjectMapPtr GameObjectMap;
extern float worldUnitsPerMeter;
extern long StatusChunkUnpackErr;

extern TeamPtr				homeTeam;
extern bool					friendlyDestroyed;
extern bool					enemyDestroyed;
extern bool					useSound;
extern bool					useOldProject;
extern bool drawExtents;
extern bool drawTerrainGrid;

extern long adjClippedCell[8][2];

extern void addMoverToList(long blockNum);

extern float MaxVelocityMag;
extern float MineDamage;
extern float MineSplashDamage;
extern float MineSplashRange;
extern long MineExplosion;
extern long MineSweepThrottle;
extern long MineLayThrottle;
extern float MapCellDiagonal;
extern float				MaxTimeRevealed;

extern bool CantTouchThis;
extern bool ShowMovers;

extern WeaponFireChunk CurMoverWeaponFireChunk;
extern void DebugWeaponFireChunk (WeaponFireChunkPtr chunk1, WeaponFireChunkPtr chunk2, GameObjectPtr attacker);
extern void LogWeaponFireChunk (WeaponFireChunkPtr chunk, GameObjectPtr attacker, GameObjectPtr target);

long DefaultPilotId	= 653;
char marineProfileName[80] = "PEM00001";

//**********************************************************************************

float vehicleTurnRate[110] = 
{
	360.0f,360.0f,360.0f,360.0f,360.0f,360.0f,360.0f,360.0f,360.0f,360.0f, //0 through 9 tons
	240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f, //10 through 19 tons
	240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f, //20 through 29 tons
	240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f, //30 through 39 tons
	240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f, //40 through 49 tons
	240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f,240.0f, //50 through 59 tons
	210.0f,210.0f,210.0f,210.0f,210.0f,210.0f,210.0f,210.0f,210.0f,210.0f, //60 through 69 tons
	140.0f,140.0f,140.0f,140.0f,140.0f,140.0f,140.0f,140.0f,140.0f,140.0f, //70 through 79 tons
	120.0f,120.0f,120.0f,120.0f,120.0f,120.0f,120.0f,120.0f,120.0f,120.0f, //80 through 89 tons
	 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, //90 through 99 tons
	 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f //100 through 109 tons
};

//---------------------------------------------------------------------------
// Game System Tweakable Data
//---------------------------------------------------------------------------


extern long TargetMoveModifierTable[5][2];

extern float WeaponRange[NUM_FIRERANGES];
extern float WeaponRanges[NUM_WEAPON_RANGE_TYPES][2];

#define MAX_GVACTOR_STATES		3

long GroundVehicleAttackerMoveModifier[MAX_GVACTOR_STATES] = {
	0,		// Stopped
	10,		// Moving
	100		// Destroyed
};

long GroundVehicleCriticalHitTable[NUM_GROUNDVEHICLE_CRITICAL_HIT_TYPES] = {
	63,		// No Effect
	1,		// Ammo or Power Unit hit
	1,		// Fuel Tank Hit
	1,		// Crew Killed
	1,		// Engine Hit
	1,		// Main Weapon Jammed
	1,		// Crew Stunned
	6,		// Movement Disabled
	8,		// Movement Hindered
	11,		// Hovercraft -ONLY- Movement Hindered
	6		// Turret Jammed
};

extern long hitLevel[2];

extern GameLog* CombatLog;

#ifdef USEHEAT
extern float HeatCheckFrequency;
#endif

extern long ClusterSizeSRM;
extern long ClusterSizeLRM;

extern float PilotCheckHalfRate;

extern float WeaponFireModifiers[NUM_WEAPONFIRE_MODIFIERS];

extern long AimedFireAbort;
extern long AimedFireHitTable[3];

extern float ContactFadeTime;

float gvCollisionThreshold = 0.0;
float gvObjectCollisionThreshold = 0.0;
float gvTonnageCollisionThreshold = 0.0;
float gvTreeDeflection = 0.0;
float gvSweepTime = 0.0;
float gvWalkSpeed = 0.0;
float gvHillSpeedFactor = 0.0;

float InclineThrottleMultiplier[NUM_GROUNDVEHICLE_CHASSIS][2] = {
	{0.2f/*0.8*/, 1.0},		// WHEELED
	{0.2f/*1.0*/, 1.0},		// TRACKED
	{0.2f/*0.75*/, 1.25}	// HOVER
};

long DefaultGroundVehicleCrashAvoidSelf = 1;
long DefaultGroundVehicleCrashAvoidPath = 1;
long DefaultGroundVehicleCrashBlockSelf = 1;
long DefaultGroundVehicleCrashBlockPath = 1;
float DefaultGroundVehicleCrashYieldTime = 2.0;

long NumMarines = 0;

extern float NewRotation;

extern long TargetRolo;
extern float MineWaitTime;
extern float StrikeWaitTime;
extern float StrikeTimeToImpact;

extern float DistanceToWaypoint;

//**********************************************************************************
// GROUNDVEHICLE TYPE class
//**********************************************************************************

void GroundVehicleType::init (void)  {

	objectTypeClass = VEHICLE_TYPE;
	objectClass = GROUNDVEHICLE;

	mineSweeper = false;
	refitPoints = 0;
	recoverPoints = 0;
	crashAvoidSelf = DefaultGroundVehicleCrashAvoidSelf;
	crashAvoidPath = DefaultGroundVehicleCrashAvoidPath;
	crashBlockSelf = DefaultGroundVehicleCrashBlockSelf;
	crashBlockPath = DefaultGroundVehicleCrashBlockPath;
	crashYieldTime = DefaultGroundVehicleCrashYieldTime;
	explDmg = 0.0;
	explRad = 0.0;
	mineLayer = 0;
	hoverCraft = false;
	aerospaceSpotter = false;
	moveType = MOVETYPE_GROUND;
	LOSFactor = 1.0f;
}

//---------------------------------------------------------------------------

void GroundVehicleType::destroy (void) 
{
	ObjectType::destroy();
}

//----------------------------------------------------------------------------------

long GroundVehicleType::init (FilePtr objFile, unsigned long fileSize) {

	const char* bodyLocationString[NUM_GROUNDVEHICLE_LOCATIONS] = {
		"Front",
		"Left",
		"Right",
		"Rear",
		"Turret"
	};

	long result = 0;

	FitIniFile vehicleFile;
	result = vehicleFile.open(objFile, fileSize);
	if (result != NO_ERR)
		Fatal(result, " GroundVehicle:Init - Unable to open file ");

	result = vehicleFile.seekBlock("General");
	if (result != NO_ERR)
		Fatal(result, " GroundVehicle:Init - Unable to find Block General ");

	result = vehicleFile.readIdLong("MoveType", moveType);
	if (result != NO_ERR)
		moveType = MOVETYPE_GROUND;

	result = vehicleFile.readIdFloat("LOSFactor",LOSFactor);
	if (result != NO_ERR)
		LOSFactor = 1.0f;
		
 	result = vehicleFile.readIdUChar("Chassis", chassis);
	if (result != NO_ERR)
		Fatal(result, " GroundVehicle:Init - Unable to find Chassis ");

	//------------------------------
	// Check Special Vehicle Info

	result = vehicleFile.readIdLong("RefitPoints", refitPoints);
	if (result != NO_ERR)
		refitPoints = 0;

	result = vehicleFile.readIdLong("ResourcePoints", resourcePoints);
	if (result != NO_ERR)
		resourcePoints = 0;

	result = vehicleFile.readIdLong("RecoverPoints", recoverPoints);
	if (result != NO_ERR)
		recoverPoints = 0;
	
	result = vehicleFile.readIdBoolean("MineSweeper", mineSweeper);
	if (result != NO_ERR)
		mineSweeper = false;

	result = vehicleFile.readIdLong("MineLayer", mineLayer);
	if (result != NO_ERR)
		mineLayer = 0;

	result = vehicleFile.readIdBoolean("HoverCraft", hoverCraft);
	if (result != NO_ERR)
		hoverCraft = false;
	
	result = vehicleFile.readIdBoolean("AeroSpaceSpotter", aerospaceSpotter);
	if (result != NO_ERR)
		aerospaceSpotter = false;
		
 	result = vehicleFile.readIdBoolean("IsSensorContact", isSensorContact);
	if (result != NO_ERR)
		isSensorContact = true;

	//------------------------------
	// Splash Damage.
	result = vehicleFile.readIdFloat("ExplosionRadius",explRad);
	if (result != NO_ERR)
		explRad = 0.0;
		
	result = vehicleFile.readIdFloat("ExplosionDamage",explDmg);
	if (result != NO_ERR)
		explDmg = 0.0;

	result = vehicleFile.readIdBoolean("PathLocks",pathLocks);
	if (result != NO_ERR)
		pathLocks = true;
		
	//------------------------------------------------------------------
	// Now, read in the max internal structure for each body location...
	result = vehicleFile.seekBlock("InternalStructure");
	if (result != NO_ERR)
		Fatal(result, " GroundVehicle:Init - Unable to find InternalStructure Block ");

	for (long curLocation = 0; curLocation < NUM_GROUNDVEHICLE_LOCATIONS; curLocation++) 
	{
		result = vehicleFile.readIdUChar(bodyLocationString[curLocation], maxInternalStructure[curLocation]);
		if (result != NO_ERR)
			Fatal(curLocation, " GroundVehicle:Init - Unable to find Internal Structure Piece");
	}

	//------------------------------------------------------------------
	// Read in the data needed to move the vehicle and create base classes
	dynamics.setType(DYNAMICS_GROUNDVEHICLE);
	dynamics.init(&vehicleFile);

	//------------------------------------------------------------------
	// Initialize the base object Type from the current file.
	result = ObjectType::init(&vehicleFile);
	return(result);
}

//-----------------------------------------------------------------------------------

bool GroundVehicleType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider) {

	//---------------------------------------------
	// The default reaction of any object in the world
	// is to simply explode.  This just returns true
	// to facilitate this behaviour.
	//
	// Don't blow us up unless we hit another mech\vehicle.
	if (MPlayer && !MPlayer->isServer())
		return(false);
	
	bool sameTeam = false;
	bool collide = true;
	switch (collider->getObjectClass())
	{
		case BATTLEMECH:
		case GROUNDVEHICLE:
		case ELEMENTAL:
		{
			if (collidee->isFriendly(collider) && (collidee->getCommanderId() == collider->getCommanderId()))
				sameTeam = true;

			if (sameTeam)
				collide = false;
			else {
				GameObjectPtr collideeRamTarget = ((MoverPtr)collidee)->getPilot()->getCurTacOrder()->getRamTarget();
				GameObjectPtr colliderRamTarget = ((MoverPtr)collider)->getPilot()->getCurTacOrder()->getRamTarget();
				if ((collideeRamTarget == collider) || (colliderRamTarget == collidee))
					collide = true;
				else
					collide = false; //(((MoverPtr)collidee)->getPilot()->checkSkill(MWS_PILOTING) >= 0);
			}
					
			if (!((MoverPtr)collidee)->pathLocks && !sameTeam)
			{
				collide = true;			//Something steppable
				
			}

			if (collide) {
				if ((collidee->getCollisionFreeFromWID() != collider->getWatchID()) || (collidee->getCollisionFreeTime() < scenarioTime))
					collide = true;
				else
					collide = false;
			}

			if (collide) {
				//----------------------------------------------------------------------
				// Don't allow collision between these two again for a period of time...
				collidee->setCollisionFreeFromWID(collider->getWatchID());
				collidee->setCollisionFreeTime(scenarioTime + 2.0);

				if (dynamics.max.groundVehicle.speed != 0.0) {
					//------------------------------------
					// Adjust my velocity and direction...
					collidee->rotate(90.0);
					((MoverPtr)collidee)->bounceToAdjCell();

				}

				//---------------------------------------------
				// Administer the damage from this collision...
				long hitLocation = collidee->calcHitLocation(collider, -1, ATTACKSOURCE_COLLISION, 0);
				WeaponShotInfo shotInfo;
// UGLY HACK HERE...
				if (sameTeam)
					shotInfo.init(collider->getWatchID(), -1, 0 /*(long)(collider->getTonnage() / 100.0 + 0.5)*/, hitLocation, collidee->relFacingTo(collider->getPosition()));
//
				else
					shotInfo.init(collider->getWatchID(), -1, (long)(collider->getTonnage() / 10.0 + 0.5), hitLocation, collidee->relFacingTo(collider->getPosition()));
				collidee->handleWeaponHit(&shotInfo, (MPlayer != NULL));
				((GroundVehiclePtr)collidee)->deadByCrushed = true;

				//---------------------------------------------
				if (!sameTeam && (collidee->getWindowsVisible() == turn))
					soundSystem->playDigitalSample(MECH_COLLIDE,collider->getPosition());

				return(false);
			}
		}
		break;

		case BUILDING:
		case TREEBUILDING:
		{
			//Should Never collide with Buildings.  Only Steppables which ARE not this king of building!!
		}
		break;

		case TERRAINOBJECT:
		case TREE:
		{
			if ((collidee->getCollisionFreeFromWID() != collider->getWatchID()) || (collidee->getCollisionFreeTime() < scenarioTime))
			{
				//----------------------------------------------------------------------
				// Don't allow collision between these two again for a period of time...
				collidee->setCollisionFreeFromWID(collider->getWatchID());
				collidee->setCollisionFreeTime(scenarioTime + 2.0);

				//------------------------------------
				// Adjust my velocity and direction...
				float turnAround = 0.0;
				float tonnageClass = collidee->getTonnage();
				if (tonnageClass < gvTonnageCollisionThreshold)
					turnAround = (gvTonnageCollisionThreshold / tonnageClass) * gvTreeDeflection;
				if (turnAround > 0.0)
					collidee->rotate(turnAround);

				//---------------------------------------------
				if (collidee->getWindowsVisible() == turn)
					soundSystem->playDigitalSample(MECH_COLLIDE,collidee->getPosition());
				return(false);
			}
		}
		break;
	}

	return(false);
}

//-----------------------------------------------------------------------------------
bool GroundVehicleType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider) {

	//-------------------------------------------------------
	// For now, a BattleMech will play a default explosion
	// and disappear after the explosion is half way through
	MoverPtr vehicle = (MoverPtr)collidee;

	if (!vehicle->getPilot())
		Fatal(0, " No Pilot in this vehicle! ");

	//--------------------------------------------------------
	// Let's let the unit know we're dying if we're a point...
	// No, Don't!!!
	if (vehicle->getPoint() == vehicle) {
		vehicle->getGroup()->setPoint(NULL);
		//--------------------------------------------------------
		// If there is no new point, all units must be blown away.
		// How do we want to handle this?
	}

	//-----------------------------
	// Immediately lose contacts...
	if (vehicle->sensorSystem)
	{
		vehicle->sensorSystem->disable();
		vehicle->sensorSystem->broken = true;
	}

	if (vehicle->withdrawing) 
	{
		vehicle->timeLeft = 0.0;
		//-----------------------------------------------
		// Let the pilot know we are about to withdraw...
		vehicle->getPilot()->triggerAlarm(PILOT_ALARM_VEHICLE_WITHDRAWN, 0);
	}
	else 
	{
		vehicle->timeLeft = 0.0;
		//------------------------------------------
		// Let the pilot know we are about to die...
		vehicle->getPilot()->triggerAlarm(PILOT_ALARM_VEHICLE_DESTROYED, collider ? collider->getWatchID() : 0);
		vehicle->setStatus(OBJECT_STATUS_DESTROYED);
		vehicle->appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
		vehicle->appearance->setVisibility(true,true);
		vehicle->appearance->update();
		vehicle->exploding = false;
		if (MPlayer)
			vehicle->setFlag(OBJECT_FLAG_REMOVED, true);
		
		//------------------------------------
		// What heroic music should be played?
		if (CombatLog) {
			char s[1024];
			sprintf(s, "[%.2f] vehicle.destroyed: (%05d)%s", scenarioTime, vehicle->getPartId(), vehicle->getName());
			CombatLog->write(s);
			CombatLog->write(" ");
		}
	}

	return(true);
}

//----------------------------------------------------------------------------------

long GroundVehicleType::loadHotSpots(FitIniFilePtr vehicleFile) {

#if 0
	if (vehicleFile) {
		long result = vehicleFile->seekBlock("HotSpots");
		if (result != NO_ERR)
			return(result);

		char hotSpotFileName[80];
		result = vehicleFile->readIdString("HotSpotFileName",hotSpotFileName,79);
		if (result != NO_ERR)
			return(result);

		FullPathFileName hsFileName;
		hsFileName.init(spritePath,hotSpotFileName,".hsp");

		FitIniFile hsFile;
		result = hsFile.open(hsFileName);
		if (result != NO_ERR)
			return(result);

		//-------------------------
		// Pull in the Feet first.
		fHotSpots = (FootHotSpotData *)ObjectTypeManager::objectTypeCache->malloc(sizeof(FootHotSpotData) * numHotSpots * 2);
		if (!fHotSpots)
			return(NO_RAM_FOR_FOOT_HS);

		for (long i=0;i<numHotSpots;i++)
		{
			long lNum = i * 2;
			long rNum = lNum + 1;

			char footBlock[80];
			sprintf(footBlock,"%sLeftFoot",HSGestureName[i]);

			result = hsFile.seekBlock(footBlock);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdLong("Frame",fHotSpots[lNum].frameNum);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdFloat("X",fHotSpots[lNum].position.x);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdFloat("Y",fHotSpots[lNum].position.y);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdFloat("Z",fHotSpots[lNum].position.z);
			if (result != NO_ERR)
				return(result);

			sprintf(footBlock,"%sRightFoot",HSGestureName[i]);

			result = hsFile.seekBlock(footBlock);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdLong("Frame",fHotSpots[rNum].frameNum);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdFloat("X",fHotSpots[rNum].position.x);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdFloat("Y",fHotSpots[rNum].position.y);
			if (result != NO_ERR)
				return(result);

			result = hsFile.readIdFloat("Z",fHotSpots[rNum].position.z);
			if (result != NO_ERR)
				return(result);
		}

		drawFootprint = (char *)ObjectTypeManager::objectTypeCache->malloc(NUM_MECH_GESTURES);
		result = vehicleFile->readIdCharArray("DrawFootprint",drawFootprint,NUM_MECH_GESTURES);
		if (result != NO_ERR)
			return(result);

		long tmpType;
		result = vehicleFile->readIdLong("FootprintType",tmpType);
		if (result != NO_ERR)
			return(result);

		footprintType = (craterTypes)tmpType;
		//---------------------------
		// Next, Pull in the Weapons
	}
#endif

	return(NO_ERR);
}

//----------------------------------------------------------------------------------

GameObjectPtr GroundVehicleType::createInstance (void) {

	GroundVehiclePtr newVehicle = new GroundVehicle;
	if (!newVehicle)
		return(NULL);

	newVehicle->init(true, this);
	//newVehicle->setIdNumber(NextIdNumber++);

	return(newVehicle);
}

//**********************************************************************************
// GROUNDVEHICLE class
//**********************************************************************************

long GroundVehicle::loadGameSystem (FitIniFilePtr sysFile) {

	long result = 0;

	result = sysFile->seekBlock("GroundVehicle:FireWeapon");
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdLongArray("AttackerMoveModifier", GroundVehicleAttackerMoveModifier, MAX_GVACTOR_STATES);
	if (result != NO_ERR)
		return(result);

	result = sysFile->seekBlock("GroundVehicle:Damage");
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdLongArray("CriticalHitTable", GroundVehicleCriticalHitTable, NUM_GROUNDVEHICLE_CRITICAL_HIT_TYPES);
	if (result != NO_ERR)
		return(result);

	result = sysFile->seekBlock("GroundVehicle:Collision");
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdFloat("collisionThreshold", gvCollisionThreshold);
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdFloat("objectThreshold", gvObjectCollisionThreshold);
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdFloat("tonnageThreshold", gvTonnageCollisionThreshold);
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdFloat("treeDeflection", gvTreeDeflection);
	if (result != NO_ERR)
		return(result);

	result = sysFile->seekBlock("GroundVehicle:Movement");
	if (result != NO_ERR)
		return(result);

	long crashSize = 0;
	result = sysFile->readIdLong("CrashAvoidSelf", crashSize);
	if (result == NO_ERR)
		DefaultGroundVehicleCrashAvoidSelf = crashSize;

	result = sysFile->readIdLong("CrashAvoidPath", crashSize);
	if (result == NO_ERR)
		DefaultGroundVehicleCrashAvoidPath = crashSize;

	result = sysFile->readIdLong("CrashBlockSelf", crashSize);
	if (result == NO_ERR)
		DefaultGroundVehicleCrashBlockSelf = crashSize;

	result = sysFile->readIdLong("CrashBlockPath", crashSize);
	if (result == NO_ERR)
		DefaultGroundVehicleCrashBlockPath = crashSize;

	float crashYield = 0.0;
	result = sysFile->readIdFloat("CrashYieldTime", crashYield);
	if (result == NO_ERR)
		DefaultGroundVehicleCrashYieldTime = crashYield;

	result = sysFile->readIdFloat("SweeperSlowTime", gvSweepTime);
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdFloat("WalkSpeed", gvWalkSpeed);
	if (result != NO_ERR)
		return(result);

	result = sysFile->readIdFloat("HillSpeedFactor", gvHillSpeedFactor);
	if (result != NO_ERR)
		return(result);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

void GroundVehicle::handleStaticCollision (void)
{
	if (getTangible() && pathLocks) 
	{
		//-----------------------------------------------------
		// What is our block and vertex number?
		int blockNumber = 0;
		int vertexNumber = 0;
		
		getBlockAndVertexNumber(blockNumber,vertexNumber);

		int numCollidables = ObjectManager->getObjBlockNumCollidables(blockNumber);
		int terrainObjHandle = ObjectManager->getObjBlockFirstHandle(blockNumber);
		int colliderBlockNumber = -1;
		int colliderVertexNumber = -1;
		for (int i = 0; i < numCollidables; i++) 
		{
			GameObjectPtr terrainObj = ObjectManager->get(terrainObjHandle + i);
			bool isTangible = false;
			switch (terrainObj->getObjectClass()) 
			{
				case TREE:
				case TREEBUILDING:
				case TERRAINOBJECT:
				case BUILDING:
					terrainObj->getBlockAndVertexNumber(colliderBlockNumber, colliderVertexNumber);
					isTangible = terrainObj->getTangible();
					break;
					
				case CAMERADRONE:
				case MINE:
					break;
			}
			
			if (isTangible)
				ObjectManager->detectStaticCollision(this, terrainObj);
		}
	}
}	

//---------------------------------------------------------------------------

void GroundVehicle::init (bool create) {

	longName[0] = 0;

	//Mover::init();
	objectClass = GROUNDVEHICLE;

	numBodyLocations = NUM_GROUNDVEHICLE_LOCATIONS;
	numArmorLocations = NUM_GROUNDVEHICLE_LOCATIONS;

	accel = 0.0;
	velocityMag = 0.0;

	movementEnabled = true;

	turretRotation = 0;
	turretEnabled = true;
	turretBlownThisFrame = false;

	deadByCrushed = false;
	
#ifdef USE_SMOKE
	dmgSmoke = NULL;
#endif

	pathLocks = true;
	
	captureable = false;
	salvageVehicle = false;
	refitBuddyWID = 0;
	recoverBuddyWID = 0;
	canRefit = false;
	refitting = false;
	canRecover = false;
	recovering = false;
	sweepTime = -1;
	mineLayer = 0;
	aerospaceSpotter = false;
	blipFrame = 0;
	cellColToMine = cellRowToMine = -1;
	timeInCurrent = 0.0;
	
	sensorSystem = NULL;			//Make sure they start WITHOUT one or badness will insue
	
	isVehiclePilot = false;

	lowestWeaponNodeID = -2;

	dVel.Zero();
	dRot.Zero();
	dAcc.Zero();
};

//------------------------------------------------------------------------------------------

void GroundVehicle::init (bool create, ObjectTypePtr objType) {

	//-------------------------------------------------------------
	// Call down the chain to init everybody else.
	GameObject::init(create, objType);
	creationTime = scenarioTime;
	
	//-------------------------------------------------------------
	// In here will reside the code which initializes all of the
	// variables for a ground vehicle based upon the type.
	// For starters, these are tangible objects
	setTangible(true);

	//----------------------------------------------------
	// Set some mech traits based upon the vehicle Type...
	GroundVehicleTypePtr vehicleT = (GroundVehicleTypePtr)objType;
	for (long curLocation = 0; curLocation < NUM_GROUNDVEHICLE_LOCATIONS; curLocation++) {
		body[curLocation].maxInternalStructure = vehicleT->maxInternalStructure[curLocation];
		body[curLocation].CASE = false;
		body[curLocation].damageState = IS_DAMAGE_NONE;
		body[curLocation].totalSpaces = 0;
	}

	chassis = vehicleT->chassis;

	crashAvoidSelf = vehicleT->crashAvoidSelf;
	crashAvoidPath = vehicleT->crashAvoidPath;
	crashBlockSelf = vehicleT->crashBlockSelf;
	crashBlockPath = vehicleT->crashBlockPath;
	crashYieldTime = vehicleT->crashYieldTime;
	setMoveType(vehicleT->moveType);
	if (vehicleT->hoverCraft)
		moveLevel = 1;
	setFollowRoads(true);

	canRefit = (vehicleT->refitPoints > 0);
	canRecover = (vehicleT->recoverPoints > 0);
	captureable = captureable || (vehicleT->resourcePoints > 0);
	if (captureable)
		setFlag(OBJECT_FLAG_CAPTURABLE,true);

	mineSweeper = vehicleT->mineSweeper;
	mineLayer = vehicleT->mineLayer;
	aerospaceSpotter = vehicleT->aerospaceSpotter;

	nullSignature = (vehicleT->isSensorContact ? 255 : 64);
	
	//-------------------------------------------------------------
	// The control Class starts up here.
	control.init();

	//-------------------------------------------------------------
	// The dynamics class starts up here.
	dynamics = ((GroundVehicleTypePtr)objType)->dynamics;
	accel = dynamics.max.groundVehicle.accel;

	//-------------------------------------------------------------
	// The appearance is initialized here using data from the type
	char *appearanceName = objType->getAppearanceTypeName();

	//--------------------------------------------------------------
	// New code!!!
	// We need to append the sprite type to the appearance num now.
	// The MechEdit tool does not assume a sprite type, nor should it.
	// MechCmdr2 features much simpler objects which only use 1 type of sprite!
	long appearanceType = (GV_TYPE << 24);

	AppearanceTypePtr vehicleAppearanceType = appearanceTypeList->getAppearance(appearanceType,appearanceName);
	if (!vehicleAppearanceType)
	{
		//---------------------------------------------------------
		// Load a dummy appearance until all vehicles ready.
		// For now, the APC!
		vehicleAppearanceType = appearanceTypeList->getAppearance(appearanceType,"APC");
	}

	if ((appearanceType>>24) == GV_TYPE)
	{
		appearance = new GVAppearance;
		if (!appearance)
			Fatal(0, " GroundVehicle.init: unable to create appearance ");

		//-----------------------------------------------------------------
		// The only kind of appearanceType the vehicless currently know how
		// to work with is a spriteTree.  Anything else is wrong.
	   	appearance->init((GVAppearanceType*)vehicleAppearanceType, (GameObjectPtr)this);
		appearance->setAlphaValue(alphaValue);
	}

	objectClass = GROUNDVEHICLE;
	markDistanceMoved = 1000.0;

	pathLocks = ((GroundVehicleTypePtr)objType)->pathLocks;
	
	setObscured(true);
}

//----------------------------------------------------------------------------------

void GroundVehicle::setControl (ControlType ctrlType) {

	control.init(ctrlType, CONTROL_DATA_GROUNDVEHICLE);
}

//----------------------------------------------------------------------------------
long GroundVehicle::init (FitIniFile* vehicleFile) 
{
	const char* BodyLocationBlockString[NUM_GROUNDVEHICLE_LOCATIONS] = {
		"Front",
		"Left",
		"Right",
		"Rear",
		"Turret"
	};

	//--------------------------
	// Read in the vehicle data.
	long result = vehicleFile->seekBlock("ObjectType");
	if (result != NO_ERR)
		return(result);

	name[0] = '\0';

	result = vehicleFile->seekBlock("General");
	if (result != NO_ERR)
		return(result);

	result = vehicleFile->readIdInt( "IconIndex", iconPictureIndex);
	if ( result != NO_ERR )
		iconPictureIndex = 0;


	result = vehicleFile->readIdFloat("CurTonnage", tonnage);
	if (result != NO_ERR)
		return(result);

	result = vehicleFile->readIdInt("DescIndex", descID);
	if (result != NO_ERR)
		descID = -1;
	else
	{
		cLoadString( descID, name, MAXLEN_MOVER_NAME );
		char buffer[256];

		cLoadString( IDS_MFDVEHICLE_PRINTSTRING, buffer, 256 );

		sprintf( longName, buffer, name, tonnage );

	}

	
	
	char cStatus = 0;
	result = vehicleFile->readIdChar("Status", cStatus);
	if (result != NO_ERR)
		return(result);
	status = cStatus;

	result = vehicleFile->readIdInt("BattleRating", battleRating);
	if (result != NO_ERR)
		battleRating = -1;

	maxMoveSpeed = (float)dynamics.max.groundVehicle.speed;

	//------------------------------------------------------------------------------
	// Build the vehicle's inventory (all components, and where they are located)...
	result = vehicleFile->seekBlock("InventoryInfo");
	if (result != NO_ERR)
		return(result);

	result = vehicleFile->readIdUChar("NumOther", numOther);
	if (result != NO_ERR)
		return(result);

	result = vehicleFile->readIdUChar("NumWeapons", numWeapons);
	if (result != NO_ERR)
		return(result);

	result = vehicleFile->readIdUChar("NumAmmo", numAmmos);
	if (result != NO_ERR)
		return(result);

	//--------------------------------------------------------
	// Read in the vehicle's non-weapon/non-ammo components...
	long curItem = 0;
	while (curItem < numOther) 
	{
		char itemString[128];
		sprintf(itemString, "Item:%d", curItem);
		result = vehicleFile->seekBlock(itemString);
		if (result != NO_ERR)
			return(result);

		result = vehicleFile->readIdUChar("MasterID", inventory[curItem].masterID);
		if (result != NO_ERR)
			return(result);
		inventory[curItem].health = MasterComponent::masterList[inventory[curItem].masterID].getHealth();
		inventory[curItem].disabled = false;
		inventory[curItem].amount = 1;
		inventory[curItem].ammoIndex = -1;
		inventory[curItem].readyTime = 0.0;
		inventory[curItem].bodyLocation = 255;

		//------------------------------------------------------------
		// Let's keep track of a few special components we may need...
		switch (MasterComponent::masterList[inventory[curItem].masterID].getForm()) 
		{
			case COMPONENT_FORM_COCKPIT:
				cockpit = curItem;
				break;
			case COMPONENT_FORM_SENSOR:
				sensor = curItem;
				sensorSystem = SensorManager->newSensor();
				sensorSystem->setOwner(this);
				sensorSystem->setRange(MasterComponent::masterList[inventory[sensor].masterID].getSensorRange());
				break;
			case COMPONENT_FORM_ENGINE:
				engine = curItem;
				break;
			case COMPONENT_FORM_LIFESUPPORT:
				lifeSupport = curItem;
				break;
			case COMPONENT_FORM_ECM:
				ecm = curItem;
				//if (sensor != 255)
				//	SensorManager->addEcm(this, MasterComponent::masterList[inventory[sensor].masterID].getSensorRange());
				break;
			case COMPONENT_FORM_PROBE:
				probe = curItem;
				break;
		}

		curItem++;
	}

	//---------------------------------
	// Read in the vehicle's weapons...
	while (curItem < (numOther + numWeapons)) 
	{
		char itemString[128];
		sprintf(itemString, "Item:%d", curItem);
		result = vehicleFile->seekBlock(itemString);
		if (result != NO_ERR)
			return(result);

		result = vehicleFile->readIdUChar("MasterID", inventory[curItem].masterID);
		if (result != NO_ERR)
			return(result);

		inventory[curItem].health = MasterComponent::masterList[inventory[curItem].masterID].getHealth();
		inventory[curItem].disabled = false;
		inventory[curItem].amount = 1;
		inventory[curItem].facing = 1;
		inventory[curItem].ammoIndex = -1;
		inventory[curItem].readyTime = 0.0;
		inventory[curItem].bodyLocation = 255;
		inventory[curItem].effectiveness = (short)(MasterComponent::masterList[inventory[curItem].masterID].getWeaponDamage() * 10.0 /	// damage over 10 seconds
										   MasterComponent::masterList[inventory[curItem].masterID].getWeaponRecycleTime());
		inventory[curItem].effectiveness *= WeaponRanges[MasterComponent::masterList[inventory[curItem].masterID].getWeaponRange()][1] / 24;

		curItem++;
	}

	//------------------------------
	// Read in the vehicle's ammo...
	while (curItem < (numOther + numWeapons + numAmmos)) 
	{
		char itemString[128];
		sprintf(itemString, "Item:%d", curItem);
		result = vehicleFile->seekBlock(itemString);
		if (result != NO_ERR)
			return(result);

		result = vehicleFile->readIdUChar("MasterID", inventory[curItem].masterID);
		if (result != NO_ERR)
			return(result);

		//-----------------------------------------------------------------------
		// Ammo is always full.
		inventory[curItem].amount = MasterComponent::masterList[inventory[curItem].masterID].getAmmoPerTon();

		inventory[curItem].startAmount = inventory[curItem].amount;
		inventory[curItem].ammoIndex = -1;
		inventory[curItem].health = MasterComponent::masterList[inventory[curItem].masterID].getHealth();
		inventory[curItem].disabled = false;
		inventory[curItem].readyTime = 0.0;
		inventory[curItem].bodyLocation = 255;

		curItem++;
	}

	//------------------------------------------------------------
	// Now, read in the component layout for each body location...
	for (long curLocation = 0; curLocation < NUM_GROUNDVEHICLE_LOCATIONS; curLocation++) 
	{

		result = vehicleFile->seekBlock(BodyLocationBlockString[curLocation]);
		if (result != NO_ERR)
			return(result);

		body[curLocation].CASE = false;

		unsigned char internalStructure;
		result = vehicleFile->readIdUChar("CurInternalStructure", internalStructure);
		if (result != NO_ERR)
			return(result);

		body[curLocation].curInternalStructure = internalStructure;

		//---------------------------------------------------------
		// Now, determine the damage state for the body location...
		float damageLevel = (float)body[curLocation].curInternalStructure / body[curLocation].maxInternalStructure;
		if (damageLevel == 0.0)
			body[curLocation].damageState = IS_DAMAGE_DESTROYED;
		else if (damageLevel <= 0.5)
			body[curLocation].damageState = IS_DAMAGE_PARTIAL;
		else
			body[curLocation].damageState = IS_DAMAGE_NONE;

		result = vehicleFile->readIdUChar("MaxArmorPoints", armor[curLocation].maxArmor);
		if (result != NO_ERR)
			return(result);

		unsigned char armorPts;

		result = vehicleFile->readIdUChar("CurArmorPoints", armorPts);
		if (result != NO_ERR)
			return(result);

		armor[curLocation].curArmor = armorPts;

		body[curLocation].totalSpaces = 0;
	}

	calcAmmoTotals();

	for (long item = numOther; item < (numOther + numWeapons); item++) 
	{
		//----------------------------------------------------------
		// Each weapon should point to its appropriate ammo total in
		// the ammo type total list...
		for (long ammoIndex = 0; ammoIndex < numAmmoTypes; ammoIndex++)
		{
			if ((long)MasterComponent::masterList[inventory[item].masterID].getWeaponAmmoMasterId() == ammoTypeTotal[ammoIndex].masterId) 
			{
				inventory[item].ammoIndex = ammoIndex;
				break;
			}
		}
	}

	//------------------------------------------------------------------------------
	// Now that we've loaded inventory, let's set aside which weapon has the longest
	// range...
	calcWeaponEffectiveness(true);
	calcWeaponEffectiveness(false);
	calcFireRanges();

	maxCV = calcCV(true);
	curCV = calcCV(false);
	setThreatRating(-1);

	if (canRefit) 
	{
		GroundVehicleTypePtr objType = (GroundVehicleTypePtr)ObjectManager->getObjectType(typeHandle);
		armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor = armor[GROUNDVEHICLE_LOCATION_TURRET].maxArmor =
		((GroundVehicleTypePtr)objType)->refitPoints;
	}
	if (canRecover) 
	{
		GroundVehicleTypePtr objType = (GroundVehicleTypePtr)ObjectManager->getObjectType(typeHandle);
		armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor = armor[GROUNDVEHICLE_LOCATION_TURRET].maxArmor =
		((GroundVehicleTypePtr)objType)->recoverPoints;
	}
	if ( mineLayer )
	{
		GroundVehicleTypePtr objType = (GroundVehicleTypePtr)ObjectManager->getObjectType(typeHandle);
		armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor = armor[GROUNDVEHICLE_LOCATION_TURRET].maxArmor =
		((GroundVehicleTypePtr)objType)->mineLayer;
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long GroundVehicle::calcCV (bool calcMax) {

	if (battleRating != -1)
		return battleRating;

	//----------------------------------------
	// First, total the CV's of all weapons...
	float weaponBR = 0.0;
	for (long weaponIndex = numOther; weaponIndex < (numOther + numWeapons); weaponIndex++)
		if (calcMax || !inventory[weaponIndex].disabled)
			weaponBR += MasterComponent::masterList[inventory[weaponIndex].masterID].getCV();

	//----------------------------------------------------------
	// Now, multiply weapon BR by the speed factor. Speed Factor
	// is based upon a standard rate of 18m/s...
	weaponBR *= (1.0 + (maxMoveSpeed - 18.0) / 18.0);

	//------------------------------
	// Now, calc the defensive BR...
	float defensiveBR = 0.0;

	//-------------
	// IS Factor...
	for (long bodyLocation = 0; bodyLocation < NUM_GROUNDVEHICLE_LOCATIONS; bodyLocation++)
		defensiveBR += (calcMax ? body[bodyLocation].maxInternalStructure : body[bodyLocation].curInternalStructure);

	//----------------
	// Armor Factor...
	for (long armorLocation = 0; armorLocation < NUM_GROUNDVEHICLE_LOCATIONS; armorLocation++)
		defensiveBR += (calcMax ? armor[armorLocation].maxArmor : armor[armorLocation].curArmor);

	//----------------------
	// Add Tonnage Factor...
	defensiveBR += getTonnage();

	//-------------------
	// Movement Factor...
    int i = 0;
	for (; i < 5; i++)
		if (maxMoveSpeed <= TargetMoveModifierTable[i][0])
			break;
	defensiveBR += (TargetMoveModifierTable[i][1] * 10);


	//---------------
	// Heat Factor...
//	float heatFactor = 0;
//	for (weaponIndex = numOther; weaponIndex < (numOther + numWeapons); weaponIndex++)
//		if (calcMax || !inventory[weaponIndex].disabled)
//			heatFactor -= inventory[weaponIndex].heatPerSec;
//	heatFactor *= 10.0;
//	if (heatFactor > 0.0)
//		heatFactor = 0.0;
//	defensiveBR += heatFactor;

	//--------------------------------
	// Any other special Components..?
	for (long itemIndex = 0; itemIndex < numOther; itemIndex++)
		if (calcMax || !inventory[itemIndex].disabled)
			defensiveBR += MasterComponent::masterList[inventory[itemIndex].masterID].getCV();

	return((long)(weaponBR + defensiveBR));
}

//-------------------------------------------------------------------------------------------

void GroundVehicle::pilotingCheck (void) {

	return;
}

//-------------------------------------------------------------------------------------------

void GroundVehicle::destroy (void) 
{
	if (appearance)
	{
		delete appearance;
		appearance = NULL;
	}
}

//------------------------------------------------------------------------------------------

void GroundVehicle::mineCheck (void) {

	if (MPlayer && !MPlayer->isServer())
		return;

	unsigned long mine = 0;
	mine = GameMap->getMine(cellPositionRow, cellPositionCol);

	if (mine == 1)
	{
		//----------------------------------------------------
		// Mark the tiles around as potentially mine occupied.
		// NOT NEEDED ANYMORE.  EVERYONE CAN SEE MINES!!!!!!!!!!!

		if (mineSweeper)
		{
			sweepTime = 0;
			Stuff::Vector3D curPos = getPosition();
			ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, curPos);
			armor[GROUNDVEHICLE_LOCATION_FRONT].curArmor--;
			if (MPlayer) 
			{
				WeaponShotInfo shot;
				shot.init(0, -2, 1.0, GROUNDVEHICLE_LOCATION_FRONT, 0);
				MPlayer->addWeaponHitChunk(this, &shot);
			}
			
			if (armor[GROUNDVEHICLE_LOCATION_FRONT].curArmor == 0)
			{
				mineSweeper = false;
				sweepTime = -1;
				pilot->clearCurTacOrder();	// stop!!!
			}

			//---------------------------------------------------------
			// Mark this tile as empty.  (we just set the mine off!!)
			GameMap->setMine(cellPositionRow, cellPositionCol, 2);
			
			if (MPlayer) 
			{
				gosASSERT("Fix more multiplyer--make sure position of mine is right!");
				//long tilePos[4];
				//getTileCellPosition(tilePos[0], tilePos[1], tilePos[2], tilePos[3]);
				//MPlayer->addMineChunk(tilePos[2],
				//					  tilePos[3],
				//					  1,
				//					  2,
				//					  1);
			}
		}
		else if (mineLayer == 0)
		{
			//---------------------------
			// Mine here, deal with it...
			if (tonnage > 35.0f)
			{
				Stuff::Vector3D curPos;
				curPos = getPosition();
				ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, curPos, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
				WeaponShotInfo shot;
				shot.init(0, -2, MineDamage, calcHitLocation(NULL,-1,ATTACKSOURCE_MINE,0), 0);
				handleWeaponHit(&shot, (MPlayer != NULL));
					
				pilot->pausePath();		//Force the pilot to recalc based on new data.
	
				//---------------------------------------------------------
				//  Mark this tile as empty.  (we just set the mine off!!)			
				GameMap->setMine(cellPositionRow, cellPositionCol, 2);
				
				if (MPlayer) 
				{
					gosASSERT("Fix position!");
					//long tilePos[4];
					//getTileCellPosition(tilePos[0], tilePos[1], tilePos[2], tilePos[3]);
					//MPlayer->addMineChunk(tilePos[2],
					//					  tilePos[3],
					//					  1,
					//					  2,
					//					  2);
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

bool GroundVehicle::pivotTo (void) {

	//If we have no maxVelocity, we are an artillery piece.  DO NOT PIVOT!!
	if (dynamics.max.groundVehicle.speed == 0.0f)
		return false;

	MovePathPtr path = pilot->getMovePath();
	long moveState = pilot->getMoveState();
	long moveStateGoal = pilot->getMoveStateGoal();
	bool isRunning = false;
	if (MPlayer && !MPlayer->isServer())
		isRunning = moveChunk.run;
	else
		isRunning = (pilot->getMovePath()->numStepsWhenNotPaused > 0) && pilot->getMoveRun();

	bool hasTarget = false;
	Stuff::Vector3D targetPosition;
	GameObjectPtr target = pilot->getCurrentTarget();
	float relFacingToTarget = 0.0;
	if (target) 
	{
		targetPosition = target->getPosition();
		relFacingToTarget = relFacingTo(targetPosition);
		//maxVehiclePivotRate = ((GroundVehicleDynamicsTypePtr)(((GroundVehicleTypePtr)type)->dynamicsType))->maxTurretYawRate * frameLength;
		hasTarget = true;
	}
	else if (pilot->getCurTacOrder()->code == TACTICAL_ORDER_ATTACK_POINT) 
	{
		targetPosition = pilot->getAttackTargetPoint();
		relFacingToTarget = relFacingTo(targetPosition);
		//maxVehiclePivotRate = ((GroundVehicleDynamicsTypePtr)(((GroundVehicleTypePtr)type)->dynamicsType))->maxTurretYawRate * frameLength;
		hasTarget = true;
	}

	if (moveState == MOVESTATE_PIVOT_FORWARD) 
	{
		if ((moveStateGoal == MOVESTATE_PIVOT_FORWARD) || (moveStateGoal == MOVESTATE_FORWARD)) 
		{
			//----------------------------------------------------------
			// We want to pivot forward (if we have a path to follow)...
			if ((path->numStepsWhenNotPaused > 0) && (path->curStep < path->numStepsWhenNotPaused)) 
			{
				Stuff::Vector3D wayPt = path->stepList[path->curStep].destination;

				control.settings.groundVehicle.throttle = 0;
				float relFacingToWayPt = relFacingTo(wayPt);
				if ((relFacingToWayPt < -45.0) || (relFacingToWayPt > 45.0)) 
				{
					float turnRate = -relFacingToWayPt;
					if (hasTarget && !isRunning) 
					{
						if (pivotDirection == -1) 
						{
							if (relFacingToTarget < 0)
								pivotDirection = 0;
							else
								pivotDirection = 1;
						}
						if (pivotDirection == 0) 
						{
							if (relFacingToWayPt < 0.0)
								turnRate = -relFacingToWayPt;
							else
								turnRate = 360.0 - relFacingToWayPt;
						}
						else 
						{
							if (relFacingToWayPt < 0.0)
								turnRate = -360.0 - relFacingToWayPt;
							else
								turnRate = -relFacingToWayPt;
						}
					}
					float maxRate = vehicleTurnRate[long(tonnage)] * frameLength;
					if (fabs(turnRate) > maxRate) 
					{
						if (turnRate > 0.0)
							turnRate = maxRate;
						else
							turnRate = -maxRate;
					}
					
					control.settings.groundVehicle.rotate = turnRate;
					control.settings.groundVehicle.pivot = true;

					NewRotation = turnRate;
					return(true);
				}
				else 
				{
					pilot->setMoveState(MOVESTATE_FORWARD);
					if (pilot->getMoveTwisting())
						pilot->setMoveTwisting(false);
				}
			}
			else
				pilot->setMoveStateGoal(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
		}
		else
			pilot->setMoveState(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
	}
	else if (moveState == MOVESTATE_PIVOT_REVERSE) 
	{
		if ((moveStateGoal == MOVESTATE_PIVOT_REVERSE) || (moveStateGoal == MOVESTATE_REVERSE)) {
			//----------------------------------------------------------
			// We want to pivot forward (if we have a path to follow)...
			if ((path->numStepsWhenNotPaused > 0) && (path->curStep < path->numStepsWhenNotPaused)) {
				Stuff::Vector3D wayPt = path->stepList[path->curStep].destination;

				control.settings.groundVehicle.throttle = 0;
				float relFacingToWayPt = relFacingTo(wayPt);
				if ((relFacingToWayPt > -175.0) && (relFacingToWayPt < 175.0)) 
				{
					float turnRate = 0.0;
					if (hasTarget && !isRunning) 
					{
						if (pivotDirection == -1) 
						{
							if (relFacingToTarget < 0)
								pivotDirection = 0;
							else
								pivotDirection = 1;
						}
						if (pivotDirection == 0)
							turnRate = 180.0 - relFacingToWayPt;
						else
							turnRate = -180.0 - relFacingToWayPt;
					}
					else 
					{
						if (relFacingToWayPt < 0)
							turnRate = -180.0 - relFacingToWayPt;
						else
							turnRate = 180.0 - relFacingToWayPt;
					}
					float maxRate = vehicleTurnRate[long(tonnage)] * frameLength;
					if (fabs(turnRate) > maxRate) 
					{
						if (turnRate > 0.0)
							turnRate = maxRate;
						else
							turnRate = -maxRate;
					}
					control.settings.groundVehicle.rotate = turnRate;
					control.settings.groundVehicle.pivot = true;

					NewRotation = turnRate;
					return(true);
				}
				else 
				{
					//--------------------------
					// Facing reverse. Now what?
					if (pilot->getMoveTwisting())
						pilot->setMoveTwisting(false);
					if (moveStateGoal == MOVESTATE_REVERSE) 
					{
						pilot->setMoveState(MOVESTATE_REVERSE);
					}
					else
						pilot->setMoveStateGoal(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
				}
			}
			else
				pilot->setMoveStateGoal(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
		}
		else
			pilot->setMoveState(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
	}
	else if (moveState == MOVESTATE_PIVOT_TARGET) 
	{
		if (moveStateGoal == MOVESTATE_PIVOT_TARGET) 
		{
			//------------------------------------------
			// We want to pivot to our current target...
			if (isRunning || !hasTarget) 
			{
				pilot->setMoveStateGoal(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
				if (!pilot->isYielding() && !pilot->isWaitingForPoint())
					pilot->resumePath();
				pivotDirection = -1;
				return(false);
			}
			control.settings.groundVehicle.throttle = 0;
			float relFacingToTarget = relFacingTo(targetPosition);
			float fireArc = getFireArc();
			if ((relFacingToTarget < -fireArc) || (relFacingToTarget > fireArc)) 
			{
				float turnRate = -relFacingToTarget;
				float maxRate = vehicleTurnRate[long(tonnage)] * frameLength;
				if (fabs(turnRate) > maxRate) 
				{
					if (turnRate > 0.0)
						turnRate = maxRate;
					else
						turnRate = -maxRate;
				}
				control.settings.groundVehicle.rotate = turnRate;
				control.settings.groundVehicle.pivot = true;

				NewRotation = turnRate;
				return(true);
			}
			else 
			{
				pilot->setMoveStateGoal(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
			}
		}
		else
			pilot->setMoveState(MOVESTATE_FORWARD /*MOVESTATE_STAND*/);
	}
	else 
	{
		//--------------------------------------------------------
		// If we're supposed to be pivoting, set the move state...
		if ((moveStateGoal == MOVESTATE_PIVOT_TARGET) || (moveStateGoal == MOVESTATE_PIVOT_FORWARD) || (moveStateGoal == MOVESTATE_PIVOT_REVERSE))
			pilot->setMoveState(moveStateGoal);
	}

	if (!pilot->isYielding() && !pilot->isWaitingForPoint())
		pilot->resumePath();
		
	pivotDirection = -1;
	return(false);
}

//---------------------------------------------------------------------------

void GroundVehicle::calcThrottleLimits (long& minThrottle, long& maxThrottle) {

	// NOT WORKING RIGHT...
	float incline = 90.0; //getVelocityTilt();
	if (incline < 90.0) {
		maxThrottle = float2long(maxThrottle*InclineThrottleMultiplier[chassis][0]);
		minThrottle = float2long(minThrottle*InclineThrottleMultiplier[chassis][0]);
		}
	else if (incline > 90.0) {
		maxThrottle = float2long(maxThrottle*InclineThrottleMultiplier[chassis][1]);
		minThrottle = float2long(minThrottle*InclineThrottleMultiplier[chassis][1]);
	}
	
}

//---------------------------------------------------------------------------

long GroundVehicle::getSpeedState (void) {

	if (velocityMag != 0.0)
		return(SPEED_STATE_MOVING_FAST);
	return(SPEED_STATE_STATIONARY);
}

//---------------------------------------------------------------------------

void GroundVehicle::updateMoveStateGoal (void) {

	Stuff::Vector3D targetPosition;
	targetPosition.Zero();
	GameObjectPtr target = pilot->getLastTarget();
	bool hasTarget = false;
	if (target) {
		targetPosition = target->getPosition();
		hasTarget = true;
		}
	else if (pilot->getCurTacOrder()->code == TACTICAL_ORDER_ATTACK_POINT) {
		targetPosition = pilot->getAttackTargetPoint();
		hasTarget = true;
	}

	MovePathPtr path = pilot->getMovePath();
	long moveStateGoal = pilot->getMoveStateGoal();
	if (path->numSteps > 0) {
		if (MPlayer && !MPlayer->isServer()) {
			if (moveChunk.run) {
				pilot->setMoveStateGoal(MOVESTATE_FORWARD);
				return;
			}
			}
		else {
			if (pilot->getMoveRun()) {
				pilot->setMoveStateGoal(MOVESTATE_FORWARD);
				return;
			}
		}
		//----------------------------------------
		// Facing choice for movement goes here...
		if (hasTarget) {
			//------------------------------------------------
			// Check if we want to go forward or in reverse...
			if ((path->numStepsWhenNotPaused > 0) && (path->curStep < path->numStepsWhenNotPaused)) {
				Stuff::Vector3D wayPt = path->stepList[path->curStep].destination;
				float facingDelta = relFacingDelta(wayPt, targetPosition);
				float totalFireArc = getFireArc() + dynamics.max.groundVehicle.turretYaw;
				long moveStateGoal = pilot->getMoveStateGoal();
				if (moveStateGoal == MOVESTATE_FORWARD) {
					if (facingDelta > totalFireArc) {
						if ((180.0 - facingDelta) <= totalFireArc) {
							if (!pilot->getMoveTwisting()) {
								pilot->setMoveTwisting(true);
								pilot->setMoveStateGoal(MOVESTATE_REVERSE);
							}
						}
					}
					}
				else {
					if ((180.0 - facingDelta) > totalFireArc) {
						if (facingDelta <= totalFireArc) {
							if (!pilot->getMoveTwisting()) {
								pilot->setMoveTwisting(true);
								pilot->setMoveStateGoal(MOVESTATE_FORWARD);
							}
						}
					}
				}
			}
			}
		else
			pilot->setMoveStateGoal(MOVESTATE_FORWARD);
		}
	else if ((moveStateGoal != MOVESTATE_PIVOT_TARGET) && (moveStateGoal != MOVESTATE_PIVOT_FORWARD) && (moveStateGoal != MOVESTATE_PIVOT_REVERSE)) {
		if (!hasTarget && (path->numStepsWhenNotPaused == 0))
			pilot->setMoveStateGoal(MOVESTATE_FORWARD);
	}

}

//---------------------------------------------------------------------------

bool GroundVehicle::updateMovePath (float& newRotate, char& newThrottleSetting, long& newMoveState, long& minThrottle, long& maxThrottle) 
{
	
	DistanceToWaypoint = 9999.0;

	MovePathPtr path = pilot->getMovePath();
	TacticalOrderPtr curOrder = pilot->getCurTacOrder();

	long curThrottleSetting = control.settings.groundVehicle.throttle;
	newThrottleSetting = curThrottleSetting;
	newRotate = 0.0;

	bool allowedToRun = pilot->getMoveRun();

	updateHustleTime();
	bool hustle = (lastHustleTime + 2.0) > scenarioTime;

	//-----------------------------------------------
	// Am I ahead of my point vehicle, if I have one?
	bool aheadOfPointVehicle = false;
	bool stopForPointVehicle = false;
	MoverPtr pointVehicle = (MoverPtr)pilot->getPoint();
	bool hasGroupMoveOrder = (curOrder->isGroupOrder() && curOrder->isMoveOrder());
	if (!allowedToRun && !hustle && pointVehicle && !pointVehicle->isDisabled() && (pointVehicle != this) && hasGroupMoveOrder) 
	{
		MechWarriorPtr pointPilot = pointVehicle->getPilot();
		float pointDistanceFromGoal = pointPilot->getMoveDistanceLeft();
		float myDistanceFromGoal = pilot->getMoveDistanceLeft();
		aheadOfPointVehicle = (myDistanceFromGoal < pointDistanceFromGoal);
		if (aheadOfPointVehicle) {
			//allowedToRun = false;
			if (getSpeedState() == SPEED_STATE_MOVING_FAST) 
			{
				//-------------------------------------------------
				// Running. So, slow to a walk for a few seconds...
				if (!pilot->isWaitingForPoint())
					pilot->setMoveWaitForPointTime(scenarioTime + 5.0);
			}
			else 
			{
				//--------------------------------------------------------------
				// Already walking, so let's just stop until point catches up...
				if (pilot->getMoveWaitForPointTime() < scenarioTime) 
				{
					stopForPointVehicle = true;
					pilot->pausePath(); //path->numSteps = 0;
					pilot->setMoveWaitForPointTime(999999.0);
				}
			}
		}
		else 
		{
			//-------------------
			// No need to wait...
			pilot->setMoveWaitForPointTime(-1.0);
			if (!pilot->isYielding()) 
			{
				pilot->resumePath();
			}
		}
	}
	else 
	{
		pilot->setMoveWaitForPointTime(-1.0);
	}

	bool goalReached = false;
	if (path->numSteps > 0) 
	{
		//---------------------------------------------------------------
		// First, make sure we are not already at the end of this path...
		if (path->curStep == path->numSteps)
			goalReached = true;
		else 
		{
			Stuff::Vector3D wayPt = path->stepList[path->curStep].destination;

			//---------------------------------------------------------
			// Let's record our last valid position, in case we need to
			// crawl back from impassable terrain we get knocked onto.
			// Ultimately, this SHOULD NOT be necessary once we agree
			// on a new gesture/movement system... gd 6/2/97
			lastValidPosition = wayPt;

			//---------------------------------
			// Have we reached the destination?
			float distanceFromWayPt = distanceFrom(wayPt);
			DistanceToWaypoint = distanceFromWayPt;

			//--------------------------------------------------
			//Calculate how far the vehicle will move this frame.
			// Vel is in m/s
			float vel = velocityMag;
			float distanceThisFrame = vel * frameLength;

			float cushion = Mover::marginOfError[0];
			if (path->curStep == (path->numSteps - 1))
				cushion = Mover::marginOfError[1];
				
			if (cushion < distanceThisFrame)
			{
				//------------------------------------------------------------------------
				//We are going to move farther then the current cushion would allow for.
				// Make the cushion larger.
				// Maybe we should make cushion this value all of the time?
				cushion = distanceThisFrame;
			}

			if (distanceFromWayPt < cushion) 
			{
				//-------------------------------------------
				// Reached it, so go to the next waypoint...
				path->curStep++;
				pilot->setMoveTimeOfLastStep(scenarioTime);
				if (path->curStep < path->numSteps) 
				{
					wayPt = path->stepList[path->curStep].destination;
				}
				else 
				{
					goalReached = true;
				}
			}

			if (!goalReached) 
			{
				//-----------------------------------------
				// First, rotate the vehicle (if moving)...

				float relFacingToWayPt = relFacingTo(wayPt);
				long moveState = pilot->getMoveState();
				long moveStateGoal = pilot->getMoveStateGoal();

				if (moveState == MOVESTATE_FORWARD) 
				{
					if (moveStateGoal == MOVESTATE_FORWARD) 
					{
						//----------------------
						// Keep going forward...

						newThrottleSetting = 100;
						
						if ((relFacingToWayPt < -5.0) || (relFacingToWayPt > 5.0)) 
						{
							//-----------------------------------------------
							// We can and will shift facing to destination...
							newRotate = -relFacingToWayPt;
							float maxRate = vehicleTurnRate[long(tonnage)] * frameLength;
							if (fabs(newRotate) > maxRate)
							{
								if (fabs(newRotate) < 50.0f)
									newThrottleSetting = 100.0f - fabs(newRotate);
								else
									newThrottleSetting = 0.0f;
 							
								if (newRotate > 0.0)
									newRotate = maxRate;
								else
									newRotate = -maxRate;
							}
						}
					}
					else if (moveStateGoal == MOVESTATE_REVERSE) 
					{
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_REVERSE;
					}
					else if (moveStateGoal == MOVESTATE_PIVOT_FORWARD) 
					{
						//---------------------------------------------------
						// Stop, and pivot fully forward to next path step...
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_FORWARD;
					}
					else if (moveStateGoal == MOVESTATE_PIVOT_REVERSE) 
					{
						//---------------------------------------------------
						// Stop, and pivot fully reverse to next path step...
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_REVERSE;
					}
					else 
					{
						//--------
						// Stop...
						pilot->pausePath();
						newMoveState = MOVESTATE_FORWARD /*MOVESTATE_STAND*/;
					}
				}
				else if (moveState == MOVESTATE_REVERSE) 
				{
					if (moveStateGoal == MOVESTATE_FORWARD) 
					{
						//---------------------------------------------------
						// Stop, and pivot fully forward to next path step...
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_FORWARD;
					}
					else if (moveStateGoal == MOVESTATE_REVERSE) 
					{
						newThrottleSetting = -100;
						//--------------------------
						// Keep moving in reverse...
						if (relFacingToWayPt < 0)
							newRotate = -(relFacingToWayPt + 180.0);
						else
							newRotate = -(relFacingToWayPt - 180.0);
							
						float maxRate = vehicleTurnRate[long(tonnage)] * frameLength;
						if (fabs(newRotate) > maxRate) 
						{
							newThrottleSetting = -50.0f;
							
							if (fabs(newRotate) < 50.0f)
								newThrottleSetting = -50.0f + fabs(newRotate);
							else
								newThrottleSetting = 0.0f;
									
 							if (newRotate > 0.0) 
							{
								newRotate = maxRate;
							}
							else 
							{
								newRotate = -maxRate;
							}
						}
						else 
						{
							//------------------------------------
							// Try to attain current goal speed...
							newThrottleSetting = -100;
						}
					}
					else if (moveStateGoal == MOVESTATE_PIVOT_FORWARD) 
					{
						//---------------------------------------------------
						// Stop, and pivot fully forward to next path step...
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_FORWARD;
					}
					else if (moveStateGoal == MOVESTATE_PIVOT_REVERSE) 
					{
						//---------------------------------------------------
						// Stop, and pivot fully reverse to next path step...
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_REVERSE;
					}
					else 
					{
						//--------
						// Stop...
						pilot->pausePath();
						newMoveState = MOVESTATE_FORWARD /*MOVESTATE_STAND*/;
					}
				}
				else 
				{
					//--------------------------
					// Not moving--should we be?
					if ((moveStateGoal == MOVESTATE_FORWARD) || (moveStateGoal == MOVESTATE_PIVOT_FORWARD)) 
					{
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_FORWARD;
					}
					else if ((moveStateGoal == MOVESTATE_REVERSE) || (moveStateGoal == MOVESTATE_PIVOT_REVERSE)) 
					{
						pilot->pausePath();
						newMoveState = MOVESTATE_PIVOT_REVERSE;
					}
				}
			}
		}
	}
	else 
	{
		//-------------------------------------------
		// We better not be walking or running, then!
		newThrottleSetting = 0;
	}

	// if we're a sweeping minesweeper or a laying minelayer, take it slow
	if (mineSweeper && sweepTime > 0 && sweepTime < gvSweepTime)
		maxThrottle = MineSweepThrottle;
	if ((mineLayer != 0) && pilot->getCurTacOrder()->moveParams.mode == MOVE_MODE_MINELAYING)
		maxThrottle = MineLayThrottle;

	if (goalReached) 
	{
		//--------------------------------------------------
		// Did we finish just a local part of a global path?
		if (pilot->getMovePathType() == MOVEPATH_COMPLEX) 
		{
			if (pilot->getMovePathGlobalStep() < (pilot->getMoveNumGlobalSteps() - 1)) 
			{
				//-----------------------------------------------------------
				// We've reached the end of our current short-range path, but
				// we have more to go before we've reached our long-range
				// goal, so calc our next short-range path...
				goalReached = false;
			}
		}
		pilot->clearMovePath(0);
		newThrottleSetting = 0;
	}

	return(goalReached);
}

//---------------------------------------------------------------------------

void GroundVehicle::setNextMovePath (char& newThrottleSetting) {

	//----------------------------------------
	// If this is only an intermediate path,
	// let's check where we need to go next...
	//pilot->clearMovePath(ORDER_CURRENT);

	Stuff::Vector3D nextWayPoint;
	bool haveWayPoint = pilot->getNextWayPoint(nextWayPoint, true);
	if (haveWayPoint) {
		pilot->setMoveGoal(MOVEGOAL_LOCATION, &nextWayPoint);
		TacticalOrderPtr curTacOrder = pilot->getCurTacOrder();
		pilot->requestMovePath(curTacOrder->selectionIndex, MOVEPARAM_FACE_TARGET + MOVEPARAM_FOLLOW_ROADS, 0);
		}
	else {
		pilot->clearMoveOrders();
		//pilot->setMoveGoal(MOVEGOAL_NONE, NULL);
		newThrottleSetting = 0;
	}
}

//---------------------------------------------------------------------------

void GroundVehicle::setControlSettings (float& newRotate, char& newThrottleSetting, long& minThrottle, long& maxThrottle) {

	//-----------------------------------------------------------------------------
	// If we have no move path, then we shouldn't be moving. Let's set our throttle
	// setting to zero, just to be safe...
	long result = NO_ERR;
	MovePathPtr curPath = pilot->getMovePath();
	bool allowedToRun = false;
	if (MPlayer && !MPlayer->isServer())
		allowedToRun = moveChunk.run;
	else
		allowedToRun = pilot->getMoveRun();
	
	if (curPath->numSteps == 0)
		newThrottleSetting = 0;

	result = NO_ERR;
	if (newThrottleSetting != -1) {
		if (newThrottleSetting < minThrottle)
			newThrottleSetting = minThrottle;
		else if (newThrottleSetting > maxThrottle)
			newThrottleSetting = maxThrottle;
		control.settings.groundVehicle.throttle = newThrottleSetting;
	}
	
	if (newRotate != 0.0)
		control.settings.groundVehicle.rotate = newRotate;
		
	control.settings.groundVehicle.isWalking = !allowedToRun;
}

//---------------------------------------------------------------------------

void GroundVehicle::updateTurret (float newRotatePerSec) {

	//----------------------------------------------------
	// Now, rotate the turret toward our current target...
	float turretRelFacing = 0.0;
	GameObjectPtr target = pilot->getCurrentTarget();
	if (target)
		turretRelFacing = relFacingTo(target->getPosition()) + turretRotation + newRotatePerSec;
	else if (pilot->getCurTacOrder()->code == TACTICAL_ORDER_ATTACK_POINT)
		turretRelFacing = relFacingTo(pilot->getAttackTargetPoint()) + turretRotation + newRotatePerSec;
	else
		turretRelFacing = turretRotation;

	float fireArc = getFireArc();
	if ((turretRelFacing < -fireArc) || (turretRelFacing > fireArc)) {
		float newRotateTurret = -turretRelFacing;
		//-----------------------------------------------
		// We can and will shift facing to destination...
		float maxRate = (float)dynamics.max.groundVehicle.turretYawRate * frameLength;
		if (fabs(newRotateTurret) > maxRate) {
			if (newRotateTurret < 0.0)
				newRotateTurret = -maxRate;
			else
				newRotateTurret = maxRate;
		}
		control.settings.groundVehicle.rotateTurret = newRotateTurret;
	}
}

//---------------------------------------------------------------------------

void GroundVehicle::updateMovement (void) {

	NewRotation = 0.0;

	if (disableThisFrame) {
		disableThisFrame = false;
		shutDownThisFrame = false;
		startUpThisFrame = false;
		setStatus(OBJECT_STATUS_DISABLED);
		control.settings.groundVehicle.throttle = 0;
		return;
	}

	if (shutDownThisFrame) {
		shutDownThisFrame = false;
		startUpThisFrame = false;
		setStatus(OBJECT_STATUS_SHUTDOWN);
		control.settings.groundVehicle.throttle = 0;
		soundSystem->playDigitalSample(VEHICLE_POWERUP,getPosition());
		return;
	}

	//------------------------------------------------
	// Do the vehicles have any form of "starting up"?
	if (startUpThisFrame) {
		startUpThisFrame = false;
		setStatus(OBJECT_STATUS_NORMAL);
		soundSystem->playDigitalSample(VEHICLE_POWERDN,getPosition());
		control.settings.groundVehicle.throttle = 100;
		return;
	}

	if (isCaptured() || isDisabled()) {
		control.settings.groundVehicle.throttle = 0;
		return;
	}

	if (engineBlowTime > -1.0)
		return;

	if (pivotTo())
		return;

	long minThrottle = -100;
	long maxThrottle = 100;
	float newRotate = 0.0;
	char newThrottleSetting = 0;
	long newMoveState = -1;
	bool goalReached = false;

	calcThrottleLimits(minThrottle, maxThrottle);
	
//	if (!pilot->isYielding())
		goalReached = updateMovePath(newRotate, newThrottleSetting, newMoveState, minThrottle, maxThrottle);

	if (goalReached)
		setNextMovePath(newThrottleSetting);

	if (newMoveState != -1)
		pilot->setMoveState(newMoveState);

	setControlSettings(newRotate, newThrottleSetting, minThrottle, maxThrottle);

	updateMoveStateGoal();

	NewRotation = newRotate;
}

//---------------------------------------------------------------------------
// NETWORK MOVEMENT UPDATE ROUTINES
//---------------------------------------------------------------------------

bool GroundVehicle::netUpdateMovePath (float& newRotate, char& newThrottleSetting, long& newMoveState, long& minThrottle, long& maxThrottle) {

	DistanceToWaypoint = 9999.0;

	MovePathPtr path = pilot->getMovePath();

	long curThrottleSetting = control.settings.groundVehicle.throttle;
	newThrottleSetting = curThrottleSetting;
	newRotate = 0.0;

	bool goalReached = false;
	if (path->numSteps > 0) {
		//---------------------------------------------------------------
		// First, make sure we are not already at the end of this path...
		if (path->curStep == path->numSteps)
			goalReached = true;
		else {
			Stuff::Vector3D wayPt = path->stepList[path->curStep].destination;

			//---------------------------------------------------------
			// Let's record our last valid position, in case we need to
			// crawl back from impassable terrain we get knocked onto.
			// Ultimately, this SHOULD NOT be necessary once we agree
			// on a new gesture/movement system... gd 6/2/97
			lastValidPosition = wayPt;

			//---------------------------------
			// Have we reached the destination?
			float distanceFromWayPt = distanceFrom(wayPt);
			DistanceToWaypoint = distanceFromWayPt;

			float cushion = Mover::marginOfError[0];
			if (path->curStep == (path->numSteps - 1))
				cushion = Mover::marginOfError[1];
			if (distanceFromWayPt < cushion) {
				//-------------------------------------------
				// Reached it, so go to the next waypoint...
				path->curStep++;
				pilot->setMoveTimeOfLastStep(scenarioTime);
				if (path->curStep < path->numSteps) {
					wayPt = path->stepList[path->curStep].destination;
					}
				else {
					goalReached = true;
				}
			}

			if (!goalReached) {
				//-----------------------------------------
				// First, rotate the vehicle (if moving)...

				float relFacingToWayPt = relFacingTo(wayPt);
				long moveState = pilot->getMoveState();
				long moveStateGoal = pilot->getMoveStateGoal();

				if (moveState == MOVESTATE_FORWARD) {
					if (moveStateGoal == MOVESTATE_FORWARD) {
						//----------------------
						// Keep going forward...

						newThrottleSetting = 100;
						
						if ((relFacingToWayPt < -5.0) || (relFacingToWayPt > 5.0)) {
							//-----------------------------------------------
							// We can and will shift facing to destination...
							newRotate = -relFacingToWayPt;
							float maxRate = dynamics.max.groundVehicle.yawRate * frameLength;
							if (fabs(newRotate) > maxRate) {
								if (newRotate > 0.0)
									newRotate = maxRate;
								else
									newRotate = -maxRate;
							}
						}
						
						}
					else if (moveStateGoal == MOVESTATE_REVERSE) {
						//---------------------------------------------------
						// Stop, and pivot fully reverse to next path step...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_REVERSE;
						}
					else if (moveStateGoal == MOVESTATE_PIVOT_FORWARD) {
						//---------------------------------------------------
						// Stop, and pivot fully forward to next path step...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_FORWARD;
						}
					else if (moveStateGoal == MOVESTATE_PIVOT_REVERSE) {
						//---------------------------------------------------
						// Stop, and pivot fully reverse to next path step...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_REVERSE;
						}
					else {
						//--------
						// Stop...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_FORWARD /*MOVESTATE_STAND*/;
					}
					}
				else if (moveState == MOVESTATE_REVERSE) {
					if (moveStateGoal == MOVESTATE_FORWARD) {
						//---------------------------------------------------
						// Stop, and pivot fully forward to next path step...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_FORWARD;
						}
					else if (moveStateGoal == MOVESTATE_REVERSE) {
						newThrottleSetting = -100;
						//--------------------------
						// Keep moving in reverse...
						if (relFacingToWayPt < 0)
							newRotate = -(relFacingToWayPt + 180.0);
						else
							newRotate = -(relFacingToWayPt - 180.0);
						float maxRate = dynamics.max.groundVehicle.yawRate * frameLength;
						if (fabs(newRotate) > maxRate) {
							if (newRotate > 0.0) {
								newRotate = maxRate;
								newThrottleSetting = -50;
								}
							else {
								newRotate = -maxRate;
								newThrottleSetting = -50;
							}
							}
						else {
							//------------------------------------
							// Try to attain current goal speed...
							newThrottleSetting = -100;
						}

						}
					else if (moveStateGoal == MOVESTATE_PIVOT_FORWARD) {
						//---------------------------------------------------
						// Stop, and pivot fully forward to next path step...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_FORWARD;
						}
					else if (moveStateGoal == MOVESTATE_PIVOT_REVERSE) {
						//---------------------------------------------------
						// Stop, and pivot fully reverse to next path step...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_REVERSE;
						}
					else {
						//--------
						// Stop...
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_FORWARD /*MOVESTATE_STAND*/;
					}
					}
				else {
					//--------------------------
					// Not moving--should we be?
					if ((moveStateGoal == MOVESTATE_FORWARD) || (moveStateGoal == MOVESTATE_PIVOT_FORWARD)) {
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_FORWARD;
						}
					else if ((moveStateGoal == MOVESTATE_REVERSE) || (moveStateGoal == MOVESTATE_PIVOT_REVERSE)) {
						//((MechActor*)appearance)->forceStop();
						pilot->pausePath(); //pilot->getMovePath()->numSteps = 0;
						newMoveState = MOVESTATE_PIVOT_REVERSE;
					}
				}
			}
		}
		}
	else {
		//-------------------------------------------
		// We better not be walking or running, then!
		newThrottleSetting = 0;
	}

	// if we're a sweeping minesweeper or a laying minelayer, take it slow
	if (mineSweeper && sweepTime > 0 && sweepTime < gvSweepTime)
		maxThrottle = MineSweepThrottle;
	if ((mineLayer != 0) && pilot->getCurTacOrder()->moveParams.mode == MOVE_MODE_MINELAYING)
		maxThrottle = MineLayThrottle;

	return(goalReached);
}

//---------------------------------------------------------------------------

void GroundVehicle::netUpdateMovement (void) {

	MovePathPtr path = pilot->getMovePath();
	bool atEndOfPath = (path->curStep == (path->numSteps - 1));
	
	Stuff::Vector3D wayPt;
	float distanceFromWayPt = 1000000.0;
	if ((path->curStep > -1) && (path->curStep < path->numSteps)) {
		wayPt = path->stepList[path->curStep].destination;
		distanceFromWayPt = distanceFrom(wayPt);
	}
	if (atEndOfPath && (distanceFromWayPt < Mover::marginOfError[1])) 
	{
		//----------------------------------------------
		// Movement states no longer exist.  Just stop!
		pilot->clearMoveOrders();
		control.settings.groundVehicle.throttle = 0;
	}

	if (disableThisFrame) {
		disableThisFrame = false;
		shutDownThisFrame = false;
		startUpThisFrame = false;
		setStatus(OBJECT_STATUS_DISABLED);
		control.settings.groundVehicle.throttle = 0;
		return;
	}

	if (shutDownThisFrame) {
		shutDownThisFrame = false;
		startUpThisFrame = false;
		setStatus(OBJECT_STATUS_SHUTDOWN);
		control.settings.groundVehicle.throttle = 0;
		return;
	}

	//------------------------------------------------
	// Do the vehicles have any form of "starting up"?
	if (startUpThisFrame) {
		startUpThisFrame = false;
		setStatus(OBJECT_STATUS_NORMAL);
		control.settings.groundVehicle.throttle = 100;
		return;
	}

	if (isCaptured() || isDisabled()) {
		control.settings.groundVehicle.throttle = 0;
		return;
	}

	if (engineBlowTime > -1.0)
		return;

	if (pivotTo())
		return;

	long minThrottle = -100;
	long maxThrottle = 100;
	float newRotate = 0;
	char newThrottleSetting = 0;
	long newMoveState = -1;
	bool goalReached = false;

	calcThrottleLimits(minThrottle, maxThrottle);
	
	goalReached = netUpdateMovePath(newRotate, newThrottleSetting, newMoveState, minThrottle, maxThrottle);

	if (newMoveState != -1)
		pilot->setMoveState(newMoveState);

	setControlSettings(newRotate, newThrottleSetting, minThrottle, maxThrottle);

	updateMoveStateGoal();
	NewRotation = newRotate;
}

//----------------------------------------------------------------------------------
// END OF MOVEMENT UPDATE ROUTINES
//----------------------------------------------------------------------------------

Stuff::Vector3D GroundVehicle::getPositionFromHS (long nodeId) 
{

	// NEED TO IMPLEMENT PROPER HOTSPOTS FOR VEHICLES...
	// How about now?
	// -fs

	//-----------------------------------------
	Stuff::Vector3D hsPos = position;
	if (appearance)
	{
		hsPos = appearance->getHitNode();
		if ((nodeId != -1) || (hsPos == position))
		{
			hsPos = appearance->getWeaponNodePosition(nodeId);
		}
	}

	return(hsPos);
}

#ifdef PROFILE
extern long srCtrlUpd;
extern long srApprUpd;
extern long srDyneUpd;
extern long srWeapUpd;
extern long srObjtUpd;
#endif

//extern L_INTEGER startCk;
//extern L_INTEGER endCk;

//----------------------------------------------------------------------------------
void GroundVehicle::disable (unsigned long cause)
{
	Mover::disable(cause);
	timeLeft = 0.0;
	
#ifdef USE_SMOKE
	dmgSmoke = (SmokePtr)createObject(450);
	if (dmgSmoke)
	{
		dmgSmoke->setOwner(this);
		dmgSmoke->setOwnerPosition(position);
	}
#endif
}	

//----------------------------------------------------------------------------------

float GroundVehicle::getStatusRating (void) {

	//-------------------------------
	// calculate Weapon effectiveness
	float weaponEffect = 1.0;
	if (maxWeaponEffectiveness)
		weaponEffect = (float)weaponEffectiveness / (float)maxWeaponEffectiveness;

	//----------------
	// Calculate armor
	float armorFront = armor[GROUNDVEHICLE_LOCATION_FRONT].curArmor / armor[GROUNDVEHICLE_LOCATION_FRONT].maxArmor * 0.6 + 0.4;
	float armorLeft = armor[GROUNDVEHICLE_LOCATION_LEFT].curArmor / armor[GROUNDVEHICLE_LOCATION_LEFT].maxArmor * 0.6 + 0.4;
	float armorRight = armor[GROUNDVEHICLE_LOCATION_RIGHT].curArmor / armor[GROUNDVEHICLE_LOCATION_RIGHT].maxArmor * 0.6 + 0.4;
	float armorRear = armor[GROUNDVEHICLE_LOCATION_REAR].curArmor / armor[GROUNDVEHICLE_LOCATION_REAR].maxArmor * 0.6 + 0.4;
	float armorEffect = armorFront * armorRear * armorLeft * armorRight;
				
	//-----------------------
	// Calculate pilot Wounds
	float pilotWoundTable[7] = {1.00f,0.95f,0.85f,0.75f,0.50f,0.30f,0.00f};
	float pilotEffect = pilotWoundTable[(long)getPilot()->getWounds()];
	if (isDestroyed() || isDisabled())
		pilotEffect = 0.0;
					
	float rating = weaponEffect * armorEffect * pilotEffect;
	return(rating);
}

//----------------------------------------------------------------------------------

bool GroundVehicle::crashAvoidanceSystem (void) {

	if (MPlayer && !MPlayer->isServer())
		return(false);

	MovePathPtr path = pilot->getMovePath();

	if (path->numStepsWhenNotPaused == 0)
		return(false);

	if (pilot->getMoveWaitForPointTime() > 999990.0)
		return(false);

	Stuff::Vector3D vel = getRotationVector();
	vel *= -velocityMag;
	vel *= frameLength;
	vel *= worldUnitsPerMeter;

	Stuff::Vector3D newPosition;
	newPosition.Add(position, vel);

	//---------------------------------------------------------------------
	// Is this new position in a pathlocked area? If so, put on the brakes!
	int cellR, cellC;
	land->worldToCell(newPosition, cellR, cellC);

	//-------------------------
	// To avoid corner stops...
	bool clippingCorner = false;
	int dir = path->getDirection(path->curStep);
	if ((dir == 1) || (dir == 3) || (dir == 5) || (dir == 7)) {
		bool firstCornerClipped = getAdjacentCellPathLocked((moveLevel == 2), cellPositionRow, cellPositionCol, adjClippedCell[dir][0]);
		bool secondCornerClipped = getAdjacentCellPathLocked((moveLevel == 2), cellPositionRow, cellPositionCol, adjClippedCell[dir][1]);
		clippingCorner = (firstCornerClipped && secondCornerClipped);
	}

	//bool stepOnMover = getPathLocked(tileR, tileC, cellR, cellC, crashAvoidSelf);
	bool reachedEnd;
	bool blockReachedEnd;
	bool nearingMover = getPathRangeLock(crashAvoidPath, &reachedEnd);
	bool pathLocked = /*stepOnMover ||*/ nearingMover;
	bool pathBlocked = getPathRangeBlocked(crashAvoidPath, &blockReachedEnd);
	long stepIntoGate = (path->crossesClosedGate(-1, 2) > 1);

	if (pilot->isYielding()) {
		if (pathLocked || pathBlocked || clippingCorner || stepIntoGate) {
			pilot->pausePath(); //path->numSteps = 0; //in theory, this should already be zero, yet is sometimes not. WHY?!
			return(true);
			}
		else {
			//-------------------
			// It's clear, now...
			pilot->resumePath();
			pilot->setMoveYieldTime(-1.0);
		}
		}
	else {
		if (pathLocked || pathBlocked || clippingCorner || stepIntoGate) {
			//-------------------------------------------------------------------
			// What exactly is wrong here? Is our goal currently occupied? If so,
			// and it's our final goal, then stop!
			if (reachedEnd || blockReachedEnd) {
				//-----------------------------------------------------------------------
				// Since we're practically there, let's just kill the current move orders
				// and let the pilot's movementDecision update decide whether a new
				// one should be set...
				/*
				pilot->setMoveWayPath(ORDER_CURRENT, NULL, 0);
				for (long i = 0; i < 2; i++)
					pilot->clearMovePath(ORDER_CURRENT, i);
				pilot->setMoveGlobalPath(ORDER_CURRENT, NULL, 0);
				*/
				//pilot->clearMoveOrders(ORDER_CURRENT);
				pilot->rethinkPath(0);
				//pilot->pausePath();
				//pilot->setMoveYieldTime(scenarioTime + crashYieldTime);
				control.brake();
				}
			else {
				pilot->pausePath();
				pilot->setMoveYieldTime(scenarioTime + crashYieldTime);
				control.brake();
			}
			return(true);
		}
	}
	
	return(false);
}

//Infantry Guy
#define VEHICLEPILOT_ID		638
//----------------------------------------------------------------------------------
void GroundVehicle::createVehiclePilot (void)
{
	//We are ALREADY a vehicle pilot, stop breeding.
	if (isVehiclePilot)
		return;

	//Don't let powered armor or infantry create a vehicle pilot.
	if (!pathLocks)
		return;

	GroundVehiclePtr vehiclePilot = ObjectManager->getOpenVehicle();
	if (!vehiclePilot)
		return;

	ObjectTypePtr objType = ObjectManager->getObjectType(VEHICLEPILOT_ID);
	if (!objType)
		return;

	//No guys allowed to come out in the water!!
	int cellR, cellC;
	land->worldToCell(position,cellR, cellC);
	if (GameMap->getDeepWater(cellR, cellC) || GameMap->getShallowWater(cellR, cellC))
		return;

	vehiclePilot->init(true, objType);

	//----------------------------------------------
	// Load the profile data into the game object...
	if (ObjectTypeManager::objectFile->seekPacket(VEHICLEPILOT_ID) == NO_ERR) 
	{
		//--------------------------------------------------------
		FitIniFile profileFile;
		long result = profileFile.open(ObjectTypeManager::objectFile,ObjectTypeManager::objectFile->getPacketSize());
		if (result == NO_ERR) 
		{
			//-------------------------------------------
			result = vehiclePilot->init(&profileFile);
			if (result != NO_ERR)
				return;
		}
		else
		{
			return;
		}

		profileFile.close();
	}
				
	vehiclePilot->setExists(true);
	vehiclePilot->setAwake(true);

	if (vehiclePilot->sensorSystem)
		SensorManager->removeTeamSensor(teamId, vehiclePilot->sensorSystem);

	vehiclePilot->sensorSystem = NULL;
	vehiclePilot->isVehiclePilot = true;

	//-------------------------------------------
	// <deleted really obnoxious Glenn Slam>
	// Thanks!

	vehiclePilot->pilot = pilot;		//Gets Current Vehicle Pilot.
	vehiclePilot->pilotHandle = pilotHandle;
	pilot->setVehicle(vehiclePilot);
	pilot->lobotomy();

	vehiclePilot->setControl(CONTROL_AI);	//AI Control
	
	//They get added to the home team so they are always visible.
	vehiclePilot->teamId = Team::home->getId();

	//BUT they are enemies on the GUI!!
	vehiclePilot->commanderId = 1;

	//------------------------------------------------------------------
	// Set the object's position, initial gesture and rotation.
	vehiclePilot->setPosition(position);
	vehiclePilot->setLastValidPosition(position);
				
	//--------------------------------------------------------------------
	// Add the object to the object list, if it exists
	// Otherwise, add it to the holder list which is a member of scenario
	ObjectManager->buildMoverLists();					

	//---------------------------------------------------------------
	// We need to override the TacOrder of the pilot so that the
	// little marine goes running about like he is trying to escape.
	pilot->clearAttackOrders();
	pilot->clearMoveOrders();
	
	Stuff::Vector3D location;
	location.Zero();
	
	pilot->orderMoveToPoint (false, true, ORDER_ORIGIN_PLAYER, location, -1, TACORDER_PARAM_RUN);

	vehiclePilot->getAppearance()->resetPaintScheme(0x00ffffff,
													0x00ffffff,
													0x00ff8000);
	vehiclePilot->setTangible(true);
	ObjectManager->rebuildCollidableList = true;
}	

//----------------------------------------------------------------------------------

void GroundVehicle::updateAIControl (void) {

	control.reset();

	if (getAwake())	{
		if (turretBlownThisFrame)
			turretBlownThisFrame = false;

		updateDamageTakenRate();

		if (!isDisabled() && pilot->alive()) {
			if (getTeamId() > -1)
				pilot->mainDecisionTree();
			updateMovement();
			}
		else if (shutDownThisFrame || disableThisFrame)
			updateMovement();
	}
}

//---------------------------------------------------------------------------

void GroundVehicle::updateNetworkControl (void) {

	control.reset();

	if (getAwake())	{
		if (turretBlownThisFrame)
			turretBlownThisFrame = false;

		//-----------------------------------------
		// Update any weaponfire chunks received...
		updateWeaponFireChunks(CHUNK_RECEIVE);

		updateCriticalHitChunks(CHUNK_RECEIVE);

		updateRadioChunks(CHUNK_RECEIVE);

		if (!isDisabled() && pilot->alive()) {
			pilot->checkAlarms();
			netUpdateMovement();
			}
		else if (shutDownThisFrame || disableThisFrame) {
			netUpdateMovement();
		}
	}
}

//---------------------------------------------------------------------------

void GroundVehicle::updatePlayerControl (void) {

	control.reset();

#if 0

	//-----------------------------------------------------------------
	// Poll the joystick and keyboards here so player can control mech.
	if (userInput->getKeyDown(KEY_T))
		control.settings.groundVehicle.rotate = dynamics.max.groundVehicle.yawRate / 4.0 * frameLength;

	if (userInput->getKeyDown(KEY_Y))
		control.settings.groundVehicle.rotate = -dynamics.max.groundVehicle.yawRate / 4.0 * frameLength;

	if (userInput->getKeyDown(KEY_INSERT))
		control.settings.groundVehicle.rotateTurret = dynamics.max.groundVehicle.yawRate / 4.0 * frameLength;

	if (userInput->getKeyDown(KEY_DELETE))
		control.settings.groundVehicle.rotateTurret = -dynamics.max.groundVehicle.yawRate / 4.0 * frameLength;

	if (userInput->getKeyDown(KEY_NEXT)) {
		control.settings.groundVehicle.throttle -= 10;
		if (control.settings.groundVehicle.throttle < 50)
			control.settings.groundVehicle.throttle = 50;
	}
		
	if (userInput->getKeyDown(KEY_PRIOR)) {
		control.settings.mech.throttle += 10;
		if (control.settings.mech.throttle > 100)
			control.settings.mech.throttle = 100;
	}
		
	if (userInput->getKeyDown(KEY_1))
		appearance->setGestureGoal(0);

	if (userInput->getKeyDown(KEY_2))
		appearance->setGestureGoal(1);		//Stand

	if (userInput->getKeyDown(KEY_3))
		appearance->setGestureGoal(2);		//Walk

	if (userInput->getKeyDown(KEY_4))
		appearance->setGestureGoal(3);		//Run

	if (userInput->getKeyDown(KEY_R))
		appearance->setGestureGoal(4);		//Reverse

	if (userInput->getKeyDown(KEY_5))	
		appearance->setGestureGoal(5);		//Limp
			
	if (userInput->getKeyDown(KEY_6))
		appearance->setGestureGoal(7);		//Fall Forward 
		
	if (userInput->getKeyDown(KEY_7))
		appearance->setGestureGoal(8);		//Fall Backward 

//	if (userInput->getKeyDown(KEY_8) && turn > 1)
//		((MechActor*)appearance)->hitMech(-1);		//Take a hit.
			
	if (userInput->getKeyDown(KEY_Z))
		ObjectManager->getObjectType(typeHandle)->handleDestruction(this, NULL);		//Blow self up!!

	if (userInput->getKeyDown(KEY_X))
		control.settings.mech.blowLeftArm = true;

	if (userInput->getKeyDown(KEY_J)) {
		Stuff::Vector3D jumpDest = getRotationVector();
		jumpDest *= 150.0;
		jumpDest += position;
		((MechActor *)appearance)->setJumpParameters(jumpDest);
		appearance->setGestureGoal(6);		//Jump
	}
		
//	if (userInput->getKeyDown(KEY_Y) && turn > 1)
//		((MechActor *)appearance)->hitMech(1);
		
//	if (userInput->getKeyDown(KEY_T) && turn > 1)
//		((MechActor *)appearance)->hitMech(-1);
		
	if (userInput->getKeyDown(KEY_G))
		((MechActor *)appearance)->setCombatMode(1);

	if (userInput->getKeyDown(KEY_F))
		((MechActor *)appearance)->setCombatMode(0);

	if (userInput->getKeyDown(KEY_C))
		control.settings.mech.blowRightArm = true;
#endif
}

//---------------------------------------------------------------------------

void GroundVehicle::updateDynamics (void) {

	if (!appearance)
		Fatal(0, " Groundvehicle.updateDynamics: no appearance ");

	if (isDestroyed())
		return;

	float yawRate = control.settings.groundVehicle.rotate;
	
	float turretYawRate = control.settings.groundVehicle.rotateTurret;

	//--------------------------------------------------------------------
	// Rotate turret by turret yaw rate, making sure we don't turn too far.
	if (turretYawRate != 0.0) {
		float newTurretRotation = turretRotation;

		if (newTurretRotation > dynamics.max.groundVehicle.turretYaw) {
			newTurretRotation = dynamics.max.groundVehicle.turretYaw;
			turretYawRate = 0.0;
		}

		if (newTurretRotation < -dynamics.max.groundVehicle.turretYaw) {
			newTurretRotation = -dynamics.max.groundVehicle.turretYaw;
			turretYawRate = 0.0;
		}

		if ((newTurretRotation + turretYawRate) > dynamics.max.groundVehicle.turretYaw)
			turretYawRate = dynamics.max.groundVehicle.turretYaw - newTurretRotation;

		if ((newTurretRotation + turretYawRate) < -dynamics.max.groundVehicle.turretYaw)
			turretYawRate = -dynamics.max.groundVehicle.turretYaw - newTurretRotation;

		newTurretRotation += turretYawRate;
		turretRotation = newTurretRotation;
	}

	rotate(yawRate);

	//------------------------------------------------------------------------
	// The gesture is how we control speed.  So, at this point, check
	// the vehicleControlData->gestureGoal and see if we need to change goals.
	// See if throttle settings work.
	float throttle = control.settings.groundVehicle.throttle;
	throttle /= 100.0;

	//--------------------------------
	// Ok, real vehicle dynamics.
	// Throttle controls maxVelocity.  Rate of velocity change is constant.
	// 50% throttle means vehicle accelerates at 50% of maxAccel upto 50% of maxVelocity
	float realMaxVelocity = dynamics.max.groundVehicle.speed;
	if (control.settings.groundVehicle.isWalking)
		realMaxVelocity = gvWalkSpeed;
		
	float maxThrottledVel = throttle * realMaxVelocity;
	float velDiff = maxThrottledVel - velocityMag;
	
	//-----------------------------------------------------
	// If we're slowing down, slow us down!
	if (((velDiff < 0.0) && (accel > 0.0)) || ((velDiff > 0.0) && (accel < 0.0)))
		accel = -accel;
	
	float velChangeThisFrame = accel * frameLength;
	if (fabs(velChangeThisFrame) > fabs(velDiff))
		velChangeThisFrame = velDiff;
	
	velocityMag += velChangeThisFrame;
}

//---------------------------------------------------------------------------
long GroundVehicle::updateAnimations (void)
{
	//-------------------------------------------------
	// Always Animate ground vehicles completely.
	// If the animations do not exist, it ain't no thing.
	// Destroyed/disabled ground vehicles do nothing!
	// Ground vehicle gestures are:
	//		0 - Stand/Idle
	// 		1 - Walk
	//		2 - Run
	//		3 - Repair
	//
	if (!MPlayer)		//ONLY allowed to animate outside of Multiplayer.  Do not want to pass anim info around.  TOO Late!
	{
		long animState = appearance->getCurrentGestureId();
		if (!isDisabled() && !isDestroyed())
		{
			switch (animState)
			{
				case -1:		//NOT UPDATED YET. SWITCH TO STAND
					if (!appearance->getInTransition())
					{
						appearance->setGesture(0);
					}
					break;
					
				case 2:			//We are running.  If we have stopped or are walking, change.
					if (velocityMag == 0.0f)
					{
						//we've stopped.
						//INSTANTLY change to stand!
						appearance->setGesture(0);
					}
					else if ((velocityMag != 0.0f) && (velocityMag <= (dynamics.max.groundVehicle.speed * 0.5f)))
					{
						//We are now walking.
						appearance->setGesture(1);
					}
					
					if (refitBuddyWID)
					{
						GameObjectPtr refitMech = ObjectManager->getByWatchID(refitBuddyWID);
						Stuff::Vector3D distance;
						distance.Subtract(refitMech->getPosition(),getPosition());
						float dist = distance.GetApproximateLength();
						if (dist <= 128.0f)
							appearance->setGesture(3);
					}
					break;
					
				case 0:			//Standing, if moving now, change.
					if (velocityMag > (dynamics.max.groundVehicle.speed * 0.5f))
					{
						//We are now running.
						appearance->setGesture(2);
					}
					else if ((velocityMag != 0.0f) && (velocityMag <= (dynamics.max.groundVehicle.speed * 0.5f)))
					{
						//We are now Walking.
						appearance->setGesture(1);
					}
					
					if (refitBuddyWID)
					{
						GameObjectPtr refitMech = ObjectManager->getByWatchID(refitBuddyWID);
						Stuff::Vector3D distance;
						distance.Subtract(refitMech->getPosition(),getPosition());
						float dist = distance.GetApproximateLength();
						if (dist <= 128.0f)
							appearance->setGesture(3);
					}
					break;
					
				case 1:			//We are walking.  If we have stopped moving or are running, change.
					if (velocityMag == 0.0f)
					{
						//we've stopped.
						//INSTANTLY change to stand!
						appearance->setGesture(0);
					}
					else if (velocityMag > (dynamics.max.groundVehicle.speed * 0.5f))
					{
						//We are now running.
						appearance->setGesture(2);
					}
					
					if (refitBuddyWID)
					{
						GameObjectPtr refitMech = ObjectManager->getByWatchID(refitBuddyWID);
						Stuff::Vector3D distance;
						distance.Subtract(refitMech->getPosition(),getPosition());
						float dist = distance.GetApproximateLength();
						if (dist <= 128.0f)
							appearance->setGesture(3);
					}
					break;
					
				case 3:			//Repairing.  Check if done repairing.  If so, switch to stand.
					if (!appearance->getInTransition())
					{
						appearance->setGesture(4);
					}
					break;
	
				case 4:
					if (!refitBuddyWID)
					{
						appearance->setGesture(5);	//Start closing up.
					}
					break;
	
				case 5:
					if (!appearance->getInTransition())
					{
						if (!refitBuddyWID)
						{
							appearance->setGesture(0);
						}
						else
						{
							appearance->setGesture(3);
						}
					}
			}
		}
		else
		{
			//He's dead.
			appearance->setGesture(0);
		}
	}

	return 0;
}

//---------------------------------------------------------------------------

long GroundVehicle::setTeamId (long _teamId, bool setup) 
{
	long result = Mover::setTeamId(_teamId,setup);

	//If we are a resource truck, add resources to our side.
	if ((turn > 10) && ((GroundVehicleTypePtr)getObjectType())->resourcePoints && (_teamId == Team::home->getId()))
	{
		LogisticsData::instance->addResourcePoints(((GroundVehicleTypePtr)getObjectType())->resourcePoints);
		soundSystem->playBettySample(BETTY_RESOURCES);
	}

	return result;	
}

//---------------------------------------------------------------------------
long GroundVehicle::update (void)
{
	if (withdrawing && (pilot->getStatus() == WARRIOR_STATUS_WITHDRAWN)) 
	{
		setTangible(false);
		return(1);
	}

	if (refitBuddyWID && !ObjectManager->getByWatchID(refitBuddyWID))
		refitBuddyWID = 0;
	if (recoverBuddyWID && !ObjectManager->getByWatchID(recoverBuddyWID))
		recoverBuddyWID = 0;

	if ( Team::home == getTeam() )
		((ObjectAppearance*)appearance)->pilotNameID = getPilot()->descID;
	else if ( MPlayer )
	{
		strcpy( ((ObjectAppearance*)appearance)->pilotName, (commanderId > -1) ? MPlayer->playerInfo[commanderId].name : " " );
	}
	else
		((ObjectAppearance*)appearance)->pilotNameID = getPilot()->descID;

	positionNormal = land->getTerrainNormal(position);

	updatePathLock(false);

	if (timeToClearSelection != 0.0 && scenarioTime > timeToClearSelection)
	{
		timeToClearSelection = 0.0;
		setSelected(false);
		setTargeted(false);
	}

	bool inView = false;
	
	if (isDestroyed() && (timeLeft < 0.0))
	{
		setTangible(false);
		appearance->setSensorLevel(0);

		//--------------------------------------------------------
		// Update the dynamics and position here during destruct
		if (!deadByCrushed)
		{
			float speed = dVel.GetLength();
			if (speed)
			{
				Stuff::Vector3D velDiff = dAcc;
				velDiff *= frameLength;
				dVel.Add(dVel,velDiff);
				speed = dVel.GetLength(); 
				if (speed < Stuff::SMALL)
				{
					dVel.x = dVel.y = dVel.z = 0.0;
				}
				
				Stuff::Vector3D posDiff = dVel;
				posDiff *= frameLength;
				position.Add(position,posDiff);
				float elev = land->getTerrainElevation(position); 
				if (position.z < elev)
				{
					position.z = elev;
					dRacc.Zero();
					dRVel.Zero();
					dTime -= frameLength;
					if (dTime < 0.0)
						dVel.x = dVel.y = dVel.z = 0.0;
				}
				
				Stuff::Vector3D rvDiff = dRacc;
				rvDiff *= frameLength;
				dRVel.Add(dRVel,rvDiff);
				
				Stuff::Vector3D rotDiff = dRVel;
				rotDiff *= frameLength;
				dRot.Add(dRot,rotDiff);
			}
		}
		else
		{
			float elev = land->getTerrainElevation(position); 
			if (position.z < elev)
			{
				position.z = elev;
			}
		}
	
		updateAnimations();		//Keep calling so they stop moving!
		
		if (appearance)
		{
			rotation = dRot.y;

			long relationship = 0;
			if (getTeamId() == Team::home->getId())
			{
				//Two Possibilities.  Either we have the same commander or we don't.
				// If we do, we are on the same "team", if not we are ALLIES!!!
				if (getCommanderId() != Commander::home->getId())
				{
					relationship = 1;
				}
			}
			else
			{
				relationship = 2;
			}

			appearance->setObjectParameters(position, rotation, drawFlags, teamId,relationship);
			appearance->setMoverParameters(turretRotation,dRot.x,dRot.z,!pathLocks);
			inView = appearance->recalcBounds();
			appearance->setObjStatus(getStatus());
	
			if (inView)
			{
				appearance->update();
				windowsVisible = turn;
			}
			
			if ( attackRange == FIRERANGE_CURRENT && !isDisabled() )
			{
				mcTextureManager->addTriangle(holdFireIconHandle,MC2_DRAWALPHA);
				mcTextureManager->addTriangle(holdFireIconHandle,MC2_DRAWALPHA);
			}
		}
			
 		return(true);
	}
	else if (isDestroyed() && timeLeft >= 0.0)
	{
		setTangible(false);
		appearance->setSensorLevel(0);
		timeLeft -= frameLength;
		if (timeLeft < 0.0)
		{
			//-------------------------------------------------------------
			// Vehicle is a stopped hulk on the terrain.  No More updates.
			// When we know how to change to destroyed appearance, put code here.
			// Nevermind that, keep the damned thing from disappearing!!
			updateAnimations();		//Keep calling so they stop moving!
			
			if (appearance)
			{
				rotation = dRot.y;
				long relationship = 0;
				if (getTeamId() == Team::home->getId())
				{
					//Two Possibilities.  Either we have the same commander or we don't.
					// If we do, we are on the same "team", if not we are ALLIES!!!
					if (getCommanderId() != Commander::home->getId())
					{
						relationship = 1;
					}
				}
				else
				{
					relationship = 2;
				}

				appearance->setObjectParameters(position, rotation, drawFlags, teamId,relationship);
				appearance->setMoverParameters(turretRotation,dRot.x,dRot.z,!pathLocks);
				inView = appearance->recalcBounds();
				appearance->setObjStatus(getStatus());
		
				if (inView)
				{
					appearance->update();
					windowsVisible = turn;
				}
				
				if ( attackRange == FIRERANGE_CURRENT && !isDisabled() )
				{
					mcTextureManager->addTriangle(holdFireIconHandle,MC2_DRAWALPHA);
					mcTextureManager->addTriangle(holdFireIconHandle,MC2_DRAWALPHA);
				}
			}
 
			//------------------------------------------------
			// Blow the Vehicle and leave the hulk next frame
			if (!appearance->playDestruction())
			{
				if (((GroundVehicleTypePtr)getObjectType())->explDmg > 0.0f)
					ObjectManager->createExplosion(SPLASH_VEHICLE_EXPLOSION_ID,NULL,position, ((GroundVehicleTypePtr)getObjectType())->explDmg,((GroundVehicleTypePtr)getObjectType())->explRad);
				else if (pathLocks)
				{
					ObjectManager->createExplosion(VEHICLE_EXPLOSION_ID,NULL,position, 0.0f,0.0f);
				}
				else
				{
					if (RollDice(25))
					{
						ObjectManager->createExplosion(INFANTRY_BLOOD_EFFECT,NULL,position, 0.0f,0.0f);
					}
					else
					{
						ObjectManager->createExplosion(INFANTRY_BOOM_EFFECT,NULL,position, 0.0f,0.0f);
					}
				}
					
				craterManager->addCrater(CRATER_4,position,RandomNumber(180));
			}

			exploding = true;

			//------------------------------------------------------------
			//Throw the vehicle up into the air and spin it for coolness.
			/*
			if ((dAcc.z == 0.0) && !deadByCrushed)
			{
				//Calc initial Velocity, rotation and set Acceleration to down in World.
				long xlatBase = 12.0f + 50.0 / tonnage / 2.0f;
				long upBase = 25.0f + 100.0 / tonnage;
				long rotBase = 25.0f + 100.0 / tonnage;
				dVel.x = RandomNumber(xlatBase * 2.0) - xlatBase;
				dVel.y = RandomNumber(xlatBase * 2.0) - xlatBase;
				dVel.z = RandomNumber(upBase) + upBase;
				
				dRVel.x = RandomNumber(rotBase * 2.0) - rotBase;
				dRVel.y = RandomNumber(rotBase * 2.0) - rotBase;
				dRVel.z = RandomNumber(rotBase * 2.0) - rotBase;
				
				dRot.Zero();
				
				dAcc.x = dVel.x * 0.1f;
				dAcc.y = dVel.y * 0.1f;
				dAcc.z = dVel.z * 0.5f;
				dAcc.Negate(dAcc);
				
				dRacc.x = dRVel.x * 0.1f;
				dRacc.y = dRVel.y * 0.1f;
				dRacc.z = dRVel.z * 0.1f;
				dRacc.Negate(dRacc);
				
				dTime = 4.0f;
			}
			*/
			
			//-----------------------------------------------------------------
			// Create a Marine with the Vehicle Brain to run away from vehicle
			if (!MPlayer && getAwake())
				createVehiclePilot();
			//-----------------------------------------------------------------
			
			return(1);
		}
	}
	else if (getAwake() && !isDisabled())
	{
		#ifdef MC_PROFILE
		QueryPerformanceCounter(startCk);
		#endif

		//-----------------------------------------------------
		// Not destroyed nor disabled yet, so update our LOS...
		GroundVehicleTypePtr vehicleType = (GroundVehicleTypePtr)ObjectManager->getObjectType(typeHandle);ObjectManager->getObjectType(typeHandle);
		if (getTeam())	   //Vehicle Pilots are just out there.  They do not help anyone with LOS!!!
			getTeam()->markSeen(position,vehicleType->LOSFactor);
		markDistanceMoved = 0.0;
	}
	else if (isDisabled() && timeLeft)
	{
		if (!startDisabled) {
#ifdef USE_SMOKE
			if (dmgSmoke)
			{
				dmgSmoke->setOwner(this);
				dmgSmoke->setOwnerPosition(position);
				dmgSmoke->setOwnerVelocity(velocity);
				dmgSmoke->update();
				timeLeft -= frameLength;
				if (timeLeft <= -30.0)
				{
					delete dmgSmoke;
					dmgSmoke = NULL;
				}
			}
#endif
		}
	}
	
	((ObjectAppearance*)appearance)->pilotNameID = IDS_NOPILOT;
	control.update(this);
	
	#ifdef MC_PROFILE
	QueryPerformanceCounter(endCk);
	srCtrlUpd += (endCk.LowPart - startCk.LowPart);
	#endif

	#ifdef MC_PROFILE
	QueryPerformanceCounter(startCk);
	#endif

	bool emergencyStop = false;
	if (!isDisabled())
		emergencyStop = crashAvoidanceSystem();

	#ifdef MC_PROFILE
	QueryPerformanceCounter(endCk);
	srDyneUpd += (endCk.LowPart - startCk.LowPart);
	#endif

	//--------------------------------------------------------------------
	// At this point, the other updates have completed, its time to
	// apply the velocity and rotations to the mech.
	// NOTE:
	// All positions in the game are in World Units.  All velocities
	// are in Meters/sec.  This is to provide easy data entry for designers
	// We can't store positions as meters because terrain tiles are fixed
	// pixel(world unit) sizes which would require a boatload of math
	// with its crazy floating point inconsistencies.  This will mess up
	// the terrain.  So, whenever you want a position derived from
	// a velocity, multiply velocity by worldUnitsPerMeter.

	#ifdef MC_PROFILE
	QueryPerformanceCounter(startCk);
	#endif

	float velMag = 0.0;
	if (!emergencyStop)
		velMag = velocityMag;


	if (withdrawing && !inView && (pilot->getStatus() != WARRIOR_STATUS_WITHDRAWN))
		getObjectType()->handleDestruction(this, NULL);

	//We are a vehicle pilot.  Whack us when we are no longer in view.
	if (isVehiclePilot && !inView)
		setExists(false);

	//We are a Vehicle Pilot. Check If I have nothing to do.  If not, move 'em out!
	if (isVehiclePilot && (pilot->getCurTacOrder()->code == TACTICAL_ORDER_NONE) )
	{
		Stuff::Vector3D location;
		location.Zero();

		pilot->orderMoveToPoint (false, true, ORDER_ORIGIN_PLAYER, location, -1, TACORDER_PARAM_RUN);
	}

	#ifdef MC_PROFILE
	QueryPerformanceCounter(endCk);
	srApprUpd += (endCk.LowPart - startCk.LowPart);
	QueryPerformanceCounter(startCk);
	#endif

	if (teleportPosition.x > -999990.0) {
		setPosition(teleportPosition);
		teleportPosition.x = -999990.0;
		teleportPosition.y = -999990.0;
		teleportPosition.z = -999990.0;
	}

	//-----------------------------------------------------
	// Terrain Angle should effect movement speed.
	// This is accomplished by finding the angle between
	// the normal to the tile and the vehicle's J vector.
	// This angle will be less than 90 if the vehicle is going uphill
	// and greater than 90 if the vehicle is going downhill.
	Stuff::Vector3D vel = getRotationVector();
	if (velMag > Stuff::SMALL)
	{
		Stuff::Vector3D worldK;

		worldK = land->getTerrainNormal(position);

		Rotate(worldK,rotation);

		//------------------------------------------------
		// Find Pitch.
		Stuff::Vector3D pitchK;
		pitchK = worldK;
		pitchK.x = 0.0f;
		pitchK.Normalize(pitchK);

		Stuff::Vector3D up;
		up.x = up.y = 0.0f;
		up.z = 1.0f;

		float pitchAngle = up * pitchK;
		pitchAngle = acos(pitchAngle) * RADS_TO_DEGREES;
		if (pitchK.y < 0.0f)
			pitchAngle = -pitchAngle;

		bool useHillFactor = (moveLevel == 0) || (!GameMap->getShallowWater(cellPositionRow, cellPositionCol) && !GameMap->getDeepWater(cellPositionRow, cellPositionCol));
		if (useHillFactor) {
			float pitch = sin(pitchAngle * DEGREES_TO_RADS);
			if ( (pitch != 0.0f))
			{
				float hillFactor = pitch * velMag * gvHillSpeedFactor;
				velMag -= hillFactor;
				if (velMag < hillFactor)
					velMag = 2.0;
			}
		}
	}

	float velMult = velMag * frameLength;
	if (velMult > DistanceToWaypoint)
		velMult = DistanceToWaypoint;
	vel *= velMult * worldUnitsPerMeter;

	{
		Stuff::Vector3D newPosition;
		newPosition.Add(position, vel);
		newPosition.z = land->getTerrainElevation(newPosition);
		if ((moveLevel == 1) && (newPosition.z < Terrain::waterElevation))
			newPosition.z = Terrain::waterElevation;
		int newCellRow = 0;
		int newCellCol = 0;
		int tileRow = 0;
		int tileCol = 0;
		land->worldToTileCell(newPosition, tileRow, tileCol, newCellRow, newCellCol);
	}

	setVelocity(vel);

	markDistanceMoved += vel.GetLength();

	Stuff::Vector3D newPosition;
	newPosition.Add(position, vel);
	newPosition.z = land->getTerrainElevation(newPosition);
	if ((moveLevel == 1) && (newPosition.z < Terrain::waterElevation))
		newPosition.z = Terrain::waterElevation;

	if (newMoveChunk) {
		if (!statusChunk.jumpOrder) {
			//------------------------------------------------------------
			// If we're too far from where we should be, perhaps we should
			// just warp to it...
			Stuff::Vector3D correctPos;
			land->cellToWorld(moveChunk.stepPos[0][0], moveChunk.stepPos[0][1], correctPos);
			correctPos.z = 0.0;
			Stuff::Vector3D curPos = position;
			curPos.z = 0.0;
			curPos.Subtract(curPos, correctPos);
			float posDelta = curPos.GetLength();
			if (posDelta > MPlayer->warpFactor)
				newPosition = correctPos;
				//vel.Subtract(correctPos, curPos);
			if (!GameMap->inBounds(moveChunk.stepPos[0][0], moveChunk.stepPos[0][1]))
				Fatal(0, " gvehicl.update: newMoveChunk stepPos not on map! ");
		}
		newMoveChunk = false;
	}
	
	setPosition(newPosition);
	updateTurret(NewRotation);

	updateDynamics();

	if (!isDisabled())
		updatePathLock(true);

	sweepTime += frameLength;
	mineCheck();
	if (!MPlayer || MPlayer->isServer()) 
	{
		if (mineLayer != 0)
		{
			if (pilot->getCurTacOrder()->moveParams.mode == MOVE_MODE_MINELAYING)
			{
				if ((cellPositionRow != cellRowToMine) || (cellPositionCol != cellColToMine))
				{
					timeInCurrent += frameLength;

					if (timeInCurrent > MineWaitTime)
					{
						cellRowToMine = cellPositionRow;
						cellColToMine = cellPositionCol;
						timeInCurrent = 0.0f;

						//-----------------------------------------------
						// Used to check for mines.  Now just lays them.
						// Always 1 and it always replaces current mine data.
						// Everyone can see mines now!
						GameMap->setMine(cellRowToMine, cellColToMine, 1);
						
						if (MPlayer) {
							MPlayer->addMineChunk(cellRowToMine,
												  cellColToMine,
												  1,
												  1,
												  0);
						}
						WeaponShotInfo shot;
						shot.init(0, -2, 1.0, GROUNDVEHICLE_LOCATION_TURRET, 0);
						handleWeaponHit(&shot, MPlayer != NULL);
					}
				}
			}
		}
		
		if (aerospaceSpotter)
		{
			//Drop airstrikes here.
			GameObjectPtr curTarget = pilot->getCurrentTarget();
			float dist = 0.0f;
			if (curTarget) 
			{
				Stuff::Vector3D distance;
				distance.Subtract(curTarget->getPosition(),getPosition());
				dist = distance.GetApproximateLength();
				dist *= metersPerWorldUnit;
			}

			if (curTarget && (dist <= WeaponRanges[WEAPON_RANGE_MEDIUM][1]) && lineOfSight(curTarget))
			{
				timeInCurrent += frameLength;
				if (timeInCurrent > StrikeWaitTime)
				{
					timeInCurrent = 0.0f;
					
					CallArtillery (getCommander()->getId(), ARTILLERY_LARGE, curTarget->getPosition(), StrikeTimeToImpact, false);
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	// Now that we have our new position, glue mech to terrain by
	// changing z to match terrainElevation at that point, IF VEHICLE IS VISIBLE!!!!
	float zPos = land->getTerrainElevation(position);
	position.z = zPos;
	if ((moveLevel == 1) && (zPos < MapData::waterDepth))
		position.z = MapData::waterDepth;
 
	if (!isDestroyed() || (timeLeft > 0.0))
	{
		if (appearance)
		{
			updateAnimations();

			long relationship = 0;
			if (getTeamId() == Team::home->getId())
			{
				//Two Possibilities.  Either we have the same commander or we don't.
				// If we do, we are on the same "team", if not we are ALLIES!!!
				if (getCommanderId() != Commander::home->getId())
				{
					relationship = 1;
				}
			}
			else
			{
				relationship = 2;
			}

			appearance->setObjectParameters(position, rotation, drawFlags, teamId,relationship);
			appearance->setMoverParameters(turretRotation,0.0f,0.0f,!pathLocks);
			inView = appearance->recalcBounds();
			appearance->setObjStatus(getStatus());
	
			//Start and stop the water wakes here.
			int watercellR, watercellC;
			land->worldToCell(position,watercellR, watercellC);
			if (GameMap->getDeepWater(watercellR, watercellC) || 
				GameMap->getShallowWater(watercellR, watercellC))
			{
				appearance->startWaterWake();
			}
			else
			{
				appearance->stopWaterWake();
			}
			
			//Check if we need to start or stop activity in appearance
			if (pilot->getCurTacOrder()->moveParams.mode == MOVE_MODE_MINELAYING)
			{
				appearance->startActivity(MINELAYER_EFFECT_ID,true);
			}
			else if (refitBuddyWID && (appearance->getCurrentGestureId() == 4))
			{
				appearance->startActivity(REPAIR_VEHICLE_EFFECT_ID,true);
				soundSystem->playDigitalSample(REPAIRBAY_FX,getPosition(),false);
			}
			else
			{
				appearance->stopActivity();
			}
			
 			//if (inView)		//Must do this regardless of inView.
			{
				//-----------------------------------------------------	
				//Must save sensor data HERE now so fades happen OK.
				if (getTeamId() != Team::home->getId())
				{
					//---------------------------------------------------------------
					conStat = getContactStatus(Team::home->getId(), true);
			
					if ((conStat == CONTACT_VISUAL) || isDestroyed() || isDisabled() || ShowMovers || (MPlayer && MPlayer->allUnitsDestroyed[MPlayer->commanderID]))
					{
						if (alphaValue != 0xff)
						{
							fadeTime += frameLength;
							if (fadeTime > ContactFadeTime)
							{
								fadeTime = ContactFadeTime;
								alphaValue = 0xff;
							}
							else
							{
								float fadeValue = fadeTime / ContactFadeTime * 255.0f;
								alphaValue = fadeValue;
							}
						}
						
						appearance->setAlphaValue(alphaValue);
						appearance->setSensorLevel(0);
					}
					else	//Pretty much anything else is fading out.
					{
						if (alphaValue != 0x0)
						{
							//We are fading.  Move it down.
							fadeTime -= frameLength;
							if (fadeTime < 0.0f)
							{
								fadeTime = 0.0f;
								alphaValue = 0x0;
							}
							else
							{
								float fadeValue = fadeTime / ContactFadeTime * 255.0f;
								alphaValue = fadeValue;
							}
						}
						
						appearance->setAlphaValue(alphaValue);
						
						if (conStat == CONTACT_SENSOR_QUALITY_1)
						{
							appearance->setSensorLevel(1);
						}
						else if (conStat == CONTACT_SENSOR_QUALITY_2)
						{
							appearance->setSensorLevel(2);
						}
						else if (conStat == CONTACT_SENSOR_QUALITY_3)
						{
							appearance->setSensorLevel(3);
						}
						else if (conStat == CONTACT_SENSOR_QUALITY_4)
						{
							appearance->setSensorLevel(4);
						}
					}
				}
				else
				{
					if (isOnGui)
					{
						alphaValue = 0xff;
						appearance->setAlphaValue(alphaValue);
					}
				}
				
				appearance->setVisibility(true,true);
				appearance->update();
				windowsVisible = turn;
				
				if ( attackRange == FIRERANGE_CURRENT && !isDisabled() )
				{
					mcTextureManager->addTriangle(holdFireIconHandle,MC2_DRAWALPHA);
					mcTextureManager->addTriangle(holdFireIconHandle,MC2_DRAWALPHA);
				}
			}
		}

		setTangible(true);
	}

	//-------------------------------------------------------------------------------
	// Let the moverBlockList know which block the Mech is in for Collision Purposes
	float xCoord = position.x - Terrain::mapTopLeft3d.x;
	float yCoord = Terrain::mapTopLeft3d.y + position.y ;

	float divisor = (Terrain::verticesBlockSide * Terrain::worldUnitsPerVertex);
	xCoord /= divisor;
	yCoord /= divisor;

	long blockNumber = float2long(xCoord) + (float2long(yCoord) * Terrain::blocksMapSide);
	addMoverToList(blockNumber);

		#ifdef MC_PROFILE
		QueryPerformanceCounter(endCk);

		srObjtUpd += (endCk.LowPart - startCk.LowPart);
		#endif

	if (getDebugFlag(OBJECT_DFLAG_DISABLE))
		disable(DEBUGGER_DEATH);
		
	return(1);		//Vehicle is still around,  false means whack this vehicle from list.
}

//---------------------------------------------------------------------------------
void GroundVehicle::renderShadows (void)
{
	if (Terrain::IsGameSelectTerrainPosition(position))
	{
		if ((getStatus() != OBJECT_STATUS_DESTROYED) && (getStatus() != OBJECT_STATUS_DISABLED))
		{
			if (getTeamId() != Team::home->getId())
			{
				//---------------------------------------------------------------
				// DO Sensor contact stuff here.  GLENN, Update with new whatevers when done and let me know!
				// -fs
				long cStat = conStat;
		
				if (cStat == CONTACT_VISUAL)
				{
					appearance->renderShadows();
				}
			}
			else		//We are Home Team and we always see ourselves.
			{
				appearance->renderShadows();
			}
		}
	}
}	

//----------------------------------------------------------------------------------

void GroundVehicle::render (void) 
{
	if (Terrain::IsGameSelectTerrainPosition(position))
	{
		if (teamId != Team::home->getId())
		{
			long cStat = conStat;
	
			if ((cStat == CONTACT_VISUAL) || isDestroyed() || isDisabled() || ShowMovers || (MPlayer && MPlayer->allUnitsDestroyed[MPlayer->commanderID]))
			{
				float barStatus = getTotalEffectiveness();
				
				DWORD color = 0xff7f7f7f;
				if ((teamId > -1) && (teamId < 8)) {
					if (getTeam()->isFriendly(Team::home))
						color = SB_GREEN;
					else if (getTeam()->isEnemy(Team::home))
						color = SB_RED;
					else
						color = SB_BLUE;
				}
					
				appearance->setBarColor(color);
				appearance->setBarStatus(barStatus);
				appearance->setObjectNameId(descID);
				appearance->render();
			}
			else if (cStat == CONTACT_SENSOR_QUALITY_1)
			{
				appearance->setBarColor(SB_RED);
				appearance->render();
				
				//No more text help at this quality level
			}
			else if (cStat == CONTACT_SENSOR_QUALITY_2)
			{
				appearance->setBarColor(SB_RED);
				appearance->render();
				
				//No more text help at this quality level
			}
			else if (cStat == CONTACT_SENSOR_QUALITY_3)
			{
				appearance->setBarColor(SB_RED);
				appearance->render();
	
				long resourceID = 0;
				if (tonnage <= 40)
					resourceID = IDS_SENSOR_LIGHT_VEHICLE;
				else if (tonnage <= 65)
					resourceID = IDS_SENSOR_MEDIUM_VEHICLE;
				else
					resourceID = IDS_SENSOR_HEAVY_VEHICLE;
					
				if ( appearance->canBeSeen() )
					drawSensorTextHelp (appearance->getScreenPos().x, appearance->getScreenPos().y+20.0f, resourceID,SD_RED,false);
			}
			else if (cStat == CONTACT_SENSOR_QUALITY_4)
			{
				appearance->setBarColor(SB_RED);
				appearance->render();
	
				if ( appearance->canBeSeen() )
					drawSensorTextHelp (appearance->getScreenPos().x, appearance->getScreenPos().y+20.0f,descID,SD_RED,false);
			}
			else if (alphaValue != 0x0)	//What if we are out of LOS and NOT on sensors!!!  Let 'em fade out.
			{
				appearance->setBarColor(SB_RED);
				appearance->render();
			}
		}
		else 
		{
			float barStatus = getTotalEffectiveness();
			
			DWORD color = 0xff7f7f7f;
			if (getTeamId() == Team::home->getId())
			{
				//Two Possibilities.  Either we have the same commander or we don't.
				// If we do, we are on the same "team", if not we are ALLIES!!!
				if (getCommanderId() != Commander::home->getId())
				{
					color = 0x000000ff; 
				}
			}
			else
			{
				color = 0x00ff0000; 
			}
				
			appearance->setBarColor(color);
			appearance->setBarStatus(barStatus);
			appearance->setObjectNameId(descID);
			appearance->render();
			if ( attackRange == FIRERANGE_CURRENT && !isDisabled() )
				appearance->drawIcon( holdFireIconHandle, 5, 5, color );
		}
	}

	if (drawTerrainGrid)
	{
		MovePathPtr path = pilot->getMovePath();
		gosASSERT(path != NULL);
		if (path->numSteps)
		{
			Stuff::Vector4D lineStart, lineEnd;
			for (long i=0;i<path->numSteps;i++)
			{
				if (i != (path->numSteps-1))
				{
					Stuff::Vector3D startPos = path->stepList[i].destination;
					Stuff::Vector3D endPos = path->stepList[i+1].destination;
					startPos.z = land->getTerrainElevation(startPos);
					endPos.z = land->getTerrainElevation(endPos);
					
					eye->projectZ(startPos,lineStart);
					eye->projectZ(endPos,lineEnd);
					
					lineStart.z = lineEnd.z = HUD_DEPTH;
					
					LineElement newElement(lineStart,lineEnd,SD_GREEN,NULL,-1);
					newElement.draw();
				}
			}
		}
	}
	
	
}

//----------------------------------------------------------------------------------

float GroundVehicle::relFacingTo (Stuff::Vector3D goal, long bodyLocation) {

	float relFacing = Mover::relFacingTo(goal);

	switch (bodyLocation) {
		case GROUNDVEHICLE_LOCATION_TURRET:
			relFacing += turretRotation;
			break;
	}
	if (relFacing < -180.0)
		relFacing += 360.0;
	else if (relFacing > 180)
		relFacing -= 360.0;
	return(relFacing);
}

//---------------------------------------------------------------------------
// COMBAT routines
//---------------------------------------------------------------------------

float GroundVehicle::calcAttackChance (GameObjectPtr target, long aimLocation, float targetTime, long weaponIndex, float modifiers, long* range, Stuff::Vector3D* targetPoint) {

	if ((weaponIndex < numOther) || (weaponIndex >= numOther + numWeapons))
		return(-1000.0);

	float attackChance = Mover::calcAttackChance(target, aimLocation, targetTime, weaponIndex, modifiers, range, targetPoint);

	return(attackChance);
}

//---------------------------------------------------------------------------

long GroundVehicle::calcHitLocation (GameObjectPtr attacker, long weaponIndex, long attackSource, long attackType) {

	if (attackSource == MECH_HIT_SECTION_BOTTOM) // if this came from a mine...
		return GROUNDVEHICLE_LOCATION_FRONT;
	
	//---------------------------------------------------------------------
	// Need to use the Line-of-fire params to calc section and arc of hit.
	// For now, just use the arc of entry to the target, and select section
	// without using obstructions...

	//---------------------------
	// First, find the section...
	long zAxisPos = (RandomNumber(100) + RandomNumber(100)) / 2;

	//-------------------------------------------------------------
	// For now, use the zAxisOffset to determine whether we hit the
	// top, middle or bottom. Ignore the xAxisPos...
	long hitLocation;
	if (zAxisPos >= 70)
		hitLocation = GROUNDVEHICLE_LOCATION_TURRET;
	else {
		float entryAngle = 0.0;
		if (attacker)
			entryAngle = relFacingTo(attacker->getPosition());
		if ((entryAngle >= -45.0) && (entryAngle <= 45.0))
			hitLocation = GROUNDVEHICLE_LOCATION_FRONT;
		else if ((entryAngle  > -135.0) && (entryAngle < -45.0))
			hitLocation = GROUNDVEHICLE_LOCATION_LEFT;
		else if ((entryAngle > 45.0) && (entryAngle < 135))
			hitLocation = GROUNDVEHICLE_LOCATION_RIGHT;
		else
			hitLocation = GROUNDVEHICLE_LOCATION_REAR;
	}

	return(hitLocation);
}

//---------------------------------------------------------------------------

bool GroundVehicle::hitInventoryItem (long itemIndex, bool setupOnly) {

	//----------------------------------------------------------------------
	// This should never be called for a ground vehicle, as a CRITICAL SPACE
	// hit is handled differently from mechs...
	Fatal(0, " Vehicles should never suffer inventory item hit ");
	return(false);
}

//---------------------------------------------------------------------------

void GroundVehicle::destroyBodyLocation (long location) {
	armor[location].curArmor = 0;
}

//------------------------------------------------------------------------------------------

bool GroundVehicle::injureBodyLocation (long bodyLocation, float damage) {

	BodyLocationPtr location = &body[bodyLocation];
	if (damage >= location->curInternalStructure) {
		location->curInternalStructure = 0;
		location->damageState = IS_DAMAGE_DESTROYED;
		return(true);
	}

	location->curInternalStructure -= damage;
	float damageLevel = (float)body[bodyLocation].curInternalStructure / body[bodyLocation].maxInternalStructure;
	if (damageLevel == 0.0)
		body[bodyLocation].damageState = IS_DAMAGE_DESTROYED;
	else if (damageLevel <= 0.5)
		body[bodyLocation].damageState = IS_DAMAGE_PARTIAL;
	else
		body[bodyLocation].damageState = IS_DAMAGE_NONE;
	return(false);
}

//---------------------------------------------------------------------------

long GroundVehicle::buildStatusChunk (void) {

	//-----------------------------------------------------------------------
	// For now, we'll just track a failed piloting check for 3 status updates.
	// If all three updates are never received, then the clients won't know
	// to fall! This should be adequate. If it proves otherwise, we can
	// change this...
	statusChunk.init();

	statusChunk.bodyState = 0;	//BODY STATES NOT USED IN GV MLR ANYMORE!!!!!!
								//THESE BITS CAN BE RECOVERED!!!

	if (pilot) {
		GameObjectPtr curTarget = pilot->getLastTarget();
		if (curTarget) {
			if (curTarget->isMover()) {
				statusChunk.targetType = STATUSCHUNK_TARGET_MOVER;
				statusChunk.targetId = ((MoverPtr)curTarget)->getNetRosterIndex();
				}
			else if (curTarget->isTerrainObject()) {
				statusChunk.targetType = STATUSCHUNK_TARGET_TERRAIN;
				statusChunk.targetId = curTarget->getPartId();
				}
			else
				Fatal(curTarget->getObjectClass(), " GroundVehicle.buildStatusChunk: bad target object class ");
		}
	}

	statusChunk.ejectOrderGiven = ejectOrderGiven;

	statusChunk.pack(this);

#ifdef DEBUG_CHUNKS
	StatusChunk chunk2;
	chunk2.data = statusChunk.data;
	chunk2.unpack(this);
	if (!statusChunk.equalTo(&chunk2))
		Fatal(0, " BAD Statuschunk: save stchunk.dbg file! ");
#endif

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long GroundVehicle::handleStatusChunk (long updateAge, unsigned long chunk) {

	statusChunk.init();
	statusChunk.data = chunk;
	statusChunk.unpack(this);
	//------------------------------------------------------------
	// HACK!!!!!!!!!!!!!! If bad packet, no statuschunk for you...
	if (StatusChunkUnpackErr != 0)
		return(NO_ERR); //Assert(0, " BAD PACKET! ");

	long lastTargetId = 0;
	if (!statusChunk.jumpOrder) {
		if (statusChunk.targetType > STATUSCHUNK_TARGET_NONE) {
			switch (statusChunk.targetType) {
				case STATUSCHUNK_TARGET_MOVER:
					if (MPlayer->moverRoster[statusChunk.targetId])
						lastTargetId = MPlayer->moverRoster[statusChunk.targetId]->getPartId();
					break;
				case STATUSCHUNK_TARGET_TERRAIN:
				case STATUSCHUNK_TARGET_SPECIAL:
					lastTargetId = statusChunk.targetId;
					break;
			}
		}
	}

	if (pilot) {
		if (lastTargetId == 0)
			pilot->setLastTarget(NULL);
		else {
			GameObjectPtr curTarget = pilot->getLastTarget();
			if (!curTarget || (curTarget->getPartId() != lastTargetId)) {
				curTarget = ObjectManager->findByPartId(lastTargetId);
				pilot->setLastTarget(curTarget);
			}
		}

		if (!ejectOrderGiven && statusChunk.ejectOrderGiven) {
			//--------------------------
			// Just given eject order...
			ejectOrderGiven = true;
			handleEjection();
		}
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long GroundVehicle::buildMoveChunk (void) {

	moveChunk.init();
	if (pilot) {
		//--------------------------------------------
		// First, make sure we have the latest path...
		pilot->getMovePath();
		//--------------------------------------------------
		// Now, use it (and any queued one, if available)...
		moveChunk.build(this, pilot->getMovePath(0), pilot->getMovePath(1));
	}
	moveChunk.pack(this);

#ifdef DEBUG_CHUNKS
	MoveChunk chunk2;
	chunk2.data = moveChunk.data;
	chunk2.unpack(this);
	//----------------------------------------------------------
	// HACK!!!!!!!!!!!!!! If bad packet, no movechunk for you...
	if (MoveChunk::err != 0) {
		moveChunk.init();
		moveChunk.build(this, NULL, NULL);
		moveChunk.pack(this);
		return(NO_ERR); //Assert(0, " BAD PACKET! ");
	}
	if (!moveChunk.equalTo(this, &chunk2))
		Fatal(0, " Bad gvehicl movechunk: save mvchunk.dbg file! ");
#endif

	return(NO_ERR);
}

//---------------------------------------------------------------------------
#pragma optimize("",off)

long GroundVehicle::handleMoveChunk (unsigned long chunk) {

	moveChunk.init();
	moveChunk.data = chunk;
	moveChunk.unpack(this);

	//----------------------------------------------------------
	// HACK!!!!!!!!!!!!!! If bad packet, no movechunk for you...
	if (MoveChunk::err != 0)
		return(NO_ERR); //Assert(0, " BAD PACKET! ");
	
	MovePathPtr path = getPilot()->getMovePath();
	setMoveChunk(path, &moveChunk);

	//-------------------------------------------------------------
	// Now, let's determine which opening step we're closest to so
	// we don't loop back to the first step everytime we get a new
	// chunk of the path...
	if (path->numStepsWhenNotPaused > 1) {
		long i = 0;
		for (i = path->numStepsWhenNotPaused - 1; i > 0; i--) {
			float distanceFromPt = distanceFrom(path->stepList[i].destination);
			if (distanceFromPt <= MapCellDiagonal)
				break;
		}
		path->setCurStep(i);
	}

	newMoveChunk = true;

	return(NO_ERR);
}

#pragma optimize("",on)

//---------------------------------------------------------------------------

float GroundVehicle::weaponLocked (long weaponIndex, Stuff::Vector3D targetPosition) {

	return(relFacingTo(targetPosition, GROUNDVEHICLE_LOCATION_TURRET));
}

//---------------------------------------------------------------------------

long GroundVehicle::handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk) {

	if (!shotInfo)
		return(NO_ERR);

	printHandleWeaponHitDebugInfo(shotInfo);

	if (getTeam() && Team::noPain[getTeamId()])
		return(NO_ERR);

	if (!MPlayer && CantTouchThis && pilot->onHomeTeam())
		return(NO_ERR);

	if (addMultiplayChunk)
		MPlayer->addWeaponHitChunk(this, shotInfo);

	//----------------------------------
	// Is this possible? Just in case...
	if (shotInfo->damage <= 0)
		return(NO_ERR);

	if (isDestroyed())
		return(NO_ERR);

	numWeaponHitsHandled++;
	//if (addMultiplayChunk)
	//	MPlayer->addWeaponHitChunk(this, shotInfo);

	//------------------------------------------------------------
	// Since Multiplayer still needs this, preserve it and restore
	// at the end of this function!
	WeaponShotInfo startShotInfo = *shotInfo;
	GameObjectPtr attacker = ObjectManager->getByWatchID(shotInfo->attackerWID);

	if ((shotInfo->hitLocation < 0) || (shotInfo->hitLocation >= NUM_GROUNDVEHICLE_LOCATIONS)) {
		char attackerStr[30];
		if (attacker) {
			if (attacker->isMover())
				strcpy(attackerStr, attacker->getName());
			else
				sprintf(attackerStr, "ID:%d", attacker->getPartId());
			}
		else
			strcpy(attackerStr, "attacker?");
		char err[128];
		sprintf(err, "GVehicle.handleWeaponHit: [%s]%d for %.2f at %d",
			attackerStr,
			shotInfo->masterId,
			shotInfo->damage,
			shotInfo->hitLocation);
		Fatal(0, err);
	}

	//----------------------------
	// Does it get past the armor?
	if (armor[shotInfo->hitLocation].curArmor > 0) {
		if (shotInfo->damage > armor[shotInfo->hitLocation].curArmor) {
			shotInfo->setDamage(shotInfo->damage - armor[shotInfo->hitLocation].curArmor);
			armor[shotInfo->hitLocation].curArmor = 0;
			}
		else {
			armor[shotInfo->hitLocation].curArmor -= shotInfo->damage;
			shotInfo->setDamage(0);
		}
		//-----------------------------------------------------------
		// Make sure, in case it's a minesweeper, it ain't no more :)
		if (shotInfo->hitLocation == GROUNDVEHICLE_LOCATION_FRONT)
			mineSweeper = false;
	}

	//-----------------------------------------------------
	// If any damage gets thru, check Internal Structure...
	bool vehicleAlreadyDisabled = isDisabled();
	bool vehicleDestroyed = false;
	if (shotInfo->damage > 0)
		vehicleDestroyed = injureBodyLocation(shotInfo->hitLocation, shotInfo->damage);

	if (vehicleDestroyed) 
	{
		pilot->handleOwnVehicleIncapacitation(0);
		getObjectType()->handleDestruction(this, NULL);
		setFlag(OBJECT_FLAG_TANGIBLE,false);
	}

	//--------------------------------------------
	// Calc current CV, after this damage taken...
	curCV = calcCV();
	if (isDisabled())
		setThreatRating(1);
	else
		setThreatRating(-1);

	if (!vehicleAlreadyDisabled && isDisabled()) {
		//--------------------------------------------------------
		// If this vehicle was destroyed, let the attacker know...
		if (attacker && attacker->isMover()) {
			attacker->getPilot()->triggerAlarm(PILOT_ALARM_KILLED_TARGET, getWatchID());
			if (!killed && MPlayer && MPlayer->isServer()) 
			{
				long attackerCID = attacker->getCommanderId();
				if (attackerCID == -1)
					attackerCID = ((MoverPtr)attacker)->prevCommanderId;
				if ((attackerCID == -1) || (attackerCID == getCommanderId()))
					attackerCID = MAX_MC_PLAYERS;
				MPlayer->addKillLossChunk(attackerCID, !lost ? getCommanderId() : MAX_MC_PLAYERS);
				lost = true;
				killed = true;
			}
		}
		else if (attacker && (attacker->getObjectClass() == ARTILLERY))
		{
			if (!killed && MPlayer && MPlayer->isServer()) 
			{
				long attackerCID = attacker->getCommanderId();
				if (attackerCID == -1)
					attackerCID = ((MoverPtr)attacker)->prevCommanderId;
				if ((attackerCID == -1) || (attackerCID == getCommanderId()))
					attackerCID = MAX_MC_PLAYERS;
				MPlayer->addKillLossChunk(attackerCID, !lost ? getCommanderId() : MAX_MC_PLAYERS);
				lost = true;
				killed = true;
			}
		}
		}
	else {
		//--------------------------------
		// Trigger the WEAPON HIT event...
		if (attacker) {
			if (shotInfo->masterId > -1)
				pilot->triggerAlarm(PILOT_ALARM_HIT_BY_WEAPONFIRE, attacker->getWatchID());
			else if (shotInfo->masterId == -4)
				pilot->triggerAlarm(PILOT_ALARM_HIT_BY_WEAPONFIRE, attacker->getWatchID());
			else
				pilot->triggerAlarm(PILOT_ALARM_COLLISION, attacker->getWatchID());
			}
		else 
		{
			//If it was -5, it was a repairvehicle burning refit points.  Do not say you are under fire.
			if (shotInfo->masterId != -5)
			{
				if (shotInfo->masterId == -4)
					pilot->triggerAlarm(PILOT_ALARM_HIT_BY_WEAPONFIRE, 0);
				else if (shotInfo->masterId < 0) {
					//------------------------------------------------------------
					// If no attacker and a negative weapon masterId, then it must
					// be a mine or artillery...
					// EXCEPT if it was -5.  Then it was a repair vehicle chunk in multiplayer!!
					pilot->triggerAlarm(PILOT_ALARM_HIT_BY_WEAPONFIRE, shotInfo->masterId);
					}
				else
					pilot->triggerAlarm(PILOT_ALARM_HIT_BY_WEAPONFIRE, 0);
			}
		}
	}

	if ((shotInfo->masterId == -2) && (shotInfo->hitLocation == GROUNDVEHICLE_LOCATION_TURRET)) {
		mineLayer--;
		//	NOT ANY MORE.  Minelayer still active.
		// Change per design meeting.  4/23/01.
		if ((mineLayer == 0) && (getCommanderId() == Commander::home->getId())) 
		{
		//	disable( POWER_USED_UP );
			soundSystem->playBettySample( BETTY_MINES_GONE );
		}
	}

	if (shotInfo->masterId == -5) {
		if (!getRefitPoints())
			disable(POWER_USED_UP);
	}

	*shotInfo = startShotInfo;

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long GroundVehicle::fireWeapon (GameObjectPtr target, float targetTime, long weaponIndex, long attackType, long aimLocation, Stuff::Vector3D* targetPoint, float &dmgDone) {

	if (status != OBJECT_STATUS_NORMAL)
		return(1);

	if (!isWeaponIndex(weaponIndex))
		return(2);

	if (!isWeaponReady(weaponIndex))
		return(3);

	if (!Terrain::IsGameSelectTerrainPosition(getPosition()))
		return (1);
		
 	if (aerospaceSpotter)
		return 4;
		
	float distanceToTarget = 0.0;
	if (target) 
	{
		if (target->isDestroyed())
			return(4);
			
		//We did this in calcWeaponsStatus.  Do we really need to do it again??!
		// I have decided NO as of now.
		/*
		if (getWeaponIndirectFire(weaponIndex))
		{
			if (!getTeam()->teamLineOfSight(target->getPosition()))
				return(4);
		}
		else
		{
			if (!lineOfSight(target))
				return(4);
		}
		*/
 			
		distanceToTarget = distanceFrom(target->getPosition());
	}
	else if (targetPoint) 
	{
		//We did this in calcWeaponsStatus.  Do we really need to do it again??!
		// I have decided NO
		/*
		if (getWeaponIndirectFire(weaponIndex))
		{
			if (!getTeam()->teamLineOfSight(*targetPoint))
				return(4);
		}
		else
		{
			if (!lineOfSight(*targetPoint))
				return(4);
		}
		*/
 			
		distanceToTarget = distanceFrom(*targetPoint);
	}
	else
		return(4);
		
	bool inRange = weaponInRange(weaponIndex, distanceToTarget, MapCellDiagonal);

	//-----------------------------------------
	// Let's make sure the target's in range...
	if (MPlayer) 
	{
		if (MPlayer->isServer() && !inRange)
			return(4);
	}
	else if (!inRange)
		return(4);

	long numShots = getWeaponShots(weaponIndex);
	if (numShots == 0)
		return(4);

	//------------------------------------------
	// As of 4/9/97, missiles cannot be aimed...
	// As of 4/30/98, they can be again... -fs
	//if ((aimLocation != -1) && (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_MISSILE))
	//	return(4);

	MechWarriorPtr targetPilot = NULL;
	if (target && target->isMover()) {
		targetPilot = ((MoverPtr)target)->getPilot();
		targetPilot->updateAttackerStatus(getWatchID(), scenarioTime);
	}

	float entryAngle = 0.0;
	if (target)
		entryAngle = target->relFacingTo(position);

	bool isStreakMissile = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponStreak(); 

	long range;
	long attackChance = (long)calcAttackChance(target, aimLocation, targetTime, weaponIndex, 0.0, &range, targetPoint);
	long hitRoll = RandomNumber(100);
	if (target && (target->getTeamId() == TEAM2))
		pilot->incNumSkillUses(COMBAT_STAT_MISSION, MWS_GUNNERY);

	//---------------------------------------------------------------
	// HACK: If aiming a shot and moving, make chance to hit equal to
	// zero, yet still take shot (denny request: 4/22/98)
	//if ((aimLocation != -1) && (getVelocity().GetLength() > 0.0))
	//	attackChance = 0;

	long hitLocation = -1;
	if (target && (hitRoll < attackChance)) {
		//---------------------------------------------------------------------
		// This really shouldn't be hardcoded to work for just IS players! --gd
		if (target->getTeamId() == TEAM2)
			pilot->incNumSkillSuccesses(COMBAT_STAT_MISSION, MWS_GUNNERY);
		//------------------------------------------------------------------
		// If it's an aimed shot, we need to calc whether we hit the desired
		// location on the target...
		if (aimLocation != -1) {
			hitLocation = aimLocation;
		}
	}

	//-------------------------------------------
	// Ripple Fire
	// Figure out what weapon Node on the Mech we are going to use
	// And see if its recycle time is Green.  If not, just return, we'll shoot again!
	long sourceHotSpot;
	if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_MISSILE)
		sourceHotSpot = MECH3D_WEAPONTYPE_MISSILE;
	else if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_BALLISTIC)
		sourceHotSpot = MECH3D_WEAPONTYPE_BALLISTIC;
	else
		sourceHotSpot = MECH3D_WEAPONTYPE_ENERGY;

	sourceHotSpot = appearance->getWeaponNode(sourceHotSpot);

	//Check if we even found a weapon node to use.
	// If so, it will be in the range from 0 to 3.
	// If not, return (3).  No weapon node available!!!
	// DO NOT ALLOW fire to come from unit base position.
	// UNLESS WE ARE IN MULTIPLAYER!!!  Vehicle may not be on OUR screen!!
	float recycleTime = appearance->getWeaponNodeRecycle(sourceHotSpot);
	if (!MPlayer)
	{
		if (sourceHotSpot < 0 || sourceHotSpot > 3)
			return (3);

		if (recycleTime > 0.0f)
			return (3);
	}
		
	appearance->setWeaponNodeUsed(sourceHotSpot);
	
	//Set the hotspot for the target
	long targetHotSpot = 0;
	if (target && target->getAppearance())
	{
		targetHotSpot = target->getAppearance()->getWeaponNode(MECH3D_WEAPONTYPE_ANY);
	}
	
	//-----------------------
	// Weapon must recycle...
	startWeaponRecycle(weaponIndex);

 	WeaponBoltPtr weaponFX = NULL;	
	if (hitRoll < attackChance) {

		if (numShots != UNLIMITED_SHOTS) {
			//-------------------------------------------------------
			// We're taking the shot, so reduce our ammo inventory...
			deductWeaponShot(weaponIndex);
		}
		
		//------------
		// Attack hit.
		if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_MISSILE) {
			//---------------------------------------------------------
			// It's a missile weapon. We need to determine how many hit
			// (and missed) the target, and in how many clusters...
			long missileAmount = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponAmmoAmount();
			long numMissiles;
			if (isStreakMissile)
				numMissiles = missileAmount;
			else {
				numMissiles = (float(missileAmount) / 2.0) + 0.5;
				if (numMissiles < 1)
					numMissiles = 1;
				if (numMissiles > missileAmount)
					numMissiles = missileAmount;
			}

			//-----------------------------------------------
			// a MissileGen Object is ALL of the clusters.
			// Don't make a thousand of them or the game implodes!
			//numClusters = 1;
	
			//----------------------------------------------------
			// Need to know which hotspot this comes from.
			// Also need to know which hotspot this is going to.
			if (target) {
				if (aimLocation == -1)
					hitLocation = target->calcHitLocation(this, weaponIndex, ATTACKSOURCE_WEAPONFIRE, attackType);
				}
			else
				hitLocation = -1;
			Assert(hitLocation != -2, 0, " GroundVehicle.FireWeapon: Bad Hit Location ");
			//--------------------------------------
			// For now, always use a bullet effect...
			WeaponShotInfo curShotInfo;
			curShotInfo.init(this->getWatchID(),
							 inventory[weaponIndex].masterID,
							 numMissiles * MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
							 hitLocation,
							 entryAngle);
	
			dmgDone = curShotInfo.damage;

			//-------------------------------------------------------------------------
			// If I'm in a multiplayer game and I'm the server, record this weapon fire
			// so it may be broadcast to all clients...
			if (MPlayer && MPlayer->isServer()) {
				WeaponFireChunk chunk;
				chunk.init();
				if (target) {
					if (target->isMover())
						chunk.buildMoverTarget(target,
											   weaponIndex - numOther,
											   true,
											   entryAngle,
											   numMissiles,
											   hitLocation);
					else if (target->getObjectClass() == CAMERADRONE)
						chunk.buildCameraDroneTarget(target,
													 weaponIndex - numOther,
													 true,
													 entryAngle,
													 numMissiles);
					else
						chunk.buildTerrainTarget(target,
												 weaponIndex - numOther,
												 true,
												 numMissiles);
					}
				else
					chunk.buildLocationTarget(*targetPoint, weaponIndex - numOther, true, numMissiles);
				chunk.pack(this);
				WeaponFireChunk chunk2;
				chunk2.init();
				chunk2.data = chunk.data;
				chunk2.unpack(this);
				if (!chunk.equalTo(&chunk2))
					Fatal(0, " GVehicle.fireWeapon: Bad WeaponFireChunk (save wfchunk.dbg file now) ");
				addWeaponFireChunk(CHUNK_SEND, &chunk);
				LogWeaponFireChunk(&chunk, this, target);
			}
			unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
			weaponFX = ObjectManager->createWeaponBolt(effectType);
			if (!weaponFX) 
			{
				if (target)
					target->handleWeaponHit(&curShotInfo, MPlayer != NULL);
				else if (targetPoint) 
				{
					//-----------------------------------------
					// Check for Mine hit and MOVE ON!!!
					int cellRow, cellCol;
					land->worldToCell(*targetPoint, cellRow, cellCol);
					if (GameMap->getMine(cellRow, cellCol) == 1) 
					{
						ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, *targetPoint, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
						GameMap->setMine(cellRow, cellCol, 2);	//Mark Exploded
					}
				}
			}
			else {
				if (target)
					weaponFX->connect(this, target, &curShotInfo, sourceHotSpot, targetHotSpot);
				else {
					weaponFX->connect(this, *targetPoint, &curShotInfo, sourceHotSpot);
					pilot->clearCurTacOrder();
				}
				printFireWeaponDebugInfo(target, targetPoint, attackChance, aimLocation, hitRoll, &curShotInfo);
			}
			}
		else {
			//----------------------------------------------------
			// Non-missile weapon, so just one weapon hit spawn...
			// For now, always use a laser effect...
			if (target) {
				if (aimLocation == -1)
					hitLocation = target->calcHitLocation(this, weaponIndex, ATTACKSOURCE_WEAPONFIRE, attackType);
				}
			else
				hitLocation = -1;
			Assert(hitLocation != -2, 0, " GroundVehicle.FireWeapon: Bad Hit Location ");

			WeaponShotInfo shotInfo;
			shotInfo.init(this->getWatchID(),
				inventory[weaponIndex].masterID,
				MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
				hitLocation,
				entryAngle);

			dmgDone = shotInfo.damage;

			//-------------------------------------------------------------------------
			// If I'm in a multiplayer game and I'm the server, record this weapon fire
			// so it may be broadcast to all clients...
			if (MPlayer && MPlayer->isServer()) {
				WeaponFireChunk chunk;
				chunk.init();
				if (target) {
					if (target->isMover())
						chunk.buildMoverTarget(target,
											   weaponIndex - numOther,
											   true,
											   entryAngle,
											   0,
											   hitLocation);
//					else if (target->getObjectClass() == TRAINCAR)
//						chunk.buildTrainTarget((BigGameObjectPtr)target,
//												weaponIndex - numOther,
//												true,
//												entryAngle,
//												0);
					else if (target->getObjectClass() == CAMERADRONE)
						chunk.buildCameraDroneTarget(target,
													 weaponIndex - numOther,
													 true,
													 entryAngle,
													 0);
					else
						chunk.buildTerrainTarget(target,
												 weaponIndex - numOther,
												 true,
												 0);
					}
				else
					chunk.buildLocationTarget(*targetPoint, weaponIndex - numOther, true, 0);
				chunk.pack(this);
				WeaponFireChunk chunk2;
				chunk2.init();
				chunk2.data = chunk.data;
				chunk2.unpack(this);
				if (!chunk.equalTo(&chunk2))
					Fatal(0, " GVehicle.fireWeapon: Bad WeaponFireChunk (save wfchunk.dbg file now) ");
				addWeaponFireChunk(CHUNK_SEND, &chunk);
				LogWeaponFireChunk(&chunk, this, target);
			}

			unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
			weaponFX = ObjectManager->createWeaponBolt(effectType);
			if (!weaponFX) {
				if (target)
					target->handleWeaponHit(&shotInfo, MPlayer != NULL);
				else if (targetPoint) 
				{
					//-----------------------------------------
					// Check for Mine hit and MOVE ON!!!
					int cellRow, cellCol;
					land->worldToCell(*targetPoint, cellRow, cellCol);
					if (GameMap->getMine(cellRow, cellCol) == 1)
					{
						ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, *targetPoint, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
						GameMap->setMine(cellRow, cellCol, 2);	//Mark Exploded
					}
				}
				}
			else {
				if (target) {
					weaponFX->connect(this, target, &shotInfo, sourceHotSpot, targetHotSpot);
					}
				else {
					weaponFX->connect(this, *targetPoint, &shotInfo, sourceHotSpot);
					if ( !suppressionFire )
						pilot->clearCurTacOrder();
				}
				printFireWeaponDebugInfo(target, targetPoint, attackChance, aimLocation, hitRoll, &shotInfo);
			}
		}
		}
	else {
		//if ((aimLocation != -1) && (RandomNumber(100) < AimedFireAbort))
		//	return(NO_ERR);
		if (!isStreakMissile) { 

			if (numShots != UNLIMITED_SHOTS) {
				//-------------------------------------------------------
				// We're taking the shot, so reduce our ammo inventory...
				deductWeaponShot(weaponIndex);
			}

			//----------------------------------------------------
			// Miss, so check for possible miss resolution here...
			if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_MISSILE)
			{
				//---------------------------------------------------------
				// It's a missile weapon. We need to determine how many hit
				// (and missed) the target, and in how many clusters...
				long missileAmount = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponAmmoAmount();
				long numMissiles = ((float)missileAmount / 2.0) + 0.5;
				if (numMissiles < 1)
					numMissiles = 1;
				if (numMissiles > missileAmount)
					numMissiles = missileAmount;

				if (numMissiles) {
					WeaponShotInfo curShotInfo;
					curShotInfo.init(this->getWatchID(),
									 inventory[weaponIndex].masterID,
									 numMissiles * MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
									 -1,
									 entryAngle);
	
					//-------------------------------------------------------------------------------------------
					// If we missed, pick sights away from the target and check LOS to each one.  If all are
					// invisible, just hit the target with zero points of damage.
					float missRadius = target ? 25.0 : 5.0;
	
					Stuff::Vector3D positionOffset;
					if (target)
						positionOffset = target->getPosition();
					else if (targetPoint)
						positionOffset = *targetPoint;
	
					positionOffset.x += missRadius;
					positionOffset.z = land->getTerrainElevation(positionOffset);

					bool canSeeHit = lineOfSight(positionOffset,true);

					if (!canSeeHit)
					{
						positionOffset.x -= (missRadius * 2.0f);
						positionOffset.z = land->getTerrainElevation(positionOffset);

						canSeeHit = lineOfSight(positionOffset,true); 

						if (!canSeeHit)
						{
							positionOffset.x += missRadius;
							positionOffset.y += missRadius;

							positionOffset.z = land->getTerrainElevation(positionOffset);

							canSeeHit = lineOfSight(positionOffset,true);

							if (!canSeeHit)
							{
								positionOffset.y -= (missRadius * 2.0f);
								positionOffset.z = land->getTerrainElevation(positionOffset);

								canSeeHit = lineOfSight(positionOffset,true);

								if (!canSeeHit)
								{
									//OK, no miss location is visible.  Hit the target with ZERO damage!!
									curShotInfo.init(this->getWatchID(),
													 inventory[weaponIndex].masterID,
													 0.0f,
													 -1,
													 entryAngle);
								}
							}
						}
					}

					//-------------------------------------------------------------------------
					// If I'm in a multiplayer game and I'm the server, record this weapon fire
					// so it may be broadcast to all clients...
					if (MPlayer && MPlayer->isServer()) {
						WeaponFireChunk chunk;
						chunk.init();
						chunk.buildLocationTarget(positionOffset, weaponIndex - numOther, false, numMissiles);
						chunk.pack(this);
						WeaponFireChunk chunk2;
						chunk2.init();
						chunk2.data = chunk.data;
						chunk2.unpack(this);
						if (!chunk.equalTo(&chunk2))
							Fatal(0, " GVehicl.fireWeapon: Bad WeaponFireChunk (save wfchunk.dbg file now) ");
						addWeaponFireChunk(CHUNK_SEND, &chunk);
						LogWeaponFireChunk(&chunk, this, target);
					}
					unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
					weaponFX = ObjectManager->createWeaponBolt(effectType);
					if (!weaponFX) {
						//-----------------------------------------
						// Check for Mine hit and MOVE ON!!!
						int cellRow, cellCol;
						land->worldToCell(positionOffset, cellRow, cellCol);
						if (GameMap->getMine(cellRow, cellCol) == 1) 
						{
							ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, positionOffset, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
							GameMap->setMine(cellRow, cellCol, 2); // Mark Exploded
						}
						}
					else
					{
						if (canSeeHit)		//miss location is in LOS.  Hit the ground
							weaponFX->connect(this,positionOffset,&curShotInfo,sourceHotSpot);
						else if (target)	//Miss location is NOT in LOS.  Hit Target with ZERO damage!!!
							weaponFX->connect(this,target,&curShotInfo,sourceHotSpot);
					}

					printFireWeaponDebugInfo(target, &positionOffset, attackChance, aimLocation, hitRoll, &curShotInfo);
				}
				}
			else {
				//----------------------------------------------------
				// Non-missile weapon, so just one weapon hit spawn...
				// For now, always use a laser effect...
				WeaponShotInfo shotInfo;
				shotInfo.init(this->getWatchID(),
					inventory[weaponIndex].masterID,
					MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
					-1,
					entryAngle);

				//-------------------------------------------------------------------------------------------
				// If we missed, pick sights away from the target and check LOS to each one.  If all are
				// invisible, just hit the target with zero points of damage.
				float missRadius = target ? 25.0 : 5.0;

				Stuff::Vector3D positionOffset;
				if (target)
					positionOffset = target->getPosition();
				else if (targetPoint)
					positionOffset = *targetPoint;

				positionOffset.x += missRadius;
				positionOffset.z = land->getTerrainElevation(positionOffset);

				bool canSeeHit = lineOfSight(positionOffset,true);

				if (!canSeeHit)
				{
					positionOffset.x -= (missRadius * 2.0f);
					positionOffset.z = land->getTerrainElevation(positionOffset);

					canSeeHit = lineOfSight(positionOffset,true); 

					if (!canSeeHit)
					{
						positionOffset.x += missRadius;
						positionOffset.y += missRadius;

						positionOffset.z = land->getTerrainElevation(positionOffset);

						canSeeHit = lineOfSight(positionOffset,true);

						if (!canSeeHit)
						{
							positionOffset.y -= (missRadius * 2.0f);
							positionOffset.z = land->getTerrainElevation(positionOffset);

							canSeeHit = lineOfSight(positionOffset,true);

							if (!canSeeHit)
							{
								//OK, no miss location is visible.  Hit the target with ZERO damage!!
								shotInfo.init(this->getWatchID(),
											 inventory[weaponIndex].masterID,
											 0.0f,
											 -1,
											 entryAngle);
							}
						}
					}
				}

				//-------------------------------------------------------------------------
				// If I'm in a multiplayer game and I'm the server, record this weapon fire
				// so it may be broadcast to all clients...
				if (MPlayer && MPlayer->isServer()) {
					WeaponFireChunk chunk;
					chunk.init();
					chunk.buildLocationTarget(positionOffset, weaponIndex - numOther, false, 0);
					chunk.pack(this);
					WeaponFireChunk chunk2;
					chunk2.init();
					chunk2.data = chunk.data;
					chunk2.unpack(this);
					if (!chunk.equalTo(&chunk2))
						Fatal(0, " GVehicl.fireWeapon: Bad WeaponFireChunk (save wfchunk.dbg file now) ");
					addWeaponFireChunk(CHUNK_SEND, &chunk);
					LogWeaponFireChunk(&chunk, this, target);
				}
				unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
				weaponFX = ObjectManager->createWeaponBolt(effectType);
				if (!weaponFX) {
					//-----------------------------------------
					// Check for Mine hit and MOVE ON!!!
					int cellRow, cellCol;
					land->worldToCell(positionOffset, cellRow, cellCol);
					if (GameMap->getMine(cellRow, cellCol) == 1) 
					{
						ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, positionOffset, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
						GameMap->setMine(cellRow, cellCol, 2);	//Mark Exploded
					}
					}
				else
				{
					if (canSeeHit)		//miss location is in LOS.  Hit the ground
						weaponFX->connect(this,positionOffset,&shotInfo,sourceHotSpot);
					else if (target)	//Miss location is NOT in LOS.  Hit Target with ZERO damage!!!
						weaponFX->connect(this,target,&shotInfo,sourceHotSpot);
				}
				printFireWeaponDebugInfo(target, &positionOffset, attackChance, aimLocation, hitRoll, &shotInfo);
			}
		}
	}

	//-------------------------------------------------------------------
	// Trigger the WEAPON TARGET event. For now, this assumes the target
	// KNOWS we were targeting him. Of course, the target wouldn't always
	// be aware of this, would they?
	pilot->triggerAlarm(PILOT_ALARM_FIRED_WEAPON, target ? target->getWatchID() : 0);
	if (targetPilot)
		targetPilot->triggerAlarm(PILOT_ALARM_TARGET_OF_WEAPONFIRE, getWatchID());

	timeSinceFiredLast = 0.0f;

	if (getGroup())
		getGroup()->handleMateFiredWeapon(getWatchID());

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long GroundVehicle::handleWeaponFire (long weaponIndex,
								   GameObjectPtr target,
								   Stuff::Vector3D* targetPoint,
								   bool hit,
								   float entryAngle,
								   long numMissiles,
								   long hitLocation) {

	//--------------------------------------------------------
	// If it's already been fired, assume the ammo is there...
	long numShots = getWeaponShots(weaponIndex);
	//Assert(numShots > 0, numShots, " handleWeaponFire: numShots is negative! ");

	//-----------------------
	// Weapon must recycle...
	startWeaponRecycle(weaponIndex);

	bool isStreakMissile = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponStreak(); 

	WeaponBoltPtr weaponFX = NULL;
	
	//----------------------------------------------------
	// Need to know which hotspot this comes from.
	// Also need to know which hotspot this is going to.
	long sourceHotSpot;
	if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_MISSILE)
		sourceHotSpot = MECH3D_WEAPONTYPE_MISSILE;
	else if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_BALLISTIC)
		sourceHotSpot = MECH3D_WEAPONTYPE_BALLISTIC;
	else
		sourceHotSpot = MECH3D_WEAPONTYPE_ENERGY;
 
	sourceHotSpot = appearance->getWeaponNode(sourceHotSpot);
	
	//No Ripple fire in MPLayer.  Must just fire the weapon when we get the packet.
	
	appearance->setWeaponNodeUsed(sourceHotSpot);
	
 	//Set the hotspot for the target
	long targetHotSpot = 0;
	if (target && target->getAppearance())
	{
		targetHotSpot = target->getAppearance()->getWeaponNode(MECH3D_WEAPONTYPE_ANY);
	}
	
 	if (hit) {
		//------------
		// Attack hit.
		if (numShots != UNLIMITED_SHOTS) {
			//-------------------------------------------------------
			// We're taking the shot, so reduce our ammo inventory...
			deductWeaponShot(weaponIndex);
		}

		if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_MISSILE) {
			//---------------------------------------------------------
			// It's a missile weapon. We need to determine how many hit
			// (and missed) the target, and in how many clusters...
			WeaponShotInfo curShotInfo;

			if (numMissiles) {
				//-------------------------------------------------------------------
				// This code will mess up if the object is not a BULLET!!!!!!!!!!!
				Assert(hitLocation != -2, TargetRolo, " GroundVehicle.handleWeaponFire: Bad Hit Location ");
	
				//--------------------------------------
				// For now, always use a bullet effect...
				curShotInfo.init(this->getWatchID(),
								 inventory[weaponIndex].masterID,
								 numMissiles * MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
								 hitLocation,
								 entryAngle);
	
				unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
				weaponFX = ObjectManager->createWeaponBolt(effectType);
				if (!weaponFX) {
					if (target) {
						if (MPlayer) {
							if (MPlayer->isServer())
								target->handleWeaponHit(&curShotInfo, true);
							}
						else
							target->handleWeaponHit(&curShotInfo);
					}
					}
				else {
					if (target) {
						weaponFX->connect(this, target, &curShotInfo, sourceHotSpot, targetHotSpot);
						}
					else {
						weaponFX->connect(this, *targetPoint, &curShotInfo, sourceHotSpot);
						if (!suppressionFire)
							pilot->clearCurTacOrder();
					}
				}
			}
		}
		else
		{
			//----------------------------------------------------
			// Non-missile weapon, so just one weapon hit spawn...
			// For now, always use a laser effect...
			
			WeaponShotInfo shotInfo;
			shotInfo.init(this->getWatchID(),
				inventory[weaponIndex].masterID,
				MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
				hitLocation,
				entryAngle);

			unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
			weaponFX = ObjectManager->createWeaponBolt(effectType);
			if (!weaponFX) {
				if (target) {
					if (MPlayer) {
						if (MPlayer->isServer())
							target->handleWeaponHit(&shotInfo, true);
						}
					else
						target->handleWeaponHit(&shotInfo);
				}
				}
			else {
				if (target) {
					weaponFX->connect(this, target, &shotInfo, sourceHotSpot, targetHotSpot);
					}
				else {
					weaponFX->connect(this, *targetPoint, &shotInfo, sourceHotSpot);
					if (!suppressionFire)
						pilot->clearCurTacOrder();
				}
			}
		}
		}
	else {
		Assert(target == NULL, 0, " GVehicl.handleWeaponFire: target should be NULL with network miss! ");
		Assert(targetPoint != NULL, 0, " GVehicl.handleWeaponFire: MUST have targetpoint with network miss! ");
		if (isStreakMissile) {
			CurMoverWeaponFireChunk.unpack(this);
			DebugWeaponFireChunk(&CurMoverWeaponFireChunk, NULL, this);
			Assert(false, 0, " GVehicl.handleWeaponFire: streaks shouldn't miss! ");
		}

		if (numShots != UNLIMITED_SHOTS) {
			//-------------------------------------------------------
			// We're taking the shot, so reduce our ammo inventory...
			deductWeaponShot(weaponIndex);
		}

		//----------------------------------------------------
		// Miss, so check for possible miss resolution here...
		if (MasterComponent::masterList[inventory[weaponIndex].masterID].getForm() == COMPONENT_FORM_WEAPON_MISSILE) {

			if (numMissiles > 0) {
				//-----------------------------------------------
				// a MissileGen Object is ALL of the clusters.
				// Don't make a thousand of them or the game implodes!
				//numClusters = 1;
				unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
				weaponFX = ObjectManager->createWeaponBolt(effectType);
				if (!weaponFX)
					Fatal(-1," couldnt create weapon FX ");
				else {
					WeaponShotInfo curShotInfo;
					curShotInfo.init(this->getWatchID(),
								 inventory[weaponIndex].masterID,
								 numMissiles * MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
								 -1,
								 entryAngle);
					weaponFX->connect(this, *targetPoint, &curShotInfo, sourceHotSpot);
				}
			}
			}
		else {
			//----------------------------------------------------
			// Non-missile weapon, so just one weapon hit spawn...
			// For now, always use a laser effect...
			WeaponShotInfo shotInfo;
			shotInfo.init(this->getWatchID(),
				inventory[weaponIndex].masterID,
				MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponDamage(),
				-1,
				entryAngle);
			unsigned char effectType = MasterComponent::masterList[inventory[weaponIndex].masterID].getWeaponSpecialEffect();
			weaponFX = ObjectManager->createWeaponBolt(effectType);
			if (!weaponFX)
				Fatal(-1," couldnt create weapon FX ");
			else
				weaponFX->connect(this, *targetPoint, &shotInfo, sourceHotSpot);
		}
	}

	//-------------------------------------------------------------------
	// Trigger the WEAPON TARGET event. For now, this assumes the target
	// KNOWS we were targeting him. Of course, the target wouldn't always
	// be aware of this, would they?
	MechWarriorPtr targetPilot = NULL;
	if (target && target->isMover()) {
		targetPilot = ((MoverPtr)target)->getPilot();
		targetPilot->updateAttackerStatus(getWatchID(), scenarioTime);
		targetPilot->triggerAlarm(PILOT_ALARM_TARGET_OF_WEAPONFIRE, getWatchID());
	}

	if (getGroup())
		getGroup()->handleMateFiredWeapon(getWatchID());

	return(NO_ERR);
}

//---------------------------------------------------------------------------

#if 0

float GroundVehicle::calcMaxSpeed (void) {

	//--------------------------------------------------
	// Until we finalize how we're setting speed limits,
	// this will do...
	if (canMove())
		return(maxRunSpeed);
	return(0.0);
}

//---------------------------------------------------------------------------

float GroundVehicle::calcSlowSpeed (void) {

	if (canMove())
		return(maxRunSpeed * 0.25);
	return(0.0);
}

//---------------------------------------------------------------------------

float GroundVehicle::calcModerateSpeed (void) {

	if (canMove())
		return(maxRunSpeed * 0.4);
	return(0.0);
}

//---------------------------------------------------------------------------

void GroundVehicle::calcSpriteSpeed (float speed, long& state, long& throttle) {

	//--------------------------------------------------------------------
	// For now, we assume the full walk speed is always 0.5 * maxRunSpeed.
	// This must be corrected when sprite speeds are finalized. I suggest
	// we add a maxWalkSpeed field to the BattleMech class...
	throttle = 100;
}

#endif

//---------------------------------------------------------------------------

float GroundVehicle::getTotalEffectiveness(void)
{
float weaponEffect, maxFront, curFront, armorFront, maxLeft, curLeft, armorLeft, maxRight, curRight, armorRight;
float maxRear, curRear, armorRear, maxTurret, curTurret, armorTurret, armorEffect, pilotEffect, result;

		if (isDestroyed() || isDisabled())
			return 0.0;

		//-------------------------------------------------------------------------
		// calculate Weapon effectiveness
		if (maxWeaponEffectiveness)
			weaponEffect = (float)weaponEffectiveness / (float)maxWeaponEffectiveness;
		else
			weaponEffect = 1.0;

		//---------------------------------------------------------------
		// Calculate armor effectiveness.
		float extraEffect = 1.0f;
		if (isRefit() || mineLayer )
		{
			//---------TURRET -- Which is Refit Points -- Factor in Please!
			curTurret = armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor;
			maxTurret = armor[GROUNDVEHICLE_LOCATION_TURRET].maxArmor;
			armorTurret = 1.0;
			
			if (maxTurret != 0.0)
				armorTurret = curTurret / maxTurret;
				
			extraEffect = armorTurret;
 		}
		
		//---------FRONT
		maxFront = armor[GROUNDVEHICLE_LOCATION_FRONT].maxArmor;
		curFront = armor[GROUNDVEHICLE_LOCATION_FRONT].curArmor;
		armorFront = curFront / maxFront * 0.6 + (0.4);
		
		//---------LEFT
		curLeft = armor[GROUNDVEHICLE_LOCATION_LEFT].curArmor;
		maxLeft = armor[GROUNDVEHICLE_LOCATION_LEFT].maxArmor;
		armorLeft = curLeft / maxLeft * 0.6 + (0.4);
		
		//---------RIGHT
		curRight = armor[GROUNDVEHICLE_LOCATION_RIGHT].curArmor;
		maxRight = armor[GROUNDVEHICLE_LOCATION_RIGHT].maxArmor;
		armorRight = curRight / maxRight * 0.6 + (0.4);
		
		//---------REAR
		curRear = armor[GROUNDVEHICLE_LOCATION_REAR].curArmor;
		maxRear = armor[GROUNDVEHICLE_LOCATION_REAR].maxArmor;
		armorRear = curRear / maxRear * 0.6 + (0.4);
		
		//---------TURRET
		curTurret = armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor;
		maxTurret = armor[GROUNDVEHICLE_LOCATION_TURRET].maxArmor;
		armorTurret = 1.0;
		
		if (maxTurret != 0.0)
			armorTurret = curTurret / maxTurret * 0.6 + (0.4);
		
		armorEffect = armorFront * armorRear * armorLeft * armorRight * armorTurret;
		
		//-------------------------------------------------------------------------
		// Calculate pilot Wounds
		float pilotWoundTable[7] = 
		{
			1.0,0.95f,0.85f,0.75f,0.50f,0.30f,0.00
		};
		
		pilotEffect = pilotWoundTable[float2long(getPilot()->getWounds())];
		

		result = weaponEffect * armorEffect * pilotEffect * extraEffect;
		return result;
}

//***************************************************************************
void GroundVehicle::Save (PacketFilePtr file, long packetNum)
{
	GroundVehicleData data;
	CopyTo(&data);

	//PacketNum incremented in ObjectManager!!
	file->writePacket(packetNum,(MemoryPtr)&data,sizeof(GroundVehicleData),STORAGE_TYPE_ZLIB);
}

//***************************************************************************
void GroundVehicle::CopyTo (GroundVehicleData *data)
{																	   
	data->accel 				   	 = accel;               
	data->velocityMag 	             = velocityMag;         
														 
	data->suspension                 = suspension;          
	data->movementEnabled            = movementEnabled;     
														 
	data->turretEnabled              = turretEnabled;       
	data->turretRotation 			 = turretRotation;      		
	data->turretBlownThisFrame       = turretBlownThisFrame;
														 
	data->captureable                = captureable;         
	data->deadByCrushed 			 = deadByCrushed;       		
														 
	data->canRefit 					 = canRefit;            	
	data->canRecover                 = canRecover;          
	data->refitting 				 = refitting;           		
	data->recovering                 = recovering;          
	data->mineSweeper                = mineSweeper;         
	data->sweepTime 				 = sweepTime;           		
	data->mineLayer                  = mineLayer;           
	data->aerospaceSpotter           = aerospaceSpotter;    
	data->cellColToMine              = cellColToMine;       
	data->cellRowToMine              = cellRowToMine;       
														 
	data->notMineYet                 = notMineYet;          
														 
	data->battleRating 				 = battleRating;        	
														 
	data->descID 					 = descID;              		
														 
	data->timeInCurrent 			 = timeInCurrent;       		
														 
	data->dVel                       = dVel;                
	data->dRot                       = dRot;                
	data->dAcc                       = dAcc;                
	data->dRVel                      = dRVel;               
	data->dRacc                      = dRacc;               
	data->dTime                      = dTime;               

	data->isVehiclePilot			 = isVehiclePilot;

	if (sensorSystem)
		data->sensorOK					 = !sensorSystem->broken;
	else
		data->sensorOK					 = false;

	Mover::CopyTo(data);
}

//---------------------------------------------------------------------------
void GroundVehicle::Load (GroundVehicleData *data)
{
	Mover::Load(data);

	accel 				   	 = data->accel;               
	velocityMag 	         = data->velocityMag;         
													 
	suspension               = data->suspension;          
	movementEnabled          = data->movementEnabled;     
													 
	turretEnabled            = data->turretEnabled;       
	turretRotation 			 = data->turretRotation;      		
	turretBlownThisFrame     = data->turretBlownThisFrame;
													 
	captureable              = data->captureable;         
	deadByCrushed 			 = data->deadByCrushed;       		
													 
	canRefit 				 = data->canRefit;            	
	canRecover               = data->canRecover;          
	refitting 				 = data->refitting;           		
	recovering               = data->recovering;          
	mineSweeper              = data->mineSweeper;         
	sweepTime 				 = data->sweepTime;           		
	mineLayer                = data->mineLayer;           
	aerospaceSpotter         = data->aerospaceSpotter;    
	cellColToMine            = data->cellColToMine;       
	cellRowToMine            = data->cellRowToMine;       
													 
	notMineYet               = data->notMineYet;          
													 
	battleRating 			 = data->battleRating;        	
													 
	descID 					 = data->descID;              		
													 
	timeInCurrent 			 = data->timeInCurrent;       		
													 
	dVel                     = data->dVel;                
	dRot                     = data->dRot;                
	dAcc                     = data->dAcc;                
	dRVel                    = data->dRVel;               
	dRacc                    = data->dRacc;               
	dTime                    = data->dTime;               

	isVehiclePilot			 = data->isVehiclePilot;

	if (data->sensorOK)
	{
		sensorSystem = SensorManager->newSensor();
		sensorSystem->setOwner(this);
		sensorSystem->setRange(MasterComponent::masterList[inventory[sensor].masterID].getSensorRange());
		SensorManager->addTeamSensor(teamId, sensorSystem);
	}
}

bool  GroundVehicle::burnRefitPoints (float pointsToBurn)
{
	if (canRefit) 
	{
		armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor -= pointsToBurn;
		if ( armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor <= 0.0f )
		{
			armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor = 0.f;
			if (getCommanderId() == Commander::home->getId())
				soundSystem->playBettySample( BETTY_REPAIR_GONE );
		}
		return(true);
	}
	else
		return(false);
}


//***************************************************************************

