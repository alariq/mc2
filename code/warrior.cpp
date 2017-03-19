//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//***************************************************************************
//
//								WARRIOR.CPP
//
//***************************************************************************

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef GROUP_H
#include"group.h"
#endif

#ifndef GAMELOG_H
#include"gamelog.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

#ifndef MOVEMGR_H
#include"movemgr.h"
#endif

#ifdef USE_MECHS
#ifndef MECH_H
#include"mech.h"
#endif
#endif

#ifndef GVEHICL_H
#include"gvehicl.h"
#endif

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifndef GATE_H
#include"gate.h"
#endif

#ifndef TURRET_H
#include"turret.h"
#endif

#ifndef TACORDR_H
#include"tacordr.h"
#endif

#ifndef RADIO_H
#include"radio.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef GOAL_H
#include"goal.h"
#endif

#ifndef TURRET_H
#include"turret.h"
#endif

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifdef USE_ELEMENTALS
#ifndef ELEMNTL_H
#include"elemntl.h"
#endif
#endif

#define	TESTING_WITH_PLAYER	1

#include"platform_windows.h"

enum {
	T_A = 0,
	T = T_A,
	T_B,
	U,
	V,
};

//***************************************************************************

#define	MAXLEN_INPUTSTR			512
#define CELL_DISTANCE			42.6667f

//***************************************************************************
typedef enum {
	MECH_BODY_LOCATION_ANY = -1,
	MECH_BODY_LOCATION_HEAD,
	MECH_BODY_LOCATION_CTORSO,
	MECH_BODY_LOCATION_LTORSO,
	MECH_BODY_LOCATION_RTORSO,
	MECH_BODY_LOCATION_LARM,
	MECH_BODY_LOCATION_RARM,
	MECH_BODY_LOCATION_LLEG,
	MECH_BODY_LOCATION_RLEG,
	NUM_MECH_BODY_LOCATIONS
} MechBodyLocationType;

BldgAppearance*		MechWarrior::wayPointMarkers[3] = 
{	0,0,0 };
GoalManager* MechWarrior::goalManager = NULL;

extern float scenarioTime;
#ifdef USE_SCENARIO
extern turn;
#endif

float FireOddsTable[NUM_FIREODDS] = {
	20.0,
	35.0,
	50.0,
	65.0,
	80.0
};

const char* pilotAlarmFunctionName[NUM_PILOT_ALARMS] = {
	"handletargetofweaponfire",
	"handlehitbyweaponfire",
	"handledamagetakenrate",
	"handledeathofmate",
	"handlecripplingoffriendlyvehicle",
	"handledestructionoffriendlyvehicle",
	"handleincapacitationofvehicle",
	"handledestructionofvehicle",
	"handlewithdraw",
	"handleattackorder",
	"handlecollision",
	"handleguardbreach",
	"handlekilledtarget",
	"handlematefiredweapon",
	"handleplayerorder",
	"handlenomovepath",
	"handlegateclosing",
	"handlefiredweapon",
	"handlenewmover"
};

// sebi: those are not used, only filled in mover.cpp
/*
const char ProfessionalismOffsetTable[NUM_OFFSET_RANGES][2] = {
	{10, 10},
	{20, 5},
	{30, 0},
	{40, 5},
	{100, -10}
};

const char DecorumOffsetTable[NUM_OFFSET_RANGES][2] = {
	{10, 10},
	{20, 5},
	{30, 0},
	{40, 5},
	{100, -10}
};

const char AmmoConservationModifiers[2][2] = {
	{50, -5},
	{20, -10}
};
*/

static const char* SkillsTable[NUM_SKILLS] = {
	"Piloting",
	"Sensors",
	"Gunnery"
};


//-----------------------------------------------
// First column is ThreatRating, second column is
// for Inner Sphere, third column is Clan...
float ThreatRatingEffect[8][3] = {
	{0.5f, -1.0f, 0.0f},
	{0.75f, -0.2f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{1.25f, 0.0f, 0.0f},
	{1.5f, 0.0f, 0.0f},
	{2.0f, 0.1f, 0.0f},
	{2.25f, 0.2f, -0.1f},
	{1000000.0f, 0.2f, -1.0f}
};

float BrainUpdateFrequency = 2.25;
float MovementUpdateFrequency = 5.0;
float CombatUpdateFrequency = 0.25;
float CommandUpdateFrequency = 6.0;
float PilotCheckUpdateFrequency = 1.0;
float ThreatAnalysisFrequency = 5.0;

long PilotCheckModifierTable[2] = {
	25,
	25
};

float SkillWeightings[NUM_SKILLS] =
{
	1.0,
	1.0,
	1.0
};

float WarriorRankScale[NUM_WARRIOR_RANKS] = {
	50.0,
	60.0,
	70.0,
	79.0,
	999.0
};



const char *SpecialtySkillsTable[NUM_SPECIALTY_SKILLS] = {
	"LightMechSpecialist",
	"LaserSpecialist",
	"LightACSpecialist",
	"MediumACSpecialist",
	"SRMSpecialist",
	"SmallArmsSpecialist",
	"SensorProfileSpecialist",
	"ToughnessSpecialist",			//Thoughness Specialty

	"MediumMechSpecialist",
	"PulseLaserSpecialist",
	"ERLaserSpecialist",
	"LRMSpecialist",
	"Scout",						//Scouting Specialty
	"LongJump",						//Jump Jet Specialty

	"HevayMechSpecialist",			//Heavy mech Specialty
	"PPCSpecialist",
	"HeavyACSpecialist",
	"ShortRangeSpecialist",
	"MediumRangeSpecialist",
	"LongRangeSpecialist",

	"AssaultMechSpecialist",
	"GaussSpecialist",
	"SharpShooter",					//Sharpshooter specialty
};


SortListPtr		MechWarrior::sortList = NULL;
long			MechWarrior::numWarriors = 0;
long			MechWarrior::numWarriorsInCombat = 0;
bool			MechWarrior::brainsEnabled[MAX_TEAMS] = {true, true, true, true, true, true, true, true};
float			MechWarrior::minSkill = 1;
float			MechWarrior::maxSkill = 100;
long			MechWarrior::increaseCap = 100;
float			MechWarrior::maxVisualRadius = 100.0;
long			MechWarrior::curEventID = 0;
long			MechWarrior::curEventTrigger = 0;
MechWarrior*	MechWarrior::warriorList[MAX_WARRIORS];

long LastMoveCalcErr = 0;
long GroupMoveTrailLen[2] = {0, 1};
float MoveTimeOut = 30.0;
float MoveYieldTime = 1.5;
float DefaultAttackRadius = 275.0;

QueuedTacOrder TacOrderQueue[MAX_QUEUED_TACORDERS];
long TacOrderQueuePos = 0;

extern long	SimpleMovePathRange;
extern long AttitudeEffectOnMovePath[NUM_ATTITUDES][3];
extern char AttitudeEffect[NUM_ATTITUDES][6];
#ifdef USE_TEAMS
extern TeamPtr homeTeam;
#endif
//extern GlobalMapPtr GlobalMoveMap[2];
extern MoverGroupPtr CurGroup;
extern GameObjectPtr CurObject;
extern long CurObjectClass;
extern MechWarriorPtr CurWarrior;
extern GameObjectPtr CurContact;
extern long CurAlarm;
extern float MapCellDiagonal;
extern float WeaponRanges[NUM_WEAPON_RANGE_TYPES][2];

//extern float MoveMarginOfError[2];

extern DebuggerPtr debugger;

extern float MechClassWeights[NUM_MECH_CLASSES];

extern char OverlayIsBridge[NUM_OVERLAY_TYPES];

extern float SkillTry[4];
extern float SkillSuccess[4];
extern float KillSkill[6];
extern float WeaponHit;
extern float SensorSkill;

extern long RamObjectWID;

extern UserHeapPtr missionHeap;

SpecialtySkillType	MechWarrior::skillTypes[NUM_SPECIALTY_SKILLS] = 
{
	CHASSIS_SPECIALTY,		//	"LightMechSpecialist",
	WEAPON_SPECIALTY,		//	"LaserSpecialist",
	WEAPON_SPECIALTY,		//	"LightACSpecialist",
	WEAPON_SPECIALTY,		//	"MediumACSpecialist",
	WEAPON_SPECIALTY,		//	"SRMSpecialist",
	WEAPON_SPECIALTY,		//	"SmallArmsSpecialist",
	OTHER_SPECIALTY,		//	"SensorProfileSpecialist",
	OTHER_SPECIALTY,		//	"ToughnessSpecialist",	
		
	CHASSIS_SPECIALTY,		//	"MediumMechSpecialist",
	WEAPON_SPECIALTY,		//	"PulseLaserSpecialist",
	WEAPON_SPECIALTY,		//	"ERLaserSpecialist",
	WEAPON_SPECIALTY,		//	"LRMSpecialist",
	OTHER_SPECIALTY,		//	"Scout",				
	OTHER_SPECIALTY,		//	"LongJump",				
	
	CHASSIS_SPECIALTY,		//	"HevayMechSpecialist",	
	WEAPON_SPECIALTY,		//	"PPCSpecialist",
	WEAPON_SPECIALTY,		//	"HeavyACSpecialist",
	RANGE_SPECIALTY,		//	"ShortRangeSpecialist",
	RANGE_SPECIALTY,		//	"MediumRangeSpecialist",
	RANGE_SPECIALTY,		//	"LongRangeSpecialist",
	
	CHASSIS_SPECIALTY,		//	"AssaultMechSpecialist",
	WEAPON_SPECIALTY,		//	"GaussSpecialist",
	OTHER_SPECIALTY,		//	"SharpShooter",			
};	
GameObjectPtr LastCoreAttackTarget = NULL;

//***************************************************************************
// MISC routines
//***************************************************************************

void SeedRandomNumbers (void) {

	srand((unsigned)time(NULL));
}

//---------------------------------------------------------------------------

long SignedRandomNumber (long range) {

	return(RandomNumber(range << 1) - range);
}

//---------------------------------------------------------------------------

long GetMissionTurn (void) {

	return(turn);
}

//***************************************************************************
// TARGET PRIORITY routines
//***************************************************************************

void* TargetPriorityList::operator new (size_t mySize) {

	void *result = missionHeap->Malloc(mySize);
	return(result);
}

//---------------------------------------------------------------------------

void TargetPriorityList::operator delete (void* us) {

	missionHeap->Free(us);
}

//---------------------------------------------------------------------------

void TargetPriorityList::init (void) {

	size = 0;
}

//---------------------------------------------------------------------------

void TargetPriorityList::destroy (void) {

	size = 0;
}

//---------------------------------------------------------------------------

long TargetPriorityList::insert (long index, TargetPriorityPtr priority) {

	if (size == MAX_TARGET_PRIORITIES)
		return(1);
	if (index == -1)
		index = size;
	if (index > size)
		return(2);

	if (index < size) {
		//------------------
		// shift 'em down...
		memmove(&list[index + 1], &list[index], (size - index) * sizeof(TargetPriority));
	}
	memcpy(&list[index], priority, sizeof(TargetPriority));
	size++;
	return(0);
}

//---------------------------------------------------------------------------

void TargetPriorityList::remove (long index) {

	if ((index > -1) && (index < size)) {
		if (index < (size - 1)) {
			//----------------
			// shift 'em up...
			memmove(&list[index], &list[index + 1], (size - index - 1) * sizeof(TargetPriority));
		}
		size--;
	}
};

//---------------------------------------------------------------------------

long TargetPriorityList::calcAction (MechWarriorPtr pilot, GameObjectPtr target) {

	if (target->isMover()) {
		if (!pilot->getTeam() || pilot->getTeam()->isEnemy(target->getTeam())) {
			if (!target->isDisabled())
				return(TACTICAL_ORDER_ATTACK_OBJECT);
			//else
			//	return(TACTICAL_ORDER_CAPTURE)

		}
		}
	else {
		switch (target->getObjectClass()) {
			case BUILDING:
				if (target->getTeam() != pilot->getTeam()) {
					if (target->getFlag(OBJECT_FLAG_CONTROLBUILDING))
						return(TACTICAL_ORDER_CAPTURE);
					return(TACTICAL_ORDER_ATTACK_OBJECT);
				}
				break;
			case TURRET:
				break;
		}
	}
	return(TACTICAL_ORDER_NONE);
}

//---------------------------------------------------------------------------

long TargetPriorityList::calcTarget (MechWarriorPtr pilot, Stuff::Vector3D location, long contactCriteria, long& action) {

	if (!pilot->getTeam())
		return(0);

	for (long i = 0; i < size; i++) {
		switch (list[i].type) {
			case TARGET_PRIORITY_NONE:
				return(TARGET_PRIORITY_NONE);
				break;
			case TARGET_PRIORITY_GAMEOBJECT: {
				// param 0 = object WID
				// param 1 = engage radius
				// param 2 = contact criteria
				GameObjectPtr obj = NULL;
				if (list[i].params[0])
					obj = (GameObjectPtr)ObjectManager->findByPartId(list[i].params[0]);
				//---------------------------------------------------
				//HACK! For now, we will not target unarmed movers...
				list[i].params[2] += CONTACT_CRITERIA_ARMED;
				/*
				if (!obj) {
					// select best gameobject that fits params
					if (list[i].params[2]) {
						// for now, just returns the closest contact...
						long contactList[MAX_MOVERS];
						long numContacts = CurObject->getContacts(contactList, list[i].params[2], CONTACT_SORT_DISTANCE);
						float smallestDistance = 999999.0;
						for (long i = 0; i < numContacts; i++) {
							GameObjectPtr contact = ObjectManager->get(contactList[i]);
							float dist = pilot->getVehicle()->distanceFrom(contact->getPosition());
							if (dist < smallestDistance) {
								obj = contact;
								smallestDistance = dist;
							}
						}
						}
					else {
						Fatal(0, " TargetPriorityList.calcTarget: bad criteria for mover target ");
					}
				}
				*/
				if (obj) 
				{
					if (pilot->getVehicle()->distanceFrom(obj->getPosition()) < list[i].params[1]) 
					{
						if (list[i].params[2] && obj->isMover() && (!obj->isMarine() || (obj->getTeam() != Team::home)))
						{
							if (pilot->getTeam()->isContact(pilot->getVehicle(), (MoverPtr)obj, list[i].params[2])) 
							{
								action = calcAction(pilot, obj);
								return(obj->getWatchID());
							}
						}
						else
							return(obj->getWatchID());
					}
				}
				}
				break;
			case TARGET_PRIORITY_MOVER: {
				// param 0 = object WID
				// param 1 = engage radius
				// param 2 = contact criteria
				GameObjectPtr obj = NULL;
				if (list[i].params[0])
					obj = (GameObjectPtr)ObjectManager->findByPartId(list[i].params[0]);
				//---------------------------------------------------
				//HACK! For now, we will not target unarmed movers...
				list[i].params[2] += CONTACT_CRITERIA_ARMED;
				if (!obj) {
					// select best gameobject that fits params
					if (list[i].params[2]) {
						// for now, just returns the closest contact...
						int contactList[MAX_MOVERS];
						long numContacts = CurObject->getContacts(contactList, list[i].params[2], CONTACT_SORT_DISTANCE);
						float smallestDistance = 999999.0;
						for (long i = 0; i < numContacts; i++) {
							GameObjectPtr contact = ObjectManager->get(contactList[i]);
							if (!contact->isMarine() || (contact->getTeam() != Team::home))
							{
								float dist = pilot->getVehicle()->distanceFrom(contact->getPosition());
								if (dist < smallestDistance) {
									obj = contact;
									smallestDistance = dist;
								}
							}
						}
						}
					else {
						Fatal(0, " TargetPriorityList.calcTarget: bad criteria for mover target ");
					}
				}
				if (obj && obj->isMover()) {
					if (pilot->getVehicle()->distanceFrom(obj->getPosition()) < list[i].params[1]) {
						if (list[i].params[2]) {
							if (pilot->getTeam()->isContact(pilot->getVehicle(), (MoverPtr)obj, list[i].params[2])) {
								action = calcAction(pilot, obj);
								return(obj->getWatchID());
							}
							}
						else
							return(obj->getWatchID());
					}
				}
				}
				break;
			case TARGET_PRIORITY_BUILDING: {
				// param 0 = object WID
				// param 1 = engage radius
				// param 2 = if -1, looks for any enemy building. Else, specified alignment.
				GameObjectPtr obj = NULL;
				if (list[i].params[0])
					obj = (GameObjectPtr)ObjectManager->findByPartId(list[i].params[0]);
				if (!obj) {
					// select best gameobject that fits params
					if (list[i].params[2] == -1) {
						//----------------------------------------------
						// for now, return the closest enemy building...
						float smallestDistance = 999999.0;
						for (long j = 0; j < ObjectManager->getNumBuildings(); j++) {
							BuildingPtr building = ObjectManager->getBuilding(j);
							if (building->isEnemy(pilot->getTeam()))
								if (building->getObjectType()->getSubType() != BUILDING_SUBTYPE_WALL)
									if (!building->isDestroyed()) {
										float dist = pilot->getVehicle()->distanceFrom(building->getPosition());
										if (dist < smallestDistance) {
											obj = building;
											smallestDistance = dist;
										}
									}
						}
						}
					else {
						Fatal(0, " TargetPriorityList.calcTarget: bad criteria for mover target ");
					}
				}
				if (obj) {
					if (pilot->getVehicle()->distanceFrom(obj->getPosition()) < list[i].params[1])
						return(obj->getWatchID());
				}
				}
				break;
			case TARGET_PRIORITY_CURTARGET: {
				// param 0 = disengage relative to point # (-1 for cur position)
				// param 1 = disengage radius
				// param 2 = contact criteria (0 for doesn't matter)
				GameObjectPtr target = NULL;
				//------------------------------------------------------------------------
				// For now, the current order's target is considered the "current target",
				// even if the mover is moving while shooting. This can be changed easily,
				// if we want it to act differently...
				if (pilot->getCurTacOrder()->code != TACTICAL_ORDER_NONE) {
					target = ObjectManager->get(pilot->getCurTacOrder()->targetWID);
					if (target && target->isDisabled())
						if (!pilot->getCurTacOrder()->attackParams.obliterate)
							target = NULL;
				}
				if (!target) {
					target = pilot->getLastTarget();
				}
				if (target) {
					Stuff::Vector3D disengageCenter;
					Stuff::Vector3D targetPosition;
					if (list[i].params[0] == -1)
						disengageCenter = pilot->getVehicle()->getPosition();
					else
						disengageCenter = pilot->getVehicle()->getPosition(); //FOR NOW... should ref point list
					targetPosition = target->getPosition();
					Stuff::Vector3D delta;
					delta.x = disengageCenter.x - targetPosition.x;
					delta.y = disengageCenter.y - targetPosition.y;
					delta.z = 0.0;
					float distanceToTarget = delta.GetLength() * metersPerWorldUnit;
					if (distanceToTarget < list[i].params[1]) {
						if (list[i].params[2] && target->isMover()) {
							if (pilot->getTeam()->isContact(pilot->getVehicle(), (MoverPtr)target, list[i].params[2])) {
								action = calcAction(pilot, target);
								return(target->getWatchID());
							}
							}
						else {
							action = calcAction(pilot, target);
							return(target->getWatchID());
						}
					}
				}
				}
				break;
			case TARGET_PRIORITY_TURRET: {
				//------------------------------------------
				// For now, we'll pick the closest Turret...
				float shortestDistance = 999999.0;
				GameObjectPtr closestTurret = NULL;
                int i;
				for (i = 0; i < ObjectManager->getNumTurrets(); i++) {
					GameObjectPtr turret = (GameObjectPtr)ObjectManager->getTurret(i);
					if (!turret->getTeam()->isFriendly(pilot->getTeam())/* && turret->isCaptureable(pilot->getTeam()->getId())*/) {
						float distance = pilot->getVehicle()->distanceFrom(turret->getPosition());
						if (distance < shortestDistance) {
							shortestDistance = distance;
							closestTurret = turret;
						}
					}
				}
				if (closestTurret) {
					if (pilot->getVehicle()->distanceFrom(closestTurret->getPosition()) < list[i].params[1]) {
						action = calcAction(pilot, closestTurret);
						return(closestTurret->getWatchID());
					}
				}
				return(0);
				}
				break;
			case TARGET_PRIORITY_TURRET_CONTROL: {
				//--------------------------------------------------
				// For now, we'll pick the closest Turret Control...
				// WE CANNOT ASSUME CAPTURE OR ATTACK HERE! FIX BELOW...
				float shortestDistance = 999999.0;
				GameObjectPtr closestControl = NULL;
				for (long i = 0; i < ObjectManager->getNumTurretControls(); i++) {
					GameObjectPtr turretControl = (GameObjectPtr)ObjectManager->getTurretControl(i);
					if (!turretControl->getTeam()->isFriendly(pilot->getTeam()) && turretControl->isCaptureable(pilot->getTeam()->getId())) {
						float distance = pilot->getVehicle()->distanceFrom(turretControl->getPosition());
						if (distance < shortestDistance) {
							shortestDistance = distance;
							closestControl = turretControl;
						}
					}
				}
				if (closestControl) {
					action = calcAction(pilot, closestControl);
					return(closestControl->getWatchID());
				}
				}
				break;
			case TARGET_PRIORITY_GATE:
				break;
			case TARGET_PRIORITY_GATE_CONTROL: {
				//--------------------------------------------------
				// For now, we'll pick the closest Turret Control...
				// WE CANNOT ASSUME CAPTURE OR ATTACK HERE! FIX BELOW...
				float shortestDistance = 999999.0;
				GameObjectPtr closestControl = NULL;
				for (long i = 0; i < ObjectManager->getNumGateControls(); i++) {
					GameObjectPtr gateControl = (GameObjectPtr)ObjectManager->getGateControl(i);
					if (!gateControl->getTeam()->isFriendly(pilot->getTeam()) && gateControl->isCaptureable(pilot->getTeam()->getId())) {
						float distance = pilot->getVehicle()->distanceFrom(gateControl->getPosition());
						if (distance < shortestDistance) {
							shortestDistance = distance;
							closestControl = gateControl;
						}
					}
				}
				if (closestControl) {
					action = calcAction(pilot, closestControl);
					return(closestControl->getWatchID());
				}
				}
				break;
			case TARGET_PRIORITY_SALVAGE:
				break;
			case TARGET_PRIORITY_MECHBAY:
				break;
			case TARGET_PRIORITY_LOCATION:
				break;
			case TARGET_PRIORITY_EVENT_TARGETED:
			//for (long i = 0; i < alarm[curEventID].numTriggers; i++)
			//	paramList[i] = alarm[curEventID].trigger[i];
				break;
			case TARGET_PRIORITY_EVENT_HIT:
				// param 0 = selection criteria
				// param 1 = engage radius
				// param 2 = contact criteria
/*				GameObjectPtr obj = NULL;
				if (list[i].params[0])
					obj = (GameObjectPtr)ObjectManager->get(list[i].params[0]);
				if (!obj) {
					// select best gameobject that fits params
					if (list[i].params[2]) {
						// for now, just returns the closest contact...
						long contactList[MAX_MOVERS];
						long numContacts = CurObject->getContacts(contactList, list[i].params[2], CONTACT_SORT_DISTANCE);
						float smallestDistance = 999999.0;
						for (long i = 0; i < numContacts; i++) {
							GameObjectPtr contact = ObjectManager->get(contactList[i]);
							float dist = pilot->getVehicle()->distanceFrom(contact->getPosition());
							if (dist < smallestDistance) {
								obj = contact;
								smallestDistance = dist;
							}
						}
						}
					else {
						Fatal(0, " TargetPriorityList.calcTarget: bad criteria for mover target ");
					}
				}
				if (obj) {
					if (pilot->getVehicle()->distanceFrom(obj->getPosition()) < list[i].params[1]) {
						if (list[i].params[2] && obj->isMover()) {
							if (pilot->getTeam()->isContact(pilot->getVehicle(), (MoverPtr)obj, list[i].params[2])) {
								action = calcAction(pilot, obj);
								return(obj->getWatchID());
							}
							}
						else
							return(obj->getWatchID());
					}
				}
				}
				*/break;
			case TARGET_PRIORITY_EVENT_DAMAGED:
				break;
			case TARGET_PRIORITY_EVENT_MATE_DIED:
				break;
			case TARGET_PRIORITY_EVENT_FRIENDLY_DISABLED:
				break;
			case TARGET_PRIORITY_EVENT_FRIENDLY_DESTROYED:
				break;
			case TARGET_PRIORITY_EVENT_FRIENDLY_WITHDRAWS:
				break;
			case TARGET_PRIORITY_SKIP:
				break;
		}
	}

	return(0);
}

//***************************************************************************
// MECHWARRIOR class routines
//***************************************************************************

void* MechWarrior::operator new (size_t mySize) {

	void *result = missionHeap->Malloc(mySize);
	return(result);
}

//---------------------------------------------------------------------------

void MechWarrior::operator delete (void* us) {

	missionHeap->Free(us);
}

//---------------------------------------------------------------------------

void MechWarrior::lobotomy (void) {

	delete brain;
	brain = NULL;
	
	//-----------------------------------------
	// Set up the pilot alarm callback table...
	for (long i = 0; i < NUM_PILOT_ALARMS; i++)
		brainAlarmCallback[i] = NULL;
}	

//---------------------------------------------------------------------------

void MechWarrior::init (bool create) {

	used = false;
	name[0] = '\0';
	callsign[0] = '\0';
	videoStr[0] = '\0';
	audioStr[0] = '\0';
	brainStr[0] = '\0';
	photoIndex = 0;
	
	rank = WARRIOR_RANK_GREEN;
	for (int i = 0; i < NUM_SKILLS; i++) {
		for (int j = 0; j < NUM_COMBAT_STATS; j++) {
			numSkillUses[i][j] = 0;
			numSkillSuccesses[i][j] = 0;
		}
		skillRank[i] = 0.0;
		skillPoints[i] = 0.0;
        //sebi: WTF???
		//originalSkills[NUM_SKILLS] = 0;
		//startingSkills[NUM_SKILLS] = 0;
        // maybe like this
		originalSkills[i] = 0;
		startingSkills[i] = 0;
		
	}
	professionalism = 40;
	professionalismModifier = 0;
	decorum = 40;
	decorumModifier = 0;
	aggressiveness = 40;
	courage = 40;
	baseCourage = courage;

	wounds = 0.0;
	health = 0.0;
	status = WARRIOR_STATUS_NORMAL;
	escapesThruEjection = false;
	radioLog.lastMessage = -1;
	radioLog.lastUnderFire = -1000.0;
	radioLog.weaponsIneffective = false;
	radioLog.weaponsOut = false;
	radioLog.lastContact = -1000.0;
	notMineYet = false;

	teamId = 0;
	vehicleWID = 0;

	numKilled = 0;
	memset( killed, 0, sizeof( GameObject* ) * MAX_MOVERS / 3 );

	for (int i = 0; i < NUM_VEHICLE_CLASSES; i++)
		for (int j = 0; j < NUM_COMBAT_STATS; j++)
			numMechKills[i][j] = 0;

	for (int i = 0; i < NUM_PHYSICAL_ATTACKS; i++)
		for (int j = 0; j < NUM_COMBAT_STATS; j++)
			numPhysicalAttacks[i][j] = 0;

	for (int i = 0; i < NUM_SPECIALTY_SKILLS; i++)
		specialtySkills[i] = false;

	descID = 0;
	nameIndex = 0;

	keepMoving = false;

	timeOfLastOrders = -1.0;
	numAttackers = 0;
	attackRadius = DefaultAttackRadius;

	//---------------------
	// ABL Brain and Memory
	for (int i = 0; i < NUM_MEMORY_CELLS; i++)
		memory[i].integer = 0;
	brain = NULL;
	for (int i = 0; i < NUM_PILOT_ALARMS; i++)
		brainAlarmCallback[i] = NULL;

	for (int i = 0; i < NUM_PILOT_DEBUG_STRINGS; i++)
		debugStrings[i][0] = '\0';

	brainUpdate = (float)(numWarriors % 30) * 0.2;
	combatUpdate = (float)(numWarriors % 15) * 0.1;
	movementUpdate = (float)(numWarriors % 15) * 0.2;
	for (long w = 0; w < MAX_WEAPONS_PER_MOVER; w++)
		weaponsStatus[w] = 0;
	weaponsStatusResult = WEAPONS_STATUS_NO_TARGET;

	useGoalPlan = false;
	mainGoalAction = GOAL_ACTION_NONE;
	mainGoalObjectWID = 0;
	mainGoalLocation.Zero();
	mainGoalControlRadius = -1.0;
	lastGoalPathSize = 0;

	newTacOrderReceived[ORDERSTATE_GENERAL] = false;
	newTacOrderReceived[ORDERSTATE_PLAYER] = false;
	newTacOrderReceived[ORDERSTATE_ALARM] = false;

	tacOrder[ORDERSTATE_GENERAL].init();
	tacOrder[ORDERSTATE_PLAYER].init();
	tacOrder[ORDERSTATE_ALARM].init();
	lastTacOrder.init();
	curTacOrder.init();

	clearAlarms();
	clearAlarmsHistory();

	alarmPriority = 0;

	tacOrderQueue = NULL;
	enableTacOrderQueue();
	curPlayerOrderFromQueue = false;
	tacOrderQueueLocked = false;
	tacOrderQueueExecuting = false;
	tacOrderQueueLooping = false;
	numTacOrdersQueued = 0;
	tacOrderQueueIndex = 0;
	nextTacOrderId = 0;
	lastTacOrderId = -1;

	coreScanTargetWID = 0;
	coreAttackTargetWID = 0;
	coreMoveTargetWID = 0;
	targetPriorityList.init();
	brainState = 0;
	willHelp = true;

	if (create)
		moveOrders.init();

	attackOrders.init();
	situationOrders.init();
	lastTargetWID = 0;
	lastTargetTime = -1.0;
	lastTargetObliterate = false;
	lastTargetFriendly = false;
	orderState = ORDERSTATE_GENERAL;
	movePathRequest = NULL;

	if (create)
	{
		for (int i = 0; i < 2; i++) 
		{
			moveOrders.path[i] = new MovePath;
			if (!moveOrders.path[i])
				Fatal(0, " No RAM for warrior path ");
		}
	}

	debugFlags = WARRIOR_DEBUG_FLAG_NONE;
	radio = NULL;
	isPlayingMsg = false;
	oldPilot = 0;

	//------------
	// Static vars
	if (!sortList) {
		sortList = new SortList;
		if (!sortList)
			Fatal(0, " Unable to create Warrior::sortList ");
		sortList->init(100);
	}

	numWarriors++;
}

//---------------------------------------------------------------------------

long MechWarrior::init (FitIniFile* warriorFile) {

	//----------------------------------------------------------------------
	// If we want to, we could make the following tables part of an ini file
	// that defines which skills, etc. are being used in the game system.
	// This way, non-programming designers could modify which skills are
	// being used (and so on), without having to change the underlying
	// system code. The global defines, NUM_SKILLS, etc. would also have to
	// a part of the ini file...

	//------------------
	// Main Info section
	long result = warriorFile->seekBlock("General");
	if (result != NO_ERR)
		return(result);
	
	result = warriorFile->readIdLong("DescIndex", descID);
	if (result != NO_ERR)
		descID = -1;

	cLoadString( descID, name, MAXLEN_PILOT_NAME - 1);
		
	result = warriorFile->readIdLong("NameIndex", nameIndex);
	if (result != NO_ERR)
		nameIndex = -1;

	result = warriorFile->readIdBoolean("NotMineYet",notMineYet);
	if (result != NO_ERR)
		notMineYet = false;
		
	result = warriorFile->readIdLong("PictureIndex", photoIndex);
	if (result != NO_ERR)
		photoIndex = 0;

	strcpy( callsign, name );
	for ( long i = 0; i < strlen( callsign ); i ++ )
		CharUpper( callsign );
		
	result = warriorFile->readIdUChar("OldPilot",oldPilot);
	if (result != NO_ERR)
		oldPilot = 0;
	
	result = warriorFile->readIdString("pilotAudio",audioStr, MAXLEN_PILOT_AUDIO - 1);
	if (result != NO_ERR)
	{
		radio = NULL;		//No pilot Audio, so no radio.
	}
	else
	{
		result = warriorFile->readIdString("pilotVideo",videoStr,MAXLEN_PILOT_VIDEO-1);
		if (result != NO_ERR)
		{
			videoStr[0] = '\0';
		}
		
		radio = new Radio;
		result = radio->init(audioStr,102400,videoStr);		//Size of radio heap should be data driven?
		if (result != NO_ERR)
		{
			delete radio;	//Radio couldn't start, so no radio.
			radio = NULL;
		}
	}
	
	result = warriorFile->readIdLong("PaintScheme", paintScheme);
	if (result != NO_ERR)
		paintScheme = -1;		//This means mech is its normal color.
	
	result = warriorFile->seekBlock("PersonalityTraits");
	if (result != NO_ERR)
		return(result);

	result = warriorFile->readIdChar("Professionalism", professionalism);
	if (result != NO_ERR)
		return(result);

	result = warriorFile->readIdChar("Decorum", decorum);
	if (result != NO_ERR)
		return(result);

	result = warriorFile->readIdChar("Aggressiveness", aggressiveness);
	if (result != NO_ERR)
		return(result);

	result = warriorFile->readIdChar("Courage", courage);
	if (result != NO_ERR)
		return(result);
	baseCourage = courage;

	result = warriorFile->seekBlock("Skills");
	if (result != NO_ERR)
		return(result);
		
	int i=0;
	for (i = 0; i < NUM_SKILLS; i++) {
		result = warriorFile->readIdChar(SkillsTable[i], skills[i]);
		if (result != NO_ERR)
			return(result);
			
		skillRank[i] = skills[i];
	}

	result = warriorFile->seekBlock("SpecialtySkills");
	if (result == NO_ERR)
	{
		for (i = 0; i < NUM_SPECIALTY_SKILLS; i++) 
		{
			char tmpChar;
			result = warriorFile->readIdChar(SpecialtySkillsTable[i], tmpChar);
			if (result == NO_ERR)
				specialtySkills[i] = (tmpChar == 1);
		}
	}

	result = warriorFile->seekBlock("OriginalSkills");
	if (result == NO_ERR)
	{
		for (i = 0; i < NUM_SKILLS; i++) 
		{
			result = warriorFile->readIdChar(SkillsTable[i], originalSkills[i]);
			if (result != NO_ERR)
				originalSkills[i] = skills[i];
		}
	}
	else
	{
		for (i = 0; i < NUM_SKILLS; i++) 
		{
			originalSkills[i] = skills[i];
		}
	}

	result = warriorFile->seekBlock("LatestSkills");
	if (result == NO_ERR)
	{
		for (i = 0; i < NUM_SKILLS; i++) 
		{
			result = warriorFile->readIdChar(SkillsTable[i], startingSkills[i]);
			if (result != NO_ERR)
				startingSkills[i] = skills[i];
		}
	}
	else
	{
		for (i = 0; i < NUM_SKILLS; i++) 
		{
			startingSkills[i] = skills[i];
		}
	}

	result = warriorFile->seekBlock("SkillPoints");
	if (result != NO_ERR)
	{
		for (long i = 0; i < NUM_SKILLS; i++)
		{
			skillPoints[i] = 0.0;
		}
	}
	else
	{
		for (long i = 0; i < NUM_SKILLS; i++) 
		{
			result = warriorFile->readIdFloat(SkillsTable[i], skillPoints[i]);
			if (result != NO_ERR)
				skillPoints[i] = 0.0;
		}
	}

	//-----------------------
	// Calc warrior's rank...
	calcRank();

	result = warriorFile->seekBlock("Status");
	if (result != NO_ERR)
		return(result);
			
	char wnds;
	result = warriorFile->readIdChar("Wounds", wnds);
	if (result != NO_ERR)
		return(result);
	wounds = (float)wnds;

	long roll = RandomNumber(100);
	escapesThruEjection = ((roll <= (getSkill(MWS_PILOTING) + 30)) && (roll < 95));
	if (MPlayer || isToughnessSpecialist())
		escapesThruEjection = true;

	enableTacOrderQueue();

	return NO_ERR;

}
						
#define OLD_ACKNOWLEDGE_MSG		57
//---------------------------------------------------------------------------

void MoveOrders::init (void) {

	time = scenarioTime;
	origin = ORDER_ORIGIN_COMMANDER;
	speedType = MOVE_SPEED_MODERATE;
	speedVelocity = 0.0;
	speedState = MECH_STATE_WALKING;
	speedThrottle = 100;
/*
	moveOrders[which].zone.angle[0] = 45.0;
	moveOrders[which].zone.angle[1] = 90.0;
	moveOrders[which].zone.range[0] = 60.0;
	moveOrders[which].zone.range[1] = 90.0;
	moveOrders[which].pattern = MOVE_PATTERN_DIRECT;
*/		
	goalType = MOVEGOAL_NONE;
	goalObjectWID = 0;
	goalObjectPosition.Zero();
	//------------------------------------------------
	// Since world coord should never reach -999999...
	goalLocation.x = -999999.0;
	goalLocation.y = -999999.0;
	goalLocation.z = -999999.0;

	nextUpdate = 0.0;
	newGoal = false;
	numWayPts = 0;
	curWayPt = 0;
	curWayDir = 0;
	pathType = MOVEPATH_UNDEFINED;
	originalGlobalGoal[0].x = -999999.0;
	originalGlobalGoal[0].y = -999999.0;
	originalGlobalGoal[0].z = -999999.0;
	originalGlobalGoal[1].x = -999999.0;
	originalGlobalGoal[1].y = -999999.0;
	originalGlobalGoal[1].z = -999999.0;
	globalGoalLocation.x = -666666.0;
	globalGoalLocation.y = -666666.0;
	globalGoalLocation.z = -666666.0;
	numGlobalSteps = 0;
	curGlobalStep = 0;
	path[0] = NULL;
	path[1] = NULL;
	timeOfLastStep = -1.0;
	moveState = MOVESTATE_FORWARD;
	moveStateGoal = MOVESTATE_FORWARD;
	twisting = false;
	yieldTime = -1.0;
	yieldState = 0;
	waitForPointTime = -1.0;
	
	run = false;
}

//---------------------------------------------------------------------------

void AttackOrders::init (void) {

	time = scenarioTime;
	origin = ORDER_ORIGIN_COMMANDER;
	type = ATTACK_NONE;
	targetWID = 0;
	targetPoint.Zero();
	aimLocation = -1;
	pursue = false;
	targetTime = 0.0;
}

//---------------------------------------------------------------------------

void SituationOrders::init (void) {

	time = 0.0;
	mode = SITUATION_NORMAL;
	defFormation = 10.0;
	curFormation = 10.0;
	openFire = false;
	uponFireOnly = false;
	fireRange = -1.0;
	fireOdds = -1.0;
	guardObjectWID = 0;
}

//---------------------------------------------------------------------------

#define	DEBUG_RADIO_MESSAGES	0

void MechWarrior::radioMessage (long message, bool propogateIfMultiplayer) 
{
bool limitFrequency = true;

	if (message >= RADIO_MESSAGE_COUNT)
		return;

  	if (radio && active() && (message != -1) && GetMissionTurn() > 0) 
	{
		if (underHomeCommand()) 
		{
			switch (message) 
			{
				case RADIO_WEAPONS_OUT:
					if (radioLog.weaponsOut)
						return;
					radioLog.weaponsOut = true;
					break;
				case RADIO_SENSOR_CONTACT:
					if (radioLog.lastContact > (scenarioTime - 15.0))
						return;
					radioLog.lastContact = scenarioTime;
					break;
				case RADIO_UNDER_ATTACK:
					if (radioLog.lastUnderFire > (scenarioTime - 20.0))
						return;
					radioLog.lastUnderFire = scenarioTime;
					break;
			}
			switch (message)
			{
			case RADIO_TARGET_ACK:
			case RADIO_MOVE_BLOCKED:
			case RADIO_ILLEGAL_ORDER:
				limitFrequency = false;
			}

			if (limitFrequency != 0 && radioLog.lastMessageType == message &&
					radioLog.lastMessageTime > (scenarioTime - 10.0))
				return;
				
			radioLog.lastMessageTime = scenarioTime;
			radioLog.lastMessage = radio->playMessage((RadioMessageType)message);
			radioLog.lastMessageType = (RadioMessageType) message;
		}
		//According to Derek this never happens.  ONLY your pilots speak!!
		/*
		else if (MPlayer && MPlayer->isServer() && propogateIfMultiplayer) 
		{
			getVehicle()->addRadioChunk(CHUNK_SEND, (unsigned char)message);
		}
		*/
	}
}

//---------------------------------------------------------------------------

void MechWarrior::clear (void) {

	//----------------------------------------------------------------------
	// Before we do anything, make sure the warrior's orders are cleared and
	// pulled from any combat...
	clearCurTacOrder();

	//------------------------
	// Now, let's clear 'em...
	used = false;
	name[0] = '\0';
	callsign[0] = '\0';
	videoStr[0] = '\0';
	audioStr[0] = '\0';
	brainStr[0] = '\0';
	
	rank = WARRIOR_RANK_GREEN;
	for (int i = 0; i < NUM_SKILLS; i++) {
		for (int j = 0; j < NUM_COMBAT_STATS; j++) {
			numSkillUses[i][j] = 0;
			numSkillSuccesses[i][j] = 0;
		}
		skillRank[i] = 0.0;
		skillPoints[i] = 0.0;
        //sebi: WTF???
		//originalSkills[NUM_SKILLS] = 0;
		//startingSkills[NUM_SKILLS] = 0;
		originalSkills[i] = 0;
		startingSkills[i] = 0;
	}
	professionalism = 40;
	professionalismModifier = 0;
	decorum = 40;
	decorumModifier = 0;
	aggressiveness = 40;
	courage = 40;
	baseCourage = courage;

	wounds = 0.0;
	health = 0.0;
	status = WARRIOR_STATUS_NORMAL;
	escapesThruEjection = false;
	radioLog.lastMessage = -1;
	radioLog.lastUnderFire = -1000.0;
	radioLog.weaponsIneffective = false;
	radioLog.weaponsOut = false;
	radioLog.lastContact = -1000.0;
	notMineYet = false;

	teamId = 0;
	vehicleWID = 0;

	numKilled = 0;
	memset( killed, 0, sizeof( GameObject* ) * MAX_MOVERS / 3 );

	for (int i = 0; i < NUM_VEHICLE_CLASSES; i++)
		for (int j = 0; j < NUM_COMBAT_STATS; j++)
			numMechKills[i][j] = 0;

	for (int i = 0; i < NUM_PHYSICAL_ATTACKS; i++)
		for (int j = 0; j < NUM_COMBAT_STATS; j++)
			numPhysicalAttacks[i][j] = 0;

	for (int i = 0; i < NUM_SPECIALTY_SKILLS; i++)
		specialtySkills[i] = false;

	descID = 0;
	nameIndex = 0;

	timeOfLastOrders = -1.0;
	numAttackers = 0;
	attackRadius = DefaultAttackRadius;

	//---------------------
	// ABL Brain and Memory
	for (int i = 0; i < NUM_MEMORY_CELLS; i++)
		memory[i].integer = 0;

	//OK if brain is NULL. Like for Infantry!!
	if (brain)
		brain->init(-1);

	for (int i = 0; i < NUM_PILOT_ALARMS; i++)
		brainAlarmCallback[i] = NULL;

	for (int i = 0; i < NUM_PILOT_DEBUG_STRINGS; i++)
		debugStrings[i][0] = '\0';

	//brainUpdate = (float)(numWarriors % 30) * 0.2;
	//combatUpdate = (float)(numWarriors % 15) * 0.1;
	//movementUpdate = (float)(numWarriors % 15) * 0.2;
	for (long w = 0; w < MAX_WEAPONS_PER_MOVER; w++)
		weaponsStatus[w] = 0;
	weaponsStatusResult = WEAPONS_STATUS_NO_TARGET;

	newTacOrderReceived[ORDERSTATE_GENERAL] = false;
	newTacOrderReceived[ORDERSTATE_PLAYER] = false;
	newTacOrderReceived[ORDERSTATE_ALARM] = false;

	tacOrder[ORDERSTATE_GENERAL].init();
	tacOrder[ORDERSTATE_PLAYER].init();
	tacOrder[ORDERSTATE_ALARM].init();
	lastTacOrder.init();
	curTacOrder.init();

	for (long curAlarm = 0; curAlarm < NUM_PILOT_ALARMS; curAlarm++)
		clearAlarm(curAlarm);

	alarmPriority = 0;

	//tacOrderQueue = NULL;
	enableTacOrderQueue();
	curPlayerOrderFromQueue = false;
	tacOrderQueueLocked = false;
	tacOrderQueueExecuting = false;
	tacOrderQueueLooping = false;
	numTacOrdersQueued = 0;
	tacOrderQueueIndex = 0;
	nextTacOrderId = 0;
	lastTacOrderId = -1;

	coreScanTargetWID = 0;
	coreAttackTargetWID = 0;
	coreMoveTargetWID = 0;
	targetPriorityList.init();
	brainState = 0;

	MovePathPtr path1 = moveOrders.path[0];
	path1->numSteps = path1->numStepsWhenNotPaused = 0;
	MovePathPtr path2 = moveOrders.path[1];
	path2->numSteps = path2->numStepsWhenNotPaused = 0;
	moveOrders.init();
	moveOrders.path[0] = path1;
	moveOrders.path[1] = path2;

	attackOrders.init();
	situationOrders.init();
	lastTargetWID = 0;
	lastTargetTime = -1.0;
	lastTargetObliterate = false;
	lastTargetFriendly = false;
	orderState = ORDERSTATE_GENERAL;
	movePathRequest = NULL;
	clearMoveOrders();
	//for (i = 0; i < 2; i++) {
	//	moveOrders.path[i] = new MovePath;
	//	if (!moveOrders.path[i])
	//		Fatal(0, " No RAM for warrior path ");
	//}
	debugFlags = WARRIOR_DEBUG_FLAG_NONE;
	//radio = NULL;
	oldPilot = 0;

	//------------
	// Static vars
	//if (!sortList) {
	//	sortList = new SortList;
	//	if (!sortList)
	//		Fatal(0, " Unable to create Warrior::sortList ");
	//	sortList->init(100);
	//}
}

//---------------------------------------------------------------------------

void MechWarrior::destroy (void) {

	if (brain) {
		delete brain;
		brain = NULL;
	}

	for (long i = 0; i < 2; i++)
		if (moveOrders.path[i]) {
			delete moveOrders.path[i];
			moveOrders.path[i] = NULL;
		}

	numWarriors--;
	if (numWarriors == 0) {
		delete sortList;
		sortList = NULL;
	}
}

//---------------------------------------------------------------------------

long MechWarrior::getAggressiveness (bool current) {

	if (current && curTacOrder.isCombatOrder())
		return(aggressiveness + (100 - aggressiveness) / 2);
	else
		return(aggressiveness);
}

//---------------------------------------------------------------------------
// QUEUED PLAYER ORDERS routines
//---------------------------------------------------------------------------

bool MechWarrior::enableTacOrderQueue (void) {

	if ((MAX_QUEUED_TACORDERS - TacOrderQueuePos) < MAX_QUEUED_TACORDERS_PER_WARRIOR)
		return(false);

	tacOrderQueue = &TacOrderQueue[TacOrderQueuePos];
	TacOrderQueuePos += MAX_QUEUED_TACORDERS_PER_WARRIOR;
	numTacOrdersQueued = 0;
	return(true);
}

//---------------------------------------------------------------------------

long MechWarrior::addQueuedTacOrder (TacticalOrder tacOrder) {

	if (!tacOrderQueue)
	{
		gosASSERT( !"No TacOrder Queue" );
	}

	if (numTacOrdersQueued == MAX_QUEUED_TACORDERS_PER_WARRIOR)
		return(2);

	tacOrderQueue[numTacOrdersQueued].moveMode = tacOrder.moveParams.wayPath.mode[0];

	if ( tacOrder.moveParams.wayPath.mode[0] == TRAVEL_MODE_SLOW )
		tacOrderQueue[numTacOrdersQueued].marker = getWayPointMarker( tacOrder.getWayPoint(0), "WalkWayPoint" );

	else if ( tacOrder.moveParams.wayPath.mode[0] == TRAVEL_MODE_FAST )
		tacOrderQueue[numTacOrdersQueued].marker = getWayPointMarker( tacOrder.getWayPoint(0), "RunWayPoint" );

	else if ( tacOrder.moveParams.wayPath.mode[0] == TRAVEL_MODE_JUMP )
		tacOrderQueue[numTacOrdersQueued].marker = getWayPointMarker( tacOrder.getWayPoint(0), "JumpWayPoint" );

	tacOrderQueue[numTacOrdersQueued].point = tacOrder.getWayPoint(0);
	tacOrderQueue[numTacOrdersQueued].tactic = tacOrder.attackParams.tactic;
	tacOrderQueue[numTacOrdersQueued].packedData[0] = tacOrder.data[0];
	tacOrderQueue[numTacOrdersQueued].packedData[1] = tacOrder.data[1];
	numTacOrdersQueued++;

	if (numTacOrdersQueued == 1) {
		if (!newTacOrderReceived[ORDERSTATE_PLAYER]) {
			if (!curPlayerOrderFromQueue || (curTacOrder.origin != ORDER_ORIGIN_PLAYER)) {
				//--------------------------------------------------------------------
				// First queued order just given, so begin executing it immediately... NO, don't do 
				if ( tacOrderQueueExecuting )
					executeTacOrderQueue();
			}
		}
	}

	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::removeQueuedTacOrder (TacticalOrderPtr tacOrder) {

	if (!tacOrderQueue)
		return(1);

	if (numTacOrdersQueued == 0)
		return(2);

	tacOrderQueueIndex = 0;
	tacOrder->data[0] = tacOrderQueue[tacOrderQueueIndex].packedData[0];
	tacOrder->data[1] = tacOrderQueue[tacOrderQueueIndex].packedData[1];
	Stuff::Vector3D point = tacOrderQueue[tacOrderQueueIndex].point;
	TacticType tactic = (TacticType)tacOrderQueue[tacOrderQueueIndex].tactic;

	if ( tacOrderQueue[tacOrderQueueIndex].marker )
		delete tacOrderQueue[tacOrderQueueIndex].marker;

	tacOrderQueue[tacOrderQueueIndex].marker = 0;

	for (long i = 0; i < numTacOrdersQueued; i++)
		tacOrderQueue[i] = tacOrderQueue[i + 1];
	numTacOrdersQueued--;

	tacOrder->unpack();
	tacOrder->attackParams.tactic = tactic;
	tacOrder->setWayPoint(0, point);

	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::getNextQueuedTacOrder (TacticalOrderPtr tacOrder) {

	if (!tacOrderQueue)
		return(1);

	if (numTacOrdersQueued == 0)
		return(2);

	if (!tacOrderQueueLooping)
		return(removeQueuedTacOrder(tacOrder));

	tacOrder->data[0] = tacOrderQueue[tacOrderQueueIndex].packedData[0];
	tacOrder->data[1] = tacOrderQueue[tacOrderQueueIndex].packedData[1];
	tacOrder->unpack();
	tacOrder->setWayPoint(0, tacOrderQueue[tacOrderQueueIndex].point);
	tacOrder->attackParams.tactic = (TacticType)tacOrderQueue[tacOrderQueueIndex].tactic;

	tacOrderQueueIndex++;
	if (tacOrderQueueIndex >= numTacOrdersQueued)
		tacOrderQueueIndex = 0;
	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::peekQueuedTacOrder (long index, TacticalOrderPtr tacOrder) {

	if (!tacOrderQueue)
		return(1);

	if (numTacOrdersQueued == 0)
		return(2);

	if (index == -1)
		index = numTacOrdersQueued - 1;

	tacOrder->data[0] = tacOrderQueue[index].packedData[0];
	tacOrder->data[1] = tacOrderQueue[index].packedData[1];
	Stuff::Vector3D point = tacOrderQueue[index].point;
	TacticType tactic = (TacticType)tacOrderQueue[index].tactic;
	long tacOrderId = tacOrderQueue[index].id;

	tacOrder->unpack();
	tacOrder->setWayPoint(0, point);
	tacOrder->attackParams.tactic = tactic;
	tacOrder->setId(tacOrderId);

	return(0);
}

//---------------------------------------------------------------------------

void MechWarrior::clearTacOrderQueue (void) {

	for ( int i = 0; i < numTacOrdersQueued; i++ )
	if ( tacOrderQueue[i].marker )
	{
		delete tacOrderQueue[i].marker;
		tacOrderQueue[i].marker = 0;
	}

	numTacOrdersQueued = 0;
	tacOrderQueueIndex = 0;
	tacOrderQueueExecuting = false;
	tacOrderQueueLooping = false;
}

//---------------------------------------------------------------------------

void MechWarrior::executeTacOrderQueue (void) {

	if (numTacOrdersQueued > 0) {
		tacOrderQueueExecuting = true;
		TacticalOrder tacOrder;
		if (getNextQueuedTacOrder(&tacOrder) == 0) {
			setPlayerTacOrder(tacOrder, true);
		}
		}
	else
		tacOrderQueueExecuting = false;
}

//---------------------------------------------------------------------------

void MechWarrior::lockTacOrderQueue (void) {

	tacOrderQueueLocked = true;
}

//---------------------------------------------------------------------------

void MechWarrior::unlockTacOrderQueue (void) {

	tacOrderQueueLocked = false;
}

//---------------------------------------------------------------------------

long MechWarrior::getTacOrderQueue (QueuedTacOrderPtr list) {

	long numQueuedOrders = 0;
	if (curPlayerOrderFromQueue) {
		//--------------------------------------------------------------------
		// First queued order just given, so begin executing it immediately...
		list[0].point = tacOrder[ORDERSTATE_PLAYER].getWayPoint(0);
		list[0].packedData[0] = tacOrder[ORDERSTATE_PLAYER].data[0];
		list[0].packedData[1] = tacOrder[ORDERSTATE_PLAYER].data[1];
		numQueuedOrders = 1;
	}

	if (numTacOrdersQueued > 0) {
		for (long i = 0; i < numTacOrdersQueued; i++) {
			list[i + 1].point = tacOrderQueue[i].point;
			list[i + 1].tactic = tacOrderQueue[i].tactic;
			list[i + 1].packedData[0] = tacOrderQueue[i].packedData[0];
			list[i + 1].packedData[1] = tacOrderQueue[i].packedData[1];
			numQueuedOrders += numTacOrdersQueued;
		}
	}

	return(numQueuedOrders);
}

//---------------------------------------------------------------------------

long compareTacOrderId (long id1, long id2) {

	if (id1 > 240) {
		if (id2 < 16)
			return(id1 - (id2 + 255));
		else
			return(id1 - id2);
		}
	else if (id1 < 16) {
		if (id2 > 240)
			return((id1 + 255) - id2);
		else
			return(id1 - id2);
	}
	return(id1 - id2);
}

//---------------------------------------------------------------------------

void MechWarrior::updateClientOrderQueue (long curTacOrderId) {

	TacticalOrder nextTacOrder;
	long peekResult = peekQueuedTacOrder(0, &nextTacOrder);

	//---------------------------------------------------------------
	// Basically, used in MPlayer games to make sure the ordersQueue
	// is current with what the server says the pilot has queued (for
	// UI purposes--e.g. displaying waypts on the client).
	if (curTacOrderId == 0) {
		if ((peekResult == 0) && (nextTacOrder.getId() == lastTacOrderId))
			removeQueuedTacOrder(&nextTacOrder);
		return;
	}

	lastTacOrderId = curTacOrderId;
	while (peekResult == 0) {
		long compareResult = compareTacOrderId(nextTacOrder.getId(), curTacOrderId);
		if (compareResult < 0) {
			removeQueuedTacOrder(&nextTacOrder);
			/*
			char s[256];
			sprintf(s, "%s - removed tacorder, %d remaining\n", getName(), numTacOrdersQueued);
			OutputDebugString(s);
			*/
			}
		else
			return;
		peekResult = peekQueuedTacOrder(0, &nextTacOrder);
	}
}

//---------------------------------------------------------------------------
// QUEUED PLAYER ORDERS end
//---------------------------------------------------------------------------

void MechWarrior::setMainGoal (long action, GameObjectPtr obj, Stuff::Vector3D* location, float range) {

	mainGoalAction = action;
	if (obj)
		mainGoalObjectWID = obj->getWatchID();
	else
		mainGoalObjectWID = 0;
	if (location)
		mainGoalLocation = *location;
	else
		mainGoalLocation.Zero();
	mainGoalControlRadius = range;
}

//---------------------------------------------------------------------------

long MechWarrior::getMainGoal (GameObjectPtr& obj, Stuff::Vector3D& location, float& radius) {

	obj = NULL;
	if (mainGoalObjectWID)
		obj = ObjectManager->getByWatchID(mainGoalObjectWID);
	location = mainGoalLocation;
	radius = mainGoalControlRadius;
	return(mainGoalAction);
}

//---------------------------------------------------------------------------

MoverGroupPtr MechWarrior::getGroup (void) {

#ifdef USE_GROUPS
	if (getVehicle())
		return(getVehicle()->getGroup());
#endif
	return(NULL);
}

//---------------------------------------------------------------------------

MoverPtr MechWarrior::getPoint (void) {

#ifdef USE_GROUPS
	if (getGroup())
		return(getGroup()->getPoint());
#endif
	return(NULL);
}

//---------------------------------------------------------------------------

bool MechWarrior::onHomeTeam (void) {

	return(getTeam() == Team::home);
}

//---------------------------------------------------------------------------

bool MechWarrior::underHomeCommand (void) {

	MoverPtr myVehicle = getVehicle();
	if (myVehicle)
		return(myVehicle->getCommanderId() == Commander::home->getId());
	return(false);

}

//---------------------------------------------------------------------------

TeamPtr MechWarrior::getTeam (void) {

	if (teamId > -1)
		return(Team::teams[teamId]);
	return(NULL);
}

//---------------------------------------------------------------------------

CommanderPtr MechWarrior::getCommander (void) {

	MoverPtr myVehicle = getVehicle();
	if (myVehicle && (myVehicle->commanderId > -1))
		return(Commander::commanders[myVehicle->commanderId]);
	return(NULL);
}

//---------------------------------------------------------------------------

MoverPtr MechWarrior::getVehicle (void) {

	//----------------------------------------------------
	// This assumes we have a Mover. Do we want to verify?
	// I say no, for speed's sake...
	return(MoverPtr(ObjectManager->getByWatchID(vehicleWID)));
}

//---------------------------------------------------------------------------

long MechWarrior::checkSkill (long skillId, float factor) {

	//------------------------------------------------------------------
	// Makes a skill check, and returns the success\fail magnitude.
	// If returned value is negative, the check failed. If non-negative,
	// the check succeeded.

	numSkillUses[skillId][COMBAT_STAT_MISSION]++;
	skillPoints[skillId] += SkillTry[skillId];
	long successMargin = (long)(getSkill(skillId) * factor) - RandomNumber(100) - 1;
	if (successMargin >= 0 && skillId != MWS_SENSORS)
	{
		numSkillSuccesses[skillId][COMBAT_STAT_MISSION]++;
		skillPoints[skillId] += SkillSuccess[skillId];
	}
	return(successMargin);
}

//---------------------------------------------------------------------------

bool MechWarrior::injure (float numWounds, bool checkEject) {

	if (status != WARRIOR_STATUS_NORMAL)
		return(false);

	wounds += numWounds;
	if (wounds >= 6.0) 
	{
		MoverPtr myVehicle = getVehicle();
		Assert(myVehicle != NULL, 0, " Pilot has no vehicle ");
		if (checkEject || escapesThruEjection) 
		{

			numSkillUses[MWS_PILOTING][COMBAT_STAT_MISSION]++;
			skillPoints[MWS_PILOTING] += SkillTry[MWS_PILOTING];

			if (escapesThruEjection) 
			{
				wounds = 3.0f;
				numSkillSuccesses[MWS_PILOTING][COMBAT_STAT_MISSION]++;
				skillPoints[MWS_PILOTING] += SkillSuccess[MWS_PILOTING];

				//---------------------------------
				// If we can't eject, we're dead...
				// We always succeed according to this function...
				// How about, we comment it out then!!
				// -fs
				if (!myVehicle->handleEjection())
					wounds = 6.0;
			}
		}

		//------------------------------------------
		// Warrior's vehicle immediately disabled...
		if ((wounds >= 6.0) && !escapesThruEjection) 
		{
			radioMessage(RADIO_DEATH);
			status = WARRIOR_STATUS_DEAD;

			if (myVehicle)
				myVehicle->disable(PILOT_DEATH);
	
			if (getGroup())
				getGroup()->handleMateDestroyed(myVehicle->getWatchID());
	
			//---------------------------------------
			// Kill our radio. Dead Men Don't Talk...
			if (radio)
				radio->turnOff();

			return(true);
		}
	}
	else		//DON'T play this if they die!
	{
		if (numWounds > 0)
			radioMessage(RADIO_PILOT_HURT);
	}

	return(false);
}

//---------------------------------------------------------------------------

void MechWarrior::eject (void) {

	if ((status != WARRIOR_STATUS_NORMAL) && (status != WARRIOR_STATUS_WITHDRAWING))
		return;

	if (wounds < 6)
		wounds++;		//Pilots take one point on ejection.  Only if won't kill them?

	if (wounds >= 6)	//Nope, eject 'em with one point and they die!
	{
		radioMessage(RADIO_DEATH);
		status = WARRIOR_STATUS_DEAD;
	}
	else
	{
		radioMessage(RADIO_EJECTING);
		status = WARRIOR_STATUS_EJECTED;
	}


	//------------------------------------------
	// Warrior's vehicle immediately disabled...
	MoverPtr myVehicle = getVehicle();
	if (myVehicle) {
		myVehicle->disable(EJECTION_DEATH);
		if (getGroup())
			getGroup()->handleMateEjected(myVehicle->getWatchID());
	}

	//-------------------------------------------------
	// Kill our radio. Dead (missing) Men Don't Talk...
	if (radio)
		radio->turnOff();
}

//---------------------------------------------------------------------------

void MechWarrior::setTeam (TeamPtr team) {

	if (team)
		teamId = team->getId();
	else
		teamId = -1;
	if (!MPlayer && (team != Team::home)) {
		setUseGoalPlan(true);
		keepMoving = true;
		}
	else {
		setUseGoalPlan(false);
		keepMoving = false;
	}
}

//---------------------------------------------------------------------------

void MechWarrior::setVehicle (GameObjectPtr vehicle) {

	if (vehicle) {
		if (!vehicle->isMover())
			Fatal(0, " bad vehicle type ");
		vehicleWID = vehicle->getWatchID();
		}
	else
		vehicleWID = 0;

	if (radio)
		radio->setOwner(this);
}

//---------------------------------------------------------------------------
void MechWarrior::setBrainName (const char *brainName)
{
	strncpy(brainStr, brainName, MAXLEN_PILOT_BRAIN - 1);
	brainStr[MAXLEN_PILOT_BRAIN - 1] = '\0';
}

//---------------------------------------------------------------------------

long MechWarrior::setBrain (long brainHandle) {

	long brainErr = NO_ERR;

	if (brain) {
		delete brain;
		brain = NULL;
		//----------------------------------------
		// Clear the pilot alarm callback table...
		for (long i = 0; i < NUM_PILOT_ALARMS; i++)
			brainAlarmCallback[i] = NULL;
	}
	
	if (brainHandle > -1) 
	{
		warriorBrainHandle = brainHandle;
		brain = new ABLModule;
		brainErr = brain->init(brainHandle);
		if (brainErr == NO_ERR) {
			char brainName[500];
			sprintf(brainName, "Pilot %s", name);
			brain->setName(brainName);
			//-----------------------------------------
			// Set up the pilot alarm callback table...
			for (long i = 0; i < NUM_PILOT_ALARMS; i++)
				brainAlarmCallback[i] = brain->findFunction(pilotAlarmFunctionName[i], true);
		}
	}
	return(brainErr);
}

//---------------------------------------------------------------------------
extern __int64 MCTimeRunBrainUpdate;

long MechWarrior::runBrain (void) {

//	if (teamId  > -1)
//		if (teamId != Team::home->getId())
//		return(0);

	if (!brain)
		return(0);

	//----------------------------------
	// Param 1 is the ID of this mech...
	//ABLi_setIntegerParam(brainParams, 0, ((BattleMechPtr)owner)->ID);
		
	//-------------------------------------
	// Param 2 is the current game clock...
	//ABLi_setRealParam(brainParams, 1, scenarioTime);

	//--------------------------------------
	// Param 3 is the type of AI event (0 is
	// standard sequential update)...
	//ABLi_setIntegerParam(brainParams, 2, 0);

	//-----------------------
	// Now, call the brain...
	CurGroup = getGroup();
	CurObject = (GameObjectPtr)getVehicle();
	CurObjectClass = getVehicle()->getObjectClass();
	CurWarrior = this;
	CurContact = NULL;
	curEventID = 0;
	curEventTrigger = 0;
	ModuleInfo moduleInfo;
	brain->getInfo(&moduleInfo);

	brain->execute();
#ifdef LAB_ONLY
	__int64 startTime = GetCycles();
#endif
	//--------------------------------------------------------------
	// Well, we'll just set it every frame so it doesn't screw up :)
	setUseGoalPlan(!MPlayer && (getCommander() != Commander::home));

	if (useGoalPlan) {
		TacticalOrder tacOrder;
		long result = calcTacOrder(mainGoalAction, mainGoalObjectWID, mainGoalLocation, mainGoalControlRadius, 0, 1, 150.0, 300, tacOrder);
		if (result == 0) {
			if (tacOrder.code != TACTICAL_ORDER_NONE) {
				if (getCommander() == Commander::home)
					setPlayerTacOrder(tacOrder);
				else
					setGeneralTacOrder(tacOrder);
			}
			}
		else if (result > 0) {
			setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
			clearCurTacOrder();
		}
	}
#ifdef LAB_ONLY
	MCTimeRunBrainUpdate += (GetCycles() - startTime);
#endif

	CurGroup = NULL;
	CurObject = NULL;
	CurObjectClass = 0;
	CurWarrior = NULL;
	CurContact = NULL;

	//-------------------------------------------------
	// All brain modules should return an error code...
	long brainErr = brain->getInteger();
	switch (brainErr) {
		case 0:
			//------------
			// No error...
			break;
		case 1:
			//-----------------------------------------------
			// General purpose error, for now, so bomb out...
			break;
	}

	clearAlarmsHistory();

	return(brainErr);
}

//---------------------------------------------------------------------------

long MechWarrior::getVehicleStatus (void) {

	long myVehicleStatus = -1;
	GameObjectPtr myVehicle = getVehicle();
	if (myVehicle)
		myVehicleStatus = myVehicle->getStatus();
	return(myVehicleStatus);
}

//---------------------------------------------------------------------------

void MechWarrior::updateAttackerStatus (unsigned long attackerWID, float time) {

	long curAttacker;
	for (curAttacker = 0; curAttacker < numAttackers; curAttacker++)
		if (attackers[curAttacker].WID == attackerWID)
			break;
	if (curAttacker == numAttackers) {
		if (numAttackers == MAX_ATTACKERS)
			return;
		attackers[numAttackers++].WID = attackerWID;
	}
	attackers[curAttacker].lastTime = time;
}

//---------------------------------------------------------------------------

GameObjectPtr MechWarrior::calcTurretThreats (float threatRange, long minThreat) {

	if (minThreat < 1)
		minThreat = 1;

	long turretThreat[256];
	long turretControlThreat[256];
	for (int i = 0; i < ObjectManager->getNumTurretControls(); i++)
		turretControlThreat[i] = 0;
	for (int i = 0; i < ObjectManager->getNumTurrets(); i++)
		turretThreat[i] = 0;
	unsigned long vehicleWID = getVehicle()->getWatchID();
	for (int i = 0; i < ObjectManager->getNumTurrets(); i++) {
		Turret* turret = ObjectManager->getTurret(i);
		if (!turret->isDisabled() && (turret->targetWID == vehicleWID)) {
			turretThreat[i] += turret->getThreatRating();
			if (turret->parent)		//Pop-up turrets do NOT have parents!!
				turretControlThreat[((BuildingPtr)ObjectManager->getByWatchID(turret->parent))->listID] += turret->getThreatRating();
		}
	}

	long biggestThreat = -1;
	for (int i = 0; i < ObjectManager->getNumTurretControls(); i++) {
		BuildingPtr controlBuilding = ObjectManager->getTurretControl(i);
		float distance = getVehicle()->distanceFrom(controlBuilding->getPosition());
		if (distance > threatRange)
			turretControlThreat[i] = 0;
		if (controlBuilding->getTeam() == getTeam())
			turretControlThreat[i] = 0;
		if (controlBuilding->isDisabled())
			turretControlThreat[i] = 0;
		if (turretControlThreat[i] < minThreat)
			turretControlThreat[i] = 0;
		else {
			//---------------------------------------------
			// It's a threat, and big enough to consider...
			if (biggestThreat == -1)
				biggestThreat = i;
			else if (turretControlThreat[i] > turretControlThreat[biggestThreat])
				biggestThreat = i;
		}
	}

	if (biggestThreat > -1)
		return(ObjectManager->getTurretControl(biggestThreat));
	return(NULL);
}

//---------------------------------------------------------------------------

AttackerRecPtr MechWarrior::getAttackerInfo (unsigned long attackerWID) {

	for (long i = 0; i < numAttackers; i++)
		if (attackers[i].WID == attackerWID)
			return(&attackers[i]);
	return(NULL);
}

//---------------------------------------------------------------------------

long MechWarrior::getAttackers (unsigned int* attackerList, float seconds) {

	long count = 0;
	float earliestTime = scenarioTime - seconds;
	for (long i = 0; i < numAttackers; i++)
		if (attackers[i].lastTime >= earliestTime)
			attackerList[count++] = attackers[i].WID;
	return(count);
	
}

//---------------------------------------------------------------------------

long MechWarrior::setAttackTarget (GameObjectPtr object) {

	if ( object )
		attackOrders.targetWID = object->getWatchID();
	else 
		attackOrders.targetWID = 0;
	
	attackOrders.targetTime = scenarioTime;
	return(NO_ERR);
}

//---------------------------------------------------------------------------

GameObjectPtr MechWarrior::getLastTarget (void) {

	GameObjectPtr target = ObjectManager->getByWatchID(lastTargetWID);

	if (target) {
		bool clearTarget = target->isDestroyed() ||
						   (target->isDisabled() && !lastTargetObliterate) ||
						   (target->isFriendly(getVehicle()) && !lastTargetFriendly);
		if (clearTarget) {
			setLastTarget(NULL);
			lastTargetTime = -1.0;
			lastTargetObliterate = false;
			lastTargetFriendly = false;
			if (curTacOrder.isCombatOrder())
				clearCurTacOrder();
			return(NULL);
		}
	}

	if (lastTargetConserveAmmo)
		curTacOrder.attackParams.type = ATTACK_CONSERVING_AMMO;

	return(target);
}

//---------------------------------------------------------------------------

void MechWarrior::setLastTarget (GameObjectPtr target, bool obliterate, bool conserveAmmo) {

	Assert(getVehicle() != NULL, vehicleWID, " MechWarrior.setLastTarget: NULL vehicle ");
	if (getCommander() == Commander::home) {
		if (lastTargetWID) {
			GameObjectPtr lastTarget = ObjectManager->getByWatchID(lastTargetWID);
			if (lastTarget)
				lastTarget->decrementAttackers();
		}
		if (target)
			target->incrementAttackers();
	}

	if (target) {
		lastTargetWID = target->getWatchID();
		lastTargetFriendly = target->isFriendly(getVehicle());
		}
	else {
		lastTargetWID = 0;
		lastTargetFriendly = false;
	}
	lastTargetTime = scenarioTime;
	lastTargetObliterate = obliterate;
	lastTargetConserveAmmo = conserveAmmo;

}

//---------------------------------------------------------------------------

GameObjectPtr MechWarrior:: getAttackTarget (void) {

	return(ObjectManager->getByWatchID(attackOrders.targetWID));
}

//---------------------------------------------------------------------------

void MechWarrior::setCurrentTarget (GameObjectPtr target) {

	setLastTarget(target);
}

//---------------------------------------------------------------------------

GameObjectPtr MechWarrior::getAttackTargetPosition (Stuff::Vector3D& pos) {

	GameObjectPtr object = ObjectManager->getByWatchID(attackOrders.targetWID);
	if (!object) {
		clearAttackOrders();
		return(NULL);
	}
	pos = object->getPosition();
	return(object);
}

//---------------------------------------------------------------------------

void MechWarrior::clearAttackOrders (void) {

	attackOrders.origin = ORDER_ORIGIN_COMMANDER;
	attackOrders.type = ATTACK_NONE;
	attackOrders.targetWID = 0;
	attackOrders.aimLocation = -1;
	attackOrders.pursue = false;
	attackOrders.targetTime = -1.0;
}

//---------------------------------------------------------------------------

void MechWarrior::clearMoveOrders (void) {

	setMoveGoal(MOVEGOAL_NONE, NULL);
	setMoveWayPath(0);
	for (long i = 0; i < 2; i++)
		clearMovePath(i);
	setMoveState(MOVESTATE_FORWARD);
	setMoveStateGoal(MOVESTATE_FORWARD);
	setMoveYieldTime(-1.0);
	setMoveYieldState(0);
	setMoveWaitForPointTime(-1.0);
	setMoveTimeOfLastStep(-1.0);
	setMoveTwisting(false);
	setMoveGlobalPath(NULL, 0);
	PathManager->remove(this);
}

//---------------------------------------------------------------------------

GameObjectPtr MechWarrior::getSituationGuardObject (void) {

	return(ObjectManager->getByWatchID(situationOrders.guardObjectWID));
}

//---------------------------------------------------------------------------

void MechWarrior::setSituationGuardObject (GameObjectWatchID objWID) {

	situationOrders.guardObjectWID = objWID;
}

//---------------------------------------------------------------------------

unsigned long MechWarrior::getSituationGuardObjectPartId (void) {

	GameObjectPtr object = getSituationGuardObject();
	if (!object)
		return(0);
	return(object->getPartId());
}

//---------------------------------------------------------------------------

long MechWarrior::setMoveGoal (unsigned long type, Stuff::Vector3D* location, GameObjectPtr obj) {

	moveOrders.goalType = type;
	switch (type) {
		case MOVEGOAL_NONE:
			clearMoveGoal();
			break;
		case MOVEGOAL_LOCATION:
			if (location->z < -10.0)
				location->z = land->getTerrainElevation(*location);
			moveOrders.goalLocation = *location;
			moveOrders.goalObjectWID = 0;
			break;
		default:
			if (location->z < -10.0)
				location->z = land->getTerrainElevation(*location);
			moveOrders.goalLocation = *location;
			//if (!obj)
			//	obj = ObjectManager->get(type);
			moveOrders.goalObjectWID = type;
			break;
	}
	return(NO_ERR);
}

//---------------------------------------------------------------------------

unsigned long MechWarrior::getMoveGoal (Stuff::Vector3D* location, GameObjectPtr* obj) {

	if (obj)
		*obj = ObjectManager->getByWatchID(moveOrders.goalObjectWID);
	if (location)
		*location = moveOrders.goalLocation;
	return(moveOrders.goalType);
}

//---------------------------------------------------------------------------

void MechWarrior::pausePath (void) {

	if (moveOrders.path[0])
		moveOrders.path[0]->numSteps = 0;
}

//---------------------------------------------------------------------------

void MechWarrior::resumePath (void) {

	if (moveOrders.path[0])
		moveOrders.path[0]->numSteps = moveOrders.path[0]->numStepsWhenNotPaused;
}

//---------------------------------------------------------------------------

void MechWarrior::rethinkPath (unsigned long strategy) {

	Stuff::Vector3D nextWayPoint;
	bool haveWayPoint = getNextWayPoint(nextWayPoint, false);

	if (moveOrders.pathType == MOVEPATH_SIMPLE) {
		//------------------------------------------------
		// Since we're close to the end, just stop here...
		if (!haveWayPoint) {
			clearMoveOrders();
			if (curTacOrder.isMoveOrder() || curTacOrder.isWayPathOrder())
				clearCurTacOrder();
			triggerAlarm(PILOT_ALARM_NO_MOVEPATH, MOVEPATH_ERR_EARLY_STOP);
			}
		else {
			moveOrders.path[0]->numSteps = 0;
			//-----------------------------------------------
			// If you got here, you care about pathlocking...
			unsigned long params = MOVEPARAM_RECALC | MOVEPARAM_AVOID_PATHLOCKS | MOVEPARAM_FACE_TARGET;
			if (curTacOrder.moveParams.jump)
				params |= MOVEPARAM_JUMP;
			requestMovePath(curTacOrder.selectionIndex, params, 2);
		}
		}
	else if (moveOrders.pathType == MOVEPATH_COMPLEX) {
		if (moveOrders.path[0]->globalStep == (moveOrders.numGlobalSteps - 1)) {
			if (!haveWayPoint) {
				clearMoveOrders();
				if (curTacOrder.isMoveOrder() || curTacOrder.isWayPathOrder())
					clearCurTacOrder();
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, MOVEPATH_ERR_EARLY_STOP);
			}
			}
		else {
			//----------------------------------------------------------------
			// Not even on our last local path, so we may want to try again...
/*			if (moveOrders[ORDER_CURRENT].yieldState == 5) {
				// We've tried enough. Just kill it...
				clearMoveOrders(ORDER_CURRENT);
				if (curTacOrder.isMoveOrder() || curTacOrder.isWayPathOrder())
					clearCurTacOrder();
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, MOVEPATH_ERR_EARLY_STOP);
				}
			else {
				moveOrders[ORDER_CURRENT].yieldState++;
*/
				//pausePath();
				//setMoveYieldTime(scenarioTime + 1.5);
				moveOrders.path[0]->numSteps = 0;
				//-----------------------------------------------
				// If you got here, you care about pathlocking...
				unsigned long params = MOVEPARAM_RECALC | MOVEPARAM_AVOID_PATHLOCKS | MOVEPARAM_FACE_TARGET;
				if (curTacOrder.moveParams.jump)
					params |= MOVEPARAM_JUMP;
				requestMovePath(curTacOrder.selectionIndex, params, 3);
//			}
		}
	}
}

//---------------------------------------------------------------------------

float MechWarrior::getMoveDistanceLeft (void) {

	float distanceLeft = 0.0;
	
	MovePathPtr curPath = moveOrders.path[0]; //getMovePath();
	if (curPath && (curPath->numStepsWhenNotPaused > 0)) {
		distanceLeft = curPath->getDistanceLeft(getVehicle()->getPosition());
		if (moveOrders.pathType == MOVEPATH_COMPLEX) {
			//-------------------------------------------------------------
			// Ultimately, we should have a pre-calced distanceToGoal field
			// for each door-to-door link which we can use as an estimate.
			// For now, we can just assume 100.0 meters per area...
			distanceLeft += (float)(moveOrders.globalPath[curPath->globalStep].costToGoal); // ((moveOrders.numGlobalSteps - curPath->globalStep) * 50.0);
		}
	}
	return(distanceLeft);
}

//---------------------------------------------------------------------------

bool MechWarrior::isJumping (Stuff::Vector3D* jumpGoal) {

	MoverPtr myVehicle = getVehicle();
	if (myVehicle)
		return(myVehicle->isJumping(jumpGoal));
	return(false);
}

//---------------------------------------------------------------------------

MovePathPtr MechWarrior::getMovePath (void) {

	Assert((moveOrders.path[0] != NULL) && (moveOrders.path[1] != NULL), 0, " NULL move paths ");

	if (moveOrders.path[0]->numStepsWhenNotPaused == 0) {
		clearMovePath(0);
		MovePathPtr temp = moveOrders.path[0];
		moveOrders.path[0] = moveOrders.path[1];
		moveOrders.path[1] = temp;
		if (moveOrders.path[0]->numStepsWhenNotPaused > 0) {
/*			if (moveOrders.path[0]->numStepsWhenNotPaused > 1) {
				MoverPtr myVehicle = getVehicle();
				long lookAhead = moveOrders.path[0]->numStepsWhenNotPaused - 1;
				if (lookAhead > 3)
					lookAhead = 3;
				for (long i = lookAhead; i > 0; i--) {
					float distanceFromPt = myVehicle->distanceFrom(moveOrders.path[0]->stepList[i].destination);
					if (distanceFromPt <= MapCellDiagonal)
						break;
				}
				moveOrders.path[0]->curStep = i;
			}
*/			//-------------------------------------------------------------
			// We have a new move path, so set move orders appropriately...
			Stuff::Vector3D goal;
			GameObjectPtr goalObject;
			unsigned long goalType = getMoveGoal(&goal, &goalObject);
			if (goalType == MOVEGOAL_NONE) {
				// No more goal, so no more path:)
				//clearMovePath(ORDER_CURRENT, 0);
				//moveOrders[which].path[0]->numSteps = 0;
				moveOrders.path[0]->numStepsWhenNotPaused = 0;
				return(moveOrders.path[0]);
			}

			GameObjectPtr target = NULL;
			if (goalType > 0) {
				//-----------------------
				// Target is an object...
				if (goalObject)
					target = goalObject;
				else
					target = ObjectManager->get(goalType);
				if (target)
					moveOrders.path[0]->target = target->getPosition();
				else {
					//--------------------------------------------------
					// If the object no longer exists, we'll stay put...
					//clearMovePath(ORDER_CURRENT, 0);
					moveOrders.path[0]->numStepsWhenNotPaused = 0;
					return(moveOrders.path[0]);
				}
			}
	
			if (isYielding()) {
				moveOrders.yieldTime = scenarioTime + 1.5;
				//moveOrders.yieldState = 0;
				moveOrders.path[0]->numSteps = 0;
				}
			else if (isWaitingForPoint()) {
				moveOrders.path[0]->numSteps = 0;
				}
			else {
				moveOrders.yieldTime = -1.0;
				moveOrders.yieldState = 0;
			}
			
			setMoveGoal((target ? target->getWatchID() : MOVEGOAL_LOCATION), &(moveOrders.path[0]->goal));
			if (moveOrders.path[0]->globalStep == (moveOrders.numGlobalSteps - 1)) {
				//if (moveOrders[which].path[0]->selectionIndex > 0) {
					moveOrders.globalGoalLocation = moveOrders.path[0]->stepList[moveOrders.path[0]->numStepsWhenNotPaused - 1].destination;
					curTacOrder.setWayPoint(0, moveOrders.globalGoalLocation);
				//}
			}
		}
	}

	return(moveOrders.path[0]);
}

//---------------------------------------------------------------------------
bool InitWayPath = true;

void MechWarrior::setMoveWayPath (WayPathPtr wayPath, bool patrol) {

	if (wayPath) {
		for (long curPt = 0; curPt < wayPath->numPoints; curPt++) {
			moveOrders.wayPath[curPt].x = wayPath->points[curPt * 3];
			moveOrders.wayPath[curPt].y = wayPath->points[curPt * 3 + 1];
			moveOrders.wayPath[curPt].z = wayPath->points[curPt * 3 + 2];
		}

		moveOrders.numWayPts = wayPath->numPoints;
		}
	else
		moveOrders.numWayPts = 0;
if (InitWayPath) {
	moveOrders.curWayPt = 0;	// really points to next way point
	if (patrol)
		moveOrders.curWayDir = 1;
	else
		moveOrders.curWayDir = 0;
}
}

//---------------------------------------------------------------------------

void MechWarrior::addMoveWayPoint (Stuff::Vector3D wayPt, bool patrol) {

	moveOrders.wayPath[moveOrders.numWayPts] = wayPt;
	moveOrders.numWayPts++;
	if (moveOrders.numWayPts == 1) {
		if (patrol)
			moveOrders.curWayDir = 1;
		else
			moveOrders.curWayDir = 0;
	}
}

//---------------------------------------------------------------------------

void MechWarrior::setMoveGlobalPath (GlobalPathStepPtr path, long numSteps) {

	if (numSteps > MAX_GLOBAL_PATH)
		Fatal(0, " Global Path Too Long ");

	memcpy(moveOrders.globalPath, path, sizeof(GlobalPathStep) * numSteps);
	moveOrders.numGlobalSteps = numSteps;
	moveOrders.curGlobalStep = 0;
}

//---------------------------------------------------------------------------

void MechWarrior::requestMovePath (long selectionIndex, unsigned long moveParams, long source) {

	PathManager->request(this, selectionIndex, moveParams, source);
}

//---------------------------------------------------------------------------
#ifdef LAB_ONLY
extern __int64 MCTimePath1Update;
extern __int64 MCTimePath2Update;
extern __int64 MCTimePath3Update;
extern __int64 MCTimePath4Update;
extern __int64 MCTimePath5Update;
#endif
long MechWarrior::calcMovePath (long selectionIndex, unsigned long moveParams) {

 	MoverPtr myVehicle = getVehicle();
	bool flying = (myVehicle->getMoveLevel() > 0);

	Stuff::Vector3D jumpGoal;
	bool jumping = isJumping(&jumpGoal);

	Stuff::Vector3D start;
	if (jumping)
		start = jumpGoal;
	else
		start = myVehicle->getPosition();

	//--------------------------------------------------------------
	// If we're starting on a blocked cell, let's use our last valid
	// position as our start...
//***NOTE***:
//Does this hold up if we're on a cell that was just changed to impassable
//(e.g. on a bridge tile just blown?)?!
//**********
	if (!flying && !jumping) {
		int cellRow, cellCol;
		myVehicle->getCellPosition(cellRow, cellCol);
		if (!GameMap->getPassable(cellRow, cellCol))
			start = myVehicle->getLastValidPosition();
	}

	int posCellR, posCellC;
	land->worldToCell(start, posCellR, posCellC);
	long startArea = GlobalMoveMap[myVehicle->getMoveLevel()]->calcArea(posCellR, posCellC);

	bool escapeTile = ((moveParams & MOVEPARAM_ESCAPE_TILE) != 0);

	Stuff::Vector3D goal;
	GameObjectPtr goalObject;
	unsigned long goalType = getMoveGoal(&goal, &goalObject);
	if (goalType == MOVEGOAL_NONE) {
		LastMoveCalcErr = MOVEPATH_ERR_NO_GOAL;
		triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
		return(LastMoveCalcErr);
	}

	GameObjectPtr target = NULL;
	if (goalType > 0) {
		//-----------------------
		// Target is an object...
		if (goalObject)
			target = goalObject;
		else
			target = ObjectManager->get(goalType);
		if (!target) {
			//--------------------------------------------------
			// If the object no longer exists, we'll stay put...
			LastMoveCalcErr = MOVEPATH_ERR_GOALOBJ_DEAD;
			triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
			return(LastMoveCalcErr);
		}
	}

	long pathNum = -1;

	if (moveParams & MOVEPARAM_INIT) {
		moveOrders.pathType = MOVEPATH_UNDEFINED;
		moveOrders.numGlobalSteps = 0;
		moveOrders.originalGlobalGoal[0] = goal;
		if ((moveOrders.moveStateGoal == MOVESTATE_PIVOT_FORWARD) ||
			(moveOrders.moveStateGoal == MOVESTATE_PIVOT_REVERSE) ||
			(moveOrders.moveStateGoal == MOVESTATE_PIVOT_TARGET)) {
			moveOrders.moveState = MOVESTATE_FORWARD;
			moveOrders.moveStateGoal = MOVESTATE_FORWARD;
		}
		pathNum = 0;
	}

	bool yielding = isYielding();
  	if (moveParams & MOVEPARAM_RECALC) {
		if (target) {
			Stuff::Vector3D pos = target->getPosition();
			setMoveGoal(target->getWatchID(), &pos, target);
			}
		else
			setMoveGoal(MOVEGOAL_LOCATION, &moveOrders.originalGlobalGoal[0]);
		goalType = getMoveGoal(&goal, &goalObject);
		//Assert(goal.x > -999000.0, 0, " Warrior:calcMovePath--Bad Original Global Goal ");
		for (long i = 0; i < 2; i++)
			clearMovePath(i);
		moveOrders.pathType = MOVEPATH_UNDEFINED;
		moveOrders.numGlobalSteps = 0;
		moveOrders.moveState = MOVESTATE_FORWARD;
		moveOrders.moveStateGoal = MOVESTATE_FORWARD;
		pathNum = 0;
	}

	//Assert(goal.x > -999000.0, 0, " Warrior:calcMovePath--Bad Original Global Goal 2");
	if (goal.x < -666000.0)
		return(LastMoveCalcErr = NO_ERR);

	//-----------------------------------------
	// Are we calcing the current or next path?
	if (pathNum == -1) {
		if (moveOrders.path[0]->numStepsWhenNotPaused == 0)
			pathNum = 0;
		else
			pathNum = 1;
	}

	__int64 startTime = 0;
	//----------------------------------------------------------------------
	// Before we do anything else, check if we already have a global path...
	if (moveOrders.pathType == MOVEPATH_UNDEFINED/*numGlobalSteps == 0*/) {
		//---------------------------------------------------------
		// Must be a new move order, so let's first calc the actual
		// move goal...
		if (!escapeTile) {
			long result = NO_ERR;
			if ((myVehicle->getCommander() == Commander::home) && (curTacOrder.code != TACTICAL_ORDER_NONE) && (curTacOrder.origin == ORDER_ORIGIN_PLAYER))
				moveParams |= MOVEPARAM_PLAYER;
			startTime = GetCycles();
			if (myVehicle->moveRadius > 0.0)
				result = myVehicle->calcMoveGoal(target, myVehicle->moveCenter, myVehicle->moveRadius, goal, selectionIndex, goal, lastGoalPathSize, lastGoalPath, moveParams);
			else
				result = myVehicle->calcMoveGoal(target, goal, -1.0, goal, selectionIndex, goal, lastGoalPathSize, lastGoalPath, moveParams);
#ifdef LAB_ONLY
			MCTimePath1Update += (GetCycles() - startTime);
#endif
			if (result != NO_ERR) {
				LastMoveCalcErr = MOVEPATH_ERR_NO_VALID_GOAL;
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
				return(LastMoveCalcErr);
			}
			//-------------------------------
			// Save the revised final goal...
		}
		moveOrders.originalGlobalGoal[1] = goal;

		if (escapeTile) {
			//------------------------------
			// Do stuff here, then return...
			Assert(pathNum == 0, pathNum, " Warrior.calcMovePath: escapePath should be pathNum 0 ");
			moveOrders.pathType = MOVEPATH_SIMPLE;
			
			myVehicle->updatePathLock(false);
			if ((curTacOrder.code == TACTICAL_ORDER_ATTACK_OBJECT) && (curTacOrder.attackParams.method == ATTACKMETHOD_RAMMING))
				RamObjectWID = curTacOrder.targetWID;
			else
				RamObjectWID = 0;
			GameObjectPtr ramObject = ObjectManager->getByWatchID(RamObjectWID);
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(false);
			if (myVehicle->getObjectClass() != ELEMENTAL)
				moveParams |= MOVEPARAM_AVOID_PATHLOCKS;
			Stuff::Vector3D escapeGoal;
			long numSteps = myVehicle->calcEscapePath(moveOrders.path[pathNum], start, goal, NULL, moveParams | MOVEPARAM_STATIONARY_MOVERS, escapeGoal);
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(true);
			myVehicle->updatePathLock(true);
			RamObjectWID = 0;

			if (numSteps > 0) {
				//----------------------------------------
				// Found escape path from this BAD tile...
				moveOrders.path[pathNum]->numSteps = numSteps;
				moveOrders.path[pathNum]->numStepsWhenNotPaused = numSteps;
				moveOrders.globalGoalLocation = moveOrders.path[pathNum]->stepList[numSteps - 1].destination;
				curTacOrder.setWayPoint(0, moveOrders.globalGoalLocation);
				if (target)
					moveOrders.path[pathNum]->target = target->getPosition();
				setMoveGoal((target ? target->getWatchID() : MOVEGOAL_LOCATION), &goal);
				moveOrders.nextUpdate = scenarioTime + MovementUpdateFrequency;

				if (pathNum == 0) {
					if (yielding) {
						moveOrders.yieldTime = scenarioTime + 1.5;
						//moveOrders[ORDER_CURRENT].yieldState = 0;
						moveOrders.path[pathNum]->numSteps = 0;
						}
					else if (isWaitingForPoint()) {
						moveOrders.path[pathNum]->numSteps = 0;
						}
					else {
						moveOrders.yieldTime = -1.0;
						moveOrders.yieldState = 0;
					}
				}
				LastMoveCalcErr = NO_ERR;
				}
			else {
				//clearMoveOrders(ORDER_CURRENT);
				LastMoveCalcErr = MOVEPATH_ERR_LR_NOT_ENABLED;
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
			}
			return(LastMoveCalcErr);

		}

		bool faceTarget = (moveParams & MOVEPARAM_FACE_TARGET) != 0;
		if ((myVehicle->distanceFrom(goal) < Mover::marginOfError[1]) && !faceTarget) {
			clearMoveGoal();
			LastMoveCalcErr = MOVEPATH_ERR_ALREADY_THERE;
			triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
			return(LastMoveCalcErr);
		}

		//----------------------------------------------------------------------------
		// If the distance between the start and goal is within our "quickmove" range,
		// don't do any global pathfinding--do it all on one movemap. In other words,
		// make it a "simple" path. Also, if we're starting on a blocked tile (burnt
		// forest tile, for example), we'll want to do a quickmove, as well...
		int goalCellR, goalCellC;
		land->worldToCell(goal, goalCellR, goalCellC);

		bool doQuickMove = false;
		long rowDiff = goalCellR - posCellR;
		if (rowDiff < 0)
			rowDiff *= -1;
		if (rowDiff <= SimpleMovePathRange) {
			long colDiff = goalCellC - posCellC;
			if (colDiff < 0)
				colDiff *= -1;
			if (colDiff <= SimpleMovePathRange)
				doQuickMove = true;
		}

		bool startAreaOpen = (startArea >= 0) && (GlobalMoveMap[myVehicle->getMoveLevel()]->areas[startArea].open || GlobalMoveMap[myVehicle->getMoveLevel()]->areas[startArea].offMap);

		if (doQuickMove) {
			moveOrders.pathType = MOVEPATH_SIMPLE;

			myVehicle->updatePathLock(false);
			if ((curTacOrder.code == TACTICAL_ORDER_ATTACK_OBJECT) && (curTacOrder.attackParams.method == ATTACKMETHOD_RAMMING))
				RamObjectWID = curTacOrder.targetWID;
			else
				RamObjectWID = 0;
			GameObjectPtr ramObject = ObjectManager->getByWatchID(RamObjectWID);
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(false);
			if (myVehicle->getObjectClass() != ELEMENTAL)
				moveParams |= MOVEPARAM_AVOID_PATHLOCKS;
			startTime = GetCycles();
			long numSteps = myVehicle->calcMovePath(moveOrders.path[pathNum], MOVEPATH_SIMPLE, start, goal, NULL, moveParams | MOVEPARAM_STATIONARY_MOVERS);
#ifdef LAB_ONLY
			MCTimePath2Update += (GetCycles() - startTime);
#endif
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(true);
			myVehicle->updatePathLock(true);
			RamObjectWID = 0;

			bool foundPath = (numSteps > 0);
			if ((numSteps > 0) && (selectionIndex > 0)) {
				//---------------------------------------------
				// We're supposed to stop early on this path...
				numSteps -= (selectionIndex / GroupMoveTrailLen[1] * GroupMoveTrailLen[0]);
				if (numSteps <= 0) {
					//----------------------------------------------------
					// Found a path, but it's not far enough to even move!
					clearMoveOrders();
					LastMoveCalcErr = MOVEPATH_ERR_ALREADY_THERE;
					triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
					return(LastMoveCalcErr);
				}
			}
			if (numSteps > 0) {
				moveOrders.path[pathNum]->numSteps = numSteps;
				moveOrders.path[pathNum]->numStepsWhenNotPaused = numSteps;
				moveOrders.globalGoalLocation = moveOrders.path[pathNum]->stepList[numSteps - 1].destination;
				curTacOrder.setWayPoint(0, moveOrders.globalGoalLocation);
				if (target)
					moveOrders.path[pathNum]->target = target->getPosition();
				setMoveGoal((target ? target->getWatchID() : MOVEGOAL_LOCATION), &goal);
				moveOrders.nextUpdate  = scenarioTime + MovementUpdateFrequency;
				}
			else if (!foundPath && (startArea == -1)) {
				Assert(pathNum == 0, pathNum, " Warrior.calcMovePath: pathNum should be 0 ");
				//----------------------------------------------------------------------
				// Starting on a non-area (i.e. bad:) tile, so try to find a SR escape
				// route to a cell that has a LR route to our desired goal (or is within
				// the same area as the goal's area)...
				TacticalOrder alarmTacOrder;
				alarmTacOrder.init(ORDER_ORIGIN_SELF, TACTICAL_ORDER_MOVETO_POINT, false);
				alarmTacOrder.setWayPoint(0, goal);
				alarmTacOrder.moveParams.escapeTile = true;
				alarmTacOrder.moveParams.wait = false;
				alarmTacOrder.moveParams.wayPath.mode[0] = (getMoveRun() ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
				setAlarmTacOrder(alarmTacOrder, 255);
				//clearMoveOrders(ORDER_CURRENT);
				LastMoveCalcErr = MOVEPATH_ERR_ESCAPING_TILE;
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
				return(LastMoveCalcErr);
				}
			else if (!foundPath/* && longRangeEnabled*/)
				doQuickMove = false;
			else {
				//clearMoveOrders(ORDER_CURRENT);
				LastMoveCalcErr = MOVEPATH_ERR_LR_NOT_ENABLED;
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
				return(LastMoveCalcErr);
			}
		}

		if (!doQuickMove) {
			//---------------------------------------------------------------
			// The path must be a multi-part, or "complex", path that travels
			// across globalmap areas/doors...
			moveOrders.globalGoalLocation = goal;
			curTacOrder.setWayPoint(0, moveOrders.globalGoalLocation);

			long goalArea = GlobalMoveMap[myVehicle->getMoveLevel()]->calcArea(goalCellR, goalCellC);

			long numSteps = -1;
			if (startAreaOpen) {
				//if ((myVehicle->control.type == CONTROL_AI) && (getTeam() != Team::home))
				if (useGoalPlan)
					if (myVehicle->numFunctionalWeapons > 0)
						GlobalMoveMap[myVehicle->getMoveLevel()]->useClosedAreas = true;

				GlobalMoveMap[myVehicle->getMoveLevel()]->moverTeamID = myVehicle->getTeamId();
				startTime = GetCycles();
				if (GlobalMap::logEnabled) {
					static char s[256];
					sprintf(s, "[%.2f] calcPath: [%05d]%s", scenarioTime, myVehicle->getPartId(), myVehicle->getName());
					GlobalMap::writeLog(s);
				}
				numSteps = GlobalMoveMap[myVehicle->getMoveLevel()]->calcPath(startArea,
																			  goalArea,
																			  moveOrders.globalPath,
																			  posCellR,
																			  posCellC,
																			  goalCellR,
																			  goalCellC);
#ifdef LAB_ONLY
				MCTimePath3Update += (GetCycles() - startTime);
#endif
				GlobalMoveMap[myVehicle->getMoveLevel()]->useClosedAreas = false;
			}
			if (numSteps == -1) {
				Assert(pathNum == 0, pathNum, " Warrior.calcMovePath: pathNum should be 0 ");
				//----------------------------------------------------------------------
				// Starting on a non-area (i.e. bad:) tile, so try to find a SR escape
				// route to a cell that has a LR route to our desired goal (or is within
				// the same area as the goal's area)...
				TacticalOrder alarmTacOrder;
				alarmTacOrder.init(ORDER_ORIGIN_SELF, TACTICAL_ORDER_MOVETO_POINT, false);
				alarmTacOrder.setWayPoint(0, goal);
				alarmTacOrder.moveParams.escapeTile = true;
				alarmTacOrder.moveParams.wait = false;
				alarmTacOrder.moveParams.wayPath.mode[0] = (getMoveRun() ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
				setAlarmTacOrder(alarmTacOrder, 255);
				//clearMoveOrders(ORDER_CURRENT);
				LastMoveCalcErr = MOVEPATH_ERR_ESCAPING_TILE;
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
				return(LastMoveCalcErr);
/*				//-------------------------------------------------------------------
				// Starting on a blocked tile. For now, we simply go short-range move
				// straight toward the goal...
				clearMoveOrders();
				LastMoveCalcErr = MOVEPATH_ERR_LR_START_BLOCKED;
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
				return(LastMoveCalcErr);
*/				}
			else if (numSteps == 0) {
				clearMoveOrders();
				LastMoveCalcErr = MOVEPATH_ERR_LR_NO_PATH;
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
				if (moveParams & MOVEPARAM_RADIO_RESULT) {
					clearCurTacOrder();
					radioMessage(RADIO_MOVE_BLOCKED, true);
				}
				return(LastMoveCalcErr);
			}
			moveOrders.pathType = MOVEPATH_COMPLEX;
			moveOrders.numGlobalSteps = numSteps;
			moveOrders.curGlobalStep = 0;
		}
		}
	else if (moveOrders.pathType == MOVEPATH_COMPLEX) {
		moveOrders.curGlobalStep++;
	}

	if (moveOrders.pathType == MOVEPATH_COMPLEX) {
		long curGlobalStep = moveOrders.curGlobalStep;
		long numGlobalSteps = moveOrders.numGlobalSteps;

		if (curGlobalStep == numGlobalSteps) {
			//--------------------------------
			// We've finished the global path.
			//triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
			return(LastMoveCalcErr = NO_ERR);
		}

		//--------------------------------------------------------------------------------
		// NEW: Now, our goal will be determined by our current global step in our global
		// path. Our moveOrders goal will contain our final goal of the entire global
		// path. Thus, our local (current) goal may not nec. be the same as our moveOrders
		// (final) goal...
		long prevThruArea = -1;
		long prevGoalDoor = -1;
		if ((curGlobalStep > 0) && (curGlobalStep < (numGlobalSteps - 1))) {
			GlobalPathStep prevGlobalStep = moveOrders.globalPath[curGlobalStep - 1];
			prevThruArea = moveOrders.globalPath[curGlobalStep - 1].thruArea;
			prevGoalDoor = moveOrders.globalPath[curGlobalStep - 1].goalDoor;
			land->cellToWorld(prevGlobalStep.goalCell[0], prevGlobalStep.goalCell[1], start);
		}

		GlobalPathStepPtr globalStep = &moveOrders.globalPath[curGlobalStep];
		GlobalMapDoorPtr globalDoor = &GlobalMoveMap[myVehicle->getMoveLevel()]->doors[globalStep->goalDoor];
		if (curGlobalStep < (numGlobalSteps - 1))
			if (!globalDoor->open) {
				LastMoveCalcErr = MOVEPATH_ERR_LR_DOOR_CLOSED;
				setMoveWayPath(NULL, 0);
				setMoveTimeOfLastStep(scenarioTime);
				setMoveGlobalPath(NULL, 0);
				PathManager->request(this, selectionIndex, MOVEPARAM_RECALC + MOVEPARAM_FACE_TARGET, 19);
				triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
				return(LastMoveCalcErr);
			}
		
		long numSteps = 0;
		if (curGlobalStep < (numGlobalSteps - 2)) {
			clearMovePath(pathNum);
			myVehicle->updatePathLock(false);
			if ((curTacOrder.code == TACTICAL_ORDER_ATTACK_OBJECT) && (curTacOrder.attackParams.method == ATTACKMETHOD_RAMMING))
				RamObjectWID = curTacOrder.targetWID;
			else
				RamObjectWID = 0;
			GameObjectPtr ramObject = ObjectManager->getByWatchID(RamObjectWID);
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(false);
			if (myVehicle->getObjectClass() != ELEMENTAL)
				moveParams |= MOVEPARAM_AVOID_PATHLOCKS;
			startTime = GetCycles();
			long thruArea[2] = {-1, -1};
			long goalDoor = -1;
			if (curGlobalStep == (numGlobalSteps - 2)) {
				// Second from last, so the actual goal cell instead of the door
				thruArea[0] = globalStep->thruArea;
				goalDoor = globalStep->goalDoor;
			}
			else {
				thruArea[0] = globalStep->thruArea;
				thruArea[1] = moveOrders.globalPath[curGlobalStep+1].thruArea;
				goalDoor = moveOrders.globalPath[curGlobalStep+1].goalDoor;
			}
			numSteps = myVehicle->calcMovePath(
                    moveOrders.path[pathNum], start, thruArea, goalDoor, moveOrders.globalGoalLocation, &goal, globalStep->goalCell, moveParams | MOVEPARAM_STATIONARY_MOVERS);
#ifdef LAB_ONLY
			MCTimePath4Update += (GetCycles() - startTime);
#endif
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(true);
			myVehicle->updatePathLock(true);
			RamObjectWID = 0;
			}
		else if (curGlobalStep == (numGlobalSteps - 2)) {
			clearMovePath(pathNum);
			goal = moveOrders.originalGlobalGoal[1];
			myVehicle->updatePathLock(false);
			if ((curTacOrder.code == TACTICAL_ORDER_ATTACK_OBJECT) && (curTacOrder.attackParams.method == ATTACKMETHOD_RAMMING))
				RamObjectWID = curTacOrder.targetWID;
			else
				RamObjectWID = 0;
			GameObjectPtr ramObject = ObjectManager->getByWatchID(RamObjectWID);
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(false);
			if (myVehicle->getObjectClass() != ELEMENTAL)
				moveParams |= MOVEPARAM_AVOID_PATHLOCKS;
			startTime = GetCycles();
			numSteps = myVehicle->calcMovePath(moveOrders.path[pathNum], MOVEPATH_COMPLEX, start, goal, globalStep->goalCell, moveParams | MOVEPARAM_STATIONARY_MOVERS);
#ifdef LAB_ONLY
			MCTimePath5Update += (GetCycles() - startTime);
#endif
			if (ramObject && ramObject->isMover())
				((MoverPtr)ramObject)->updatePathLock(true);
			myVehicle->updatePathLock(true);
			RamObjectWID = 0;
			if (numSteps > 0) {
				if (selectionIndex > 0) {
					//---------------------------------------------
					// We're supposed to stop early on this path...
					numSteps -= (selectionIndex / GroupMoveTrailLen[1] * GroupMoveTrailLen[0]);
					if (numSteps <= 0) {
						//----------------------------------------------------
						// Found a path, but it's not far enough to even move!
						clearMoveOrders();
						LastMoveCalcErr = MOVEPATH_ERR_ALREADY_THERE;
						triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
						return(LastMoveCalcErr);
					}
					if (pathNum == 0) {
						moveOrders.globalGoalLocation = moveOrders.path[pathNum]->stepList[numSteps - 1].destination;
						curTacOrder.setWayPoint(0, moveOrders.globalGoalLocation);
					}
				}
				curGlobalStep++;
			}
		}
		else if (curGlobalStep != (numGlobalSteps - 1))
			STOP((" Mechwarrior.calcMovePath: bad global step (%d/%d)", curGlobalStep, numGlobalSteps));

		if (numSteps > 0) {
//			Assert(!isWaitingForPoint() || (pathNum != 0), 0, " fudge ");
			moveOrders.path[pathNum]->numSteps = numSteps;
			moveOrders.path[pathNum]->numStepsWhenNotPaused = numSteps;
			moveOrders.path[pathNum]->globalStep = curGlobalStep;
			if (pathNum == 0) {
				if (target)
					moveOrders.path[pathNum]->target = target->getPosition();
				setMoveGoal((target ? target->getWatchID() : MOVEGOAL_LOCATION), &goal);
			}
			//moveOrders[ORDER_CURRENT].nextUpdate  = scenarioTime + MovementUpdateFrequency;
			}
		else if (curGlobalStep != (numGlobalSteps - 1)) {
			//clearMoveOrders(ORDER_CURRENT);
			moveOrders.curGlobalStep--;
			if (numSteps == -999) {
				//---------------------------------------------------------------------------------------
				// Moving to a blocked door (all possible goal cells for the door are currently blocked).
				LastMoveCalcErr = MOVEPATH_ERR_LR_DOOR_BLOCKED;
				}
			else
				LastMoveCalcErr = MOVEPATH_ERR_LR_NO_SR_PATH;
			triggerAlarm(PILOT_ALARM_NO_MOVEPATH, LastMoveCalcErr);
			return(LastMoveCalcErr);
		}
		}
	else if ((moveOrders.pathType != MOVEPATH_SIMPLE) && (moveOrders.pathType != MOVEPATH_UNDEFINED))
		Fatal(0, " Bad Move Path Type ");

	if (pathNum == 0) {
		if (yielding) {
			moveOrders.yieldTime = scenarioTime + 1.5;
			//moveOrders[ORDER_CURRENT].yieldState = 0;
			moveOrders.path[pathNum]->numSteps = 0;
			}
		else if (isWaitingForPoint()) {
			moveOrders.path[pathNum]->numSteps = 0;
			}
		else {
			moveOrders.yieldTime = -1.0;
			moveOrders.yieldState = 0;
		}
	}

	return(LastMoveCalcErr = NO_ERR);
}

//---------------------------------------------------------------------------

bool MechWarrior::getNextWayPoint (Stuff::Vector3D& nextPoint, bool incWayPoint) {

	if (!curTacOrder.isWayPathOrder())
		return(false);

	// if curWayDir == 0, then the path is one-time only (loop through it once).
	// if curWayDir == 1, then loop through it forward. Once end is hit, loop back...
	// if curWayDir == -1, then loop through it backward. Once start is hit, loop forward...
	if (moveOrders.curWayDir == 0) {
		//-------------------------------------------------
		// This is not a patrol path, so stop at the end...
		long nextWayPt = moveOrders.curWayPt + 1;
		if (nextWayPt < moveOrders.numWayPts)
			nextPoint = moveOrders.wayPath[nextWayPt];
		if (incWayPoint) {
			//curTacOrder.moveParams.wayPath.points[0] = nextPoint.x;
			//curTacOrder.moveParams.wayPath.points[1] = nextPoint.y;
			//curTacOrder.moveParams.wayPath.points[2] = nextPoint.z;
			moveOrders.curWayPt = nextWayPt;
			if (curTacOrder.moveParams.wayPath.mode[nextWayPt] == TRAVEL_MODE_FAST)
				setMoveRun(true);
			else
				setMoveRun(false);
			curTacOrder.moveParams.wayPath.curPoint = nextWayPt;
			if (nextWayPt == moveOrders.numWayPts)
				curTacOrder.setStage(2);
		}
		if (nextWayPt == moveOrders.numWayPts)
			return(false);
		}
	else {
		long nextWayPt = 0;
		long nextWayDir = moveOrders.curWayDir;
		if (moveOrders.curWayDir == 1) {
			if (moveOrders.curWayPt == (moveOrders.numWayPts - 1)) {
				//------------------------------------------------------
				// Reached end, so turn around and start backtracking...
				nextWayDir = -1;
				nextWayPt = moveOrders.numWayPts - 2;
				}
			else
				//----------------------------------
				// Going forward to next waypoint...
				nextWayPt = moveOrders.curWayPt + 1;
			}
		else {
			if (moveOrders.curWayPt == 0) {
				nextWayDir = 1;
				nextWayPt = 1;
				}
			else
				nextWayPt = moveOrders.curWayPt - 1;
		}
		nextPoint = moveOrders.wayPath[nextWayPt];
		if (incWayPoint) {
			//curTacOrder.moveParams.wayPath.points[0] = nextPoint.x;
			//curTacOrder.moveParams.wayPath.points[1] = nextPoint.y;
			//curTacOrder.moveParams.wayPath.points[2] = nextPoint.z;
			moveOrders.curWayDir = nextWayDir;
			moveOrders.curWayPt = nextWayPt;
			if (curTacOrder.moveParams.wayPath.mode[nextWayPt] == TRAVEL_MODE_FAST)
				setMoveRun(true);
			else
				setMoveRun(false);
			curTacOrder.moveParams.wayPath.curPoint = nextWayPt;
		}
	}
	return(true);
}

//---------------------------------------------------------------------------
// COMBAT DECISION TREE
//---------------------------------------------------------------------------

long MechWarrior::calcWeaponsStatus (GameObjectPtr target, int* weaponList, Stuff::Vector3D* targetPoint) {

	MoverPtr myVehicle = getVehicle();
	if (!myVehicle->canFireWeapons())
		return(WEAPONS_STATUS_CANNOT_FIRE);

	Stuff::Vector3D targetPosition;
	if (target)
		targetPosition = target->getPosition();
	else if (targetPoint)
		targetPosition = *targetPoint;
	else
		return(WEAPONS_STATUS_NO_TARGET);

	//------------------------
	// Am I even within range?
	float distanceToTarget = myVehicle->distanceFrom(targetPosition);
	if ((distanceToTarget <= myVehicle->getMinFireRange()) || (distanceToTarget >= myVehicle->getMaxFireRange()))
		return(WEAPONS_STATUS_OUT_OF_RANGE);

	long numWeaponsWithShot = 0;
	for (long curWeapon = 0; curWeapon < myVehicle->numWeapons; curWeapon++) 
	{
		long weaponIndex = myVehicle->numOther + curWeapon;
		if (!myVehicle->isWeaponReady(weaponIndex))
			weaponList[curWeapon] = WEAPON_STATUS_NOT_READY;
		else 
		{
			long curShots = myVehicle->getWeaponShots(weaponIndex);
			if (curShots < 1)
				weaponList[curWeapon] = WEAPON_STATUS_OUT_OF_AMMO;
			else 
			{
				if (!myVehicle->weaponInRange(weaponIndex, distanceToTarget, MapCellDiagonal))
					weaponList[curWeapon] = WEAPON_STATUS_OUT_OF_RANGE;
				else 
				{
					float relAngle = myVehicle->weaponLocked(weaponIndex, targetPosition);
					float fireArc = myVehicle->getFireArc();
					bool lineOfFire = false;
					//ALWAYS do the SIMPLE calculation first.  If they have indirect fire weapons, direct LOS is Still VALID!
					if (target)
						lineOfFire = myVehicle->lineOfSight(target);
					else
						lineOfFire = myVehicle->lineOfSight(targetPosition);

					//Indirect fire weapons can fire if ANYONE can see target! 
					if (!lineOfFire && myVehicle->getWeaponIndirectFire(weaponIndex))
					{
						if (!myVehicle->getTeam())
							lineOfFire = false;
						else if (target)
							lineOfFire = myVehicle->getTeam()->teamLineOfSight(target->getLOSPosition(), target->getAppearRadius());
						else
							lineOfFire = myVehicle->getTeam()->teamLineOfSight(targetPosition, CELL_DISTANCE);
					}
					
					if (!lineOfFire || (relAngle < -fireArc) || (relAngle > fireArc))
						weaponList[curWeapon] = WEAPON_STATUS_NOT_LOCKED;
					else 
					{
						long aimLocation = -1;
						if (curTacOrder.isCombatOrder())
							aimLocation = curTacOrder.attackParams.aimLocation;
						float odds = myVehicle->calcAttackChance(target, aimLocation, scenarioTime, weaponIndex, 0.0, NULL, targetPoint);
						if (odds < 1.0)
							weaponList[curWeapon] = WEAPON_STATUS_NO_CHANCE;
						else 
						{
							weaponList[curWeapon] = odds;
							numWeaponsWithShot++;
						}
					}
				}
			}
		}
	}
					
	return(numWeaponsWithShot);
}

//---------------------------------------------------------------------------

void MechWarrior::printWeaponsStatus (char* s) {

	if (weaponsStatusResult == WEAPONS_STATUS_OUT_OF_RANGE)
		sprintf(s, "Out of Range");
	else if (getCurrentTarget() && !getVehicle()->lineOfSight(getCurrentTarget()))
		sprintf(s, "No Line-of-sight");
	else if (weaponsStatusResult == WEAPONS_STATUS_CANNOT_FIRE)
		sprintf(s, "Cannot Fire");
	else if (weaponsStatusResult == WEAPONS_STATUS_NO_TARGET)
		sprintf(s, "No Target");
	else {
		long tally[6] = {0, 0, 0, 0, 0, 0};
		long numWithChance = 0;
		for (long curWeapon = 0; curWeapon < getVehicle()->numWeapons; curWeapon++) {
			if (weaponsStatus[curWeapon] < 0)
				tally[weaponsStatus[curWeapon] + 6]++;
			else if (weaponsStatus[curWeapon] > 0)
				numWithChance++;
		}
		sprintf(s, "#=%02d(%02d),RD=%02d,RN=%02d,LK=%02d,CH=%02d,AM=%02d",
			getVehicle()->numWeapons,
			numWithChance,
			tally[5],
			tally[3],
			tally[2],
			tally[1],
			tally[4]);
	}
}

//---------------------------------------------------------------------------

long MechWarrior::combatDecisionTree (void) {

	long result = -1;

	combatUpdate = scenarioTime + CombatUpdateFrequency;

	Stuff::Vector3D tPoint;

	MoverPtr myVehicle = getVehicle();
	Assert(myVehicle != NULL, 0, " Pilot has no vehicle! ");

	//----------------------------------
	// Let's see if we have ANY ammo...
	bool outOfAmmo = true;
	for (long i = 0; i < myVehicle->getNumAmmoTypes(); i++)
		if (myVehicle->getAmmoTypeTotal(i) > 0) {
			outOfAmmo = false;
			break;
		}


	GameObjectPtr target = getLastTarget(); //getAttackTarget(ORDER_CURRENT);

	//------------------------------------------------------------------------
	// Our target is our last target, if we had one (and we weren't explicitly
	// told to hold fire since then)...
	Stuff::Vector3D *targetPoint = NULL;
	long attackType = ATTACK_TO_DESTROY;
	long aimLocation = -1;
	if (curTacOrder.isCombatOrder()) {
		attackType = curTacOrder.attackParams.type;
		aimLocation = curTacOrder.attackParams.aimLocation;
		if (curTacOrder.code == TACTICAL_ORDER_ATTACK_POINT)
		{
			tPoint = getAttackTargetPoint();
			targetPoint = &tPoint;

			//MUST set the suppressionFire flag if we are host and this is from client
			// AND we are an artilleryPiece WITHOUT suppressionFire set AND
			// OUR attackParams are method == ATTACKMETHOD_RANGED, range = FIRERANGE_CURRENT
			// AND pursue = false.  Complex but keeps from having to figure out how to pass
			// the friggering flag around.
			if (MPlayer && MPlayer->isServer())
			{
				if (myVehicle->isGuardTower() && !(myVehicle->suppressionFire) &&
					(curTacOrder.attackParams.method == ATTACKMETHOD_RANGED) &&
					(curTacOrder.attackParams.range == FIRERANGE_CURRENT) &&
					(curTacOrder.attackParams.pursue == false))
				{
					myVehicle->suppressionFire = true;
				}
			}
		}
	}
	else
	{
		if (lastTargetConserveAmmo)
			attackType = ATTACK_CONSERVING_AMMO;
	}

	if (target) {
		if (!curTacOrder.isCombatOrder()) {
			Stuff::Vector3D pos = target->getPosition();
			if (myVehicle->distanceFrom(pos) > attackRadius) {
				setLastTarget(NULL);
				target = NULL;
			}
		}
	}
	
	if (!target && (curTacOrder.code != TACTICAL_ORDER_ATTACK_POINT)) {
		#if WARRIOR_DEBUGGING_ENABLED
			if (debugFlags & WARRIOR_DEBUG_FLAG_COMBAT) {
				char s[128];
				sprintf(s, "%s (%.2f) has no attack target.\n", callsign, getSituationFireRange());
				debugPrint(s, true);
			}
		#endif
		return(result);
	}

	if (target) {
		if (target->isDestroyed()) {
			#if WARRIOR_DEBUGGING_ENABLED
				if (debugFlags & WARRIOR_DEBUG_FLAG_COMBAT) {
					char s[128];
					sprintf(s, "%s (%.2f) has a destroyed target.\n", callsign, getSituationFireRange());
					debugPrint(s, true);
				}
			#endif
			return(result);
		}

		if (target->isDisabled() && !lastTargetObliterate) {
			#if WARRIOR_DEBUGGING_ENABLED
				if (debugFlags & WARRIOR_DEBUG_FLAG_COMBAT) {
					char s[128];
					sprintf(s, "%s (%.2f) has a disabled target.\n", callsign, getSituationFireRange());
					debugPrint(s, true);
				}
			#endif
			return(result);
		}
	}

	#if WARRIOR_DEBUGGING_ENABLED
		if (debugFlags & WARRIOR_DEBUG_FLAG_COMBAT) {
			char s[512];
			if (!myVehicle->canFireWeapons()) {
				sprintf(s, "%s's (%.2f) vehicle cannot fire now.\n", callsign, getSituationFireRange());
				debugPrint(s, true);
				}
			else if (weaponsStatusResult < 1) {
				switch (weaponsStatusResult) {
					case WEAPONS_STATUS_CANNOT_FIRE:
						sprintf(s, "%s's (%.2f) vehicle cannot fire now.", callsign, getSituationFireRange());
						break;
					case WEAPONS_STATUS_NO_TARGET:
						sprintf(s, "%s (%.2f) has no target.\n", callsign, getSituationFireRange());
						break;
					case WEAPONS_STATUS_OUT_OF_RANGE:
						sprintf(s, "%s (%.2f) out of range.\n", callsign, getSituationFireRange());
						break;
					case 0: {
						long numWeaponsNotReady = 0;
						long numWeaponsOutOfRange = 0;
						long numWeaponsNotLocked = 0;
						long numWeaponsOutOfAmmo = 0;
						long numWeaponsNoChance = 0;
						long numWeaponsTooHot = 0;
						for (long i = 0; i < myVehicle->numWeapons; i++) {
							if (weaponsStatus[i] == WEAPON_STATUS_NOT_READY)
								numWeaponsNotReady++;
							if (weaponsStatus[i] == WEAPON_STATUS_OUT_OF_AMMO)
								numWeaponsOutOfAmmo++;
							if (weaponsStatus[i] == WEAPON_STATUS_OUT_OF_RANGE)
								numWeaponsOutOfRange++;
							if (weaponsStatus[i] == WEAPON_STATUS_NOT_LOCKED)
								numWeaponsNotLocked++;
							if (weaponsStatus[i] == WEAPON_STATUS_NO_CHANCE)
								numWeaponsNoChance++;
							if (weaponsStatus[i] == WEAPON_STATUS_TOO_HOT)
								numWeaponsTooHot++;
						}
						sprintf(s, "%s (%.2f) has no shot: %d !ready, %d !ammo, %d !inrange, %d !locked, %d !chance, %d hot\n", callsign, getSituationFireRange(), numWeaponsNotReady, numWeaponsOutOfAmmo, numWeaponsOutOfRange, numWeaponsNotLocked, numWeaponsNoChance, numWeaponsTooHot);
						}
						break;
					default:
						sprintf(s, "%s (%.2f)  cannot fire for unknown reason.\n", callsign,  getSituationFireRange());
				}
				debugPrint(s, true);
			}
		}
	#endif

	//----------------------------------------------------------------------
	// So we don't muck up the brains at this late date, simply report we're
	// out of ammo...
	if (attackType != ATTACK_CONSERVING_AMMO && outOfAmmo )
	{
		radioMessage(RADIO_AMMO_OUT, TRUE);
	}

	BOOL cantConserve = FALSE;	// report if we're asked to use ammo conservation and that's impossible
	if (attackType == ATTACK_CONSERVING_AMMO)
		cantConserve = TRUE;


	bool firedWeapon = false;
	if (myVehicle->canFireWeapons() && (weaponsStatusResult > 0)) {
		//------------------
		// First, a taunt...
		// Only Taunt Moving things!!
		if (target && target->isMover() && RollDice(1))
			radioMessage(RADIO_TAUNT, true);

		//-------------
		// Let loose...
		float targetTime = getLastTargetTime();

		// Track how much damage I've done to the target when weapons finally hit.
		// If I've done enough to blow the target, STOP firing!!
		float damageDoneToTarget = 0.0f;

		for (long curWeapon = 0; curWeapon < myVehicle->numWeapons; curWeapon++)
		{
			long weaponIndex = myVehicle->numOther + curWeapon;
			if (weaponsStatus[curWeapon] > 0)
			{
				if (!target || (target && (damageDoneToTarget < target->getDestructLevel())))
				{
					if ( attackType != ATTACK_CONSERVING_AMMO || myVehicle->getWeaponIsEnergy(weaponIndex))
					{
						float dmgDone = 0.0f;
						if ( NO_ERR == myVehicle->fireWeapon(target, targetTime, myVehicle->numOther + curWeapon, attackType, aimLocation, targetPoint, dmgDone) )
							cantConserve = FALSE;
						firedWeapon = true;
						damageDoneToTarget += dmgDone;
					}
				}
			}
			result = NO_ERR;
		}
	}

	if (curTacOrder.attackParams.tactic == TACTIC_STOP_AND_FIRE) {
		if (firedWeapon)
			clearMoveOrders();
	}
	if (cantConserve)	// probably a lie
	{
		for (long curWeapon = myVehicle->numOther; curWeapon < myVehicle->numOther + myVehicle->numWeapons; curWeapon++)
		{
			if (myVehicle->getWeaponShots(curWeapon) == UNLIMITED_SHOTS && myVehicle->isWeaponWorking(curWeapon))
			{
				cantConserve = FALSE;	// it is a lie! we have a working energy weapon, it's just recycling or something...
				break;
			}
		}
	}

	if (cantConserve)
	{
		radioMessage(RADIO_ILLEGAL_ORDER);
		setLastTarget(NULL);
		clearCurTacOrder();	// so we don't keep trying (and announcing that it doesn't work!!)
	}

	return(result);
}

//---------------------------------------------------------------------------
// MOVEMENT DECISION TREE
//---------------------------------------------------------------------------

#if 0

Stuff::Vector3D vectorOffset (Stuff::Vector3D start, Stuff::Vector3D goal, long how) {

	Stuff::Vector2DOf<float> start2d(start.x, start.y);
	Stuff::Vector2DOf<float> goal2d(goal.x, goal.y);
	Stuff::Vector2DOf<float> deltaVector;

	//start2d.init(start.x, start.y);
	//goal2d.init(goal.x, goal.y);
	if (how == 0) {
		// check from start to goal
		deltaVector.x = goal2d.x - start2d.x;
		deltaVector.y = goal2d.y - start2d.y;
		}
	else {
		// check from goal to start
		deltaVector.x = start2d.x - goal2d.x;
		deltaVector.y = start2d.y - goal2d.y;
	}

	//-------------------------------------------------------------
	// First, we need to calc the delta vector--how much we extend
	// the ray everytime we check the map cell for clear placement.
	deltaVector.Normalize(deltaVector);
	float cellLength = Terrain::MetersPerCell;
	cellLength *= 0.5;
	deltaVector *= cellLength;
	if (deltaVector.GetLength() == 0.0)
		return(start);

	//-------------------------------------------------
	// Determine the max length the ray must be cast...
	float maxLength = distance_from(start, goal);

	//------------------------------------------------------------
	// We'll start at the target, and if it's blocked, we'll move
	// toward our start location, looking for the first valid/open
	// cell...
	vector_2d curPoint;
	vector_2d startPoint;
	if (how == 0)
		curPoint.init(start2d.x, start2d.y);
	else
		curPoint.init(goal2d.x, goal2d.y);
	startPoint = curPoint;
	vector_2d curRay;
	curRay.zero();
	float rayLength = 0.0;

	long tileR, tileC, cellR, cellC;
	Stuff::Vector3D curPoint3d;
	curPoint3d.init(curPoint.x, curPoint.y, 0.0);
	GameMap->worldToMapPos(curPoint3d, tileR, tileC, cellR, cellC);
	bool cellClear = GameMap->cellPassable(tileR, tileC, cellR, cellC);

	while (!cellClear && (rayLength < maxLength)) {
		curPoint3d.init(curPoint.x, curPoint.y, 0.0);
		GameMap->worldToMapPos(curPoint3d, tileR, tileC, cellR, cellC);
		cellClear = GameMap->cellPassable(tileR, tileC, cellR, cellC);

		curPoint += deltaVector;
		curRay = curPoint - startPoint;
		rayLength = curRay.magnitude();
	}

	curPoint3d.init(curPoint.x, curPoint.y, 0.0);
	curPoint3d.z = GameMap->getTerrainElevation(curPoint3d);
	return(curPoint3d);
}

#endif

//---------------------------------------------------------------------------

Stuff::Vector3D MechWarrior::calcWithdrawGoal (float withdrawRange) {

	Stuff::Vector3D escapeVector;

#ifdef USE_TEAMS

	//------------------------------------------------------------------------------
	// Should also check against allied team, if the clan team is
	// withdrawing (but will fix after we know this first works with just one team:)
	if ((teamId == 	INNER_SPHERE_TEAM_ID) || (teamId == ALLIED_TEAM_ID))
		escapeVector = GetClanTeam->calcEscapeVector((MoverPtr(BaseObjectPtr(vehicle))), withdrawRange);
	else
		escapeVector = GetInnerSphereTeam->calcEscapeVector((MoverPtr(BaseObjectPtr(vehicle))), withdrawRange);

	MoverPtr myVehicle = (MoverPtr(BaseObjectPtr(vehicle)));
	Assert(myVehicle != NULL, 0, " Warrior has NULL Vehicle ");
	if (escapeVector.magnitude() == 0.0)
		return(myVehicle->getPosition());

	//-------------------------------------------------------------------------
	// Let's just go as far as we can in this direction (if not far, then we're
	// screwed, for now. Perhaps we should go berserk, and just attack the closest
	// enemy if they're near. Otherwise, ask player for help? Or, we could scan
	// a few other directions for possible escape)...
	//-------------------------------------------------------------
	// First, we need to calc the delta vector--how much we extend
	// the ray everytime we check the map cell for clear placement.
	float cellLength = Terrain::MetersPerCell;
	cellLength *= 0.5;
	escapeVector *= cellLength;

	Stuff::Vector3D curPoint = myVehicle->getPosition();
	Stuff::Vector3D curRay = curPoint - myVehicle->getPosition();
	float rayLength = curRay.magnitude() * metersPerWorldUnit;
	long lastTileRow, lastTileCol;
	GameMap->worldToMapTilePos(curPoint, lastTileRow, lastTileCol);

	while (rayLength < withdrawRange) {
		long tileRow, tileCol;
		GameMap->worldToMapTilePos(curPoint, tileRow, tileCol);
		if ((tileRow != lastTileRow) || (tileCol != lastTileCol)) {
			if (!GameMap->inBounds(tileRow, tileCol))
				break;
			MapTile tile = GameMap->getTile(tileRow, tileCol);
			if (tile.isFullyBlocked())
				break;
			//----------------------------------------------------
			// We've entered a new tile, so record where we are...
			lastTileRow = tileRow;
			lastTileCol = tileCol;
		}
		
		curPoint += escapeVector;
		curRay = curPoint - myVehicle->getPosition();
		rayLength = curRay.magnitude() * metersPerWorldUnit;
	}

	return(curPoint);
#else
	return(getVehicle()->getPosition());
#endif
}

//---------------------------------------------------------------------------

bool MechWarrior::movingOverBlownBridge (void) {

	if (getMovePath()) {
#ifdef USE_BRIDGES
		//-------------------------------------
		// If on a blown bridge, return true...
		MoverPtr myVehicle = getVehicle();
		long tileRow = 0, tileCol = 0, cellRow = 0, cellCol = 0;
		myVehicle->getTileCellPosition(tileRow, tileCol, cellRow, cellCol);
		long overlay = GameMap->getOverlayType(tileRow, tileCol);
		if (OverlayIsBridge[overlay]) {
			long bridgeArea = GlobalMoveMap->calcArea(tileRow, tileCol);
			if (GlobalMoveMap->isClosedArea(bridgeArea))
				return(true);
		}
		//-----------------------------------------------------
		// If SR path will cross a blown bridge, return true...
		long bridgeArea = getMovePath()->crossesBridge(-1, 3);
		if (bridgeArea > -1)
			if (GlobalMoveMap->isClosedArea(bridgeArea))
				return(true);
		//-----------------------------------------------------
		// If LR path will cross a blown bridge, return true...
		if (moveOrders.pathType == MOVEPATH_COMPLEX) {
			long curGlobalStep = moveOrders.curGlobalStep;
			long numGlobalSteps = moveOrders.numGlobalSteps;
			for (long i = curGlobalStep; i < numGlobalSteps; i++) {
				long thruArea = moveOrders.globalPath[i].thruArea;
				if (GlobalMoveMap->isBridgeArea(thruArea)) {
					if (GlobalMoveMap->isClosedArea(thruArea))
						return(true);
					}
				else
					return(false);
			}
		}
#endif
	}
	return(false);
}

//---------------------------------------------------------------------------

bool MechWarrior::movementDecisionTree (void) {

	if (moveOrders.timeOfLastStep > -1.0) {
		if (moveOrders.timeOfLastStep < (scenarioTime - MoveTimeOut)) {
			//-------------------------------------------------------
			// Abort move order, 'cause we are unable to take a step!
			clearMoveOrders();
			if (curTacOrder.isMoveOrder() || curTacOrder.isWayPathOrder()) {
				if (curTacOrder.time < (scenarioTime - MoveTimeOut)) {
					radioMessage(RADIO_MOVE_BLOCKED, true);
					clearCurTacOrder();
				}
			}
			triggerAlarm(PILOT_ALARM_NO_MOVEPATH, MOVEPATH_ERR_TIME_OUT);
		}
	}

	MoverPtr myVehicle = getVehicle();

	if (isYielding()) {
		if (getMoveYieldTime() < scenarioTime) {
			setMoveYieldTime(scenarioTime + MoveYieldTime);
			//-----------------------------------------------------------------------
			// For now, we'll use random chance to decide if we should recalc or not.
			// DEFINITELY use a smarter priority scheme before this thing ships :)
			bool onBlownBridge = false;
			MoverPtr myVehicle = getVehicle();
			int cellRow = 0, cellCol = 0;
			myVehicle->getCellPosition(cellRow, cellCol);
			#ifdef USE_OVERLAYS
			long overlay = GameMap->getOverlay(cellRow, cellCol);
			if (OverlayIsBridge[overlay]) {
				long bridgeArea = GlobalMoveMap->calcArea(tileRow, tileCol);
				if (GlobalMoveMap->isClosedArea(bridgeArea))
					onBlownBridge = true;
			}
			#endif
			bool reroute = onBlownBridge || (RandomNumber(100) < 75);
			if (getMovePath()) {
				long bridgeArea = getMovePath()->crossesBridge(-1, 3);
				if (bridgeArea > -1)
					reroute = reroute || movingOverBlownBridge();
			}
			if (reroute) {
				unsigned long params = MOVEPARAM_RECALC | MOVEPARAM_FACE_TARGET;
				if (curTacOrder.moveParams.jump)
					params |= MOVEPARAM_JUMP;
				requestMovePath(curTacOrder.selectionIndex, params, 4);
			}
		}
	}

	if (getMovePathType() == MOVEPATH_COMPLEX) {
		//-------------------------------------------------------------------
		// Currently following a complex (LR) path--make sure we queue up the
		// next path segment if we haven't already...
		if (getMoveCurGlobalStep() < (getMoveNumGlobalSteps() - 1)) {
			if (moveOrders.path[1]->numStepsWhenNotPaused == 0) {
				//------------------------------------------------
				// Currently, we don't have any paths queued up...
				if (!movePathRequest) {
					//----------------------------------------------------------------
					// And, we don't already have a request to fill the queue for this
					// complex path, so let's do it...
					TacticalOrderPtr curTacOrder = getCurTacOrder();
					unsigned long params = MOVEPARAM_FACE_TARGET;
					if (curTacOrder->moveParams.jump)
						params |= MOVEPARAM_JUMP;
					requestMovePath(curTacOrder->selectionIndex, params, 5);
				}
			}
		}
	}

	//-----------------------------------------------------------------
	// Everything up to this point is done EVERY frame update. Now, see
	// if it's time to do the ugly stuff...
	if (movementUpdate > scenarioTime)
		return(true);

	movementUpdate = scenarioTime + MovementUpdateFrequency;

//	if (myVehicle->isSelected())
//		OutputDebugString("DebugMe\n");

	if ((curTacOrder.code == TACTICAL_ORDER_NONE) || (curTacOrder.code == TACTICAL_ORDER_STOP)) {
		GameObjectPtr target = getLastTarget();
		if (target)
			setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
		return(true);
	}

	if (getVehicle()->attackRange == FIRERANGE_CURRENT)
		if (!curTacOrder.isMoveOrder() && (curTacOrder.code != TACTICAL_ORDER_CAPTURE)) {
			GameObjectPtr target = getLastTarget();
			if (target)
				setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
			return(true);
		}

	GameObjectPtr goalObject = NULL;
	unsigned long goalType = getMoveGoal(NULL, &goalObject);

	if (goalType == MOVEGOAL_NONE) {
		//--------------------------------------------
		// Are we currently pursuing an attack target?
		// Is this what we really want?
		if (curTacOrder.code == TACTICAL_ORDER_WITHDRAW) {
			//--------------------------------------------------------
			// We are on the run, so let's choose a new escape goal...
			Stuff::Vector3D escapeGoal = calcWithdrawGoal();
			setMoveGoal(MOVEGOAL_LOCATION, &escapeGoal);
			unsigned long params = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
			if (curTacOrder.moveParams.jump)
				params |= MOVEPARAM_JUMP;
			requestMovePath(curTacOrder.selectionIndex, params, 6);
			}
		else if (curTacOrder.code == TACTICAL_ORDER_MOVETO_POINT) {
			long msg;
			curTacOrder.execute(this, msg);
			}
		else if ((curTacOrder.code == TACTICAL_ORDER_MOVETO_OBJECT) || (curTacOrder.code == TACTICAL_ORDER_CAPTURE)) {
			GameObjectPtr moveTarget = curTacOrder.getTarget();
			//-------------------------------------------------
			// Pick the best move goal based upon the object...
			Stuff::Vector3D targetPos = moveTarget->getPosition();
			if (moveTarget->isMover())
				targetPos = moveTarget->getPosition();
			else {
				//------------------------------------------------
				// If not a mover, then pick an open cell adjacent
				// to it. If it happens to be adjacent to blocked
				// cells, this could be off...
				if (moveTarget->isBuilding()) {
					BuildingPtr building = (BuildingPtr)moveTarget;
					long goalRow = 0, goalCol = 0;
					bool foundGoal = building->calcAdjacentAreaCell(getVehicle()->moveLevel, -1, goalRow, goalCol);
					if (foundGoal)
						land->cellToWorld(goalRow, goalCol, targetPos);
					else {
						Stuff::Vector3D objectPos = moveTarget->getPosition();
						targetPos = getVehicle()->calcOffsetMoveGoal(objectPos);
					}
					}
				else {
					Stuff::Vector3D objectPos = moveTarget->getPosition();
					targetPos = getVehicle()->calcOffsetMoveGoal(objectPos);
				}
			}
			setMoveGoal(moveTarget->getWatchID(), &targetPos, moveTarget);
			unsigned long params = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
			if (curTacOrder.moveParams.jump)
				params |= MOVEPARAM_JUMP;
			if (!moveTarget->isMover())
				requestMovePath(curTacOrder.selectionIndex, params | MOVEPARAM_STEP_ADJACENT_TARGET, 7);
			else
				requestMovePath(curTacOrder.selectionIndex, params, 8);
			}
		else if (curTacOrder.isCombatOrder()) {
			Stuff::Vector3D targetPosition;
			GameObjectPtr target = getCurrentTarget();
			if (target)
				targetPosition = target->getPosition();
			else if (curTacOrder.code == TACTICAL_ORDER_ATTACK_POINT)
				targetPosition = getAttackTargetPoint();
			else
				return(true);
			if (curTacOrder.attackParams.method == ATTACKMETHOD_RAMMING) {
				Stuff::Vector3D pos = target->getPosition();
				setMoveGoal(target->getWatchID(), &pos, target);
				unsigned long params = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
				if (curTacOrder.moveParams.jump)
					params |= MOVEPARAM_JUMP;
				requestMovePath(curTacOrder.selectionIndex, params, 9);
				}
			else if (/*!getAttackPursuit() && */!curTacOrder.attackParams.pursue)
				setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
			else {
				bool recalcPath = false;
				unsigned long moveParams = MOVEPARAM_NONE;

				//---------------------------------------------------------------
				// We will want to re-evaluate optimal range if our optimal range
				// has changed...
				if (target && target->isMover())
					if (((MoverPtr)target)->getLastWeaponEffectivenessCalc() > myVehicle->getLastOptimalRangeCalc())
						if (myVehicle->calcFireRanges()) {
							recalcPath = true;
							moveParams = MOVEPARAM_SOMEWHERE_ELSE;
						}

				float distanceToTarget = myVehicle->distanceFrom(targetPosition);
				float desiredAttackRange = myVehicle->getOrderedFireRange();
				float attackRangeDelta = distanceToTarget - desiredAttackRange;
				float recalcDelta = Terrain::worldUnitsPerVertex * metersPerWorldUnit;
				if ((curTacOrder.attackParams.tactic != TACTIC_STOP_AND_FIRE) && (curTacOrder.attackParams.tactic != TACTIC_TURRET) && ((attackRangeDelta > recalcDelta) || (attackRangeDelta < -recalcDelta))) {
					//----------------------------------------
					// Try to maintain attack range...
					recalcPath = true;
					moveParams = MOVEPARAM_SOMEWHERE_ELSE;
					}
				else if ((curTacOrder.attackParams.tactic != TACTIC_TURRET) && (weaponsStatusResult == WEAPONS_STATUS_OUT_OF_RANGE)) {
					recalcPath = true;
					moveParams = MOVEPARAM_SOMEWHERE_ELSE;
					}
				else if (weaponsStatusResult >= 0) {
					long numWeaponsNotReady = 0;
					long numWeaponsOutOfAmmo = 0;
					long numWeaponsOutOfRange = 0;
					long numWeaponsNotLocked = 0;
					long numWeaponsNoChance = 0;
					long numWeaponsTooHot = 0;
					for (long i = 0; i < myVehicle->numWeapons; i++) {
						if (weaponsStatus[i] == WEAPON_STATUS_NOT_READY)
							numWeaponsNotReady++;
						if (weaponsStatus[i] == WEAPON_STATUS_OUT_OF_AMMO)
							numWeaponsOutOfAmmo++;
						if (weaponsStatus[i] == WEAPON_STATUS_OUT_OF_RANGE)
							numWeaponsOutOfRange++;
						if (weaponsStatus[i] == WEAPON_STATUS_NOT_LOCKED)
							numWeaponsNotLocked++;
						if (weaponsStatus[i] == WEAPON_STATUS_NO_CHANCE)
							numWeaponsNoChance++;
						if (weaponsStatus[i] == WEAPON_STATUS_TOO_HOT)
							numWeaponsTooHot++;
					}
					switch (myVehicle->getObjectClass()) {
						case BATTLEMECH:
							if (weaponsStatusResult == 0) {
								if ((target && !myVehicle->lineOfSight(target)) ||
									(!target && !myVehicle->lineOfSight(targetPosition))) {
										recalcPath = true;
										moveParams = MOVEPARAM_SOMEWHERE_ELSE;
									}
								else if ((numWeaponsNotReady > 0) || (numWeaponsNotLocked > 0) || (numWeaponsTooHot > 0)) {
									if ((getMovePath()->numSteps == 0) && !getMoveTwisting())
										setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
									}
								else {
									recalcPath = true;
									moveParams = MOVEPARAM_SOMEWHERE_ELSE;
								}
							}
							break;
						case GROUNDVEHICLE:
							if (weaponsStatusResult == 0) {
								if ((target && !myVehicle->lineOfSight(target)) ||
									(!target && !myVehicle->lineOfSight(target))) {
										recalcPath = true;
										moveParams = MOVEPARAM_SOMEWHERE_ELSE;
									}
								else if (numWeaponsNotReady == 0) {
									if ((numWeaponsNotLocked > 0) || (numWeaponsTooHot > 0)) {
										if ((getMovePath()->numSteps == 0) && !getMoveTwisting())
											setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
										}
									else {
										recalcPath = true;
										moveParams = MOVEPARAM_SOMEWHERE_ELSE;
									}
								}
                            }
							break;
						case ELEMENTAL:
							if (weaponsStatusResult == 0) {
								if (numWeaponsNotReady == 0) {
									if ((numWeaponsNotLocked > 0) || (numWeaponsTooHot > 0)) {
										if ((getMovePath()->numSteps == 0) && !getMoveTwisting())
											setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
										}
									else {
										recalcPath = true;
										moveParams = MOVEPARAM_SOMEWHERE_ELSE;
									}
								}
                            }
							break;
					}
				}
				if (!recalcPath && (curTacOrder.attackParams.tactic != TACTIC_STOP_AND_FIRE))
					if (keepMoving && curTacOrder.moveParams.keepMoving)
						if (RandomNumber(100) < 60) {
							recalcPath = true;
							moveParams = MOVEPARAM_SOMEWHERE_ELSE + MOVEPARAM_RANDOM_OPTIMAL;
						}
				if (recalcPath && (curTacOrder.attackParams.tactic != TACTIC_TURRET)) {
//					setMoveStateGoal(MOVESTATE_FORWARD);
					if (target) {
						setMoveGoal(target->getWatchID(), &targetPosition, target);
						if (curTacOrder.moveParams.jump)
							moveParams |= MOVEPARAM_JUMP;
						requestMovePath(curTacOrder.selectionIndex, moveParams | MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET, 10);
						}
					else {
						setMoveGoal(MOVEGOAL_LOCATION, &targetPosition);
						unsigned long params = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
						if (curTacOrder.moveParams.jump)
							params |= MOVEPARAM_JUMP;
						requestMovePath(curTacOrder.selectionIndex, params, 11);
					}
				}
			}
			}
		else {
			GameObjectPtr target = getLastTarget();
			if (target)
				setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
		}
		}
	else if (goalType == MOVEGOAL_LOCATION) {
		if (!isYielding() && !isJumping() && !getMoveTwisting() && !isWaitingForPoint() && (getMovePath()->numSteps == 0)) {
			//---------------------------------------------
			// We may be so close that we don't need a move
			// path...
			if (!getMovePathRequest()) {
				unsigned long params = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
				if (curTacOrder.moveParams.jump)
					params |= MOVEPARAM_JUMP;
				requestMovePath(curTacOrder.selectionIndex, params, 12);
			}
		}
		}
	else {
		//-------------------------------------------------------
		// Check if our target is still around. If not, clear our
		// current move goal...
		if (goalObject) {
			//---------------------------------------------------------------
			// We may want to call a modified orderMoveToObject routine (but,
			// we don't want to play with the tacOrders)...
			Stuff::Vector3D targetPos = goalObject->getPosition();
			if (!curTacOrder.isCombatOrder()) {
				if (distance_from(targetPos, moveOrders.goalObjectPosition) > 50.0) {
					Stuff::Vector3D pos = goalObject->getPosition();
					setMoveGoal(goalObject->getWatchID(), &pos, goalObject);
					unsigned long params = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
					if (curTacOrder.moveParams.jump)
						params |= MOVEPARAM_JUMP;
					requestMovePath(curTacOrder.selectionIndex, params, 13);
					}
				else if (getMovePath()->numStepsWhenNotPaused == 0) {
					Stuff::Vector3D pos = goalObject->getPosition();
					setMoveGoal(goalObject->getWatchID(), &pos, goalObject);
					unsigned long params = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
					if (curTacOrder.moveParams.jump)
						params |= MOVEPARAM_JUMP;
					requestMovePath(curTacOrder.selectionIndex, params, 13);
				}
				}
			else {
				GameObjectPtr target = getCurrentTarget();
				//float relFacing = myVehicle->relViewFacingTo(targetPos);
				if (target && (target == goalObject) && getAttackPursuit()) {
					bool recalcPath = false;
					unsigned long moveParams = MOVEPARAM_NONE;

					//---------------------------------------------------------------
					// We will want to re-evaluate optimal range if our optimal range
					// has changed...
					if (target->isMover()) {
						if (((MoverPtr)target)->getLastWeaponEffectivenessCalc() > myVehicle->getLastOptimalRangeCalc())
							if (myVehicle->calcFireRanges()) {
								recalcPath = true;
								moveParams = MOVEPARAM_SOMEWHERE_ELSE;
							}
						}
					else {
						if (getMovePath()->numSteps > 0)
							return(true);
					}

					float distanceToTarget = myVehicle->distanceFrom(target->getPosition());
					float desiredAttackRange = myVehicle->getOrderedFireRange();
					float attackRangeDelta = distanceToTarget - desiredAttackRange;
					float recalcDelta = Terrain::worldUnitsPerVertex * metersPerWorldUnit;
					if ((curTacOrder.attackParams.tactic != TACTIC_STOP_AND_FIRE) && (curTacOrder.attackParams.tactic != TACTIC_TURRET) && ((attackRangeDelta > recalcDelta) || (attackRangeDelta < -recalcDelta))) {
						//----------------------------------------
						// Try to maintain attack range...
						recalcPath = true;
						moveParams = MOVEPARAM_SOMEWHERE_ELSE;
						}
					else if ((curTacOrder.attackParams.tactic != TACTIC_TURRET) && (weaponsStatusResult == WEAPONS_STATUS_OUT_OF_RANGE)) {
						recalcPath = true;
						moveParams = MOVEPARAM_SOMEWHERE_ELSE;
						}
					else if (weaponsStatusResult >= 0) {
						long numWeaponsNotReady = 0;
						long numWeaponsOutOfRange = 0;
						long numWeaponsOutOfAmmo = 0;
						long numWeaponsNotLocked = 0;
						long numWeaponsNoChance = 0;
						long numWeaponsTooHot = 0;
						for (long i = 0; i < myVehicle->numWeapons; i++) {
							if (weaponsStatus[i] == WEAPON_STATUS_NOT_READY)
								numWeaponsNotReady++;
							if (weaponsStatus[i] == WEAPON_STATUS_OUT_OF_AMMO)
								numWeaponsOutOfAmmo++;
							if (weaponsStatus[i] == WEAPON_STATUS_OUT_OF_RANGE)
								numWeaponsOutOfRange++;
							if (weaponsStatus[i] == WEAPON_STATUS_NOT_LOCKED)
								numWeaponsNotLocked++;
							if (weaponsStatus[i] == WEAPON_STATUS_NO_CHANCE)
								numWeaponsNoChance++;
							if (weaponsStatus[i] == WEAPON_STATUS_TOO_HOT)
								numWeaponsTooHot++;
						}
						switch (myVehicle->getObjectClass()) {
							case BATTLEMECH:
								if (weaponsStatusResult == 0) {
									if (!myVehicle->lineOfSight(target)) {
										recalcPath = true;
										moveParams = MOVEPARAM_SOMEWHERE_ELSE;
										}
									else if ((numWeaponsNotReady > 0) || (numWeaponsTooHot > 0) || (numWeaponsNotLocked > 0)) {
										//-----------------------------------------------------------
										// If we already have a path, don't pivot, finish the move...
										if ((getMovePath()->numSteps == 0) && !getMoveTwisting())
											setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
										}
									else {
										recalcPath = true;
										moveParams = MOVEPARAM_SOMEWHERE_ELSE;
									}
								}
								break;
							case GROUNDVEHICLE:
								if (weaponsStatusResult == 0) {
									if (!myVehicle->lineOfSight(target)) {
										recalcPath = true;
										moveParams = MOVEPARAM_SOMEWHERE_ELSE;
										}
									else if (numWeaponsNotReady == 0) {
										if ((numWeaponsNotLocked > 0) || (numWeaponsTooHot > 0)) {
											if ((getMovePath()->numSteps == 0) && !getMoveTwisting())
												setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
											}
										else {
											recalcPath = true;
											moveParams = MOVEPARAM_STEP_TOWARD_TARGET;
										}
									}
                                }
								break;
							case ELEMENTAL:
								if (weaponsStatusResult == 0) {
									if (numWeaponsNotReady == 0) {
										if ((numWeaponsNotLocked > 0) || (numWeaponsTooHot > 0)) {
											if ((getMovePath()->numSteps == 0) && !getMoveTwisting())
												setMoveStateGoal(MOVESTATE_PIVOT_TARGET);
											}
										else {
											recalcPath = true;
											moveParams = MOVEPARAM_SOMEWHERE_ELSE;
										}
									}
                                }
								break;
						}
					}
					//--added 6/3/01 by GD
					if (!recalcPath && (curTacOrder.attackParams.tactic != TACTIC_STOP_AND_FIRE))
						if (keepMoving && curTacOrder.moveParams.keepMoving)
							if (RandomNumber(100) < 60) {
								recalcPath = true;
								moveParams = MOVEPARAM_SOMEWHERE_ELSE + MOVEPARAM_RANDOM_OPTIMAL;
							}
					//--
					if (recalcPath && (curTacOrder.attackParams.tactic != TACTIC_TURRET)) {
//						setMoveStateGoal(MOVESTATE_FORWARD);
						Stuff::Vector3D pos = target->getPosition();
						setMoveGoal(target->getWatchID(), &pos, target);
						if (curTacOrder.moveParams.jump)
							moveParams |= MOVEPARAM_JUMP;
						requestMovePath(curTacOrder.selectionIndex, moveParams | MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET, 14);
					}
				}
			}
		}
	}

	return(true);
}

//---------------------------------------------------------------------------
// MAIN DECISION TREE
//---------------------------------------------------------------------------

void MechWarrior::clearCurTacOrder (bool updateTacOrder) {

	if (curTacOrder.isCombatOrder())
	{
		numWarriorsInCombat--;
	}

	gosASSERT(numWarriorsInCombat >= 0);

	if (getVehicle())
		getVehicle()->suppressionFire = 0;

	curTacOrder.init();
	clearMoveOrders();
	triggerAlarm(PILOT_ALARM_NO_MOVEPATH, MOVEPATH_ERR_TACORDER_CLEARED);
	clearAttackOrders();
	lastTacOrder.lastTime = scenarioTime;

	if (updateTacOrder) {
		TacticalOrder newTacOrder;
		switch (orderState) {
			case ORDERSTATE_GENERAL:
				if (!newTacOrderReceived[ORDERSTATE_GENERAL])
					tacOrder[ORDERSTATE_GENERAL].init();
				//--------------------------------------------
				// If we reach this, then we have no orders...
				break;
			case ORDERSTATE_PLAYER:
				if (!newTacOrderReceived[ORDERSTATE_PLAYER]) {
					tacOrder[ORDERSTATE_PLAYER].init();
					curPlayerOrderFromQueue = false;
				}
				newTacOrder = tacOrder[ORDERSTATE_GENERAL];
				orderState = ORDERSTATE_GENERAL;
				break;
			case ORDERSTATE_ALARM:
				if (!newTacOrderReceived[ORDERSTATE_ALARM])
					tacOrder[ORDERSTATE_ALARM].init();
				alarmPriority = 0;
				if (tacOrder[ORDERSTATE_PLAYER].code != TACTICAL_ORDER_NONE) {
					newTacOrder = tacOrder[ORDERSTATE_PLAYER];
					orderState = ORDERSTATE_PLAYER;
					}
				else {
					newTacOrder = tacOrder[ORDERSTATE_GENERAL];
					orderState = ORDERSTATE_GENERAL;
				}
				break;

		}
		if (newTacOrder.code != TACTICAL_ORDER_NONE) {
			Assert((moveOrders.path[0] != NULL) && (moveOrders.path[1] != NULL), 0, " bad warrior path ");
			MovePathPtr tempPath[2];
			for (long i = 0; i < 2; i++) {
				tempPath[i] = moveOrders.path[i];
				tempPath[i]->numSteps = 0;
			}
			long tempMoveState = moveOrders.moveState;
			long tempMoveStateGoal = moveOrders.moveStateGoal;
			moveOrders.init();
			PathManager->remove(this);
			moveOrders.moveState = tempMoveState;
			moveOrders.moveStateGoal = tempMoveStateGoal;
			moveOrders.path[0] = tempPath[0];
			moveOrders.path[1] = tempPath[1];
			attackOrders.init();
			long message = -1;
			newTacOrder.execute(this, message);
			if (orderState == ORDERSTATE_PLAYER)
				radioMessage(message, true);
			setCurTacOrder(newTacOrder);
			if (useGoalPlan) {
				Stuff::Vector3D movePoint = newTacOrder.getWayPoint(0);
				if (!newTacOrder.subOrder)
					switch (newTacOrder.code) {
						case TACTICAL_ORDER_MOVETO_POINT:
							setMainGoal(GOAL_ACTION_MOVE, NULL, &movePoint, -1.0);
							break;
						case TACTICAL_ORDER_ATTACK_OBJECT:
							setMainGoal(GOAL_ACTION_ATTACK, newTacOrder.getTarget(), NULL, -1.0);
							break;
						case TACTICAL_ORDER_CAPTURE:
							setMainGoal(GOAL_ACTION_CAPTURE, newTacOrder.getTarget(), NULL, 400.0);
							break;
						default:
							setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
					}
			}
			timeOfLastOrders = -1.0;
		}
		//-------------------------------------
		// These must be cleared every frame...
		newTacOrderReceived[ORDERSTATE_GENERAL] = false;
		newTacOrderReceived[ORDERSTATE_PLAYER] = false;
		newTacOrderReceived[ORDERSTATE_ALARM] = false;
	}


}

//---------------------------------------------------------------------------
bool MechWarrior::anyPlayerInCombat (void)
{
	for (long i=0;i<MAX_WARRIORS;i++)
	{
		if (warriorList[i] && 
			(warriorList[i]->getVehicle() && (warriorList[i]->getVehicle()->getCommanderId() == Commander::home->getId())) &&
			(warriorList[i]->getCurrentTarget()))
		{
			return true;
		}
	}
	
	return false;
}

//---------------------------------------------------------------------------

void MechWarrior::setCurTacOrder (TacticalOrder newTacOrder) {

	curTacOrder = newTacOrder;
	lastTacOrder = newTacOrder;
	if (curTacOrder.isCombatOrder())
	{
		numWarriorsInCombat++;
	}
}

//---------------------------------------------------------------------------

void MechWarrior::setGeneralTacOrder (TacticalOrder newTacOrder) {

	tacOrder[ORDERSTATE_GENERAL] = newTacOrder;
	newTacOrderReceived[ORDERSTATE_GENERAL] = true;
}

//---------------------------------------------------------------------------

void MechWarrior::setPlayerTacOrder (TacticalOrder newTacOrder, bool fromQueue) {

	
	tacOrder[ORDERSTATE_PLAYER] = newTacOrder;
	newTacOrderReceived[ORDERSTATE_PLAYER] = true;
	curPlayerOrderFromQueue = fromQueue;
	if (!fromQueue)
		clearTacOrderQueue();
}

//---------------------------------------------------------------------------

void MechWarrior::setAlarmTacOrder (TacticalOrder newTacOrder, long priority) {

	if (priority >= alarmPriority) {
		tacOrder[ORDERSTATE_ALARM] = newTacOrder;
		newTacOrderReceived[ORDERSTATE_ALARM] = true;
		alarmPriority = priority;
	}
}

//---------------------------------------------------------------------------

long MechWarrior::triggerAlarm (long alarmCode, unsigned long triggerId) {

	if (alarm[alarmCode].numTriggers == MAX_ALARM_TRIGGERS)
		return(-1);
	if (alarmHistory[alarmCode].numTriggers == MAX_ALARM_TRIGGERS)
		return(-1);

	alarm[alarmCode].trigger[alarm[alarmCode].numTriggers++] = triggerId;
	alarmHistory[alarmCode].trigger[alarmHistory[alarmCode].numTriggers++] = triggerId;
	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::getEventHistory (long alarmCode, int* paramList) {

	long numValidTriggers = 0;
	switch (alarmCode) {
		case PILOT_ALARM_ATTACK_ORDER:
		case PILOT_ALARM_DEATH_OF_MATE:
		case PILOT_ALARM_KILLED_TARGET:
		case PILOT_ALARM_FRIENDLY_VEHICLE_CRIPPLED:
		case PILOT_ALARM_VEHICLE_DESTROYED:
		case PILOT_ALARM_MATE_FIRED_WEAPON:
		case PILOT_ALARM_COLLISION:
		case PILOT_ALARM_TARGET_OF_WEAPONFIRE:
		case PILOT_ALARM_FIRED_WEAPON:
		{
			for (long i = 0; i < alarmHistory[alarmCode].numTriggers; i++) 
			{
				GameObjectPtr obj = ObjectManager->getByWatchID(alarmHistory[alarmCode].trigger[i]);
				if (obj)
					paramList[numValidTriggers++] = obj->getPartId();
			}
		}
		break;
		case PILOT_ALARM_HIT_BY_WEAPONFIRE: {
			for (long i = 0; i < alarmHistory[alarmCode].numTriggers; i++)
				if (alarmHistory[alarmCode].trigger[i] < 0)
					paramList[numValidTriggers++] = alarmHistory[alarmCode].trigger[i];
				else {
					GameObjectPtr obj = ObjectManager->getByWatchID(alarmHistory[alarmCode].trigger[i]);
					if (obj)
						paramList[numValidTriggers++] = obj->getPartId();
				}
			}
			break;
		default:
			for (long i = 0; i < alarmHistory[alarmCode].numTriggers; i++)
				paramList[i] = alarmHistory[alarmCode].trigger[i];
			numValidTriggers = alarmHistory[alarmCode].numTriggers;
			break;
	}
	return(numValidTriggers);
}

//-----------------------------------------------------------------------------

long MechWarrior::getNextEventHistory (int* paramList) {

	if (curEventID < NUM_PILOT_ALARMS) {
		while ((curEventID < NUM_PILOT_ALARMS) && (alarmHistory[curEventID].numTriggers == 0))
			curEventID++;
		if (curEventID < NUM_PILOT_ALARMS) {
			getEventHistory(curEventID, paramList);
			curEventID++;
			return(curEventID - 1);
		}
	}
	return(-1);
}

//---------------------------------------------------------------------------

long MechWarrior::handleAlarm (long alarmCode, unsigned long triggerId) {

	MoverPtr myVehicle = getVehicle();
	Assert(myVehicle != NULL, 0, " bad vehicle for pilot ");

	if (!myVehicle->getAwake())
		return(NO_ERR);

	//------------------------------------------------------------------
	// Handle Alarm Events. Only three alarms may be called outside the
	// normal pilot alarm procedure: basically, the three that cause the
	// mover to be wiped from the game before the normal pilot alarm
	// update would be called.
	switch (alarmCode) {
		case PILOT_ALARM_VEHICLE_INCAPACITATED:
			//----------------------------------------------------------
			// NOTE: This will never happen, since a disabled mech won't
			// get into this routine. The alarm is handled IMMEDIATELY
			// when it happens...
			handleOwnVehicleIncapacitation(triggerId);
			break;
		case PILOT_ALARM_VEHICLE_DESTROYED:
			//-----------------------------------------------------------
			// NOTE: This will never happen, since a destroyed mech won't
			// get into this routine. The alarm is handled IMMEDIATELY
			// when it happens...
			handleOwnVehicleDestruction(triggerId);
			break;
		case PILOT_ALARM_VEHICLE_WITHDRAWN:
			//-----------------------------------------------------------
			// NOTE: This will never happen, since a withdrawn mech won't
			// get into this routine. The alarm is handled IMMEDIATELY
			// when it happens...
			handleOwnVehicleWithdrawn();
			break;
	}

	//-----------------------------------------------------
	// After applying game-system effects for the alarm,
	// check for ABL-handler in the pilot's brain module...
	if (MPlayer && !MPlayer->isServer())
		return(NO_ERR);

	if (brainAlarmCallback[alarmCode]) {
		CurGroup = getGroup();
		CurObject = (GameObjectPtr)getVehicle();
		CurObjectClass = getVehicle()->getObjectClass();
		CurWarrior = this;
		CurContact = NULL;
		CurAlarm = alarmCode;

		brain->execute(NULL, brainAlarmCallback[alarmCode]);
		
		CurGroup = NULL;
		CurObject = NULL;
		CurObjectClass = 0;
		CurWarrior = NULL;
		CurContact = NULL;
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::getAlarmTriggers (long alarmCode, unsigned long* triggerList) {

	memcpy(triggerList, alarm[alarmCode].trigger, sizeof(unsigned long) * alarm[alarmCode].numTriggers);
	return(alarm[alarmCode].numTriggers);
}

//---------------------------------------------------------------------------

long MechWarrior::getAlarmTriggersHistory (long alarmCode, unsigned long* triggerList) {

	memcpy(triggerList, alarmHistory[alarmCode].trigger, sizeof(unsigned long) * alarmHistory[alarmCode].numTriggers);
	return(alarmHistory[alarmCode].numTriggers);
}

//---------------------------------------------------------------------------

void MechWarrior::clearAlarm (long alarmCode) {

	alarm[alarmCode].numTriggers = 0;
}

//---------------------------------------------------------------------------

void MechWarrior::clearAlarms (void) {

	for (long code = 0; code < NUM_PILOT_ALARMS; code++)
		clearAlarm(code);
}

//---------------------------------------------------------------------------

void MechWarrior::clearAlarmHistory (long alarmCode) {

	alarmHistory[alarmCode].numTriggers = 0;
}

//---------------------------------------------------------------------------

void MechWarrior::clearAlarmsHistory (void) {

	for (long code = 0; code < NUM_PILOT_ALARMS; code++)
		clearAlarmHistory(code);
}

//---------------------------------------------------------------------------

long MechWarrior::checkAlarms (void) {

	//-------------------------------------------
	// In case we call ABL-handlers, set these...
	if (brain) {
		CurGroup = getGroup();
		CurObject = (GameObjectPtr)getVehicle();
		CurObjectClass = getVehicle()->getObjectClass();
		CurWarrior = this;
		CurContact = NULL;
	}

	//-----------------------
	// Handle Alarm Events...
	for (long code = 0; code < NUM_PILOT_ALARMS; code++)
		if (alarm[code].numTriggers > 0) {
			switch (code) {
				case PILOT_ALARM_TARGET_OF_WEAPONFIRE:
					handleTargetOfWeaponFire();
					break;
				case PILOT_ALARM_HIT_BY_WEAPONFIRE:
					handleHitByWeaponFire();
					break;
#ifdef USEHEAT
				case PILOT_ALARM_OVERHEAT:
					handleOverHeat();
					break;
#endif

				case PILOT_ALARM_DAMAGE_TAKEN_RATE:
					handleDamageTakenRate();
					break;
				case PILOT_ALARM_DEATH_OF_MATE:
					handleUnitMateDeath();
					break;
				case PILOT_ALARM_FRIENDLY_VEHICLE_CRIPPLED:
					handleFriendlyVehicleCrippled();
					break;
				case PILOT_ALARM_FRIENDLY_VEHICLE_DESTROYED:
					handleFriendlyVehicleDestruction();
					break;
				case PILOT_ALARM_VEHICLE_INCAPACITATED:
					//----------------------------------------------------------
					// NOTE: This will never happen, since a disabled mech won't
					// get into this routine. Thei alarm is handled IMMEDIATELY
					// when it happens...
					handleOwnVehicleIncapacitation(0);
					break;
				case PILOT_ALARM_VEHICLE_DESTROYED:
					//----------------------------------------------------------
					// NOTE: This will never happen, since a disabled mech won't
					// get into this routine. Thei alarm is handled IMMEDIATELY
					// when it happens...
					handleOwnVehicleDestruction(0);
					break;
				case PILOT_ALARM_VEHICLE_WITHDRAWN:
					//----------------------------------------------------------
					// NOTE: This will never happen, since a disabled mech won't
					// get into this routine. Thei alarm is handled IMMEDIATELY
					// when it happens...
					handleOwnVehicleWithdrawn();
					break;
				case PILOT_ALARM_ATTACK_ORDER:
					handleAttackOrder();
					break;
				case PILOT_ALARM_COLLISION:
					handleCollision();
					break;
				case PILOT_ALARM_KILLED_TARGET:
					handleKilledTarget();
					break;
				case PILOT_ALARM_MATE_FIRED_WEAPON:
					handleUnitMateFiredWeapon();
					break;
				case PILOT_ALARM_PLAYER_ORDER:
					handlePlayerOrder();
					break;
				case PILOT_ALARM_NO_MOVEPATH:
					handleNoMovePath();
					break;
				case PILOT_ALARM_GATE_CLOSING:
					handleGateClosing();
					break;
				case PILOT_ALARM_FIRED_WEAPON:
					handleFiredWeapon();
					break;
				case PILOT_ALARM_NEW_MOVER:
					handleNewMover();
					break;
			}
			//------------------------------------------------------
			// After applying game-system effects for the alarm,
			// check for ABL-handlers in the pilot's brain module...
			if (MPlayer) {
				if (MPlayer->isServer())
					if (brain && brainAlarmCallback[code]) {
						CurAlarm = code;
						brain->execute(NULL, brainAlarmCallback[code]);
					}
				}
			else {
				if (brain && brainAlarmCallback[code]) {
					CurAlarm = code;
					brain->execute(NULL, brainAlarmCallback[code]);
				}
			}
			//----------------------------------
			// Clear these alarms every frame...
			alarm[code].numTriggers = 0;
		}

	if (brain) {
		CurGroup = NULL;
		CurObject = NULL;
		CurObjectClass = 0;
		CurWarrior = NULL;
		CurContact = NULL;
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

void MechWarrior::updateActions (void) {

	if ((teamId == -1) || !brainsEnabled[teamId]) {
		clearCurTacOrder(false);
		clearMoveOrders();
		clearAttackOrders();
		tacOrder[ORDERSTATE_GENERAL].init();
		tacOrder[ORDERSTATE_PLAYER].init();
		tacOrder[ORDERSTATE_ALARM].init();
		setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
		return;
	}

	MoverPtr myVehicle = getVehicle();

	if (myVehicle->isCaptured()) {
		clearCurTacOrder();
		setLastTarget(NULL);
		return;
	}

	if (combatUpdate <= scenarioTime)
		combatDecisionTree();

	movementDecisionTree();
}

//extern L_INTEGER startCk;
//extern L_INTEGER endCk;
//extern long		 srWAblUpd;
//---------------------------------------------------------------------------

long MechWarrior::mainDecisionTree (void) {

    // sebi: WTF??? path is an array of2 elements => comparison always true
	//Assert(moveOrders.path != NULL, 0, " bad warrior path ");
	Assert(moveOrders.path[0] != NULL, 0, " bad warrior path ");

	if ((teamId == -1) || !brainsEnabled[teamId]) {
		clearCurTacOrder(false);
		tacOrder[ORDERSTATE_GENERAL].init();
		tacOrder[ORDERSTATE_PLAYER].init();
		tacOrder[ORDERSTATE_ALARM].init();
		setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
	}

	//---------------------------------------------------------------------------
	//If we currently have no order, check if we have any player orders queued...
	if (curTacOrder.code == TACTICAL_ORDER_NONE) {
		if (tacOrderQueueExecuting && (numTacOrdersQueued > 0)) {
			if (!newTacOrderReceived[ORDERSTATE_PLAYER]) {
				//--------------------------------------------------------------------
				// First queued order just given, so begin executing it immediately...
				executeTacOrderQueue();
			}
		}
	}
	
	//---------------------------------------
	// Is the current tac order accomplished?
	if ((curTacOrder.code != TACTICAL_ORDER_NONE) && (curTacOrder.status(this) == TACORDER_SUCCESS)) {
		bool playerOrderQueued = false;
		if ((orderState == ORDERSTATE_PLAYER) && tacOrderQueueExecuting) {
			executeTacOrderQueue();
			playerOrderQueued = newTacOrderReceived[ORDERSTATE_PLAYER];
		}
		clearCurTacOrder();
		newTacOrderReceived[ORDERSTATE_PLAYER] = playerOrderQueued;
	}

	//------------------------------------------------------
	// If we're without orders now, let's record the time...
	if (underHomeCommand() && (curTacOrder.code == TACTICAL_ORDER_NONE) && (timeOfLastOrders < 0.0))
		timeOfLastOrders = scenarioTime;

	//-----------------------------------------------
	// Update Weapons Status, if needed this frame...
	if ((brainUpdate <= scenarioTime) || (combatUpdate <= scenarioTime) || (movementUpdate <= scenarioTime)) {
		GameObjectPtr target = getCurrentTarget(); //getAttackTarget(ORDER_CURRENT);
		if (target)
			weaponsStatusResult = calcWeaponsStatus(target, weaponsStatus);
		else if (curTacOrder.code == TACTICAL_ORDER_ATTACK_POINT) {
			Stuff::Vector3D pos = getAttackTargetPoint();
			weaponsStatusResult = calcWeaponsStatus(target, weaponsStatus, &pos);
		}
	}

	//-------------------------------------------------------------------------------------
	// Make sure, if I have a target, it's still legit.
	// Now, when we go into runBrain, we know the current target is up-to-date with current
	// game conditions...
	GameObjectPtr tacOrderTarget = NULL;
	if (curTacOrder.code != TACTICAL_ORDER_NONE)
		tacOrderTarget = ObjectManager->getByWatchID(curTacOrder.targetWID);
	GameObjectPtr target = getLastTarget();
	if (target) {
		bool clearTarget = false;
		if (target->isDestroyed())
			clearTarget = true;
		else if (target->isDisabled() && !lastTargetObliterate)
			clearTarget = true;
		if (clearTarget) {
			setLastTarget(NULL);
			if ((curTacOrder.code == TACTICAL_ORDER_ATTACK_OBJECT) && (target == tacOrderTarget)) {
				clearCurTacOrder();
				target = NULL;
				tacOrderTarget = NULL;
			}
		}
	}
	if ((curTacOrder.code == TACTICAL_ORDER_ATTACK_OBJECT) && (target != tacOrderTarget))
		setLastTarget(tacOrderTarget);
	
	//----------------------
	// Update pilot brain...
	if ((brainUpdate <= scenarioTime) && ((teamId == -1) || brainsEnabled[teamId])) {
		runBrain();
		brainUpdate += BrainUpdateFrequency;
	}

	//------------------------------------------------------------------
	// In case the brain set a new target, let's recalc weaponsStatus...
	target = getLastTarget(); //getAttackTarget(ORDER_CURRENT);
	if (target && (lastTargetTime/*getLastTargetTime()*/ == scenarioTime))
		weaponsStatusResult = calcWeaponsStatus(target, weaponsStatus);

	//------------
	// Any Alarms?
	checkAlarms();

	//-------------------------------------------------
	// Now, check if we have a new current tac order...
	TacticalOrder newTacOrder;
	switch (orderState) {
		case ORDERSTATE_GENERAL:
			if (newTacOrderReceived[ORDERSTATE_ALARM]) {
				clearCurTacOrder(false);
				newTacOrder = tacOrder[ORDERSTATE_ALARM];
				orderState = ORDERSTATE_ALARM;
				}
			else if (newTacOrderReceived[ORDERSTATE_PLAYER]) {
				//--------------------------------------------------------
				// If we're currently executing a general (brain) order,
				// purge it permanently (NOW, we DON'T want them to
				// fall back to the general order--the brain must re-order
				// it)...
  				tacOrder[ORDERSTATE_GENERAL].init();
				//-----------------------------
				// Now grab the player order...
				newTacOrder = tacOrder[ORDERSTATE_PLAYER];
				orderState = ORDERSTATE_PLAYER;
				}
			else if (newTacOrderReceived[ORDERSTATE_GENERAL])
				newTacOrder = tacOrder[ORDERSTATE_GENERAL];
			break;
		case ORDERSTATE_PLAYER:
			if (newTacOrderReceived[ORDERSTATE_ALARM]) {
				newTacOrder = tacOrder[ORDERSTATE_ALARM];
				orderState = ORDERSTATE_ALARM;
				}
			else if (newTacOrderReceived[ORDERSTATE_PLAYER]) {
				//--------------------------------------------------------
				// Make sure we have no general order we fall back to when
				// we finish this, per Tim's request...
  				tacOrder[ORDERSTATE_GENERAL].init();
				newTacOrder = tacOrder[ORDERSTATE_PLAYER];
			}
			break;
		case ORDERSTATE_ALARM:
			//--------------------------------------------------------------
			// If the player issues a new order, let it override any current
			// alarm (self-issued) order...
			if (newTacOrderReceived[ORDERSTATE_PLAYER]) {
				alarmPriority = 0;
  				tacOrder[ORDERSTATE_ALARM].init();
				newTacOrder = tacOrder[ORDERSTATE_PLAYER];
				}
			else if (newTacOrderReceived[ORDERSTATE_ALARM])
				newTacOrder = tacOrder[ORDERSTATE_ALARM];
			break;
	}
	if (newTacOrder.code != TACTICAL_ORDER_NONE) {
		//------------------------------------------------------
		// Note how we need to copy the move path over, since we
		// allocate it only once now...
		MovePathPtr tempPath[2];
		if ((newTacOrder.code == TACTICAL_ORDER_MOVETO_POINT) || (newTacOrder.code == TACTICAL_ORDER_MOVETO_OBJECT)) {
			for (long i = 0; i < 2; i++) {
				tempPath[i] = moveOrders.path[i];
				if (i > 0)
					tempPath[i]->numSteps = 0;
			}
			}
		else {
			for (long i = 0; i < 2; i++) {
				tempPath[i] = moveOrders.path[i];
				tempPath[i]->numSteps = 0;
			}
		}
		long tempMoveState = moveOrders.moveState;
		long tempMoveStateGoal = moveOrders.moveStateGoal;
		bool running = moveOrders.run;
		moveOrders.init();
		moveOrders.run = running;
		PathManager->remove(this);
		moveOrders.moveState = tempMoveState;
		moveOrders.moveStateGoal = tempMoveStateGoal;
		moveOrders.path[0] = tempPath[0];
		moveOrders.path[1] = tempPath[1];
		attackOrders.init();
		long message = -1;
		newTacOrder.execute(this, message);
		if (orderState == ORDERSTATE_PLAYER)
			radioMessage(message, true);
		setCurTacOrder(newTacOrder);
		if (useGoalPlan) {
			Stuff::Vector3D movePoint = newTacOrder.getWayPoint(0);
			if (!newTacOrder.subOrder)
				switch (newTacOrder.code) {
					case TACTICAL_ORDER_MOVETO_POINT:
						setMainGoal(GOAL_ACTION_MOVE, NULL, &movePoint, -1.0);
						break;
					case TACTICAL_ORDER_ATTACK_OBJECT:
						setMainGoal(GOAL_ACTION_ATTACK, newTacOrder.getTarget(), NULL, -1.0);
						break;
					case TACTICAL_ORDER_CAPTURE:
						setMainGoal(GOAL_ACTION_CAPTURE, newTacOrder.getTarget(), NULL, 400.0);
						break;
					default:
						setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
				}
		}
		timeOfLastOrders = -1.0;
	}

	//-------------------------------------
	// These must be cleared every frame...
	newTacOrderReceived[ORDERSTATE_GENERAL] = false;
	newTacOrderReceived[ORDERSTATE_PLAYER] = false;
	newTacOrderReceived[ORDERSTATE_ALARM] = false;

	//--------------------------------------------
	// Now that we know what we're doing, do it...
	updateActions();

	return(NO_ERR);
}

//---------------------------------------------------------------------------

void MechWarrior::setDebugFlag (unsigned long flag, bool on) {

	if (on)
		debugFlags |= flag;
	else
		debugFlags &= (flag ^ 0xFFFFFFFF);
}

//---------------------------------------------------------------------------

bool MechWarrior::getDebugFlag (unsigned long flag) {

	return((debugFlags & flag) != 0);
}

//---------------------------------------------------------------------------

void MechWarrior::debugPrint (char* s, bool debugMode) {

	if (debugger) {
		debugger->print(s);
		if (debugMode)
			debugger->debugMode();
		}
}

//---------------------------------------------------------------------------

void MechWarrior::debugOrders (void) {

	char s[256];

	switch (curTacOrder.code) {
		case TACTICAL_ORDER_NONE:
			sprintf(s, "CURRENT ORDERS: None");
			break;
		case TACTICAL_ORDER_WAIT:
			sprintf(s, "CURRENT ORDERS: Wait");
			break;
		case TACTICAL_ORDER_MOVETO_POINT:
			sprintf(s, "CURRENT ORDERS: Move to (%.2f, %.2f, %.2f)", curTacOrder.getWayPoint(0).x, curTacOrder.getWayPoint(0).y, curTacOrder.getWayPoint(0).z);
			break;
		case TACTICAL_ORDER_MOVETO_OBJECT: {
			GameObjectPtr obj = ObjectManager->getByWatchID(curTacOrder.targetWID);
			sprintf(s, "CURRENT ORDERS: Move To Object %d", obj ? obj->getPartId() : 0);
			}
			break;
		case TACTICAL_ORDER_JUMPTO_POINT:
			sprintf(s, "CURRENT ORDERS: Jump to (%.2f, %.2f, %.2f)", curTacOrder.getWayPoint(0).x, curTacOrder.getWayPoint(0).y, curTacOrder.getWayPoint(0).z);
			break;
		case TACTICAL_ORDER_JUMPTO_OBJECT: {
			GameObjectPtr obj = ObjectManager->getByWatchID(curTacOrder.targetWID);
			sprintf(s, "CURRENT ORDERS: Jump To Object %d", obj ? obj->getPartId() : 0);
			}
			break;
		case TACTICAL_ORDER_TRAVERSE_PATH:
			sprintf(s, "CURRENT ORDERS: Traverse Path");
			break;
		case TACTICAL_ORDER_PATROL_PATH:
			sprintf(s, "CURRENT ORDERS: Patrol Path");
			break;
		case TACTICAL_ORDER_ESCORT:
			sprintf(s, "CURRENT ORDERS: Escort");
			break;
		case TACTICAL_ORDER_FOLLOW:
			sprintf(s, "CURRENT ORDERS: Follow");
			break;
		case TACTICAL_ORDER_GUARD:
			sprintf(s, "CURRENT ORDERS: Guard");
			break;
		case TACTICAL_ORDER_STOP:
			sprintf(s, "CURRENT ORDERS: Stop");
			break;
		case TACTICAL_ORDER_POWERUP:
			sprintf(s, "CURRENT ORDERS: Power Up");
			break;
		case TACTICAL_ORDER_POWERDOWN:
			sprintf(s, "CURRENT ORDERS: Power Down");
			break;
		case TACTICAL_ORDER_WAYPOINTS_DONE:
			sprintf(s, "CURRENT ORDERS: Waypoints Done");
			break;
		case TACTICAL_ORDER_EJECT:
			sprintf(s, "CURRENT ORDERS: Eject");
			break;
		case TACTICAL_ORDER_ATTACK_OBJECT: {
			GameObjectPtr obj = ObjectManager->getByWatchID(curTacOrder.targetWID);
			sprintf(s, "CURRENT ORDERS: Attack Object %d", obj ? obj->getPartId() : 0);
			}
			break;
		case TACTICAL_ORDER_HOLD_FIRE:
			sprintf(s, "CURRENT ORDERS: Hold Fire");
			break;
		case TACTICAL_ORDER_WITHDRAW:
			sprintf(s, "CURRENT ORDERS: Withdraw");
			break;
		default:
			sprintf(s, "CURRENT ORDERS: Unknown Tac Order Type");
	}
	debugPrint(s);

	GameObjectPtr target = getCurrentTarget();
	sprintf(s, "     CURRENT TARGET: Object %d", target ? target->getPartId() : 0);
	debugPrint(s);
}

//---------------------------------------------------------------------------

void MechWarrior::setMoveSpeedType (long type) {

	moveOrders.speedType = type;
}

//---------------------------------------------------------------------------

void MechWarrior::setMoveSpeedVelocity (float speed) {

	moveOrders.speedVelocity = speed;
	MoverPtr myVehicle = getVehicle();

	long state, throttle;

	myVehicle->calcSpriteSpeed(speed, 0, state, throttle);
	
	moveOrders.speedState = state;
	moveOrders.speedThrottle = throttle;
}

//----------------------------------------------------------------------------

void MechWarrior::initTargetPriorities (void) {

	targetPriorityList.init();
}

//----------------------------------------------------------------------------

long MechWarrior::setTargetPriority (long index, long type, long param1, long param2, long param3) {

	targetPriorityList.list[index].type = type;
	targetPriorityList.list[index].params[0] = param1;
	targetPriorityList.list[index].params[1] = param2;
	targetPriorityList.list[index].params[2] = param3;
	if (index >= targetPriorityList.size)
		targetPriorityList.size = index + 1;
	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::insertTargetPriority (long index, long type, long param1, long param2, long param3) {

	TargetPriority priority;
	priority.type = type;
	priority.params[0] = param1;
	priority.params[1] = param2;
	priority.params[2] = param3;
	return(targetPriorityList.insert(index, &priority));
}

//---------------------------------------------------------------------------

long MechWarrior::setBrainState (long newState) {

	long oldBrainState = brainState;
	brainState = newState;
	return(oldBrainState);
}

//----------------------------------------------------------------------------

long MechWarrior::getBrainState (void) {

	return(brainState);
}

//---------------------------------------------------------------------------

#if 0
GameObjectPtr MechWarrior::selectTarget (bool moversOnly, long criteria) {
}
#endif

//---------------------------------------------------------------------------
// CORE COMMANDS (MC2 BEHAVIORS)
//---------------------------------------------------------------------------

long MechWarrior::coreMoveTo (Stuff::Vector3D location, unsigned long params) {

	bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);
	bool wait = ((params & TACORDER_PARAM_WAIT) != 0);
	bool layMines = ((params & TACORDER_PARAM_LAY_MINES) != 0);
	bool escape = ((params & TACORDER_PARAM_ESCAPE_TILE) != 0);
	bool setOrder = ((params & TACORDER_PARAM_DONT_SET_ORDER) == 0);
	bool keepMoving = ((params & TACORDER_PARAM_DONT_KEEP_MOVING) == 0);

	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_MOVETO_POINT, false);
	tacOrder.moveParams.wayPath.points[0] = location.x;
	tacOrder.moveParams.wayPath.points[1] = location.y;
	tacOrder.moveParams.wayPath.points[2] = location.z;
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.wait = wait;
	tacOrder.moveParams.mode = (layMines ? MOVE_MODE_MINELAYING : MOVE_MODE_NORMAL);
	tacOrder.moveParams.escapeTile = escape;
	tacOrder.moveParams.jump = jump;
	tacOrder.moveParams.keepMoving = keepMoving;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	if (setOrder)
		clearCurTacOrder();

	setMoveWayPath(NULL, 0);
	setMoveGoal(MOVEGOAL_LOCATION, &location);
	moveOrders.timeOfLastStep = scenarioTime;
	setMoveRun(run);
	if (setOrder)
		clearAttackOrders();

	unsigned long moveParams = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
	if (jump)
		moveParams |= MOVEPARAM_JUMP;
	PathManager->request(this, -1, moveParams, 20);
	if (setOrder) {
		setGeneralTacOrder(tacOrder);
		if (useGoalPlan)
			setMainGoal(GOAL_ACTION_MOVE, NULL, &location, -1.0);
		else
			setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
	}

	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::coreMoveToObject (GameObjectPtr object, unsigned long params) {

	bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);
	bool faceObject = ((params & TACORDER_PARAM_FACE_OBJECT) != 0);
	bool layMines = ((params & TACORDER_PARAM_LAY_MINES) != 0);
	bool setOrder = ((params & TACORDER_PARAM_DONT_SET_ORDER) == 0);
	bool keepMoving = ((params & TACORDER_PARAM_DONT_KEEP_MOVING) == 0);

	if (!object) {
		if (!coreScanTargetWID)
			coreScan(NULL, params);
		object = ObjectManager->getByWatchID(coreScanTargetWID);
	}
	if (!object)
		return(1);

	//-------------------------------------------------
	// Pick the best move goal based upon the object...
	Stuff::Vector3D goal;
	if (object->isMover())
		goal = object->getPosition();
	else {
		//------------------------------------------------
		// If not a mover, then pick an open cell adjacent
		// to it. If it happens to be adjacent to blocked
		// cells, this could be off...
		Stuff::Vector3D objectPos = object->getPosition();
		goal = getVehicle()->calcOffsetMoveGoal(objectPos);
	}
	
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_MOVETO_OBJECT, false);
	tacOrder.targetWID = object->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.fromArea = -1;
	tacOrder.moveParams.wayPath.points[0] = goal.x;
	tacOrder.moveParams.wayPath.points[1] = goal.y;
	tacOrder.moveParams.wayPath.points[2] = goal.z;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = faceObject;
	tacOrder.moveParams.wait = false;
	tacOrder.moveParams.mode = (layMines ? MOVE_MODE_MINELAYING : MOVE_MODE_NORMAL);
	tacOrder.moveParams.jump = jump;
	tacOrder.moveParams.keepMoving = keepMoving;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	if (setOrder)
		clearCurTacOrder();

	//-----------------------------------
	// Is this already our current order?
	//if ((curTacOrder.code == TACTICAL_ORDER_MOVETO_OBJECT) &&
	//	(curTacOrder.params.move.targetId == target->getIdNumber()) &&
	//	(curTacOrder.params.move.selectionIndex == selectionIndex))
	//	return(TACORDER_FAILURE);
	tacOrder.targetWID = object->getWatchID();
	Stuff::Vector3D pos = object->getPosition();
	setMoveGoal(object->getWatchID(), &pos, object);
	setMoveRun(run);

	if (setOrder)
		clearAttackOrders();

	unsigned long moveParams = MOVEPARAM_INIT;
	if (jump)
		moveParams |= MOVEPARAM_JUMP;
	if (faceObject)
		moveParams |= MOVEPARAM_FACE_TARGET;
	if (setOrder && !object->isMover())
		moveParams |= MOVEPARAM_STEP_ADJACENT_TARGET;
	requestMovePath(-1, moveParams, 15);
	moveOrders.goalObjectPosition = object->getPosition();
	if (setOrder) {
		setGeneralTacOrder(tacOrder);
		if (useGoalPlan)
			setMainGoal(GOAL_ACTION_MOVE, object, NULL, -1.0);
		else
			setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
	}

	return(0);
}

//---------------------------------------------------------------------------

#if 0

long MechWarrior::coreJumpTo (Stuff::Vector3D location, unsigned long params) {

	bool setOrder = ((params & TACORDER_PARAM_DONT_SET_ORDER) == 0);

	MoverPtr myVehicle = getVehicle();
	if (myVehicle->getJumpRange() < myVehicle->distanceFrom(location))
		return(1);

	if (myVehicle->getObjectClass() == BATTLEMECH) {
		long cellR, cellC;
		land->worldToCell(location, cellR, cellC);
		if (!GameMap->getPassable(cellR, cellC))
			return(1);
	}

	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_JUMPTO_POINT, false);
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.fromArea = -1;
	tacOrder.moveParams.wayPath.points[0] = location.x;
	tacOrder.moveParams.wayPath.points[1] = location.y;
	tacOrder.moveParams.wayPath.points[2] = location.z;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	if (setOrder) {
		clearMoveOrders();
		clearAttackOrders();
		clearCurTacOrder();
	}

	if (setOrder) {
		setGeneralTacOrder(tacOrder);
		if (useGoalPlan)
			setMainGoal(GOAL_ACTION_MOVE, object, NULL, -1.0);
		else
			setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
	}

	return(TACORDER_SUCCESS);
}

#endif

//---------------------------------------------------------------------------

long MechWarrior::coreEject (void) {

	MoverPtr myVehicle = getVehicle();
	if (myVehicle->getObjectClass() != BATTLEMECH)
		return(1);

	if (myVehicle->isDisabled())
		return(1);

	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_EJECT, false);

	//POINTLESS TO CHECK STATUS...
	//long result = tacOrder.status(this);
	//if (result == TACORDER_SUCCESS)
	//	return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	//--------------------------
	// Order has no move goal...
	clearMoveOrders();
	//------------------------------
	// Order has no attack target...
	clearAttackOrders();

	myVehicle->handleEjection();

	setGeneralTacOrder(tacOrder);
	return(1);
}

//---------------------------------------------------------------------------

long MechWarrior::corePower (bool powerUp) {

	MoverPtr myVehicle = getVehicle();
	if (myVehicle->getObjectClass() != BATTLEMECH)
		return(1);

	if (myVehicle->isDisabled())
		return(1);

	if (powerUp) {
		if (myVehicle->getStatus() == OBJECT_STATUS_NORMAL)
			return(1);
		//if (myVehicle->getStatus() == OBJECT_STATUS_STARTING_UP)
		//	return(0);
	}
	else {
		//if (myVehicle->getStatus() == OBJECT_STATUS_SHUTTING_DOWN)
		//	return(0);
		if (myVehicle->getStatus() == OBJECT_STATUS_SHUTDOWN)
			return(1);
	}

	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_COMMANDER, powerUp ? TACTICAL_ORDER_POWERUP : TACTICAL_ORDER_POWERDOWN, false);

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	//--------------------------
	// Order has no move goal...
	clearMoveOrders();
	//------------------------------
	// Order has no attack target...
	clearAttackOrders();

	//-------------------------------------------
	// Check if we've cooled enough to restart...
	if (myVehicle) {
		if (powerUp) {
			if (myVehicle->canPowerUp())
				myVehicle->startUp();
			}
		else
			myVehicle->shutDown();
	}
	setGeneralTacOrder(tacOrder);
	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::coreAttack (GameObjectPtr target, unsigned long params) {

	bool pursue = ((params & TACORDER_PARAM_PURSUE) != 0);
	bool obliterate = ((params & TACORDER_PARAM_OBLITERATE) != 0);
	//bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);
	//bool scan = ((params & TACORDER_PARAM_SCAN) != 0);
	bool keepMoving = ((params & TACORDER_PARAM_DONT_KEEP_MOVING) == 0);

	long attackMethod = ATTACKMETHOD_RANGED;
	switch (params & TACORDER_ATTACK_MASK) {
		case TACORDER_PARAM_ATTACK_DFA:
			attackMethod = ATTACKMETHOD_DFA;
			break;
		case TACORDER_PARAM_ATTACK_RAMMING:
			attackMethod = ATTACKMETHOD_RAMMING;
			break;
	}

	long attackRange = FIRERANGE_OPTIMAL;
	switch (params & TACORDER_RANGE_MASK) {
		case TACORDER_PARAM_RANGE_RAMMING:
			attackRange = FIRERANGE_RAMMING;
			break;
		case TACORDER_PARAM_RANGE_LONGEST:
			attackRange = FIRERANGE_LONGEST;
			break;
		case TACORDER_PARAM_RANGE_OPTIMAL:
			attackRange = FIRERANGE_OPTIMAL;
			break;
		case TACORDER_PARAM_RANGE_SHORT:
			attackRange = FIRERANGE_SHORT;
			break;
		case TACORDER_PARAM_RANGE_MEDIUM:
			attackRange = FIRERANGE_MEDIUM;
			break;
		case TACORDER_PARAM_RANGE_LONG:
			attackRange = FIRERANGE_LONG;
			break;
	}

	long aimLocation = -1;
	switch (params & TACORDER_AIM_MASK) {
		case TACORDER_PARAM_AIM_HEAD:
			aimLocation = MECH_BODY_LOCATION_HEAD;
			break;
		case TACORDER_PARAM_AIM_ARM:
			aimLocation = MECH_BODY_LOCATION_RARM;
			break;
		case TACORDER_PARAM_AIM_LEG:
			aimLocation = MECH_BODY_LOCATION_RLEG;
			break;
	}

	long tactic = TACTIC_NONE;
	switch (params & TACORDER_TACTIC_MASK) {
		case TACORDER_PARAM_TACTIC_FLANK_RIGHT:
			tactic = TACTIC_FLANK_RIGHT;
			break;
		case TACORDER_PARAM_TACTIC_FLANK_LEFT:
			tactic = TACTIC_FLANK_LEFT;
			break;
		case TACORDER_PARAM_TACTIC_FLANK_REAR:
			tactic = TACTIC_FLANK_REAR;
			break;
		case TACORDER_PARAM_TACTIC_STOP_AND_FIRE:
			tactic = TACTIC_STOP_AND_FIRE;
			pursue = true;
			break;
		case TACORDER_PARAM_TACTIC_TURRET:
			tactic = TACTIC_TURRET;
			pursue = false;
			jump = false;
			keepMoving = false;
			break;
		case TACORDER_PARAM_TACTIC_JOUST:
			tactic = TACTIC_JOUST;
			break;
	}

	if ((getVehicle()->numWeapons == 0) && (attackMethod != ATTACKMETHOD_RAMMING)) {
		clearAttackOrders();
		return(1);
	}

	if (!target) {
		if (!coreScanTargetWID)
			coreScan(NULL, params);
		target = ObjectManager->getByWatchID(coreScanTargetWID);
	}
	LastCoreAttackTarget = target;
	if (!target)
		return(1);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_ATTACK_OBJECT, false);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.attackParams.method = (AttackMethod)attackMethod;
	tacOrder.attackParams.aimLocation = aimLocation;
	if (attackMethod == ATTACKMETHOD_RAMMING)
		attackRange = FIRERANGE_RAMMING;
	tacOrder.attackParams.range = (FireRangeType)attackRange;
	tacOrder.attackParams.pursue = pursue;
	tacOrder.attackParams.obliterate = obliterate;
	tacOrder.attackParams.tactic = (TacticType)tactic;
	tacOrder.moveParams.keepMoving = keepMoving;
	tacOrder.moveParams.jump = jump;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	clearCurTacOrder();

	setSituationFireRange(getVehicle()->getOrderedFireRange(&attackRange));

	//------------------------------------------------------------
	// If we also want to pursue it, set up the move order here...
	if (pursue) {
		//------------------------------------------------------
		// Order moveToObject, making sure we face the object...
		params |= TACORDER_PARAM_FACE_OBJECT;
		//coreMoveToObject(target, params + TACORDER_PARAM_DONT_SET_ORDER);
		}
	else
		clearMoveOrders();

	//------------------------------------------------------------
	// Now, set up the tactical order, and pass it to the pilot...
	setAttackType(ATTACK_TO_DESTROY);
	setAttackTarget(target);
	setAttackAimLocation(aimLocation);
	setAttackPursuit(pursue);
	setSituationOpenFire(true);

	//-----------------------------------
	// Remember the most recent target...
	setLastTarget(target, obliterate);

	setGeneralTacOrder(tacOrder);

	if (useGoalPlan)
		setMainGoal(GOAL_ACTION_ATTACK, target, NULL, -1.0);
	else
		setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);

	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::coreCapture (GameObjectPtr object, unsigned long params) {

	if (!object) {
		if (!coreScanTargetWID)
			coreScan(NULL, params);
		object = ObjectManager->getByWatchID(coreScanTargetWID);
	}

	if (!getTeam())
		return(1);

	if (!object || !object->isCaptureable(getTeam()->getId()) || object->isFriendly(getVehicle()))
		return(1);
	
	//-------------------------------------------------------------------------
	// Once Goal Planning is in, this should deal with the blockers rather than
	// abort the order...
//	GameObjectPtr blockerList[MAX_MOVERS];
//	long numBlockers = object->getCaptureBlocker(getVehicle(), blockerList);
//	if (numBlockers)
//		return(1);

	bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);
	bool faceObject = ((params & TACORDER_PARAM_FACE_OBJECT) != 0);

	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_CAPTURE, false);
	tacOrder.targetWID = object->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.fromArea = -1;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = faceObject;
	tacOrder.moveParams.wait = false;
	tacOrder.moveParams.keepMoving = false;
	tacOrder.moveParams.jump = jump;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	setGeneralTacOrder(tacOrder);

	if (useGoalPlan)
		setMainGoal(GOAL_ACTION_CAPTURE, object, NULL, 250.0);
	else
		setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);

	return(0);
}

//---------------------------------------------------------------------------

long MechWarrior::coreScan (GameObjectPtr object, unsigned long params) {

	if (object && object->getTeam()) {
		//--------------------------------------------------------------
		// If this object is currently a contact, set myTarget to it and
		// return true. Otherwise, return false.
		if (object->isMover()) {
			if (getTeam()->isContact(getVehicle(), (MoverPtr)object, (long)params)) {
				coreScanTargetWID = object->getWatchID();
				return(coreScanTargetWID);
			}
			}
		else {
			if (/*CurWarrior->*/getVehicle()->lineOfSight(object)) {
				coreScanTargetWID = object->getWatchID();
				return(coreScanTargetWID);
			}
		}
		}
	else {
		//-------------------------------------------
		// Scan for anything matching the criteria...
		Stuff::Vector3D myPosition = getVehicle()->getPosition();
		long action = -1;
		coreScanTargetWID = targetPriorityList.calcTarget(this, myPosition, params, action);
		return(coreScanTargetWID);
	}
	return(coreScanTargetWID = 0);
}

//---------------------------------------------------------------------------

long MechWarrior::coreControl (GameObjectPtr object, unsigned long params) {

	if (!object) {
		if (!coreScanTargetWID)
			coreScan(NULL, params);
		object = ObjectManager->getByWatchID(coreScanTargetWID);
	}
	if (!object)
		return(1);

	long action = targetPriorityList.calcAction(this, object);
	if (action == TACTICAL_ORDER_ATTACK_OBJECT)
		return(coreAttack(object, params));
	else if (action == TACTICAL_ORDER_CAPTURE)
		return(coreCapture(object, params));
	return(1);
}

//---------------------------------------------------------------------------

#if 0

long MechWarrior::coreWithdraw (Stuff::Vector3D location, unsigned long params) {

	bool run = ((params & TACORDER_PARAM_RUN) != 0);

	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_WITHDRAW, false);
	tacOrder.moveParams.wayPath.points[0] = location.x;
	tacOrder.moveParams.wayPath.points[1] = location.y;
	tacOrder.moveParams.wayPath.points[2] = location.z;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(1);

	if (curTacOrder.equals(&tacOrder))
		return(0);

	clearCurTacOrder();

	//location = calcWithdrawGoal();
	//result = orderMoveToPoint(false, false/*true*/, ORDER_ORIGIN_COMMANDER, location, -1, run ? TACORDER_PARAM_RUN : TACORDER_PARAM_NONE);
	MoverPtr myVehicle = getVehicle();
	Assert(myVehicle != NULL, 0, " orderWithdraw:Warrior has no Vehicle ");

	myVehicle->withdrawing = true;

	setGeneralTacOrder(tacOrder);
	if (useGoalPlan)
		setMainGoal(GOAL_ACTION_MOVE, NULL, &location, -1.0);
	else
		setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);

	curTacOrder.code = TACTICAL_ORDER_WITHDRAW;

	return(0);
}

#endif

//---------------------------------------------------------------------------

long MechWarrior::coreSetState (long stateID, bool thinkAgain) {

	return(0);
}

//---------------------------------------------------------------------------
// CORE TACTICAL ORDERS
//---------------------------------------------------------------------------

long MechWarrior::orderWait (bool unitOrder, long origin, long seconds, bool clearLastTarget) {

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_WAIT, unitOrder);
	tacOrder.delayedTime = scenarioTime + seconds;

	//--------------------------
	// Order has no move goal...
	clearMoveOrders();
	//------------------------------
	// Order has no attack target...
	clearAttackOrders();

	if (clearLastTarget)
		setLastTarget(NULL);

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(tacOrder.status(this));
}

//---------------------------------------------------------------------------

long MechWarrior::orderStop (bool unitOrder, bool setTacOrder) {

	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_STOP, unitOrder);

	clearTacOrderQueue();

	//--------------------------
	// Order has no move goal...
	clearMoveOrders();
	//------------------------------
	// Order has no attack target...
	clearAttackOrders();

//	if (clearLastTarget)
		setLastTarget(NULL);

//	if (origin == ORDER_ORIGIN_COMMANDER)
//		setGeneralTacOrder(tacOrder);


	getVehicle()->suppressionFire = 0;

	return(tacOrder.status(this));
}

//---------------------------------------------------------------------------

long MechWarrior::orderMoveToPoint (bool unitOrder, bool setTacOrder, long origin, Stuff::Vector3D location, long selectionIndex, unsigned long params) {

	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);
	bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool wait = ((params & TACORDER_PARAM_WAIT) != 0);
	bool layMines = ((params & TACORDER_PARAM_LAY_MINES) != 0);
	bool escape = ((params & TACORDER_PARAM_ESCAPE_TILE) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_MOVETO_POINT, unitOrder);
	tacOrder.moveParams.wayPath.points[0] = location.x;
	tacOrder.moveParams.wayPath.points[1] = location.y;
	tacOrder.moveParams.wayPath.points[2] = location.z;
	tacOrder.selectionIndex = selectionIndex;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.wait = wait;
	tacOrder.moveParams.mode = (layMines ? MOVE_MODE_MINELAYING : MOVE_MODE_NORMAL);
	tacOrder.moveParams.escapeTile = escape;
	tacOrder.moveParams.jump = jump;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	//clearMoveOrders(which);
	setMoveGoal(MOVEGOAL_NONE, NULL);
	setMoveWayPath(NULL, 0);
	//for (long i = 0; i < 2; i++)
	//	clearMovePath(which, i);
	//setMoveState(which, MOVESTATE_FORWARD);
	//setMoveStateGoal(which, MOVESTATE_FORWARD);
	//setMoveYieldTime(which, -1.0);
	//setMoveYieldState(which, 0);
	//setMoveGlobalPath(which, NULL, 0);
	
	setMoveGoal(MOVEGOAL_LOCATION, &location);
	moveOrders.timeOfLastStep = scenarioTime;

	setMoveRun(run);

	//-----------------------------------
	// Move order has no attack target...
	if (setTacOrder)
		clearAttackOrders();

	unsigned long moveParams = MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET;
	if (escape)
		moveParams |= MOVEPARAM_ESCAPE_TILE;
	if (jump)
		moveParams |= MOVEPARAM_JUMP;
	if (setTacOrder && (origin == ORDER_ORIGIN_PLAYER))
		if (!unitOrder || (getPoint() == getVehicle()))
			moveParams |= MOVEPARAM_RADIO_RESULT;
		PathManager->request(this, selectionIndex, moveParams, setTacOrder ? 21 : 23);
	if (setTacOrder && (result == TACORDER_FAILURE) && (origin == ORDER_ORIGIN_COMMANDER))
		setGeneralTacOrder(tacOrder);

	return(result);
}

//---------------------------------------------------------------------------

void MechWarrior::clearMovePath (long pathNum) {

	if (moveOrders.path[pathNum]) {
		//if (moveOrders.path[pathNum]->numStepsWhenNotPaused > 0)
		//	long i = 0;
		moveOrders.path[pathNum]->clear();
	}
}


long MechWarrior::orderMoveToObject (bool unitOrder, bool setTacOrder, long origin, GameObjectPtr target, long fromArea, long selectionIndex, unsigned long params) {

	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);
	bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool faceObject = ((params & TACORDER_PARAM_FACE_OBJECT) != 0);
	bool layMines = ((params & TACORDER_PARAM_LAY_MINES) != 0);
		
	if (!target)
		return(TACORDER_SUCCESS);

	//-------------------------------------------------
	// Pick the best move goal based upon the object...
	Stuff::Vector3D goal;
	if (target->isMover())
		goal = target->getPosition();
	else {
		//------------------------------------------------
		// If not a mover, then pick an open cell adjacent
		// to it. If it happens to be adjacent to blocked
		// cells, this could be off...
		if (target->isBuilding()) {
			BuildingPtr building = (BuildingPtr)target;
			long goalRow = 0, goalCol = 0;
			bool foundGoal = building->calcAdjacentAreaCell(getVehicle()->moveLevel, fromArea, goalRow, goalCol);
			if (foundGoal)
				land->cellToWorld(goalRow, goalCol, goal);
			else {
				Stuff::Vector3D objectPos = target->getPosition();
				goal = getVehicle()->calcOffsetMoveGoal(objectPos);
			}
			}
		else {
			Stuff::Vector3D objectPos = target->getPosition();
			goal = getVehicle()->calcOffsetMoveGoal(objectPos);
		}
	}

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_MOVETO_OBJECT, unitOrder);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.selectionIndex = selectionIndex;
	tacOrder.moveParams.fromArea = fromArea;
	tacOrder.moveParams.wayPath.points[0] = goal.x;
	tacOrder.moveParams.wayPath.points[1] = goal.y;
	tacOrder.moveParams.wayPath.points[2] = goal.z;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = faceObject;
	tacOrder.moveParams.wait = false;
	tacOrder.moveParams.mode = (layMines ? MOVE_MODE_MINELAYING : MOVE_MODE_NORMAL);
	tacOrder.moveParams.jump = jump;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	//-----------------------------------
	// Is this already our current order?
	//if ((curTacOrder.code == TACTICAL_ORDER_MOVETO_OBJECT) &&
	//	(curTacOrder.params.move.targetId == target->getIdNumber()) &&
	//	(curTacOrder.params.move.selectionIndex == selectionIndex))
	//	return(TACORDER_FAILURE);
	tacOrder.targetWID = target->getWatchID();
	Stuff::Vector3D pos = target->getPosition();
	setMoveGoal(target->getWatchID(), &pos, target);
	setMoveRun(run);

	//-----------------------------------
	// Move order has no attack target...
	if (setTacOrder)
		clearAttackOrders();

	unsigned long moveParams = MOVEPARAM_INIT;
	if (jump)
		moveParams |= MOVEPARAM_JUMP;
	if (faceObject)
		moveParams |= MOVEPARAM_FACE_TARGET;
	if (setTacOrder && !target->isMover())
		moveParams |= MOVEPARAM_STEP_ADJACENT_TARGET;
	if (setTacOrder && (origin == ORDER_ORIGIN_PLAYER))
		if (!unitOrder || (getPoint() == getVehicle()))
				moveParams |= MOVEPARAM_RADIO_RESULT;
	requestMovePath(selectionIndex, moveParams, 16);
	moveOrders.goalObjectPosition = target->getPosition();
	if (setTacOrder && (result == TACORDER_FAILURE) && (origin == ORDER_ORIGIN_COMMANDER))
		setGeneralTacOrder(tacOrder);

	return(result);
}

//---------------------------------------------------------------------------

long MechWarrior::orderJumpToPoint (bool unitOrder, bool setTacOrder, long origin, Stuff::Vector3D location, long selectionIndex) {

	MoverPtr myVehicle = getVehicle();
	if (myVehicle->getJumpRange() < myVehicle->distanceFrom(location))
		return(TACORDER_SUCCESS);

	if (myVehicle->getObjectClass() == BATTLEMECH) {
		int cellR, cellC;
		land->worldToCell(location, cellR, cellC);
		if (!GameMap->getPassable(cellR, cellC))
			return(TACORDER_SUCCESS);
	}

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_JUMPTO_POINT, unitOrder);
	tacOrder.selectionIndex = selectionIndex;
	tacOrder.moveParams.wayPath.points[0] = location.x;
	tacOrder.moveParams.wayPath.points[1] = location.y;
	tacOrder.moveParams.wayPath.points[2] = location.z;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	//-----------------------------------
	// Move order has no attack target...
	if (setTacOrder) {
		clearMoveOrders();
		clearAttackOrders();
		//setLastTarget(NULL);
	}

	if (setTacOrder && (result == TACORDER_FAILURE) && (origin == ORDER_ORIGIN_COMMANDER))
		setGeneralTacOrder(tacOrder);

	return(TACORDER_SUCCESS);
}

//---------------------------------------------------------------------------

long MechWarrior::orderJumpToObject (bool unitOrder, bool setTacOrder, long origin, GameObjectPtr target, long selectionIndex) {

	MoverPtr myVehicle = getVehicle();
	Stuff::Vector3D location = target->getPosition();

	if (target->isMover()) {
		if (target->getTeam() == myVehicle->getTeam())
			return(TACORDER_SUCCESS);
	}

	if (myVehicle->getJumpRange() < myVehicle->distanceFrom(location))
		return(TACORDER_SUCCESS);

	if (myVehicle->getObjectClass() == BATTLEMECH) {
		int cellR, cellC;
		land->worldToCell(location, cellR, cellC);
		if (!GameMap->getPassable(cellR, cellC))
			return(TACORDER_SUCCESS);
	}

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_JUMPTO_POINT, unitOrder);
	tacOrder.selectionIndex = selectionIndex;
	tacOrder.moveParams.wayPath.points[0] = location.x;
	tacOrder.moveParams.wayPath.points[1] = location.y;
	tacOrder.moveParams.wayPath.points[2] = location.z;
	tacOrder.targetWID = target->getWatchID();

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	//-----------------------------------
	// Move order has no attack target...
	if (setTacOrder) {
		clearMoveOrders();
		clearAttackOrders();
	}

	if (setTacOrder && (result == TACORDER_FAILURE) && (origin == ORDER_ORIGIN_COMMANDER))
		setGeneralTacOrder(tacOrder);

	return(TACORDER_SUCCESS);
}

//---------------------------------------------------------------------------

long MechWarrior::orderTraversePath (bool unitOrder, bool setTacOrder, long origin, WayPathPtr wayPath, unsigned long params) {

	bool layMines = ((params & TACORDER_PARAM_LAY_MINES) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_TRAVERSE_PATH, unitOrder);
	memcpy(&tacOrder.moveParams.wayPath, wayPath, sizeof(WayPath));
	tacOrder.moveParams.mode = (layMines ? MOVE_MODE_MINELAYING : MOVE_MODE_NORMAL);

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	Stuff::Vector3D location;
	location.x = tacOrder.moveParams.wayPath.points[0];
	location.y = tacOrder.moveParams.wayPath.points[1];
	location.z = tacOrder.moveParams.wayPath.points[2];
	setMoveGoal(MOVEGOAL_LOCATION, &location);
	setMoveWayPath(wayPath, false);

	//-----------------------------------
	// Move order has no attack target...
	if (setTacOrder)
		clearAttackOrders();

	requestMovePath(-1, MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET, 17);
	if (setTacOrder && (result == TACORDER_FAILURE) && (origin == ORDER_ORIGIN_COMMANDER))
		setGeneralTacOrder(tacOrder);

	return(result);	
}

//---------------------------------------------------------------------------

long MechWarrior::orderPatrolPath (bool unitOrder, bool setTacOrder, long origin, WayPathPtr wayPath) {

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_PATROL_PATH, unitOrder);
	memcpy(&tacOrder.moveParams.wayPath, wayPath, sizeof(WayPath));
	
	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	Stuff::Vector3D location;
	location.x = tacOrder.moveParams.wayPath.points[0];
	location.y = tacOrder.moveParams.wayPath.points[1];
	location.z = tacOrder.moveParams.wayPath.points[2];
	setMoveGoal(MOVEGOAL_LOCATION, &location);
	setMoveWayPath(wayPath, true);
	//-----------------------------------
	// Move order has no attack target...
	if (setTacOrder)
		clearAttackOrders();

	requestMovePath(-1, MOVEPARAM_INIT | MOVEPARAM_FACE_TARGET, 18);
	if (setTacOrder && (result == TACORDER_FAILURE) && (origin == ORDER_ORIGIN_COMMANDER))
		setGeneralTacOrder(tacOrder);

	return(result);	
}

//---------------------------------------------------------------------------

long MechWarrior::orderPowerUp (bool unitOrder, long origin) {

	MoverPtr myVehicle = getVehicle();
	if ((myVehicle->getStatus() != OBJECT_STATUS_SHUTDOWN))
		return TACORDER_SUCCESS;

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_POWERUP, unitOrder);

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	//--------------------------
	// Order has no move goal...
	clearMoveOrders();
	//------------------------------
	// Order has no attack target...
	clearAttackOrders();

	//-------------------------------------------
	// Check if we've cooled enough to restart...
	if (myVehicle)
		if (myVehicle->canPowerUp())
			myVehicle->startUp();
	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);
	else if (origin == ORDER_ORIGIN_SELF)
		setAlarmTacOrder(tacOrder, 255);

	return(result);	
}

//---------------------------------------------------------------------------

long MechWarrior::orderPowerDown (bool unitOrder, long origin) {

	MoverPtr myVehicle = getVehicle();
	if ((myVehicle->getStatus() == OBJECT_STATUS_SHUTDOWN) || (myVehicle->getStatus() == OBJECT_STATUS_SHUTTING_DOWN))
		return(TACORDER_SUCCESS);
	if (myVehicle->isDisabled())
		return(TACORDER_SUCCESS);
	
	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_POWERDOWN, unitOrder);

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	//--------------------------
	// Order has no move goal...
	clearMoveOrders();
	//------------------------------
	// Order has no attack target...
	clearAttackOrders();

	if (myVehicle)
		myVehicle->shutDown();
	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(result);	
}

//---------------------------------------------------------------------------

long MechWarrior::orderUseSpeed (float speed) {

	//-----------------------------------------------------------
	// The speed we're given is in meters/sec. We need to convert
	// this to our proper "sprite speed", using gesture state and
	// (if appropriate) throttle setting...
	moveOrders.speedVelocity = speed;
	MoverPtr myVehicle = getVehicle();
	long state, throttle;
	myVehicle->calcSpriteSpeed(speed, 0, state, throttle);
	moveOrders.speedState = state;
	moveOrders.speedThrottle = throttle;

	return(TACORDER_SUCCESS);
}

//---------------------------------------------------------------------------

long MechWarrior::orderOrbitPoint (Stuff::Vector3D location) {

	return(1);
}

//---------------------------------------------------------------------------

long orderOrbitObject (GameObjectPtr target) {

	return(1);
}

//---------------------------------------------------------------------------

long orderUseOrbitRange (long type, float range) {

	return(1);
}

//---------------------------------------------------------------------------

long MechWarrior::orderAttackObject (bool unitOrder, long origin, GameObjectPtr target, long type, long method, long range, long aimLocation, long fromArea, unsigned long params) {

	bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);
	bool pursue = ((params & TACORDER_PARAM_PURSUE) != 0);
	bool obliterate = ((params & TACORDER_PARAM_OBLITERATE) != 0);
	bool conserve = (type == ATTACK_CONSERVING_AMMO);
	bool keepMoving = ((params & TACORDER_PARAM_DONT_KEEP_MOVING) == 0);

	long tactic = TACTIC_NONE;
	switch (params & TACORDER_TACTIC_MASK) {
		case TACORDER_PARAM_TACTIC_FLANK_RIGHT:
			tactic = TACTIC_FLANK_RIGHT;
			break;
		case TACORDER_PARAM_TACTIC_FLANK_LEFT:
			tactic = TACTIC_FLANK_LEFT;
			break;
		case TACORDER_PARAM_TACTIC_FLANK_REAR:
			tactic = TACTIC_FLANK_REAR;
			break;
		case TACORDER_PARAM_TACTIC_STOP_AND_FIRE:
			tactic = TACTIC_STOP_AND_FIRE;
			pursue = true;
			break;
		case TACORDER_PARAM_TACTIC_TURRET:
			tactic = TACTIC_TURRET;
			jump = false;
			pursue = false;
			keepMoving = false;
			break;
		case TACORDER_PARAM_TACTIC_JOUST:
			tactic = TACTIC_JOUST;
			break;
	}

	if (getVehicle()->attackRange == FIRERANGE_CURRENT) {
		range = FIRERANGE_CURRENT;
		pursue = false;
		jump = false;
	}

	if (!target) {
		//-----------------------
		// Clear Attack orders...
		clearAttackOrders();
		return(TACORDER_SUCCESS);
	}

	if ((getVehicle()->numWeapons == 0) && (method != ATTACKMETHOD_RAMMING)) {
		clearAttackOrders();
		return(TACORDER_SUCCESS);
	}

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_ATTACK_OBJECT, unitOrder);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.attackParams.type = (AttackType)type;
	tacOrder.attackParams.method = (AttackMethod)method;
	tacOrder.attackParams.aimLocation = aimLocation;
	tacOrder.attackParams.tactic = (TacticType)tactic;
	if (method == ATTACKMETHOD_RAMMING)
		range = FIRERANGE_RAMMING;
	tacOrder.attackParams.range = (FireRangeType)range;
	tacOrder.attackParams.pursue = pursue;
	tacOrder.attackParams.obliterate = obliterate;
	tacOrder.moveParams.fromArea = fromArea;
	tacOrder.moveParams.jump = jump;
	tacOrder.moveParams.keepMoving = keepMoving;
	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);
	if (curTacOrder.equals(&tacOrder))
		return(0);

	setSituationFireRange(getVehicle()->getOrderedFireRange(&range));

	//------------------------------------------------------------
	// If we also want to pursue it, set up the move order here...
	if (pursue) {
		//------------------------------------------------------
		// Order moveToObject, making sure we face the object...
		setMoveRun(run);
		params |= TACORDER_PARAM_FACE_OBJECT;
		//orderMoveToObject(unitOrder, false, origin, target, fromArea, -1/*unitMateId*/, params);
		}
	else
		clearMoveOrders();

	//------------------------------------------------------------
	// Now, set up the tactical order, and pass it to the pilot...
	setAttackType(type);
	setAttackTarget(target);
	setAttackAimLocation(aimLocation);
	setAttackPursuit(pursue);
	setSituationOpenFire(true);

	//-----------------------------------
	// Remember the most recent target...
	setLastTarget(target, obliterate, conserve);

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	/*
	if (GameSystemWindow) {
		char debugStr[200];
		GameSystemWindow->print("");
		GameSystemWindow->print("-----------------------------------");
		sprintf(debugStr, "%s:", name);
		GameSystemWindow->print(debugStr);
		sprintf(debugStr, "Longest Range Weapon = %s (%.4f)",
			MasterComponentList[(MoverPtr(BaseObjectPtr(vehicle)))->inventory[(MoverPtr(BaseObjectPtr(vehicle)))->longestRangeWeapon].masterID].getName(),
			MasterComponentList[(MoverPtr(BaseObjectPtr(vehicle)))->inventory[(MoverPtr(BaseObjectPtr(vehicle)))->longestRangeWeapon].masterID].getWeaponLongRange());
		GameSystemWindow->print(debugStr);
		sprintf(debugStr, "Optimal Range = %.4f", (MoverPtr(BaseObjectPtr(vehicle)))->optimalRange);
		GameSystemWindow->print(debugStr);
		GameSystemWindow->print("-----------------------------------");
	}
	*/

	return(TACORDER_FAILURE);
}

//---------------------------------------------------------------------------

long MechWarrior::orderAttackPoint (bool unitOrder, long origin, Stuff::Vector3D location, long type, long method, long range, unsigned long params) {

	bool pursue = ((params & TACORDER_PARAM_PURSUE) != 0);
	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_ATTACK_POINT, unitOrder);
	tacOrder.attackParams.type = (AttackType)type;
	tacOrder.attackParams.method = (AttackMethod)method;
	tacOrder.attackParams.range = (FireRangeType)range;
	tacOrder.attackParams.pursue = pursue;
	tacOrder.attackParams.targetPoint = location;
	//tacOrder.moveParams.fromArea = fromArea;
	tacOrder.moveParams.jump = jump;

	long result = tacOrder.status(this);
	if (result == TACORDER_SUCCESS)
		return(TACORDER_SUCCESS);

	setSituationFireRange(getVehicle()->getOrderedFireRange(&range));

	//------------------------------------------------------------
	// If we also want to pursue it, set up the move order here...
	if (pursue)
		orderMoveToPoint(unitOrder, false, origin, location, -1/*unitMateId*/, params);
	else
		clearMoveOrders();

	//------------------------------------------------------------
	// Now, set up the tactical order, and pass it to the pilot...
	setAttackType(type);
	setAttackTarget(NULL);
	setAttackTargetPoint(location);
	setAttackAimLocation(-1);
	setAttackPursuit(pursue);
	setSituationOpenFire(true);

	//-----------------------------------
	// Remember the most recent target...
	setLastTarget(NULL);

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);
	
	/*
	if (GameSystemWindow) {
		char debugStr[200];
		GameSystemWindow->print("");
		GameSystemWindow->print("-----------------------------------");
		sprintf(debugStr, "%s:", name);
		GameSystemWindow->print(debugStr);
		sprintf(debugStr, "Longest Range Weapon = %s (%.4f)",
			MasterComponentList[(MoverPtr(BaseObjectPtr(vehicle)))->inventory[(MoverPtr(BaseObjectPtr(vehicle)))->longestRangeWeapon].masterID].getName(),
			MasterComponentList[(MoverPtr(BaseObjectPtr(vehicle)))->inventory[(MoverPtr(BaseObjectPtr(vehicle)))->longestRangeWeapon].masterID].getWeaponLongRange());
		GameSystemWindow->print(debugStr);
		sprintf(debugStr, "Optimal Range = %.4f", (MoverPtr(BaseObjectPtr(vehicle)))->optimalRange);
		GameSystemWindow->print(debugStr);
		GameSystemWindow->print("-----------------------------------");
	}
	*/

	return(TACORDER_FAILURE);
}

//---------------------------------------------------------------------------

long MechWarrior::orderWithdraw (bool unitOrder, long origin, Stuff::Vector3D location) {

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_WITHDRAW, unitOrder);
	tacOrder.moveParams.wayPath.points[0] = location.x;
	tacOrder.moveParams.wayPath.points[1] = location.y;
	tacOrder.moveParams.wayPath.points[2] = location.z;

	location = calcWithdrawGoal();
	long result = orderMoveToPoint(unitOrder, true, origin, location, -1, TACORDER_PARAM_RUN);
	MoverPtr myVehicle = getVehicle();
	Assert(myVehicle != NULL, 0, " orderWithdraw:Warrior has no Vehicle ");

	myVehicle->withdrawing = true;

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	curTacOrder.code = TACTICAL_ORDER_WITHDRAW;

	return(result);
}

//---------------------------------------------------------------------------

long MechWarrior::orderEject (bool unitOrder, bool setTacOrder, long origin) {

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_EJECT, unitOrder);

	MoverPtr myVehicle = getVehicle();
	Assert(myVehicle != NULL, 0, " orderWithdraw:Warrior has no Vehicle ");

	myVehicle->handleEjection();

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);
	
	return(TACORDER_SUCCESS);
}

//---------------------------------------------------------------------------

#if 0

long MechWarrior::orderUseFireOdds (long odds) {

	setSituationFireOdds(FireOddsTable[odds]);

	return(TACORDER_SUCCESS);
}

#endif

//---------------------------------------------------------------------------

long MechWarrior::orderRefit (long origin, GameObjectPtr target, unsigned long params)
{
	if (!target || target->getObjectClass() != BATTLEMECH)
		return(TACORDER_SUCCESS);

	bool run = ((params & TACORDER_PARAM_RUN) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_REFIT, false);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.wait = false;

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::orderRecover (long origin, GameObjectPtr target, unsigned long params)
{
	if (!target || target->getObjectClass() != BATTLEMECH)
		return(TACORDER_SUCCESS);

	bool run = ((params & TACORDER_PARAM_RUN) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_RECOVER, false);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.wait = false;

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::orderGetFixed (long origin, GameObjectPtr target, unsigned long params)
{

	if (!target || (target->getObjectClass() != TREEBUILDING && target->getRefitPoints() <= 0.0))
		return(TACORDER_SUCCESS);
#ifdef USE_BUILDINGS
	if ((getVehicle()->getObjectClass() == BATTLEMECH && ((TreeBuildingPtr) target)->mechBay == false) ||
		(getVehicle()->getObjectClass() == GROUNDVEHICLE && ((TreeBuildingPtr) target)->mechBay == true))
		return(TACORDER_SUCCESS);
#endif

	bool run = ((params & TACORDER_PARAM_RUN) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_GETFIXED, false);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.wait = false;

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::orderLoadIntoCarrier (long origin, GameObjectPtr target, unsigned long params) {

#ifdef USE_ELEMENTALS
	if ((getVehicle()->getObjectClass() != ELEMENTAL) ||
		!target ||
		(target->getObjectClass() != GROUNDVEHICLE) ||
		(((GroundVehiclePtr) target)->elementalCarrier == false))
		return(TACORDER_SUCCESS);
#endif

	bool run = ((params & TACORDER_PARAM_RUN) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_LOAD_INTO_CARRIER, false);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.wait = false;

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::orderDeployElementals (long origin, unsigned long params) {

#ifdef USE_ELEMENTALS
	if ((getVehicle()->getObjectClass() != GROUNDVEHICLE) || (((GroundVehiclePtr)getVehicle())->elementalCarrier == false))
		return(TACORDER_SUCCESS);
#endif

	bool run = ((params & TACORDER_PARAM_RUN) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_DEPLOY_ELEMENTALS, false);
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.wait = false;

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::orderCapture (long origin, GameObjectPtr target, long fromArea, unsigned long params) {

	if (!target || /*!target->isCaptureable(getTeam()->getId()) ||*/ target->isFriendly(getVehicle()) || 
			target->getCaptureBlocker(getVehicle()) != 0)
		return(1);

	bool run = ((params & TACORDER_PARAM_RUN) != 0);
	bool jump = ((params & TACORDER_PARAM_JUMP) != 0);

	TacticalOrder tacOrder;
	tacOrder.init((OrderOriginType)origin, TACTICAL_ORDER_CAPTURE, false);
	tacOrder.targetWID = target->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = (run ? TRAVEL_MODE_FAST : TRAVEL_MODE_SLOW);
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.wait = false;
	tacOrder.moveParams.fromArea = fromArea;
	tacOrder.moveParams.jump = jump;

	if (origin == ORDER_ORIGIN_COMMANDER)
		setGeneralTacOrder(tacOrder);

	return(NO_ERR);
}

//---------------------------------------------------------------------------
// ALARM EVENTS
//---------------------------------------------------------------------------

long MechWarrior::handleTargetOfWeaponFire (void) {

	//unsigned long attackerId = alarm[PILOT_ALARM_TARGET_OF_WEAPONFIRE].trigger[0];

	//GameObjectPtr myVehicle = getVehicle();
#ifdef USE_IFACE
	if (myVehicle)
		theInterface->ObjectAttacked(myVehicle->getPartId());
#endif
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleHitByWeaponFire (void) {

	long attackerWID = alarm[PILOT_ALARM_HIT_BY_WEAPONFIRE].trigger[0];

	if (attackerWID == 0) 
	{
		//----------------
		// Ammo Explosion?
	}
	else if (attackerWID == -3)
		radioMessage(RADIO_UNDER_AIRSTRIKE, true);
	else if (attackerWID == -2)
		radioMessage(RADIO_HITTING_MINES, true);
	else
	{
		long targetWID = 0;
		if (getCurTacOrder()->code != TACTICAL_ORDER_NONE)
			targetWID = getCurTacOrder()->targetWID;

		if (!targetWID)
			radioMessage(RADIO_UNDER_ATTACK, true);
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleCollision (void) {

	//--------------------------------------------
	// Colliding with trees should not get here...

	//long colliderHandle = alarm[PILOT_ALARM_COLLISION].trigger[0];
	//GameObjectPtr collider = ObjectManager->get(colliderHandle);

#if 0
	//----------------------------------------------------------
	// If we're Clan, we'll fire back. IS ignore this for now...
	if (alignment == INNER_SPHERE)
		return(NO_ERR);

	//---------------------------------------------------------
	// Fire back on the attacker ONLY if I'm Clan and currently
	// have no target...
	if (colliderObjectClass != BUILDING) {
		if (getAttackTargetId(ORDER_CURRENT))
			return(NO_ERR);
		if (colliderId != getAttackTargetId(ORDER_CURRENT)) {
			TacticalOrder tacOrder;
			tacOrder.init(ORDER_ORIGIN_SELF, TACTICAL_ORDER_ATTACK_OBJECT);
			tacOrder.params.attack.targetId = colliderId;
			tacOrder.params.attack.range = FIRERANGE_SHORT;
			tacOrder.params.attack.pursue = true;
			tacOrder.params.attack.type = ATTACK_TO_DESTROY;
			setAlarmTacOrder(tacOrder, 10);
		}
	}
#endif

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleDamageTakenRate (void) {


	//long damageRate = alarm[PILOT_ALARM_DAMAGE_TAKEN_RATE].trigger[0];

#if 0
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_SELF, TACTICAL_ORDER_WITHDRAW);
	Stuff::Vector3D withdrawLocation = ((LancePtr)unit)->getWithdrawThresholdGate();
	tacOrder.params.move.location[0] = withdrawLocation.x;
	tacOrder.params.move.location[1] = withdrawLocation.y;
	tacOrder.params.move.location[2] = withdrawLocation.z;
	tacOrder.params.move.selectionIndex = -1;
	setAlarmTacOrder(tacOrder, 15);
#endif

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleUnitMateDeath (void) {

	unsigned long mateWID = alarm[PILOT_ALARM_DEATH_OF_MATE].trigger[0];

	//--------------------------------------
	// First, check if the death was seen...
	MoverPtr myVehicle = getVehicle();
	if ((unsigned long)myVehicle->getWatchID() == mateWID)
		return(NO_ERR);
	MoverPtr mateVehicle = (MoverPtr)ObjectManager->getByWatchID(mateWID);
	if (!mateVehicle)
		return(-1);

#if 0
	float rangeToMate = myVehicle->distanceFrom((GameObjectPtr(BaseObjectPtr(mateVehicle)))->getPosition());
	float factor = 1.5;
	if (/*(rangeToMate <= scenario->maxVisualRange) &&*/ myVehicle->lineOfSight((GameObjectPtr(BaseObjectPtr(mateVehicle)))))
		factor = 1.0;

	//--------------------------------
	// Check whether we snap or not...
	changeMorale(-5);
	if (checkMorale() < 0) {
#if 0
		long checkDelta = checkRelationship(matePilot->getID());
		if (checkDelta >= 0) {
			//----------------------------------------------------
			// Since this is an interrupt, we don't care about the
			// updateTime...
			float time = 0.0;
			commandDecisionTree(1, time, checkDelta);
		}
#endif
	}
#endif

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleFriendlyVehicleCrippled (void) {

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleFriendlyVehicleDestruction (void) {

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleOwnVehicleIncapacitation (unsigned long cause) {

	MoverPtr myVehicle = getVehicle();
	Assert(myVehicle != NULL, 0, " pilot has no vehicle ");

	switch (cause) {
		case 0:
		case 1:
		case 66:
			//---------------------------------------------------------------
			// Center Torso Internal Structure gone. Automatically ejects, if
			// can...
			myVehicle->handleEjection();
			break;
	}

	//----------------
	// Clear orders...
	clearCurTacOrder(false);
	setOrderState(ORDERSTATE_GENERAL);
	MovePathPtr tempPath[2];
	for (long i = 0; i < 2; i++) {
		tempPath[i] = moveOrders.path[i];
		tempPath[i]->numSteps = 0;
	}
	moveOrders.init();
	PathManager->remove(this);
	moveOrders.path[0] = tempPath[0];
	moveOrders.path[1] = tempPath[1];
	attackOrders.init();
	setLastTarget(NULL);

	//--------------------------------------------------------
	// Let's let the unit know we're dying if we're a point...
	// No, Don't!!!
//	if (getPoint() == vehicle) {
//		GameObjectPtr newPointVehicle = getGroup()->selectPoint(true);
		//--------------------------------------------------------
		// If there is no new point, all units must be blown away.
		// How do we want to handle this?
//	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleOwnVehicleDestruction (unsigned long cause) {

	//unsigned long attackerId = cause;

	//MoverPtr myVehicle = getVehicle();
	//Assert(myVehicle != NULL, 0, "handleOwnVehicleDestruction:pilot has no vehicle ");

	//--------------------------------------------------------
	// Let's let the unit know we're dying if we're a point...
	// No, Don't!!!
//	if (getPoint() == myVehicle) {
//		GameObjectPtr newPointVehicle = getGroup()->selectPoint(true);
		//--------------------------------------------------------
		// If there is no new point, all units must be blown away.
		// How do we want to handle this?
//	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleOwnVehicleWithdrawn (void) {

	//MoverPtr myVehicle = getVehicle();
	//Assert(myVehicle != NULL, 0, "handleOwnVehicleWithdrawn:pilot has no vehicle ");

	//--------------------------------------------------------
	// Let's let the unit know we're dying if we're a point...
	// No, Don't!!!
//	if (getPoint() == myVehicle) {
//		GameObjectPtr newPointVehicle = getGroup()->selectPoint(true);
		//--------------------------------------------------------
		// If there is no new point, all units must be blown away.
		// How do we want to handle this?
//	}

	setStatus(WARRIOR_STATUS_WITHDRAWN);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleAttackOrder (void) {

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleFiredWeapon (void) {

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleNewMover (void) {

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleCollisionAlert (void) {

	return(NO_ERR);
}

//---------------------------------------------------------------------------
long MechWarrior::handleKilledTarget (void) {

	unsigned long targetWID = alarm[PILOT_ALARM_KILLED_TARGET].trigger[0];
	GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
	if (target) 
	{
		long message = -1;
		float skillIncrement = 0;
		long vehicleClass = -1;
		switch (target->getObjectClass()) 
		{
			case BATTLEMECH:
				if (((MoverPtr)target)->getMoveType() != MOVETYPE_AIR)	//Killing Helicopters does not count as a mech.
				{
					vehicleClass = target->getMechClass();
					skillIncrement = KillSkill[vehicleClass];
					numMechKills[vehicleClass][COMBAT_STAT_MISSION]++;
					message = RADIO_MECH_DEAD;
					if (numKilled < MAX_MOVERS/3)
						killed[numKilled++] = target->getWatchID();
				}
				else
				{
					vehicleClass = VEHICLE_CLASS_GROUND;
					skillIncrement = KillSkill[4];
					numMechKills[VEHICLE_CLASS_GROUND][COMBAT_STAT_MISSION]++;
					message = RADIO_VEHICLE_DEAD;
					if (numKilled < MAX_MOVERS/3)
						killed[numKilled++] = target->getWatchID();
				}
				break;

			case GROUNDVEHICLE:
			{
				MoverPtr gv = (MoverPtr)target;
				if (gv->pathLocks)		//Killing Infantry/Powered Armor is meaningless!
				{
					vehicleClass = VEHICLE_CLASS_GROUND;
					skillIncrement = KillSkill[4];
					numMechKills[VEHICLE_CLASS_GROUND][COMBAT_STAT_MISSION]++;
					message = RADIO_VEHICLE_DEAD;
					if (numKilled < MAX_MOVERS/3)
						killed[numKilled++] = target->getWatchID();
				}
			}
			break;

			case TURRET:
			case GATE:
				message = RADIO_OBJECT_DEAD;
				break;

			case ELEMENTAL:
				vehicleClass = VEHICLE_CLASS_ELEMENTAL;
				skillIncrement = KillSkill[5];
				numMechKills[VEHICLE_CLASS_ELEMENTAL][COMBAT_STAT_MISSION]++;

			case BUILDING:
				message = RADIO_BUILDING_DEAD;
				break;

			default:
				message = RADIO_OBJECT_DEAD;
		}

		radioMessage(message);
		skillPoints[MWS_GUNNERY] += skillIncrement;
		if (MPlayer && MPlayer->isServer() && (vehicleClass != -1))
			MPlayer->addPilotKillStat(getVehicle(), vehicleClass);
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleUnitMateFiredWeapon (void) {

	//unsigned long mateVehicleHandle = alarm[PILOT_ALARM_MATE_FIRED_WEAPON].trigger[0];

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handlePlayerOrder (void) {

	if ((getVehicleStatus() == OBJECT_STATUS_SHUTDOWN) && (curTacOrder.code != TACTICAL_ORDER_POWERDOWN))
		orderPowerUp(false, ORDER_ORIGIN_SELF);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::handleNoMovePath (void) {
	if (getCurTacOrder()->code == TACTICAL_ORDER_GETFIXED)
	{
		clearCurTacOrder();
		radioMessage(RADIO_MOVE_BLOCKED);
	}
	return(NO_ERR);	
}

//---------------------------------------------------------------------------

long MechWarrior::handleGateClosing (void) {

/*
	//------------------------------------------------------------
	// Do this here for now... but, we may want to put it into the
	// player brain instead and allow kamikazee pilots :)
	for (long i = 0; i < alarm[PILOT_ALARM_GATE_CLOSING].numTriggers; i++) {
		long gateTileRow = alarm[PILOT_ALARM_GATE_CLOSING].trigger[i] / MAX_MAPWIDTH;
		long gateTileCol = alarm[PILOT_ALARM_GATE_CLOSING].trigger[i] % MAX_MAPWIDTH;
		long overlayType = GameMap->getOverlayType(gateTileRow, gateTileCol);
		bool check = true;
		if (alignment == CLANS)
			check = ((overlayType == OVERLAY_GATE_IS_NS_CLOSED) || (overlayType == OVERLAY_GATE_IS_EW_CLOSED));
		else
			check = ((overlayType == OVERLAY_GATE_CLAN_NS_CLOSED) || (overlayType == OVERLAY_GATE_IS_EW_CLOSED));
		if (check && getMovePath()) {
			long firstStep = getMovePath()->crossesTile(-1, 100, gateTileRow, gateTileCol);
			if (firstStep > 1) {
				//setMoveGoal(MOVEGOAL_NONE, NULL);
				setMoveWayPath(0);
				for (long i = 0; i < 2; i++)
					clearMovePath(i);
				setMoveState(MOVESTATE_FORWARD);
				setMoveStateGoal(MOVESTATE_FORWARD);
				setMoveYieldTime(-1.0);
				setMoveYieldState(0);
				setMoveWaitForPointTime(-1.0);
				setMoveTimeOfLastStep(-1.0);
				setMoveTwisting(false);
				setMoveGlobalPath(NULL, 0);
				PathManager->remove(this);
				break;
			}
		}
	}
*/
	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MechWarrior::missionLog (FilePtr file, long unitLevel) 
{
#if 0
	for (long i = 0; i < (unitLevel * 2); i++)
		file->writeString(" ");

	char s[80];
	sprintf(s, "MechWarrior: %s\n", name);
	file->writeString(s);
	for (long skill = 0; skill < NUM_SKILLS; skill) 
	{
		for (i = 0; i < ((unitLevel + 1) * 2); i++)
		{
			file->writeString(" ");
		}

		sprintf(s, "%s: %04d/04%d\n",
			SkillsTable[skill],
			numSkillSuccesses[skill][COMBAT_STAT_MISSION],
			numSkillUses[skill][COMBAT_STAT_MISSION]);
	}
#endif
	return(0);
}

//---------------------------------------------------------------------------
void MechWarrior::calcRank (void)
{
	float skillAverage = skillRank[MWS_GUNNERY] + skillRank[MWS_PILOTING];
	skillAverage /= 2.0f;

	float avg = skillAverage;

	if ( avg > 79.f )
		rank = WARRIOR_RANK_ACE;
	else if ( avg > 70.f )
		rank = WARRIOR_RANK_ELITE;
	else if ( avg > 60.f )
		rank = WARRIOR_RANK_VETERAN;
	else if ( avg > 50.f )
		rank = WARRIOR_RANK_REGULAR;
	else
		rank = WARRIOR_RANK_GREEN;
}

//---------------------------------------------------------------------------
void MechWarrior::updateMissionSkills()
{
	// DO NOT CALL THIS IF THE MISSION FAILED!

	float MaxPilotSkill = 80.f;
	
	// increment skills and see if pilot has been promoted
	skillRank[MWS_GUNNERY] += (float)skillPoints[MWS_GUNNERY] * 0.01f;
	if (skillRank[MWS_GUNNERY] > MaxPilotSkill)
		skillRank[MWS_GUNNERY] = MaxPilotSkill;

	skillRank[MWS_PILOTING]++;		//One free point just for coming along!

	skillRank[MWS_PILOTING] += skillPoints[MWS_PILOTING];
	if (skillRank[MWS_PILOTING] > MaxPilotSkill)
		skillRank[MWS_PILOTING] = MaxPilotSkill;
}

//---------------------------------------------------------------------------
long MechWarrior::loadBrainParameters (FitIniFile* brainFile, long warriorId) {

	if (!brain)
		Fatal(0, " Warrior.loadBrainParameters: NULL brain ");

	//------------------
	// Main Info section
	char warriorName[32];
	sprintf(warriorName, "Warrior%d", warriorId);
	long result = brainFile->seekBlock(warriorName);
	if (result != NO_ERR)
		return(result);

	long numCells = 0;
	result = brainFile->readIdLong("NumCells", numCells);
	if (result != NO_ERR)
		return(result);
		
	long numStaticVars = 0;
	result = brainFile->readIdLong("NumStaticVars", numStaticVars);
	if (result != NO_ERR)
		return(result);

	for (long i = 0; i < numCells; i++) {
		char blockName[64];
		sprintf(blockName, "%sCell%d", warriorName, i);
		long result = brainFile->seekBlock(blockName);
		if (result != NO_ERR)
			return(result);
		long cell = 0;
		result = brainFile->readIdLong("Cell", cell);
		if (result != NO_ERR)
			return(result);
		long memType = 0;
		result = brainFile->readIdLong("MemType", memType);
		if (result != NO_ERR)
			return(result);
		switch (memType) {
			case 0: {
				// Integer Memory
				long value = 0;
				result = brainFile->readIdLong("Value", value);
				if (result != NO_ERR)
					return(result);
				setIntegerMemory(cell, value);
				}
				break;
			case 1: {
				// Real Memory
				float value = 0.0;
				result = brainFile->readIdFloat("Value", value);
				if (result != NO_ERR)
					return(result);
				setRealMemory(cell, value);
				}
				break;
			default:
				result = 666;
				return(result);
		}
	}

	for (int i = 0; i < numStaticVars; i++) {
		char blockName[64];
		sprintf(blockName, "%sStatic%d", warriorName, i);
		long result = brainFile->seekBlock(blockName);
		if (result != NO_ERR)
			return(result);
		long type = 0;
		result = brainFile->readIdLong("type", type);
		if (result != NO_ERR)
			return(result);
		char varName[256];
		result = brainFile->readIdString("Name", varName, 255);
		if (result != NO_ERR)
			return(result);
		switch (type) {
			case 0: {
				// Integer
				long value = 0;
				result = brainFile->readIdLong("Value", value);
				if (result != NO_ERR)
					return(result);

				brain->setStaticInteger(varName, value);
				}
				break;
			case 1: {
				// Real
				float value = 0;
				result = brainFile->readIdFloat("Value", value);
				if (result != NO_ERR)
					return(result);

				brain->setStaticReal(varName, value);
				}
				break;
			case 2: {
				// Integer Array
				long numValues = 0;
				result = brainFile->readIdLong("NumValues", numValues);
				if (result != NO_ERR)
					return(result);

				long values[1024];
				result = brainFile->readIdLongArray("Values", values, numValues);
				if (result != NO_ERR)
					return(result);

				brain->setStaticIntegerArray(varName, numValues, values);
				}
				break;
			case 3: {
				// Real Array
				long numValues = 0;
				result = brainFile->readIdLong("NumValues", numValues);
				if (result != NO_ERR)
					return(result);

				float values[1024];
				result = brainFile->readIdFloatArray("Values", values, numValues);
				if (result != NO_ERR)
					return(result);

				brain->setStaticRealArray(varName, numValues, values);
				}
				break;
			default:
				result = 667;
				return(result);
		}
	}

	return(NO_ERR);
}

void MechWarrior::drawWaypointPath()
{
	if ( tacOrderQueue )
	{
		Stuff::Vector3D tmpPos = this->getVehicle()->getPosition();
		
		{
			Stuff::Vector4D screenPos1;
			Stuff::Vector4D screenPos2;

			Stuff::Vector3D pos;
			pos.x = getCurTacOrder()->moveParams.wayPath.points[0];
			pos.y = getCurTacOrder()->moveParams.wayPath.points[1];
			pos.z = getCurTacOrder()->moveParams.wayPath.points[2];

			if ( tacOrderQueueLooping )
			{
				tmpPos = tacOrderQueue[numTacOrdersQueued - 1].point;
			}

			else if ( pos.x != 0 && pos.y != 0  )
			{

				pos.z = land->getTerrainElevation( pos );
				eye->projectZ( tmpPos, screenPos1 );
				eye->projectZ( pos, screenPos2 );

				Stuff::Vector4D vertices[2];
				vertices[0].x = screenPos1.x;
				vertices[0].y = screenPos1.y;
				vertices[1].x = screenPos2.x;
				vertices[1].y = screenPos2.y;

				vertices[0].z = vertices[1].z = 0.1f;
				vertices[0].w = vertices[1].w = 0.9999f;

				LineElement line( vertices[0], vertices[1], 0xffff0000, 0, -1 );
				line.draw();
				
				tmpPos = pos;
			}
		}

		for ( int i = 0; i < numTacOrdersQueued; i++ )
		{
			tacOrderQueue[i].marker->render();
			
			Stuff::Vector4D screenPos1;
			Stuff::Vector4D screenPos2;

			eye->projectZ( tacOrderQueue[i].point, screenPos1 );
			eye->projectZ( tmpPos, screenPos2 );

   			Stuff::Vector4D vertices[2];
   			vertices[0].x = screenPos1.x;
   			vertices[0].y = screenPos1.y;
   			vertices[1].x = screenPos2.x;
   			vertices[1].y = screenPos2.y;

   			vertices[0].z = vertices[1].z = 0.1f;
   			vertices[0].w = vertices[1].w = 0.9999f;

   			LineElement line( vertices[0], vertices[1], 0xffff0000, 0, -1 );
   			line.draw();

			tmpPos = tacOrderQueue[i].point;
		}

	}

}

void MechWarrior::updateDrawWaypointPath()
{
	if ( tacOrderQueue )
	{
		for ( int i = 0; i < numTacOrdersQueued; i++ )
		{
			tacOrderQueue[i].marker->setObjectParameters(tacOrderQueue[i].point,((ObjectAppearance*)tacOrderQueue[i].marker)->rotation,false,0,0);
			tacOrderQueue[i].marker->recalcBounds();
	
			//------------------------------------------------
			tacOrderQueue[i].marker->update();
			
			float zPos = land->getTerrainElevation(tacOrderQueue[i].point);
			tacOrderQueue[i].point.z = zPos;
		}
	}
}

bool MechWarrior::isCloseToFirstTacOrder( Stuff::Vector3D& pos )
{
	if ( tacOrderQueue )
	{
		Stuff::Vector3D tmp;
		tmp.Subtract(tacOrderQueue[0].point, pos );
		if ( tmp.GetLength() < 5 ) // random amount
			return true;
	}

	return false;
}

//---------------------------------------------------------------------------

void MechWarrior::setup (void) {

	TacOrderQueuePos = 0;
	for (long i = 0; i < MAX_WARRIORS; i++) {
		warriorList[i] = new MechWarrior;
		warriorList[i]->index = i;
		warriorList[i]->used = false;
		if (warriorList[i] == NULL)
			Fatal(i, " MechWarrior.setup: NULL warrior ");
	}

}

//---------------------------------------------------------------------------

MechWarrior* MechWarrior::newWarrior (void) {

	for (long i = 1; i < MAX_WARRIORS; i++)
		if (!warriorList[i]->used) {
			warriorList[i]->used = true;
			return(warriorList[i]);
		}
	return(NULL);
}

//---------------------------------------------------------------------------

void MechWarrior::freeWarrior (MechWarrior* warrior) {

	warrior->clear();
}

void MechWarrior::changeAttackRange( long newRange )
{
	if ( curTacOrder.isCombatOrder() )
	{
		TacticalOrder order;
		// hope this assignment operator works
		order = curTacOrder;
		order.attackParams.range = (FireRangeType)newRange;
		order.pack( 0, 0 );
		getVehicle()->handleTacticalOrder( order );
	}
}

//---------------------------------------------------------------------------

void MechWarrior::shutdown (void) {

	for (long i = 0; i < MAX_WARRIORS; i++) {
		if (warriorList[i]) {
			delete warriorList[i];
			warriorList[i] = NULL;
		}
	}
}

BldgAppearance* MechWarrior::getWayPointMarker( const Stuff::Vector3D& pos, const char* name )
{
	int appearanceType = (BLDG_TYPE << 24);
	AppearanceTypePtr buildingAppearanceType = appearanceTypeList->getAppearance(appearanceType, name );

	BldgAppearance* appearance = new BldgAppearance;
	gosASSERT(appearance != NULL);

	//--------------------------------------------------------------
	// The only appearance type for buildings is MLR_APPEARANCE.
	gosASSERT(buildingAppearanceType->getAppearanceClass() == BLDG_TYPE);
	
	appearance->init((BldgAppearanceType*)buildingAppearanceType, (GameObjectPtr)this);

	appearance->setObjectParameters( (const_cast<Stuff::Vector3D&>(pos)), 0, 1, 0, 0);
	appearance->recalcBounds();
	appearance->update();

	return appearance;
}

//---------------------------------------------------------------------------

long MechWarrior::calcTacOrder (long goalAction,
								long goalWID,
								Stuff::Vector3D goalLocation,
								float controlRadius,
								long aggressiveness,
								long searchDepth,
								float turretRange,
								long turretThreat,
								TacticalOrder& newTacOrder) {

	if (getCommander() == Commander::home)
		STOP(("Mechwarrior.runBrain: player pilot using goalplanning"));

	bool canCapture = ((getVehicle()->getObjectClass() != GROUNDVEHICLE) && (getVehicle()->moveType != MOVETYPE_AIR));

	//--------------------------------------------------------------------
	// If we have no goal, we may want to at least assess turret threat...
	bool alreadyAssessedTurrets = false;
	if (goalWID == 0) {
		if (turretRange > 0.0) {
			//if (getLastTarget())
			//	if (getLastTarget()->getThreatRating() > turretThreat)
			//		turretThreat = getLastTarget()->getThreatRating();
			GameObjectPtr turretTarget = calcTurretThreats(turretRange, turretThreat);
			if (turretTarget) {
				if (turretTarget->getObjectClass() == TURRET)
					goalAction = GOAL_ACTION_ATTACK;
				else if (turretTarget->getCaptureBlocker(getVehicle()))
					goalAction = GOAL_ACTION_ATTACK;
				else if (canCapture)
					goalAction = GOAL_ACTION_CAPTURE;
				else
					goalAction = GOAL_ACTION_ATTACK;
				goalWID = turretTarget->getWatchID();
				goalLocation.Zero();
			}
			alreadyAssessedTurrets = true;
		}
	}

	//------------------------------------------------------
	// At this point, if we don't have any goal, we're done.
	GameObjectPtr mainTarget = NULL;
	if (goalWID) {
		mainTarget = ObjectManager->getByWatchID(goalWID);
		if (!mainTarget) {
			goalAction = GOAL_ACTION_NONE;
			goalWID = 0;
			goalLocation.Zero();
			lastGoalPathSize = 0;
			return(-1);
		}
	}

	//-----------------------------------------------------------
	// Preserve our original goal, as it may change short-term...
	long currentAction = goalAction;
	long currentWID = goalWID;
	Stuff::Vector3D currentLocation = goalLocation;
	GameObjectPtr currentTarget = mainTarget;

	OrderOriginType orderOrigin = ORDER_ORIGIN_COMMANDER;
	if (getCommander() == Commander::home)
		orderOrigin = ORDER_ORIGIN_PLAYER;

	if (currentAction == GOAL_ACTION_UNDECIDED) {
		//-------------------
		// Pick one for me...
		if (currentTarget)
			if (currentTarget->isMover())
				if (getTeam()->isEnemy(currentTarget->getTeam())) {
					if (!currentTarget->isDisabled())
						currentAction = GOAL_ACTION_ATTACK;
					}
				else {
					if (!currentTarget->isDisabled())
						currentAction = GOAL_ACTION_GUARD;
				}
			else
				switch (currentTarget->getObjectClass()) {
					case BUILDING:
						if (currentTarget->getFlag(OBJECT_FLAG_CONTROLBUILDING))
							currentAction = GOAL_ACTION_ATTACK; //CONTROL_ACTION_CAPTURE;
						break;
					case TURRET:
						currentAction = GOAL_ACTION_ATTACK;
						break;
				}
		else {
		}
	}

	if (!alreadyAssessedTurrets && (turretRange > 0.0)) {
		if (currentAction == GOAL_ACTION_ATTACK)
			if (getLastTarget())
				if (getLastTarget()->getThreatRating() > turretThreat)
					turretThreat = getLastTarget()->getThreatRating();
		GameObjectPtr turretTarget = calcTurretThreats(turretRange, turretThreat);
		if (turretTarget) {
			if (turretTarget->getObjectClass() == TURRET)
				currentAction = GOAL_ACTION_ATTACK;
			else if (turretTarget->getCaptureBlocker(getVehicle()))
				goalAction = GOAL_ACTION_ATTACK;
			else
				currentAction = canCapture ? GOAL_ACTION_CAPTURE : GOAL_ACTION_ATTACK;
			currentWID = turretTarget->getWatchID();
			currentLocation.Zero();
			currentTarget = turretTarget;
		}
	}

	TacticalOrder newestTacOrder;
	newestTacOrder.init();
	if (newTacOrderReceived[ORDERSTATE_GENERAL])
		newestTacOrder = tacOrder[ORDERSTATE_GENERAL];
	else if (newTacOrderReceived[ORDERSTATE_PLAYER])
		newestTacOrder = tacOrder[ORDERSTATE_PLAYER];
	else if (curTacOrder.code != TACTICAL_ORDER_NONE)
		newestTacOrder = curTacOrder;

	if (!canCapture)
		if (currentAction == GOAL_ACTION_CAPTURE)
			STOP(("Mechwarrior.calcTacOrder: vehicle cannot capture")); //currentAction = GOAL_ACTION_ATTACK;

	newTacOrder.init(ORDER_ORIGIN_COMMANDER, TACTICAL_ORDER_NONE);
	if (currentAction == GOAL_ACTION_MOVE) {
		GlobalPathStep globalPath[50];
		long numSteps = 0;
		if (currentTarget)
			numSteps = getVehicle()->calcGlobalPath(globalPath, currentTarget, NULL, true);
		else
			numSteps = getVehicle()->calcGlobalPath(globalPath, NULL, &currentLocation, true);
		if (numSteps > 0) {
			lastGoalPathSize = numSteps;
			for (long i = 0; i < numSteps; i++)
				lastGoalPath[i] = globalPath[i].thruArea;
		}
		//-----------------------------------------------------------
		// What's the next subgoal to overcome before we can move on?
		for (long i = 0; i < numSteps; i++) {
			GlobalMapAreaPtr area = &GlobalMoveMap[getVehicle()->moveLevel]->areas[globalPath[i].thruArea];
			if (area->type == AREA_TYPE_GATE) {
				GatePtr gate = (GatePtr)ObjectManager->getByWatchID(area->ownerWID);
				if (gate && !gate->isDestroyed() && (gate->getTeam() != getTeam())) {
					GameObjectPtr controlBuilding = ObjectManager->getByWatchID(gate->parent);
					if (canCapture && controlBuilding && (searchDepth > 0)) {
						Stuff::Vector3D tempLocation;
						tempLocation.Zero();
						long result = calcTacOrder(GOAL_ACTION_CAPTURE, controlBuilding->getWatchID(), tempLocation, -1.0, aggressiveness, searchDepth - 1, -1.0, 0, newTacOrder);
						return(result);
						}
					else {
						newTacOrder.init(orderOrigin, TACTICAL_ORDER_ATTACK_OBJECT);
						newTacOrder.subOrder = true;
						newTacOrder.targetWID = gate->getWatchID();
						newTacOrder.attackParams.type = ATTACK_TO_DESTROY;
						newTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
						newTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
						newTacOrder.attackParams.tactic = newestTacOrder.attackParams.tactic;
						newTacOrder.attackParams.pursue = true;
						newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
						newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
						newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
						return(0);
					}
				}
			}
			else if (area->type == AREA_TYPE_WALL) {
				BuildingPtr wall = (BuildingPtr)ObjectManager->getByWatchID(area->ownerWID);
				if (wall && (!wall->isDestroyed())) {
					newTacOrder.init(orderOrigin, TACTICAL_ORDER_ATTACK_OBJECT);
					newTacOrder.subOrder = true;
					newTacOrder.targetWID = wall->getWatchID();
					newTacOrder.attackParams.type = ATTACK_TO_DESTROY;
					newTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
					newTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
					newTacOrder.attackParams.tactic = newestTacOrder.attackParams.tactic;
					newTacOrder.attackParams.pursue = true;
					newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
					newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
					newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
					return(0);
				}
			}
		}

		//------------------------------------------------------------------------------------------
		// If we made it here, then there are no steps in the global path blocked. So, let's just go
		// straight for the main goal location...
		if (currentTarget) {
			long fromArea = -1;	// change this?
			//-------------------------------------------------
			// Pick the best move goal based upon the object...
			Stuff::Vector3D goal;
			if (currentTarget->isMover())
				goal = currentTarget->getPosition();
			else {
				//------------------------------------------------
				// If not a mover, then pick an open cell adjacent
				// to it. If it happens to be adjacent to blocked
				// cells, this could be off...
				if (currentTarget->isBuilding()) {
					BuildingPtr building = (BuildingPtr)currentTarget;
					long goalRow = 0, goalCol = 0;
					bool foundGoal = building->calcAdjacentAreaCell(getVehicle()->moveLevel, fromArea, goalRow, goalCol);
					if (foundGoal)
						land->cellToWorld(goalRow, goalCol, goal);
					else {
						Stuff::Vector3D objectPos = currentTarget->getPosition();
						goal = getVehicle()->calcOffsetMoveGoal(objectPos);
					}
					}
				else {
					Stuff::Vector3D objectPos = currentTarget->getPosition();
					goal = getVehicle()->calcOffsetMoveGoal(objectPos);
				}
			}
			newTacOrder.init(orderOrigin, TACTICAL_ORDER_MOVETO_OBJECT);
			newTacOrder.subOrder = true;
			newTacOrder.targetWID = currentTarget->getWatchID();
			newTacOrder.selectionIndex = -1;
			newTacOrder.moveParams.fromArea = fromArea;
			newTacOrder.moveParams.wayPath.points[0] = goal.x;
			newTacOrder.moveParams.wayPath.points[1] = goal.y;
			newTacOrder.moveParams.wayPath.points[2] = goal.z;
			newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
			newTacOrder.moveParams.faceObject = true;
			newTacOrder.moveParams.wait = false;
			newTacOrder.moveParams.mode = MOVE_MODE_NORMAL;
			newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
			newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
			}
		else {
			newTacOrder.init(orderOrigin, TACTICAL_ORDER_MOVETO_POINT);
			newTacOrder.subOrder = true;
			newTacOrder.setWayPoint(0, currentLocation);
			newTacOrder.moveParams.wayPath.mode[0] =  newestTacOrder.moveParams.wayPath.mode[0];
			newTacOrder.moveParams.wait = false;
			newTacOrder.moveParams.mode = MOVE_MODE_NORMAL;
			newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
			newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
		}
		}
	else if (currentAction == GOAL_ACTION_ATTACK) {
		GlobalPathStep globalPath[50];
		long numSteps = getVehicle()->calcGlobalPath(globalPath, currentTarget, NULL, true);
		if (numSteps > 0) {
			lastGoalPathSize = numSteps;
			for (long i = 0; i < numSteps; i++)
				lastGoalPath[i] = globalPath[i].thruArea;
		}
		//-----------------------------------------------------------
		// What's the next subgoal to overcome before we can move on?
		for (long i = 0; i < numSteps; i++) {
			GlobalMapAreaPtr area = &GlobalMoveMap[getVehicle()->moveLevel]->areas[globalPath[i].thruArea];
			if (area->type == AREA_TYPE_GATE) {
				GatePtr gate = (GatePtr)ObjectManager->getByWatchID(area->ownerWID);
				if (gate && !gate->isDestroyed() && (gate->getTeam() != getTeam())) {
					GameObjectPtr controlBuilding = ObjectManager->getByWatchID(gate->parent);
					if (canCapture && controlBuilding && (searchDepth > 0)) {
						Stuff::Vector3D tempLocation;
						tempLocation.Zero();
						long result = calcTacOrder(GOAL_ACTION_CAPTURE, controlBuilding->getWatchID(), tempLocation, -1.0, aggressiveness, searchDepth - 1, -1.0, 0, newTacOrder);
						return(result);
						}
					else {
						newTacOrder.init(orderOrigin, TACTICAL_ORDER_ATTACK_OBJECT);
						newTacOrder.subOrder = true;
						newTacOrder.targetWID = gate->getWatchID();
						newTacOrder.attackParams.type = ATTACK_TO_DESTROY;
						newTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
						newTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
						newTacOrder.attackParams.tactic = newestTacOrder.attackParams.tactic;
						newTacOrder.attackParams.pursue = true;
						newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
						newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
						newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
						return(0);
					}
				}
			}
			else if (area->type == AREA_TYPE_WALL) {
				BuildingPtr wall = (BuildingPtr)ObjectManager->getByWatchID(area->ownerWID);
				if (wall && (!wall->isDestroyed())) {
					newTacOrder.init(orderOrigin, TACTICAL_ORDER_ATTACK_OBJECT);
					newTacOrder.subOrder = true;
					newTacOrder.targetWID = wall->getWatchID();
					newTacOrder.attackParams.type = ATTACK_TO_DESTROY;
					newTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
					newTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
					newTacOrder.attackParams.tactic = newestTacOrder.attackParams.tactic;
					newTacOrder.attackParams.pursue = true;
					newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
					newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
					newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
					return(0);
				}
			}
		}

		//------------------------------------------------------------------------------------------
		// If we made it here, then there are no steps in the global path blocked. So, let's just go
		// straight for the main target...
		if (currentTarget->isDisabled())
			return(1);
		newTacOrder.init(orderOrigin, TACTICAL_ORDER_ATTACK_OBJECT);
		newTacOrder.subOrder = true;
		newTacOrder.targetWID = currentTarget->getWatchID();
		newTacOrder.attackParams.type = ATTACK_TO_DESTROY;
		newTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
		newTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
		newTacOrder.attackParams.tactic = newestTacOrder.attackParams.tactic;
		newTacOrder.attackParams.pursue = true;
		newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
		newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
		newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
		}
	else if (currentAction == GOAL_ACTION_CAPTURE) {
		bool mainGoalControlled = (currentTarget->getTeam() == getTeam());
		if (!mainGoalControlled) {
			//---------------------------------------------
			// Are there any enemies blocking this capture?
			//float distanceToGoal = getVehicle()->distanceFrom(currentTarget->getPosition());
			GameObjectPtr blockerList[MAX_MOVERS];
			long numBlockers = currentTarget->getCaptureBlocker(getVehicle(), blockerList);
			if (numBlockers > 0) {
				//-----------------------------------------------------
				// Why is it blocked, and can we do something about it?
				/*
				if ((origin == ORDER_ORIGIN_SELF) && (numBlockers > 3)) {
					TacticalOrder alarmTacOrder;
					alarmTacOrder.init(orderOrigin,  TACTICAL_ORDER_ATTACK_OBJECT);
					alarmTacOrder.playerSubOrder = true;
					alarmTacOrder.targetWID = target->getWatchID();
					alarmTacOrder.attackParams.type = ATTACK_TO_DESTROY;
					alarmTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
					alarmTacOrder.attackParams.aimLocation = -1;
					alarmTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
					alarmTacOrder.attackParams.pursue = true;
					alarmTacOrder.attackParams.obliterate = false;
					alarmTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
					warrior->setAlarmTacOrder(alarmTacOrder, 255);
					}
				else {
				*/
				GameObjectPtr curTarget = getLastTarget();
				long blockerToAttack = -1;
				for (long i = 0; i < numBlockers; i++)
					if (blockerList[i] == curTarget) {
						blockerToAttack = i;
						break;
					}
				if (blockerToAttack == -1)
					blockerToAttack = 0;

				currentAction = GOAL_ACTION_ATTACK;
				currentWID = blockerList[blockerToAttack]->getWatchID();
				currentLocation.Zero();
				currentTarget = blockerList[blockerToAttack];
				//}
			}
			GlobalPathStep globalPath[50];
			long numSteps = getVehicle()->calcGlobalPath(globalPath, currentTarget, NULL, true);
			if (numSteps > 0) {
				lastGoalPathSize = numSteps;
				for (long i = 0; i < numSteps; i++)
					lastGoalPath[i] = globalPath[i].thruArea;
			}
			//-----------------------------------------------------------
			// What's the next subgoal to overcome before we can move on?
			for (long i = 0; i < numSteps; i++) {
				GlobalMapAreaPtr area = &GlobalMoveMap[getVehicle()->moveLevel]->areas[globalPath[i].thruArea];
				if (area->type == AREA_TYPE_GATE) {
					GatePtr gate = (GatePtr)ObjectManager->getByWatchID(area->ownerWID);
					if (gate && !gate->isDestroyed() && (gate->getTeam() != getTeam())) {
						GameObjectPtr controlBuilding = ObjectManager->getByWatchID(gate->parent);
						if (controlBuilding && (searchDepth > 0)) {
							Stuff::Vector3D tempLocation;
							tempLocation.Zero();
							calcTacOrder(GOAL_ACTION_CAPTURE, controlBuilding->getWatchID(), tempLocation, -1.0, aggressiveness, searchDepth - 1, -1.0, 0, newTacOrder);
							return(0);
							}
						else {
							newTacOrder.init(orderOrigin, TACTICAL_ORDER_ATTACK_OBJECT);
							newTacOrder.subOrder = true;
							newTacOrder.targetWID = gate->getWatchID();
							newTacOrder.attackParams.type = ATTACK_TO_DESTROY;
							newTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
							newTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
							newTacOrder.attackParams.tactic = newestTacOrder.attackParams.tactic;
							newTacOrder.attackParams.pursue = true;
							newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
							newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
							newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
							return(0);
						}
					}
				}
				else if (area->type == AREA_TYPE_WALL) {
					BuildingPtr wall = (BuildingPtr)ObjectManager->getByWatchID(area->ownerWID);
					if (wall && (!wall->isDestroyed())) {
						newTacOrder.init(orderOrigin, TACTICAL_ORDER_ATTACK_OBJECT);
						newTacOrder.subOrder = true;
						newTacOrder.targetWID = wall->getWatchID();
						newTacOrder.attackParams.type = ATTACK_TO_DESTROY;
						newTacOrder.attackParams.method = ATTACKMETHOD_RANGED;
						newTacOrder.attackParams.range = FIRERANGE_OPTIMAL;
						newTacOrder.attackParams.tactic = newestTacOrder.attackParams.tactic;
						newTacOrder.attackParams.pursue = true;
						newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
						newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
						newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
						return(0);
					}
				}
			}
			newTacOrder.init(orderOrigin, TACTICAL_ORDER_CAPTURE );
			newTacOrder.subOrder = true;
			newTacOrder.targetWID = currentTarget->getWatchID();
			newTacOrder.attackParams.type = ATTACK_NONE;
			newTacOrder.attackParams.method = ATTACKMETHOD_RAMMING;
			newTacOrder.attackParams.pursue = true;
			newTacOrder.moveParams.fromArea = globalPath[numSteps - 2].thruArea;
			newTacOrder.moveParams.wayPath.mode[0] = newestTacOrder.moveParams.wayPath.mode[0];
			newTacOrder.moveParams.jump = newestTacOrder.moveParams.jump;
			newTacOrder.moveParams.keepMoving = newestTacOrder.moveParams.keepMoving;
			}
		else if (controlRadius > 0.0) {
			//-------------------------------------------------------------------------------------------
			// Main Goal has been captured, so focus on clearing out the area. First, look for turrets...
			bool noTargetSelected = true;
			GameObjectPtr nextTarget = getLastTarget();
			//-----------------------------------------------------------------------
			// If our current target is a turret threat, then just keep attacking it.
			if (nextTarget)
				if (nextTarget->getObjectClass() == TURRET)
					if (getVehicle()->isEnemy(nextTarget->getTeam()))
						if (mainTarget/*getVehicle()*/->distanceFrom(nextTarget->getPosition()) <= ((Turret*)nextTarget)->maxRange)
							noTargetSelected = false;
			if (noTargetSelected) {
				//---------------------------------------
				// Look for any turret threats around us.
				long numTurrets = ObjectManager->getNumTurrets();
				for (long i = 0; i < numTurrets; i++) {
					Turret* turret = ObjectManager->getTurret(i);
					if (!turret->isDisabled())
						if (getVehicle()->isEnemy(turret->getTeam()))
							if (mainTarget/*getVehicle()*/->distanceFrom(turret->getPosition()) <= turret->maxRange) {
								nextTarget = turret;
								noTargetSelected = false;
								break;
							}
				}
			}
			if (noTargetSelected) {
				//----------------------------------------------------------------------------
				// Are we currently targeting an enemy mover that is within our control range?
				if (nextTarget)
					if (nextTarget->isMover())
						if (mainTarget/*getVehicle()*/->distanceFrom(nextTarget->getPosition()) <= controlRadius)
							noTargetSelected = false;
			}
			if (noTargetSelected) {
				//---------------------------------------------------------
				// Let's look at any enemy movers within our control range.
				long numMovers = ObjectManager->getNumMovers();
				MoverPtr biggestMoverThreat = NULL;
				long biggestThreatRating = 0;
				for (long i = 0; i < numMovers; i++) {
					MoverPtr mover = ObjectManager->getMover(i);
					if (getVehicle()->isEnemy(mover->getTeam()))
						if (!mover->isDisabled())
							if (mainTarget/*getVehicle()*/->distanceFrom(mover->getPosition()) <= controlRadius)
								if (mover->getThreatRating() > biggestThreatRating) {
									biggestThreatRating = mover->getThreatRating();
									biggestMoverThreat = mover;
								}
				}
				nextTarget = biggestMoverThreat;
			}
			if (nextTarget) {
				if (nextTarget->getObjectClass() == TURRET) {
					GameObjectPtr turretControl = ObjectManager->getByWatchID(((TurretPtr)nextTarget)->parent);
					if (turretControl && (mainTarget/*getVehicle()*/->distanceFrom(turretControl->getPosition()) < controlRadius))
						if (!turretControl->isDisabled()) {
							Stuff::Vector3D tempLocation;
							tempLocation.Zero();
							calcTacOrder(GOAL_ACTION_CAPTURE, turretControl->getWatchID(), tempLocation, -1.0, aggressiveness, searchDepth - 1, -1.0, 0, newTacOrder);
							return(0);
						}
				}
				Stuff::Vector3D tempLocation;
				tempLocation.Zero();
				calcTacOrder(GOAL_ACTION_ATTACK, nextTarget->getWatchID(), tempLocation, -1.0, aggressiveness, searchDepth, -1.0, 0, newTacOrder);
				return(0);
			}
		}
	}
	else if (currentAction == GOAL_ACTION_GUARD) {
	}

//	long numBlockers = controlObject->getCaptureBlocker (GameObjectPtr capturingMover, GameObjectPtr* blockerList) {
	return(0);
}

//---------------------------------------------------------------------------

void MechWarrior::initGoalManager (long poolSize) {

	goalManager = new GoalManager;
	gosASSERT(goalManager != NULL);
	goalManager->setup(poolSize);
	goalManager->build();
	//--------------------------------------
	// For now, process this map manually...

}

//---------------------------------------------------------------------------

void MechWarrior::logPilots (GameLogPtr log) {

	for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
		MoverPtr mover = ObjectManager->getMover(i);
		if (mover) {
			char s[256];
			sprintf(s, "pilot = (%d)%s",
				i,
				mover->getPilot()->getName());
			log->write(s);
			sprintf(s, "     vehicle = (%d)%s",
				mover->getPartId(),
				mover->getName());
			log->write(s);
		}
	}
}

void MechWarrior::copyToData (MechWarriorData &data)
{
	data.used = used;

	strcpy(data.name,name);
	strcpy(data.callsign,callsign);
	strcpy(data.videoStr,videoStr);
	strcpy(data.audioStr,audioStr);
	strcpy(data.brainStr,brainStr);
	data.index = index;
	data.paintScheme = paintScheme;
	data.photoIndex = photoIndex;
	
	data.rank = rank;
	memcpy(data.skills,skills,sizeof(char)*NUM_SKILLS);
	data.professionalism = professionalism;
	data.professionalismModifier = professionalismModifier;
	data.decorum = decorum;
	data.decorumModifier = decorumModifier;
	data.aggressiveness = aggressiveness;
	data.courage = courage;
	data.baseCourage = baseCourage;
	
	data.wounds = wounds;
	data.health = health;
	data.status = status;
	data.escapesThruEjection = escapesThruEjection;
	data.radioLog = radioLog;
	data.notMineYet = notMineYet;
	
	data.teamId = teamId;
	data.vehicleWID = vehicleWID;
	
	memcpy(data.numSkillUses,numSkillUses,sizeof(long) * NUM_SKILLS * NUM_COMBAT_STATS);
	memcpy(data.numSkillSuccesses,numSkillSuccesses, sizeof(long) * NUM_SKILLS *NUM_COMBAT_STATS);
	memcpy(data.numMechKills,numMechKills,sizeof(long) * NUM_VEHICLE_CLASSES * NUM_COMBAT_STATS);
	memcpy(data.numPhysicalAttacks,numPhysicalAttacks,sizeof(long) * NUM_PHYSICAL_ATTACKS * NUM_COMBAT_STATS);
	memcpy(data.skillRank,skillRank,sizeof(float) * NUM_SKILLS);
	memcpy(data.skillPoints,skillPoints,sizeof(float) * NUM_SKILLS);
	memcpy(data.originalSkills,originalSkills,sizeof(char) * NUM_SKILLS);
	memcpy(data.startingSkills,startingSkills,sizeof(char) * NUM_SKILLS);
	memcpy(data.specialtySkills,specialtySkills,sizeof(bool) * NUM_SPECIALTY_SKILLS);
	memcpy(data.killed,killed,sizeof(GameObjectWatchID) * (MAX_MOVERS / 3));
	data.numKilled = numKilled;
	
	data.descID = descID;
	data.nameIndex = nameIndex;
	
	data.timeOfLastOrders = timeOfLastOrders;
	memcpy(data.attackers,attackers,sizeof(AttackerRec) * MAX_ATTACKERS);
	data.numAttackers = numAttackers;
	data.attackRadius = attackRadius;
	
	memcpy(data.memory,memory,sizeof(MemoryCell) * NUM_MEMORY_CELLS);
	memcpy(data.debugStrings,debugStrings,sizeof(char) * NUM_PILOT_DEBUG_STRINGS * MAXLEN_PILOT_DEBUG_STRING);
	
	data.brainUpdate = brainUpdate;
	data.combatUpdate = combatUpdate;
	data.movementUpdate = movementUpdate;
	memcpy(data.weaponsStatus,weaponsStatus,sizeof(long) * MAX_WEAPONS_PER_MOVER);
	data.weaponsStatusResult = weaponsStatusResult;
	
	data.useGoalPlan = useGoalPlan;
	data.mainGoalAction = mainGoalAction;
	data.mainGoalObjectWID = mainGoalObjectWID;
	data.mainGoalLocation = mainGoalLocation;
	data.mainGoalControlRadius = mainGoalControlRadius;
	data.lastGoalPathSize = lastGoalPathSize;
	memcpy(data.lastGoalPath,lastGoalPath,sizeof(short) * MAX_GLOBAL_PATH);
	
	memcpy(data.newTacOrderReceived,newTacOrderReceived, sizeof(bool) * NUM_ORDERSTATES);
	memcpy(data.tacOrder,tacOrder, sizeof(TacticalOrder) * NUM_ORDERSTATES);
	data.lastTacOrder = lastTacOrder;
	data.curTacOrder = curTacOrder;
	memcpy(data.alarm,alarm,sizeof(PilotAlarm) * NUM_PILOT_ALARMS);
	memcpy(data.alarmHistory,alarmHistory, sizeof(PilotAlarm) * NUM_PILOT_ALARMS);	// used by brain update in ABL
	data.alarmPriority = alarmPriority;
	data.curPlayerOrderFromQueue = curPlayerOrderFromQueue;
	data.tacOrderQueueLocked = tacOrderQueueLocked;
	data.tacOrderQueueExecuting = tacOrderQueueExecuting;
	data.tacOrderQueueLooping = tacOrderQueueLooping;
	data.numTacOrdersQueued = numTacOrdersQueued;	

	//Pilots which are not used may not have this assigned.
	if (tacOrderQueue)
		memcpy(data.tacOrderQueue,tacOrderQueue,sizeof(QueuedTacOrder) * MAX_QUEUED_TACORDERS_PER_WARRIOR);

	data.tacOrderQueueIndex = tacOrderQueueIndex;
	data.nextTacOrderId = nextTacOrderId;
	data.lastTacOrderId = lastTacOrderId;
	
	data.coreScanTargetWID = coreScanTargetWID;
	data.coreAttackTargetWID = coreAttackTargetWID;
	data.coreMoveTargetWID = coreMoveTargetWID;
	data.targetPriorityList = targetPriorityList;
	data.brainState = brainState;
	data.willHelp = willHelp;
	
	data.moveOrders.copy(moveOrders);
	data.attackOrders = attackOrders;
	data.situationOrders = situationOrders;
	data.lastTargetWID = lastTargetWID;
	data.lastTargetTime = lastTargetTime;
	data.lastTargetObliterate = lastTargetObliterate;
	data.lastTargetFriendly = lastTargetFriendly;
	data.lastTargetConserveAmmo = lastTargetConserveAmmo;
	data.keepMoving = keepMoving;
	
	data.orderState = orderState;
	
	data.debugFlags = debugFlags;
	
	data.oldPilot = oldPilot;

	if (brain)
		data.warriorBrainHandle = brain->getRealId();
	else
		data.warriorBrainHandle = -1;
}

//---------------------------------------------------------------------------
void MechWarrior::copyFromData (MechWarriorData &data)
{
	used = data.used;

	strcpy(name,data.name);
	strcpy(callsign,data.callsign);
	strcpy(videoStr,data.videoStr);
	strcpy(audioStr,data.audioStr);
	strcpy(brainStr,data.brainStr);

	index = data.index;
	paintScheme = data.paintScheme;
	photoIndex = data.photoIndex;
	
	rank = data.rank;
	memcpy(skills,data.skills,sizeof(char)*NUM_SKILLS);

	professionalism = data.professionalism;
	professionalismModifier = data.professionalismModifier;
	decorum = data.decorum;
	decorumModifier = data.decorumModifier;
	aggressiveness = data.aggressiveness;
	courage = data.courage;
	baseCourage = data.baseCourage;
	
	wounds = data.wounds;
	health = data.health;
	status = data.status;
	escapesThruEjection = data.escapesThruEjection;
	radioLog = data.radioLog;
	notMineYet = data.notMineYet;
	
	teamId = data.teamId;
	vehicleWID = data.vehicleWID;
	
	memcpy(numSkillUses,data.numSkillUses,sizeof(long) * NUM_SKILLS * NUM_COMBAT_STATS);
	memcpy(numSkillSuccesses,data.numSkillSuccesses,sizeof(long) * NUM_SKILLS *NUM_COMBAT_STATS);
	memcpy(numMechKills,data.numMechKills,sizeof(long) * NUM_VEHICLE_CLASSES * NUM_COMBAT_STATS);
	memcpy(numPhysicalAttacks,data.numPhysicalAttacks,sizeof(long) * NUM_PHYSICAL_ATTACKS * NUM_COMBAT_STATS);
	memcpy(skillRank,data.skillRank,sizeof(float) * NUM_SKILLS);
	memcpy(skillPoints,data.skillPoints,sizeof(float) * NUM_SKILLS);
	memcpy(originalSkills,data.originalSkills,sizeof(char) * NUM_SKILLS);
	memcpy(startingSkills,data.startingSkills,sizeof(char) * NUM_SKILLS);
	memcpy(specialtySkills,data.specialtySkills,sizeof(bool) * NUM_SPECIALTY_SKILLS);
	memcpy(killed,data.killed,sizeof(GameObjectWatchID) * (MAX_MOVERS / 3));
	numKilled = data.numKilled;
	
	descID = data.descID;
	nameIndex = data.nameIndex;
	
	timeOfLastOrders = data.timeOfLastOrders;
	memcpy(attackers,data.attackers,sizeof(AttackerRec) * MAX_ATTACKERS);
	numAttackers = data.numAttackers;
	attackRadius = data.attackRadius;
	
	memcpy(memory,data.memory,sizeof(MemoryCell) * NUM_MEMORY_CELLS);
	memcpy(debugStrings,data.debugStrings,sizeof(char) * NUM_PILOT_DEBUG_STRINGS * MAXLEN_PILOT_DEBUG_STRING);
	
	brainUpdate = data.brainUpdate;
	combatUpdate = data.combatUpdate;
	movementUpdate = data.movementUpdate;
	memcpy(weaponsStatus,data.weaponsStatus,sizeof(long) * MAX_WEAPONS_PER_MOVER);
	weaponsStatusResult = data.weaponsStatusResult;
	
	useGoalPlan = data.useGoalPlan;
	mainGoalAction = data.mainGoalAction;
	mainGoalObjectWID = data.mainGoalObjectWID;
	mainGoalLocation = data.mainGoalLocation;
	mainGoalControlRadius = data.mainGoalControlRadius;
	lastGoalPathSize = data.lastGoalPathSize;
	memcpy(lastGoalPath,data.lastGoalPath,sizeof(short) * MAX_GLOBAL_PATH);
	
	memcpy(newTacOrderReceived, data.newTacOrderReceived,sizeof(bool) * NUM_ORDERSTATES);
	memcpy(tacOrder, data.tacOrder,sizeof(TacticalOrder) * NUM_ORDERSTATES);
	lastTacOrder = data.lastTacOrder;
	curTacOrder = data.curTacOrder;
	memcpy(alarm,data.alarm,sizeof(PilotAlarm) * NUM_PILOT_ALARMS);
	memcpy(alarmHistory,data.alarmHistory, sizeof(PilotAlarm) * NUM_PILOT_ALARMS);	// used by brain update in ABL
	alarmPriority = data.alarmPriority;
	curPlayerOrderFromQueue = data.curPlayerOrderFromQueue;
	tacOrderQueueLocked = data.tacOrderQueueLocked;
	tacOrderQueueExecuting = data.tacOrderQueueExecuting;
	tacOrderQueueLooping = data.tacOrderQueueLooping;
	numTacOrdersQueued = data.numTacOrdersQueued;
	memcpy(tacOrderQueue,data.tacOrderQueue,sizeof(QueuedTacOrder) * MAX_QUEUED_TACORDERS_PER_WARRIOR);
	for (long i=0;i<MAX_QUEUED_TACORDERS_PER_WARRIOR;i++)
	{
		//Set the Marker pointers to NULL and then recreate them from the movemode data.
		tacOrderQueue[i].marker = NULL;

		if ( tacOrderQueue[i].moveMode == TRAVEL_MODE_SLOW )
			tacOrderQueue[i].marker = getWayPointMarker( tacOrderQueue[i].point, "WalkWayPoint" );

		else if ( tacOrderQueue[i].moveMode == TRAVEL_MODE_FAST )
			tacOrderQueue[i].marker = getWayPointMarker( tacOrderQueue[i].point, "RunWayPoint" );

		else if ( tacOrderQueue[i].moveMode == TRAVEL_MODE_JUMP )
			tacOrderQueue[i].marker = getWayPointMarker( tacOrderQueue[i].point, "JumpWayPoint" );
	}
	tacOrderQueueIndex = data.tacOrderQueueIndex;
	nextTacOrderId = data.nextTacOrderId;
	lastTacOrderId = data.lastTacOrderId;
	
	coreScanTargetWID = data.coreScanTargetWID;
	coreAttackTargetWID = data.coreAttackTargetWID;
	coreMoveTargetWID = data.coreMoveTargetWID;
	targetPriorityList = data.targetPriorityList;
	brainState = data.brainState;
	willHelp = data.willHelp;
	
	data.moveOrders.copyTo(moveOrders);

	attackOrders = data.attackOrders;
	situationOrders = data.situationOrders;
	lastTargetWID = data.lastTargetWID;
	lastTargetTime = data.lastTargetTime;
	lastTargetObliterate = data.lastTargetObliterate;
	lastTargetFriendly = data.lastTargetFriendly;
	lastTargetConserveAmmo = data.lastTargetConserveAmmo;
	keepMoving = data.keepMoving;
								
	orderState = data.orderState;
								
	debugFlags = data.debugFlags;
	
	oldPilot = data.oldPilot;

	warriorBrainHandle = data.warriorBrainHandle;
}

#define USE_ABL_LOAD
//---------------------------------------------------------------------------
void MechWarrior::init (MechWarriorData data)
{
	init(false);

	//First, copy all of the data to the class.
	copyFromData(data);

#ifdef USE_ABL_LOAD
	//Set the brain Pointer from the Handle.
	// NOT IMPLEMENTED BY GLENNDOR YET!!!!!!!!!
	if (warriorBrainHandle > -1) 
	{
		brain = ABLi_getModule(warriorBrainHandle);

		//-----------------------------------------
		// Set up the pilot alarm callback table...
		for (long i = 0; i < NUM_PILOT_ALARMS; i++)
			brainAlarmCallback[i] = brain->findFunction(pilotAlarmFunctionName[i], true);
	}
	else
	{
		brain = NULL;
	}
#else
	long numErrors, numLinesProcessed;
	FullPathFileName brainFileName;

	brainFileName.init(warriorPath, brainStr, ".abl");

	if (brainStr[0])
	{
		long moduleHandle = ABLi_preProcess(brainFileName, &numErrors, &numLinesProcessed);
		gosASSERT(moduleHandle >= 0);

		long error = setBrain(moduleHandle);
		gosASSERT(error == 0);
	}
#endif

	//Set anything else.
	movePathRequest = NULL;

	if (audioStr[0] != 0)
	{
		radio = new Radio;
		long result = radio->init(audioStr,102400,videoStr);		//Size of radio heap should be data driven?
		if (result != NO_ERR)
		{
			delete radio;	//Radio couldn't start, so no radio.
			radio = NULL;
		}
	}

	isPlayingMsg = false;	//Never playing when we reload an in-mission save!

	if (radio)
		radio->setOwner(this);
}

//---------------------------------------------------------------------------
void MechWarrior::save (PacketFilePtr file, long packetNum) 
{
	MechWarriorData warriorData;
	copyToData(warriorData);
	file->writePacket(packetNum,(MemoryPtr)&warriorData,sizeof(warriorData),STORAGE_TYPE_ZLIB);
}

//---------------------------------------------------------------------------
long MechWarrior::Save (PacketFilePtr file, long packetNum)
{
	StaticMechWarriorData staticData;
	staticData.numWarriors 					= numWarriors;
	staticData.numWarriorsInCombat			= numWarriorsInCombat;
	memcpy(staticData.brainsEnabled,brainsEnabled,sizeof(bool) * MAX_TEAMS);
	staticData.minSkill						= minSkill;
	staticData.maxSkill						= maxSkill;
	staticData.increaseCap					= increaseCap;
	staticData.maxVisualRadius				= maxVisualRadius;
	staticData.curEventID					= curEventID;
	staticData.curEventTrigger				= curEventTrigger;

	file->writePacket(packetNum,(MemoryPtr)&staticData,sizeof(StaticMechWarriorData),STORAGE_TYPE_RAW);
	packetNum++;

	for (long i = 0; i < numWarriors; i++)
	{
		MechWarrior::warriorList[i]->save(file,packetNum);
		packetNum++;
	}

	return packetNum;
}

//---------------------------------------------------------------------------
void MechWarrior::load (PacketFilePtr file, long packetNum)
{
	MechWarriorData warriorData;
	file->readPacket(packetNum,(MemoryPtr)&warriorData);
	init(warriorData);
}

//---------------------------------------------------------------------------
long MechWarrior::Load (PacketFilePtr file, long packetNum)
{
	StaticMechWarriorData staticData;
	file->readPacket(packetNum,(MemoryPtr)&staticData);
	packetNum++;

	numWarriors 			= staticData.numWarriors;
	numWarriorsInCombat		= staticData.numWarriorsInCombat;

	memcpy(brainsEnabled,staticData.brainsEnabled,sizeof(bool) * MAX_TEAMS);

	minSkill				= staticData.minSkill;
	maxSkill				= staticData.maxSkill;
	increaseCap				= staticData.increaseCap;
	maxVisualRadius			= staticData.maxVisualRadius;
	curEventID				= staticData.curEventID;
	curEventTrigger			= staticData.curEventTrigger;

	TacOrderQueuePos = 0;

	for (long i = 0; i < numWarriors; i++)
	{
		MechWarrior::warriorList[i]->load(file,packetNum);
		packetNum++;

		//Kind of a hack but Glenn increments numWarriors with each init.
		// Take it back to the real number here!!
		numWarriors--;
	}

	//initGoalManager(200);

	//------------
	if (!sortList) 
	{
		sortList = new SortList;
		if (!sortList)
			Fatal(0, " Unable to create Warrior::sortList ");
		sortList->init(100);
	}

	return packetNum;
}

bool MechWarrior::warriorInUse (char *warriorName)
{
	for (long i=0; i < numWarriors;i++)
	{
		if (S_stricmp(MechWarrior::warriorList[i]->name,warriorName) == 0)
			return true;
	}

	return false;
}
//---------------------------------------------------------------------------


