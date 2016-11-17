//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GOAL_H
#define GOAL_H

#include"dgoal.h"
#include"dgameobj.h"
#include"move.h"

#define	MAX_GATES					30
#define	MAX_MOVEGATES_CONTROLLED	10
#define	MAX_MOVEGATES_PER_ROOM		4
#define	MAX_OBJECTS_PER_ROOM		10
#define	MAX_CONTROLLED_OBJECTS		200

typedef enum {
	GOAL_NONE,
	GOAL_OBJECT,
	GOAL_REGION,
	NUM_GOAL_TYPES
} GoalType;

typedef enum {
	GOAL_LINK_NONE,
	GOAL_LINK_MOVE,
	GOAL_LINK_CONTROLS,
	GOAL_LINK_CONTROLLED,
	NUM_GOAL_LINK_TYPES
} GoalLinkType;

typedef union {
	struct {
		unsigned long		WID;
	} object;
	struct {
		short				minRow;
		short				minCol;
		short				maxRow;
		short				maxCol;
	} region;
} GoalInfo;

typedef struct _GoalLink {
	GoalObjectPtr			goalObject;
	GoalLinkType			type;
	long					cost;
	struct _GoalLink*		next;
} GoalLink;

typedef struct _GoalPathFindInfo {
	long					cost;
	long					parent;
	long					fromIndex;
	unsigned long			flags;
	long					g;
	long					hPrime;
	long					fPrime;
} GoalPathFindInfo;

typedef GoalLink* GoalLinkPtr;

class GoalObject {

	public:

		bool				used;
		GoalType			type;
		unsigned short		id;
		char				name[20];
		GoalLinkPtr			links;
		GoalObjectPtr		controller;
		GoalInfo			info;
		GoalObjectPtr		next;
		GoalObjectPtr		prev;
		GoalPathFindInfo	pathInfo;

	public:

		void* operator new (size_t ourSize);

		void operator delete (void* us);
		
		void init (void);

		GoalObject (void) {
			init ();
		}

		void destroy (void);

		~GoalObject (void)	{
			destroy();
		}

		void initObject (char* name, GameObjectPtr obj);

		void initRegion (char* name, long minRow, long minCol, long maxRow, long maxCol);

		void addLink (GoalObjectPtr gobject, GoalLinkType linkType);

		void addController (GoalObjectPtr gobject);
};

class GoalManager {

	public:

		long			numGoalObjects;
		GoalObjectPtr	goalObjects;
		long			goalObjectPoolSize;
		GoalObjectPtr	goalObjectPool;
		short			regionMap[2/*MAX_MAP_CELL_WIDTH*/][2/*MAX_MAP_CELL_WIDTH*/];
		long			numRegions;
		short*			fillStack;
		long			fillStackIndex;

	public:

		void* operator new (size_t ourSize);

		void operator delete (void* us);
		
		void init (void);

		GoalManager (void) {
			init ();
		}

		void destroy (void);

		~GoalManager (void)	{
			destroy();
		}

		void setup (long poolSize);

		void build (void);

		bool fillWallGateRegion (long row, long col, long region);

		bool fillRegion (long row, long col, long region);

		void calcRegions (void);

		long addLinks (GoalObjectPtr gobject, long numObjs, GameObjectPtr* objList);

		//long setControl (GoalObjectPtr controller, GoalObjectPtr controllee);

		GoalObjectPtr addRegion (GoalObjectPtr parent, GoalLinkType linkType, char* name, long minRow, long minCol, long maxRow, long maxCol);

		GoalObjectPtr addObject (GoalObjectPtr parent, GoalLinkType linkType, char* name, GameObjectPtr object);

		void clear (void);

		GoalObjectPtr newGoalObject (void);

		GoalObjectPtr calcGoal (int startCell[2], int goalCell[2]);

		GoalObjectPtr calcGoal (GameObjectPtr attacker, GameObjectPtr target);
		
		GoalObjectPtr calcGoal (GameObjectPtr attacker, Stuff::Vector3D location);
		
		GoalObjectPtr calcGoal (Stuff::Vector3D start, Stuff::Vector3D location);
};

#endif
