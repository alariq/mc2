//***************************************************************************
//
//	dmovemgr.h -- Class definitions for the move path manager.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DMOVEMGR_H
#define DMOVEMGR_H

//***************************************************************************

#define	MAX_MOVERS		255	// Should probably equal that in Collision System

typedef struct _PathQueueRec* PathQueueRecPtr;

class MoveMap;
typedef MoveMap* MoveMapPtr;

class MovePathManager;
typedef MovePathManager* MovePathManagerPtr;

#endif

//***************************************************************************




