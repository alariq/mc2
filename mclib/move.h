//***************************************************************************
//
//	Move.h -- Defs for Movement/Pathfinding routines
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MOVE_H
#define MOVE_H

//***************************************************************************

//--------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DMOVEMGR_H
#include"dmovemgr.h"
#endif

#ifndef TERRAIN_H
#include"terrain.h"
#endif

#ifndef DOBJCLASS_H
#include"dobjclass.h"
#endif

#ifndef DOBJBLCK_H
#include"dobjblck.h"
#endif

#ifndef DGAMELOG_H
#include"dgamelog.h"
#endif

#include<gameos.hpp>

//***************************************************************************

#define	USE_FORESTS			0

#define	MAX_GLOBALMAP_DOORS		10000
#define	MAX_GLOBALMAP_AREAS		10000

#define	NUM_EXTRA_DOOR_LINKS	2

//#define	INNER_SPHERE	TEAM1
//#define	CLANS			TEAM2
//#define	ALLIED			TEAM3
//#define NEUTRAL			0xff		//NEUTRALS are NOT part of the eight Teams!
									//These are inanimate things like buildings, trees, rocks, etc.

#define	SIMPLE_PATHMAP	0
#define	SECTOR_PATHMAP	1

typedef enum {
	DOOR_OFFSET_START,
	DOOR_OFFSET_GOAL,
	NUM_DOOR_OFFSETS
} GlobalMapDoorOffset;

#define	SECTOR_DIM				30

#define	MOVEMAP_WIDTH			12			// Used outside of move file
#define	MOVEMAP_MAX_CELLOBJECTS	4

#define	TILE_PRESERVED_FLAG		0x8000

#define	NUM_DIRECTIONS			8
#define	HPRIME_NOT_CALCED		0xFFFFFFFF

#define	MOVEFLAG_OPEN			1
#define	MOVEFLAG_CLOSED			2
#define	MOVEFLAG_STEP			4
#define	MOVEFLAG_GOAL			8
#define	MOVEFLAG_MOVER_HERE		16
#define	MOVEFLAG_GOAL_BLOCKED	32
#define	MOVEFLAG_OFFMAP			64

#define	CELLSTATE_OPEN			0
#define	CELLSTATE_BLOCKED		1

#define	FOREST_INCREMENT		2

//----------------
// cell move costs
#define	COST_CLEAR				100
#define	COST_BLOCKED			10000

#define	OVERLAYINDEX_NULL		41
#define	IS_DIRTROAD_OVERLAY		0
#define	IS_PAVEDROAD_OVERLAY	1
#define IS_RAILROAD_OVERLAY		2

#define	SCENARIOMAP_MAX_ELEVATION			63
#define	SCENARIOMAP_MAX_TILETYPE			127
#define	SCENARIOMAP_MAX_OVERLAYTYPE			127
//#define	SCENARIOMAP_LAST_BASETYPE_INDEX		63

//TileType and OverlayType now in terrain.h!
typedef enum {
	OVERLAY_WEIGHT_VEHICLE,
	OVERLAY_WEIGHT_MECH,
	OVERLAY_WEIGHT_OTHER1,
	OVERLAY_WEIGHT_OTHER2,
	OVERLAY_WEIGHT_OTHER3,
	NUM_OVERLAY_WEIGHT_CLASSES
} OverlayWeightClass;

//------------------
// class Definitions
#define	MOVEPARAM_NONE						0
#define	MOVEPARAM_FACE_TARGET				1
#define	MOVEPARAM_DIR_OFFSET				2
#define	MOVEPARAM_BACK_UP					4
#define	MOVEPARAM_SOMEWHERE_ELSE			8
#define	MOVEPARAM_MOVING_MOVERS				16
#define	MOVEPARAM_MYSTERY_PARAM				32
#define	MOVEPARAM_STATIONARY_MOVERS			64
#define	MOVEPARAM_AVOID_PATHLOCKS			128
#define	MOVEPARAM_INIT						256
#define	MOVEPARAM_RECALC					512
#define	MOVEPARAM_STEP_TOWARD_TARGET		1024
#define	MOVEPARAM_PLAYER					2048
#define	MOVEPARAM_RADIO_RESULT				4096
#define	MOVEPARAM_ESCAPE_TILE				8192
#define MOVEPARAM_STEP_ADJACENT_TARGET		16384
#define	MOVEPARAM_SWEEP_MINES				32768
#define	MOVEPARAM_FOLLOW_ROADS				65536
#define	MOVEPARAM_WATER_SHALLOW				131072
#define	MOVEPARAM_WATER_DEEP				262144
#define	MOVEPARAM_RANDOM_OPTIMAL			524288
#define	MOVEPARAM_JUMP						1048576

#define	TACORDER_PARAM_NONE					0
#define	TACORDER_PARAM_RUN					(1<<0)
#define	TACORDER_PARAM_WAIT					(1<<1)
#define	TACORDER_PARAM_FACE_OBJECT			(1<<2)
#define	TACORDER_PARAM_LAY_MINES			(1<<3)
#define	TACORDER_PARAM_PURSUE				(1<<4)
#define	TACORDER_PARAM_OBLITERATE			(1<<5)
#define	TACORDER_PARAM_ESCAPE_TILE			(1<<6)
#define	TACORDER_PARAM_SCAN					(1<<7)
#define	TACORDER_PARAM_ATTACK_DFA			(1<<8)
#define	TACORDER_PARAM_ATTACK_RAMMING		(1<<9)
#define	TACORDER_PARAM_RANGE_RAMMING		(1<<10)
#define	TACORDER_PARAM_RANGE_LONGEST		(1<<11)
#define	TACORDER_PARAM_RANGE_OPTIMAL		(1<<12)
#define	TACORDER_PARAM_RANGE_SHORT			(1<<13)
#define	TACORDER_PARAM_RANGE_MEDIUM			(1<<14)
#define	TACORDER_PARAM_RANGE_LONG			(1<<15)
#define	TACORDER_PARAM_AIM_HEAD				(1<<16)
#define	TACORDER_PARAM_AIM_LEG				(1<<17)
#define	TACORDER_PARAM_AIM_ARM				(1<<18)
#define	TACORDER_PARAM_DONT_SET_ORDER		(1<<19)
#define	TACORDER_PARAM_JUMP					(1<<20)
#define	TACORDER_PARAM_DONT_KEEP_MOVING		(1<<21)
#define	TACORDER_PARAM_TACTIC_FLANK_RIGHT	(1<<22) //4194304
#define	TACORDER_PARAM_TACTIC_FLANK_LEFT	(1<<23) //8388608
#define	TACORDER_PARAM_TACTIC_FLANK_REAR	(1<<24) //16777216
#define	TACORDER_PARAM_TACTIC_STOP_AND_FIRE	(1<<25) //33554432
#define	TACORDER_PARAM_TACTIC_TURRET		(1<<26) //67108864
#define	TACORDER_PARAM_TACTIC_JOUST			(1<<27) //134217728

#define	TACORDER_ATTACK_MASK				0x00000300
#define	TACORDER_RANGE_MASK					0x0000FC00
#define	TACORDER_AIM_MASK					0x00070000
#define	TACORDER_TACTIC_MASK				0x0FC00000

#define	RELPOS_FLAG_ABS					1
#define	RELPOS_FLAG_PASSABLE_START		2
#define	RELPOS_FLAG_PASSABLE_GOAL		4

#define MAX_GLOBAL_PATH					50

#define	NUM_MOVE_LEVELS					2

#define	MAX_WALL_OBJECTS				2000
#define	MAX_GATES_OBJECT				1000
#define	MAX_CELL_COORDS					5000
#define	FILL_STACK_SIZE					810000


//***************************************************************************


#pragma pack(1)

//---------------------------------------------------------------------------

#define	CELL_INFO_FLAG_PASSABLE			1
#define	CELL_INFO_FLAG_LOS				2
#define	CELL_INFO_FLAG_WATER_DEEP		4
#define	CELL_INFO_FLAG_WATER_SHALLOW	8

#define	SPECIAL_NONE					0
#define	SPECIAL_WALL					1
#define	SPECIAL_GATE					2
#define	SPECIAL_LAND_BRIDGE				3
#define	SPECIAL_FOREST					4

#define	MINE_NONE						0
#define	MINE_INTACT						1
#define	MINE_EXPLODED					2

typedef struct _ScenarioMapCellInfo {
	unsigned char		terrain;
	unsigned char		overlay;
	bool				road;
	bool				gate;
	bool				forest;
	char				mine;
	unsigned char		specialType;
	short				specialID;
	bool				passable;
	int                 lineOfSight;
} MissionMapCellInfo;

//---------------------------------------------------------------------------

#define	MAPCELL_TERRAIN_SHIFT			0
#define	MAPCELL_TERRAIN_MASK			0x0000000F

#define	MAPCELL_OVERLAY_SHIFT			4
#define	MAPCELL_OVERLAY_MASK			0x00000030

#define	MAPCELL_MOVER_SHIFT				6
#define	MAPCELL_MOVER_MASK				0x00000040

#define	MAPCELL_UNUSED1_SHIFT			7				// THIS BIT AVAILABLE!
#define	MAPCELL_UNUSED1_MASK			0x00000080

#define	MAPCELL_GATE_SHIFT				8
#define	MAPCELL_GATE_MASK				0x00000100

#define	MAPCELL_OFFMAP_SHIFT			9
#define	MAPCELL_OFFMAP_MASK				0x00000200

#define	MAPCELL_PASSABLE_SHIFT			10
#define	MAPCELL_PASSABLE_MASK			0x00000400

#define	MAPCELL_PATHLOCK_SHIFT			11
#define	MAPCELL_PATHLOCK_MASK			0x00001800
#define	MAPCELL_PATHLOCK_BASE			0x00000800

#define	MAPCELL_MINE_SHIFT				13
#define	MAPCELL_MINE_MASK				0x0001E000

#define	MAPCELL_PRESERVED_SHIFT			17
#define	MAPCELL_PRESERVED_MASK			0x00020000

#define	MAPCELL_HEIGHT_SHIFT			18
#define	MAPCELL_HEIGHT_MASK				0x003C0000

#define	MAPCELL_DEBUG_SHIFT				22
#define	MAPCELL_DEBUG_MASK				0x00C00000

#define	MAPCELL_WALL_SHIFT				24
#define	MAPCELL_WALL_MASK				0x01000000

#define	MAPCELL_ROAD_SHIFT				25
#define	MAPCELL_ROAD_MASK				0x02000000

#define	MAPCELL_SHALLOW_SHIFT			26
#define	MAPCELL_SHALLOW_MASK			0x04000000

#define	MAPCELL_DEEP_SHIFT				27
#define	MAPCELL_DEEP_MASK				0x08000000

#define	MAPCELL_FOREST_SHIFT			28
#define	MAPCELL_FOREST_MASK				0x10000000

//------------------------------------------------------
// The following are used ONLY when building map data...
#define	MAPCELL_BUILD_WALL_SHIFT		29
#define	MAPCELL_BUILD_WALL_MASK			0x20000000

#define	MAPCELL_BUILD_GATE_SHIFT		30
#define	MAPCELL_BUILD_GATE_MASK			0x40000000

#define	MAPCELL_BUILD_LAND_BRIDGE_SHIFT	11
#define	MAPCELL_BUILD_LAND_BRIDGE_MASK	0x00000800

#define	MAPCELL_BUILD_SPECIAL_MASK		0x60000800

#define	MAPCELL_BUILD_NOT_SET_SHIFT		31
#define	MAPCELL_BUILD_NOT_SET_MASK		0x80000000

typedef struct _MapCell {
	unsigned int data;

	unsigned int getTerrain (void) {
		return((data & MAPCELL_TERRAIN_MASK) >> MAPCELL_TERRAIN_SHIFT);
	}

	void setTerrain (unsigned int terrain) {
		data &= (MAPCELL_TERRAIN_MASK ^ 0xFFFFFFFF);
		data |= (terrain << MAPCELL_TERRAIN_SHIFT);
	}

	unsigned int getOverlay (void) {
		return((data & MAPCELL_OVERLAY_MASK) >> MAPCELL_OVERLAY_SHIFT);
	}

	void setOverlay (unsigned int overlay) {
		data &= (MAPCELL_OVERLAY_MASK ^ 0xFFFFFFFF);
		data |= (overlay << MAPCELL_OVERLAY_SHIFT);
	}

	bool getMover (void) {
		return((data & MAPCELL_MOVER_MASK) ? true : false);
	}

	void setMover (bool moverHere ) {
		data &= (MAPCELL_MOVER_MASK ^ 0xFFFFFFFF);
		if (moverHere)
			data |= MAPCELL_MOVER_MASK;
	}

	unsigned int getGate (void) {
		return((data & MAPCELL_GATE_MASK) >> MAPCELL_GATE_SHIFT);
	}

	void setGate (unsigned int gate) {
		data &= (MAPCELL_GATE_MASK ^ 0xFFFFFFFF);
		data |= (gate << MAPCELL_GATE_SHIFT);
	}
	
	bool getPassable (void) {
		return((data & MAPCELL_PASSABLE_MASK) ? true : false);
	}

	void setPassable (bool passable) {
		data &= (MAPCELL_PASSABLE_MASK ^ 0xFFFFFFFF);
		if (passable)
			data |= MAPCELL_PASSABLE_MASK;
	}

	bool getPathlock (DWORD level) {
		return((data & (MAPCELL_PATHLOCK_BASE << level)) ? true : false);
	}

	void setPathlock (DWORD level, bool pathlock) {
		DWORD bit = MAPCELL_PATHLOCK_BASE << level;
		data &= (bit ^ 0xFFFFFFFF);
		if (pathlock)
			data |= bit;
	}

	unsigned int getMine (void) {
		return((data & MAPCELL_MINE_MASK) >> MAPCELL_MINE_SHIFT);
	}

	void setMine (unsigned int mine) {
		data &= (MAPCELL_MINE_MASK ^ 0xFFFFFFFF);
		data |= (mine << MAPCELL_MINE_SHIFT);
	}

	bool getPreserved (void) {
		return((data & MAPCELL_PRESERVED_MASK) ? true : false);
	}

	void setPreserved (bool preserved) {
		data &= (MAPCELL_PRESERVED_MASK ^ 0xFFFFFFFF);
		if (preserved)
			data |= MAPCELL_PRESERVED_MASK;
	}

	void setLocalHeight (DWORD localElevation) {
		data &= (MAPCELL_HEIGHT_MASK ^ 0xFFFFFFFF);
		data |= (localElevation << MAPCELL_HEIGHT_SHIFT);
	}
	
	DWORD getLocalHeight (void) {
		return((data & MAPCELL_HEIGHT_MASK) >> MAPCELL_HEIGHT_SHIFT);
	}

	void setDebug (DWORD value) {
		data &= (MAPCELL_DEBUG_MASK ^ 0xFFFFFFFF);
		data |= (value << MAPCELL_DEBUG_SHIFT);
	}

	DWORD getDebug (void) {
		return((data & MAPCELL_DEBUG_MASK) >> MAPCELL_DEBUG_SHIFT);
	}

	bool getWall (void) {
		return((data & MAPCELL_WALL_MASK) ? true : false);
	}

	void setWall (bool wallHere) {
		data &= (MAPCELL_WALL_MASK ^ 0xFFFFFFFF);
		if (wallHere)
			data |= MAPCELL_WALL_MASK;
	}

	bool getRoad (void) {
		return((data & MAPCELL_ROAD_MASK) ? true : false);
	}

	void setRoad (bool roadHere) {
		data &= (MAPCELL_ROAD_MASK ^ 0xFFFFFFFF);
		if (roadHere)
			data |= MAPCELL_ROAD_MASK;
	}

	bool getShallowWater (void) {
		return((data & MAPCELL_SHALLOW_MASK) ? true : false);
	}

	void setShallowWater (bool shallowWaterHere) {
		data &= (MAPCELL_SHALLOW_MASK ^ 0xFFFFFFFF);
		if (shallowWaterHere)
			data |= MAPCELL_SHALLOW_MASK;
	}

	bool getDeepWater (void) {
		return((data & MAPCELL_DEEP_MASK) ? true : false);
	}

	void setDeepWater (bool deepWaterHere) {
		data &= (MAPCELL_DEEP_MASK ^ 0xFFFFFFFF);
		if (deepWaterHere)
			data |= MAPCELL_DEEP_MASK;
	}

	bool getBuildGate (void) {
		return((data & MAPCELL_BUILD_GATE_MASK) ? true : false);
	}

	void setBuildGate (bool set) {
		data &= (MAPCELL_BUILD_GATE_MASK ^ 0xFFFFFFFF);
		if (set)
			data |= MAPCELL_BUILD_GATE_MASK;
	}

	bool getBuildWall (void) {
		return((data & MAPCELL_BUILD_WALL_MASK) ? true : false);
	}

	void setBuildWall (bool set) {
		data &= (MAPCELL_BUILD_WALL_MASK ^ 0xFFFFFFFF);
		if (set)
			data |= MAPCELL_BUILD_WALL_MASK;
	}

	bool getBuildLandBridge (void) {
		return((data & MAPCELL_BUILD_LAND_BRIDGE_MASK) ? true : false);
	}

	void setBuildLandBridge (bool set) {
		data &= (MAPCELL_BUILD_LAND_BRIDGE_MASK ^ 0xFFFFFFFF);
		if (set)
			data |= MAPCELL_BUILD_LAND_BRIDGE_MASK;
	}

	bool getForest (void) {
		return((data & MAPCELL_FOREST_MASK) ? true : false);
	}

	void setForest (bool set) {
		data &= (MAPCELL_FOREST_MASK ^ 0xFFFFFFFF);
		if (set)
			data |= MAPCELL_FOREST_MASK;
	}

	bool getOffMap (void) {
		return((data & MAPCELL_OFFMAP_MASK) ? true : false);
	}

	void setOffMap (bool set) {
		data &= (MAPCELL_OFFMAP_MASK ^ 0xFFFFFFFF);
		if (set)
			data |= MAPCELL_OFFMAP_MASK;
	}

	bool getBuildSpecial (void) {
		return((data & MAPCELL_BUILD_SPECIAL_MASK) ? true : false);
	}

	bool getBuildNotSet (void) {
		return((data & MAPCELL_BUILD_NOT_SET_MASK) ? true : false);
	}

	void setBuildNotSet (bool set) {
		data &= (MAPCELL_BUILD_NOT_SET_MASK ^ 0xFFFFFFFF);
		if (set)
			data |= MAPCELL_BUILD_NOT_SET_MASK;
	}

} MapCell;

typedef MapCell* MapCellPtr;

typedef struct _PreservedCell {
	short				row;
	short				col;
	unsigned int        data;
} PreservedCell;

typedef PreservedCell* PreservedCellPtr;

#pragma pack()

//---------------------------------------------------------------------------

extern float VerticesMapSideDivTwo;
extern float MetersMapSideDivTwo;

#define	MAX_DEBUG_CELLS		1000

class MissionMap {

	public:

		MapCellPtr			map;
		long				height;
		long				width;
		long				planet;
		bool				preserveCells;
		long				numPreservedCells;
		PreservedCell		preservedCells[MAX_MOVERS];
		long				numDebugCells;
		long				debugCells[MAX_DEBUG_CELLS][3];

		void				(*placeMoversCallback) (void);
		
	public:

		void* operator new (size_t mySize);
		void operator delete (void* us);
		
		void init (void) {
			map = NULL;
			height = 0;
			width = 0;
			preserveCells = false;
			numPreservedCells = 0;
			placeMoversCallback = NULL;
			numDebugCells = 0;  
		}
		
		MissionMap (void) {
			init();
		}
		
		void destroy (void);

		~MissionMap (void) {
			destroy();
		}

		void init (long h, long w);

		long init (long curHeight, long curWidth, long curPlanet, MissionMapCellInfo* mapData);

		void setPassable (long row, long col, char* footPrint, bool passable);

		long init (PacketFile* packetFile, long whichPacket = 0);

		long initOld (File* mapFile);

		long write (PacketFile* packetFile, long whichPacket = 0);

		unsigned long getTerrain (long row, long col) {
			return(map[row * width + col].getTerrain());
		}

		void setTerrain (long row, long col, unsigned long terrain) {
			map[row * width + col].setTerrain(terrain);
		}

		unsigned long getOverlay (long row, long col) {
			return(map[row * width + col].getOverlay());
		}

		void setOverlay (long row, long col, unsigned long overlay) {
			map[row * width + col].setOverlay(overlay);
		}

		bool getMover (long row, long col) {
			return(map[row * width + col].getMover());
		}

		void setMover (long row, long col, bool moverHere) {
			map[row * width + col].setMover(moverHere);
		}

		unsigned long getGate (long row, long col) {
			return(map[row * width + col].getGate());
		}

		void setGate (long row, long col, unsigned long gate) {
			map[row * width + col].setGate(gate);
		}

		bool getPassable (long row, long col) {
			return(map[row * width + col].getPassable());
		}

		void setPassable (long row, long col, bool passable) {
			map[row * width + col].setPassable(passable);
		}

		bool getPassable (Stuff::Vector3D cellPosition);

		bool getPathlock (long level, long row, long col) {
			return(map[row * width + col].getPathlock(level));
		}

		void setPathlock (long level, long row, long col, bool pathlock) {
			map[row * width + col].setPathlock(level, pathlock);
		}

		unsigned long getMine (long row, long col) {
			return(map[row * width + col].getMine());
		}

		void setMine (long row, long col, unsigned long mine) {
			map[row * width + col].setMine(mine);
		}

		bool getPreserved (long row, long col) {
			return(map[row * width + col].getPreserved());
		}

		void setPreserved (long row, long col, bool preserved) {
			map[row * width + col].setPreserved(preserved);
		}

		void setLocalHeight (long row, long col, DWORD localElevation) {
			map[row * width + col].setLocalHeight(localElevation);
		}

		DWORD getLocalHeight (long row, long col) {
			return(map[row * width + col].getLocalHeight());
		}

		void setCellDebug (long row, long col, DWORD value, DWORD level) {
			if( row >= 0 && row < height && col >= 0 && col < width )
			{
				DWORD curValue = map[row * width + col].getDebug();
				if (curValue && (curValue != value)) {
					for (long i = 0; i < numDebugCells; i++)
						if (debugCells[i][0] == row)
							if (debugCells[i][1] == col) {
								if (debugCells[i][2] < level)
									break;
								return;
							}
				}
				if (value == 0) {
					// remove it...
					for (long i = 0; i < numDebugCells; i++)
						if (debugCells[i][0] == row)
							if (debugCells[i][1] == col) {
								numDebugCells--;
								debugCells[i][0] = debugCells[numDebugCells][0];
								debugCells[i][1] = debugCells[numDebugCells][1];
								debugCells[i][2] = debugCells[numDebugCells][2];
								break;
							}
					map[row * width + col].setDebug(0);
					}
				else {
					if (numDebugCells < MAX_DEBUG_CELLS) {
						debugCells[numDebugCells][0] = row;
						debugCells[numDebugCells][1] = col;
						debugCells[numDebugCells][2] = level;
						numDebugCells++;
						map[row * width + col].setDebug(value);
					}
				}
			}
		}

		void clearCellDebugs (void) {
			for (long i = 0; i < numDebugCells; i++)
				map[debugCells[i][0] * width + debugCells[i][1]].setDebug(0);
			numDebugCells = 0;
		}

		void clearCellDebugs (long level) {
			long i = 0;
			while (i < numDebugCells) {
				if (debugCells[i][2] == level) {
					numDebugCells--;
					map[debugCells[i][0] * width + debugCells[i][1]].setDebug(0);
					debugCells[i][0] = debugCells[numDebugCells][0];
					debugCells[i][1] = debugCells[numDebugCells][1];
					debugCells[i][2] = debugCells[numDebugCells][2];
					}
				else
					i++;
			}
		}

		DWORD getCellDebug (long row, long col) {
			return(map[row * width + col].getDebug());
		}

		bool getWall (long row, long col) {
			return(map[row * width + col].getWall());
		}

		void setWall (long row, long col, bool wallHere) {
			map[row * width + col].setWall(wallHere);
		}

		bool getRoad (long row, long col) {
			return(map[row * width + col].getRoad());
		}

		void setRoad (long row, long col, bool roadHere) {
			map[row * width + col].setRoad(roadHere);
		}

		bool getShallowWater (long row, long col) {
			return(map[row * width + col].getShallowWater());
		}

		void setShallowWater (long row, long col, bool shallowWaterHere) {
			map[row * width + col].setShallowWater(shallowWaterHere);
		}

		bool getDeepWater (long row, long col) {
			return(map[row * width + col].getDeepWater());
		}

		void setDeepWater (long row, long col, bool deepWaterHere) {
			map[row * width + col].setDeepWater(deepWaterHere);
		}

		bool getBuildGate (long row, long col) {
			return(map[row * width + col].getBuildGate());
		}

		void setBuildGate (long row, long col, bool set) {
			map[row * width + col].setBuildGate(set);
		}
		
		bool getBuildWall (long row, long col) {
			return(map[row * width + col].getBuildWall());
		}

		void setBuildWall (long row, long col, bool set) {
			map[row * width + col].setBuildWall(set);
		}

		bool getBuildLandBridge (long row, long col) {
			return(map[row * width + col].getBuildLandBridge());
		}

		void setBuildLandBridge (long row, long col, bool set) {
			map[row * width + col].setBuildLandBridge(set);
		}
		
		bool getForest (long row, long col) {
			return(map[row * width + col].getForest());
		}

		void setForest (long row, long col, bool set) {
			map[row * width + col].setForest(set);
		}

		bool getOffMap (long row, long col) {
			return(map[row * width + col].getOffMap());
		}

		void setOffMap (long row, long col, bool set) {
			map[row * width + col].setOffMap(set);
		}

		bool getBuildSpecial (long row, long col) {
			return(map[row * width + col].getBuildSpecial());
		}

		bool getBuildNotSet (long row, long col) {
			return(map[row * width + col].getBuildNotSet());
		}

		void setBuildNotSet (long row, long col, bool set) {
			map[row * width + col].setBuildNotSet(set);
		}

		long getHeight (void) {
			return(height);
		}
		
		long getWidth (void) {
			return(width);
		}

		bool inBounds (long row, long col) {
			return((row > -1) && (row < height) && (col > -1) && (col < width));
		}

		MapCellPtr getCell (long row, long col) {
			gosASSERT(inBounds(row, col));
			return(&map[row * width + col]);
		}

		void preserveCell (long row, long col) {
			preservedCells[numPreservedCells].data = map[row * width + col].data;
			preservedCells[numPreservedCells].row = row;
			preservedCells[numPreservedCells].col = col;
			map[row * width + col].setPreserved(1);
			numPreservedCells++;
		}

		long getLOF (Stuff::Vector3D position);

		unsigned long cellPassable (Stuff::Vector3D cellPosition);

		void lineOfSensor (Stuff::Vector3D start, Stuff::Vector3D target, long& numBlockingTiles, long& numBlockingObjects);

		long getOverlayWeight (long row, long col, long moverOverlayWeightClass, long moverRelation);

		void spreadState (long r, long c, long radius);

		long placeObject (Stuff::Vector3D position, float radius);

		void placeTerrainObject (long row,
								 long col,
								 long objectClass,
								 long objectTypeID,
								 __int64 footPrint,
								 bool blocksLineOfFire,
								 long mineType);

		void print (char* fileName);
};

typedef MissionMap* MissionMapPtr;

//***************************************************************************


class MovePath;
typedef MovePath* MovePathPtr;


//---------------------------------------------------------------------------
#define	MAX_STEPS_PER_MOVEPATH 200

typedef struct _PathStep {
	short					cell[2];							// obvious
	float					distanceToGoal;						// dist, in meters, to goal from this step
	Stuff::Vector3D			destination;						// world pos of this step
	char					direction;							// 0 thru 7 direction into this step
	short					area;
} PathStep;

typedef PathStep* PathStepPtr;

class MovePath {

	public:
	
		Stuff::Vector3D		goal;								// world pos of path goal
		Stuff::Vector3D		target;								// world pos of object target
		long				numSteps;							// if paused or no steps, == 0, else == numStepsWhenNotPaused
		long				numStepsWhenNotPaused;				// total number of steps
		long				curStep;							// cuurent step we're headed for
		long				cost;								// total cost of path
		PathStep			stepList[MAX_STEPS_PER_MOVEPATH];	// actual steps :)
		bool				marked;								// is it currently marked
		long				globalStep;							// if part of a complex path
		
	public:
	
		void* operator new (size_t mySize);
		void operator delete (void* us);
		
		void init (void) {
			goal.Zero();
			numSteps = 0;
			numStepsWhenNotPaused = 0;
			curStep = 0;
			cost = 0;
			marked = false;
			globalStep = -1;
		}
		
		long init (long numberOfSteps);

		void clear (void);

		void setDirection (long stepNumber, char direction) {
			stepList[stepNumber].direction = direction;
		}

		long getDirection (long stepNumber) {
			return(stepList[stepNumber].direction);
		}

		void setDistanceToGoal (long stepNumber, float distance) {
			stepList[stepNumber].distanceToGoal = distance;
		}

		float getDistanceToGoal (long stepNumber) {
			return(stepList[stepNumber].distanceToGoal);
		}

		void setDestination (long stepNumber, Stuff::Vector3D	v) {
			stepList[stepNumber].destination = v;
		}

		Stuff::Vector3D getDestination (long stepNumber) {
			return(stepList[stepNumber].destination);
		}

		void setCell (long stepNumber, long r, long c) {
			stepList[stepNumber].cell[0] = r;
			stepList[stepNumber].cell[1] = c;
		}

		float getDistanceLeft (Stuff::Vector3D position, long stepNumber = -1);

		void lock (long level, long start, long range, bool setting);

		bool isLocked (long level, long start, long range, bool* reachedEnd = NULL);

		bool isBlocked (long start, long range, bool* reachedEnd = NULL);

		long crossesBridge (long start, long range);

		long crossesCell (long start, long range, long cellR, long cellC);

		long crossesClosedClanGate (long start, long range);

		long crossesClosedISGate (long start, long range);

		long crossesClosedGate (long start, long range);

		MovePath (void) {
			init();
		}
		
		void destroy (void);

		~MovePath (void) {
			destroy();
		}

		void setCurStep (long _curStep) {
			curStep = _curStep;
		}

		long getNumSteps (void);

		long getCost (void) {
			return(cost);
		}
};

//******************************************************************************************


//******************************************************************************************
#define	GLOBAL_FLAG_SPECIAL_IMPOSSIBLE	0x80
#define	GLOBAL_FLAG_SPECIAL_CALC		0x40
#define	GLOBAL_FLAG_NORMAL_OPENS		0x20
#define	GLOBAL_FLAG_NORMAL_CLOSES		0x10

#define	GLOBAL_CONFIDENCE_BAD			0
#define GLOBAL_CONFIDENCE_AT_LEAST		1
#define	GLOBAL_CONFIDENCE_GOOD			2

#define	GLOBALPATH_EXISTS_UNKNOWN		0
#define	GLOBALPATH_EXISTS_TRUE			1
#define	GLOBALPATH_EXISTS_FALSE			2

#pragma pack(1)

typedef struct _DoorLink {
	short					doorIndex;
	char					doorSide;
	int					cost;
	int					openCost;
} DoorLink;

typedef DoorLink* DoorLinkPtr;

typedef struct _GlobalMapDoor {
	//----------------
	// Map layout data
	short					row;
	short					col;
	char					length;					// in cells
	bool					open;
	char					teamID;
	short					area[2];
	short					areaCost[2];
	char					direction[2];
	short					numLinks[2];
	//DoorLinkPtr				links[2];
	DWORD                   links_Legacy32bitPtr[2];
	//------------------
	// Pathfinding  data
	int					cost;
	int					parent;
	int					fromAreaIndex;
	unsigned int	    flags;
	int					g;
	int					hPrime;
	int					fPrime;
} GlobalMapDoor;

typedef GlobalMapDoor* GlobalMapDoorPtr;

typedef struct _DoorInfo {
	short					doorIndex;
	char					doorSide;
} DoorInfo;

typedef DoorInfo* DoorInfoPtr;

typedef enum _AreaType {
	AREA_TYPE_NORMAL,
	AREA_TYPE_WALL,
	AREA_TYPE_GATE,
	AREA_TYPE_LAND_BRIDGE,
	AREA_TYPE_FOREST,
	NUM_AREA_TYPES
} AreaType;

typedef enum _WaterType {
	WATER_TYPE_NONE,
	WATER_TYPE_SHALLOW,
	WATER_TYPE_DEEP,
	NUM_WATER_TYPES
} WaterType;

typedef struct _GlobalMapArea {
	//----------------
	// Map layout data
	short					sectorR;
	short					sectorC;
	//DoorInfoPtr				doors;
	DWORD                   doors_Legacy32bitPtr;
	AreaType				type;
	short					numDoors;
	int					    ownerWID;
	char					teamID;
	bool					offMap;
	bool					open;
    // sebi: 64bit fix 
	//short*					cellsCovered;
	DWORD                   cellsCovered_Legacy32bitPtr;
} GlobalMapArea;

#pragma pack()

typedef GlobalMapArea* GlobalMapAreaPtr;

typedef struct _GlobalPathStep {
	int					startDoor;
	int					thruArea;
	int					goalDoor;
	Stuff::Vector3D			start;			// "start" in this area
	Stuff::Vector3D			goal;			// "goal" in this area
	int					goalCell[2];	// which cell did we actually exit thru
	int					costToGoal;
} GlobalPathStep;

typedef GlobalPathStep* GlobalPathStepPtr;

#define	MAX_SPECIAL_AREAS		1500
#define	MAX_SPECIAL_SUB_AREAS	25
#define	MAX_CELLS_PER_SUB_AREA	49
#define	MAX_OFFMAP_AREAS		60
#define	MAX_GAME_OBJECT_CELLS	64

typedef struct _GameObjectFootPrint {
	short				cellPositionRow;
	short				cellPositionCol;
	short				preNumCells;
	short				numCells;
	short				cells[MAX_GAME_OBJECT_CELLS][2];
} GameObjectFootPrint;

typedef struct _GlobalSpecialAreaInfo {
	unsigned char			type;
	short					numSubAreas;
	short					subAreas[MAX_SPECIAL_SUB_AREAS];
	short					numCells;
	short					cells[MAX_CELLS_PER_SUB_AREA][2];
} GlobalSpecialAreaInfo;

typedef DoorLinkPtr DoorInfoLinksPtr[2];
class GlobalMap {

	public:

		int						height;				// in cells
		int						width;				// in cells
		int						sectorDim;			// in cells
		int						sectorHeight;		// in sectors
		int						sectorWidth;		// in sectors
		int						numAreas;
		int						numDoors;
		int						numDoorInfos;
		int						numDoorLinks;

		short*						areaMap;
		GlobalMapAreaPtr			areas;
        // sebi: moved pointer out of a struct which is loaded from file
        // because pointer size is platform dependent (64 vs. 32bit)
	    short**					    areas_cellsCovered;
	    DoorInfoPtr*                areas_doors;

		GlobalMapDoorPtr			doors;
        DoorInfoLinksPtr*           doors_links;

		DoorInfoPtr					doorInfos;
		DoorLinkPtr					doorLinks;
		GlobalMapDoorPtr			doorBuildList;
        DoorInfoLinksPtr*           doorBuildList_links;
#ifdef USE_PATH_COST_TABLE
		unsigned char*				pathCostTable;
#endif
		unsigned char*				pathExistsTable;

		int						    numSpecialAreas;
		GlobalSpecialAreaInfo*		specialAreas;	// used when building data

		bool						closes;
		bool						opens;

		int						    goalArea;
		int						    goalSector[2];

		int						    startCell[2];
		int						    goalCell[2];

		bool						blank;
		bool						hover;
		bool						useClosedAreas;
		char						moverTeamID;
		bool						badLoad;
		bool						calcedPathCost;

		int						    numOffMapAreas;
		short						offMapAreas[MAX_OFFMAP_AREAS];

		bool (*isGateDisabledCallback) (int objectWID);
		bool (*isGateOpenCallback) (int objectWID);

		static int					minRow;
		static int					maxRow;
		static int					minCol;
		static int					maxCol;

		static GameLogPtr			log;
		static bool					logEnabled;

	public:

		void* operator new (size_t mySize);
		void operator delete (void* us);
		
		void init (void) {
			height = 0;
			width = 0;
			sectorDim = 30;
			sectorHeight = 0;
			sectorWidth = 0;

			numAreas = 0;
			areaMap = NULL;
			areas = NULL;
            areas_cellsCovered = NULL;
            areas_doors = NULL;

			numDoors = 0;
			numDoorInfos = 0;
			numDoorLinks = 0;
			doors = NULL;
            doors_links = NULL;
			doorInfos = NULL;
			doorLinks = NULL;
			doorBuildList = NULL;
			doorBuildList_links = NULL;

			goalSector[0] = goalSector[1] = 0;
			blank = false;
			hover = false;
			useClosedAreas = false;
			badLoad = false;
			calcedPathCost = false;

			startCell[0] = -1;
			startCell[1] = -1;
			goalCell[0] = -1;
			goalCell[1] = -1;

			specialAreas = NULL;
			closes = false;
			opens = false;
			numOffMapAreas = 0;

			log = NULL;
			logEnabled = false;

			isGateDisabledCallback = NULL;
			isGateOpenCallback = NULL;
		}

		GlobalMap (void) {
			init();
		}
		
		void destroy (void);

		~GlobalMap (void) {
			destroy();
		}

		void init (long h, long w);

		long build (MissionMapCellInfo* mapData);

		long init (PacketFilePtr mapFile, long whichPacket = 0);

		long write (PacketFile* packetFile, long whichPacket = 0);

		long setTempArea (long row, long col, long cost);

		bool fillNorthSouthBridgeArea (long row, long col, long area);

		bool fillEastWestBridgeArea (long row, long col, long area);

		bool fillNorthSouthRailroadBridgeArea (long row, long col, long area);

		bool fillEastWestRailroadBridgeArea (long row, long col, long area);

		bool fillSpecialArea (long row, long col, long area, long wallGateID);

		bool fillArea (long row, long col, long area, bool offMap);

		void calcSectorAreas (long sectorR, long sectorC);

		void beginDoorProcessing (void);

		long numAreaDoors (long area);

		void getAreaDoors (long area, DoorInfoPtr dorrList);

		void addDoor (long adjArea, long curArea, long row, long col, long length, long dir);

		void endDoorProcessing (void);

		void calcAreas (void);

		void calcSpecialAreas (MissionMapCellInfo* mapData);

		void calcCellsCovered (void);

		void calcSpecialTypes (void);

		void calcGlobalDoors (void);

		void calcAreaDoors (void);

		long calcLinkCost (long startDoor, long thruArea, long goalDoor);

		void changeAreaLinkCost (long area, long cost);

		void calcDoorLinks (void);

        long getPathCost (int startArea, int goalArea, bool withSpecialAreas, int& confidence, bool calcIt);

#ifdef USE_PATH_COST_TABLE
		void initPathCostTable (void);

		void resetPathCostTable (void);

		void calcPathCostTable (void);

		void setPathCost (long startArea, long goalArea, bool withSpecialAreas, unsigned char cost);

		void setPathFlag (long startArea, long goalArea, unsigned char flag, bool set);

		long getPathFlag (long startArea, long goalArea, unsigned char flag);
#endif
		void clearPathExistsTable (void);

		void setPathExists (long fromArea, long toArea, unsigned char set);

		unsigned char getPathExists (long fromArea, long toArea);

		long exitDirection (long doorIndex, long fromArea);

		void setStartDoor (long startArea);

		void resetStartDoor (long startArea);

		void setAreaTeamID (long area, char teamID);

		void setAreaOwnerWID (long area, long objWID);

		void setGoalDoor (long goalArea);

		void resetGoalDoor (long goalArea);

		long calcHPrime (long area);

		//void propogateCost (long area, long g);
		void propogateCost (long door, long cost, long fromAreaIndex, long g);

		long calcPath (long startArea,
					   long goalArea,
					   GlobalPathStepPtr path,
					   long startRow = -1,
					   long startCol = -1,
					   long goalRow = -1,
					   long goalCol = -1);

		long calcPath (Stuff::Vector3D start, Stuff::Vector3D goal, GlobalPathStepPtr path);

		long calcArea (long row, long col) {
			long areaId = areaMap[row * width + col];
			if (areaId < 0)
				return(-1);
			return(areaId);
		}

		void getDoorTiles (long area, long door, GlobalMapDoorPtr areaDoor);

		void getSectorCoords (long area, long& sectorR, long& sectorC) {
			sectorR = areas[area].sectorR;
			sectorC = areas[area].sectorC;
		}

		Stuff::Vector3D getDoorWorldPos (long area, long door, long* goalCell);

		void openDoor (long door);

		void closeDoor (long door);

		void closeArea (long area);

		void closeArea (long row, long col) {
			long area = calcArea(row, col);
			gosASSERT(area > -1);
			closeArea(area);
		}

		void openArea (long area);

		void openArea (long row, long col) {
			long area = calcArea(row, col);
			gosASSERT(area > -1);
			openArea(area);
		}

		void openOffMapAreas (void) {
			for (long i = 0; i < numOffMapAreas; i++)
				openArea(offMapAreas[i]);
		}

		void closeOffMapAreas (void) {
			for (long i = 0; i < numOffMapAreas; i++)
				closeArea(offMapAreas[i]);
		}

		bool getDoorOpen (long door) {
			return(doors[door].open);
		}

		bool isClosedArea (long area) {
			return(!areas[area].open);
		}

		bool getAdjacentAreaCell (long area, long adjacentArea, long& cellRow, long& cellCol);

		void print (char* fileName);

		static bool toggleLog (void);

		static void writeLog (char* s);
};

typedef GlobalMap* GlobalMapPtr;

//******************************************************************************************

#define	NUM_ADJ_CELLS	8

typedef struct _MoveMapNode {
	short		    adjCells[NUM_ADJ_CELLS];
	int				cost;								// normal cost to travel here, based upon terrain
	int				parent;								// where we came from (parent cell)
	unsigned int    flags;								// CLOSED, OPEN, STEP flags
	int				g;									// known cost from START to this node
	int				hPrime;								// estimated cost from this node to GOAL
	int				fPrime;								// = g + hPrime

	void setFlag (unsigned int flag) {
		flags |= flag;
	}

	void clearFlag (unsigned int flag) {
		flags &= (flag ^ 0xFFFFFFFF);
	}
} MoveMapNode;

typedef MoveMapNode* MoveMapNodePtr;

#define	DISTANCE_TABLE_DIM 80

class MoveMap {

	public:

		int				    ULr;		// upper-left cell row
		int				    ULc;		// upper-left cell col
		int				    width;
		int				    height;
		int				    minRow;
		int				    maxRow;
		int				    minCol;
		int				    maxCol;
		int				    maxWidth;
		int				    maxHeight;
		MoveMapNodePtr		map;
		int*				mapRowStartTable;
		int*				mapRowTable;
		int*				mapColTable;
		int				    moveLevel;
		Stuff::Vector3D		start;
		int				    startR;
		int				    startC;
		Stuff::Vector3D		goal;			// actual world-coord goal
		int				    goalR;			// cell goal row relative to move map
		int				    goalC;			// cell goal col relative to move map
		int				    thruAreas[2];
		int				    door;
		int				    doorSide;
		int				    doorDirection;	// if goal is not a door, set to -1
		Stuff::Vector3D		target;			// actual world-coord target
		int				    clearCost;		// cost, in tenths of secs, to move to clear cell
		int				    jumpCost;		// cost, in tenths of secs, to jump to cell
		int				    numOffsets;		// set by calcMovePath function (don't touch:)
		float				calcTime;
		int*				overlayWeightTable;
		int				    moverWID;
		int				    moverTeamID;
		bool				moverLayingMines;
		bool				moverWithdrawing;
		bool				travelOffMap;
		bool				cannotEnterOffMap;

		void				(*blockedDoorCallback) (int moveLevel, int door, char* openCells);
		void				(*placeStationaryMoversCallback) (MoveMapPtr map);

		static float		distanceFloat[DISTANCE_TABLE_DIM][DISTANCE_TABLE_DIM];
		static int			distanceInt[DISTANCE_TABLE_DIM][DISTANCE_TABLE_DIM];
		static int			forestCost;

	protected:

		bool adjacentCellOpen (long mapCellIndex, long dir);
		bool adjacentCellOpenJUMP (long r, long c, long dir);
		void propogateCost (long mapCellIndex, long cost, long g);
		void propogateCostJUMP (long r, long c, long cost, long g);
		long calcHPrime (long r, long c);
		
	public:

		void* operator new (size_t mySize);
		void operator delete (void* us);
		
		void init (void) {
			ULr = 0;
			ULc = 0;
			maxHeight = 0;
			maxWidth = 0;
			height = 0;
			width = 0;
			minRow = 0;
			maxRow = 0;
			minCol = 0;
			maxCol = 0;
			map = NULL;
			mapRowTable = NULL;
			moveLevel = 0;
			startR = -1;
			startC = -1;
			goalR = -1;
			goalC = -1;
			thruAreas[0] = -1;
			thruAreas[1] = -1;
			goal.Zero();
			door = -1;
			doorSide = -1;
			doorDirection = -1;
			target.x = -999999.0;
			target.y = -999999.0;
			target.z = -999999.0;
			clearCost = 1;
			jumpCost = 0;
			numOffsets = 8;
			calcTime = 0.0;
			travelOffMap = false;
			cannotEnterOffMap = true;
			overlayWeightTable = NULL;
			blockedDoorCallback = NULL;
			placeStationaryMoversCallback = NULL;
		}
		
		MoveMap (void) {
			init();
		}
		
		void destroy (void);

		~MoveMap (void) {
			destroy();
		}

		void init (long h, long w);

		long init (FitIniFile* mapFile);

		long setUp (long ULr,
					long ULc,
					long height,
					long width,
					long moveLevel,
					Stuff::Vector3D* startPos,
					long startRow,
					long startCol,
					Stuff::Vector3D goalPos,
					long goalRow,
					long goalCol,
					long clearCellCost,
					long jumpCellCost,
					long offsets,
					unsigned long params = MOVEPARAM_NONE);

		long setUp (long moveLevel,
					Stuff::Vector3D* startPos,
					long startRow,
					long startCol,
					long thruArea[2],
					long goalDoor,
					Stuff::Vector3D finalGoal,
					long clearCellCost,
					long jumpCellCost,
					long offsets,
					unsigned long params = MOVEPARAM_NONE);

		void clear (void);

		void placeMovers (bool stationaryOnly);

		void setOverlayWeightTable (int* table) {
			overlayWeightTable = table;
		}

		void setStart (Stuff::Vector3D* startPos, long startCellRow, long startCellCol);

		void setGoal (Stuff::Vector3D goalPos, long goalCellRow, long goalCellCol);

		void setGoal (long thruArea, long goalDoor);

		long markGoals (Stuff::Vector3D finalGoal);

		long markEscapeGoals (Stuff::Vector3D finalGoal);

		void setTarget (Stuff::Vector3D targetPos);

		char getCost (long row, long col) {
			return(map[row * width + col].cost);
		}

		void setCost (long row, long col, long newCost);

		void adjustCost (long row, long col, long costAdj) {
			long index = row * width + col;
			long cost = map[index].cost + costAdj;
			if (cost < 1)
				cost = 1;
			map[index].cost = cost;
		}

		void setStart (long row, long col) {
			startR = row;
			startC = col;
		}

		void setClearCost (long cost) {
			clearCost = cost;
		}

		void setJumpCost (long cost, long offsets = 8) {
			jumpCost = cost;
			numOffsets = offsets;
		}

		void setMover (long watchID, long teamID = 0, bool layingMines = false, bool withdrawing = false) {
			moverWID = watchID;
			moverTeamID = teamID;
			moverLayingMines = layingMines;
			moverWithdrawing = withdrawing;
		}

		long calcPath (MovePathPtr path, Stuff::Vector3D* goalWorldPos, long* goalCell);

		long calcPathJUMP (MovePathPtr path, Stuff::Vector3D* goalWorldPos, long* goalCell);

		long calcEscapePath (MovePathPtr path, Stuff::Vector3D* goalWorldPos, long* goalCell);

		float getDistanceFloat (long rowDelta, long colDelta) {
			return(distanceFloat[rowDelta][colDelta]);
		}

		float getDistanceLong (long rowDelta, long colDelta) {
			return(distanceInt[rowDelta][colDelta]);
		}

		void writeDebug (File* debugFile);

		bool inBounds (long row, long col) {
			return((row >= minRow) && (row <= maxRow) && (col >= minCol) && (col <= maxCol));
		}
};

//---------------------------------------------------------------------------

inline void MoveMap::setCost (long row, long col, long newCost) {

	map[row * maxWidth + col].cost = newCost;
}

//---------------------------------------------------------------------------

void SaveMapCells (char* fileName,  long height, long width,  MissionMapCellInfo* mapData);

MissionMapCellInfo* LoadMapCells (char* fileName, long& height, long& width);

void DeleteMapCells (MissionMapCellInfo* mapData);

void MOVE_init (long moveRange);
void MOVE_buildData (long height, long width, MissionMapCellInfo* mapData, long numSpecialAreas, GameObjectFootPrint* specialAreaFootPrints);
long MOVE_saveData (PacketFile* packetFile, long whichPacket = 0);
long MOVE_readData (PacketFile* packetFile, long whichPacket);
void MOVE_cleanup (void);

//long BuildAndSaveMoveData (char* fileName, long height, long width, MissionMapCellInfo* mapData);

//***************************************************************************

extern MissionMapPtr		GameMap;
extern GlobalMapPtr			GlobalMoveMap[3];
extern MoveMapPtr			PathFindMap[2];
extern long					SimpleMovePathRange;

//***************************************************************************

#endif

