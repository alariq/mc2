//***************************************************************************
//
//	unitdesg.h - This file contains the unit designation definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef UNITDESG_H
#define UNITDESG_H

//***************************************************************************

#define	OBJ_ID_FIRST_COMMANDER	492
#define	OBJ_ID_LAST_COMMANDER	499

#define	OBJ_ID_FIRST_TEAM		500
#define	OBJ_ID_LAST_TEAM		508

//#define	OBJ_ID_FIRST_GROUP
#define PlayerId				500
#define ClanId					501
#define AlliedId				502

#define PlayerLance0			16
#define PlayerLance1			17
#define PlayerLance2			18
#define	PlayerLance3			19
#define	PlayerLance4			20
#define	PlayerLance5			21
#define	PlayerLance6			22
#define	PlayerLance7			23

#define ClanStar0				165
#define ClanStar1				166
#define ClanStar2				167
#define	ClanStar3				168
#define	ClanStar4				169
#define	ClanStar5				170
#define	ClanStar6				171
#define	ClanStar7				172

#define AlliedLance0			329
#define AlliedLance1			330
#define AlliedLance2			331
#define	AlliedLance3			332
#define	AlliedLance4			333
#define	AlliedLance5			334
#define	AlliedLance6			335
#define	AlliedLance7			336


#define	MIN_UNIT_PART_ID		1
#define	MAX_UNIT_PART_ID		511

#define	MIN_TEAM_PART_ID		500			//Teams are specialized units...
#define	MAX_TEAM_PART_ID		511

#define MIN_MOVER_PART_ID			512		//Movers now start here
#define	MIN_REINFORCEMENT_PART_ID	2050	//Reinforcements start here...
#define	MAX_REINFORCEMENT_PART_ID	3550	//Only 1500 reinforcements allowed...
#define MAX_MOVER_PART_ID			4095	//MIN_MOVER_PART_ID +
											//MAX_COMMANDERS * MAX_MOVERGROUPS * MAX_MOVERGROUP_COUNT_START

#define	MIN_TERRAIN_PART_ID		4096
#define	MAX_TERRAIN_PART_ID		464895		//MIN_TERRAIN_PART_ID +
											//block * (MAX_TERRAIN_BLOCK_VERTICES * MAX_TERRAIN_TILE_ITEMS) +
											//vertex * MAX_TERRAIN_TILE_ITEMS +
											//item

#define	MIN_TRAIN_PART_ID		512000
#define	MAX_TRAIN_PART_ID		521999		//MIN_TRAIN_PART_ID +
											//train * MAX_TRAIN_CARS +
											//car

#define	MIN_CAMERA_DRONE_ID		525000
#define	MAX_CAMERA_DRONE_ID		525999		//MIN_CAMERA_DRONE_ID + cameraDrone

#define	MAX_COMMANDERS				8
#define	MAX_MOVERGROUPS				16
#define MAX_MOVERGROUP_COUNT_START	12
#define MAX_MOVERGROUP_COUNT		16
#define	MAX_TRAIN_CARS				100
#define	MAX_TERRAIN_BLOCKS			144
#define	MAX_TERRAIN_BLOCK_VERTICES	400
#define	MAX_TERRAIN_TILE_ITEMS		8
#define	MAX_CAMERA_DRONES			1000
										
//***************************************************************************

#endif
