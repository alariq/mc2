//---------------------------------------------------------------------------
//
// DObjType.h -- File contains the Basic Game Object Type definitions
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DOBJTYPE_H
#define DOBJTYPE_H
//---------------------------------------------------------------------------
// Include Files
	
//---------------------------------------------------------------------------
// Macro definitions
#ifndef NO_ERR
#define NO_ERR	0
#endif

#define CANT_LOAD_INVALID_OBJECT				0xBEEF0001
#define OBJECT_TYPE_NUMBER_OUT_OF_RANGE			0xBEEF0002
#define OBJECT_TYPE_NUMBER_UNDEFINED			0xBEEF0003
#define CANT_OPEN_OBJECT_FILE					0xBEEF0004
#define CANT_DELETE_OBJECT_TYPE					0xBEEF0005
#define CANT_CREATE_OBJECT						0xBEEF0006
#define NO_RAM_FOR_OBJECT_TYPE_LIST				0xBEEF0007
#define NO_RAM_FOR_OBJECT_TYPE_FILE				0xBEEF0008
#define NO_RAM_FOR_OBJECT_TYPE_CACHE			0xBEEF0009
#define NO_RAM_FOR_OBJECT_CACHE					0xBEEF000A

enum ObjectTypeClass
{
	CRAPPY_OBJECT = -1,
	TREE_TYPE,
	BUILDING_TYPE,
	BATTLEMECH_TYPE,
	VEHICLE_TYPE,
	EXPLOSION_TYPE, //was EXPLODER_TYPE
	FIRE_TYPE,
	LASER_TYPE,
	SMOKES_TYPE,
	PROJECTILE_TYPE,
	MECHARM_TYPE,
	MAPICON_TYPE,
	TERRAINOBJECT_TYPE,
	ARTILLERY_TYPE,
	MINE_TYPE,
	ELEMENTAL_TYPE,
	BRIDGE_TYPE,
	JET_TYPE,
	PROJLASER_TYPE,
	TREEBUILDING_TYPE,
	CAMERADRONE_TYPE,
	TRAINCAR_TYPE,
	TURRET_TYPE,
	GATE_TYPE,
	KLIEG_LIGHT_TYPE,
	WEAPONBOLT_TYPE
};

/*
-	INVALID = -1,
	BASEOBJECT = 0,
	TERRN,
-	BATTLEMECH,
-	GROUNDVEHICLE,
-	ELEMENTAL,
-	EXPLODE,
-	FIRE,
-	ARTILLERY,
	MOVER,
	GAMEOBJECT,
	BIGGAMEOBJECT,
	COMPONENT,
	WEAPON,
-	PROJECTILE,
-	LASERWEAPON,
	PPC,
-	BUILDING,
-	SMOKE,
	BULLET,
-	DEBRIS,
-	MAP_ICON,
-	TREE,
-	TERRAINOBJECT,
-	MINE,
-	BRIDGE,
-	JET,
-	PROJLASER,
-	TREEBUILDING,
-	CAMERADRONE,
-	TRAINCAR,
-	TURRET,
-	GATE,
-	KLIEG_LIGHT
*/

//---------------------------------------------------------------------------
// Class definitions

class ObjectType;
typedef ObjectType* ObjectTypePtr;

class ObjectTypeManager;
typedef ObjectTypeManager* ObjectTypeManagerPtr;

typedef long ObjectTypeNumber;
//---------------------------------------------------------------------------
#endif
