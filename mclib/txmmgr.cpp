//---------------------------------------------------------------------
//
//
// This class will manage the texture memory provided by GOS
// GOS gives me a maximum of 256 256x256 pixel texture pages.
// I want GOS to think I only use 256x256 textures.  This class
// will insure that GOS believes that completely and provided
// smaller texture surfaces out of the main surface if necessary
// as well as returning the necessary UVs to get to the other surface.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#ifndef TGAINFO_H
#include"tgainfo.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

#ifndef TIMING_H
#include"timing.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef LZ_H
#include"lz.h"
#endif

#ifndef CIDENT_H
#include"cident.h"
#endif

#ifndef PATHS_H
#include"paths.h"
#endif

#include<gameos.hpp>
#include<mlr/mlr.hpp>
#include<gosfx/gosfxheaders.hpp>

//---------------------------------------------------------------------------
// static globals
MC_TextureManager *mcTextureManager = NULL;
gos_VERTEXManager *MC_TextureManager::gvManager = NULL;
MemoryPtr			MC_TextureManager::lzBuffer1 = NULL;
MemoryPtr			MC_TextureManager::lzBuffer2 = NULL;
int				MC_TextureManager::iBufferRefCount = 0;

bool MLRVertexLimitReached = false;
extern bool useFog;

DWORD actualTextureSize = 0;
DWORD compressedTextureSize = 0;

#define MAX_SENDDOWN		10002

//------------------------------------------------------
// Frees up gos_VERTEX manager memory
void MC_TextureManager::freeVertices (void)
{
	if (gvManager)
	{
		gvManager->destroy();
		delete gvManager;
		gvManager = NULL;
	}
}
		
//------------------------------------------------------
// Creates gos_VERTEX Manager and allocates RAM.  Will not allocate if already done!
void MC_TextureManager::startVertices (long maxVertices)
{
	if (gvManager == NULL)
	{
		gvManager = new gos_VERTEXManager;
		gvManager->init(maxVertices);
		gvManager->reset();
	}
}
	 
//----------------------------------------------------------------------
// Class MC_TextureManager
void MC_TextureManager::start (void)
{
	init();

	//------------------------------------------
	// Create nodes from systemHeap.
	long nodeRAM = MC_MAXTEXTURES * sizeof(MC_TextureNode);
	masterTextureNodes = (MC_TextureNode *)systemHeap->Malloc(nodeRAM);
	gosASSERT(masterTextureNodes != NULL);

	for (long i=0;i<MC_MAXTEXTURES;i++)
		masterTextureNodes[i].init();
		
	//-------------------------------------------
	// Create VertexNodes from systemHeap
	nodeRAM = MC_MAXTEXTURES * sizeof(MC_VertexArrayNode);
	masterVertexNodes = (MC_VertexArrayNode *)systemHeap->Malloc(nodeRAM);
	gosASSERT(masterVertexNodes != NULL);
	
	memset(masterVertexNodes,0,nodeRAM);

	textureCacheHeap = new UserHeap;
	textureCacheHeap->init(TEXTURE_CACHE_SIZE,"TXMCache");
	textureCacheHeap->setMallocFatals(false);
	
	textureStringHeap = new UserHeap;
	textureStringHeap->init(512000,"TXMString");

	if (!textureManagerInstrumented)
	{
		StatisticFormat( "" );
		StatisticFormat( "MechCommander 2 Texture Manager" );
		StatisticFormat( "===============================" );
		StatisticFormat( "" );

		AddStatistic("Handles Used","Handles",gos_DWORD, &(currentUsedTextures), Stat_Total);

		AddStatistic("Cache Misses","",gos_DWORD, &(totalCacheMisses), Stat_Total);

		StatisticFormat( "" );
		StatisticFormat( "" );

		textureManagerInstrumented = true;
	}
	
	indexArray = (WORD *)systemHeap->Malloc(sizeof(WORD) * MC_MAXFACES);
	for (int i=0;i<MC_MAXFACES;i++)
		indexArray[i] = i;
		
	//Add an Empty Texture node for all untextured triangles to go down into.
	masterTextureNodes[0].gosTextureHandle = 0;
	masterTextureNodes[0].nodeName = NULL;
	masterTextureNodes[0].uniqueInstance = false;
	masterTextureNodes[0].neverFLUSH = 0x1;
	masterTextureNodes[0].numUsers = 0;
	masterTextureNodes[0].key = gos_Texture_Solid;
	masterTextureNodes[0].hints = 0; 
	masterTextureNodes[0].width = 0;
	masterTextureNodes[0].lastUsed = -1;
	masterTextureNodes[0].textureData = NULL;
}

extern Stuff::MemoryStream *effectStream;
extern MidLevelRenderer::MLRClipper * theClipper;
//----------------------------------------------------------------------
void MC_TextureManager::destroy (void)
{
	if (masterTextureNodes)
	{
		//-----------------------------------------------------
		// Traverses list of texture nodes and frees each one.
		long usedCount = 0;
		for (long i=0;i<MC_MAXTEXTURES;i++)
			masterTextureNodes[i].destroy();		// Destroy for nodes whacks GOS Handle
		
		currentUsedTextures = usedCount;			//Can this have been the damned bug all along!?
	}
	
	gos_PushCurrentHeap(MidLevelRenderer::Heap);

	delete MidLevelRenderer::MLRTexturePool::Instance;
	MidLevelRenderer::MLRTexturePool::Instance = NULL; 

	delete theClipper;
	theClipper = NULL;
	
	gos_PopCurrentHeap();

	//------------------------------------------------------
	// Shutdown the GOS FX and MLR.
	gos_PushCurrentHeap(gosFX::Heap);
	
	delete gosFX::EffectLibrary::Instance;
	gosFX::EffectLibrary::Instance = NULL;

	delete effectStream;
	effectStream = NULL;
	
	delete gosFX::LightManager::Instance;
	gosFX::LightManager::Instance = NULL;

	gos_PopCurrentHeap();

	//------------------------------------------
	// free SystemHeap Memory
	systemHeap->Free(masterTextureNodes);
	masterTextureNodes = NULL;
	
	systemHeap->Free(masterVertexNodes);
	masterVertexNodes = NULL;
	
	delete textureCacheHeap;
	textureCacheHeap = NULL;

	delete textureStringHeap;
	textureStringHeap = NULL;
}

//----------------------------------------------------------------------
MC_TextureManager::~MC_TextureManager (void)
{
	MC_TextureManager::iBufferRefCount--;
	if (0 == MC_TextureManager::iBufferRefCount)
	{
		if (lzBuffer1)
		{
			gosASSERT(lzBuffer2 != NULL);
			if (textureCacheHeap)
			{
				textureCacheHeap->Free(lzBuffer1);
				textureCacheHeap->Free(lzBuffer2);
			}
			lzBuffer1 = NULL;
			lzBuffer2 = NULL;
		}
	}

	destroy();
}

//----------------------------------------------------------------------
void MC_TextureManager::flush (bool justTextures)
{
	if (masterTextureNodes)
	{
		//-----------------------------------------------------
		// Traverses list of texture nodes and frees each one.
		long usedCount = 0;
		for (long i=0;i<MC_MAXTEXTURES;i++)
		{
			if (!masterTextureNodes[i].neverFLUSH)
				masterTextureNodes[i].destroy();		// Destroy for nodes whacks GOS Handle
		}
		
		currentUsedTextures = usedCount;				//Can this have been the damned bug all along!?
	}
	
	//If we just wanted to free up RAM, just return and let the MUNGA stuff go later.
	if (justTextures)
		return;

	gos_PushCurrentHeap(MidLevelRenderer::Heap);

	delete MidLevelRenderer::MLRTexturePool::Instance;
	MidLevelRenderer::MLRTexturePool::Instance = NULL; 

	delete theClipper;
	theClipper = NULL;
	
	gos_PopCurrentHeap();

	//------------------------------------------------------
	// Shutdown the GOS FX and MLR.
	gos_PushCurrentHeap(gosFX::Heap);
	
	delete gosFX::EffectLibrary::Instance;
	gosFX::EffectLibrary::Instance = NULL;

	delete effectStream;
	effectStream = NULL;
	
	delete gosFX::LightManager::Instance;
	gosFX::LightManager::Instance = NULL;

	gos_PopCurrentHeap();

	//------------------------------------------------------
	//Restart MLR and the GOSFx
	gos_PushCurrentHeap(MidLevelRenderer::Heap);

	MidLevelRenderer::TGAFilePool *pool = new MidLevelRenderer::TGAFilePool("data" PATH_SEPARATOR "tgl" PATH_SEPARATOR "128" PATH_SEPARATOR);
	MidLevelRenderer::MLRTexturePool::Instance = new MidLevelRenderer::MLRTexturePool(pool);

	MidLevelRenderer::MLRSortByOrder *cameraSorter = new MidLevelRenderer::MLRSortByOrder(MidLevelRenderer::MLRTexturePool::Instance);
	theClipper = new MidLevelRenderer::MLRClipper(0, cameraSorter);
	
	gos_PopCurrentHeap();

	//------------------------------------------------------
	// ReStart the GOS FX.
	gos_PushCurrentHeap(gosFX::Heap);
	
	gosFX::EffectLibrary::Instance = new gosFX::EffectLibrary();
	Check_Object(gosFX::EffectLibrary::Instance);

	FullPathFileName effectsName;
	effectsName.init(effectsPath,"mc2.fx","");

	File effectFile;
	long result = effectFile.open(effectsName);
	if (result != NO_ERR)
		STOP(("Could not find MC2.fx"));
		
	long effectsSize = effectFile.fileSize();
	MemoryPtr effectsData = (MemoryPtr)systemHeap->Malloc(effectsSize);
	effectFile.read(effectsData,effectsSize);
	effectFile.close();
	
	effectStream = new Stuff::MemoryStream(effectsData,effectsSize);
	gosFX::EffectLibrary::Instance->Load(effectStream);
	
	gosFX::LightManager::Instance = new gosFX::LightManager();

	gos_PopCurrentHeap();

	systemHeap->Free(effectsData);
}

//----------------------------------------------------------------------
void MC_TextureManager::removeTextureNode (DWORD textureNode)
{
	if (textureNode != 0xffffffff)
	{
		//-----------------------------------------------------------
		masterTextureNodes[textureNode].destroy();
		if (masterTextureNodes[textureNode].textureData)
		{
			textureCacheHeap->Free(masterTextureNodes[textureNode].textureData);
			masterTextureNodes[textureNode].textureData = NULL;

			if (masterTextureNodes[textureNode].nodeName)
			{
				textureStringHeap->Free(masterTextureNodes[textureNode].nodeName);
				masterTextureNodes[textureNode].nodeName = NULL;
			}
		}
	}
}

//----------------------------------------------------------------------
void MC_TextureManager::removeTexture (DWORD gosHandle)
{
	//-----------------------------------------------------------
    long i = 0;
	for (;i<MC_MAXTEXTURES;i++)
	{
		if (masterTextureNodes[i].gosTextureHandle == gosHandle)
		{
			masterTextureNodes[i].numUsers--;
			break;			
		}
	}
	
	if (i < MC_MAXTEXTURES && masterTextureNodes[i].numUsers == 0)
	{
		masterTextureNodes[i].destroy();
		if (masterTextureNodes[i].textureData)
		{
			textureCacheHeap->Free(masterTextureNodes[i].textureData);
			masterTextureNodes[i].textureData = NULL;

			if (masterTextureNodes[i].nodeName)
			{
				textureStringHeap->Free(masterTextureNodes[i].nodeName);
				masterTextureNodes[i].nodeName = NULL;
			}
		}
	}
}

#define cache_Threshold		150
//----------------------------------------------------------------------
bool MC_TextureManager::flushCache (void)
{
	bool cacheNotFull = false;
	totalCacheMisses++;
	currentUsedTextures = 0;
	
	//Count ACTUAL number of textures being used.
	// ALSO can't count on turn being right.  Logistics does not update unless simple Camera is up!!
	for (long i=0;i<MC_MAXTEXTURES;i++)
	{
		if ((masterTextureNodes[i].gosTextureHandle != CACHED_OUT_HANDLE) &&
			(masterTextureNodes[i].gosTextureHandle != 0xffffffff))
		{
			currentUsedTextures++;
		}
	}

	//If we are now below the magic number, return that the cache is NOT full.
	if (currentUsedTextures < MAX_MC2_GOS_TEXTURES)
		return true;

	for (int i=0;i<MC_MAXTEXTURES;i++)
	{
		if ((masterTextureNodes[i].gosTextureHandle != CACHED_OUT_HANDLE) &&
			(masterTextureNodes[i].gosTextureHandle != 0xffffffff) &&
			(!masterTextureNodes[i].uniqueInstance))
		{
			if (masterTextureNodes[i].lastUsed <= (turn-cache_Threshold))
			{
				//----------------------------------------------------------------
				// Cache this badboy out.  Textures don't change.  Just Destroy!
				if (masterTextureNodes[i].gosTextureHandle)
					gos_DestroyTexture(masterTextureNodes[i].gosTextureHandle);

				masterTextureNodes[i].gosTextureHandle = CACHED_OUT_HANDLE;
				
				currentUsedTextures--;
				cacheNotFull = true;
				return cacheNotFull;
			}
		}
	}
	
	for (int i=0;i<MC_MAXTEXTURES;i++)
	{
		if ((masterTextureNodes[i].gosTextureHandle != CACHED_OUT_HANDLE) &&
			(masterTextureNodes[i].gosTextureHandle != 0xffffffff) &&
			(masterTextureNodes[i].gosTextureHandle) &&
			(!masterTextureNodes[i].uniqueInstance))
		{
			if (masterTextureNodes[i].lastUsed <= (turn-30))
			{
				//----------------------------------------------------------------
				// Cache this badboy out.  Textures don't change.  Just Destroy!
				if (masterTextureNodes[i].gosTextureHandle)
					gos_DestroyTexture(masterTextureNodes[i].gosTextureHandle);

				masterTextureNodes[i].gosTextureHandle = CACHED_OUT_HANDLE;
				
				currentUsedTextures--;
				cacheNotFull = true;
				return cacheNotFull;
			}
		}
	}
	
	for (int i=0;i<MC_MAXTEXTURES;i++)
	{
		if ((masterTextureNodes[i].gosTextureHandle != CACHED_OUT_HANDLE) &&
			(masterTextureNodes[i].gosTextureHandle != 0xffffffff) &&
			(!masterTextureNodes[i].uniqueInstance))
		{
			if (masterTextureNodes[i].lastUsed <= (turn-1))
			{
				//----------------------------------------------------------------
				// Cache this badboy out.  Textures don't change.  Just Destroy!
				if (masterTextureNodes[i].gosTextureHandle)
					gos_DestroyTexture(masterTextureNodes[i].gosTextureHandle);

				masterTextureNodes[i].gosTextureHandle = CACHED_OUT_HANDLE;
				
				currentUsedTextures--;
				cacheNotFull = true;
				return cacheNotFull;
			}
		}
	}
	
  	//gosASSERT(cacheNotFull);
	return cacheNotFull;
}

//----------------------------------------------------------------------
// Draws all textures with isTerrain set that are solid first,
// then draws all alpha with isTerrain set.
void MC_TextureManager::renderLists (void)
{
	if (Environment.Renderer == 3)
	{
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
		gos_SetRenderState( gos_State_MonoEnable, 1);
		gos_SetRenderState( gos_State_Perspective, 0);
		gos_SetRenderState( gos_State_Clipping, 1);
		gos_SetRenderState( gos_State_AlphaTest, 0);
		gos_SetRenderState( gos_State_Specular, 0);
		gos_SetRenderState( gos_State_Dither, 0);
		gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendDecal);
		gos_SetRenderState( gos_State_Filter, gos_FilterNone);
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
		gos_SetRenderState( gos_State_ZCompare, 1);
		gos_SetRenderState(	gos_State_ZWrite, 1);
	}
	else
	{
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
		gos_SetRenderState( gos_State_MonoEnable, 0);
		gos_SetRenderState( gos_State_Perspective, 1);
		gos_SetRenderState( gos_State_Clipping, 1);
		gos_SetRenderState( gos_State_AlphaTest, 0);
		gos_SetRenderState( gos_State_Specular, 1);
		gos_SetRenderState( gos_State_Dither, 1);
		gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulate);
		gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear);
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
		gos_SetRenderState( gos_State_ZCompare, 1);
		gos_SetRenderState(	gos_State_ZWrite, 1);
	}
		
	DWORD fogColor = eye->fogColor;
	//-----------------------------------------------------
	// FOG time.  Set Render state to FOG on!
	if (useFog)
	{
		//gos_SetRenderState( gos_State_Fog, (int)&fogColor);
		gos_SetRenderState( gos_State_Fog, fogColor); // sebi
	}
	else
	{
		gos_SetRenderState( gos_State_Fog, 0);
	}
	
	for (long i=0;i<nextAvailableVertexNode;i++)
	{
		if ((masterVertexNodes[i].flags & MC2_DRAWSOLID) &&
			(masterVertexNodes[i].vertices))
		{
			if (masterVertexNodes[i].flags & MC2_ISTERRAIN)
				gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
			else
				gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );

			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}

			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}

			//Reset the list to zero length to avoid drawing more then once!
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}
	
	if (Environment.Renderer == 3)
	{
		//Do NOT draw the water as transparent in software
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
	}
	else
	{
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
		gos_SetRenderState( gos_State_AlphaTest, 1);
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
	}
		
	for (int i=0;i<nextAvailableVertexNode;i++)
	{
		if ((masterVertexNodes[i].flags & MC2_ISTERRAIN) &&
			(masterVertexNodes[i].flags & MC2_DRAWALPHA) &&
			!(masterVertexNodes[i].flags & MC2_ISCRATERS) &&
			(masterVertexNodes[i].vertices))
		{
			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}
	
			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}
			
			//Reset the list to zero length to avoid drawing more then once!			
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}

	if (Environment.Renderer == 3)
	{
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
		gos_SetRenderState( gos_State_AlphaTest, 1);
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
	}
	
	gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
	gos_SetRenderState(	gos_State_ZWrite, 0);
	gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
	
 	//Draw the Craters after the detail textures on the terrain.  There should never be anything here in the OLD universe.
	// DO NOT draw craters or footprints in software
	if (Environment.Renderer != 3)
	{
		for (int i=0;i<nextAvailableVertexNode;i++)
		{
			if (!(masterVertexNodes[i].flags & MC2_ISTERRAIN) &&
				(masterVertexNodes[i].flags & MC2_DRAWALPHA) &&
				(masterVertexNodes[i].flags & MC2_ISCRATERS) && 
				(masterVertexNodes[i].vertices))
			{
				DWORD totalVertices = masterVertexNodes[i].numVertices;
				if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
				{
					totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
				}
		
				if (totalVertices && (totalVertices < MAX_SENDDOWN))
				{
					gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
					gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
				}
				else if (totalVertices > MAX_SENDDOWN)
				{
					gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
					
					//Must divide up vertices into batches of 10,000 each to send down.
					// Somewhere around 20000 to 30000 it really gets screwy!!!
					long currentVertices = 0;
					while (currentVertices < totalVertices)
					{
						gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
						long tVertices = totalVertices - currentVertices;
						if (tVertices > MAX_SENDDOWN)
							tVertices = MAX_SENDDOWN;
						
						gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
						
						currentVertices += tVertices;
					}
				}
				
				//Reset the list to zero length to avoid drawing more then once!			
				//Also comes in handy if gameLogic is not called.
				masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
			}
		}
	}

	if (Environment.Renderer != 3)
	{
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
		// sebi: do not draw in depth for terrain overlays, otherwise other overlay data, like craters, start to flicker
		gos_SetRenderState(	gos_State_ZWrite, 0);
	}

 	//Draw the Overlays after the detail textures on the terrain.  There should never be anything here in the OLD universe.
	for (int i=0;i<nextAvailableVertexNode;i++)
	{
		if ((masterVertexNodes[i].flags & MC2_ISTERRAIN) &&
			(masterVertexNodes[i].flags & MC2_DRAWALPHA) &&
			(masterVertexNodes[i].flags & MC2_ISCRATERS) && 
			(masterVertexNodes[i].vertices))
		{
			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}
	
			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}
			
			//Reset the list to zero length to avoid drawing more then once!			
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}

	if (Environment.Renderer == 3)
	{
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
		gos_SetRenderState(	gos_State_ZWrite, 1);
		gos_SetRenderState( gos_State_ZCompare, 2);
	}
	else
	{
		gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
		gos_SetRenderState( gos_State_MonoEnable, 1);
		gos_SetRenderState( gos_State_Perspective, 0);
		gos_SetRenderState( gos_State_Specular, 1);
		// sebi: shadows do not draw in depth, we do not want z-fighting
		gos_SetRenderState(	gos_State_ZWrite, 0);
		gos_SetRenderState( gos_State_ZCompare, 2);
	}

	//NEVER draw shadows in Software.
	if (Environment.Renderer != 3)
	{
		for (int i=0;i<nextAvailableVertexNode;i++)
		{
			 if	((masterVertexNodes[i].flags & MC2_ISSHADOWS) &&
				(masterVertexNodes[i].flags & MC2_DRAWALPHA) &&
				(masterVertexNodes[i].vertices))
			{
				DWORD totalVertices = masterVertexNodes[i].numVertices;
				if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
				{
					totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
				}
			
				if (totalVertices && (totalVertices < MAX_SENDDOWN))
				{
					gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
					gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
				}
				else if (totalVertices > MAX_SENDDOWN)
				{
					gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
					
					//Must divide up vertices into batches of 10,000 each to send down.
					// Somewhere around 20000 to 30000 it really gets screwy!!!
					long currentVertices = 0;
					while (currentVertices < totalVertices)
					{
						gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
						long tVertices = totalVertices - currentVertices;
						if (tVertices > MAX_SENDDOWN)
							tVertices = MAX_SENDDOWN;
						
						gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
						
						currentVertices += tVertices;
					}
				}
				
				//Reset the list to zero length to avoid drawing more then once!
				//Also comes in handy if gameLogic is not called.
				masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
			}
		}
	}


	gos_SetRenderState( gos_State_ZCompare, 1);
	if (Environment.Renderer != 3)
	{
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
		gos_SetRenderState(	gos_State_ZWrite, 1);
	}
	
  	for (int i=0;i<nextAvailableVertexNode;i++)
	{
		if (!(masterVertexNodes[i].flags & MC2_ISTERRAIN) &&
			!(masterVertexNodes[i].flags & MC2_ISSHADOWS) &&
			!(masterVertexNodes[i].flags & MC2_ISCOMPASS) &&
			!(masterVertexNodes[i].flags & MC2_ISCRATERS) &&
			(masterVertexNodes[i].flags & MC2_DRAWALPHA) &&
			(masterVertexNodes[i].vertices))
		{
			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}
		
			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}
			
			//Reset the list to zero length to avoid drawing more then once!
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}
	
	if (Environment.Renderer == 3)
	{
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
		gos_SetRenderState( gos_State_ZCompare, 1);
		gos_SetRenderState( gos_State_Fog, 0);
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneOne);
		gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);
		gos_SetRenderState(	gos_State_ZWrite, 0);
		gos_SetRenderState( gos_State_MonoEnable, 1);
	}
	else
	{
		gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
		gos_SetRenderState( gos_State_Perspective, 1);
		gos_SetRenderState( gos_State_ZCompare, 1);
		gos_SetRenderState( gos_State_Fog, 0);
		gos_SetRenderState( gos_State_Specular, 0);
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneOne);
		gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);
		gos_SetRenderState(	gos_State_ZWrite, 0);
		gos_SetRenderState( gos_State_MonoEnable, 0);
	}
				
	for (int i=0;i<nextAvailableVertexNode;i++)
	{
		if ((masterVertexNodes[i].flags & MC2_ISEFFECTS) &&
			(masterVertexNodes[i].vertices))
		{
			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}
			
			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}
	
			//Reset the list to zero length to avoid drawing more then once!
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}
	
	gos_SetRenderState(	gos_State_ZWrite, 1);
	
	for (int i=0;i<nextAvailableVertexNode;i++)
	{
		if ((masterVertexNodes[i].flags & MC2_ISSPOTLGT) &&
			(masterVertexNodes[i].vertices))
		{
			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}
			
			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}
	
			//Reset the list to zero length to avoid drawing more then once!
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}
	
	gos_SetRenderState( gos_State_ZWrite, 0);
	gos_SetRenderState( gos_State_ZCompare, 0);
	gos_SetRenderState( gos_State_Perspective, 1);
 	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
	gos_SetRenderState( gos_State_AlphaTest, 1);
	
 	for (int i=0;i<nextAvailableVertexNode;i++)
	{
		if ((masterVertexNodes[i].flags & MC2_ISCOMPASS) &&
			(masterVertexNodes[i].vertices))
		{
			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}
			
			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}
	
			//Reset the list to zero length to avoid drawing more then once!
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}
	
	gos_SetRenderState( gos_State_Filter, gos_FilterNone);
	
 	for (int i=0;i<nextAvailableVertexNode;i++)
	{
		if ((masterVertexNodes[i].flags & MC2_ISHUDLMNT) &&
			(masterVertexNodes[i].vertices))
		{
			DWORD totalVertices = masterVertexNodes[i].numVertices;
			if (masterVertexNodes[i].currentVertex != (masterVertexNodes[i].vertices + masterVertexNodes[i].numVertices))
			{
				totalVertices = masterVertexNodes[i].currentVertex - masterVertexNodes[i].vertices;
			}
			
			if (totalVertices && (totalVertices < MAX_SENDDOWN))
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				gos_RenderIndexedArray( masterVertexNodes[i].vertices, totalVertices, indexArray, totalVertices );
			}
			else if (totalVertices > MAX_SENDDOWN)
			{
				gos_SetRenderState( gos_State_Texture, masterTextureNodes[masterVertexNodes[i].textureIndex].get_gosTextureHandle());
				
				//Must divide up vertices into batches of 10,000 each to send down.
				// Somewhere around 20000 to 30000 it really gets screwy!!!
				long currentVertices = 0;
				while (currentVertices < totalVertices)
				{
					gos_VERTEX *v = masterVertexNodes[i].vertices + currentVertices;
					long tVertices = totalVertices - currentVertices;
					if (tVertices > MAX_SENDDOWN)
						tVertices = MAX_SENDDOWN;
					
					gos_RenderIndexedArray(v, tVertices, indexArray, tVertices );
					
					currentVertices += tVertices;
				}
			}
	
			//Reset the list to zero length to avoid drawing more then once!
			//Also comes in handy if gameLogic is not called.
			masterVertexNodes[i].currentVertex = masterVertexNodes[i].vertices;
		}
	}

	//Must turn zCompare back on for FXs
	gos_SetRenderState( gos_State_ZCompare, 1 );
}

//----------------------------------------------------------------------
DWORD MC_TextureManager::update (void)
{
	DWORD numTexturesFreed = 0;
	currentUsedTextures = 0;
	
	for (long i=0;i<MC_MAXTEXTURES;i++)
	{
		if ((masterTextureNodes[i].gosTextureHandle != CACHED_OUT_HANDLE) &&
			(masterTextureNodes[i].gosTextureHandle != 0xffffffff))
		{
			if (!masterTextureNodes[i].uniqueInstance &&
				!(masterTextureNodes[i].neverFLUSH & 1))		//Only uncachable if BIT 1 is set, otherwise, cache 'em out!
			{
				if (masterTextureNodes[i].lastUsed <= (turn-60))
				{
					//----------------------------------------------------------------
					// Cache this badboy out.  Textures don't change.  Just Destroy!
					if (masterTextureNodes[i].gosTextureHandle)
						gos_DestroyTexture(masterTextureNodes[i].gosTextureHandle);

					masterTextureNodes[i].gosTextureHandle = CACHED_OUT_HANDLE;
					numTexturesFreed++;
				}
			}

			//Count ACTUAL number of textures being used.
			// ALSO can't count on turn being right.  Logistics does not update unless simple Camera is up!!
			if (masterTextureNodes[i].gosTextureHandle != CACHED_OUT_HANDLE)
				currentUsedTextures++;
		}
	}
	
	return numTexturesFreed;
}

//----------------------------------------------------------------------
DWORD MC_TextureManager::textureFromMemory (DWORD *data, gos_TextureFormat key, DWORD hints, DWORD width, DWORD bitDepth)
{
	long i=0;

	//--------------------------------------------------------
	// If we called this, we KNOW the texture is NOT loaded!
	//
	// Find first empty NODE
	for (i=0;i<MC_MAXTEXTURES;i++)
	{
		if (masterTextureNodes[i].gosTextureHandle == 0xffffffff)
		{
			break;
		}
	}

	if (i == MC_MAXTEXTURES)
		STOP(("TOO Many textures in game.  We have exceeded 4096 game handles"));
		
	//--------------------------------------------------------
	// New Method.  Just store memory footprint of texture.
	// DO NOT create GOS handle until we need it.
 	masterTextureNodes[i].gosTextureHandle = CACHED_OUT_HANDLE;
	masterTextureNodes[i].nodeName = NULL;

	masterTextureNodes[i].numUsers = 1;
	masterTextureNodes[i].key = key;
	masterTextureNodes[i].hints = hints;

	//------------------------------------------
	// Find and store the width.
	masterTextureNodes[i].width = width;
	long txmSize = width * width * bitDepth;
	
	if (!lzBuffer1)
	{
		lzBuffer1 = (MemoryPtr)textureCacheHeap->Malloc(MAX_LZ_BUFFER_SIZE);
		gosASSERT(lzBuffer1 != NULL);
		
		lzBuffer2 = (MemoryPtr)textureCacheHeap->Malloc(MAX_LZ_BUFFER_SIZE);
		gosASSERT(lzBuffer2 != NULL);
	}
	
	actualTextureSize += txmSize;
	DWORD txmCompressSize = LZCompress(lzBuffer2,(MemoryPtr)data,txmSize);
	compressedTextureSize += txmCompressSize;
	
 	//-------------------------------------------------------
	// Create a block of cache memory to hold this texture.
	if (!masterTextureNodes[i].textureData )
		masterTextureNodes[i].textureData = (DWORD *)textureCacheHeap->Malloc(txmCompressSize);
	
	//No More RAM.  Do not display this texture anymore.
	if (masterTextureNodes[i].textureData == NULL)
		masterTextureNodes[i].gosTextureHandle = 0;
	else
	{
		memcpy(masterTextureNodes[i].textureData,lzBuffer2,txmCompressSize);
		masterTextureNodes[i].lzCompSize = txmCompressSize;
	}
	
	//------------------	
	return(i);
}

//----------------------------------------------------------------------
DWORD MC_TextureManager::textureInstanceExists (const char *textureFullPathName, gos_TextureFormat key, DWORD hints, DWORD uniqueInstance, DWORD nFlush)
{
	long i=0;

	//--------------------------------------
	// Is this texture already Loaded?
	for (i=0;i<MC_MAXTEXTURES;i++)
	{
		if (masterTextureNodes[i].nodeName)
		{
			if (stricmp(masterTextureNodes[i].nodeName,textureFullPathName) == 0)
			{
				if (uniqueInstance == masterTextureNodes[i].uniqueInstance)
				{
					masterTextureNodes[i].numUsers++;
					return(i);							//Return the texture Node Id Now.
				}
				else
				{
					//------------------------------------------------
					// Copy the texture from old Handle to a new one.
					// Return the NEW handle.
					//
					// There should be no code here!!!
				}
			}
		}
	}
	return 0;
}

//----------------------------------------------------------------------
DWORD MC_TextureManager::loadTexture (const char *textureFullPathName, gos_TextureFormat key, DWORD hints, DWORD uniqueInstance, DWORD nFlush)
{
	long i=0;

	//--------------------------------------
	// Is this texture already Loaded?
	for (i=0;i<MC_MAXTEXTURES;i++)
	{
		if (masterTextureNodes[i].nodeName && (stricmp(masterTextureNodes[i].nodeName,textureFullPathName) == 0))
		{
			if (uniqueInstance == masterTextureNodes[i].uniqueInstance)
			{
				masterTextureNodes[i].numUsers++;
				return(i);							//Return the texture Node Id Now.
			}
			else
			{
				//------------------------------------------------
				// Copy the texture from old Handle to a new one.
				// Return the NEW handle.
				//
				// There should be no code here!!!
			}
		}
	}

	//--------------------------------------------------
	// If we get here, texture has not been loaded yet.
	// Load it now!
	//
	// Find first empty NODE
	for (i=0;i<MC_MAXTEXTURES;i++)
	{
		if (masterTextureNodes[i].gosTextureHandle == 0xffffffff)
		{
			break;
		}
	}

	if (i == MC_MAXTEXTURES)
		STOP(("TOO Many textures in game.  We have exceeded 4096 game handles"));
		
	if (key == gos_Texture_Alpha && Environment.Renderer == 3)
	{
		key = gos_Texture_Keyed;
	}

 	//--------------------------------------------------------
	// New Method.  Just store memory footprint of texture.
	// DO NOT create GOS handle until we need it.
 	masterTextureNodes[i].gosTextureHandle = CACHED_OUT_HANDLE;
	masterTextureNodes[i].nodeName = (char *)textureStringHeap->Malloc(strlen(textureFullPathName) + 1);
	gosASSERT(masterTextureNodes[i].nodeName != NULL);

	strcpy(masterTextureNodes[i].nodeName,textureFullPathName);
	masterTextureNodes[i].numUsers = 1;
	masterTextureNodes[i].key = key;
	masterTextureNodes[i].hints = hints;
	masterTextureNodes[i].uniqueInstance = uniqueInstance;
	masterTextureNodes[i].neverFLUSH = nFlush;

	//----------------------------------------------------------------------------------------------
	// Store 0xf0000000 & fileSize in width so that cache knows to create new texture from memory.
	// This way, we never need to know anything about the texture AND we can store PMGs
	// in memory instead of TGAs which use WAY less RAM!
	File textureFile;
#ifdef _DEBUG
	long textureFileOpenResult = 
#endif
		textureFile.open(textureFullPathName);
	gosASSERT(textureFileOpenResult == NO_ERR);
	
	long txmSize = textureFile.fileSize();
	
	if (!lzBuffer1)
	{
		lzBuffer1 = (MemoryPtr)textureCacheHeap->Malloc(MAX_LZ_BUFFER_SIZE);
		gosASSERT(lzBuffer1 != NULL);
		
		lzBuffer2 = (MemoryPtr)textureCacheHeap->Malloc(MAX_LZ_BUFFER_SIZE);
		gosASSERT(lzBuffer2 != NULL);
	}
	
	//Try reading the RAW data out of the fastFile.
	// If it succeeds, we just saved a complete compress, decompress and two memcpys!!
	//
	long result = textureFile.readRAW(masterTextureNodes[i].textureData,textureCacheHeap);
	if (!result)
	{
		textureFile.read(lzBuffer1,txmSize);

		textureFile.close();

		actualTextureSize += txmSize;
		DWORD txmCompressSize = LZCompress(lzBuffer2,lzBuffer1,txmSize);
		compressedTextureSize += txmCompressSize;

		masterTextureNodes[i].textureData = (DWORD *)textureCacheHeap->Malloc(txmCompressSize);
		if (masterTextureNodes[i].textureData == NULL)
			masterTextureNodes[i].gosTextureHandle = 0;
		else
			memcpy(masterTextureNodes[i].textureData,lzBuffer2,txmCompressSize);

		masterTextureNodes[i].lzCompSize = txmCompressSize;
	}
	else
	{
		masterTextureNodes[i].lzCompSize = result;
	}

	masterTextureNodes[i].width = 0xf0000000 + txmSize;

 	//-------------------
	return(i);
}

//----------------------------------------------------------------------
long MC_TextureManager::saveTexture (DWORD textureIndex, const char *textureFullPathName)
{
	if ((MC_MAXTEXTURES <= textureIndex) || (NULL == masterTextureNodes[textureIndex].textureData))
	{
		return (~NO_ERR);
	}
	File textureFile;
	long textureFileOpenResult = textureFile.create(textureFullPathName);
	if (NO_ERR != textureFileOpenResult)
	{
		textureFile.close();
		return textureFileOpenResult;
	}

	{
		if (masterTextureNodes[textureIndex].width == 0)
		{
			textureFile.close();
			return (~NO_ERR);		//These faces have no texture!!
		}

		{
			//------------------------------------------
			// Badboys are now LZ Compressed in texture cache.
			long origSize = LZDecomp(MC_TextureManager::lzBuffer2,(MemoryPtr)masterTextureNodes[textureIndex].textureData,masterTextureNodes[textureIndex].lzCompSize);
			if (origSize != (masterTextureNodes[textureIndex].width & 0x0fffffff))
				STOP(("Decompressed to different size from original!  Txm:%s  Width:%d  DecompSize:%d",masterTextureNodes[textureIndex].nodeName,(masterTextureNodes[textureIndex].width & 0x0fffffff),origSize));

			if (origSize >= MAX_LZ_BUFFER_SIZE)
				STOP(("Texture TOO large: %s",masterTextureNodes[textureIndex].nodeName));

			textureFile.write(MC_TextureManager::lzBuffer2, origSize);
		}
		textureFile.close();
	}

	return NO_ERR;
}

DWORD MC_TextureManager::copyTexture( DWORD texNodeID )
{
	gosASSERT( texNodeID < MC_MAXTEXTURES );
	if ( masterTextureNodes[texNodeID].gosTextureHandle != -1 )
	{
		masterTextureNodes[texNodeID].numUsers++;
		return texNodeID;
	}
	else
	{
		STOP(( "tried to copy an invalid texture" ));
	}

	return -1;

}
//----------------------------------------------------------------------
// MC_TextureNode
DWORD MC_TextureNode::get_gosTextureHandle (void)	//If texture is not in VidRAM, cache a texture out and cache this one in.
{
	if (gosTextureHandle == 0xffffffff)
	{
		//Somehow this texture is bad.  Probably we are using a handle which got purged between missions.
		// Just send back, NO TEXTURE and we should be able to debug from there because the tri will have no texture!!
		PAUSE(("txmmgr: Bad texture handle!"));
		return 0x0;
	}
	
	if (gosTextureHandle != CACHED_OUT_HANDLE)
	{
		lastUsed = turn;
		return gosTextureHandle;
	}
	else
	{
		if ((mcTextureManager->currentUsedTextures >= MAX_MC2_GOS_TEXTURES) && !mcTextureManager->flushCache())
		{
			PAUSE(("txmmgr: Out of texture handles!"));
			return 0x0;		//No texture!
		}
	   
		if (width == 0)
		{
			PAUSE(("txmmgr: Textur has zero width!"));
			return 0;		//These faces have no texture!!
		}

		if (!textureData)
		{
			PAUSE(("txmmgr: Cache is out of RAM!"));
			return 0x0;		//No Texture.  Cache is out of RAM!!
		}

		if (width > 0xf0000000)
		{
			//------------------------------------------
			// Cache this badboy IN.
			// Badboys are now LZ Compressed in texture cache.
			// Uncompress, then memcpy.
			long origSize = LZDecomp(MC_TextureManager::lzBuffer2,(MemoryPtr)textureData,lzCompSize);
			if (origSize != (width & 0x0fffffff))
				STOP(("Decompressed to different size from original!  Txm:%s  Width:%d  DecompSize:%d",nodeName,(width & 0x0fffffff),origSize));

			if (origSize >= MAX_LZ_BUFFER_SIZE)
				STOP(("Texture TOO large: %s",nodeName));
			
			gosTextureHandle = gos_NewTextureFromMemory(key,nodeName,MC_TextureManager::lzBuffer2,(width & 0x0fffffff),hints);
			mcTextureManager->currentUsedTextures++;
			lastUsed = turn;
			
			return gosTextureHandle;
		}
		else
		{
			gosTextureHandle = gos_NewEmptyTexture(key,nodeName,width,hints);
			mcTextureManager->currentUsedTextures++;
			
			//------------------------------------------
			// Cache this badboy IN.
			TEXTUREPTR pTextureData;
			gos_LockTexture(gosTextureHandle, 0, 0, &pTextureData);
		 
			//-------------------------------------------------------
			// Create a block of cache memory to hold this texture.
			DWORD txmSize = pTextureData.Height * pTextureData.Height * sizeof(DWORD);
			gosASSERT(textureData);
			
			LZDecomp(MC_TextureManager::lzBuffer2,(MemoryPtr)textureData,lzCompSize);
			memcpy(pTextureData.pTexture,MC_TextureManager::lzBuffer2,txmSize);
			 
			//------------------------
			// Unlock the texture
			gos_UnLockTexture(gosTextureHandle);
			 
			lastUsed = turn;
			return gosTextureHandle;
		}
	}
}

//----------------------------------------------------------------------
void MC_TextureNode::destroy (void)
{
	if ((gosTextureHandle != CACHED_OUT_HANDLE) && (gosTextureHandle != 0xffffffff) && (gosTextureHandle != 0x0))
	{
		gos_DestroyTexture(gosTextureHandle);
	}
	
	mcTextureManager->textureStringHeap->Free(nodeName);
	mcTextureManager->textureCacheHeap->Free(textureData);
	init();
}

//----------------------------------------------------------------------
