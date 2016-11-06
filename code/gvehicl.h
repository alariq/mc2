//******************************************************************************************
//
//	gvehicl.h - This file contains the GroundVehicle Class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GVEHICL_H
#define GVEHICL_H

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

#ifndef DGVEHICL_H
#include"dgvehicl.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef DOBJNUM_H
#include"dobjnum.h"
#endif

#ifndef CMPONENT_H
#include"cmponent.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifndef MECHCLASS_H
#include"mechclass.h"
#endif

#ifdef USE_ELEMENTALS
#ifndef DELEMNTL_H
#include<delemntl.h>
#endif
#endif

extern float metersPerWorldUnit;

//***************************************************************************

#define	NUM_GROUNDVEHICLE_HIT_ARCS			4
#define GROUNDVEHICLE_HIT_ARC_FRONT			0
#define	GROUNDVEHICLE_HIT_ARC_REAR			1
#define	GROUNDVEHICLE_HIT_ARC_LEFT			2
#define	GROUNDVEHICLE_HIT_ARC_RIGHT			3

#define NO_APPEARANCE_FOR_VEHICLE			0xFFFD0001
#define	NO_APPEARANCE_TYPE_FOR_VEHICLE		0xFFFD0009
#define	APPEARANCE_NOT_GV_TYPE				0xFFFD000A

#define	NUM_CRITICAL_SPACES_FRONT		1
#define	NUM_CRITICAL_SPACES_LEFT		1
#define	NUM_CRITICAL_SPACES_RIGHT		1
#define	NUM_CRITICAL_SPACES_REAR		1
#define	NUM_CRITICAL_SPACES_TURRET		1

#define MAX_TOADS			10
#define MAX_SEATS			4

typedef enum {
	GROUNDVEHICLE_LOCATION_ANY = -1,
	GROUNDVEHICLE_LOCATION_FRONT,
	GROUNDVEHICLE_LOCATION_LEFT,
	GROUNDVEHICLE_LOCATION_RIGHT,
	GROUNDVEHICLE_LOCATION_REAR,
	GROUNDVEHICLE_LOCATION_TURRET,
	NUM_GROUNDVEHICLE_LOCATIONS
} VehicleLocationType;

typedef enum {
	GROUNDVEHICLE_CHASSIS_WHEELED,
	GROUNDVEHICLE_CHASSIS_TRACKED,
	GROUNDVEHICLE_CHASSIS_HOVER,
	NUM_GROUNDVEHICLE_CHASSIS
} GroundVehicleChassisType;

typedef enum {
	GROUNDVEHICLE_CRITICAL_NO_EFFECT,
	GROUNDVEHICLE_CRITICAL_AMMO_POWER_HIT,
	GROUNDVEHICLE_CRITICAL_FUEL_TANK_HIT,
	GROUNDVEHICLE_CRITICAL_CREW_KILLED,
	GROUNDVEHICLE_CRITICAL_ENGINE_HIT,
	GROUNDVEHICLE_CRITICAL_MAIN_WEAPON_JAMMED,
	GROUNDVEHICLE_CRITICAL_CREW_STUNNED,
	GROUNDVEHICLE_CRITICAL_MOVE_DISABLED,
	GROUNDVEHICLE_CRITICAL_MOVEMENT_HINDERED,
	GROUNDVEHICLE_CRITICAL_HOVERCRAFT_MOVEMENT_HINDERED,
	GROUNDVEHICLE_CRITICAL_TURRET_JAMMED,
	NUM_GROUNDVEHICLE_CRITICAL_HIT_TYPES
} GroundVehicleCriticalHitType;

//******************************************************************************************

class GroundVehicleType : public ObjectType {

	//-------------
	// Data Members

	public:

		unsigned char		chassis;
		unsigned char		maxInternalStructure[NUM_GROUNDVEHICLE_LOCATIONS];

		MoverDynamics		dynamics;								// what dynamics do we use.

		long				crashAvoidSelf;
		long				crashAvoidPath;
		long				crashBlockSelf;
		long				crashBlockPath;
		float				crashYieldTime;
		
		float				explDmg;
		float				explRad;

		// Special Vehicle Info
		long				refitPoints;						// number of refit points remaining
		long				recoverPoints;				
		long				resourcePoints;
		bool				mineSweeper;
		long				mineLayer;
		bool				aerospaceSpotter;
		bool				hoverCraft;

		long				moveType;
		
		bool				pathLocks;
		bool				isSensorContact;
		
		float				LOSFactor;

	//----------------
	//Member Functions

	public:

		void init (void);

		GroundVehicleType (void)
		{
			ObjectType::init();
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);
		
		long loadHotSpots (FitIniFilePtr vehicleFile);
	
		virtual GameObjectPtr createInstance (void);
		
		virtual void destroy (void);

		~GroundVehicleType (void) {
			destroy();
		}

		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);
		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

//---------------------------------------------------------------------------
typedef struct _GroundVehicleData : public MoverData
{
	float				accel;			
	float				velocityMag;	

	float				suspension;
	bool				movementEnabled;

	bool				turretEnabled;
	float				turretRotation;					
	bool				turretBlownThisFrame;

	bool				captureable;
	bool				deadByCrushed;					

	bool				canRefit;						
	bool				canRecover;
	bool				refitting;						
	bool				recovering;
	bool				mineSweeper;
	float				sweepTime;						
	long				mineLayer;
	bool				aerospaceSpotter;
	long				cellColToMine;
	long				cellRowToMine;

	bool				notMineYet;

	long				battleRating;					

	long				descID;							

	float				timeInCurrent;					

	Stuff::Vector3D		dVel;
	Stuff::Vector3D		dRot;
	Stuff::Vector3D		dAcc;
	Stuff::Vector3D		dRVel;
	Stuff::Vector3D		dRacc;
	float				dTime;

	bool				sensorOK;
	bool				isVehiclePilot;

} GroundVehicleData;

class GroundVehicle : public Mover {

	//------------
	//Data Members
	
	public:

		float				accel;			//Current acceleration of vehicle in meters per sec per sec
		float				velocityMag;	//Current Speed of vehicle.  Not a vector quantity!! in M/s

		float				suspension;
		bool				movementEnabled;
		
		bool				turretEnabled;
		float				turretRotation;					// Current Rotation of Turret in Degrees
		bool				turretBlownThisFrame;
		
		bool				captureable;
		bool				deadByCrushed;					//Were we stepped on to die?

		bool				canRefit;									
		bool				canRecover;
		bool				refitting;						// actually working, playing refit animation
		bool				recovering;
		bool				mineSweeper;
		float				sweepTime;						// time since we last swept a mine
		long				mineLayer;
		bool				aerospaceSpotter;
		long				cellColToMine;
		long 				cellRowToMine;
		
		bool				notMineYet;
		
		long				battleRating;					// Override of BattleRating

		long				descID;							//Used by Logistics to Desc.		

		float				timeInCurrent;					//Used by MineLayers to lay mines better

		bool				isVehiclePilot;					//Is this a pilot running away?
		
		//Destruction Coolness
		Stuff::Vector3D		dVel;
		Stuff::Vector3D		dRot;
		Stuff::Vector3D		dAcc;
		Stuff::Vector3D		dRVel;
		Stuff::Vector3D		dRacc;
		float				dTime;
		
	//----------------
	//Member Functions

	public:
		
		virtual void init (bool create);
	
		GroundVehicle (void) {
			init(true);
		}
			
		virtual void init (bool create, ObjectTypePtr objType);

		virtual long init (FitIniFile* vehicleFile);
		
		virtual void destroy (void);

		virtual void setControl (ControlType ctrlType);

		virtual bool crashAvoidanceSystem (void);

		virtual void updateAIControl (void);

		virtual void updatePlayerControl (void);

		virtual void updateNetworkControl (void);

		virtual void updateDynamics (void);

		virtual long update (void);
		
		virtual void render (void);

		virtual void renderShadows (void);
		
		virtual float getStatusRating (void);

		virtual long calcHitLocation (GameObjectPtr attacker, long weaponIndex, long attackSource, long attackType);

		virtual long handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk = false);

		virtual float weaponLocked (long weaponIndex, Stuff::Vector3D targetPosition);

		virtual void pilotingCheck (void);

		virtual void mineCheck (void);

		bool pivotTo (void);

		void updateMoveStateGoal (void);

		bool updateMovePath (float& newRotate, char& newThrottleSetting, long& newMoveState, long& minThrottle, long& maxThrottle);

		bool netUpdateMovePath (float& newRotate, char& newThrottleSetting, long& newMoveState, long& minThrottle, long& maxThrottle);

		void setNextMovePath (char& newThrottleSetting);

		void setControlSettings (float& newRotate, char& newThrottleSetting, long& minThrottle, long& maxThrottle);

		void updateTurret (float newRotate);

		virtual void updateMovement (void);

		virtual void netUpdateMovement (void);

		virtual long calcCV (bool calcMax = false);

		virtual float calcAttackChance (GameObjectPtr target, long aimLocation, float targetTime, long weaponIndex, float modifiers, long* range, Stuff::Vector3D* targetPoint = NULL);

		virtual float getTotalEffectiveness(void);

		virtual bool hitInventoryItem (long itemIndex, bool setupOnly = false);

		virtual void destroyBodyLocation (long location);

		virtual long buildStatusChunk (void);

		virtual long handleStatusChunk (long updateAge, unsigned long chunk);

		virtual long buildMoveChunk (void);

		virtual long handleMoveChunk (unsigned long chunk);

		void calcThrottleLimits (long& minThrottle, long& maxThrottle);

		virtual bool injureBodyLocation (long bodyLocation, float damage);

		virtual long handleWeaponFire (long weaponIndex,
									   GameObjectPtr target,
									   Stuff::Vector3D* targetPoint,
									   bool hit,
									   float entryAngle,
									   long numMissiles,
									   long hitLocation);

		virtual long fireWeapon (GameObjectPtr target, float targetTime, long weaponIndex, long attackType, long aimLocation, Stuff::Vector3D* targetPoint, float &damageDone);

		virtual float relFacingTo (Stuff::Vector3D goal, long bodyLocation = -1);
		
		virtual float relViewFacingTo (Stuff::Vector3D goal) {
			return(relFacingTo(goal, GROUNDVEHICLE_LOCATION_TURRET));
		}

		virtual Stuff::Vector3D getPositionFromHS (long weaponType);

		virtual bool canMove (void) {
			return(movementEnabled && (dynamics.max.groundVehicle.speed != 0) );
		}

		virtual long getSpeedState (void);

		virtual void disable (unsigned long cause);
		
		virtual float getDestructLevel (void)
		{
			float maxArmorLocation = 0.0f;

			//Find MAX armor location.
			for (long curLocation = 0; curLocation < NUM_GROUNDVEHICLE_LOCATIONS; curLocation++) 
			{
				if (armor[curLocation].curArmor > maxArmorLocation)
					maxArmorLocation = armor[curLocation].curArmor;

			}

			return maxArmorLocation + 1;	//Always assume you need one point more than the best armor to be sure.
		}

		//----------------------------------------------
		// Additional ground vehicle-specific routines...		
		bool canRotateTurret (void) {
			return(body[GROUNDVEHICLE_LOCATION_TURRET].damageState == IS_DAMAGE_NONE);
		}

		virtual long getThrottle (void) {
			return(control.settings.groundVehicle.throttle);
		}

		bool isMoving (void) {
			return(control.settings.groundVehicle.throttle != 0);
		}

		bool isReversing (void) {
			return(control.settings.groundVehicle.throttle < 0);
		}

		virtual bool isLayingMines (void) {
			return(pilot && (pilot->getCurTacOrder()->moveParams.mode == MOVE_MODE_MINELAYING));
		}

		virtual bool isMineSweeper (void) {
			return(mineSweeper);
		}

		virtual bool isMineLayer (void) {
			return(mineLayer != 0);
		}

		~GroundVehicle (void)
		{
			destroy();
		}

#ifdef USE_SALVAGE
		virtual bool isCaptureable (long capturingTeamID) {
			//return ((captureable || salvage != NULL) && (getTeamId() != capturingTeamID) && !isDestroyed());
			return ((getFlag(OBJECT_FLAG_CAPTURABLE) || (salvage != NULL)) && (getTeamId() != capturingTeamID) && !isDestroyed());
		}
#else
		virtual bool isCaptureable (long capturingTeamID) {
			//return (captureable && (getTeamId() != capturingTeamID) && !isDestroyed());
			return (getFlag(OBJECT_FLAG_CAPTURABLE) && (getTeamId() != capturingTeamID) && !isDestroyed());
		}
#endif
		
		virtual void handleStaticCollision (void);

		virtual const char* getIfaceName(void) {
			return(longName);
		}
		
		void createVehiclePilot (void);

		float getRefitPoints(void) {
			if (canRefit)
				return(armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor);
			else
				return(0.0);
		}

		float getTotalRefitPoints(void) {
			if (canRefit)
				return(armor[GROUNDVEHICLE_LOCATION_TURRET].maxArmor);
			else
				return(0.0);
		}

		bool burnRefitPoints (float pointsToBurn); 

		float getRecoverPoints(void) {
			if (canRecover)
				return(armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor);
			else
				return(0.0);
		}

		float getTotalRecoverPoints(void) {
			if (canRecover)
				return(armor[GROUNDVEHICLE_LOCATION_TURRET].maxArmor);
			else
				return(0.0);
		}

		bool burnRecoverPoints (float pointsToBurn) {
			if (canRecover && pointsToBurn <= armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor) {
				armor[GROUNDVEHICLE_LOCATION_TURRET].curArmor -= pointsToBurn;
				return(true);
				}
			else
				return(false);
		}

		virtual bool isRefit(){ return canRefit; }
		virtual bool isRecover() { return canRecover; }

		static long loadGameSystem (FitIniFilePtr sysFile);
		
		long updateAnimations (void);
		
		virtual bool isMech (void)
		{
			return false;
		}
		
		virtual bool isVehicle (void)
		{
			if (dynamics.max.groundVehicle.speed != 0.0) 
				return true;
			
			return false;
		}
		
		virtual bool isGuardTower (void)
		{
			if (dynamics.max.groundVehicle.speed != 0.0) 
				return false;
			
			return true;
		}
		
		virtual long setTeamId (long _teamId, bool setup);
		
		virtual float getLOSFactor (void)
		{
			return (((GroundVehicleType *)getObjectType())->LOSFactor);
		}

		virtual void Save (PacketFilePtr file, long packetNum);

		void CopyTo (GroundVehicleData *data);

		void Load (GroundVehicleData *data);
};

//******************************************************************************************

#endif
