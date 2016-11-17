//---------------------------------------------------------------------------
//
// Terrain.h -- File contains class definitions for the terrain class.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TERRAIN_H
#define TERRAIN_H
//---------------------------------------------------------------------------
// Include Files
#ifndef MAPDATA_H
#include"mapdata.h"
#endif

#ifndef TERRTXM_H
#include"terrtxm.h"
#endif

#ifndef TERRTXM2_H
#include"terrtxm2.h"
#endif

#ifndef BITLAG_H
#include"bitflag.h"
#endif

#ifndef INIFILE_H
#include"inifile.h"
#endif

#ifndef MATHFUNC_H
#include"mathfunc.h"
#endif

#ifndef DQUAD_H
#include"dquad.h"
#endif

#ifndef DVERTEX_H
#include"dvertex.h"
#endif

#ifndef CLOUDS_H
#include"clouds.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions
#ifndef NO_ERR
#define NO_ERR		0
#endif

#define	MAPCELL_DIM				3
#define	MAX_MAP_CELL_WIDTH		720
#define TACMAP_SIZE				128.f

//------------------------------------------------
// Put back in Move code when Glenn moves it over.
// 07/28/99 these numbers didn't correspond to clan/IS despite comment, so I chagned 'em.
// These MUST be these numbers or the game will not mark LOS correctly!
#define NOTEAM					-1
//#define TEAM1					0 	//this is PLAYER TEAM -- Single Player
#define TEAM2					1	//this is OPFOR TEAM -- Single Player
#define TEAM3					2	// this is allies 
#define TEAM4					3
#define TEAM5					4
#define TEAM6					5
#define TEAM7					6
#define TEAM8					7

//-------------------------------------------
// 08/01/99 -- Must have generic alignments or Heidi goes WAY south!
#define EDITOR_TEAMNONE			-1		//Allied
#define EDITOR_TEAM1			0		//Player
#define EDITOR_TEAM2			1		//Enemy
#define EDITOR_TEAM3			2
#define EDITOR_TEAM4			3
#define EDITOR_TEAM5			4
#define EDITOR_TEAM6			5
#define EDITOR_TEAM7			6
#define EDITOR_TEAM8			7

//---------------------------------------------------------------------------
// Used by the object system to load the objects on the terrain.
typedef struct _ObjBlockInfo 
{
	bool		active;
	long		numCollidableObjects;
	long		numObjects;					// includes collidable objects
	long		firstHandle;				// collidables, followed by non
} ObjBlockInfo;

//---------------------------------------------------------------------------
//Everything goes through here now.
// This will understand the original MC2 format and new format and will convert between
class Terrain
{
	//Data Members
	//-------------
	protected:

		unsigned long							terrainHeapSize;
		
		long									numberVertices;
		long									numberQuads;
		VertexPtr								vertexList;
		TerrainQuadPtr							quadList;
		
	public:
		//For editor
		static long								userMin;
		static long								userMax;
		static unsigned long					baseTerrain;
		static unsigned char					fractalThreshold;
		static unsigned char					fractalNoise;

		static long								halfVerticesMapSide;		//Half of the below value.
		static long								realVerticesMapSide;		//Number of vertices on each side of map.
		
		static const long						verticesBlockSide;			//Always 20.
		static long								blocksMapSide;				//Calced from above and 
		static float							worldUnitsMapSide;			//Total world units map is across.
		static float							oneOverWorldUnitsMapSide;	//Inverse of the above.

		static long								visibleVerticesPerSide;		//How many should I process to be sure I got all I could see.

		static const float						worldUnitsPerVertex;		//How many world Units between each vertex.  128.0f in current universe.
		static const float						worldUnitsPerCell;			//How many world units between cells.  42.66666667f ALWAYS!!!!
		static const float						halfWorldUnitsPerCell;		//Above divided by two.
		static const float						metersPerCell;				//Number of meters per cell.  8.53333333f ALWAYS!!
		static const float						oneOverWorldUnitsPerVertex;	//Above numbers inverse.
		static const float						oneOverWorldUnitsPerCell;
		static const float						oneOverMetersPerCell;
		static const float						oneOverVerticesBlockSide;
		static const float						worldUnitsBlockSide;		//Total world units each block of 20 vertices is.  2560.0f in current universe.

		static Stuff::Vector3D					mapTopLeft3d;				//Where does the terrain start.
		
		static MapDataPtr						mapData;					//Pointer to class that manages terrain mesh data.
		static TerrainTexturesPtr				terrainTextures;			//Pointer to class that manages terrain textures.
		static TerrainColorMapPtr				terrainTextures2;			//Pointer to class that manages the NEW color map terrain texture.
		static UserHeapPtr						terrainHeap;				//Heap used for terrain.

//		static ByteFlag							*VisibleBits;				//What can currently be seen

		static char 							*terrainName;				//Name of terrain data file.
		static char								*colorMapName;				//Name of colormap, if different from terrainName.

		static float							oneOverWorldUnitsPerElevationLevel;

		static float							waterElevation;				//Actual height of water in world units.
		static float							frameAngle;					//Used to animate the waves
		static float							frameCos;
		static float							frameCosAlpha;
		static DWORD 							alphaMiddle;				//Used to alpha the water into the shore.
		static DWORD 							alphaEdge;
		static DWORD 							alphaDeep;
		static float							waterFreq;					//Used to animate waves.
		static float							waterAmplitude;

		static long		   						numObjBlocks;				//Stores terrain object info.
		static ObjBlockInfo						*objBlockInfo;				//Dynamically allocate this please!!
		
		static bool								*objVertexActive;			//Stores whether or not this vertices objects need to be updated

		static float 							*tileRowToWorldCoord;		//Arrays used to help change from tile and cell to actual world position.
		static float 							*tileColToWorldCoord;		//TILE functions will be obsolete with new system.
		static float 							*cellToWorldCoord;
		static float 							*cellColToWorldCoord;
		static float 							*cellRowToWorldCoord;

		static bool								recalcShadows;				//Should we recalc the shadow map!
		static bool								recalcLight;				//Should we recalc the light data.

		static Clouds							*cloudLayer;

	//Member Functions
	//-----------------
	public:

		void init (void);

		Terrain (void)
		{
			init();
		}

		void destroy (void);

		~Terrain (void)
		{
			destroy();
		}

		long init (PacketFile* file, int whichPacket, unsigned long visibleVertices, 
			volatile float& progress, float progressRange); // open an existing file
		long init( unsigned long verticesPerMapSide, PacketFile* file, unsigned long visibleVertices,
				volatile float& percent,
					float percentRange); // pass in null for a blank new map

		float getTerrainElevation (const Stuff::Vector3D &position);
		short getTerrainType (const Stuff::Vector3D &position);
		float getTerrainAngle (const Stuff::Vector3D &position, Stuff::Vector3D* normal = NULL);
		Stuff::Vector3D getTerrainNormal (const Stuff::Vector3D &position);
		float getTerrainLight (const Stuff::Vector3D& position);
		bool isVisible (const Stuff::Vector3D &looker, const Stuff::Vector3D &looked_at);

		float getWaterElevation ()
		{
			return mapData->waterElevation();
		}

		void markSeen (const Stuff::Vector3D &looker, byte who, float specialUnitExpand);
		void markRadiusSeen (const Stuff::Vector3D &looker, float dist, byte who);

		long update (void);
		void render (void);
		void renderWater (void);
		
		void geometry (void);

		void drawTopView (void);

		static bool IsValidTerrainPosition (const Stuff::Vector3D pos);
		static bool IsEditorSelectTerrainPosition (const Stuff::Vector3D pos);
		static bool IsGameSelectTerrainPosition (const Stuff::Vector3D pos);

		long save( PacketFile* fileName, int whichPacket, bool QuickSave = false);
		bool save( FitIniFile* fitFile ); // save stuff like water info
		bool load( FitIniFile* fitFile );

		// old overlay stuff
		void setOverlayTile (long block, long vertex, long offset);
		long getOverlayTile (long block, long vertex);
	
		// new overlay stuff
		void setOverlay( long tileR, long tileC, Overlays type, DWORD Offset );
		void getOverlay( long tileR, long tileC, Overlays& type, DWORD& Offset );
		void setTerrain( long tileR, long tileC, int terrainType );
		int	 getTerrain( long tileR, long tileC );
		unsigned long getTexture( long tileR, long tileC ); 
		float getTerrainElevation( long tileR, long tileC );

		void  setVertexHeight( int vertexIndex, float value ); 
		float getVertexHeight( int vertexIndex );

		void calcWater (float waterDepth, float waterShallowDepth, float waterAlphaDepth);

		void updateAllObjects (void);

		void setObjBlockActive (long blockNum, bool active);
		void clearObjBlocksActive (void);

		inline void worldToTile( const Stuff::Vector3D& pos, int& tileR, int& tileC );
		inline void worldToCell( const Stuff::Vector3D& pos, int& cellR, int& cellC );
		inline void worldToTileCell (const Stuff::Vector3D& pos, int& tileR, int& tileC, int& cellR, int& cellC);
		inline void tileCellToWorld (int tileR, int tileC, int cellR, int cellC, Stuff::Vector3D& worldPos);
		inline void cellToWorld (int cellR, int cellC, Stuff::Vector3D& worldPos);

		inline void getCellPos( int cellR, int cellC,  Stuff::Vector3D& cellPos );
		
		void initMapCellArrays(void);

		void unselectAll();
		void selectVerticesInRect( const Stuff::Vector4D& topLeft, const Stuff::Vector4D& bottomRight, bool bToggle );
		bool hasSelection();
		bool isVertexSelected( long tileR, long tileC );
		bool selectVertex( long tileR, long tileC, bool bSelect = true );

		float getHighestVertex( long& tileR, long& tileC );
		float getLowestVertex(  long& tileR, long& tileC );

		static void setUserSettings( long min, long max, int terrainType );
		static void getUserSettings( long& min, long& max, int& terrainType );

		void recalcWater();
		void reCalcLight(bool doShadows = false);
		void clearShadows();

		long getWater (const Stuff::Vector3D& worldPos);

		float getClipRange()
		{
			return 0.5 * worldUnitsPerVertex * (float)(visibleVerticesPerSide);
		}

		void setClipRange(float clipRange)
		{
			visibleVerticesPerSide = 2.0 * clipRange / worldUnitsPerVertex;
		}
		
		void purgeTransitions (void);
		
		TerrainQuadPtr getQuadList (void)
		{
			return(quadList);
		}
		
		VertexPtr getVertexList (void)
		{
			return(vertexList);
		}
		
		long getNumVertices (void)
		{
			return(numberVertices);
		}
		
		long getNumQuads (void)
		{
			return(numberQuads);
		}
		
		void setObjVertexActive (long vertexNum, bool active);
		
		void clearObjVerticesActive (void);

		void resetVisibleVertices(long maxVisibleVertices);

		void getColorMapName (FitIniFile *file);
		void setColorMapName (char *mapName);
		void saveColorMapName (FitIniFile *file);
};

typedef Terrain *TerrainPtr;

extern TerrainPtr land;

//---------------------------------------------------------------------------

inline void Terrain::worldToTile( const Stuff::Vector3D& pos, int& tileR, int& tileC )
{
	float tmpX = pos.x - land->mapTopLeft3d.x;
	float tmpY = land->mapTopLeft3d.y - pos.y;

	tileC = (int)(tmpX * oneOverWorldUnitsPerVertex);
	tileR =	(int)(tmpY * oneOverWorldUnitsPerVertex);
}

//---------------------------------------------------------------------------

inline void Terrain::worldToCell( const Stuff::Vector3D& pos, int& cellR, int& cellC )
{
	cellC = (int)(( pos.x - land->mapTopLeft3d.x ) * (oneOverWorldUnitsPerVertex*3.0f));
	cellR = (int)(( land->mapTopLeft3d.y - pos.y ) * (oneOverWorldUnitsPerVertex*3.0f));
}

//---------------------------------------------------------------------------

inline void Terrain::worldToTileCell( const Stuff::Vector3D& pos, int& tileR, int& tileC, int& cellR, int& cellC )
{
	float tmpX = pos.x - land->mapTopLeft3d.x;
	float tmpY = land->mapTopLeft3d.y - pos.y;

	tileC = tmpX * oneOverWorldUnitsPerVertex;
	tileR =	tmpY * oneOverWorldUnitsPerVertex;

	if ((tileC < 0) ||
		(tileR < 0) ||
		(tileC >= Terrain::realVerticesMapSide) ||
		(tileR >= Terrain::realVerticesMapSide))
	{
	#ifdef _DEBUG
		PAUSE(("called worldToTileCell with POS out of bounds? Result TC:%d TR:%d",tileC,tileR));
	#endif
		tileC = tileR = 0;
	}
		
	cellC = (pos.x - tileColToWorldCoord[tileC]) * oneOverWorldUnitsPerCell;
	cellR = (tileRowToWorldCoord[tileR] - pos.y) * oneOverWorldUnitsPerCell;
}

//---------------------------------------------------------------------------

inline void Terrain::tileCellToWorld (int tileR, int tileC, int cellR, int cellC, Stuff::Vector3D& worldPos) 
{
	if ((tileC < 0) ||
		(tileR < 0) ||
		(tileC >= Terrain::realVerticesMapSide) ||
		(tileR >= Terrain::realVerticesMapSide) ||
		(cellC < 0) ||
		(cellR < 0) ||
		(cellC >= MAPCELL_DIM) ||
		(cellR >= MAPCELL_DIM))
	{
	#ifdef _DEBUG
		PAUSE(("called cellToWorld with tile or cell out of bounds. TC:%d TR:%d CR:%d CC:%d",tileC,tileR,cellR,cellC));
	#endif
		tileR = tileC = cellR = cellC = 0;
	}
	else
	{
		worldPos.x = tileColToWorldCoord[tileC] + cellToWorldCoord[cellC] + halfWorldUnitsPerCell;
		worldPos.y = tileRowToWorldCoord[tileR] - cellToWorldCoord[cellR] - halfWorldUnitsPerCell;
		worldPos.z = (float)0.0;
	}
}

//---------------------------------------------------------------------------

inline void Terrain::cellToWorld (int cellR, int cellC, Stuff::Vector3D& worldPos) 
{
	if ((cellR < 0) || 
		(cellC < 0) || 
		(cellR >= (Terrain::realVerticesMapSide * MAPCELL_DIM)) ||
		(cellC >= (Terrain::realVerticesMapSide * MAPCELL_DIM)))
	{
	#ifdef _DEBUG
		PAUSE(("called cellToWorld with cell out of bounds. CellR:%d   CellC:%d",cellR,cellC));
	#endif
		worldPos.x = worldPos.y = worldPos.z = 0.0f;		
	}
	else
	{
		worldPos.x = cellColToWorldCoord[cellC] + halfWorldUnitsPerCell;
		worldPos.y = cellRowToWorldCoord[cellR] - halfWorldUnitsPerCell;
		worldPos.z = (float)0.0;
	}
}

//---------------------------------------------------------------------------

inline void Terrain::getCellPos( int cellR, int cellC,  Stuff::Vector3D& cellPos )
{
	cellPos.x = (cellC * (worldUnitsPerVertex/3.)) + (worldUnitsPerVertex/6.);
	cellPos.y = (cellR * (worldUnitsPerVertex/3.)) + (worldUnitsPerVertex/6.);

	cellPos.x += land->mapTopLeft3d.x;
	cellPos.y = land->mapTopLeft3d.y - cellPos.y;

	cellPos.z = land->getTerrainElevation( cellPos );
}

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//---------------------------------------------------------------------------
