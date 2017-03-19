//***************************************************************************
//
//	PQueue.cpp -- Prototype for Priority Queue class
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------
// This implementation of a priority queue sorts with the SMALLEST
// item at the front of the queue (used for pathfinding purposes).

//***************************************************************************

//--------------
// Include Files

#ifndef HEAP_H
#include"heap.h"
#endif

#ifndef PQUEUE_H
#include"pqueue.h"
#endif

#include<gameos.hpp>
//***************************************************************************
// Class PriorityQueue
//***************************************************************************

int PriorityQueue::init (int max, int keyMinValue) {

	//-------------------------
	// Create the queue list...
	pqList = (PQNode*)systemHeap->Malloc(sizeof(PQNode) * (max + 2));
	gosASSERT (pqList != NULL);

	//----------------------------------------------------------------------
	// Note that two additional nodes are added, as the first and last nodes
	// of the queue list are used as sentinels (to assist implementation
	// execution speed)...
	maxItems = max + 2;
	keyMin = keyMinValue;
	return(0);
}

//---------------------------------------------------------------------------

void PriorityQueue::upHeap (int curIndex) {

	PQNode startNode = pqList[curIndex];
	long stopKey = startNode.key;
	pqList[0].key = keyMin;
	pqList[0].id = 0xFFFFFFFF;
	
	//--------------------
	// sort up the heap...
	while (pqList[curIndex/2].key >= stopKey) {
		pqList[curIndex] = pqList[curIndex/2];
		curIndex /= 2;
	}
	pqList[curIndex] = startNode;
}

//---------------------------------------------------------------------------

int PriorityQueue::insert (PQNode& item) {

	if (numItems == maxItems)
		return(1);

	pqList[++numItems] = item;
	upHeap(numItems);
	return(0);
}

//---------------------------------------------------------------------------

void PriorityQueue::downHeap (int curIndex) {

	//----------------------------------
	// Start at the top from curIndex...
	PQNode startNode = pqList[curIndex];
	int stopKey = startNode.key;

	//----------------------
	// Sort down the heap...
	while (curIndex <= numItems/2) {
		int nextIndex = curIndex << 1;
		if ((nextIndex < numItems) && (pqList[nextIndex].key > pqList[nextIndex + 1].key))
			nextIndex++;
		if (stopKey <= pqList[nextIndex].key)
			break;
		pqList[curIndex] = pqList[nextIndex];
		curIndex = nextIndex;
	}
	pqList[curIndex] = startNode;
}

//---------------------------------------------------------------------------

void PriorityQueue::remove (PQNode& item) {

	item = pqList[1];
	pqList[1] = pqList[numItems--];
	downHeap(1);
}

//---------------------------------------------------------------------------

void PriorityQueue::change (int itemIndex, int newValue) {

	if (newValue > pqList[itemIndex].key) {
		pqList[itemIndex].key = newValue;
		downHeap(itemIndex);
    }
	else if (newValue < pqList[itemIndex].key) {
		pqList[itemIndex].key = newValue;
		upHeap(itemIndex);
	}
}

//---------------------------------------------------------------------------

int PriorityQueue::find (unsigned int id) {

	for (int index = 0; index <= numItems; index++)
		if (pqList[index].id == id)
			return(index);
	return(0);
}

//---------------------------------------------------------------------------
	
void PriorityQueue::destroy (void) {

	systemHeap->Free(pqList);
	pqList = NULL;
	maxItems = 0;
	numItems = 0;
}

//***************************************************************************
