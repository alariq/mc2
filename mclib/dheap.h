//---------------------------------------------------------------------------
//
// Dheap.h -- This file contains the definition for the Base HEAP
//					Manager Class.  The Base HEAP manager creates,
//					manages and destroys block of memory using Win32
//					virtual memory calls.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DHEAP_H
#define DHEAP_H

//---------------------------------------------------------------------------
// Class declarations

struct HeapBlock;
typedef HeapBlock *HeapBlockPtr;

class HeapManager;
typedef HeapManager *HeapManagerPtr;

class UserHeap;
typedef UserHeap *UserHeapPtr;

//---------------------------------------------------------------------------
#endif