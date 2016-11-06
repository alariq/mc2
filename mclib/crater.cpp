//---------------------------------------------------------------------
//
// crater.cpp -- Crater/footprint manager for MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------
// Include Files
#ifndef CRATER_H
#include"crater.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef TERRAIN_H
#include"terrain.h"
#endif

#ifndef CIDENT_H
#include"cident.h"
#endif

#ifndef PATHS_H
#include"paths.h"
#endif

#ifndef CEVFX_H
#include"cevfx.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif
//---------------------------------------------------------------------
// Static Globals
CraterManagerPtr craterManager = NULL;
extern bool useFog;
extern bool drawOldWay;
extern bool useNonWeaponEffects;

#define MAX_CRATER_TEXTURES 			2
#define MAX_FOOTPRINTS					64
//---------------------------------------------------------------------
float craterUVTable [136] = 
{
	0.000, 0.000,			//Anubis
	0.125, 0.000,				
	0.250, 0.000,			//Atlas               
	0.375, 0.000,				
	0.500, 0.000,			//BloodAsp               
	0.625, 0.000,				   
	0.750, 0.000,			//Bushwacker             
	0.875, 0.000,				  
	0.000, 0.125,			//Catapult               
	0.125, 0.125,			   
	0.250, 0.125,			//Cougar                 
	0.375, 0.125,			   
	0.500, 0.125,			//Cyclops                
	0.625, 0.125,			   
	0.750, 0.125,			//Enfield                
	0.875, 0.125,			   
	0.000, 0.250,			//Flea (fireant)         
	0.125, 0.250, 			  
	0.250, 0.250, 			//highlander             
	0.375, 0.250, 			 
	0.500, 0.250, 			//hollander              
	0.625, 0.250, 			
	0.750, 0.250, 			//hunchback              
	0.875, 0.250, 			
	0.000, 0.375,			//jaegermech              
	0.125, 0.375, 			
	0.250, 0.375, 			//laohu                    
	0.375, 0.375, 			
	0.500, 0.375, 			//madcat                    
	0.625, 0.375,           
	0.750, 0.375,           //menshen                    
	0.875, 0.375,           
	0.000, 0.500,           //Raven                       
	0.125, 0.500,           
	0.250, 0.500,           //Ryoken                       
	0.375, 0.500,           
	0.500, 0.500,           //Shadowcat                     
	0.625, 0.500,           
	0.750, 0.500,           //Shayu                          
	0.875, 0.500,           
	0.000, 0.625,           //Shootist                        
	0.125, 0.625,           
	0.250, 0.625,           //Starslayer                       
	0.375, 0.625,           
	0.500, 0.625,           //thor                              
	0.625, 0.625,           
	0.750, 0.625,           //Uller                              
	0.875, 0.625,           
	0.000, 0.750,           //Urbanmech                           
	0.125, 0.750,           
	0.250, 0.750,           //Vulture                              
	0.375, 0.750,           
	0.500, 0.750,           //Werewolf (razorback)                  
	0.625, 0.750,           
	0.750, 0.750,           //Wolfhound                              
	0.875, 0.750,           
	0.000, 0.875,           //Zeus                                    
	0.125, 0.875,
	 0.0, 0.0,				//CRATER_1,
	0.50, 0.0,				//CRATER_2,
	 0.0,0.50,				//CRATER_3,
	0.50,0.50				//CRATER_4,
};

//---------------------------------------------------------------------
// class CraterManager
long CraterManager::init (long numCraters, unsigned long craterTypeSize, const char *craterFileName)
{
	init();
	
	//-----------------------------------------------------
	// Allocate Heap to store crater Info
	maxCraters = numCraters;
	craterPosHeapSize = numCraters * (sizeof(CraterData) + allocatedBlockSize);
	
	//-----------------------------------------------------
	// create Heaps
	craterPosHeap = new HeapManager;
	gosASSERT(craterPosHeap != NULL);
	
	long result = craterPosHeap->createHeap(craterPosHeapSize);
	gosASSERT(result == NO_ERR);
		
	result = craterPosHeap->commitHeap(craterPosHeapSize);
	gosASSERT(result == NO_ERR);

	numCraterTextures = MAX_CRATER_TEXTURES;
	//---------------------------------------------------------
	// Setup Crater Texture handles
	craterTextureHandles = (DWORD *)malloc(sizeof(DWORD) * numCraterTextures);
	memset((MemoryPtr)craterTextureHandles,0xff,sizeof(DWORD) * numCraterTextures);

	craterTextureIndices = (DWORD *)malloc(sizeof(DWORD) * numCraterTextures);
	memset((MemoryPtr)craterTextureIndices,0xff,sizeof(DWORD) * numCraterTextures);

	//-----------------------------------------------------
	// Preload all of the craters for the mission.
	// This should just be one texture with all of the craters on it
	// and a generic set of UVs to mark each one.
	for (int i=0;i<numCraterTextures;i++)
	{
		char craterName[1024];
		sprintf(craterName,"defaults\\feet%04d",i);
		
		FullPathFileName craterPath;
		craterPath.init(texturePath,craterName,".tga");

		if (!i)
			craterTextureIndices[i] = mcTextureManager->loadTexture(craterPath, gos_Texture_Keyed, gosHint_DisableMipmap);
		else
			craterTextureIndices[i] = mcTextureManager->loadTexture(craterPath, gos_Texture_Alpha, gosHint_DisableMipmap);
	}

	//-----------------------------------------------------
	// Setup pointer and initial values
	craterList = (CraterDataPtr)craterPosHeap->getHeapPtr();
	memset(craterPosHeap->getHeapPtr(),-1,craterPosHeapSize);

	return(NO_ERR);
}	

//---------------------------------------------------------------------
void CraterManager::destroy (void)
{
	//---------------------------------------------
	// Close file and whack it.
	if (craterFile)
		craterFile->close();
	
	delete craterFile;
	craterFile = NULL;
	
	//----------------------------------------------
	// Whack the Happy Crater Heaps
	delete craterShpHeap;
	craterShpHeap = NULL;
	
	delete craterPosHeap;
	craterPosHeap = NULL;
	
	craterPosHeapSize = craterShpHeapSize = 0;
	craterList = NULL;
	currentCrater = 0;

	free(craterTextureHandles);
	craterTextureHandles = NULL;

	free(craterTextureIndices);
	craterTextureIndices = NULL;
}			

//---------------------------------------------------------------------
long CraterManager::addCrater (long craterType, Stuff::Vector3D &position, float rotation)
{
	if (!useNonWeaponEffects)
		return NO_ERR;

	//----------------------------------------------------------------
	bool drawMe = TRUE;

	if (drawMe)
	{
		craterList[currentCrater].craterShapeId = craterType;
		
		//-----------------------------------------------------------
		// Craters and foot prints are ON the terrain now.
		// Use Elevation Data to move them around.
		// Craters are always 32x32
		// Footprints and always 16x16
		// Rotation of 0 is south.
		float size = 16.0;
		if (craterList[currentCrater].craterShapeId > MAX_FOOTPRINTS)
		{
			size = 32.0;
		}
		
		craterList[currentCrater].position[0].x = -size;
		craterList[currentCrater].position[0].y = -size;
		craterList[currentCrater].position[0].z = 0.0f;

		craterList[currentCrater].position[1].x = size;
		craterList[currentCrater].position[1].y = -size;
		craterList[currentCrater].position[1].z = 0.0f;

		craterList[currentCrater].position[2].x = size;
		craterList[currentCrater].position[2].y = size;
		craterList[currentCrater].position[2].z = 0.0f;
		
		craterList[currentCrater].position[3].x = -size;
		craterList[currentCrater].position[3].y = size;
		craterList[currentCrater].position[3].z = 0.0f;
		
		float cRotation = rotation;

		OppRotate(craterList[currentCrater].position[0],cRotation);
		OppRotate(craterList[currentCrater].position[1],cRotation);
		OppRotate(craterList[currentCrater].position[2],cRotation);
		OppRotate(craterList[currentCrater].position[3],cRotation);
		
		craterList[currentCrater].position[0].Add(craterList[currentCrater].position[0],position);
		craterList[currentCrater].position[1].Add(craterList[currentCrater].position[1],position);
		craterList[currentCrater].position[2].Add(craterList[currentCrater].position[2],position);
		craterList[currentCrater].position[3].Add(craterList[currentCrater].position[3],position);
		
		craterList[currentCrater].position[0].z = land->getTerrainElevation(craterList[currentCrater].position[0]);
		craterList[currentCrater].position[1].z = land->getTerrainElevation(craterList[currentCrater].position[1]);
		craterList[currentCrater].position[2].z = land->getTerrainElevation(craterList[currentCrater].position[2]);
		craterList[currentCrater].position[3].z = land->getTerrainElevation(craterList[currentCrater].position[3]);
		
		currentCrater++;
		if (currentCrater == maxCraters)
		{
			currentCrater = 0;
		}
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------
long CraterManager::update (void)
{
	if (!useNonWeaponEffects)
		return NO_ERR;

	//---------------------------------------------------------------
	// Run through all craters in list and do geometry during update
	CraterDataPtr currentCrater = craterList;
	for (long i=0;i<(long)maxCraters;i++,currentCrater++)
	{
		if (currentCrater->craterShapeId != -1)
		{
			if (currentCrater->craterShapeId > TURKINA_FOOTPRINT)	//We are standard crater.
			{
				craterTextureHandles[0] = mcTextureManager->get_gosTextureHandle(craterTextureIndices[0]);
				mcTextureManager->addTriangle(craterTextureIndices[0],MC2_ISCRATERS | MC2_DRAWALPHA | MC2_ISTERRAIN);
				mcTextureManager->addTriangle(craterTextureIndices[0],MC2_ISCRATERS | MC2_DRAWALPHA | MC2_ISTERRAIN);
			}
			else		//We are a footprint.
			{
				craterTextureHandles[1] = mcTextureManager->get_gosTextureHandle(craterTextureIndices[1]);
				mcTextureManager->addTriangle(craterTextureIndices[1],MC2_ISCRATERS | MC2_DRAWALPHA);
				mcTextureManager->addTriangle(craterTextureIndices[1],MC2_ISCRATERS | MC2_DRAWALPHA);
			}
		}
	}

	return (TRUE);
}

//---------------------------------------------------------------------
void CraterManager::render (void)
{
	if (!useNonWeaponEffects)
		return;

	//-----------------------------------------------------
	// Run through all craters in list and render anything
	// which not clipped or an invalid_crater
	CraterDataPtr currCrater = craterList;

	for (long i=0;i<(long)maxCraters;i++,currCrater++)
	{
		if (currCrater->craterShapeId != -1)
		{
			long handleOffset = 1;
			float uvAdd = 0.125;
			if (currCrater->craterShapeId >= MAX_FOOTPRINTS)
			{
				handleOffset = 0;
				uvAdd = 0.50;
			}
				
			bool onScreen1 = eye->projectZ(currCrater->position[0],currCrater->screenPos[0]);
			bool onScreen2 = eye->projectZ(currCrater->position[1],currCrater->screenPos[1]);
			bool onScreen3 = eye->projectZ(currCrater->position[2],currCrater->screenPos[2]);
			bool onScreen4 = eye->projectZ(currCrater->position[3],currCrater->screenPos[3]);

			//--------------------------------------------------
			// First, if we are using perspective, figure out
			// if object too far from camera.  Far Clip Plane.
			float hazeFactor = 0.0f;
			if (eye->usePerspective)
			{
				Stuff::Point3D Distance;
				Stuff::Point3D objPosition;
				Stuff::Point3D eyePosition(eye->getCameraOrigin());
				objPosition.x = -currCrater->position[0].x;
				objPosition.y = currCrater->position[0].z;
				objPosition.z = currCrater->position[0].y;
		
				Distance.Subtract(objPosition,eyePosition);
				float eyeDistance = Distance.GetApproximateLength();
				if (eyeDistance > Camera::MaxClipDistance)
				{
					onScreen1 = false;
					onScreen2 = false;
					onScreen3 = false;
					onScreen4 = false;
				}
				else if (eyeDistance > Camera::MinHazeDistance)
				{
					hazeFactor = (eyeDistance - Camera::MinHazeDistance) * Camera::DistanceFactor;
				}
				else
				{
					hazeFactor = 0.0f;
				}
			}
					
			//----------------
			// Check clipping
			if (onScreen1 || onScreen2 || onScreen3 || onScreen4)
			{
				DWORD lightRGB = 0xffffffff;
				DWORD specR = 0, specB = 0, specG = 0;
				
				unsigned char lightr = 0xff,lightg = 0xff,lightb = 0xff;
				lightr = eye->ambientRed;
				lightg = eye->ambientGreen;
				lightb = eye->ambientBlue;
						
				lightRGB = lightb + (lightr<<16) + (lightg << 8) + (0xff << 24);
				
				if (Terrain::terrainTextures2)
				{
					if (TerrainQuad::rainLightLevel < 1.0f)
					{
						lightr = (float)lightr * TerrainQuad::rainLightLevel;
						lightb = (float)lightb * TerrainQuad::rainLightLevel;
						lightg = (float)lightg * TerrainQuad::rainLightLevel;
					}
					
					if (TerrainQuad::lighteningLevel > 0x0)
					{
						specR = specG = specB = TerrainQuad::lighteningLevel;
					}
					
					lightRGB = lightb + (lightr<<16) + (lightg << 8) + (0xff << 24);
				}

				DWORD fogRGB = (0xff<<24) + (specR<<16) + (specG<<8) + specB;
				
				if (useFog)
				{
					DWORD fogValue = 0xff;
					float fogStart = eye->fogStart;
					float fogFull = eye->fogFull;

					if (currCrater->position[0].z < fogStart)
					{
						float fogFactor = fogStart - currCrater->position[0].z;
						if (fogFactor < 0.0)
							fogRGB = (0xff<<24) + (specR<<16) + (specG<<8) + specB; 
						else
						{
							fogFactor /= (fogStart - fogFull);
							if (fogFactor <= 1.0)
							{
								fogFactor *= fogFactor;
								fogFactor = 1.0 - fogFactor;
								fogFactor *= 256.0;
							}
							else
							{
								fogFactor = 256.0;
							}

							unsigned char fogResult = fogFactor;
							fogValue = fogFactor;
							fogRGB = (fogResult << 24) + (specR<<16) + (specG<<8) + specB;
						}
					}
					else
					{
						fogRGB = (0xff<<24) + (specR<<16) + (specG<<8) + specB;
					}
					
					if (hazeFactor != 0.0f)
					{
						float fogFactor = 1.0 - hazeFactor;
						DWORD distFog = float2long(fogFactor * 255.0f);
						
						if (distFog < fogValue)
							fogValue = distFog;
							
						fogRGB = (fogValue << 24) + (specR << 16) + (specG << 8) + (specB);
					}
				}
   
				if (drawOldWay)
				{
					//------------------------------------
					// Replace with Polygon Quad Elements
					gos_VERTEX gVertex[4];
	
					gVertex[0].x		= currCrater->screenPos[0].x;
					gVertex[0].y		= currCrater->screenPos[0].y;
					gVertex[0].z		= currCrater->screenPos[0].z;
					gVertex[0].rhw		= currCrater->screenPos[0].w;
					gVertex[0].u		= craterUVTable[(currCrater->craterShapeId*2)];
					gVertex[0].v		= craterUVTable[(currCrater->craterShapeId*2)+1];
					gVertex[0].argb		= lightRGB;
					gVertex[0].frgb		= fogRGB;
	
					gVertex[1].x		= currCrater->screenPos[1].x;
					gVertex[1].y		= currCrater->screenPos[1].y;
					gVertex[1].z		= currCrater->screenPos[1].z;
					gVertex[1].rhw		= currCrater->screenPos[1].w;
					gVertex[1].u		= gVertex[0].u + uvAdd;
					gVertex[1].v		= gVertex[0].v;
					gVertex[1].argb		= lightRGB;
					gVertex[1].frgb		= fogRGB;
	
					gVertex[2].x		= currCrater->screenPos[2].x;
					gVertex[2].y		= currCrater->screenPos[2].y;
					gVertex[2].z		= currCrater->screenPos[2].z;
					gVertex[2].rhw		= currCrater->screenPos[2].w;
					gVertex[2].u		= gVertex[1].u;
					gVertex[2].v		= gVertex[0].v + uvAdd;
					gVertex[2].argb		= lightRGB;
					gVertex[2].frgb		= fogRGB;
	
					gVertex[3].x		= currCrater->screenPos[3].x;
					gVertex[3].y		= currCrater->screenPos[3].y;
					gVertex[3].z		= currCrater->screenPos[3].z;
					gVertex[3].rhw		= currCrater->screenPos[3].w;
					gVertex[3].u		= gVertex[0].u;
					gVertex[3].v		= gVertex[2].v;
					gVertex[3].argb		= lightRGB;
					gVertex[3].frgb		= fogRGB;
					
 					TexturedPolygonQuadElement element;
					element.init(gVertex,craterTextureHandles[handleOffset],false,false);
	
					//-----------------------------------------------------
					// FOG time.  Set Render state to FOG on!
					if (useFog)
					{
						DWORD fogColor = eye->fogColor;
						//gos_SetRenderState( gos_State_Fog, (int)&fogColor);
						gos_SetRenderState( gos_State_Fog, fogColor);
					}
					else
					{
						gos_SetRenderState( gos_State_Fog, 0);
					}
	
					element.draw();
					gos_SetRenderState( gos_State_Fog, 0);		//ALWAYS SHUT FOG OFF WHEN DONE!
				}
				else
				{
					//------------------------------------
					gos_VERTEX gVertex[3];
					gos_VERTEX sVertex[3];
	
					gVertex[0].x		= sVertex[0].x        = currCrater->screenPos[0].x;
					gVertex[0].y		= sVertex[0].y        = currCrater->screenPos[0].y;
					gVertex[0].z		= sVertex[0].z        = currCrater->screenPos[0].z;
					gVertex[0].rhw		= sVertex[0].rhw      = currCrater->screenPos[0].w;
					gVertex[0].u		= sVertex[0].u        = craterUVTable[(currCrater->craterShapeId*2)];
					gVertex[0].v		= sVertex[0].v        = craterUVTable[(currCrater->craterShapeId*2)+1];
					gVertex[0].argb		= sVertex[0].argb     = lightRGB;
					gVertex[0].frgb		= sVertex[0].frgb     = fogRGB;
	
					gVertex[1].x		= currCrater->screenPos[1].x;
					gVertex[1].y		= currCrater->screenPos[1].y;
					gVertex[1].z		= currCrater->screenPos[1].z;
					gVertex[1].rhw		= currCrater->screenPos[1].w;
					gVertex[1].u		= gVertex[0].u + uvAdd;
					gVertex[1].v		= gVertex[0].v;
					gVertex[1].argb		= lightRGB;
					gVertex[1].frgb		= fogRGB;
	
					gVertex[2].x		= sVertex[1].x        = currCrater->screenPos[2].x;
					gVertex[2].y		= sVertex[1].y        = currCrater->screenPos[2].y;
					gVertex[2].z		= sVertex[1].z        = currCrater->screenPos[2].z;
					gVertex[2].rhw		= sVertex[1].rhw      = currCrater->screenPos[2].w;
					gVertex[2].u		= sVertex[1].u        = gVertex[1].u;
					gVertex[2].v		= sVertex[1].v        = gVertex[0].v + uvAdd;
					gVertex[2].argb		= sVertex[1].argb     = lightRGB;
					gVertex[2].frgb		= sVertex[1].frgb     = fogRGB;
	
					sVertex[2].x		= currCrater->screenPos[3].x;
					sVertex[2].y		= currCrater->screenPos[3].y;
					sVertex[2].z		= currCrater->screenPos[3].z;
					sVertex[2].rhw		= currCrater->screenPos[3].w;
					sVertex[2].u		= gVertex[0].u;
					sVertex[2].v		= gVertex[2].v;
					sVertex[2].argb		= lightRGB;
					sVertex[2].frgb		= fogRGB;
 				
					if (currCrater->craterShapeId > TURKINA_FOOTPRINT)	//We are standard crater.
					{
						mcTextureManager->addVertices(craterTextureIndices[handleOffset],gVertex,MC2_ISCRATERS | MC2_DRAWALPHA | MC2_ISTERRAIN);
						mcTextureManager->addVertices(craterTextureIndices[handleOffset],sVertex,MC2_ISCRATERS | MC2_DRAWALPHA | MC2_ISTERRAIN);
					}
					else		//We are a footprint
					{
						mcTextureManager->addVertices(craterTextureIndices[handleOffset],gVertex,MC2_ISCRATERS | MC2_DRAWALPHA);
						mcTextureManager->addVertices(craterTextureIndices[handleOffset],sVertex,MC2_ISCRATERS | MC2_DRAWALPHA);
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------
