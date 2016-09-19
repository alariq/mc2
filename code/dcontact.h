//***************************************************************************
//
//	dcontact.h - This file contains the Contact Class header definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DCONTACT_H
#define DCONTACT_H

#define	MAX_SENSORS						150	// # sensor systems allowed per mission
#define	MAX_ECMS						50
#define	MAX_CONTACTS_PER_SENSOR			200

typedef enum _ContactStatus {
	CONTACT_NONE,
	CONTACT_SENSOR_QUALITY_1,
	CONTACT_SENSOR_QUALITY_2,
	CONTACT_SENSOR_QUALITY_3,
	CONTACT_SENSOR_QUALITY_4,
	CONTACT_VISUAL,
	NUM_CONTACT_STATUSES
} ContactStatus;

typedef enum {
	CONTACT_CRITERIA_NONE = 0,
	CONTACT_CRITERIA_ENEMY = 1,
	CONTACT_CRITERIA_VISUAL = 2,
	CONTACT_CRITERIA_GUARD_BREACH = 4,
	CONTACT_CRITERIA_NOT_CHALLENGED = 8,
	CONTACT_CRITERIA_SENSOR = 16,
	CONTACT_CRITERIA_VISUAL_OR_SENSOR = 32,
	CONTACT_CRITERIA_NOT_DISABLED = 64,
	CONTACT_CRITERIA_ARMED = 128,
	NUM_CONTACT_CRITERIA = 9
} ContactCriteria;

typedef enum {
	CONTACT_SORT_NONE,
	CONTACT_SORT_CV,
	CONTACT_SORT_DISTANCE,
	NUM_CONTACT_SORTS
} ContactSortType;

class ContactInfo;
typedef ContactInfo* ContactInfoPtr;

class SensorSystem;
typedef SensorSystem* SensorSystemPtr;

class TeamSensorSystem;
typedef TeamSensorSystem* TeamSensorSystemPtr;

class SensorSystemManager;
typedef SensorSystemManager* SensorSystemManagerPtr;

#endif

//***************************************************************************

