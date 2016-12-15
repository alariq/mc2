//---------------------------------------------------------------------------
//
// DGameObj.h -- File contains the Base game Object definition
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DGAMEOBJ_H
#define DGAMEOBJ_H

//---------------------------------------------------------------------------

#ifndef OBJSTATUS_H
#include"objstatus.h"
#endif

#ifndef DOBJCLASS_H
#include"dobjclass.h"
#endif

#include<stdint.h>

#ifndef NO_ERR
#define NO_ERR		0
#endif

//#define CANT_MAKE_BASE_FROM_TYPE				0xBEAD0001
#define CANT_MAKE_GAMEOBJECT_FROM_TYPE			0xBEAD0002
//#define CANT_DELETE_BASEOBJECT					0xBEAD0003
#define OBJECT_DEAD								0xBEADDEAD

//------------------------------------------------------------------------------------------
// Enum Definitions

//---------------------------------------------------------------------------

// ALL GAMEOBJECT flags
#define	OBJECT_FLAG_NONE			0x00000000
#define	OBJECT_FLAG_AWAKE			0x00000001
#define	OBJECT_FLAG_EXISTS			0x00000002
#define	OBJECT_FLAG_USEME			0x00000004
#define	OBJECT_FLAG_CAPTURED		0x00000008
#define	OBJECT_FLAG_TANGIBLE		0x00000010
#define	OBJECT_FLAG_SELECTED		0x00000020
#define	OBJECT_FLAG_TARGETED		0x00000040

#define	OBJECT_FLAG_JUSTCREATED		0x00000080
#define	OBJECT_FLAG_ONFIRE			0x00000100
#define	OBJECT_FLAG_ANIMATED		0x00000200
#define	OBJECT_FLAG_CAPTURABLE		0x00000400
#define	OBJECT_FLAG_FALLING			0x00000800
#define	OBJECT_FLAG_FALLEN			0x00001000
#define	OBJECT_FLAG_DAMAGED			0x00002000
#define	OBJECT_FLAG_MECHBAY			0x00004000
#define	OBJECT_FLAG_CANREFIT		0x00008000
#define	OBJECT_FLAG_TILECHANGED		0x00010000
#define	OBJECT_FLAG_DESTROYED		0x00020000
#define	OBJECT_FLAG_SPREAD			0x00040000
#define	OBJECT_FLAG_DONE			0x00080000
#define OBJECT_FLAG_OBSCURED		0x00100000	//Used to determine if we draw the object in revealed but not visible terrain
#define	OBJECT_FLAG_SENSOR			0x00200000
#define	OBJECT_FLAG_REMOVED			0x00400000
#define	OBJECT_FLAG_CONTROLBUILDING	0x00800000

//NOTE: 8 Most-Significant Bits used for object-specific flags:

//GroundVehicle and TURRET flags
#define	OBJECT_FLAG_CHECKED_ONSCREEN	0x01000000
#define	OBJECT_FLAG_CAN_FIRE			0x02000000
#define	OBJECT_FLAG_VEHICLE_APPR		0x04000000
#define	OBJECT_FLAG_NEVER_REVEALED		0x08000000
#define	OBJECT_FLAG_FACING_TARGET		0x10000000
#define	OBJECT_FLAG_POP_NEUTRALS		0x20000000

//Artillery flags
#define	OBJECT_FLAG_BOOM				0x01000000
#define	OBJECT_FLAG_SENSORS_GOING		0x02000000
#define	OBJECT_FLAG_RANDOM_OFFSET		0x04000000
#define	OBJECT_FLAG_WHOOSH				0x08000000

#define	OBJECT_DFLAG_DISABLE			0x0001

//---------------------------------------------------------------------------

#define	WEAPONHITCHUNK_TARGETTYPE_BITS		2
#define	WEAPONHITCHUNK_CAUSE_BITS			3
#define	WEAPONHITCHUNK_DAMAGE_BITS			10
#define	WEAPONHITCHUNK_REFIT_BITS			1
#define	WEAPONHITCHUNK_ENTRYQUAD_BITS		2
#define	WEAPONHITCHUNK_HITLOCATION_BITS		4
#define	WEAPONHITCHUNK_MOVERINDEX_BITS		7
#define	WEAPONHITCHUNK_SPECIALTYPE_BITS		2
#define	WEAPONHITCHUNK_SPECIALID_BITS		8
#define	WEAPONHITCHUNK_TARGETID_BITS		20

#define	WEAPONHITCHUNK_TARGETTYPE_MASK		0x00000003
#define	WEAPONHITCHUNK_CAUSE_MASK			0x00000007
#define	WEAPONHITCHUNK_DAMAGE_MASK			0x000003FF
#define	WEAPONHITCHUNK_REFIT_MASK			0x00000001
#define	WEAPONHITCHUNK_ENTRYQUAD_MASK		0x00000003
#define	WEAPONHITCHUNK_HITLOCATION_MASK		0x0000000F
#define	WEAPONHITCHUNK_MOVERINDEX_MASK		0x0000007F
#define	WEAPONHITCHUNK_SPECIALTYPE_MASK		0x00000003
#define	WEAPONHITCHUNK_SPECIALID_MASK		0x000000FF
#define	WEAPONHITCHUNK_TARGETID_MASK		0x000FFFFF

#define	WEAPONHITCHUNK_TARGET_MOVER			0
#define	WEAPONHITCHUNK_TARGET_TERRAIN		1
#define	WEAPONHITCHUNK_TARGET_SPECIAL		2
#define	WEAPONHITCHUNK_TARGET_LOCATION		3

#define	WEAPONHITCHUNK_SPECIAL_CAMERADRONE	0

//---------------------------------------------------------------------------

typedef int32_t GameObjectHandle;
typedef int32_t GameObjectTypeHandle;
typedef int32_t GameObjectWatchID;

class GameObject;
typedef GameObject* GameObjectPtr;

class SalvageItem;
typedef SalvageItem* SalvageItemPtr;

class WeaponFireChunk;
typedef WeaponFireChunk* WeaponFireChunkPtr;

class WeaponHitChunk;
typedef WeaponHitChunk* WeaponHitChunkPtr;

typedef struct _WeaponShotInfo* WeaponShotInfoPtr;

//typedef long *AICodePtr;
//---------------------------------------------------------------------------
// Macro Definitions

//---------------------------------------------------------------------------
// Enum Definitions

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit log
//
//
//---------------------------------------------------------------------------


