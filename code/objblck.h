//---------------------------------------------------------------------------
//
// ObjBlck.h -- File contains class definitions for the Terrain Object classes
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef OBJBLCK_H
#define OBJBLCK_H
//---------------------------------------------------------------------------
// Include Files
#ifndef DSTD_H
#include<dstd.h>
#endif

#ifndef DOBJBLCK_H
#include<dobjblck.h>
#endif

#ifndef DOBJTYPE_H
#include<dobjtype.h>
#endif

#ifndef DPACKET_H
#include<dpacket.h>
#endif

#ifndef DHEAP_H
#include<dheap.h>
#endif

#ifndef cVMATH_H
#include<cvmath.h>
#endif

#ifndef DOBJQUE_H
#include<dobjque.h>
#endif
//---------------------------------------------------------------------------
// Macro Definitions
#ifndef NO_ERR
#define NO_ERR		0
#endif

//---------------------------------------------------------
// This number is the number of reserved objects per block
// There are always this many entries in the block.
// This is so that blocks do not change size so packet file
// is easier to deal with.  Number is not fixed in stone.
// Blocks should compress really well since the objdata
// for empty slots should repeat often.
#define MAX_OBJECTS_PER_BLOCK	200

// Error codes
//Starts with 0xBAAA0014

#define NO_RAM_FOR_TERRAIN_OBJECT_FILE		0xBAAA0014
#define NO_RAM_FOR_TERRAIN_OBJECT_HEAP		0xBAAA0015
#define NO_RAM_FOR_OBJECT_BLOCK_NUM			0xBAAA0016
#define NO_RAM_FOR_OBJECT_LISTS				0xBAAA0017
#define NO_RAM_FOR_OBJECT_DATA_BLOCK		0xBAAA0018
#define NO_RAM_FOR_OBJECT_BLOCK_USER		0xBAAA0019
#define NO_RAM_FOR_LAST_BLOCK				0xBAAA001A
#define NO_RAM_FOR_CENTER_BLOCK				0xBAAA001B
#define OBJECTBLOCK_OUTOFRANGE				0xBAAA001C
#define NO_AVAILABLE_OBJQUEUE				0xBAAA001D
#define COULDNT_MAKE_TERRAIN_OBJECT			0xBAAA001E
#define OBJECTBLOCK_NULL					0xBAAA001F
#define BLOCK_NOT_CACHED					0xBAAA0020
#define COULDNT_CREATE_OBJECT				0xBAAA0021
#define OBJECT_NOT_FOUND					0xBAAA0022

//---------------------------------------------------------------------------
// Class Definitions
#pragma pack(1)
#ifdef TERRAINEDIT
struct OldObjData
{
	ObjectTypeNumber	objTypeNum;		//Type number of object
	short				pixelOffsetX;	//Distance from vertex 0 of tile.
	short				pixelOffsetY;	//Distance from vertex 0 of tile.
	short				vertexNumber;	//Vertex Number in Block.
	short				blockNumber;	//Which terrain Block.
	unsigned int        damage;			//Damage
	float				positionX;		//Where, physically is object.  TOO DAMNED HARD TO CALCULATE!!!!!!!!
	float 				positionY;
};
#endif

struct ObjData
{
	short				objTypeNum;		//Type number of object
	unsigned short		vertexNumber;	//Vertex Number in Block.
	unsigned short		blockNumber;	//Which terrain Block.
	unsigned char		damage;			//Damage
};

struct MiscObjectData
{
	int				blockNumber;	//Terrain Block I occupy
	int				vertexNumber;	//Terrain Vertex I occupy
	int				objectTypeNum;	//ObjectTypeNumber for this overlay tile
	int				damaged;		//Is this overlay tile damaged or not
};

#pragma pack()

typedef ObjData *ObjDataPtr;
//---------------------------------------------------------------------------
class ObjectBlockManager
{
	//Data Members
	//-------------
	protected:
	
		unsigned long		terrainObjectHeapSize;	//Size of TerrainObject Heap
		UserHeapPtr			terrainObjectHeap;		//Pointer to Heap.

		ObjectQueueNodePtr	*objectQueues;			//Array of QueueNode Ptrs for each
													//Active block of terrain.

		unsigned long		numObjectsInDataBlock;	//Number of objects to be read/written
		ObjDataPtr			objDataBlock;			//Block of object data to read/write
		
		PacketFilePtr		objectDataFile;			//Packet file with blocks in it.
		
	//Member Functions
	//-----------------
	protected:
	
		long setupObjectQueue (unsigned long blockNum, unsigned long blockSize);

	public:
	
		void init (void)
		{
		   terrainObjectHeapSize = 0;
		   terrainObjectHeap = NULL;
		   
		   objectQueues = NULL;
		   
		   numObjectsInDataBlock = 0;
		   objDataBlock = NULL;
		   
		   objectDataFile = NULL;
		}
		
		ObjectBlockManager (void)
		{
			init();
		}
		
		void destroy (void);
		
		long init (char *packetFileName);
		
		~ObjectBlockManager (void)
		{
			destroy();
		}
		
		PacketFilePtr getObjectDataFile (void) {
			return(objectDataFile);
		}

		long update (BOOL createAll = FALSE);

		void updateAllObjects (void);
		
		void destroyAllObjects (void);

		ObjectQueueNodePtr getObjectList (long idNum)
		{
			return objectQueues[idNum];
		}
		
		#ifdef TERRAINEDIT
		long addObject (ObjectTypeNumber objNum, vector_2d &pOffset, vector_2d &numbers, vector_3d &position, long dmg = 0, long expTime  = -1);
		long removeObject (BaseObjectPtr deadObject);
		#endif
			
};

//---------------------------------------------------------------------------
#endif
