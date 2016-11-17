//******************************************************************************************
//
//	group.h - This file contains the MoverGroup Class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GROUP_H
#define GROUP_H

//--------------
// Include Files

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif

#ifndef DGROUP_H
#include"dgroup.h"
#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#ifndef TACORDR_H
#include"tacordr.h"
#endif

//---------------------------------------------------------------------------

#define	GOALFLAG_OPEN			1
#define	GOALFLAG_CLOSED			2
#define	GOALFLAG_AVAILABLE		4
#define	GOALFLAG_NO_NEIGHBORS	8
#define	GOALFLAG_MOVER_HERE		16

typedef struct _GoalMapNode {
	short				cost;								// normal cost to travel here, based upon terrain
	//long				parent;								// where we came from (parent cell)
	unsigned char		flags;								// CLOSED, OPEN, STEP flags
	long				g;									// known cost from START to this node
	//long				hPrime;								// estimated cost from this node to GOAL
	//long				fPrime;								// = g + hPrime

	void setFlag (unsigned char flag) {
		flags |= flag;
	}

	void clearFlag (unsigned char flag) {
		flags &= (flag ^ 0xFFFFFFFF);
	}
} GoalMapNode;

typedef struct _MoverGroupData
{
	long					id;
	long					numMovers;
	GameObjectWatchID		moverWIDs[MAX_MOVERGROUP_COUNT];
	GameObjectWatchID		pointWID;
	bool					disbandOnNoPoint;

} MoverGroupData;

class MoverGroup {

	public:

		long					id;
		long					numMovers;
		GameObjectWatchID		moverWIDs[MAX_MOVERGROUP_COUNT];
		GameObjectWatchID		pointWID;
		bool					disbandOnNoPoint;

		static SortList			sortList;
		static GoalMapNode*		goalMap;

	public:

		void* operator new (size_t ourSize);
		void operator delete (void* us);

		virtual void init (void) {
			id = -1;
			numMovers = 0;
			pointWID = 0;
			disbandOnNoPoint = false;
			goalMap = NULL;
		}
		
		void init (MoverGroupData &data);

		MoverGroup (void) {
			init();
		}
			
		virtual void destroy (void);

		~MoverGroup (void) {
			destroy();
		}

		virtual long getId (void) {
			return(id);
		}

		virtual void setId (long _id) {
			id = _id;
		}

		//long calcRosterSize (BOOL checkUnits = TRUE);

		//void addToRoster (TeamPtr team, long* rosterIndex);

		long getNumMovers (void) {
			return(numMovers);
		}

		void setNumMovers (long num) {
			numMovers = num;
		}

		virtual bool add (MoverPtr mover);

		virtual bool remove (MoverPtr mover);

		virtual bool isMember (MoverPtr mover);

		virtual long disband (void);

		virtual long setPoint (MoverPtr mover);

		virtual MoverPtr getPoint (void);

		virtual void setDisbandOnNoPoint (bool setting) {
			disbandOnNoPoint = setting;
		}

		virtual bool getDisbandOnNoPoint (void) {
			return(disbandOnNoPoint);
		}

		MoverPtr getMover (long i);

		MoverPtr selectPoint (bool excludePoint);

		virtual long getMovers (MoverPtr* moverList);

		MechWarriorPtr getPointPilot (void);

		void statusCount (int* statusTally);

		void addToGUI (bool visible = true);

		long calcMoveGoals (Stuff::Vector3D goal, Stuff::Vector3D* goalList);

		long calcJumpGoals (Stuff::Vector3D goal, Stuff::Vector3D* goalList, GameObjectPtr DFATarget = NULL);

		//----------------
		// Tactical Orders

		virtual long handleTacticalOrder (TacticalOrder tacOrder, long priority, Stuff::Vector3D* jumpGoalList = NULL, bool queueGroupOrder = false);

		long orderMoveToPoint (bool setTacOrder, long origin, Stuff::Vector3D location, unsigned long params);

		long orderMoveToObject (bool setTacOrder, long origin, GameObjectPtr target, long fromArea, unsigned long params);

		long orderTraversePath (long origin, WayPathPtr wayPath, unsigned long params);

		long orderPatrolPath (long origin, WayPathPtr wayPath);

		long orderPowerDown (long origin);

		long orderPowerUp (long origin);

		long orderAttackObject (long origin, GameObjectPtr target, long attackType, long attackMethod, long attackRange, long aimLocation, long fromArea, unsigned long params);

		long orderWithdraw (long origin, Stuff::Vector3D location);

		long orderEject (long origin);

		//--------------
		// Combat Events
		void triggerAlarm (long alarmCode, unsigned long triggerId);

		long handleMateCrippled (unsigned long mateWID);

		long handleMateDisabled (unsigned long mateWID);

		long handleMateDestroyed (unsigned long mateWID);

		long handleMateEjected (unsigned long mateWID);

		void handleMateFiredWeapon (unsigned long mateWID);

		static void sortMovers (long numMoversInGroup, MoverPtr* moverList, Stuff::Vector3D dest);

		static long calcMoveGoals (Stuff::Vector3D goal, long numMovers, Stuff::Vector3D* goalList);

		static long calcJumpGoals (Stuff::Vector3D goal, long numMovers, Stuff::Vector3D* goalList, GameObjectPtr DFATarget);

		//----------------
		// Save Load
		void copyTo (MoverGroupData &data);
};

//***************************************************************************

#endif
