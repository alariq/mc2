//---------------------------------------------------------------------------
//
//	objmgr.cpp - This file contains the GameObject Manager class code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// Include Files

//=============
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DOBJMGR_H
#include"dobjmgr.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#ifndef ARTLRY_H
#include"artlry.h"
#endif

#ifndef TURRET_H
#include"turret.h"
#endif

#ifndef GROUP_H
#include"group.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

#ifndef MECH_H
#include"mech.h"
#endif

#ifndef GVEHICL_H
#include"gvehicl.h"
#endif

#ifndef TERROBJ_H
#include"terrobj.h"
#endif

#ifndef WEAPONBOLT_H
#include"weaponbolt.h"
#endif

#ifndef CARNAGE_H
#include"carnage.h"
#endif

#ifndef LIGHT_H
#include"light.h"
#endif

#ifndef GATE_H
#include"gate.h"
#endif

#ifdef USE_ELEMENTALS
#ifndef ELEMNTL_H
#include"elemntl.h"
#endif
#endif

#ifndef COLLSN_H
#include"collsn.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#define BRIDGE_OBJTYPE				448
#define MINE1						60
#define MINE2						251
#define MINE3						521
#define MINE4						522
#define MINE5						610
#define MINE6						611
#define MINE7						612

//---------------------------------------------------------------------------
// Static Globals
#if 1	// ids can be 8 long, INCLUDING TERMINATOR!!!!!
char DEFAULT_LIST_ID[] = "DEFAULT";
char CLANMECH_LIST_ID[] = "CLANMEC";
char ISMECH_LIST_ID[] = "ISMECH";
char ICON_LIST_ID[] = "ICONS";
char WEAPON_LIST_ID[] = "WEAPON";
#else
char DEFAULT_LIST_ID[] = "DEFAULT!";
char CLANMECH_LIST_ID[] = "CLANMECH";
char ISMECH_LIST_ID[] = "ISMECH";
char ICON_LIST_ID[] = "ICONS";
char WEAPON_LIST_ID[] = "WEAPON";
#endif
//long ObjectQueue::objectsInList = 0;

extern long usedBlockList[];			//Trust ME~~!!!!!!!!!!!!!!!!!!!!!!!!
extern long moverBlockList[];			//Trust ME~~!!!!!!!!!!!!!!!!!!!!!!!!  AGAIN 
extern bool updateTerrainObjects;
extern bool	updateObjects;
extern bool	renderTerrainObjects;
extern bool	renderObjects;
extern GameObjectPtr MoverRoster[MAX_MOVER_PART_ID - MIN_MOVER_PART_ID + 1];
extern bool MaxObjectsDrawn;
extern bool drawOldWay;

/*
ObjectQueuePtr objectList = NULL;
ObjectQueueNodePtr clanMechList = NULL;
ObjectQueueNodePtr innerSphereMechList = NULL;
ObjectQueueNodePtr iconList = NULL;
ObjectQueueNodePtr weaponList = NULL;
*/

#define	VISIBLE_THRESHOLD	1

GameObjectManagerPtr ObjectManager = NULL;
GameObjectPtr* collisionList = NULL;
long numCollidables = 0;
//***************************************************************************
//* MISC routines
//***************************************************************************

bool blockInList (long blockNum) 
{
	long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;

	for (long i = 0; i < totalBlocks; i++) {
		if (usedBlockList[i] == blockNum)
			return(TRUE);
		else if (usedBlockList[i] == -1)
			return(FALSE);
	}
	
	return(FALSE);
}

//---------------------------------------------------------------------------

bool moverInList (long blockNum) 
{
	long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;

	for (long i = 0; i < totalBlocks; i++) {
		if (moverBlockList[i] == blockNum)
			return(TRUE);
		else if (moverBlockList[i] == -1)
			return(FALSE);
	}
	
	return(FALSE);
}

//***************************************************************************
//* GAMEOBJECT MANAGER class
//***************************************************************************

void* GameObjectManager::operator new (size_t ourSize) {

	void* result = systemHeap->Malloc(ourSize);
	if (!result) 
	{
		Fatal(0, " GameObjectManager.new: unable to create GameObject Manager ");
	}

	return(result);
}

//---------------------------------------------------------------------------

void GameObjectManager::operator delete (void* us) {

	systemHeap->Free(us);
}

//---------------------------------------------------------------------------

void GameObjectManager::init (void) {

	objTypeManager = NULL;

	numMechs = 0;
	numVehicles = 0;
	numElementals = 0;
	numTerrainObjects = 0;
	numBuildings = 0;
	numTurrets = 0;
	numWeapons = 0;
	numCarnage = 0;
	numLights = 0;
	numGates = 0;
	numArtillery = 0;

	numGateControls = 0;
	numTurretControls = 0;
	numPowerGenerators = 0;
	numSpecialBuildings = 0;

	//carnageCount = 0;
	Terrain::numObjBlocks = 0;

	mechs = NULL;
	vehicles = NULL;
	elementals = NULL;
	terrainObjects = NULL;
	buildings = NULL;
	turrets = 0;
	weapons = NULL;
	carnage = NULL;
	lights = NULL;
	artillery = NULL;
	gates = NULL;

	objList = NULL;
	collidableList = NULL;
	numCollidables = 0;
	numGoodMovers = 0;
	numBadMovers = 0;
	numMovers = 0;
	nextReinforcementPartId = MIN_REINFORCEMENT_PART_ID;
	numRemoved = 0;
	nextWatchID = 1;

	currentWeaponsIndex = 0;
	currentCarnageIndex = 0;
	currentLightIndex = 0;
	currentArtilleryIndex = 0;
	
	long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;

	for (long i = 0; i < totalBlocks; i++) {
		Terrain::objBlockInfo[i].active = false;
		Terrain::objBlockInfo[i].firstHandle = 0;
		Terrain::objBlockInfo[i].numObjects = 0;
	}

}

//---------------------------------------------------------------------------

void GameObjectManager::init (const char* objTypeDataFile, long objTypeHeapSize, long objHeapSize) {

	if (objTypeManager)
		delete objTypeManager;

	objTypeManager = new ObjectTypeManager;
	if (!objTypeManager)
		Fatal(0, " GameObjectManager.init: unable to create objTypeManager ");
	long result = objTypeManager->init(objTypeDataFile, objTypeHeapSize, objHeapSize);
	if (result != NO_ERR)
		Fatal(0, " GameObjectManager.init: unable to init objTypeManager ");
}

//---------------------------------------------------------------------------

void GameObjectManager::setNumObjects (long nMechs,
									   long nVehicles,
									   long nElementals,
									   long nTerrainObjects,
									   long nBuildings,
									   long nTurrets,
									   long nWeapons,
									   long nCarnage,
									   long nLights,
									   long nArtillery,
									   long nGates) 
{

	long i=0;

	numMechs = nMechs;
	numVehicles = nVehicles;
	numElementals = nElementals;

	//-------------------------------------------------------------------------
	// We need to make room for a potential max # of reinforcements per team...
	//
	// Please DONT add 2 to each of these.   You wind up pointing into the next list!!!!
	// -fs   6/14/2000
	maxMechs = numMechs + MAX_TEAMS * MAX_REINFORCEMENTS_PER_TEAM;
	maxVehicles = numVehicles + MAX_TEAMS * MAX_REINFORCEMENTS_PER_TEAM;
	maxMovers = maxMechs + maxVehicles + numElementals;

	if (nTerrainObjects > -1)
		numTerrainObjects = nTerrainObjects;
	if (nBuildings > -1)
		numBuildings = nBuildings;
	if (nTurrets > -1)
		numTurrets = nTurrets;
	if (nWeapons > -1)
		numWeapons = nWeapons;
	if (nCarnage > -1)
		numCarnage = nCarnage;
	if (nLights > -1)
		numLights = nLights;
	if (nArtillery > -1)
		numArtillery = nArtillery;
	if (nGates > -1)
		numGates = nGates;

	GameObject::setInitialize(true);

	//-----------------------------------------------------------
	// First element in list is NULL (handle of 0 is always NULL)
	objList = (GameObjectPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GameObjectPtr) * (getMaxObjects() + 1));
	memset(objList,0,sizeof(GameObjectPtr) * (getMaxObjects() + 1));

	watchList = (GameObjectPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GameObjectPtr) * (getMaxObjects() + 1));
	memset(watchList,0,sizeof(GameObjectPtr) * (getMaxObjects() + 1));

	long curHandle = 1;
	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (maxMechs > 0) {
		mechs = (BattleMechPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(BattleMechPtr) * maxMechs);
		if (!mechs)
			Fatal(maxMechs, " GameObjectManager.setNumObjects: cannot malloc mechs ");
		for (i = 0; i < maxMechs; i++) {
			mechs[i] = new BattleMech;
			mechs[i]->setHandle(curHandle);
			objList[curHandle++] = mechs[i];
		}
		for (i = numMechs; i < maxMechs; i++)
			mechs[i]->setExists(false);
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (maxVehicles > 0) {
		vehicles = (GroundVehiclePtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GroundVehiclePtr) * maxVehicles);
		if (!vehicles)
			Fatal(maxVehicles, " GameObjectManager.setNumObjects: cannot malloc vehicles ");
		for (i = 0; i < maxVehicles; i++) {
			vehicles[i] = new GroundVehicle;
			vehicles[i]->setHandle(curHandle);
			objList[curHandle++] = vehicles[i];
		}
		for (i = numVehicles; i < maxVehicles; i++)
			vehicles[i]->setExists(false);
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
#ifdef USE_ELEMENTALS
	if (numElementals > 0) {
		elementals = (ElementalPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(ElementalPtr) * numElementals);
		if (!elementals)
			Fatal(numElementals, " GameObjectManager.setNumObjects: cannot malloc elementals ");
		for (i = 0; i < numElementals; i++) {
			elementals[i] = new Elemental;
			elementals[i]->setHandle(curHandle);
			objList[curHandle++] = elementals[i];
		}
	}
#endif

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (numTerrainObjects > 0) {
		terrainObjects = (TerrainObjectPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(TerrainObjectPtr) * numTerrainObjects);
		if (!terrainObjects)
			Fatal(numTerrainObjects, " GameObjectManager.setNumObjects: cannot malloc terrain objects ");

		for (i = 0; i < numTerrainObjects; i++) {
			terrainObjects[i] = new TerrainObject;
			terrainObjects[i]->setHandle(curHandle);
			objList[curHandle++] = terrainObjects[i];
		}
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (numBuildings > 0) {
		buildings = (BuildingPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(BuildingPtr) * numBuildings);
		if (!buildings)
			Fatal(numBuildings, " GameObjectManager.setNumObjects: cannot malloc buildings ");
		for (i = 0; i < numBuildings; i++) {
			buildings[i] = new Building;
			buildings[i]->setHandle(curHandle);
			objList[curHandle++] = buildings[i];
		}
	}

	if (numTurrets > 0)
	{
		turrets = (TurretPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(TurretPtr) * numTurrets);
		if ( !turrets )
			Fatal(numTurrets, " GameObjectManager.setNumObjects: cannot malloc Turrets ");
			
		for (i = 0; i < numTurrets; i++) 
		{
			turrets[i] = new Turret;
			turrets[i]->setHandle(curHandle);
			objList[curHandle++] = turrets[i];
		}
	}

	if (numGates > 0)
	{
		gates = (GatePtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GatePtr) * numGates);
		if ( !gates )
			Fatal(numGates, " GameObjectManager.setNumObjects: cannot malloc Gates ");
			
		for (i = 0; i < numGates; i++) 
		{
			gates[i] = new Gate;
			gates[i]->setHandle(curHandle);
			objList[curHandle++] = gates[i];
		}
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (numWeapons > 0) {
		weapons = (WeaponBoltPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(WeaponBoltPtr) * numWeapons);
		if (!weapons)
			Fatal(numWeapons, " GameObjectManager.setNumObjects: cannot malloc weapons ");
		for (i = 0; i < numWeapons; i++) {
			weapons[i] = new WeaponBolt;
			weapons[i]->setHandle(curHandle);
			objList[curHandle++] = weapons[i];
		}
	}

	if (numCarnage > 0) {
		carnage = (CarnagePtr*)ObjectTypeManager::objectCache->Malloc(sizeof(CarnagePtr) * numCarnage);
		if (!carnage)
			Fatal(numCarnage, " GameObjectManager.setNumObjects: cannot malloc carnage ");
		for (i = 0; i < numCarnage; i++) {
			carnage[i] = new Carnage;
			carnage[i]->setHandle(curHandle);
			objList[curHandle++] = carnage[i];
		}
	}

	if (numArtillery > 0) {
		artillery = (ArtilleryPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(ArtilleryPtr) * numArtillery);
		if (!artillery)
			Fatal(numArtillery, " GameObjectManager.setNumObjects: cannot malloc artillery ");
		for (i = 0; i < numArtillery; i++) {
			artillery[i] = new Artillery;
			artillery[i]->setHandle(curHandle);
			objList[curHandle++] = artillery[i];
		}
	}

	useMoverLineOfSightTable = true;
	moverLineOfSightTable = (char*)systemHeap->Malloc(maxMovers * maxMovers);
	if (!moverLineOfSightTable)
		Fatal(numGates, " GameObjectManager.setNumObjects: cannot malloc moverLineOfSightTable ");

	GameObject::setInitialize(false);
}

//---------------------------------------------------------------------------

BattleMechPtr GameObjectManager::getMech (long mechIndex) {

	if (!mechs || (mechIndex < 0) || (mechIndex >= numMechs))
		return(NULL);

	return(mechs[mechIndex]);
}

//---------------------------------------------------------------------------

GroundVehiclePtr GameObjectManager::getVehicle (long vehicleIndex) {

	if (!vehicles || (vehicleIndex < 0) || (vehicleIndex >= numVehicles))
		return(NULL);

	return(vehicles[vehicleIndex]);
}

//---------------------------------------------------------------------------

GroundVehiclePtr GameObjectManager::getOpenVehicle (void)
{
	for (long i=0;i<maxVehicles;i++)
	{
		if (!vehicles[i]->getExists())
		{
			if (i > numVehicles)
				numVehicles = i;

			return vehicles[i];
		}
	}

	return NULL;
}

//---------------------------------------------------------------------------

ElementalPtr GameObjectManager::getElemental (long elementalIndex) {

	if (!elementals || (elementalIndex < 0) || (elementalIndex >= numElementals))
		return(NULL);

	return(elementals[elementalIndex]);
}

//---------------------------------------------------------------------------

BattleMechPtr GameObjectManager::newMech (void) {

	if (numMechs < maxMechs) {
		mechs[numMechs]->init(false);
		mechs[numMechs]->setExists(true);
		setPartId(mechs[numMechs], -1, -1, -1);
		mechs[numMechs]->watchID = 0;
		
		//Gotta rebuild the list or the new stuff don't show up!
		// This flag will force rebuild at next collision check
		rebuildCollidableList = true;
		
		return(mechs[numMechs++]);
	}

	return(NULL);
}

//---------------------------------------------------------------------------

GroundVehiclePtr GameObjectManager::newVehicle (void) {

	if (numVehicles < maxVehicles) {
		vehicles[numVehicles]->init(false);
		vehicles[numVehicles]->setExists(true);
		setPartId(vehicles[numVehicles], -1, -1, -1);
		vehicles[numVehicles]->watchID = 0;
		
		//Gotta rebuild the list or the new stuff don't show up!
		// This flag will force rebuild at next collision check
		rebuildCollidableList = true;
		
 		return(vehicles[numVehicles++]);
	}

	return(NULL);
}

//---------------------------------------------------------------------------

void GameObjectManager::setWatchID (GameObjectPtr obj) {

	if (obj->watchID == 0) {
		watchList[nextWatchID] = obj;
		obj->watchID = nextWatchID++;
	}
}

//---------------------------------------------------------------------------

void GameObjectManager::freeMover (MoverPtr mover) {

	bool foundIt = modifyMoverLists(mover, MOVERLIST_DELETE);
	if (foundIt) {
		mover->release();
		mover->setExists(false);
		mover->setFlag(OBJECT_FLAG_REMOVED, true);
		mover->setPartId(0);
		watchList[mover->watchID] = NULL;
		mover->watchID = 0;
	}
}

//---------------------------------------------------------------------------

void GameObjectManager::tradeMover (MoverPtr mover, long newTeamID, long newCommanderID) {

	if (newTeamID > -1) {
		if (mover->teamId != newTeamID)
			mover->salvaged = true;
		if (mover->getObjectClass() == BATTLEMECH) {
			((BattleMech*)mover)->killed = false;
			((BattleMech*)mover)->lost = false;
		}
		mover->tradeRefresh();
	}
	mover->setTeamId(newTeamID, true);
	if (mover->getGroup())
		mover->getGroup()->remove(mover);
	mover->watchID = 0;
	if (newTeamID > -1)
		Team::teams[newTeamID]->addToRoster(mover);
	mover->setCommanderId(newCommanderID);
	modifyMoverLists(mover, MOVERLIST_TRADE);
}

//---------------------------------------------------------------------------

#ifdef USE_ELEMENTALS

ElementalPtr GameObjectManager::addElemental (void) {

	if (numMechs < maxMechs) {
		elementals[numElementals]->setExists(true);
		setPartId(elementals[numElementals], -1, -1, -1);
		return(elementals[numElementals++]);
	}
	return(NULL);
}

#endif

//---------------------------------------------------------------------------

TerrainObjectPtr GameObjectManager::getTerrainObject (long terrainObjectIndex) {

	if (!terrainObjects || (terrainObjectIndex < 0) || (terrainObjectIndex >= numTerrainObjects))
		return(NULL);

	return(terrainObjects[terrainObjectIndex]);
}

//---------------------------------------------------------------------------

BuildingPtr GameObjectManager::getBuilding (long buildingIndex) {

	if (!buildings || (buildingIndex < 0) || (buildingIndex >= numBuildings))
		return(NULL);

	return(buildings[buildingIndex]);
}

//---------------------------------------------------------------------------

TurretPtr GameObjectManager::getTurret (long turretIndex) 
{
	if (!turrets || (turretIndex < 0) || (turretIndex >= numTurrets))
		return(NULL);

	return(turrets[turretIndex]);
}

//---------------------------------------------------------------------------

GatePtr GameObjectManager::getGate (long gateIndex) 
{
	if (!gates || (gateIndex < 0) || (gateIndex >= numGates))
		return(NULL);

	return(gates[gateIndex]);
}

//---------------------------------------------------------------------------

WeaponBoltPtr GameObjectManager::getWeapon (void) 
{
	if (!weapons || (currentWeaponsIndex < 0) || (currentWeaponsIndex >= numWeapons))
		return(NULL);

	currentWeaponsIndex++;
	if (currentWeaponsIndex >= numWeapons)
		currentWeaponsIndex = 0;

	//Make sure this weapon has an opportunity to damage its target
	weapons[currentWeaponsIndex]->finishNow();	

	return(weapons[currentWeaponsIndex]);
}

//---------------------------------------------------------------------------

CarnagePtr GameObjectManager::getCarnage (CarnageEnumType carnageType) {

	if (!carnage || (currentCarnageIndex < 0) || (currentCarnageIndex >= numCarnage))
		return(NULL);

	currentCarnageIndex++;
	if (currentCarnageIndex >= numCarnage)
		currentCarnageIndex = 0;

	carnage[currentCarnageIndex]->finishNow();
	carnage[currentCarnageIndex]->init(carnageType);
	
	return(carnage[currentCarnageIndex]);
}

//---------------------------------------------------------------------------

void GameObjectManager::releaseCarnage (CarnagePtr obj) {

	obj->setExists(false);
	obj->setOwner(NULL);
}

//---------------------------------------------------------------------------

LightPtr GameObjectManager::getLight (void) {

	if (!lights || (currentLightIndex < 0) || (currentLightIndex >= numLights))
		return(NULL);

	if (currentLightIndex >= numLights)
		currentLightIndex = 0;

//	lights[currentLightIndex]->finishNow();
	lights[currentLightIndex]->init(false);
	lights[currentLightIndex]->setExists(true);
	return(lights[currentLightIndex++]);
}

//---------------------------------------------------------------------------

void GameObjectManager::releaseLight (LightPtr obj) {

	obj->setExists(false);
//	obj->setOwner(NULL);
}

//---------------------------------------------------------------------------

ArtilleryPtr GameObjectManager::getArtillery (void) 
{
	if (!artillery || (currentArtilleryIndex < 0) || (currentArtilleryIndex >= numArtillery))
		STOP(("Artillery Strikes are out of range in ObjectManager %d",currentArtilleryIndex));

	currentArtilleryIndex++;
	if (currentArtilleryIndex >= numArtillery)
		currentArtilleryIndex = 0;

	//OK to return NULL now.  Lets Multiplayer know that there are no more strikes available.
	if (artillery[currentArtilleryIndex]->getExists())
		return NULL;
		
	artillery[currentArtilleryIndex]->init(false);
	artillery[currentArtilleryIndex]->setExists(true);
	
	return(artillery[currentArtilleryIndex]);
}

//---------------------------------------------------------------------------
#define	MAX_TERRAIN_OBJECTS		3000

#define OLD_FILE_SIZE	2200

void GameObjectManager::countTerrainObjects (PacketFile* terrainFile, long firstHandle) 
{
	int packet = terrainFile->getCurrentPacket();
	int size = terrainFile->getPacketSize();
	MemoryPtr pBuffer = new unsigned char[size];

#ifdef _DEBUG	
	int bytesRead = 
#endif
		terrainFile->readPacket( packet, pBuffer );
	gosASSERT( bytesRead == size );
	
	File* terrainObjectFile = new File;
	terrainObjectFile->open( (char*)pBuffer, size );
	
	totalObjCount = terrainObjectFile->readInt();

	if (totalObjCount)
	{
		objData = (ObjDataLoader *)systemHeap->Malloc(sizeof(ObjDataLoader) * totalObjCount);
		memset(objData,0,sizeof(ObjDataLoader) * totalObjCount);
	}
	else
	{
		objData = NULL;
	}
	
	ObjDataLoader *data = objData;
	for ( int i = 0; i < totalObjCount; ++i )
	{
		data->objTypeNum = terrainObjectFile->readInt();
		data->vector.x = terrainObjectFile->readFloat();
		data->vector.y = terrainObjectFile->readFloat();
		data->vector.z = terrainObjectFile->readFloat();

		data->rotation = terrainObjectFile->readFloat();		
		data->damage = terrainObjectFile->readInt();

		data->teamId = terrainObjectFile->readInt();
		data->parentId = terrainObjectFile->readInt();
		
		// padding
		terrainObjectFile->readInt();
		terrainObjectFile->readInt();

		// convert to block and vertex
		int tileCol;
		int tileRow;

		land->worldToTile( data->vector, tileRow, tileCol );

		int blockI = tileCol/20;
		int blockJ = tileRow/20;
		
		data->blockNumber = blockJ * land->blocksMapSide + blockI;
		tileCol -= blockI * 20;
		tileRow -= blockJ * 20;
		
		data->vertexNumber = tileRow * 20 + tileRow;
		countObject(data);
		
		data++;
	}

	// fix up handles
	long curHandle = firstHandle;
	for (int i = 0; i < Terrain::numObjBlocks; ++i )
	{
		Terrain::objBlockInfo[i].firstHandle = curHandle;
		curHandle += Terrain::objBlockInfo[i].numObjects;						
	}

	delete terrainObjectFile;
	delete pBuffer;

}

void GameObjectManager::countObject( ObjDataLoader *data)
{
	ObjectTypePtr objType = objTypeManager->get(data->objTypeNum);
	if (!objType)
		objType = objTypeManager->load(data->objTypeNum);
	
	if (!objType)
		return;
			//Fatal(objDataBlock[i].objTypeNum, " GameObjectManager.countTerrainObjects: bad objType ");
		switch (objType->getObjectClass()) 
		{
			case TERRAINOBJECT:
			case TREE:
				Terrain::objBlockInfo[data->blockNumber].numCollidableObjects++;
				numTerrainObjects++;
				break;
			case TURRET:
				Terrain::objBlockInfo[data->blockNumber].numCollidableObjects++;
				numTurrets++;
				break;
			case GATE:
				Terrain::objBlockInfo[data->blockNumber].numCollidableObjects++;
				numGates++;
				break;
			case BUILDING:
			case TREEBUILDING:
			if (((((BuildingTypePtr)objType)->perimeterAlarmRange > 0.0f) &&
				(((BuildingTypePtr)objType)->perimeterAlarmTimer > 0.0f)) ||
				(((BuildingTypePtr)objType)->lookoutTowerRange > 0.0f) ||
				(((BuildingTypePtr)objType)->sensorRange > 0.0f))
				{
					Terrain::objBlockInfo[data->blockNumber].numCollidableObjects++;
				}
				numBuildings++;
				break;
			case BRIDGE:
				numTerrainObjects++;
				break;
			default:
				Fatal(objType->getObjectClass(), " GameObjectManager.countTerrainObjects: bad object type ");
		}
		Terrain::objBlockInfo[data->blockNumber].numObjects++;
}

//---------------------------------------------------------------------------

inline bool isLandMine (long objTypeNum) 
{
	return false;
}

//---------------------------------------------------------------------------

long GameObjectManager::getSpecificObjects (long objClass, long objSubType, GameObjectPtr* objects, long maxObjects) {

	long numValidObjects = 0;
	for (long i = 0; i < getNumObjects(); i++) {
		GameObjectPtr obj = objList[i];
		if (obj && (obj->getObjectClass() == objClass) && (obj->getObjectType()->getSubType() == objSubType)) {
			if (numValidObjects == maxObjects)
				Fatal(objClass, " GameObjectManager.getSpecificObjects: too many ");
			objects[numValidObjects++] = obj;
		}
	}
	return(numValidObjects);
}

//---------------------------------------------------------------------------

void GameObjectManager::loadTerrainObjects (PacketFile* terrainFile,
											volatile float& progress, float progressRange ) 
{
	long curTerrainObjectIndex = 0;
	long curBuildingIndex = 0;
	long curTurretIndex = 0;
	long curGateIndex = 0;

	//------------------------------------------------------------------
	long* handles = new long[2 * Terrain::numObjBlocks];

	for ( int i = 0; i < Terrain::numObjBlocks; ++i )
	{
		handles[2 * i] = Terrain::objBlockInfo[i].firstHandle;
		handles[(2 * i) + 1] = Terrain::objBlockInfo[i].firstHandle + Terrain::objBlockInfo[i].numCollidableObjects;
	}

	ObjDataLoader *data = objData;

	float increment = 0.0f;
	if (totalObjCount)
		increment = progressRange/totalObjCount;
	for (int i = 0; i < totalObjCount; ++i )
	{
		addObject( data, 
			curTerrainObjectIndex, curBuildingIndex, 
			curTurretIndex, curGateIndex, handles[2 * data->blockNumber], 
			handles[(2 * data->blockNumber) + 1] 
			);

		data++;
		progress += increment;
	}

	delete handles;
	handles = NULL;
	
	//Done loading the objects, free the memory holding them!!
	systemHeap->Free(objData);
	objData = NULL;

	//---------------------------------------------------
	// Finally, let's build the control building lists...
	for (int i = 0; i < numTurrets; i++) 
	{
		if ((turrets[i]->parentId != 0xffffffff) && (turrets[i]->parentId != 0)) 
		{
			BuildingPtr controlBuilding = (BuildingPtr)ObjectManager->findByCellPosition((turrets[i]->parentId>>16),(turrets[i]->parentId & 0x0000ffff));
			if (controlBuilding && !controlBuilding->getFlag(OBJECT_FLAG_CONTROLBUILDING)) 
			{
				controlBuilding->setFlag(OBJECT_FLAG_CAPTURABLE, true);
				controlBuilding->setFlag(OBJECT_FLAG_CONTROLBUILDING, true);
				controlBuilding->listID = numTurretControls;
				turretControls[numTurretControls++] = controlBuilding;
			}
			else if (!controlBuilding)
			{
				Stuff::Vector3D worldPos;
				if (land)
					land->cellToWorld((turrets[i]->parentId>>16),(turrets[i]->parentId & 0x0000ffff),worldPos);
				PAUSE(("Turret linked to bldg @ R %d, C %d  X:%f Y:%f No Bldg there!",(turrets[i]->parentId>>16),(turrets[i]->parentId & 0x0000ffff),worldPos.x,worldPos.y));
				turrets[i]->parentId = 0xffffffff;
			}
		}
	}

	for (int i = 0; i < numGates; i++) 
	{
		if ((gates[i]->parentId != 0xffffffff) && (gates[i]->parentId != 0))
		{
			BuildingPtr controlBuilding = (BuildingPtr)ObjectManager->findByCellPosition((gates[i]->parentId>>16),(gates[i]->parentId & 0x0000ffff));
			if (controlBuilding && !controlBuilding->getFlag(OBJECT_FLAG_CONTROLBUILDING)) 
			{
				controlBuilding->setFlag(OBJECT_FLAG_CONTROLBUILDING, true);
				controlBuilding->listID = numGateControls;
				gateControls[numGateControls++] = controlBuilding;
			}
	   		else if (!controlBuilding)
	   		{
	   			PAUSE(("Gate tried to link to building at Row %d, Col %d.  No Bldg there!",(gates[i]->parentId>>16),(gates[i]->parentId & 0x0000ffff)));
				gates[i]->parentId = 0xffffffff;
	   		}
		}
	}

	//-----------------------------------------------------------------------------------
	// Create list of special buildings.  These buildings will be updated at least once
	// every frame regardless of where they are on the terrain and what is visible.
	// This is because perimeter alarms and lookout buildings and ????? must work even
	// if the player is NOT looking at them!!
	for (int i=0;i<numBuildings;i++)
	{
		if (buildings[i]->isSpecialBuilding())
		{
			specialBuildings[numSpecialBuildings++] = buildings[i];
		}
	}
	
 	//--------------------------------------------------------------------
	//Now, lets point every lit building to at least one power generator.
	for (int i=0;i<numBuildings;i++)
	{
		if (buildings[i]->isPowerSource())
		{
			powerGenerators[numPowerGenerators++] = buildings[i];
		}
	}
	
	if (numPowerGenerators)
	{
		for (int i=0;i<numBuildings;i++)
		{
			Stuff::Vector3D maxDist;
			long generatorIndex = 0;
			maxDist.Subtract(buildings[i]->getPosition(),powerGenerators[0]->getPosition());
			float minDistance = maxDist.GetApproximateLength();
			
			for (long j=1;j<numPowerGenerators;j++)
			{
				maxDist.Subtract(buildings[i]->getPosition(),powerGenerators[j]->getPosition()); 
				float newDistance = maxDist.GetApproximateLength(); 
				
				if (newDistance < minDistance)
				{
					generatorIndex = j;
					minDistance = newDistance;
				}
			}	
			
			buildings[i]->setPowerSupply(powerGenerators[generatorIndex]);
		}
		
		for (int i=0;i<numTerrainObjects;i++)
		{
			Stuff::Vector3D maxDist;
			long generatorIndex = 0;
			maxDist.Subtract(terrainObjects[i]->getPosition(),powerGenerators[0]->getPosition());
			float minDistance = maxDist.GetApproximateLength();
			
			for (long j=1;j<numPowerGenerators;j++)
			{
				maxDist.Subtract(terrainObjects[i]->getPosition(),powerGenerators[j]->getPosition()); 
				float newDistance = maxDist.GetApproximateLength(); 
				
				if (newDistance < minDistance)
				{
					generatorIndex = j;
					minDistance = newDistance;
				}
			}	
			
			terrainObjects[i]->setPowerSupply(powerGenerators[generatorIndex]);
		}
	}
}

extern GameObjectFootPrint* tempSpecialAreaFootPrints;
extern long tempNumSpecialAreas;

void GameObjectManager::addObject (ObjDataLoader *objData, long& curTerrainObjectIndex, 
								  long& curBuildingIndex, long& curTurretIndex, long &curGateIndex,
								  long& curCollidableHandle, long& curNonCollidableHandle )
{
	// THIS IS NOT COMPLETELY DONE! MAKE SURE ALL TERRAIN OBJ TYPES
	// ARE ACCOUNTED FOR HERE!

	long objTypeNum = objData->objTypeNum;
	if (!isLandMine(objTypeNum)) {
		ObjectTypePtr objType = getObjectType(objTypeNum);
		GameObjectPtr obj = NULL;
		if (!objType)
			return;
			//Fatal();
		
		Stuff::Vector2DOf<long> numbers;
		numbers.x = objData->vertexNumber;
		numbers.y = objData->blockNumber;

		unsigned char realDmg = objData->damage & 0x0f;
		switch (objType->getObjectClass()) {
			case TERRAINOBJECT:
			case TREE:
				obj = getTerrainObject(curTerrainObjectIndex++);
				if (obj)
				{
					((TerrainObjectPtr)obj)->init(true, objType);
					if (realDmg)
						((TerrainObjectPtr)obj)->setDamage(((TerrainObjectTypePtr)objType)->getDamageLevel());
						
					objList[curCollidableHandle] = obj;
					obj->setHandle(curCollidableHandle++);
					
					obj->setExists(true);
				}
				else
				{
					Fatal(curBuildingIndex," No More Trees ");
				}
				break;
			case TURRET:
				obj = getTurret(curTurretIndex++);
				if (obj)
				{
					((TurretPtr)obj)->init(true, objType);
					if (realDmg)
						obj->setDamage(((TurretTypePtr)objType)->getDamageLevel());

					objList[curCollidableHandle] = obj;
					obj->setHandle(curCollidableHandle++);
					obj->setExists(true);
					obj->setParentId(objData->parentId);
					obj->setTeamId(objData->teamId,true);
				}
				else
				{
					Fatal(curTurretIndex," No More Turrets ");
				}
				break;

			case GATE:
				obj = getGate(curGateIndex++);
				if (obj)
				{
					((GatePtr)obj)->init(true, objType);
					if (realDmg)
						obj->setDamage(((GateTypePtr)objType)->getDamageLvl());

					objList[curCollidableHandle] = obj;
					obj->setHandle(curCollidableHandle++);
					obj->setExists(true);
					obj->setParentId(objData->parentId);
					obj->setTeamId(objData->teamId,true);
				}
				else
				{
					Fatal(curGateIndex," No More Gates ");
				}
				break;

			case BUILDING:
			case TREEBUILDING:
				obj = getBuilding(curBuildingIndex++);
				if (obj)
				{
					((BuildingPtr)obj)->init(true, objType);
					if (realDmg)
						obj->setDamage(((BuildingTypePtr)objType)->getDamageLevel());
					((BuildingPtr)obj)->baseTileId = (objData->damage >> 4);
					if (obj->isSpecialBuilding())
					{
						objList[curCollidableHandle] = obj;
						obj->setHandle(curCollidableHandle++);
					}
					else
					{
						objList[curNonCollidableHandle] = obj;
						obj->setHandle(curNonCollidableHandle++);
					}
					
					obj->setExists(true);
					obj->setParentId(objData->parentId);
					obj->setTeamId(objData->teamId,true);
				}
				else
				{
					Fatal(curBuildingIndex," No More Buildings ");
				}
				break;
			case BRIDGE:
				break;
			default:
				Fatal(objType->getObjectClass(), " GameObjectManager.countTerrainObjects: bad object type ");
		}

		if (obj) {
			obj->setTerrainPosition(objData->vector, numbers);
			int cellRow, cellCol;
			obj->getCellPosition(cellRow, cellCol);
			setPartId(obj, cellRow, cellCol);

			//Just keep the designers from hurting themselves
			if (objType->getObjectClass() != TURRET)
				obj->setRotation(objData->rotation);

			if (obj->isTerrainObject()) {
				Stuff::Vector3D pos = objData->vector;
				((TerrainObjectPtr)obj)->calcCellFootprint(pos);
				if (obj->getObjectClass() == BUILDING) {
					if (objType->getSubType() == BUILDING_SUBTYPE_WALL) {
						for (long i = 0; i < tempNumSpecialAreas; i++)
							if (tempSpecialAreaFootPrints[i].cellPositionRow == cellRow)
								if (tempSpecialAreaFootPrints[i].cellPositionCol == cellCol) {
									((BuildingPtr)obj)->calcSubAreas(tempSpecialAreaFootPrints[i].numCells, tempSpecialAreaFootPrints[i].cells);
									break;
								}
						((BuildingPtr)obj)->closeSubAreas();
						}
					else if (objType->getSubType() == BUILDING_SUBTYPE_LANDBRIDGE) {
						for (long i = 0; i < tempNumSpecialAreas; i++)
							if (tempSpecialAreaFootPrints[i].cellPositionRow == cellRow)
								if (tempSpecialAreaFootPrints[i].cellPositionCol == cellCol) {
									((BuildingPtr)obj)->calcSubAreas(tempSpecialAreaFootPrints[i].numCells, tempSpecialAreaFootPrints[i].cells);
									break;
								}
						((BuildingPtr)obj)->openSubAreas();
					}
					}
				else if (obj->getObjectClass() == GATE) {
					for (long i = 0; i < tempNumSpecialAreas; i++)
						if (tempSpecialAreaFootPrints[i].cellPositionRow == cellRow)
							if (tempSpecialAreaFootPrints[i].cellPositionCol == cellCol) {
								((BuildingPtr)obj)->calcSubAreas(tempSpecialAreaFootPrints[i].numCells, tempSpecialAreaFootPrints[i].cells);
								break;
							}
					((GatePtr)obj)->openSubAreas();
					for (int i = 0; i < ((GatePtr)obj)->numSubAreas0; i++) {
						GlobalMoveMap[0]->setAreaOwnerWID(((GatePtr)obj)->subAreas0[i], ((GatePtr)obj)->getWatchID());
						GlobalMoveMap[1]->setAreaOwnerWID(((GatePtr)obj)->subAreas1[i], ((GatePtr)obj)->getWatchID());
						if (((GatePtr)obj)->status == OBJECT_STATUS_DESTROYED) {
							GlobalMoveMap[0]->setAreaTeamID(((GatePtr)obj)->subAreas0[i], -1);
							GlobalMoveMap[1]->setAreaTeamID(((GatePtr)obj)->subAreas1[i], -1);
							}
						else {
							GlobalMoveMap[0]->setAreaTeamID(((GatePtr)obj)->subAreas0[i], ((GatePtr)obj)->teamId);
							GlobalMoveMap[1]->setAreaTeamID(((GatePtr)obj)->subAreas1[i], -1);
						}
					}
					((GatePtr)obj)->setTeamId(obj->getTeamId(), true);
					short* curCoord = ((GatePtr)obj)->cellsCovered;
					for (int i = 0; i < ((GatePtr)obj)->numCellsCovered; i++) {
						long r = *curCoord++;
						long c = *curCoord++;
						GameMap->setGate(r, c, true);
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

void GameObjectManager::destroy (void) 
{
	//--------------------------------------------------------------
	// Free 'em all up!!
	long i=0;
	if (mechs && maxMechs > 0) 
	{
		for (i = 0; i < maxMechs; i++) 
		{
			delete mechs[i];
			mechs[i] = NULL;
		}
	}
	mechs = NULL;

	//--------------------------------------------------------------
	if (vehicles && maxVehicles > 0) 
	{
		for (i = 0; i < maxVehicles; i++) 
		{
			delete vehicles[i];
			vehicles[i] = NULL;
		}
	}
	vehicles = NULL;

	//--------------------------------------------------------------
	if (terrainObjects && numTerrainObjects > 0) 
	{
		for (i = 0; i < numTerrainObjects; i++) 
		{
			delete terrainObjects[i];
			terrainObjects[i] = NULL;
		}
	}
	terrainObjects = NULL;

	//--------------------------------------------------------------
	if (buildings && numBuildings > 0) 
	{
		for (i = 0; i < numBuildings; i++) 
		{
			delete buildings[i];
			buildings[i] = NULL;
		}
	}
	buildings = NULL;

	//--------------------------------------------------------------
	if (turrets && numTurrets > 0)
	{
		for (i = 0; i < numTurrets; i++) 
		{
			delete turrets[i];
			turrets[i] = NULL;
		}
	}
	turrets = NULL;

	//--------------------------------------------------------------
	if (gates && numGates > 0)
	{
		for (i = 0; i < numGates; i++) 
		{
			delete gates[i];
			gates[i] = NULL;
		}
	}
	gates = NULL;

	//--------------------------------------------------------------
	if (weapons && numWeapons > 0) 
	{
		for (i = 0; i < numWeapons; i++) 
		{
			delete weapons[i];
			weapons[i] = NULL;
		}
	}
	weapons = NULL;

	//--------------------------------------------------------------
	if (carnage && numCarnage > 0) 
	{
		for (i = 0; i < numCarnage; i++) 
		{
			delete carnage[i];
			carnage[i] = NULL;
		}
	}
	carnage = NULL;

	//--------------------------------------------------------------
	if (lights && numLights > 0) 
	{
		for (i = 0; i < numLights; i++) 
		{
			delete lights[i];
			lights[i] = NULL;
		}
	}
	lights = NULL;

	//--------------------------------------------------------------
	if (artillery && numArtillery > 0) 
	{
		for (i = 0; i < numArtillery; i++) 
		{
			delete artillery[i];
			artillery[i] = NULL;
		}
	}
	artillery = NULL;

	//--------------------------------------------------------------
	if (objTypeManager) 
	{
		delete objTypeManager;
		objTypeManager = NULL;
	}

	delete collisionSystem;
	collisionSystem = NULL;

	systemHeap->Free(moverLineOfSightTable);
	moverLineOfSightTable = NULL;
}

//---------------------------------------------------------------------------

void GameObjectManager::render (bool terrain, bool movers, bool other) {

	//-----------------------------------------------------
	//Set render states as few times as possible.

	if (drawOldWay)
	{
		//--------------------------------
		//Set States for Software Renderer
		if (Environment.Renderer == 3)
		{
			gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
	
			gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
			gos_SetRenderState( gos_State_MonoEnable, 1);
			gos_SetRenderState( gos_State_Perspective, 0);
			gos_SetRenderState( gos_State_Clipping, 1);
			gos_SetRenderState( gos_State_Specular, 0);
			gos_SetRenderState( gos_State_Dither, 0);
			gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulate);
			gos_SetRenderState( gos_State_Filter, gos_FilterNone);
			gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
			gos_SetRenderState( gos_State_ZCompare, 1);
			gos_SetRenderState(	gos_State_ZWrite, 1);
		}
		//--------------------------------
		//Set States for Hardware Renderer	
		else
		{
			gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
			gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
			gos_SetRenderState( gos_State_MonoEnable, 0);
			gos_SetRenderState( gos_State_Perspective, 1);
			gos_SetRenderState( gos_State_Clipping, 1);
			gos_SetRenderState( gos_State_Specular, 1);
			gos_SetRenderState( gos_State_Dither, 1);
			gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulate);
			gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
			gos_SetRenderState( gos_State_ZCompare, 1);
			gos_SetRenderState(	gos_State_ZWrite, 1);
		}
	}

	if (terrain && renderObjects) 
	{
		for (long terrainBlock = 0; terrainBlock < Terrain::numObjBlocks; terrainBlock++) 
		{
			if (Terrain::objBlockInfo[terrainBlock].active) 
			{
				long numObjs = Terrain::objBlockInfo[terrainBlock].numObjects;
				long objIndex = Terrain::objBlockInfo[terrainBlock].firstHandle;
				for (long terrainObj = 0; terrainObj < numObjs; terrainObj++, objIndex++) 
				{
					if (objList[objIndex] &&
						Terrain::objVertexActive[objList[objIndex]->getVertexNum()])
					{
						objList[objIndex]->render();
						if (MaxObjectsDrawn) 
						{
							//-----------------------------------------
							// No more element groups, so stop drawing.
							return;
						}
					}
				}
			}
		}
	}

	if (movers) {
		if (mechs && (numMechs < maxMechs)) 
		{
			for (long i = 0; i < numMechs; i++)
			{
				if (mechs[i] && mechs[i]->getExists())
					mechs[i]->render();
			}
		}

		if (vehicles) {
			for (long i = 0; i < maxVehicles; i++)
				if (vehicles[i] && vehicles[i]->getExists())
					vehicles[i]->render();
		}

#ifdef USE_ELEMENTALS
		if (elementals) {
			for (long i = 0; i < numElementals; i++)
				if (elementals[i] && elementals[i]->getExists())
					elementals[i]->render();
		}
#endif
	}

	if (other) {
		//----------------------------------------
		// All other objects should be rendered...
		if (weapons) {
			for (long i = 0; i < numWeapons; i++) {
				if (weapons[i] && weapons[i]->getExists())
					weapons[i]->render();
			}
		}

		if (carnage) {
			for (long i = 0; i < numCarnage; i++) {
				if (carnage[i] && carnage[i]->getExists())
				{
					carnage[i]->render();
				}
			}
		}

		if (lights) {
			for (long i = 0; i < numLights; i++) {
				if (lights[i] && lights[i]->getExists())
					lights[i]->render();
			}
		}

		if (artillery) {
			for (long i = 0; i < numArtillery; i++) {
				if (artillery[i] && artillery[i]->getExists())
					artillery[i]->render();
			}
		}
	}
	gos_SetRenderState( gos_State_Fog, 0);
}

//---------------------------------------------------------------------------

void GameObjectManager::renderShadows (bool terrain, bool movers, bool other) {

	//-----------------------------------------------------
	//Set render states as few times as possible.

	//--------------------------------
	//Set States for Software Renderer
	if (Environment.Renderer == 3)
	{
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
		gos_SetRenderState( gos_State_MonoEnable, 1);
		gos_SetRenderState( gos_State_Perspective, 0);
		gos_SetRenderState( gos_State_Clipping, 1);
		gos_SetRenderState( gos_State_AlphaTest, 0);
		gos_SetRenderState( gos_State_Specular, 0);
		gos_SetRenderState( gos_State_Dither, 0);
		gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendDecal);
		gos_SetRenderState( gos_State_Filter, gos_FilterNone);
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
		gos_SetRenderState( gos_State_ZCompare, 0);
		gos_SetRenderState(	gos_State_ZWrite, 0);
	}
	//--------------------------------
	//Set States for Hardware Renderer	
	else
	{
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
		gos_SetRenderState( gos_State_MonoEnable, 1);
		gos_SetRenderState( gos_State_Perspective, 0);
		gos_SetRenderState( gos_State_Clipping, 1);
		gos_SetRenderState( gos_State_AlphaTest, 1);
		gos_SetRenderState( gos_State_Specular, 0);
		gos_SetRenderState( gos_State_Dither, 1);
		gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulate);
		gos_SetRenderState( gos_State_Filter, gos_FilterNone);
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
		gos_SetRenderState( gos_State_ZCompare, 2);
		gos_SetRenderState(	gos_State_ZWrite, 1);
	}

	if (terrain && renderObjects) 
	{
		for (long terrainBlock = 0; terrainBlock < Terrain::numObjBlocks; terrainBlock++) 
		{
			if (Terrain::objBlockInfo[terrainBlock].active) 
			{
				long numObjs = Terrain::objBlockInfo[terrainBlock].numObjects;
				long objIndex = Terrain::objBlockInfo[terrainBlock].firstHandle;
				for (long terrainObj = 0; terrainObj < numObjs; terrainObj++, objIndex++) 
				{
					if (objList[objIndex] &&
						Terrain::objVertexActive[objList[objIndex]->getVertexNum()]) 
					{
						objList[objIndex]->renderShadows();
						if (MaxObjectsDrawn) {
							//-----------------------------------------
							// No more element groups, so stop drawing.
							return;
						}
					}
				}
			}
		}
	}

	if (movers) {
		if (mechs) {
			for (long i = 0; i < numMechs; i++)
				if (mechs[i] && mechs[i]->getExists())
					mechs[i]->renderShadows();
		}

		if (vehicles) {
			for (long i = 0; i < numVehicles; i++)
				if (vehicles[i] && vehicles[i]->getExists())
					vehicles[i]->renderShadows();
		}
#ifdef USE_ELEMENTALS
		if (elementals) {
			for (long i = 0; i < numElementals; i++)
				if (elementals[i] && elementals[i]->getExists())
					elementals[i]->renderShadows();
		}
#endif
	}

	if (other) {
		if (carnage) {
			for (long i = 0; i < numCarnage; i++) {
				if (carnage[i] && carnage[i]->getExists())
					carnage[i]->renderShadows();
			}
		}
	}

	gos_SetRenderState( gos_State_Fog, 0);
}

//---------------------------------------------------------------------------
#ifdef LAB_ONLY
__int64 MCTimeTerrainObjectsUpdate 	= 0;
__int64 MCTimeMechsUpdate 			= 0;
__int64 MCTimeVehiclesUpdate		= 0;
__int64 MCTimeTurretsUpdate			= 0;

__int64 MCTimeTerrainObjectsTL 		= 0;
__int64 MCTimeMechsTL 				= 0;
__int64 MCTimeVehiclesTL			= 0;
__int64 MCTimeTurretsTL				= 0;

__int64 MCTimeAllElseUpdate			= 0;
__int64 MCTimeCaptureListUpdate		= 0;
extern __int64 MCTimeTransformandLight;
extern __int64 MCTimeAnimationandMatrix;
extern __int64 MCTimePerShapeTransform;

unsigned long bldgCount = 0;
#endif

void GameObjectManager::update (bool terrain, bool movers, bool other) 
{
	//----------------------------
	// Now, update game objects...
	#ifdef LAB_ONLY
	__int64 x;
	x=GetCycles();
	#endif
	
	updateCaptureList();
	
	#ifdef LAB_ONLY
	x=GetCycles()-x;
	MCTimeCaptureListUpdate = x;
	
	x=GetCycles(); 
	#endif
	
 	if (terrain && renderObjects) 
	{
		#ifdef LAB_ONLY
		bldgCount = 0;
		#endif
		
		//First Update all of the Special Buildings.
		// They will mark themselves updated and not re-update below.
		for (long spBuilding = 0; spBuilding < numSpecialBuildings;spBuilding++)
		{
			if (specialBuildings[spBuilding] && specialBuildings[spBuilding]->getExists()) 
			{
			#ifdef LAB_ONLY
				bldgCount++;
				MCTimeAnimationandMatrix =
				MCTimePerShapeTransform =
				MCTimeTransformandLight = 0;
			#endif
				if (!specialBuildings[spBuilding]->update()) 
				{
					//-----------------------------------------
					// Update failed, so it no longer exists...
					specialBuildings[spBuilding]->setExists(false);
				}
			#ifdef LAB_ONLY
				MCTimeTerrainObjectsTL += MCTimeTransformandLight;
			#endif
			}
		}

		//Then update all of the gates.
		// They too will mark themselves and not re-update.
		// MUST update every frame or they don't open!!
		for (long nGates = 0;nGates < numGates;nGates++)
		{
			if (gates[nGates] && gates[nGates]->getExists()) 
			{
			#ifdef LAB_ONLY
				bldgCount++;
				MCTimeAnimationandMatrix =
				MCTimePerShapeTransform =
				MCTimeTransformandLight = 0;
			#endif
				if (!gates[nGates]->update()) 
				{
					//-----------------------------------------
					// Update failed, so it no longer exists...
					gates[nGates]->setExists(false);
				}
			#ifdef LAB_ONLY
				MCTimeTerrainObjectsTL += MCTimeTransformandLight;
			#endif
			}
		}
		
		for (long terrainBlock = 0; terrainBlock < Terrain::numObjBlocks; terrainBlock++) 
		{
			if (Terrain::objBlockInfo[terrainBlock].active || (turn < 3)) 
			{
				long numObjs = Terrain::objBlockInfo[terrainBlock].numObjects;
				long objIndex = Terrain::objBlockInfo[terrainBlock].firstHandle;
				for (long terrainObj = 0; terrainObj < numObjs; terrainObj++,objIndex++) 
				{
					if (objList[objIndex] && 
						(Terrain::objVertexActive[objList[objIndex]->getVertexNum()] || (turn < 3)) && 
						objList[objIndex]->getExists())
					{
			#ifdef LAB_ONLY
				bldgCount++;
				
				MCTimeAnimationandMatrix =
				MCTimePerShapeTransform =
				MCTimeTransformandLight = 0;
			#endif
						
						if (!objList[objIndex]->update()) 
						{
							//-----------------------------------------
							// Update failed, so it no longer exists...
							objList[objIndex]->setExists(false);
						}
						
			#ifdef LAB_ONLY
				MCTimeTerrainObjectsTL += MCTimeTransformandLight;
			#endif
					}
				}
			}
		}
	}

	#ifdef LAB_ONLY
	x=GetCycles()-x;
	MCTimeTerrainObjectsUpdate = x;
	#endif
	
 	if (movers) {
		static MoverPtr removeList[MAX_MOVERS];
		long numRemoved = 0;
		
		#ifdef LAB_ONLY
		x=GetCycles();
		#endif
		
		if (mechs)
		{
			for (long i = 0; i < numMechs; i++) 
			{
				MoverPtr mover = mechs[i];
				if (mover && mover->getExists()) 
				{
			#ifdef LAB_ONLY
				MCTimeAnimationandMatrix =
				MCTimePerShapeTransform =
				MCTimeTransformandLight = 0;
			#endif
					if (!mover->update())
						mover->setExists(false);
			#ifdef LAB_ONLY
				MCTimeMechsTL += MCTimeTransformandLight;
			#endif
					if (mover->getFlag(OBJECT_FLAG_REMOVED))
						removeList[numRemoved++] = mover;
				}
			}
		}

		#ifdef LAB_ONLY
		x=GetCycles()-x;
		MCTimeMechsUpdate = x;
		
		x=GetCycles(); 
		#endif
	
		if (vehicles)
		{
			for (long i = 0; i < maxVehicles; i++) 
			{
				MoverPtr mover = vehicles[i];
				if (mover && mover->getExists()) 
				{
			#ifdef LAB_ONLY
				MCTimeAnimationandMatrix =
				MCTimePerShapeTransform =
				MCTimeTransformandLight = 0;
			#endif
					if (!mover->update())
						mover->setExists(false);
			#ifdef LAB_ONLY
				MCTimeVehiclesTL += MCTimeTransformandLight;
			#endif
					if (mover->getFlag(OBJECT_FLAG_REMOVED))
						removeList[numRemoved++] = mover;
				}
			}
		}
			
		#ifdef LAB_ONLY
		x=GetCycles()-x;
		MCTimeVehiclesUpdate = x;
		#endif
		
		for (long i = 0; i < numRemoved; i++)
			mission->removeMover(removeList[i]);
	}

	if (other) {
		//---------------------------------------
		// All other objects should be updated...
		#ifdef LAB_ONLY
		__int64 x;
		x=GetCycles();
		#endif
	
		if (turrets) 
		{
			for (long i=0;i<numTurrets;i++) 
			{
				if (turrets[i] && turrets[i]->getExists()) 
				{
			#ifdef LAB_ONLY
				MCTimeAnimationandMatrix =
				MCTimePerShapeTransform =
				MCTimeTransformandLight = 0;
			#endif
					if (!turrets[i]->update())
						turrets[i]->setExists(false);
			#ifdef LAB_ONLY
				MCTimeTurretsTL += MCTimeTransformandLight;
			#endif
				}
			}
		}
		
		#ifdef LAB_ONLY
		x=GetCycles()-x;
		MCTimeTurretsUpdate = x;
		
		x=GetCycles(); 
		#endif
		
		if (weapons) {
			for (long i=0;i<numWeapons;i++) {
				if (weapons[i] && weapons[i]->getExists()) {
					if (!weapons[i]->update())
						weapons[i]->setExists(false);
				}
			}
		}

		if (carnage) {
			for (long i = 0; i < numCarnage; i++) {
				if (carnage[i] && carnage[i]->getExists()) {
					if (!carnage[i]->update())
						carnage[i]->setExists(false);
				}
			}
		}

		if (lights) {
			for (long i = 0; i < numLights; i++) {
				if (lights[i] && lights[i]->getExists()) {
					if (!lights[i]->update())
						lights[i]->setExists(false);
				}
			}
		}

		if (artillery) {
			for (long i = 0; i < numArtillery; i++) {
				if (artillery[i] && artillery[i]->getExists()) {
					if (!artillery[i]->update())
						artillery[i]->setExists(false);
				}
			}
		}
		
		#ifdef LAB_ONLY
		x=GetCycles()-x;
		MCTimeAllElseUpdate = x;
		#endif
	}
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::get (GameObjectHandle handle) {

	if ((handle < 1) || (handle > getMaxObjects()))
		return(NULL);

	return(objList[handle]);
}

//---------------------------------------------------------------------------

long GameObjectManager::buildMoverLists (void) {

	numMovers = 0;
	numGoodMovers = 0;
	numBadMovers = 0;

	for (long i = 0; i < numMechs; i++) {
		MoverPtr mover = dynamic_cast<MoverPtr>(mechs[i]);
		if (!mover->getTeam())
			continue;
		moverList[numMovers++] = mover;
		if (mover->getTeam()->isFriendly(Team::home))
			goodMoverList[numGoodMovers++] = mover;
		else if (mover->getTeam()->isEnemy(Team::home))
			badMoverList[numBadMovers++] = mover;
	}

	for (int i = 0; i < numVehicles; i++) {
		MoverPtr mover = dynamic_cast<MoverPtr>(vehicles[i]);
		if (!mover->getTeam())
			continue;
		moverList[numMovers++] = mover;
		if (mover->getTeam()->isFriendly(Team::home))
			goodMoverList[numGoodMovers++] = mover;
		else if (mover->getTeam()->isEnemy(Team::home))
			badMoverList[numBadMovers++] = mover;
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

bool GameObjectManager::modifyMoverLists (MoverPtr mover, long action) {

	switch (action) {
		case MOVERLIST_DELETE: {
			bool foundIt = false;
			if (mover->getObjectClass() == BATTLEMECH) {
				long i = 0;
				for (i = 0; i < numMechs; i++) {
					if (mechs[i] == mover)
						break;
				}
				if (i < numMechs) {
					foundIt = true;
					BattleMechPtr mech = mechs[i];
					memmove(&mechs[i], &mechs[i + 1], (maxMechs - i - 1) * sizeof(BattleMechPtr));
					mechs[maxMechs - 1] = mech;
					numMechs--;
				}
				}
			else if (mover->getObjectClass() == GROUNDVEHICLE) {
				long i = 0;
				for (i = 0; i < numVehicles; i++) {
					if (vehicles[i] == mover)
						break;
				}
				if (i < numVehicles) {
					foundIt = true;
					GroundVehiclePtr vehicle = vehicles[i];
					memmove(&vehicles[i], &vehicles[i + 1], (maxVehicles - i - 1) * sizeof(GroundVehiclePtr));
					vehicles[maxVehicles - 1] = vehicle;
					numVehicles--;
				}
			}

			for (long i = 0; i < numMovers; i++)
				if (moverList[i] == mover) {
					moverList[i] = moverList[--numMovers];
					break;
				}

			if (foundIt && mover->getTeam()) {
				if (mover->getTeam()->isFriendly(Team::home)) {
					long i = 0;
					for (i = 0; i < numGoodMovers; i++)
						if (mover == goodMoverList[i])
							break;
					if (i < numGoodMovers)
						goodMoverList[i] = goodMoverList[--numGoodMovers];
					}
				else if (mover->getTeam()->isEnemy(Team::home)) {
					long i = 0;
					for (i = 0; i < numBadMovers; i++)
						if (mover == badMoverList[i])
							break;
					if (i < numBadMovers)
						badMoverList[i] = badMoverList[--numBadMovers];
				}
			}
			return(foundIt);
		}
		case MOVERLIST_ADD:
			moverList[numMovers++] = mover;
			if (mover->getTeam()) {
				if (mover->getTeam()->isFriendly(Team::home))
					goodMoverList[numGoodMovers++] = mover;
				else if (mover->getTeam()->isEnemy(Team::home))
					badMoverList[numBadMovers++] = mover;
			}
			break;
		case MOVERLIST_TRADE:
			//-----------------------------------------------
			// First, remove it from whatever list it's on...
			long i = 0;
			for (i = 0; i < numGoodMovers; i++)
				if (mover == goodMoverList[i])
					break;
			if (i < numGoodMovers)
				goodMoverList[i] = goodMoverList[--numGoodMovers];
			else {
				long i = 0;
				for (i = 0; i < numBadMovers; i++)
					if (mover == badMoverList[i])
						break;
				if (i < numBadMovers)
					badMoverList[i] = badMoverList[--numBadMovers];
			}
			//----------------------------------
			// Now, add it to the proper list...
			if (mover->getTeam()) {
				if (mover->getTeam()->isFriendly(Team::home))
					goodMoverList[numGoodMovers++] = mover;
				else if (mover->getTeam()->isEnemy(Team::home))
					badMoverList[numBadMovers++] = mover;
			}
	}
	return(true);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findObject (Stuff::Vector3D position) {

	float closestDistance = 10000.0;
	GameObjectPtr closestObj = NULL;
	long numObjects = getMaxObjects();
	for (long objIndex = 1; objIndex <= numObjects; objIndex++) {
		GameObjectPtr obj = objList[objIndex];
		Assert(obj != NULL, objIndex, " GameObjectManager.findObject: NULL obj ");
		if (obj->getExists() && !obj->inTransport()) {
			float distanceFromObject = obj->distanceFrom(position);
			if (distanceFromObject < closestDistance)
				if (distanceFromObject < obj->getExtentRadius()) {
					closestObj = obj;
					closestDistance = distanceFromObject;
				}
		}
	}

	return(closestObj);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findObjectByTypeHandle (long typeHandle) {

	//-------------------------------------------------
	// This function was called findObjectId() in MC...

	long numObjects = getMaxObjects();
	for (long objIndex = 1; objIndex <= numObjects; objIndex++) {
		GameObjectPtr obj = objList[objIndex];
		if (obj->getExists() && (obj->getTypeHandle() == typeHandle))
			return(obj);
	}
	return(NULL);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findByPartId (long partId) {

	if (partId == 0)
		return(NULL);

	long numObjects = getMaxObjects();
	for (long objIndex = 1; objIndex <= numObjects; objIndex++) 
	{
		GameObjectPtr obj = objList[objIndex];
		if (obj && obj->getExists() && (obj->getPartId() == partId))
			return(obj);
	}
	return(NULL);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findByBlockVertex (long blockNum, long vertex) {

	//----------------------------------------
	// SLOW compared to using object handle...
	long partId = calcPartId(TERRAINOBJECT, blockNum, vertex);
	return(findByPartId(partId));
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findByCellPosition (long row, long col) 
{
	//-------------------------------------------------------------------------------------
	// Must implement for Linkage code.  10/20/99 -fs
	// PLEASE DO NOT CALL EVERY FRAME. THIS ONE WILL BE SLOWER THEN CHRISTMAS!!!!!!!!!!!!
	// Store the pointer if at all possible.  YOU HAVE BEEN WARNED!
	long numObjects = getMaxObjects();
	for (long objIndex = 1; objIndex <= numObjects; objIndex++)
	{
		GameObjectPtr obj = objList[objIndex];
		if (obj->getExists())
		{
			int cellR, cellC;
			objList[objIndex]->getCellPosition(cellR,cellC);

			if ((cellR == row) && (cellC == col))
				return objList[objIndex];
		}
	}

	return(NULL);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findByUnitInfo (long commander, long group, long mate) {

	//----------------------------------------
	// SLOW compared to using object handle...
	long partId = calcPartId(MOVER, commander, group, mate);
	return(findByPartId(partId));
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findObjectByMouse (long mouseX,
													long mouseY,
													GameObjectPtr* searchList,
													long listSize,
													bool skipDisabled) {

	if (!searchList)
		Fatal(0, " GameObjectManager.findObjectByMouse: NULL searchList ");

	for (long objIndex = 0; objIndex < listSize; objIndex++) 
	{
		if (searchList[objIndex] && searchList[objIndex]->getExists()) 
		{
			GameObjectPtr obj = searchList[objIndex];
			Assert(obj != NULL, objIndex, " GameObjectManager.findObjectByMouse: NULL obj ");
			AppearancePtr objAppearance = obj->getAppearance();
			if (objAppearance && objAppearance->canBeSeen()) 
			{
				if (obj->getWindowsVisible() == (turn - VISIBLE_THRESHOLD))
				{
					//-----------------------------------------------------
					float tlx = objAppearance->upperLeft.x;
					float tly = objAppearance->upperLeft.y;
					float brx = objAppearance->lowerRight.x;
					float bry = objAppearance->lowerRight.y;
						
					if ((mouseX >= tlx) && 
						(mouseX <= brx) &&
						(mouseY >= tly) &&
						(mouseY <= bry)) 
					{
						//---------------------------
						// We're on it, so save it...
						if (!obj->isMover() || (obj->isMover() && obj->isOnGUI() && Terrain::IsGameSelectTerrainPosition(obj->getPosition())))
						{
							if (skipDisabled) 
							{
								if (!obj->isDisabled() && 
									(obj->getObjectClass() != TREE) && 
									(obj->getDamageLevel() != 36000000) && 				//We are a rock clump
									objAppearance->PerPolySelect(mouseX, mouseY))
									return(obj);
							}
							else
							{
								//Do not target trees or artillery strikes!!
								if ((obj->getObjectClass() != TREE) && 
									(obj->getObjectClass() != ARTILLERY) &&                                  									(obj->getDamageLevel() != 36000000) && 				//We are a rock clump
									(obj->getDamageLevel() != 36000000) && 				//We are a rock clump
									objAppearance->PerPolySelect(mouseX, mouseY))
									return(obj);
							}
						}
					}
				}
			}
		}
	}
	
	return(NULL);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findMoverByMouse (long mouseX,
												   long mouseY,
												   long commanderId,
												   bool skipDisabled) {

	GameObjectPtr* searchList = NULL;
	long numMovers = getMaxMovers();
	if (objList)
		searchList = &objList[1];

	if (!searchList)
		return(NULL);

	if (commanderId == -1)
		for (long objIndex = 0; objIndex < numMovers; objIndex++) 
		{
			if (searchList[objIndex] && searchList[objIndex]->getExists()) 
			{
				GameObjectPtr obj = searchList[objIndex];
				Assert(obj != NULL, objIndex, " GameObjectManager.findObjectByMouse: NULL obj ");
				AppearancePtr objAppearance = obj->getAppearance();
				if (objAppearance && objAppearance->canBeSeen()) 
				{
					if (obj->getWindowsVisible() == (turn - VISIBLE_THRESHOLD)) 
					{
						//-----------------------------------------------------
						float tlx = objAppearance->upperLeft.x;
						float tly = objAppearance->upperLeft.y;
						float brx = objAppearance->lowerRight.x;
						float bry = objAppearance->lowerRight.y;
							
						if ((mouseX >= tlx) && 
							(mouseX <= brx) &&
							(mouseY >= tly) &&
							(mouseY <= bry)) 
						{
							//---------------------------
							// We're on it, so save it...
							// Movers are NOT per poly!!
							if (!obj->isMover() || (obj->isMover() && obj->isOnGUI() && Terrain::IsGameSelectTerrainPosition(obj->getPosition())))
							{
								if (skipDisabled) 
								{
									if (!obj->isDisabled())
										return(obj);
								}
								else
									return(obj);
							}
						}
					}
				}
			}
		}
	else
		for (long objIndex = 0; objIndex < numMovers; objIndex++) 
		{
			if (searchList[objIndex] && searchList[objIndex]->getExists()) 
			{
				GameObjectPtr obj = searchList[objIndex];
				Assert(obj != NULL, objIndex, " GameObjectManager.findObjectByMouse: NULL obj ");
				if (obj->getCommanderId() == commanderId)
					continue;
				AppearancePtr objAppearance = obj->getAppearance();
				if (objAppearance && objAppearance->canBeSeen()) 
				{
					if (obj->getWindowsVisible() == (turn - VISIBLE_THRESHOLD)) 
					{
						//-----------------------------------------------------
						float tlx = objAppearance->upperLeft.x;
						float tly = objAppearance->upperLeft.y;
						float brx = objAppearance->lowerRight.x;
						float bry = objAppearance->lowerRight.y;
							
						if ((mouseX >= tlx) && 
							(mouseX <= brx) &&
							(mouseY >= tly) &&
							(mouseY <= bry)) 
						{
							if (!obj->isMover() || (obj->isMover() && obj->isOnGUI() && Terrain::IsGameSelectTerrainPosition(obj->getPosition())))
							{
								//---------------------------
								// We're on it, so save it...
								if (skipDisabled) 
								{
									if (!obj->isDisabled())
										return(obj);
								}
								else
									return(obj);
							}
						}
					}
				}
			}
		}

	return(NULL);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findTerrainObjectByMouse (long mouseX,
														   long mouseY,
														   bool skipDisabled) 
{
	for (long terrainBlock = 0; terrainBlock < Terrain::numObjBlocks; terrainBlock++) 
	{
		if (Terrain::objBlockInfo[terrainBlock].active) 
		{
			long numObjs = Terrain::objBlockInfo[terrainBlock].numObjects;
			long objIndex = Terrain::objBlockInfo[terrainBlock].firstHandle;
			GameObjectPtr obj = findObjectByMouse(mouseX, mouseY, &objList[objIndex], numObjs, skipDisabled);
			if (obj)
				return(obj);
		}
	}

	return(NULL);
}

//---------------------------------------------------------------------------

GameObjectPtr GameObjectManager::findObjectByMouse (long mouseX, long mouseY) {

	GameObjectPtr obj = NULL;

	//-------------------------------------------------------------
	// This function will search movers first. If we find an object
	// near the mouse, we're done...
	// Mitch wants to find live ones before dead ones
	long homeCommanderId = Commander::home->getId();
	obj = findMoverByMouse(mouseX, mouseY, homeCommanderId, true);
	if (obj)
		return(obj);
	obj = findMoverByMouse(mouseX, mouseY, homeCommanderId, false);
	if (obj)
		return(obj);
	
	obj = findMoverByMouse(mouseX, mouseY, -1, true);
	if (obj)
		return(obj);
	obj = findMoverByMouse(mouseX, mouseY, -1, false);
	if (obj)
		return(obj);
	

	//-------------------------------------------------------------------------
	// If we still haven't found anything, check everything (including terrain)
	// with disregard to disabled status...
	// Skip disabled should be on.  I have Spoken! -fs
	// Frank -- I need to find the nav markers so I can get their little highlight text.... HKG
	obj = findObjectByMouse(mouseX, mouseY, &objList[1], getMaxObjects(),false);
	if (obj)
		return(obj);
		
	return(findTerrainObjectByMouse(mouseX, mouseY,true));
}

//---------------------------------------------------------------------------
bool GameObjectManager::moverInRect(long index, Stuff::Vector3D &dStart, Stuff::Vector3D &dEnd)
{
	//------------------------------------------------------------------------------
	// This function checks the mover passed in to see if it
	// is within the magic rectangle.  It assumes we are looking for our alignment.
	// This is because we are drag selecting in the GUI!
	if ((index < 0) || (index >= getMaxMovers()))
		return(false);
		
	MoverPtr checkMover = getMover(index);
	if (checkMover && checkMover->getExists() && (checkMover->getTeam() == Team::home))
	{
		AppearancePtr objAppearance = checkMover->getAppearance();
		if (objAppearance /*&& objAppearance->canBeSeen() */) 
		{
//			if (checkMover->getWindowsVisible() > (turn - VISIBLE_THRESHOLD)) 
			{
				//-----------------------------------------------------
				// Note that this effectively tests inTransport, since
				// windowsVisible only gets set when not inTransport...
				//objAppearance->recalcBounds();	//Shouldn't need to do this.  They should already be correct!
				long left 	= fmin(dStart.x, dEnd.x);
				long right 	= fmax(dStart.x, dEnd.x);
				long top 	= fmin(dStart.y, dEnd.y);
				long bottom = fmax(dStart.y, dEnd.y);
				
				if ((left <= objAppearance->getScreenPos().x) && 
					(right >= objAppearance->getScreenPos().x) &&
					(top <= objAppearance->getScreenPos().y) &&
					(bottom >= objAppearance->getScreenPos().y))
				{
					return(true);
				}
			}
		}
	}
	
	return(false);
}	

//---------------------------------------------------------------------------

ObjectTypePtr GameObjectManager::loadObjectType (ObjectTypeNumber typeHandle) {

	return(objTypeManager->load(typeHandle));
}

//---------------------------------------------------------------------------

ObjectTypePtr GameObjectManager::getObjectType (ObjectTypeNumber typeHandle) {

	return(objTypeManager->get(typeHandle));
}

//---------------------------------------------------------------------------

void GameObjectManager::removeObjectType (ObjectTypeNumber typeHandle) {

	objTypeManager->remove(typeHandle);
}

//---------------------------------------------------------------------------

GameObjectHandle GameObjectManager::getHandle (GameObjectPtr obj) {

	for (long i = 1; i <= getMaxObjects(); i++)
		if (objList[i] == obj)
			return(i);
	return(0);
}

//---------------------------------------------------------------------------

long GameObjectManager::calcPartId (long objectClass, long param1, long param2, long param3) {

	//-------------------------------------------------------------------
	// This should be the only function used to calc partIds for objects.
	// This will allow easy modification of the formulas without making
	// changes everywhere...
	long partId = 0;
	switch (objectClass) {
		case MOVER:
		case BATTLEMECH:
		case GROUNDVEHICLE:
		case ELEMENTAL:
			if (param1 == -1) {
				if (nextReinforcementPartId == (MAX_REINFORCEMENT_PART_ID + 1))
					return(0);
				partId = nextReinforcementPartId++;
				}
			else
				partId = MIN_MOVER_PART_ID +
						 param1 * MAX_MOVERGROUPS * MAX_MOVERGROUP_COUNT_START +
						 param2 * MAX_MOVERGROUP_COUNT_START +
						 param3;
			break;
		case TERRAINOBJECT:
		case TREE:
		case BUILDING:
		case TREEBUILDING:
		case TURRET:
		case GATE:
			partId = MIN_TERRAIN_PART_ID + param1 * MAX_MAP_CELL_WIDTH + param2;
			break;
		case TRAINCAR:
			Fatal(0, " TRAINS NEED PART IDS ");
			break;
	}
	return(partId);
}

//---------------------------------------------------------------------------

void GameObjectManager::setPartId (GameObjectPtr obj, long param1, long param2, long param3) {

	//------------------------------------------------------------------------
	// ALL game objects should have their partIds set through this function.
	// This simply allows easier modification and monitoring of the partIds,
	// should we have any reason to modify or test 'em as missions are loaded.
	long partId = calcPartId(obj->getObjectClass(), param1, param2, param3);
	obj->setPartId(partId);
}

//---------------------------------------------------------------------------

long GameObjectManager::initCollisionSystem (FitIniFile* missionFile) {

	collisionSystem = new CollisionSystem;
	gosASSERT(collisionSystem != NULL);
		
	collisionSystem->init(missionFile);

	return(0);
}

//---------------------------------------------------------------------------

long GameObjectManager::buildCollidableList (void) 
{
	rebuildCollidableList = false;
	
	if (collidableList) 
	{
		ObjectTypeManager::objectCache->Free(collidableList);
		collidableList = NULL;
	}

	// First, how many collidables are there?
	numCollidables = numMechs + numVehicles + numElementals + numTurrets + numGates + numCarnage + numArtillery;

	collidableList = (GameObjectPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GameObjectPtr) * numCollidables);

	long curIndex = 0;
	for (long i = 0; i < numMechs; i++)
		collidableList[curIndex++] = mechs[i];

	for (int i = 0; i < numVehicles; i++)
		collidableList[curIndex++] = vehicles[i];

	for (int i = 0; i < numElementals; i++)
		collidableList[curIndex++] = (GameObjectPtr)elementals[i];

	for (int i=0;i<numTurrets;i++)
		collidableList[curIndex++] = turrets[i];

	for (int i=0;i<numGates;i++)
		collidableList[curIndex++] = gates[i];

	for (int i=0;i<numCarnage;i++)
		collidableList[curIndex++] = carnage[i];
		
	for (int i=0;i<numArtillery;i++)
		collidableList[curIndex++] = artillery[i];

	Assert(curIndex == numCollidables, curIndex, " GameObjectManager.buildCollidableList: oof ");
	
	return(0);
}

//---------------------------------------------------------------------------

long GameObjectManager::getCollidableList (GameObjectPtr*& objList) 
{
	if (rebuildCollidableList)
		buildCollidableList();
		
	objList = collidableList;
	return(numCollidables);
}

//---------------------------------------------------------------------------

long GameObjectManager::updateCollisions (void) {

	if (!collidableList)
		buildCollidableList();

	collisionSystem->checkObjects();

	return(0);
}

//---------------------------------------------------------------------------

void GameObjectManager::detectStaticCollision (GameObjectPtr obj1, GameObjectPtr obj2) {

	collisionSystem->detectStaticCollision(obj1, obj2);
}

//---------------------------------------------------------------------------

void GameObjectManager::updateCaptureList (void) {

	for (int i = 0; i < MAX_TEAMS; i++)
		numCaptures[i] = 0;
	for (int i = 0; i < getNumMovers(); i++) {
		MoverPtr mover = getMover(i);
		if (mover->isDisabled())
			continue;
		MechWarriorPtr pilot = mover->getPilot();
		if (pilot) {
			TacticalOrderPtr tacOrder;
			tacOrder = pilot->getCurTacOrder();
			if (tacOrder->code == TACTICAL_ORDER_CAPTURE) {
				captureList[mover->getTeamId()][numCaptures[mover->getTeamId()]++] = tacOrder->targetWID;
			}
		}
	}
}

//---------------------------------------------------------------------------

bool GameObjectManager::isTeamCapturing (TeamPtr team, GameObjectWatchID targetWID) {

	if (team) {
		long teamID = team->getId();
		for (long i = 0; i < numCaptures[teamID]; i++)
			if (targetWID == captureList[teamID][i])
				return(true);
		}
	else {
		for (long teamID = 0; teamID < MAX_TEAMS; teamID++)
			for (long i = 0; i < numCaptures[teamID]; i++)
				if (targetWID == captureList[teamID][i])
					return(true);
	}
	return(false);
}

//---------------------------------------------------------------------------

CarnagePtr GameObjectManager::createFire (ObjectTypeNumber fireObjTypeHandle,
										  GameObjectPtr owner,
										  Stuff::Vector3D& position,
										  float tonnage) {

	CarnagePtr fire = getCarnage(CARNAGE_FIRE);
	if (fire) {
		ObjectTypePtr objectType = getObjectType(fireObjTypeHandle);
		if (objectType) {
			//------------------------------------------------------------
			// Make sure the object type we loaded is really a fire object
			// type. If not, NULL out of here...
			if (objectType->getObjectClass() != FIRE)
				return(NULL);
			fire->init(false, objectType);
			fire->setOwner(owner);
			fire->setTonnage(tonnage);
			fire->setPosition(position);
			fire->setExists(true);
		}
	}
	return(fire);
}

//---------------------------------------------------------------------------

CarnagePtr GameObjectManager::createExplosion (long effectId,
											   GameObjectPtr owner,
											   Stuff::Vector3D& position,
											   float damage,
											   float radius) 
{
	long explosionObjTypeHandle = weaponEffects->GetEffectObjNum(effectId);
	if (explosionObjTypeHandle != INVALID_OBJECT) 
	{
		CarnagePtr explosion = getCarnage(CARNAGE_EXPLOSION);
		if (explosion)
		{
			ObjectTypePtr objectType = getObjectType(explosionObjTypeHandle);
			if (objectType && (objectType->getObjectClass() == EXPLOSION))
			{
				//Call in this order or badness.
				explosion->init(effectId);
				explosion->init(false, objectType);
				explosion->setPosition(position);
				explosion->setExtentRadius(radius);
				explosion->setExplDmg(damage);
				explosion->setExists(true);
				explosion->setOwner(owner);
				return(explosion);
			}
			else if (objectType && (objectType->getObjectClass() == FIRE))
			{
				//Call in this order or badness.
				explosion->init(effectId);
				explosion->init(false, objectType);
				explosion->setPosition(position);
				explosion->setExtentRadius(radius);
				explosion->setExplDmg(damage);
				explosion->setExists(true);
				explosion->setOwner(owner);
				return(explosion);
			}
			else
			{
				//Turn on when effects are officially working!
				// Don't really care if nothing happens.  Should be a bug not a crash!
				// -fs
			}
		}
		else
		{
			//Should we warn if we are out of explosions or just go on?
			//-fs
		}
	}

	return(NULL);
}

//---------------------------------------------------------------------------

LightPtr GameObjectManager::createLight (ObjectTypeNumber lightObjTypeHandle) {

	LightPtr light = getLight();
	if (light) {
		ObjectTypePtr objectType = getObjectType(lightObjTypeHandle);
		light->init(false, objectType);
//		light->setOwner(owner);
//		light->setTonnage(tonnage);
		Stuff::Vector3D startPosition;
		startPosition.Zero();
		light->setPosition(startPosition);
		light->setExists(true);
	}
	return(light);
}

//---------------------------------------------------------------------------
WeaponBoltPtr GameObjectManager::createWeaponBolt (long effectId) 
{
	WeaponBoltPtr weaponBolt = getWeapon();
	if (weaponBolt) 
	{
		ObjectTypeNumber weaponBoltObjTypeHandle = weaponEffects->GetEffectObjNum(effectId);
		ObjectTypePtr objectType = getObjectType(weaponBoltObjTypeHandle);
		if (!objectType)
			STOP(("Object Type for a weapon Bolt was NULL.  EffectId: %d  ObjType: %d",effectId, weaponBoltObjTypeHandle));
		if (objectType->getObjectClass() != WEAPONBOLT)
			return(NULL);
		//ALWAYS CALL IN THIS ORDER OR NO EFFECT!!!!!!!!!!!!!!!!!!!!
		weaponBolt->init(effectId);
		weaponBolt->init(false, objectType);
		weaponBolt->setExists(true);
	}

	return(weaponBolt);
}

//---------------------------------------------------------------------------

ArtilleryPtr GameObjectManager::createArtillery (long artilleryType, Stuff::Vector3D& position) 
{
	ArtilleryPtr artillery = getArtillery();
	if (artillery) 
	{
		static long strikeObjectId[NUM_ARTILLERY_TYPES] = 
		{
			SMALL_ARTLRY, BIG_ARTLRY, SENSOR_ARTLRY
		};
		
 		ObjectTypePtr objectType = getObjectType(strikeObjectId[artilleryType]);
		artillery->init(false, objectType);
		artillery->setPosition(position);
		artillery->setExists(true);
	}
	
	return(artillery);
}

void GameObjectManager::updateAppearancesOnly( bool terrain, bool movers, bool other)
{

	if (terrain && renderObjects) 
	{
		for (long terrainBlock = 0; terrainBlock < Terrain::numObjBlocks; terrainBlock++) 
		{
			if (Terrain::objBlockInfo[terrainBlock].active) 
			{
				long numObjs = Terrain::objBlockInfo[terrainBlock].numObjects;
				long objIndex = Terrain::objBlockInfo[terrainBlock].firstHandle;
				for (long terrainObj = 0; terrainObj < numObjs; terrainObj++, objIndex++) 
				{
					if (objList[objIndex] && 
						(Terrain::objVertexActive[objList[objIndex]->getVertexNum()] || (turn < 3)) && 
						objList[objIndex]->getExists())
					{
						if (objList[objIndex]->getAppearance()->recalcBounds()) 
						{
							// Must force here as well.
							Stuff::Vector3D pos = objList[objIndex]->getPosition();
							float rot = objList[objIndex]->getRotation();
							pos.z = land->getTerrainElevation(pos);
							long drawFlags = objList[objIndex]->drawFlags;
							int teamID = objList[objIndex]->getTeamId();
							objList[objIndex]->getAppearance()->setObjectParameters(pos,rot,drawFlags,teamID,Team::getRelation(teamID, Team::home->getId()));

							objList[objIndex]->getAppearance()->update( false );
							objList[objIndex]->getAppearance()->setVisibility(true, true);	
						}
					}
				}
			}
		}
	}

	if (movers) 
	{
		if (mechs) 
		{
			for (long i = 0; i < numMechs; i++)
				if (mechs[i] && mechs[i]->getExists())
				{
					bool inView = mechs[i]->getAppearance()->recalcBounds();
					if (inView)
						mechs[i]->windowsVisible = turn;
					
					// Must force here as well.
					Stuff::Vector3D pos = mechs[i]->getPosition();
					float rot = mechs[i]->getRotation();
					pos.z = land->getTerrainElevation(pos);
					bool selected = mechs[i]->isSelected();
					int teamID = mechs[i]->getTeamId();
					mechs[i]->getAppearance()->setObjectParameters(pos,rot,mechs[i]->drawFlags,teamID,Team::getRelation(teamID, Team::home->getId()));

					mechs[i]->getAppearance()->update( false );
					mechs[i]->getAppearance()->setVisibility(inView,true);
				}
		}

		if (vehicles) 
		{
			for (long i = 0; i < numVehicles; i++)
				if (vehicles[i] && vehicles[i]->getExists())
				{
					bool inView = vehicles[i]->getAppearance()->recalcBounds();
					vehicles[i]->windowsVisible = turn;

					// Must force here as well.
					Stuff::Vector3D pos = vehicles[i]->getPosition();
					float rot = vehicles[i]->getRotation();
					pos.z = land->getTerrainElevation(pos);
					if ((pos.z < MapData::waterDepth))
						pos.z = MapData::waterDepth;
					bool selected = vehicles[i]->isSelected();
					int teamID = vehicles[i]->getTeamId();
					vehicles[i]->getAppearance()->setObjectParameters(pos, rot, vehicles[i]->drawFlags, teamID, Team::getRelation(teamID, Team::home->getId()));
					vehicles[i]->getAppearance()->update( false );
					vehicles[i]->getAppearance()->setInView( inView );
				}
		}

	}

	if (other) {
		//---------------------------------------
		// All other objects should be updated...
		if (turrets) 
		{
			for (long i=0;i<numTurrets;i++) 
			{
				if (turrets[i] && turrets[i]->getExists()) 
				{
					if (turrets[i]->getAppearance()->recalcBounds())
					{
						// Must force here as well.
						Stuff::Vector3D pos = turrets[i]->getPosition();
						float rot = turrets[i]->getRotation();
						pos.z = land->getTerrainElevation(pos);
						bool selected = turrets[i]->isSelected();
						int teamID = turrets[i]->getTeamId();
						turrets[i]->getAppearance()->setObjectParameters(pos,rot,selected,teamID,Team::getRelation(teamID, Team::home->getId()));

						turrets[i]->getAppearance()->update( false );
						turrets[i]->getAppearance()->setInView( 1 );
					}
					else
						turrets[i]->getAppearance()->setInView( 0 );
				}
			}
		}
		
/*		if (weapons) {
			for (long i=0;i<numWeapons;i++) {
				if (weapons[i] && weapons[i]->getExists()) {
					if (!weapons[i]->update())
						weapons[i]->setExists(false);
				}
			}
		}*/

		/*if (carnage) {
			for (long i = 0; i < numCarnage; i++) {
				if (carnage[i] && carnage[i]->getExists()) {
					if (!carnage[i]->update())
						carnage[i]->setExists(false);
				}
			}
		}*/

		if (lights) 
		{
			for (long i = 0; i < numLights; i++) 
			{
				if (lights[i] && lights[i]->getExists()) 
				{
					if (lights[i]->getAppearance()->recalcBounds() )
					{
						lights[i]->getAppearance()->update( false );
						lights[i]->getAppearance()->setInView( 1 );
					}
					else
						lights[i]->getAppearance()->setInView(0);
				}
			}
		}

		if (artillery) 
		{
			for (long i = 0; i < numArtillery; i++) 
			{
				if (artillery[i] && artillery[i]->getExists()) 
				{
					if (artillery[i]->getAppearance()->recalcBounds() )
					{
						artillery[i]->getAppearance()->update( false );
						artillery[i]->getAppearance()->setInView( 1 );
					}
					else
						artillery[i]->getAppearance()->setInView( 0 );
				}
			}
		}
	}
}

//-------------------------------------------------------------------
void GameObjectManager::CopyTo (ObjectManagerData *data)
{
	data->maxObjects 		= getMaxObjects();
	data->numElementals		= numElementals;
	data->numTerrainObjects = numTerrainObjects;
	data->numBuildings		= numBuildings;
	data->numTurrets		= numTurrets;
	data->numWeapons		= numWeapons;
	data->numCarnage		= numCarnage;
	data->numLights			= numLights;
	data->numArtillery		= numArtillery;
	data->numGates			= numGates;
	data->maxMechs			= maxMechs;
	data->maxVehicles		= maxVehicles;
	data->numMechs			= numMechs;
	data->numVehicles		= numVehicles;
	data->nextWatchId		= nextWatchID;
}

//-------------------------------------------------------------------
void GameObjectManager::CopyFrom (ObjectManagerData *data)
{
	numElementals		=    data->numElementals;
	numTerrainObjects   =    data->numTerrainObjects;
	numBuildings        =    data->numBuildings;
	numTurrets          =    data->numTurrets;
	numWeapons          =    data->numWeapons;
	numCarnage          =    data->numCarnage;
	numLights           =    data->numLights;
	numArtillery        =    data->numArtillery;
	numGates            =    data->numGates;
	maxMechs            =    data->maxMechs;
	maxVehicles         =    data->maxVehicles;
	numMechs            =    data->numMechs;
	numVehicles         =    data->numVehicles;
	nextWatchID			=	 data->nextWatchId;
}

//-------------------------------------------------------------------
long GameObjectManager::Save (PacketFilePtr file, long packetNum)
{
	long *watchSave = (long *)malloc(sizeof(long) * (getMaxObjects() + 1));
	memset(watchSave,0,sizeof(long) * (getMaxObjects() + 1));

	ObjectManagerData data;
	CopyTo(&data);

	file->writePacket(packetNum,(MemoryPtr)&data,sizeof(ObjectManagerData),STORAGE_TYPE_RAW);
	packetNum++;

	for (long i=0;i<=getMaxObjects();i++)
	{
		if (objList[i])
		{
			objList[i]->Save(file,packetNum);
			packetNum++;

			//Find the watchID from the watchlist for this object.
			// If none, let it be.  It'll be zero already
			// DO NOT CALL getWatchID!!!!!!!
			// That will assign them to objects which don't have them!!!!
			for (long j=0;j<=nextWatchID;j++)
			{
				if (watchList[j] == objList[i])
				{
					watchSave[j] = i;
				}
			}
		}
		else
		{
			packetNum++;
		}
	}

	file->writePacket(packetNum,(MemoryPtr)watchSave,sizeof(long) * (getMaxObjects() + 1),STORAGE_TYPE_ZLIB);
	packetNum++;

	free(watchSave);
	watchSave = NULL;

	return packetNum;
}

//-------------------------------------------------------------------
long GameObjectManager::Load (PacketFilePtr file, long packetNum)
{
	ObjectManagerData data;
	file->readPacket(packetNum,(MemoryPtr)&data);
	packetNum++;

	CopyFrom(&data);

	//------------------------------------------------------------------
	//Create the ObjList and WatchList EXACTLY like we normally do.
	GameObject::setInitialize(true);

	//-----------------------------------------------------------
	// First element in list is NULL (handle of 0 is always NULL)
	objList = (GameObjectPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GameObjectPtr) * (getMaxObjects() + 1));
	memset(objList,0,sizeof(GameObjectPtr) * (getMaxObjects() + 1));

	watchList = (GameObjectPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GameObjectPtr) * (getMaxObjects() + 1));
	memset(watchList,0,sizeof(GameObjectPtr) * (getMaxObjects() + 1));

	long i = 0;
	long curHandle = 1;
	maxMovers = maxMechs + maxVehicles + numElementals;
	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (maxMechs > 0) 
	{
		mechs = (BattleMechPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(BattleMechPtr) * maxMechs);
		if (!mechs)
			Fatal(maxMechs, " GameObjectManager.setNumObjects: cannot malloc mechs ");
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (maxVehicles > 0) 
	{
		vehicles = (GroundVehiclePtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GroundVehiclePtr) * maxVehicles);
		if (!vehicles)
			Fatal(maxVehicles, " GameObjectManager.setNumObjects: cannot malloc vehicles ");
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (numTerrainObjects > 0) 
	{
		terrainObjects = (TerrainObjectPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(TerrainObjectPtr) * numTerrainObjects);
		if (!terrainObjects)
			Fatal(numTerrainObjects, " GameObjectManager.setNumObjects: cannot malloc terrain objects ");
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (numBuildings > 0) 
	{
		buildings = (BuildingPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(BuildingPtr) * numBuildings);
		if (!buildings)
			Fatal(numBuildings, " GameObjectManager.setNumObjects: cannot malloc buildings ");
	}

	if (numTurrets > 0)
	{
		turrets = (TurretPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(TurretPtr) * numTurrets);
		if ( !turrets )
			Fatal(numTurrets, " GameObjectManager.setNumObjects: cannot malloc Turrets ");
	}

	if (numGates > 0)
	{
		gates = (GatePtr*)ObjectTypeManager::objectCache->Malloc(sizeof(GatePtr) * numGates);
		if ( !gates )
			Fatal(numGates, " GameObjectManager.setNumObjects: cannot malloc Gates ");
	}

	//--------------------------------------------------------------
	// For now, we'll use an array of pointers due to the irritating
	// 'new' for arrays problem...
	if (numWeapons > 0) 
	{
		weapons = (WeaponBoltPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(WeaponBoltPtr) * numWeapons);
		if (!weapons)
			Fatal(numWeapons, " GameObjectManager.setNumObjects: cannot malloc weapons ");
	}

	if (numCarnage > 0) 
	{
		carnage = (CarnagePtr*)ObjectTypeManager::objectCache->Malloc(sizeof(CarnagePtr) * numCarnage);
		if (!carnage)
			Fatal(numCarnage, " GameObjectManager.setNumObjects: cannot malloc carnage ");
	}

	if (numArtillery > 0) 
	{
		artillery = (ArtilleryPtr*)ObjectTypeManager::objectCache->Malloc(sizeof(ArtilleryPtr) * numArtillery);
		if (!artillery)
			Fatal(numArtillery, " GameObjectManager.setNumObjects: cannot malloc artillery ");
	}

	useMoverLineOfSightTable = true;
	moverLineOfSightTable = (char*)systemHeap->Malloc(maxMovers * maxMovers);
	if (!moverLineOfSightTable)
		Fatal(numGates, " GameObjectManager.setNumObjects: cannot malloc moverLineOfSightTable ");

	long curTerrObjNum = 0;
	long curBuildingNum = 0;
	long curTurretNum = 0;
	long curGateNum = 0;
	long curArtilleryNum = 0;
	long curCarnageNum = 0;
	long curMechNum = 0;
	long curVehicleNum = 0;
	long curBoltNum = 0;

	long oldBlockNum = -1;
	for (i=0;i<=getMaxObjects();i++)
	{
		//OK.
		// For each object that was saved, Load the data.
		// If objectTypeNum != 0, create the object from the type EXACTLY like we do above.
		// then init from the savedData.
		// Next Object.

		//First, get the size of the packet saved.
		// This will tell me what kind of object it was!
		file->seekPacket(packetNum);
		DWORD packetSize = file->getPacketSize();
		if (packetSize == 0)
		{
			//NO Object stored here.
			// Increment Packet and move on
			// First object in ObjList, for example!!
			packetNum++;
		}
		else if ((packetSize == sizeof(GameObjectData)) ||
				(packetSize == sizeof(MoverData)))
		{
			STOP(("We saved a pure GameObject or Pure Mover in slot %d, packet %d",i,packetNum));
		}
		else if (packetSize == sizeof(TerrainObjectData))
		{
			//We have a TerrainObject.
			// Get its data.
			TerrainObjectData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			TerrainObjectPtr obj = new TerrainObject; 
			objList[i] = terrainObjects[curTerrObjNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
				obj->setDamage(data.damage);
				obj->setRotation(data.rotation);

				if (data.blockNumber != oldBlockNum)
				{
					oldBlockNum = data.blockNumber;
					Terrain::objBlockInfo[data.blockNumber].firstHandle = i;
				}

				Terrain::objBlockInfo[data.blockNumber].numCollidableObjects++;
				Terrain::objBlockInfo[data.blockNumber].numObjects++;
			}
			else
			{
				obj->setExists(false);
			}

			curTerrObjNum++;
		}
		else if (packetSize == sizeof(BuildingData))
		{
			//We have a Building.
			BuildingData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			BuildingPtr obj = new Building; 
			objList[i] = buildings[curBuildingNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
				obj->setDamage(data.damage);
				obj->setRotation(data.rotation);

				if (data.blockNumber != oldBlockNum)
				{
					oldBlockNum = data.blockNumber;
					Terrain::objBlockInfo[data.blockNumber].firstHandle = i;
				}

				if (((((BuildingTypePtr)objType)->perimeterAlarmRange > 0.0f) &&
					(((BuildingTypePtr)objType)->perimeterAlarmTimer > 0.0f)) ||
					(((BuildingTypePtr)objType)->lookoutTowerRange > 0.0f) ||
					(((BuildingTypePtr)objType)->sensorRange > 0.0f))
				{
					Terrain::objBlockInfo[data.blockNumber].numCollidableObjects++;
				}

				Terrain::objBlockInfo[data.blockNumber].numObjects++;
			}
			else
			{
				obj->setExists(false);
			}
			curBuildingNum++;
		}
		else if (packetSize == sizeof(TurretData))
		{
			//We have a Turret.
			TurretData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			TurretPtr obj = new Turret; 
			objList[i] = turrets[curTurretNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
				obj->setDamage(data.damage);
				obj->setRotation(data.rotation);

				if (data.blockNumber != oldBlockNum)
				{
					oldBlockNum = data.blockNumber;
					Terrain::objBlockInfo[data.blockNumber].firstHandle = i;
				}

				Terrain::objBlockInfo[data.blockNumber].numCollidableObjects++;
				Terrain::objBlockInfo[data.blockNumber].numObjects++;
			}
			else
			{
				obj->setExists(false);
			}
			curTurretNum++;
		}
		else if (packetSize == sizeof(GateData))
		{
			//We have a Gate.
			GateData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			GatePtr obj = new Gate; 
			objList[i] = gates[curGateNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
				obj->setDamage(data.damage);
				obj->setRotation(data.rotation);

				if (data.blockNumber != oldBlockNum)
				{
					oldBlockNum = data.blockNumber;
					Terrain::objBlockInfo[data.blockNumber].firstHandle = i;
				}

				Terrain::objBlockInfo[data.blockNumber].numCollidableObjects++;
				Terrain::objBlockInfo[data.blockNumber].numObjects++;
			}
			else
			{
				obj->setExists(false);
			}
			curGateNum++;
		}
		else if (packetSize == sizeof(ArtilleryData))
		{
			//We have an Artillery.
			ArtilleryData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			ArtilleryPtr obj = new Artillery; 
			objList[i] = artillery[curArtilleryNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
			}
			else
			{
				obj->setExists(false);
			}
			curArtilleryNum++;
		}
		else if (packetSize == sizeof(CarnageData))
		{
			//We have a Carnage.
			CarnageData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			CarnagePtr obj = new Carnage; 
			objList[i] = carnage[curCarnageNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
			}
			else
			{
				obj->setExists(false);
			}
			curCarnageNum++;
		}
		else if (packetSize == sizeof(MechData))
		{
			//We have a BattleMech.
			MechData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			BattleMechPtr obj = new BattleMech; 
			objList[i] = mechs[curMechNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
			}
			else
			{
				obj->setExists(false);
			}
			curMechNum++;
		}
		else if (packetSize == sizeof(GroundVehicleData))
		{
			//We have a groundVehicle.
			GroundVehicleData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			GroundVehiclePtr obj = new GroundVehicle; 
			objList[i] = vehicles[curVehicleNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
			}
			else
			{
				obj->setExists(false);
			}
			curVehicleNum++;
		}
		else if (packetSize == sizeof(WeaponBoltData))
		{
			//We have a WeaponBolt.
			WeaponBoltData data;
			file->readPacket(packetNum,(MemoryPtr)(&data));
			packetNum++;

			WeaponBoltPtr obj = new WeaponBolt; 
			objList[i] = weapons[curBoltNum] = obj;
			objList[i]->setHandle(i);

			if (data.objectTypeNum > 0)	//We have an object type to copy from!!
			{
				ObjectTypePtr objType = getObjectType(data.objectTypeNum);
				if (!objType)
					STOP(("We saved an Object we don't know how to re-create %d",data.objectTypeNum));

				obj->init(true, objType);
				obj->setExists(true);
				obj->Load(&data);
			}
			else
			{
				obj->setExists(false);
			}
			curBoltNum++;
		}
	}

	if (curTerrObjNum != numTerrainObjects)
		STOP(("Didn't load %d but instead %d TerrainObjects",numTerrainObjects,curTerrObjNum));

	if (curBuildingNum != numBuildings)
		STOP(("Didn't load %d but instead %d Buildings",numBuildings,curBuildingNum));

	if (curTurretNum != numTurrets)
		STOP(("Didn't load %d but instead %d Turrets",numTurrets,curTurretNum));

	if (curGateNum != numGates)
		STOP(("Didn't load %d but instead %d Gates",numGates,curGateNum));

	if (curArtilleryNum != numArtillery)
		STOP(("Didn't load %d but instead %d Artillery",numArtillery,curArtilleryNum));

	if (curCarnageNum != numCarnage)
		STOP(("Didn't load %d but instead %d Carnage",numCarnage,curCarnageNum));

	if (curMechNum != maxMechs)
		STOP(("Didn't load %d but instead %d Mechs",maxMechs,curMechNum));

	if (curVehicleNum != maxVehicles)
		STOP(("Didn't load %d but instead %d Vehicles",maxVehicles,curVehicleNum));

	if (curBoltNum != numWeapons)
		STOP(("Didn't load %d but instead %d WeaponBolts",numWeapons,curBoltNum));

	rebuildCollidableList = true;

	//---------------------------------------------------
	// Finally, let's build the control building lists...
	for (i = 0; i < numTurrets; i++) 
	{
		if ((turrets[i]->parentId != 0xffffffff) && (turrets[i]->parentId != 0)) 
		{
			BuildingPtr controlBuilding = (BuildingPtr)ObjectManager->findByCellPosition((turrets[i]->parentId>>16),(turrets[i]->parentId & 0x0000ffff));
			if (controlBuilding && !controlBuilding->getFlag(OBJECT_FLAG_CONTROLBUILDING)) 
			{
				controlBuilding->setFlag(OBJECT_FLAG_CAPTURABLE, true);
				controlBuilding->setFlag(OBJECT_FLAG_CONTROLBUILDING, true);
				controlBuilding->listID = numTurretControls;
				turretControls[numTurretControls++] = controlBuilding;
			}
			else if (!controlBuilding)
			{
				Stuff::Vector3D worldPos;
				if (land)
					land->cellToWorld((turrets[i]->parentId>>16),(turrets[i]->parentId & 0x0000ffff),worldPos);
				PAUSE(("Turret linked to bldg @ R %d, C %d  X:%f Y:%f No Bldg there!",(turrets[i]->parentId>>16),(turrets[i]->parentId & 0x0000ffff),worldPos.x,worldPos.y));
				turrets[i]->parentId = 0xffffffff;
			}
		}
	}

	for (i = 0; i < numGates; i++) 
	{
		if ((gates[i]->parentId != 0xffffffff) && (gates[i]->parentId != 0))
		{
			BuildingPtr controlBuilding = (BuildingPtr)ObjectManager->findByCellPosition((gates[i]->parentId>>16),(gates[i]->parentId & 0x0000ffff));
			if (controlBuilding && !controlBuilding->getFlag(OBJECT_FLAG_CONTROLBUILDING)) 
			{
				controlBuilding->setFlag(OBJECT_FLAG_CONTROLBUILDING, true);
				controlBuilding->listID = numGateControls;
				gateControls[numGateControls++] = controlBuilding;
			}
	   		else if (!controlBuilding)
	   		{
	   			PAUSE(("Gate tried to link to building at Row %d, Col %d.  No Bldg there!",(gates[i]->parentId>>16),(gates[i]->parentId & 0x0000ffff)));
				gates[i]->parentId = 0xffffffff;
	   		}
		}
	}

	//-----------------------------------------------------------------------------------
	// Create list of special buildings.  These buildings will be updated at least once
	// every frame regardless of where they are on the terrain and what is visible.
	// This is because perimeter alarms and lookout buildings and ????? must work even
	// if the player is NOT looking at them!!
	for (i=0;i<numBuildings;i++)
	{
		if (buildings[i]->isSpecialBuilding())
		{
			specialBuildings[numSpecialBuildings++] = buildings[i];
		}
	}
	
 	//--------------------------------------------------------------------
	//Now, lets point every lit building to at least one power generator.
	for (i=0;i<numBuildings;i++)
	{
		if (buildings[i]->isPowerSource())
		{
			powerGenerators[numPowerGenerators++] = buildings[i];
		}
	}
	
	if (numPowerGenerators)
	{
		for (i=0;i<numBuildings;i++)
		{
			Stuff::Vector3D maxDist;
			long generatorIndex = 0;
			maxDist.Subtract(buildings[i]->getPosition(),powerGenerators[0]->getPosition());
			float minDistance = maxDist.GetApproximateLength();
			
			for (long j=1;j<numPowerGenerators;j++)
			{
				maxDist.Subtract(buildings[i]->getPosition(),powerGenerators[j]->getPosition()); 
				float newDistance = maxDist.GetApproximateLength(); 
				
				if (newDistance < minDistance)
				{
					generatorIndex = j;
					minDistance = newDistance;
				}
			}	
			
			buildings[i]->setPowerSupply(powerGenerators[generatorIndex]);
		}
		
		for (i=0;i<numTerrainObjects;i++)
		{
			Stuff::Vector3D maxDist;
			long generatorIndex = 0;
			maxDist.Subtract(terrainObjects[i]->getPosition(),powerGenerators[0]->getPosition());
			float minDistance = maxDist.GetApproximateLength();
			
			for (long j=1;j<numPowerGenerators;j++)
			{
				maxDist.Subtract(terrainObjects[i]->getPosition(),powerGenerators[j]->getPosition()); 
				float newDistance = maxDist.GetApproximateLength(); 
				
				if (newDistance < minDistance)
				{
					generatorIndex = j;
					minDistance = newDistance;
				}
			}	
			
			terrainObjects[i]->setPowerSupply(powerGenerators[generatorIndex]);
		}
	}

	GameObject::setInitialize(false);

	//Reload the Object Watchers
	long *watchSave = (long *)malloc(sizeof(long) * (getMaxObjects() + 1));
	memset(watchSave,0,sizeof(long) * (getMaxObjects() + 1));

	file->readPacket(packetNum,(MemoryPtr)watchSave);
	packetNum++;

	//Find the watchID from the watchlist for this object.
	// If none, let it be.  It'll be zero already
	// DO NOT CALL getWatchID!!!!!!!
	// That will assign them to objects which don't have them!!!!
	for (long j=0;j<getMaxObjects();j++)
	{
		watchList[j] = objList[watchSave[j]];
	}

	free(watchSave);
	watchSave = NULL;

	return packetNum;
}

//***************************************************************************
