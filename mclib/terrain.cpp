//---------------------------------------------------------------------------
//
// Terrain.cpp -- File contains calss definitions for the Terrain
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files
#ifndef TERRAIN_H
#include"terrain.h"
#endif

#ifndef VERTEX_H
#include"vertex.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef TERRTXM_H
#include"terrtxm.h"
#endif

#ifndef DBASEGUI_H
#include"dbasegui.h"
#endif

#ifndef CIDENT_H
#include"cident.h"
#endif

#ifndef PATHS_H
#include"paths.h"
#endif

#ifndef USERINPUT_H
#include"userinput.h"
#endif

#ifndef TIMING_H
#include"timing.h"
#endif

#ifndef PACKET_H
#include"packet.h"
#endif

#ifndef INIFILE_H
#include"fitinifile.h"
#endif

#ifndef TGAINFO_H
#include"tgainfo.h"
#endif

#include "stdlib_win.h"

//---------------------------------------------------------------------------
// Static Globals
float worldUnitsPerMeter = 5.01f;
float metersPerWorldUnit = 0.2f;
long terrainLineChanged = 0;

MapDataPtr					Terrain::mapData = NULL;
TerrainTexturesPtr			Terrain::terrainTextures = NULL;
TerrainColorMapPtr			Terrain::terrainTextures2 = NULL;

const long					Terrain::verticesBlockSide = 20;			//Changes for new terrain?
long						Terrain::blocksMapSide = 0;					//Calced during load.

long						Terrain::visibleVerticesPerSide = 0;		//Passed in.

const float					Terrain::worldUnitsPerVertex = 128.0;
const float					Terrain::worldUnitsPerCell = Terrain::worldUnitsPerVertex / MAPCELL_DIM;
const float					Terrain::halfWorldUnitsPerCell = Terrain::worldUnitsPerCell / 2.0f;
const float					Terrain::metersPerCell = Terrain::worldUnitsPerCell * metersPerWorldUnit;
const float					Terrain::worldUnitsBlockSide = Terrain::worldUnitsPerVertex * Terrain::verticesBlockSide;
const float					Terrain::oneOverWorldUnitsPerVertex = 1.0f / Terrain::worldUnitsPerVertex;
const float					Terrain::oneOverWorldUnitsPerCell = 1.0f / Terrain::worldUnitsPerCell;
const float					Terrain::oneOverMetersPerCell = 1.0f / Terrain::metersPerCell;
const float					Terrain::oneOverVerticesBlockSide = 1.0f / Terrain::verticesBlockSide;

float						Terrain::worldUnitsMapSide = 0.0;		//Calced during load.
float						Terrain::oneOverWorldUnitsMapSide = 0.0f;
long						Terrain::halfVerticesMapSide = 0;
long						Terrain::realVerticesMapSide = 0;

Stuff::Vector3D				Terrain::mapTopLeft3d;					//Calced during load.

UserHeapPtr					Terrain::terrainHeap = NULL;			//Setup at load time.
char *						Terrain::terrainName = NULL;
char * 						Terrain::colorMapName = NULL;			

long		   				Terrain::numObjBlocks = 0;
ObjBlockInfo				*Terrain::objBlockInfo = NULL;
bool						*Terrain::objVertexActive = NULL;

float 						*Terrain::tileRowToWorldCoord = NULL;
float 						*Terrain::tileColToWorldCoord = NULL;
float 						*Terrain::cellToWorldCoord = NULL;
float 						*Terrain::cellColToWorldCoord = NULL;
float 						*Terrain::cellRowToWorldCoord = NULL;

float 						Terrain::waterElevation = 0.0f;
float						Terrain::frameAngle = 0.0f;
float 						Terrain::frameCos = 1.0f;
float						Terrain::frameCosAlpha = 1.0f;
DWORD						Terrain::alphaMiddle = 0xaf000000;
DWORD						Terrain::alphaEdge = 0x3f000000;
DWORD						Terrain::alphaDeep = 0xff000000;
float						Terrain::waterFreq = 4.0f;
float						Terrain::waterAmplitude = 10.0f;

long						Terrain::userMin = 0;
long						Terrain::userMax = 0;
unsigned long				Terrain::baseTerrain = 0;
unsigned char				Terrain::fractalThreshold = 1;
unsigned char				Terrain::fractalNoise = 0;
bool						Terrain::recalcShadows = false;
bool						Terrain::recalcLight = false;

Clouds						*Terrain::cloudLayer = NULL;

bool 						drawTerrainGrid = false;		//Override locally in editor so game don't come with these please!  Love -fs
bool						drawLOSGrid = false;
bool						drawTerrainTiles = true;
bool						drawTerrainOverlays = true;
bool						drawTerrainMines = true;
bool						renderObjects = true;
bool						renderTrees = true;

TerrainPtr					land = NULL;

long 						*usedBlockList;					//Used to determine what objects to deal with.
long 						*moverBlockList;

unsigned long 				blockMemSize = 0;				//Misc Flags.
bool 						useOldProject = FALSE;
bool 						projectAll = FALSE;
bool 						useClouds = false;
bool 						useFog = true;
bool 						useVertexLighting = true;
bool 						useFaceLighting = false;
extern bool					useRealLOS;

unsigned char 				godMode = 0;			//Can I simply see everything, enemy and friendly?

extern long 				DrawDebugCells;

#define						MAX_TERRAIN_HEAP_SIZE		1024000

long						visualRangeTable[256];
extern bool 				justResaveAllMaps;
//---------------------------------------------------------------------------
// These are used to determine what terrain objects to process.
// They date back to GenCon 1996!!
void addBlockToList (long blockNum)
{
	long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;
	for (long i=0;i<totalBlocks;i++)
	{
		if (usedBlockList[i] == blockNum)
		{
			return;
		}
		else if (usedBlockList[i] == -1)
		{
			usedBlockList[i] = blockNum;
			return;
		}
	}
}

//---------------------------------------------------------------------------
void addMoverToList (long blockNum)
{
	long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;
	for (long i=0;i<totalBlocks;i++)
	{
		if (moverBlockList[i] == blockNum)
		{
			return;
		}
		else if (moverBlockList[i] == -1)
		{
			moverBlockList[i] = blockNum;
			return;
		}
	}
}

//---------------------------------------------------------------------------
void clearList (void)
{
	long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;
	blockMemSize = totalBlocks * sizeof(long);
	
	if (usedBlockList)
		memset(usedBlockList,-1,blockMemSize);
}

//---------------------------------------------------------------------------
void clearMoverList (void)
{
	long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;
	blockMemSize = totalBlocks * sizeof(long);
	
	if (moverBlockList)
		memset(moverBlockList,-1,blockMemSize);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// class Terrain
void Terrain::init (void)
{
	vertexList = NULL;
	numberVertices = 0;
	
	quadList = NULL;
	numberQuads = 0;
}

//---------------------------------------------------------------------------
void Terrain::initMapCellArrays (void)
{
	if (!tileRowToWorldCoord)
	{
		tileRowToWorldCoord = (float *)terrainHeap->Malloc(sizeof(float) * realVerticesMapSide);
		gosASSERT(tileRowToWorldCoord != NULL);
	}

	if (!tileColToWorldCoord)
	{
		tileColToWorldCoord = (float *)terrainHeap->Malloc(sizeof(float) * realVerticesMapSide); 
		gosASSERT(tileColToWorldCoord != NULL);
	}

	if (!cellToWorldCoord)
	{
		cellToWorldCoord = (float *)terrainHeap->Malloc(sizeof(float) * MAPCELL_DIM); 
		gosASSERT(cellToWorldCoord != NULL);
	}

	if (!cellColToWorldCoord)
	{
		cellColToWorldCoord = (float *)terrainHeap->Malloc(sizeof(float) * realVerticesMapSide * MAPCELL_DIM); 
		gosASSERT(cellColToWorldCoord != NULL);
	}

	if (!cellRowToWorldCoord)
	{
		cellRowToWorldCoord = (float *)terrainHeap->Malloc(sizeof(float) * realVerticesMapSide * MAPCELL_DIM); 
		gosASSERT(cellRowToWorldCoord != NULL);
	}

	long i=0;

	long height = realVerticesMapSide, width = height;
	for (i = 0; i < height; i++)
		tileRowToWorldCoord[i] = (worldUnitsMapSide / 2.0) - (i * worldUnitsPerVertex);

	for (i = 0; i < width; i++)
		tileColToWorldCoord[i] = (i * worldUnitsPerVertex) - (worldUnitsMapSide / 2.0);

	for (i = 0; i < MAPCELL_DIM; i++)
		cellToWorldCoord[i] = (worldUnitsPerVertex / (float)MAPCELL_DIM) * i;

	long maxCell = height * MAPCELL_DIM;
	for (i = 0; i < maxCell; i++)
		cellRowToWorldCoord[i] = (worldUnitsMapSide / 2.0) - (i * worldUnitsPerCell);

	maxCell = width * MAPCELL_DIM;
	for (i = 0; i < maxCell; i++)
		cellColToWorldCoord[i] = (i * worldUnitsPerCell) - (worldUnitsMapSide / 2.0);
}	

//---------------------------------------------------------------------------
long Terrain::init (PacketFile* pakFile, int whichPacket, unsigned long visibleVertices, volatile float& percent,
					float percentRange )
{
	clearList();
	clearMoverList();
	
	long result = pakFile->seekPacket( whichPacket );
	if (result != NO_ERR)
		STOP(("Unable to seek Packet %d in file %s",whichPacket,pakFile->getFilename()));
	
	int tmp = pakFile->getPacketSize();
	realVerticesMapSide = sqrt( float(tmp/ sizeof(PostcompVertex)));
	
	if (!justResaveAllMaps && 
		(realVerticesMapSide != 120) &&
		(realVerticesMapSide != 100) && 
		(realVerticesMapSide != 80) &&
		(realVerticesMapSide != 60))
	{
		PAUSE(("This map size NO longer supported %d.  Must be 120, 100, 80 or 60 now!  Can Continue, for NOW!!",realVerticesMapSide));
//		return -1;
	}
	
	init( realVerticesMapSide, pakFile, visibleVertices, percent, percentRange );	
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
void Terrain::getColorMapName (FitIniFile *file)
{
	if (file)
	{
		if (file->seekBlock("ColorMap") == NO_ERR)
		{
			char mapName[1024];
			if (file->readIdString("ColorMapName",mapName,1023) == NO_ERR)
			{
				colorMapName = new char[strlen(mapName)+1];
				strcpy(colorMapName,mapName);
				return;
			}
		}
	}

	colorMapName = NULL;
}

//---------------------------------------------------------------------------
void Terrain::setColorMapName (char *mapName)
{
	if (colorMapName)
	{
		delete [] colorMapName;
		colorMapName = NULL;
	}

	if (mapName)
	{
		colorMapName = new char [strlen(mapName)+1];
		strcpy(colorMapName,mapName);
	}
}

//---------------------------------------------------------------------------
void Terrain::saveColorMapName (FitIniFile *file)
{
	if (file && colorMapName)
	{
		file->writeBlock("ColorMap");
		file->writeIdString("ColorMapName",colorMapName);
	}
}

//---------------------------------------------------------------------------
long Terrain::init( unsigned long verticesPerMapSide, PacketFile* pakFile, unsigned long visibleVertices,
				   volatile float& percent,
					float percentRange)
{
	//Did we pass in the hi-res colormap?
	// If so, convert back to old verticesPerMapSide!
	if (verticesPerMapSide > 300)
		verticesPerMapSide /= 12.8;
		
	realVerticesMapSide = verticesPerMapSide;
	halfVerticesMapSide = realVerticesMapSide >> 1;
	blocksMapSide = realVerticesMapSide / verticesBlockSide;
	worldUnitsMapSide = realVerticesMapSide * worldUnitsPerVertex;
	if (worldUnitsMapSide > Stuff::SMALL)
		oneOverWorldUnitsMapSide = 1.0f / worldUnitsMapSide;
	else
		oneOverWorldUnitsMapSide = 0.0f;

	Terrain::numObjBlocks = blocksMapSide * blocksMapSide;
	visibleVerticesPerSide = visibleVertices;
	terrainHeapSize = MAX_TERRAIN_HEAP_SIZE;

	//-----------------------------------------------------------------
	// Startup to Terrain Heap
	if( !terrainHeap )
	{
		terrainHeap = new UserHeap;
		gosASSERT(terrainHeap != NULL);
		terrainHeap->init(terrainHeapSize,"TERRAIN");
	}

	percent += percentRange/5.f;
	//-----------------------------------------------------------------
	// Startup the Terrain Texture Maps
	if ( !terrainTextures )
	{
		char baseName[256];
		if (pakFile)
		{
			_splitpath(pakFile->getFilename(),NULL,NULL,baseName,NULL);
		}
		else
		{
			strcpy(baseName,"newmap");
		}

		terrainTextures = new TerrainTextures;
		terrainTextures->init("textures",baseName);
	}

	percent += percentRange/5.f;


	if ( !pakFile && !realVerticesMapSide )
		return NO_ERR;

	//-----------------------------------------------------------------
	// Startup the Terrain Color Map
	if ( !terrainTextures2 && pakFile)
	{
		char name[1024];

		_splitpath(pakFile->getFilename(),NULL,NULL,name,NULL);
		terrainName = new char[strlen(name)+1];
		strcpy(terrainName,name);

		if (colorMapName)
			strcpy(name,colorMapName);

		FullPathFileName tgaColorMapName;
		tgaColorMapName.init(texturePath,name,".tga");
		
		FullPathFileName tgaColorMapBurninName;
		tgaColorMapBurninName.init(texturePath,name,".burnin.tga");

		FullPathFileName tgaColorMapJPGName;
		tgaColorMapJPGName.init(texturePath,name,".burnin.jpg");
				
		if (fileExists(tgaColorMapName) || fileExists(tgaColorMapBurninName) || fileExists(tgaColorMapJPGName))
		{
			terrainTextures2 = new TerrainColorMap;		//Otherwise, this will stay NULL and we know not to use them
		}
	}

	percent += percentRange/5.f;


	mapTopLeft3d.x = -worldUnitsMapSide / 2.0f;
	mapTopLeft3d.y = worldUnitsMapSide / 2.0f;

	percent += percentRange/5.f;


	//----------------------------------------------------------------------
	// Setup number of blocks
	long numberBlocks = blocksMapSide * blocksMapSide;
	
	numObjBlocks = numberBlocks;
	objBlockInfo = (ObjBlockInfo *)terrainHeap->Malloc(sizeof(ObjBlockInfo)*numObjBlocks);
	gosASSERT(objBlockInfo != NULL);
	
	memset(objBlockInfo,0,sizeof(ObjBlockInfo)*numObjBlocks);
	
	objVertexActive = (bool *)terrainHeap->Malloc(sizeof(bool) * realVerticesMapSide * realVerticesMapSide);
	gosASSERT(objVertexActive != NULL);
	
	memset(objVertexActive,0,sizeof(bool)*numObjBlocks);
	
	moverBlockList = (long *)terrainHeap->Malloc(sizeof(long) * numberBlocks);
	gosASSERT(moverBlockList != NULL);
	
	usedBlockList = (long *)terrainHeap->Malloc(sizeof(long) * numberBlocks);
	gosASSERT(usedBlockList != NULL);
	
	clearList();
	clearMoverList();

	//----------------------------------------------------------------------
	// Calculate size of each mapblock
	long blockSize = verticesBlockSide * verticesBlockSide;
	blockSize *= sizeof(PostcompVertex);

	//----------------------------------------------------------------------
	// Create the MapBlock Manager and allocate its RAM
	if ( !mapData )
	{
		mapData = new MapData;
		if ( pakFile )
			mapData->newInit( pakFile, realVerticesMapSide*realVerticesMapSide);
		else
			mapData->newInit( realVerticesMapSide*realVerticesMapSide );

		mapTopLeft3d.z = mapData->getTopLeftElevation();
	}

	percent += percentRange/5.f;

	
	//----------------------------------------------------------------------
	// Create the VertexList
	numberVertices = 0;
	vertexList = (VertexPtr)terrainHeap->Malloc(sizeof(Vertex) * visibleVertices * visibleVertices);
	gosASSERT(vertexList != NULL);
	memset(vertexList,0,sizeof(Vertex) * visibleVertices * visibleVertices);

	//----------------------------------------------------------------------
	// Create the QuadList
	numberQuads = 0;
	quadList = (TerrainQuadPtr)terrainHeap->Malloc(sizeof(TerrainQuad) * visibleVertices * visibleVertices);
	gosASSERT(quadList != NULL);
	memset(quadList,0,sizeof(TerrainQuad) * visibleVertices * visibleVertices);

	//-------------------------------------------------------------------
	initMapCellArrays();

	//-----------------------------------------------------------------
	// Startup the Terrain Color Map
	if ( terrainTextures2  && !(terrainTextures2->colorMapStarted))
	{
		if (colorMapName)
			terrainTextures2->init(colorMapName);
		else
			terrainTextures2->init(terrainName);
	}

	return NO_ERR;
}

void Terrain::resetVisibleVertices (long maxVisibleVertices)
{
	terrainHeap->Free(vertexList);
	vertexList = NULL;

	terrainHeap->Free(quadList);
	quadList = NULL;

	visibleVerticesPerSide = maxVisibleVertices;
	//----------------------------------------------------------------------
	// Create the VertexList
	numberVertices = 0;
	vertexList = (VertexPtr)terrainHeap->Malloc(sizeof(Vertex) * visibleVerticesPerSide * visibleVerticesPerSide);
	gosASSERT(vertexList != NULL);
	memset(vertexList,0,sizeof(Vertex) * visibleVerticesPerSide * visibleVerticesPerSide);

	//----------------------------------------------------------------------
	// Create the QuadList
	numberQuads = 0;
	quadList = (TerrainQuadPtr)terrainHeap->Malloc(sizeof(TerrainQuad) * visibleVerticesPerSide * visibleVerticesPerSide);
	gosASSERT(quadList != NULL);
	memset(quadList,0,sizeof(TerrainQuad) * visibleVerticesPerSide * visibleVerticesPerSide);

	
}

//---------------------------------------------------------------------------
bool Terrain::IsValidTerrainPosition (const Stuff::Vector3D pos)
{
	float metersCheck = (Terrain::worldUnitsMapSide / 2.0f);

	if ((pos.x > -metersCheck) &&
		(pos.x < metersCheck) &&
		(pos.y > -metersCheck) &&
		(pos.y < metersCheck))
	{
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
bool Terrain::IsEditorSelectTerrainPosition (const Stuff::Vector3D pos)
{
	float metersCheck = (Terrain::worldUnitsMapSide / 2.0f) - Terrain::worldUnitsPerVertex;

	if ((pos.x > -metersCheck) &&
		(pos.x < metersCheck) &&
		(pos.y > -metersCheck) &&
		(pos.y < metersCheck))
	{
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
bool Terrain::IsGameSelectTerrainPosition (const Stuff::Vector3D pos)
{
	float metersCheck = (Terrain::worldUnitsMapSide / 2.0f) - (Terrain::worldUnitsPerVertex * 2.0f);

	if ((pos.x > -metersCheck) &&
		(pos.x < metersCheck) &&
		(pos.y > -metersCheck) &&
		(pos.y < metersCheck))
	{
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
void Terrain::purgeTransitions (void)
{
	terrainTextures->purgeTransitions();
	mapData->calcTransitions();
}

//---------------------------------------------------------------------------
void Terrain::destroy (void)
{
	if (terrainTextures)
	{
		terrainTextures->destroy();
		delete terrainTextures;
		terrainTextures = NULL;
	}

	if (terrainTextures2)
	{
		terrainTextures2->destroy();
		delete terrainTextures2;
		terrainTextures2 = NULL;
	}

	delete mapData;
	mapData = NULL;

	if (terrainName)
	{
		delete [] terrainName;
		terrainName = NULL;
	}

	if (colorMapName)
	{
		delete [] colorMapName;
		colorMapName = NULL;
	}

	if (tileRowToWorldCoord)
	{
		terrainHeap->Free(tileRowToWorldCoord);
		tileRowToWorldCoord = NULL;
	}

	if (tileColToWorldCoord)
	{
		terrainHeap->Free(tileColToWorldCoord); 
		tileColToWorldCoord = NULL;
	}

	if (cellToWorldCoord)
	{
		terrainHeap->Free(cellToWorldCoord); 
		cellToWorldCoord = NULL;
	}

	if (cellColToWorldCoord)
	{
		terrainHeap->Free(cellColToWorldCoord); 
		cellColToWorldCoord = NULL;
	}

	if (cellRowToWorldCoord)
	{
		terrainHeap->Free(cellRowToWorldCoord); 
		cellRowToWorldCoord = NULL;
	}

	if (moverBlockList)
	{
		terrainHeap->Free(moverBlockList);
		moverBlockList = NULL;
	}

	if (usedBlockList)
	{
		terrainHeap->Free(usedBlockList);
		usedBlockList = NULL;
	}

	if (vertexList)
	{
		terrainHeap->Free(vertexList);
		vertexList = NULL;
	}

	if (quadList)
	{
		terrainHeap->Free(quadList);
		quadList = NULL;
	}

	if (objBlockInfo)
	{
		terrainHeap->Free(objBlockInfo);
		objBlockInfo = NULL;
	}
	
	if (objVertexActive)
	{
		terrainHeap->Free(objVertexActive);
		objVertexActive = NULL;
	}
	
 	if (terrainHeap)
	{
		terrainHeap->destroy();
		delete terrainHeap;
		terrainHeap = NULL;
	}
	
	numberVertices =
	numberQuads =
	
	halfVerticesMapSide = 
	realVerticesMapSide = 
		
	visibleVerticesPerSide =
	blocksMapSide = 0;
	
	worldUnitsMapSide = 0.0f;
	
	mapTopLeft3d.Zero();
		
	numObjBlocks = 0;

	recalcShadows = 
	recalcLight = false;

	//Reset these.  This will fix the mine problem.
	TerrainQuad::rainLightLevel = 1.0f;
	TerrainQuad::lighteningLevel = 0;
	TerrainQuad::mineTextureHandle = 0xffffffff;
	TerrainQuad::blownTextureHandle = 0xffffffff;
}

extern float textureOffset;
//---------------------------------------------------------------------------
long Terrain::update (void)
{
	//-----------------------------------------------------------------
	// Startup the Terrain Color Map
	if ( terrainTextures2  && !(terrainTextures2->colorMapStarted))
	{
		if (colorMapName)
			terrainTextures2->init(colorMapName);
		else
			terrainTextures2->init(terrainName);
	}

	//----------------------------------------------------------------
	// Nothing is ever visible.  We recalc every frame.  True LOS!
//	Terrain::VisibleBits->resetAll(0);
		
	if (godMode)	
	{
//		Terrain::VisibleBits->resetAll(0xff);
	}

	if (turn > terrainLineChanged+10)
	{
		if (userInput->getKeyDown(KEY_UP) && userInput->ctrl() && userInput->alt() && !userInput->shift())
		{
			textureOffset += 0.1f;;
			terrainLineChanged = turn;
		}
		
		if (userInput->getKeyDown(KEY_DOWN) && userInput->ctrl() && userInput->alt() && !userInput->shift())
		{
			textureOffset -= 0.1f;;
			terrainLineChanged = turn;
		}
	}
	
 	//---------------------------------------------------------------------
	Terrain::mapData->update();
	Terrain::mapData->makeLists(vertexList,numberVertices,quadList,numberQuads);

	return TRUE;
}

//---------------------------------------------------------------------------
void Terrain::setOverlayTile (long block, long vertex, long offset)
{
	mapData->setOverlayTile(block,vertex,offset);
}	

//---------------------------------------------------------------------------
void Terrain::setOverlay( long tileR, long tileC, Overlays type, DWORD offset )
{
	mapData->setOverlay( tileR, tileC, type, offset );
}

//---------------------------------------------------------------------------
void Terrain::setTerrain( long tileR, long tileC, int terrainType )
{
	mapData->setTerrain( tileR, tileC, terrainType );
}

//---------------------------------------------------------------------------
int Terrain::getTerrain( long tileR, long tileC )
{
	return mapData->getTerrain( tileR, tileC );
}

//---------------------------------------------------------------------------
void Terrain::calcWater (float waterDepth, float waterShallowDepth, float waterAlphaDepth)
{
	mapData->calcWater(waterDepth, waterShallowDepth, waterAlphaDepth);
}	

//---------------------------------------------------------------------------
long Terrain::getOverlayTile (long block, long vertex)
{
	return (mapData->getOverlayTile(block,vertex));
}	

//---------------------------------------------------------------------------
void Terrain::getOverlay( long tileR, long tileC, enum Overlays& type, DWORD& Offset )
{
	mapData->getOverlay( tileR, tileC, type, Offset );
}

//---------------------------------------------------------------------------
void Terrain::setVertexHeight( int VertexIndex, float Val )
{
	if ( VertexIndex > -1 && VertexIndex < realVerticesMapSide * realVerticesMapSide )
		mapData->setVertexHeight( VertexIndex, Val );
}

//---------------------------------------------------------------------------
float Terrain::getVertexHeight( int VertexIndex )
{
	if ( VertexIndex > -1 && VertexIndex < realVerticesMapSide * realVerticesMapSide )
		return mapData->getVertexHeight(VertexIndex);

	return -1.f;
}

//---------------------------------------------------------------------------
void Terrain::render (void)
{
	//-----------------------------------
	// render the cloud layer
	if (Terrain::cloudLayer)
		Terrain::cloudLayer->render();
	
	//-----------------------------------
	// Draw resulting terrain quads
	TerrainQuadPtr currentQuad = quadList;
	DWORD fogColor = eye->fogColor;

	for (long i=0;i<numberQuads;i++)
	{
		if (drawTerrainTiles)
			currentQuad->draw();
			
		if (drawTerrainTiles)
			currentQuad->drawMine();
			
		//--------------------------
		// Used to debug stuff
		if (drawTerrainGrid)
		{
			if (useFog)
				gos_SetRenderState( gos_State_Fog, 0);

			currentQuad->drawLine();

			if (useFog)
				gos_SetRenderState( gos_State_Fog, fogColor);
		}
		else if (DrawDebugCells) 
		{
			if (useFog)
				gos_SetRenderState( gos_State_Fog, 0);
			currentQuad->drawDebugCellLine();
			if (useFog)
				gos_SetRenderState( gos_State_Fog, fogColor);
		}
		else if (drawLOSGrid)
		{
			if (useFog)
				gos_SetRenderState( gos_State_Fog, 0);

			currentQuad->drawLOSLine();

			if (useFog)
				gos_SetRenderState( gos_State_Fog, fogColor);
		}
		
		currentQuad++;
	}
}

//---------------------------------------------------------------------------
void Terrain::renderWater (void)
{
	//-----------------------------------
	// Draw resulting terrain quads
	TerrainQuadPtr currentQuad = quadList;

	for (long i=0;i<numberQuads;i++)
	{
		if (drawTerrainTiles)
			currentQuad->drawWater();
			
		currentQuad++;
	}
}

float cosineEyeHalfFOV = 0.0f; 
#define MAX_CAMERA_RADIUS		(250.0f)
#define CLIP_THRESHOLD_DISTANCE	(768.0f)

//a full triangle.
#define VERTEX_EXTENT_RADIUS	(384.0f)

float leastZ = 1.0f,leastW = 1.0f;
float mostZ = -1.0f, mostW = -1.0;
float leastWY = 0.0f, mostWY = 0.0f;
extern bool InEditor;
//---------------------------------------------------------------------------
void Terrain::geometry (void)
{
	//---------------------------------------------------------------------
	leastZ = 1.0f;leastW = 1.0f;
	mostZ = -1.0f; mostW = -1.0;
	leastWY = 0.0f; mostWY = 0.0f;

	//-----------------------------------
	// Transform entire list of vertices
	VertexPtr currentVertex = vertexList;

	Stuff::Vector3D cameraPos;
	cameraPos.x = -eye->getCameraOrigin().x;
	cameraPos.y = eye->getCameraOrigin().z;
	cameraPos.z = eye->getCameraOrigin().y;

	float vClipConstant = eye->verticalSphereClipConstant;
	float hClipConstant = eye->horizontalSphereClipConstant; 
	
 	long i=0;
	for (i=0;i<numberVertices;i++)
	{
		//----------------------------------------------------------------------------------------
		// Figure out if we are in front of camera or not.  Should be faster then actual project!
		// Should weed out VAST overwhelming majority of vertices!
		bool onScreen = false;
	
		//-----------------------------------------------------------------
		// Find angle between lookVector of Camera and vector from camPos
		// to Target.  If angle is less then halfFOV, object is visible.
		if (eye->usePerspective)
		{
			//-------------------------------------------------------------------
			//NEW METHOD from the WAY BACK Days
			onScreen = true;
			
			Stuff::Vector3D vPosition;
			vPosition.x = currentVertex->vx;
			vPosition.y = currentVertex->vy;
			vPosition.z = currentVertex->pVertex->elevation;
  
			Stuff::Vector3D objectCenter;
			objectCenter.Subtract(vPosition,cameraPos);
			Camera::cameraFrame.trans_to_frame(objectCenter);
			float distanceToEye = objectCenter.GetApproximateLength();

			Stuff::Vector3D clipVector = objectCenter;
			clipVector.z = 0.0f;
			float distanceToClip = clipVector.GetApproximateLength();
			float clip_distance = fabs(1.0f / objectCenter.y);
			
			if (distanceToClip > CLIP_THRESHOLD_DISTANCE)
			{
				//Is vertex on Screen OR close enough to screen that its triangle MAY be visible?
				// WE have removed the atans here by simply taking the tan of the angle we want above.
				float object_angle = fabs(objectCenter.z) * clip_distance;
				float extent_angle = VERTEX_EXTENT_RADIUS / distanceToEye;
				if (object_angle > (vClipConstant + extent_angle))
				{
					//In theory, we would return here.  Object is NOT on screen.
					onScreen = false;
				}
				else
				{
					object_angle = fabs(objectCenter.x) * clip_distance;
					if (object_angle > (hClipConstant + extent_angle))
					{
						//In theory, we would return here.  Object is NOT on screen.
						onScreen = false;
					}
				}
			}
			
			if (onScreen)
			{
				if (distanceToEye > Camera::MaxClipDistance)
				{
					currentVertex->hazeFactor = 1.0f;
				}
				else if (distanceToEye > Camera::MinHazeDistance)
				{
					currentVertex->hazeFactor = (distanceToEye - Camera::MinHazeDistance) * Camera::DistanceFactor;
				}
				else
				{
					currentVertex->hazeFactor = 0.0f;
				}
				
				//---------------------------------------
				// Vertex is at edge of world or beyond.
				Stuff::Vector3D vPos(currentVertex->vx,currentVertex->vy,currentVertex->pVertex->elevation);
				bool isVisible = Terrain::IsGameSelectTerrainPosition(vPos) || drawTerrainGrid;
				if (!isVisible)
				{
					currentVertex->hazeFactor = 1.0f;
					onScreen = true;
				}
			}
			else
			{
				currentVertex->hazeFactor = 1.0f;
			}
		}
		else
		{
			currentVertex->hazeFactor = 0.0f;
			onScreen = true;
		}

		bool inView = false;
		Stuff::Vector4D screenPos(-10000.0f,-10000.0f,-10000.0f,-10000.0f);
		if (onScreen)
		{
			Stuff::Vector3D vertex3D(currentVertex->vx,currentVertex->vy,currentVertex->pVertex->elevation);
			inView = eye->projectZ(vertex3D,screenPos);
		
			currentVertex->px = screenPos.x;
			currentVertex->py = screenPos.y;
			currentVertex->pz = screenPos.z;
			currentVertex->pw = screenPos.w;
			
			//----------------------------------------------------------------------------------
			//We must transform these but should NOT draw any face where all three are fogged. 
//			if (currentVertex->hazeFactor == 1.0f)		
//				onScreen = false;
		}
		else
		{
			currentVertex->px = currentVertex->py = 10000.0f;
			currentVertex->pz = -0.5f;
			currentVertex->pw = 0.5f;
			currentVertex->hazeFactor = 0.0f;
		}	
		
		//------------------------------------------------------------
		// Fix clip.  Vertices can all be off screen and triangle
		// still needs to be drawn!
		if (eye->usePerspective && Environment.Renderer != 3)
		{
			currentVertex->clipInfo = onScreen;
		}
		else
			currentVertex->clipInfo = inView;
		
		if (currentVertex->clipInfo)				//ONLY set TRUE ones.  Otherwise we just reset the FLAG each vertex!
		{
			setObjBlockActive(currentVertex->getBlockNumber(), true);
			setObjVertexActive(currentVertex->vertexNum,true);
			
			if (inView)
			{
				if (screenPos.z < leastZ)
				{
					leastZ = screenPos.z;
				}
				
				if (screenPos.z > mostZ)
				{
					mostZ = screenPos.z;
				}
				
				if (screenPos.w < leastW)
				{
					leastW = screenPos.w;
					leastWY = screenPos.y;
				}
				
				if (screenPos.w > mostW)
				{
					mostW = screenPos.w;
					mostWY = screenPos.y;
				}
			}
		}

		currentVertex++;
	}
	
	//-----------------------------------
	// setup terrain quad textures
	// Also sets up mine data.
	TerrainQuadPtr currentQuad = quadList;
	for (i=0;i<numberQuads;i++)
	{
		currentQuad->setupTextures();
		currentQuad++;
	}

	float ywRange = 0.0f, yzRange = 0.0f;
	if (fabs(mostWY - leastWY) > Stuff::SMALL)
	{
		ywRange = (mostW - leastW) / (mostWY - leastWY);
		yzRange = (mostZ - leastZ) / (mostWY - leastWY);
	}

	eye->setInverseProject(mostZ,leastW,yzRange,ywRange);

	//-----------------------------------
	// update the cloud layer
	if (Terrain::cloudLayer)
		Terrain::cloudLayer->update();
}

//---------------------------------------------------------------------------
float Terrain::getTerrainElevation (const Stuff::Vector3D &position)
{
	float result = mapData->terrainElevation(position);
	return(result);
}

//---------------------------------------------------------------------------
float Terrain::getTerrainElevation( long tileR, long tileC )
{
	return mapData->terrainElevation( tileR, tileC );
}

//---------------------------------------------------------------------------
unsigned long Terrain::getTexture( long tileR, long tileC )
{
	return mapData->getTexture( tileR, tileC );
}

//---------------------------------------------------------------------------
float Terrain::getTerrainAngle (const Stuff::Vector3D &position, Stuff::Vector3D* normal)
{
	float result = mapData->terrainAngle(position, normal);
	return(result);
}

//---------------------------------------------------------------------------
float Terrain::getTerrainLight (const Stuff::Vector3D &position)
{
	float result = mapData->terrainLight(position);
	return(result);
}

//---------------------------------------------------------------------------
Stuff::Vector3D Terrain::getTerrainNormal (const Stuff::Vector3D &position)
{
	Stuff::Vector3D result = Terrain::mapData->terrainNormal(position);
	return(result);
}

//---------------------------------------------------------------------------
// Uses a simple value to mark radius.  It never changes now!!
// First value in range table!!
void Terrain::markSeen (const Stuff::Vector3D &looker, byte who, float specialUnitExpand)
{
	return;

	/*		Not needed anymore.  Real LOS now.
	//-----------------------------------------------------------
	// This function marks vertices has being seen by a given side.
	Stuff::Vector3D position = looker;
	position.x -= mapTopLeft3d.x;
	position.y = mapTopLeft3d.y - looker.y;
	
	Stuff::Vector2DOf<float> upperLeft;
	upperLeft.x = floor(position.x * oneOverWorldUnitsPerVertex);
	upperLeft.y = floor(position.y * oneOverWorldUnitsPerVertex);

	Stuff::Vector2DOf<long> meshOffset;
	meshOffset.x = float2long(upperLeft.x);
	meshOffset.y = float2long(upperLeft.y);

	unsigned long xCenter = meshOffset.x;
	unsigned long yCenter = meshOffset.y;

	//Figure out altitude above minimum terrain altitude and look up in table.
	float baseElevation = MapData::waterDepth;
	if (MapData::waterDepth < Terrain::userMin)
		baseElevation = Terrain::userMin;

	float altitude = position.z - baseElevation;
	float altitudeIntegerRange = (Terrain::userMax - baseElevation) * 0.00390625f;
	long altLevel = 0;
	if (altitudeIntegerRange > Stuff::SMALL)
		altLevel = altitude / altitudeIntegerRange;
	
	if (altLevel < 0)
		altLevel = 0;

	if (altLevel > 255)
		altLevel = 255;

	float radius = visualRangeTable[altLevel];
	
	radius += (radius * specialUnitExpand);

	if (radius <= 0.0f)
		return;

	//-----------------------------------------------------
	// Who is the shift value to create the mask
	BYTE wer = (1 << who);

	VisibleBits->setCircle(xCenter,yCenter,float2long(radius),wer);
	*/
}

//---------------------------------------------------------------------------
// Uses dist passed in as radius.
void Terrain::markRadiusSeen (const Stuff::Vector3D &looker, float dist, byte who)
{
	return;

	//Not needed.  Real LOS now!
	/*
	if (dist <= 0.0f)
		return;

	//-----------------------------------------------------------
	// This function marks vertices has being seen by
	// a given side.
	dist *= worldUnitsPerMeter;
	dist *= Terrain::oneOverWorldUnitsPerVertex;
	
	Stuff::Vector3D position = looker;
	position.x -= mapTopLeft3d.x;
	position.y = mapTopLeft3d.y - looker.y;
	
	Stuff::Vector2DOf<float> upperLeft;
	upperLeft.x = floor(position.x * oneOverWorldUnitsPerVertex);
	upperLeft.y = floor(position.y * oneOverWorldUnitsPerVertex);

	Stuff::Vector2DOf<long> meshOffset;
	meshOffset.x = floor(upperLeft.x);
	meshOffset.y = floor(upperLeft.y);

	unsigned long xCenter = meshOffset.x;
	unsigned long yCenter = meshOffset.y;

	//-----------------------------------------------------
	// Who is the shift value to create the mask
	BYTE wer = (1 << who);

	VisibleBits->setCircle(xCenter,yCenter,dist,wer);
	*/
}

//---------------------------------------------------------------------------
void Terrain::setObjBlockActive (long blockNum, bool active)
{
	if ((blockNum >= 0) && (blockNum < numObjBlocks))
		objBlockInfo[blockNum].active = active;	
}	

//---------------------------------------------------------------------------
void Terrain::clearObjBlocksActive (void)
{
	for (long i = 0; i < numObjBlocks; i++)
		setObjBlockActive(i, false);
}	

//---------------------------------------------------------------------------
void Terrain::setObjVertexActive (long vertexNum, bool active)
{
	if ( (vertexNum >= 0) && (vertexNum < (realVerticesMapSide * realVerticesMapSide)) )
		objVertexActive[vertexNum] = active;	
}	

//---------------------------------------------------------------------------
void Terrain::clearObjVerticesActive (void)
{
	memset(objVertexActive,0,sizeof(bool) * realVerticesMapSide * realVerticesMapSide);
}

//---------------------------------------------------------------------------
long Terrain::save( PacketFile* fileName, int whichPacket, bool quickSave )
{ 
	if (!quickSave)
	{
		recalcShadows = true;
		mapData->calcLight();
	}
	else
	{
		recalcShadows = false;
	}
		
	return mapData->save( fileName, whichPacket ); 
}


//-----------------------------------------------------
bool Terrain::save( FitIniFile* fitFile )
{
	// write out the water info
#ifdef _DEBUG
	long result = 
#endif
	fitFile->writeBlock( "Water" );
	gosASSERT( result > 0 );


	fitFile->writeIdFloat( "Elevation", mapData->waterDepth );
	fitFile->writeIdFloat( "Frequency", waterFreq );
	fitFile->writeIdFloat( "Ampliture", waterAmplitude );
	fitFile->writeIdULong( "AlphaShallow", alphaEdge );
	fitFile->writeIdULong( "AlphaMiddle", alphaMiddle );
	fitFile->writeIdULong( "AlphaDeep", alphaDeep );
	fitFile->writeIdFloat( "AlphaDepth", mapData->alphaDepth );
	fitFile->writeIdFloat( "ShallowDepth", mapData->shallowDepth );

	fitFile->writeBlock( "Terrain" );
	fitFile->writeIdLong( "UserMin", userMin );
	fitFile->writeIdLong( "UserMax", userMax );
	fitFile->writeIdFloat( "TerrainMinX", tileColToWorldCoord[0] );
	fitFile->writeIdFloat( "TerrainMinY", tileRowToWorldCoord[0] );
	fitFile->writeIdUChar( "Noise", fractalNoise);
	fitFile->writeIdUChar( "Threshold", fractalThreshold);

	if (terrainTextures2)
	{
		terrainTextures2->saveTilingFactors(fitFile);
	}
	return true;
}

bool Terrain::load( FitIniFile* fitFile )
{
	// write out the water info
	long result = fitFile->seekBlock( "Water" );
	gosASSERT( result == NO_ERR );

	result = fitFile->readIdFloat( "Elevation", mapData->waterDepth );
	gosASSERT( result == NO_ERR );
	waterElevation = mapData->waterDepth;
	result = fitFile->readIdFloat( "Frequency", waterFreq );
	gosASSERT( result == NO_ERR );
	result = fitFile->readIdFloat( "Ampliture", waterAmplitude );
	gosASSERT( result == NO_ERR );
	result = fitFile->readIdULong( "AlphaShallow", alphaEdge );
	gosASSERT( result == NO_ERR );
	result = fitFile->readIdULong( "AlphaMiddle", alphaMiddle );
	gosASSERT( result == NO_ERR );
	result = fitFile->readIdULong( "AlphaDeep", alphaDeep );
	gosASSERT( result == NO_ERR );
	result = fitFile->readIdFloat( "AlphaDepth", mapData->alphaDepth );
	gosASSERT( result == NO_ERR );
	result = fitFile->readIdFloat( "ShallowDepth", mapData->shallowDepth );
	gosASSERT( result == NO_ERR );

	fitFile->seekBlock( "Terrain" );
	fitFile->readIdLong( "UserMin", userMin );
	fitFile->readIdLong( "UserMax", userMax );

	fitFile->readIdUChar( "Noise", fractalNoise);
	fitFile->readIdUChar( "Threshold", fractalThreshold);

	return true;

}

//---------------------------------------------------------------------------
void Terrain::unselectAll()
{
	mapData->unselectAll();
}

//---------------------------------------------------------------------------
void Terrain::selectVerticesInRect( const Stuff::Vector4D& topLeft, const Stuff::Vector4D& bottomRight, bool bToggle )
{
	Stuff::Vector3D worldPos;
	Stuff::Vector4D screenPos;

	int xMin, xMax;
	int yMin, yMax;

	if ( topLeft.x < bottomRight.x )
	{
		xMin = topLeft.x;
		xMax = bottomRight.x;
	}
	else
	{
		xMin = bottomRight.x;
		xMax = topLeft.x;
	}

	if ( topLeft.y < bottomRight.y )
	{
		yMin = topLeft.y;
		yMax = bottomRight.y;
	}
	else
	{
		yMin = bottomRight.y;
		yMax = topLeft.y;
	}
	
	for ( int i = 0; i < realVerticesMapSide; ++i )
	{
		for ( int j = 0; j < realVerticesMapSide; ++j )
		{
			worldPos.y = tileRowToWorldCoord[j];
			worldPos.x = tileColToWorldCoord[i];
			worldPos.z = mapData->terrainElevation( j, i );

			eye->projectZ( worldPos, screenPos );

			if ( screenPos.x >= xMin && screenPos.x <= xMax &&
				 screenPos.y >= yMin && screenPos.y <= yMax )
			{
				mapData->selectVertex( j, i, true, bToggle );		
			}
		}
	}
}

//---------------------------------------------------------------------------
bool Terrain::hasSelection()
{
	return mapData->selection();
}

//---------------------------------------------------------------------------
bool Terrain::isVertexSelected( long tileR, long tileC )
{
	return mapData->isVertexSelected( tileR, tileC );
}

//---------------------------------------------------------------------------
bool Terrain::selectVertex( long tileR, long tileC, bool bSelect )
{
	//We never use the return value so just send back false.
	if ( (tileR <= -1) || (tileR >= realVerticesMapSide) )
		return false;

	if ( (tileC <= -1) || (tileC >= realVerticesMapSide) )
		return false;

	mapData->selectVertex( tileR, tileC, bSelect, 0 );
	return true;
}

//---------------------------------------------------------------------------
float Terrain::getHighestVertex( long& tileR, long& tileC )
{
	float highest = -9999999.; // an absurdly small number
	for ( int i = 0; i < realVerticesMapSide * realVerticesMapSide; ++i )
	{
		float tmp = getVertexHeight( i );
		if ( tmp > highest )
		{
			highest = tmp;
			tileR = i/realVerticesMapSide;
			tileC = i % realVerticesMapSide;
		}
	}

	return highest;
}

//---------------------------------------------------------------------------
float Terrain::getLowestVertex(  long& tileR, long& tileC )
{
	float lowest = 9999999.; // an absurdly big number
	for ( int i = 0; i < realVerticesMapSide * realVerticesMapSide; ++i )
	{
		float tmp = getVertexHeight( i );
		if ( tmp < lowest )
		{
			lowest = tmp;
			tileR = i/realVerticesMapSide;
			tileC = i % realVerticesMapSide;
		}
	}

	return lowest;
}

//---------------------------------------------------------------------------
void  Terrain::setUserSettings( long min, long max, int terrainType )
{
	userMin = min;
	userMax = max;
	baseTerrain = terrainType;
}

//---------------------------------------------------------------------------
void Terrain::getUserSettings( long& min, long& max, int& terrainType )
{
	min = userMin;
	max = userMax;
	terrainType = baseTerrain;
}

//---------------------------------------------------------------------------
void Terrain::recalcWater()
{
	mapData->recalcWater();
}

//---------------------------------------------------------------------------
void Terrain::reCalcLight(bool doShadows)
{
	recalcLight = true;
	recalcShadows = doShadows;
	
	//Do a new burnin for the colormap
	if (terrainTextures2)
	{
		if (colorMapName)
			terrainTextures2->recalcLight(colorMapName);
		else
			terrainTextures2->recalcLight(terrainName);
	}
}

//---------------------------------------------------------------------------
void Terrain::clearShadows()
{
	mapData->clearShadows();
}

//---------------------------------------------------------------------------

long Terrain::getWater (const Stuff::Vector3D& worldPos) {
	//-------------------------------------------------
	// Get elevation at this point and compare to deep
	// water altitude for this map.
	float elevation = getTerrainElevation(worldPos);
	
	if (elevation < (waterElevation - MapData::shallowDepth))
		return(2);
	if (elevation < waterElevation)
		return(1);
	return(0);
}

//---------------------------------------------------------------------------
