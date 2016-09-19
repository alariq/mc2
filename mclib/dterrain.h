//---------------------------------------------------------------------------
//
// DTerrain.h -- File contains definitions for Terrain and Terrain Window classes
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DTERRAIN_H
#define DTERRAIN_H
//---------------------------------------------------------------------------
// Include Files

//---------------------------------------------------------------------------
// Macro Definitions
/*
#define WATER			42
#define DIRT			0
#define GRASS			1
#define ROUGH			2
#define MOUNTAIN		3
#define GRASS2DIRT		4
#define DIRT2WATER		44
#define GRASS2WATER		43
#define MOUNT2GRASS		16
#define MOUNT2DIRT		27
#define CLIFF			40
*/
/*
#define VERROAD			59
#define R2W				61
#define HORROAD			49
#define ROAD			HORROAD
#define	RIVER			43
#define LEFT			8
#define RIGHT			2
#define UP				1
#define DOWN			4
#define VERTICAL		0
#define HORIZONTAL		1
*/
#define	TILESPERSIDE	(Terrain::visibleVerticesPerSide - 1)

//---------------------------------------------------------------------------
// Class Definitions
class Terrain;
typedef Terrain *TerrainPtr;

class TerrainWindow;
typedef TerrainWindow *TerrainWindowPtr;

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//---------------------------------------------------------------------------

