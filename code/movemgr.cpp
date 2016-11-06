//***************************************************************************
//
//	movemgr.cpp -- File contains the MovePathManager class code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef MOVEMGR_H
#include"movemgr.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#include"gameos.hpp"


long MovePathManager::numPaths = 0;
long MovePathManager::peakPaths = 0;
long MovePathManager::sourceTally[50];
MovePathManagerPtr PathManager = NULL;

//***************************************************************************
// PATH MANAGER class
//***************************************************************************

void* MovePathManager::operator new (size_t mySize) {

	void *result = systemHeap->Malloc(mySize);
	return(result);
}

//---------------------------------------------------------------------------

void MovePathManager::operator delete (void* us) {

	systemHeap->Free(us);
}

//---------------------------------------------------------------------------

long MovePathManager::init (void) {

	for (long i = 0; i < MAX_MOVERS; i++) {
		pool[i].pilot = NULL;
		pool[i].selectionIndex = 0;
		pool[i].moveParams = 0;
		if (i > 0)
			pool[i].prev = &pool[i - 1];
		else
			pool[i].prev = NULL;
		if (i < (MAX_MOVERS - 1))
			pool[i].next = &pool[i + 1];
		else
			pool[i].next = NULL;
	}

	//------------------------------
	// All start on the free list...
	queueFront = NULL;
	queueEnd = NULL;
	freeList = &pool[0];

	numPaths = 0;
	peakPaths = 0;
	for (int i =0; i < 50; i++)
		sourceTally[i] = 0;

	return(NO_ERR);
}

//---------------------------------------------------------------------------

void MovePathManager::destroy (void) {

}

//---------------------------------------------------------------------------

void MovePathManager::remove (PathQueueRecPtr rec) {

	//------------------------------------
	// Remove it from the pending queue...
	if (rec->prev)
		rec->prev->next = rec->next;
	else
		queueFront = rec->next;
	if (rec->next)
		rec->next->prev = rec->prev;
	else
		queueEnd = rec->prev;

	//------------------------------------
	// Return the QRec to the free list...
	rec->prev = NULL;
	rec->next = freeList;
	freeList = rec;

	sourceTally[rec->num]--;
	numPaths--;
}

//---------------------------------------------------------------------------

PathQueueRecPtr MovePathManager::remove (MechWarriorPtr pilot) {

	PathQueueRecPtr rec = pilot->getMovePathRequest();
	if (rec) {
		remove(rec);
		pilot->setMovePathRequest(NULL);
		return(rec);
	}
	return(NULL);
}

//---------------------------------------------------------------------------

#define	DEBUG_MOVEPATH_QUEUE	0

void MovePathManager::request (MechWarriorPtr pilot, long selectionIndex, unsigned long moveParams, long source) {

	//-----------------------------------------------------
	// If the pilot is already awaiting a calc, purge it...
	remove(pilot);

	if (!freeList)
		Fatal(0, " Too many pilots calcing paths ");

	//---------------------------------------------
	// Grab the first free move path rec in line...
	PathQueueRecPtr pathQRec = freeList;

	//-----------------------------------------
	// Cut the new record from the free list...
	freeList = freeList->next;
	if (freeList)
		freeList->prev = NULL;

	//---------------------------------------------------
	// New record has no next. Already has no previous...
	pathQRec->num = source;
	pathQRec->pilot = pilot;
	pathQRec->selectionIndex = selectionIndex;
	pathQRec->moveParams = moveParams;

	if (queueEnd) {
		queueEnd->next = pathQRec;
		pathQRec->prev = queueEnd;
		pathQRec->next = NULL;
		queueEnd = pathQRec;
		}
	else {
		pathQRec->prev = NULL;
		pathQRec->next = NULL;
		queueFront = queueEnd = pathQRec;
	}
	pilot->setMovePathRequest(pathQRec);
	
	numPaths++;
	sourceTally[source]++;

	if (numPaths > peakPaths)
		peakPaths = numPaths;
}

//---------------------------------------------------------------------------

void MovePathManager::calcPath (void) {

	if (queueFront) {
		//------------------------------
		// Grab the next in the queue...
		PathQueueRecPtr curQRec = queueFront;
		remove(queueFront);

		//--------------------------------------------------
		// If the mover is no longer around, don't bother...
		MechWarriorPtr pilot = curQRec->pilot;
		pilot->setMovePathRequest(NULL);
		MoverPtr mover = pilot->getVehicle();
		if (!mover)
			return;

		/*long err = */pilot->calcMovePath(curQRec->selectionIndex, curQRec->moveParams);
	}
}

//----------------------------------------------------------------------------------
void DEBUGWINS_print (char* s, long window);
#ifdef LAB_ONLY
extern __int64 MCTimePath1Update;
extern __int64 MCTimePath2Update;
extern __int64 MCTimePath3Update;
extern __int64 MCTimePath4Update;
extern __int64 MCTimePath5Update;
extern __int64 MCTimeCalcGoal1Update;
extern __int64 MCTimeCalcGoal2Update;
extern __int64 MCTimeCalcGoal3Update;
extern __int64 MCTimeCalcGoal4Update;
extern __int64 MCTimeCalcGoal5Update;
extern __int64 MCTimeCalcGoal6Update;
#endif
void MovePathManager::update (void) {

	#ifdef LAB_ONLY
	MCTimePath1Update = 0;
	MCTimePath2Update = 0;
	MCTimePath3Update = 0;
	MCTimePath4Update = 0;
	MCTimePath5Update = 0;

	MCTimeCalcGoal1Update = 0;
	MCTimeCalcGoal2Update = 0;
	MCTimeCalcGoal3Update = 0;
	MCTimeCalcGoal4Update = 0;
	MCTimeCalcGoal5Update = 0;
	MCTimeCalcGoal6Update = 0;
#endif

	#ifdef MC_PROFILE
	QueryPerformanceCounter(startCk);
	#endif

	long numPathsToProcess = 6;
	//if (numPaths > 15)
	//	numPathsToProcess = 10;
	for (long i = 0; i < numPathsToProcess; i++) {
		if (!queueFront)
			break;
		calcPath();
	}

//	char s[50];
//	sprintf(s, "num paths = %d", numPaths);
//	DEBUGWINS_print(s, 0);

	#ifdef MC_PROFILE
	QueryPerformanceCounter(endCk);
	srMvPathUpd += (endCk.LowPart - startCk.LowPart);
	#endif
}

//***************************************************************************
