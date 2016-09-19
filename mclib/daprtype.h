//---------------------------------------------------------------------------
//
// DAprType.h -- File contains the Basic Game Appearance Type Definitions
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DAPRTYPE_H
#define DAPRTYPE_H
//---------------------------------------------------------------------------
// Include Files

//---------------------------------------------------------------------------
// Macro definitions
#define BASE_APPEARANCE	0L
#define SPRITE_TREE		0x01
#define VFX_APPEAR		0x02
#define FSY_APPEAR		0x03
#define LINE_APPEAR		0x04
#define GV_TYPE			0x05
#define ARM_APPEAR		0x06
#define BUILD_APPEAR	0x07
#define ELM_TREE		0x08
#define PU_TYPE			0x09
#define SMOKE_TYPE		0x0a
#define POLY_APPEARANCE	0x0b
#define MLR_APPEARANCE	0x0c
#define MECH_TYPE		0x0d
#define BLDG_TYPE		0x0e
#define TREED_TYPE		0x0f

#define BUILDING_APPR_TYPE	0x10
#define TREE_APPR_TYPE		0x11
#define VEHICLE_APPR_TYPE	0x12
#define MECH_APPR_TYPE		0x13
#define GENERIC_APPR_TYPE	0x14
//---------------------------------------------------------------------------
// Class definitions

class AppearanceType;
typedef AppearanceType *AppearanceTypePtr;

class AppearanceTypeList;
typedef AppearanceTypeList *AppearanceTypeListPtr;

//---------------------------------------------------------------------------
#endif