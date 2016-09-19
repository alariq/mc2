//***************************************************************************
//
//	movemgr.h -- Class definitions for the move path manager.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MOVEMGR_H
#define MOVEMGR_H

//---------------------------------------------------------------------------

#ifndef DMOVEMGR_H
#include"dmovemgr.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

//***************************************************************************

typedef struct _PathQueueRec* PathQueueRecPtr;

typedef struct _PathQueueRec {
	long				num;
	MechWarriorPtr		pilot;
	long				selectionIndex;
	unsigned long		moveParams;
	bool				initPath;
	bool				faceObject;
	PathQueueRecPtr		prev;
	PathQueueRecPtr		next;
} PathQueueRec;

//---------------------------------------------------------------------------

class MovePathManager {

	public:

		PathQueueRec		pool[MAX_MOVERS];
		PathQueueRecPtr		queueFront;
		PathQueueRecPtr		queueEnd;
		PathQueueRecPtr		freeList;
		static long			numPaths;
		static long			peakPaths;
		static long			sourceTally[50];

	public:

		void* operator new (size_t ourSize);

		void operator delete (void* us);
		
		MovePathManager (void) {
			init();
		}
		
		~MovePathManager (void) {
			destroy();
		}
		
		void destroy (void);
		
		long init (void);

		void remove (PathQueueRecPtr rec);

		PathQueueRecPtr remove (MechWarriorPtr pilot);

		void request (MechWarriorPtr pilot, long selectionIndex, unsigned long moveParams, long source);

		void calcPath (void);

		void update (void);
};

typedef MovePathManager* MovePathManagerPtr;

#endif

//***************************************************************************




