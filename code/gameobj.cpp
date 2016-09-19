//***************************************************************************
//
//	gameobj.cpp -- File contains the Game Object class code
//
//	MechCommander 2
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

#ifndef CARNAGE_H
#include"carnage.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef DOBJNUM_H
#include"dobjnum.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#ifndef CMPONENT_H
#include"cmponent.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

#ifndef TURRET_H
#include"turret.h"
#endif

#ifndef GAMELOG_H
#include"gamelog.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

//---------------------------------------------------------------------------
extern GameLog* CombatLog;

extern MissionMapPtr GameMap;
extern float metersPerWorldUnit;
extern float worldUnitsPerMeter;

extern TeamPtr homeTeam;
extern float MechClassWeights[NUM_MECH_CLASSES];

extern char* ExceptionGameMsg;
char ChunkDebugMsg[5120];

unsigned long GameObject::spanMask = 0;
float GameObject::blockCaptureRange = 0.0;
bool GameObject::initialize = false;

extern float maxVisualRange;
extern long	visualRangeTable[];

float applyDifficultyWeapon (float dmg, bool isPlayer);

//***************************************************************************
// WEAPONFIRECHUNK class
//***************************************************************************

#define	WEAPONFIRECHUNK_HIT_BITS			1
#define	WEAPONFIRECHUNK_WEAPON_BITS			5
#define	WEAPONFIRECHUNK_TARGETTYPE_BITS		2
#define	WEAPONFIRECHUNK_ENTRYQUAD_BITS		2
#define	WEAPONFIRECHUNK_HITLOCATION_BITS	4
#define	WEAPONFIRECHUNK_MOVERINDEX_BITS		7
#define	WEAPONFIRECHUNK_CELLPOS_BITS		10
#define	WEAPONFIRECHUNK_MISSILES_BITS		4
#define	WEAPONFIRECHUNK_TARGETID_BITS		20
#define	WEAPONFIRECHUNK_SPECIALTYPE_BITS	2
#define	WEAPONFIRECHUNK_SPECIALID_BITS		8

#define	WEAPONFIRECHUNK_HIT_MASK			0x00000001
#define	WEAPONFIRECHUNK_WEAPON_MASK			0x0000001F
#define	WEAPONFIRECHUNK_TARGETTYPE_MASK		0x00000003
#define	WEAPONFIRECHUNK_ENTRYQUAD_MASK		0x00000003
#define	WEAPONFIRECHUNK_HITLOCATION_MASK	0x0000000F
#define	WEAPONFIRECHUNK_MOVERINDEX_MASK		0x0000007F
#define	WEAPONFIRECHUNK_TERRAINBLOCK_MASK	0x000000FF
#define	WEAPONFIRECHUNK_TERRAINVERTEX_MASK	0x000001FF
#define	WEAPONFIRECHUNK_TERRAINITEM_MASK	0x00000007
#define	WEAPONFIRECHUNK_TRAIN_MASK			0x000000FF
#define	WEAPONFIRECHUNK_TRAINCAR_MASK		0x000000FF
#define	WEAPONFIRECHUNK_CELLPOS_MASK		0x000003FF
#define	WEAPONFIRECHUNK_MISSILES_MASK		0x0000000F
#define	WEAPONFIRECHUNK_TARGETID_MASK		0x000FFFFF
#define	WEAPONFIRECHUNK_SPECIALTYPE_MASK	0x00000003
#define	WEAPONFIRECHUNK_SPECIALID_MASK		0x000000FF

#define	WEAPONFIRECHUNK_TARGET_MOVER		0
#define	WEAPONFIRECHUNK_TARGET_TERRAIN		1
#define	WEAPONFIRECHUNK_TARGET_SPECIAL		2
#define	WEAPONFIRECHUNK_TARGET_LOCATION		3

#define	WEAPONFIRECHUNK_SPECIAL_CAMERADRONE	0

//---------------------------------------------------------------------------

void WeaponShotInfo::init (GameObjectWatchID _attackerWID, long _masterId, float _damage, long _hitLocation, float _entryAngle) {

	attackerWID = _attackerWID;
	masterId = _masterId;
	damage = _damage;
	hitLocation = _hitLocation;
	entryAngle = _entryAngle;
	if (!MPlayer && _attackerWID)		//No Multiplayer skill levels
	{
		//---------------------------------------------------
		// SKill Levels -- Need Attacker Alignment and Class
		GameObjectPtr _attacker = ObjectManager->getByWatchID(_attackerWID);
		if (_attacker)
		{
			ObjectClass objClass = _attacker->getObjectClass();
			long commanderId = _attacker->getCommanderId();
			bool isPlayer =  commanderId == Commander::home->getId();

			if (!isPlayer)		//Only enemy Mechs, vehicles, elementals and turrets get modified
			{
				if ((objClass == BATTLEMECH) ||
					(objClass == GROUNDVEHICLE) ||
					(objClass == TURRET))
				{
					damage = applyDifficultyWeapon(_damage,isPlayer);
				}
			}
			else
			{
				damage = applyDifficultyWeapon(_damage,isPlayer);
			}
		}
	}

	Assert((damage >= 0.0) && (damage <= 255.0), (long)damage, " WeaponShotInfo.init: damage out of range ");
	if (MPlayer && MPlayer->isServer()) {
		damage = (float)((unsigned long)(damage * 4.0)) * 0.25;
		if ((entryAngle >= -45.0) && (entryAngle <= 45.0))
			entryAngle = 0.0; //MECH_HIT_ARC_FRONT;
		else if ((entryAngle  > -135.0) && (entryAngle < -45.0))
			entryAngle = -90.0; //MECH_HIT_ARC_LEFT;
		else if ((entryAngle > 45.0) && (_entryAngle < 135))
			entryAngle = 90.0; //MECH_HIT_ARC_RIGHT;
		else
			entryAngle = 180.0; //MECH_HIT_ARC_REAR;
	}
}

//---------------------------------------------------------------------------

void WeaponShotInfo::setDamage (float _damage) {

	damage = _damage;
	if (MPlayer && MPlayer->isServer())
		damage = (float)((unsigned long)(damage * 4.0)) * 0.25;
}

//---------------------------------------------------------------------------

void WeaponShotInfo::setEntryAngle (float _entryAngle) {

	entryAngle = _entryAngle;
	if (MPlayer && MPlayer->isServer()) {
		if ((entryAngle >= -45.0) && (entryAngle <= 45.0))
			entryAngle = 0.0; //MECH_HIT_ARC_FRONT;
		else if ((entryAngle  > -135.0) && (entryAngle < -45.0))
			entryAngle = -90.0; //MECH_HIT_ARC_LEFT;
		else if ((entryAngle > 45.0) && (_entryAngle < 135))
			entryAngle = 90.0; //MECH_HIT_ARC_RIGHT;
		else
			entryAngle = 180.0; //MECH_HIT_ARC_REAR;
	}
}

//---------------------------------------------------------------------------

void* WeaponFireChunk::operator new (size_t ourSize) {

	void* result;
	result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void WeaponFireChunk::operator delete (void* us) {

	systemHeap->Free(us);
}
	

//---------------------------------------------------------------------------

void DebugWeaponFireChunk (WeaponFireChunkPtr chunk1, WeaponFireChunkPtr chunk2, GameObjectPtr attacker) {

	ChunkDebugMsg[0] = NULL;

	char outString[512];

	if (attacker) {
		if (attacker->isMover()) {
			sprintf(outString, "attacker = %s (%d)\n", attacker->getName(), attacker->getPartId());
			strcat(ChunkDebugMsg, outString);
			}
		else {
			sprintf(outString, "attacker = objClass %d (%d)\n", attacker->getObjectClass(), attacker->getPartId());
			strcat(ChunkDebugMsg, outString);
		}
		}
	else
		strcat(ChunkDebugMsg, "attacker = ???\n");

	if (chunk1) {
		strcat(ChunkDebugMsg, "\nCHUNK1\n");

		GameObjectPtr target = NULL;
		Stuff::Vector3D targetPoint;
		targetPoint.Zero();
		bool isTargetPoint = false;
		if (chunk1->targetType == WEAPONFIRECHUNK_TARGET_MOVER)
			target = (GameObjectPtr)MPlayer->moverRoster[chunk1->targetId];
		else if (chunk1->targetType == WEAPONFIRECHUNK_TARGET_TERRAIN)
			target = ObjectManager->findByPartId(chunk1->targetId);
		else if (chunk1->targetType == WEAPONFIRECHUNK_TARGET_SPECIAL)
			target = ObjectManager->findByPartId(chunk1->targetId);
		else if (chunk1->targetType == WEAPONFIRECHUNK_TARGET_LOCATION) {
			targetPoint.x = (float)chunk1->targetCell[1] * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
			targetPoint.y = (Terrain::worldUnitsMapSide / 2) - ((float)chunk1->targetCell[0] * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
			targetPoint.z = (float)land->getTerrainElevation(targetPoint);
			isTargetPoint = true;
		}

		if (target) {
			if (target->isMover()) {
				sprintf(outString, "target = %s (%d)\n", target->getName(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
				}
			else {
				sprintf(outString, "target = objClass %d (%d)\n", target->getObjectClass(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
			}
			}
		else if (isTargetPoint) {
			sprintf(outString, "target point = (%f, %f, %f)\n", targetPoint.x, targetPoint.y, targetPoint.z);
			strcat(ChunkDebugMsg, outString);
			}
		else {
			strcat(ChunkDebugMsg, "target = ???\n");
		}

		sprintf(outString, "targetType = %d\n", chunk1->targetType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "targetId = %d\n", chunk1->targetId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialType = %d\n", chunk1->specialType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialId = %d\n", chunk1->specialId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "targetCell = [%d, %d]\n", chunk1->targetCell[0], chunk1->targetCell[1]);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "weaponIndex = %d\n", chunk1->weaponIndex);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "hit = %c\n", chunk1->hit ? 'T' : 'N');
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "entryAngle = %d\n", chunk1->entryAngle);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "numMissiles = %d\n", chunk1->numMissiles);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "hitLocation = %d\n", chunk1->hitLocation);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "data = %x\n", chunk1->data);
		strcat(ChunkDebugMsg, outString);
	}

	if (chunk2) {
		strcat(ChunkDebugMsg, "\nCHUNK2\n");

		GameObjectPtr target = NULL;
		Stuff::Vector3D targetPoint;
		targetPoint.Zero();
		bool isTargetPoint = false;
		if (chunk2->targetType == WEAPONFIRECHUNK_TARGET_MOVER)
			target = (GameObjectPtr)MPlayer->moverRoster[chunk2->targetId];
		else if (chunk2->targetType == WEAPONFIRECHUNK_TARGET_TERRAIN)
			target = ObjectManager->findByPartId(chunk2->targetId);
		else if (chunk2->targetType == WEAPONFIRECHUNK_TARGET_SPECIAL)
			target = ObjectManager->findByPartId(chunk2->targetId);
		else if (chunk2->targetType == WEAPONFIRECHUNK_TARGET_LOCATION) {
			targetPoint.x = (float)chunk2->targetCell[1] * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
			targetPoint.y = (Terrain::worldUnitsMapSide / 2) - ((float)chunk2->targetCell[0] * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
			targetPoint.z = (float)land->getTerrainElevation(targetPoint);
			isTargetPoint = true;
		}

		if (target) {
			if (target->isMover()) {
				sprintf(outString, "target = %s (%d)\n", target->getName(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
				}
			else {
				sprintf(outString, "target = objClass %d (%d)\n", target->getObjectClass(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
			}
			}
		else if (isTargetPoint) {
			sprintf(outString, "target point = (%f, %f, %f)\n", targetPoint.x, targetPoint.y, targetPoint.z);
			strcat(ChunkDebugMsg, outString);
			}
		else {
			strcat(ChunkDebugMsg, "target = ???\n");
		}

		sprintf(outString, "targetType = %d\n", chunk2->targetType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "targetId = %d\n", chunk2->targetId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialType = %d\n", chunk2->specialType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialId = %d\n", chunk2->specialId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "targetCell = [%d, %d]\n", chunk2->targetCell[0], chunk1->targetCell[1]);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "weaponIndex = %d\n", chunk2->weaponIndex);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "hit = %c\n", chunk2->hit ? 'T' : 'N');
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "entryAngle = %d\n", chunk2->entryAngle);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "numMissiles = %d\n", chunk2->numMissiles);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "hitLocation = %d\n", chunk2->hitLocation);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "data = %x\n", chunk2->data);
		strcat(ChunkDebugMsg, outString);
	}

	File* debugFile = new File;
	debugFile->create("wfchunk.dbg");
	debugFile->writeString(ChunkDebugMsg);
	debugFile->close();
	delete debugFile;
	debugFile = NULL;

	ExceptionGameMsg = ChunkDebugMsg;
}

//---------------------------------------------------------------------------

#ifdef _DEBUG
#define	LOGWEAPONFIRECHUNKS
#define	WEAPONFIRELOG_SIZE		256

long NumWeaponFiresInLog = 0;
long NumWeaponFiresInLogQueue = 0;
GameObjectPtr WeaponFireAttackerLog[WEAPONFIRELOG_SIZE];
GameObjectPtr WeaponFireTargetLog[WEAPONFIRELOG_SIZE];
unsigned long WeaponFireDataLog[WEAPONFIRELOG_SIZE];
File* WeaponFireLog = NULL;
#endif

//---------------------------------------------------------------------------

#ifdef LOGWEAPONFIRECHUNKS
void DumpWeaponFireLog (void) {

	//----------------
	// Dump to file...
	for (long i = 0; i < NumWeaponFiresInLogQueue; i++) {
		GameObjectPtr attacker = WeaponFireAttackerLog[i];
		GameObjectPtr target = WeaponFireTargetLog[i];
		unsigned long data = WeaponFireDataLog[i];

		WeaponFireChunk chunk;
		chunk.init();
		chunk.data = data;
		chunk.unpack(attacker);

		char s[512];
		char attackerName[128];
		if (attacker->isMover())
			sprintf(attackerName, "%s (%d)", attacker->getName(), attacker->getPartId());
		else
			sprintf(attackerName, "objClass %d (%d)", attacker->getObjectClass(), attacker->getPartId());
		char targetName[128];
		if (target)
			if (target->isMover())
				sprintf(targetName, "%s (%d)", target->getName(), target->getPartId());
			else
				sprintf(targetName, "objClass %d (%d)", target->getObjectClass(), target->getPartId());
		else
			sprintf(targetName, "NA/LOCATION");

		sprintf(s, "attacker = %s, target = %s, data = %x, hit = %d, numMissiles = %d\n", attackerName, targetName, data, chunk.hit, chunk.numMissiles);

		WeaponFireLog->writeString(s);
	}
	NumWeaponFiresInLogQueue = 0;
}

//---------------------------------------------------------------------------

void CloseWeaponFireLog (void) {

	if (WeaponFireLog) {
		DumpWeaponFireLog();
		char s[512];
		sprintf(s, "\nNum Total WeaponFires = %d\n", NumWeaponFiresInLog);
		WeaponFireLog->writeString(s);
		WeaponFireLog->close();
		delete WeaponFireLog;
		WeaponFireLog = NULL;
		NumWeaponFiresInLog = 0;
		NumWeaponFiresInLogQueue = 0;
	}
}
#endif

//---------------------------------------------------------------------------

void OpenWeaponFireLog (void) {
#ifdef LOGWEAPONFIRECHUNKS
	if (WeaponFireLog)
		CloseWeaponFireLog();

	NumWeaponFiresInLog = 0;
	NumWeaponFiresInLogQueue = 0;
	WeaponFireLog = new File;
	if (!WeaponFireLog)
		Fatal(0, " unable to malloc WeaponFireLog ");
	if (WeaponFireLog->create("wf.log") != NO_ERR)
		Fatal(0, " unable to create WeaponFireLog ");
#endif
}

//---------------------------------------------------------------------------

void LogWeaponFireChunk (WeaponFireChunkPtr chunk, GameObjectPtr attacker, GameObjectPtr target) {
#ifdef LOGWEAPONFIRECHUNKS

	if (!WeaponFireLog)
		return;

	if (NumWeaponFiresInLogQueue == WEAPONFIRELOG_SIZE)
		DumpWeaponFireLog();

	WeaponFireAttackerLog[NumWeaponFiresInLogQueue] = attacker;
	WeaponFireTargetLog[NumWeaponFiresInLogQueue] = target;
	WeaponFireDataLog[NumWeaponFiresInLogQueue] = chunk->data;
	NumWeaponFiresInLog++;
	NumWeaponFiresInLogQueue++;
#endif
}

//---------------------------------------------------------------------------

void WeaponFireChunk::buildMoverTarget (GameObjectPtr target,
										long _weaponIndex,
										bool _hit,
										float _entryAngle,
										long _numMissiles,
										long _hitLocation) {

	targetType = WEAPONFIRECHUNK_TARGET_MOVER;
	targetId = ((MoverPtr)target)->getNetRosterIndex();
	weaponIndex = _weaponIndex;
	hit = _hit;
	if ((_entryAngle >= -45.0) && (_entryAngle <= 45.0))
		entryAngle = 0; //MECH_HIT_ARC_FRONT;
	else if ((_entryAngle  > -135.0) && (_entryAngle < -45.0))
		entryAngle = 2; //MECH_HIT_ARC_LEFT;
	else if ((_entryAngle > 45.0) && (_entryAngle < 135))
		entryAngle = 3; //MECH_HIT_ARC_RIGHT;
	else
		entryAngle = 1; //MECH_HIT_ARC_REAR;
	numMissiles = _numMissiles;
	hitLocation = _hitLocation;

	Assert((targetId > -1) && (targetId < MAX_MULTIPLAYER_MOVERS), targetId, " WeaponFireChunk.buildMoverTarget: bad targetId ");
	Assert((weaponIndex > -1) && (weaponIndex < 32), weaponIndex, " WeaponFireChunk.buildMoverTarget: bad weaponIndex ");
	Assert((numMissiles > -1) && (numMissiles < 16), numMissiles, " WeaponFireChunk.buildMoverTarget: bad numMissiles ");
	Assert((hitLocation > -2) && (hitLocation < 12), hitLocation, " WeaponFireChunk.buildMoverTarget: bad hitLocation ");

	data = 0;
}

//---------------------------------------------------------------------------

void WeaponFireChunk::buildTerrainTarget (GameObjectPtr target,
										  long _weaponIndex,
										  bool _hit,
										  long _numMissiles) {

	targetType = WEAPONFIRECHUNK_TARGET_TERRAIN;
	targetId = target->getPartId();

	//target->getCellPosition(targetCell[0], targetCell[1]);

	weaponIndex = _weaponIndex;
	hit = _hit;
	numMissiles = _numMissiles;

	Assert(target->getPartId() != -1, target->getObjectClass(), " WeaponFireChunk.buildTerrainTarget: -1 partId ");

	data = 0;
}

//---------------------------------------------------------------------------

void WeaponFireChunk::buildCameraDroneTarget (GameObjectPtr target,
											  long _weaponIndex,
											  bool _hit,
											  float _entryAngle,
											  long _numMissiles) {

	targetType = WEAPONFIRECHUNK_TARGET_SPECIAL;
	targetId = target->getPartId();

	targetCell[0] = 128;
	targetCell[1] = targetId - MIN_CAMERA_DRONE_ID;

	weaponIndex = _weaponIndex;
	hit = _hit;
	if ((_entryAngle >= -45.0) && (_entryAngle <= 45.0))
		entryAngle = 0; //MECH_HIT_ARC_FRONT;
	else if ((_entryAngle  > -135.0) && (_entryAngle < -45.0))
		entryAngle = 2; //MECH_HIT_ARC_LEFT;
	else if ((_entryAngle > 45.0) && (_entryAngle < 135))
		entryAngle = 3; //MECH_HIT_ARC_RIGHT;
	else
		entryAngle = 1; //MECH_HIT_ARC_REAR;
	numMissiles = _numMissiles;
	data = 0;
}

//---------------------------------------------------------------------------

void WeaponFireChunk::buildLocationTarget (Stuff::Vector3D location,
										   long _weaponIndex,
										   bool _hit,
										   long _numMissiles) {

	targetType = WEAPONFIRECHUNK_TARGET_LOCATION;
	land->worldToCell(location, targetCell[0], targetCell[1]);
	weaponIndex = _weaponIndex;
	hit = _hit;
	numMissiles = _numMissiles;

	data = 0;
}

//---------------------------------------------------------------------------

#ifdef _DEBUG
#define DEBUG_WEAPONFIRECHUNK
#endif

void WeaponFireChunk::pack (GameObjectPtr attacker) {

	data = 0;

	switch (targetType) {
		case WEAPONFIRECHUNK_TARGET_MOVER:
			//------------------------
			// Mover Target...
			// current size is 23 bits
			data |= entryAngle;
			data <<= WEAPONFIRECHUNK_HITLOCATION_BITS;
			data |= (hitLocation + 2);
			data <<= WEAPONFIRECHUNK_MOVERINDEX_BITS;
			data |= targetId;
			if (numMissiles > 0) {
				//------------------------------
				// Weapon is a missile weapon...
				data <<= WEAPONFIRECHUNK_MISSILES_BITS;
				data |= numMissiles;
			}
			data <<= WEAPONFIRECHUNK_HIT_BITS;
			break;
		case WEAPONFIRECHUNK_TARGET_TERRAIN:
			//-------------------------
			// Terrain Object Target...
			// current size is 
			data |= (targetId - MIN_TERRAIN_PART_ID);
			if (numMissiles > 0) {
				//------------------------------
				// Weapon is a missile weapon...
				data <<= WEAPONFIRECHUNK_MISSILES_BITS;
				data |= numMissiles;
			}
			data <<= WEAPONFIRECHUNK_HIT_BITS;
			break;
		case WEAPONFIRECHUNK_TARGET_SPECIAL:
			data |= entryAngle;
			data <<= WEAPONFIRECHUNK_CELLPOS_BITS;
			data |= targetCell[0];
			data <<= WEAPONFIRECHUNK_CELLPOS_BITS;
			data |= targetCell[1];
			if (numMissiles > 0) {
				//------------------------------
				// Weapon is a missile weapon...
				data <<= WEAPONFIRECHUNK_MISSILES_BITS;
				data |= numMissiles;
			}
			data <<= WEAPONFIRECHUNK_HIT_BITS;
			break;
		case WEAPONFIRECHUNK_TARGET_LOCATION:
			//-----------------------------------------
			// Must be a Location Target (or a Miss)...
			data |= targetCell[0];
			data <<= WEAPONFIRECHUNK_CELLPOS_BITS;
			data |= targetCell[1];
			if (numMissiles > 0) {
				//------------------------------
				// Weapon is a missile weapon...
				data <<= WEAPONFIRECHUNK_MISSILES_BITS;
				data |= numMissiles;
			}
			data <<= WEAPONFIRECHUNK_HIT_BITS;
			break;
	}
	if (hit)
		data |= 1;
	data <<= WEAPONFIRECHUNK_WEAPON_BITS;
	data |= weaponIndex;
	data <<= WEAPONFIRECHUNK_TARGETTYPE_BITS;
	data |= targetType;

#ifdef DEBUG_WEAPONFIRECHUNK
	//-------------------------
	// Lots'a error checking...
	if ((targetType < 0) || (targetType > 3)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.pack: bad targetType %d (save wfchunk.dbg file) ", targetType);
		Assert(false, targetType, errMsg);
	}
	if ((weaponIndex < 0) || (weaponIndex > 31)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.pack: bad weaponIndex %d (save wfchunk.dbg file) ", weaponIndex);
		Assert(false, weaponIndex, errMsg);
	}
	if ((hitLocation < -1) || (hitLocation >= 12)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.pack: bad hitLocation %d (save wfchunk.dbg file) ", hitLocation);
		Assert(false, hitLocation, errMsg);
	}
	if ((entryAngle < 0) || (entryAngle > 3)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.pack: bad entryAngle %d (save wfchunk.dbg file) ", entryAngle);
		Assert(false, entryAngle, errMsg);
	}
	if ((numMissiles < 0) || (numMissiles > 15)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.pack: bad numMissiles %d (save wfchunk.dbg file) ", numMissiles);
		Assert(false, numMissiles, errMsg);
	}
#endif

}

//---------------------------------------------------------------------------

void WeaponFireChunk::unpack (GameObjectPtr attacker) {

	unsigned long tempData = data;

	targetType = (tempData & WEAPONFIRECHUNK_TARGETTYPE_MASK);
	tempData >>= WEAPONFIRECHUNK_TARGETTYPE_BITS;

	weaponIndex = (tempData & WEAPONFIRECHUNK_WEAPON_MASK);
	tempData >>= WEAPONFIRECHUNK_WEAPON_BITS;

	hit = ((tempData & WEAPONFIRECHUNK_HIT_MASK) != 0) ? true : false;
	tempData >>= WEAPONFIRECHUNK_HIT_BITS;

	bool isMissileWeapon = false;
	if (attacker->isMover()) {
		long itemIndex = ((MoverPtr)attacker)->numOther + weaponIndex;
		isMissileWeapon = ((MoverPtr)attacker)->isWeaponMissile(itemIndex);
		}
	else if (attacker->getObjectClass() == TURRET) {
		//----------------------
		// Attacker is turret...
		isMissileWeapon = ((Turret*)attacker)->isWeaponMissile(weaponIndex);
	}

	switch (targetType) {
		case WEAPONFIRECHUNK_TARGET_MOVER:
			//----------------
			// Mover Target...
			if (isMissileWeapon) {
				numMissiles = (tempData & WEAPONFIRECHUNK_MISSILES_MASK);
				tempData >>= WEAPONFIRECHUNK_MISSILES_BITS;
			}

			targetId = (tempData & WEAPONFIRECHUNK_MOVERINDEX_MASK);
			tempData >>= WEAPONFIRECHUNK_MOVERINDEX_BITS;

			hitLocation = ((tempData & WEAPONFIRECHUNK_HITLOCATION_MASK) - 2);
			tempData >>= WEAPONFIRECHUNK_HITLOCATION_BITS;

			entryAngle = (tempData & WEAPONFIRECHUNK_ENTRYQUAD_MASK);
			break;
		case WEAPONFIRECHUNK_TARGET_TERRAIN:
			//-------------------------
			// Terrain Object Target...
			if (isMissileWeapon) {
				numMissiles = (tempData & WEAPONFIRECHUNK_MISSILES_MASK);
				tempData >>= WEAPONFIRECHUNK_MISSILES_BITS;
			}
			targetId = MIN_TERRAIN_PART_ID + (tempData & WEAPONFIRECHUNK_TARGETID_MASK);
			tempData >>= WEAPONFIRECHUNK_TARGETID_BITS;
			break;
		case WEAPONFIRECHUNK_TARGET_SPECIAL:
			//-----------------------
			// Special (Train, CameraDrone) Object Target...
			if (isMissileWeapon) {
				numMissiles = (tempData & WEAPONFIRECHUNK_MISSILES_MASK);
				tempData >>= WEAPONFIRECHUNK_MISSILES_BITS;
			}

			specialId = (tempData & WEAPONFIRECHUNK_SPECIALID_MASK);
			tempData >>= WEAPONFIRECHUNK_SPECIALID_BITS;

			specialType = (tempData & WEAPONFIRECHUNK_SPECIALTYPE_MASK);
			tempData >>= WEAPONFIRECHUNK_SPECIALTYPE_BITS;

			switch (specialType) {
				case WEAPONFIRECHUNK_SPECIAL_CAMERADRONE:
					targetId = MIN_CAMERA_DRONE_ID + specialId;
					break;
				default:
					Fatal(specialType, " WeaponFireChunk.unpack: bad specialType ");
			}

			entryAngle = (tempData & WEAPONFIRECHUNK_ENTRYQUAD_MASK);
			break;
		case WEAPONFIRECHUNK_TARGET_LOCATION:
			//-----------------------------------------
			// Must be a Location Target (or a Miss)...
			if (isMissileWeapon) {
				numMissiles = (tempData & WEAPONFIRECHUNK_MISSILES_MASK);
				tempData >>= WEAPONFIRECHUNK_MISSILES_BITS;
			}

			targetCell[1] = (tempData & WEAPONFIRECHUNK_CELLPOS_MASK);
			tempData >>= WEAPONFIRECHUNK_CELLPOS_BITS;
					
			targetCell[0] = (tempData & WEAPONFIRECHUNK_CELLPOS_MASK);
			break;
	}

	//-------------------------
	// Lots'a error checking...
#ifdef DEBUG_WEAPONFIRECHUNK
	//-------------------------
	// Lots'a error checking...
	if ((targetType < 0) || (targetType > 3)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.unpack: bad targetType %d (save wfchunk.dbg file) ", targetType);
		Assert(false, targetType, errMsg);
	}
	if ((weaponIndex < 0) || (weaponIndex > 31)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.unpack: bad weaponIndex %d (save wfchunk.dbg file) ", weaponIndex);
		Assert(false, weaponIndex, errMsg);
	}
	if ((hitLocation < -1) || (hitLocation >= 12)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.unpack: bad hitLocation %d (save wfchunk.dbg file) ", hitLocation);
		Assert(false, hitLocation, errMsg);
	}
	if ((entryAngle < 0) || (entryAngle > 3)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.unpack: bad entryAngle %d (save wfchunk.dbg file) ", entryAngle);
		Assert(false, entryAngle, errMsg);
	}
	if ((numMissiles < 0) || (numMissiles > 15)) {
		DebugWeaponFireChunk(this, NULL, attacker);
		char errMsg[1024];
		sprintf(errMsg, " WeaponFireChunk.unpack: bad numMissiles %d (save wfchunk.dbg file) ", numMissiles);
		Assert(false, numMissiles, errMsg);
	}
	if (!hit) {
		bool isStreakMissile = false;
		if (attacker->isMover()) {
			long itemIndex = ((MoverPtr)attacker)->numOther + weaponIndex;
			isStreakMissile = MasterComponent::masterList[((MoverPtr)attacker)->inventory[itemIndex].masterID].getWeaponStreak();
			}
		else if (attacker->getObjectClass() == TURRET) {
			//----------------------
			// Attacker is turret...
			isStreakMissile = ((TurretPtr)attacker)->isWeaponStreak(weaponIndex);
		}
		if (isStreakMissile) {
			DebugWeaponFireChunk(this, NULL, attacker);
			Assert(false, 0, " WeaponFireChunk.unpack: streak missile missed (save wfchunk.dbg file) ");
		}
	}
	GameObjectPtr target = NULL;
	if (targetType == 0 /*WEAPONFIRECHUNK_TARGET_MOVER*/) {
		target = (GameObjectPtr)MPlayer->moverRoster[targetId];
		//----------------------------------------------------------------------------
		// Mover targets could be NULL now, since we free them when they're destroyed.
		//if (target == NULL) {
		//	DebugWeaponFireChunk (this, NULL, attacker);
		//	Assert(false, 0, " WeaponFireChunk.unpack: NULL Mover Target (save wfchunk.dbg file) ");
		//}
		}
	else if (targetType == 1 /*WEAPONFIRECHUNK_TARGET_TERRAIN*/) {
		target = ObjectManager->findByPartId(targetId);
		if (target == NULL) {
			DebugWeaponFireChunk (this, NULL, attacker);
			Assert(false, 0, " WeaponFireChunk.unpack: NULL Terrain Target (save wfchunk.dbg file) ");
		}
		}
	else if (targetType == 2 /*WEAPONFIRECHUNK_TARGET_TRAIN*/) {
		target = ObjectManager->findByPartId(targetId);
		if (target == NULL) {
			DebugWeaponFireChunk (this, NULL, attacker);
			Assert(false, 0, " WeaponFireChunk.unpack: NULL Special Target (save wfchunk.dbg file) ");
		}
		}
	else if (targetType == 3 /*WEAPONFIRECHUNK_TARGET_LOCATION*/) {
		// Do nothing...
	}
#endif
}

//---------------------------------------------------------------------------

bool WeaponFireChunk::equalTo (WeaponFireChunkPtr chunk) {

	if (targetType != chunk->targetType) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (targetId != chunk->targetId) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (targetCell[0] != chunk->targetCell[0]) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (targetCell[1] != chunk->targetCell[1]) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (specialType != chunk->specialType) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (specialId != chunk->specialId) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (weaponIndex != chunk->weaponIndex) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (hit != chunk->hit) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (entryAngle != chunk->entryAngle) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (numMissiles != chunk->numMissiles) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	if (hitLocation != chunk->hitLocation) {
		DebugWeaponFireChunk(this, chunk, NULL);
		return(false);
	}

	return(true);
}

//***************************************************************************
// WEAPONHITCHUNK class
//***************************************************************************

void DebugWeaponHitChunk (WeaponHitChunkPtr chunk1, WeaponHitChunkPtr chunk2) {

	ChunkDebugMsg[0] = NULL;

	char outString[512];

	if (chunk1) {
		strcat(ChunkDebugMsg, "\nCHUNK1\n");

		GameObjectPtr target = NULL;
		bool isTargetPoint = false;
		if (chunk1->targetType == WEAPONHITCHUNK_TARGET_MOVER)
			target = (GameObjectPtr)MPlayer->moverRoster[chunk1->targetId];
		else if (chunk1->targetType == WEAPONHITCHUNK_TARGET_TERRAIN)
			target = ObjectManager->findByPartId(chunk1->targetId);
		else if (chunk1->targetType == WEAPONHITCHUNK_TARGET_SPECIAL)
			target = ObjectManager->findByPartId(chunk1->targetId);
		else if (chunk1->targetType == WEAPONHITCHUNK_TARGET_LOCATION)
			isTargetPoint = true;

		if (target) {
			if (target->isMover()) {
				sprintf(outString, "target = %s (%d)\n", target->getName(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
				}
			else {
				sprintf(outString, "target = objClass %d (%d)\n", target->getObjectClass(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
			}
			}
		else if (isTargetPoint)
			strcat(ChunkDebugMsg, "target point\n");
		else
			strcat(ChunkDebugMsg, "target = ???\n");

		sprintf(outString, "targetType = %d\n", chunk1->targetType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "targetId = %d\n", chunk1->targetId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialType = %d\n", chunk1->specialType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialId = %d\n", chunk1->specialId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "cause = %d\n", chunk1->cause);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "damage = %f\n", chunk1->damage);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "hitLocation = %d\n", chunk1->hitLocation);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "entryAngle = %d\n", chunk1->entryAngle);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "refit = %s\n", chunk1->refit ? "true" : "false");
		strcat(ChunkDebugMsg, outString);
	}

	if (chunk2) {
		strcat(ChunkDebugMsg, "\nCHUNK2\n");

		GameObjectPtr target = NULL;
		bool isTargetPoint = false;
		if (chunk2->targetType == WEAPONHITCHUNK_TARGET_MOVER)
			target = (GameObjectPtr)MPlayer->moverRoster[chunk2->targetId];
		else if (chunk2->targetType == WEAPONHITCHUNK_TARGET_TERRAIN)
			target = ObjectManager->findByPartId(chunk2->targetId);
		else if (chunk2->targetType == WEAPONHITCHUNK_TARGET_SPECIAL)
			target = ObjectManager->findByPartId(chunk2->targetId);
		else if (chunk2->targetType == WEAPONHITCHUNK_TARGET_LOCATION)
			isTargetPoint = true;

		if (target) {
			if (target->isMover()) {
				sprintf(outString, "target = %s (%d)\n", target->getName(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
				}
			else {
				sprintf(outString, "target = objClass %d (%d)\n", target->getObjectClass(), target->getPartId());
				strcat(ChunkDebugMsg, outString);
			}
			}
		else if (isTargetPoint)
			strcat(ChunkDebugMsg, "target point");
		else
			strcat(ChunkDebugMsg, "target = ???\n");

		sprintf(outString, "targetType = %d\n", chunk2->targetType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "targetId = %d\n", chunk2->targetId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialType = %d\n", chunk2->specialType);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "specialId = %d\n", chunk2->specialId);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "cause = %d\n", chunk2->cause);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "damage = %f\n", chunk2->damage);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "hitLocation = %d\n", chunk2->hitLocation);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "entryAngle = %d\n", chunk2->entryAngle);
		strcat(ChunkDebugMsg, outString);

		sprintf(outString, "refit = %s\n", chunk2->refit ? "true" : "false");
		strcat(ChunkDebugMsg, outString);
	}

	File* debugFile = new File;
	debugFile->create("whchunk.dbg");
	debugFile->writeString(ChunkDebugMsg);
	debugFile->close();
	delete debugFile;
	debugFile = NULL;

	ExceptionGameMsg = ChunkDebugMsg;
}

//---------------------------------------------------------------------------

void* WeaponHitChunk::operator new (size_t ourSize) {

	void* result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void WeaponHitChunk::operator delete (void* us) {

	systemHeap->Free(us);
}	

//---------------------------------------------------------------------------

void WeaponHitChunk::buildMoverTarget (GameObjectPtr target,
									   long _cause,
									   float _damage,
									   long _hitLocation,
									   float _entryAngle,
									   bool isRefit) {

	targetType = WEAPONHITCHUNK_TARGET_MOVER;
	targetId = ((MoverPtr)target)->getNetRosterIndex();
	//Assert((targetId > -1) && (targetId < MPlayer->numMovers), targetId, " WeaponHitChunk.buildMoverTarget: bad targetId ");
	cause = _cause;
	//Assert((cause
	damage = _damage;
	//Assert((damage >= ) && (damage < 1024), damage, " WeaponHitChunk.buildMoverTarget: bad damage ");
	hitLocation = _hitLocation;
	//Assert((hitLocation > -2) && (hitLocation < 12), hitLocation, " WeaponHitChunk.buildMoverTarget: bad hitLocation ");
	if ((_entryAngle >= -45.0) && (_entryAngle <= 45.0))
		entryAngle = 0; //MECH_HIT_ARC_FRONT;
	else if ((_entryAngle  > -135.0) && (_entryAngle < -45.0))
		entryAngle = 2; //MECH_HIT_ARC_LEFT;
	else if ((_entryAngle > 45.0) && (_entryAngle < 135))
		entryAngle = 3; //MECH_HIT_ARC_RIGHT;
	else
		entryAngle = 1; //MECH_HIT_ARC_REAR;
	refit = isRefit;

	data = 0;
}

//---------------------------------------------------------------------------

void WeaponHitChunk::buildTerrainTarget (GameObjectPtr target,
										 float _damage) {

	targetType = WEAPONHITCHUNK_TARGET_TERRAIN;
	targetId = target->getPartId();

	damage = _damage;

	data = 0;
}

//---------------------------------------------------------------------------

void WeaponHitChunk::buildCameraDroneTarget (GameObjectPtr target,
											 float _damage,
											 float _entryAngle) {

	targetType = WEAPONHITCHUNK_TARGET_SPECIAL;
	targetId = target->getPartId();

	specialType = WEAPONFIRECHUNK_SPECIAL_CAMERADRONE;
	specialId = targetId - MIN_CAMERA_DRONE_ID;

	damage = _damage;
	//Assert((damage >= ) && (damage < 1024), damage, " WeaponHitChunk.buildMoverTarget: bad damage ");

	if ((_entryAngle >= -45.0) && (_entryAngle <= 45.0))
		entryAngle = 0; //MECH_HIT_ARC_FRONT;
	else if ((_entryAngle  > -135.0) && (_entryAngle < -45.0))
		entryAngle = 2; //MECH_HIT_ARC_LEFT;
	else if ((_entryAngle > 45.0) && (_entryAngle < 135))
		entryAngle = 3; //MECH_HIT_ARC_RIGHT;
	else
		entryAngle = 1; //MECH_HIT_ARC_REAR;

	data = 0;
}

//---------------------------------------------------------------------------

void WeaponHitChunk::build (GameObjectPtr target, WeaponShotInfoPtr shotInfo, bool isRefit) {

	if (!target)
		Fatal(0, " WeaponHitChunk.build: NULL target ");
	Assert(((float)((unsigned long)(shotInfo->damage * 4.0)) * 0.25) == shotInfo->damage, 0, " WeaponHitChunk.build: damage round error ");
	if (target->isMover()) {
		//---------------------------------------------------------------
		// HACK fix for ammoExplosions without needing to save the weapon
		// master ID...
		if (shotInfo->masterId > 0) {
			if (MasterComponent::masterList[shotInfo->masterId].getForm() == COMPONENT_FORM_AMMO)
				shotInfo->masterId = -4;
			else
				shotInfo->masterId = 0;
		}
		buildMoverTarget(target,
						 shotInfo->masterId,
						 shotInfo->damage,
						 shotInfo->hitLocation,
						 shotInfo->entryAngle,
						 isRefit);
		}
	else {
		switch (target->getObjectClass()) {
			case BUILDING:
			case TREEBUILDING:
			case TREE:
			case GATE:
			case TURRET:
			case BRIDGE:
				buildTerrainTarget(target, shotInfo->damage);
				break;
			case CAMERADRONE:
				buildCameraDroneTarget(target, shotInfo->damage, shotInfo->entryAngle);
				break;
			default:
				//Fatal(0, " WeaponHitChunk.build: bad target type ");
				break;
		}
	}
}

//---------------------------------------------------------------------------

void WeaponHitChunk::pack (void) {

	data = 0;

	switch (targetType) {
		case WEAPONHITCHUNK_TARGET_MOVER:
			//----------------
			// Mover Target...
			if (refit)
				data |= 1;
			data <<= WEAPONHITCHUNK_ENTRYQUAD_BITS;

			data |= entryAngle;
			data <<= WEAPONHITCHUNK_HITLOCATION_BITS;

			data |= (hitLocation + 2);
			data <<= WEAPONHITCHUNK_CAUSE_BITS;

			data |= (cause + 7);
			data <<= WEAPONHITCHUNK_MOVERINDEX_BITS;

			data |= targetId;
			data <<= WEAPONHITCHUNK_DAMAGE_BITS;
			break;
		case WEAPONHITCHUNK_TARGET_TERRAIN:
			//-------------------------
			// Terrain Object Target...
			data |= (targetId - MIN_TERRAIN_PART_ID);
			data <<= WEAPONHITCHUNK_DAMAGE_BITS;
			break;
		case WEAPONHITCHUNK_TARGET_SPECIAL:
			data |= entryAngle;
			data <<= WEAPONHITCHUNK_SPECIALTYPE_BITS;

			data |= specialType;
			data <<= WEAPONHITCHUNK_SPECIALID_BITS;

			data |= specialId;
			data <<= WEAPONHITCHUNK_DAMAGE_BITS;
			break;
		default:
			Fatal(0, " Bad WeaponHitChunk Target Type ");
	}

	data |= (unsigned long)(damage * 4.0);
	data <<= WEAPONHITCHUNK_TARGETTYPE_BITS;

	data |= targetType;
}

//---------------------------------------------------------------------------
		
void WeaponHitChunk::unpack (void) {

	unsigned long tempData = data;

	targetType = (tempData & WEAPONHITCHUNK_TARGETTYPE_MASK);
	tempData >>= WEAPONHITCHUNK_TARGETTYPE_BITS;

	damage = (float)(tempData & WEAPONHITCHUNK_DAMAGE_MASK) * 0.25;
	tempData >>= WEAPONHITCHUNK_DAMAGE_BITS;
	Assert((damage >= 0.0) && (damage <= 255.0), 0, " WeaponHitChunk.unpack: bad damage ");

	switch (targetType) {
		case WEAPONHITCHUNK_TARGET_MOVER:
			//----------------
			// Mover Target...
			targetId = (tempData & WEAPONHITCHUNK_MOVERINDEX_MASK);
			tempData >>= WEAPONHITCHUNK_MOVERINDEX_BITS;
			Assert((targetId > -1) && (targetId < MAX_MULTIPLAYER_MOVERS), targetId, " WeaponHitChunk.unpack: bad targetId ");

			cause = ((tempData & WEAPONHITCHUNK_CAUSE_MASK) - 7);
			tempData >>= WEAPONHITCHUNK_CAUSE_BITS;
			Assert((cause >= -7) && (cause <= 0), cause, " WeaponHitChunk.unpack: bad cause ");

			hitLocation = ((tempData & WEAPONHITCHUNK_HITLOCATION_MASK) - 2);
			tempData >>= WEAPONHITCHUNK_HITLOCATION_BITS;
			Assert((hitLocation > -2) && (hitLocation < 12), hitLocation, " WeaponHitChunk.unpack: bad hitLocation ");

			entryAngle = (tempData & WEAPONHITCHUNK_ENTRYQUAD_MASK);
			tempData >>= WEAPONHITCHUNK_ENTRYQUAD_BITS;

			refit = (tempData & WEAPONHITCHUNK_REFIT_MASK) ? true : false;

			break;
		case WEAPONHITCHUNK_TARGET_TERRAIN:
			//-------------------------
			// Terrain Object Target...
			targetId = MIN_TERRAIN_PART_ID + (tempData & WEAPONHITCHUNK_TARGETID_MASK);
			tempData >>= WEAPONHITCHUNK_TARGETID_BITS;
			break;
		case WEAPONHITCHUNK_TARGET_SPECIAL:
			//-----------------------
			// Train Object Target...
			specialId = (tempData & WEAPONHITCHUNK_SPECIALID_MASK);
			tempData >>= WEAPONHITCHUNK_SPECIALID_BITS;

			specialType = (tempData & WEAPONHITCHUNK_SPECIALTYPE_MASK);
			tempData >>= WEAPONHITCHUNK_SPECIALTYPE_BITS;

			switch (specialType) {
				case WEAPONHITCHUNK_SPECIAL_CAMERADRONE:
					targetId = MIN_CAMERA_DRONE_ID + specialId;
					break;
				default:
					Fatal(specialType, " WeaponHitChunk.unpack: bad specialType ");
			}
			entryAngle = (tempData & WEAPONHITCHUNK_ENTRYQUAD_MASK);
			break;
		default:
			DebugWeaponHitChunk(this, NULL);
			Fatal(0, " Bad WeaponHitChunk Target Type ");
	}
}

//---------------------------------------------------------------------------

bool WeaponHitChunk::equalTo (WeaponHitChunkPtr chunk) {

	if (targetType != chunk->targetType) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (targetId != chunk->targetId) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (specialType != chunk->specialType) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (specialId != chunk->specialId) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (cause != chunk->cause) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (damage != chunk->damage) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (entryAngle != chunk->entryAngle) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (refit != chunk->refit) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	if (hitLocation != chunk->hitLocation) {
		DebugWeaponHitChunk(this, chunk);
		return(false);
	}

	return(true);
}

//---------------------------------------------------------------------------

bool WeaponHitChunk::valid (long from) {

	if (refit) {
		if (targetType != 0)
			Fatal(0, " Multiplayer.handleAppWeaponHitUpdate: bad targetType for refit ");
		MoverPtr target = MPlayer->moverRoster[targetId];
		if (!target) {
			if (CombatLog)
				CombatLog->write("WeaponHitChunk INVALID: refit has NULL target");
			DebugWeaponHitChunk(this, NULL);
			return(false);
		}
		}
	else {
		WeaponShotInfo shotInfo;
		shotInfo.attackerWID = 0;
		shotInfo.masterId = cause;
		shotInfo.damage = damage;
		shotInfo.hitLocation = hitLocation;
		//shotInfo.entryAngle = entryQuadTable[entryAngle];
		switch (targetType) {
			case WEAPONHITCHUNK_TARGET_MOVER:
				#ifdef _DEBUG
				//if (WeaponHitLog)
				//	WeaponHitLog->writeString("==>handleAppWeaponHit\n");
				#endif
				//FromMP = true;
				//if (moverRoster[chunk.targetId])
				//	moverRoster[chunk.targetId]->handleWeaponHit(&shotInfo);
				//FromMP = false;
				break;
			case WEAPONHITCHUNK_TARGET_TERRAIN:
			case WEAPONHITCHUNK_TARGET_SPECIAL: {
				GameObjectPtr target = ObjectManager->findByPartId(targetId);
				if (!target) {
					char s[512];
					long r = (targetId - MIN_TERRAIN_PART_ID) / MAX_MAP_CELL_WIDTH;
					long c = targetId - MIN_TERRAIN_PART_ID - (MAX_MAP_CELL_WIDTH * r);
					sprintf(s, "WeaponHitChunk INVALID: NULL terrain target (%d), rc = %d,%d, type = %d, damage = %.2f, data=%d", targetId, r, c, targetType, damage, data);
					if (CombatLog) {
						CombatLog->write(s);
						CombatLog->dump();
						//CombatLog->close();
					}
					DebugWeaponHitChunk(this, NULL);
					return(false);
				}
				}
				break;
			default:
				if (CombatLog) {
					CombatLog->write("WeaponHitChunk INVALID: bad targetType");
					CombatLog->dump();
				}
				DebugWeaponHitChunk(this, NULL);
				return(false);
		}
	}

	return(true);
}

//---------------------------------------------------------------------------
// class GameObject	
//---------------------------------------------------------------------------

void* GameObject::operator new (size_t ourSize) {

	void* result = ObjectTypeManager::objectCache->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void GameObject::operator delete (void* us) {

	ObjectTypeManager::objectCache->Free(us);
}	

//---------------------------------------------------------------------------

void GameObject::init (bool create) {

	objectClass = GAMEOBJECT;
	if (initialize) {
		handle = 0;
		appearance = NULL;
		threatRating = 0;
	}
	partId = 0;
	watchID = 0;
	typeHandle = 0;
	position.Zero();
	cellPositionRow = 0;
	cellPositionCol = 0;
	flags = OBJECT_FLAG_USEME | OBJECT_FLAG_AWAKE;
	debugFlags = 0;
	status = OBJECT_STATUS_NORMAL;
	tonnage = 0.0;
	d_vertexNum = -1;
	//team = 255;
	collisionFreeFromWID = 0;
	collisionFreeTime = 0.0;
	screenPos.x = 0.0f;
	screenPos.y = 0.0f;
	screenPos.z = 0.0f;
	screenPos.w = 0.0f;
	windowsVisible = 0;
	explDamage = 0.0;
	explRadius = 0.0;
	maxCV = 0;
	curCV = 0;
	lastFrameTime = 0.0;
	blipFrame = 0;
	numAttackers = 0;
	rotation = 0.0;
	drawFlags = 0;
}

//---------------------------------------------------------------------------

void GameObject::set (GameObject copy) {

	objectClass = copy.objectClass;
	handle = copy.handle;
	partId = copy.partId;
	watchID = copy.watchID;
	typeHandle = copy.typeHandle;
	position = copy.position;
	cellPositionRow = copy.cellPositionRow;
	cellPositionCol = copy.cellPositionCol;
	flags = copy.flags;
	debugFlags = copy.debugFlags;
	status = copy.status;
	tonnage = copy.tonnage;
	appearance = copy.appearance;
	d_vertexNum = copy.d_vertexNum;
	collisionFreeFromWID = copy.collisionFreeFromWID;
	collisionFreeTime = copy.collisionFreeTime;
	screenPos = copy.screenPos;
	windowsVisible = copy.windowsVisible;
	explRadius = copy.explRadius;
	explDamage = copy.explDamage;
	maxCV = copy.maxCV;
	curCV = copy.curCV;
	lastFrameTime = copy.lastFrameTime;
	blipFrame = copy.blipFrame;
	numAttackers = copy.numAttackers;
}

//---------------------------------------------------------------------------

ObjectTypePtr GameObject::getObjectType (void) {

	return(ObjectManager->getObjectType(typeHandle));
}

//---------------------------------------------------------------------------

void GameObject::init (bool create, ObjectTypePtr _type) {

	typeHandle = _type->whatAmI();
#ifdef _DEBUG
//	id = _type->getName();
#endif
	objectClass = GAMEOBJECT;
}

//---------------------------------------------------------------------------

unsigned long GameObject::getWatchID (bool assign) {

	if ((watchID == 0) && assign)
		ObjectManager->setWatchID(this);
	return(watchID);
}

//---------------------------------------------------------------------------

void GameObject::getBlockAndVertexNumber (long &blockNum, long &vertexNum) {

	Assert(Terrain::worldUnitsPerVertex==128,0," Optimizations now broken ");

	// What is our block and vertex number?
	long mx = (float2long(position.x) >> 7) + Terrain::halfVerticesMapSide;
	long blockX = float2long(mx * Terrain::oneOverVerticesBlockSide);

	long my = Terrain::halfVerticesMapSide - ((float2long(position.y) >> 7) + 1);
	long blockY = float2long(my * Terrain::oneOverVerticesBlockSide);

	blockNum = blockX + (blockY * Terrain::blocksMapSide);

	long vertexX = mx - (blockX * Terrain::verticesBlockSide);
	long vertexY = my - (blockY * Terrain::verticesBlockSide);

	vertexNum = vertexX + (vertexY * Terrain::verticesBlockSide);
}	

//---------------------------------------------------------------------------

long GameObject::kill (void) 
{
	//------------------------------------------------------------
	//Once new MC II ObjMgr is up and running, put this back in...
	// DO NOT DO THIS ANYMORE. EACH OBJECT MUST HAVE ITS OWN DESTRUCTION NOW>
	// GOSFX Change 12/15/99 -fs  Code Deleted

	return(OBJECT_DEAD);
}

//---------------------------------------------------------------------------

float GameObject::relFacingTo (Stuff::Vector3D goal, long bodyLocation) {

	Stuff::Vector3D facingVec = getRotationVector();

	Stuff::Vector3D goalVec;
	goalVec.Subtract(goal, position);

	float angle = angle_from(facingVec, goalVec);

	//--------------------------------
	// Get sign of relative angle.
	float z = (facingVec.x * goalVec.y) - (facingVec.y * goalVec.x);
	if (z > 0.0f)
		angle = -angle;
	
	return(angle);
}

//---------------------------------------------------------------------------

Stuff::Vector3D GameObject::relativePosition (float angle, float distance, unsigned long flags) {

	//--------------------------------------------------------
	// Note that the angle should be -180 <= angle <= 180, and
	// the distance is in meters...

#ifdef USE_ROTATION
	Stuff::Vector2DOf<float> curPos;
	curPos.x = position.x;
	curPos.y = position.y;

	distance *= -worldUnitsPerMeter;
	Stuff::Vector2DOf<float> shiftVect;

	//--------------------------------------------
	// Absolute facing, based upon north facing...
	shiftVect.x = 0.0;
	shiftVect.y = 1.0;

	float tx = shiftVect.x;
	float sine = sin(angle);
	float cosine = cos(angle);

	shiftVect.x *= cosine;
	shiftVect.x += (shiftVect.y * sine);

	shiftVect.y *= cosine;
	shiftVect.y -= (tx * sine);

	//shiftVect.rotate(angle);
	shiftVect *= distance;

	Stuff::Vector2DOf<float> relPos;
	relPos.x = curPos.x + shiftVect.x;
	relPos.y = curPos.y + shiftVect.y;

	Stuff::Vector2DOf<float> start2d;
	Stuff::Vector2DOf<float> goal2d;
	Stuff::Vector2DOf<float> deltaVector;
	if (flags & RELPOS_FLAG_PASSABLE_START) {
		start2d  = curPos;
		goal2d = relPos;
		}
	else {
		start2d = relPos;
		goal2d = curPos;
	}

	deltaVector.x= goal2d.x - start2d.x;
	deltaVector.y = goal2d.y - start2d.y;

	//-------------------------------------------------------------
	// First, we need to calc the delta vector--how much we extend
	// the ray everytime we check the map cell for clear placement.
	deltaVector.normalize();
	float cellLength = Terrain::metersPerVertex / (float)MAPCELL_DIM;
	cellLength *= 0.5;
	deltaVector *= cellLength;
	if (deltaVector.magnitude() == 0.0)
		return(curPos);

	//-------------------------------------------------
	// Determine the max length the ray must be cast...
	float maxLength = start2d.distance_from(goal2d);

	//------------------------------------------------------------
	// We'll start at the target, and if it's blocked, we'll move
	// toward our start location, looking for the first valid/open
	// cell...
	Stuff::Vector2DOf<float> curPoint = start2d;
	Stuff::Vector2DOf<float> curRay;
	curRay.zero();
	float rayLength = 0.0;

	long tileR, tileC, cellR, cellC;
	Stuff::Vector3D curPoint3d;
	curPoint3d.init(curPoint.x, curPoint.y, 0.0);
	GameMap->worldToMapPos(curPoint3d, tileR, tileC, cellR, cellC);
	bool cellClear = GameMap->cellPassable(tileR, tileC, cellR, cellC);

	Stuff::Vector2DOf<float> lastGoodPoint = curPoint;
	if (flags & RELPOS_FLAG_PASSABLE_START)
		while (cellClear && (rayLength < maxLength)) {
			lastGoodPoint = curPoint;
			curPoint += deltaVector;
			curRay = curPoint - start2d;
			rayLength = curRay.magnitude();

			curPoint3d.init(curPoint.x, curPoint.y, 0.0);
			GameMap->worldToMapPos(curPoint3d, tileR, tileC, cellR, cellC);
			cellClear = GameMap->cellPassable(tileR, tileC, cellR, cellC);
		}
	else
		while (!cellClear && (rayLength < maxLength)) {
			lastGoodPoint = curPoint;
			curPoint += deltaVector;
			curRay = curPoint - start2d;
			rayLength = curRay.magnitude();

			curPoint3d.init(curPoint.x, curPoint.y, 0.0);
			GameMap->worldToMapPos(curPoint3d, tileR, tileC, cellR, cellC);
			cellClear = GameMap->cellPassable(tileR, tileC, cellR, cellC);
		}

	curPoint3d.init(lastGoodPoint.x, lastGoodPoint.y, 0.0);
	curPoint3d.z = GameMap->getTerrainElevation(curPoint3d);
	return(curPoint3d);
#else
	return(position);
#endif
}

//---------------------------------------------------------------------------

void GameObject::setPosition (const Stuff::Vector3D& newPosition, bool calcPositions) {

	position = newPosition;

	if (calcPositions) {
		long newCellRow = 0;
		long newCellCol = 0;
		long tileRow = 0;
		long tileCol = 0;

		land->worldToTileCell(position, tileRow, tileCol, newCellRow, newCellCol);
		cellPositionRow = newCellRow + tileRow * MAPCELL_DIM;
		cellPositionCol = newCellCol + tileCol * MAPCELL_DIM;

		d_vertexNum = tileRow * Terrain::realVerticesMapSide + tileCol;
	}

	Assert((cellPositionRow >= 0) && (cellPositionRow < GameMap->getHeight()), 0, " Object moved off map ");
	Assert((cellPositionCol >= 0) && (cellPositionCol < GameMap->getWidth()), 0, " Object moved off map ");
}

//---------------------------------------------------------------------------

float GameObject::distanceFrom (Stuff::Vector3D goal) {

	Stuff::Vector3D result;
	result.x = position.x - goal.x;
	result.y = position.y - goal.y;
	result.z = 0.0;
	return((result.GetLength() * metersPerWorldUnit));
}

//---------------------------------------------------------------------------

long GameObject::cellDistanceFrom (Stuff::Vector3D goal) {

	long cellRow = 0;
	long cellCol = 0;
	land->worldToCell(goal, cellRow, cellCol);

	long rowDelta = 0;
	if (cellPositionRow > cellRow)
		rowDelta = cellPositionRow - cellRow;
	else
		rowDelta = cellRow - cellPositionRow;
	long colDelta = 0;
	if (cellPositionCol > cellCol)
		colDelta = cellPositionCol - cellCol;
	else
		colDelta = cellCol - cellPositionCol;
	return(rowDelta > colDelta ? rowDelta : colDelta);
}

//---------------------------------------------------------------------------

long GameObject::cellDistanceFrom (GameObjectPtr obj) {

	long rowDelta = 0;
	if (cellPositionRow > obj->cellPositionRow)
		rowDelta = cellPositionRow - obj->cellPositionRow;
	else
		rowDelta = obj->cellPositionRow - cellPositionRow;
	long colDelta = 0;
	if (cellPositionCol > obj->cellPositionCol)
		colDelta = cellPositionCol - obj->cellPositionCol;
	else
		colDelta = obj->cellPositionCol - cellPositionCol;
	return(rowDelta > colDelta ? rowDelta : colDelta);
}

//---------------------------------------------------------------------------

long GameObject::getLineOfSightNodes (long eyeCellRow, long eyeCellCol, long* cells) {

	cells[0] = cellPositionRow;
	cells[1] = cellPositionCol;
	return(1);
}

//---------------------------------------------------------------------------

bool GameObject::lineOfSight (long cellRow, long cellCol, bool checkVisibleBits) {

	bool LOSclear = Team::lineOfSight(0.0f,cellPositionRow, cellPositionCol, cellRow, cellCol, getTeamId(), 15.0, checkVisibleBits);
	return(LOSclear);
}

//---------------------------------------------------------------------------

bool GameObject::lineOfSight (Stuff::Vector3D point, bool checkVisibleBits) {

	Stuff::Vector3D firingPosition = getLOSPosition();
	Stuff::Vector3D targetPosition = point;
	
	bool LOSclear = false;
	if (land->IsGameSelectTerrainPosition(point))
		LOSclear = Team::lineOfSight(firingPosition, targetPosition, getTeamId(), 15.0, checkVisibleBits);
	
	return(LOSclear);
}

//---------------------------------------------------------------------------

#ifdef LAB_ONLY
extern __int64 MCTimeLOSUpdate;
#endif

inline bool GameObject::lineOfSight (GameObjectPtr target, float startExtRad, bool checkVisibleBits) 
{
	__int64 timeStart = GetCycles(); 

	//If we call this without a target, we have no LOS!!
	// Keeps it from crashing, too.
	// Not sure where all of the calls Glenn makes to this are, but I'm looking!
	if (!target) {
#ifdef LAB_ONLY
		MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
		return false;
	}

	Stuff::Vector3D distance;
	distance.Subtract(target->getPosition(),getPosition());
	float dist = distance.GetApproximateLength();

	//Figure out altitude above minimum terrain altitude and look up in table.
	float baseElevation = MapData::waterDepth;
	if (MapData::waterDepth < Terrain::userMin)
		baseElevation = Terrain::userMin;

	float altitude = position.z - baseElevation;
	float altitudeIntegerRange = (Terrain::userMax - baseElevation) * 0.00390625f;
	long altLevel = 0;
	if (altitudeIntegerRange > Stuff::SMALL)
		altLevel = altitude / altitudeIntegerRange;
	
	if (altLevel < 0)
		altLevel = 0;

	if (altLevel > 255)
		altLevel = 255;

	float radius = visualRangeTable[altLevel];
	
	//Scouting specialty skill.
	if (isMover())
	{
		MoverPtr mover = (MoverPtr)this;
		if (mover->pilot && mover->pilot->isScout())
			radius += (radius * 0.2f);
			
		radius *= mover->getLOSFactor();
	}

	if (dist > (radius * 25.0f * worldUnitsPerMeter))
	{
#ifdef LAB_ONLY
		MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
		return false;
	}
		
	//--------------------------------------------------------------------------
	// For now, we hardcode the "height" of the firer and target (to 10 meters).
	// Easily changed when we add a height field to the object class...
	if (target->isMover()) 
	{
		if (isMover() && ObjectManager->useMoverLineOfSightTable) 
		{
			long index = (handle * ObjectManager->maxMovers) + target->handle;
			char losStatus = ObjectManager->moverLineOfSightTable[index];
			if (losStatus == -1) 
			{
				bool los = Team::lineOfSight(getLOSPosition(), target->getLOSPosition(), getTeamId(), target->getAppearRadius(), startExtRad, checkVisibleBits);
				if (los) 
				{
					ObjectManager->moverLineOfSightTable[index] = 1;
					//Inverse is NOT always true!!!!
					// I can demonstrate a case!!
					// -fs
				   // ObjectManager->moverLineOfSightTable[(target->handle * ObjectManager->maxMovers) + handle] = 1;
				}
				else 
				{
					ObjectManager->moverLineOfSightTable[index] = 0;
					// ObjectManager->moverLineOfSightTable[(target->handle * ObjectManager->maxMovers) + handle] = 0;
				}
			}
			
			if (ObjectManager->moverLineOfSightTable[index])
			{
#ifdef LAB_ONLY
			MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
				return true;
			}
			else
			{
#ifdef LAB_ONLY
			MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
				return false;
			}
		}
		
		if (Team::lineOfSight(getLOSPosition(), target->getLOSPosition(), getTeamId(), target->getAppearRadius(), startExtRad, checkVisibleBits))
		{
#ifdef LAB_ONLY
		MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
			return(true);
		}
		else
		{
#ifdef LAB_ONLY
		MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
			return(false);
		}
	}

	//Try yanking these and just use the location the weapon fire is going to.
	// With the extent radius, like I originally did.
	// -fs
	//long lineOfSightNodes[20];
	//long numNodes = target->getLineOfSightNodes(cellPositionRow, cellPositionCol, lineOfSightNodes);
//	for (long i = 0; i < numNodes; i++)
//	{
//		if (land->IsGameSelectTerrainPosition(getLOSPosition()))
//		{
			float elev = land->getTerrainElevation(getLOSPosition());
			float localStart = getLOSPosition().z - elev;

			Stuff::Vector3D targetPosition = target->getLOSPosition();
		
			if (Team::lineOfSight(getLOSPosition(), target->getLOSPosition(), getTeamId(), target->getAppearRadius(), startExtRad, checkVisibleBits)) 
			{
#ifdef LAB_ONLY
				MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
				return(true);
			}
//		}
//	}

#ifdef LAB_ONLY
	MCTimeLOSUpdate += (GetCycles() - timeStart);
#endif
	return(false);
}

//---------------------------------------------------------------------------

void GameObject::destroy (void) {

	//Never need to call this.  Heap destruct will get this!!
//	ObjectManager->removeObjectType(typeHandle);

}

//---------------------------------------------------------------------------

bool GameObject::onScreen (void) {

	//----------------------------------------------------------------------
	// This function is the meat and potatoes of the object cull system.
	// Its job is to determine if the object is on screen or not.
	// It does this by transforming the position for each active camera to
	// its screen coords and saving them.  It then checks each set of coords
	// to see if they are in the viewport of each camera.  Returned value
	// is number of windows that object can be seen in.
	long isVisible = 0;
	if (eye) {
		Stuff::Vector3D objPosition = position;
		isVisible = eye->projectZ(objPosition, screenPos);
	}

	if (isVisible) {
		windowsVisible = turn;
		return(true);
	}
		
	return(false);
}

//---------------------------------------------------------------------------

float GameObject::getExtentRadius (void) {

	//---------------------------------------------------------------------
	// Can be overridden by explosions to return an instance based value.
	ObjectTypePtr objType = ObjectManager->getObjectType(typeHandle);
	if (objType)
		return(objType->getExtentRadius());
	return(-1.0);
}

//---------------------------------------------------------------------------

void GameObject::setExtentRadius (float newRadius) {

	//---------------------------------------------------------------------
	// Can be overridden by explosions to set an instance based value.
	ObjectTypePtr objType = ObjectManager->getObjectType(typeHandle);
	if (objType)
		objType->setExtentRadius(newRadius);
}

//---------------------------------------------------------------------------

MechClass GameObject::getMechClass(void) {

	if (getObjectClass() != BATTLEMECH)
		return(MECH_CLASS_NONE);
	if (tonnage < MechClassWeights[MECH_CLASS_LIGHT])
		return(MECH_CLASS_LIGHT);
	if (tonnage < MechClassWeights[MECH_CLASS_MEDIUM])
		return(MECH_CLASS_MEDIUM);
	if (tonnage < MechClassWeights[MECH_CLASS_HEAVY])
		return(MECH_CLASS_HEAVY);
	return(MECH_CLASS_ASSAULT);
}

//---------------------------------------------------------------------------

long GameObject::getCaptureBlocker (GameObjectPtr capturingMover, GameObjectPtr* blockerList) {

	long numBlockers = 0;
	TeamPtr capturingTeam = capturingMover->getTeam();
	if (!capturingTeam)
		STOP(("GameObject.getCaptureBlocker: NULL capturingTeam"));

	if (distanceFrom(capturingMover->getPosition()) <= 30.0) {
		for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
			MoverPtr mover = ObjectManager->getMover(i);
			if (capturingTeam->isEnemy(mover->getTeam()))
				if (!mover->isMarine() && (mover->numWeapons > 0))
					if ((distanceFrom(mover->getPosition()) < blockCaptureRange) && !mover->isDisabled() && mover->getAwake())
						if (blockerList)
							blockerList[numBlockers++] = mover;
						else
							return(1);
		}
		}
	else {
		for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
			MoverPtr mover = ObjectManager->getMover(i);
			if (!mover->getTeam() || capturingTeam->isEnemy(mover->getTeam()))
				if (!mover->isMarine() && (mover->numWeapons > 0))
					if ((distanceFrom(mover->getPosition()) < blockCaptureRange) && !mover->isDisabled() && mover->getAwake())
						if (capturingMover->getTeam()->isContact(capturingMover, mover, CONTACT_CRITERIA_VISUAL_OR_SENSOR + CONTACT_CRITERIA_ENEMY + CONTACT_CRITERIA_NOT_DISABLED))
							if (blockerList)
								blockerList[numBlockers++] = mover;
							else
								return(1);
		}			
	}

	return(numBlockers);
}

//---------------------------------------------------------------------------

bool GameObject::isFriendly (GameObjectPtr obj) {

	TeamPtr myTeam = getTeam();
	TeamPtr objTeam = obj->getTeam();
	if (myTeam && objTeam)
		return(myTeam->isFriendly(objTeam));
	return(false);
}

//---------------------------------------------------------------------------

bool GameObject::isEnemy (GameObjectPtr obj) {
			
	TeamPtr myTeam = getTeam();
	TeamPtr objTeam = obj->getTeam();
	if (myTeam && objTeam)
		return(myTeam->isEnemy(objTeam));
	return(false);
}

//---------------------------------------------------------------------------

bool GameObject::isNeutral (GameObjectPtr obj) {
			
	TeamPtr myTeam = getTeam();
	TeamPtr objTeam = obj->getTeam();
	if (myTeam && objTeam)
		return(myTeam->isNeutral(objTeam));
	return(false);
}

//---------------------------------------------------------------------------
void GameObject::CopyTo (GameObjectData *data)
{
	if (getObjectType())
		data->objectTypeNum = getObjectType()->getObjTypeNum();
	else
		data->objectTypeNum = 0;

	data->objectClass = objectClass;
	data->handle = handle;
	data->partId = partId;
	data->watchID = watchID;

	data->typeHandle = typeHandle;
	data->position = position;
	data->cellPositionRow = cellPositionRow;
	data->cellPositionCol = cellPositionCol;
	data->d_vertexNum = d_vertexNum;
	data->flags = flags;
	data->debugFlags = debugFlags;
	data->status = status;

	data->tonnage = tonnage;
	data->rotation = rotation;
	if (getObjectType() && getObjectType()->getAppearanceTypeName())
	{
		if (strlen(getObjectType()->getAppearanceTypeName()) <= 255)
			strcpy(data->appearanceTypeID,getObjectType()->getAppearanceTypeName());
		else
			STOP(("Object Appearance name too long for Save.  %s",getObjectType()->getAppearanceTypeName()));
	}
	else
	{
		strcpy(data->appearanceTypeID,"NONE");
	}

	data->collisionFreeFromWID = collisionFreeFromWID;
	data->collisionFreeTime = collisionFreeTime;
	data->screenPos = screenPos;
	data->windowsVisible = windowsVisible;
	data->explRadius = explRadius;
	data->explDamage = explDamage;
	data->maxCV = maxCV;
	data->curCV = curCV;
	data->threatRating = threatRating;
	data->lastFrameTime = lastFrameTime;
	data->blipFrame = blipFrame;
	data->numAttackers = numAttackers;

	data->drawFlags = drawFlags;
}

//---------------------------------------------------------------------------
void GameObject::Save (PacketFilePtr file, long packetNum)
{
	STOP(("Should never save a gameObj!!"));
}

//---------------------------------------------------------------------------
void GameObject::Load (GameObjectData *data)
{
	objectClass = data->objectClass;
	handle = data->handle;
	partId = data->partId;
	watchID = data->watchID;

	position = data->position;
	cellPositionRow = data->cellPositionRow;
	cellPositionCol = data->cellPositionCol;
	d_vertexNum = data->d_vertexNum;
	flags = data->flags;
	debugFlags = data->debugFlags;
	status = data->status;

	tonnage = data->tonnage;
	rotation = data->rotation;

	collisionFreeFromWID = data->collisionFreeFromWID;
	collisionFreeTime = data->collisionFreeTime;
	screenPos = data->screenPos;
	windowsVisible = 0;		//Force back to zero so I don't have to save the turn.
	explRadius = data->explRadius;
	explDamage = data->explDamage;
	maxCV = data->maxCV;
	curCV = data->curCV;
	threatRating = data->threatRating;
	lastFrameTime = data->lastFrameTime;
	blipFrame = data->blipFrame;
	numAttackers = data->numAttackers;

	drawFlags = data->drawFlags;
}

//***************************************************************************




