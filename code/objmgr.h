//***************************************************************************
//
//	objmgr.h - This file contains the GameObject Manager class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef OBJMGR_H
#define OBJMGR_H

//---------------------------------------------------------------------------

#ifndef DOBJMGR_H
#include"dobjmgr.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

#ifndef DMECH_H
#include"dmech.h"
#endif

#ifndef DGVEHICL_H
#include"dgvehicl.h"
#endif

#ifndef DELEMNTL_H
#include"delemntl.h"
#endif

#ifndef DTERROBJ_H
#include"dterrobj.h"
#endif

#ifndef DBLDNG_H
#include"dbldng.h"
#endif

#ifndef DWEAPONBOLT_H
#include"dweaponbolt.h"
#endif

#ifndef DCARNAGE_H
#include"dcarnage.h"
#endif

#ifndef DARTLRY_H
#include"dartlry.h"
#endif

#ifndef DOBJBLCK_H
#include"dobjblck.h"
#endif

#ifndef DTURRET_H
#include"dturret.h"
#endif

#ifndef DGATE_H
#include"dgate.h"
#endif

#ifndef DCOLLSN_H
#include"dcollsn.h"
#endif

class PacketFile;

//---------------------------------------------------------------------------

#define	MOVERLIST_DELETE	0
#define	MOVERLIST_ADD		1
#define	MOVERLIST_TRADE		2

#define NO_RAM_FOR_TERRAIN_OBJECT_FILE		0xBAAA0014
#define NO_RAM_FOR_TERRAIN_OBJECT_HEAP		0xBAAA0015
#define NO_RAM_FOR_OBJECT_BLOCK_NUM			0xBAAA0016
#define NO_RAM_FOR_OBJECT_LISTS				0xBAAA0017
#define NO_RAM_FOR_OBJECT_DATA_BLOCK		0xBAAA0018
#define NO_RAM_FOR_OBJECT_BLOCK_USER		0xBAAA0019
#define NO_RAM_FOR_LAST_BLOCK				0xBAAA001A
#define NO_RAM_FOR_CENTER_BLOCK				0xBAAA001B
#define OBJECTBLOCK_OUTOFRANGE				0xBAAA001C
#define NO_AVAILABLE_OBJQUEUE				0xBAAA001D
#define COULDNT_MAKE_TERRAIN_OBJECT			0xBAAA001E
#define OBJECTBLOCK_NULL					0xBAAA001F
#define BLOCK_NOT_CACHED					0xBAAA0020
#define COULDNT_CREATE_OBJECT				0xBAAA0021
#define OBJECT_NOT_FOUND					0xBAAA0022

#pragma pack(1)

struct ObjData {
	short				objTypeNum;		//Type number of object
	unsigned short		vertexNumber;	//Vertex Number in Block.
	unsigned short		blockNumber;	//Which terrain Block.
	unsigned char		damage;			//Damage
	int				    teamId;
	unsigned int		parentId;		//hOW AM i LINKED.
};

struct MiscObjectData {
	int				blockNumber;	//Terrain Block I occupy
	int				vertexNumber;	//Terrain Vertex I occupy
	int				objectTypeNum;	//ObjectTypeNumber for this overlay tile
	int				damaged;		//Is this overlay tile damaged or not
};

struct ObjDataLoader {
		int objTypeNum;
		Stuff::Vector3D vector;

		float rotation;		
		int damage;
		
		int teamId;
		int parentId;

		// convert to block and vertex
		int tileCol;
		int tileRow;
		
		int blockNumber;
		int vertexNumber;
};

#pragma pack()

typedef ObjData *ObjDataPtr;

//---------------------------------------------------------------------------

#define	MAX_REINFORCEMENTS_PER_TEAM		16
#define	MAX_GATE_CONTROLS				50
#define	MAX_TURRET_CONTROLS				50
#define MAX_POWER_GENERATORS			50
#define MAX_SPECIAL_BUILDINGS			200

#pragma pack(1)

typedef struct _TeamObjectInfo {
	short		numMechs;
	short		numVehicles;
	short		numElementals;
	short		mechHandleIndex;
	short		vehicleHandleIndex;
	short		elementalHandleIndex;
} TeamObjectInfo;

#pragma pack()

#define	MAX_REMOVED	20
#define	MAX_WATCHES 4000

#define	MAX_CAPTURES_PER_TEAM	30

typedef struct _RemovedMoverRec {
	unsigned int	turn;
	int			partID;
} RemovedMoverRec;

typedef struct _ObjectManagerData
{
	int					maxObjects;
	int					numElementals;
	int					numTerrainObjects;
	int					numBuildings;
	int					numTurrets;
	int					numWeapons;
	int					numCarnage;
	int					numLights;
	int					numArtillery;
	int					numGates;
	int					maxMechs;
	int					maxVehicles;
	int					numMechs;
	int					numVehicles;
	int					nextWatchId;
} ObjectManagerData;

class GameObjectManager {

	public:

		ObjectTypeManagerPtr	objTypeManager;

		long					numMechs;
		long					numVehicles;
		long					numElementals;
		long					numTerrainObjects;
		long					numBuildings;
		long					numTurrets;
		long					numWeapons;
		long					numCarnage;
		long					numLights;
		long					numArtillery;
		long					numGates;
		long					maxMechs;
		long					maxVehicles;
		long					maxMovers;

		BattleMechPtr*			mechs;
		GroundVehiclePtr*		vehicles;
		ElementalPtr*			elementals;
		TerrainObjectPtr*		terrainObjects;
		BuildingPtr*			buildings;
		WeaponBoltPtr*			weapons;
		LightPtr*				lights;
		CarnagePtr*				carnage;
		ArtilleryPtr*			artillery;
		TurretPtr*				turrets;
		GatePtr*				gates;
		
		BuildingPtr				gateControls[MAX_GATE_CONTROLS];
		BuildingPtr				turretControls[MAX_TURRET_CONTROLS];
		BuildingPtr				powerGenerators[MAX_POWER_GENERATORS];
		BuildingPtr				specialBuildings[MAX_SPECIAL_BUILDINGS];		//These must be updated EVERY FRAME.  Perimeter alarms, etc.
		long					numGateControls;
		long					numTurretControls;
		long					numPowerGenerators;
		long					numSpecialBuildings;

		char*					moverLineOfSightTable;
		bool					useMoverLineOfSightTable;

		GameObjectPtr*			objList;
		GameObjectPtr*			collidableList;
		MoverPtr				moverList[MAX_MOVERS];
		MoverPtr				goodMoverList[MAX_MOVERS];
		MoverPtr				badMoverList[MAX_MOVERS];
		long					numCollidables;
		long					numGoodMovers;
		long					numBadMovers;
		long					numMovers;
		long					nextReinforcementPartId;
		long					numRemoved;
		RemovedMoverRec			moversRemoved[MAX_REMOVED];
		unsigned long			nextWatchID;
		GameObjectPtr			*watchList;

		long					currentWeaponsIndex;			//points to next entry in rotating weapon array.
		long					currentCarnageIndex;			//points to next entry in rotating carnage list		
		long					currentLightIndex;
		long					currentArtilleryIndex;
		
		bool					rebuildCollidableList;

		long					numCaptures[MAX_TEAMS];
		GameObjectWatchID		captureList[MAX_TEAMS][MAX_CAPTURES_PER_TEAM];

		long					totalObjCount;
		ObjDataLoader			*objData;						//Used to keep from loading twice!!
		CollisionSystemPtr		collisionSystem;

	private:

		GameObjectPtr findObjectByMouse (long mouseX,
										 long mouseY,
										 GameObjectPtr* searchList,
										 long listSize,
										 bool skipDisabled = false);

		GameObjectPtr findMoverByMouse (long mouseX,
										long mouseY,
										long teamId,
										bool skipDisabled = false);

		GameObjectPtr findTerrainObjectByMouse (long mouseX,
												long mouseY,
												bool skipDisabled = false);

		void addObject(ObjDataLoader *objData, long& objIndex, long& buildIndex, 
			long &turretIndex, long &gateIndex, long& curCollideHandle, long& curNonCollideHandle);

		void countObject( ObjDataLoader* objType);


	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void);
		
		GameObjectManager (void) {
			init();
		}

		void destroy (void);

		~GameObjectManager (void) {
			destroy();
		}
		
		void init (const char* objTypeDataFile, long objTypeCacheSize, long objCacheSize);

		void setNumObjects (long nMechs,
							long nVehicles,
							long nElementals,
							long nTerrainObjects,
							long nBuildings,
							long nTurrets,
							long nWeapons,
							long nCarnage,
							long nLights,
							long nArtillery,
							long nGates);

		long getNumObjects (void) {
			return(numMechs +
				   numVehicles +
				   numElementals +
				   numTerrainObjects +
				   numBuildings +
				   numTurrets +
				   numWeapons +
				   numCarnage +
				   numLights +
				   numArtillery + 
				   numGates);
		}

		long getMaxObjects (void) {
			return(maxMechs +
				   maxVehicles +
				   numElementals +
				   numTerrainObjects +
				   numBuildings +
				   numTurrets +
				   numWeapons +
				   numCarnage +
				   numLights +
				   numArtillery + 
				   numGates);
		}

		long load (File* gameFile);

		long save (File* gameFile);

		BattleMechPtr newMech (void);

		GroundVehiclePtr newVehicle (void);

		void freeMover (MoverPtr mover);
		
		void tradeMover (MoverPtr mover, long newTeamID, long newCommanderID);

#ifdef USE_ELEMENTAL
		BattleMechPtr addElemental (void);
#endif

		BattleMechPtr getMech (long mechIndex);

		GroundVehiclePtr getVehicle (long vehicleIndex);

		GroundVehiclePtr getOpenVehicle (void);

		ElementalPtr getElemental (long elementalIndex);

		TerrainObjectPtr getTerrainObject (long terrainObjectIndex);

		BuildingPtr getBuilding (long buildingIndex);

		TurretPtr getTurret (long turretIndex);
		
		GatePtr	getGate (long gateIndex);
		
		WeaponBoltPtr getWeapon (void);				//gets next available weapon.

		CarnagePtr getCarnage (CarnageEnumType carnageType);

		void releaseCarnage (CarnagePtr obj);

		LightPtr getLight (void);

		void releaseLight (LightPtr obj);

		ArtilleryPtr getArtillery (void);

		long getNumArtillery (void) {
			return(numArtillery);
		}

		ArtilleryPtr getArtillery (long artilleryIndex) {
			return(artillery[artilleryIndex]);
		}

		long getNumGateControls (void) {
			return(numGateControls);
		}

		long getNumTurretControls (void) {
			return(numTurretControls);
		}

		BuildingPtr getGateControl (long index) {
			return(gateControls[index]);
		}

		BuildingPtr getTurretControl (long index) {
			return(turretControls[index]);
		}

		void countTerrainObjects (PacketFile* pFile, long firstHandle);

		void loadTerrainObjects ( PacketFile* pFile, volatile float& progress, float progressRange );

		long getSpecificObjects (long objClass, long objSubType, GameObjectPtr* objects, long maxObjects);

		GameObjectPtr getObjBlockObject (long blockNumber, long objLocalIndex) {
			return(objList[Terrain::objBlockInfo[blockNumber].firstHandle + objLocalIndex]);
		}

		long getObjBlockNumObjects (long blockNumber) {
			return(Terrain::objBlockInfo[blockNumber].numObjects);
		}

		long getObjBlockNumCollidables (long blockNumber) {
			return(Terrain::objBlockInfo[blockNumber].numCollidableObjects);
		}

		long getObjBlockFirstHandle (long blockNumber) {
			return(Terrain::objBlockInfo[blockNumber].firstHandle);
		}

		long getNumMechs (void) {
			return(numMechs);
		}

		long getNumVehicles (void) {
			return(numVehicles);
		}

		long getNumMovers (void) {
			return(numMovers);
		}

		long getMaxMovers (void) {
			return(maxMovers);
		}

		long getNumGoodMovers (void) {
			return(numGoodMovers);
		}

		long getNumBadMovers (void) {
			return(numBadMovers);
		}

		long getNumTerrainObjects (void) {
			return(numTerrainObjects);
		}

		long getNumBuildings (void) {
			return(numBuildings);
		}

		long getNumTurrets (void) {
			return(numTurrets);
		}

		long getNumGates (void) {
			return(numGates);
		}

		void render (bool terrain, bool movers, bool other);

		void renderShadows (bool terrain, bool movers, bool other);

		void update (bool terrain, bool movers, bool other);
		void updateAppearancesOnly( bool terrain, bool mover, bool other);

		GameObjectPtr get (long handle);

		GameObjectPtr getByWatchID (unsigned long watchID) {
			if ((watchID > 0) && (watchID < nextWatchID))
				return(watchList[watchID]);
			return(NULL);
		}

		long buildMoverLists (void);

		bool modifyMoverLists (MoverPtr mover, long action);

		MoverPtr getMover (long index) {
			return(moverList[index]);
		}

		MoverPtr getGoodMover (long index) {
			return(goodMoverList[index]);
		}

		MoverPtr getBadMover (long index) {
			return(badMoverList[index]);
		}

		GameObjectPtr findObject (Stuff::Vector3D position);

		GameObjectPtr findByPartId (long partId);

		GameObjectPtr findByBlockVertex (long blockNum, long vertex);

		GameObjectPtr findByCellPosition (long row, long col);

		GameObjectPtr findByUnitInfo (long commander, long group, long mate);

		GameObjectPtr findObjectByTypeHandle (long typeHandle);

		GameObjectPtr findObjectByMouse (long mouseX, long mouseY);
		
		bool moverInRect(long index, Stuff::Vector3D &dStart, Stuff::Vector3D &dEnd);

		ObjectTypePtr loadObjectType (ObjectTypeNumber typeHandle);

		ObjectTypePtr getObjectType (ObjectTypeNumber typeHandle);

		void removeObjectType (ObjectTypeNumber typeHandle);

		GameObjectHandle getHandle (GameObjectPtr obj);

		long calcPartId (long objectClass, long param1 = 0, long param2 = 0, long param3 = 0);

		void setPartId (GameObjectPtr obj, long param1 = 0, long param2 = 0, long param3 = 0);

		long buildCollidableList (void);

		long initCollisionSystem (FitIniFile* missionFile);

		long getCollidableList (GameObjectPtr*& objList);

		long updateCollisions (void);

		void detectStaticCollision (GameObjectPtr obj1, GameObjectPtr obj2);
		
		void updateCaptureList (void);

		bool isTeamCapturing (TeamPtr team, GameObjectWatchID targetWID);

		CarnagePtr createFire (ObjectTypeNumber fireObjTypeHandle,
							   GameObjectPtr owner,
							   Stuff::Vector3D& position,
							   float tonnage);

		CarnagePtr createExplosion (long effectId,
									GameObjectPtr owner,
									Stuff::Vector3D& position,
									float dmg = 0.0,
									float radius = 0.0);

		LightPtr createLight (ObjectTypeNumber lightObjTypeHandle);

		WeaponBoltPtr createWeaponBolt (long effectId);

		ArtilleryPtr createArtillery (long artilleryType, Stuff::Vector3D& position);

		RemovedMoverRec* getRemovedMovers (long& numRemovedMovers) {
			numRemovedMovers = numRemoved;
			return(moversRemoved);
		}

		bool isRemoved (MoverPtr mover);

		void setWatchID (GameObjectPtr obj);

		void CopyTo (ObjectManagerData *data);
		void CopyFrom (ObjectManagerData *data);

		long Save (PacketFilePtr file, long packetNum);
		long Load (PacketFilePtr file, long packetNum);
};

extern GameObjectManagerPtr ObjectManager;

//***************************************************************************

#endif
