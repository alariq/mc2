//***************************************************************************
//
//	mover.h - This file contains the Mover Class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MOVER_H
#define MOVER_H

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef MOVE_H
#include"move.h"
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

#ifndef DGROUP_H
#include"dgroup.h"
#endif

#ifndef TRIGGER_H
#include"trigger.h"
#endif

#ifndef DTEAM_H
#include"dteam.h"
#endif

#ifndef DCOMNDR_H
#include"comndr.h"
#endif

extern float metersPerWorldUnit;

//***************************************************************************

#define	DEBUG_CHUNKS

#define YIELD_WAITING		0
#define YIELDED				1
#define COLLIDER_YIELDED	2
#define COLLIDER_BEHIND		3
#define NO_ACTION			4
#define ON_THE_MOVE			5

#define	MAX_LOCK_RANGE		10

#define	MAX_WEAPONFIRE_CHUNKS			128
#define	MAX_CRITICALHIT_CHUNKS			128
#define	MAX_RADIO_CHUNKS				7

#define	CHUNK_SEND						0
#define	CHUNK_RECEIVE					1

#define	NUM_WEAPONFIRE_MODIFIERS		30

#define	MAX_ANTI_MISSILE_SYSTEMS		16			// Way more than we should need!

#define	ARMOR_TYPE_STANDARD				0
#define	ARMOR_TYPE_FERROFIBROUS			1

#define	CHASSIS_TYPE_STANDARD			0
#define	CHASSIS_TYPE_ENDOSTEEL			1

#define	ENGINE_TYPE_COMBUSTION			0
#define	ENGINE_TYPE_FUSION_STANDARD		1
#define	ENGINE_TYPE_FUSION_XL			2

#define	HEATSINK_TYPE_SINGLE			1
#define	HEATSINK_TYPE_DOUBLE			2

#define	IS_DAMAGE_NONE					0
#define	IS_DAMAGE_PARTIAL				1
#define	IS_DAMAGE_DESTROYED				2

#define	UNLIMITED_SHOTS					9999

#define NO_APPEARANCE_FOR_MECH			0xFFFA0001
#define	NO_RAM_FOR_DYNAMICS_TYPE		0xFFFA0002
#define	INVALID_DYNAMICS_TYPE			0xFFFA0003
#define	NO_RAM_FOR_CONTROL				0xFFFA0004
#define	INVALID_CONTROL_TYPE			0xFFFA0005
#define	NO_RAM_FOR_CONTROL_DATA			0xFFFA0006
#define	INVALID_CONTROL_DATA_TYPE		0xFFFA0007
#define	NO_RAM_FOR_DYNAMICS				0xFFFA0008
#define	NO_APPEARANCE_TYPE_FOR_MECH		0xFFFA0009
#define	APPEARANCE_NOT_SPRITE_TREE		0xFFFA000A
#define NO_RAM_FOR_FOOT_HS				0xFFFA000B
#define NO_RAM_FOR_WEAPON_HS			0xFFFA000C
#define FOOT_HS_WRONG_SIZE				0xFFFA000D
#define BAD_HS_PACKET					0xFFFA000E
#define NO_RAM_FOR_XLAT_HS				0xFFFA000F

//------------------------------------------------------------------------------
// Enums
typedef enum {
	WEAPONSORT_ATTACKCHANCE,
	NUM_WEAPONSORT_TYPES
} WeaponSortType;

typedef enum {
	SPEED_STATE_STATIONARY,
	SPEED_STATE_MOVING,
	SPEED_STATE_MOVING_FAST,
	NUM_MOVE_STATES
} SpeedState;

enum DeathCause
{
	DEBUGGER_DEATH = 0,
	ENGINE_DEATH = 1,
	PILOT_DEATH = 2,
	EJECTION_DEATH = 3,
	UNDETERMINED_DEATH = 4,
	POWER_USED_UP = 5
};

//---------------------------------------------------------------------------

class MoveChunk;
typedef MoveChunk* MoveChunkPtr;

class MoveChunk {

	public:

		long				stepPos[4][2];		// 0 = curPos, 1 thru 3 = next steps
		long				stepRelPos[3];		// delta for steps 1 thru 3
		long				numSteps;
		bool				run;
		bool				moving;
		unsigned long		data;
		static long			err;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void) {
			stepPos[0][0] = 0xFFFFFFFF;
			stepPos[0][1] = 0xFFFFFFFF;
			numSteps = 0;
			run = false;
			moving = false;
			data = 0;
		}

		void destroy (void) {
		}

		MoveChunk (void) {
			init();
		}

		~MoveChunk (void) {
			destroy();
		}

		void build (MoverPtr mover, MovePath* path1, MovePath* path2 = NULL);

		void build (MoverPtr mover, Stuff::Vector3D jumpGoal);

		void pack (MoverPtr mover);

		void unpack (MoverPtr mover);

		bool equalTo (MoverPtr mover, MoveChunkPtr chunk);
};

//---------------------------------------------------------------------------
#define MAX_YAW		64

typedef enum {
	DYNAMICS_BASE,
	DYNAMICS_MECH,
	DYNAMICS_GROUNDVEHICLE,
	DYNAMICS_ELEMENTAL
} DynamicsType;

typedef union {
	struct {
		int32_t	yawRate;			//Degrees per sec
	} elemental;
	struct {
		int32_t	yawRate;			//Degrees per sec
		int32_t	turretYawRate;		//Degrees per sec
		int32_t	pivotRate;			//Degrees per sec
		float	accel;				//Meters per sec per sec
		float	speed;				//Meters per sec
		int32_t	turretYaw;			//Degrees
	} groundVehicle;
	struct {
		int32_t	torsoYawRate;		//Degrees per sec
		int32_t	torsoYaw;			//Degrees
	} mech;
} DynamicsLimits;

class MoverDynamics {

	public:

		DynamicsType			type;
		DynamicsLimits			max;

	public:

		void operator = (MoverDynamics copy) {
			type = copy.type;
			max = copy.max;
			//cur = copy.cur;
		}
		
		virtual void init (void);

		MoverDynamics (void) {
			init();
		}
			
		virtual void destroy (void) {
		}

		~MoverDynamics (void) {
			destroy();
		}

		void init (DynamicsType newType);

		virtual void init (CSVFilePtr dynamicsFile);

		virtual void init (FitIniFilePtr dynamicsFile);
													
		void setType (DynamicsType newType) {
			type = newType;
		}

		DynamicsType getType (void) {
			return(type);
		}

		long brake (void);
};

//---------------------------------------------------------------------------

typedef enum {
	CONTROL_BASE,
	CONTROL_PLAYER,
	CONTROL_AI,
	CONTROL_NET
} ControlType;

typedef enum {
	CONTROL_DATA_BASE,
	CONTROL_DATA_MECH,
	CONTROL_DATA_GROUNDVEHICLE,
	CONTROL_DATA_ELEMENTAL
} ControlDataType;

typedef union {
	struct {
		float	rotate;
	} elemental;
	struct {
		char	throttle;			
		float	rotate;
		float	rotateTurret;
		char	gestureGoal;
		bool	pivot;
		bool	isWalking;
	} groundVehicle;
	struct {
		char	throttle;			
		float	rotate;				//aka mechYaw
		float	facingRotate;		//Direction mech is FACING, NOT MOVING!!!!!
		float	rotateTorso;
		float	rotateLeftArm;		//aka leftArmYaw
		float	rotateRightArm;		//aka rightArmYaw
		char	gestureGoal;
		bool	blowLeftArm;
		bool	blowRightArm;
		bool	pivot;
	} mech;
} ControlSettings;

class MoverControl {

	public:

		ControlType			type;
		ControlDataType		dataType;
		ControlSettings		settings;

	public:

		void operator = (MoverControl copy) {
			type = copy.type;
			dataType = copy.dataType;
			settings = copy.settings;
		}

		virtual void init (void) {
			type = CONTROL_BASE;
			dataType = CONTROL_DATA_BASE;
		}

		MoverControl (void) {
			init();
		}
			
		virtual void destroy (void) {
		}

		~MoverControl (void) {
			destroy();
		}

		void init (ControlType newType, ControlDataType newDataType) {
			setType(newType);
			setDataType(newDataType);
		}

		virtual long init (FitIniFilePtr controlFile);

		void setType (ControlType newType) {
			type = newType;
		}

		ControlType getType (void) {
			return(type);
		}

		void setDataType (ControlDataType newDataType) {
			dataType = newDataType;
			reset();
		}

		ControlDataType getDataType (void) {
			return(dataType);
		}

		void reset (void);

		void brake (void);

		void update (MoverPtr mover);
};

//---------------------------------------------------------------------------

#define	MAX_RANGE_RATINGS		51

typedef struct _InventoryItem* InventoryItemPtr;

typedef struct _RangeRating {
	float				accuracy;
	float				adjAccuracy;
} RangeRating;

typedef struct _RangeRating* RangeRatingPtr;

typedef struct _InventoryItem {
	//------------------
	// general item info
	unsigned char		masterID;		// master component ID
	unsigned char		health;			// number of points left before destroyed
	bool				disabled;		// TRUE if effectively destroyed

	//----------------
	// weapon specific
	unsigned char		facing;			// weapon facing in torso: 0 = forward, 1 = rear -- NO WEAPONS fire rear.  This is the weapon Node ID now!!!!
	short				startAmount;	// ammo's mission-start level
	short				amount;			// generally for ammo, and weapon's total ammo
	short				ammoIndex;		// used by ammo to reference ammo pools
	float				readyTime;		// next time weapon will be ready
	unsigned char		bodyLocation;	// where is the weapon located
	short				effectiveness;	// weapon max effectiveness
} InventoryItem;

//------------------------------------------------------------------------------------------

#define	MAX_AMMO_TYPES			10

typedef struct _AmmoTally {
	long				masterId;		// ammo type's Master Component Id
	long				curAmount;		// current amount of this ammo type in inventory
	long				maxAmount;		// starting amount of this ammo type in inventory
} AmmoTally;

typedef AmmoTally* AmmoTallyPtr;

//------------------------------------------------------------------------------------------

typedef struct _CriticalSpace* CriticalSpacePtr;

typedef struct _CriticalSpace {

	unsigned char		inventoryID;	// indexes into mech's inventory
	bool				hit;			// TRUE, if this space has been hit

	void operator = (struct _CriticalSpace copy) {
		inventoryID = copy.inventoryID;
		hit = copy.hit;
	}

} CriticalSpace;

//------------------------------------------------------------------------------------------

#define	MAX_CRITSPACES_PER_BODYLOCATION		12

class BodyLocation {

	public:

		bool				CASE;
		int32_t				totalSpaces;
		CriticalSpace		criticalSpaces[MAX_CRITSPACES_PER_BODYLOCATION];
		float				curInternalStructure;
		unsigned char		hotSpotNumber;
		unsigned char		maxInternalStructure;
		unsigned char		damageState;

	public:

		void operator = (BodyLocation copy) {
			CASE = copy.CASE;
			totalSpaces = copy.totalSpaces;
			for (int i = 0; i < MAX_CRITSPACES_PER_BODYLOCATION; i++)
				criticalSpaces[i] = copy.criticalSpaces[i];
			curInternalStructure = copy.curInternalStructure;
			hotSpotNumber = copy.hotSpotNumber;
			maxInternalStructure = copy.maxInternalStructure;
			damageState = copy.damageState;
		}
};

typedef BodyLocation* BodyLocationPtr;

//---------------------------------------------------------------------------

class ArmorLocation {

	public:

		float			curArmor;
		unsigned char	maxArmor;
};

typedef ArmorLocation* ArmorLocationPtr;

//---------------------------------------------------------------------------

#define	STATUSCHUNK_BODYSTATE_BITS		3
#define	STATUSCHUNK_TARGETTYPE_BITS		3
#define	STATUSCHUNK_MOVERINDEX_BITS		7
#define	STATUSCHUNK_TERRAINPARTID_BITS	20
#define	STATUSCHUNK_TERRAINBLOCK_BITS	8
#define	STATUSCHUNK_TERRAINVERTEX_BITS	9
#define	STATUSCHUNK_TERRAINITEM_BITS	3
#define	STATUSCHUNK_TRAIN_BITS			8
#define	STATUSCHUNK_TRAINCAR_BITS		8
#define	STATUSCHUNK_CELLPOS_BITS		10
#define	STATUSCHUNK_EJECTORDER_BITS		1
#define	STATUSCHUNK_JUMPORDER_BITS		1

#define	STATUSCHUNK_BODYSTATE_MASK		0x00000007
#define	STATUSCHUNK_TARGETTYPE_MASK		0x00000007
#define	STATUSCHUNK_MOVERINDEX_MASK		0x0000007F
#define	STATUSCHUNK_TERRAINPARTID_MASK	0x000FFFFF
#define	STATUSCHUNK_TERRAINBLOCK_MASK	0x000000FF
#define	STATUSCHUNK_TERRAINVERTEX_MASK	0x000001FF
#define	STATUSCHUNK_TERRAINITEM_MASK	0x00000007
#define	STATUSCHUNK_TRAIN_MASK			0x000000FF
#define	STATUSCHUNK_TRAINCAR_MASK		0x000000FF
#define	STATUSCHUNK_CELLPOS_MASK		0x000003FF
#define	STATUSCHUNK_EJECTORDER_MASK		0x00000001
#define	STATUSCHUNK_JUMPORDER_MASK		0x00000001

typedef enum {
	STATUSCHUNK_TARGET_NONE,
	STATUSCHUNK_TARGET_MOVER,
	STATUSCHUNK_TARGET_TERRAIN,
	STATUSCHUNK_TARGET_SPECIAL,
	STATUSCHUNK_TARGET_LOCATION
} StatusChunkTarget;

class StatusChunk;
typedef StatusChunk* StatusChunkPtr;

class StatusChunk {

	public:

		unsigned long		bodyState;
		char				targetType;
		long				targetId;
		long				targetBlockOrTrainNumber;
		long				targetVertexOrCarNumber;
		char				targetItemNumber;
		short				targetCellRC[2];
		bool				ejectOrderGiven;
		bool				jumpOrder;

		unsigned long		data;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void) {
			bodyState = 0;
			targetType = 0;
			targetId = 0;
			targetBlockOrTrainNumber = 0;
			targetVertexOrCarNumber = 0;
			targetItemNumber = 0;
			targetCellRC[0] = -1;
			targetCellRC[1] = -1;
			ejectOrderGiven = false;
			jumpOrder = false;

			data = 0;
		}

		void destroy (void) {
		}

		StatusChunk (void) {
			init ();
		}

		~StatusChunk (void) {
			destroy();
		}

		virtual void build (MoverPtr mover);

		virtual void pack (MoverPtr mover);

		virtual void unpack (MoverPtr mover);

		bool equalTo (StatusChunkPtr chunk);
};


//---------------------------------------------------------------------------

#define	MAXLEN_MOVER_NAME			65
#define	MAX_MOVER_BODY_LOCATIONS	8
#define	MAX_MOVER_ARMOR_LOCATIONS	11
#define	MAX_MOVER_INVENTORY_ITEMS	72
#define	MAXLEN_NET_PLAYER_NAME		256
#define	MAXLEN_MECH_LONGNAME	35

#define	MAX_ATTACK_CELLRANGE		30
#define	MAX_ATTACK_INCREMENTS		32
#define	RANGED_CELLS_DIM			(MAX_ATTACK_CELLRANGE * 2 + 1) * (MAX_ATTACK_CELLRANGE * 2 + 1)

typedef struct _MoverData : public GameObjectData
{
	bool				killed;
	bool				lost;
	Stuff::Vector3D		positionNormal;						
	Stuff::Vector3D		velocity;							
	char				name[MAXLEN_MOVER_NAME];			
	unsigned char		chassis;							
	bool				startDisabled;
	float				creationTime;

	int32_t				moveType;
	int32_t				moveLevel;
	bool				followRoads;

	int32_t				lastMapCell[2];

	float				damageRateTally;					
	float				damageRateCheckTime;				
	float				pilotCheckDamageTally;				

	BodyLocation		body[MAX_MOVER_BODY_LOCATIONS];		
	char				numBodyLocations;					
	int32_t				fieldedCV;

	int32_t				attackRange;						

	ArmorLocation		armor[MAX_MOVER_ARMOR_LOCATIONS];	
	char				numArmorLocations;
	char				longName[MAXLEN_MECH_LONGNAME];		

	InventoryItem		inventory[MAX_MOVER_INVENTORY_ITEMS];
	unsigned char		numOther;
	unsigned char		numWeapons;
	unsigned char		numAmmos;
	AmmoTally			ammoTypeTotal[MAX_AMMO_TYPES];	
	char				numAmmoTypes;					
	int32_t				pilotHandle;

	unsigned char		cockpit;										
	unsigned char		engine;											
	unsigned char		lifeSupport;									
	unsigned char		sensor;											
	unsigned char		ecm;											
	unsigned char		probe;											
	unsigned char		jumpJets;										
	unsigned char		nullSignature;									
	float				maxWeaponEffectiveness;							
	float				weaponEffectiveness;							

	float				minRange;										
	float				maxRange;										
	float				optimalRange;									
	int32_t				numFunctionalWeapons;							

	char				numAntiMissileSystems;							
	unsigned char		antiMissileSystem[MAX_ANTI_MISSILE_SYSTEMS];	

	float				engineBlowTime;
	float				maxMoveSpeed;
	bool				shutDownThisFrame;
	bool				startUpThisFrame;
	bool				disableThisFrame;

	char				teamId;
	char				groupId;
	int32_t				squadId;
	int32_t				selectionIndex;					
	int32_t				teamRosterIndex;				
	char				commanderId;
	int32_t				unitGroup;						
														
	int32_t				iconPictureIndex;				
	bool				suppressionFire;				

	int32_t				pilotCheckModifier;
	int32_t				prevPilotCheckModifier;
	int32_t				prevPilotCheckDelta;
	float				prevPilotCheckUpdate;
	bool				failedPilotingCheck;			
	float				lastWeaponEffectivenessCalc;	
	float				lastOptimalRangeCalc;			
	GameObjectWatchID	challengerWID;

	char				lastGesture;

	MoverControl		control;						
	MoverDynamics		dynamics;						

	int32_t				numWeaponHitsHandled;
	float				timeLeft;						
	bool				exploding;
	bool				withdrawing;

	float				yieldTimeLeft;					
	Stuff::Vector3D		lastValidPosition;				
	char				pivotDirection;					
	float				lastHustleTime;					

	bool				salvageVehicle;

	float				markDistanceMoved;				

	GameObjectWatchID	refitBuddyWID;
	GameObjectWatchID	recoverBuddyWID;

	int32_t				crashAvoidSelf;
	int32_t				crashAvoidPath;
	int32_t				crashBlockSelf;
	int32_t				crashBlockPath;
	float				crashYieldTime;
	int32_t				pathLockLength;
	int32_t				pathLockList[MAX_LOCK_RANGE][2];
	Stuff::Vector3D		moveCenter;
	float				moveRadius;

	int32_t				overlayWeightClass;

	float				timeToClearSelection;

	float				timeSinceMoving;

	float 				timeSinceFiredLast;				

	GameObjectWatchID	lastMovingTargetWID;

	bool 				mechSalvage;					

	Stuff::Vector3D		teleportPosition;				
	int32_t				debugPage;

	bool				pathLocks;						
	bool				isOnGui;						

	int32_t				conStat;						
	float				fadeTime;						
	BYTE				alphaValue;						
	int32_t				causeOfDeath;

	int32_t				lowestWeaponNodeID;

	DWORD				psRed;
	DWORD				psBlue;
	DWORD				psGreen;

} MoverData;


typedef enum {
	MOVETYPE_GROUND,
	MOVETYPE_AIR,
	NUM_MOVETYPES
} MoveType;

class LogisticsPilot;

class Mover : public GameObject {

	//------------
	//Data Members
	
	public:

		Stuff::Vector3D		positionNormal;						// normal to terrain at current position
		Stuff::Vector3D		velocity;							//How fast am I going?
		char				name[MAXLEN_MOVER_NAME];		// Name of this particular mover
		unsigned char		chassis;							// type of mover's chassis
		bool				startDisabled;
		float				creationTime;

		bool				killed;							// used to record when the kill score has been awarded
		bool				lost;							// used to record when the loss score has been awarded
		int32_t             moveType;
		int32_t             moveLevel;
		bool				followRoads;

		int32_t             lastMapCell[2];
		
		float				damageRateTally;					// damage points taken since last check
		float				damageRateCheckTime;				// time (in game time) of next damage check
		float				pilotCheckDamageTally;				// damage points taken since last pilot check

		BodyLocation		body[MAX_MOVER_BODY_LOCATIONS];		// body parts of this mech
		char				numBodyLocations;					// should be set based upon mover type
		int32_t             fieldedCV;

		int32_t             attackRange;						// attack range

		bool				salvaged;

		// Armor
		ArmorLocation		armor[MAX_MOVER_ARMOR_LOCATIONS];	// armor locations of this mover
		char				numArmorLocations;
		char				longName[MAXLEN_MECH_LONGNAME];		//Used by logistics (and the interface) to get long name.

		// Inventory
		InventoryItem		inventory[MAX_MOVER_INVENTORY_ITEMS];
		unsigned char		numOther;
		unsigned char		numWeapons;
		unsigned char		numAmmos;
		AmmoTally			ammoTypeTotal[MAX_AMMO_TYPES];	// tracks total ammo per ammo type
		char				numAmmoTypes;					// number of different ammo types
		MechWarriorPtr		pilot;
		int32_t             pilotHandle;
		SensorSystemPtr		sensorSystem;
		ContactInfoPtr		contactInfo;

		// Critical Component Indices
		unsigned char		cockpit;										// cockpit inventory index
		unsigned char		engine;											// engine inventory index
		unsigned char		lifeSupport;									// life support inventory index
		unsigned char		sensor;											// sensor inventory index
		unsigned char		ecm;											// ecm inventory index
		unsigned char		probe;											// probe inventory index
		unsigned char		jumpJets;										// jump jets inventory index
		unsigned char		nullSignature;									// null signature inventory index
		float				maxWeaponEffectiveness;							// max total damage possible
		float				weaponEffectiveness;							// basically, total damage possible
		
		float				minRange;										// current min attack range
		float				maxRange;										// current max attack range
		float				optimalRange;									// current optimum attack range
		int32_t             numFunctionalWeapons;							// takes into account damage, etc.

		char				numAntiMissileSystems;							// number of anti-missile systems
		unsigned char		antiMissileSystem[MAX_ANTI_MISSILE_SYSTEMS];	// anti-missile system list

		// Engine
		float				engineBlowTime;
		float				maxMoveSpeed;
		bool				shutDownThisFrame;
		bool				startUpThisFrame;
		bool				disableThisFrame;

		// Team
		//MoverGroupPtr		group;							// what group am I a member of?
		char				teamId;
		char				groupId;
		int32_t				squadId;
		int32_t				selectionIndex;					// > 0 when in selected group
		int32_t				teamRosterIndex;				// where am I in my team's roster?
		char				commanderId;
		int32_t				unitGroup;						// the thing the user sets by hitting ctrl and a number
															// this is a field since they can belong to more than one
		int32_t				iconPictureIndex;				// the little picture that shows arms and stuff falling off
		bool				suppressionFire;				// is this guy permanently shooting at ground
		char				prevTeamId;
		char				prevCommanderId;
		

		// Update Info
		int32_t				pilotCheckModifier;
		int32_t				prevPilotCheckModifier;
		int32_t				prevPilotCheckDelta;
		float				prevPilotCheckUpdate;
		bool				failedPilotingCheck;			// Passed or failed this frame...
//		BaseObjectPtr		collisionFreeFrom;
//		float				collisionFreeTime;
		float				lastWeaponEffectivenessCalc;	// time of last calc
		float				lastOptimalRangeCalc;			// time of last calc
		GameObjectWatchID	challengerWID;

		char				lastGesture;

//		AppearancePtr		appearance;						// pointer to the Actor which is the appearance.
		MoverControl		control;						// control settings for this mover
		MoverDynamics		dynamics;						// dynamics settings for this mover
		
		// Network
		//DWORD				netPlayerId;
		char				netPlayerName[MAXLEN_NET_PLAYER_NAME];	// netPlayerName is the player who owns this mover
		long				localMoverId;					// if >= 0, is locally controlled
		long				netRosterIndex;					// used for mover id in net packets
		StatusChunk			statusChunk;					// last status chunk built/received
		bool				newMoveChunk;					// set if last movechunk not yet processed
		MoveChunk			moveChunk;						// last move chunk built/received
		long				numWeaponFireChunks[2];
		unsigned long		weaponFireChunks[2][MAX_WEAPONFIRE_CHUNKS];
		long				numCriticalHitChunks[2];
		unsigned char		criticalHitChunks[2][MAX_CRITICALHIT_CHUNKS];
		long				numRadioChunks[2];
		unsigned char		radioChunks[2][MAX_RADIO_CHUNKS];
		bool				ejectOrderGiven;
															// Still awaiting final destruct orders from update.
		int32_t				numWeaponHitsHandled;
		float				timeLeft;						// How long before we return FALSE to update.
		bool				exploding;
		bool				withdrawing;
	
		float				yieldTimeLeft;					// How much time do I have left to wait
		Stuff::Vector3D		lastValidPosition;				// Last valid move path point I've been to
		char				pivotDirection;					// Used in pivotTo(): -1 = not pivoting
		float				lastHustleTime;					// last time we had to hustle (on bridge, etc.)

		static long			numMovers;
		static SortListPtr	sortList;

		bool				salvageVehicle;
		
		float				markDistanceMoved;				//Used to track distance object has moved since last mark of terrain visible.

		GameObjectWatchID	refitBuddyWID;
		GameObjectWatchID	recoverBuddyWID;

		int32_t				crashAvoidSelf;
		int32_t				crashAvoidPath;
		int32_t				crashBlockSelf;
		int32_t				crashBlockPath;
		float				crashYieldTime;
		int32_t				pathLockLength;
		int32_t             pathLockList[MAX_LOCK_RANGE][2];
		Stuff::Vector3D		moveCenter;
		float				moveRadius;

		int32_t				overlayWeightClass;

		float				timeToClearSelection;

		float				timeSinceMoving;

		float 				timeSinceFiredLast;				//used to track when they stop revealing

		//GameObjectPtr		lastMovingTarget;
		GameObjectWatchID	lastMovingTargetWID;

		bool 				mechSalvage;					//As this guy dies, ONLY check once if he's salvagable!

		Stuff::Vector3D		teleportPosition;				//debug feature :)
		int32_t				debugPage;

		static float		newThreatMultiplier;
		static float		marginOfError[2];
		static float		refitRange;
		static float		refitTime;
		static float		refitCostArray[NUM_COSTS][NUM_FIXERS];
		static float		refitAmount;
		static float		recoverRange;
		static float		recoverTime;
		static float		recoverCost;
		static float		recoverAmount;
		static bool			inRecoverUpdate;
		static char			optimalCells[MAX_ATTACK_CELLRANGE][MAX_ATTACK_INCREMENTS][2];
		static long			numOptimalIncrements;
		static short		rangedCellsIndices[MAX_ATTACK_CELLRANGE][2];
		static char			rangedCells[RANGED_CELLS_DIM][2];
		static long			IndirectFireWeapons[20];
		static long			AreaEffectWeapons[20];
		static unsigned long holdFireIconHandle;
		
		static TriggerAreaManager* triggerAreaMgr;


		bool				pathLocks;						//For movers which can be stepped on.  They do NOT lock!
		bool				isOnGui;						//For movers which start out on player team but not on gui.  Like raven in 0103

		int32_t             conStat;						//Contact status stored for this frame, for this machine
		float				fadeTime;						//Time between fade from LOS to non-LOS
		BYTE				alphaValue;						//Current Fade value;
		int32_t             causeOfDeath;

		int32_t				lowestWeaponNodeID;
		float				lowestWeaponNodeZ;

	//----------------
	//Member Functions

	public:

		virtual void set (Mover copy);

		virtual void init (bool create);

		Mover (void) {
			init(true);
		}
			
		virtual void destroy (void);

		virtual void init (bool create, ObjectTypePtr objType) {
			GameObject::init(create, objType);
		}

		virtual long init (FitIniFile* objProfile) {
			return(NO_ERR);
		}

		virtual long init (DWORD variantNum) {
			return(NO_ERR);
		}

		virtual void release (void);

		//virtual long update (void);

		//virtual void render (void);

		virtual void updateDebugWindow (GameDebugWindow* debugWindow);

		virtual char* getName (void) {
			return(name);
		}

		virtual void setName (char* s) {
			strncpy(name, s, MAXLEN_MOVER_NAME);
		}

		virtual void setControl (ControlType ctrlType) {
		}

		virtual void updateAIControl (void) {
		}

		virtual void updateNetworkControl (void) {
		}

		virtual void updatePlayerControl (void) {
		}

		virtual void updateDynamics (void) {
		}

		virtual void setPartId (long newPartId);

		virtual void setPosition (Stuff::Vector3D& newPosition);
		
		virtual void setTeleportPosition (Stuff::Vector3D& newPos);

		ContactInfoPtr getContactInfo (void) {
			return(contactInfo);
		}

		virtual void tradeRefresh (void);

		virtual void setMoveType (long type);

		virtual long getMoveType (void) {
			return(moveType);
		}

		virtual void setMoveLevel (long level) {
			moveLevel = level;
		}

		virtual long getMoveLevel (void) {
			return(moveLevel);
		}

		virtual bool getFollowRoads (void) {
			return(followRoads);
		}

		virtual void setFollowRoads (bool setting) {
			followRoads = setting;
		}

		virtual Stuff::Vector3D getVelocity (void) {
			return(velocity);
		}

		virtual void setVelocity (Stuff::Vector3D& newVelocity) {
			velocity = newVelocity;
		}

		virtual float getSpeed (void) {
			return(velocity.GetLength());
		}

		virtual bool hasWeaponNode (void);

		Stuff::Vector3D getLOSPosition (void);

		virtual void rotate (float angle);

		virtual void rotate (float angle, float facingAngle);
		
		virtual void setAwake (bool state);

		virtual bool isMarine(void) 
		{
			return(pathLocks == false);
		}

		void drawWaypointPath();

		void updateDrawWaypointPath();
		
//		virtual frame_of_ref getFrame (void)
//		{
//			return(frame);
//		}

		virtual long getSpeedState (void) {
			return(SPEED_STATE_STATIONARY);
		}

		virtual float getTerrainAngle (void);

		virtual float getVelocityTilt (void);

//		virtual void setFrame (frame_of_ref &newFrame)
//		{
//			frame = newFrame;
//		}
		
		virtual Stuff::Vector3D relativePosition (float angle, float radius, unsigned long flags);

		long calcLineOfSightView (long range);
		
		virtual void setSensorRange (float range);
		
		virtual Stuff::Vector3D getRotationVector (void) 
		{
			Stuff::Vector3D rotationVec;
			rotationVec.x = 0.0f;
			rotationVec.y = -1.0f;
			rotationVec.z = 0.0f;
			Rotate(rotationVec, -rotation);
			return(rotationVec);
		}

		virtual bool hasActiveProbe (void);

		virtual float getEcmRange (void);

		virtual bool hasNullSignature (void);

		virtual void setSelected (bool set, long newSelectionIndex = 0) {
			GameObject::setSelected(set);
			selectionIndex = newSelectionIndex;
		}

		virtual long handleTacticalOrder (TacticalOrder tacOrder, long priority = 1, bool queuePlayerOrder = false);

		virtual AppearancePtr getAppearance (void) {
			return(appearance);
		}

//		virtual float getAppearRadius (void)
//		{
//			return appearance->getRadius();
//		}

		virtual void reduceAntiMissileAmmo (long numAntiMissiles);

		virtual void pilotingCheck (unsigned long situation = 0, float modifier = 0.0);

		virtual void forcePilotingCheck (void) {
			if (pilotCheckModifier < 0)
				pilotCheckModifier = 0;
		}

		virtual bool canFireWeapons (void) {
			return(true);
		}

		virtual void updateDamageTakenRate (void);

		virtual long checkShortRangeCollision (void) {
			return(NO_ACTION);
		}

		virtual void setOverlayWeightClass (long overlayClass) {
			overlayWeightClass = overlayClass;
		}

		virtual long getOverlayWeightClass (void) {
			return(overlayWeightClass);
		}

		virtual void getStopInfo (float &stopTime, float &stopDistance) {}

		virtual bool getAdjacentCellPathLocked (long level, long cellRow, long cellCol, long dir);

		virtual void updatePathLock (bool set);

		virtual bool getPathRangeLock (long range, bool* reachedEnd = NULL);

		virtual long setPathRangeLock (bool set, long range = 0);

		virtual bool getPathRangeBlocked (long range, bool* reachedEnd = NULL);

		virtual bool crashAvoidanceSystem (void) {
			return(false);
		}

		void setLastHustleTime (float t) {
			lastHustleTime = t;
		}

		float getLastHustleTime (void) {
			return(lastHustleTime);
		}

		virtual void updateHustleTime (void);

		virtual void mineCheck (void) {
			//----------------------------
			// Does nothing, be default...
		}

		virtual void updateMovement (void) {}

		virtual long bounceToAdjCell (void);

#ifdef USE_MOVERCONTROLS
		unsigned long getControlClass (void) {
			return(control->getControlClass());
		}
#endif

		virtual void setSquadId (char newSquadId) {
			squadId = newSquadId;
		}

		virtual char getSquadId (void) {
			return(squadId);
		}

		virtual long setTeamId (long _teamId, bool setup);
		
		virtual long getTeamId (void) {
			return(teamId);
		}

		virtual TeamPtr getTeam (void);

		virtual bool isFriendly (TeamPtr team);

		virtual bool isEnemy (TeamPtr team);

		virtual bool isNeutral (TeamPtr team);

		virtual long setGroupId (long _groupId, bool setup);

		virtual long getGroupId (void) {
			return(groupId);
		}

		virtual MoverGroupPtr getGroup (void);

		virtual void setPilotHandle (long _pilotHandle);

		virtual void loadPilot (const char* pilotFileName, const char* brainFileName, LogisticsPilot *lPilot);

		virtual void setCommanderId (long _commanderId);

		virtual long getCommanderId (void) {
			return(commanderId);
		}

		virtual CommanderPtr getCommander (void);
		
		virtual MechWarriorPtr getPilot (void) {
			return(pilot);
		}

		MoverPtr getPoint (void);

		void setTeamRosterIndex (long index) {
			teamRosterIndex = index;
		}

		long getTeamRosterIndex (void) {
			return(teamRosterIndex);
		}

		virtual long getContacts (int* contactList, int contactCriteria, int sortType);

		long getContactStatus (long scanningTeamID, bool includingAllies);

//		void setNetPlayerId (DWORD playerId) {
//			netPlayerId = playerId;
//		}

//		DWORD getNetPlayerId (void) {
//			return(netPlayerId);
//		}

		void setNetPlayerName(const char *name) {
            strncpy(netPlayerName,name,255);
		}

		const char* getNetPlayerName(void) {
			return(netPlayerName);
		}

		void setLocalMoverId (long id) {
			localMoverId = id;
		}

		long getLocalMoverId (void) {
			return(localMoverId);
		}

		virtual long getCBills (void) {
			return(0);
		}

		void setNetRosterIndex (long index) {
			netRosterIndex = index;
		}

		long getNetRosterIndex (void) {
			return(netRosterIndex);
		}

		long getNumWeaponFireChunks (long which) {
			return(numWeaponFireChunks[which]);
		}

		long clearWeaponFireChunks (long which);

		long addWeaponFireChunk (long which, WeaponFireChunkPtr chunk);

		long addWeaponFireChunks (long which, unsigned long* packedChunkBuffer, long numChunks);

		long grabWeaponFireChunks (long which, unsigned long* packedChunkBuffer, long maxChunks);

		virtual long updateWeaponFireChunks (long which);

		long getNumCriticalHitChunks (long which) {
			return(numCriticalHitChunks[which]);
		}

		long clearCriticalHitChunks (long which);

		long addCriticalHitChunk (long which, long bodyLocation, long criticalSpace);

		long addCriticalHitChunks (long which, unsigned char* packedChunkBuffer, long numChunks);

		long grabCriticalHitChunks (long which, unsigned char* packedChunkBuffer);

		virtual long updateCriticalHitChunks (long which);

		long getNumRadioChunks (long which) {
			return(numRadioChunks[which]);
		}

		long clearRadioChunks (long which);

		long addRadioChunk (long which, unsigned char msg);

		long addRadioChunks (long which, unsigned char* packedChunkBuffer, long numChunks);

		long grabRadioChunks (long which, unsigned char* packedChunkBuffer);

		virtual long updateRadioChunks (long which);

		virtual StatusChunkPtr getStatusChunk (void) {
			return(&statusChunk);
		}

		virtual long buildStatusChunk (void) {
			return(NO_ERR);
		}

		virtual long handleStatusChunk (long updateAge, unsigned long chunk) {
			return(NO_ERR);
		}

		virtual MoveChunkPtr getMoveChunk (void) {
			return(&moveChunk);
		}

		virtual long buildMoveChunk (void) {
			return(NO_ERR);
		}

		virtual long handleMoveChunk (unsigned long chunk) {
			return(NO_ERR);
		}

		void setMoveChunk (MovePathPtr path, MoveChunkPtr chunk);

		void playMessage (RadioMessageType messageId, bool propogateIfMultiplayer = false);

		virtual long calcCV (bool calcMax = false) {
			return(0);
		}

		void setFieldedCV (long CV) {
			fieldedCV = CV;
		}

		unsigned long getFieldedCV (void) {
			return(fieldedCV);
		}

		virtual void setThreatRating (short rating);

		virtual long getThreatRating (void);

//		virtual void getDamageClass (long& damageClass, bool& shutDown);

		virtual bool refit (float pointsAvailable, float& pointsUsed, bool ammoOnly = false);

		float calcRecoverPrice (void);

		virtual bool recover (void);

		long getInventoryDamage (long itemIndex);

		long getInventoryMax (long itemIndex) {
			return(MasterComponent::masterList[inventory[itemIndex].masterID].getHealth());
		}

		virtual long getBodyState (void) {
			return(-1);
		}

		long getSensorMax (void) {
            gosASSERT(sensor < MAX_MOVER_INVENTORY_ITEMS);
			if(sensor < MAX_MOVER_INVENTORY_ITEMS)
		    	return(MasterComponent::masterList[inventory[sensor].masterID].getHealth());
			else
				return 0;
		}

		long getSensorHealth (void) {
            gosASSERT(sensor < MAX_MOVER_INVENTORY_ITEMS);
			if(sensor < MAX_MOVER_INVENTORY_ITEMS)
            	return(inventory[sensor].health);
			else
				return 0;
		}

		float getVisualRange (void);

		float getLastWeaponEffectivenessCalc (void) {
			return(lastWeaponEffectivenessCalc);
		}

		float getLastOptimalRangeCalc (void) {
			return(lastOptimalRangeCalc);
		}

		virtual float getTotalEffectiveness(void)
		{
			return (0.0);
		}
		
		//ContactWatchPtr addContactWatch (GameObjectPtr who, ContactRecPtr contact) {
		//	return(contactWatches.add(who, contact));
		//}

		//void removeContactWatch (ContactWatchPtr watch) {
		//	contactWatches.remove(watch);
		//}

		void setChallenger (GameObjectPtr challenger);
		
		GameObjectPtr getChallenger (void);

/*		void addContact (ContactRecPtr contact) {
			sensorSystem.addContact(contact);
		}
*/
		long scanContact (long contactType, unsigned long contactHandle);

		long analyzeContact (long contactType, unsigned long contactHandle);

		long scanBattlefield (long quadrant, long contactType, long potentialContactType);

		virtual Stuff::Vector3D calcOffsetMoveGoal (Stuff::Vector3D target);

		virtual long calcGlobalPath (GlobalPathStep* globalPath, GameObjectPtr obj, Stuff::Vector3D* location, bool useClosedAreas);

		virtual long calcMoveGoal (GameObjectPtr target,
						  Stuff::Vector3D moveCenter,
						  float moveRadius,
						  Stuff::Vector3D moveGoal,
						  long selectionIndex,
						  Stuff::Vector3D& newGoal,
						  long numValidAreas,
						  short* validAreas,
						  unsigned long moveParams);
								   

		virtual long calcMovePath (MovePathPtr path,
								   long pathType,
								   Stuff::Vector3D start,
								   Stuff::Vector3D goal,
								   long* goalCell,
								   unsigned long moveParams = MOVEPARAM_NONE);

		virtual long calcEscapePath (MovePathPtr path,
									 Stuff::Vector3D start,
									 Stuff::Vector3D goal,
									 long* goalCell,
									 unsigned long moveParams,
									 Stuff::Vector3D& escapeGoal);

		virtual long calcMovePath (MovePathPtr path,
								   Stuff::Vector3D start,
								   long thruArea[2],
								   long goalDoor,
								   Stuff::Vector3D finalGoal,
								   Stuff::Vector3D* goalWorldPos,
								   long* goalCell,
								   unsigned long moveParams = MOVEPARAM_NONE);

		virtual float weaponLocked (long weaponIndex, Stuff::Vector3D targetPosition);

		virtual bool weaponInRange (long weaponIndex, float metersToTarget, float buffer);

		virtual long getWeaponsReady (int* list, int listSize);

		virtual long getWeaponsLocked (int* list, int listSize);

		virtual long getWeaponsInRange (int * list, int listSize, float orderFireRange);

		virtual long getWeaponShots (long weaponIndex);
		
		virtual bool getWeaponIndirectFire (long weaponIndex);

		virtual bool getWeaponAreaEffect (long weaponIndex);
		
		virtual float getWeaponAmmoLevel (long weaponIndex);

		bool	getWeaponIsEnergy( long weaponIndx );

		virtual void calcWeaponEffectiveness (bool setMax);

		virtual void calcAmmoTotals (void);

		virtual long calcFireRanges (void);

		virtual float getOrderedFireRange (long* attackRange = NULL);

		virtual float getMinFireRange (void);

		virtual float getMaxFireRange (void);

		virtual float getOptimalFireRange (void);

		virtual bool isWeaponIndex (long itemIndex);

		virtual bool isWeaponReady (long weaponIndex);

		virtual bool isWeaponMissile (long weaponIndex);

		virtual void startWeaponRecycle (long weaponIndex);

		virtual long tallyAmmo (long ammoMasterId);

		virtual long reduceAmmo (long ammoMasterId, long amount);

		virtual long getNumAmmoTypes (void) {
			return(numAmmoTypes);
		}

		bool hasNonAreaWeapon (void);
	
		virtual long getAmmoType (long ammoTypeIndex) {
			return(ammoTypeTotal[ammoTypeIndex].masterId);
		}

		virtual long getAmmoTypeTotal (long ammoTypeIndex) {
			//--------------------------------------------------
			// This assumes 0 <= ammoTypeIndex < numAmmoTypes...
			return(ammoTypeTotal[ammoTypeIndex].curAmount);
		}

		virtual long getAmmoTypeStart (long ammoTypeIndex) {
			//--------------------------------------------------
			// This assumes 0 <= ammoTypeIndex < numAmmoTypes...
			return(ammoTypeTotal[ammoTypeIndex].maxAmount);
		}

		virtual void deductWeaponShot (long weaponIndex, long ammoAmount = 1);

		virtual bool needsRefit(void);
		
		virtual long sortWeapons (int* weaponList, long* valueList, long listSize, long sortType, bool skillCheck);

		virtual float calcAttackChance (GameObjectPtr target, long aimLocation, float targetTime, long weaponIndex, float modifiers, long* range, Stuff::Vector3D* targetPoint = NULL);

		virtual float calcAttackModifier (GameObjectPtr target, long weaponIndex, bool skillCheck) {
			return(0.0);
		}

		MasterComponentPtr getMasterComponent (long itemIndex) {
			return(&MasterComponent::masterList[inventory[itemIndex].masterID]);
		}

		virtual bool hitInventoryItem (long itemIndex, bool setupOnly = FALSE) {
			return(false);
		}

		bool isInventoryDisabled (long itemIndex) {
			return(inventory[itemIndex].disabled);
		}

		virtual void disable (unsigned long cause);

		virtual void shutDown (void);
		
		virtual void startUp (void);

		virtual void destroyBodyLocation (long location) {}

		virtual void calcCriticalHit (long hitLocation) {}

		virtual bool injureBodyLocation (long bodyLocation, float damage) {
			return(false);
		}

		virtual void ammoExplosion (long ammoIndex);

		virtual long fireWeapon (GameObjectPtr target, float targetTime, long weaponIndex, long attackType, long aimLocation, Stuff::Vector3D* targetPoint, float &damageDone) {
			return(NO_ERR);
		}

		virtual long handleWeaponFire (long weaponIndex,
									   GameObjectPtr target,
									   Stuff::Vector3D* targetPoint,
									   bool hit,
									   float entryAngle,
									   long numMissiles,
									   long hitLocation) {
			return(NO_ERR);
		}

		virtual void printFireWeaponDebugInfo (GameObjectPtr target, Stuff::Vector3D* targetPoint, long chance, long aimLocation, long roll, WeaponShotInfo* shotInfo);

		virtual void printHandleWeaponHitDebugInfo (WeaponShotInfo* shotInfo);

		virtual float relFacingDelta (Stuff::Vector3D goalPos, Stuff::Vector3D targetPos);

		virtual float relFacingTo (Stuff::Vector3D goal, long bodyLocation = -1);

		virtual float relViewFacingTo (Stuff::Vector3D goal) {
			return(GameObject::relFacingTo(goal));
		}

		bool canMoveHere (Stuff::Vector3D worldPos);

		void setLastValidPosition (Stuff::Vector3D pt) {
			lastValidPosition = pt;
		}

		Stuff::Vector3D getLastValidPosition (void) {
			return(lastValidPosition);
		}

		virtual bool canPowerUp (void) {
			return(true);
		}
		
		virtual bool canMove (void) {
			return(true);
		}
		
		virtual bool canJump (void) {
			return(false);
		}

		virtual float getJumpRange (long* numOffsets = NULL, long* jumpCost = NULL) {
			if (numOffsets)
				*numOffsets = 8;
			if (jumpCost)
				*jumpCost = 0;
			return(0.0);
		}

		virtual bool isJumping (Stuff::Vector3D* jumpGoal = NULL) {
			return(false);
		}

		virtual bool isMineSweeper (void) {
			return(false);
		}

		virtual bool isMineLayer (void) {
			return(false);
		}

		virtual bool isLayingMines (void) {
			return(false);
		}

		virtual float getFireArc (void);

		virtual float calcMaxSpeed (void) {
			return(0.0);
		}

		virtual float calcSlowSpeed (void) {
			return(0.0);
		}

		virtual float calcModerateSpeed (void) {
			return(0.0);
		}

		virtual long calcSpriteSpeed (float speed, unsigned long flags, long& state, long& throttle) {
			state = 0;
			throttle = 100;
			return(-1);
		}

		virtual Stuff::Vector3D getPositionFromHS (long weaponType) 
		{
			Stuff::Vector3D pos;
			pos.Zero();
			return(pos);
		}

		virtual float getGestureStopDistance (void) {
			return 0.0;
		}
		
		virtual bool handleEjection (void);

		virtual bool isWithdrawing (void);
		
		virtual bool underPlayerControl (void) {
			return (true);
		}
		
		~Mover (void) {
			destroy();
		}

		bool enemyRevealed (void);

		virtual const char* getIfaceName(void) {
            STOP((""));
            return ("No Name");
        }

		void drawSensorTextHelp (float screenX, float screenY, long resID, DWORD color, bool drawBOLD);

		virtual SensorSystem* getSensorSystem(){ return sensorSystem; }

		
		static long loadGameSystem (FitIniFilePtr mechFile, float visualRange);

		bool isCloseToFirstTacOrder( Stuff::Vector3D& pos );

		void removeFromUnitGroup( int id );
		void addToUnitGroup( int id );
		bool isInUnitGroup( int id );
		virtual bool isRefit(){ return 0; }
		virtual bool isRecover() { return 0; }

		int getIconPictureIndex(){ return iconPictureIndex; }

		bool isWeaponWorking(long index );

		virtual void startShutDown (void)
		{
		
		}
		
		virtual bool isMech (void)
		{
			return false;
		}
		
		virtual bool isVehicle (void)
		{
			return false;
		}
		
		virtual bool isGuardTower (void)
		{
			return false;
		}

		virtual bool isOnGUI (void)
		{
			return isOnGui;
		}
		
		virtual void setOnGUI (bool onGui)
		{
			isOnGui = onGui;
		}
		
		virtual void Save (PacketFilePtr file, long packetNum);

		void CopyTo (MoverData *data);

		void Load (MoverData *data);

		static void initOptimalCells (long numIncrements);
};

//---------------------------------------------------------------------------

typedef struct _MoverInitData {
	char			pilotFileName[50];
	bool			overrideLoadedPilot;
	float			gunnerySkill;
	float			pilotingSkill;
	bool			specialtySkills[NUM_SPECIALTY_SKILLS];
	char			brainFileName[50];
	char			csvFileName[50];
	int32_t			objNumber;
	int32_t 		rosterIndex;
	DWORD           controlType;
	DWORD           controlDataType;
	DWORD           variant;
	Stuff::Vector3D	position;
	int32_t 		rotation;
	char			teamID;
	char			commanderID;
	DWORD			baseColor;
	DWORD			highlightColor1;
	DWORD			highlightColor2;
	char			gestureID;
	bool			active;
	bool			exists;
	char			icon;
	bool			capturable;
	long            numComponents;
	long            components[50];
} MoverInitData;

//---------------------------------------------------------------------------

extern MoverPtr getMoverFromHandle (long partId);

//******************************************************************************************

#endif
