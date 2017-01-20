//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//***************************************************************************
//
//								TACORDR.CPP
//
//***************************************************************************

#ifndef MCLIB
#include"mclib.h"
#endif

#ifndef TACORDR_H
#include"tacordr.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef MECH_H
#include"mech.h"
#endif

#ifndef GVEHICL_H
#include"gvehicl.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifndef GROUP_H
#include"group.h"
#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef SOUNDS_H
#include"sounds.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

#include"logisticsdata.h"

#include<stdio.h>

//***************************************************************************

#define	TACORDERCHUNK_UNITORDER_BITS		1
#define	TACORDERCHUNK_ORIGIN_BITS			2
#define	TACORDERCHUNK_CODE_BITS				5
#define	TACORDERCHUNK_GROUP_BITS			16
#define	TACORDERCHUNK_ATTACKTYPE_BITS		2
#define	TACORDERCHUNK_ATTACKMETHOD_BITS		2
#define	TACORDERCHUNK_ATTACKRANGE_BITS		3
#define	TACORDERCHUNK_AIMLOCATION_BITS		4
#define	TACORDERCHUNK_PURSUE_BITS			1
#define	TACORDERCHUNK_OBLITERATE_BITS		1
#define	TACORDERCHUNK_CELLPOS_BITS			10
#define	TACORDERCHUNK_FACEOBJECT_BITS		1
#define	TACORDERCHUNK_WAIT_BITS				1
#define	TACORDERCHUNK_MODE_BITS				1
#define	TACORDERCHUNK_RUN_BITS				1
#define	TACORDERCHUNK_TARGETTYPE_BITS		2
#define	TACORDERCHUNK_MOVERINDEX_BITS		7
#define	TACORDERCHUNK_TERRAINPARTID_BITS	20
#define	TACORDERCHUNK_TERRAINBLOCK_BITS		8
#define	TACORDERCHUNK_TERRAINVERTEX_BITS	9
#define	TACORDERCHUNK_TERRAINITEM_BITS		3
#define	TACORDERCHUNK_TRAIN_BITS			8
#define	TACORDERCHUNK_TRAINCAR_BITS			8

#define	TACORDERCHUNK_UNITORDER_MASK		0x00000001
#define	TACORDERCHUNK_ORIGIN_MASK			0x00000003
#define	TACORDERCHUNK_CODE_MASK				0x0000001F
#define	TACORDERCHUNK_GROUP_MASK			0x0000FFFF
#define	TACORDERCHUNK_ATTACKTYPE_MASK		0x00000003
#define	TACORDERCHUNK_ATTACKMETHOD_MASK		0x00000003
#define	TACORDERCHUNK_ATTACKRANGE_MASK		0x00000007
#define	TACORDERCHUNK_AIMLOCATION_MASK		0x0000000F
#define	TACORDERCHUNK_PURSUE_MASK			0x00000001
#define	TACORDERCHUNK_OBLITERATE_MASK		0x00000001
#define	TACORDERCHUNK_CELLPOS_MASK			0x000003FF
#define	TACORDERCHUNK_FACEOBJECT_MASK		0x00000001
#define	TACORDERCHUNK_WAIT_MASK				0x00000001
#define	TACORDERCHUNK_MODE_MASK				0x00000001
#define	TACORDERCHUNK_RUN_MASK				0x00000001
#define	TACORDERCHUNK_TARGETTYPE_MASK		0x00000003
#define	TACORDERCHUNK_MOVERINDEX_MASK		0x0000007F
#define	TACORDERCHUNK_TERRAINPARTID_MASK	0x000FFFFF
#define	TACORDERCHUNK_TERRAINBLOCK_MASK		0x000000FF
#define	TACORDERCHUNK_TERRAINVERTEX_MASK	0x000001FF
#define	TACORDERCHUNK_TERRAINITEM_MASK		0x00000007
#define	TACORDERCHUNK_TRAIN_MASK			0x000000FF
#define	TACORDERCHUNK_TRAINCAR_MASK			0x000000FF

extern TeamPtr homeTeam;
extern UserHeapPtr systemHeap;
extern float scenarioTime;
extern float FireArc;
extern long LastMoveCalcErr;

extern char OverlayIsBridge[NUM_OVERLAY_TYPES];

long statusCode;

//***************************************************************************

long AttitudeEffectOnMovePath[NUM_ATTITUDES][3] = {
	{10, 10, 2},	// Cautious
	{8, 8, 4},		// Conservative
	{6, 6, 6},		// Normal
	{4, 4, 8},		// Aggressive
	{2, 2, 10},		// Berserker
	{1, 1, 10}		// Suicidal
};

//***************************************************************************
// TACTICAL ORDER class
//***************************************************************************

void* TacticalOrder::operator new (size_t mySize) {

	void *result = systemHeap->Malloc(mySize);
	return(result);
}

//---------------------------------------------------------------------------

void TacticalOrder::operator delete (void* us) {

	systemHeap->Free(us);
}

//---------------------------------------------------------------------------

void TacticalOrder::init (void) {

	id = 0;
	time = -1.0;
	delayedTime = -1.0;
	lastTime = -1.0;
	unitOrder = false;
	origin = ORDER_ORIGIN_COMMANDER;
	code = TACTICAL_ORDER_NONE;
	moveParams.wayPath.numPoints = 0;
	moveParams.wayPath.curPoint = 0;
	moveParams.wayPath.points[0] = 0.0;
	moveParams.wayPath.points[1] = 0.0;
	moveParams.wayPath.points[2] = 0.0;
	moveParams.wayPath.mode[0] = TRAVEL_MODE_SLOW;
	moveParams.fromArea = -1;
	moveParams.faceObject = true;
	moveParams.wait = false;
	moveParams.mode = MOVE_MODE_NORMAL;
	moveParams.escapeTile = false;
	moveParams.jump = false;
	moveParams.keepMoving = true;
	attackParams.type = ATTACK_TO_DESTROY;
	attackParams.method = ATTACKMETHOD_RANGED;
	attackParams.range = FIRERANGE_OPTIMAL;
	attackParams.tactic = TACTIC_NONE;
	attackParams.aimLocation = -1;
	attackParams.pursue = true;
	attackParams.obliterate = false;
	attackParams.targetPoint.Zero();
	targetWID = 0;
	targetObjectClass = BASEOBJECT;
	selectionIndex = -1;
	stage = 1;

	pointLocalMoverId = 0x0F;
	groupFlags = 0;
	
	subOrder = false;
	statusCode = 0;
}

//---------------------------------------------------------------------------

void TacticalOrder::init (OrderOriginType _origin, TacticalOrderCode _code, bool _unitOrder) {

	init();
	time = scenarioTime;
	unitOrder = _unitOrder;
	origin = _origin;
	code = _code;
}

//---------------------------------------------------------------------------

void TacticalOrder::initWayPath (LocationNodePtr path) {

	long numPoints = 0;
	while (path) {
		if (numPoints == MAX_WAYPTS)
			Fatal(0, " Way Path Too Long ");
		else {
			moveParams.wayPath.points[numPoints * 3] = path->location.x;
			moveParams.wayPath.points[numPoints * 3 + 1] = path->location.y;
			moveParams.wayPath.points[numPoints * 3 + 2] = path->location.z;
			if (path->run)
				moveParams.wayPath.mode[numPoints] = TRAVEL_MODE_FAST;
			else
				moveParams.wayPath.mode[numPoints] = TRAVEL_MODE_SLOW;
		}
		numPoints++;
		path = path->next;
	}
	moveParams.wayPath.numPoints = numPoints;
}

//---------------------------------------------------------------------------

Stuff::Vector3D TacticalOrder::getWayPoint (long index) {

	Stuff::Vector3D wayPoint(moveParams.wayPath.points[index * 3],
							 moveParams.wayPath.points[index * 3 + 1],
							 moveParams.wayPath.points[index * 3 + 2]);
	return(wayPoint);
}

//---------------------------------------------------------------------------

void TacticalOrder::setWayPoint (long index, Stuff::Vector3D wayPoint) {

	moveParams.wayPath.points[index * 3] = wayPoint.x;
	moveParams.wayPath.points[index * 3 + 1] = wayPoint.y;
	moveParams.wayPath.points[index * 3 + 2] = wayPoint.z;
}

//---------------------------------------------------------------------------

void TacticalOrder::addWayPoint (Stuff::Vector3D wayPoint, long travelMode) {

	if (moveParams.wayPath.numPoints == MAX_WAYPTS)
		Fatal(MAX_WAYPTS, " tacticalOrder.addWayPoint: too many! ");

	moveParams.wayPath.points[moveParams.wayPath.numPoints * 3] = wayPoint.x;
	moveParams.wayPath.points[moveParams.wayPath.numPoints * 3 + 1] = wayPoint.y;
	moveParams.wayPath.points[moveParams.wayPath.numPoints * 3 + 2] = wayPoint.z;
	moveParams.wayPath.mode[moveParams.wayPath.numPoints] = (unsigned char)travelMode;
	moveParams.wayPath.numPoints++;
}

//---------------------------------------------------------------------------

GameObjectPtr TacticalOrder::getRamTarget (void) {

	if (code == TACTICAL_ORDER_ATTACK_OBJECT)
		if (attackParams.method == ATTACKMETHOD_RAMMING)
			return(ObjectManager->getByWatchID(targetWID));
	return(NULL);
}

//---------------------------------------------------------------------------

GameObjectPtr TacticalOrder::getJumpTarget (void) {

	if (code == TACTICAL_ORDER_JUMPTO_POINT)
		return(ObjectManager->get(targetWID));
	return(NULL);
}

//---------------------------------------------------------------------------

bool TacticalOrder::isGroupOrder (void) {

	return(unitOrder);
}

//---------------------------------------------------------------------------

bool TacticalOrder::isCombatOrder (void) {

	return((code == TACTICAL_ORDER_ATTACK_OBJECT) || (code == TACTICAL_ORDER_ATTACK_POINT));
}

//---------------------------------------------------------------------------

bool TacticalOrder::isMoveOrder (void) {

	return((code == TACTICAL_ORDER_MOVETO_POINT) || (code == TACTICAL_ORDER_MOVETO_OBJECT));
}

//---------------------------------------------------------------------------

bool TacticalOrder::isWayPathOrder (void) {

	return((code == TACTICAL_ORDER_TRAVERSE_PATH) || (code == TACTICAL_ORDER_PATROL_PATH));
}

//---------------------------------------------------------------------------

bool TacticalOrder::isJumpOrder (void) {

	return((code == TACTICAL_ORDER_JUMPTO_POINT) || (code == TACTICAL_ORDER_JUMPTO_OBJECT));
}

//---------------------------------------------------------------------------

void TacticalOrder::setId (MechWarriorPtr pilot) {

	Assert(id == 0, id, " TacticalOrder.setId: id != 0 ");

	long nextId = pilot->getNextTacOrderId();
	id = nextId;
	if (nextId == 255)
		pilot->setNextTacOrderId(1);
	else
		pilot->setNextTacOrderId(nextId + 1);
}

//---------------------------------------------------------------------------

long TacticalOrder::getParamData (float* timeOfOrder, int* paramList) {

	if (timeOfOrder)
		*timeOfOrder = time;

	GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
	paramList[0] = code;
	paramList[1] = origin;
	paramList[2] = unitOrder ? 1 : 0;
	paramList[3] = target ? target->getPartId() : 0;
	paramList[4] = targetObjectClass;
//	UnitPtr targetUnit = NULL;
//	if (obj && obj->isMover())
//		targetUnit = ((MoverPtr)obj)->getUnit();
	paramList[5] = subOrder ? 1 : 0;
	paramList[6] = selectionIndex;

	switch (code) {
		case TACTICAL_ORDER_MOVETO_POINT:
		case TACTICAL_ORDER_GUARD:
			paramList[7] = (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST) ? 1 : 0;
			paramList[8] = moveParams.wait;
			paramList[9] = moveParams.wayPath.points[0];
			paramList[10] = moveParams.wayPath.points[1];
			paramList[11] = moveParams.wayPath.points[2];
			break;
		case TACTICAL_ORDER_MOVETO_OBJECT:
			paramList[7] = (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST) ? 1 : 0;
			paramList[8] = moveParams.wait;
			paramList[9] = moveParams.faceObject;
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT:
			paramList[7] = (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST) ? 1 : 0;
			paramList[8] = attackParams.type;
			paramList[9] = attackParams.method;
			paramList[10] = attackParams.range;
			paramList[11] = attackParams.aimLocation;
			paramList[12] = attackParams.pursue ? 1 : 0;
			paramList[13] = attackParams.obliterate ? 1 : 0;
			break;
		case TACTICAL_ORDER_ATTACK_POINT:
			break;
	}

	return(code);
}

//---------------------------------------------------------------------------

long TacticalOrder::pack (MoverGroupPtr group, MoverPtr point) {

	long tempData = 0;

	//--------------------
	// First data DWORD...
	tempData |= (long)(attackParams.aimLocation + 2);
	tempData <<= TACORDERCHUNK_ATTACKMETHOD_BITS;

	tempData |= (long)attackParams.method;
	tempData <<= TACORDERCHUNK_ATTACKTYPE_BITS;

	tempData |= (long)attackParams.type;
	tempData <<= TACORDERCHUNK_GROUP_BITS;

	tempData |= groupFlags;
	tempData <<= TACORDERCHUNK_UNITORDER_BITS;

	tempData |= (unitOrder ? 1 : 0);
	tempData <<= TACORDERCHUNK_ORIGIN_BITS;

	tempData |= (long)origin;
	tempData <<= TACORDERCHUNK_CODE_BITS;

	//------------------------
	// MAJOR HACK FOR GUARD...
	if (code == TACTICAL_ORDER_GUARD) {
		//GameObjectPtr curTarget = ObjectManager->getByWatchID(targetWID);
		if (targetWID)
			tempData |= (unsigned long)TACTICAL_ORDER_GUARD;
		else
			tempData |= (unsigned long)TACORDERCHUNK_CODE_MASK;
		}
	else
		tempData |= (long)code;

	data[0] = tempData;

	//---------------------
	// Second data DWORD...
	tempData = 0;

	bool saveTarget = false;
	long targetType = 0;
	bool saveLocation = false;

	GameObjectPtr curTarget = ObjectManager->getByWatchID(targetWID);
	switch (code) {
		case TACTICAL_ORDER_MOVETO_POINT:
		case TACTICAL_ORDER_JUMPTO_POINT:
			saveLocation = true;
			break;
		case TACTICAL_ORDER_GUARD:
			if (curTarget) {
				saveTarget = true;
				if (curTarget->isMover())
					targetType = 0;
				else
					targetType = 1;
				}
			else
				saveLocation = true;
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT:
		case TACTICAL_ORDER_MOVETO_OBJECT:
		case TACTICAL_ORDER_JUMPTO_OBJECT:
		case TACTICAL_ORDER_CAPTURE:
		case TACTICAL_ORDER_REFIT:
		case TACTICAL_ORDER_RECOVER:
		case TACTICAL_ORDER_GETFIXED:
			saveTarget = true;
			if (curTarget->isMover())
				targetType = 0;
			else
				targetType = 1;
			break;
		case TACTICAL_ORDER_ATTACK_POINT:
			saveTarget = true;
			targetType = 3;
			break;
	}

	if (saveTarget) {
		switch (targetType) {
			case 0:
				//------
				// Mover
				tempData |= ((MoverPtr)curTarget)->getNetRosterIndex();
				tempData <<= TACORDERCHUNK_TARGETTYPE_BITS;

				tempData |= 0;
				break;
			case 1: {
				// Terrain
				tempData |= (curTarget->getPartId() - MIN_TERRAIN_PART_ID);
				tempData <<= TACORDERCHUNK_TARGETTYPE_BITS;

				tempData |= 1;
				}
				break;
			case 3:
				// Location
				int worldCell[2] = {0, 0};
				land->worldToCell(attackParams.targetPoint, worldCell[0], worldCell[1]);
				tempData |= worldCell[0];
				tempData <<= TACORDERCHUNK_CELLPOS_BITS;

				tempData |= worldCell[1];
				//tempData <<= TACORDERCHUNK_TARGETTYPE_BITS;

				//tempData |= 3; // not really nec, but we'll store it anyway...
				break;
		}
		tempData <<= TACORDERCHUNK_RUN_BITS;
		}
	else if (saveLocation) {
		//--------------
		// Move Location
		int worldCell[2] = {0, 0};
		land->worldToCell(getWayPoint(0), worldCell[0], worldCell[1]);
		tempData |= worldCell[0];
		tempData <<= TACORDERCHUNK_CELLPOS_BITS;

		tempData |= worldCell[1];
		tempData <<= TACORDERCHUNK_RUN_BITS;

	}

	tempData |= ((moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST) ? 1 : 0);
	tempData <<= TACORDERCHUNK_MODE_BITS;

	tempData |= (long)moveParams.mode;
	tempData <<= TACORDERCHUNK_WAIT_BITS;

	tempData |= (moveParams.wait ? 1 : 0);
	tempData <<= TACORDERCHUNK_FACEOBJECT_BITS;

	tempData |= (moveParams.faceObject ? 1 : 0);
	tempData <<= TACORDERCHUNK_OBLITERATE_BITS;

	tempData |= (attackParams.obliterate ? 1 : 0);
	tempData <<= TACORDERCHUNK_PURSUE_BITS;

	tempData |= (attackParams.pursue ? 1 : 0);
	tempData <<= TACORDERCHUNK_ATTACKRANGE_BITS;

	tempData |= (long)(attackParams.range + 4);

	data[1] = tempData;

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long TacticalOrder::unpack (void) {

	long tempData = data[0];

	code = (TacticalOrderCode)(tempData & TACORDERCHUNK_CODE_MASK);
	tempData >>= TACORDERCHUNK_CODE_BITS;

	//------------------------
	// MAJOR HACK FOR GUARD...
	bool guardLocation = false;
	if (code == TACORDERCHUNK_CODE_MASK) {
		code = TACTICAL_ORDER_GUARD;
		guardLocation = true;
	}

	origin = (OrderOriginType)(tempData & TACORDERCHUNK_ORIGIN_MASK);
	tempData >>= TACORDERCHUNK_ORIGIN_BITS;

	unitOrder = ((tempData & TACORDERCHUNK_UNITORDER_MASK) ? true : false);
	tempData >>= TACORDERCHUNK_UNITORDER_BITS;

	init(origin, code, unitOrder);

	//time = scenarioTime;
	//delayedTime = -1.0;
	//lastTime = -1.0;
	//selectionIndex = -1;

	groupFlags = (tempData & TACORDERCHUNK_GROUP_MASK);
	tempData >>= TACORDERCHUNK_GROUP_BITS;

	attackParams.type = (AttackType)(tempData & TACORDERCHUNK_ATTACKTYPE_MASK);
	tempData >>= TACORDERCHUNK_ATTACKTYPE_BITS;

	attackParams.method = (AttackMethod)(tempData & TACORDERCHUNK_ATTACKMETHOD_MASK);
	tempData >>= TACORDERCHUNK_ATTACKMETHOD_BITS;

	attackParams.aimLocation = (tempData & TACORDERCHUNK_AIMLOCATION_MASK) - 2;

	tempData = data[1];

	attackParams.range = (FireRangeType)((tempData & TACORDERCHUNK_ATTACKRANGE_MASK) - 4);
	tempData >>= TACORDERCHUNK_ATTACKRANGE_BITS;

	attackParams.pursue = ((tempData & TACORDERCHUNK_PURSUE_MASK) ? true : false);
	tempData >>= TACORDERCHUNK_PURSUE_BITS;

	attackParams.obliterate = ((tempData & TACORDERCHUNK_OBLITERATE_MASK) ? true : false);
	tempData >>= TACORDERCHUNK_OBLITERATE_BITS;

	moveParams.faceObject = ((tempData & TACORDERCHUNK_FACEOBJECT_MASK) ? true : false);
	tempData >>= TACORDERCHUNK_FACEOBJECT_BITS;
	
	moveParams.wait = ((tempData & TACORDERCHUNK_WAIT_MASK) ? true : false);
	tempData >>= TACORDERCHUNK_WAIT_BITS;

	moveParams.mode = (SpecialMoveMode)(tempData & TACORDERCHUNK_MODE_MASK);
	tempData >>= TACORDERCHUNK_MODE_BITS;

	moveParams.wayPath.mode[0] = ((tempData & TACORDERCHUNK_RUN_MASK) ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tempData >>= TACORDERCHUNK_RUN_BITS;

	switch (code) {
		case TACTICAL_ORDER_MOVETO_POINT:
		case TACTICAL_ORDER_JUMPTO_POINT: {
			long worldCell[2] = {0, 0};
			worldCell[1] = (tempData & TACORDERCHUNK_CELLPOS_MASK);
			tempData >>= TACORDERCHUNK_CELLPOS_BITS;
					
			worldCell[0] = (tempData & TACORDERCHUNK_CELLPOS_MASK);

			Stuff::Vector3D worldPos;
			land->cellToWorld(worldCell[0], worldCell[1], worldPos);
			setWayPoint(0, worldPos);
			moveParams.wayPath.numPoints = 1;
			}
			break;
		case TACTICAL_ORDER_GUARD:
			if (guardLocation) {
				long worldCell[2] = {0, 0};
				worldCell[1] = (tempData & TACORDERCHUNK_CELLPOS_MASK);
				tempData >>= TACORDERCHUNK_CELLPOS_BITS;
					
				worldCell[0] = (tempData & TACORDERCHUNK_CELLPOS_MASK);

				Stuff::Vector3D worldPos;
				land->cellToWorld(worldCell[0], worldCell[1], worldPos);
				setWayPoint(0, worldPos);
				moveParams.wayPath.numPoints = 1;
				}
			else {
				long targetType = (tempData & TACORDERCHUNK_TARGETTYPE_MASK);
				tempData >>= TACORDERCHUNK_TARGETTYPE_BITS;
				if (targetType == 0) {
					// Mover Index
					long netMoverIndex = (tempData & TACORDERCHUNK_MOVERINDEX_MASK);
					if (MPlayer) {
						targetWID = 0;
						if (MPlayer->moverRoster[netMoverIndex])
							targetWID = MPlayer->moverRoster[netMoverIndex]->getWatchID();
					}
					}
				else if (targetType == 1) {
					// Terrain
					unsigned long targetPartId = (tempData & TACORDERCHUNK_TERRAINPARTID_MASK) + MIN_TERRAIN_PART_ID;
					tempData >>= TACORDERCHUNK_TERRAINPARTID_BITS;

					GameObjectPtr obj = ObjectManager->findByPartId(targetPartId);
					if (!obj)
						STOP(("TacticalOrder.unpack: NULL Terrain Target"));
					targetWID = obj->getWatchID();
					}
				else
					Fatal(targetType, " TacticalOrder.unpack: Bad targetType ");
			}
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT:
		case TACTICAL_ORDER_MOVETO_OBJECT:
		case TACTICAL_ORDER_JUMPTO_OBJECT:
		case TACTICAL_ORDER_CAPTURE:
		case TACTICAL_ORDER_REFIT:
		case TACTICAL_ORDER_RECOVER:
		case TACTICAL_ORDER_GETFIXED: {
			long targetType = (tempData & TACORDERCHUNK_TARGETTYPE_MASK);
			tempData >>= TACORDERCHUNK_TARGETTYPE_BITS;
			if (targetType == 0) {
				// Mover Index
				long netMoverIndex = (tempData & TACORDERCHUNK_MOVERINDEX_MASK);
				if (MPlayer) {
					targetWID = 0;
					if (MPlayer->moverRoster[netMoverIndex])
						targetWID = MPlayer->moverRoster[netMoverIndex]->getWatchID();
				}
				}
			else if (targetType == 1) {
				// Terrain
				unsigned long targetPartId = (tempData & TACORDERCHUNK_TERRAINPARTID_MASK) + MIN_TERRAIN_PART_ID;
				tempData >>= TACORDERCHUNK_TERRAINPARTID_BITS;

				GameObjectPtr obj = ObjectManager->findByPartId(targetPartId);
				if (!obj)
					STOP(("TacticalOrder.unpack: NULL Terrain Target"));
				targetWID = obj->getWatchID();
				}
			else
				Fatal(0, " Bad targetType ");
			}
			break;
		case TACTICAL_ORDER_ATTACK_POINT: {
			long worldCell[2] = {0, 0};
			worldCell[1] = (tempData & TACORDERCHUNK_CELLPOS_MASK);
			tempData >>= TACORDERCHUNK_CELLPOS_BITS;
					
			worldCell[0] = (tempData & TACORDERCHUNK_CELLPOS_MASK);

			land->cellToWorld(worldCell[0], worldCell[1], attackParams.targetPoint);
			attackParams.targetPoint.z = land->getTerrainElevation(attackParams.targetPoint);
			}
			break;
	}

	if (targetWID) {
		targetObjectClass = ObjectManager->getByWatchID(targetWID)->getObjectClass();
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

void TacticalOrder::setGroupFlag (long localMoverId, bool set) {

	unsigned long mask = (1 << localMoverId);
	if (set)
		groupFlags |= mask;
	else
		groupFlags &= (mask ^ 0xFFFFFFFF);
}

//---------------------------------------------------------------------------

long TacticalOrder::getGroup (long commanderID, MoverPtr* moverList, MoverPtr* point, long sortType) {

	if (!MPlayer)
		return(0);

	long numMovers = 0;
	long gFlags = groupFlags;
	for (long i = 0; i < MAX_LOCAL_MOVERS; i++) {
		if (gFlags & 1)
			moverList[numMovers++] = MPlayer->playerMoverRoster[commanderID][i];
		gFlags >>= 1;
	}

	*point = NULL;
	if (pointLocalMoverId != 0x0F)
		*point = MPlayer->playerMoverRoster[commanderID][pointLocalMoverId];

	return(numMovers);
}

//---------------------------------------------------------------------------

long TacticalOrder::execute (MechWarriorPtr warrior, long& message) {

	long result = TACORDER_FAILURE;
	message = -1;

	if (warrior->getVehicle()->refitBuddyWID)	// am I in a refitting operation? if is, point my and my refit
	{												// buddy to NULL, taking us out of the operation
		GameObjectPtr refitBuddy = ObjectManager->getByWatchID(warrior->getVehicle()->refitBuddyWID);
		MoverPtr vehicle = warrior->getVehicle();
		// turn off refitting (may not be on, but make sure)
		if (vehicle->getObjectClass() == GROUNDVEHICLE)
			((GroundVehiclePtr) vehicle)->refitting = false;
		if (refitBuddy) {
			if (refitBuddy->getObjectClass() == GROUNDVEHICLE)
				((GroundVehiclePtr)refitBuddy)->refitting = false;

			if (refitBuddy->isMover())
				((MoverPtr)refitBuddy)->refitBuddyWID = 0;
			else if (refitBuddy->getObjectClass() == BUILDING)
				((BuildingPtr)refitBuddy)->refitBuddyWID = 0;
		}
		warrior->getVehicle()->refitBuddyWID = 0;
	}

	if (warrior->getVehicle()->recoverBuddyWID)	{
		GameObjectPtr recoverBuddy = ObjectManager->getByWatchID(warrior->getVehicle()->recoverBuddyWID);
		MoverPtr vehicle = warrior->getVehicle();
		// turn off recovering (may not be on, but make sure)
		if (vehicle->getObjectClass() == GROUNDVEHICLE)
			((GroundVehiclePtr) vehicle)->recovering = false;
		if (recoverBuddy) {
			if (recoverBuddy->getObjectClass() == GROUNDVEHICLE)
				((GroundVehiclePtr)recoverBuddy)->recovering = false;

			if (recoverBuddy->isMover())
				((MoverPtr)recoverBuddy)->recoverBuddyWID = 0;
//			else if (recoverBuddy->getObjectClass() == BUILDING)
//				((BuildingPtr)recoverBuddy)->recoverBuddyWID = 0;
		}
		warrior->getVehicle()->recoverBuddyWID = 0;
	}

	//-------------------------------------------------------------------------
	// NOTE: If not a lance order, formationPos will equal -1. Otherwise,
	// will equal position in formation. And, if the lance point, will equal 0.
	GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
	switch (code) {
		case TACTICAL_ORDER_MOVETO_POINT: {
			if (delayedTime != -1.0) {
				if (delayedTime > scenarioTime)
					return(TACORDER_FAILURE);
				delayedTime = -1.0;
			}
			Stuff::Vector3D location;
			location.x = moveParams.wayPath.points[0];
			location.y = moveParams.wayPath.points[1];
			location.z = moveParams.wayPath.points[2];
			unsigned long params = TACORDER_PARAM_NONE;
			if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
				params |= TACORDER_PARAM_RUN;
			if (moveParams.wait)
				params |= TACORDER_PARAM_WAIT;
			if (moveParams.mode == MOVE_MODE_MINELAYING)
				params |= TACORDER_PARAM_LAY_MINES;
			if (moveParams.escapeTile)
				params |= TACORDER_PARAM_ESCAPE_TILE;
			if (moveParams.jump)
				params |= TACORDER_PARAM_JUMP;
			result = warrior->orderMoveToPoint(unitOrder, true, origin, location, selectionIndex, params);
			message = -1;
			}
			break;
		case TACTICAL_ORDER_MOVETO_OBJECT: {
			unsigned long params = TACORDER_PARAM_NONE;
			if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
				params |= TACORDER_PARAM_RUN;
			if (moveParams.faceObject)
				params |= TACORDER_PARAM_FACE_OBJECT;
			if (moveParams.mode == MOVE_MODE_MINELAYING)
				params |= TACORDER_PARAM_LAY_MINES;
			warrior->orderMoveToObject(unitOrder, true, origin, target, moveParams.fromArea, selectionIndex, params);
			message = -1;
			}
			break;
		case TACTICAL_ORDER_JUMPTO_POINT: 
			{
			//-----------------------------------------------------
			// If we got into here, we know the jump can be done...
			if (delayedTime != -1.0) {
				if (delayedTime > scenarioTime)
					return(TACORDER_FAILURE);
				delayedTime = -1.0;
			}
			Stuff::Vector3D location;
			location.x = moveParams.wayPath.points[0];
			location.y = moveParams.wayPath.points[1];
			location.z = moveParams.wayPath.points[2];
			result = warrior->orderJumpToPoint(unitOrder, true, origin, location);
			}
			break;
		case TACTICAL_ORDER_JUMPTO_OBJECT:
			//-----------------------------------------------------
			// If we got into here, we know the jump can be done...
			if (delayedTime != -1.0) {
				if (delayedTime > scenarioTime)
					return(TACORDER_FAILURE);
				delayedTime = -1.0;
			}
			if (!target)
				return(TACORDER_SUCCESS);
			result = warrior->orderJumpToObject(unitOrder, true, origin, target);
			break;
/*
		case TACTICAL_ORDER_TRAVERSE_PATH: {
			unsigned long params = TACORDER_PARAM_NONE;
			if (moveParams.mode == MOVE_MODE_MINELAYING)
				params |= TACORDER_PARAM_LAY_MINES;
			warrior->orderTraversePath( unitOrder, TRUE, origin, &moveParams.wayPath, params);
			message = RADIO_MOVETO;
			}
			break;
		case TACTICAL_ORDER_PATROL_PATH:
			//-------------
			// NEW BRAIN...
			warrior->orderPatrolPath( unitOrder, TRUE, origin, &moveParams.wayPath);
			message = RADIO_MOVETO;
			break;
*/
		case TACTICAL_ORDER_ESCORT:
			break;
		case TACTICAL_ORDER_FOLLOW:
			break;
		case TACTICAL_ORDER_GUARD:
			attackParams.type = ATTACK_TO_DESTROY;
			attackParams.method = ATTACKMETHOD_RANGED;
			attackParams.pursue = TRUE;
			attackParams.range = FIRERANGE_OPTIMAL;
			message = RADIO_GUARD;
			break;
		case TACTICAL_ORDER_STOP:
			warrior->orderStop(unitOrder, true);
			message = RADIO_ACK;
			break;
		case TACTICAL_ORDER_POWERUP:
			if (delayedTime > -1.0) {
				if (scenarioTime < delayedTime)
					return(TACORDER_FAILURE);
				delayedTime = -1.0;
			}
			warrior->orderPowerUp(unitOrder, origin);
			break;
		case TACTICAL_ORDER_POWERDOWN:
			if (delayedTime > -1.0) {
				if (scenarioTime < delayedTime)
					return(TACORDER_FAILURE);
				delayedTime = -1.0;
			}
			warrior->orderPowerDown(unitOrder, origin);
			break;
		case TACTICAL_ORDER_WAYPOINTS_DONE:
			break;
		case TACTICAL_ORDER_EJECT:
			warrior->orderEject(unitOrder, true, origin);
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT:
			message = RADIO_ILLEGAL_ORDER;
			result = -1;
			if (target)
			{
				if (target->inTransport())
				{
#ifdef USE_ELEMENTALS
					target = ((ElementalPtr)target)->transport;
					targetHandle = target->getHandle();
#endif
				}
				warrior->getVehicle()->calcFireRanges();
				targetObjectClass = target->getObjectClass();
					
				//----------------------------------------------------------------
				// If the target is already disabled, I guess that means we simply
				// want to obliterate the sucker...
				attackParams.obliterate = target->isDisabled();
					
				if (!target->isDestroyed())
				{
					unsigned long params = TACORDER_PARAM_NONE;
					if (attackParams.obliterate)
						params |= TACORDER_PARAM_OBLITERATE;
					if (attackParams.pursue)
						params |= TACORDER_PARAM_PURSUE;
					if (attackParams.tactic != TACTIC_NONE)
						params |= (TACORDER_PARAM_TACTIC_FLANK_RIGHT << (attackParams.tactic - 1));

					// 07-05-00 need to keep run info -- HKG
					if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
						params |= TACORDER_PARAM_RUN;

					result = warrior->orderAttackObject(unitOrder, origin, target, attackParams.type, attackParams.method, attackParams.range, attackParams.aimLocation, moveParams.fromArea, params);
					switch (result)
					{
						case TACORDER_SUCCESS:
							break;
						case TACORDER_FAILURE:
							message = (RollDice(50) ? RADIO_TARGET_ACK : RADIO_ACK);
							break;
					}
				}
			}
			else
			{
				//--------------------------------
				// Targeting a terrain location...
				warrior->setLastTarget(NULL);
				warrior->getVehicle()->calcFireRanges();
				//-------------------------------------------------------------------
				// If we want to refuse the order if the range is beyond our personal
				// attack range, uncomment the following if/then...
				//if (warrior->getVehicle()->getFireRange(attackParams.range) <= warrior->getVehicle()->getFireRange(FIRERANGE_LONGEST)) {
					unsigned long params = TACORDER_PARAM_NONE;
					if (attackParams.obliterate)
						params |= TACORDER_PARAM_OBLITERATE;
					if (attackParams.pursue)
						params |= TACORDER_PARAM_PURSUE;
					if (attackParams.tactic != TACTIC_NONE)
						params |= (TACORDER_PARAM_TACTIC_FLANK_RIGHT << (attackParams.tactic - 1));
					result = warrior->orderAttackPoint(unitOrder, origin, attackParams.targetPoint, attackParams.type, attackParams.method, attackParams.range, params);
					switch (result)
					{
						case TACORDER_SUCCESS:
							break;
						case TACORDER_FAILURE:
							//-------------------------------
							// Now, look for specific case...
							message = (RollDice(50) ? RADIO_TARGET_ACK : RADIO_ACK);
							break;
					}
				//}
			}
			break;
		case TACTICAL_ORDER_ATTACK_POINT: {
			message = RADIO_ILLEGAL_ORDER;
			result = -1;
			//--------------------------------
			// Targeting a terrain location...
			warrior->setLastTarget(NULL);
			warrior->getVehicle()->calcFireRanges();
			//-------------------------------------------------------------------
			// If we want to refuse the order if the range is beyond our personal
			// attack range, uncomment the following if/then...
			//if (warrior->getVehicle()->getFireRange(attackParams.range) <= warrior->getVehicle()->getFireRange(FIRERANGE_LONGEST)) {
				unsigned long params = TACORDER_PARAM_NONE;
				if (attackParams.obliterate)
					params |= TACORDER_PARAM_OBLITERATE;
				if (attackParams.pursue)
					params |= TACORDER_PARAM_PURSUE;
				if (attackParams.tactic != TACTIC_NONE)
					params |= (TACORDER_PARAM_TACTIC_FLANK_RIGHT << (attackParams.tactic - 1));
				result = warrior->orderAttackPoint(unitOrder, origin, attackParams.targetPoint, attackParams.type, attackParams.method, attackParams.range, params);
				switch (result)
				{
					case TACORDER_SUCCESS:
						break;
					case TACORDER_FAILURE:
						//-------------------------------
						// Now, look for specific case...
						message = (RollDice(50) ? RADIO_TARGET_ACK : RADIO_ACK);
						break;
				}
			//}
			}
			break;
		case TACTICAL_ORDER_HOLD_FIRE:
			warrior->orderWait(unitOrder, origin, 0.0, TRUE);
			break;
		case TACTICAL_ORDER_WITHDRAW:
			{
			Stuff::Vector3D location;
			location.x = moveParams.wayPath.points[0];
			location.y = moveParams.wayPath.points[1];
			location.z = moveParams.wayPath.points[2];
			warrior->orderWithdraw( unitOrder, origin, location);
			}
			break;
		case TACTICAL_ORDER_SCRAMBLE: {
			Stuff::Vector3D location;
			location.x = moveParams.wayPath.points[0];
			location.y = moveParams.wayPath.points[1];
			location.z = moveParams.wayPath.points[2];
			//warrior->orderScramble(ORDER_CURRENT, unitOrder, origin, location, moveParams.radius);
			}
			break;
		case TACTICAL_ORDER_REFIT:
			result = TACORDER_SUCCESS;
			if (target && (target->getObjectClass() == BATTLEMECH) && ((MoverPtr)target)->needsRefit())
			{
				if ((warrior->getVehicle()->getObjectClass() == GROUNDVEHICLE) &&
					(((GroundVehiclePtr)warrior->getVehicle())->getRefitPoints() > 0.0) &&
					(warrior->getVehicle()->refitBuddyWID == 0))
				{
					unsigned long params = TACORDER_PARAM_FACE_OBJECT;
					if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
						params |= TACORDER_PARAM_RUN;

//					result = warrior->orderMoveToObject(unitOrder, false, origin, target, moveParams.fromArea, selectionIndex, params);

					int cellRow, cellCol;
					Stuff::Vector3D		destination;
					land->worldToCell(target->getPosition(), cellRow, cellCol);
//					cellRow++;
//					cellCol++;
					land->cellToWorld(cellRow, cellCol, destination);

					result = warrior->orderMoveToPoint(unitOrder, false, origin, destination, selectionIndex, params);
					time = 0;
					if (result == TACORDER_FAILURE)
					{
						((MoverPtr)target)->refitBuddyWID = warrior->getVehicle()->getWatchID();
						warrior->getVehicle()->refitBuddyWID = target->getWatchID();
					}
				}

				if (result != TACORDER_FAILURE)
					stage = -1;	// order failed
			}
			break;
		case TACTICAL_ORDER_GETFIXED:
			result = TACORDER_SUCCESS;
			if (target && (target->getObjectClass() == BUILDING)) {
				BuildingPtr repairBuilding = (BuildingPtr)target;
				if ((repairBuilding->getRefitPoints() > 0.0) &&
					(repairBuilding->refitBuddyWID == 0) &&
					((warrior->getVehicle()->getObjectClass() == BATTLEMECH && repairBuilding->getFlag(OBJECT_FLAG_MECHBAY)) ||
					 (warrior->getVehicle()->getObjectClass() == GROUNDVEHICLE && !repairBuilding->getFlag(OBJECT_FLAG_MECHBAY))))
				{
					unsigned long params = TACORDER_PARAM_FACE_OBJECT;
					if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
						params |= TACORDER_PARAM_RUN;

					result = warrior->orderMoveToPoint(unitOrder, false, origin, repairBuilding->getPosition(), selectionIndex, params);
					time = 0;
					if (result == TACORDER_FAILURE)
					{
						repairBuilding->refitBuddyWID = warrior->getVehicle()->getWatchID();
						warrior->getVehicle()->refitBuddyWID = target->getWatchID();
					}
				}

				if (result!= TACORDER_FAILURE)
					stage = -1;
			}
			break;
		case TACTICAL_ORDER_LOAD_INTO_CARRIER:
			result = TACORDER_SUCCESS;
#ifdef USE_ELEMENTALS
			if ((warrior->getVehicle()->getObjectClass() == ELEMENTAL) &&
				target && (target->getObjectClass() == GROUNDVEHICLE) &&
				((GroundVehiclePtr)target)->elementalCarrier)
				result = ((GameObjectPtr) target)->getPilot()->orderMoveToObject(false, false, origin, warrior->getVehicle(), moveParams.fromArea);
#endif
			if (result != TACORDER_FAILURE)
				stage = -1;
			break;
		case TACTICAL_ORDER_DEPLOY_ELEMENTALS: {
			unsigned long params = TACORDER_PARAM_NONE;
			if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
				params |= TACORDER_PARAM_RUN;
			if (moveParams.wait)
				params |= TACORDER_PARAM_WAIT;
			result = TACORDER_SUCCESS;
			if (result == TACORDER_SUCCESS)
				stage = -1;
			}
			break;
		case TACTICAL_ORDER_CAPTURE:
			result = TACORDER_SUCCESS;
			if (target && (target->isCaptureable(warrior->getTeam()->getId())) && !warrior->getVehicle()->isFriendly(target->getTeam())) {
				if (!target->getCaptureBlocker(warrior->getVehicle())) {
					bool prison = false;
					if (target->isBuilding()) {
						if ((target->getObjectClass() == BUILDING) && ((BuildingPtr)target)->isPrison())
							prison = true;
					}

					// if this isn't a prison, go for it. if it is, only a ground vehicle with seats for pilots can capture it
					// NOTHING IS EVER A PRISON NOW!!
					result = TACORDER_FAILURE;
				}
			}

			if (result == TACORDER_FAILURE) 
			{
				//What if we're already there??
				float distanceToGoal = warrior->getVehicle()->distanceFrom(target->getPosition());
				result = TACORDER_FAILURE;
				GameObjectPtr blockerList[MAX_MOVERS];
				long numBlockers = target->getCaptureBlocker(warrior->getVehicle(), blockerList);
				if (target->getTeam() == warrior->getTeam()) 
				{
					//--------------------------------
					// our team already captured it...
					result = TACORDER_SUCCESS;
				}
				else if (numBlockers > 0) 
				{
					statusCode = 1;
					result = TACORDER_SUCCESS;
					warrior->radioMessage(RADIO_CANNOT_CAPTURE, true);
				}
				else if (distanceToGoal < 30.0 && target->isCaptureable(warrior->getTeam()->getId()))
				{
					switch (target->getObjectClass()) 
					{
						case BUILDING:
							if (MPlayer) {
								if (MPlayer->isServer())
									MPlayer->addCaptureBuildingChunk((BuildingPtr)target, ((BuildingPtr)target)->getCommanderId(), warrior->getVehicle()->getCommanderId());
								if (target->getObjectType()->getObjTypeNum() == GENERIC_HQ_BUILDING_OBJNUM)
									if (MPlayer->missionSettings.missionType == MISSION_TYPE_CAPTURE_BASE)
										target->setFlag(OBJECT_FLAG_CAPTURABLE, false);
							}
							target->setCommanderId(warrior->getVehicle()->getCommanderId());
							target->setTeamId(warrior->getTeam()->getId(),false);

							result = TACORDER_SUCCESS;
							warrior->radioMessage(RADIO_CAPTURED_BUILDING, TRUE);
							break;

						case GROUNDVEHICLE: 
						{
							int cellRow, cellCol;
							target->getCellPosition(cellRow, cellCol);
							target->setTeamId(warrior->getTeam()->getId(),false);
							result = TACORDER_SUCCESS;
							warrior->radioMessage(RADIO_CAPTURED_VEHICLE, true);
						}
						break;

						case BATTLEMECH: 
						{
							int cellRow, cellCol;
							target->getCellPosition(cellRow, cellCol);
						}
						break;
					}
				}

				if (result == TACORDER_FAILURE)
				{
					unsigned long params = TACORDER_PARAM_NONE;
					if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
						params |= TACORDER_PARAM_RUN;
					if (moveParams.faceObject)
						params |= TACORDER_PARAM_FACE_OBJECT;
					message = RADIO_ACK;
					result = warrior->orderMoveToObject(false, false, origin, target, moveParams.fromArea, 0, params);
				}
			}
			else
				message = RADIO_CANNOT_CAPTURE;

			if (result == TACORDER_SUCCESS)
				stage = -1;
			break;
		case TACTICAL_ORDER_RECOVER:
			result = TACORDER_SUCCESS;
			if (target && (target->getObjectClass() == BATTLEMECH))	{
				if ((warrior->getVehicle()->getObjectClass() == GROUNDVEHICLE) &&
					(((GroundVehiclePtr)warrior->getVehicle())->getRecoverPoints() > 0.0) &&
					(warrior->getVehicle()->recoverBuddyWID == 0))
				{
					unsigned long params = TACORDER_PARAM_FACE_OBJECT;
					if (moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST)
						params |= TACORDER_PARAM_RUN;
					result = warrior->orderMoveToObject(unitOrder, false, origin, target, moveParams.fromArea, selectionIndex, params);
					time = 0;
					if (result == TACORDER_FAILURE)	{
						((MoverPtr)target)->recoverBuddyWID = warrior->getVehicle()->getWatchID();
						warrior->getVehicle()->recoverBuddyWID = target->getWatchID();
					}
				}

				if (result != TACORDER_FAILURE)
					stage = -1;	// order failed
			}
			break;
	}

	if (origin == ORDER_ORIGIN_PLAYER)
		warrior->triggerAlarm(PILOT_ALARM_PLAYER_ORDER, code);
	else
		message = -1;

	if (code != TACTICAL_ORDER_WITHDRAW) {
		if (warrior->getVehicle()->getStatus() != OBJECT_STATUS_DESTROYED) {
			//-------------------------------------------------------------------
			// Any non-withdraw order forces us out of withdraw mode, if in it...
			((MoverPtr)warrior->getVehicle())->withdrawing = false;
		}
	}

	return(result);
}

//---------------------------------------------------------------------------
extern Mission *mission;

long TacticalOrder::status (MechWarriorPtr warrior) {

	long result = TACORDER_SUCCESS;

	GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
	switch (code) {
		case TACTICAL_ORDER_WAIT:
			result = (scenarioTime > delayedTime) ? TACORDER_SUCCESS : TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_MOVETO_POINT: {
			long formation = -1;
			long formationPos = -1;
			if ((formation > -1) && (formationPos > 0))
				result = TACORDER_FAILURE;
			else {
				if (delayedTime > -1.0) {
					if (scenarioTime < delayedTime)
						return(TACORDER_FAILURE);
					delayedTime = -1.0;
					long message;
					execute(warrior, message);
				}
				Stuff::Vector3D location;
				location.x = moveParams.wayPath.points[0];
				location.y = moveParams.wayPath.points[1];
				location.z = moveParams.wayPath.points[2];
				//float distanceToGoal = warrior->getVehicle()->distanceFrom(location);
				long cellDistanceToGoal = warrior->getVehicle()->cellDistanceFrom(location);
				if (cellDistanceToGoal < 1 /*< Mover::marginOfError[1]*/) {
					if (moveParams.wait) {
						code = TACTICAL_ORDER_WAIT;
						result = TACORDER_FAILURE;
					}
					}
				else
					result = TACORDER_FAILURE;
			}
			}
			break;
		case TACTICAL_ORDER_MOVETO_OBJECT:
			if (target) {
				Stuff::Vector3D location;
				location.x = moveParams.wayPath.points[0];
				location.y = moveParams.wayPath.points[1];
				location.z = moveParams.wayPath.points[2];
				//float distanceToGoal = warrior->getVehicle()->distanceFrom(location);
				long cellDistanceToGoal = warrior->getVehicle()->cellDistanceFrom(location);
				if (cellDistanceToGoal > 0/*Mover::marginOfError[1]*/)
					result = TACORDER_FAILURE;
			//	else if (moveParams.faceObject) {
			//		float relFacing = warrior->getVehicle()->relViewFacingTo(target->getPosition());
			//		float fireArc = warrior->getVehicle()->getFireArc();
			//		if ((relFacing < -fireArc) || (relFacing > fireArc))
			//			result = TACORDER_FAILURE;
			//	}
			}
			break;
		case TACTICAL_ORDER_JUMPTO_POINT:
			if (stage != 3)
				result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_JUMPTO_OBJECT: {
#ifdef USE_JUMPING
			BattleMechPtr mech = (BattleMechPtr)warrior->getVehicle();
			if (!mech->inJump || ((MechActor*)mech->appearance)->inJump)
				result = TACORDER_FAILURE;
#endif
			}
			break;
#if 0
		case TACTICAL_ORDER_TRAVERSE_PATH:
			if (stage == 2)
				result = TACORDER_SUCCESS;
			else
				result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_PATROL_PATH:
			result = TACORDER_FAILURE;
			break;
#endif
		case TACTICAL_ORDER_ESCORT:
		case TACTICAL_ORDER_FOLLOW:
			break;
		case TACTICAL_ORDER_GUARD:
			result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_STOP:
			result = TACORDER_SUCCESS;
			break;
		case TACTICAL_ORDER_POWERUP:
			if (delayedTime > -1.0) {
				if (scenarioTime < delayedTime)
					return(TACORDER_FAILURE);
				delayedTime = -1.0;
				long message;
				execute(warrior, message);
			}
			if (warrior->getVehicleStatus() != OBJECT_STATUS_NORMAL)
				result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_POWERDOWN:
			if (delayedTime > -1.0) {
				if (scenarioTime < delayedTime)
					return(TACORDER_FAILURE);
				delayedTime = -1.0;
				long message;
				execute(warrior, message);
			}
			//-----------------------------------------------------------------
			// For now, they're always powering down until given a new order...
			result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_WAYPOINTS_DONE:
			break;
		case TACTICAL_ORDER_EJECT:
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT:
			if (target) {
#ifdef USE_ELEMENTALS
				if ((target->getObjectClass() == ELEMENTAL) && ((ElementalPtr)target)->transport) {
					target = ((ElementalPtr)target)->transport;
					targetHandle = target->getHandle();
				}
#endif
				if (target->isDestroyed())
					result = TACORDER_SUCCESS;
				else if (target->isDisabled()) {
					if (attackParams.obliterate)
						result = TACORDER_FAILURE;
					else
						result = TACORDER_SUCCESS;
					}
				else
					result = TACORDER_FAILURE;
			}
			break;
		case TACTICAL_ORDER_ATTACK_POINT:
			result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_HOLD_FIRE:
			result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_WITHDRAW:
			result = TACORDER_FAILURE;
			break;
		case TACTICAL_ORDER_SCRAMBLE: {
			result = TACORDER_FAILURE;
			long formation = -1;
			long formationPos = -1;
			if ((formation > -1) && (formationPos > 0))
				result = TACORDER_FAILURE;
			else {
				Stuff::Vector3D location;
				location.x = moveParams.wayPath.points[0];
				location.y = moveParams.wayPath.points[1];
				location.z = moveParams.wayPath.points[2];
				float distanceToGoal = warrior->getVehicle()->distanceFrom(location);
				if (distanceToGoal < 10.0) {
					if (moveParams.wait) {
						//-------------------
						// Calc a new goal...
					}
				}
			}
			}
			break;

		case TACTICAL_ORDER_REFIT:
			// has refitter or refitee gotten distracted?
			if (!target || (warrior->getVehicle()->refitBuddyWID == 0) || (((MoverPtr)target)->refitBuddyWID == 0))
				result = TACORDER_SUCCESS;
			else 
			{
				// refitter and refitee *should* be pointing at each other...
				//
				if (warrior->getVehicle()->refitBuddyWID != target->getWatchID())
					warrior->getVehicle()->refitBuddyWID = target->getWatchID();
				if (((MoverPtr) target)->refitBuddyWID != warrior->getVehicle()->getWatchID())
					((MoverPtr) target)->refitBuddyWID = warrior->getVehicle()->getWatchID();

				result = TACORDER_FAILURE;
				switch(stage) 
				{
					case 1:	// move
						if (warrior->getMovePath()->numStepsWhenNotPaused == 0 && warrior->getMovePathRequest() == NULL)
						{
							if (warrior->getVehicle()->distanceFrom(target->getPosition()) < Mover::refitRange)
							{
								stage++;
							}
							else
							{
								int cellRow, cellCol;
								Stuff::Vector3D		destination;
								land->worldToCell(target->getPosition(), cellRow, cellCol);
								cellRow++;
								cellCol++;
								land->cellToWorld(cellRow, cellCol, destination);
								unsigned long params = TACORDER_PARAM_FACE_OBJECT;
								result = warrior->orderMoveToPoint(unitOrder, false, origin, destination, selectionIndex, params);
							}
						}
						break;
					case 2: // power-down (or other cool animation...)
						if (time == 0)
						{
							target->getPilot()->orderPowerDown(unitOrder, ORDER_ORIGIN_SELF);
							time = scenarioTime;
						}
						else if (scenarioTime > time + 3)
						{
							stage++;
							time = scenarioTime;
						}
						break;
					case 3:	// do the deed
						((GroundVehiclePtr)warrior->getVehicle())->refitting = true;
						if (scenarioTime > time + Mover::refitTime)
						{
							float pointsUsed = 0;
							float pointsAvailable = warrior->getVehicle()->getRefitPoints();
							long result = ((MoverPtr)target)->refit(pointsAvailable, pointsUsed);
							warrior->getVehicle()->burnRefitPoints(pointsUsed);
							if (warrior->getVehicle()->getRefitPoints() <= 0)
							{
								//I died trying to repair someone.  Power the someone back up.
								target->getPilot()->orderPowerUp(unitOrder, ORDER_ORIGIN_SELF);
								((MoverPtr)target)->refitBuddyWID = 0;
								warrior->getVehicle()->refitBuddyWID = 0;
							}

							if (MPlayer) 
							{
								if (pointsAvailable > 255.0f)
									pointsAvailable = 255.0f;

								WeaponShotInfo refitInfo;
								refitInfo.init(0,
											   -5,
											   pointsUsed,
											   GROUNDVEHICLE_LOCATION_TURRET,
											   0.0);
								MPlayer->addWeaponHitChunk((GameObjectPtr)warrior->getVehicle(), &refitInfo);
								refitInfo.init(0,
											   -5,
											   pointsAvailable,
											   0,
											   0.0);
								MPlayer->addWeaponHitChunk(target, &refitInfo, TRUE);
							}
							stage += result;
							if (result == 0)
								time = scenarioTime;
							if (!warrior->getVehicle()->getRefitPoints())
								warrior->getVehicle()->disable(POWER_USED_UP);

						}
						break;
					case 4: // done. power-up (or whatever) and report
						((GroundVehiclePtr)warrior->getVehicle())->refitting = false;

						target->getPilot()->orderPowerUp(unitOrder, ORDER_ORIGIN_SELF);

						result = TACORDER_SUCCESS;
						((MoverPtr)target)->refitBuddyWID = 0;
						warrior->getVehicle()->refitBuddyWID = 0;

						break;
					case -1:	// execute failed
						result = TACORDER_SUCCESS;
						break;
				}
			}
			break;
		case TACTICAL_ORDER_GETFIXED:
			if (!target || (warrior->getVehicle()->refitBuddyWID == 0) || (((BuildingPtr)target)->refitBuddyWID == 0))
				result = TACORDER_SUCCESS;
			else
			{
				// refitter and refitee *should* be pointing at each other...
				Assert(warrior->getVehicle()->refitBuddyWID == target->getWatchID() &&
						((BuildingPtr)target)->refitBuddyWID == warrior->getVehicle()->getWatchID(), 0, "Refitee and refitter aren't pointing at each other.");
				result = TACORDER_FAILURE;
				switch(stage) {
					case 1:	// move
						if (warrior->getMovePath()->numStepsWhenNotPaused == 0 && warrior->getMovePathRequest() == NULL)
							stage++;
						break;
					case 2: // power-down (or other cool animation...)
						if (time == 0)
						{
							warrior->orderPowerDown(unitOrder, ORDER_ORIGIN_SELF);
							time = scenarioTime;
						}
						else if (scenarioTime > time + 3)
						{
							stage++;
							time = scenarioTime;
							soundSystem->playDigitalSample( REPAIRBAY_FX, target->getPosition() );
						}
						break;
					case 3:	// do the deed
						if (scenarioTime > time + Mover::refitTime)
						{
							float pointsUsed = 0;
							float pointsAvailable = target->getRefitPoints();
							long result = warrior->getVehicle()->refit(pointsAvailable, pointsUsed);
							target->burnRefitPoints(pointsUsed);
							if (MPlayer) 
							{
								if (pointsAvailable > 255.0f)
									pointsAvailable = 255.0f;

								WeaponShotInfo refitInfo;
								refitInfo.init(0,
											   -5,
											   pointsUsed,
											   -1,
											   0.0);
								MPlayer->addWeaponHitChunk(target, &refitInfo);
								refitInfo.init(0,
											   -5,
											   pointsAvailable,
											   0,
												0.0);
								MPlayer->addWeaponHitChunk(warrior->getVehicle(), &refitInfo, true);
							}

							stage += result;
							if (result == 0)
							{
								time = scenarioTime;
							}
							else  // power-up (or whatever) and walk off
							{
								warrior->getVehicle()->startUp();
								{
									BuildingPtr repairBuilding = (BuildingPtr)target;
									int			tileRow, tileCol;
									Stuff::Vector3D		destination;
									land->worldToTile(repairBuilding->getPosition(), tileRow, tileCol);
									tileRow++;
									tileCol++;
									land->tileCellToWorld(tileRow, tileCol, 2, 2, destination);
									warrior->orderMoveToPoint(unitOrder, false, ORDER_ORIGIN_SELF, destination, selectionIndex, 0);
								}
							}
						}
						break;

					case 4: // walking off...
						if (warrior->getMovePath()->numStepsWhenNotPaused == 0 && warrior->getMovePathRequest() == NULL)
							stage++;
						break;

					case 5: // done. report
						result = TACORDER_SUCCESS;
						((BuildingPtr)target)->refitBuddyWID = 0;
						warrior->getVehicle()->refitBuddyWID = 0;
						break;
					case -1:	// execute failed
						result = TACORDER_SUCCESS;
						break;
				}
			}
			break;

		case TACTICAL_ORDER_LOAD_INTO_CARRIER: {
			result = TACORDER_FAILURE;
#ifdef USE_ELEMENTALS
			MoverGroupPtr myGroup = warrior->getGroup();
			if (!myGroup)
				break;
			switch (stage) {
				case 1:
					if (warrior->getVehicle()->getPosition().distance_from(target->getPosition()) < 200)
					{
						warrior->clearMoveOrders();
						if (myGroup)
							for (long i = 0; i < myGroup->getNumMovers(); i++)
								myGroup->getMover(i)->getPilot()->clearMoveOrders();	// stop!
						
						target->getPilot()->clearMoveOrders();	// stop!

						time = scenarioTime;
						stage++;	// not really stage here. see below
					}
					break;
				case 2:
					if ((scenarioTime > time + 3) && myGroup) {
						for (long i = 0; i< myGroup->getNumMovers(); i++)
							myGroup->getMover(i)->getPilot()->orderMoveToObject(false, false, ORDER_ORIGIN_SELF, target, moveParams.fromArea);
						stage++;
					}
					break;
				case 3:
					if (scenarioTime > time + 5) {
						for (long i=0; i < myGroup->getNumMovers(); i++) {
							((ElementalPtr)myGroup->getMover(i))->transport = (MoverPtr)target;
							for (int j=0; j<MAX_TOADS; j++)
							{
								if (((GroundVehiclePtr)target)->toads[j] == 0)	{
									((GroundVehiclePtr)target)->toads[j] = myGroup->getMover(i/*-1*/)->getHandle();
									break;
								}
							}
						}
						result = TACORDER_SUCCESS;
					}
					break;
				case -1:	// execute failed
					result = TACORDER_SUCCESS;
					break;

			}
#endif
			}
			break;

		case TACTICAL_ORDER_DEPLOY_ELEMENTALS:
#ifdef USE_ELEMENTALS
			if (stage != -1)
			{
				result = TACORDER_FAILURE;
				if (scenarioTime > time + 5)
				{
				short	deployCount = 5;
				int		i;

					for (i=0; i<MAX_TOADS; i++)
					{
					ElementalPtr	toad = (ElementalPtr) (BaseObjectPtr) ((GroundVehiclePtr) warrior->getVehicle())->toads[i];

						if (toad)
						{
						vector_2d	offset;
						Stuff::Vector3D	targetLocation;
						float		angle;

							deployCount--;
							offset.x = 50;
							offset.y = 50;
							angle = deployCount * 72;
							offset.rotate(angle);
							targetLocation = warrior->getVehicle()->getPosition();
							targetLocation.x += offset.x;
							targetLocation.y += offset.y;
							toad->setPosition(targetLocation);
							toad->transport = NULL;
							((GroundVehiclePtr) warrior->getVehicle())->toads[i] = NULL;
							if (deployCount == 0)
							{
								time = scenarioTime;	// come back for another load
								break;
							}
						}
					}
					if (i == MAX_TOADS)
						result = TACORDER_SUCCESS;
				}
			}
#endif
			break;

		case TACTICAL_ORDER_CAPTURE:
			if (stage != -1) 
			{
				float distanceToGoal = warrior->getVehicle()->distanceFrom(target->getPosition());
				result = TACORDER_FAILURE;
				GameObjectPtr blockerList[MAX_MOVERS];
				long numBlockers = target->getCaptureBlocker(warrior->getVehicle(), blockerList);
				if (target->getTeam() == warrior->getTeam()) {
					//--------------------------------
					// our team already captured it...
					result = TACORDER_SUCCESS;
				}
				else if (numBlockers > 0) {
					// is someone blocking the capture now?
					//if (warrior->getTeam() == Team::home) {
						statusCode = 1;
						result = TACORDER_SUCCESS;
						warrior->radioMessage(RADIO_CANNOT_CAPTURE, true);
					/*
						}
					else
					{
						//-----------------------------------------------------
						// Why is it blocked, and can we do something about it?
						if (MPlayer)
							Fatal(0, " MULTIPLAYER! Handle this case: tacorder.status--capture ");
						if ((origin == ORDER_ORIGIN_SELF) && (numBlockers > 3)) {
							TacticalOrder alarmTacOrder;
							alarmTacOrder.init(ORDER_ORIGIN_PLAYER,  TACTICAL_ORDER_ATTACK_OBJECT);
							alarmTacOrder.playerSubOrder = true;
							alarmTacOrder.targetWID = target->getWatchID();
							alarmTacOrder.attackParams.type = ATTACK_TO_DESTROY;
							alarmTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
							alarmTacOrder.attackParams.aimLocation = -1;
							alarmTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
							alarmTacOrder.attackParams.pursue = true;
							alarmTacOrder.attackParams.obliterate = false;
							alarmTacOrder.moveParams.wayPath.mode[0] = TRAVEL_MODE_FAST;
							warrior->setAlarmTacOrder(alarmTacOrder, 255);
						}
						else {
							TacticalOrder alarmTacOrder;
							alarmTacOrder.init(ORDER_ORIGIN_PLAYER,  TACTICAL_ORDER_ATTACK_OBJECT);
							alarmTacOrder.playerSubOrder = true;
							alarmTacOrder.targetWID = blockerList[0]->getWatchID();
							alarmTacOrder.attackParams.type = ATTACK_TO_DESTROY;
							alarmTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
							alarmTacOrder.attackParams.aimLocation = -1;
							alarmTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
							alarmTacOrder.attackParams.pursue = true;
							alarmTacOrder.attackParams.obliterate = false;
							alarmTacOrder.moveParams.wayPath.mode[0] = TRAVEL_MODE_FAST;
							warrior->setAlarmTacOrder(alarmTacOrder, 255);
						}
					}
					*/
				}
				else if (distanceToGoal < 30.0 && target->isCaptureable(warrior->getTeam()->getId()))
				{

					switch (target->getObjectClass()) 
					{
						case BUILDING:
							if (MPlayer) {
								if (MPlayer->isServer())
									MPlayer->addCaptureBuildingChunk((BuildingPtr)target, ((BuildingPtr)target)->getCommanderId(), warrior->getVehicle()->getCommanderId());
								if (target->getObjectType()->getObjTypeNum() == GENERIC_HQ_BUILDING_OBJNUM)
									if (MPlayer->missionSettings.missionType == MISSION_TYPE_CAPTURE_BASE)
										target->setFlag(OBJECT_FLAG_CAPTURABLE, false);
							}
							target->setCommanderId(warrior->getVehicle()->getCommanderId());
							target->setTeamId(warrior->getTeam()->getId(),false);
							result = TACORDER_SUCCESS;
							warrior->radioMessage(RADIO_CAPTURED_BUILDING, TRUE);
							break;

						case GROUNDVEHICLE: 
						{
							int cellRow, cellCol;
							target->getCellPosition(cellRow, cellCol);
							target->setTeamId(warrior->getTeam()->getId(),false);
							result = TACORDER_SUCCESS;
							warrior->radioMessage(RADIO_CAPTURED_VEHICLE, true);
						}
						break;

						case BATTLEMECH: 
						{
							int cellRow, cellCol;
							target->getCellPosition(cellRow, cellCol);
						}
						break;
					}
				}
			}
			break;
		case TACTICAL_ORDER_RECOVER:
			// has recoverer gotten distracted?
			STOP(("RECOVER TACORDER NO LONGER VALID"));
#if 0
			if (!target || (warrior->getVehicle()->recoverBuddyWID == 0) || (((MoverPtr)target)->recoverBuddyWID == 0))
				result = TACORDER_SUCCESS;
			else {
				// refitter and refitee *should* be pointing at each other...
				//Assert(warrior->getVehicle()->recoverBuddyWID == target->getWatchID() &&
				//		((MoverPtr) target)->recoverBuddyWID == warrior->getVehicle()->getWatchID(), 0, "Recoveree and recoverer aren't pointing at each other.");
				result = TACORDER_FAILURE;
				switch(stage) {
					case 1:	// move
						if (warrior->getVehicle()->distanceFrom(target->getPosition()) < Mover::recoverRange)
							stage++;
						break;
					case 2: // power-down (or other cool animation...)
						if (target->getObjectClass() == BATTLEMECH) {
							((BattleMechPtr)target)->leftArmBlownThisFrame = false;
							((BattleMechPtr)target)->rightArmBlownThisFrame = false;
							((MoverPtr)target)->getPilot()->clearAlarms();
						}
						if (target->isDisabled())
							target->setStatus(OBJECT_STATUS_SHUTDOWN);
						mission->tradeMover((Mover*)target, Team::home->getId(), Commander::home->getId(), 
							(char*)LogisticsData::instance->getBestPilot( (target)->tonnage ), "pbrain");
						((MoverPtr)target)->recoverBuddyWID = warrior->getVehicle()->getWatchID();
						warrior->getVehicle()->recoverBuddyWID = target->getWatchID();
						stage++;
						break;
					case 3:
						if (time <= 0.0) {
							target->getPilot()->orderPowerDown(unitOrder, ORDER_ORIGIN_SELF);
							time = scenarioTime;
						}
						else if (scenarioTime > time + 3.0) {
							stage++;
							time = scenarioTime;
						}
						break;
					case 4:	// do the deed
						((GroundVehiclePtr)warrior->getVehicle())->refitting = true;
						if (scenarioTime > time + Mover::refitTime) {
							float pointsUsed = 0;
							long result = ((MoverPtr)target)->recover(warrior->getVehicle()->getRecoverPoints(), pointsUsed);
							warrior->getVehicle()->burnRecoverPoints(pointsUsed);
							if (MPlayer) {
								STOP((" Need to implement recover for multiplayer "));
								WeaponShotInfo recoverInfo;
								recoverInfo.init(NULL,
											   -5,
											   pointsUsed,
											   GROUNDVEHICLE_LOCATION_TURRET,
											   0.0);
								MPlayer->addWeaponHitChunk((GameObjectPtr)warrior->getVehicle(), &recoverInfo);
								recoverInfo.init(NULL,
											   -5,
											   pointsUsed,
											   0,
											   0.0);
								MPlayer->addWeaponHitChunk(target, &recoverInfo, TRUE);
							}
							stage += result;
							if (result == 0)
								time = scenarioTime;
						}
						break;
					case 5: // done. power-up (or whatever) and report
						((GroundVehiclePtr)warrior->getVehicle())->recovering = false;
						target->getPilot()->orderPowerUp(unitOrder, ORDER_ORIGIN_SELF);
						result = TACORDER_SUCCESS;
						((MoverPtr)target)->recoverBuddyWID = 0;
						warrior->getVehicle()->recoverBuddyWID = 0;
						warrior->getVehicle()->setSelected( 0 );
						warrior->getVehicle()->disable(POWER_USED_UP) ;
						break;
					case -1:	// execute failed
						result = TACORDER_SUCCESS;
						break;
				}
			}
#endif
			break;
	}

	return(result);
}

//---------------------------------------------------------------------------

GameObjectPtr TacticalOrder::getTarget (void) {

	return(ObjectManager->getByWatchID(targetWID));
}

//---------------------------------------------------------------------------

bool TacticalOrder::equals (TacticalOrder* tacOrder) {

	if (code != tacOrder->code)
		return(false);
	if (targetWID != tacOrder->targetWID)
		return(false);
	if (stage != tacOrder->stage)
		return(false);
	if (unitOrder != tacOrder->unitOrder)
		return(false);
	if (origin != tacOrder->origin)
		return(false);
	if (groupFlags != tacOrder->groupFlags)
		return(false);

	switch (code) {
		case TACTICAL_ORDER_WAIT:
			break;
		case TACTICAL_ORDER_MOVETO_POINT: {
			if (moveParams.faceObject != tacOrder->moveParams.faceObject)
				return(false);
			if (moveParams.wait != tacOrder->moveParams.wait)
				return(false);
			if (moveParams.mode != tacOrder->moveParams.mode)
				return(false);
			if (moveParams.escapeTile != tacOrder->moveParams.escapeTile)
				return(false);
			if (moveParams.wayPath.numPoints != tacOrder->moveParams.wayPath.numPoints)
				return(false);
			long numCoords = moveParams.wayPath.numPoints * 3;
			for (long i = 0; i < numCoords; i++) {
				if (moveParams.wayPath.points[i] != tacOrder->moveParams.wayPath.points[i])
					return(false);
				if (moveParams.wayPath.mode[i] != tacOrder->moveParams.wayPath.mode[i])
					return(false);
			}
			}
			break;
		case TACTICAL_ORDER_MOVETO_OBJECT:
			break;
		case TACTICAL_ORDER_JUMPTO_POINT:
			break;
		case TACTICAL_ORDER_JUMPTO_OBJECT:
			break;
		case TACTICAL_ORDER_TRAVERSE_PATH:
			break;
		case TACTICAL_ORDER_PATROL_PATH:
			break;
		case TACTICAL_ORDER_ESCORT:
			break;
		case TACTICAL_ORDER_FOLLOW:
			break;
		case TACTICAL_ORDER_GUARD:
			break;
		case TACTICAL_ORDER_STOP:
			break;
		case TACTICAL_ORDER_POWERUP:
			break;
		case TACTICAL_ORDER_POWERDOWN:
			break;
		case TACTICAL_ORDER_WAYPOINTS_DONE:
			break;
		case TACTICAL_ORDER_EJECT:
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT:
			if (attackParams.method != tacOrder->attackParams.method)
				return(false);
			if (attackParams.range != tacOrder->attackParams.range)
				return(false);
			if (attackParams.aimLocation != tacOrder->attackParams.aimLocation)
				return(false);
			if (attackParams.pursue != tacOrder->attackParams.pursue)
				return(false);
			if (attackParams.obliterate != tacOrder->attackParams.obliterate)
				return(false);
			if (attackParams.tactic != tacOrder->attackParams.tactic)
				return(false);
			break;
		case TACTICAL_ORDER_ATTACK_POINT:
			if (attackParams.method != tacOrder->attackParams.method)
				return(false);
			if (attackParams.range != tacOrder->attackParams.range)
				return(false);
			if (attackParams.aimLocation != tacOrder->attackParams.aimLocation)
				return(false);
			if (attackParams.pursue != tacOrder->attackParams.pursue)
				return(false);
			if (attackParams.obliterate != tacOrder->attackParams.obliterate)
				return(false);
			if (attackParams.targetPoint != tacOrder->attackParams.targetPoint)
				return(false);
			if (attackParams.tactic != tacOrder->attackParams.tactic)
				return(false);
			break;
		case TACTICAL_ORDER_HOLD_FIRE:
			break;
		case TACTICAL_ORDER_WITHDRAW:
			break;
		case TACTICAL_ORDER_SCRAMBLE:
			break;
		case TACTICAL_ORDER_CAPTURE:
			break;
		case TACTICAL_ORDER_REFIT:
			break;
		case TACTICAL_ORDER_GETFIXED:
			break;
		case TACTICAL_ORDER_LOAD_INTO_CARRIER:
			break;
		case TACTICAL_ORDER_DEPLOY_ELEMENTALS:
			break;
	}

	return(true);
}

//---------------------------------------------------------------------------

void TacticalOrder::destroy (void) {
}

//***************************************************************************

void TacticalOrder::debugString (MechWarriorPtr pilot, char* s) {

	GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
	switch (code) {
		case TACTICAL_ORDER_NONE:
			strcpy(s, "none");
			break;
		case TACTICAL_ORDER_WAIT:
			strcpy(s, "wait");
			break;
		case TACTICAL_ORDER_MOVETO_POINT: {
			Stuff::Vector3D curPos;
			curPos.x = moveParams.wayPath.points[0];
			curPos.y = moveParams.wayPath.points[1];
			curPos.z = 0.0;
			int cell[2];
			land->worldToCell(curPos, cell[0], cell[1]);
			sprintf(s, "move to pt (%.0f, %.0f) [%d, %d]",
				moveParams.wayPath.points[0],
				moveParams.wayPath.points[1],
				cell[0], cell[1]);
			}
			break;
		case TACTICAL_ORDER_MOVETO_OBJECT:
			sprintf(s, "move to obj (%d)", target ? target->getPartId() : 0);
			break;
		case TACTICAL_ORDER_JUMPTO_POINT: {
			Stuff::Vector3D curPos;
			curPos.x = moveParams.wayPath.points[0];
			curPos.y = moveParams.wayPath.points[1];
			curPos.z = 0.0;
			int cell[2];
			land->worldToCell(curPos, cell[0], cell[1]);
			sprintf(s, "jump to pt (%.0f, %.0f), cell:(%d, %d)",
				moveParams.wayPath.points[0],
				moveParams.wayPath.points[1],
				cell[0], cell[1]);
			}
			break;
		case TACTICAL_ORDER_JUMPTO_OBJECT:
			sprintf(s, "jump to obj (%d)", target ? target->getPartId() : 0);
			break;
		case TACTICAL_ORDER_TRAVERSE_PATH:
			strcpy(s, "traverse path");
			break;
		case TACTICAL_ORDER_PATROL_PATH:
			strcpy(s, "patrol path");
			break;
		case TACTICAL_ORDER_ESCORT:
			strcpy(s, "escort");
			break;
		case TACTICAL_ORDER_FOLLOW:
			strcpy(s, "follow");
			break;
		case TACTICAL_ORDER_GUARD:
			strcpy(s, "guard");
			break;
		case TACTICAL_ORDER_STOP:
			strcpy(s, "stop");
			break;
		case TACTICAL_ORDER_POWERUP:
			strcpy(s, "power up");
			break;
		case TACTICAL_ORDER_POWERDOWN:
			strcpy(s, "power down");
			break;
		case TACTICAL_ORDER_WAYPOINTS_DONE:
			strcpy(s, "waypoints done");
			break;
		case TACTICAL_ORDER_EJECT:
			strcpy(s, "eject");
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT: {
			const char* tacticCode[] = {"  ", "RT", "LT", "RR", "SF", "TT", "JS"};
			sprintf(s, "attack obj (%d) %s", target ? target->getPartId() : 0, tacticCode[attackParams.tactic]);
			}
			break;
		case TACTICAL_ORDER_ATTACK_POINT: {
			const char* tacticCode[] = {"  ", "RT", "LT", "RR", "SF", "TT", "JS"};
			sprintf(s, "attack pt (%.0f, %.0f) %s",
				attackParams.targetPoint.x,
				attackParams.targetPoint.y,
				tacticCode[attackParams.tactic]);
			}
			break;
		case TACTICAL_ORDER_HOLD_FIRE:
			strcpy(s, "hold fire");
			break;
		case TACTICAL_ORDER_WITHDRAW:
			strcpy(s, "withdraw");
			break;
		case TACTICAL_ORDER_SCRAMBLE:
			strcpy(s, "scramble");
			break;
		case TACTICAL_ORDER_CAPTURE: {
			Stuff::Vector3D goalPos;
			GameObjectPtr obj;
			long goalType = pilot->getMoveGoal(&goalPos, &obj);
			if ((goalType > 0) && obj) {
				sprintf(s, "capture obj (%d) [%d]", target ? target->getPartId() : 0, obj->getPartId());
				}
			else
				sprintf(s, "capture obj (%d) [GOAL?]", target ? target->getPartId() : 0);
			}
			break;
		case TACTICAL_ORDER_REFIT:
			sprintf(s, "refit obj (%d)", target ? target->getPartId() : 0);
			break;
		case TACTICAL_ORDER_GETFIXED:
			sprintf(s, "get fixed (%d)", target ? target->getPartId() : 0);
			break;
		case TACTICAL_ORDER_LOAD_INTO_CARRIER:
			strcpy(s, "load carrier");
			break;
		case TACTICAL_ORDER_DEPLOY_ELEMENTALS:
			strcpy(s, "deploy elementals");
			break;
	}
}

//***************************************************************************

