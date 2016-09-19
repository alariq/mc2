//***************************************************************************
//
//	SortList.cpp -- Sort List class
//
//	MechCommander II
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//--------------
// Include Files

#ifndef SORTLIST_H
#include"sortlist.h"
#endif

#ifndef ERR_H
#include"err.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include<stdlib.h>

#include<gameos.hpp>
//***************************************************************************
// Class SortList
//***************************************************************************

long SortList::init (long _numItems) {

	//-------------------------
	// Create the sort list...
	numItems = _numItems;
	list = (SortListNode*)systemHeap->Malloc(sizeof(SortListNode) * numItems);
	if (!list)
		Fatal(0, " Unable to init sortList ");
	return(list == NULL);
}

//---------------------------------------------------------------------------

void SortList::clear (bool setToMin) {

	for (long i = 0; i < numItems; i++)
		list[i].id = i;

	if (setToMin)
    {
		for (int i = 0; i < numItems; i++)
			list[i].value = float(-3.4E38);
    } else {
		for (int i = 0; i < numItems; i++)
			list[i].value = float(3.4E38);
    }
}

//---------------------------------------------------------------------------

int descendingCompare (const void* arg1, const void* arg2 ) {

	float value1 = ((SortListNode*)arg1)->value;
	float value2 = ((SortListNode*)arg2)->value;
	if (value1 > value2)
		return(-1);
	else if (value1 < value2)
		return(1);
	else
		return(0);
}

//---------------------------------------------------------------------------

int ascendingCompare (const void* arg1, const void* arg2 ) {

	float value1 = ((SortListNode*)arg1)->value;
	float value2 = ((SortListNode*)arg2)->value;
	if (value1 > value2)
		return(1);
	else if (value1 < value2)
		return(-1);
	else
		return(0);
}

//---------------------------------------------------------------------------

void SortList::sort (bool descendingOrder) {

	//------------------------------------------------------------------
	// For now, just use ANSI C's built-in qsort (ugly, but functional).
	if (descendingOrder)
		qsort((void*)list, (size_t)numItems, sizeof(SortListNode), descendingCompare);
	else
		qsort((void*)list, (size_t)numItems, sizeof(SortListNode), ascendingCompare);
}

//---------------------------------------------------------------------------

void SortList::destroy (void) {

	systemHeap->Free(list);
	list = NULL;
}

//***************************************************************************

