//******************************************************************************************
//
//	tacordr.h - This file contains the Tactical Orders header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TACORDR_H
#define TACORDR_H

//--------------
// Include Files

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef DGROUP_H
#include"dgroup.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif

#ifndef DTACORDR_H
#include"dtacordr.h"
#endif

//***************************************************************************

#define	NUM_MOVE_SPEEDS		4

#define	TACORDER_FAILURE	0
#define	TACORDER_SUCCESS	1

typedef enum {
	MOVE_SPEED_BEST,
	MOVE_SPEED_MAXIMUM,
	MOVE_SPEED_SLOW,
	MOVE_SPEED_MODERATE
} MoveSpeedType;

#define	NUM_MOVE_PATTERNS	7

typedef enum {
	MOVE_PATTERN_DIRECT,
	MOVE_PATTERN_ZIGZAG_NARROW,
	MOVE_PATTERN_ZIGZAG_MEDIUM,
	MOVE_PATTERN_ZIGZAG_WIDE,
	MOVE_PATTERN_RANDOM_NARROW,
	MOVE_PATTERN_RANDOM_MEDIUM,
	MOVE_PATTERN_RANDOM_WIDE
} MovePatternType;

#define	NUM_ATTITUDES		6

typedef enum {
	ATTITUDE_CAUTIOUS,
	ATTITUDE_CONSERVATIVE,
	ATTITUDE_NORMAL,
	ATTITUDE_AGGRESSIVE,
	ATTITUDE_BERSERKER,
	ATTITUDE_SUICIDAL
} AttitudeType;

//#define	FIRERANGE_OPTIMUM	-1

typedef enum {
	FIRERANGE_DEFAULT = -4,
	FIRERANGE_RAMMING,
	FIRERANGE_LONGEST,
	FIRERANGE_OPTIMAL,
	FIRERANGE_SHORT,
	FIRERANGE_MEDIUM,
	FIRERANGE_LONG,
	FIRERANGE_CURRENT,
	NUM_FIRERANGES
} FireRangeType;

typedef enum {
	ATTACK_NONE,
	ATTACK_TO_DESTROY,
	ATTACK_TO_DISABLE,
	ATTACK_CONSERVING_AMMO,
	NUM_ATTACK_TYPES
} AttackType;

typedef enum {
	ATTACKMETHOD_RANGED,
	ATTACKMETHOD_DFA,
	ATTACKMETHOD_RAMMING,
	NUM_ATTACKMETHODS
} AttackMethod;

typedef enum {
	FIREODDS_LUCKY,
	FIREODDS_BAD,
	FIREODDS_MEDIUM,
	FIREODDS_GOOD,
	FIREODDS_GREAT,
	NUM_FIREODDS
} FireOddsType;

typedef enum {
	TACTIC_NONE,
	TACTIC_FLANK_LEFT,
	TACTIC_FLANK_RIGHT,
	TACTIC_FLANK_REAR,
	TACTIC_STOP_AND_FIRE,
	TACTIC_TURRET,
	TACTIC_JOUST,
	NUM_TACTICS
} TacticType;

typedef enum {
	ORDER_ORIGIN_PLAYER,
	ORDER_ORIGIN_COMMANDER,
	ORDER_ORIGIN_SELF
} OrderOriginType;

typedef enum {
	TACTICAL_ORDER_NONE,
	TACTICAL_ORDER_WAIT,
	TACTICAL_ORDER_MOVETO_POINT,
	TACTICAL_ORDER_MOVETO_OBJECT,
	TACTICAL_ORDER_JUMPTO_POINT,
	TACTICAL_ORDER_JUMPTO_OBJECT,
	TACTICAL_ORDER_TRAVERSE_PATH,
	TACTICAL_ORDER_PATROL_PATH,
	TACTICAL_ORDER_ESCORT,
	TACTICAL_ORDER_FOLLOW,
	TACTICAL_ORDER_GUARD,
	TACTICAL_ORDER_STOP,
	TACTICAL_ORDER_POWERUP,
	TACTICAL_ORDER_POWERDOWN,
	TACTICAL_ORDER_WAYPOINTS_DONE,
	TACTICAL_ORDER_EJECT,
	TACTICAL_ORDER_ATTACK_OBJECT,
	TACTICAL_ORDER_ATTACK_POINT,
	TACTICAL_ORDER_HOLD_FIRE,
	TACTICAL_ORDER_WITHDRAW,
	TACTICAL_ORDER_SCRAMBLE,
	TACTICAL_ORDER_CAPTURE,
	TACTICAL_ORDER_REFIT,
	TACTICAL_ORDER_GETFIXED,
	TACTICAL_ORDER_LOAD_INTO_CARRIER,
	TACTICAL_ORDER_DEPLOY_ELEMENTALS,
	TACTICAL_ORDER_RECOVER,
	NUM_TACTICAL_ORDERS	// IF THIS CHANGES, ADD GUARD_POINT AS SEP. ORDER (see pack/unpack hack)
} TacticalOrderCode;

//***************************************************************************

struct LocationNode {
	Stuff::Vector3D	location;
	bool			run;
	LocationNodePtr	next;
};

typedef enum _TravelModeType {
	TRAVEL_MODE_INVALID = -1,
	TRAVEL_MODE_SLOW,
	TRAVEL_MODE_FAST,
	TRAVEL_MODE_JUMP,
	NUM_TRAVEL_MODES
} TravelModeType;

typedef struct _WayPath {
	long			numPoints;
	long			curPoint;
	float			points[3 * MAX_WAYPTS];
	unsigned char	mode[MAX_WAYPTS];
} WayPath;

typedef WayPath* WayPathPtr;

typedef enum {
	MOVE_MODE_NORMAL,
	MOVE_MODE_MINELAYING
} SpecialMoveMode;

typedef struct _TacOrderMoveParams {
	WayPath			wayPath;
	bool			faceObject;
	bool			wait;
	SpecialMoveMode	mode;
	bool			escapeTile;
	bool			jump;
	long			fromArea;
	bool			keepMoving;
} TacOrderMoveParams;

typedef struct _TacOrderAttackParams {
	AttackType		type; //NO LONGER USED! 1/7/00
	AttackMethod	method;
	FireRangeType	range;
	TacticType		tactic;
	long			aimLocation;
	bool			pursue;
	bool			obliterate;
	Stuff::Vector3D	targetPoint;
} TacOrderAttackParams;

class TacticalOrder {

	public:

		long					id;
		float					time;
		float					delayedTime;
		float					lastTime;
		bool					unitOrder;		// TRUE if unit order, else individual order
		bool					subOrder;
		OrderOriginType			origin;
		TacticalOrderCode		code;
		TacOrderMoveParams		moveParams;
		TacOrderAttackParams	attackParams;
		GameObjectWatchID		targetWID;
		long					targetObjectClass;
		long					selectionIndex;
		char					stage;
		char					statusCode;
		Stuff::Vector3D			lastMoveGoal;

		// for network use only
		char					pointLocalMoverId;
		unsigned long			groupFlags;
		unsigned long			data[2];

	public:

		void* operator new (size_t ourSize);
		void operator delete (void* us);

		void operator = (TacticalOrder copy) {
			time = copy.time;
			delayedTime = copy.delayedTime;
			lastTime = copy.lastTime;
			unitOrder = copy.unitOrder;
			origin = copy.origin;
			code = copy.code;
			moveParams = copy.moveParams;
			attackParams = copy.attackParams;
			targetWID = copy.targetWID;
			targetObjectClass = copy.targetObjectClass;
			selectionIndex = copy.selectionIndex;
			stage = copy.stage;
			subOrder = copy.subOrder;
			lastMoveGoal.x = -99999.0;
			statusCode = 0;
			pointLocalMoverId = copy.pointLocalMoverId;
			groupFlags = copy.groupFlags;
			data[0] = copy.data[0];
			data[1] = copy.data[1];
		}
		
		void init (void);

		void init (OrderOriginType _origin, TacticalOrderCode _code, bool _unitOrder = false);

		void initWayPath (LocationNodePtr path);
		
		void initAttackWayPath (LocationNodePtr path);
		
		void destroy (void);

		TacticalOrder (void) {
			init();
		}

		~TacticalOrder (void)	{
			destroy();
		}

		void setId (long newId) {
			id = newId;
		}

		void setId (MechWarriorPtr pilot);

		long getId (void) {
			return(id);
		}

		long execute (MechWarriorPtr warrior, long& message);

		long status (MechWarriorPtr warrior);

		Stuff::Vector3D getWayPoint (long index);

		void setWayPoint (long index, Stuff::Vector3D wayPoint);

		void addWayPoint (Stuff::Vector3D wayPoint, long travelMode);

		GameObjectPtr getRamTarget (void);

		GameObjectPtr getJumpTarget (void);

		bool isGroupOrder (void);

		bool isCombatOrder (void);

		bool isMoveOrder (void);

		bool isJumpOrder (void);

		bool isWayPathOrder (void);

		long getParamData (float* time, long* paramList);

		long pack (MoverGroupPtr unit, MoverPtr point);

		long unpack (void);

		void setGroupFlag (long localMoverId, bool set);

		long getGroup (long commanderID, MoverPtr* moverList, MoverPtr* point, long sortType = 0);

		void setStage (long newStage) {
			stage = newStage;
		}

		GameObjectPtr getTarget (void);

		bool equals (TacticalOrder* tacOrder);

		char getStatusCode (void) {
			return(statusCode);
		}

		void debugString (MechWarriorPtr pilot, char* s);

};

//***************************************************************************

#endif
