//---------------------------------------------------------------------------
//
//	trigger.h - This file contains the class declaration for TriggerArea
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TRIGGER_H
#define TRIGGER_H

#ifndef TERRAIN_H
#include"terrain.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif

//#ifndef DSTD_H
#include"dstd.h"
//#endif

//---------------------------------------------------------------------------

#define	MAX_TRIGGER_AREAS			16

typedef enum {
	TRIGGER_AREA_NONE,
	TRIGGER_AREA_MOVER,
	TRIGGER_AREA_TEAM,
	TRIGGER_AREA_GROUP,
	TRIGGER_AREA_COMMANDER,
	NUM_TRIGGER_AREA_TYPES
} TriggerAreaType;

typedef struct {
	char	type;
	long	param;
	long	dim[4];
	bool	hit;
} TriggerArea;

class TriggerAreaManager {

	public:

		TriggerArea				triggerAreas[MAX_TRIGGER_AREAS];
		unsigned char			map[MAX_MAP_CELL_WIDTH / 3][MAX_MAP_CELL_WIDTH / 3];
		
	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
			
		void init (void) {
			for (long i = 0; i < MAX_TRIGGER_AREAS; i++)
				triggerAreas[i].type = TRIGGER_AREA_NONE;
			for (long r = 0; r < MAX_MAP_CELL_WIDTH / 3; r++)
				for (long c = 0; c < MAX_MAP_CELL_WIDTH / 3; c++)
					map[r][c] = 0;
		}

		TriggerAreaManager (void) {
			init();
		}

		void destroy (void);

		~TriggerAreaManager (void) {
			destroy();
		}

		long add (long ULrow, long ULcol, long LRrow, long LRcol, long type, long param);

		void remove (long areaHandle);

		void reset (long areaHandle);

		bool isHit (long areaHandle);

		void setHit (MoverPtr mover);
};


//---------------------------------------------------------------------------
#endif

