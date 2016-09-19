//***************************************************************************
//
//	dmover.h - This file contains the Mover Class header definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DMOVER_H
#define DMOVER_H

//---------------------------------------------------------------------------

#define	MAX_WEAPONS_PER_MOVER				32

enum {
	REFIT_VEHICLE,
	REFIT_BAY,
	NUM_FIXERS
};

enum {
	ARMOR_REFIT_COST,
	INTERNAL_REFIT_COST,
	AMMO_REFIT_COST,
	NUM_COSTS
};

struct LocationNode;
typedef LocationNode *LocationNodePtr;

class Mover;
typedef Mover* MoverPtr;

//******************************************************************************************

#endif
