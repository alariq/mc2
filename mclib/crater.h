//---------------------------------------------------------------------
//
// crater.h -- Crater/footprint manager for MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CRATER_H
#define CRATER_H
//---------------------------------------------------------------------
// Include Files
#ifndef HEAP_H
#include"heap.h"
#endif

#ifndef PACKET_H
#include"packet.h"
#endif


#include<stuff/stuff.hpp>

//---------------------------------------------------------------------
// Macro Definitions
enum craterTypes
{
	INVALID_CRATER = -1,
	SML_FOOTPRINT = 0,
	AVG_FOOTPRINT,
	BIG_FOOTPRINT,
	ULLER_FOOTPRINT,
	MADCAT_FOOTPRINT,
	LEFT_MASAKARI_FOOTPRINT,
	RIGHT_MASAKARI_FOOTPRINT,
	BUSH_FOOTPRINT,
	NOVACAT_FOOTPRINT,
	TURKINA_FOOTPRINT = 63,
	CRATER_1 = 64,
	CRATER_2,
	CRATER_3,
	CRATER_4,
	MAX_CRATER_SHAPES
};

#define FOOTPRINT_ROTATIONS		16
#define BIG_CRATER_OFFSET		0
#define SMALL_CRATER_OFFSET		1
//---------------------------------------------------------------------
// struct CraterData
typedef struct _CraterData
{
	long			craterShapeId;
	Stuff::Vector3D position[4];
	Stuff::Vector4D screenPos[4];
} CraterData;

typedef CraterData *CraterDataPtr;
//---------------------------------------------------------------------
// class CraterManager
class CraterManager
{
	//Data Members
	//-------------
	protected:
		
		HeapManagerPtr		craterPosHeap;
		UserHeapPtr			craterShpHeap;
		
		unsigned long		craterPosHeapSize;
		unsigned long		craterShpHeapSize;
		
		PacketFilePtr		craterFile;
		
		unsigned long		maxCraters;
		unsigned long		currentCrater;
		CraterDataPtr		craterList;
		long				numCraterTextures;

		DWORD				*craterTextureIndices;
		DWORD				*craterTextureHandles;
				
	//Member Functions
	//-----------------
	protected:
		
	public:
	
		void init (void)
		{
			craterPosHeap = NULL;
			craterShpHeap = NULL;

			craterPosHeapSize = craterShpHeapSize = 0;
			
			currentCrater = maxCraters = 0;
			craterList = NULL;

			numCraterTextures = 0;
			
			craterTextureHandles = NULL;
			craterTextureIndices = NULL;
			
			craterFile = NULL;
		}
		
		CraterManager (void)
		{
			init();
		}
		
		long init (long numCraters, unsigned long craterTypeSize, const char *craterFileName);
		
		~CraterManager (void)
		{
			destroy();
		}
		
		void destroy (void);
		
		long addCrater (long craterType, Stuff::Vector3D &position, float rotation);
		
		long update (void);
		void render (void);
};

//---------------------------------------------------------------------
typedef CraterManager *CraterManagerPtr;
extern CraterManagerPtr craterManager;

//---------------------------------------------------------------------
#endif
