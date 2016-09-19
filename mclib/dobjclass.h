//***************************************************************************
//
//	dobjclass.h -- File contains the Basic Game Object Class definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DOBJCLASS_H
#define DOBJCLASS_H

enum ObjectClass {
	INVALID = -1,
	BASEOBJECT = 0,
	TERRN,
	BATTLEMECH,
	GROUNDVEHICLE,
	ELEMENTAL,
	EXPLOSION,   //was EXPLODE
	FIRE,
	ARTILLERY,
	MOVER,
	GAMEOBJECT,
	BIGGAMEOBJECT,
	COMPONENT,
	WEAPON,
	PROJECTILE,
	LASERWEAPON,
	PPC,
	BUILDING,
	SMOKE,
	BULLET,
	DEBRIS,
	MAP_ICON,
	TREE,
	TERRAINOBJECT,
	MINE,
	BRIDGE,
	JET,
	PROJLASER,
	TREEBUILDING,
	CAMERADRONE,
	TRAINCAR,
	TURRET,
	GATE,
	KLIEG_LIGHT,
	WEAPONBOLT
};	

//***************************************************************************

#endif
