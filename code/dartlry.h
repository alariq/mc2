//***************************************************************************
//
//	dartlry.h -- File contains the Artillery Strike Object
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DARTLRY_H
#define DARTLRY_H

//---------------------------------------------------------------------------

class ArtilleryChunk;
typedef ArtilleryChunk* ArtilleryChunkPtr;

class ArtilleryType;
typedef ArtilleryType* ArtilleryTypePtr;

class Artillery;
typedef Artillery* ArtilleryPtr;

class CameraDroneType;
typedef CameraDroneType* CameraDroneTypePtr;

class CameraDrone;
typedef CameraDrone* CameraDronePtr;

//---------------------------------------------------------------------------

enum {
	ARTILLERY_SMALL,
	ARTILLERY_LARGE,
	ARTILLERY_SENSOR,
	NUM_ARTILLERY_TYPES
};

//***************************************************************************

#endif

