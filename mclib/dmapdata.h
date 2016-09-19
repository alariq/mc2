//---------------------------------------------------------------------------
//
// DMapData.h -- File contains class definitions for the Terrain Mesh
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DMAPDATA_H
#define DMAPDATA_H
//---------------------------------------------------------------------------
// Include Files

//---------------------------------------------------------------------------
// Macro Definitions

//---------------------------------------------------------------------------
// Macro Definitions
typedef enum
{
	MC_BLUEWATER_TYPE,
	MC_GREEN_WATER_TYPE,
	MC_MUD_TYPE,
	MC_MOSS_TYPE,
	MC_DIRT_TYPE,
	MC_ASH_TYPE,
	MC_MOUNTAIN_TYPE,
	MC_TUNDRA_TYPE,
	MC_FORESTFLOOR_TYPE,
	MC_GRASS_TYPE,
	MC_CONCRETE_TYPE,
	MC_CLIFF_TYPE,
	MC_SLIMY_TYPE,
	MC_CEMENT2_TYPE,
	MC_CEMENT3_TYPE,
	MC_CEMENT4_TYPE,
	MC_CEMENT5_TYPE,
	MC_CEMENT6_TYPE,
	MC_CEMENT7_TYPE,
	MC_CEMENT8_TYPE,
	MC_NONE_TYPE,
	NUM_TERRAIN_TYPES
} TerrainType;

//---------------------------------------------------------------------------
// Class Definitions
struct PostcompVertex;
typedef PostcompVertex *PostcompVertexPtr;

class MapData;
typedef MapData *MapDataPtr;

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//---------------------------------------------------------------------------

