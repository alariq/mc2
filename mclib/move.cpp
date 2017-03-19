//***************************************************************************
//
//	move.cp - This file contains the Movement\Pathfinding routines
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//--------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#define	MOVELOG

#ifdef MOVELOG
#ifndef GAMELOG_H
#include"gamelog.h"
#endif
#endif

//***************************************************************************

#define	USE_SEPARATE_WATER_MAPS	FALSE

#ifndef TEAM_H
typedef enum {
	RELATION_FRIENDLY,
	RELATION_NEUTRAL,
	RELATION_ENEMY,
	NUM_RELATIONS
} Relation;
#define	MAX_TEAMS						8
#endif

//------------
// EXTERN vars

extern void memclear(void *Dest,int Length);
extern UserHeapPtr systemHeap;
extern float metersPerWorldUnit;

extern char* ExceptionGameMsg;
extern char ChunkDebugMsg[5120];

int GlobalMap::minRow = 0;
int GlobalMap::maxRow = 0;
int GlobalMap::minCol = 0;
int GlobalMap::maxCol = 0;
GameLogPtr GlobalMap::log = NULL;
bool GlobalMap::logEnabled = false;
//------------
// GLOBAL vars
bool ZeroHPrime = false;
bool CullPathAreas = false;

const float LOSincrement = 0.33f;		// based upon cell size
const float LOFincrement = 0.33f;		// based upon cell size
const float LOSensorIncrement = 2.0;	// based upon cell size (big, since we care only about tiles)
bool debugMoveMap = false;
bool ClearBridgeTiles = false;

long RamObjectWID = 0;

PriorityQueuePtr openList = NULL;
bool JumpOnBlocked = false;
bool FindingEscapePath = false;
bool BlockWallTiles = true;
MissionMapPtr GameMap = NULL;
GlobalMapPtr GlobalMoveMap[3] = {NULL, NULL, NULL};

extern float worldUnitsPerMeter;		//Assumes 90 pixel mechs
extern float metersPerWorldUnit;			//Assumes 90 pixel mechs

long DebugMovePathType = 0;

long SimpleMovePathRange = 21;

char reverseDir[NUM_DIRECTIONS] = {4, 5, 6, 7, 0, 1, 2, 3};
char rowShift[NUM_DIRECTIONS] = {-1, -1, 0, 1, 1, 1, 0, -1};
char colShift[NUM_DIRECTIONS] = {0, 1, 1, 1, 0, -1, -1, -1};

#define	NUM_CELL_OFFSETS 128
long cellShift[NUM_CELL_OFFSETS * 2] = {
	-1, 0,
	-1, 1,
	0, 1,
	1, 1,
	1, 0,
	1, -1,
	0, -1,
	-1, -1,

	-2, 0,
	-2, 2,
	0, 2,
	2, 2,
	2, 0,
	2, -2,
	0, -2,
	-2, -2,

	-3, 0,
	-3, 3,
	0, 3,
	3, 3,
	3, 0,
	3, -3,
	0, -3,
	-3, -3,

	-3, 2,
	-2, 3,
	2, 3,
	3, 2,
	3, -2,
	2, -3,
	-2, -3,
	-3, -2,

	-4, 0,
	-4, 4,
	0, 4,
	4, 4,
	4, 0,
	4, -4,
	0, -4,
	-4, -4,

	-5, 0,
	-5, 5,
	0, 5,
	5, 5,
	5, 0,
	5, -5,
	0, -5,
	-5, -5,

	-6, 0,
	-6, 6,
	0, 6,
	6, 6,
	6, 0,
	6, -6,
	0, -6,
	-6, -6,

	-7, 0,
	-7, 7,
	0, 7,
	7, 7,
	7, 0,
	7, -7,
	0, -7,
	-7, -7,

	-8, 0,
	-8, 8,
	0, 8,
	8, 8,
	8, 0,
	8, -8,
	0, -8,
	-8, -8,

	-9, 0,
	-9, 9,
	0, 9,
	9, 9,
	9, 0,
	9, -9,
	0, -9,
	-9, -9,

	-10, 0,
	-10, 10,
	0, 10,
	10, 10,
	10, 0,
	10, -10,
	0, -10,
	-10, -10,

	-11, 0,
	-11, 11,
	0, 11,
	11, 11,
	11, 0,
	11, -11,
	0, -11,
	-11, -11,

	-12, 0,
	-12, 12,
	0, 12,
	12, 12,
	12, 0,
	12, -12,
	0, -12,
	-12, -12,

	-13, 0,
	-13, 13,
	0, 13,
	13, 13,
	13, 0,
	13, -13,
	0, -13,
	-13, -13,

	-14, 0,
	-14, 14,
	0, 14,
	14, 14,
	14, 0,
	14, -14,
	0, -14,
	-14, -14,

	-15, 0,
	-15, 15,
	0, 15,
	15, 15,
	15, 0,
	15, -15,
	0, -15,
	-15, -15
};

float cellShiftDistance[NUM_CELL_OFFSETS];
char reverseShift[NUM_CELL_OFFSETS] = {
	4, 5, 6, 7, 0, 1, 2, 3,
	12, 13, 14, 15, 8, 9, 10, 11,
	20, 21, 22, 23, 16, 17, 18, 19,
	28, 29, 30, 31, 24, 25, 26, 27,
	36, 37, 38, 39, 32, 33, 34, 35,
	44, 45, 46, 47, 40, 41, 42, 43,
	52, 53, 54, 55, 48, 49, 50, 51,
	60, 61, 62, 63, 56, 57, 58, 59,
	68, 69, 70, 71, 64, 65, 66, 67,
	76, 77, 78, 79, 72, 73, 74, 75,
	84, 85, 86, 87, 80, 81, 82, 83,
	92, 93, 94, 95, 88, 89, 90, 91,
	100, 101, 102, 103, 96, 97, 98, 99,
	108,109,110,111,104,105,106,107,
	116,117,118,119,112,113,114,115,
	124,125,126,127,120,121,122,123
};

long tileMulMAPCELL_DIM[MAX_MAP_CELL_WIDTH];
float MapCellDiagonal = 0.0;
float HalfMapCell = 0.0;
float VerticesMapSideDivTwo = 0.0;
float MetersMapSideDivTwo = 0.0;

bool IsDiagonalStep[NUM_CELL_OFFSETS] = {
	false, true, false, true, false, true, false, true,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false
};

long StepAdjDir[9] = {-1, 0, 2, 2, 4, 4, 6, 6, 0};

long adjTile[4][2] = {
	{-1, 0},
	{0, 1},
	{1, 0},
	{0, -1}
};

inline float agsqrt( float _a, float _b )
{
	return sqrt(_a*_a + _b*_b);
}

//-------------------------------------------------
// Only pattern 2 is used now.
char mineLayout[4][MAPCELL_DIM * MAPCELL_DIM] = {
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{1,0,1,0,1,0,1,0,1},
	{0,0,0,0,0,0,0,0,0}
};

char TeamRelations[MAX_TEAMS][MAX_TEAMS] = {
	{0, 2, RELATION_NEUTRAL, 2, 2, 2, 2, 2},
	{2, 0, 2, 2, 2, 2, 2, 2},
	{RELATION_NEUTRAL, 2, 0, 2, 2, 2, 2, 2},
	{2, 2, 2, 0, 2, 2, 2, 2},
	{2, 2, 2, 2, 0, 2, 2, 2},
	{2, 2, 2, 2, 2, 0, 2, 2},
	{2, 2, 2, 2, 2, 2, 0, 2},
	{2, 2, 2, 2, 2, 2, 2, 0}
};

bool GoalIsDoor = false;
long numNodesVisited = 0;
long topOpenNodes = 0;
long MaxHPrime = 1000;
bool PreserveMapTiles = false;

MoveMapPtr PathFindMap[2] = {NULL, NULL};

float MoveMap::distanceFloat[DISTANCE_TABLE_DIM][DISTANCE_TABLE_DIM];
int MoveMap::distanceInt[DISTANCE_TABLE_DIM][DISTANCE_TABLE_DIM];
int MoveMap::forestCost = 50;

#ifdef LAB_ONLY
__int64 MCTimeCalcPath1Update = 0;
__int64 MCTimeCalcPath2Update = 0;
__int64 MCTimeCalcPath3Update = 0;
__int64 MCTimeCalcPath4Update = 0;
__int64 MCTimeCalcPath5Update = 0;
#endif

static const char* typeString[] = {
    "normal area",
    "wall area",
    "gate area",
    "land bridge area",
    "forest area"
};

//***************************************************************************
// MISC routines
//***************************************************************************

inline void calcAdjNode (long& r, long& c, long direction) {

	r += rowShift[direction];
	c += colShift[direction];
}

//---------------------------------------------------------------------------

inline bool inMapBounds (long r, long c, long mapHeight, long mapWidth) {

	return((r >= 0) && (r < mapHeight) && (c >= 0) && (c < mapWidth));
}

//---------------------------------------------------------------------------

Stuff::Vector3D relativePositionToPoint (Stuff::Vector3D point, float angle, float distance, unsigned long flags) {

	//--------------------------------------------------------
	// Note that the angle should be -180 <= angle <= 180, and
	// the distance is in meters...

	Stuff::Vector3D curPos;
	curPos.x = point.x;
	curPos.y = point.y;
	curPos.z = 0.0;

	distance *= -worldUnitsPerMeter;

	Stuff::Vector3D shiftVect;
	shiftVect.x = 0.0;
	shiftVect.y = 1.0;
	shiftVect.z = 0.0;

	Rotate(shiftVect,angle);

	shiftVect *= distance;
	Stuff::Vector3D relPos;
	relPos.Zero();
	relPos.Add(curPos,shiftVect);

	Stuff::Vector3D start2d;
	Stuff::Vector3D goal2d;
	Stuff::Vector3D deltaVector;
	if (flags & RELPOS_FLAG_PASSABLE_START) {
		start2d  = curPos;
		goal2d = relPos;
		}
	else {
		start2d = relPos;
		goal2d = curPos;
	}

	deltaVector.Subtract(goal2d,start2d);

	//-------------------------------------------------------------
	// First, we need to calc the delta vector--how much we extend
	// the ray everytime we check the map cell for clear placement.
	deltaVector.Normalize(deltaVector);
	deltaVector *= Terrain::halfWorldUnitsPerCell;
	if (deltaVector.GetLength() == 0.0)
		return(curPos);

	//-------------------------------------------------
	// Determine the max length the ray must be cast...
	float maxLength = distance_from(goal2d,start2d);

	//------------------------------------------------------------
	// We'll start at the target, and if it's blocked, we'll move
	// toward our start location, looking for the first valid/open
	// cell...
	Stuff::Vector3D curPoint = start2d;
	Stuff::Vector3D curRay;
	curRay.Zero();
	float rayLength = 0.0;

	int cellR, cellC;
	Stuff::Vector3D curPoint3d(curPoint.x, curPoint.y, 0.0);
	land->worldToCell(curPoint3d, cellR, cellC);
	unsigned int cellClear = GameMap->getPassable(cellR, cellC);

	Stuff::Vector3D lastGoodPoint = curPoint;
	if (flags & RELPOS_FLAG_PASSABLE_START)
		while (cellClear && (rayLength < maxLength)) {
			lastGoodPoint = curPoint;
			curPoint += deltaVector;
			curRay.Subtract(curPoint,start2d);
			rayLength = curRay.GetLength();

			curPoint3d.x = curPoint.x;
			curPoint3d.x = curPoint.y;
			curPoint3d.z = 0.0;

			land->worldToCell(curPoint3d, cellR, cellC);
			cellClear = GameMap->getPassable(cellR, cellC);
		}
	else
		while (!cellClear && (rayLength < maxLength)) {
			lastGoodPoint = curPoint;
			curPoint += deltaVector;
			curRay.Subtract(curPoint,start2d);
			rayLength = curRay.GetLength();

			curPoint3d.x = curPoint.x;
			curPoint3d.x = curPoint.y;
			curPoint3d.z = 0.0;

			land->worldToCell(curPoint3d, cellR, cellC);
			cellClear = GameMap->getPassable(cellR, cellC);
		}

	//----------------------------------------
	// lastGoodPoint MUST be on Map or BOOM!!
	// back on map if necessary
	float maxMap = Terrain::worldUnitsMapSide / 2.0;
	float safety = maxMap - Terrain::worldUnitsPerVertex;
	bool offMap = false;

	if (lastGoodPoint.x < -safety)
	{
		lastGoodPoint.x = -safety;
		offMap = true;
	}

	if (lastGoodPoint.x > safety)
	{
		lastGoodPoint.x = safety;
		offMap = true;
	}

	if (lastGoodPoint.y < -safety)
	{
		lastGoodPoint.y = -safety;
		offMap = true;
	}

	if (lastGoodPoint.y > safety)
	{
		lastGoodPoint.y = safety;
		offMap = true;
	}
	
	curPoint3d.x = lastGoodPoint.x;
	curPoint3d.x = lastGoodPoint.y;
	curPoint3d.z = 0.0;

	curPoint3d.z = land->getTerrainElevation(curPoint3d);
	return(curPoint3d);
}

//***************************************************************************
// MISSION MAP class (new Game Map class)
//***************************************************************************
//#define DEBUG_GLOBALMAP_BUILD

void MOVE_init (long moveRange) {

	if (PathFindMap[SECTOR_PATHMAP])
		Fatal(0, " MOVE_Init: Already called this! ");

	PathFindMap[SECTOR_PATHMAP] = new MoveMap;
	if (!PathFindMap[SECTOR_PATHMAP])
		Fatal(0, " MOVE_Init: Cannot initialize PathFindMap ");
	PathFindMap[SECTOR_PATHMAP]->init(SECTOR_DIM*2, SECTOR_DIM*2);

	PathFindMap[SIMPLE_PATHMAP] = new MoveMap;
	if (!PathFindMap[SIMPLE_PATHMAP])
		Fatal(0, " MOVE_Init: Cannot initialize PathFindMap ");
	SimpleMovePathRange = moveRange;
	if (SimpleMovePathRange <= 20)
		Fatal(0, " MOVE_Init: MoveRange TOO SMALL. Go see Glenn. ");
	PathFindMap[SIMPLE_PATHMAP]->init(SimpleMovePathRange * 2 + 1, SimpleMovePathRange * 2 + 1);
}

//---------------------------------------------------------------------------

bool EditorSave = false;
long tempNumSpecialAreas = 0;
GameObjectFootPrint* tempSpecialAreaFootPrints = NULL;

void MOVE_buildData (long height, long width, MissionMapCellInfo* mapData, long numSpecialAreas, GameObjectFootPrint* specialAreaFootPrints) {

	EditorSave = true;

	if (GameMap)
		delete GameMap;
	GameMap = new MissionMap;
	gosASSERT(GameMap != NULL);

	if (mapData)
		GameMap->init(height, width, 0, mapData);
	else
		GameMap->init( height, width );

	long numOffMap = 0;
	for (long r = 0; r < height; r++)
		for (long c = 0; c < width; c++) {
			Stuff::Vector3D worldPos;
			land->cellToWorld(r, c, worldPos);
			bool set = (land->IsEditorSelectTerrainPosition(worldPos) && !land->IsGameSelectTerrainPosition(worldPos));
			GameMap->setOffMap(r, c, set);
			if (set)
				numOffMap++;
		}

	if (tempSpecialAreaFootPrints) {
		systemHeap->Free(tempSpecialAreaFootPrints);
		tempSpecialAreaFootPrints = NULL;
	}
	tempNumSpecialAreas = numSpecialAreas;
	if (tempNumSpecialAreas) {
		tempSpecialAreaFootPrints = (GameObjectFootPrint*)systemHeap->Malloc(sizeof(GameObjectFootPrint) * tempNumSpecialAreas);
		memcpy(tempSpecialAreaFootPrints, specialAreaFootPrints, sizeof(GameObjectFootPrint) * tempNumSpecialAreas);
	}
	if (!PathFindMap[SECTOR_PATHMAP])
		Fatal(0, " MOVE_BuildData: Must call MOVE_Init()! ");

	if (GlobalMoveMap[0])
	{
		delete GlobalMoveMap[0];
		GlobalMoveMap[0] = NULL;
	}

	if (GlobalMoveMap[1])
	{
		delete GlobalMoveMap[1];
		GlobalMoveMap[1] = NULL;
	}

	if (GlobalMoveMap[2])
	{
		delete GlobalMoveMap[2];
		GlobalMoveMap[2] = NULL;
	}

	GlobalMoveMap[0] = new GlobalMap;
	if (!GlobalMoveMap[0])
		Fatal(0, " MOVE_Init: Cannot initialize GlobalMoveMap ");
	GlobalMoveMap[0]->build(mapData);

	GlobalMoveMap[1] = new GlobalMap;
	if (!GlobalMoveMap[1])
		Fatal(0, " MOVE_Init: Cannot initialize GlobalMoveMap ");
	GlobalMoveMap[1]->hover = true;
	GlobalMoveMap[1]->build(mapData);

	GlobalMoveMap[2] = new GlobalMap;
	if (!GlobalMoveMap[2])
		Fatal(0, " MOVE_Init: Cannot initialize HeliGlobalMoveMap ");
	GlobalMoveMap[2]->blank = true;
	GlobalMoveMap[2]->build(NULL);

	//-----------------------
	// Just some debugging...
	//for (r = 0; r < height; r++)
	//	for (long c = 0; c < width; c++)
	//		if (GameMap->getOffMap(r, c))
	//			numOffMap++;

	//-------------------------------------------------------------------------------------
	// Since we use this bitfield during the globalmap build for something else, we can now
	// reset it...
	for (long row = 0; row < GameMap->height; row++)
		for (long col = 0; col < GameMap->width; col++)
			for (long i = 0; i < NUM_MOVE_LEVELS; i++)
				GameMap->setPathlock(i, row, col, false);
	
	EditorSave = false;
}

//---------------------------------------------------------------------------

long MOVE_saveData (PacketFile* packetFile, long whichPacket) {

	if (!GameMap)
		Fatal(0, " MOVE_SaveData: Cannot initialize GameMap ");
	if (!GlobalMoveMap[0])
		Fatal(0, " MOVE_SaveData: Cannot initialize GlobalMoveMap ");
	if (!packetFile)
		return(GameMap->write(NULL) + GlobalMoveMap[0]->write(NULL) + GlobalMoveMap[1]->write(NULL) + GlobalMoveMap[2]->write(NULL) + 2);

	//-----------------------
	// Just some debugging...
	//long numOffMap = 0;
	//for (long r = 0; r < GameMap->height; r++)
	//	for (long c = 0; c < GameMap->width; c++)
	//		if (GameMap->getOffMap(r, c))
	//			numOffMap++;

	long numMissionMapPackets = GameMap->write(packetFile, whichPacket);
	long numGlobalMap0Packets = GlobalMoveMap[0]->write(packetFile, whichPacket + numMissionMapPackets);
	long numGlobalMap1Packets = GlobalMoveMap[1]->write(packetFile, whichPacket + numMissionMapPackets + numGlobalMap0Packets);
	long numGlobalMap2Packets = GlobalMoveMap[2]->write(packetFile, whichPacket + numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets);

	long result = packetFile->writePacket(whichPacket + numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets + numGlobalMap2Packets, (unsigned char*)&tempNumSpecialAreas, sizeof(long));
	if (result <= 0)
		Fatal(result, " MOVE_saveData: Unable to write num special area ");
	//-------------------------------------------------------------------------------------------------
	// If there are no special areas, we just write the specialArea count as a filler for the packet...
	if (tempNumSpecialAreas == 0)
		result = packetFile->writePacket(whichPacket + numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets + numGlobalMap2Packets + 1, (unsigned char*)&tempNumSpecialAreas, sizeof(long));
	else
		result = packetFile->writePacket(whichPacket + numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets + numGlobalMap2Packets + 1, (unsigned char*)tempSpecialAreaFootPrints, sizeof(GameObjectFootPrint) * tempNumSpecialAreas);
	if (result <= 0)
		Fatal(result, " MOVE_saveData: Unable to write special area footprints ");

	return(numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets + numGlobalMap2Packets + 2);
}

//---------------------------------------------------------------------------

long MOVE_readData (PacketFile* packetFile, long whichPacket) {

	if (GameMap)
		delete GameMap;
	GameMap = new MissionMap;
	gosASSERT(GameMap != NULL);
	long numMissionMapPackets = GameMap->init(packetFile, whichPacket);

	//-----------------------
	// Just some debugging...
	//long numOffMap[2] = {0,0};
	//for (long r = 0; r < GameMap->height; r++)
	//	for (long c = 0; c < GameMap->width; c++) {
	//		if (GameMap->getOffMap(r, c))
	//			numOffMap[0]++;
	//		Stuff::Vector3D worldPos;
	//		land->cellToWorld(r, c, worldPos);
	//		bool set = (land->IsEditorSelectTerrainPosition(worldPos) && !land->IsGameSelectTerrainPosition(worldPos));
	//		if (set)
	//			numOffMap[1]++;
	//		if (set && !GameMap->getOffMap(r, c))
	//			set = false;
	//	}
	
	if (!PathFindMap[SECTOR_PATHMAP])
		Fatal(0, " MOVE_BuildData: Must call MOVE_Init()! ");

	if (GlobalMoveMap[0])
		delete GlobalMoveMap[0];
	GlobalMoveMap[0] = new GlobalMap;
	if (!GlobalMoveMap[0])
		Fatal(0, " MOVE_Init: Cannot initialize GlobalMoveMap0 ");
	long numGlobalMap0Packets = GlobalMoveMap[0]->init(packetFile, whichPacket + numMissionMapPackets);
	long numGlobalMap1Packets = 0;
	long numGlobalMap2Packets = 0;
	if (!GlobalMoveMap[0]->badLoad) {
		//---------------------------------
		// Let's read the hover move map...
		if (GlobalMoveMap[1]) {
			delete GlobalMoveMap[1];
			GlobalMoveMap[1] = NULL;
		}
		GlobalMoveMap[1] = new GlobalMap;
		if (!GlobalMoveMap[1])
			Fatal(0, " MOVE_Init: Cannot initialize GlobalMoveMap1 ");
		GlobalMoveMap[1]->hover = true;
		numGlobalMap1Packets = GlobalMoveMap[1]->init(packetFile, whichPacket + numMissionMapPackets + numGlobalMap0Packets);

		
		//--------------------------------------
		// Let's read the helicoptor move map...
		if (GlobalMoveMap[2]) {
			delete GlobalMoveMap[2];
			GlobalMoveMap[2] = NULL;
		}
		GlobalMoveMap[2] = new GlobalMap;
		if (!GlobalMoveMap[2])
			Fatal(0, " MOVE_Init: Cannot initialize GlobalMoveMap2 ");
		GlobalMoveMap[2]->blank = true;
		numGlobalMap2Packets = GlobalMoveMap[2]->init(packetFile, whichPacket + numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets);
		//------------------------------------------------------------------
		// Delete the pathExistsTable for this one, since we don't use it...
		systemHeap->Free(GlobalMoveMap[2]->pathExistsTable);
		GlobalMoveMap[2]->pathExistsTable = NULL;

		long numBytes = packetFile->readPacket(whichPacket + numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets + numGlobalMap2Packets, (unsigned char*)&tempNumSpecialAreas);
		if (numBytes <= 0)
			Fatal(numBytes, " MOVE_readData: Unable to read num special areas ");
		if (tempSpecialAreaFootPrints) {
			systemHeap->Free(tempSpecialAreaFootPrints);
			tempSpecialAreaFootPrints = NULL;
		}
		if (tempNumSpecialAreas > 0) {
			tempSpecialAreaFootPrints = (GameObjectFootPrint*)systemHeap->Malloc(sizeof(GameObjectFootPrint) * tempNumSpecialAreas);
			numBytes = packetFile->readPacket(whichPacket + numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets + numGlobalMap2Packets + 1, (unsigned char*)tempSpecialAreaFootPrints);
			if (numBytes <= 0)
				Fatal(numBytes, " MOVE_readData: Unable to read num special areas ");
		}

	}

	//-----------------------
	// Just some debugging...
	//numOffMap[0] = 0;
	//for (r = 0; r < GameMap->height; r++)
	//	for (long c = 0; c < GameMap->width; c++)
	//		if (GameMap->getOffMap(r, c))
	//			numOffMap[0]++;

	return(numMissionMapPackets + numGlobalMap0Packets + numGlobalMap1Packets + numGlobalMap2Packets + 2);
}

//---------------------------------------------------------------------------

void MOVE_cleanup (void) {

	if (GameMap) {
		delete GameMap;
		GameMap = NULL;
	}

	if (GlobalMoveMap[0]) 
	{
		delete GlobalMoveMap[0];
		GlobalMoveMap[0] = NULL;
	}
	if (GlobalMoveMap[1]) 
	{
		delete GlobalMoveMap[1];
		GlobalMoveMap[1] = NULL;
	}
	if (GlobalMoveMap[2]) 
	{
		delete GlobalMoveMap[2];
		GlobalMoveMap[2] = NULL;
	}

	if (PathFindMap[SECTOR_PATHMAP]) {
		delete PathFindMap[SECTOR_PATHMAP];
		PathFindMap[SECTOR_PATHMAP] = NULL;
	}

	if (PathFindMap[SIMPLE_PATHMAP]) {
		delete PathFindMap[SIMPLE_PATHMAP];
		PathFindMap[SIMPLE_PATHMAP] = NULL;
	}
#ifdef DEBUG_GLOBALMAP_BUILD
	GameLog::cleanup();
#endif
}

//---------------------------------------------------------------------------

void* MissionMap::operator new (size_t ourSize) {

	void* result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void MissionMap::operator delete (void* us) {

	systemHeap->Free(us);
}	

//---------------------------------------------------------------------------

void MissionMap::init (long h, long w) {

	height = h;
	width = w;

	VerticesMapSideDivTwo = (Terrain::blocksMapSide * Terrain::verticesBlockSide) / 2.0;
	MetersMapSideDivTwo = Terrain::worldUnitsMapSide / 2;
	MapCellDiagonal = Terrain::worldUnitsPerCell * 1.4142 * metersPerWorldUnit;
	HalfMapCell = Terrain::worldUnitsPerCell / 2.0;

	for (long i = 0; i < MAX_MAP_CELL_WIDTH; i++)
		tileMulMAPCELL_DIM[i] = i * MAPCELL_DIM;

	long tileWidth = width / MAPCELL_DIM;
	long tileHeight = height / MAPCELL_DIM;
	for (int i = 0; i < tileHeight; i++)
		Terrain::tileRowToWorldCoord[i] = (Terrain::worldUnitsMapSide / 2.0) - (i * Terrain::worldUnitsPerVertex);
	for (int i = 0; i < tileWidth; i++)
		Terrain::tileColToWorldCoord[i] = (i * Terrain::worldUnitsPerVertex) - (Terrain::worldUnitsMapSide / 2.0);
	for (int i = 0; i < MAPCELL_DIM; i++)
		Terrain::cellToWorldCoord[i] = (Terrain::worldUnitsPerVertex / (float)MAPCELL_DIM) * i;

	for (int i = 0; i < height; i++)
		Terrain::cellRowToWorldCoord[i] = (Terrain::worldUnitsMapSide / 2.0) - (i * Terrain::worldUnitsPerCell);
	for (int i = 0; i < width; i++)
		Terrain::cellColToWorldCoord[i] = (i * Terrain::worldUnitsPerCell) - (Terrain::worldUnitsMapSide / 2.0);

	if (map) {
		systemHeap->Free(map);
		map = NULL;
	}
	long mapSize = sizeof(MapCell) * width * height;
	if (mapSize > 0)
		map = (MapCellPtr)systemHeap->Malloc(sizeof(MapCell) * width * height);
	gosASSERT(map != NULL);
	memclear(map, sizeof(MapCell) * width * height);
}

//---------------------------------------------------------------------------

#define	NUM_MISSIONMAP_PACKETS	4

long MissionMap::init (PacketFile* packetFile, long whichPacket) {

	packetFile->readPacket(whichPacket++, (unsigned char*)&height);
	packetFile->readPacket(whichPacket++, (unsigned char*)&width);
	packetFile->readPacket(whichPacket++, (unsigned char*)&planet);
	init(height, width);
	packetFile->readPacket(whichPacket++, (unsigned char*)map);

	//-----------------------
	// Just some debugging...
	//long numOffMap = 0;
	//for (long r = 0; r < height; r++)
	//	for (long c = 0; c < width; c++) {
	//		Stuff::Vector3D worldPos;
	//		land->cellToWorld(r, c, worldPos);
	//		bool set = (land->IsEditorSelectTerrainPosition(worldPos) && !land->IsGameSelectTerrainPosition(worldPos));
	//		GameMap->setOffMap(r, c, set);
	//		if (set)
	//			numOffMap++;
	//	}

	for (long r = 0; r < height; r++)
		for (long c = 0; c < width; c++) {
			GameMap->setMover(r, c, false);
		}

	return(NUM_MISSIONMAP_PACKETS);
}

//---------------------------------------------------------------------------

long MissionMap::write (PacketFile* packetFile, long whichPacket) {

	//-------------------------------------------
	// Return number of packets if !packetFile...
	if (!packetFile)
		return(NUM_MISSIONMAP_PACKETS);

	long result = packetFile->writePacket(whichPacket++, height);
	if (result <= 0)
		Fatal(result, " MissionMap.write: Unable to write height packet ");
	result = packetFile->writePacket(whichPacket++, width);
	if (result <= 0)
		Fatal(result, " MissionMap.write: Unable to write width packet ");
	result = packetFile->writePacket(whichPacket++, planet);
	if (result <= 0)
		Fatal(result, " MissionMap.write: Unable to write planet packet ");
	result = packetFile->writePacket(whichPacket++, (unsigned char*)map, sizeof(MapCell) * height * width);
	if (result <= 0)
		Fatal(result, " MissionMap.write: Unable to write map packet ");
	return(NUM_MISSIONMAP_PACKETS);
}

//---------------------------------------------------------------------------

bool MissionMap::getPassable (Stuff::Vector3D cellPosition) {

	int row, col;
	land->worldToCell(cellPosition, row, col);
	return(getPassable(row, col));
}

//---------------------------------------------------------------------------

long MissionMap::init (long cellHeight, long cellWidth, long curPlanet, MissionMapCellInfo* mapData) {

	if (!map)
		init(cellHeight, cellWidth);

	planet = curPlanet;

	//---------------------------------------------------------------------------
	// Be damned sure map is empty.  This is required or mines will NOT go away.
	memset(map, 0, sizeof(MapCell) * width * height);

	long terrainTypes[1024];
	long overlayTypes[1024];
	for (long i = 0; i < 256; i++)
		terrainTypes[i] = overlayTypes[i] = 0;

	if (mapData)
		for (long row = 0; row < height; row++)
			for (long col = 0; col < width; col++) {
				setTerrain(row, col, mapData[row * width + col].terrain);
				terrainTypes[mapData[row * width + col].terrain]++;
				setOverlay(row, col, mapData[row * width + col].overlay);
				overlayTypes[mapData[row * width + col].overlay]++;
				setGate(row, col, mapData[row * width + col].gate);
				setRoad(row, col, mapData[row * width + col].road);
				setPassable(row, col, mapData[row * width + col].passable);
				setLocalHeight(row, col, mapData[row * width + col].lineOfSight);
				setPreserved(row, col, false);
				setMine(row, col, mapData[row * width + col].mine);
				setForest(row, col, mapData[row * width + col].forest);
				setBuildWall(row, col, false);
				setBuildGate(row, col, false);
				Stuff::Vector3D worldPos;
				land->cellToWorld(row, col, worldPos);
				long cellWaterType = land->getWater(worldPos);
				if (cellWaterType == WATER_TYPE_SHALLOW)
					setShallowWater(row, col, true);
				if (cellWaterType == WATER_TYPE_DEEP)
					setDeepWater(row, col, true);
				for (long i = 0; i < NUM_MOVE_LEVELS; i++)
					GameMap->setPathlock(i, row, col, false);
				switch (mapData[row * width + col].specialType) {
					case SPECIAL_WALL:
						setBuildWall(row, col, true);
						break;
					case SPECIAL_GATE:
						setBuildGate(row, col, true);
						break;
					case SPECIAL_LAND_BRIDGE:
						setBuildLandBridge(row, col, true);
						break;
					//case SPECIAL_FOREST:
					//	setBuildForest(row, col, true);
					//	break;
				}
			}
	else {
		for (long row = 0; row < height; row++)
			for (long col = 0; col < width; col++) {
				setTerrain(row, col, MC_GRASS_TYPE);
				setOverlay(row, col, 0);
				setGate(row, col, false);
				if ((row == 0) || (col == 0) || (row == (height - 1)) || (col == (width - 1)))
					setPassable(row, col, false);
				else
					setPassable(row, col, true);
				for (long i = 0; i < NUM_MOVE_LEVELS; i++)
					GameMap->setPathlock(i, row, col, false);
				setPreserved(row, col, false);
				setMine(row, col, 0);
			}
	}

	return(0);
}

//---------------------------------------------------------------------------

void MissionMap::setPassable (long row, long col, char* footPrint, bool passable) {

	/* FootPrint Data format:
			r1, c1, l1, r2, c2, l2, r3, c3, l3, ...
			where r1 and c1 are offsets from row,col and l1 is the runlength of
			the passable/impassable block of cells. This function keeps reading in the
			(r, c, l) sets of data until (-1, -1, -1) is read in. Obviously, the data
			is a block of chars, so all values must be -127 < x < 127. This should
			allow foot prints big enuff for all buildings we have.
	*/

	char* data = footPrint;
	long r = row + *data++;
	long c = col + *data++;
	long len = *data++;
	while (len != -1) {
		MapCellPtr cell = &map[r * width + c];
		for (long i = 0; i < len; i++) {
			cell->setPassable(passable);
			cell++;
		}
		r = row + *data++;
		c = col + *data++;
		len = *data++;
	}
}

//---------------------------------------------------------------------------

void MissionMap::spreadState (long cellRow, long cellCol, long radius) {

	//---------------------------------------------------------------------------
	// This does a simple depth-first spread from the center. The way it's coded,
	// it essentially grows a box from the center cell.
	if (inMapBounds(cellRow, cellCol, height, width) && (radius > 0)) {
		//-------------------------------------------------------------------
		// If we're placing moving objects for this frame only, we'll want to
		// preserve the cell states of this tile...
		if (!getPreserved(cellRow, cellCol))
			setPreserved(cellRow, cellCol, true);
		map[cellRow * width + cellCol].setPassable(false);
		for (long dir = 0; dir < NUM_DIRECTIONS; dir++) {
			long adjR = cellRow;
			long adjC = cellCol;
			calcAdjNode(adjR, adjC, dir);
			spreadState(adjR, adjC, radius - 1);
		}
	}
}

//---------------------------------------------------------------------------

long MissionMap::placeObject (Stuff::Vector3D position, float radius) {

	int cellRow, cellCol;
	land->worldToCell(position, cellRow, cellCol);

	//---------------------------------------------------------
	// Now, set the cell states based upon the object's size...
	float cellRadius = (radius / (Terrain::worldUnitsPerCell  * metersPerWorldUnit));
	if ((cellRadius > 0.5) && (cellRadius < 1.0))
		cellRadius = 1.0;
	spreadState(cellRow, cellCol, (long)cellRadius);
	return(NO_ERR);
}

//---------------------------------------------------------------------------

void MissionMap::placeTerrainObject(long objectClass,
									 long objectTypeID,
									 long cellRow,
									 long cellCol,
									 __int64 footPrint,
									 bool blocksLineOfFire,
									 long mineType) {

#if 0

	Assert(objectClass != -1, 0, " MissionMap.placeTerrainObject: bad ObjectClass ");

	long maskTemplateLo = footPrint;
	long maskTemplateHi = footPrint >> 32;
	
	long posTileR = cellRow / MAPCELL_DIM;
	long posTileC = cellCol / MAPCELL_DIM;

	long tileOriginR = posTileR;
	long tileOriginC = posTileC;

	long startCellR = cellRow % MAPCELL_DIM;
	long startCellC = cellCol % MAPCELL_DIM;
	
	long cellR = startCellR - 4;
	long cellC = startCellC - 4;

	while (cellR < 0) {
		posTileR--;
		cellR += MAPCELL_DIM;
	}
	
	while (cellC < 0) {
		posTileC--;
		cellC += MAPCELL_DIM;
	}
	
	if (objectClass == GATE)
		blocksLineOfFire = false;

	long startCol = posTileC;
	long firstCellC = cellC;
	//------------------------------------------------------------------
	// We now use the objectClass to do this correctly.
	if ((objectClass == BUILDING) ||
		(objectClass == TREEBUILDING) ||
		(objectClass == GATE) ||
		(objectClass == TURRET) ||
		(objectClass == TERRAINOBJECT)) {
		if (maskTemplateLo || maskTemplateHi) {
			//---------------------------------------------------------------------
			// We also need to set the BUILDING flag in the scenario map to indicate
			// a building exists in this tile...
			//map[tileOriginR * width + tileOriginC].setBuildingHere(true);
			switch (objectTypeID) {
				case 644:
				case 647:
				case 690:
				case 691:
					map[tileOriginR * width + tileOriginC].setOverlayType(OVERLAY_GATE_CLAN_EW_CLOSED);
					break;
				case 645:
				case 646:
				case 693:
				case 692:
					map[tileOriginR * width + tileOriginC].setOverlayType(OVERLAY_GATE_CLAN_NS_CLOSED);
					break;
			}
				
			long bits = 0;
				
			for (long totalCelRSet = 0;totalCelRSet<8;totalCelRSet++) {
				for (long totalCelCSet = 0;totalCelCSet<8;totalCelCSet++) {
					long bitMask = 1 << bits;
					long maskTemplate = maskTemplateLo;
					
					if (bits >= 32) {
						bitMask = 1 << (bits-32);
						maskTemplate = maskTemplateHi;
					}
					
					if ((bitMask & maskTemplate) == bitMask) {
						map[posTileR * width + posTileC].setCellPassable(cellR, cellC, 0);
						if (blocksLineOfFire)
							map[posTileR * width + posTileC].setCellLOS(cellR, cellC, 0);
					}
						
					cellC++;
					if (cellC == MAPCELL_DIM) {
						cellC = 0;
						posTileC++;
					}
						
					bits++;
				}
					
				posTileC = startCol;
				cellC = firstCellC;
				cellR++;
				if (cellR == MAPCELL_DIM) {
					cellR = 0;
					posTileR++;
				}
			}
		}
		}
	else if (objectClass == MINE) {

		//-----------------------------------------------------------
		// We need to know what cell is in the center of this MINE
		// We then mark the tile as MINED!!!!
		// We base it on the MineTypeNumber stored in the Mine Class.
		if (mineType == 3)		//Force into only one slots instead of three
			mineType = 2;
		if (mineType == 1)		//Force into only one slots instead of three
			mineType = 2;
		if (mineType == 13)		//Force into only one slots instead of three
			mineType = 12;
		if (mineType == 11)		//Force into only one slots instead of three
			mineType = 12;
		if (mineType > 10) // 10-07-98 HKG hack!  Mines are off by a tile in each direction
			map[(tileOriginR) * width + tileOriginC].setInnerSphereMine(mineType - 10);
		else
			map[(tileOriginR)* width + tileOriginC].setClanMine(mineType);
	}
#endif
}	

//---------------------------------------------------------------------------

long MissionMap::getOverlayWeight (long cellR, long cellC, long moverOverlayWeightClass, long moverRelation) {

	//long overlay = getOverlay(cellR, cellC);
/*
	if (overlay) {
		long* overlayWeightTable = &OverlayWeightTable[moverOverlayWeightClass * NUM_OVERLAY_TYPES * MAPCELL_DIM * MAPCELL_DIM];
		if ((overlayType >= OVERLAY_GATE_CLAN_NS_OPEN) && (overlayType <= OVERLAY_GATE_IS_EW_CLOSED)) {
			overlayType = GateOverlayTable[moverRelation][overlayType - OVERLAY_GATE_CLAN_NS_OPEN];
			if (overlayType == -1)
				return(COST_BLOCKED * 2);
			else {
				long overlayCostIndex = OverlayWeightIndex[overlayType] + cellR * MAPCELL_DIM + cellC;
				return(overlayWeightTable[overlayCostIndex]);
			}
			}
		else {
			long overlayCostIndex = OverlayWeightIndex[overlayType] + cellR * MAPCELL_DIM + cellC;
			return(overlayWeightTable[overlayCostIndex]);
		}
	}
*/
  return(0);
}

//---------------------------------------------------------------------------

void MissionMap::print (char* fileName) {

	File* debugFile = new File;
	debugFile->create(fileName);
	
	for (long cellR = 0; cellR < height; cellR++) {
		char outString[1024];
		outString[0] = '\0';
		for (long cellC = 0; cellC < width; cellC++) {
			if (!map[cellR * width + cellC].getPassable())
				strcat(outString, "X");
			else
				strcat(outString, ".");
		}
		strcat(outString, "\n");
		debugFile->writeString(outString);
	}
	debugFile->writeString("\n");

	debugFile->close();
	delete debugFile;
	debugFile = NULL;
}

//---------------------------------------------------------------------------

void MissionMap::destroy (void) {

	if (map) {
		systemHeap->Free(map);
		map = NULL;
	}
}

//***************************************************************************
// MOVE PATH class
//***************************************************************************

void* MovePath::operator new (size_t ourSize) {

	void* result;
	result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void MovePath::operator delete (void* us) {

	systemHeap->Free(us);
}	

//---------------------------------------------------------------------------

long MovePath::init (long numberOfSteps) {

	numSteps = numStepsWhenNotPaused = numberOfSteps;
	static long maxNumberOfSteps = 0;
	if (numberOfSteps > maxNumberOfSteps) {
		maxNumberOfSteps = numberOfSteps;
		return(maxNumberOfSteps);
	}
	for (long i = 0; i < MAX_STEPS_PER_MOVEPATH; i++) {
		stepList[i].distanceToGoal = 0.0;
		stepList[i].destination.x = 0.0;
		stepList[i].destination.y = 0.0;
		stepList[i].destination.z = 0.0;
		stepList[i].direction = 0;
	}
	return(-1);
}

//---------------------------------------------------------------------------

void MovePath::clear (void) {

	init();
}

//---------------------------------------------------------------------------

void MovePath::destroy (void) {

	numSteps = 0;
}

//---------------------------------------------------------------------------

float MovePath::getDistanceLeft (Stuff::Vector3D position, long stepNumber) {

	if (stepNumber == -1)
		stepNumber = curStep;
	float distance = distance_from(position,stepList[stepNumber].destination);
	distance *= metersPerWorldUnit;
	distance += stepList[stepNumber].distanceToGoal;
	return(distance);
}

//---------------------------------------------------------------------------

void MovePath::lock (long level, long start, long range, bool setting) {

#ifdef _DEBUG
	if (level > 1)
		STOP(("MovePath.lock: bad level %d", level));
#endif

	if (start == -1)
		start = curStep;
	long lastStep = start + range;
	if (lastStep >= numStepsWhenNotPaused)
		lastStep = numStepsWhenNotPaused;
	for (long i = start; i < lastStep; i++)
		GameMap->setPathlock(level, stepList[i].cell[0], stepList[i].cell[1], setting);
}

//---------------------------------------------------------------------------

bool MovePath::isLocked (long level, long start, long range, bool* reachedEnd) {

#ifdef _DEBUG
	if (level > 1)
		STOP(("MovePath.isLocked: bad moveLevel %d", level));
#endif

	if (start == -1)
		start = curStep;
	long lastStep = start + range;
	if (reachedEnd)
		*reachedEnd = false;
	if (lastStep >= numStepsWhenNotPaused) {
		if (reachedEnd)
			*reachedEnd = true;
		lastStep = numStepsWhenNotPaused;
	}
	for (long i = start; i < lastStep; i++)
		if (GameMap->getPathlock(level, stepList[i].cell[0], stepList[i].cell[1]))
			return(true);
	return(false);
}

//---------------------------------------------------------------------------

bool MovePath::isBlocked (long start, long range, bool* reachedEnd) {

	if (start == -1)
		start = curStep;
	long lastStep = start + range;
	if (reachedEnd)
		*reachedEnd = false;
	if (lastStep >= numStepsWhenNotPaused) {
		if (reachedEnd)
			*reachedEnd = true;
		lastStep = numStepsWhenNotPaused;
	}
	for (long i = start; i < lastStep; i++)
		if (!GameMap->getPassable(stepList[i].cell[0], stepList[i].cell[1]))
			return(true);
	return(false);
}

//---------------------------------------------------------------------------

long MovePath::crossesBridge (long start, long range) {

	if (start == -1)
		start = curStep;
	long lastStep = start + range;
	if (lastStep >= numStepsWhenNotPaused)
		lastStep = numStepsWhenNotPaused;
	//We need to redo this code when bridges are in cause they are objects now!!
#if 0
	for (long i = start; i < lastStep; i++) {
		long overlayType = GameMap->getOverlay(stepList[i].cell[0], stepList[i].cell[1]);
		if (OverlayIsBridge[overlayType])
			return(GlobalMoveMap[0]->calcArea(stepList[i].cell[0], stepList[i].cell[1]));
	}
#endif
	return(-1);
}

//---------------------------------------------------------------------------

long MovePath::crossesCell (long start, long range, long cellR, long cellC) {

	if (start == -1)
		start = curStep;
	long lastStep = start + range;
	if (lastStep >= numStepsWhenNotPaused)
		lastStep = numStepsWhenNotPaused;
	for (long i = start; i < lastStep; i++) {
		if ((cellR == stepList[i].cell[0]) && (cellC == stepList[i].cell[1]))
			return(i);
	}
	return(-1);
}

//---------------------------------------------------------------------------

long MovePath::crossesClosedClanGate (long start, long range) {

	return(-1);
}

//---------------------------------------------------------------------------

long MovePath::crossesClosedISGate (long start, long range) {

	return(-1);
}

//---------------------------------------------------------------------------

long MovePath::crossesClosedGate (long start, long range) {

	if (start == -1)
		start = curStep;
	long lastStep = start + range;
	if (lastStep >= numStepsWhenNotPaused)
		lastStep = numStepsWhenNotPaused;
	//Probably should have redone this code cause gates aren't overlays anymore?
#if 0
	for (long i = start; i < lastStep; i++) {
		long overlayType = GameMap->getOverlay(stepList[i].cell[0], stepList[i].cell[1]);
		if (OverlayIsClosedGate[overlayType])
			return(i);
	}
#endif
	return(-1);
}

//**********************************************************************************
// GLOBAL MAP class
//**********************************************************************************

void* GlobalMap::operator new (size_t ourSize) {

	void* result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void GlobalMap::operator delete (void* us) {

	systemHeap->Free(us);
}	

//------------------------------------------------------------------------------------------

void GlobalMap::init (long w, long h) {

	width = w;
	height = h;
	areaMap = (short*)systemHeap->Malloc(sizeof(short) * w * h);
	gosASSERT(areaMap != NULL);
	for (long r = 0; r < height; r++)
		for (long c = 0; c < width; c++)
			areaMap[r * width + c] = -1;

	gosASSERT(((width % SECTOR_DIM) == 0) && ((height % SECTOR_DIM) == 0));
	sectorWidth = w / SECTOR_DIM;
	sectorHeight = w / SECTOR_DIM;

	numAreas = 0;
	areas = NULL;
	numDoors = 0;
	doors = NULL;
	doorBuildList = NULL;
	pathExistsTable = NULL;
#ifdef USE_PATH_COST_TABLE
	pathCostTable = NULL;
#endif
	blank = false;
}

//------------------------------------------------------------------------------------------

#define	GLOBALMAP_VERSION_NUMBER	0x022523	// My 51st birthday:)

long GlobalMap::init (PacketFilePtr packetFile, long whichPacket) {

	long startPacket = whichPacket;

	unsigned int version = 0;
	long result = packetFile->readPacket(whichPacket++, (unsigned char*)&version);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read version packet ");
	bool badVersion = false;
	if (version != GLOBALMAP_VERSION_NUMBER)
		badVersion = true;

	result = packetFile->readPacket(whichPacket++, (unsigned char*)&height);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read height packet ");
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&width);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read width packet ");
	int sectorDim; // this is no longer used...
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&sectorDim);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read sectorDim packet ");
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&sectorHeight);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read sectorHeight packet ");
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&sectorWidth);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read sectorWidth packet ");
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&numAreas);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read numAreas packet ");
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&numDoors);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read numDoors packet ");
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&numDoorInfos);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read numDoorInfos packet ");
	result = packetFile->readPacket(whichPacket++, (unsigned char*)&numDoorLinks);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read numDoorLinks packet ");

	if (badVersion) {
		//---------------------------------------------------------
		// Bad version map, so return number of packets but bail...
		badLoad = true;
#ifdef USE_PATH_COST_TABLE
		pathCostTable = NULL;
		return(14 + numDoorInfos + (numDoors + NUM_DOOR_OFFSETS) * 2);
#else
		return(13 + numDoorInfos + (numDoors + NUM_DOOR_OFFSETS) * 2);
#endif
	}

	areaMap = (short*)systemHeap->Malloc(sizeof(short) * height * width);
	gosASSERT(areaMap != NULL);
	result = packetFile->readPacket(whichPacket++, (unsigned char*)areaMap);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read areaMap packet ");

	areas = (GlobalMapAreaPtr)systemHeap->Malloc(sizeof(GlobalMapArea) * numAreas);
	areas_cellsCovered = (short**)systemHeap->Malloc(sizeof(short*) * numAreas);
	areas_doors = (DoorInfoPtr*)systemHeap->Malloc(sizeof(DoorInfoPtr) * numAreas);
	gosASSERT(areas != NULL);
	gosASSERT(areas_cellsCovered != NULL);
	gosASSERT(areas_doors != NULL);
	result = packetFile->readPacket(whichPacket++, (unsigned char*)areas);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read areas packet ");

	doorInfos = (DoorInfoPtr)systemHeap->Malloc(sizeof(DoorInfo) * numDoorInfos);
	gosASSERT(doorInfos != NULL);
	long curDoorInfo = 0;
	for (long i = 0; i < numAreas; i++)
		if (areas[i].numDoors) {
			result = packetFile->readPacket(whichPacket++, (unsigned char*)&doorInfos[curDoorInfo]);
			if (result == 0)
				Fatal(result, " GlobalMap.init: unable to read doorInfos packet ");
			curDoorInfo += areas[i].numDoors;
		}
	Assert(numDoorInfos == curDoorInfo, 0, " GlobalMap.init: bad doorInfo count ");

	//------------------------------------------------------------
	// Set up the areas so they point to the correct door infos...
	curDoorInfo = 0;
	for (long curArea = 0; curArea < numAreas; curArea++) {
		//areas[curArea].doors = &doorInfos[curDoorInfo];
		areas_doors[curArea] = &doorInfos[curDoorInfo];
		curDoorInfo += areas[curArea].numDoors;
	}

	doors = (GlobalMapDoorPtr)systemHeap->Malloc(sizeof(GlobalMapDoor) * (numDoors + NUM_DOOR_OFFSETS));
	doors_links = (DoorInfoLinksPtr*)systemHeap->Malloc(sizeof(DoorInfoLinksPtr) * (numDoors + NUM_DOOR_OFFSETS));
	gosASSERT(doors != NULL);
	result = packetFile->readPacket(whichPacket++, (unsigned char*)doors);
	if (result == 0)
		Fatal(result, " GlobalMap.init: unable to read doors packet ");

	//--------------
	// Door Links...
	doorLinks = (DoorLinkPtr)systemHeap->Malloc(sizeof(DoorLink) * numDoorLinks);
	gosASSERT(doorLinks != NULL);
	long numLinksRead = 0;
	for (int i = 0; i < (numDoors + NUM_DOOR_OFFSETS); i++) {
		long numLinks = doors[i].numLinks[0] + NUM_EXTRA_DOOR_LINKS;
		gosASSERT(numLinks >= 2);
		result = packetFile->readPacket(whichPacket++, (unsigned char*)&doorLinks[numLinksRead]);
		if (result <= 0)
			Fatal(result, " GlobalMap.init: Unable to write doorLinks packet ");
		doors_links[i][0] = &doorLinks[numLinksRead];
		numLinksRead += numLinks;

		numLinks = doors[i].numLinks[1] + NUM_EXTRA_DOOR_LINKS;
		gosASSERT(numLinks >= 2);
		result = packetFile->readPacket(whichPacket++, (unsigned char*)&doorLinks[numLinksRead]);
		if (result <= 0)
			Fatal(result, " GlobalMap.init: Unable to write doorLinks packet ");
		doors_links[i][1] = &doorLinks[numLinksRead];
		numLinksRead += numLinks;
	}
	Assert(numLinksRead == numDoorLinks, 0, " GlobalMap.init: Incorrect Links count ");

	numOffMapAreas = 0;
	for (int i = 0; i < numAreas; i++)
		if (areas[i].offMap) {
			offMapAreas[numOffMapAreas++] = i;
			closeArea(i);
			}
		else
			openArea(i);

	//--------------------------
	// Create pathExistsTable...
	pathExistsTable = (unsigned char*)systemHeap->Malloc(numAreas * (numAreas / 4 + 1));
	if (!pathExistsTable)
		STOP(("GlobalMap.init: unable to malloc pathExistsTable"));
	clearPathExistsTable();

	if (logEnabled && !blank) {
		char s[256];
		for (long i = 0; i < numDoors; i++) {
			sprintf(s, "door %05d, %s(%d), areas %d & %d", i, doors[i].open ? "opened" : "CLOSED", doors[i].teamID, doors[i].area[0], doors[i].area[1]);
			log->write(s);
			for (long side = 0; side < 2; side++) {
				sprintf(s, "     side %d", side);
				log->write(s); 
				for (long j = 0; j < doors[i].numLinks[side]; j++) {
					sprintf(s, "          link %03d, to door %05d, cost %d",
						j,
						doors_links[i][side][j].doorIndex,
						doors_links[i][side][j].cost);
					log->write(s); 
				}
			}
		}
		log->write(" "); 

		for (int i = 0; i < numAreas; i++) {
			sprintf(s, "area %05d, %s(%d)", i, areas[i].open ? "opened" : "CLOSED", areas[i].teamID);
			log->write(s); 
			if (areas[i].ownerWID > 0) {
				sprintf(s, "     ownerWID is %d", areas[i].ownerWID);
				log->write(s);
			}
			sprintf(s, "     %s", typeString[areas[i].type]);
			log->write(s);
			for (long d = 0; d < areas[i].numDoors; d++) {
                const DoorInfo& door = areas_doors[i][d];
				sprintf(s, "     door %03d is %d (%d:%d & %d) ",
					d, door.doorIndex, door.doorSide,
					doors[door.doorIndex].area[0], doors[door.doorIndex].area[1]);
				log->write(s);
			}
			
		}
		log->write(" ");
	}

	return(whichPacket - startPacket);
}

//------------------------------------------------------------------------------------------

long GlobalMap::write (PacketFilePtr packetFile, long whichPacket) {

	//---------------------------------------------------------
	// If mapFile NULL, return the number of packets we need...

	long numDrInfos = 0;
	for (long i = 0; i < numAreas; i++)
		if (areas[i].numDoors)
			numDrInfos++;
#ifdef USE_PATH_COST_TABLE
		long numPackets = 14 + numDrInfos + (numDoors + NUM_DOOR_OFFSETS) * 2;
#else
		long numPackets = 13 + numDrInfos + (numDoors + NUM_DOOR_OFFSETS) * 2;
#endif

	if (!packetFile)
		return(numPackets);

	unsigned long version = GLOBALMAP_VERSION_NUMBER;
	long result = packetFile->writePacket(whichPacket++, version);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write version packet ");
	result = packetFile->writePacket(whichPacket++, height);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write height packet ");
	result = packetFile->writePacket(whichPacket++, width);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write width packet ");
	long sectorDim = SECTOR_DIM;
	result = packetFile->writePacket(whichPacket++, sectorDim);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write sectorDim packet ");
	result = packetFile->writePacket(whichPacket++, sectorHeight);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write sectorHeight packet ");
	result = packetFile->writePacket(whichPacket++, sectorWidth);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write sectorWidth packet ");
	result = packetFile->writePacket(whichPacket++, numAreas);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write numAreas packet ");
	result = packetFile->writePacket(whichPacket++, numDoors);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write numDoors packet ");
	result = packetFile->writePacket(whichPacket++, numDoorInfos);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write numDoorInfos packet ");
	result = packetFile->writePacket(whichPacket++, numDoorLinks);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write numDoorLinks packet ");

	result = packetFile->writePacket(whichPacket++, (unsigned char*)areaMap, sizeof(short) * height * width);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write areaMap packet ");

	//----------------------------------
	// Write out the areas themselves...
	DoorInfoPtr doorInfoSave[MAX_GLOBALMAP_AREAS];
	for (int i = 0; i < numAreas; i++) {
		//-------------------------
		// clear it for the save...
		//doorInfoSave[i] = areas[i].doors;
		//areas[i].doors = NULL;
		doorInfoSave[i] = areas_doors[i];
		areas_doors[i] = NULL;

		areas[i].ownerWID = 0;
		areas[i].teamID = -1;
		//areas[i].offMap = false;
		areas[i].open = true;
	}
	result = packetFile->writePacket(whichPacket++, (unsigned char*)areas, sizeof(GlobalMapArea) * numAreas);
	for (int i = 0; i < numAreas; i++) {
		//---------------------
		// restore the doors...
		areas_doors[i] = doorInfoSave[i];
	}
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write areas packet ");

	//------------------------------------
	// Now, write out all the DoorInfos...
	long maxDoors = 0;
	for (long d = 0; d < numDoors; d++) {
		for (long s = 0; s < 2; s++) {
			long areaNumDoors = areas[doors[d].area[s]].numDoors;
			if (areaNumDoors > maxDoors)
				maxDoors = areaNumDoors;
		}
	}
	doors[numDoors + DOOR_OFFSET_START].numLinks[0] = maxDoors;
	doors[numDoors + DOOR_OFFSET_START].numLinks[1] = 0;
	doors[numDoors + DOOR_OFFSET_GOAL].numLinks[0] = maxDoors;
	doors[numDoors + DOOR_OFFSET_GOAL].numLinks[1] = 0;

	long numDoorInfosWritten = 0;
	for (int i = 0; i < numAreas; i++)
		if (areas[i].numDoors) {
			long packetSize = sizeof(DoorInfo) * areas[i].numDoors;
			//result = packetFile->writePacket(whichPacket++, (unsigned char*)areas[i].doors, packetSize);
			result = packetFile->writePacket(whichPacket++, (unsigned char*)areas_doors[i], packetSize);
			if (result <= 0)
				Fatal(result, " GlobalMap.write: Unable to write doorInfos packet ");
			numDoorInfosWritten += areas[i].numDoors;
		}
	Assert(numDoorInfosWritten == numDoorInfos, 0, " GlobalMap.write: Error in writing DoorInfos ");

	DoorLinkPtr doorLinkSave[MAX_GLOBALMAP_DOORS][2];
	for (int i = 0; i < numDoors + NUM_DOOR_OFFSETS; i++) {
		//-------------------------
		// clear it for the save...
		doorLinkSave[i][0] = doors_links[i][0];
		doorLinkSave[i][1] = doors_links[i][1];
		//doors[i].cost = 0;
		doors_links[i][0] = NULL;
		doors_links[i][1] = NULL;
		doors[i].parent = 0;
		doors[i].fromAreaIndex = 0;
		doors[i].flags = 0;
		doors[i].g = 0;
		doors[i].hPrime = 0;
		doors[i].fPrime = 0;
	}
	result = packetFile->writePacket(whichPacket++, (unsigned char*)doors, sizeof(GlobalMapDoor) * (numDoors + NUM_DOOR_OFFSETS));
	for (int i = 0; i < (numDoors + NUM_DOOR_OFFSETS); i++) {
		//---------------------
		// restore the links...
		doors_links[i][0] = doorLinkSave[i][0];
		doors_links[i][1] = doorLinkSave[i][1];
	}
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write doors packet ");

	//------------------------------------
	// Now, write out all the DoorLinks...
	
	long numberL = 0;
	for (int i = 0; i < (numDoors + NUM_DOOR_OFFSETS); i++) {
		long numLinks = doors[i].numLinks[0] + NUM_EXTRA_DOOR_LINKS;
		gosASSERT(numLinks >= 2);
		result = packetFile->writePacket(whichPacket++, (unsigned char*)doors_links[i][0], sizeof(DoorLink) * numLinks);
		if (result <= 0)
			Fatal(result, " GlobalMap.write: Unable to write doorLinks packet ");
		numberL += numLinks;

		numLinks = doors[i].numLinks[1] + NUM_EXTRA_DOOR_LINKS;
		gosASSERT(numLinks >= 2);
		result = packetFile->writePacket(whichPacket++, (unsigned char*)doors_links[i][1], sizeof(DoorLink) * numLinks);
		if (result <= 0)
			Fatal(result, " GlobalMap.write: Unable to write doorLinks packet ");
		numberL += numLinks;
	}

	if (numberL != numDoorLinks)
		PAUSE(("Number of DoorLinks Calculated does not match numDoorLinks"));

#ifdef USE_PATH_COST_TABLE
	if (blank)
		calcPathCostTable();
	else
		initPathCostTable();
	result = packetFile->writePacket(whichPacket++, (unsigned char*)pathCostTable, numAreas * numAreas);
	if (result <= 0)
		Fatal(result, " GlobalMap.write: Unable to write doors packet ");
#endif
	return(numPackets);
}

//------------------------------------------------------------------------------------------

void GlobalMap::calcSpecialAreas (MissionMapCellInfo* mapData) {

	bool areaLogged[64000];
	memset(areaLogged, false, 64000);
	long biggestID = -1;
	long smallestID = 99999;
	for (long r = 0; r < height; r++)
		for (long c = 0; c < width; c++) {
			if (mapData[r * width + c].specialType != SPECIAL_NONE) {
				long ID = mapData[r * width + c].specialID;
				if (ID > biggestID)
					biggestID = ID;
				if (ID < smallestID)
					smallestID = ID;
				switch (mapData[r * width + c].specialType) {
					case SPECIAL_GATE:
						if (!areaLogged[ID]) {
							specialAreas[ID].type = SPECIAL_GATE;
							specialAreas[ID].numSubAreas = 0;
							specialAreas[ID].numCells = 0;
							areaLogged[ID] = true;
						}
						areaMap[r * width + c] = ID;
						GameMap->setBuildGate(r, c, true);
						specialAreas[ID].cells[specialAreas[ID].numCells][0] = r;
						specialAreas[ID].cells[specialAreas[ID].numCells][1] = c;
						specialAreas[ID].numCells++;
						break;
					case SPECIAL_WALL:
						if (!areaLogged[ID]) {
							specialAreas[ID].type = SPECIAL_WALL;
							specialAreas[ID].numSubAreas = 0;
							specialAreas[ID].numCells = 0;
							areaLogged[ID] = true;
						}
						areaMap[r * width + c] = ID;
						GameMap->setBuildWall(r, c, true);
						specialAreas[ID].cells[specialAreas[ID].numCells][0] = r;
						specialAreas[ID].cells[specialAreas[ID].numCells][1] = c;
						specialAreas[ID].numCells++;
						break;
					case SPECIAL_LAND_BRIDGE:
						if (!areaLogged[ID]) {
							specialAreas[ID].type = SPECIAL_LAND_BRIDGE;
							specialAreas[ID].numSubAreas = 0;
							specialAreas[ID].numCells = 0;
							areaLogged[ID] = true;
						}
						areaMap[r * width + c] = ID;
						GameMap->setBuildLandBridge(r, c, true);
						specialAreas[ID].cells[specialAreas[ID].numCells][0] = r;
						specialAreas[ID].cells[specialAreas[ID].numCells][1] = c;
						specialAreas[ID].numCells++;
						break;
					//case SPECIAL_FOREST:
					//	if (!areaLogged[ID]) {
					//		specialAreas[ID].type = SPECIAL_FOREST;
					//		specialAreas[ID].numSubAreas = 0;
					//		specialAreas[ID].numCells = 0;
					//		areaLogged[ID] = true;
					//	}
					//	areaMap[r * width + c] = ID;
					//	GameMap->setBuildForest(r, c, true);
					//	break;
				}
			}
		}
	if (biggestID > -1)
		numSpecialAreas = (biggestID + 1);
	for (long i = 0; i < numSpecialAreas; i++) {
		specialAreas[i].numSubAreas = 0;
		//if (!areaLogged[i])
		//	STOP(("GlobalMap.calcSpecialAreas: unlogged special area %d", i));
	}
}

//------------------------------------------------------------------------------------------

long GlobalMap::setTempArea (long tileR, long tileC, long cost) {

	long numStartDoors = 0;
	for (long dir = 0; dir < 4; dir++) {
		long adjR = tileR + adjTile[dir][0];
		long adjC = tileC + adjTile[dir][1];
		long adjArea = calcArea(adjR, adjC);
		if (adjArea > -1) {
// SET TEMP DOORS HERE...
//			areas[numAreas].doors[numStartDoors].r = tileR;
//			areas[numAreas].doors[numStartDoors].c = tileC;
//			areas[numAreas].doors[numStartDoors].length = 1;
//			areas[numAreas].doors[numStartDoors].direction = dir;
//			areas[numAreas].doors[numStartDoors].area = adjArea;
//			numStartDoors++;
		}
	}
	areas[numAreas].numDoors = numStartDoors;

	//----------------------------
	// Now, setup the temp area...
	areas[numAreas].sectorR = tileR / SECTOR_DIM;
	areas[numAreas].sectorC = tileC / SECTOR_DIM;
	//areas[numAreas].cost = 1;
	return(numAreas);
}

//------------------------------------------------------------------------------------------

bool GlobalMap::fillSpecialArea (long row, long col, long area, long specialID) {

	//----------------------------------------------------------------------
	// This is used to fill any dynamic or "special" areas. Currently, these
	// are: wall, gate and forest.
	areaMap[row * width + col] = area;
	GameMap->setBuildNotSet(row, col, false);

	for (long dir = 0; dir < 4; dir ++) {
		long adjR = row + adjTile[dir][0];
		long adjC = col + adjTile[dir][1];
		if ((adjR >= minRow) && (adjR < maxRow) && (adjC >= minCol) && (adjC < maxCol))
			if (GameMap->getBuildNotSet(adjR, adjC) && (areaMap[adjR * width + adjC] == specialID))
				fillSpecialArea(adjR, adjC, area, specialID);
	}
	
	return(true);
}

//------------------------------------------------------------------------------------------

bool GlobalMap::fillArea (long row, long col, long area, bool offMap) {

	if ((row < minRow) || (row >= maxRow) || (col < minCol) || (col >= maxCol))
		return(false);

	//---------------------------------------------------------------------
	// If we hit a special (wall/gate/forest) tile, politely stop expanding
	// this area into it...
	if (!blank)
		if (GameMap->getBuildSpecial(row, col))
			return(false);

	//-----------------------------------------------------------------------------
	// If we're processing an offMap area, then stop if we hit a non-offMap cell...
	if (offMap) {
		if (!GameMap->getOffMap(row, col))
			return(false);
		}
	else {
		if (GameMap->getOffMap(row, col))
			return(false);
	}
	
	GameMap->setBuildNotSet(row, col, false);

	if (!blank) {
		if (!hover && GameMap->getDeepWater(row, col)) {
			//GameMap->setPassable(row, col, false);
			areaMap[row * width + col] = -2;
			return(false);
		}

		if (!offMap && !GameMap->getPassable(row, col)) {
			areaMap[row * width + col] = -2;
			return(false);
		}
	}

	gosASSERT(area != -1);
	areaMap[row * width + col] = area;
	for (long dir = 0; dir < 4; dir ++) {
		long adjR = row + adjTile[dir][0];
		long adjC = col + adjTile[dir][1];
		if ((adjR >= minRow) && (adjR < maxRow) && (adjC >= minCol) && (adjC < maxCol))
			if (GameMap->getBuildNotSet(adjR, adjC))
				fillArea(adjR, adjC, area, offMap);
	}
	
	return(true);
}

//------------------------------------------------------------------------------------------

void GlobalMap::calcSectorAreas (long sectorR, long sectorC) {

	minRow = sectorR * SECTOR_DIM;
	maxRow = minRow + SECTOR_DIM;
	minCol = sectorC * SECTOR_DIM;
	maxCol = minCol + SECTOR_DIM;

	if (blank) {
		for (long r = minRow; r < maxRow; r++)
			for (long c = minCol; c < maxCol; c++)
				if (GameMap->getBuildNotSet(r, c)) {
					if (fillArea(r, c, numAreas, GameMap->getOffMap(r, c)))
						numAreas++;
				}
		}
	else {
		for (long r = minRow; r < maxRow; r++)
			for (long c = minCol; c < maxCol; c++)
				if (GameMap->getBuildNotSet(r, c)) {
					if (GameMap->getBuildSpecial(r, c)) {
						long specialHere = areaMap[r * width + c];
						gosASSERT(specialHere > -1);
						//-------------------------------------------------
						// List this new area as part of the region covered
						// by this gate or wall or forest...
						if (specialAreas[specialHere].numSubAreas == MAX_SPECIAL_SUB_AREAS)
							Fatal(MAX_SPECIAL_SUB_AREAS, " GlobalMap.calcSectorAreas: too many special subareas ");
						specialAreas[specialHere].subAreas[specialAreas[specialHere].numSubAreas] = numAreas;
						specialAreas[specialHere].numSubAreas++;
						//-------------------
						// Now, fill it in...
						fillSpecialArea(r, c, numAreas++, specialHere);
						}
					else if (fillArea(r, c, numAreas, GameMap->getOffMap(r, c)))
						numAreas++;
				}
	}
}

//------------------------------------------------------------------------------------------

void GlobalMap::calcAreas (void) {

	//----------------------------------------------------------------------
	// Large area maps use -1 and -2 to indicate blocked area. CalcArea will
	// always return -1 for blocked-area cells.

	//-----------------------------------------
	// First, process each sector's area map...

	for (long r = 0; r < sectorHeight; r++)
		for (long c = 0; c < sectorWidth; c++)
			calcSectorAreas(r, c);

	//-----------------------------------------------------------------------
	// NOTE: We allocate one extra area--this is used by the calcPath routine
	// as a "scratch" area in some cases...
	gosASSERT(numAreas <= MAX_GLOBALMAP_AREAS);
	areas = (GlobalMapAreaPtr)systemHeap->Malloc(sizeof(GlobalMapArea) * (numAreas + 1));
	gosASSERT(areas != NULL);
	for (long i = 0; i < (numAreas + 1); i++) {
		areas[i].sectorR = 0;
		areas[i].sectorC = 0;
		areas[i].type = AREA_TYPE_NORMAL;
		areas[i].numDoors = 0;
		areas[i].doors_Legacy32bitPtr = 0;
		areas_doors[i] = NULL;
		areas[i].ownerWID = 0;
		areas[i].teamID = -1;
		areas[i].offMap = false;
		areas[i].open = true;
		areas[i].cellsCovered_Legacy32bitPtr = 0;
		areas_cellsCovered[i] = NULL;
	}

	//-----------------------------------------
	// Set each area's sector row and column...
	for (long sectorR = 0; sectorR < sectorHeight; sectorR++)
		for (long sectorC = 0; sectorC < sectorWidth; sectorC++) {
			minRow = sectorR * SECTOR_DIM;
			maxRow = minRow + SECTOR_DIM;
			minCol = sectorC * SECTOR_DIM;
			maxCol = minCol + SECTOR_DIM;
			for (long r = minRow; r < maxRow; r++)
				for (long c = minCol; c < maxCol; c++) {
					long curArea = areaMap[r * width + c];
					if (curArea > -1) {
						areas[curArea].sectorR = sectorR;
						areas[curArea].sectorC = sectorC;
					}
				}
		}
}

//------------------------------------------------------------------------------------------

void GlobalMap::calcCellsCovered (void) {

/*	for (long r = 0; r < height; r++)
		for (long c = 0; c < width; c++) {
			long area = areaMap[r * width + c];
			if (areas[area].type
		}
*/
}

//------------------------------------------------------------------------------------------

void GlobalMap::calcSpecialTypes (void) {

//systemHeap->walkHeap();
	for (long i = 0; i < numSpecialAreas; i++) {
		if (specialAreas[i].type == SPECIAL_WALL)
			for (long j = 0; j < specialAreas[i].numSubAreas; j++)
				areas[specialAreas[i].subAreas[j]].type = AREA_TYPE_WALL;
		else if (specialAreas[i].type == SPECIAL_GATE)
			for (long j = 0; j < specialAreas[i].numSubAreas; j++)
				areas[specialAreas[i].subAreas[j]].type = AREA_TYPE_GATE;
		else if (specialAreas[i].type == SPECIAL_LAND_BRIDGE)
			for (long j = 0; j < specialAreas[i].numSubAreas; j++)
				areas[specialAreas[i].subAreas[j]].type = AREA_TYPE_LAND_BRIDGE;
		else if (specialAreas[i].type == SPECIAL_FOREST)
			for (long j = 0; j < specialAreas[i].numSubAreas; j++)
				areas[specialAreas[i].subAreas[j]].type = AREA_TYPE_FOREST;
	}

	for (long r = 0; r < height; r++)
		for (long c = 0; c < width; c++)
			if (GameMap->getOffMap(r, c)) {
				long area = calcArea(r, c);
				if (area > -1)
					areas[area].offMap = true;
			}
//systemHeap->walkHeap();
}

//------------------------------------------------------------------------------------------

void GlobalMap::beginDoorProcessing (void) {

//systemHeap->walkHeap();
	doorBuildList = (GlobalMapDoorPtr)systemHeap->Malloc(sizeof(GlobalMapDoor) * MAX_GLOBALMAP_DOORS);
	for (long i = 0; i < MAX_GLOBALMAP_DOORS; i++) {
		doorBuildList[i].row = -1;
		doorBuildList[i].col = -1;
		doorBuildList[i].length = -1;
		doorBuildList[i].open = true;
		doorBuildList[i].teamID = -1;
		doorBuildList[i].numLinks[0] = 0;
		doorBuildList[i].numLinks[1] = 0;
	}
	gosASSERT(doorBuildList != NULL);
}

//------------------------------------------------------------------------------------------

void GlobalMap::addDoor (long area1, long area2, long row, long col, long length, long dir) {

	/*
	if (area1 > area2) {
		long savedArea = area1;
		area1 = area2;
		area2 = savedArea;
		dir = (dir + 2) % 4;
	}
	*/

	//-------------------------------------------------------
	// First, make sure the door isn't already in the list...
	bool newDoor = true;
	for (long curDoorIndex = 0; curDoorIndex < numDoors; curDoorIndex++) {
		GlobalMapDoorPtr curDoor = &doorBuildList[curDoorIndex];
		if ((curDoor->row == row) && (curDoor->col == col))
			if ((curDoor->length == length) && (curDoor->direction[0] == dir))
				newDoor = false;
	}

	if (newDoor) {
		/*
		if (area1 > area2) {
			//-----------------------------
			// Smaller area number first...
			long savedArea1 = area1;
			area1 = area2;
			area2 = savedArea1;
			dir = (dir + 2) % 4;
		}
		*/
		doorBuildList[numDoors].row = row;
		doorBuildList[numDoors].col = col;
		doorBuildList[numDoors].length = length;
		doorBuildList[numDoors].open = true;
		doorBuildList[numDoors].teamID = -1;
		doorBuildList[numDoors].area[0] = area1;
		doorBuildList[numDoors].areaCost[0] = 1;
		doorBuildList[numDoors].direction[0] = dir;
		doorBuildList[numDoors].area[1] = area2;
		doorBuildList[numDoors].areaCost[1] = 1;
		doorBuildList[numDoors].direction[1] = (dir + 2) % 4;
		numDoors++;
		if (numDoors >= MAX_GLOBALMAP_DOORS)
			Fatal(numDoors, " Too many Global Doors ");
	}

	if (numDoors >= MAX_GLOBALMAP_DOORS)
		STOP(("Too Many Doors %d", numDoors));
}

//------------------------------------------------------------------------------------------

void GlobalMap::endDoorProcessing (void) {

//	systemHeap->walkHeap();
	if (doorBuildList) {
		//-----------------------------------------------------------------------
		// First, save the door list. Note that we make 2 extra doors, to be used
		// by the pathfinder...
		doors = (GlobalMapDoorPtr)systemHeap->Malloc(sizeof(GlobalMapDoor) * (numDoors + NUM_DOOR_OFFSETS));
		memcpy(doors, doorBuildList, sizeof(GlobalMapDoor) * (numDoors + NUM_DOOR_OFFSETS));
		//----------------------------
		// Free the temp build list...
		systemHeap->Free(doorBuildList);
		doorBuildList = NULL;
	}
//	systemHeap->walkHeap();
}

//------------------------------------------------------------------------------------------

long GlobalMap::numAreaDoors (long area) {

	long doorCount = 0;
	for (long doorIndex = 0; doorIndex < numDoors; doorIndex++)
		if ((doors[doorIndex].area[0] == area) || (doors[doorIndex].area[1] == area))
			doorCount++;
	if (doorCount > 255)
		Fatal(doorCount, " Too many area doors ");
	return(doorCount);
}

//------------------------------------------------------------------------------------------

void GlobalMap::getAreaDoors (long area, DoorInfoPtr doorList) {

	long doorCount = 0;
	for (long doorIndex = 0; doorIndex < numDoors; doorIndex++)
		if ((doors[doorIndex].area[0] == area) || (doors[doorIndex].area[1] == area)) {
			doorList[doorCount].doorIndex = doorIndex;
			long doorSide = (doors[doorIndex].area[1] == area);
			doorList[doorCount].doorSide = doorSide;
			doorCount++;
		}
}

//------------------------------------------------------------------------------------------

#define	MAX_DOORS_PER_SECTOR	500		// This should be WAY more than we'll ever need...

long maxNumDoors = 0;

void GlobalMap::calcGlobalDoors (void) {

	short doorMap[SECTOR_DIM][SECTOR_DIM];


	beginDoorProcessing();

	for (long sectorR = 0; sectorR < sectorHeight; sectorR++) {
		for (long sectorC = 0; sectorC < sectorWidth; sectorC++) {
			for (long dir = 1; dir < 3; dir++) {
				for (long x = 0; x < SECTOR_DIM; x++)
					for (long y = 0; y < SECTOR_DIM; y++)
						doorMap[x][y] = -1;

				minRow = sectorR * SECTOR_DIM;
				maxRow = minRow + SECTOR_DIM;
				minCol = sectorC * SECTOR_DIM;
				maxCol = minCol + SECTOR_DIM;

				//-------------------------------------------
				// First, find all doors in this direction...
				for (long r = minRow; r < maxRow; r++)
					for (long c = minCol; c < maxCol; c++) {
						//-------------------------------------------------------
						// First, check each cell in the sector to see if
						// it's a "door" cell (is open and adjacent to an
						// open cell in another area in the current direction)...
						long curArea = areaMap[r * width + c];
						if (curArea > -1) {
							//---------------------------------------------------------
							// The doorMap tracks, for each tile, what area is adjacent
							// to it thru a door...
							long adjR = r + adjTile[dir][0];
							long adjC = c + adjTile[dir][1];
							if ((adjR >= 0) && (adjR < height) && (adjC >= 0) && (adjC < width)) {
								long adjArea = areaMap[adjR * width + adjC];
								if ((adjArea > -1) && (curArea != adjArea)) {
									AreaType curAreaType = areas[curArea].type;
									AreaType adjAreaType = areas[adjArea].type;
									bool validDoor = false;
									if ((curAreaType == AREA_TYPE_NORMAL) && (adjAreaType == AREA_TYPE_NORMAL))
										validDoor = true;
									if (validDoor)
										doorMap[r - minRow][c - minCol] = adjArea;
								}
							}
						}
					}
		
				//--------------------------------------------------------------------------
				// Now, process the doors and add them to the global door list. The
				// direction we're currently processing will dictate how to
				// make our sweep across the sector map. So, we have a chunk a code for each
				// direction (THIS MUST BE MODIFIED IF WE INCLUDE DIAGONAL DIRECTIONS WHEN
				// MOVING ON THE GLOBAL MAP)...
				if (dir == 1) {
					for (long c = maxCol - 1; c >= minCol; c--) {
						long r = minRow;
						while (r < maxRow) {
							long adjArea = doorMap[r - minRow][c - minCol];
							if (adjArea > -1) {
								long curArea = areaMap[r * width + c];
								//-----------------------------------
								// We have a door. Calc its length...
								long length = 0;
								while ((r < maxRow) && (areaMap[r * width + c] == curArea) && (doorMap[r - minRow][c - minCol] == adjArea)) {
									length++;
									r++;
								}
								//---------------------------------------
								// Now, add it to the global door list...
								addDoor(curArea, adjArea, r - length, c, length, dir);
								}
							else
								r++;
						}
					}
					}
				else {
					for (long r = maxRow - 1; r >= minRow; r--) {
						long c = minCol;
						while (c < maxCol) {
							long adjArea = doorMap[r - minRow][c - minCol];
							if (adjArea > -1) {
								long curArea = areaMap[r * width + c];
								//-----------------------------------
								// We have a door. Calc its length...
								long length = 0;
								while ((c < maxCol) && (areaMap[r * width + c] == curArea) && (doorMap[r - minRow][c - minCol] == adjArea)) {
									length++;
									c++;
								}
								//-----------------------------------------
								// Now, add it to the sector's door list...
								addDoor(curArea, adjArea, r, c - length, length, dir);
								}
							else
								c++;
						}
					}
				}
			}
		}
	}

	endDoorProcessing();
}

//------------------------------------------------------------------------------------------

void GlobalMap::calcAreaDoors (void) {

	numDoorInfos = 0;
	for (long area = 0; area < numAreas; area++) {
		areas[area].numDoors = numAreaDoors(area);
		numDoorInfos += areas[area].numDoors;
		if (areas[area].numDoors) {
			areas_doors[area] = (DoorInfoPtr)systemHeap->Malloc(sizeof(DoorInfo) * areas[area].numDoors);
			getAreaDoors(area, areas_doors[area]);
			}
		else
			areas_doors[area] = NULL;
	}
}

//------------------------------------------------------------------------------------------

long GlobalMap::calcLinkCost (long startDoor, long thruArea, long goalDoor) {

	if ((doors[startDoor].area[0] != thruArea) && (doors[startDoor].area[1] != thruArea))
		return(-1);

	long startSide = (doors[startDoor].area[1] == thruArea);
	long startRow = doors[startDoor].row;
	long startCol = doors[startDoor].col;
	if (doors[startDoor].direction[0] == 1) {
		//-------------------------------------------------------------------------------
		// Door goes from west to east
		// For now, use the midpoint of the door for purposes of calcing the cost between
		// doors...
		startRow += (doors[startDoor].length / 2);
		startCol += startSide;
		}
	else {
		//------------------------------
		// Door goes from north to south
		// For now, use the midpoint of the door for purposes of calcing the cost between
		// doors...
		startRow += startSide;
		startCol += (doors[startDoor].length / 2);
	}

	if ((doors[goalDoor].area[0] != thruArea) && (doors[goalDoor].area[1] != thruArea))
		return(-2);

	long goalSide = (doors[goalDoor].area[1] == thruArea);
	long goalRow = doors[goalDoor].row;
	long goalCol = doors[goalDoor].col;
	if (doors[goalDoor].direction[0] == 1) {
		//-------------------------------------------------------------------
		// Door goes from west to east. For now, use the midpoint of the door
		// for purposes of calcing the cost between doors...
		goalRow += (doors[goalDoor].length / 2);
		goalCol += goalSide;
		}
	else {
		//------------------------------
		// Door goes from north to south
		goalRow += goalSide;
		goalCol += (doors[goalDoor].length / 2);
	}

	Stuff::Vector3D goalWorldPos;
	land->cellToWorld(goalRow, goalCol, goalWorldPos);
	//goalWorldPos.x = (float)goalCol * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
	//goalWorldPos.y = (Terrain::worldUnitsMapSide / 2) - ((float)goalCellR * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
	//goalWorldPos.z = (float)0; // How do we get the elevation for this point? Do we care?

	long cost = 9999;
	
	if (!PathFindMap[SECTOR_PATHMAP]) {
		PathFindMap[SECTOR_PATHMAP] = new MoveMap;
		PathFindMap[SECTOR_PATHMAP]->init(30, 30);
	}
	if (!PathFindMap[SIMPLE_PATHMAP]) {
		PathFindMap[SIMPLE_PATHMAP] = new MoveMap;
		PathFindMap[SIMPLE_PATHMAP]->init(SimpleMovePathRange * 2 + 1, SimpleMovePathRange * 2 + 1);
	}

	//-------------------------------------------------------------------------
	// Let's make sure the bridge tiles are NOT blocked here (since, during the
	// game, they won't be, unless destroyed, in which case we won't care since
	// they're their own area)...
	MovePath newPath;
	if (blank) {
		newPath.numSteps = 10;
		newPath.cost = 10;
		}
	else if ((areas[thruArea].type == AREA_TYPE_WALL) || (areas[thruArea].type == AREA_TYPE_GATE) || (areas[thruArea].type == AREA_TYPE_FOREST)) {
		//---------------------------------------------
		// These costs are for when the area is open...
		newPath.numSteps = 10;
		newPath.cost = 10;
		}
	else {
		ClearBridgeTiles = true;
		long mapULr = areas[thruArea].sectorR * SECTOR_DIM;
		long mapULc = areas[thruArea].sectorC * SECTOR_DIM;
		long moveParams = MOVEPARAM_NONE;
		if (hover)
			moveParams |= (MOVEPARAM_WATER_SHALLOW + MOVEPARAM_WATER_DEEP);
		else if (!blank)
			moveParams |= MOVEPARAM_WATER_SHALLOW;
#if USE_SEPARATE_WATER_MAPS
		// To REALLY fix the separate water maps, we should do this for vehicles and mechs separately (and vehicles
		// don't use shallow). But, if nothing else, this fixes the bug where we weren't passing in the moveParams.
		PathFindMap[SECTOR_PATHMAP]->setUp(mapULr,
					   mapULc,
					   SECTOR_DIM,
					   SECTOR_DIM,
					   hover ? 1 : 0,
					   NULL,
					   startRow,
					   startCol,
					   goalWorldPos,
					   goalRow - mapULr,
					   goalCol - mapULc,
					   10,
					   0,
					   8,
					   moveParams);
#else
		PathFindMap[SECTOR_PATHMAP]->setUp(mapULr,
					   mapULc,
					   SECTOR_DIM,
					   SECTOR_DIM,
					   hover ? 1 : 0,
					   NULL,
					   startRow,
					   startCol,
					   goalWorldPos,
					   goalRow - mapULr,
					   goalCol - mapULc,
					   10,
					   0,
					   8,
					   MOVEPARAM_NONE);
#endif
		long goalCell[2];
		PathFindMap[SECTOR_PATHMAP]->calcPath(&newPath, NULL, goalCell);
		ClearBridgeTiles = false;
	}

	//-------------------------------------------------------------------------
	// If there is no path, find out why! In theory, this should never occur...
	if (newPath.numSteps == 0) {
		/*
		File* pathDebugFile = new File;
		pathDebugFile->create("movemap1.dbg");
		PathFindMap->writeDebug(pathDebugFile);
		pathDebugFile->close();
		delete pathDebugFile;
		pathDebugFile = NULL;
		*/
		//--------------------------------------------------------------------------
		// If the cost is 1, then our start and goal cells are the same. Thus, there
		// is a path, it's just REALLY short :)
		if (newPath.cost == 1)
			return(1);
		else
			return(9999);
		}
	else
		cost = newPath.getCost();
//#endif
	gosASSERT(cost != 0);
	return(cost);
}

//------------------------------------------------------------------------------------------

void GlobalMap::changeAreaLinkCost (long area, long cost) {

	GlobalMapAreaPtr thruArea = &areas[area];
    DoorInfoPtr thruDoorInfo = areas_doors[area];
	long numDoors = thruArea->numDoors;
	for (long i = 0; i < numDoors; i++) {
		//GlobalMapDoorPtr curDoor = &doors[thruArea->doors[i].doorIndex];
		//long doorSide = thruArea->doors[i].doorSide;
		GlobalMapDoorPtr curDoor = &doors[thruDoorInfo[i].doorIndex];
		DoorInfoLinksPtr& curDoor_links = doors_links[thruDoorInfo[i].doorIndex];
		long doorSide = thruDoorInfo[i].doorSide;
		for (long j = 0; j < curDoor->numLinks[doorSide]; j++)
			curDoor_links[doorSide][j].cost = cost;
	}
}

//------------------------------------------------------------------------------------------

void GlobalMap::calcDoorLinks (void) {

	numDoorLinks = 0;

	long maxDoors = 0;
	for (long d = 0; d < numDoors; d++) {
		GlobalMapDoorPtr thisDoor = &doors[d];
		DoorInfoLinksPtr& thisDoor_links = doors_links[d];
		for (long s = 0; s < 2; s++) {
			thisDoor->numLinks[s] = 0;
			thisDoor_links[s] = NULL;

			long area = thisDoor->area[s];
			long areaNumDoors = areas[area].numDoors;
			//if (areaNumDoors >= 0) {
				thisDoor->numLinks[s] = areaNumDoors - 1;
				//----------------------------------------------------------------
				// Allocate enough links for all links to this door plus a scratch
				// link used during path calc...
				thisDoor_links[s] = (DoorLinkPtr)systemHeap->Malloc(sizeof(DoorLink) * (thisDoor->numLinks[s] + NUM_EXTRA_DOOR_LINKS));
				numDoorLinks += (thisDoor->numLinks[s] + NUM_EXTRA_DOOR_LINKS);
				gosASSERT(thisDoor_links[s] != NULL);
				long linkIndex = 0;
				for (long areaDoor = 0; areaDoor < areaNumDoors; areaDoor++) {
					//long doorIndex = areas[area].doors[areaDoor].doorIndex;
					long doorIndex = areas_doors[area][areaDoor].doorIndex;
					GlobalMapDoorPtr curDoor = &doors[doorIndex];
					if (curDoor != thisDoor) {
						thisDoor_links[s][linkIndex].doorIndex = doorIndex;
						thisDoor_links[s][linkIndex].doorSide = (curDoor->area[1] == area);
						thisDoor_links[s][linkIndex].cost = calcLinkCost(d, area, doorIndex);
						thisDoor_links[s][linkIndex].openCost = thisDoor_links[s][linkIndex].cost;
						linkIndex++;
					}
				}
			//}
			if (areaNumDoors > maxDoors)
				maxDoors = areaNumDoors;
		}
	}

	//----------------------------------------
	// Now, set up the start and goal doors...
	doors[numDoors + DOOR_OFFSET_START].numLinks[0] = maxDoors;
	numDoorLinks += (doors[numDoors + DOOR_OFFSET_START].numLinks[0] + NUM_EXTRA_DOOR_LINKS);
	doors_links[numDoors + DOOR_OFFSET_START][0] = (DoorLinkPtr)systemHeap->Malloc(sizeof(DoorLink) * (doors[numDoors + DOOR_OFFSET_START].numLinks[0] + NUM_EXTRA_DOOR_LINKS));
	doors[numDoors + DOOR_OFFSET_START].numLinks[1] = 0;
	numDoorLinks += (doors[numDoors + DOOR_OFFSET_START].numLinks[1] + NUM_EXTRA_DOOR_LINKS);
	doors_links[numDoors + DOOR_OFFSET_START][1] = (DoorLinkPtr)systemHeap->Malloc(sizeof(DoorLink) * (doors[numDoors + DOOR_OFFSET_START].numLinks[1] + NUM_EXTRA_DOOR_LINKS));

	doors[numDoors + DOOR_OFFSET_GOAL].numLinks[0] = maxDoors;
	numDoorLinks += (doors[numDoors + DOOR_OFFSET_GOAL].numLinks[0] + NUM_EXTRA_DOOR_LINKS);
	doors_links[numDoors + DOOR_OFFSET_GOAL][0] = (DoorLinkPtr)systemHeap->Malloc(sizeof(DoorLink) * (doors[numDoors + DOOR_OFFSET_GOAL].numLinks[0] + NUM_EXTRA_DOOR_LINKS));
	doors[numDoors + DOOR_OFFSET_GOAL].numLinks[1] = 0;
	numDoorLinks += (doors[numDoors + DOOR_OFFSET_GOAL].numLinks[1] + NUM_EXTRA_DOOR_LINKS);
	doors_links[numDoors + DOOR_OFFSET_GOAL][1] = (DoorLinkPtr)systemHeap->Malloc(sizeof(DoorLink) * (doors[numDoors + DOOR_OFFSET_GOAL].numLinks[1] + NUM_EXTRA_DOOR_LINKS));

	long numberL = 0;
	for (long i = 0; i < (numDoors + NUM_DOOR_OFFSETS); i++) {
		long numLinks = doors[i].numLinks[0] + NUM_EXTRA_DOOR_LINKS;
		gosASSERT(numLinks >= 2);
		//result = packetFile->writePacket(whichPacket++, (unsigned char*)doors[i].links[0], sizeof(DoorLink) * numLinks);
		//if (result <= 0)
		//	Fatal(result, " GlobalMap.write: Unable to write doorLinks packet ");
		numberL += numLinks;

		numLinks = doors[i].numLinks[1] + NUM_EXTRA_DOOR_LINKS;
		gosASSERT(numLinks >= 2);
		//result = packetFile->writePacket(whichPacket++, (unsigned char*)doors[i].links[1], sizeof(DoorLink) * numLinks);
		//if (result <= 0)
		//	Fatal(result, " GlobalMap.write: Unable to write doorLinks packet ");
		numberL += numLinks;
	}
	Assert(numberL == numDoorLinks, 0, " HUH ");
}

//------------------------------------------------------------------------------------------

long GlobalMap::getPathCost (int startArea, int goalArea, bool withSpecialAreas, int& confidence, bool calcIt) {

	//------------------------------------------------------------------------
	// This could be cleaned up if we re-save this data, 0 being no path and 1
	// indicating startArea == goalArea.
	if (startArea < 0)
		return(0);
	if (goalArea < 0)
		return(0);
	if (!areas[startArea].open)
		return(0);
	if (!areas[goalArea].open)
		return(0);
	if (startArea == goalArea)
		return(1);

#if 1
	GlobalPathStep path[MAX_GLOBAL_PATH];
	if (withSpecialAreas)
		useClosedAreas = true;
	else
		useClosedAreas = false;
	long cost = calcPath(startArea, goalArea, path);
	useClosedAreas = false;
	confidence = GLOBAL_CONFIDENCE_GOOD;
	return(cost);
#else
	calcedPathCost = false;
	unsigned char data = pathCostTable[startArea * numAreas + goalArea];
	if (withSpecialAreas) {
		unsigned char cost = (data & 0x0C) >> 2;
		if (data & GLOBAL_FLAG_SPECIAL_IMPOSSIBLE) {
			cost = 0;
			confidence = GLOBAL_CONFIDENCE_GOOD;
			}
		else if (data & GLOBAL_FLAG_SPECIAL_CALC) {
			if (calcIt) {
				GlobalPathStep path[MAX_GLOBAL_PATH];
				useClosedAreas = true;
				calcPath(startArea, goalArea, path);
				useClosedAreas = false;
				cost = getPathCost(startArea, goalArea, true, confidence, false);
				calcedPathCost = true;
				confidence = GLOBAL_CONFIDENCE_GOOD;
				}
			else
				confidence = GLOBAL_CONFIDENCE_BAD;
			}
		else
			confidence = GLOBAL_CONFIDENCE_GOOD;
		return(cost);
		}
	else {
		unsigned char cost = (data & 0x03);
		if (data & GLOBAL_FLAG_SPECIAL_IMPOSSIBLE) {
			cost = 0;
			confidence = GLOBAL_CONFIDENCE_GOOD;
			}
		else if (data & GLOBAL_FLAG_NORMAL_CLOSES) {
			if (calcIt) {
				GlobalPathStep path[MAX_GLOBAL_PATH];
				calcPath(startArea, goalArea, path);
				cost = getPathCost(startArea, goalArea, false, confidence, false);
				calcedPathCost = true;
				confidence = GLOBAL_CONFIDENCE_GOOD;
				}
			else if (cost)
				confidence = GLOBAL_CONFIDENCE_BAD;
			else
				confidence = GLOBAL_CONFIDENCE_GOOD;
			}
		else if (data & GLOBAL_FLAG_NORMAL_OPENS) {
			if (calcIt) {
				GlobalPathStep path[MAX_GLOBAL_PATH];
				calcPath(startArea, goalArea, path);
				cost = getPathCost(startArea, goalArea, false, confidence, false);
				calcedPathCost = true;
				confidence = GLOBAL_CONFIDENCE_GOOD;
				}
			else
				confidence = GLOBAL_CONFIDENCE_AT_LEAST;
			}
		else
			confidence = GLOBAL_CONFIDENCE_GOOD;
		return(cost);
	}
	return(0);
#endif
}

//---------------------------------------------------------------------------

#if USE_PATH_COST_TABLE

void GlobalMap::setPathFlag (long startArea, long goalArea, unsigned char flag, bool set) {

	long index = startArea * numAreas + goalArea;
	pathCostTable[index] &= (flag ^ 0xFF);
	if (set)
		pathCostTable[index] |= flag;
}

//---------------------------------------------------------------------------

long GlobalMap::getPathFlag (long startArea, long goalArea, unsigned char flag) {

	return(pathCostTable[startArea * numAreas + goalArea] & flag);
}

//---------------------------------------------------------------------------

void GlobalMap::setPathCost (long startArea, long goalArea, bool withSpecialAreas, unsigned char cost) {

	if (cost > 0) {
		if (cost < 6) 
			cost = 1;
		else if (cost < 11)
			cost = 2;
		else
			cost = 3;
	}
	long index = startArea * numAreas + goalArea;
	if (withSpecialAreas) {
		pathCostTable[index] &= 0xF3;
		cost <<= 2;
		}
	else
		pathCostTable[index] &= 0xFC;
	pathCostTable[index] |= cost;
}

//------------------------------------------------------------------------------------------

void GlobalMap::initPathCostTable (void) {

	if (pathCostTable) {
		systemHeap->Free(pathCostTable);
		pathCostTable = NULL;
	}

	long oldNumAreas = numAreas;
//	if (numAreas > 600)
//		numAreas = 600;

	pathCostTable = (unsigned char*)systemHeap->Malloc(numAreas * numAreas);
	gosASSERT(pathCostTable != NULL);

	for (long startArea = 0; startArea < numAreas; startArea++)
		for (goalArea = 0; goalArea < numAreas; goalArea++) {
			setPathCost(startArea, goalArea, false, 0);
			setPathCost(startArea, goalArea, true, 0);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_IMPOSSIBLE, false);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_CALC, true);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_OPENS, true);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_CLOSES, true);
		}
	closes = false;
	opens = false;

	numAreas = oldNumAreas;
}

//------------------------------------------------------------------------------------------

void GlobalMap::resetPathCostTable (void) {

	if (!pathCostTable)
		return;

	if (!closes && !opens)
		return;

	for (long startArea = 0; startArea < numAreas; startArea++)
		for (goalArea = 0; goalArea < numAreas; goalArea++) {
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_OPENS, opens);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_CLOSES, closes);
		}
	closes = false;
	opens = false;

}

//------------------------------------------------------------------------------------------

void GlobalMap::calcPathCostTable (void) {

	if (pathCostTable) {
		systemHeap->Free(pathCostTable);
		pathCostTable = NULL;
	}
	pathCostTable = (unsigned char*)systemHeap->Malloc(numAreas * numAreas);
	gosASSERT(pathCostTable != NULL);

	for (long i = 0; i < numAreas; i++)
		openArea(i);

	GlobalPathStep globalPath[MAX_GLOBAL_PATH];
	for (long startArea = 0; startArea < numAreas; startArea++)
		for (goalArea = 0; goalArea < numAreas; goalArea++) {

			long numSteps = calcPath(startArea, goalArea, globalPath);
			//----------------------------------------------------------
			// For now, we'll just store the number of area steps in the
			// table. If no path (or start and goal are the same), we'll store 0.
			// 255 is reserved to mark the need to re-calc during the game...
			setPathCost(startArea, goalArea, true, numSteps);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_IMPOSSIBLE, false);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_CALC, true);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_OPENS, false);
			setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_CLOSES, false);
		}

	for (i = 0; i < numAreas; i++)
		if ((areas[i].type == AREA_TYPE_WALL) || (areas[i].type == AREA_TYPE_GATE) || (areas[i].type == AREA_TYPE_FOREST))
			openArea(i);

	for (startArea = 0; startArea < numAreas; startArea++)
		for (goalArea = 0; goalArea < numAreas; goalArea++) {

			long numSteps = calcPath(startArea, goalArea, globalPath);
			//----------------------------------------------------------
			// For now, we'll just store the number of area steps in the
			// table. If no path (or start and goal are the same), we'll store 0.
			// 255 is reserved to mark the need to re-calc during the game...
			setPathCost(startArea, goalArea, true, numSteps);
		}

	for (i = 0; i < numAreas; i++)
		if ((areas[i].type == AREA_TYPE_WALL) || (areas[i].type == AREA_TYPE_GATE) || (areas[i].type == AREA_TYPE_FOREST))
			closeArea(i);

	closes = false;
	opens = false;
}

#endif
//------------------------------------------------------------------------------------------

void GlobalMap::clearPathExistsTable (void) {

	long tableSize = numAreas * (numAreas / 4 + 1);
	memset(pathExistsTable, GLOBALPATH_EXISTS_UNKNOWN, tableSize);
}

//------------------------------------------------------------------------------------------

void GlobalMap::setPathExists (long fromArea, long toArea, unsigned char set) {

	if (!pathExistsTable)
		return;
	if (fromArea < 0)
		return;
	if (toArea < 0)
		return;

	long rowWidth = numAreas / 4 + 1;
	unsigned char* pathByte = pathExistsTable;
	pathByte += (rowWidth * fromArea + (toArea / 4));
	unsigned char pathShift = (toArea % 4) * 2;
	unsigned char pathBit = 0x03 << pathShift;
	*pathByte &= (pathBit ^ 0xFF);
	if (set)
		*pathByte |= (set << pathShift);
}

//------------------------------------------------------------------------------------------

unsigned char GlobalMap::getPathExists (long fromArea, long toArea) {

	if (!pathExistsTable)
		return(false);
	if (fromArea < 0)
		return(false);
	if (toArea < 0)
		return(false);

	long rowWidth = numAreas / 4 + 1;
	unsigned char* pathByte = pathExistsTable;
	pathByte += (rowWidth * fromArea + (toArea / 4));
	unsigned char pathShift = (toArea % 4) * 2;
	unsigned char pathBit = 0x03 << pathShift;
	return(*pathByte & pathBit);
}

//------------------------------------------------------------------------------------------

long GlobalMap::exitDirection (long doorIndex, long fromArea) {

	if (doors[doorIndex].area[0] == fromArea)
		return(doors[doorIndex].direction[0]);
	else if (doors[doorIndex].area[1] == fromArea)
		return(doors[doorIndex].direction[1]);
	return(-1);
}

//------------------------------------------------------------------------------------------

void GlobalMap::getDoorTiles (long area, long door, GlobalMapDoorPtr areaDoor) {

	
	*areaDoor = doors[areas_doors[area][door].doorIndex];
}

//------------------------------------------------------------------------------------------

bool GlobalMap::getAdjacentAreaCell (long area, long adjacentArea, long& cellRow, long& cellCol) {

	for (long i = 0; i < areas[area].numDoors; i++) {
		long doorIndex = areas_doors[area][i].doorIndex;
		long doorSide = areas_doors[area][i].doorSide;
		if (doors[doorIndex].area[doorSide % 1] == adjacentArea) {
			if ((doors[doorIndex].area[0] != area) && (doors[doorIndex].area[1] != area))
				STOP(("bad adjacent area door", 0));
			long goalSide = (doors[doorIndex].area[1] == adjacentArea);
			cellRow = doors[doorIndex].row;
			cellCol = doors[doorIndex].col;
			if (doors[doorIndex].direction[0] == 1) {
				//-------------------------------------------------------------------
				// Door goes from west to east. For now, use the midpoint of the door
				// for purposes of calcing the cost between doors...
				cellRow += (doors[doorIndex].length / 2);
				cellCol += goalSide;
				}
			else {
				//------------------------------
				// Door goes from north to south
				cellRow += goalSide;
				cellCol += (doors[doorIndex].length / 2);
			}
			return(true);
		}
	}
	return(false);
}

//------------------------------------------------------------------------------------------

Stuff::Vector3D GlobalMap::getDoorWorldPos (long area, long door, long* prevGoalCell) {

	long cellR, cellC;
	cellR = prevGoalCell[0];// + adjTile[areas[area].doors[door].direction][0];
	cellC = prevGoalCell[1];// += adjTile[areas[area].doors[door].direction][1];
	Stuff::Vector3D pos;
	pos.Zero();
	pos.x = (float)cellC * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
	pos.y = (Terrain::worldUnitsMapSide / 2) - ((float)cellR * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
	pos.z = (float)land->getTerrainElevation(pos); // How do we get the elevation for this point? Do we care?
	return(pos);
}

//------------------------------------------------------------------------------------------

long GlobalMap::build (MissionMapCellInfo* mapData) {

	#ifdef _DEBUG
	//systemHeap->walkHeap(false,false,"GlobalMap BAD HEAP1\n");
	#endif

	//--------------------------------
	// Note that h and w are in tiles.
	bool wasBlank = blank;
	init(GameMap->height, GameMap->width);
	blank = wasBlank;

	for (long r = 0; r < height; r++)
		for (long c = 0; c < width; c++)
			GameMap->setBuildNotSet(r, c, true);

	numAreas = 0;
	numSpecialAreas = 0;
	if (specialAreas) {
		systemHeap->Free(specialAreas);
		specialAreas = NULL;
	}
	if (mapData) {
		specialAreas = (GlobalSpecialAreaInfo*)systemHeap->Malloc(sizeof(GlobalSpecialAreaInfo) * MAX_SPECIAL_AREAS);
		if (specialAreas == NULL)
			Fatal(0, " GlobalMap.build: unable to malloc specialAreas ");
		calcSpecialAreas(mapData);
	}

	calcAreas();

	calcCellsCovered();

	calcGlobalDoors();

	calcSpecialTypes();

	calcAreaDoors();

	calcDoorLinks();

	//----------------------------------------------
	// Now, build the path tables for each sector...
/*	for (r = 0; r < tileHeight; r++)
		for (long c = 0; c < tileWidth; c++)
			calcSectorPaths(scenarioMap, r, c);
*/
	#ifdef _DEBUG
	//systemHeap->walkHeap(false,false,"GlobalMap BAD HEAP2\n");
	#endif

	//if (blank)
	//	calcPathCostTable();

	if (specialAreas) {
		systemHeap->Free(specialAreas);
		specialAreas = NULL;
	}

#ifdef DEBUG_GLOBALMAP_BUILD
	if (logEnabled && !blank) {
		char s[256];
		for (long i = 0; i < numDoors; i++) {
			sprintf(s, "door %05d, %s(%d), areas %d & %d", i, doors[i].open ? "opened" : "CLOSED", doors[i].teamID, doors[i].area[0], doors[i].area[1]);
			log->write(s);
			for (long side = 0; side < 2; side++) {
				sprintf(s, "     side %d", side);
				log->write(s); 
				for (long j = 0; j < doors[i].numLinks[side]; j++) {
					sprintf(s, "          link %03d, to door %05d, cost %d",
						j,
						doors[i].links[side][j].doorIndex,
						doors[i].links[side][j].cost);
					log->write(s); 
				}
			}
		}
		log->write(" "); 

		for (int i = 0; i < numAreas; i++) {
			sprintf(s, "area %05d, %s(%d)", i, areas[i].open ? "opened" : "CLOSED", areas[i].teamID);
			log->write(s); 
			if (areas[i].ownerWID > 0) {
				sprintf(s, "     ownerWID is %d", areas[i].ownerWID);
				log->write(s);
			}
			sprintf(s, "     %s", typeString[areas[i].type]);
			log->write(s);
			for (long d = 0; d < areas[i].numDoors; d++) {
				sprintf(s, "     door %03d is %d (%d:%d & %d) ",
					d, areas[i].doors[d].doorIndex, areas[i].doors[d].doorSide,
					doors[areas[i].doors[d].doorIndex].area[0], doors[areas[i].doors[d].doorIndex].area[1]);
				log->write(s);
			}
			
		}
		log->write(" ");
	}
#endif

	return(NO_ERR);	
}

//---------------------------------------------------------------------------

void GlobalMap::setStartDoor (long startArea) {

	GlobalMapDoorPtr startDoor = &doors[numDoors + DOOR_OFFSET_START];
	const DoorInfoLinksPtr& startDoor_links = doors_links[numDoors + DOOR_OFFSET_START];
	startDoor->row = 0;
	startDoor->col = 0;
	startDoor->length = 0;
	startDoor->open = true;
	startDoor->teamID = -1;
	startDoor->area[0] = startArea;
	startDoor->area[1] = startArea;
	startDoor->areaCost[0] = 1;
	startDoor->areaCost[1] = 1;
	startDoor->direction[0] = -1;
	startDoor->direction[1] = -1;
	startDoor->numLinks[0] = areas[startArea].numDoors;
	startDoor->numLinks[1] = 0;
	startDoor->fromAreaIndex = 1;

	for (long curLink = 0; curLink < startDoor->numLinks[0]; curLink++) {
		//---------------------------------------------
		// Point the goal "door" to its area's doors...
		const long doorIndex = areas_doors[startArea][curLink].doorIndex;
		const long doorSide = areas_doors[startArea][curLink].doorSide;
		GlobalMapDoorPtr curDoor = &doors[doorIndex];
		const DoorInfoLinksPtr& curDoor_links = doors_links[doorIndex];
		long costSum = 1;
		if (startCell[0] > -1) {
			if (startCell[0] > curDoor->row)
				costSum += (startCell[0] - curDoor->row);
			else	
				costSum += (curDoor->row - startCell[0]);

			if (startCell[1] > curDoor->col)
				costSum += (startCell[1] - curDoor->col);
			else
				costSum += (curDoor->col - startCell[1]);
		}

		//startDoor->links[0][curLink].doorIndex = doorIndex;
		//startDoor->links[0][curLink].doorSide = doorSide;
		//startDoor->links[0][curLink].cost = costSum;
		//startDoor->links[0][curLink].openCost = costSum;
        
		startDoor_links[0][curLink].doorIndex = doorIndex;
		startDoor_links[0][curLink].doorSide = doorSide;
		startDoor_links[0][curLink].cost = costSum;
		startDoor_links[0][curLink].openCost = costSum;
		
		//----------------------------------------------------
		// Make sure this area door points to the goal door...
		//curDoor->links[doorSide][curDoor->numLinks[doorSide]].doorIndex = numDoors + DOOR_OFFSET_START;
		//curDoor->links[doorSide][curDoor->numLinks[doorSide]].doorSide = 0;
		//curDoor->links[doorSide][curDoor->numLinks[doorSide]].cost = costSum;
		//curDoor->links[doorSide][curDoor->numLinks[doorSide]].openCost = costSum;

		curDoor_links[doorSide][curDoor->numLinks[doorSide]].doorIndex = numDoors + DOOR_OFFSET_START;
		curDoor_links[doorSide][curDoor->numLinks[doorSide]].doorSide = 0;
		curDoor_links[doorSide][curDoor->numLinks[doorSide]].cost = costSum;
		curDoor_links[doorSide][curDoor->numLinks[doorSide]].openCost = costSum;

		curDoor->numLinks[doorSide]++;
	}

}

//---------------------------------------------------------------------------

void GlobalMap::resetStartDoor (long startArea) {

	GlobalMapDoorPtr startDoor = &doors[numDoors + DOOR_OFFSET_START];
	
	for (long curLink = 0; curLink < startDoor->numLinks[0]; curLink++) {
		long doorSide = areas_doors[startArea][curLink].doorSide;
		doors[areas_doors[startArea][curLink].doorIndex].numLinks[doorSide]--;
	}
}

//---------------------------------------------------------------------------

void GlobalMap::setAreaTeamID (long area, char teamID) {

	//-----------------------------------------------------------------------
	// NOTE: This assumes there won't be adjacent areas with team alignments.
	// If there are, we should record two teamIDs for each door (to track
	// the teamID for each "side" of the door).
	if (area < 0)
		return;

	GlobalMapAreaPtr curArea = &areas[area];
	const DoorInfoPtr curArea_doors = areas_doors[area];
	curArea->teamID = teamID;
	for (long d = 0; d < curArea->numDoors; d++) {
		doors[curArea_doors[d].doorIndex].teamID = teamID;
	}
	//opens = true;
}

//---------------------------------------------------------------------------

void GlobalMap::setAreaOwnerWID (long area, long objWID) {

	if (area < 0)
		return;

	areas[area].ownerWID = objWID;
}

//---------------------------------------------------------------------------

void GlobalMap::setGoalDoor (long goalArea) {

	if ((goalArea < 0) || (goalArea >= numAreas)) {
		char errMsg[256];
		sprintf(errMsg, " GlobalMap.setGoalDoor: bad goalArea (%d of %d) ", goalArea, numAreas);
		gosASSERT((goalArea >= 0) || (goalArea < numAreas));
	}

	GlobalMapDoorPtr goalDoor = &doors[numDoors + DOOR_OFFSET_GOAL];
	DoorInfoLinksPtr& goalDoor_links = doors_links[numDoors + DOOR_OFFSET_GOAL];

	goalSector[0] = areas[goalArea].sectorR;
	goalSector[1] = areas[goalArea].sectorC;

	goalDoor->row = 0;
	goalDoor->col = 0;
	goalDoor->length = 0;
	goalDoor->open = true;
	goalDoor->teamID = -1;
	goalDoor->area[0] = goalArea;
	goalDoor->area[1] = goalArea;
	goalDoor->areaCost[0] = 1;
	goalDoor->areaCost[1] = 1;
	goalDoor->direction[0] = -1;
	goalDoor->direction[1] = -1;
	goalDoor->numLinks[0] = areas[goalArea].numDoors;
	goalDoor->numLinks[1] = 0;
	
	for (long curLink = 0; curLink < goalDoor->numLinks[0]; curLink++) {
		//---------------------------------------------
		// Point the goal "door" to its area's doors...
		long doorIndex = areas_doors[goalArea][curLink].doorIndex;
		long doorSide = areas_doors[goalArea][curLink].doorSide;
		gosASSERT((doorIndex >= 0) && (doorIndex < (numDoors + NUM_DOOR_OFFSETS)));
		GlobalMapDoorPtr curDoor = &doors[doorIndex];
	    DoorInfoLinksPtr& curDoor_links = doors_links[doorIndex];
		long costSum = 1;
		if (goalCell[0] > -1) {
			if (goalCell[0] > curDoor->row)
				costSum += (goalCell[0] - curDoor->row);
			else	
				costSum += (curDoor->row - goalCell[0]);

			if (goalCell[1] > curDoor->col)
				costSum += (goalCell[1] - curDoor->col);
			else
				costSum += (curDoor->col - goalCell[1]);
		}
		goalDoor_links[0][curLink].doorIndex = doorIndex;
		goalDoor_links[0][curLink].doorSide = doorSide;
		goalDoor_links[0][curLink].cost = costSum;
		goalDoor_links[0][curLink].openCost = costSum;
		
		//----------------------------------------------------
		// Make sure this area door points to the goal door...
		curDoor_links[doorSide][curDoor->numLinks[doorSide]].doorIndex = numDoors + DOOR_OFFSET_GOAL;
		curDoor_links[doorSide][curDoor->numLinks[doorSide]].doorSide = 0;
		curDoor_links[doorSide][curDoor->numLinks[doorSide]].cost = costSum;
		curDoor_links[doorSide][curDoor->numLinks[doorSide]].openCost = costSum;
		curDoor->numLinks[doorSide]++;
	}
}

//---------------------------------------------------------------------------

void GlobalMap::resetGoalDoor (long goalArea) {

	GlobalMapDoorPtr goalDoor = &doors[numDoors + DOOR_OFFSET_GOAL];
	
	for (long curLink = 0; curLink < goalDoor->numLinks[0]; curLink++) {
		long doorSide = areas_doors[goalArea][curLink].doorSide;
		doors[areas_doors[goalArea][curLink].doorIndex].numLinks[doorSide]--;
	}
}

//---------------------------------------------------------------------------

#define	AREA_ID_BASE		1000000

long GlobalMap::calcHPrime (long door) {

	gosASSERT((door > -1) && (door < (numDoors + NUM_DOOR_OFFSETS)));
	long sectorR = (areas[doors[door].area[0]].sectorR + areas[doors[door].area[1]].sectorR) / 2;
	long sectorC = (areas[doors[door].area[0]].sectorC + areas[doors[door].area[1]].sectorC) / 2;

	long sum = 0;
	if (sectorR > goalSector[0])
		sum += (sectorR - goalSector[0]);
	else
		sum += (goalSector[0] - sectorR);

	if (sectorC > goalSector[1])
		sum += (sectorC - goalSector[1]);
	else
		sum += (goalSector[1] - sectorC);

	return(sum);	
}

//---------------------------------------------------------------------------

inline void GlobalMap::propogateCost (long door, long cost, long fromAreaIndex, long g) {

	gosASSERT((door >= 0) && (door < (numDoors + NUM_DOOR_OFFSETS)) && ((fromAreaIndex == 0) || (fromAreaIndex == 1)));

	GlobalMapDoorPtr curMapDoor = &doors[door];
	const DoorInfoLinksPtr& curMapDoor_links = doors_links[door];
	if (curMapDoor->g > (g + cost)) {
		curMapDoor->g = g + cost;
		curMapDoor->fPrime = curMapDoor->g + curMapDoor->hPrime;
		if (curMapDoor->flags & MOVEFLAG_OPEN) {
			long openIndex = openList->find(door);
			if (!openIndex) {
				char s[128];
				sprintf(s, "GlobalMap.propogateCost: Cannot find globalmap door [%d, %d, %d, %d] for change\n", door, cost, fromAreaIndex, g);
				gosASSERT(openIndex != 0);
			}
			openList->change(door, curMapDoor->fPrime);
			}
		else {
			long toAreaIndex = 1 - fromAreaIndex;
			long numLinks = curMapDoor->numLinks[toAreaIndex];
			for (long curLink = 0; curLink < numLinks; curLink++) {
				long succDoor = curMapDoor_links[toAreaIndex][curLink].doorIndex;
				gosASSERT((succDoor >= 0) && (succDoor < numDoors + NUM_DOOR_OFFSETS));
				GlobalMapDoorPtr succMapDoor = &doors[succDoor];
				long succDoorCost = curMapDoor_links[toAreaIndex][curLink].cost;
				if (useClosedAreas) {
					if ((succMapDoor->teamID > -1) && (succMapDoor->teamID != moverTeamID))
						succDoorCost = 1000;
					}
				else {
					if ((succMapDoor->teamID > -1) && (succMapDoor->teamID != moverTeamID))
						succDoorCost = COST_BLOCKED;
					//--------------------------------------------------
					// Whether the door is even open is another issue...
					if (!succMapDoor->open)
						succDoorCost = COST_BLOCKED;
				}
				long succFromAreaIndex = (succMapDoor->area[1] == curMapDoor->area[toAreaIndex]);
				if ((succMapDoor->open || useClosedAreas) && (succDoorCost < COST_BLOCKED))
					if ((succMapDoor->hPrime != HPRIME_NOT_CALCED)/* && (succMapDoor->hPrime < MaxHPrime)*/) {
						if (door == succMapDoor->parent)
							propogateCost(succDoor, succDoorCost, toAreaIndex, curMapDoor->g);
						else if ((curMapDoor->g + succDoorCost) < succMapDoor->g) {
							succMapDoor->cost = succDoorCost;
							succMapDoor->parent = door;
							succMapDoor->fromAreaIndex = succFromAreaIndex;
							propogateCost(succDoor, succDoorCost, succFromAreaIndex, curMapDoor->g);
						}
					}
			}
		}
	}
}

//---------------------------------------------------------------------------
#ifdef TERRAINEDIT
#define MAX_GLOBAL_PATH		50
#endif

long GlobalMap::calcPath (long startArea,
						  long goalArea,
						  GlobalPathStepPtr path,
						  long startRow,
						  long startCol,
						  long goalRow,
						  long goalCol) {

	#ifdef _DEBUG
	//systemHeap->walkHeap(false,false,"GlobalMap:calc BAD HEAP1\n");
	#endif

	if ((startArea == -1) || (goalArea == -1))
		return(-1);

	startCell[0] = startRow;
	startCell[1] = startCol;
	goalCell[0] = goalRow;
	goalCell[1] = goalCol;

	if (logEnabled) {
		char s[50];
		sprintf(s, "     start = %d, goal = %d", startArea, goalArea);
		log->write(s);
		if (useClosedAreas)
			log->write("     USE CLOSED AREAS");
	}

	//----------------------------------------------------------------------
	// Door costs should be set here, if we want to modify them real-time...

	//-------------------------------------------------------------------------
	// We will not limit the search, as the global map should never be so big
	// that we'd WANT to limit the search. If so, time to change the sectorDim,
	// etc...

	//-----------------------------------------------
	// If we haven't already, create the OPEN list...
	if (!openList) {
		openList = new PriorityQueue;
		gosASSERT(openList != NULL);
		openList->init(5000);
	}

	//---------------------------------------------------------------
	// NOTE: The last 6 doors are reserved for use by the pathfinder:
	//			numDoors + 0 = startArea
	//			numDoors + 1 = goalArea
	//			numDoors + 2 thru 4 = doors for "blocked" start area
	const long startDoor = numDoors + DOOR_OFFSET_START;
	const long goalDoor = numDoors + DOOR_OFFSET_GOAL;

	//---------------------------------------------
	// Clear the doors and prep 'em for the calc...
	long initHPrime = ZeroHPrime ? 0 : HPRIME_NOT_CALCED;
	for (long d = 0; d < numDoors + NUM_DOOR_OFFSETS; d++) {
		doors[d].cost = 1;
		doors[d].parent = -1;
		doors[d].fromAreaIndex = -1;
		doors[d].flags = 0;
		doors[d].g = 0;
		doors[d].hPrime = initHPrime;
		doors[d].fPrime = 0;
	}
	
	setStartDoor(startArea);
	setGoalDoor(goalArea);

	if (areas[startArea].offMap)
		openArea(startArea);
	if (areas[goalArea].offMap)
		openArea(goalArea);

	if (!isGateOpenCallback || !isGateDisabledCallback)
		STOP(("Globalmap.calcPath: NULL gate callback"));

	for (long i = 0; i < numAreas; i++)
		if (areas[i].type == AREA_TYPE_GATE) {
			if ((areas[i].teamID == moverTeamID) || (areas[i].teamID == -1)) {
				if (areas[i].ownerWID > 0) {
					if (isGateDisabledCallback(areas[i].ownerWID))
						closeArea(i);
					else
						openArea(i);
					}
				else
					openArea(i);
				}
			else if (isGateOpenCallback(areas[i].ownerWID))
				openArea(i);
			else
				closeArea(i);
		}

	//-------------------------------------------------------------
	// Start with the area we're in, and process the possible doors
	// we can start thru...

	//-----------------------------------------------
	// Put the START vertex on the empty OPEN list...
	PQNode initialVertex;
	initialVertex.key = 0;
	initialVertex.id = startDoor;
	openList->clear();
#ifdef _DEBUG
	long insertErr = 
#endif
		openList->insert(initialVertex);
	gosASSERT(insertErr == NO_ERR);
	doors[startDoor].flags |= MOVEFLAG_OPEN;

	//******************
	//THROW THE STARTING LINKS ON THE QUEUE...
	//******************

	bool goalFound = false;

	while (!openList->isEmpty()) {
		//----------------------
		// Grab the best node...
		PQNode bestPQNode;
		openList->remove(bestPQNode);
		long bestDoor = bestPQNode.id;
		GlobalMapDoorPtr bestMapDoor = &doors[bestDoor];
		const DoorInfoLinksPtr& bestMapDoor_links = doors_links[bestDoor];
		bestMapDoor->flags &= (MOVEFLAG_OPEN ^ 0xFFFFFFFF);

		long bestDoorG = bestMapDoor->g;

		long fromAreaIndex = bestMapDoor->fromAreaIndex;

		//----------------------------
		// Now, close the best node...
		bestMapDoor->flags |= MOVEFLAG_CLOSED;
		
		//--------------------------
		// Have we found the goal...
		if (bestDoor == goalDoor) {
			goalFound = true;
			break;
		}

		//-------------------------------------------
		// Now, check the door links for this door...
		long toAreaIndex = 1 - fromAreaIndex;
		long thruArea = bestMapDoor->area[toAreaIndex];
		long numLinks = bestMapDoor->numLinks[toAreaIndex];
		
		if (logEnabled) {
			char s[50];
			sprintf(s, "     thruArea = %d, bestDoor = %d, numLinks = %d", thruArea, bestDoor, numLinks);
			log->write(s);
		}
		
		for (long curLink = 0; curLink < numLinks; curLink++) {
			//------------------------------------------------------
			// If we want to limit the doors thru which the path may
			// travel, we must check right here...
			
			//--------------------------
			// Now, process this door...
			long succDoor = bestMapDoor_links[toAreaIndex][curLink].doorIndex;
			gosASSERT((succDoor >= 0) && (succDoor < numDoors + NUM_DOOR_OFFSETS));
			GlobalMapDoorPtr succMapDoor = &doors[succDoor];

			if (logEnabled) {
				char s[50];
				sprintf(s, "          %02d) succDoor = %d", curLink, succDoor);
				log->write(s);
			}

			long succDoorCost = bestMapDoor_links[toAreaIndex][curLink].cost;
			//----------------------------------------------------------------------------
			// If this is an aligned door, make it more expensive for unfriendly movers...
			if (useClosedAreas) {
				if (succMapDoor->teamID > -1)
					if (TeamRelations[succMapDoor->teamID][moverTeamID] == RELATION_FRIENDLY)
						succDoorCost = 50;
				if (!succMapDoor->open)
					succDoorCost = 1000;
				}
			else {
				if ((succMapDoor->teamID > -1) && (succMapDoor->teamID != moverTeamID))
					succDoorCost = COST_BLOCKED;
				//--------------------------------------------------
				// Whether the door is even open is another issue...
				if (!succMapDoor->open)
					succDoorCost = COST_BLOCKED;
			}

			if (logEnabled) {
				char s[50];
				sprintf(s, "                  succDoorCost = %d", succDoorCost);
				log->write(s);
			}

			if (succDoorCost < COST_BLOCKED) {
				if (succMapDoor->hPrime == HPRIME_NOT_CALCED)
					succMapDoor->hPrime = calcHPrime(succDoor);

				//----------------------------------------------------
				// What's our cost to go from START to this SUCCESSOR?
				long succDoorG = bestDoorG + succDoorCost;

				long succFromAreaIndex = (succMapDoor->area[1] == thruArea);
				if (succMapDoor->flags & MOVEFLAG_OPEN) {
					//----------------------------------------------
					// This node is already in the OPEN queue to be
					// be processed. Let's check if we have a better
					// path thru this route...
					if (succDoorG < succMapDoor->g) {
						//----------------------------
						// This new path is cheaper...
						succMapDoor->cost = succDoorCost;
						succMapDoor->parent = bestDoor;
						succMapDoor->fromAreaIndex = succFromAreaIndex;
						succMapDoor->g = succDoorG;
						succMapDoor->fPrime = succDoorG + succMapDoor->hPrime;
						long openIndex = openList->find(succDoor);
						if (!openIndex) {
							char s[128];
							sprintf(s, "GlobalMap.calcPath: Cannot find globalmap door [%d, %d, %d, %d] for change\n", succDoor, curLink, succFromAreaIndex, succDoorCost);
#ifdef USE_OBJECTS
							DebugOpenList(s);
#endif
							gosASSERT(openIndex != 0);
						}
						openList->change(openIndex, succMapDoor->fPrime);
					}
					}
				else if (succMapDoor->flags & MOVEFLAG_CLOSED) {
					//-------------------------------------------------
					// This path may be better than this node's current
					// path. If so, we may have to propogate thru...
					if (succDoorG < succMapDoor->g) {
						//----------------------------------
						// This new path is cheaper. We
						// have to propogate the new cost...
						succMapDoor->cost = succDoorCost;
						succMapDoor->parent = bestDoor;
						succMapDoor->fromAreaIndex = succFromAreaIndex;
						propogateCost(succDoor, succDoorCost, succFromAreaIndex, bestDoorG);
					}
					}
				else {
					//-------------------------------------------------
					// This node is neither OPEN nor CLOSED, so toss it
					// into the OPEN list...
					succMapDoor->cost = succDoorCost;
					succMapDoor->parent = bestDoor;
					succMapDoor->fromAreaIndex = succFromAreaIndex;
					succMapDoor->g = succDoorG;
					succMapDoor->fPrime = succDoorG + succMapDoor->hPrime;
					PQNode succPQNode;
					succPQNode.key = succMapDoor->fPrime;
					succPQNode.id = succDoor;
#ifdef _DEBUG
					long insertErr = 
#endif
						openList->insert(succPQNode);
					gosASSERT(insertErr == NO_ERR);
					succMapDoor->flags |= MOVEFLAG_OPEN;
				}
			}
		}
	}

	resetStartDoor(startArea);
	resetGoalDoor(goalArea);

	if (areas[startArea].offMap)
		closeArea(startArea);
	if (areas[goalArea].offMap)
		closeArea(goalArea);

	if (goalFound) {
		//-------------------------------------------
		// First, let's count how long the path is...
		long curDoor = goalDoor;
		long numSteps = 1;
		while (curDoor != startDoor) {
			numSteps++;
			curDoor = doors[curDoor].parent;
		}
		
		//-----------------------------------------------------------------------
		// Number of doors to travel to, including the "goal door". We don't care
		// about the "start door" in the path...
		long numDoors = numSteps - 1;
		gosASSERT((numDoors < MAX_GLOBAL_PATH));

		//-----------------------------
		// Now, let's build the path...
		curDoor = goalDoor;
		long curPathDoor = numDoors;

		//--------------------------------
		// Do the doors leading to goal...
		long costToGoal = 0;
		while (curPathDoor > 0) {
			path[curPathDoor - 1].thruArea = doors[curDoor].area[doors[curDoor].fromAreaIndex];
			path[curPathDoor - 1].goalDoor = curDoor;
			path[curPathDoor - 1].costToGoal = costToGoal;
			costToGoal += doors[curDoor].cost;
			curDoor = doors[curDoor].parent;
			curPathDoor--;
		}

		#ifdef _DEBUG
		//systemHeap->walkHeap(false,false,"GlobalMap:calc BAD HEAP2\n");
		#endif

#ifdef USE_PATH_COST_TABLE
		if (pathCostTable) {
			setPathCost(startArea, goalArea, useClosedAreas, numDoors);
			if (useClosedAreas) {
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_IMPOSSIBLE, false);
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_CALC, false);
				}
			else {
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_OPENS, false);
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_CALC, false);
			}
		}
#endif
		if (logEnabled) {
			char s[50];
			sprintf(s, "     PATH FOUND: %d steps", numDoors);
			log->write(s);
			log->write(" ");
		}
		return(numDoors);
		}
	else {
#ifdef USE_PATH_COST_TABLE
		if (pathCostTable) {
			setPathCost(startArea, goalArea, useClosedAreas, 0);
			if (useClosedAreas) {
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_IMPOSSIBLE, true);
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_CALC, false);
				}
			else {
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_NORMAL_OPENS, false);
				setPathFlag(startArea, goalArea, GLOBAL_FLAG_SPECIAL_CALC, false);
			}
		}
#endif
		if (logEnabled)
			log->write("     NO PATH FOUND");
	}
	if (logEnabled)
		log->write(" ");
	return(0);
}

//------------------------------------------------------------------------------------------

long GlobalMap::calcPath (Stuff::Vector3D start, Stuff::Vector3D goal, GlobalPathStepPtr path) {

	int startR, startC;
	land->worldToCell(start, startR, startC);

	int goalR, goalC;
	land->worldToCell(goal, goalR, goalC);

	long numSteps = calcPath(calcArea(startR, startC), calcArea(goalR, goalC), path, startR, startC, goalR, goalC);
	return(numSteps);
}

//---------------------------------------------------------------------------

void GlobalMap::openDoor (long door) {

	doors[door].cost = 10;
	doors[door].open = true;
}

//---------------------------------------------------------------------------

void GlobalMap::closeDoor (long door) {

	doors[door].open = false;
}

//---------------------------------------------------------------------------

void GlobalMap::closeArea (long area) {

	if (area < 0)
		return;

	GlobalMapAreaPtr closedArea = &areas[area];
    const DoorInfoPtr& closedArea_doors = areas_doors[area];
	closedArea->open = false;
	for (long d = 0; d < closedArea->numDoors; d++)
		closeDoor(closedArea_doors[d].doorIndex);

	for (long i = 0; i < closedArea->numDoors; i++) {
		GlobalMapDoorPtr curDoor = &doors[closedArea_doors[i].doorIndex];
		DoorInfoLinksPtr& curDoor_links = doors_links[closedArea_doors[i].doorIndex];
		long doorSide = closedArea_doors[i].doorSide;
		for (long j = 0; j < curDoor->numLinks[doorSide]; j++)
			curDoor_links[doorSide][j].cost = 1000;
	}

	closes = true;
}

//---------------------------------------------------------------------------

void GlobalMap::openArea (long area) {

	if (area < 0)
		return;

	GlobalMapAreaPtr openedArea = &areas[area];
    const DoorInfoPtr& openedArea_doors = areas_doors[area];

	openedArea->open = true;
	for (long d = 0; d < openedArea->numDoors; d++) {
		long areaSide1 = doors[openedArea_doors[d].doorIndex].area[0];
		long areaSide2 = doors[openedArea_doors[d].doorIndex].area[1];
		if (!isClosedArea(areaSide1) && !isClosedArea(areaSide2))
			openDoor(openedArea_doors[d].doorIndex);
	}

	for (long i = 0; i < openedArea->numDoors; i++) {
		GlobalMapDoorPtr curDoor = &doors[openedArea_doors[i].doorIndex];
		DoorInfoLinksPtr& curDoor_links = doors_links[openedArea_doors[i].doorIndex];
		long doorSide = openedArea_doors[i].doorSide;
		for (long j = 0; j < curDoor->numLinks[doorSide]; j++)
			curDoor_links[doorSide][j].cost = curDoor_links[doorSide][j].openCost;
	}

	opens = true;
}

//---------------------------------------------------------------------------

void GlobalMap::print (char* fileName) {

	if (areaMap)
		return;

	File* debugFile = new File;
	debugFile->create(fileName);

	char outString[500];
	for (long row = 0; row < height; row++) {
		outString[0] = '\0';
		for (long col = 0; col < width; col++) {
			if (areaMap[row * width + col] == -2)
				strcat(outString, ">< ");
			else if (areaMap[row * width + col] == -1)
				strcat(outString, "** ");
			else {
				char chStr[8];
				sprintf(chStr, "%02x ", areaMap[row * width + col]);
				strcat(outString, chStr);
			}
		}
		strcat(outString, "\n");
		debugFile->writeString(outString);
	}
	debugFile->writeString("\n");

	debugFile->close();
	delete debugFile;
	debugFile = NULL;
}

//---------------------------------------------------------------------------

void GlobalMap::destroy (void) {

	#ifdef _DEBUG
//	systemHeap->walkHeap(false,false,"GlobalMap BAD HEAP1\n");
	#endif

	if (areaMap) {
		systemHeap->Free(areaMap);
		areaMap = NULL;
	}

	if (areas) {
		if (!doorInfos) {
			for (long i = 0; i < (numAreas + 1); i++) {
				if (areas_cellsCovered[i]) {
					systemHeap->Free(areas_cellsCovered[i]);
				    areas_cellsCovered[i] = NULL;
				}
				if (areas_doors[i]) {
					systemHeap->Free(areas_doors[i]);
					areas_doors[i] = NULL;
				}
			}
		}
		systemHeap->Free(areas);
		areas = NULL;
        areas_doors = NULL;
	}

	if (doors) {
		if (!doorLinks) {
			for (long i = 0; i < (numDoors + NUM_DOOR_OFFSETS); i++)
				for (long s = 0; s < 2; s++) {
					if (doors_links[i][s]) {
						systemHeap->Free(doors_links[i][s]);
						doors_links[i][s] = NULL;
					}
				}
		}
		systemHeap->Free(doors);
		doors = NULL;
        doors_links = NULL;
	}

	if (doorInfos) {
		systemHeap->Free(doorInfos);
		doorInfos = NULL;
	}

	if (doorLinks) {
		systemHeap->Free(doorLinks);
		doorLinks = NULL;
	}

	if (pathExistsTable) {
		systemHeap->Free(pathExistsTable);
		pathExistsTable = NULL;
	}

#ifdef USE_PATH_COST_TABLE
	if (pathCostTable) {
		systemHeap->Free(pathCostTable);
		pathCostTable = NULL;
	}
#endif
}

//----------------------------------------------------------------------------------

bool GlobalMap::toggleLog (void) {

	if (!log) {
		GameLog::setup();
		log = GameLog::getNewFile();
		if (!log)
			Fatal(0, " Couldn't create lrmove log ");
		long err = log->open("lrmove.log");
		if (err)
			Fatal(0, " Couldn't open lrmove log ");
#if 1
		GlobalMapPtr map = GlobalMoveMap[0];
		if (true) {
			char s[256];
			for (long i = 0; i < map->numDoors; i++) {
				sprintf(s, "door %05d, %s(%d), areas %d & %d", i, map->doors[i].open ? "opened" : "CLOSED", map->doors[i].teamID, map->doors[i].area[0], map->doors[i].area[1]);
				log->write(s);
				for (long side = 0; side < 2; side++) {
					sprintf(s, "     side %d", side);
					log->write(s); 
					for (long j = 0; j < map->doors[i].numLinks[side]; j++) {
						sprintf(s, "          link %03d, to door %05d, cost %d",
							j,
							map->doors_links[i][side][j].doorIndex,
							map->doors_links[i][side][j].cost);
						log->write(s); 
					}
				}
			}
			log->write(" "); 

			for (int i = 0; i < map->numAreas; i++) {
				sprintf(s, "area %05d, %s(%d)", i, map->areas[i].open ? "opened" : "CLOSED", map->areas[i].teamID);
				log->write(s); 
				if (map->areas[i].ownerWID > 0) {
					sprintf(s, "     ownerWID is %d", map->areas[i].ownerWID);
					log->write(s);
				}
				sprintf(s, "     %s", typeString[map->areas[i].type]);
				log->write(s);
				for (long d = 0; d < map->areas[i].numDoors; d++) {
                    const DoorInfo& di = map->areas_doors[i][d];
					sprintf(s, "     door %03d is %d (%d:%d & %d) ",
						d, di.doorIndex, di.doorSide,
						map->doors[di.doorIndex].area[0], map->doors[di.doorIndex].area[1]);
					log->write(s);
				}
				
			}
			log->write(" ");
		}
#endif
	}
	logEnabled = !logEnabled;
	return(logEnabled);
}

//----------------------------------------------------------------------------------

void GlobalMap::writeLog (char* s) {

	if (log)
		log->write(s);
}

//**********************************************************************************
// MOVE MAP class
//**********************************************************************************

void* MoveMap::operator new (size_t ourSize) {

	void* result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void MoveMap::operator delete (void* us) {

	systemHeap->Free(us);
}	

//---------------------------------------------------------------------------

void MoveMap::init (long maxW, long maxH) {

	width = maxWidth = maxW;
	height = maxHeight = maxH;
	long mapByteSize = sizeof(MoveMapNode) * maxWidth * maxHeight;
	map = (MoveMapNodePtr)systemHeap->Malloc(mapByteSize);
	gosASSERT(map != NULL);

	mapRowStartTable = (int*)systemHeap->Malloc(maxHeight * sizeof(int));
	gosASSERT(mapRowStartTable != NULL);
	for (long r = 0; r < maxHeight; r++)
		mapRowStartTable[r] = r * maxWidth;

	mapRowTable = (int*)systemHeap->Malloc(maxHeight * maxWidth * sizeof(int));
	gosASSERT(mapRowTable != NULL);
	mapColTable = (int*)systemHeap->Malloc(maxHeight * maxWidth * sizeof(int));
	gosASSERT(mapColTable != NULL);
	for (int r = 0; r < maxHeight; r++)
		for (int c = 0; c < maxWidth; c++) {
			int index = mapRowStartTable[r] + c;
			mapRowTable[index] = r;
			mapColTable[index] = c;
		}

	for (int r = 0; r < maxHeight; r++)
		for (int c = 0; c < maxWidth; c++) {
			int mapCellIndex = r * maxWidth + c;
			for (int d = 0; d < NUM_ADJ_CELLS; d++) {
				int indexStart = d * 2;
				int adjRow = r + cellShift[indexStart];
				int adjCol = c + cellShift[indexStart + 1];
				if (inMapBounds(adjRow, adjCol, height, width))
					map[mapCellIndex].adjCells[d] = adjRow * maxWidth + adjCol;
				else
					map[mapCellIndex].adjCells[d] = -1;
			}
		}


	float cellLength = Terrain::worldUnitsPerCell * metersPerWorldUnit;
	for (long i = 0; i < DISTANCE_TABLE_DIM; i++)
		for (long j = 0; j < DISTANCE_TABLE_DIM; j++) {
			distanceFloat[i][j] = agsqrt(i, j) * cellLength;
			distanceInt[i][j] = (int)distanceFloat[i][j];
		}

	clear();
}

//---------------------------------------------------------------------------

void MoveMap::clear (void) {

	int numMapCells = maxWidth * height;
	int initHPrime = ZeroHPrime ? 0 : HPRIME_NOT_CALCED;
	for (int i = 0; i < numMapCells; i++) {
		MoveMapNodePtr node = &map[i];
		//------------------------------------------------
		// DON'T NEED TO SET THIS SINCE IS SET IN SETUP...
		node->cost = COST_BLOCKED;
		node->parent = -1;
		node->flags = 0;
		node->hPrime = initHPrime;
	}

	goal.Zero();
	target.x = -999999.0;
	target.y = -999999.0;
	target.z = -999999.0;
}

//---------------------------------------------------------------------------

void MoveMap::setTarget (Stuff::Vector3D targetPos) {

	target = targetPos;
}

//---------------------------------------------------------------------------

void MoveMap::setStart (Stuff::Vector3D* startPos, long startRow, long startCol) {

	if (startPos)
		start = *startPos;
	else {
		start.x = -999999.0;
		start.y = -999999.0;
		start.z = -999999.0;
	}
	if (startRow == -1) {
		land->worldToCell(*startPos, startR, startC);
		startR -= ULr;
		startC -= ULc;
		}
	else {
		startR = startRow;
		Assert(startRow > -1, 0, " HUH ");
		startC = startCol;
	}
}

//---------------------------------------------------------------------------

void MoveMap::setGoal (Stuff::Vector3D goalPos, long goalRow, long goalCol) {

	goal = goalPos;
	if (goalRow == -1) {
		land->worldToCell(goal, goalR, goalC);
		goalR -= ULr;
		goalC -= ULc;
		}
	else {
		goalR = goalRow;
		goalC = goalCol;
	}
	doorDirection = -1;
	GoalIsDoor = false;
}

//---------------------------------------------------------------------------

inline void adjustMoveMapCellCost (MoveMapNodePtr cell, long costAdj) {

	long cost = cell->cost + costAdj;
	if (cost < 1)
		cost = 1;
	cell->cost = cost;
}

//---------------------------------------------------------------------------

void MoveMap::setGoal (long thruArea, long goalDoor) {

	//------------------------------------------------------------------------------
	// We need to set goalR, goalC for the calcHPrime routine (until we come up with
	// a better one that uses the door range. If the global path's final goal is on
	// the other side of this door, we should select THAT cell of the door...
	goal.x = -999999.0;
	goal.y = -999999.0;
	goal.z = -999999.0;

	GoalIsDoor = true;
	door = goalDoor;
	doorSide = (GlobalMoveMap[moveLevel]->doors[goalDoor].area[1] == thruArea);
	long doorDirTable[4][2] = {
		{-1, -1},
		{1, 3},
		{2, 0},
		{-1, -1}
	};

	long goalDoorDir = GlobalMoveMap[moveLevel]->doors[goalDoor].direction[0];
	gosASSERT((goalDoorDir == 1) || (goalDoorDir == 2));
	doorDirection = doorDirTable[goalDoorDir][doorSide];

	long doorLength = GlobalMoveMap[moveLevel]->doors[goalDoor].length;

	if ((doorDirection == 0) || (doorDirection == 2)) {
		//----------------------------------------------------------------
		// We need to set goalR and goalC for the calcHPrime function used
		// in pathfinding...
		goalR = GlobalMoveMap[moveLevel]->doors[goalDoor].row + doorSide - ULr;
		goalC = GlobalMoveMap[moveLevel]->doors[goalDoor].col + (doorLength / 2) - ULc;
		}
	else if ((doorDirection == 1) || (doorDirection == 3)) {
		//----------------------------------------------------------------
		// We need to set goalR and goalC for the calcHPrime function used
		// in pathfinding...
		goalR = GlobalMoveMap[moveLevel]->doors[goalDoor].row + (doorLength / 2) - ULr;
		goalC = GlobalMoveMap[moveLevel]->doors[goalDoor].col + doorSide - ULc;
	}
}

//---------------------------------------------------------------------------

inline long MoveMap::markGoals (Stuff::Vector3D finalGoal) {

	//--------------------------------------
	// First, mark the blocked goal cells...
	static char doorCellState[1024];
	for (int j = 0; j < GlobalMoveMap[moveLevel]->doors[door].length; j++)
		doorCellState[j] = 1;

	if (blockedDoorCallback)
		(*blockedDoorCallback)(moveLevel, door, &doorCellState[0]);

	//-------------------------------------------------------------------------
	// Ultimately, we should do this conversion once for the finalGoal and then
	// store it in the moveOrders data...
	int finalGoalR, finalGoalC;
	land->worldToCell(finalGoal, finalGoalR, finalGoalC);
	//-----------------------------------------
	// Localize the coords for this move map...
	finalGoalR -= ULr;
	finalGoalC -= ULc;

	int doorLength = GlobalMoveMap[moveLevel]->doors[door].length;

	int numGoalCells = 0;
	if ((doorDirection == 0) || (doorDirection == 2)) {
		//--------------------------------
		// Mark the door cells as goals...
		int cellR = goalR;
		int cellC = goalC - (doorLength / 2);
		bool nextToGoal = false;
		if (doorSide == 0) {
			if (finalGoalR == (cellR + 1))
//				if (!doorCellBlocked(cellR, cellC))
					if ((finalGoalC >= cellC) && (finalGoalC < (cellC + doorLength))) {
						//-----------------------------------------------------
						// Our global goal is on the other side of this door...
						map[cellR * width + finalGoalC].setFlag(MOVEFLAG_GOAL);
						numGoalCells++;
						nextToGoal = true;
					}
			}
		else {
			if (finalGoalR == (cellR - 1))
//				if (!doorCellBlocked(cellR, cellC)
					if ((finalGoalC >= cellC) && (finalGoalC < (cellC + doorLength))) {
						//-----------------------------------------------------
						// Our global goal is on the other side of this door...
						map[cellR * maxWidth + finalGoalC].setFlag(MOVEFLAG_GOAL);
						numGoalCells++;
						nextToGoal = true;
					}
		}

		if (!nextToGoal) {
			long adjCost = clearCost / 2;
			long doorCenter = doorLength / 2;
			long cellIndex = cellR * maxWidth + cellC;
			long curCost = doorCenter * adjCost;
			for (long c = 0; c < doorCenter; c++) {
				if (doorCellState[c]) {
					map[cellIndex].setFlag(MOVEFLAG_GOAL);
					numGoalCells++;
					adjustMoveMapCellCost(&map[cellIndex], curCost);
				}
				cellIndex++;
				curCost -= adjCost;
			}
			cellIndex = cellR * maxWidth + cellC + doorCenter;
			curCost = 0;
			for (int c = doorCenter; c < doorLength; c++) {
				if (doorCellState[c]) {
					map[cellIndex].setFlag(MOVEFLAG_GOAL);
					numGoalCells++;
					adjustMoveMapCellCost(&map[cellIndex], curCost);
				}
				cellIndex++;
				curCost += adjCost;
			}
		}
		}
	else if ((doorDirection == 1) || (doorDirection == 3)) {
		long cellR = goalR - (doorLength / 2);
		long cellC = goalC;
		bool nextToGoal = false;
		if (finalGoalC == (cellC + 1 - doorSide * 2))
			if ((finalGoalR >= cellR) && (finalGoalR < (cellR + doorLength))) {
				//-----------------------------------------------------
				// Our global goal is on the other side of this door...
				map[finalGoalR * maxWidth + cellC].setFlag(MOVEFLAG_GOAL);
				numGoalCells++;
				nextToGoal = true;
			}
			
		if (!nextToGoal) {
			long adjCost = clearCost / 2;
			long doorCenter = doorLength / 2;
			long cellIndex = cellR * maxWidth + cellC;
			long curCost = doorCenter * adjCost;
			for (long r = 0; r < doorCenter; r++) {
				if (doorCellState[r]) {
					map[cellIndex].setFlag(MOVEFLAG_GOAL);
					numGoalCells++;
					adjustMoveMapCellCost(&map[cellIndex], curCost);
				}
				cellIndex += maxWidth;
				curCost -= adjCost;
			}
			curCost = 0;
			cellIndex = (cellR + doorCenter) * maxWidth + cellC;
			for (int r = doorCenter; r < doorLength; r++) {
				if (doorCellState[r]) {
					map[cellIndex].setFlag(MOVEFLAG_GOAL);
					numGoalCells++;
					adjustMoveMapCellCost(&map[cellIndex], curCost);
				}
				cellIndex += maxWidth;
				curCost += adjCost;
			}
		}
	}
	
//	if (numGoalCells == 0)
//		OutputDebugString("No Goal Cells");
	return(numGoalCells);
}

//---------------------------------------------------------------------------

inline long MoveMap::markEscapeGoals (Stuff::Vector3D finalGoal) {

	long numGoalCells = 0;

	//-------------------------------------------------------------------------
	// Ultimately, we should do this conversion once for the finalGoal and then
	// store it in the moveOrders data...
	int finalGoalR, finalGoalC;
	land->worldToCell(finalGoal, finalGoalR, finalGoalC);
	int finalGoalArea = GlobalMoveMap[moveLevel]->calcArea(finalGoalR, finalGoalC);

	//------------------------------------------------------------------------
	// For each tile, mark its cells as valid goals if:
	//		1) the tile's areaId == the areaId of the finalGoal
	//		2) OR, if a LR path exists between the tile and the finalGoal tile
	int cellIndex = 0;
	for (int row = 0; row < height; row++)
		for (int col = 0; col < width; col++) {
			if (GameMap->inBounds(ULr + row, ULc + col)) {
				int curArea = GlobalMoveMap[moveLevel]->calcArea(ULr + row, ULc + col);
				int confidence;
				int numLRSteps = GlobalMoveMap[moveLevel]->getPathCost(curArea, finalGoalArea, false, confidence, true);
				bool validGoal = (numLRSteps > 0);
				if (validGoal)
					map[cellIndex].setFlag(MOVEFLAG_GOAL);
			}
			cellIndex++;
		}

	return(numGoalCells);
}

//---------------------------------------------------------------------------

long MoveMap::setUp (long mapULr,
					 long mapULc,
					 long mapWidth,
					 long mapHeight,
					 long level,
					 Stuff::Vector3D* startPos,
					 long startRow,
					 long startCol,
					 Stuff::Vector3D goalPos,
					 long goalRow,
					 long goalCol,
					 long clearCellCost,
					 long jumpCellCost,
					 long offsets,
					 unsigned long params) {

	//-----------------------------------------------------------------------------
	// If the map has not been allocated yet, then the tile height and width passed
	// is used as both the max and current dimensions. Otherwise, they are only
	// used as the current dimensions...
	if ((params & MOVEPARAM_JUMP) && (jumpCellCost > 0))
		jumpCellCost = clearCellCost;
	if (!map) {
		init(mapHeight, mapWidth);
		setClearCost(clearCellCost);
		setJumpCost(jumpCellCost, offsets);
		}
	else {
		width = mapWidth;
		height = mapHeight;
		setClearCost(clearCellCost);
		setJumpCost(jumpCellCost, offsets);
		clear();
	}

	thruAreas[0] = -1;
	thruAreas[1] = -1;

	//---------------------------------------------
	// First, gotta set some params for the calc...
	ULr = mapULr;
	ULc = mapULc;
	minRow = 0;
	minCol = 0;
	maxRow = mapHeight - 1;
	maxCol = mapWidth - 1;
	moveLevel = level;


	startRow -= ULr;
	startCol -= ULc;
	Assert(startRow > -1, 0, " huh ");
	setStart(startPos, startRow, startCol);
	if (FindingEscapePath) {
		setGoal(goalPos, goalRow, goalCol);
		}
	else
		setGoal(goalPos, goalRow, goalCol);

	travelOffMap = false;
	cannotEnterOffMap = true;
	if (GameMap->getOffMap(ULr + startRow, ULc + startCol))
		travelOffMap = true;
	if (moverWithdrawing) {
		travelOffMap = true;
		cannotEnterOffMap = false;
	}

	bool followRoads = ((params & MOVEPARAM_FOLLOW_ROADS) != 0);
	bool traverseShallowWater = ((params & (MOVEPARAM_WATER_SHALLOW + MOVEPARAM_WATER_DEEP)) != 0);
	bool traverseDeepWater = ((params & MOVEPARAM_WATER_DEEP) != 0);
	bool avoidMines = true;
	if (params & MOVEPARAM_SWEEP_MINES)
		avoidMines = false;

	//-------------------------------------------------
	// Now that the params are set up, build the map...
	long pathLockCost = clearCost << 3;

	//--------------------------------------------------------------
	// Set the map costs based upon the tiles in the scenario map...
	bool groundMover = ((moveLevel == 0) || (moveLevel == 1));
	for (long cellRow = 0; cellRow < height; cellRow++)
		for (long cellCol = 0; cellCol < width; cellCol++) {			
			if (GameMap->inBounds(ULr + cellRow, ULc + cellCol)) {
				MapCellPtr mapCell = GameMap->getCell(ULr + cellRow, ULc + cellCol);
				long moveMapIndex = cellRow * maxWidth + cellCol;

				long cost = clearCost;
				bool offMapCell = mapCell->getOffMap();
				if (offMapCell)
					map[moveMapIndex].setFlag(MOVEFLAG_OFFMAP);

				//-----------------------
				// Tile (terrain) type...
				//long tileType = curTile.getTileType();
				if (offMapCell && !travelOffMap)
					cost = COST_BLOCKED;
				else if (groundMover) {
					if (mapCell->getShallowWater()) {
						if (!traverseShallowWater)
							cost = COST_BLOCKED;
						else if (!mapCell->getPassable())
							cost = COST_BLOCKED;
						}
					else if (mapCell->getDeepWater()) {
						if (!traverseDeepWater)
							cost = COST_BLOCKED;
						else if (!mapCell->getPassable())
							cost = COST_BLOCKED;
						}
					else if (mapCell->getGate()) 
					{
						long areaID = GlobalMoveMap[moveLevel]->calcArea(ULr + cellRow, ULc + cellCol);
						long teamID = -1;
						if (areaID > -1) 
						{
							teamID = GlobalMoveMap[moveLevel]->areas[areaID].teamID;
							//Its possible for the ownerWIDs to be invalid for one or two frames after a quick save.
							// We handle this ok later on!!  OwnerWIDs restablish themselves right after the first unpaused update!
							/*
							if (!EditorSave)
								if (GlobalMoveMap[moveLevel]->areas[areaID].ownerWID < 1)
									PAUSE(("Gate has no ownerWID.  CellR: %d, CellC: %d, WID: %d, MoveLvl: %d",cellRow,cellCol,GlobalMoveMap[moveLevel]->areas[areaID].ownerWID,moveLevel));
							*/
						}
						if (!EditorSave && (areaID > -1) && GlobalMoveMap[moveLevel]->isGateDisabledCallback(GlobalMoveMap[moveLevel]->areas[areaID].ownerWID))
							cost = COST_BLOCKED;
						else if ((teamID > -1) && (TeamRelations[teamID][moverTeamID] != RELATION_FRIENDLY)) {
							if (mapCell->getPassable())
								cost <<= 2;
							else
								cost = COST_BLOCKED;
							}
						else if (mapCell->getRoad() && followRoads)
							cost >>= 2;
						}
					else {
						if (!mapCell->getPassable())
							cost = COST_BLOCKED;
						else if (mapCell->getRoad() && followRoads)
							cost >>= 2;
					}
					if (mapCell->getForest())
						cost += forestCost;
				}
				map[moveMapIndex].cost = cost;

				//---------------------------------------------------------------
				// NOTE: With gates, we may want them to set the cell cost rather
				// than just adjust it. Let's see how they play. Since they're
				// set as an overlay, we'll just treat them as such for now.
				
				if (mapCell->getPathlock(moveLevel == 2))
					adjustMoveMapCellCost(&map[cellRow * maxWidth + cellCol], pathLockCost);
			}
		}


	if (FindingEscapePath)
		markEscapeGoals(goalPos);
	else
		map[goalR * maxWidth + goalC].setFlag(MOVEFLAG_GOAL);

	__int64 startTime = GetCycles();

	if (params & MOVEPARAM_STATIONARY_MOVERS)
		if (placeStationaryMoversCallback)
			(*placeStationaryMoversCallback)(this);

#ifdef LAB_ONLY
	MCTimeCalcPath1Update += (GetCycles() - startTime);
#endif

	return(NO_ERR);
}

//---------------------------------------------------------------------------

long MoveMap::setUp (long level,
					 Stuff::Vector3D* startPos,
					 long startRow,
					 long startCol,
					 long thruArea[2],
					 long goalDoor,
					 Stuff::Vector3D finalGoal,
					 long clearCellCost,
					 long jumpCellCost,
					 long offsets,
					 unsigned long params) {

	__int64 startTime = GetCycles();

	//-----------------------------------------------------------------------------
	// If the map has not been allocated yet, then the tile height and width passed
	// is used as both the max and current dimensions. Otherwise, they are only
	// used as the current dimensions...
	if (!map)
		init(SECTOR_DIM*2, SECTOR_DIM*2);
	else {
		width = SECTOR_DIM*2;
		height = SECTOR_DIM*2;
		clear();
	}

	thruAreas[0] = thruArea[0];
	thruAreas[1] = thruArea[1];

	//---------------------------------------------
	// First, gotta set some params for the calc...
	long firstSectorRow = GlobalMoveMap[level]->areas[thruArea[0]].sectorR;
	long firstSectorCol = GlobalMoveMap[level]->areas[thruArea[0]].sectorC;
	if (thruArea[1] == -1) {
		ULr = firstSectorRow * SECTOR_DIM;
		ULc = firstSectorCol * SECTOR_DIM;
		minRow = 0;
		minCol = 0;
		maxRow = SECTOR_DIM - 1;
		maxCol = SECTOR_DIM - 1;
		}
	else {
		long secondSectorRow = GlobalMoveMap[level]->areas[thruArea[1]].sectorR;
		long secondSectorCol = GlobalMoveMap[level]->areas[thruArea[1]].sectorC;
		if (secondSectorRow > firstSectorRow) {
			if (secondSectorCol == firstSectorCol) {
				ULr = firstSectorRow * SECTOR_DIM;
				ULc = firstSectorCol * SECTOR_DIM;
				minRow = 0;
				minCol = 0;
				maxRow = SECTOR_DIM * 2 - 1;
				maxCol = SECTOR_DIM - 1;
				}
			else
				STOP(("MoveMap.setup: diagonal sectors not allowed #1"));
			}
		else if (secondSectorRow < firstSectorRow) {
			if (secondSectorCol == firstSectorCol) {
				ULr = secondSectorRow * SECTOR_DIM;
				ULc = secondSectorCol * SECTOR_DIM;
				minRow = 0;
				minCol = 0;
				maxRow = SECTOR_DIM * 2 - 1;
				maxCol = SECTOR_DIM - 1;
				}
			else
				STOP(("MoveMap.setup: diagonal sectors not allowed #2"));
			}
		else {
			if (secondSectorCol > firstSectorCol) {
				ULr = firstSectorRow * SECTOR_DIM;
				ULc = firstSectorCol * SECTOR_DIM;
				minRow = 0;
				minCol = 0;
				maxRow = SECTOR_DIM - 1;
				maxCol = SECTOR_DIM * 2 - 1;
				}
			else if (secondSectorCol < firstSectorCol) {
				ULr = secondSectorRow * SECTOR_DIM;
				ULc = secondSectorCol * SECTOR_DIM;
				minRow = 0;
				minCol = 0;
				maxRow = SECTOR_DIM - 1;
				maxCol = SECTOR_DIM * 2 - 1;
				}
			else {
				ULr = firstSectorRow * SECTOR_DIM;
				ULc = firstSectorCol * SECTOR_DIM;
				minRow = 0;
				minCol = 0;
				maxRow = SECTOR_DIM - 1;
				maxCol = SECTOR_DIM - 1;
				//STOP(("MoveMap.setup: same sectors not allowed"));
			}
		}
	}

	moveLevel = level;

	setClearCost(clearCellCost);
	if ((params & MOVEPARAM_JUMP) && (jumpCellCost > 0))
		jumpCellCost = clearCellCost;
	setJumpCost(jumpCellCost, offsets);

	startRow -= ULr;
	startCol -= ULc;
	setStart(startPos, startRow, startCol);
	if (thruArea[1] == -1)
		setGoal(thruArea[0], goalDoor);
	else
		setGoal(thruArea[1], goalDoor);

	long pathLockCost = clearCost << 3;

	travelOffMap = false;
	cannotEnterOffMap = true;
	if (GameMap->getOffMap(ULr + startRow, ULc + startCol))
		travelOffMap = true;
	if (moverWithdrawing) {
		travelOffMap = true;
		cannotEnterOffMap = false;
	}

	//-------------------------------------------------
	// Now that the params are set up, build the map...
	bool followRoads = ((params & MOVEPARAM_FOLLOW_ROADS) != 0);
	bool traverseShallowWater = ((params & (MOVEPARAM_WATER_SHALLOW + MOVEPARAM_WATER_DEEP)) != 0);
	bool traverseDeepWater = ((params & MOVEPARAM_WATER_DEEP) != 0);
	bool avoidMines = true;
	if (params & MOVEPARAM_SWEEP_MINES)
		avoidMines = false;

	//--------------------------------------------------------------
	// Set the map costs based upon the tiles in the scenario map...
	bool groundMover = ((moveLevel == 0) || (moveLevel == 1));
	for (long cellRow = 0; cellRow < height; cellRow++)
		for (long cellCol = 0; cellCol < width; cellCol++) {			
			if (GameMap->inBounds(ULr + cellRow, ULc + cellCol) && inBounds(cellRow, cellCol)) {
				MapCellPtr mapCell = GameMap->getCell(ULr + cellRow, ULc + cellCol);

				long moveMapIndex = cellRow * maxWidth + cellCol;

				long cost = clearCost;
				bool offMapCell = mapCell->getOffMap();
				if (offMapCell)
					map[moveMapIndex].setFlag(MOVEFLAG_OFFMAP);

				long areaID = GlobalMoveMap[moveLevel]->calcArea(ULr + cellRow, ULc + cellCol);
				if (CullPathAreas && (areaID != thruAreas[0]) && (areaID != thruAreas[1]))
					cost = COST_BLOCKED;
				else if (offMapCell && !travelOffMap)
					cost = COST_BLOCKED;
				else if (groundMover) {
					if (mapCell->getShallowWater()) {
						if (!traverseShallowWater)
							cost = COST_BLOCKED;
						else if (!mapCell->getPassable())
							cost = COST_BLOCKED;
						}
					else if (mapCell->getDeepWater()) {
						if (!traverseDeepWater)
							cost = COST_BLOCKED;
						else if (!mapCell->getPassable())
							cost = COST_BLOCKED;
						}
					else if (mapCell->getGate()) {
						long areaID = GlobalMoveMap[moveLevel]->calcArea(ULr + cellRow, ULc + cellCol);
						long teamID = -1;
						if (areaID > -1) {
							teamID = GlobalMoveMap[moveLevel]->areas[areaID].teamID;
							/*  See Above
							if (!EditorSave)
								if (GlobalMoveMap[moveLevel]->areas[areaID].ownerWID < 1)
									PAUSE(("Gate has no ownerWID"));
							*/
						}
						if (!EditorSave && (areaID > -1) && GlobalMoveMap[moveLevel]->isGateDisabledCallback(GlobalMoveMap[moveLevel]->areas[areaID].ownerWID))
							cost = COST_BLOCKED;
						else if ((teamID > -1) && (TeamRelations[teamID][moverTeamID] != RELATION_FRIENDLY)) {
							if (mapCell->getPassable())
								cost <<= 2;
							else
								cost = COST_BLOCKED;
							}
						else if (mapCell->getRoad() && followRoads)
							cost >>= 2;
						}
					else {
						if (!mapCell->getPassable())
							cost = COST_BLOCKED;
						else if (mapCell->getRoad() && followRoads)
							cost >>= 2;
					}
					if (mapCell->getForest())
						cost += forestCost;
				}
				setCost(cellRow, cellCol, cost);

				if (mapCell->getPathlock(moveLevel == 2))
					adjustMoveMapCellCost(&map[cellRow * maxWidth + cellCol], pathLockCost);
			}
		}

	if (markGoals(finalGoal) == 0)
		return(-1);

	if (params & MOVEPARAM_STATIONARY_MOVERS)
		if (placeStationaryMoversCallback)
			(*placeStationaryMoversCallback)(this);

#ifdef LAB_ONLY
	MCTimeCalcPath2Update += (GetCycles() - startTime);
#endif
	return(NO_ERR);
}

//---------------------------------------------------------------------------

inline bool MoveMap::adjacentCellOpen (long mapCellIndex, long dir) {

	long adjCellIndex = map[mapCellIndex].adjCells[dir];
	if (adjCellIndex == -1)
		return(false);

	if (map[adjCellIndex].flags & MOVEFLAG_MOVER_HERE)
		return(false);

#ifdef USE_MINES_IN_MC2
	if (moverRelation == RELATION_ENEMY) {
		if (GameMap->getTile(adjRow,adjCol).getClanMineMove())
			return(false);
		}
	else {
		if (GameMap->getTile(adjRow,adjCol).getInnerSphereMineMove())
			return(false);
	}
#endif

	return(map[adjCellIndex].cost < COST_BLOCKED);
}

//---------------------------------------------------------------------------

inline bool MoveMap::adjacentCellOpenJUMP (long r, long c, long dir) {

	long indexStart = dir * 2;
	long adjRow = r + cellShift[indexStart];
	long adjCol = c + cellShift[indexStart + 1];

	if (!inMapBounds(adjRow, adjCol, height, width))
		return(false);

	if (map[adjRow * maxWidth + adjCol].flags & MOVEFLAG_MOVER_HERE)
		return(false);

#ifdef USE_MINES_IN_MC2
	if (moverRelation == RELATION_ENEMY) {
		if (GameMap->getTile(adjRow,adjCol).getClanMineMove())
			return(false);
		}
	else {
		if (GameMap->getTile(adjRow,adjCol).getInnerSphereMineMove())
			return(false);
	}
#endif

	return(map[adjRow * maxWidth + adjCol].cost < COST_BLOCKED);
}

//---------------------------------------------------------------------------

inline long MoveMap::calcHPrime (long r, long c) {
	__int64 startTime = GetCycles();
	long sum = 0;
	if (r > goalR)
		sum += (r - goalR);
	else
		sum += (goalR - r);
		
	if (c > goalC)
		sum += (c - goalC);
	else
		sum += (goalC - c);
#ifdef LAB_ONLY
	MCTimeCalcPath3Update += (GetCycles() - startTime);		
#endif
	return(sum);
}

#define MAX_MAPWIDTH		1000

//---------------------------------------------------------------------------

inline void MoveMap::propogateCost (long mapCellIndex, long cost, long g) {

	gosASSERT(cost > 0);
	gosASSERT(g >= 0);
	MoveMapNodePtr curMapNode = &map[mapCellIndex];
	if (curMapNode->g > (g + cost)) {
		curMapNode->cost = cost;
		curMapNode->g = g + cost;
		curMapNode->fPrime = curMapNode->g + curMapNode->hPrime;
		if (curMapNode->flags & MOVEFLAG_OPEN) {
			long openIndex = openList->find(mapCellIndex);
			if (!openIndex) {
				char s[128];
				sprintf(s, "MoveMap.propogateCost: Cannot find movemap node [%d] for change\n", mapCellIndex);
				gosASSERT(openIndex != 0);
				}
			else
				openList->change(openIndex, curMapNode->fPrime);
			}
		else {
			for (long dir = 0; dir < numOffsets; dir++) {
				//------------------------------------------------------------
				// First, make sure this is a legit direction to go. We do NOT
				// want to clip corners, so we'll check that here...
				bool isDiagonalWalk = IsDiagonalStep[dir];
				if (isDiagonalWalk) {

					// MINE CHECK should go in these adj tests...
					bool adj1Open = false;
					long adjCellIndex = map[mapCellIndex].adjCells[StepAdjDir[dir]];
					if (adjCellIndex > -1)
						if ((map[adjCellIndex].flags & MOVEFLAG_MOVER_HERE) == 0)
							adj1Open = (map[adjCellIndex].cost < COST_BLOCKED);

					bool adj2Open = false;
					adjCellIndex = map[mapCellIndex].adjCells[StepAdjDir[dir + 1]];
					if (adjCellIndex > -1)
						if ((map[adjCellIndex].flags & MOVEFLAG_MOVER_HERE) == 0)
							adj2Open = (map[adjCellIndex].cost < COST_BLOCKED);
					
					if (!adj1Open && !adj2Open)
						continue;
					
					//if (!adjacentCellOpen(mapCellIndex, StepAdjDir[dir]) && !adjacentCellOpen(mapCellIndex, StepAdjDir[dir + 1]))
					//	continue;
				}

				//----------------------------------------------------------
				// Calc the cell we're checking, offset from current cell...
				long succCellIndex = curMapNode->adjCells[dir];
				//--------------------------------
				// If it's on the map, check it...
				if (succCellIndex > -1) {
					MoveMapNodePtr succMapNode = &map[succCellIndex];
					if (succMapNode->cost < COST_BLOCKED)
						if ((succMapNode->hPrime != HPRIME_NOT_CALCED) && (succMapNode->hPrime < MaxHPrime)) {
							char dirToParent = reverseShift[dir];
							long cost = succMapNode->cost;
							//------------------------------------
							// Diagonal movement is more costly...
							gosASSERT(cost > 0);
							if (isDiagonalWalk)
								cost += (cost << 1);
							if (dirToParent == succMapNode->parent)
								propogateCost(succCellIndex, cost, curMapNode->g);
							else if ((curMapNode->g + cost) < succMapNode->g) {
								succMapNode->parent = dirToParent;
								propogateCost(succCellIndex, cost, curMapNode->g);
							}
						}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

inline void MoveMap::propogateCostJUMP (long r, long c, long cost, long g) {

	gosASSERT(cost > 0);
	gosASSERT(g >= 0);
	MoveMapNodePtr curMapNode = &map[r * maxWidth + c];
	if (curMapNode->g > (g + cost)) {
		curMapNode->cost = cost;
		curMapNode->g = g + cost;
		curMapNode->fPrime = curMapNode->g + curMapNode->hPrime;
		if (curMapNode->flags & MOVEFLAG_OPEN) {
			long openIndex = openList->find(r * MAX_MAPWIDTH + c);
			if (!openIndex) {
				char s[128];
				sprintf(s, "MoveMap.propogateCost: Cannot find movemap node [%d, %d, %d] for change\n", r, c, r * MAX_MAPWIDTH + c);
				gosASSERT(openIndex != 0);
				}
			else
				openList->change(openIndex, curMapNode->fPrime);
			}
		else {
			long cellOffsetIndex = 0;
			for (long dir = 0; dir < numOffsets; dir++) {
				//------------------------------------------------------------
				// First, make sure this is a legit direction to go. We do NOT
				// want to clip corners, so we'll check that here...
				bool isDiagonalWalk = IsDiagonalStep[dir];
				if (isDiagonalWalk) {
					if (!adjacentCellOpenJUMP(r, c, StepAdjDir[dir]) && !adjacentCellOpenJUMP(r, c, StepAdjDir[dir + 1])) {
						cellOffsetIndex += 2;
						continue;
					}
				}

				//----------------------------------------------------------
				// Calc the cell we're checking, offset from current cell...
				long succRow = r + cellShift[cellOffsetIndex++];
				long succCol = c + cellShift[cellOffsetIndex++];
				//--------------------------------
				// If it's on the map, check it...
				if (inMapBounds(succRow, succCol, height, width)) {
					MoveMapNodePtr succMapNode = &map[succRow * maxWidth + succCol];
					if (succMapNode->cost < COST_BLOCKED)
						if ((succMapNode->hPrime != HPRIME_NOT_CALCED) && (succMapNode->hPrime < MaxHPrime)) {
							char dirToParent = reverseShift[dir];

							bool jumping = false;
							long cost = succMapNode->cost;
							
							//------------------------------------
							// Diagonal movement is more costly...
							gosASSERT(cost > 0);
							if (dir > 7) {
								jumping = true;
								if (JumpOnBlocked)
									cost = jumpCost;
								else
									cost += jumpCost;
								}
							else if (isDiagonalWalk)
								cost += (cost / 2);

							if (dirToParent == succMapNode->parent)
								propogateCostJUMP(succRow, succCol, cost, curMapNode->g);
							else if ((curMapNode->g + cost) < succMapNode->g) {
								succMapNode->parent = dirToParent;
								propogateCostJUMP(succRow, succCol, cost, curMapNode->g);
							}
						}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

//#define DEBUG_PATH
//#define BLOCKED_PATH_TEST
//#define TIME_PATH
//#define DEBUG_MOVE_MAP

long MoveMap::calcPath (MovePathPtr path, Stuff::Vector3D* goalWorldPos, long* goalCell) {

	#ifdef TIME_PATH
		L_INTEGER calcStart, calcStop;
		QueryPerformanceCounter(calcStart);
	#endif

	if ((goalR < 0) || (goalR >= height) || (goalC < 0) || (goalC >= width)) {
		Stuff::Vector3D p;
		p.x = (float)(goalC) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
		p.y = (Terrain::worldUnitsMapSide / 2) - ((float)(goalR) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
		p.z = (float)0; // How do we get the elevation for this point? Do we care?
		char msg[200];
		sprintf(msg, " Bad Move Goal: %d [%d(%d), %d(%d)], (%.2f, %.2f, %.2f)", DebugMovePathType, goalR, height, goalC, width, p.x, p.y, p.z);
		gosASSERT((goalR >= 0) && (goalR < height) && (goalC >= 0) && (goalC < width));
	}

	//------------------------------------------------------------------
	// Let's use their hPrime as a barrier for cutting off the search...
	MaxHPrime = calcHPrime(startR, startC) * 2.5;
	if (MaxHPrime < 500)
		MaxHPrime = 500;
	
	//-----------------------------------------------
	// If we haven't already, create the OPEN list...
	if (!openList) {
		openList = new PriorityQueue;
		gosASSERT(openList != NULL);
		openList->init(5000);
	}
		
	long curCol = startC;
	long curRow = startR;
	
	MoveMapNodePtr curMapNode = &map[mapRowStartTable[curRow] + curCol];
	curMapNode->g = 0;
	if (!ZeroHPrime)
		curMapNode->hPrime = calcHPrime(curRow, curCol);
	curMapNode->fPrime = curMapNode->hPrime;

	//-----------------------------------------------
	// Put the START vertex on the empty OPEN list...
	PQNode initialVertex;
	initialVertex.key = curMapNode->fPrime;
	initialVertex.id = mapRowStartTable[curRow] + curCol;
	initialVertex.row = curRow;
	initialVertex.col = curCol;
	openList->clear();
#ifdef _DEBUG
	long insertErr = 
#endif
		openList->insert(initialVertex);
	gosASSERT(insertErr == NO_ERR);
	curMapNode->setFlag(MOVEFLAG_OPEN);

	bool goalFound = false;
	long bestRow = -1;
	long bestCol = -1;

	#ifdef DEBUG_PATH
		topOpenNodes = 1;
		numNodesVisited = 1;
	#endif

	while (!openList->isEmpty()) {

		#ifdef DEBUG_MOVE_MAP
			if (debugMoveMap) {
				File* pathDebugFile = new File;
					pathDebugFile->create("mm.dbg");
				writeDebug(pathDebugFile);
				pathDebugFile->close();
				delete pathDebugFile;
				pathDebugFile = NULL;
			}
		#endif

		#ifdef DEBUG_PATH
			if (topOpenNodes < openList->getNumItems())
				topOpenNodes = openList->getNumItems();
		#endif

		//----------------------
		// Grab the best node...
		PQNode bestPQNode;
		openList->remove(bestPQNode);
		bestRow = bestPQNode.row;
		bestCol = bestPQNode.col;
		MoveMapNodePtr bestMapNode = &map[bestPQNode.id];
		bestMapNode->clearFlag(MOVEFLAG_OPEN);
		
		long bestNodeG = bestMapNode->g;

		//----------------------------
		// Now, close the best node...
		bestMapNode->setFlag(MOVEFLAG_CLOSED);
		
		//--------------------------
		// Have we found the goal...
		if (bestMapNode->flags & MOVEFLAG_GOAL) {
			goalFound = true;
			break;
		}
		
		for (long dir = 0; dir < 8; dir++) {
			//------------------------------------------------------------
			// First, make sure this is a legit direction to go. We do NOT
			// want to clip corners, so we'll check that here...
			bool isDiagonalWalk = IsDiagonalStep[dir];
			if (isDiagonalWalk) {
			
				// MINE CHECK should go in these adj tests...
				bool adj1Open = false;
				long adjCellIndex = map[bestPQNode.id].adjCells[StepAdjDir[dir]];
				if (adjCellIndex > -1)
					if ((map[adjCellIndex].flags & MOVEFLAG_MOVER_HERE) == 0)
						adj1Open = (map[adjCellIndex].cost < COST_BLOCKED);

				bool adj2Open = false;
				adjCellIndex = map[bestPQNode.id].adjCells[StepAdjDir[dir + 1]];
				if (adjCellIndex > -1)
					if ((map[adjCellIndex].flags & MOVEFLAG_MOVER_HERE) == 0)
						adj2Open = (map[adjCellIndex].cost < COST_BLOCKED);
					
				if (!adj1Open && !adj2Open)
					continue;

				//if (!adjacentCellOpen(bestPQNode.id, StepAdjDir[dir]) && !adjacentCellOpen(bestPQNode.id, StepAdjDir[dir + 1]))
				//	continue;
			}

			//-------------------------------
			// Now, process this direction...
			long succCellIndex = bestMapNode->adjCells[dir];

			//-----------------------------------------------------------------------------------
			// If we're doing offMapTravel, make sure we aren't going back into an offMap cell...
            
            // sebi: we do not want to address by negative offset
            if(succCellIndex > -1) {
    			if (map[succCellIndex].flags & MOVEFLAG_OFFMAP)
	    			if (cannotEnterOffMap)
		    			if ((map[bestPQNode.id].flags & MOVEFLAG_OFFMAP) == 0)
			    			continue;
            }
			
			//--------------------------------
			// If it's on the map, check it...
			if (succCellIndex > -1) {
				if (!inBounds(mapRowTable[succCellIndex], mapColTable[succCellIndex]))
					continue;
				MoveMapNodePtr succMapNode = &map[succCellIndex];
				if (succMapNode->cost < COST_BLOCKED) {
					if (succMapNode->hPrime == HPRIME_NOT_CALCED)
						succMapNode->hPrime = calcHPrime(mapRowTable[succCellIndex/*bestPQNode.id*/], mapColTable[succCellIndex/*bestPQNode.id*/]);

					if (succMapNode->hPrime < MaxHPrime) {

						#ifdef DEBUG_PATH
							numNodesVisited++;
						#endif
				
						//--------------------------------------------------
						// How can we get back to BESTNODE from SUCCESSOR...
						char dirToParent = reverseShift[dir];

						//----------------------------------------------------
						// What's our cost to go from START to this SUCCESSOR?
						long cost = succMapNode->cost;
						//------------------------------------
						// Diagonal movement is more costly...
						gosASSERT(cost > 0);
						if (isDiagonalWalk)
							cost += (cost / 2);
						gosASSERT(cost > 0);
						
						long succNodeG = bestNodeG + cost;

						if (succMapNode->flags & MOVEFLAG_OPEN) {
							//----------------------------------------------
							// This node is already in the OPEN queue to be
							// be processed. Let's check if we have a better
							// path thru this route...
							if (succNodeG < succMapNode->g) {
								//----------------------------
								// This new path is cheaper...
								succMapNode->parent = dirToParent;
								succMapNode->g = succNodeG;
								succMapNode->fPrime = succNodeG + succMapNode->hPrime;
								long openIndex = openList->find(succCellIndex);
								if (!openIndex) {
									char s[128];
									sprintf(s, "MoveMap.calcPath: Cannot find movemap node [%d, %d] for change\n", succCellIndex, dir);
#ifdef USE_OBJECTS
									DebugOpenList(s);
#endif
									gosASSERT(openIndex != 0);
									}
								else
									openList->change(openIndex, succMapNode->fPrime);
							}
							}
						else if (succMapNode->flags & MOVEFLAG_CLOSED) {
							//-------------------------------------------------
							// This path may be better than this node's current
							// path. If so, we may have to propogate thru...
							if (succNodeG < succMapNode->g) {
								//----------------------------
								// This new path is cheaper...
								succMapNode->parent = dirToParent;

								//------------------------------------------
								// Now, we have to propogate the new cost...
								propogateCost(succCellIndex, cost, bestNodeG);
							}
							}
						else {
							//-------------------------------------------------
							// This node is neither OPEN nor CLOSED, so toss it
							// into the OPEN list...
							succMapNode->parent = dirToParent;
							succMapNode->g = succNodeG;
							succMapNode->fPrime = succNodeG + succMapNode->hPrime;
							PQNode succPQNode;
							succPQNode.key = succMapNode->fPrime;
							succPQNode.id = succCellIndex;
							succPQNode.row = mapRowTable[succCellIndex];
							succPQNode.col = mapColTable[succCellIndex];
#ifdef _DEBUG
							long insertErr = 
#endif
								openList->insert(succPQNode);
							gosASSERT(insertErr == NO_ERR);
							succMapNode->setFlag(MOVEFLAG_OPEN);
						}
					}
				}
			}
		}
	}


	#ifdef DEBUG_MOVE_MAP
		if (debugMoveMap) {
			File* pathDebugFile = new File;
			pathDebugFile->create("mm.dbg");
			writeDebug(pathDebugFile);
			pathDebugFile->close();
			delete pathDebugFile;
			pathDebugFile = NULL;
		}
	#endif
	
	if (goalFound) {
		//-------------------------------------------
		// First, let's count how long the path is...
		long curRow = goalCell[0] = (long)bestRow;
		long curCol = goalCell[1] = (long)bestCol;
		long numCells = 0;
		while ((curRow != startR) || (curCol != startC)) {
			numCells += 1;
			long cellOffsetIndex = (map[mapRowStartTable[curRow] + curCol].parent << 1);
			//if ((cellOffsetIndex < 0) || (cellOffsetIndex > 14))
			//	OutputDebugString("PathFinder: whoops\n");
			curRow += cellShift[cellOffsetIndex++];
			curCol += cellShift[cellOffsetIndex];
		}

		//---------------------------------------------------------------
		// If our goal is a door, the path will be one step longer, since
		// we need to walk "thru" the door...
		if (doorDirection != -1)
			numCells++;

		#ifdef _DEBUG
		if (numCells > MAX_STEPS_PER_MOVEPATH) {
			File* pathDebugFile = new File;
			pathDebugFile->create("longpath.dbg");
			char s[512];
			sprintf(s, "Path Too Long: %d steps (Max = %d) (please save longpath.dbg file:)", numCells, MAX_STEPS_PER_MOVEPATH);
			pathDebugFile->writeString(s);
			pathDebugFile->writeString("\n\n");
			writeDebug(pathDebugFile);
			pathDebugFile->close();
			delete pathDebugFile;
			pathDebugFile = NULL;
			gosASSERT(numCells <= MAX_STEPS_PER_MOVEPATH);
		}
		#endif
		//-----------------------------
		// Now, let's build the path...

		//-------------------------------
		// Grab the path and return it...
		path->init();
		if (numCells) {
			#ifdef _DEBUG			
			long maxSteps = 
			#endif

			path->init(numCells);

			#ifdef _DEBUG
			if (maxSteps > -1) {
				File* pathDebugFile = new File;
				pathDebugFile->create("longpath.dbg");
				char s[512];
				sprintf(s, "New Longest Path: %d steps (Max = %d)\n\n", numCells, MAX_STEPS_PER_MOVEPATH);
				pathDebugFile->writeString(s);
				writeDebug(pathDebugFile);
				pathDebugFile->close();
				delete pathDebugFile;
				pathDebugFile = NULL;
			}
			#endif

			path->target = target;
			path->cost = map[mapRowStartTable[bestRow] + bestCol].g;
			curRow = (long)bestRow;
			curCol = (long)bestCol;
			long curCell = numCells;
			if (doorDirection == -1) {
				if (goalWorldPos) {
					goalWorldPos->x = (float)(ULc + bestCol) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
					goalWorldPos->y = (Terrain::worldUnitsMapSide / 2) - ((float)(ULr + bestRow) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
					goalWorldPos->z = (float)0; // How do we get the elevation for this point? Do we care?
					path->goal = *goalWorldPos;
					}
				else
					path->goal = goal;
				}
			else {
				//--------------------------------------------------
				// It's a door, so it's the last on the path list...
				curCell--;
				path->setDirection(curCell, /*reverseShift[*/doorDirection * 2/*]*/);
				long doorR = bestRow + adjTile[doorDirection][0];
				long doorC = bestCol + adjTile[doorDirection][1];

				goalCell[0] = ULr + doorR;
				goalCell[1] = ULc + doorC;
				Stuff::Vector3D stepDest;
				stepDest.x = (float)(ULc + doorC) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
				stepDest.y = (Terrain::worldUnitsMapSide / 2) - ((float)(ULr + doorR) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
				stepDest.z = (float)0; // How do we get the elevation for this point? Do we care?
				path->setDestination(curCell, stepDest);
				path->setDistanceToGoal(curCell, 0.0);
				path->setCell(curCell, goalCell[0], goalCell[1]);
				path->goal = stepDest;
				if (goalWorldPos) {
					//--------------------------------------------------------------------
					// We didn't know the exact goal world pos coming into this, since are
					// goal was an area door....
					*goalWorldPos = stepDest;
				}
			}
			
			//---------------------------------------------------------
			// We need to set this table up our first time thru here...
			static bool setTable = false;
			if (!setTable) {
				float cellLength = Terrain::worldUnitsPerCell * metersPerWorldUnit;
				for (long i = 0; i < NUM_CELL_OFFSETS; i++) {
					float distance = agsqrt( cellShift[i * 2], cellShift[i * 2 + 1] ) * cellLength;
					cellShiftDistance[i] = distance;
				}
				setTable = true;
			}
			
			while ((curRow != startR) || (curCol != startC)) {
				curCell--;
				long parent = reverseShift[map[mapRowStartTable[curRow] + curCol].parent];
				if (parent > 7)
					path->setDirection(curCell, parent);
				else
					path->setDirection(curCell, parent);

				Stuff::Vector3D stepDest;
				long cell[2];
				cell[0] = ULr + curRow;
				cell[1] = ULc + curCol;
				stepDest.x = (float)(cell[1]) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
				stepDest.y = (Terrain::worldUnitsMapSide / 2) - ((float)(cell[0]) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
				stepDest.z = (float)0; // How do we get the elevation for this point? Do we care?
				if (curCell == (numCells - 1))
					path->setDistanceToGoal(curCell, 0.0);
				else {
					long tempDir = path->getDirection(curCell + 1);
					float tempDist = path->getDistanceToGoal(curCell + 1);
					path->setDistanceToGoal(curCell, cellShiftDistance[tempDir] + tempDist);
				}
				path->setDestination(curCell, stepDest);
				path->setCell(curCell, cell[0], cell[1]);
				path->stepList[curCell].area = GlobalMoveMap[moveLevel]->calcArea(cell[0], cell[1]);

				map[curRow * maxWidth + curCol].setFlag(MOVEFLAG_STEP);
				long cellOffsetIndex = map[mapRowStartTable[curRow] + curCol].parent << 1;
				curRow += cellShift[cellOffsetIndex++];
				curCol += cellShift[cellOffsetIndex];
				//calcAdjNode(curRow, curCol, map[curRow * maxCellWidth + curCol].parent);
			}

			if (thruAreas[1] != -1)
				if (thruAreas[1] != path->stepList[path->numSteps - 1].area) {
					for (long i = 0; i < path->numSteps; i++) {
						if (path->stepList[i].area == thruAreas[1]) {
							path->numStepsWhenNotPaused = path->numSteps = i + 1;
							goalCell[0] = path->stepList[i].cell[0];
							goalCell[1] = path->stepList[i].cell[1];
							land->cellToWorld(goalCell[0], goalCell[1], *goalWorldPos);
							break;
						}
					}
				}
			
			#ifdef BLOCKED_PATH_TEST
				//---------------------------------------------------------------------
				// Let's test every path point to make sure it is not a blocked cell...
				for (long i = 0; i < path->numSteps; i++) {
					long tile[2], cell[2];
					GameMap->worldToMapPos(path->stepList[i].destination, tile[0], tile[1], cell[0], cell[1]);
					bool cellPassable = GameMap->cellPassable(tile[0], tile[0], cell[0], cell[1]);
					if (!cellPassable)
						cellPassable = GameMap->cellPassable(tile[0], tile[0], cell[0], cell[1]);
					Assert(cellPassable || (i < 3), 0, " Bad Path Point ");
				}
			#endif
		}

		#ifdef TIME_PATH
			QueryPerformanceCounter(calcStop);
			calcTime = float(calcStop.LowPart - calcStart.LowPart) / float(countsPerSecond.LowPart);
			if (debugger && (scenarioTime > 0.0)) {
				char s[50];
				sprintf(s, "path calc: %.4f\n", calcTime);
				OutputDebugString(s);
				debugger->print(s);
			}
		#endif

		//------------------------------------------------------------------------------------
		// If we're starting on the goal cell, set the cost to 1 as an indicator that there is
		// a path--it's just that we were already there!
		if (numCells == 0)
			path->cost = 1;

		return(path->numSteps);
	}

	#ifdef TIME_PATH
		QueryPerformanceCounter(calcStop);
		calcTime = float(calcStop.LowPart - calcStart.LowPart) / float(countsPerSecond.LowPart);
		if (debugger && (scenarioTime > 0.0)) {
			char s[50];
			sprintf(s, "path calc: %.4f", calcTime);
			debugger->print(s);
		}
	#endif
	
	return(0);
}

//---------------------------------------------------------------------------

long MoveMap::calcPathJUMP (MovePathPtr path, Stuff::Vector3D* goalWorldPos, long* goalCell) {

	#ifdef TIME_PATH
		L_INTEGER calcStart, calcStop;
		QueryPerformanceCounter(calcStart);
	#endif

	//------------------------------------------------------------------
	// Clear the start cell, in case we're starting in a blocked cell...
	//map[startR * maxCellWidth + startC].cost = clearCost;

	//-------------------------------------------------------------------
	// If the start tile is blocked, let's clear it so we may move off of
	// it, at least. Change the blocked tiles to just barely passable...
/*	long startULr = startR - 1;
	long startULc = startC - 1;
	for (long cellR = 0; cellR < MAPCELL_DIM; cellR++)
		for (long cellC = 0; cellC < MAPCELL_DIM; cellC++) {
			long curCellR = startULr + cellR;
			long curCellC = startULc + cellC;
			long mapIndex = curCellR * maxCellWidth + curCellC;
			if ((curCellR >= 0) && (curCellR < cellHeight) && (curCellC >= 0) && (curCellC < cellWidth))
				if (map[mapIndex].cost >= COST_BLOCKED)
					map[mapIndex].cost = COST_BLOCKED - 1;
		}
*/

	if ((goalR < 0) || (goalR >= height) || (goalC < 0) || (goalC >= width)) {
		Stuff::Vector3D p;
		p.x = (float)(goalC) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
		p.y = (Terrain::worldUnitsMapSide / 2) - ((float)(goalR) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
		p.z = (float)0; // How do we get the elevation for this point? Do we care?
		char msg[200];
		sprintf(msg, " Bad Move Goal: %d [%d(%d), %d(%d)], (%.2f, %.2f, %.2f)", DebugMovePathType, goalR, height, goalC, width, p.x, p.y, p.z);
		gosASSERT((goalR >= 0) && (goalR < height) && (goalC >= 0) && (goalC < width));
	}

	//------------------------------------------------------------------
	// Let's use their hPrime as a barrier for cutting off the search...
	MaxHPrime = calcHPrime(startR, startC) * 2.5;
	if (MaxHPrime < 500)
		MaxHPrime = 500;
	
	//-----------------------------------------------
	// If we haven't already, create the OPEN list...
	if (!openList) {
		openList = new PriorityQueue;
		gosASSERT(openList != NULL);
		openList->init(5000);
	}
		
	long curCol = startC;
	long curRow = startR;
	
	MoveMapNodePtr curMapNode = &map[curRow * maxWidth + curCol];
	curMapNode->g = 0;
	if (!ZeroHPrime)
		curMapNode->hPrime = calcHPrime(curRow, curCol);
	curMapNode->fPrime = curMapNode->hPrime;

	//-----------------------------------------------
	// Put the START vertex on the empty OPEN list...
	PQNode initialVertex;
	initialVertex.key = curMapNode->fPrime;
	initialVertex.id = curRow * MAX_MAPWIDTH + curCol;
	initialVertex.row = curRow;
	initialVertex.col = curCol;
	openList->clear();
#ifdef _DEBUG
	long insertErr = 
#endif
		openList->insert(initialVertex);
	gosASSERT(insertErr == NO_ERR);
	curMapNode->setFlag(MOVEFLAG_OPEN);

	bool goalFound = false;
	long bestRow = -1;
	long bestCol = -1;

	#ifdef DEBUG_PATH
		topOpenNodes = 1;
		numNodesVisited = 1;
	#endif

	while (!openList->isEmpty()) {

		#ifdef DEBUG_MOVE_MAP
			if (debugMoveMap) {
				File* pathDebugFile = new File;
					pathDebugFile->create("mm.dbg");
				writeDebug(pathDebugFile);
				pathDebugFile->close();
				delete pathDebugFile;
				pathDebugFile = NULL;
			}
		#endif

		#ifdef DEBUG_PATH
			if (topOpenNodes < openList->getNumItems())
				topOpenNodes = openList->getNumItems();
		#endif

		//----------------------
		// Grab the best node...
		PQNode bestPQNode;
		openList->remove(bestPQNode);
		bestRow = bestPQNode.row;
		bestCol = bestPQNode.col;
		MoveMapNodePtr bestMapNode = &map[bestRow * maxWidth + bestCol];
		bestMapNode->clearFlag(MOVEFLAG_OPEN);
		
		long bestNodeG = bestMapNode->g;

		//----------------------------
		// Now, close the best node...
		bestMapNode->setFlag(MOVEFLAG_CLOSED);
		
		//--------------------------
		// Have we found the goal...
		if (bestMapNode->flags & MOVEFLAG_GOAL) {
			goalFound = true;
			break;
		}
		
		long cellOffsetIndex = 0;
		for (long dir = 0; dir < numOffsets; dir++) {
			//------------------------------------------------------------
			// First, make sure this is a legit direction to go. We do NOT
			// want to clip corners, so we'll check that here...
			bool isDiagonalWalk = IsDiagonalStep[dir];
				if (isDiagonalWalk) {
					if (!adjacentCellOpenJUMP(bestRow, bestCol, StepAdjDir[dir]) && !adjacentCellOpenJUMP(bestRow, bestCol, StepAdjDir[dir + 1])) {
						cellOffsetIndex += 2;
						continue;
					}
				}

			//------------------------------------------------------------
			// Now, process this direction. First, calc the cell to check,
			// offset from the current cell...
			long succRow = bestRow + cellShift[cellOffsetIndex++];
			long succCol = bestCol + cellShift[cellOffsetIndex++];
			//--------------------------------
			// If it's on the map, check it...
			if (inMapBounds(succRow, succCol, height, width)) {

				MoveMapNodePtr succMapNode = &map[succRow * maxWidth + succCol];
				if (succMapNode->cost < COST_BLOCKED) {
					if (succMapNode->hPrime == HPRIME_NOT_CALCED)
						succMapNode->hPrime = calcHPrime(succRow, succCol);

					if (succMapNode->hPrime < MaxHPrime) {

						#ifdef DEBUG_PATH
							numNodesVisited++;
						#endif
				
						//--------------------------------------------------
						// How can we get back to BESTNODE from SUCCESSOR...
						char dirToParent = reverseShift[dir];

						//----------------------------------------------------
						// What's our cost to go from START to this SUCCESSOR?
						bool jumping = false;
						long cost = succMapNode->cost;
						//------------------------------------
						// Diagonal movement is more costly...
						gosASSERT(cost > 0);
						if (dir > 7) {
							jumping = true;
							if (JumpOnBlocked)
								cost = jumpCost;
							else
								cost += jumpCost;
							}
						else {
							if (isDiagonalWalk)
								cost += (cost / 2);
						}
						gosASSERT(cost > 0);
						
						long succNodeG = bestNodeG + cost;

						if (succMapNode->flags & MOVEFLAG_OPEN) {
							//----------------------------------------------
							// This node is already in the OPEN queue to be
							// be processed. Let's check if we have a better
							// path thru this route...
							if (succNodeG < succMapNode->g) {
								//----------------------------
								// This new path is cheaper...
								succMapNode->parent = dirToParent;
								succMapNode->g = succNodeG;
								succMapNode->fPrime = succNodeG + succMapNode->hPrime;
								long openIndex = openList->find(succRow * MAX_MAPWIDTH + succCol);
								if (!openIndex) {
									char s[128];
									sprintf(s, "MoveMap.calcPath: Cannot find movemap node [%d, %d, %d, %d] for change\n", succRow, succCol, succRow * MAX_MAPWIDTH + succCol, dir);
#ifdef USE_OBJECTS
									DebugOpenList(s);
#endif
									gosASSERT(openIndex != 0);
									}
								else
									openList->change(openIndex, succMapNode->fPrime);
							}
							}
						else if (succMapNode->flags & MOVEFLAG_CLOSED) {
							//-------------------------------------------------
							// This path may be better than this node's current
							// path. If so, we may have to propogate thru...
							if (succNodeG < succMapNode->g) {
								//----------------------------
								// This new path is cheaper...
								succMapNode->parent = dirToParent;

								//------------------------------------------
								// Now, we have to propogate the new cost...
								propogateCostJUMP(succRow, succCol, cost, bestNodeG);
							}
							}
						else {
							//-------------------------------------------------
							// This node is neither OPEN nor CLOSED, so toss it
							// into the OPEN list...
							succMapNode->parent = dirToParent;
							succMapNode->g = succNodeG;
							succMapNode->fPrime = succNodeG + succMapNode->hPrime;
							PQNode succPQNode;
							succPQNode.key = succMapNode->fPrime;
							succPQNode.id = succRow * MAX_MAPWIDTH + succCol;
							succPQNode.row = succRow;
							succPQNode.col = succCol;
#ifdef _DEBUG
							long insertErr = 
#endif
								openList->insert(succPQNode);
							gosASSERT(insertErr == NO_ERR);
							succMapNode->setFlag(MOVEFLAG_OPEN);
						}
					}
				}
			}
		}
	}


	#ifdef DEBUG_MOVE_MAP
		if (debugMoveMap) {
			File* pathDebugFile = new File;
			pathDebugFile->create("mm.dbg");
			writeDebug(pathDebugFile);
			pathDebugFile->close();
			delete pathDebugFile;
			pathDebugFile = NULL;
		}
	#endif

#if 0		//REdo when bridges are done
	if (ClearBridgeTiles) {
		overlayWeightTable[OVERLAY_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
	}
#endif

	if (goalFound) {
		//-------------------------------------------
		// First, let's count how long the path is...
		long curRow = goalCell[0] = (long)bestRow;
		long curCol = goalCell[1] = (long)bestCol;
		long numCells = 0;
		while ((curRow != startR) || (curCol != startC)) {
			numCells += 1;
			long cellOffsetIndex = map[curRow * maxWidth + curCol].parent * 2;
			//if ((cellOffsetIndex < 0) || (cellOffsetIndex > 14))
			//	OutputDebugString("PathFinder: whoops\n");
			curRow += cellShift[cellOffsetIndex++];
			curCol += cellShift[cellOffsetIndex];
		}
		//---------------------------------------------------------------
		// If our goal is a door, the path will be one step longer, since
		// we need to walk "thru" the door...
		if (doorDirection != -1)
			numCells++;

#ifdef _DEBUG
		if (numCells > MAX_STEPS_PER_MOVEPATH) {
			File* pathDebugFile = new File;
			pathDebugFile->create("longpath.dbg");
			char s[512];
			sprintf(s, "Path Too Long: %d steps (Max = %d) (please save longpath.dbg file:)", numCells, MAX_STEPS_PER_MOVEPATH);
			pathDebugFile->writeString(s);
			pathDebugFile->writeString("\n\n");
			writeDebug(pathDebugFile);
			pathDebugFile->close();
			delete pathDebugFile;
			pathDebugFile = NULL;
			gosASSERT(numCells <= MAX_STEPS_PER_MOVEPATH);
		}
#endif
		//-----------------------------
		// Now, let's build the path...

		//-------------------------------
		// Grab the path and return it...
		path->init();
		if (numCells) {
#ifdef _DEBUG			
			long maxSteps = 
#endif

			path->init(numCells);

#ifdef _DEBUG
			if (maxSteps > -1) {
				File* pathDebugFile = new File;
				pathDebugFile->create("longpath.dbg");
				char s[512];
				sprintf(s, "New Longest Path: %d steps (Max = %d)\n\n", numCells, MAX_STEPS_PER_MOVEPATH);
				pathDebugFile->writeString(s);
				writeDebug(pathDebugFile);
				pathDebugFile->close();
				delete pathDebugFile;
				pathDebugFile = NULL;
			}
#endif
			path->target = target;
			path->cost = map[bestRow * maxWidth + bestCol].g;
			curRow = (long)bestRow;
			curCol = (long)bestCol;
			long curCell = numCells;
			if (doorDirection == -1) {
				if (goalWorldPos) {
					goalWorldPos->x = (float)(ULc + bestCol) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
					goalWorldPos->y = (Terrain::worldUnitsMapSide / 2) - ((float)(ULr + bestRow) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
					goalWorldPos->z = (float)0; // How do we get the elevation for this point? Do we care?
					path->goal = *goalWorldPos;
					}
				else
					path->goal = goal;
				}
			else {
				//--------------------------------------------------
				// It's a door, so it's the last on the path list...
				curCell--;
				path->setDirection(curCell, /*reverseShift[*/doorDirection * 2/*]*/);
				long doorR = bestRow + adjTile[doorDirection][0];
				long doorC = bestCol + adjTile[doorDirection][1];

				goalCell[0] = ULr + doorR;
				goalCell[1] = ULc + doorC;
				Stuff::Vector3D stepDest;
				stepDest.x = (float)(ULc + doorC) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
				stepDest.y = (Terrain::worldUnitsMapSide / 2) - ((float)(ULr + doorR) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
				stepDest.z = (float)0; // How do we get the elevation for this point? Do we care?
				path->setDestination(curCell, stepDest);
				path->setDistanceToGoal(curCell, 0.0);
				path->setCell(curCell, goalCell[0], goalCell[1]);
				path->goal = stepDest;
				if (goalWorldPos) {
					//--------------------------------------------------------------------
					// We didn't know the exact goal world pos coming into this, since are
					// goal was an area door....
					*goalWorldPos = stepDest;
				}
			}
			
			//---------------------------------------------------------
			// We need to set this table up our first time thru here...
			static bool setTable = false;
			if (!setTable) {
				float cellLength = Terrain::worldUnitsPerCell * metersPerWorldUnit;
				for (long i = 0; i < NUM_CELL_OFFSETS; i++) {
					float distance = agsqrt( cellShift[i * 2], cellShift[i * 2 + 1] ) * cellLength;
					cellShiftDistance[i] = distance;
				}
				setTable = true;
			}
			
			while ((curRow != startR) || (curCol != startC)) {
				curCell--;
				long parent = reverseShift[map[curRow * maxWidth + curCol].parent];
				if (parent > 7)
					path->setDirection(curCell, parent);
				else
					path->setDirection(curCell, parent);

				Stuff::Vector3D stepDest;
				long cell[2];
				cell[0] = ULr + curRow;
				cell[1] = ULc + curCol;
				stepDest.x = (float)(cell[1]) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
				stepDest.y = (Terrain::worldUnitsMapSide / 2) - ((float)(cell[0]) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
				stepDest.z = (float)0; // How do we get the elevation for this point? Do we care?
				if (curCell == (numCells - 1))
					path->setDistanceToGoal(curCell, 0.0);
				else {
					long tempDir = path->getDirection(curCell + 1);
					float tempDist = path->getDistanceToGoal(curCell + 1);
					path->setDistanceToGoal(curCell, cellShiftDistance[tempDir] + tempDist);
				}
				path->setDestination(curCell, stepDest);
				path->setCell(curCell, cell[0], cell[1]);

				map[curRow * maxWidth + curCol].setFlag(MOVEFLAG_STEP);
				long cellOffsetIndex = map[curRow * maxWidth + curCol].parent << 1;
				curRow += cellShift[cellOffsetIndex++];
				curCol += cellShift[cellOffsetIndex];
				//calcAdjNode(curRow, curCol, map[curRow * maxCellWidth + curCol].parent);
			}
			
			#ifdef BLOCKED_PATH_TEST
				//---------------------------------------------------------------------
				// Let's test every path point to make sure it is not a blocked cell...
				for (long i = 0; i < path->numSteps; i++) {
					long tile[2], cell[2];
					GameMap->worldToMapPos(path->stepList[i].destination, tile[0], tile[1], cell[0], cell[1]);
					bool cellPassable = GameMap->cellPassable(tile[0], tile[0], cell[0], cell[1]);
					if (!cellPassable)
						cellPassable = GameMap->cellPassable(tile[0], tile[0], cell[0], cell[1]);
					Assert(cellPassable || (i < 3), 0, " Bad Path Point ");
				}
			#endif
		}

		#ifdef TIME_PATH
			QueryPerformanceCounter(calcStop);
			calcTime = float(calcStop.LowPart - calcStart.LowPart) / float(countsPerSecond.LowPart);
			if (debugger && (scenarioTime > 0.0)) {
				char s[50];
				sprintf(s, "path calc: %.4f\n", calcTime);
				OutputDebugString(s);
				debugger->print(s);
			}
		#endif

		return(path->numSteps);
	}

	#ifdef TIME_PATH
		QueryPerformanceCounter(calcStop);
		calcTime = float(calcStop.LowPart - calcStart.LowPart) / float(countsPerSecond.LowPart);
		if (debugger && (scenarioTime > 0.0)) {
			char s[50];
			sprintf(s, "path calc: %.4f", calcTime);
			debugger->print(s);
		}
	#endif
	
	return(0);
}

//---------------------------------------------------------------------------

long MoveMap::calcEscapePath (MovePathPtr path, Stuff::Vector3D* goalWorldPos, long* goalCell) {

	#ifdef TIME_PATH
		L_INTEGER calcStart, calcStop;
		QueryPerformanceCounter(calcStart);
	#endif

	//------------------------------------------------------------------
	// Clear the start cell, in case we're starting in a blocked cell...
	//map[startR * maxCellWidth + startC].cost = clearCost;

	//-------------------------------------------------------------------
	// If the start tile is blocked, let's clear it so we may move off of
	// it, at least. Change the blocked tiles to just barely passable...
/*	long startULr = startR - 1;
	long startULc = startC - 1;
	for (long cellR = 0; cellR < MAPCELL_DIM; cellR++)
		for (long cellC = 0; cellC < MAPCELL_DIM; cellC++) {
			long curCellR = startULr + cellR;
			long curCellC = startULc + cellC;
			long mapIndex = curCellR * maxCellWidth + curCellC;
			if ((curCellR >= 0) && (curCellR < cellHeight) && (curCellC >= 0) && (curCellC < cellWidth))
				if (map[mapIndex].cost >= COST_BLOCKED)
					map[mapIndex].cost = COST_BLOCKED - 1;
		}
*/

/*
	if ((goalR < 0) || (goalR >= cellHeight) || (goalC < 0) || (goalC >= cellWidth)) {
		Stuff::Vector3D p;
		p.x = (float)(goalC) * MetersPerCell + MetersPerCell / 2 - Terrain::worldUnitsMapSide / 2;
		p.y = (Terrain::worldUnitsMapSide / 2) - ((float)(goalR) * MetersPerCell) - MetersPerCell / 2;
		p.z = (float)0; // How do we get the elevation for this point? Do we care?
		char msg[200];
		sprintf(msg, " Bad Move Goal: %d [%d(%d), %d(%d)], (%.2f, %.2f, %.2f)", DebugMovePathType, goalR, cellHeight, goalC, cellWidth, p.x, p.y, p.z);
		Fatal(0, msg);
	}
*/

	//------------------------------------------------------------------
	// Let's use their hPrime as a barrier for cutting off the search...
	MaxHPrime = 500; //float2short(calcHPrime(startR, startC) * 2.5);
	if (MaxHPrime < 500)
		MaxHPrime = 500;
	
	//-----------------------------------------------
	// If we haven't already, create the OPEN list...
	if (!openList) {
		openList = new PriorityQueue;
		gosASSERT(openList != NULL);
		openList->init(5000);
	}
		
	long curCol = startC;
	long curRow = startR;
	
	MoveMapNodePtr curMapNode = &map[curRow * maxWidth + curCol];
	curMapNode->g = 0;
	curMapNode->hPrime = 10; //calcHPrime(curRow, curCol);
	curMapNode->fPrime = curMapNode->hPrime;

	//-----------------------------------------------
	// Put the START vertex on the empty OPEN list...
	PQNode initialVertex;
	initialVertex.key = curMapNode->fPrime;
	initialVertex.id = curRow * MAX_MAPWIDTH + curCol;
	initialVertex.row = curRow;
	initialVertex.col = curCol;
	openList->clear();
#ifdef _DEBUG
	long insertErr = 
#endif
		openList->insert(initialVertex);
	gosASSERT(insertErr == NO_ERR);
	curMapNode->setFlag(MOVEFLAG_OPEN);

	bool goalFound = false;
	long bestRow = -1;
	long bestCol = -1;

	#ifdef DEBUG_PATH
		topOpenNodes = 1;
		numNodesVisited = 1;
	#endif

	while (!openList->isEmpty()) {

		#ifdef DEBUG_MOVE_MAP
			if (debugMoveMap) {
				File* pathDebugFile = new File;
					pathDebugFile->create("mm.dbg");
				writeDebug(pathDebugFile);
				pathDebugFile->close();
				delete pathDebugFile;
				pathDebugFile = NULL;
			}
		#endif

		#ifdef DEBUG_PATH
			if (topOpenNodes < openList->getNumItems())
				topOpenNodes = openList->getNumItems();
		#endif

		//----------------------
		// Grab the best node...
		PQNode bestPQNode;
		openList->remove(bestPQNode);
		bestRow = bestPQNode.row;
		bestCol = bestPQNode.col;
		MoveMapNodePtr bestMapNode = &map[bestRow * maxWidth + bestCol];
		bestMapNode->clearFlag(MOVEFLAG_OPEN);
		
		long bestNodeG = bestMapNode->g;

		//----------------------------
		// Now, close the best node...
		bestMapNode->setFlag(MOVEFLAG_CLOSED);
		
		//--------------------------
		// Have we found the goal...
		if (bestMapNode->flags & MOVEFLAG_GOAL) {
			goalFound = true;
			break;
		}
		
		long cellOffsetIndex = 0;
		for (long dir = 0; dir < numOffsets; dir++) {
			//------------------------------------------------------------
			// First, make sure this is a legit direction to go. We do NOT
			// want to clip corners, so we'll check that here...
			bool isDiagonalWalk = IsDiagonalStep[dir];
			if (isDiagonalWalk) {
				if (!adjacentCellOpenJUMP(bestRow, bestCol, StepAdjDir[dir]) && !adjacentCellOpenJUMP(bestRow, bestCol, StepAdjDir[dir + 1])) {
					cellOffsetIndex += 2;
					continue;
				}
			}

			//------------------------------------------------------------
			// Now, process this direction. First, calc the cell to check,
			// offset from the current cell...
			long succRow = bestRow + cellShift[cellOffsetIndex++];
			long succCol = bestCol + cellShift[cellOffsetIndex++];
			//--------------------------------
			// If it's on the map, check it...
			if (inMapBounds(succRow, succCol, height, width)) {

				MoveMapNodePtr succMapNode = &map[succRow * maxWidth + succCol];
				if (succMapNode->cost < COST_BLOCKED) {
					if (succMapNode->hPrime == HPRIME_NOT_CALCED)
						succMapNode->hPrime = 10; //calcHPrime(succRow, succCol);

					if (succMapNode->hPrime < MaxHPrime) {

						#ifdef DEBUG_PATH
							numNodesVisited++;
						#endif
				
						//--------------------------------------------------
						// How can we get back to BESTNODE from SUCCESSOR...
						char dirToParent = reverseShift[dir];

						//----------------------------------------------------
						// What's our cost to go from START to this SUCCESSOR?
						bool jumping = false;
						long cost = succMapNode->cost;
						//------------------------------------
						// Diagonal movement is more costly...
						if (dir > 7) {
							jumping = true;
							if (JumpOnBlocked)
								cost = jumpCost;
							else
								cost += jumpCost;
							}
						else {
							if (isDiagonalWalk)
								cost += (cost / 2);
						}
						gosASSERT(cost > 0);
						
						long succNodeG = bestNodeG + cost;

						if (succMapNode->flags & MOVEFLAG_OPEN) {
							//----------------------------------------------
							// This node is already in the OPEN queue to be
							// be processed. Let's check if we have a better
							// path thru this route...
							if (succNodeG < succMapNode->g) {
								//----------------------------
								// This new path is cheaper...
								succMapNode->parent = dirToParent;
								succMapNode->g = succNodeG;
								succMapNode->fPrime = succNodeG + succMapNode->hPrime;
								long openIndex = openList->find(succRow * MAX_MAPWIDTH + succCol);
								if (!openIndex) {
									char s[128];
									sprintf(s, "MoveMap.calcEscapePath: Cannot find movemap node [%d, %d, %d, %d] for change\n", succRow, succCol, succRow * MAX_MAPWIDTH + succCol, dir);
#ifdef USE_OBJECTS
									DebugOpenList(s);
#endif
									gosASSERT(openIndex != 0);
									}
								else
									openList->change(openIndex, succMapNode->fPrime);
							}
							}
						else if (succMapNode->flags & MOVEFLAG_CLOSED) {
							//-------------------------------------------------
							// This path may be better than this node's current
							// path. If so, we may have to propogate thru...
							if (succNodeG < succMapNode->g) {
								//----------------------------
								// This new path is cheaper...
								succMapNode->parent = dirToParent;

								//------------------------------------------
								// Now, we have to propogate the new cost...
								propogateCostJUMP(succRow, succCol, cost, bestNodeG);
							}
							}
						else {
							//-------------------------------------------------
							// This node is neither OPEN nor CLOSED, so toss it
							// into the OPEN list...
							succMapNode->parent = dirToParent;
							succMapNode->g = succNodeG;
							succMapNode->fPrime = succNodeG + succMapNode->hPrime;
							PQNode succPQNode;
							succPQNode.key = succMapNode->fPrime;
							succPQNode.id = succRow * MAX_MAPWIDTH + succCol;
							succPQNode.row = succRow;
							succPQNode.col = succCol;
#ifdef _DEBUG
							long insertErr = 
#endif
								openList->insert(succPQNode);
							gosASSERT(insertErr == NO_ERR);
							succMapNode->setFlag(MOVEFLAG_OPEN);
						}
					}
				}
			}
		}
	}


	#ifdef DEBUG_MOVE_MAP
		if (debugMoveMap) {
			File* pathDebugFile = new File;
			pathDebugFile->create("mm.dbg");
			writeDebug(pathDebugFile);
			pathDebugFile->close();
			delete pathDebugFile;
			pathDebugFile = NULL;
		}
	#endif

#if 0	//Redo when bridges are done
	if (ClearBridgeTiles) {
		overlayWeightTable[OVERLAY_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_NS * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 1] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 4] += COST_BLOCKED;
		overlayWeightTable[OVERLAY_RAILROAD_WATER_BRIDGE_EW * MAPCELL_DIM * MAPCELL_DIM + 7] += COST_BLOCKED;
	}
#endif

	if (goalFound) {
		//-------------------------------------------
		// First, let's count how long the path is...
		long curRow = goalCell[0] = (long)bestRow;
		long curCol = goalCell[1] = (long)bestCol;
		long numCells = 0;
		while ((curRow != startR) || (curCol != startC)) {
			numCells += 1;
			long cellOffsetIndex = map[curRow * maxWidth + curCol].parent * 2;
			//if ((cellOffsetIndex < 0) || (cellOffsetIndex > 14))
			//	OutputDebugString("PathFinder: whoops\n");
			curRow += cellShift[cellOffsetIndex++];
			curCol += cellShift[cellOffsetIndex];
		}
		//---------------------------------------------------------------
		// If our goal is a door, the path will be one step longer, since
		// we need to walk "thru" the door...
		if (doorDirection != -1)
			numCells++;

#ifdef _DEBUG
		if (numCells > MAX_STEPS_PER_MOVEPATH) {
			File* pathDebugFile = new File;
			pathDebugFile->create("longpath.dbg");
			char s[512];
			sprintf(s, "Path Too Long: %d steps (Max = %d) (please save longpath.dbg file:)", numCells, MAX_STEPS_PER_MOVEPATH);
			pathDebugFile->writeString(s);
			pathDebugFile->writeString("\n\n");
			writeDebug(pathDebugFile);
			pathDebugFile->close();
			delete pathDebugFile;
			pathDebugFile = NULL;
			gosASSERT(numCells <= MAX_STEPS_PER_MOVEPATH);
		}
#endif
		//-----------------------------
		// Now, let's build the path...

		//-------------------------------
		// Grab the path and return it...
		path->init();
		if (numCells) {
			
#ifdef _DEBUG			
			long maxSteps = path->init(numCells);

			if (maxSteps > -1) {
				File* pathDebugFile = new File;
				pathDebugFile->create("longpath.dbg");
				char s[512];
				sprintf(s, "New Longest Path: %d steps (Max = %d)\n\n", numCells, MAX_STEPS_PER_MOVEPATH);
				pathDebugFile->writeString(s);
				writeDebug(pathDebugFile);
				pathDebugFile->close();
				delete pathDebugFile;
				pathDebugFile = NULL;
			}
#endif
			path->target = target;
			path->cost = map[bestRow * maxWidth + bestCol].g;
			curRow = (long)bestRow;
			curCol = (long)bestCol;
			long curCell = numCells;
			if (doorDirection == -1) {
				if (goalWorldPos) {
					goalWorldPos->x = (float)(ULc + bestCol) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
					goalWorldPos->y = (Terrain::worldUnitsMapSide / 2) - ((float)(ULr + bestRow) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
					goalWorldPos->z = (float)0; // How do we get the elevation for this point? Do we care?
					path->goal = *goalWorldPos;
					}
				else
					path->goal = goal;
				}
			else {
				//--------------------------------------------------
				// It's a door, so it's the last on the path list...
				curCell--;
				path->setDirection(curCell, /*reverseShift[*/doorDirection * 2/*]*/);
				long doorR = bestRow + adjTile[doorDirection][0];
				long doorC = bestCol + adjTile[doorDirection][1];

				goalCell[0] = ULr + doorR;
				goalCell[1] = ULc + doorC;
				Stuff::Vector3D stepDest;
				stepDest.x = (float)(ULc + doorC) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
				stepDest.y = (Terrain::worldUnitsMapSide / 2) - ((float)(ULr + doorR) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
				stepDest.z = (float)0; // How do we get the elevation for this point? Do we care?
				path->setDestination(curCell, stepDest);
				path->setDistanceToGoal(curCell, 0.0);
				path->setCell(curCell, goalCell[0], goalCell[1]);
				path->goal = stepDest;
				if (goalWorldPos) {
					//--------------------------------------------------------------------
					// We didn't know the exact goal world pos coming into this, since are
					// goal was an area door....
					*goalWorldPos = stepDest;
				}
			}
			
			//---------------------------------------------------------
			// We need to set this table up our first time thru here...
			static bool setTable = false;
			if (!setTable) {
				float cellLength = Terrain::worldUnitsPerCell * metersPerWorldUnit;
				for (long i = 0; i < NUM_CELL_OFFSETS; i++) {
					float distance = agsqrt( cellShift[i * 2], cellShift[i * 2 + 1] ) * cellLength;
					cellShiftDistance[i] = distance;
				}
				setTable = true;
			}
			
			while ((curRow != startR) || (curCol != startC)) {
				curCell--;
				long parent = reverseShift[map[curRow * maxWidth + curCol].parent];
				if (parent > 7)
					path->setDirection(curCell, parent);
				else
					path->setDirection(curCell, parent);

				Stuff::Vector3D stepDest;
				long cell[2];
				cell[0] = ULr + curRow;
				cell[1] = ULc + curCol;
				stepDest.x = (float)(cell[1]) * Terrain::worldUnitsPerCell + Terrain::worldUnitsPerCell / 2 - Terrain::worldUnitsMapSide / 2;
				stepDest.y = (Terrain::worldUnitsMapSide / 2) - ((float)(cell[0]) * Terrain::worldUnitsPerCell) - Terrain::worldUnitsPerCell / 2;
				stepDest.z = (float)0; // How do we get the elevation for this point? Do we care?
				if (curCell == (numCells - 1) && (parent <= 7))
					path->setDistanceToGoal(curCell, 0.0);
				else
					path->setDistanceToGoal(curCell, cellShiftDistance[path->getDirection(curCell + 1)] + path->getDistanceToGoal(curCell + 1));
				path->setDestination(curCell, stepDest);
				path->setCell(curCell, cell[0], cell[1]);

				map[curRow * maxWidth + curCol].setFlag(MOVEFLAG_STEP);
				long cellOffsetIndex = map[curRow * maxWidth + curCol].parent << 1;
				curRow += cellShift[cellOffsetIndex++];
				curCol += cellShift[cellOffsetIndex];
				//calcAdjNode(curRow, curCol, map[curRow * maxCellWidth + curCol].parent);
			}
			
			#ifdef BLOCKED_PATH_TEST
				//---------------------------------------------------------------------
				// Let's test every path point to make sure it is not a blocked cell...
				for (long i = 0; i < path->numSteps; i++) {
					long tile[2], cell[2];
					GameMap->worldToMapPos(path->stepList[i].destination, tile[0], tile[1], cell[0], cell[1]);
					bool cellPassable = GameMap->cellPassable(tile[0], tile[0], cell[0], cell[1]);
					if (!cellPassable)
						cellPassable = GameMap->cellPassable(tile[0], tile[0], cell[0], cell[1]);
					Assert(cellPassable || (i < 3), 0, " Bad Path Point ");
				}
			#endif
		}

		#ifdef TIME_PATH
			QueryPerformanceCounter(calcStop);
			calcTime = float(calcStop.LowPart - calcStart.LowPart) / float(countsPerSecond.LowPart);
			if (debugger && (scenarioTime > 0.0)) {
				char s[50];
				sprintf(s, "path calc: %.4f\n", calcTime);
				OutputDebugString(s);
				debugger->print(s);
			}
		#endif

		return(path->numSteps);
	}

	#ifdef TIME_PATH
		QueryPerformanceCounter(calcStop);
		calcTime = float(calcStop.LowPart - calcStart.LowPart) / float(countsPerSecond.LowPart);
		if (debugger && (scenarioTime > 0.0)) {
			char s[50];
			sprintf(s, "path calc: %.4f", calcTime);
			debugger->print(s);
		}
	#endif
	
	return(0);
}

//---------------------------------------------------------------------------

void MoveMap::writeDebug (File* debugFile) {

	char outString[512];
	sprintf(outString, "Time = %.6f\n\n", calcTime);
	debugFile->writeString(outString);
	sprintf(outString, "Start = (%d, %d)\n", startR, startC);
	debugFile->writeString(outString);
	sprintf(outString, "Goal = (%d, %d)\n", goalR, goalC);
	debugFile->writeString(outString);
	strcpy(outString, "\n");
	debugFile->writeString(outString);
	
	debugFile->writeString("PARENT:\n");
	debugFile->writeString("-------\n");
	for (long r = 0; r < height; r++) {
		outString[0] = '\0';
		char numStr[10];
		for (long c = 0; c < width; c++) {
			/*if ((goalR == r) && (goalC == c))
				sprintf(numStr, "G");
			else*/
			if ((startR == r) && (startC == c))
				sprintf(numStr, "S");
			else if (map[r * maxWidth + c].parent == -1)
				sprintf(numStr, ".");
			else if (map[r * maxWidth + c].flags & MOVEFLAG_STEP)
				sprintf(numStr, "X");
			else			
				sprintf(numStr, "%d", map[r * maxWidth + c].parent);
			strcat(outString, numStr);
		}
		strcat(outString, "\n");
		debugFile->writeString(outString);
	}
	debugFile->writeString("\n");

	debugFile->writeString("MAP:\n");
	debugFile->writeString("-------\n");
	for (int r = 0; r < height; r++) {
		outString[0] = '\0';
		char numStr[10];
		for (long c = 0; c < width; c++) {
			if ((goalR == r) && (goalC == c))
				sprintf(numStr, "G");
			else if ((startR == r) && (startC == c))
				sprintf(numStr, "S");
			else if (map[r * maxWidth + c].cost == clearCost)
				sprintf(numStr, ".");
			else if (map[r * maxWidth + c].cost >= COST_BLOCKED)
				sprintf(numStr, " ");
			else if (map[r * maxWidth + c].cost < 256)
				sprintf(numStr, "%x", map[r * maxWidth + c].cost);
			strcat(outString, numStr);
		}
		strcat(outString, "\n");
		debugFile->writeString(outString);
	}
	debugFile->writeString("\n");

	debugFile->writeString("PATH:\n");
	debugFile->writeString("-------\n");
	for (int r = 0; r < height; r++) {
		outString[0] = '\0';
		char numStr[10];
		for (long c = 0; c < width; c++) {
			if ((goalR == r) && (goalC == c))
				sprintf(numStr, "G");
			else if ((startR == r) && (startC == c))
				sprintf(numStr, "S");
			else if (map[r * maxWidth + c].flags & MOVEFLAG_STEP)
				sprintf(numStr, "*");
			else if (map[r * maxWidth + c].cost == clearCost)
				sprintf(numStr, ".");
			else if (map[r * maxWidth + c].cost >= COST_BLOCKED)
				sprintf(numStr, " ");
			else			
				sprintf(numStr, "%d", map[r * maxWidth + c].cost);
			strcat(outString, numStr);
		}
		strcat(outString, "\n");
		debugFile->writeString(outString);
	}
	debugFile->writeString("\n");

}

//---------------------------------------------------------------------------

void MoveMap::destroy (void) {

	if (map) 
	{
		systemHeap->Free(map);
		map = NULL;
	}

	if (mapRowStartTable)
	{
		systemHeap->Free(mapRowStartTable);
		mapRowStartTable = NULL;
	}

	if (mapRowTable)
	{
		systemHeap->Free(mapRowTable);
		mapRowTable = NULL;
	}

	if (mapColTable)
	{
		systemHeap->Free(mapColTable);
		mapColTable = NULL;
	}

}

//***************************************************************************

