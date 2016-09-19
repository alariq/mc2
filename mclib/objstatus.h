//-----------------------------------------------------------------------------
// MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef OBJSTATUS_H
#define OBJSTATUS_H

typedef enum 
{
	OBJECT_STATUS_NORMAL,
	OBJECT_STATUS_DISABLED,
	OBJECT_STATUS_DESTROYED,
	OBJECT_STATUS_STARTING_UP,
	OBJECT_STATUS_SHUTTING_DOWN,
	OBJECT_STATUS_SHUTDOWN,
	NUM_OBJECT_STATUSES
} ObjectStatusType;

#endif