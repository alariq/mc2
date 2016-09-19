//***************************************************************************
//
//	Dteam.h -- File contains the Team Definition
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DTEAM_H
#define DTEAM_H

//---------------------------------------------------------------------------

#define	MAX_TEAMS						8
#define	MAX_MOVERS_PER_TEAM				120
#define	MAX_CONTACTS_PER_TEAM			MAX_MOVERS
#define	NUM_CONTACT_UPDATES_PER_PASS	3

typedef enum {
	RELATION_FRIENDLY,
	RELATION_NEUTRAL,
	RELATION_ENEMY,
	NUM_RELATIONS
} Relation;

//---------------------------------------------------------------------------

class Team;
typedef Team* TeamPtr;

struct _SystemTracker;
typedef _SystemTracker* SystemTrackerPtr;

#endif

//***************************************************************************


