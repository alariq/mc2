//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------------------------
// Include Files
#ifndef ABL_H
#include"abl.h"
#endif

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef MOVE_H
#include"move.h"
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

#ifndef TURRET_H
#include"turret.h"
#endif

#ifndef GATE_H
#include"gate.h"
#endif

#ifndef ARTLRY_H
#include"artlry.h"
#endif

#ifndef GAMELOG_H
#include"gamelog.h"
#endif

#ifndef MISSIONBEGIN_H
#include"missionbegin.h"
#endif

#ifndef LOGISTICS_H
#include"logistics.h"
#endif

MoverGroupPtr			CurGroup = NULL;
GameObjectPtr			CurObject = NULL;
long					CurObjectClass = 0;
MechWarriorPtr			CurWarrior = NULL;
GameObjectPtr			CurContact = NULL;
long					CurAlarm;
MoverPtr				moverList[256];
bool					TacOrderOrigin = ORDER_ORIGIN_COMMANDER;
long					CurMultiplayCode = 0;
long					CurMultiplayParam = 0;
extern float			MaxVisualRadius;
extern float			WeaponRange[NUM_FIRERANGES];
extern GameLog*			BugLog;

UserHeapPtr				AblStackHeap = NULL;
UserHeapPtr				AblCodeHeap = NULL;
UserHeapPtr				AblSymbolHeap = NULL;

extern bool				GeneralAlarm;
extern bool 			friendlyDestroyed;
extern bool 			enemyDestroyed;
extern bool 			invulnerableON;		//Used for tutorials so mechs can take damage, but look like they are taking damage!  Otherwise, I'd just use NOPAIN!!

void DEBUGWINS_setGameObject (long debugObj, GameObjectPtr obj);
void DEBUGWINS_print (const char* s, long window);

//*****************************************************************************
// MISC AI
//*****************************************************************************

long getMoversWithinRadius (MoverPtr* moverList, Stuff::Vector3D center, float radius, long teamID, long commanderID, bool getEnemies, bool sortDescending, bool ignoreOrder) {

	static float sortValues[MAX_MOVERS];
	if (!Team::sortList) {
		Team::sortList = new SortList;
		if (!Team::sortList)
			Fatal(0, " Unable to create Team Contact sortList ");
		Team::sortList->init(MAX_CONTACTS_PER_SENSOR);
	}
	Team::sortList->clear(sortDescending);

	TeamPtr team = NULL;
	if (teamID > -1)
		team = Team::teams[teamID];

	long numValidMovers = 0;
	if (getEnemies) {
		for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
			MoverPtr mover = ObjectManager->getMover(i);
			if (mover->getExists() && !mover->isDisabled() && mover->isEnemy(team))
				if (mover->numFunctionalWeapons > 0)
					if (mover->distanceFrom(center) < radius) {
						if (ignoreOrder || (mover->getPilot()->getCurTacOrder()->code == TACTICAL_ORDER_NONE)) {
							Team::sortList->setId(numValidMovers, mover->getHandle());
							Team::sortList->setValue(numValidMovers, mover->getThreatRating());
							numValidMovers++;
						}
					}
		}
		}
	else {
		if (commanderID == -1)
			for (long i = 0; i < team->rosterSize; i++) {
				MoverPtr mover = team->getMover(i);
				if (mover->getExists() && !mover->isDisabled() && (mover->numFunctionalWeapons > 0))
					if (mover->distanceFrom(center) < radius) {
						if (mover->getPilot()->getWillHelp()) {
							if (ignoreOrder || (mover->getPilot()->getCurTacOrder()->code == TACTICAL_ORDER_NONE)) {
								Team::sortList->setId(numValidMovers, mover->getHandle());
								Team::sortList->setValue(numValidMovers, mover->getThreatRating());
								numValidMovers++;
							}
						}
					}
			}
		else
			for (long i = 0; i < team->rosterSize; i++) {
				MoverPtr mover = team->getMover(i);
				if ((commanderID == mover->commanderId) && mover->getExists() && !mover->isDisabled() && (mover->numFunctionalWeapons > 0))
					if (mover->distanceFrom(center) < radius) {
						if (mover->getPilot()->getWillHelp()) {
							if (ignoreOrder || (mover->getPilot()->getCurTacOrder()->code == TACTICAL_ORDER_NONE)) {
								Team::sortList->setId(numValidMovers, mover->getHandle());
								Team::sortList->setValue(numValidMovers, mover->getThreatRating());
								numValidMovers++;
							}
						}
					}
			}
	}

	if (numValidMovers > 0) {
		Team::sortList->sort(sortDescending);
		for (long i = 0; i < numValidMovers; i++)
			moverList[i] = (MoverPtr)ObjectManager->get(Team::sortList->getId(i));
	}
	return(numValidMovers);
}

//-----------------------------------------------------------------------------

void calcAttackPlan (long numAttackers, GameObjectPtr* attackers, long numDefenders, GameObjectPtr* defenders) {

	if ((numDefenders == 0) || (numAttackers == 0))
		return;

	//------------------------------------------------------------------
	// This assumes the attackers and defenders lists are already sorted
	// in descending order...
	float attackRatio[MAX_MOVERS];
	long attackTotal[MAX_MOVERS];
	GameObjectPtr target[MAX_MOVERS];
	for (long i = 0; i < numDefenders; i++) {
		attackTotal[i] = 0;
		attackRatio[i] = 0.0;
		target[i] = NULL;
	}

	//-------------------------------------------------------------------------
	// Calc who they should attack, trying to spread the wealth, so to speak...
	for (long a = 0; a < numAttackers; a++) {
		long toughest = numDefenders - 1;
		for (long d = (numDefenders - 2); d > -1; d--) {
			if (attackRatio[d] < attackRatio[toughest])
				toughest = d;
			else if (attackRatio[d] == attackRatio[toughest]) {
				if (attackTotal[d] > attackTotal[toughest])
					toughest = d;
			}
		}
		attackTotal[toughest] += attackers[a]->getThreatRating();
		attackRatio[toughest] = attackTotal[toughest] / defenders[toughest]->getThreatRating();
		target[a] = defenders[toughest];
	}

	//---------------------------------------------
	// For now, let's let 'em know their targets...
	for (int i = 0; i < numAttackers; i++)
		attackers[i]->getPilot()->triggerAlarm(PILOT_ALARM_ATTACK_ORDER, target[i]->getWatchID());
}

//-----------------------------------------------------------------------------

GameObjectPtr calcBestTarget (MoverPtr attacker, long numAttackers, MoverPtr* attackers, long numDefenders, MoverPtr* defenders) {

	//------------------------------------------------------------------
	// This assumes the attackers and defenders lists are already sorted
	// in descending order...

	if (numDefenders == 0)
		return(NULL);

	float attackRatio[MAX_MOVERS];
	long attackTotal[MAX_MOVERS];
	GameObjectPtr target[MAX_MOVERS];
    int i=0;
	for (; i < numDefenders; i++) {
		attackTotal[i] = 0;
		attackRatio[i] = 0.0;
		target[i] = NULL;
	}

	//---------------------------------------------------------------
	// First, find out who everyone else in our group is attacking...
	for (long a = 0; a < numAttackers; a++) {
		GameObjectPtr target = attackers[a]->getPilot()->getCurrentTarget();
		for (int d = 0; d < numDefenders; d++)
			if (defenders[d] == target) {
				attackTotal[i] += attackers[a]->getThreatRating();
				break;
			}
	}
	for (int d = 0; d < numDefenders; d++)
		attackRatio[d] = attackTotal[d] / defenders[d]->getThreatRating();

	//----------------------------------------------
	// Now, find out who this pilot should attack...
	long bestTarget = numDefenders - 1;
	if (numDefenders > 1)
		for (int d = (numDefenders - 2); d > -1; d--) {
			if (attackRatio[d] < attackRatio[bestTarget])
				bestTarget = d;
			else if (attackRatio[d] == attackRatio[bestTarget]) {
				if (attackTotal[d] > attackTotal[bestTarget])
					bestTarget = d;
			}
		}
	attackTotal[bestTarget] += attacker->getThreatRating();
	attackRatio[bestTarget] = attackTotal[bestTarget] / defenders[bestTarget]->getThreatRating();
	return(defenders[bestTarget]);
}

//*****************************************************************************

inline signed int double2long(double _in) {

	_in+=6755399441055744.0;
	return(*(signed int*)&_in);
}

//-----------------------------------------------------------------------------

inline bool isObjectId (long partId) {

	return(partId > MAX_UNIT_PART_ID);
}

//---------------------------------------------------------------------------

inline bool isUnitId (long partId) {

	return((partId >= MIN_UNIT_PART_ID) && (partId <= MAX_UNIT_PART_ID));
}

//---------------------------------------------------------------------------

inline bool isTeamId (long partId) {

	return((partId >= MIN_TEAM_PART_ID) && (partId <= MAX_TEAM_PART_ID));
}

//---------------------------------------------------------------------------

GameObjectPtr getObject (long partId, bool existsOnly = false);

inline GameObjectPtr getObject (long partId, bool existsOnly) {

	GameObjectPtr obj = NULL;
	if (partId == -1)
		obj = CurObject;
	else
		obj = ObjectManager->findByPartId(partId);
	if (existsOnly) {
		if (obj && obj->getExists())
			return(obj);
		return(NULL);
	}
	return(obj);
}

//---------------------------------------------------------------------------

inline MechWarriorPtr getWarrior (long warriorIndex) {

	if (warriorIndex == -1)
		return(CurWarrior);
	else
		return(MechWarrior::warriorList[warriorIndex]);
}

//---------------------------------------------------------------------------

long getMovers (long partId, MoverPtr* list, bool existsOnly = false);

inline long getMovers (long partId, MoverPtr* list, bool existsOnly) {

	//---------------------------------------------------------------------
	// NOTE: It should be safe to assume that all objects here are movers,
	// since only movers may be members of a unit/team.
	// ALSO NOTE: If a group is referenced, we better be in a single-player
	// game (and it better be a computer-commander, which won't re-arrange
	// its groups)!

	long numObjects = 0;
	if ((partId >= OBJ_ID_FIRST_COMMANDER) && (partId <= OBJ_ID_LAST_COMMANDER))
		Fatal(3, " ABL.getMovers: bad id ");
		//numObjects = Commander::commanders[partId - OBJ_ID_FIRST_COMMANDER]->getRoster((GameObjectPtr*)list);
	else if ((partId >= OBJ_ID_FIRST_TEAM) && (partId <= OBJ_ID_LAST_TEAM)) {
		TeamPtr team = Team::teams[partId - OBJ_ID_FIRST_TEAM];
		if (team)
			numObjects = Team::teams[partId - OBJ_ID_FIRST_TEAM]->getRoster((GameObjectPtr*)list, existsOnly);
		}
	else {
		for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
			MoverPtr mover = ObjectManager->getMover(i);
			if (mover && (mover->getSquadId() == partId))
				list[numObjects++] = mover;
		}
	}

	return(numObjects);
}

//*****************************************************************************

void execGetId (void) {

	//-----------------------------------------------------
	//
	//	GET ID function:
	//
	//		Returns object id of current object.
	//
	//		PARAMS:	none
	//
	//		RETURN: integer
	//
	//-----------------------------------------------------

	long partID = 0;
	if (CurObject)
		partID = CurObject->getPartId();
	ABLi_pushInteger(partID);
}

//***************************************************************************

void execGetTime (void) {

	//-----------------------------------------------------
	//
	//	GET TIME function:
	//
	//		Returns current mission(scenario) time.
	//
	//		PARAMS:	none
	//
	//		RETURN: real
	//
	//-----------------------------------------------------

	ABLi_pushReal(mission->actualTime);
}

//***************************************************************************

void execGetTimeLeft (void) {

	//-----------------------------------------------------
	//
	//	GET TIME function:
	//
	//		Returns mission(scenario) time left.
	//
	//		PARAMS:	none
	//
	//		RETURN: real
	//
	//-----------------------------------------------------

	if (mission->m_timeLimit > -1)
		if (mission->m_timeLimit - mission->actualTime > 0)
			ABLi_pushReal(mission->m_timeLimit - mission->actualTime);
		else
			ABLi_pushReal(0.0);
	else
		ABLi_pushReal(-1.0);
}

//*****************************************************************************

void execSelectObject (void) {

	//-----------------------------------------------------
	//
	//	SELECT OBJECT function:
	//
	//		Sets the specified object as the Current Object.
	//		It returns an error code if it fails, else it returns the
	//		id of the previous current object.
	//
	//				integer		object id
	//
	//		RETURN: integer		object id or error code (-1 = not an object)
	//
	//-----------------------------------------------------

	long objectID = ABLi_peekInteger();

	//---------------------------------------
	// Remember what the current object is...
	long curObjectID = 0;
	if (CurObject)
		curObjectID = CurObject->getPartId();

	GameObjectPtr newObject = getObject(objectID);
	if (newObject) {
		CurObject = newObject;
		ABLi_pokeInteger(curObjectID);
		}
	else
		ABLi_pokeInteger(-1);
}

//***************************************************************************

void execSelectWarrior (void) {

	//----------------------------------------------------------------------
	//
	//	SELECT WARRIOR function:
	//
	//		Sets the specified warrior as the Current Pilot. All pilot-assumed
	//		ABL functions reference the Current Pilot(Warrior).
	//		It returns an error code if it fails, else it returns the
	//		warrior id of the previous current pilot.
	//
	//				integer				warrior id
	//
	//		RETURN: integer				warrior id or error code (-1 = not a pilot)
	//
	//----------------------------------------------------------------------


	long warriorIndex = ABLi_popInteger();

	//---------------------------------------
	// Remember who the current warrior is...
	long curWarriorIndex = 0;
	if (CurWarrior)
		curWarriorIndex = CurWarrior->getIndex();

	CurWarrior = getWarrior(warriorIndex);
	ABLi_pushInteger(curWarriorIndex);
}

//*****************************************************************************

void execGetWarriorStatus (void) {

	//-----------------------------------------------------
	//
	//	GET WARRIOR STATUS function:
	//
	//		Returns the status of the warrior.
	//
	//		PARAMS:	integer				warrior id 
	//
	//		RETURN: integer				status
	//
	//-----------------------------------------------------

	long warriorId = ABLi_peekInteger();

	MechWarriorPtr warrior = getWarrior(warriorId);
	if (warrior)
		ABLi_pokeInteger(warrior->getStatus());
	else
		ABLi_pokeInteger(-1);
}

//*****************************************************************************

void execGetContacts (void) {

	//-----------------------------------------------------
	//
	//	GET CONTACTS function:
	//
	//		Returns a sorted list of all criteria'd contacts for the pilot.
	//
	//		PARAMS:	integer[]			contact list
	//
	//				integer				contact criteria
	//
	//				integer				sort criteria
	//
	//		RETURN: integer				result code
	//
	//-----------------------------------------------------

	int* contactList = ABLi_popIntegerPtr();
	int contactCriteria = ABLi_popInteger();
	int sortCriteria = ABLi_popInteger();
	
	long numContacts = CurObject->getContacts(contactList, contactCriteria, sortCriteria);
	ABLi_pushInteger(numContacts);

	for (long i = 0; i < numContacts; i++) {
		GameObjectPtr obj = ObjectManager->get(contactList[i]);
		Assert(obj != NULL, 0, " NULL contact");
		contactList[i] = obj->getPartId();
	}
}

//*****************************************************************************

void execGetEnemyCount (void) {

	//-----------------------------------------------------
	//
	//	GET ENEMY COUNT function:
	//
	//		Returns number of sighted enemies.
	//
	//		PARAMS:	integer				object id
	//
	//		RETURN: integer				result code
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();

	long result = -1;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		if ((objectId >= OBJ_ID_FIRST_TEAM) && (objectId <= OBJ_ID_LAST_TEAM)) {
			TeamPtr team = Team::teams[objectId - OBJ_ID_FIRST_TEAM];
			if (team)
				result = SensorManager->getTeamSensor(team->getId())->numContacts;
		}
		}
	else {
		GameObjectPtr object = getObject(objectId);
		if (object) {
			if (object->isMover()) {
				if (object->getTeam())
					result = SensorManager->getTeamSensor(object->getTeamId())->numContacts;
				}
			else
				switch (object->getObjectClass()) {
					case BUILDING:
					case ARTILLERY:
						if (object->getTeamId() > -1)
							result = SensorManager->getTeamSensor(object->getTeamId())->numContacts;
						break;
				}
		}
	}

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execSelectContact (void) {

	//-----------------------------------------------------
	//
	//	SELECT CONTACT function:
	//
	//		Sets the specified contact as the Current Contact. All pilot
	//		contact-related ABL functions reference the Current Contact.
	//		The Current Pilot's contact lists are referenced.
	//
	//		PARAMS:	integer				contact handle
	//
	//		RETURN: integer				error code (1 = not a contact)
	//
	//-----------------------------------------------------

	long type = ABLi_popInteger();
	long id = ABLi_popInteger();

	long code = -1;
	if (CurObject->isMover()) {
		MoverPtr me = (MoverPtr)CurObject;
		MoverPtr object = (MoverPtr)ObjectManager->findByPartId(id);
		//-------------------------------------------------------
		// Passed an object id, so select it if it's a contact...
		if (object && object->getTeam() && object->getContactStatus(me->getTeam()->getId(), true) != CONTACT_NONE) {
			CurContact = object;
			code = NO_ERR;
			}
		else
			code = 1;
	}

	ABLi_pushInteger(code);
}

//*****************************************************************************

void execIsContact (void) {

	long objectId = ABLi_popInteger();
	long criteria = ABLi_popInteger();
	long select = ABLi_popInteger();

	long result = -1;
	if (CurObject->isMover() && CurObject->getTeam()) {
		GameObjectPtr object = getObject(objectId);
		if (object->isMover() && (SensorManager->getTeamSensor(CurObject->getTeamId())->meetsCriteria(CurObject, (MoverPtr)object, criteria))) {
			result = objectId;
			if (select)
				CurContact = object;
			}
		else
			result = 0;
	}

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execGetContactId (void) {

	//-----------------------------------------------------
	//
	//	GET CONTACT ID function:
	//
	//		Retrieves the current contact's object id number.
	//
	//		PARAMS:	none
	//
	//		RETURN: integer				contact's object id
	//
	//-----------------------------------------------------

	long result = 0;
	if (CurContact)
		result = CurContact->getPartId();

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execGetContactStatus (void) {

	//-----------------------------------------------------
	//
	//	GET CONTACT STATUS function:
	//
	//		Retrieves the contact status of the current contact.
	//
	//		PARAMS:	@integer			is set to tag state, if any
	//
	//		RETURN: integer				contact's status
	//
	//-----------------------------------------------------

	int* contactTagged = ABLi_popIntegerPtr();

	*contactTagged = 0;

	long status = 0;
	if (CurContact && CurObject->isMover())
		status = ((MoverPtr)CurContact)->getContactStatus(CurObject->getTeamId(), true);
	ABLi_pushInteger(status);
}

//*****************************************************************************

void execGetContactRelativePosition (void) {

	//-----------------------------------------------------
	//
	//	GET CONTACT RELATIVE POSITION function:
	//
	//		Retrieves the range and angle to the current contact.
	//
	//		PARAMS:	@real				is set to the range (in meters) to contact
	//
	//				@real				is set to the angle (in degrees) to contact
	//
	//		RETURN: integer				error code
	//
	//-----------------------------------------------------

	float* range = ABLi_popRealPtr();
	float* angle = ABLi_popRealPtr();

	*range = -1.0;
	*angle = 0.0;

	long result = 1;
	if (CurContact && CurObject) {
		*range = CurObject->distanceFrom(CurContact->getPosition());
		*angle = CurObject->relFacingTo(CurContact->getPosition());
		result = NO_ERR;
	}

	ABLi_pushInteger(result);
}

//***************************************************************************

void execSetTarget (void) {

	long attackerId = ABLi_popInteger();
	long targetId = ABLi_popInteger();

	GameObjectPtr target = getObject(targetId);
	if ((attackerId >= MIN_UNIT_PART_ID) && (attackerId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		long numObjects = getMovers(attackerId, moverList);
		for (long i = 0; i < numObjects; i++) {
			MechWarriorPtr pilot = moverList[i]->getPilot();
			if (pilot) {
				pilot->setCurrentTarget(target);
				pilot->getVehicle()->calcFireRanges();
			}
		}
		}
	else {
		GameObjectPtr attacker = getObject(attackerId);
		if (attacker) {
			MechWarriorPtr pilot = ((MoverPtr)attacker)->getPilot();
			if (pilot) {
				pilot->setCurrentTarget(target);
				pilot->getVehicle()->calcFireRanges();
			}
		}
	}
}

//*****************************************************************************

void execGetTarget (void) {

	long objectId = ABLi_popInteger();

	long partID = 0;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		}
	else {
		GameObjectPtr object = getObject(objectId);
		if (object && object->isMover()) {
			MechWarriorPtr pilot = ((MoverPtr)object)->getPilot();
			Assert(pilot != NULL, 0, " execHbGetTarget:No pilot in mover! ");
			GameObjectPtr target = pilot->getCurrentTarget();
			if (target)
				partID = target->getPartId();
		}
	}
	ABLi_pushInteger(partID);
}

//*****************************************************************************

void execGetWeaponsReady (void) {

	int* weaponList = ABLi_popIntegerPtr();
	int listSize = ABLi_popInteger();

	long numWeapons = 0;
	if (CurObject->isMover())
		numWeapons = ((MoverPtr)CurObject)->getWeaponsReady(weaponList, listSize);

	ABLi_pushInteger(numWeapons);
}

//*****************************************************************************

void execGetWeaponsLocked (void) {

	int* weaponList = ABLi_popIntegerPtr();
	int listSize = ABLi_popInteger();

	long numWeapons = 0;
	if (CurObject->isMover())
		numWeapons = ((MoverPtr)CurObject)->getWeaponsLocked(weaponList, listSize);

	ABLi_pushInteger(numWeapons);
}

//*****************************************************************************

void execGetWeaponsInRange (void) {

	int* weaponList = ABLi_popIntegerPtr();
	int listSize = ABLi_popInteger();

	GameObjectPtr target = CurWarrior->getCurrentTarget();
	long numWeapons = 0;
	if (CurObject->isMover() && target)
		numWeapons = ((MoverPtr)CurObject)->getWeaponsInRange(weaponList, listSize, CurObject->distanceFrom(target->getPosition()));

	ABLi_pushInteger(numWeapons);
}

//*****************************************************************************

void execGetWeaponShots (void) {

	long weaponIndex = ABLi_popInteger();

	long numShots = 0;
	if (CurObject->isMover())
		numShots = ((MoverPtr)CurObject)->getWeaponShots(weaponIndex);

	ABLi_pushInteger(numShots);
}

//*****************************************************************************

void execGetWeaponRanges (void) {

	//-----------------------------------------------------
	//
	//	GET WEAPON RANGES function:
	//
	//		Returns the range in meters for the object's shortest,
	//		optimal and longest weapon ranges.
	//
	//		PARAMS:	integer				object id
	//
	//				real[3]				list to store weapon ranges
	//
	//		RETURN: none
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float* rangeList = ABLi_popRealPtr();

	GameObjectPtr object = getObject(objectId);
	if (object && object->isMover()) {
		rangeList[0] = ((MoverPtr)object)->getMinFireRange();
		rangeList[1] = ((MoverPtr)object)->getOptimalFireRange();
		rangeList[2] = ((MoverPtr)object)->getMaxFireRange();
		}
	else
		rangeList[0] = rangeList[1] = rangeList[2] = 0.0;
}

//*****************************************************************************

void execGetObjectPosition (void) {

	long objectId = ABLi_popInteger();
	float* coordList = ABLi_popRealPtr();

	coordList[0] = 0.0;
	coordList[1] = 0.0;
	coordList[2] = 0.0;
		
	GameObjectPtr obj = getObject(objectId);
	if (obj) {
		Stuff::Vector3D pos = obj->getPosition();
		coordList[0] = pos.x;
		coordList[1] = pos.y;
		coordList[2] = pos.z;
	}
}

//*****************************************************************************

void execGetIntegerMemory (void) {

	//-----------------------------------------------------
	//
	//	GET MEMORY INTEGER function:
	//
	//		Returns current integer value in memory cell of pilot.
	//
	//		PARAMS:	integer
	//
	//		RETURN: integer
	//
	//-----------------------------------------------------

	long memIndex = ABLi_peekInteger();
	ABLi_pokeInteger(CurWarrior->getIntegerMemory(memIndex));
}

//*****************************************************************************

void execGetRealMemory (void) {

	//-----------------------------------------------------
	//
	//	GET MEMORY REAL function:
	//
	//		Returns current real value in memory cell of pilot.
	//
	//		PARAMS:	integer
	//
	//		RETURN: real
	//
	//-----------------------------------------------------

	long memIndex = ABLi_peekInteger();
	ABLi_pokeReal(CurWarrior->getRealMemory(memIndex));
}

//*****************************************************************************

void execGetAlarmTriggers (void) {

	//-----------------------------------------------------
	//
	//	GET ALARM TRIGGERS function:
	//
	//		Returns number of triggers for the current combat event handler.
	//		It also fills the list with all of the trigger ids for this
	//		frame(call).
	//
	//		PARAMS:	integer[]
	//
	//		RETURN: integer
	//
	//-----------------------------------------------------

	int* triggerList = ABLi_popIntegerPtr();
	ABLi_pushInteger(CurWarrior->getEventHistory(CurAlarm, triggerList));
}

//*****************************************************************************

void execGetChallenger (void) {

	//-----------------------------------------------------
	//
	//	GET CHALLENGER function:
	//
	//		Returns the challenger for the specified object.
	//
	//		PARAMS:	integer				object id
	//
	//		RETURN: integer				challenger object id
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();

	long challengerPartID = 0;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		}
	else {
		GameObjectPtr object = getObject(objectId);
		if (object) {
			if (object->isMover()) {
				GameObjectPtr challenger = ((MoverPtr)object)->getChallenger();
				if (challenger)
					challengerPartID = challenger->getPartId();
			}
		}
	}

	ABLi_pushInteger(challengerPartID);
}

//*****************************************************************************

void execGetTimeWithoutOrders (void) {

	//-----------------------------------------------------
	//
	//	GET TIME WITHOUT ORDERS function:
	//
	//		Returns the current pilot's time (in secs) without orders.
	//
	//		PARAMS:	none
	//
	//		RETURN: real				time (in seconds)
	//
	//-----------------------------------------------------

	ABLi_pushReal(0.0);
	if (CurWarrior) {
		float lastTime = CurWarrior->getTimeOfLastOrders();
		if (lastTime >= 0.0)
			ABLi_pokeReal(scenarioTime - lastTime);
	}
}

//*****************************************************************************

void execGetFireRanges (void) {

	//-----------------------------------------------------
	//
	//	GET FIRE RANGES function:
	//
	//		Retrieves the current settings for SHORT, MEDIUM and LONG range,
	//		as well as MAX WEAPON range
	//
	//		PARAMS:	@real[4]			sets the game's current ranges (meters)				
	//
	//		RETURN: none
	//
	//-----------------------------------------------------

	float* ranges = ABLi_popRealPtr();

	ranges[0] = WeaponRange[FIRERANGE_SHORT];
	ranges[1] = WeaponRange[FIRERANGE_MEDIUM];
	ranges[2] = WeaponRange[FIRERANGE_LONG];
	ranges[3] = 250.0; //No one should use this in MC2!
}

//*****************************************************************************

void execGetAttackers (void) {

	//-----------------------------------------------------
	//
	//	GET ATTACKERS function:
	//
	//		Retrieves all objects that have attacked current pilot
	//		in the last x seconds.
	//
	//		PARAMS:	@integer[]			attacker list (make it big!)
	//
	//				real				seconds
	//
	//		RETURN: integer				num attackers
	//
	//-----------------------------------------------------


	int* attackers = ABLi_popIntegerPtr();
	float seconds = ABLi_popReal();

	long numAttackers = 0;
	if (CurWarrior)
		numAttackers = CurWarrior->getAttackers((unsigned int*)attackers, seconds);
	ABLi_pushInteger(numAttackers);
}

//*****************************************************************************

void execGetAttackerInfo (void) {

	//-----------------------------------------------------
	//
	//	GET ATTACKER INFO function:
	//
	//		If the object has attacked the current pilot, it returns the #
	//		of seconds since the last attack (FUTURE: may return # of hits,
	//		damage done, etc. thru params). If the object never attacked
	//		'em, returns -1.0.
	//
	//		PARAMS:	integer				attacker id
	//
	//		RETURN: real				seconds since last attack
	//
	//-----------------------------------------------------

	unsigned long attackerId = ABLi_popInteger();

	float timeSince = 1000000.0;
	if ((attackerId >= MIN_UNIT_PART_ID) && (attackerId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		}
	else {
		if (CurWarrior) {
			AttackerRecPtr attackerRec = CurWarrior->getAttackerInfo(attackerId);
			if (attackerRec)
				timeSince = scenarioTime - attackerRec->lastTime;
		}
	}
	ABLi_pushReal(timeSince);
}

//*****************************************************************************

void execSetChallenger (void) {

	//-----------------------------------------------------
	//
	//	SET CHALLENGER function:
	//
	//		Sets the challenger for the specified object.
	//
	//		PARAMS:	integer				object1
	//
	//				integer				object2 (challenger)
	//
	//		RETURN: integer				error code
	//
	//-----------------------------------------------------

	long victimId = ABLi_popInteger();
	long challengerId = ABLi_popInteger();

	long result = NO_ERR;
	
	if ((challengerId >= MIN_UNIT_PART_ID) && (challengerId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		result = -1;
		}
	else {
		GameObjectPtr challenger = getObject(challengerId);
		GameObjectPtr victim = getObject(victimId);
		if (victim && victim->isMover())
			((MoverPtr)victim)->setChallenger(challenger);
		else
			result = -2;
	}
	ABLi_pushInteger(result);
}

//*****************************************************************************

void execSetIntegerMemory (void) {

	long memIndex = ABLi_popInteger();
	long memValue = ABLi_popInteger();
	CurWarrior->setIntegerMemory(memIndex, memValue);
}

//***************************************************************************

void execSetRealMemory (void) {

	long memIndex = ABLi_popInteger();
	float memValue = ABLi_popReal();
	CurWarrior->setRealMemory(memIndex, memValue);
}

//*****************************************************************************

void execHasMoveGoal (void) {

	bool result = false;
	if (CurWarrior)
		result = (CurWarrior->getMoveNewGoal() && CurWarrior->hasMoveGoal());
	ABLi_pushBoolean(result);
}

//*****************************************************************************

void execHasMovePath (void) {

	bool result = false;
	if (CurWarrior)
		result = (CurWarrior->getMovePath() && !CurWarrior->getMoveNewGoal());
	ABLi_pushBoolean(result);
}

//*****************************************************************************

void execSortWeapons (void) {


	int* weaponList = ABLi_popIntegerPtr();
	long listSize = ABLi_popInteger();
	long sortType = ABLi_popInteger();

	long valueList[50];
	if (CurObject && CurObject->isMover())
		((MoverPtr)CurObject)->sortWeapons(weaponList, valueList, listSize, sortType, true);
}

//*****************************************************************************

void execGetVisualRange (void) {

	//-----------------------------------------------------
	//
	//	GET VISUAL RANGE function:
	//
	//		Returns visual range of the indicated object (taking into
	//		account the infamous Beagle Probe).
	//
	//		PARAMS:	integer				object id
	//
	//		RETURN: real				range in meters
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();

	float range = 0.0;
	GameObjectPtr object = NULL;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		range = -1;
		}
	else {
		object = getObject(objectId);
		if (object && object->isMover())
			range = ((MoverPtr)object)->getVisualRange();
	}
	ABLi_pushReal(range);
}

//*****************************************************************************

void execGetUnitMates (void) {

	//-----------------------------------------------------
	//
	//	GET UNIT MATES function:
	//
	//		Returns number and list of unit mates of the specified object.
	//
	//		PARAMS:	integer			object (unit) id
	//
	//				integer[]		object list
	//
	//		RETURN: integer			num objects in unit
	//
	//-----------------------------------------------------


	long objectId = ABLi_popInteger();
	int* mateList = ABLi_popIntegerPtr();

	long numObjs = 0;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		long numObjects = getMovers(objectId, moverList);
		for (long i = 0; i < numObjects; i++)
			mateList[i] = moverList[i]->getPartId();
		numObjs = numObjects;
		}
	else {
		GameObjectPtr object = getObject(objectId);
		if (object && object->isMover()) {
			MoverGroupPtr group = ((MoverPtr)object)->getGroup();
			if (group) {
				long numObjects = group->getMovers(moverList);
				for (long i = 0; i < numObjects; i++)
					mateList[i] = moverList[i]->getPartId();
				numObjs = numObjects;
			}
		}
	}
	ABLi_pushInteger(numObjs);		
}

//*****************************************************************************

void execGetTacOrder(void) {

	//-----------------------------------------------------
	//
	//	GET TACORDER:
	//
	//		Retrieves the current tactical order for the specified object.
	//		Also sets the order time and order parameters, which are passed
	//		as call-by-ref parameters.
	//
	//		PARAMS:	integer				object id
	//
	//				@real				time order was given
	//
	//				integer[]			order parameters (size of list varies by order)
	//
	//		RETURN: integer				tac order code
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float* time = ABLi_popRealPtr();
	int* paramList = ABLi_popIntegerPtr();
	
	long code = 0;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		}
	else {
		GameObjectPtr object = getObject(objectId);
		if (object && object->isMover()) {
			MechWarriorPtr pilot = ((MoverPtr)object)->getPilot();
			if (pilot) {
				code = pilot->getCurTacOrder()->getParamData(time, paramList);
				GameObjectPtr goalObj = NULL;
				Stuff::Vector3D goalLoc;
				float goalRange = 0.0;
				paramList[17] = pilot->getMainGoal(goalObj, goalLoc, goalRange);
				paramList[18] = goalObj ? goalObj->getPartId() : 0;
			}
		}
	}

	ABLi_pushInteger(code);
}	

//*****************************************************************************

void execGetLastTacOrder(void) {

	//-----------------------------------------------------
	//
	//	GET LAST TACORDER:
	//
	//		Retrieves the last tactical order for the specified object.
	//		Also sets the order time and order parameters, which are passed
	//		as call-by-ref parameters.
	//
	//		PARAMS:	integer				object id
	//
	//				@real				time order was given
	//
	//				integer[]			order parameters (size of list varies by order)
	//
	//		RETURN: integer				tac order code
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float* time = ABLi_popRealPtr();
	int* paramList = ABLi_popIntegerPtr();
	
	long code = 0;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		}
	else {
		GameObjectPtr object = getObject(objectId);
		if (object && object->isMover()) {
			MechWarriorPtr pilot = ((MoverPtr)object)->getPilot();
			if (pilot)
				code = pilot->getLastTacOrder()->getParamData(time, paramList);
		}
	}

	ABLi_pushInteger(code);
}	

//*****************************************************************************

void execGetObjects (void) {

	//-----------------------------------------------------
	//
	//	GET OBJECTS function:
	//
	//		Returns number and list of unit mates of the specified object.
	//
	//		PARAMS:	integer			object criteria
	//
	//				integer[]		object list
	//
	//		RETURN: integer			num objects in unit
	//
	//-----------------------------------------------------

	long criteria = ABLi_popInteger();
	int* objList = ABLi_popIntegerPtr();

	long numObjects = 0;
	switch (criteria) {
		case 0: {
			// MOVERS
			long listSize = ObjectManager->getNumMovers();
			for (long i = 0; i < listSize; i++) {
				MoverPtr mover = ObjectManager->getMover(i);
				if (mover && mover->getExists())
					objList[numObjects++] = mover->getPartId();
			}
			}
			break;
		case 1: {
			// TERRAIN OBJECTS
			long listSize = ObjectManager->getNumTerrainObjects();
			for (long i = 0; i < listSize; i++) {
				TerrainObjectPtr terObj = ObjectManager->getTerrainObject(i);
				if (terObj) {
					//long row, col;
					//((GameObjectPtr)terObj)->getCellPosition(row, col);
					//long partID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//Assert(partID == terObj->getPartId(), partID, "ugh");
					objList[numObjects++] = terObj->getPartId();
				}
			}
			}
			break;
		case 2: {
			// BUILDINGS
			long listSize = ObjectManager->getNumBuildings();
			for (long i = 0; i < listSize; i++) {
				BuildingPtr building = ObjectManager->getBuilding(i);
				if (building) {
					//long row, col;
					//((GameObjectPtr)building)->getCellPosition(row, col);
					//long calcPartID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//long partID = building->getPartId();
					//Assert(partID == calcPartID, partID, "ugh");
					objList[numObjects++] = building->getPartId();
				}
			}
			}
			break;
		case 3: {
			// TURRETS
			long listSize = ObjectManager->getNumTurrets();
			for (long i = 0; i < listSize; i++) {
				TurretPtr turret = ObjectManager->getTurret(i);
				if (turret) {
					//long row, col;
					//((GameObjectPtr)turret)->getCellPosition(row, col);
					//long partID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//Assert(partID == turret->getPartId(), partID, "ugh");
					objList[numObjects++] = turret->getPartId();
				}
			}
			}
			break;
		case 4: {
			// GATES
			long listSize = ObjectManager->getNumGates();
			for (long i = 0; i < listSize; i++) {
				GatePtr gate = ObjectManager->getGate(i);
				if (gate) {
					//long row, col;
					//((GameObjectPtr)gate)->getCellPosition(row, col);
					//long partID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//Assert(partID == gate->getPartId(), partID, "ugh");
					objList[numObjects++] = gate->getPartId();
				}
			}
			}
			break;
		case 5: {
			// ALL TERRAIN OBJECTS, including buildings, turrets and gates
			long listSize = ObjectManager->getNumTerrainObjects();
			for (long i = 0; i < listSize; i++) {
				TerrainObjectPtr terObj = ObjectManager->getTerrainObject(i);
				if (terObj) {
					//long row, col;
					//((GameObjectPtr)terObj)->getCellPosition(row, col);
					//long partID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//Assert(partID == terObj->getPartId(), partID, "ugh");
					objList[numObjects++] = terObj->getPartId();
				}
			}
			listSize = ObjectManager->getNumBuildings();
			for (int i = 0; i < listSize; i++) {
				BuildingPtr building = ObjectManager->getBuilding(i);
				if (building) {
					//long row, col;
					//((GameObjectPtr)building)->getCellPosition(row, col);
					//long calcPartID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//long partID = building->getPartId();
					//Assert(partID == calcPartID, partID, "ugh");
					objList[numObjects++] = building->getPartId();
				}
			}
			listSize = ObjectManager->getNumTurrets();
			for (int i = 0; i < listSize; i++) {
				TurretPtr turret = ObjectManager->getTurret(i);
				if (turret) {
					//long row, col;
					//((GameObjectPtr)turret)->getCellPosition(row, col);
					//long partID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//Assert(partID == turret->getPartId(), partID, "ugh");
					objList[numObjects++] = turret->getPartId();
				}
			}
			listSize = ObjectManager->getNumGates();
			for (int i = 0; i < listSize; i++) {
				GatePtr gate = ObjectManager->getGate(i);
				if (gate) {
					//long row, col;
					//((GameObjectPtr)gate)->getCellPosition(row, col);
					//long partID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
					//Assert(partID == gate->getPartId(), partID, "ugh");
					objList[numObjects++] = gate->getPartId();
				}
			}
			}
			break;
	}
	
	ABLi_pushInteger(numObjects);
}

//*****************************************************************************

void execOrderWait (void) {

	float seconds = ABLi_popReal();
	bool clearLastTarget = ABLi_peekBoolean();

	if (ABLi_getSkipOrder()) {
		ABLi_pokeInteger(1);
		return;
	}

	long result = TACORDER_FAILURE;
	result = CurWarrior->orderWait(false, ORDER_ORIGIN_COMMANDER, double2long(seconds), clearLastTarget);
	ABLi_pokeInteger(result);
}

//*****************************************************************************

void execOrderMoveTo (void) {

	//-----------------------------------------------------
	//
	//	MOVE TO POINT tactical order:
	//
	//		Set tactical order to move to specified location, using the
	//		current attitude orders and other move factors. Returns tacorder
	//		result code.
	//
	//		PARAMS:	real[3]					location
	//
	//				boolean					run
	//
	//		RETURN: integer
	//
	//-----------------------------------------------------


	float* coordList = ABLi_popRealPtr();
	bool run = ABLi_popBoolean();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long result = TACORDER_FAILURE;
	Stuff::Vector3D location;
	location.x = coordList[0];
	location.y = coordList[1];
	location.z = coordList[2];
	
	unsigned long params = TACORDER_PARAM_NONE;
	if (run)
		params |= TACORDER_PARAM_RUN;

	result = CurWarrior->orderMoveToPoint(false, true, ORDER_ORIGIN_COMMANDER, location, -1, params);

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execOrderMoveToObject (void) {

	//-----------------------------------------------------
	//
	//	MOVE TO OBJECT tactical order:
	//
	//		Set tactical order to move to specified object, using the given
	//		movement parameters (stealth, etc.). Returns error code.
	//
	//		PARAMS:	integer		contact id
	//
	//				boolean		run
	//
	//		RETURN: integer		error code
	//
	//-----------------------------------------------------

	unsigned long objectId = ABLi_popInteger();
	bool run = ABLi_popBoolean();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long result = 1;
	GameObjectPtr object = NULL;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		result = 1;
		}
	else {
		object = getObject(objectId);
		if (object)
			result = CurWarrior->orderMoveToObject(false, true, ORDER_ORIGIN_COMMANDER, object, -1, -1, run);
	}

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execOrderMoveToContact (void) {

	//-----------------------------------------------------
	//
	//	MOVE TO CONTACT tactical order:
	//
	//		Set tactical order to move to specified contact, using the given
	//		movement parameters (stealth, etc.). Returns error code.
	//
	//		PARAMS:	boolean		run
	//
	//		RETURN: integer		error code
	//
	//-----------------------------------------------------


	bool run = ABLi_popBoolean();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long result = TACORDER_FAILURE;
	//---------------------------------------------------------------
	// First, get the contact's object ID and its current location...
	if (CurContact) {
		//------------------------------------------------------------
		// Now, set up the tactical order, and pass it to the pilot...
		result = CurWarrior->orderMoveToObject(false, true, ORDER_ORIGIN_COMMANDER, CurContact, -1, -1, run);
	}

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execOrderPowerDown (void) {

	//-----------------------------------------------------
	//
	//	ORDER POWER DOWN function:
	//
	//		Set tactical order to power down.
	//		Returns tacorder result	code.
	//
	//		PARAMS:	NONE
	//
	//		RETURN: integer
	//
	//-----------------------------------------------------

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	ABLi_pushInteger(CurWarrior->orderPowerDown(false, TacOrderOrigin));
}

//*****************************************************************************

void execOrderPowerUp (void) {

	//-----------------------------------------------------
	//
	//	ORDER POWER UP function:
	//
	//		Set tactical order to power up.
	//		Returns tacorder result	code.
	//
	//		PARAMS:	NONE
	//
	//		RETURN: integer
	//
	//-----------------------------------------------------

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	ABLi_pushInteger(CurWarrior->orderPowerUp(false, TacOrderOrigin));
}

//*****************************************************************************

void execOrderAttackObject (void) {

	//-----------------------------------------------------
	//
	//	ORDER ATTACK OBJECT tactical order:
	//
	//		Set tactical order to direct all weapons fire at the specified
	//		object. Returns an error code.
	//
	//		PARAMS:	integer		object id
	//
	//				integer		attack type (0 = none, 1 = destroy, 2 = disable)
	//
	//				integer		method (0 = ranged, 1 = dfa, 2 = ramming)
	//
	//				integer		range (0 = short, 1 = medium, 2 = long)
	//
	//				integer		pursue (0 = False, 1 = True)
	//
	//		RETURN: integer		error code
	//
	//-----------------------------------------------------

	unsigned long objectId = ABLi_popInteger();
	long attackType = ABLi_popInteger();
	long attackMethod = ABLi_popInteger();
	long attackRange = ABLi_popInteger();
	long pursue = ABLi_popInteger();

	long result = 1;
	if (!ABLi_getSkipOrder()) {
		unsigned long params = TACORDER_PARAM_NONE;
		if (pursue)
			params |= TACORDER_PARAM_PURSUE;

		GameObjectPtr object = NULL;
		if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
			//--------------------------------------------
			// We have a group.  Act accordingly.
			// Run through all objects fitting the group.
			}
		else {
			if (objectId > 0)
				object = ObjectManager->findByPartId(objectId);
			result = CurWarrior->orderAttackObject(false, ORDER_ORIGIN_COMMANDER, object, attackType, attackMethod, attackRange, -1, -1, params);
		}
	}

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execOrderAttackContact (void) {

	//-----------------------------------------------------
	//
	//	ORDER ATTACK CONTACT tactical order:
	//
	//		Set tactical order to direct all weapons fire at the current
	//		contact. Returns an error code.
	//
	//		PARAMS:	integer		attack type (0 = none, 1 = destroy, 2 = disable)
	//
	//				integer		range (0 = short, 1 = medium, 2 = long)
	//
	//				integer		method (0 = ranged, 1 = dfa, 2 = ramming)
	//
	//				integer		pursue (0 = False, 1 = True)
	//
	//		RETURN: integer		error code
	//
	//-----------------------------------------------------

	long attackType = ABLi_popInteger();
	long attackMethod = ABLi_popInteger();
	long attackRange = ABLi_popInteger();
	long pursue = ABLi_popInteger();

	long result = 1;
	if (!ABLi_getSkipOrder()) {
		unsigned long params = TACORDER_PARAM_NONE;
		if (pursue)
			params |= TACORDER_PARAM_PURSUE;
		result = -2;
		if (CurContact)
			result = CurWarrior->orderAttackObject(false, ORDER_ORIGIN_COMMANDER, CurContact, attackType, attackMethod, attackRange, -1, -1, params);
	}

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execOrderWithdraw(void) {

	//Makes the object(s) passed in withdraw 
	//
	//		PARAMS:	NONE
	//
	//		Returns: integer (result)

	ABLi_pushInteger(1);
	if (!ABLi_getSkipOrder()) {
		Stuff::Vector3D location;
		location.Zero();
		ABLi_pushInteger(0);

		if (CurWarrior)
			CurWarrior->orderWithdraw(false, ORDER_ORIGIN_COMMANDER, location);
		else
			ABLi_pokeInteger(-2);
	}
}	

//*****************************************************************************

void execObjectInWithdrawal(void) {

	//Checks the object(s) passed in to see if they are withdrawing
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectId1 = ABLi_popInteger();

	long result = 1;
	GameObjectPtr object1 = NULL;
	if ((objectId1 >= MIN_UNIT_PART_ID) && (objectId1 <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		long numObjects = getMovers(objectId1, moverList);
		for (long i = 0; i < numObjects; i++) {
			if (!moverList[i]->isWithdrawing()) {
				result = 0;
				break;
			}
		}
		}
	else {
		object1 = getObject(objectId1);
		if (object1 && !object1->isWithdrawing())
			result = 0;			
	}
	
	ABLi_pushInteger(result);
}	

//*****************************************************************************

void execDamageObject (void) {

	//----------------------------------------------------------------------
	//
	//	DAMAGE OBJECT function:
	//
	//		Directly damages an object as if it were a weapon shot.
	//		Returns error code.
	//
	//		PARAMS:	integer				target object id
	//
	//				integer				attacker object id
	//
	//				integer				weapon master component id
	//
	//				real				damage points
	//
	//				integer				hit location
	//
	//				real				hit roll
	//
	//				real				entry angle
	//
	//		RETURN: integer				error code (always returns 0, for now)
	//
	//									-1 = bad target
	//
	//									-2 = bad attacker
	//
	//----------------------------------------------------------------------

	long targetId = ABLi_popInteger();
	long attackerId = ABLi_popInteger();
	long weaponMasterId = ABLi_popInteger();
	float damage = ABLi_popReal();
	long hitLocation = ABLi_popInteger();
	float hitRoll = ABLi_popReal();
	float entryAngle = ABLi_peekReal();

	GameObjectPtr attacker = getObject(attackerId);

	ABLi_pokeInteger(-1);
	if (attacker) {
		if ((targetId >= MIN_UNIT_PART_ID) && (targetId <= MAX_UNIT_PART_ID)) {
			//--------------------------------------------
			// We have a group.  Act accordingly.
			long numObjects = getMovers(targetId, moverList, true);
			WeaponShotInfo shotInfo;
			shotInfo.init(attacker->getWatchID(), weaponMasterId, damage, hitLocation, entryAngle);
			if (MPlayer) {
				if (MPlayer->isServer()) {
					for (long i = 0; i < numObjects; i++) {
						moverList[i]->handleWeaponHit(&shotInfo, true);
					}
				}
				}
			else
				for (long i = 0; i < numObjects; i++)
					moverList[i]->handleWeaponHit(&shotInfo);
			ABLi_pokeInteger(numObjects);
			}
		else {
			GameObjectPtr target = getObject(targetId, true);
			if (target) {
				WeaponShotInfo shotInfo;
				shotInfo.init(attacker->getWatchID(), weaponMasterId, damage, hitLocation, entryAngle);
				if (MPlayer) {
					if (MPlayer->isServer()) {
						target->handleWeaponHit(&shotInfo, true);
					}
					}
				else
					target->handleWeaponHit(&shotInfo);
				ABLi_pokeInteger(1);
				}
			else
				ABLi_pokeInteger(-2);
		}
	}
}

//*****************************************************************************

void execSetAttackRadius (void) {

	//-----------------------------------------------------
	//
	//	SET ATTACK RADIUS tactical order:
	//
	//		Sets the range at which the pilot will break off attack from its
	//		last target. Returns previous attack radius.
	//
	//		PARAMS:	real				attack radius in meters
	//
	//		RETURN: real				previous attack radius
	//
	//-----------------------------------------------------

	float radius = ABLi_peekReal();
	ABLi_pokeReal(CurWarrior->getAttackRadius());
	CurWarrior->setAttackRadius(radius);
}

//*****************************************************************************

void execObjectChangeSides (void) {

	//Object Change Side Function.
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: nothing!
	
	long objectId = ABLi_popInteger();
	long newObjectSide = ABLi_popInteger();

	GameObjectPtr object = NULL;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		Fatal(0, " Cannot ABL:ObjectChangeSides for Mover Units ");

		}
	else {
		object = getObject(objectId);
		if (object)
			object->setTeam(Team::teams[newObjectSide]);
	}
}

//*****************************************************************************

void execDistanceToObject (void) {

	//Distance to Object From Object Function.
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: (real) distance in meters
	
	long objectId1 = ABLi_popInteger();
	long objectId2 = ABLi_peekInteger();

	ABLi_pokeReal(-1.0);

	GameObjectPtr object2 = getObject(objectId2);
	if (object2) {
		Stuff::Vector3D position2 = object2->getPosition();
		if ((objectId1 >= MIN_UNIT_PART_ID) && (objectId1 <= MAX_UNIT_PART_ID)) {
			//--------------------------------------------
			// We have a group.  Act accordingly.
			// Run through all objects fitting the group.
			float minDistance = 3.4E38f;
			float distance = 3.4E38f;
			long numObjects = getMovers(objectId1, moverList);
			for (long i = 0; i < numObjects; i++) {
				MoverPtr mover = moverList[i];
				if (!mover->getExistsAndAwake())
					continue;
				Stuff::Vector3D position1 = mover->getPosition();
				position2.z = position1.z;
		
				Stuff::Vector3D resultVector;
				resultVector.Subtract(position2, position1);
		
				distance = resultVector.GetLength();
				if (distance < minDistance)
					minDistance = distance;
			}
			if (minDistance < 3.4E38)
				ABLi_pokeReal(minDistance * metersPerWorldUnit);
			}
		else {
			GameObjectPtr object1 = getObject(objectId1);
			if (object1) {
				Stuff::Vector3D position1 = object1->getPosition();
				position2.z = position1.z;
			
				Stuff::Vector3D resultVector;
				resultVector.Subtract(position2, position1);
			
				ABLi_pokeReal(resultVector.GetLength() * metersPerWorldUnit);
			}
		}
	}
}
			
//***************************************************************************

void execDistanceToPosition (void) {

	//Distance to Object From Position Function.
	//
	//		PARAMS:	integer, real[3]
	//
	//		Returns: (real) distance in meters
	
	long objectId = ABLi_popInteger();
	float* coordList = ABLi_popRealPtr();

	ABLi_pushReal(-1.0);

	//---------------------------------------
	// For now, we only care about x and y...
	Stuff::Vector3D position2;
	position2.Zero();

	if (!isnan(coordList[0]))
		position2.x = coordList[0];

	if (!isnan(coordList[1]))
		position2.y = coordList[1];
	
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		long numObjects = getMovers(objectId, moverList);
		float minDistance = 3.4E38f;
		float distance = 3.4E38f;
		for (long i = 0; i < numObjects; i++) {
			MoverPtr mover = moverList[i];
			if (!mover->getExistsAndAwake())
				continue;
			Stuff::Vector3D position1 = mover->getPosition();
			position2.z = position1.z;
		
			Stuff::Vector3D resultVector;
			resultVector.Subtract(position2, position1);
		
			distance = resultVector.GetLength();
			if (distance < minDistance)
				minDistance = distance;
		}
		if (minDistance < 3.4E38)
			ABLi_pokeReal(minDistance * metersPerWorldUnit);
		}
	else {
		GameObjectPtr obj = getObject(objectId);
		if (obj) {
			Stuff::Vector3D position1 = obj->getPosition();
			position2.z = position1.z;
			
			Stuff::Vector3D resultVector;
			resultVector.Subtract(position2, position1);
			ABLi_pokeReal(resultVector.GetLength() * metersPerWorldUnit);
		}
	}
}
			
//*****************************************************************************

void execObjectSuicide (void) {

	//Cause object to destroy itself Function.
	//
	//		PARAMS:	integer
	//
	//		Returns: nothing
	
	//-----------------
	long objectId = ABLi_popInteger();
	
	GameObjectPtr object1 = NULL;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		long numObjects = getMovers(objectId, moverList);
		for (long i = 0; i < numObjects; i++)
			moverList[i]->setExists(false);
		}
	else {
		object1 = getObject(objectId);
		if (object1)
			object1->setExists(false);
	}
}
			
//***************************************************************************

void execObjectCreate (void) {

	//Cause object to create itself Function.
	//
	//		PARAMS:	integer
	//
	//		Returns: id of object created 
	
	long objectId = ABLi_popInteger();
	
	long result = 0;

	GameObjectPtr object = getObject(objectId);
	if (object && !object->getExists()) {
		object->setExists(true);
		for (long i = 0; i < Team::numTeams; i++)
			SensorManager->getTeamSensor(i)->scanBattlefield();
		result = objectId;
	}
	ABLi_pushInteger(result);
}
			
//***************************************************************************

void execObjectExists (void) {

	//test if object exists Function.
	//
	//		PARAMS:	integer
	//
	//		Returns: integer
	
	long objectId = ABLi_peekInteger();
	ABLi_pokeInteger(0);

	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		long numObjs = getMovers(objectId, moverList);
		long i = 0;
		while ((i < numObjs) && !moverList[i]->getExists())
			i++;
		ABLi_pokeInteger(i < numObjs);
		}
	else {
		GameObjectPtr obj = getObject(objectId);
		if (obj && obj->getExists())
			ABLi_pokeInteger(1);
	}
}
			
//***************************************************************************

void execObjectStatus (void) {

	//returns object's status.
	//
	//		PARAMS:	integer
	//
	//		Returns: integer
	
	long objectId = ABLi_popInteger();
	
	long result = -1;
	
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//-------------------------------------------------------
		// We have a group.  Act accordingly.
		// No Group Check of actual status, return -1.
		// If any unit member is still alive, then the unit is...
		bool disabled = true;
		long numObjects = getMovers(objectId, moverList);
		for (long i = 0; i < numObjects; i++) {
			long status = moverList[i]->getStatus();
			if ((status != OBJECT_STATUS_DESTROYED) && (status != OBJECT_STATUS_DISABLED)) {
				MechWarriorPtr pilot = moverList[i]->getPilot();
				if (pilot && (pilot->getStatus() != WARRIOR_STATUS_WITHDRAWN)) {
					disabled = false;
					break;
				}
			}
		}
		if (disabled)
			result = OBJECT_STATUS_DISABLED;
		else
			result = OBJECT_STATUS_NORMAL;
		}
	else {
		GameObjectPtr obj = getObject(objectId);
		if (obj)
			result = obj->getStatus();
	}
	ABLi_pushInteger(result);
}
			
//*****************************************************************************

void execObjectStatusCount (void) {

	//return object status count for an object, unit or team.
	//
	//		PARAMS:	integer							partId
	//
	//				integer[9]						tally list
	//
	//		Returns: NONE

	long objectId = ABLi_popInteger();
	int* tallyList = ABLi_popIntegerPtr();
	
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// No Group Check of actual status, return -1.
		if ((objectId >= PlayerLance0) && (objectId < PlayerLance0 + MAX_MOVERGROUPS))
			//Fatal(0, " ABL.objectStatusCount: bad id ");
			Commander::commanders[0]->getGroup(objectId - PlayerLance0)->statusCount(tallyList);
		else if ((objectId >= AlliedLance0) && (objectId < AlliedLance0 + MAX_MOVERGROUPS))
			//Fatal(0, " ABL.objectStatusCount: bad id ");
			Commander::commanders[2]->getGroup(objectId - AlliedLance0)->statusCount(tallyList);
		else if ((objectId >= ClanStar0) && (objectId < ClanStar0 + MAX_MOVERGROUPS))
			//Fatal(0, " ABL.objectStatusCount: bad id ");
			Commander::commanders[1]->getGroup(objectId - ClanStar0)->statusCount(tallyList);
		else if ((objectId >= OBJ_ID_FIRST_TEAM) && (objectId <= OBJ_ID_LAST_TEAM)) {
			TeamPtr team = Team::teams[objectId - OBJ_ID_FIRST_TEAM];
			if (team)
				team->statusCount(tallyList);
		}
		}
	else {
		GameObjectPtr object1 = getObject(objectId);
		if (object1)
			tallyList[object1->getStatus()]++;
	}
}
			
//*****************************************************************************

void execObjectVisible (void) {

	//test if object2 is visible from object1
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: integer
	
	long objectId1 = ABLi_popInteger();
	long objectId2 = ABLi_popInteger();
	
	GameObjectPtr object1 = NULL;
	GameObjectPtr object2 = getObject(objectId2);
	long result = 0;
	
	if (object2) {
		if ((objectId1 >= MIN_UNIT_PART_ID) && (objectId1 <= MAX_UNIT_PART_ID)) {
			//--------------------------------------------
			// We have a group.  Act accordingly.
			// Run through all objects fitting the group.
			// Can anyone in the group see the object?
			long numObjects = getMovers(objectId1, moverList);
			for (long i = 0; i < numObjects; i++)
				if (object1->lineOfSight(moverList[i])) {
					result = 1;
					break;
				}
			}
		else {
			object1 = getObject(objectId1);
			if (object1)
				result = object1->lineOfSight(object2);
		}
	}

	ABLi_pushInteger(result);
}				
			
//*****************************************************************************

void execObjectTeam (void) {

	//return what side object plays for Function.
	//
	//		PARAMS:	integer
	//
	//		Returns: integer
	
	long objectId = ABLi_popInteger();

	long result = -1;
	GameObjectPtr object = getObject(objectId);
	if (object)
		result = MIN_TEAM_PART_ID + object->getTeamId();
	
	ABLi_pushInteger(result);
}

//*****************************************************************************

void execObjectCommander (void) {

	long objectId = ABLi_peekInteger();
	ABLi_pokeInteger(-1);
	
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//-----------------------------------
		// We have a group.  Act accordingly.
		}
	else {
		GameObjectPtr obj = getObject(objectId);
		if (obj)
			ABLi_pokeInteger(obj->getCommanderId());
	}
}

//*****************************************************************************

void execObjectClass(void) {

	//return what object class the obj is.
	//
	//		PARAMS:	integer
	//
	//		Returns: integer
	
	long objectId = ABLi_popInteger();
	
	long result = -1;
	GameObjectPtr object = getObject(objectId);
	if (object)
		result = object->getObjectClass();
	ABLi_pushInteger(result);
}
			
//*****************************************************************************

void execSetTimer (void) {

	//Creates a timer which counts down from time passed in
	//
	//		PARAMS:	integer, real
	//
	//		Returns: integer (ID of timer created)
	
	short timerNumber = ABLi_popInteger();
	float duration = ABLi_popReal();

	if (timerNumber < SCENARIO_TIMER_1 || timerNumber > SCENARIO_TIMER_8)
		timerNumber = 0;
	else {
		TimerPtr timer = timerManager->getTimer(timerNumber);
		Assert(timer != NULL, timerNumber, " ABL.execHbSetTimer: NULL timer ");
		timer->setTimer(duration);
	}

	ABLi_pushInteger(timerNumber);
}
			
//*****************************************************************************

void execCheckTimer (void) {

	//Checks a timer created and return time left
	//
	//		PARAMS:	integer
	//
	//		Returns: real (time left)
	
	short timerNumber = ABLi_popInteger();

	float timeLeft = 0.0;
	if ((timerNumber >= 0) && (timerNumber < MAX_TIMERS)) {
		TimerPtr timer = timerManager->getTimer(timerNumber);
		Assert(timer != NULL, timerNumber, " ABL.execHbSetTimer: NULL timer ");
		timeLeft = timer->getCurrentTime();
	}

	if (timeLeft < 0.0f)
		timeLeft = 0.0f;

	ABLi_pushReal(timeLeft);
}
			
//*****************************************************************************

void execEndTimer (void) {

	//sets time to zero and destroys timer.
	//
	//		PARAMS:	integer
	//
	//		Returns: nothing
	
	long timerID = ABLi_popInteger();

// NOT REALLY NECESSARY ANYMORE--since the timers have no callback, they just
// keep counting down... no harm...
#if 0
	if ((timerId < SCENARIOTIMER1) || (timerId > SCENARIOTIMER8))
		timerId = 0;
	else
		application->RemoveTimer(application,timerId);
#endif
}
	
//*****************************************************************************

void execSetObjectiveTimer (void) {

	//Sets timer of objective passed in.
	//
	//		PARAMS:	integer, real
	//
	//		Returns: integer (result)
	
	long objectiveNum = ABLi_popInteger();
	float duration = ABLi_popReal();

	ABLi_pushInteger(mission->setObjectiveTimer(objectiveNum, duration));
}

//*****************************************************************************

void execCheckObjectiveTimer (void) {

	//Checks an objective timer passed in and returns time left
	//
	//		PARAMS:	integer
	//
	//		Returns: real (time left)
	
	long objectiveNum = ABLi_popInteger();
	
	ABLi_pushReal(mission->checkObjectiveTimer(objectiveNum));
}

//*****************************************************************************

void execSetObjectiveStatus (void) {

	//Sets status of objective passed in.
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: integer (result)

	long objectiveNum = ABLi_popInteger();
	long status = ABLi_popInteger();
	
	ABLi_pushInteger(mission->setObjectiveStatus(objectiveNum,status));
}

//*****************************************************************************

void execCheckObjectiveStatus (void) {

	//Checks an objective status passed in and returns it
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (status)

	long objectiveNum = ABLi_popInteger();
	
	ABLi_pushInteger(mission->checkObjectiveStatus(objectiveNum));
}

//*****************************************************************************

void execSetObjectiveType (void) {

	//Sets type of objective passed in.
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: integer (result)

	long objectiveNum = ABLi_popInteger();
	long type = ABLi_popInteger();
	
	ABLi_pushInteger(mission->setObjectiveType(objectiveNum,type));
}

//*****************************************************************************

void execCheckObjectiveType (void) {

	//Checks an objective type passed in and returns it
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (status)

	long objectiveNum = ABLi_popInteger();
	
	ABLi_pushInteger(mission->checkObjectiveType(objectiveNum));
}

//*****************************************************************************

void execPlayDigitalMusic (void) {

	//Starts playback of Digital music
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long soundNum = ABLi_popInteger();

	if (soundSystem)
		soundSystem->playABLDigitalMusic(soundNum);
		
	ABLi_pushInteger(0);	
}

//***************************************************************************

void execStopMusic(void) {

	//Stops playback of Digitalmusic
	//
	//		PARAMS:	None
	//
	//		Returns: None

	if (soundSystem)
		soundSystem->stopABLMusic();
}

//*****************************************************************************

void execPlaySoundEffect(void) {

	//Starts playback of Digital sound effect
	//
	//		PARAMS:	integer, real
	//
	//		Returns: integer (result)

	long soundNum = ABLi_popInteger();
	
	if (soundSystem)
		soundSystem->playABLSFX(soundNum);
	ABLi_pushInteger(0);
}

//*****************************************************************************

void execPlayVideo(void) {

	//Starts playback of video sequence
	//
	//		PARAMS:	integer, real
	//
	//		Returns: integer (result)

	char* fileName = ABLi_popCharPtr();

	mission->missionInterface->playMovie(fileName);

	ABLi_pushInteger(0);
}

//*****************************************************************************

void execSetRadio (void) {

	//-----------------------------------------------------
	//
	//	SET RADIO function:
	//
	//		Turns the radio(s) on or off.
	//
	//		PARAMS:	integer					warrior id
	//
	//				integer					FALSE = off, TRUE= on
	//
	//		RETURN: none
	//
	//-----------------------------------------------------


	long warriorIndex = ABLi_popInteger();
	bool radioSetting = ABLi_popBoolean();

	MechWarriorPtr pilot = getWarrior(warriorIndex);

	if (pilot && pilot->getRadio()) {
		if (radioSetting)
			pilot->getRadio()->turnOn();
		else
			pilot->getRadio()->turnOff();
	}
}

//*****************************************************************************

void execPlaySpeech (void) {

	//Starts playback of Digital speech
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: integer (result)

	long warriorIndex = ABLi_popInteger();
	long message = ABLi_popInteger();

	MechWarriorPtr pilot = getWarrior(warriorIndex);
	if (pilot)
		pilot->radioMessage(message, true);

	ABLi_pushInteger(0);		
}

//*****************************************************************************

void execPlayBetty (void) {

	//Starts playback of betty message
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	//-----------------------------------------------
	// Get the ID of the pilot whose speech this is
	long messageIndex = ABLi_popInteger();
	long result = soundSystem->playBettySample(messageIndex);
	
	ABLi_pushInteger(result);
}

//*****************************************************************************
void execGetMissionWon (void)
{
	if (Mission::terminationResult != -1)
	{
		ABLi_pushBoolean(Mission::terminationResult >= mis_PLAYER_WIN_SMALL);
	}
	else
	{
		ABLi_pushBoolean(false);
	}
}

//*****************************************************************************
void execGetMissionLost (void) 
{
	ABLi_pushBoolean((Mission::terminationResult == mis_PLAYER_LOST_BIG) || 
					(Mission::terminationResult == mis_PLAYER_LOST_SMALL));
}

//*****************************************************************************
void execGetObjectiveSuccess (void) 
{
	ABLi_pushBoolean(mission->checkObjectiveSuccess());
}

//*****************************************************************************
void execGetObjectiveFailed (void) 
{
	ABLi_pushBoolean(mission->checkObjectiveFailed());
}

//*****************************************************************************
void execGetEnemyDestroyed (void) 
{
	ABLi_pushBoolean(enemyDestroyed);
	enemyDestroyed = false;
}

//*****************************************************************************
void execGetFriendlyDestroyed (void) 
{
	ABLi_pushBoolean(friendlyDestroyed);
	friendlyDestroyed = false;
}

//*****************************************************************************
void execPlayerInCombat (void) 
{
	ABLi_pushBoolean(MechWarrior::anyPlayerInCombat());
}

//*****************************************************************************
void execGetSensorsActive (void) 
{
	ABLi_pushBoolean(SensorSystemManager::enemyInLOS);
}

//*****************************************************************************
void execGetCurrentMusicId (void) 
{
	ABLi_pushInteger(soundSystem->getCurrentMusicId());
}

void execGetMissionTuneId (void)
{
	ABLi_pushInteger(mission->getMissionTuneId());
}

//*****************************************************************************

void execSetObjectActive(void) {

	//Turns on/off the AI for the object(s) passed in
	//
	//		PARAMS:	integer				object id
	//
	//				boolean				true = on, false = off
	//
	//		Returns: integer (result)

	long objectId1 = ABLi_popInteger();
	bool active = ABLi_peekBoolean();

	//------------------------------------------------
	// Code to make this work goes here
	GameObjectPtr object1 = NULL;
	long numActivated = 0;

	if ((objectId1 >= MIN_UNIT_PART_ID) && (objectId1 <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		long numObjects = getMovers(objectId1, moverList);
		for (long i = 0; i < numObjects; i++)
			if (moverList[i]->getAwake() != active) {
				object1->setAwake(active);
#ifdef USE_IFACE
				theInterface->ActivateMech(moverList[i]->getPartId());
#endif
				numActivated++;
			}
		}
	else  {
		object1 = getObject(objectId1);
		if (object1 && (object1->getAwake() != active)) {
			object1->setAwake(active);
#ifdef USE_IFACE
			theInterface->ActivateMech(object1->getPartId());
#endif
			numActivated++;
		}
	}

	ABLi_pokeInteger(numActivated);
}	

//*****************************************************************************

void execObjectTypeID (void) {

	//Returns the objectTypeId for the ObjectNum passed in
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectId1 = ABLi_popInteger();
	
	long partID = -1;
	//---------------------------------
	// Code to make this work goes here
	GameObjectPtr object1 = getObject(objectId1);
	if (object1)
		partID = object1->getObjectType()->whatAmI();
	ABLi_pushInteger(partID);		
}	

//*****************************************************************************

void execGetTerrainObjectPartID (void) {

	//Returns the object PartID for a terrain Object
	//
	//		PARAMS: integer, integer
	//
	//		Returns: integer (result)

	long row = ABLi_popInteger();
	long col = ABLi_popInteger();
	
	long partID = MIN_TERRAIN_PART_ID + row * MAX_MAP_CELL_WIDTH + col;
	pushInteger(partID);
}	

//*****************************************************************************

void execGetWeaponAmmo (void) {

	//Returns number of rounds left based on objectId and weaponId 
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_popInteger();
	long weaponNum = ABLi_popInteger();

	//------------------------------------------------
	// Code to make this work goes here
	GameObjectPtr object1 = getObject(objectNum);
	
	if (object1 && object1->isMover())
		ABLi_pushInteger(((MoverPtr)object1)->getWeaponShots(weaponNum));
	else
		ABLi_pushInteger(-1);
}	

//*****************************************************************************

void execInArea (void) {

	//returns TRUE if object/unit/team inside area.
	//
	//		PARAMS:	integer				object/unit/team id
	//
	//				real[3]				center of area
	//
	//				real				radius of area
	//
	//				integer				min number to pass (-1 = ALL must be in)
	//
	//		RETURN: boolean				yes or no?

	long objectId = ABLi_popInteger();
	float* areaCenter = ABLi_popRealPtr();
	float areaRadius = ABLi_popReal();
	long minIn = ABLi_peekInteger();
	
	Stuff::Vector3D center;
	center.x = areaCenter[0];
	center.y = areaCenter[1];
	center.z = areaCenter[2];

	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		long numObjects = getMovers(objectId, moverList);
		if (minIn == -1) {
			ABLi_pokeInteger(1);
			long numAlive = 0;
			for (long i = 0; i < numObjects; i++) {
				GameObjectPtr obj = moverList[i];
				if (!obj->isDisabled()) {
					numAlive++;
					if (obj->getExists() && obj->getAwake()) {
						if (obj->distanceFrom(center) > areaRadius) {
							ABLi_pokeInteger(0);
							break;
						}
					}
				}
			}
			if (numAlive == 0)
				ABLi_pokeInteger(0);
			}
		else {
			ABLi_pokeInteger(0);
			long numIn = 0;
			for (long i = 0; i < numObjects; i++) {
				MoverPtr mover = moverList[i];
				if (mover->getExists() && mover->getAwake() && !mover->isDisabled() && !mover->isDestroyed())
					if (mover->distanceFrom(center) <= areaRadius) {
						if (++numIn == minIn) {
							ABLi_pokeInteger(1);
							break;
						}
					}
			}
		}
		}
	else {
		ABLi_pokeInteger(1);
		if (minIn != 0) {
			ABLi_pokeInteger(0);
			GameObjectPtr obj = getObject(objectId);
			if (obj && obj->getExists() && obj->getAwake() && !obj->isDisabled() && !obj->isDestroyed()) {
				if (obj->distanceFrom(center) <= areaRadius)
					ABLi_pokeInteger(1);
			}
		}
	}
}

//*****************************************************************************

void execObjectRemove (void) {

	//---------------------------------------------------------------------
	//
	//	Object Remove
	//
	//		Remove the object from the game (COMPLETELY removes).
	//		Currently only works for Movers.
	//
	//		PARAMS:	integer			object id
	//
	//		RETURN:	integer			result (0 = success)
	//
	//---------------------------------------------------------------------

	long objectId = ABLi_popInteger();
	
	GameObjectPtr object = getObject(objectId);
	if (object) {
		if (object->isMover()) {
			//-------------------------------------------------------------
			// If the object we're removing is the CurObject, then we can't
			// do it until after we finish this object's update!
			// So, we'll just set the removed flag, and then check it
			// at the end of the update.
			if (object == CurObject)
				CurObject->setFlag(OBJECT_FLAG_REMOVED, true);
			else
				mission->removeMover((MoverPtr)object);
			ABLi_pushInteger(0);
			}	
		else
			ABLi_pushInteger(1);
	}
}

//*****************************************************************************

void execCreateInfantry (void) {

	long teamID = ABLi_popInteger();
	long commanderID = ABLi_popInteger();
	float* worldPos = ABLi_popRealPtr();
	long buildingPartID = ABLi_popInteger();

	MoverInitData moverData;
	memset(&moverData, 0, sizeof(MoverInitData));
	strcpy(moverData.pilotFileName, "pmw00080");
	strcpy(moverData.brainFileName, "infantry001");
	strcpy(moverData.csvFileName, "PM101100");
	moverData.objNumber = 53;
	moverData.rosterIndex = 255;
	moverData.controlType = 2;
	moverData.controlDataType = 0;
	moverData.variant = 0;
	moverData.position.x = worldPos[0];
	moverData.position.y = worldPos[1];
	moverData.position.z = 0.0;
	moverData.rotation = 0.0;
	moverData.teamID = teamID;
	moverData.commanderID = commanderID;
	moverData.gestureID = 2;
	moverData.active = true;
	moverData.exists = true;
	moverData.icon = 0;
	moverData.capturable = false;
	long infantryHandle = mission->addMover(&moverData);

	MoverPtr infantry = (MoverPtr)ObjectManager->get(infantryHandle);
	if (infantry)
		infantry->getPilot()->setIntegerMemory(0, buildingPartID);

	ABLi_pushInteger(0);
}

//*****************************************************************************

void execGetSensorsWorking (void) {

	//Returns whether or not sensors are operative for object passed in
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_popInteger();

	long result = -1;
	GameObjectPtr object1 = getObject(objectNum);
	if (object1 && object1->isMover() && ((MoverPtr)object1)->sensorSystem)
		result = ((MoverPtr)object1)->sensorSystem->enabled();
	
	ABLi_pushInteger(result);
}	

//*****************************************************************************

void execGetCurrentBRValue (void) {

	//Returns the current BR value for the object passed in.
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_popInteger();

	long result = -1;
	//------------------------------------------------
	// Code to make this work goes here
	GameObjectPtr object1 = getObject(objectNum);
	if (object1)
		result = object1->getCurCV();

	ABLi_pushInteger(result);		
}	

//*****************************************************************************

void execSetCurrentBRValue (void) {

	//Sets the current BR value for the object passed in.
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: None

	long objectNum = tos->integer;
	long newBRValue = tos->integer;

	//------------------------------------------------
	// Code to make this work goes here
	GameObjectPtr object1 = getObject(objectNum);
	if (object1)
		object1->setCurCV(newBRValue);
}	

//*****************************************************************************

void execGetArmorPts (void) {

	//Returns the current number of armor points remaining on the object passed in
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_peekInteger();

	//---------------------------------
	// Code to make this work goes here
	GameObjectPtr obj = getObject(objectNum);
	long armorPts = 0;
	ABLi_pokeInteger(0);	
	if (obj && obj->isMover()) {
		MoverPtr mover = (MoverPtr)obj;
		for (long i = 0; i < mover->numArmorLocations; i++)
			armorPts += mover->armor[i].curArmor;
		ABLi_pokeInteger(armorPts);
	}
}	

//*****************************************************************************

void execGetMaxArmor (void) {

	//Returns the max number of armor points on the object passed in
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_peekInteger();

	//---------------------------------
	// Code to make this work goes here
	ABLi_pokeInteger(0);
	GameObjectPtr obj = getObject(objectNum);
	long armorPts = 0;
	if (obj && obj->isMover()) {
		MoverPtr mover = (MoverPtr)obj;
		for (long i = 0; i < mover->numArmorLocations; i++)
			armorPts += mover->armor[i].maxArmor;
		ABLi_pokeInteger(armorPts);
	}
}	

//*****************************************************************************

void execGetPilotID (void) {

	//Returns the pilot ID of the driver of the object passed in.
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_peekInteger();

	//---------------------------------
	// Code to make this work goes here
	GameObjectPtr obj = getObject(objectNum);
	ABLi_pokeInteger(-1);
	if (obj) {
		MechWarriorPtr pilot = obj->getPilot();
		if (pilot)
			ABLi_pokeInteger(pilot->getIndex());
	}
}	

//*****************************************************************************

void execGetPilotWounds (void) {

	//Returns number of wounds pilot of vehicle has sustained.
	//
	//		PARAMS:	integer
	//
	//		Returns: real (result)

	long objectNum = ABLi_peekInteger();

	//---------------------------------
	// Code to make this work goes here
	GameObjectPtr obj = getObject(objectNum);
	ABLi_pokeReal(0.0);
	if (obj) {
		MechWarriorPtr pilot = obj->getPilot();
		if (pilot)
			ABLi_pokeReal(pilot->getWounds());
	}
}	

//*****************************************************************************

void execSetPilotWounds (void) {

	//Returns number of wounds pilot of vehicle has sustained.
	//
	//		PARAMS:	integer
	//
	//		Returns: real (result)

	long objectNum = ABLi_popInteger();
	long wounds = ABLi_popInteger();

	//------------------------------------------------
	// Code to make this work goes here
	if (wounds > 6)
		wounds = 6;
		
	GameObjectPtr obj = getObject(objectNum);
	if (obj) {
		MechWarriorPtr pilot = obj->getPilot();
		if (pilot)
			pilot->setWounds(wounds);
	}
}	

//*****************************************************************************

void execGetObjectActive (void) {

	//Checks to see if the object(s) passed in are active
	//
	//		PARAMS:	integer, real
	//
	//		Returns: integer (result)

	long objectId = ABLi_popInteger();

	long result = 0;
	GameObjectPtr object1 = NULL;
	if ((objectId >= MIN_UNIT_PART_ID) && (objectId <= MAX_UNIT_PART_ID)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		// Run through all objects fitting the group.
		long numObjects = getMovers(objectId, moverList);
		long numAwake = 0;
		for (long i = 0; i < numObjects; i++)
			if (moverList[i]->getAwake())
				numAwake++;
		result = numAwake;
		}
	else {
		object1 = getObject(objectId);
		if (object1 && object1->getAwake())
			result = 1;
	}

	ABLi_pushInteger(result);
}	

//***************************************************************************

void execGetObjectDamage (void) {

	//Returns the damage level of the object passed in
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_peekInteger();

	//------------------------------------------------
	// Code to make this work goes here
	ABLi_pokeInteger(0);
	GameObjectPtr obj = getObject(objectNum);
	if (obj && obj->isTerrainObject()) {
		float dmgLevel = obj->getDamageLevel();
		//Assert(dmgLevel > 0.0, (long)dmgLevel, " ABL.execHbGetObjDamage: ZERO damageLevel ");
		float dmgResult = 100.0;
		if (dmgLevel > 0.0)
			dmgResult = floor((obj->getDamage() / dmgLevel) * 100.0);
		ABLi_pokeInteger((long)dmgResult);
	}
}	

//***************************************************************************

void execGetObjectDmgPts (void) {

	//Returns the damage points of the object passed in
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_peekInteger();

	//------------------------------------------------
	// Code to make this work goes here
	ABLi_pokeInteger(0);
	GameObjectPtr obj = getObject(objectNum);
	if (obj && obj->isTerrainObject()) {
		long dmgResult = (long)obj->getDamage();
		ABLi_pokeInteger(dmgResult);
	}
}	

//***************************************************************************

void execGetObjectMaxDmg (void) {

	//Returns the damage level of the object passed in
	//
	//		PARAMS:	integer
	//
	//		Returns: integer (result)

	long objectNum = ABLi_peekInteger();

	//------------------------------------------------
	// Code to make this work goes here
	ABLi_pokeInteger(0);
	GameObjectPtr obj = getObject(objectNum);
	if (obj) {
		float damage = obj->getDamage();
		ABLi_pokeInteger((long)damage);
	}
}	

//***************************************************************************

void execSetObjectDamage (void) {

	//Sets the damage level of the object passed in
	//Note that the damage can only be given, not taken away!
	//
	//		PARAMS:	integer, integer
	//
	//		Returns: none

	long objectNum = ABLi_popInteger();
	long dmgPercentage = ABLi_popInteger();
	if (dmgPercentage > 100)
		dmgPercentage = 100;

	//---------------------------------
	// Code to make this work goes here
	GameObjectPtr obj = getObject(objectNum);
	if (obj && (dmgPercentage > 0)) {
		float maxDmg = obj->getDamageLevel();
		float curDmg = obj->getDamage();
		float dmgPoints = ((float)dmgPercentage / 100.0) * maxDmg - curDmg;
		dmgPoints += 1.0f;	//One to be sure.
		if (dmgPoints > 0.0) 
		{
			WeaponShotInfo shot;
			shot.init(0, -1, dmgPoints, 0, 0);
			if (MPlayer) 
			{
				if (MPlayer->isServer()) 
				{
					obj->handleWeaponHit(&shot, true);
				}
			}
			else
				obj->handleWeaponHit(&shot);
		}
	}
}	

//*****************************************************************************

void execGetGlobalValue (void) {

	//Gets the value stored in global variable specified by passed in number.
	//
	//		PARAMS:	integer
	//
	//		Returns: real (result)

	long variableNum = ABLi_peekInteger();

	//------------------------------------------------
	// Code to make this work goes here
	ABLi_pokeReal(0.0);
	if ((variableNum >= 0) && (variableNum < MAX_GLOBAL_MISSION_VALUES))
		ABLi_pokeReal(globalMissionValues[variableNum]);
}	

//***************************************************************************

void execSetGlobalValue (void) {

	//Sets the value stored in global variable specified by passed in number.
	//
	//		PARAMS:	integer, real
	//
	//		Returns: nothing

	long variableNum = ABLi_popInteger();
	float value = ABLi_popReal();

	//------------------------------------------------
	// Code to make this work goes here
	if ((variableNum >= 0) && (variableNum < MAX_GLOBAL_MISSION_VALUES))
		globalMissionValues[variableNum] = value;
}	

//***************************************************************************

void execSetObjectivePos (void) {

	//Sets the position for objective passed in.
	//
	//		PARAMS:	integer, real, real, real
	//
	//		Returns: nothing

	long variableNum = ABLi_popInteger();
	float X = ABLi_popReal();
	float Y = ABLi_popReal();
	float Z = ABLi_popReal();

	mission->setObjectivePos(variableNum, X, Y, Z);
}	

//*****************************************************************************

void execSetSensorRange (void) {

	//-----------------------------------------------------
	//
	//	SET SENSOR RANGE function:
	//
	//		Sets the sensor range of the object.
	//
	//		PARAMS:	integer				object id
	//
	//				real				range
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float range = ABLi_popReal();

	GameObjectPtr obj = getObject(objectId);
	if (obj)
		obj->setSensorRange(range);
}	

//***************************************************************************

void execSetTonnage (void) {

	//-----------------------------------------------------
	//
	//	SET TONNAGE function:
	//
	//		Sets the tonnage of the object.
	//
	//		PARAMS:	integer				object id
	//
	//				real				tons
	//
	//		RETURN: nothing
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float tonnage = ABLi_popReal();

	GameObjectPtr object = getObject(objectId);
	if (object)
		object->setTonnage(tonnage);
}	

//*****************************************************************************

void execSetExplosionDamage (void) {

	//-----------------------------------------------------
	//
	//	SET Explosion Damage function:
	//
	//		Sets the Explosion Damage of the object.
	//
	//		PARAMS:	integer				object id
	//
	//				real				points of damage
	//
	//		RETURN: nothing
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float dmg = ABLi_popReal();

	GameObjectPtr obj = getObject(objectId);
	if (obj)
		obj->setExplDmg(dmg);
}	

//*****************************************************************************

void execSetExplosionRadius (void) {

	//-----------------------------------------------------
	//
	//	SET Explosion Radius function:
	//
	//		Sets the explosion radius of the object.
	//
	//		PARAMS:	integer				object id
	//
	//				real				radius (in Meters)
	//
	//		RETURN: nothing
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float radius = ABLi_popReal();

	GameObjectPtr obj = getObject(objectId);
	if (obj)
		obj->setExplRad(radius);
}	

//*****************************************************************************

void execSetSalvage (void) {

	//-----------------------------------------------------
	//
	//	SET Salvage function:
	//
	//		Sets the salvage for a building object.
	//
	//		PARAMS:	integer				object id 
	//
	//				integer				MasterComponentID of salvage
	//
	//				integer				Number of these components
	//
	//		RETURN: BOOLEAN				True if function succeeds
	//
	//-----------------------------------------------------

	//long objectId = 
	ABLi_popInteger();
	//long component = 
	ABLi_popInteger();
	//long itemCount = ABLi_peekInteger();

	ABLi_pokeInteger(0);
#ifdef USE_SALVAGE
	GameObjectPtr obj = getObject(objectId);
	if (obj) {
		SalvageItemPtr tmpSalvage = object->getSalvage();
		SalvageItemPtr newSalvage = new SalvageItem;

		if (newSalvage) {
			ABLi_pokeInteger(1);
			newSalvage->itemID = component;
			newSalvage->numItems = itemCount;
			while (tmpSalvage && tmpSalvage->next)
				tmpSalvage = tmpSalvage->next;

			if (object->getSalvage())
				tmpSalvage->next = newSalvage;
			else
				object->setSalvage(newSalvage);
		}
	}
#endif
}	

//*****************************************************************************

void execSetSalvageStatus (void){

	//-----------------------------------------------------
	//
	//	SET Salvage function:
	//
	//		Sets a mover on or off the salvage list
	//
	//		PARAMS:	integer				object id 
	//
	//				BOOLEAN				TRUE = on list, FALSE = off
	//
	//		RETURN: BOOLEAN				True if function succeeds
	//
	//-----------------------------------------------------

	//long objectId = 
	ABLi_popInteger();
	//bool on = ABLi_peekBoolean();

	ABLi_pokeBoolean(false);
#ifdef USE_SALVAGE
	GameObjectPtr obj = getObject(objectId);
	if (obj && TACMAP && (object->isMover() || object->isBuilding())) {
		if (on)
			ABLi_pokeBoolean(TACMAP->AddSalvage(object));
		else
			ABLi_pokeBoolean(TACMAP->RemoveSalvage(object));
	}
#endif
}	

//*****************************************************************************

void execSetAnimation (void) {

	//-----------------------------------------------------
	//
	//	SET Animation function:
	//
	//		Sets the salvage for a building object.	(Say what?! --jm)
	//		ALL RIGHT, ALLRIGHT!!!!  Sets the animation State and subState. -fs
	//
	//		PARAMS:	integer				object id 
	//
	//				integer				Animation State
	//
	//				integer				Animation Sub State
	//
	//		RETURN: Nothing
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	//long newState = 
	ABLi_popInteger();
	//long newSubState = 
	ABLi_popInteger();

	GameObjectPtr obj = getObject(objectId);
	if (obj && obj->isBuilding()) {
#ifdef USE_BUILDING_APPR
		((BuildingPtr)building)->setAnimState(newState, newSubState);
#endif
	}
}	

//*****************************************************************************

void execSetRevealed (void) {

	//Reveals terrain around point a distance out.
	//
	//		PARAMS:	integer, real, real[2]
	//
	//		Returns: nothing
	
	long teamID = ABLi_popInteger();
	float distance = ABLi_popReal();
	float* coordList = ABLi_popRealPtr();
	
	Stuff::Vector3D position2;
	position2.x = coordList[0];
	position2.y = coordList[1];
	position2.z = 0.0;
	
	if (teamID > -1)
		land->markRadiusSeen(position2,distance,teamID);

	//------------------------------------------------------------------
	// This is sloppy, and we need a better way to update their scans...
	for (long i = 0; i < Team::numTeams; i++)
		SensorManager->getTeamSensor(i)->scanBattlefield();
}
	
//*****************************************************************************

void execGetSalvage (void) {

	//-----------------------------------------------------
	//
	//	GET Salvage function:
	//
	//		Returns two list of itemIds and quantities of salvage in an object
	//
	//		PARAMS:	integer				building ID
	//
	//				integer				number of item in array
	//
	//				integer[]			item list
	//
	//				integer[]			quantity list
	//
	//		RETURN: none
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	long itemCount = ABLi_popInteger();
	int* items = ABLi_popIntegerPtr();
	int* quantities = ABLi_popIntegerPtr();

	for (long i = 0; i < itemCount; i++) {
		items[i] = -1;
		quantities[i] = -1;
	}

	GameObjectPtr obj = getObject(objectId);
	if (obj) {
#ifdef USE_SALVAGE
		SalvageItemPtr tmpSalvage = object->getSalvage();
		while (tmpSalvage && itemCount > 0)
		{
			*tmpItems = tmpSalvage->itemID;
			*tmpQuantities = tmpSalvage->numItems;
			tmpItems++;
			tmpQuantities++;
			tmpSalvage = tmpSalvage->next;
			itemCount--;
		}
#endif
	}
}

//*****************************************************************************

void execOrderRefit (void) {

	//-----------------------------------------------------
	//
	//	Refit function:
	//
	//		Current Object starts refiting mover
	//
	//		PARAMS:	integer				mover ID
	//
	//				integer				order params
	//
	//		RETURN: none
	//
	//-----------------------------------------------------

	long refiteeId = ABLi_popInteger();
	long params = ABLi_popInteger();

	if (CurObject) {
		MechWarriorPtr pilot = CurObject->getPilot();
		if (pilot) {
			GameObjectPtr refitee = getObject(refiteeId);
			if (refitee && refitee->getObjectClass() == BATTLEMECH)
				pilot->orderRefit(ORDER_ORIGIN_COMMANDER, refitee, params);
		}
	}
}

//*****************************************************************************

void execSetCaptured (void) {

	//-----------------------------------------------------
	//
	//	Set Captured function:
	//
	//		Sets an objects captured flag to TRUE, and starts the capture display
	//
	//		PARAMS:	integer				building ID
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();

	GameObjectPtr object = getObject(objectId);
	if (object)
		object->setCaptured(true);
}

//*****************************************************************************

void execOrderCapture (void) {

	//-----------------------------------------------------
	//
	//	Capture Object function:
	//
	//		Sets CurObject to capture object specified by target ID
	//
	//		PARAMS:	integer				target ID
	//
	//				integer				order params
	//
	//		RETURN:	NONE
	//
	//-----------------------------------------------------

	long targetId = ABLi_popInteger();
	long params = ABLi_popInteger();

	GameObjectPtr target = NULL;
	if (CurObject && CurObject->isMover())
		target = getObject(targetId);

	if (target)
		CurObject->getPilot()->orderCapture(ORDER_ORIGIN_COMMANDER, target, -1, params);
}

//*****************************************************************************

void execSetCapturable (void) {

	//-----------------------------------------------------
	//
	//	Set Capturable function:
	//
	//		Sets an objects captureabl flag to captureable.
	//
	//		PARAMS:	integer				building ID
	//
	//				boolean				captureable
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	//bool captureable = 
	ABLi_popBoolean();

	GameObjectPtr obj = getObject(objectId);
	if (obj) {
		if (MPlayer)
			obj->clearCaptured();
		if (obj->isBuilding() || (obj->getObjectClass() == GROUNDVEHICLE))
			obj->setFlag(OBJECT_FLAG_CAPTURABLE, true);
	}
}

//*****************************************************************************

void execIsCaptured (void) {

	//-----------------------------------------------------
	//
	//	Is Captured function:
	//
	//		Returns the number of captured object in the group 
	//		(0 or 1 if called for a separate object)
	//
	//		PARAMS:	integer				object ID
	//
	//		RETURN: integer
	//
	//-----------------------------------------------------

	long objectId = ABLi_peekInteger();

	long numCaptured = 0;
	if (isUnitId(objectId)) {
		//--------------------------------------------
		// We have a group.  Act accordingly.
		long numObjects = getMovers(objectId, moverList);
		for (long i = 0; i < numObjects; i++)
			if (moverList[i]->isCaptured())
				numCaptured++;
		}
	else {
		GameObjectPtr obj = getObject(objectId);
		if (obj && obj->isCaptured())
			numCaptured++;
	}

	ABLi_pokeInteger(numCaptured);
}

//*****************************************************************************

void execIsCapturable (void) {

	//-----------------------------------------------------
	//
	//	Is Capturable function:
	//
	//		Returns whether or not an object is capturable
	//
	//		PARAMS:	integer				object ID
	//
	//				integer				capturing team ID
	//
	//		RETURN: BOOL
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	long capturingTeamID = ABLi_popInteger();

	ABLi_pushBoolean(false);
	GameObjectPtr obj = getObject(objectId);
	if (capturingTeamID == -2) {
		if (obj && obj->getFlag(OBJECT_FLAG_CAPTURABLE) && !obj->isDestroyed())
			ABLi_pokeBoolean(true);
	}
	else {
		if (obj && obj->isCaptureable(capturingTeamID))
			ABLi_pokeBoolean(true);
	}
}

//*****************************************************************************

void execWasEverCapturable (void) {

	//-----------------------------------------------------
	//
	//	Was Ever Is Capturable function:
	//
	//		Returns whether or not object was ever capturable
	//			(regardless of whether it's dead, currently captured, etc.)
	//			Only applies to vehicles!
	//
	//		PARAMS:	integer				object ID
	//
	//		RETURN: BOOL
	//
	//-----------------------------------------------------

	long objectId = ABLi_peekInteger();

	GameObjectPtr obj = getObject(objectId);
	ABLi_pokeBoolean(false);
	if (obj && obj->getFlag(OBJECT_FLAG_CAPTURABLE))
		ABLi_pokeBoolean(true);
}

//*****************************************************************************

void execSetBuildingName (void) {

	//-----------------------------------------------------
	//
	//	SetBuildingName Function:
	//
	//		Sets a building's name
	//
	//		PARAMS:	integer				building ID
	//
	//				integer				name Resource ID
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	//long newName = 
	ABLi_popInteger();

	GameObjectPtr obj = getObject(objectId);
	if (obj) {
#ifdef USE_RESOURCE_STRINGS
		char tmpString[255];
		cLoadString(thisInstance,newName+languageOffset,tmpString,254);
		obj->setName(tmpString);
#endif
	}
}

//*****************************************************************************

void execCallStrike (void) {

	//-----------------------------------------------------
	//
	//	CallStrike function:
	//
	//		calls an air or artillery strike at the specified location
	//
	//		PARAMS:	integer			strike type (248 = big arty, 249 = sm arty, 250 sensor arty,
	//											 507 = big air strike, 508 = sm air, 509 = sensor air
	//
	//				integer			id of target object
	//
	//				real			x coord of strike
	//
	//				real			y coord of strike
	//
	//				real			z coord of strike
	//
	//				bool			clan (is this coming from the clan?)
	//
	//		RETURN: nothing
	//
	//-----------------------------------------------------

	//---------------------------------------------------------------------
	// Since artillery strikes are commander(player) based, not team based,
	// & current missions are coded to assume team based, we'll just
	// fatal if we're not in a campaign mission. Then, we can decide if
	// it's worth re-coding ABL scripts (since we'll have to change the
	// parameters for this call to specify the player that's calling the
	// strike)...
	if (MPlayer)
		Fatal(0, " ABL: Calling ArtilleryStrike in Multiplayer game ");

	long strikeType = ABLi_popInteger();
	long targetId = ABLi_popInteger();
	Stuff::Vector3D strikeLoc;
	strikeLoc.x = ABLi_popReal();
	strikeLoc.y = ABLi_popReal();
	strikeLoc.z = ABLi_popReal();
	bool clanStrike = ABLi_popBoolean();

	GameObjectPtr target = getObject(targetId);

	if (target)
		IfaceCallStrike(strikeType, NULL, target, false);
	else {
		strikeLoc.z = land->getTerrainElevation(strikeLoc);
		IfaceCallStrike(strikeType, &strikeLoc, NULL, false, clanStrike);
	}
}

//*****************************************************************************

void execCallStrikeEx (void) {

	//-----------------------------------------------------
	//
	//	CallStrikeEx function:
	//
	//		calls an air or artillery strike at the specified location delayed by time
	//
	//		PARAMS:	integer			strike type (248 = big arty, 249 = sm arty, 250 sensor arty,
	//											 507 = big air strike, 508 = sm air, 509 = sensor air
	//
	//				integer			id of target object
	//
	//				real			x coord of strike
	//
	//				real			y coord of strike
	//
	//				real			z coord of strike
	//
	//				bool			clan (is this coming from the clan?)
	//
	//				real			time to impact
	//
	//		RETURN: nothing
	//
	//-----------------------------------------------------

	//---------------------------------------------------------------------
	// Since artillery strikes are commander(player) based, not team based,
	// & current missions are coded to assume team based, we'll just
	// fatal if we're not in a campaign mission. Then, we can decide if
	// it's worth re-coding ABL scripts (since we'll have to change the
	// parameters for this call to specify the player that's calling the
	// strike)...
	if (MPlayer)
		Fatal(0, " ABL: Calling ArtilleryStrike in Multiplayer game ");

	long strikeType = ABLi_popInteger();
	long targetId = ABLi_popInteger();
	Stuff::Vector3D strikeLoc;
	strikeLoc.x = ABLi_popReal();
	strikeLoc.y = ABLi_popReal();
	strikeLoc.z = ABLi_popReal();
	bool clanStrike = ABLi_popBoolean();
	float timeToImpact = ABLi_popReal();

	if (timeToImpact < 0.0)
		timeToImpact = 0.0;

	GameObjectPtr target = getObject(targetId);

	if (target)
		IfaceCallStrike(strikeType, NULL, target, false, clanStrike, timeToImpact);
	else {
		strikeLoc.z = land->getTerrainElevation(strikeLoc);
		IfaceCallStrike(strikeType, &strikeLoc, NULL, false, clanStrike, timeToImpact);
	}
}

//*****************************************************************************

void execOrderLoadElementals (void) {

	//-----------------------------------------------------
	//
	// LoadElementals function:	tells elementals of selected group to load into specified carrier
	//
	//		PARAMS:	integer			carrier id
	//
	//		RETURN: nothing
	//
	//-----------------------------------------------------

	//long carrierId = 
	ABLi_popInteger();

#ifdef USE_ELEMENTALS
	GameObjectPtr carrior = NULL;
	if (CurObject && CurObject->getObjectClass() == ELEMENTAL)
		carrier = getObject(carrierID);

	if (carrier && carrier->getObjectClass() == GROUNDVEHICLE && ((GroundVehiclePtr) carrier)->elementalCarrier)
		CurObject->getPilot()->orderLoadIntoCarrier(ORDER_ORIGIN_COMMANDER, carrier);
#endif
}

//*****************************************************************************

void execOrderDeployElementals (void) {

	//-----------------------------------------------------
	//
	// CallStrike function:	tells elemental carrier to deploy any loaded elementals
	//
	//		PARAMS:	real[3]			coord of deployment location
	//
	//				integer			misc. order params (set to 0, typically)
	//
	//		RETURN: nothing
	//
	//-----------------------------------------------------

	//long params = 
	ABLi_popInteger();

#ifdef USE_ELEMENTALS
	if (CurObject && CurObject->getObjectClass() == GROUNDVEHICLE && ((GroundVehiclePtr) CurObject)->elementalCarrier)
		CurObject->getPilot()->orderDeployElementals(ORDER_ORIGIN_COMMANDER, params);
#endif
}

//***************************************************************************

void execAddPrisoner (void) {

	//-----------------------------------------------------
	//
	// AddPrisoner function:	adds specified prisoner to specified building
	//
	//		PARAMS:	integer		building id
	//
	//				integer		prisoner id
	//
	//		RETURN: 0 if successful
	//
	//-----------------------------------------------------

	//long prisonId = 
	ABLi_popInteger();
	//long prisonerId = ABLi_peekInteger();

	//GameObjectPtr prison = getObject(prisonId);
	ABLi_pokeInteger(-1);

#ifdef USE_PRISONS
	if (prison && prison->isBuilding() && mission)
	{
		for (i=1; i<=scenario->getNumWarriors(); i++)
		{
			prisoner = scenario->getWarrior(i);
			if (prisoner && prisoner->getIndex() == prisonerID)
				break;
			prisoner = NULL;
		}
		if (prisoner)
		{
			if (prison->getObjectClass() == BUILDING)
			{
			BuildingPtr bPrison = (BuildingPtr) prison;
				
				for (i=0; i<MAX_PRISONERS; i++)
					if (bPrison->prisoners[i] == NULL)
					{
						bPrison->prisoners[i] = prisoner;
						result = 0;
					}
			}
			if (prison->getObjectClass() == TREEBUILDING)
			{
			TreeBuildingPtr tPrison = (TreeBuildingPtr) prison;
				
				for (i=0; i<MAX_PRISONERS; i++)
					if (tPrison->prisoners[i] == NULL)
					{
						tPrison->prisoners[i] = prisoner;
						result = 0;
					}
			}
		}
	}
#endif
}

//*****************************************************************************

void execLockGateOpen (void) {

	//-----------------------------------------------------
	//
	// LockGateOpen function:	Forces a gate to be Open Forever
	//
	//		PARAMS:	integer		gate id
	//
	//		RETURN: none
	//
	//-----------------------------------------------------

	//long gateID = 
	ABLi_popInteger();

#ifdef USE_GATES
	GameObjectPtr obj = getObject(gateID);
	if (obj && obj->getObjectClass() == GATE)
		((GatePtr) obj)->setLockedOpen();
#endif
}

//***************************************************************************

void execLockGateClosed (void) {

	//-----------------------------------------------------
	//
	// LockGateOpen function:	Forces a gate to be Closed Forever
	//
	//		PARAMS:	integer		gate id
	//
	//		RETURN: none
	//
	//-----------------------------------------------------

	//long gateID = 
	ABLi_popInteger();

#ifdef USE_GATES
	GameObjectPtr obj = getObject(gateID);
	if (obj && obj->getObjectClass() == GATE)
		((GatePtr) obj)->setLockedClose();
#endif
}

//*****************************************************************************

void execReleaseGateLock (void) {

	//-----------------------------------------------------
	//
	// LockGateOpen function:	Releases any lock on a gate.
	//
	//		PARAMS:	integer		gate id
	//
	//		RETURN: none
	//
	//-----------------------------------------------------

	//long gateID = 
	ABLi_popInteger();

#ifdef USE_GATES
	GameObjectPtr obj = getObject(gateID);
	if (obj && obj->getObjectClass() == GATE)
		((GatePtr) obj)->releaseLocks();
#endif
}

//*****************************************************************************

void execIsGateOpen (void) {

	//-----------------------------------------------------
	//
	//	IsGateOpen function:
	//
	//		Returns whether or not a gate is open
	//
	//		PARAMS:	integer				object ID
	//
	//		RETURN: BOOL				Open
	//
	//-----------------------------------------------------

	//long objectId = ABLi_peekInteger();

	ABLi_pokeBoolean(false);
#ifdef USE_GATES
	GameObjectPtr obj = getObject(objectId);
	if (obj && (obj->getObjectClass() == GATE))	{
		ABLi_pokeBoolean(((GatePtr)object)->opened);
#endif
}

//*****************************************************************************

void execGetRelativePositionToPoint (void) {

	//-----------------------------------------------------
	//
	//	Get Relative Position To Point function:
	//
	//		Calcs a position relative to the given point
	//
	//		PARAMS:	real[3]				point
	//
	//				real				angle (-180 <= angle <= 180.0, + is left, - is right)
	//
	//				real				distance (in meters)
	//
	//				integer				flags
	//
	//				real[3]				new point
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* pos = ABLi_popRealPtr();
	float angle = ABLi_popReal();
	float distance = ABLi_popReal();
	long flags = ABLi_popInteger();
	float* relPos = ABLi_popRealPtr();

	Stuff::Vector3D curPos;
	curPos.x = pos[0];
	curPos.y = pos[1];
	curPos.z = 0;
	Stuff::Vector3D newPos;
	newPos = relativePositionToPoint(curPos, angle, distance, flags);
	relPos[0] = newPos.x;
	relPos[1] = newPos.y;
	relPos[2] = newPos.z;
}

//*****************************************************************************

void execGetRelativePositionToObject (void) {

	//----------------------------------------------------------------------
	//
	//	Get Relative Position To Object function:
	//
	//		Calcs a position relative to the given object
	//
	//		PARAMS:	integer				object id
	//
	//				real				angle (-180 <= angle <= 180.0, + is left, - is right)
	//
	//				real				distance (in meters)
	//
	//				integer				flags
	//
	//				real[3]				new point
	//
	//		RETURN: NONE
	//
	//----------------------------------------------------------------------

	long objectId = ABLi_popInteger();
	float angle = ABLi_popReal();
	float distance = ABLi_popReal();
	long flags = ABLi_popInteger();
	float* relPos = ABLi_popRealPtr();

	GameObjectPtr object = getObject(objectId);
	if (object) {
		Stuff::Vector3D newPos;
		newPos = ((MoverPtr)object)->relativePosition(angle, distance, flags);
		relPos[0] = newPos.x;
		relPos[1] = newPos.y;
		relPos[2] = newPos.z;
	}
}

//*****************************************************************************

void execGetUnitStatus (void) {

	//Returns the current state of the health bar in percent format from 0 to 100
	//
	//		PARAMS:	integer
	//
	//		Returns: real (result)

	long objectNum = ABLi_popInteger();

	//---------------------------------
	// Code to make this work goes here
	GameObjectPtr obj = getObject(objectNum);
	ABLi_pushReal(0.0);
	if (obj)
		ABLi_pokeReal(obj->getStatusRating() * 100.0);
}	

//*****************************************************************************

void execRepair (void) {

	//-----------------------------------------------------
	//
	//	Repair function:
	//
	//		Repairs armor and internals. Currently only works on mechs
	//
	//		PARAMS:	integer				object id
	//
	//				real				points (of armor or structure) to repair
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();
	float points = ABLi_popReal();

	GameObjectPtr repairTarget = getObject(objectId);
	if (repairTarget && (repairTarget->getObjectClass() == BATTLEMECH)) {
		BattleMechPtr mech = (BattleMechPtr) repairTarget;
		for (long i = 0; i < mech->numBodyLocations; i++) {
			float diff = (float)mech->body[i].maxInternalStructure - mech->body[i].curInternalStructure;
			if (mech->body[i].damageState == IS_DAMAGE_DESTROYED)	// can't repair it if it ain't there!
				continue;
			if (diff > 0) {
				if (diff < points) {
					points -= diff;
					mech->body[i].curInternalStructure = mech->body[i].maxInternalStructure;
					}
				else {
					mech->body[i].curInternalStructure += points;
					points = 0;
					break;
				}
			}
		}
		for (int i = 0; i < mech->numArmorLocations; i++) {
			float diff = (float) mech->armor[i].maxArmor - mech->armor[i].curArmor;
			if (i < mech->numBodyLocations)	{
				if (mech->body[i].damageState == IS_DAMAGE_DESTROYED)	// can't repair it if it ain't there!
					continue;
				}
			else {	// for back armor, be sure you're looking at the front...
				if (mech->body[i - mech->numBodyLocations + 1].damageState == IS_DAMAGE_DESTROYED)	// can't repair it if it ain't there!
					continue;
			}
			if (diff > 0) {
				if (diff < points) {
					points -= diff;
					mech->armor[i].curArmor= mech->armor[i].maxArmor;
					}
				else {
					mech->armor[i].curArmor += points;
					points = 0;
					break;
				}
			}
		}
	}
}

//*****************************************************************************

void execGetFixed (void) {

	//-----------------------------------------------------
	//
	//	Get Fixed function:
	//
	//		Sends mech or vehicle to appropriate repair bay
	//		returns error code if call is illegal
	//
	//		PARAMS:	integer				mech or vehicle (fixee) to repair id
	//
	//				integer				repair bay id
	//
	//				integer				parameters
	//
	//		RETURN: integer				error code
	//
	//		-1	unknown error
	//	   *	1	bay is out of repair points (probably means the bay is destroyed)
	//	   *	2	wrong kind of bay (either sending a vehicle to a mech bay, or mech to vehicle bay)
	//	   *	3	bay is already repairing this fixee
	//	   *	4	bay is repairing something else
	//	   *	5	fixee is being repaired by something else (either another bay or a refit vehicle)
	//	   *	6	fixee doesn't need repair
	//	   *	7	illegal fixee object (either there's no object with that ID, or the object isn't a mech or a vehicle)
	//	   *	8	illegal bay object (either there's no object with that ID, or the object is not a repair bay)
	//	   *	9	alignment mismatch (bay and fixee are on different sides.)
	//	* is checked by -fs on 5/5/97
	//-----------------------------------------------------
	
	long fixeeId = ABLi_popInteger();
	long bayId = ABLi_popInteger();
	long params = ABLi_popInteger();

	BuildingPtr bay = NULL;
	GameObjectPtr obj = getObject(bayId);
	if (obj && obj->isBuilding() && obj->getFlag(OBJECT_FLAG_CANREFIT))
		bay = (BuildingPtr)obj;

	long result = 8;
	if (bay) {
		result = 1;
		if (bay->getRefitPoints() > 0.0) {
			result = 7;
			obj = getObject(fixeeId);
			if (obj && ((obj->getObjectClass() == BATTLEMECH) || (obj->getObjectClass() == GROUNDVEHICLE))) {
				result = 9;
				if (obj->getTeam() && bay->isFriendly(obj->getTeam())) {
					GameObjectPtr refitBuddy = ObjectManager->getByWatchID(bay->refitBuddyWID);
					if (refitBuddy) {
						if (refitBuddy == obj)
							result = 3;
						else
							result = 4;
						}
					else if (obj->getObjectClass() == BATTLEMECH) {
						BattleMechPtr mech = (BattleMechPtr)obj;
						result = 2;
						if (bay->getFlag(OBJECT_FLAG_MECHBAY)) {
							result = 5;
							refitBuddy = ObjectManager->getByWatchID(mech->refitBuddyWID);
							if (!refitBuddy) {
								result = 6;
								if (mech->needsRefit()) {
									if (mech->getPilot()->orderGetFixed(ORDER_ORIGIN_COMMANDER, bay, params) == NO_ERR)
										result = 0;
									else
										result = -1;
								}
							}
						}
						}
					else {
						GroundVehiclePtr vehicle = (GroundVehiclePtr)obj;
						result = 2;
						if (bay->getFlag(OBJECT_FLAG_MECHBAY)) {
							result = 5;
							refitBuddy = ObjectManager->getByWatchID(vehicle->refitBuddyWID);
							if (!refitBuddy) {
								result = 6;
								if (vehicle->needsRefit()) {
									if (vehicle->getPilot()->orderGetFixed(ORDER_ORIGIN_COMMANDER, bay, params) == NO_ERR)
										result = 0;
									else
										result = -1;
								}
							}
						}
					}
				}
			}
		}
	}
	ABLi_pushInteger(result);
}

//***************************************************************************

void execGetRepairState (void) {

	//Returns the percentage of repairable armor and IS points of the objectId passed in
	//
	//		PARAMS: integer
	//
	//		Returns: integer (result)

	long objectId = ABLi_peekInteger();

	ABLi_pokeInteger(0);
	long max = 0;
	long cur = 0.0;
	GameObjectPtr object = getObject(objectId);
	if (object && object->isMover()) {
		MoverPtr mover = (MoverPtr) object;
		for (long i = 0; i < mover->numBodyLocations; i++) {
			// can't repair it if it ain't there, so don't report it if destroyed...
			if (mover->body[i].damageState == IS_DAMAGE_DESTROYED)
				continue;
			max += mover->body[i].maxInternalStructure;
			cur += mover->body[i].curInternalStructure;
		}
		for (int i = 0; i < mover->numArmorLocations; i++) {
			if (i < mover->numBodyLocations) {
				if (mover->body[i].damageState == IS_DAMAGE_DESTROYED)	// can't repair it if it ain't there!
					continue;
				}
			else {
				// for back armor, be sure you're looking at the front...
				if (mover->body[i - mover->numBodyLocations + 1].damageState == IS_DAMAGE_DESTROYED)	// can't repair it if it ain't there!
					continue;
			}
			max += mover->armor[i].maxArmor;
			cur += mover->armor[i].curArmor;
		}
	}
	// make it a percentage...
	cur *= 100;
	ABLi_pokeInteger(cur / max);
}

//*****************************************************************************

void execIsTeamTargeting (void) {

	//-----------------------------------------------------
	//
	//	Is Team Targeting function:
	//
	//		Returns whether anyone of the specified team is currently
	//		targeting the specified target with the possible exception
	//		of the one team member (which may be set to 0, if desired).
	//
	//		PARAMS:	integer				team id
	//
	//				integer				target id
	//
	//				integer				except id (may be 0)
	//
	//		RETURN: boolean				yes or no
	//
	//-----------------------------------------------------

	long teamId = ABLi_popInteger();
	long targetId = ABLi_popInteger();
	long exceptId = ABLi_popInteger();

	bool targeting = false;
	if ((teamId >= OBJ_ID_FIRST_TEAM) && (teamId <= OBJ_ID_LAST_TEAM)) {
		TeamPtr team = Team::teams[teamId - OBJ_ID_FIRST_TEAM];
		if (team)
			targeting = team->isTargeting(targetId, exceptId);
		}
	else if (teamId == 0) {
		GameObjectPtr target = ObjectManager->get(targetId);
		if (target)
			targeting = (target->getNumAttackers() > 0);
	}

	ABLi_pushBoolean(targeting);
}

//*****************************************************************************

void execIsTeamCapturing (void) {

	//-----------------------------------------------------
	//
	//	Is Team Capturing function:
	//
	//		Returns whether anyone of the specified team is currently
	//		capturing the specified target with the possible exception
	//		of the one team member (which may be set to 0, if desired).
	//
	//		PARAMS:	integer				team id
	//
	//				integer				target id
	//
	//		RETURN: boolean				yes or no
	//
	//-----------------------------------------------------

	long teamId = ABLi_popInteger();
	long targetId = ABLi_popInteger();
	long exceptId = ABLi_popInteger();

	bool targeting = false;
	GameObjectPtr target = getObject(targetId);
	unsigned long targetWID = target ? target->getWatchID(false) : 0;
	if (targetWID) {
		if ((teamId >= OBJ_ID_FIRST_TEAM) && (teamId <= OBJ_ID_LAST_TEAM)) {
			TeamPtr team = Team::teams[teamId - OBJ_ID_FIRST_TEAM];
			if (team)
				targeting = ObjectManager->isTeamCapturing(team, targetWID);
			}
		else if (teamId == 0) {
			targeting = ObjectManager->isTeamCapturing(NULL, targetWID);
		}
	}

	ABLi_pushBoolean(targeting);
}

//*****************************************************************************

void execSendMessage (void) {

	CurMultiplayCode = ABLi_popInteger();
	CurMultiplayParam = ABLi_popInteger();

	if (MPlayer && MPlayer->isServer()) {
		//------------------------------------
		// We must be server if we got here...
		MPlayer->addMissionScriptMessageChunk(CurMultiplayCode, CurMultiplayParam);
#ifdef DEBUG_MISSION_SCRIPT
		if (NumMissionScriptMessages == MAX_MISSION_MSGS) {
			DebugMissionScriptMessages();
			Assert(false, NumMissionScriptMessages, " Way too many Mission Script Messages! ");
		}
		MissionScriptMessageLog[NumMissionScriptMessages][0] = execLineNumber;
		MissionScriptMessageLog[NumMissionScriptMessages][1] = CurMultiplayCode;
		MissionScriptMessageLog[NumMissionScriptMessages][2] = CurMultiplayParam;
		NumMissionScriptMessages++;
#endif
	}
}

//*****************************************************************************

void execGetMessage (void) {

	int* messageParam = ABLi_peekIntegerPtr();

	*messageParam = CurMultiplayParam;
	ABLi_pokeInteger(CurMultiplayCode);
}

//*****************************************************************************

void execGetHomeTeam (void) {

	//-----------------------------------------------------
	//
	//	Get HomeTeam function:
	//
	//		Returns id for home team: 500 = IS, 501 = CLAN, 502 = ALLIED
	//
	//		PARAMS:	NONE
	//
	//		RETURN: integer				hometeam id
	//
	//-----------------------------------------------------

	ABLi_pushInteger(Team::home->getId() + MIN_TEAM_PART_ID);
}

//***************************************************************************

void execIsServer (void) {

	//-----------------------------------------------------
	//
	//	Is Server function:
	//
	//		Returns TRUE if I'm the server in a multi-player game.
	//
	//		PARAMS:	NONE
	//
	//		RETURN: boolean				TRUE = yes I am:)
	//
	//-----------------------------------------------------

	if (MPlayer && MPlayer->isServer())
		pushInteger(1);
	else
		pushInteger(0);
}

//*****************************************************************************

void execCalcPartID (void) {

	long objectClass = ABLi_popInteger();
	long param1 = ABLi_popInteger();
	long param2 = ABLi_popInteger();
	long param3 = ABLi_popInteger();

	long partId = ObjectManager->calcPartId(objectClass, param1, param2, param3);

	ABLi_pushInteger(partId);
}

//*****************************************************************************

void execSetDebugString (void) {

	long objectId = ABLi_popInteger();
	long stringNum = ABLi_popInteger();
	char* debugString = ABLi_popCharPtr();
	GameObjectPtr obj = getObject(objectId);
	if (obj) {
		MechWarriorPtr pilot = obj->getPilot();
		if (pilot)
			pilot->setDebugString(stringNum, debugString);
	}
#if 0
	ABLi_popInteger();
	ABLi_popInteger();
	ABLi_popCharPtr();
#endif
}

//*****************************************************************************

void execBreak (void) {

	if (ABLi_getCurrentState()) {
		DEBUGWINS_print("BREAK", 0);
	}
}

//*****************************************************************************

void execPathExists (void) {

	//-----------------------------------------------------
	//
	//	PATH EXISTS function:
	//
	//		Returns number and list of objects of the specified object type.
	//
	//		PARAMS:	integer		mover id. if set to 0, assumes a mech-type
	//
	//				integer		startCellRow
	//
	//				integer		startCellCol
	//
	//				integer		goalCellRow
	//
	//				integer		goalCellCol
	//
	//		RETURN: integer		answer: 0 = no, else number of areas in
	//										LR path
	//
	//-----------------------------------------------------

	int moverID = ABLi_popInteger();
	int startRow = ABLi_popInteger();
	int startCol = ABLi_popInteger();
	int goalRow = ABLi_popInteger();
	int goalCol = ABLi_popInteger();

	//-------------------------------------------------
	// For now, assumes we mean the ground level (0)...
	int startArea = GlobalMoveMap[0]->calcArea(startRow, startCol);
	int goalArea = GlobalMoveMap[0]->calcArea(goalRow, goalCol);
	int confidence;
	long areaPathCost = GlobalMoveMap[0]->getPathCost(startArea, goalArea, false, confidence, true);

	ABLi_pushInteger(areaPathCost);
}

//*****************************************************************************

void execConvertCoords (void) {

	int convertType = ABLi_popInteger();
	float* worldPos = ABLi_popRealPtr();
	int* cellPos = ABLi_popIntegerPtr();

	ABLi_pushInteger(0);

	if (convertType == 0) {
		// world to cell
		Stuff::Vector3D worldP;
		worldP.x = worldPos[0];
		worldP.y = worldPos[1];
		worldP.z = worldPos[2];
		land->worldToCell(worldP, cellPos[0], cellPos[1]);
		}
	else {
		// cell to world
		Stuff::Vector3D worldP;
		land->cellToWorld(cellPos[0], cellPos[1], worldP);
		worldPos[0] = worldP.x;
		worldPos[1] = worldP.y;
		worldPos[2] = 0.0;
	}
}

//*****************************************************************************

void execCoreMoveTo (void) {

	float* location = ABLi_popRealPtr();
	unsigned long params = (unsigned long)ABLi_popInteger();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	if (isnan(location[0]) || isnan(location[1]))
	{
		STOP(("Move to order location is Not a Number in Brain %s.  Check patrol path array indices!",CurWarrior->getBrainString()));
	}

	long result = 0;
	if (CurWarrior) 
	{
		Stuff::Vector3D loc(location[0], location[1], 0.0);
		result = CurWarrior->coreMoveTo(loc, params);
	}

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execCoreMoveToObject (void) {

	long objectID = ABLi_popInteger();
	unsigned long params = (unsigned long)ABLi_popInteger();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	GameObjectPtr target = getObject(objectID);
	long result = 0;
	if (CurWarrior)
		result = CurWarrior->coreMoveToObject(target, params);

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execCorePower (void) {

	bool up = ABLi_popBoolean();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long result = 0;
	if (CurWarrior)
		result = CurWarrior->corePower(up);

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execCoreAttack (void) {

	long objectID = ABLi_popInteger();
	unsigned long params = (unsigned long)ABLi_popInteger();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	GameObjectPtr target = getObject(objectID);
	long result = 0;
	if (CurWarrior)
		result = CurWarrior->coreAttack(target, params);

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execCoreCapture (void) {

	long objectID = ABLi_popInteger();
	unsigned long params = (unsigned long)ABLi_popInteger();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long targetID = 0;
	GameObjectPtr obj = getObject(objectID);
	if (CurWarrior && ((objectID == 0) || obj))
		targetID = CurWarrior->coreCapture((objectID == 0) ? NULL : obj, params);

	ABLi_pushInteger(targetID);
}

//*****************************************************************************

void execCoreScan (void) {

	long objectID = ABLi_popInteger();
	unsigned long params = (unsigned long)ABLi_popInteger();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long targetID = 0;
	GameObjectPtr obj = getObject(objectID);
	if (CurWarrior && ((objectID == 0) || obj))
		targetID = CurWarrior->coreScan((objectID == 0) ? NULL : obj, params);

	ABLi_pushInteger(targetID);
}

//*****************************************************************************

void execCoreControl (void) {

	long objectID = ABLi_popInteger();
	unsigned long params = (unsigned long)ABLi_popInteger();

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long targetID = 0;
	GameObjectPtr obj = getObject(objectID);
	if (CurWarrior && ((objectID == 0) || obj))
		targetID = CurWarrior->coreControl((objectID == 0) ? NULL : obj, params);

	ABLi_pushInteger(targetID);
}

//*****************************************************************************

void execCoreEject (void) {

	if (ABLi_getSkipOrder()) {
		ABLi_pushInteger(1);
		return;
	}

	long result = 0;
	if (CurWarrior)
		result = CurWarrior->coreEject();

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execSetPilotState (void) {

	//-----------------------------------------------------
	//
	// SetPilotState: Sets the current pilot's brain state.
	//
	//		PARAMS:	integer		new state ID
	//
	//		RETURN: integer		previous state ID
	//
	//-----------------------------------------------------

	unsigned long newStateHandle = ABLi_popInteger();
	
	long curStateHandle = ABLi_getCurrentState();
	if (curStateHandle > 0) {
		ABLi_transState(newStateHandle);
		ABLi_pushInteger(curStateHandle);
		}
	else {
		ABLi_resetOrders();
		long prevState = CurWarrior->setBrainState(newStateHandle);
		ABLi_pushInteger(prevState);
	}
}

//*****************************************************************************

void execGetPilotState (void) {

	//-----------------------------------------------------
	//
	// GetPilotState: Gets the current pilot's brain state.
	//
	//		PARAMS:	NONE
	//
	//		RETURN: integer		current state ID
	//
	//-----------------------------------------------------

	ABLi_pushInteger(CurWarrior->getBrainState());
}

//*****************************************************************************

void execGetNextPilotEvent (void) {

	//-----------------------------------------------------
	//
	// GetNextPilotEvent:	Removes & returns the next queued pilot event.
	//						Any params for the event are put into the
	//						integer array passed in.
	//
	//		PARAMS:	@integer[3]		parameter list
	//
	//		RETURN: integer			event ID
	//
	//-----------------------------------------------------

	int* paramList = ABLi_popIntegerPtr();
	
	long eventID = CurWarrior->getNextEventHistory(paramList);

	ABLi_pushInteger(eventID);
}

//*****************************************************************************

void execSetTargetPriority (void) {

	//-----------------------------------------------------
	//
	// SetTargetPriority:	Sets the target priority specified for the
	//						current pilot.
	//
	//		PARAMS:	integer			target priority index (0 thru ?)
	//
	//				integer			target priority type
	//
	//				integer			param 1
	//
	//				integer			param 2
	//
	//				integer			param 3
	//
	//		RETURN: integer			error code (0 = OK)
	//
	//-----------------------------------------------------

	long index = ABLi_popInteger();
	long type = ABLi_popInteger();
	long param1 = ABLi_popInteger();
	long param2 = ABLi_popInteger();
	long param3 = ABLi_popInteger();
	
	long err = CurWarrior->setTargetPriority(index, type, param1, param2, param3);
	
	ABLi_pushInteger(err);
}

//*****************************************************************************

void execSetDebugWindow (void) {

	//-----------------------------------------------------
	//
	// SetDebugWindow:	Sets the game object for the specified debug window.
	//
	//		PARAMS:	integer			debug window id (0 thru 3)
	//
	//				integer			object id
	//
	//		RETURN: integer			object id, if set. Else 0.
	//
	//-----------------------------------------------------

	long windowIndex = ABLi_popInteger();
	long objectID = ABLi_popInteger();
	
	GameObjectPtr obj = getObject(objectID);
	if (obj)
		DEBUGWINS_setGameObject(windowIndex, obj);
	else
		objectID = 0;

	ABLi_pushInteger(objectID);
}

//*****************************************************************************

void execSetMovieMode (void) 
{
	//-----------------------------------------------------
	//
	//	SetMovieMode
	//
	//		Changes game to movie playback mode.  GUI disappears, Commands not allowed, Letterbox
	//
	//		PARAMS: NONE
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	if (eye)
		eye->setMovieMode();
}

//*****************************************************************************

void execEndMovieMode (void) 
{
	//-----------------------------------------------------
	//
	//	EndMovieMode
	//
	//		Changes game to play mode.  GUI reappears, Commands allowed, no Letterbox
	//
	//		PARAMS: NONE
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	if (eye)
		eye->endMovieMode();
}

//*****************************************************************************

void execGetGeneralAlarm (void) 
{
	//-----------------------------------------------------
	//
	//	GetGeneralAlarm
	//
	//		Returns value of GeneralAlarm set by Perimeter alarms in game
	//
	//		PARAMS: NONE
	//
	//		RETURN: Integer
	//
	//-----------------------------------------------------

	ABLi_pushInteger(GeneralAlarm);
}

//*****************************************************************************

void execSetGeneralAlarm (void) 
{
	//-----------------------------------------------------
	//
	//	SetGeneralAlarm
	//
	//		sets value of GeneralAlarm for Perimeter alarms in game
	//
	//		PARAMS: Integer
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	GeneralAlarm = (ABLi_popInteger() != 0);
}

//*****************************************************************************

void execFadeToColor (void) 
{
	//-----------------------------------------------------
	//
	//	FadeToColor
	//
	//		Fades entire screen to color read in by the time read in.
	//
	//		PARAMS: NONE
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	DWORD fadeColor = ABLi_popInteger();
	DWORD fadeTime = ABLi_popReal();
	
	if (eye)
		eye->fadeToColor(fadeColor,fadeTime);
}

//*****************************************************************************

void execForceMovieEnd (void)
{
	//-----------------------------------------------------
	//
	// ForceMovieEnd
	//
	//		Checks if the player has requested the movie to end.  If so, the script should endMovieMode immediately!!
	//
	//		PARAMS: None
	//
	//		RETURN: 1 if user asked movie to end.  Zero otherwise
	//
	//-----------------------------------------------------
	DWORD result = 0;
	if (eye && eye->forceMovieEnd)
		result = 1;
		
	ABLi_pushInteger(result);
}

//*****************************************************************************

void execGetCameraPosition (void) {

	//-----------------------------------------------------
	//
	//	GetCameraPosition 
	//
	//		Sets the value passed in to the camera position.
	//
	//		PARAMS:	real[3]				camPos
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camPos = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
	if (eye)
		result = eye->getPosition();
	
	camPos[0] = result.x;
	camPos[1] = result.y;
	camPos[2] = result.z;
}

//*****************************************************************************

void execSetCameraPosition(void) {

	//-----------------------------------------------------
	//
	//	SetCameraPosition 
	//
	//		Sets the camera position to the value passed in.
	//
	//		PARAMS:	real[3]				camPos
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camPos = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
	result.x = camPos[0];
	result.y = camPos[1];
	result.z = camPos[2];
	
 	if (eye)
		eye->setPosition(result,false);
}

//*****************************************************************************

void execSetCameraGoalPosition (void) {

	//-----------------------------------------------------
	//
	//	SetCameraGoalPosition 
	//
	//		Sets the camera position to the value passed in.
	//
	//		PARAMS:	real[3]				camPos
	//				real				time
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camPos = ABLi_popRealPtr();
	float time = ABLi_popReal();

	Stuff::Vector3D result(0.0,0.0,0.0);
	result.x = camPos[0];
	result.y = camPos[1];
	result.z = camPos[2];
	
 	if (eye) {
		eye->setGoalPosition(result);
		eye->setGoalPosTime(time);
	}
}

//*****************************************************************************

void execGetCameraGoalPosition (void) {

	//-----------------------------------------------------
	//
	//	GetCameraGoalPosition 
	//
	//		Gets the camera position to the value passed in.
	//
	//		PARAMS:	real[3]				camPos
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camPos = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
   
 	if (eye)
		result = eye->getGoalPosition();
   
	camPos[0] = result.x;
	camPos[1] = result.y;
	camPos[2] = result.z;
}

//*****************************************************************************

void execGetCameraRotation (void) {

	//-----------------------------------------------------
	//
	//	GetCameraRotation 
	//
	//		Sets the value passed in to the camera Rotation.
	//
	//		PARAMS:	real[3]				camRot
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camRot = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
	if (eye)
		result = eye->getRotation();
	
	camRot[0] = result.x;
	camRot[1] = result.y;
	camRot[2] = result.z;
}

//*****************************************************************************

void execSetCameraRotation (void) {

	//-----------------------------------------------------
	//
	//	SetCameraRotation 
	//
	//		Sets the value passed in to be the camera Rotation.
	//
	//		PARAMS:	real[3]				camRot
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camRot = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
	
	result.x = camRot[0];
	result.y = camRot[1];
	result.z = camRot[2];
 	
	if (eye)
		eye->setRotation(result);
}

//*****************************************************************************

void execSetCameraGoalRotation (void) {

	//-----------------------------------------------------
	//
	//	SetCameraGoalRotation 
	//
	//		Sets the camera Rotation to the value passed in.
	//
	//		PARAMS:	real[3]				camRot
	//				real				time
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camRot = ABLi_popRealPtr();
	float time = ABLi_popReal();

	Stuff::Vector3D result(0.0,0.0,0.0);
	result.x = camRot[0];
	result.y = camRot[1];
	result.z = camRot[2];
	
 	if (eye) {
		eye->setGoalRotation(result);
		eye->setGoalRotTime(time);
	}
}

//*****************************************************************************

void execGetCameraGoalRotation (void) {

	//-----------------------------------------------------
	//
	//	GetCameraGoalRotation 
	//
	//		Gets the camera Rotation into the value passed in.
	//
	//		PARAMS:	real[3]				camRot
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camRot = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
   
 	if (eye)
		result = eye->getGoalRotation();
   
	camRot[0] = result.x;
	camRot[1] = result.y;
	camRot[2] = result.z;
}

//*****************************************************************************

void execGetCameraZoom (void) {

	//-----------------------------------------------------
	//
	//	GetCameraZoom
	//
	//		Gets the camera FOV into the value passed in.
	//
	//		PARAMS:	real				camFOV
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float result = 0.0;
   
 	if (eye)
		result = eye->getFieldOfView();
   
	ABLi_pushReal(result);	
}

//*****************************************************************************

void execSetCameraZoom (void) {

	//-----------------------------------------------------
	//
	//	SetCameraZoom
	//
	//		Sets the camera FOV to the value passed in.
	//
	//		PARAMS:	real				camFOV
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float camFOV = ABLi_popReal();

 	if (eye)
		eye->setFieldOfView(camFOV);
}

//*****************************************************************************

void execGetCameraGoalZoom (void) {


	//-----------------------------------------------------
	//
	//	GetCameraGoalZoom
	//
	//		Gets the camera FOV into the value passed in.
	//
	//		PARAMS:	real				camFOV
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float result = 0.0;
 	if (eye)
		result = eye->getFieldOfViewGoal();

	ABLi_pushReal(result);	
}

//*****************************************************************************

void execSetCameraGoalZoom (void) {

	//-----------------------------------------------------
	//
	//	SetCameraGoalZoom
	//
	//		Sets the camera FOV to the value passed in.
	//
	//		PARAMS:	real				camFOV
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float camFOV = ABLi_popReal();
	float time = ABLi_popReal();

 	if (eye)
		eye->setFieldOfViewGoal(camFOV, time);
}

//*****************************************************************************

void execSetCameraVelocity (void) {

	//-----------------------------------------------------
	//
	//	SetCameraVelocity 
	//
	//		Sets the camera velocity to the value passed in.
	//
	//		PARAMS:	real[3]				camVel
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camVel = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
	result.x = camVel[0];
	result.y = camVel[1];
	result.z = camVel[2];
	
 	if (eye)
		eye->setVelocity(result);
}

//*****************************************************************************

void execGetCameraVelocity (void) {

	//-----------------------------------------------------
	//
	//	GetCameraVelocity 
	//
	//		gets the camera velocity into the value passed in.
	//
	//		PARAMS:	real[3]				camVel
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camVel = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
   
 	if (eye)
		result = eye->getVelocity();
   
	camVel[0] = result.x;
	camVel[1] = result.y;
	camVel[2] = result.z;
}

//*****************************************************************************

void execSetCameraGoalVelocity (void) {

	//-----------------------------------------------------
	//
	//	SetCameraGoalVelocity 
	//
	//		Sets the camera Velocity Goal to the value passed in.
	//
	//		PARAMS:	real[3]				camVel
	//				real				time
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camVel = ABLi_popRealPtr();
	float time = ABLi_popReal();

	Stuff::Vector3D result(0.0,0.0,0.0);
	result.x = camVel[0];
	result.y = camVel[1];
	result.z = camVel[2];
	
 	if (eye) {
		eye->setGoalVelocity(result);
		eye->setGoalVelTime(time);
	}
}

//*****************************************************************************

void execGetCameraGoalVelocity (void) {

	//-----------------------------------------------------
	//
	//	GetCameraGoalVelocity 
	//
	//		Sets the camera Velocity Goal into the value passed in.
	//
	//		PARAMS:	real[3]				camVel
	//				real				time
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	float* camVel = ABLi_popRealPtr();

	Stuff::Vector3D result(0.0,0.0,0.0);
   
 	if (eye)
		result = eye->getGoalVelocity();
   
	camVel[0] = result.x;
	camVel[1] = result.y;
	camVel[2] = result.z;
}

//*****************************************************************************

void execSetCameraLookObject (void) {

	//-----------------------------------------------------
	//
	//	SetCameraLookObject 
	//
	//		Sets the camera Look Object
	//
	//		PARAMS:	long				objectId
	//
	//		RETURN: NONE
	//
	//-----------------------------------------------------

	long objectId = ABLi_popInteger();

 	if (eye)
		eye->setCameraTargetId(objectId);
}

//*****************************************************************************

void execGetCameraLookObject (void) {

	//-----------------------------------------------------
	//
	//	GetCameraGetLookObject
	//
	//		Gets the camera FOV into the value passed in.
	//
	//		PARAMS:	NONE
	//
	//		RETURN: Integer
	//
	//-----------------------------------------------------

	long result = 0;
  	if (eye)
		result = eye->getCameraTargetId();

	ABLi_pushInteger(result);
}

//*****************************************************************************

void execGetCameraFrameLength (void) {

	//-----------------------------------------------------
	//
	//	GetCameraFrameLength
	//
	//		Gets the Last Frame's Length
	//
	//		PARAMS:	NONE
	//
	//		RETURN: real
	//
	//-----------------------------------------------------


	ABLi_pushReal(frameLength);	
}

//*****************************************************************************
void execAnimationCallout (void)
{
	// Takes an integer for button num, bool for if its a button, bool for if it should press it, real for time to get there, integer for num flashes.
	//
	// Returns a bool.  True if the animation played, false if an animation is running.
	//
	
	long buttonId = ABLi_popInteger();
	bool isButton = ABLi_popBoolean();
	bool isPressed = ABLi_popBoolean();
	float timeToScroll = ABLi_popReal();
	long numFlashes = ABLi_popInteger();
	
	bool result = mission->missionInterface->startAnimation(buttonId,isButton,isPressed,timeToScroll,numFlashes);
	
	ABLi_pushBoolean(result);
}

static char tutorialMessages[MAX_CHAT_COUNT][1024];
static unsigned char currentMessage = 0;
//*****************************************************************************
void execTutorialText (void)
{
	//Takes an integer for TextID to send.
	//
	// returns nothing.
   	//
	
	long textId = ABLi_popInteger();
	
	cLoadString(textId,tutorialMessages[currentMessage],1023);
	
	mission->missionInterface->setTutorialText(tutorialMessages[currentMessage]);
	
	currentMessage++;
	if (currentMessage >= MAX_CHAT_COUNT)
		currentMessage = 0;
}

//*****************************************************************************
void execGUIIsAOEStyle (void) 
{
	//Takes NOTHING
	//
	// Returns state of GUI.
	
	ABLi_pushBoolean(mission->missionInterface->isAOEStyle());
}

//*****************************************************************************
void execSetInvulnerable (void) 
{
	//Takes a bool and sets local team invulnerability to the flag passed in.
	//
	// Returns NOTHING.
	
	bool invulnerableFlag = ABLi_popBoolean();
	invulnerableON = invulnerableFlag;
}

//*****************************************************************************
void execFreezeGUI (void) 
{
	//Takes a bool and turns mouse input on or off based on flag passed in.
	//
	// Returns NOTHING.
	
	bool guiFlag = ABLi_popBoolean();
	mission->missionInterface->freezeGUI(guiFlag);
}

//*****************************************************************************
void execLogisticsScreenId (void)
{
	//Takes NOTHING
	//
	//Returns ID of logistics Screen we are IN.  Returns -1 if not in logistics or missionBegin is NULL!
	//singlePlayerScreens[0][1] = pMissionSelectionScreen;		ID 1
	//singlePlayerScreens[1][1] = pBriefingScreen;				ID 11
	//singlePlayerScreens[2][1] = pMechBayScreen;				ID 21
	//singlePlayerScreens[3][1] = pPilotSelectionScreen;		ID 31
	//singlePlayerScreens[2][0] = pPurchaseMechScreen;			ID 20
	//singlePlayerScreens[2][2] = pMechLabScreen;				ID 22
	//singlePlayerScreens[4][1] = pLoadScreen;					ID 41

	if (logistics && logistics->getMissionBegin())
	{
		ABLi_pushInteger(logistics->getMissionBegin()->getCurrentScreenId());
	}
	else
	{
		ABLi_pushInteger(-1);
	}
}

//*****************************************************************************
void execLogisticsAnimationCallout (void)
{
	// Takes an integer for button num, real for time to get there, integer for num flashes.
	//
	// Returns a bool.  True if the animation played, false if an animation is running.
	//
	
	long buttonId = ABLi_popInteger();
	bool isButton = ABLi_popBoolean();
	float timeToScroll = ABLi_popReal();
	long numFlashes = ABLi_popInteger();
	
	if (logistics && logistics->getMissionBegin())
	{
		bool result = logistics->getMissionBegin()->startAnimation(buttonId,isButton,timeToScroll,numFlashes);
		ABLi_pushBoolean(result);
	}
	else
	{
		ABLi_pushBoolean(false);
	}
}

//*****************************************************************************
void execLogisticsInCallout (void)
{
	//takes NOTHING
	//
	// Returns a bool.  True if we are currently animating the mouse to the button area OR no logistics, false if not animating

	if (logistics && logistics->getMissionBegin())
	{
		ABLi_pushBoolean(logistics->getMissionBegin()->isInCalloutAnimation());
	}
	else
	{
		ABLi_pushBoolean(true);
	}
}

//*****************************************************************************
void execInCallout (void)
{
	//takes NOTHING
	//
	// Returns a bool.  True if we are currently animating the mouse to the button area OR no logistics, false if not animating

	if (mission && mission->missionInterface)
	{
		ABLi_pushBoolean(mission->missionInterface->isInCalloutAnimation());
	}
	else
	{
		ABLi_pushBoolean(true);
	}
}

//*****************************************************************************
void execIsPlayingVoiceOver(void)
{
	//Takes NOTHING
	//
	// Returns a bool,  true if voiceover channel is active, false if not.

	ABLi_pushBoolean(soundSystem->isPlayingVoiceOver());
}

//*****************************************************************************
void execStopVoiceOver(void)
{
	//Takes NOTHING
	//
	// Returns a NOTHING

	soundSystem->stopSupportSample();
}

//*****************************************************************************
void execGetLogisticsTime(void)
{
	//Takes NOTHING
	//
	// Returns a bool,  true if voiceover channel is active, false if not.

	ABLi_pushReal(timeGetTime());
}

//*****************************************************************************

void execRequestHelp (void) {

	//--------------------------------------------------
	// Defender is the CurPilot (calling this function).

	long enemyPartID = ABLi_popInteger();
	float* friendlyCenter = ABLi_popRealPtr();
	float friendlyRadius = ABLi_popReal();
	float* enemyCenter = ABLi_popRealPtr();
	float enemyRadius = ABLi_popReal();
	long priority = ABLi_popInteger();

	if (CurWarrior->getTeam()) {
		Stuff::Vector3D friendlyPos;
		friendlyPos.x = friendlyCenter[0];
		friendlyPos.y = friendlyCenter[1];
		friendlyPos.z = 0.0;
		MoverPtr friendlies[MAX_MOVERS];
		long commanderID = -1;
		if (MPlayer || (CurWarrior->getTeam() == Team::home))
			commanderID = CurWarrior->getCommander()->getId();
		long numFriendlies = getMoversWithinRadius(friendlies, friendlyPos, friendlyRadius, CurWarrior->getTeam()->getId(), commanderID, false, true, priority > 0);

		GameObjectPtr enemy = ObjectManager->findByPartId(enemyPartID);
		if (enemy && enemy->getTeam() && (enemy->getTeam() != CurWarrior->getTeam())) {
			Stuff::Vector3D enemyPos;
			enemyPos.x = enemyCenter[0];
			enemyPos.y = enemyCenter[1];
			enemyPos.z = 0.0;
			MoverPtr enemies[MAX_MOVERS];
			long numEnemies = getMoversWithinRadius(enemies, enemyPos, enemyRadius, enemy->getTeamId(), -1, false, true, true);
			calcAttackPlan(numFriendlies, (GameObjectPtr*)friendlies, numEnemies, (GameObjectPtr*)enemies);
		}
	}
	
	ABLi_pushReal(0.0);
}

//*****************************************************************************

void execRequestTarget (void) {

	float* center = ABLi_popRealPtr();
	float radius = ABLi_popReal();

	if (CurWarrior->getTeam()) {
		Stuff::Vector3D friendlyPos;
		friendlyPos.x = center[0];
		friendlyPos.y = center[1];
		friendlyPos.z = 0.0;
		MoverPtr friendlies[MAX_MOVERS];
		MoverPtr enemies[MAX_MOVERS];
		long commanderID = -1;
		if (MPlayer || (CurWarrior->getTeam() == Team::home))
			commanderID = CurWarrior->getCommander()->getId();
		long numFriendlies = getMoversWithinRadius(friendlies, friendlyPos, radius, CurWarrior->getTeam()->getId(), commanderID, false, true, true);
		long numEnemies = getMoversWithinRadius(enemies, friendlyPos, radius, CurWarrior->getTeam()->getId(), -1,true, true, true);
		GameObjectPtr bestTarget = NULL;
		if (numEnemies > 0)
			bestTarget = calcBestTarget(CurWarrior->getVehicle(), numFriendlies, friendlies, numEnemies, enemies);
	
		if (bestTarget)
			ABLi_pushInteger(bestTarget->getPartId());
		else
			ABLi_pushInteger(0);
	}
}

//*****************************************************************************

#define	MAX_SHELTERS	10

void execRequestShelter (void) {

	float range = ABLi_popIntegerReal();

	long numValidBuildings = 0;
	BuildingPtr validBuildings[MAX_SHELTERS];
	long numBuildings = ObjectManager->getNumBuildings();
	for (long i = 0; i < numBuildings; i++) {
		BuildingPtr building = ObjectManager->getBuilding(i);
		if (CurObject->distanceFrom(building->getPosition()) < range) {
			validBuildings[numValidBuildings++] = building;
			if (numValidBuildings == MAX_SHELTERS)
				break;
		}
	}
	if (numValidBuildings > 0) {
		long buildingIndex = RandomNumber(numValidBuildings);
		ABLi_pushInteger(validBuildings[buildingIndex]->getPartId());
		}
	else
		ABLi_pushInteger(0);
}

//*****************************************************************************

void execMCPrint (void) {

	ABLStackItem value;
	ABLi_popAnything(&value);
	char s[256];
	switch (value.type) {
		case ABL_STACKITEM_CHAR:
			sprintf(s, "char=%c", value.data.character);
			DEBUGWINS_print(s, 0);
			break;
		case ABL_STACKITEM_INTEGER:
			sprintf(s, "int=%d", value.data.integer);
			DEBUGWINS_print(s, 0);
			break;
		case ABL_STACKITEM_REAL:
			sprintf(s, "real=%.2f", value.data.real);
			DEBUGWINS_print(s, 0);
			break;
		case ABL_STACKITEM_BOOLEAN:
			sprintf(s, "bool=%s", value.data.boolean ? "true" : "false");
			DEBUGWINS_print(s, 0);
			break;
		case ABL_STACKITEM_CHAR_PTR:
			sprintf(s, "char*=%s", (char*)value.data.characterPtr);
			DEBUGWINS_print(s, 0);
			break;
		case ABL_STACKITEM_INTEGER_PTR:
			sprintf(s, "int*=%d,%d,%d", value.data.integerPtr[0], value.data.integerPtr[1], value.data.integerPtr[2]);
			DEBUGWINS_print(s, 0);
			break;
		case ABL_STACKITEM_REAL_PTR:
			sprintf(s, "real*=%.2f,%.2f,%.2f", value.data.realPtr[0], value.data.realPtr[1], value.data.realPtr[2]);
			DEBUGWINS_print(s, 0);
			break;
		case ABL_STACKITEM_BOOLEAN_PTR:
			sprintf(s, "bool*=%d,%d,%d", value.data.booleanPtr[0], value.data.booleanPtr[1], value.data.booleanPtr[2]);
			DEBUGWINS_print(s, 0);
			break;	
	}
}

//*****************************************************************************

void execGetMissionStatus (void) {

	ABLi_pushInteger(mission->getStatus());
}

//*****************************************************************************

void execAddTriggerArea (void) {

	long ULrow = ABLi_popInteger();
	long ULcol = ABLi_popInteger();
	long LRrow = ABLi_popInteger();
	long LRcol = ABLi_popInteger();
	long type = ABLi_popInteger();
	long param = ABLi_popInteger();

	long handle = Mover::triggerAreaMgr->add(ULrow, ULcol, LRrow, LRcol, type, param);
	if (handle == 0)
		Fatal(0, " Too many Trigger Areas ");
	ABLi_pushInteger(handle);
}

//*****************************************************************************

void execIsTriggerAreaHit (void) {

	long handle = ABLi_popInteger();
	ABLi_pushBoolean(Mover::triggerAreaMgr->isHit(handle));
}

//*****************************************************************************

void execResetTriggerArea (void) {

	long handle = ABLi_popInteger();
	Mover::triggerAreaMgr->reset(handle);
}

//*****************************************************************************

void execRemoveTriggerArea (void) {

	long handle = ABLi_popInteger();
	Mover::triggerAreaMgr->remove(handle);
}

//*****************************************************************************

void execGetWeapons (void) {

	int* weaponList = ABLi_popIntegerPtr();
	long infoType = ABLi_popInteger();

	long numWpns = 0;
	if (CurObject->isMover()) {
		MoverPtr mover = (MoverPtr)CurObject;
		switch (infoType) {
			case 0: {
				for (long curWeapon = mover->numOther; curWeapon < (mover->numOther + mover->numWeapons); curWeapon++)
					weaponList[numWpns++] = mover->inventory[curWeapon].masterID;
				}
				break;
			case 1: {
				for (long curWeapon = mover->numOther; curWeapon < (mover->numOther + mover->numWeapons); curWeapon++) {
					if (!mover->inventory[curWeapon].disabled && (mover->getWeaponShots(curWeapon) > 0))
						weaponList[numWpns++] = mover->inventory[curWeapon].masterID;
				}
				#ifdef BUGLOG
				if ((numWpns != mover->numFunctionalWeapons) && BugLog) {
					char s[50];
					sprintf(s, "[%.2f] ablmc2.execGetWeapons: numWpns != numFunctionalWeapons (%05d)%s", scenarioTime, mover->getPartId(), mover->getName());
					BugLog->write(s);
					BugLog->write(" ");
				}
				#endif
				}
				break;
		}
	}
	ABLi_pushInteger(numWpns);
}

//*****************************************************************************

void execGetWeaponsStatus (void) {

	int* weaponList = ABLi_popIntegerPtr();

	long status = 0;
	if (CurObject && CurObject->isMover()) {
		MoverPtr mover = (MoverPtr)CurObject;
		status = mover->getPilot()->getWeaponsStatus(weaponList);
	}
	ABLi_pushInteger(status);
}

//*****************************************************************************

void execSetMoveArea (void) {

	float* center = ABLi_popRealPtr();
	float radius = ABLi_popReal();

	if (CurObject->isMover()) {
		Stuff::Vector3D loc;
		loc.x = center[0];
		loc.y = center[1];
		loc.z =  land->getTerrainElevation(loc);
		((MoverPtr)CurObject)->moveCenter = loc;
		((MoverPtr)CurObject)->moveRadius = radius;
	}
}

//*****************************************************************************

void execClearTacOrder (void) {

	if (CurObject)
		CurObject->getPilot()->clearCurTacOrder();
}

//*****************************************************************************

void execPlayWave (void) {

	char* fileName = ABLi_popCharPtr();
	long type = ABLi_popInteger();

	soundSystem->playSupportSample(-1, fileName);

	ABLi_pushInteger(0);
}

//*****************************************************************************

void execSetWillHelp (void) {

	bool willHelp = ABLi_popBoolean();

	bool wasHelping = CurWarrior->getWillHelp();
	CurWarrior->setWillHelp(willHelp);

	ABLi_pushInteger(wasHelping);
}

//*****************************************************************************

void execGetLastScan (void) {

	GameObjectPtr lastScanTarget = ObjectManager->getByWatchID(CurWarrior->getCoreScanTargetWID());
	long lastScanTargetID = 0;
	if (lastScanTarget)
		lastScanTargetID = lastScanTarget->getPartId();

	ABLi_pushInteger(lastScanTargetID);
}

//*****************************************************************************

void execGetMapInfo (void) {

	int* mapInfo = ABLi_popIntegerPtr();

	mapInfo[0] = GameMap->getHeight();
	mapInfo[1] = GameMap->getWidth();
}

//*****************************************************************************

void execIsOffMap (void) {

	float* worldPos = ABLi_popRealPtr();

	Stuff::Vector3D pos;
	pos.x = worldPos[0];
	pos.y = worldPos[1];
	if (pos.x > 999000) {
		if (CurObject)
			pos = CurObject->getPosition();
		else {
			ABLi_pushBoolean(false);
			return;
		}
	}
	int row, col;
	land->worldToCell(pos, row, col);
	ABLi_pushBoolean(GameMap->getOffMap(row, col) ? true : false);
}

//*****************************************************************************

void execSetGoalPlanning (void) {

	bool setting = ABLi_popBoolean();

	bool oldSetting = false;
	if (CurObject) {
		oldSetting = ((MoverPtr)CurObject)->getPilot()->getUseGoalPlan();
		((MoverPtr)CurObject)->getPilot()->setUseGoalPlan(setting);
		if (oldSetting != setting)
			((MoverPtr)CurObject)->getPilot()->setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
	}
	ABLi_pushBoolean(oldSetting);
}

//*****************************************************************************

void execSetEject (void) {

	bool setting = ABLi_popBoolean();

	bool oldSetting = false;
	if (CurObject) {
		oldSetting = ((MoverPtr)CurObject)->getPilot()->getEscapesThruEjection();
		((MoverPtr)CurObject)->getPilot()->setEscapesThruEjection(setting);
	}
	ABLi_pushBoolean(oldSetting);
}

//*****************************************************************************

void execSetKeepMoving (void) {

	bool setting = ABLi_popBoolean();

	bool oldSetting = false;
	if (CurObject) {
		oldSetting = ((MoverPtr)CurObject)->getPilot()->getKeepMoving();
		((MoverPtr)CurObject)->getPilot()->setKeepMoving(setting);
	}
	ABLi_pushBoolean(oldSetting);
}

//*****************************************************************************

void* ablSystemMallocCallback (unsigned long memSize) {

	return(systemHeap->Malloc(memSize));
}

//-----------------------------------------------------------------------------

void* ablStackMallocCallback (unsigned long memSize) {

	if (!AblSymbolHeap)
		Fatal(0, " ablSymbolMallocCallback: NULL heap ");
	return(AblStackHeap->Malloc(memSize));
}

//-----------------------------------------------------------------------------

void* ablCodeMallocCallback (unsigned long memSize) {

	if (!AblCodeHeap)
		Fatal(0, " ablCodeFreeCallback: NULL heap ");
	return(AblCodeHeap->Malloc(memSize));
}

//-----------------------------------------------------------------------------

void* ablSymbolMallocCallback (unsigned long memSize) {

	if (!AblSymbolHeap)
		Fatal(0, " ablSymbolMallocCallback: NULL heap ");
	return(AblSymbolHeap->Malloc(memSize));
}

//-----------------------------------------------------------------------------

void ablSystemFreeCallback (void* memBlock) {

	systemHeap->Free(memBlock);
}

//-----------------------------------------------------------------------------

void ablStackFreeCallback (void* memBlock) {

	if (!AblStackHeap)
		Fatal(0, " ablStackFreeCallback: NULL heap ");
	AblStackHeap->Free(memBlock);
}

//-----------------------------------------------------------------------------

void ablCodeFreeCallback (void* memBlock) {

	if (!AblCodeHeap)
		Fatal(0, " ablCodeFreeCallback: NULL heap ");
	AblCodeHeap->Free(memBlock);
}

//-----------------------------------------------------------------------------

void ablSymbolFreeCallback (void* memBlock) {

	if (!AblSymbolHeap)
		Fatal(0, " ablSymbolFreeCallback: NULL heap ");
	AblSymbolHeap->Free(memBlock);
}

//*****************************************************************************

long ablFileCreateCB (void** file, const char* fName) {

	*file = new File;
	if (*file == NULL)
		Fatal(0, " unable to create ABL file");
	if (((FilePtr)*file)->create(fName) != NO_ERR) {
		char s[256];
		sprintf(s, " ABL.ablFileOpenCB: unable to create file [%s] ", fName);
		Fatal(0, s);
	}
	return(NO_ERR);
}

//-----------------------------------------------------------------------------

long ablFileOpenCB (void** file, const char* fName) {

	*file = new File;
	if (*file == NULL)
		Fatal(0, " unable to create ABL file");

	//Filenames MUST be all lowercase or Hash won't find 'em!
    char* tmpstr = strdup(fName);
	CharLower(tmpstr);
	if (((FilePtr)*file)->open(tmpstr) != NO_ERR)
		STOP((" unable to open ABL File %s",fName));
    free(tmpstr);
	return(NO_ERR);
}

//-----------------------------------------------------------------------------

long ablFileCloseCB (void** file) {

	((FilePtr)*file)->close();


	delete (FilePtr)*file;
	*file = NULL;

	return(0);
}

//-----------------------------------------------------------------------------

bool ablFileEofCB (void* file) {

	return(((FilePtr)file)->eof());

}

//-----------------------------------------------------------------------------

long ablFileReadCB (void* file, unsigned char* buffer, long length) {

	return(((FilePtr)file)->read(buffer, length));
}

//-----------------------------------------------------------------------------

int32_t ablFileReadIntCB (void* file) {

	return(((FilePtr)file)->readInt());
}

//-----------------------------------------------------------------------------

long ablFileReadLongCB (void* file) {

	return(((FilePtr)file)->readLong());
}

//-----------------------------------------------------------------------------

long ablFileReadStringCB (void* file, unsigned char* buffer) {

	return(((FilePtr)file)->readString(buffer));
}

//-----------------------------------------------------------------------------

long ablFileReadLineExCB (void* file, unsigned char* buffer, long maxLength) {

	return(((FilePtr)file)->readLineEx(buffer, maxLength));
}

//-----------------------------------------------------------------------------

long ablFileWriteCB (void* file, unsigned char* buffer, long length) {

	return(((FilePtr)file)->write(buffer, length));
}

//-----------------------------------------------------------------------------

long ablFileWriteByteCB (void* file, unsigned char byte) {

	return(((FilePtr)file)->writeByte(byte));
}

//-----------------------------------------------------------------------------

long ablFileWriteIntCB (void* file, int32_t value) {

	return(((FilePtr)file)->writeInt(value));
}

//-----------------------------------------------------------------------------

long ablFileWriteLongCB (void* file, long value) {

	return(((FilePtr)file)->writeLong(value));
}

//-----------------------------------------------------------------------------

long ablFileWriteStringCB (void* file, const char* buffer) {

	return(((FilePtr)file)->writeString(buffer));
}

//*****************************************************************************

void ablDebuggerPrintCallback (const char* s) {

	//ABLDebuggerOut->print(s);
	char msg[1024];
	sprintf(msg, "%s\n", s);
	SPEW((0,msg));
}

//*****************************************************************************

void ablDebugPrintCallback (const char* s) {

	DEBUGWINS_print(s, 0);
}

//*****************************************************************************

void ablSeedRandom (unsigned long seed) {

	gos_srand(seed);
}

//*****************************************************************************

void ablFatalCallback (long code, const char* s) {

	STOP((s));
}

//*****************************************************************************

extern GameObjectPtr LastCoreAttackTarget;

void ablEndlessStateCallback (UserFile* log) {

	char s[256];
	sprintf(s, "Mover = %s (%d)", CurWarrior->getVehicle()->getName(), CurWarrior->getVehicle()->getPartId());
	log->write(s);
	sprintf(s, "     Status = %d", CurWarrior->getVehicle()->getStatus());
	log->write(s);
	sprintf(s, "     NumWeapons = %d", CurWarrior->getVehicle()->numWeapons);
	log->write(s);
	sprintf(s, "     NumFunctionalWeapons = %d", CurWarrior->getVehicle()->numFunctionalWeapons);
	log->write(s);
	sprintf(s, "     TacOrder = ");
	CurWarrior->getCurTacOrder()->debugString(CurWarrior, &s[16]);
	log->write(s);
	log->write(" ");
	if (LastCoreAttackTarget) {
		sprintf(s, "Target = %s (%d)", LastCoreAttackTarget->getName(), LastCoreAttackTarget->getPartId());
		log->write(s);
		Stuff::Vector3D pos;
		pos = LastCoreAttackTarget->getPosition();
		sprintf(s, "Target Position = (%.3f, %.3f, %.3f) [%d, %d]",
			pos.x, pos.y, pos.z,
			LastCoreAttackTarget->cellPositionRow,
			LastCoreAttackTarget->cellPositionCol);
		log->write(s);
		sprintf(s, "     Status = %d", LastCoreAttackTarget->getStatus());
		log->write(s);
		if (LastCoreAttackTarget->isMover()) {
			MoverPtr mover = (MoverPtr)LastCoreAttackTarget;
			sprintf(s, "     NumWeapons = %d", mover->numWeapons);
			log->write(s);
			sprintf(s, "     NumFunctionalWeapons = %d", mover->numFunctionalWeapons);
			log->write(s);
			sprintf(s, "     TacOrder = ");
			mover->getPilot()->getCurTacOrder()->debugString(CurWarrior, &s[16]);
			log->write(s);
		}
		}
	else {
		sprintf(s, "Target = NULL");
		log->write(s);
	}
}

//*****************************************************************************

void initABL (void) {

	AblSymbolHeap = new UserHeap;
	long heapErr = AblSymbolHeap->init(767999);
	if (heapErr != NO_ERR)
		ABL_Fatal(0, "ABLi_init: unable to create ABL symbol table heap");

	AblStackHeap = new UserHeap;
	heapErr = AblStackHeap->init(511999);
	if (heapErr != NO_ERR)
		ABL_Fatal(0, "ABLi_init: unable to create ABL stack heap");

	AblCodeHeap = new UserHeap;
	heapErr = AblCodeHeap->init(307199);
	if (heapErr != NO_ERR)
		ABL_Fatal(0, "ABLi_init: unable to create ABL code heap");

	ABLi_init(20479, //AblRunTimeStackSize,
			  102400, //AblMaxCodeBlockSize,
			  200, //AblMaxRegisteredModules,
			  100, //AblMaxStaticVariables,
			  ablSystemMallocCallback,
			  ablStackMallocCallback,
			  ablCodeMallocCallback,
			  ablSymbolMallocCallback,
			  ablSystemFreeCallback,
			  ablStackFreeCallback,
			  ablCodeFreeCallback,
			  ablSymbolFreeCallback,
			  ablFileCreateCB,
			  ablFileOpenCB,
			  ablFileCloseCB,
			  ablFileEofCB,
			  ablFileReadCB,
			  ablFileReadIntCB,
			  ablFileReadLongCB,
			  ablFileReadStringCB,
			  ablFileReadLineExCB,
			  ablFileWriteCB,
			  ablFileWriteByteCB,
			  ablFileWriteIntCB,
			  ablFileWriteLongCB,
			  ablFileWriteStringCB,
			  ablDebuggerPrintCallback,
			  ablFatalCallback,
			  true,
			  false);

	ABLi_setDebugPrintCallback(ablDebugPrintCallback);
	ABLi_setRandomCallbacks(ablSeedRandom, RandomNumber);
	ABLi_setEndlessStateCallback(ablEndlessStateCallback);

	ABLi_addFunction("getid", false, NULL, "i", execGetId);
	ABLi_addFunction("gettime", false, NULL, "r", execGetTime);
	ABLi_addFunction("gettimeleft", false, NULL, "r", execGetTimeLeft);
	ABLi_addFunction("selectobject", false, "i", "i", execSelectObject);
	//ABLi_addFunction("selectunit", false, "i", "i", execSelectUnit);
	ABLi_addFunction("selectwarrior", false, "i", "i", execSelectWarrior);
	ABLi_addFunction("getwarriorstatus", false, "i", "i", execGetWarriorStatus);
	ABLi_addFunction("getcontacts", false, "Iii", "i", execGetContacts);
	ABLi_addFunction("getenemycount", false, "i", "i", execGetEnemyCount);
	ABLi_addFunction("selectcontact", false, "ii", "i", execSelectContact);
	ABLi_addFunction("getcontactid", false, NULL, "i", execGetContactId);
	ABLi_addFunction("iscontact", false, "iii", "i", execIsContact);
	ABLi_addFunction("getcontactstatus", false, "I", "i", execGetContactStatus);
	ABLi_addFunction("getcontactrelativeposition", false, "rr", "i", execGetContactRelativePosition);
	ABLi_addFunction("settarget", false, "ii", NULL, execSetTarget);
	ABLi_addFunction("gettarget", false, "i", "i", execGetTarget);
	ABLi_addFunction("getweaponsready", false, "Ii", "i", execGetWeaponsReady);
	ABLi_addFunction("getweaponslocked", false, "Ii", "i", execGetWeaponsLocked);
	ABLi_addFunction("getweaponsinrange", false, "Ii", "i", execGetWeaponsInRange);
	ABLi_addFunction("getweaponshots", false, "i", "i", execGetWeaponShots);
	ABLi_addFunction("getweaponranges", false, "iR", NULL, execGetWeaponRanges);
	ABLi_addFunction("getobjectposition", false, "iR", NULL, execGetObjectPosition);
	ABLi_addFunction("getintegermemory", false, "i", "i", execGetIntegerMemory);
	ABLi_addFunction("getrealmemory", false, "i", "r", execGetRealMemory);
	ABLi_addFunction("getalarmtriggers", false, "I", "i", execGetAlarmTriggers);
	ABLi_addFunction("getchallenger", false, "i", "i", execGetChallenger);
	ABLi_addFunction("gettimewithoutorders", false, NULL, "r", execGetTimeWithoutOrders);
	ABLi_addFunction("getfireranges", false, "R", NULL, execGetFireRanges);
	ABLi_addFunction("getattackers", false, "Ir", "i", execGetAttackers);
	ABLi_addFunction("getattackerinfo", false, "i", "r", execGetAttackerInfo);
	ABLi_addFunction("setchallenger", false, "ii", "i", execSetChallenger);
	ABLi_addFunction("setintegermemory", false, "ii", NULL, execSetIntegerMemory);
	ABLi_addFunction("setrealmemory", false, "ir", NULL, execSetRealMemory);
	ABLi_addFunction("hasmovegoal", false, NULL, "b", execHasMoveGoal);
	ABLi_addFunction("hasmovepath", false, NULL, "b", execHasMovePath);
	ABLi_addFunction("sortweapons", false, "Ii", "i", execSortWeapons);
	ABLi_addFunction("getvisualrange", false, "i", "r", execGetVisualRange);
	ABLi_addFunction("getunitmates", false, "iI", "i", execGetUnitMates);
	ABLi_addFunction("gettacorder", false, "irI", "i", execGetTacOrder);
	ABLi_addFunction("getlasttacorder", false, "irI", "i", execGetLastTacOrder);
	ABLi_addFunction("getobjects", false, "iI", "i", execGetObjects);
	ABLi_addFunction("orderwait", false, "rb", "i", execOrderWait);
	ABLi_addFunction("ordermoveto", false, "Rb", "i", execOrderMoveTo);
	ABLi_addFunction("ordermovetoobject", false, "ib", "i", execOrderMoveToObject);
	ABLi_addFunction("ordermovetocontact", false, "b", "i", execOrderMoveToContact);
	ABLi_addFunction("orderpowerdown", false, NULL, "i", execOrderPowerDown);
	ABLi_addFunction("orderpowerup", false, NULL, "i", execOrderPowerUp);
	ABLi_addFunction("orderattackobject", false, "iiiib", "i", execOrderAttackObject);
	ABLi_addFunction("orderattackcontact", false, "iiib", "i", execOrderAttackContact);
	ABLi_addFunction("orderwithdraw", false, NULL, "i", execOrderWithdraw);
	ABLi_addFunction("objectinwithdrawal", false, "i", "i", execObjectInWithdrawal);
	ABLi_addFunction("damageobject", false, "iiirirr", "i", execDamageObject);
	ABLi_addFunction("setattackradius", false, "r", "r", execSetAttackRadius);
	ABLi_addFunction("objectchangesides", false, "ii", NULL, execObjectChangeSides);
	ABLi_addFunction("distancetoobject", false, "ii", "r", execDistanceToObject);
	ABLi_addFunction("distancetoposition", false, "iR", "r", execDistanceToPosition);
	ABLi_addFunction("objectsuicide", false, "i", NULL, execObjectSuicide);
	ABLi_addFunction("objectcreate", false, "i", "i", execObjectCreate);
	ABLi_addFunction("objectexists", false, "i", "i", execObjectExists);
	ABLi_addFunction("objectstatus", false, "i", "i", execObjectStatus);
	ABLi_addFunction("objectstatuscount", false, "iI", NULL, execObjectStatusCount);
	ABLi_addFunction("objectvisible", false, "ii", "i", execObjectVisible);
	ABLi_addFunction("objectside", false, "i", "i", execObjectTeam);
	ABLi_addFunction("objectcommander", false, "i", "i", execObjectCommander);
	ABLi_addFunction("objectclass", false, "i", "i", execObjectClass);
	ABLi_addFunction("settimer", false, "i*", "i", execSetTimer);
	ABLi_addFunction("checktimer", false, "i", "r", execCheckTimer);
	ABLi_addFunction("endtimer", false, "i", NULL, execEndTimer);
//	ABLi_addFunction("setobjectivetimer", false, "i*", "i", execSetObjectiveTimer);
//	ABLi_addFunction("checkobjectivetimer", false, "i", "r", execCheckObjectiveTimer);
	ABLi_addFunction("setobjectivestatus", false, "ii", "i", execSetObjectiveStatus);
	ABLi_addFunction("checkobjectivestatus", false, "i", "i", execCheckObjectiveStatus);
//	ABLi_addFunction("setobjectivetype", false, "ii", "i", execSetObjectiveType);
//	ABLi_addFunction("checkobjectivetype", false, "i", "i", execCheckObjectiveType);
	ABLi_addFunction("playdigitalmusic", false, "i", "i", execPlayDigitalMusic);
	ABLi_addFunction("stopmusic", false, NULL, "i", execStopMusic);
	ABLi_addFunction("playsoundeffect", false, "i", "i", execPlaySoundEffect);
	ABLi_addFunction("playvideo", false, "C", "i", execPlayVideo);
	ABLi_addFunction("setradio", false, "ib", "i", execSetRadio);
	ABLi_addFunction("playspeech", false, "ii", "i", execPlaySpeech);
	ABLi_addFunction("playbetty", false, "i", "i", execPlayBetty);
	ABLi_addFunction("setobjectactive", false, "ib", "i", execSetObjectActive);
	ABLi_addFunction("objecttypeid", false, "i", "i", execObjectTypeID);
	ABLi_addFunction("getterrainobjectpartid", false, "ii", "i", execGetTerrainObjectPartID);
	ABLi_addFunction("objectremove", false, "i", "i", execObjectRemove);
	ABLi_addFunction("inarea", false, "iRri", "b", execInArea);
	ABLi_addFunction("createinfantry", false, "Ri", "i", execCreateInfantry);
	ABLi_addFunction("getsensorsworking", false, "i", "i", execGetSensorsWorking);
	ABLi_addFunction("getcurrentbrvalue", false, "i", "i", execGetCurrentBRValue);
	ABLi_addFunction("setcurrentbrvalue", false, "ii", NULL, execSetCurrentBRValue);
	ABLi_addFunction("getarmorpts", false, "i", "i", execGetArmorPts);
	ABLi_addFunction("getmaxarmor", false, "i", "i", execGetMaxArmor);
	ABLi_addFunction("getpilotid", false, "i", "i", execGetPilotID);
	ABLi_addFunction("getpilotwounds", false, "i", "r", execGetPilotWounds);
	ABLi_addFunction("setpilotwounds", false, "ii", NULL, execSetPilotWounds);
	ABLi_addFunction("getobjectactive", false, "i", "i", execGetObjectActive);
	ABLi_addFunction("getobjectdamage", false, "i", "i", execGetObjectDamage);
	ABLi_addFunction("getobjectdmgpts", false, "i", "i", execGetObjectDmgPts);
	ABLi_addFunction("getobjectmaxdmg", false, "i", "i", execGetObjectMaxDmg);
	ABLi_addFunction("setobjectdamage", false, "ii", "i", execSetObjectDamage);
	ABLi_addFunction("getglobalvalue", false, "i", "r", execGetGlobalValue);
	ABLi_addFunction("setglobalvalue", false, "i*", NULL, execSetGlobalValue);
	ABLi_addFunction("setobjectivepos", false, "i***", NULL, execSetObjectivePos);
	ABLi_addFunction("setsensorrange", false, "ir", NULL, execSetSensorRange);
	ABLi_addFunction("settonnage", false, "ir", NULL, execSetTonnage);
	ABLi_addFunction("setexplosiondamage", false, "ir", NULL, execSetExplosionDamage);
	ABLi_addFunction("setexplosionradius", false, "ir", NULL, execSetExplosionRadius);
	ABLi_addFunction("setsalvage", false, "iii", "b", execSetSalvage);
	ABLi_addFunction("setsalvagestatus", false, "ib", "b", execSetSalvageStatus);
	ABLi_addFunction("setanimation", false, "iii", NULL, execSetAnimation);
	ABLi_addFunction("setrevealed", false, "i*R", NULL, execSetRevealed);
	ABLi_addFunction("getsalvage", false, "iiII", NULL, execGetSalvage);
	ABLi_addFunction("orderrefit", false, "ii", NULL, execOrderRefit);
	ABLi_addFunction("setcaptured", false, "i", NULL, execSetCaptured);
	ABLi_addFunction("ordercapture", false, "ii", NULL, execOrderCapture);
	ABLi_addFunction("setcapturable", false, "ib", NULL, execSetCapturable);
	ABLi_addFunction("iscaptured", false, "i", "i", execIsCaptured);
	ABLi_addFunction("iscapturable", false, "ii", "b", execIsCapturable);
	ABLi_addFunction("wasevercapturable", false, "i", "b", execWasEverCapturable);
	ABLi_addFunction("setbuildingname", false, "ii", NULL, execSetBuildingName);
	ABLi_addFunction("callstrike", false, "iirrrb", NULL, execCallStrike);
	ABLi_addFunction("callstrikeex", false, "iirrrbr", NULL, execCallStrikeEx);
	ABLi_addFunction("orderloadelementals", false, "i", NULL, execOrderLoadElementals);
	ABLi_addFunction("orderdeployelementals", false, "i", NULL, execOrderDeployElementals);
	ABLi_addFunction("addprisoner", false, "ii", "i", execAddPrisoner);
	ABLi_addFunction("lockgateopen", false, "i", NULL, execLockGateOpen);
	ABLi_addFunction("lockgateclosed", false, "i", NULL, execLockGateClosed);
	ABLi_addFunction("releasegatelock", false, "i", NULL, execReleaseGateLock);
	ABLi_addFunction("isgateopen", false, "i", "b", execIsGateOpen);
	ABLi_addFunction("getrelativepositiontopoint", false, "RrriR", NULL, execGetRelativePositionToPoint);
	ABLi_addFunction("getrelativepositiontoobject", false, "irriR", NULL, execGetRelativePositionToObject);
	ABLi_addFunction("getunitstatus", false, "i", "r", execGetUnitStatus);
	ABLi_addFunction("repair", false, "ir", NULL, execRepair);
	ABLi_addFunction("getfixed", false, "iii", "i", execGetFixed);
	ABLi_addFunction("getrepairstate", false, "i", "i", execGetRepairState);
	ABLi_addFunction("isteamtargeting", false, "iii", "b", execIsTeamTargeting);
	ABLi_addFunction("isteamcapturing", false, "iii", "b", execIsTeamCapturing);
	ABLi_addFunction("sendmessage", false, "ii", NULL, execSendMessage);
	ABLi_addFunction("getmessage", false, "i", "i", execGetMessage);
	ABLi_addFunction("gethometeam", false, NULL, "i", execGetHomeTeam);
//	ABLi_addFunction("getstrikes", false, "ii", "i", execGetStrikes);
//	ABLi_addFunction("setstrikes", false, "iii", NULL, execSetStrikes);
//	ABLi_addFunction("addstrikes", false, "iii", NULL, execAddStrikes);
	ABLi_addFunction("isserver", false, NULL, "b", execIsServer);
	ABLi_addFunction("calcpartid", false, "iiii", "i", execCalcPartID);
	ABLi_addFunction("setdebugstring", false, "iiC", NULL, execSetDebugString);
	ABLi_addFunction("break", false, NULL, NULL, execBreak);
	ABLi_addFunction("pathexists", false, "iiiii", "i", execPathExists);
	ABLi_addFunction("convertcoords", false, "iRI", "i", execConvertCoords);
	ABLi_addFunction("newmoveto", true, "Ri", "i", execCoreMoveTo);
	ABLi_addFunction("newmovetoobject", true, "ii", "i", execCoreMoveToObject);
	ABLi_addFunction("newpower", true, "b", "i", execCorePower);
	ABLi_addFunction("newattack", true, "ii", "i", execCoreAttack);
	ABLi_addFunction("newcapture", true, "ii", "i", execCoreCapture);
	ABLi_addFunction("newscan", true, "ii", "i", execCoreScan);
	ABLi_addFunction("newcontrol", true, "ii", "i", execCoreControl);
	ABLi_addFunction("coremoveto", true, "Ri", "i", execCoreMoveTo);
	ABLi_addFunction("coremovetoobject", true, "ii", "i", execCoreMoveToObject);
	ABLi_addFunction("corepower", true, "b", "i", execCorePower);
	ABLi_addFunction("coreattack", true, "ii", "i", execCoreAttack);
	ABLi_addFunction("corecapture", true, "ii", "i", execCoreCapture);
	ABLi_addFunction("corescan", true, "ii", "i", execCoreScan);
	ABLi_addFunction("corecontrol", true, "ii", "i", execCoreControl);
	ABLi_addFunction("coreeject", true, NULL, "i", execCoreEject);
	ABLi_addFunction("setpilotstate", false, "i", "i", execSetPilotState);
	ABLi_addFunction("getpilotstate", false, NULL, "i", execGetPilotState);
	ABLi_addFunction("getnextpilotevent", false, "I", "i", execGetNextPilotEvent);
	ABLi_addFunction("settargetpriority", false, "iiiii", "i", execSetTargetPriority);
	ABLi_addFunction("setdebugwindow", false, "ii", "i", execSetDebugWindow);
	
	ABLi_addFunction("setmoviemode", false, NULL, NULL, execSetMovieMode);
	ABLi_addFunction("endmoviemode", false, NULL, NULL, execEndMovieMode);
	ABLi_addFunction("fadetocolor", false, "ir", NULL, execFadeToColor);
	ABLi_addFunction("forcemovieend", false, NULL, "i", execForceMovieEnd);
	
	ABLi_addFunction("getcameraposition", false, "R", NULL, execGetCameraPosition);
	ABLi_addFunction("setcameraposition", false, "R", NULL, execSetCameraPosition);
	ABLi_addFunction("setcameragoalposition", false, "Rr", NULL, execSetCameraGoalPosition);
	ABLi_addFunction("getcameragoalposition", false, "R", NULL, execGetCameraGoalPosition);
	ABLi_addFunction("getcamerarotation", false, "R", NULL, execGetCameraRotation);
	ABLi_addFunction("setcamerarotation", false, "R", NULL, execSetCameraRotation);
	ABLi_addFunction("setcameragoalrotation", false, "Rr", NULL, execSetCameraGoalRotation);
	ABLi_addFunction("getcameragoalrotation", false, "R", NULL, execGetCameraGoalRotation);
	ABLi_addFunction("getcamerazoom", false, NULL, "r", execGetCameraZoom);
	ABLi_addFunction("setcamerazoom", false, "r", NULL, execSetCameraZoom);
	ABLi_addFunction("getcameragoalzoom", false, NULL, "r", execGetCameraGoalZoom);
	ABLi_addFunction("setcameragoalzoom", false, "rr", NULL, execSetCameraGoalZoom);
	ABLi_addFunction("setcameravelocity", false, "R", NULL, execSetCameraVelocity);
	ABLi_addFunction("getcameravelocity", false, "R", NULL, execGetCameraVelocity);
	ABLi_addFunction("setcameragoalvelocity", false, "Rr", NULL, execSetCameraGoalVelocity);
	ABLi_addFunction("getcameragoalvelocity", false, "R", NULL, execGetCameraGoalVelocity);
	ABLi_addFunction("setcameralookobject", false, "i", NULL, execSetCameraLookObject);
	ABLi_addFunction("getcameralookobject", false, NULL, "i", execGetCameraLookObject);
	ABLi_addFunction("getcameraframelength", false, NULL, "r", execGetCameraFrameLength);
	
	ABLi_addFunction("getmissionwon", false, NULL, "b", execGetMissionWon);
	ABLi_addFunction("getmissionlost", false, NULL, "b", execGetMissionLost);
	ABLi_addFunction("getobjectivesuccess", false, NULL, "b", execGetObjectiveSuccess);
	ABLi_addFunction("getobjectivefailed", false, NULL, "b", execGetObjectiveFailed);
	ABLi_addFunction("getenemydestroyed", false, NULL, "b", execGetEnemyDestroyed);
	ABLi_addFunction("getfriendlydestroyed", false, NULL, "b", execGetFriendlyDestroyed);
	ABLi_addFunction("getplayerincombat", false, NULL, "b", execPlayerInCombat);
	ABLi_addFunction("getsensorsactive", false, NULL, "b", execGetSensorsActive);
	ABLi_addFunction("getcurrentmusicid", false, NULL, "i", execGetCurrentMusicId);
	ABLi_addFunction("getmissiontune", false, NULL, "i", execGetMissionTuneId);
	
	ABLi_addFunction("requesthelp", false, "iRrRri", "r", execRequestHelp);
	ABLi_addFunction("requesttarget", false, "Rr", "i", execRequestTarget);
	ABLi_addFunction("requestshelter", false, "*", "i", execRequestShelter);
	ABLi_addFunction("mcprint", false, "?", NULL, execMCPrint);

	ABLi_addFunction("getmissionstatus", false, NULL, "i", execGetMissionStatus);
	ABLi_addFunction("addtriggerarea", false, "iiiiii", "i", execAddTriggerArea);
	ABLi_addFunction("istriggerareahit", false, "i", "b", execIsTriggerAreaHit);
	ABLi_addFunction("resettriggerarea", false, "i", NULL, execResetTriggerArea);
	ABLi_addFunction("removetriggerarea", false, "i", NULL, execRemoveTriggerArea);
	ABLi_addFunction("getweapons", false, "Ii", "i", execGetWeapons);
	ABLi_addFunction("setmovearea", false, "Rr", NULL, execSetMoveArea);
	ABLi_addFunction("getweaponsstatus", false, "I", "i", execGetWeaponsStatus);
	ABLi_addFunction("cleartacorder", false, NULL, NULL, execClearTacOrder);
	ABLi_addFunction("playwave", false, "Ci", "i", execPlayWave);
	ABLi_addFunction("objectteam", false, "i", "i", execObjectTeam);
	ABLi_addFunction("setwillhelp", false, "b", "b", execSetWillHelp);
	ABLi_addFunction("getlastscan", false, NULL, "i", execGetLastScan);
	ABLi_addFunction("getmapinfo", false, "I", NULL, execGetMapInfo);
	
	ABLi_addFunction("getgeneralalarm", false, NULL, "i", execGetGeneralAlarm);
	ABLi_addFunction("setgeneralalarm", false, "i", NULL, execSetGeneralAlarm);

	ABLi_addFunction("isoffmap", false, "R", "b", execIsOffMap);
	ABLi_addFunction("setgoalplanning", false, "b", "b", execSetGoalPlanning);

	ABLi_addFunction("seteject", false, "b", "b", execSetEject);
	ABLi_addFunction("setkeepmoving", false, "b", "b", execSetKeepMoving);

	//Tutorial Functions
	ABLi_addFunction("animationcallout", false, "ibbri", "b", execAnimationCallout);
	ABLi_addFunction("tutorialtext", false, "i", NULL, execTutorialText);
	ABLi_addFunction("guiisaoe", false, NULL, "b", execGUIIsAOEStyle);
	ABLi_addFunction("logisticsscreenid", false, NULL, "i", execLogisticsScreenId);
	ABLi_addFunction("logisticsanimationcallout", false, "ibri", "b", execLogisticsAnimationCallout);
	ABLi_addFunction("logisticsincallout", false, NULL, "b", execLogisticsInCallout);
	ABLi_addFunction("isplayingvoiceover", false, NULL, "b", execIsPlayingVoiceOver);
	ABLi_addFunction("getlogisticstime", false, NULL, "r", execGetLogisticsTime);
	ABLi_addFunction("stopvoiceover", false, NULL, NULL, execStopVoiceOver);
	ABLi_addFunction("incallout", false, NULL, "b", execInCallout);
	ABLi_addFunction("setinvulnerable", false, "b", NULL, execSetInvulnerable);
	ABLi_addFunction("freezegui", false, "b", NULL, execFreezeGUI);
	
	//static long Godzilla = 120;
	//static long GodzillaList[5] = {10, 20, 30, 40, 50};
	//ABLi_registerInteger("godzilla", &Godzilla);
	//ABLi_registerInteger("godzillalist", &GodzillaList, 5);
}

//*****************************************************************************

void closeABL (void) {

	ABLi_close();

	if (AblSymbolHeap) {
		delete AblSymbolHeap;
		AblSymbolHeap = NULL;
	}
	if (AblStackHeap) {
		delete AblStackHeap;
		AblStackHeap = NULL;
	}
	if (AblCodeHeap) {
		delete AblCodeHeap;
		AblCodeHeap = NULL;
	}
}

//*****************************************************************************


