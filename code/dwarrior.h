//***************************************************************************
//
//	dwarrior.h - This file contains the MechWarrior Class header definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DWARRIOR_H
#define DWARRIOR_H

#define	MAX_WAYPTS						15

#define	WEAPONS_STATUS_CANNOT_FIRE		-1
#define	WEAPONS_STATUS_NO_TARGET		-2
#define	WEAPONS_STATUS_OUT_OF_RANGE		-3

#define	WEAPON_STATUS_NOT_READY			-1
#define	WEAPON_STATUS_OUT_OF_AMMO		-2
#define	WEAPON_STATUS_OUT_OF_RANGE		-3
#define	WEAPON_STATUS_NOT_LOCKED		-4
#define	WEAPON_STATUS_NO_CHANCE			-5
#define	WEAPON_STATUS_TOO_HOT			-6

class GoalObject;
typedef GoalObject* GoalObjectPtr;

class MechWarrior;
typedef MechWarrior* MechWarriorPtr;

//------------------------------------------------------------------------------
#endif
