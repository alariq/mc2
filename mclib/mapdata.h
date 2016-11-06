//---------------------------------------------------------------------------
//
// MapData.h -- File contains class definitions for the Terrain Mesh
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MAPDATA_H
#define MAPDATA_H
//---------------------------------------------------------------------------
// Include Files

#ifndef DMAPDATA_H
#include"dmapdata.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#ifndef PACKET_H
#include"packet.h"
#endif

#ifndef VERTEX_H
#include"vertex.h"
#endif

#ifndef QUAD_H
#include"quad.h"
#endif

#include<stuff/stuff.hpp>

//---------------------------------------------------------------------------
enum  Overlays
{
	INVALID_OVERLAY = -1,
	DIRT_ROAD = 0,
	PAVED_ROAD = 1,
	ROUGH = 2,
	DIRT_ROAD_TO_PAVED_ROAD = 3,
	PAVED_ROAD_CROSSING_DIRT_ROAD = 4,
	TWO_LANE_DIRT_ROAD = 5,
	DAMAGED_ROAD = 6,
	RUNWAY = 7,
	X_DIRT_PAVED = 8,
	X_DIRT_2LANEDIRT = 9,
	X_DIRT_DAMAGED = 10,
	X_2LANEDIRT_PAVED = 11,
	X_DAMAGED_PAVED = 12,
	X_2LANEDIRT_DAMAGED = 13,
	OBRIDGE = 14,
	X_PAVED_ROAD_BRIDGE = 15,
	DAMAGED_BRIDGE = 16,
	NUM_OVERLAY_TYPES = 17
};

//---------------------------------------------------------------------------
// Classes
class MapData : public HeapManager
{
	//Data Members
	//-------------
	protected:
		PostcompVertexPtr			blocks;
		PostcompVertexPtr			blankVertex;
		int							hasSelection;
									
	public:
		Stuff::Vector2DOf<float>	topLeftVertex;

		static float				shallowDepth;
		static float				waterDepth;
		static float				alphaDepth;
		static DWORD				WaterTXMData;

	//Member Functions
	//-----------------
	public:

		void *operator new (size_t mySize);
		void operator delete (void *us);
		
		void init (void)
		{
			HeapManager::init();

			topLeftVertex.Zero();			

			blocks = NULL;

			blankVertex = NULL;

			hasSelection = false;

			shallowDepth = 0.0f;
			waterDepth = 0.0f;
			alphaDepth = 0.0f;

			WaterTXMData = 0xffffffff;
		}

		MapData (void) : HeapManager()
		{
			init();
		}

		void destroy (void);
		
		~MapData (void)
		{
			destroy();
		}

		long init (char *fileName, long numBlocks, long blockSize);

		void newInit (PacketFile* file, long numVertices);
		void newInit (long numVertices);

		long update (void);
		void makeLists (VertexPtr vertexList, long &numVerts, TerrainQuadPtr quadList, long &numTiles);
		
		Stuff::Vector2DOf<float> getTopLeftVertex (void) 
		{
			return topLeftVertex;
		}

		void calcLight (void);
		void clearShadows();
		
		float terrainElevation (const Stuff::Vector3D &position);
		float terrainElevation ( long tileR, long tileC );

		float terrainAngle (const Stuff::Vector3D &position, Stuff::Vector3D* normal = NULL);
		Stuff::Vector3D terrainNormal (const Stuff::Vector3D& position);
		float terrainLight (const Stuff::Vector3D& position);
		
		float getTopLeftElevation (void);
		
		// old overlay stuff
		void setOverlayTile (long block, long vertex, long offset);
		long getOverlayTile (long block, long vertex);

		// new overlay stuff
		void setOverlay( long tileR, long tileC, Overlays type, DWORD Offset );
		void getOverlay( long tileR, long tileC, Overlays& type, DWORD& Offset );
		void setTerrain( long tileR, long tileC, int terrainType );
		long getTerrain( long tileR, long tileC );

		void  setVertexHeight( int vertexIndex, float value ); 
		float getVertexHeight( int vertexIndex );

		PostcompVertexPtr getData (void)
		{
			return blocks;
		}

		unsigned long getTexture( long tileR, long tileC );

		long save( PacketFile* file, int whichPacket);

		void calcWater (float waterDepth, float waterShallowDepth, float waterAlphaDepth);
		void recalcWater (void);									//Uses above values already passed in to just recalc the water
		
		float waterElevation () { return waterDepth; }

		void markSeen (const Stuff::Vector2DOf<float> &topLeftPosition, VertexPtr vertexList, const Stuff::Vector3D &looker, const Stuff::Vector3D &lookVector, float cone, float dist, byte who);

		void unselectAll();
		void unhighlightAll();
		void highlightAllTransitionsOver2 (void);
		void selectVertex( unsigned long tileRow, unsigned long tileCol, bool bSelect, bool bToggle );
		bool selection(){ return hasSelection ? true : false; }
		bool isVertexSelected( unsigned long tileRow, unsigned long tileCol );

		void calcTransitions();
};

//-----------------------------------------------------------------------------------------------
#endif
