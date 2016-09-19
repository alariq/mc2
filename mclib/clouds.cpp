//---------------------------------------------------------------------------
//
// Clouds.cpp -- File contains class definitions for the Clouds
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files
#ifndef CLOUDS_H
#include"clouds.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#ifndef CIDENT_H
#include"cident.h"
#endif

#ifndef PATHS_H
#include"paths.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef CLIP_H
#include"clip.h"
#endif

#ifndef TIMING_H
#include"timing.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions
#define MAX_CLOUDS_ANGLE	45.0f
#define MAX_CLOUDS_SIZE    	4500.0f
#define MAX_UV_REPEAT		4.0f
#define CLOUD_START_U		-2.0f
#define CLOUD_START_V		-2.0f				 
#define CLOUD_ALTITUDE		200.0f
#define SCROLL_U_FACTOR		0.01f;
#define SCROLL_V_FACTOR		0.01f;

//----------------------------------------
// Externs
extern bool useFog;
extern bool hasGuardBand;

//---------------------------------------------------------------------------
// Class Clouds
void Clouds::init (char *textureName, long gSize)
{
	FullPathFileName cloudName;
	cloudName.init(texturePath,textureName,".tga");
	
	//-------------------------------------
	// Load Texture here.
	mcTextureNodeIndex = mcTextureManager->loadTexture(cloudName,gos_Texture_Alpha,/*gosHint_DisableMipmap |*/ gosHint_DontShrink);
	
	//-------------------------------------
	// Create the CloudVertices here.
	gridSize = gSize;
	long gridTotal = gridSize * gridSize;
	cloudVertices = (CloudVertexPtr)systemHeap->Malloc(sizeof(CloudVertex)*gridTotal);
	gosASSERT(cloudVertices != NULL);
	
	memset(cloudVertices,0,sizeof(CloudVertex)*gridTotal);
}
		
//---------------------------------------------------------------------------
void Clouds::update (void)
{
	renderClouds = false;
	
	scrollU += frameLength * SCROLL_U_FACTOR;
	scrollV += frameLength * SCROLL_V_FACTOR;

	if (scrollU > 1.0f)
		scrollU -= 1.0f;
		
	if (scrollV > 1.0f)
		scrollV -= 1.0f;
		
	if (scrollU < -1.0f)
		scrollU += 1.0f;
		
	if (scrollV < -1.0f)
		scrollV += 1.0f;
		
	//---------------------------------------------------------------------
	// If projectionAngle is less then some magic number, draw the clouds.
	// Otherwise, do not do anything!
	if (eye->active && eye->usePerspective && (eye->getProjectionAngle() < MAX_CLOUDS_ANGLE))
	{
		//renderClouds = true;
		
		//-------------------------------------------------------
		// Create the cloud grid based on camera CENTER position.
		Stuff::Vector3D centerPosition(eye->getPosition());
		
		//-------------------------------------------------------
		// Create a topLeft vertex
		float tLeftX = centerPosition.x + MAX_CLOUDS_SIZE;
		float tLeftY = centerPosition.y + MAX_CLOUDS_SIZE;
		
		//-------------------------------------------------------
		// Create the grid.
		long cloudInc = float2long(MAX_CLOUDS_SIZE * 2.0f / gridSize);
		float uvInc = MAX_UV_REPEAT / float(gridSize);
		
		for (long y=0;y<gridSize;y++)
		{
			for (long x=0;x<gridSize;x++)
			{
				cloudVertices[x + (y*gridSize)].vx = tLeftX - (cloudInc * x);
				cloudVertices[x + (y*gridSize)].vy = tLeftY - (cloudInc * y);
				
				cloudVertices[x + (y*gridSize)].pu = CLOUD_START_U + (uvInc * x);
				cloudVertices[x + (y*gridSize)].pv = CLOUD_START_V + (uvInc * y);
 			}
		}
		
		//-------------------------------------------------------
		// Transform Grid
		long gridTotal = gridSize * gridSize;
		for (long i=0;i<gridTotal;i++)
		{
			//----------------------------------------------------------------------------------------
			// Figure out if we are in front of camera or not.  Should be faster then actual project!
			// Should weed out VAST overwhelming majority of vertices!
			bool onScreen = true;
		
			//-----------------------------------------------------------------
			// Find angle between lookVector of Camera and vector from camPos
			// to Target.  If angle is less then halfFOV, object is visible.
			
			//-------------------------------------------------------------------
			// Then figure out if FarClipped.  Should weed out a boatload more!
			float hazeFactor = 0.0f;
			Stuff::Point3D Distance;
			Stuff::Point3D vPosition;
			Stuff::Point3D eyePosition(eye->getPosition());
			vPosition.x = cloudVertices[i].vx;;
			vPosition.y = cloudVertices[i].vy;
			vPosition.z = centerPosition.z;
				
			Distance.Subtract(eyePosition,vPosition);
			float eyeDistance = Distance.GetApproximateLength();
			if (eyeDistance > Camera::MaxClipDistance)
			{
				hazeFactor = 1.0f;
				//onScreen = false;
			}
			else if (eyeDistance > Camera::MinHazeDistance)
			{
				hazeFactor = (eyeDistance - Camera::MinHazeDistance) * Camera::DistanceFactor;
			}
			else
			{
				hazeFactor = 0.0f;
			}
					
			//------------------------------------------------------------
			// Calculate the HazeDWORD here
			if (hazeFactor != 0.0f)
			{
				float fogFactor = 1.0 - hazeFactor;
				DWORD distFog = float2long(fogFactor * 255.0f);
				
				cloudVertices[i].fogRGB = (distFog<<24) + (0xffffff);
			}
			else
			{
				cloudVertices[i].fogRGB = 0xffffffff;
			}
			
			if (onScreen)
			{
				Stuff::Vector3D Distance;
				Stuff::Point3D objPosition;
				Stuff::Point3D eyePosition(eye->getCameraOrigin());
				objPosition.x = -cloudVertices[i].vx;
				objPosition.y = CLOUD_ALTITUDE;
				objPosition.z = cloudVertices[i].vy;
			
				Distance.Subtract(objPosition,eyePosition);
				Distance.Normalize(Distance);
					
				float cosine = Distance * eye->getLookVector();
				if (cosine > eye->cosHalfFOV)
					onScreen = true;
				else
					onScreen = false;
			}
			else
			{
				hazeFactor = 1.0f;
			}
			
			Stuff::Vector3D vertex3D(cloudVertices[i].vx,cloudVertices[i].vy,(CLOUD_ALTITUDE+eye->getCameraOrigin().y));
			Stuff::Vector4D screenPos;
			bool inView = eye->projectZ(vertex3D,screenPos);
			
			cloudVertices[i].px = screenPos.x;
			cloudVertices[i].py = screenPos.y;
			cloudVertices[i].pz = screenPos.z;
			cloudVertices[i].pw = screenPos.w;
			
			//------------------------------------------------------------
			// Fix clip.  Vertices can all be off screen and triangle
			// still needs to be drawn!
			cloudVertices[i].clipInfo = onScreen && inView;
			
			//------------------------------------------------------------
			// Still need to scrollUVs here!
			cloudVertices[i].pu += scrollU;
			cloudVertices[i].pv += scrollV;
		}
		
		for (long y=0;y<(gridSize-1);y++)
		{
			for (long x=0;x<(gridSize-1);x++)
			{
				CloudVertexPtr cloudVertex0 = &(cloudVertices[x     + (y    *gridSize)]);
				CloudVertexPtr cloudVertex1 = &(cloudVertices[(x+1) + (y    *gridSize)]); 
				CloudVertexPtr cloudVertex2 = &(cloudVertices[(x+1) + ((y+1)*gridSize)]); 
				CloudVertexPtr cloudVertex3 = &(cloudVertices[x     + ((y+1)*gridSize)]); 
				
				bool clipCheck = (cloudVertex0->clipInfo || cloudVertex1->clipInfo || cloudVertex2->clipInfo);
				if (clipCheck && ((cloudVertex0->pz < 1.0f) && (cloudVertex0->pz > 0.0f) &&
									(cloudVertex1->pz < 1.0f) && (cloudVertex1->pz > 0.0f) && 
									(cloudVertex2->pz < 1.0f) && (cloudVertex2->pz > 0.0f)))
				{
					mcTextureManager->addTriangle(mcTextureNodeIndex,MC2_DRAWALPHA);
				}
				
				clipCheck = (cloudVertex0->clipInfo || cloudVertex2->clipInfo || cloudVertex3->clipInfo);
				if (clipCheck && ((cloudVertex0->pz < 1.0f) && (cloudVertex0->pz > 0.0f) &&
									(cloudVertex2->pz < 1.0f) && (cloudVertex2->pz > 0.0f) && 
									(cloudVertex3->pz < 1.0f) && (cloudVertex3->pz > 0.0f)))
				{
					mcTextureManager->addTriangle(mcTextureNodeIndex,MC2_DRAWALPHA);
				}
			}
		}			 
	}
}

#define CLOUD_Z_VALUE	0.9900f
//---------------------------------------------------------------------------
void Clouds::render (void)
{
	if (!renderClouds)
		return;
		
	for (long y=0;y<(gridSize-1);y++)
	{
		for (long x=0;x<(gridSize-1);x++)
		{
			CloudVertexPtr cloudVertex0 = &(cloudVertices[x     + (y    *gridSize)]);
			CloudVertexPtr cloudVertex1 = &(cloudVertices[(x+1) + (y    *gridSize)]); 
			CloudVertexPtr cloudVertex2 = &(cloudVertices[(x+1) + ((y+1)*gridSize)]); 
			CloudVertexPtr cloudVertex3 = &(cloudVertices[x     + ((y+1)*gridSize)]); 
			
			gos_VERTEX gVertex[3];
			bool clipCheck = (cloudVertex0->clipInfo || cloudVertex1->clipInfo || cloudVertex2->clipInfo);
			if (clipCheck && ((cloudVertex0->pz < 1.0f) && (cloudVertex0->pz > 0.0f) &&
								(cloudVertex1->pz < 1.0f) && (cloudVertex1->pz > 0.0f) && 
								(cloudVertex2->pz < 1.0f) && (cloudVertex2->pz > 0.0f)))
			{
				//--------------------------
				// Top Triangle
				gVertex[0].x		= cloudVertex0->px;
				gVertex[0].y		= cloudVertex0->py;
				gVertex[0].z		= CLOUD_Z_VALUE;
				gVertex[0].rhw		= cloudVertex0->pw;
				gVertex[0].u		= cloudVertex0->pu;
				gVertex[0].v		= cloudVertex0->pv;
				gVertex[0].argb		= cloudVertex0->fogRGB; 
				gVertex[0].frgb		= 0xff000000;
			
				gVertex[1].x		= cloudVertex1->px;
				gVertex[1].y		= cloudVertex1->py;
				gVertex[1].z		= CLOUD_Z_VALUE; 
				gVertex[1].rhw		= cloudVertex1->pw;
				gVertex[1].u		= cloudVertex1->pu;
				gVertex[1].v		= cloudVertex1->pv;
				gVertex[1].argb		= cloudVertex1->fogRGB; 
				gVertex[1].frgb		= 0xff000000;
				
				gVertex[2].x		= cloudVertex2->px;
				gVertex[2].y		= cloudVertex2->py;
				gVertex[2].z		= CLOUD_Z_VALUE; 
				gVertex[2].rhw		= cloudVertex2->pw;
				gVertex[2].u		= cloudVertex2->pu;
				gVertex[2].v		= cloudVertex2->pv;
				gVertex[2].argb		= cloudVertex2->fogRGB; 
				gVertex[2].frgb		= 0xff000000;
				
				mcTextureManager->addVertices(mcTextureNodeIndex,gVertex,MC2_DRAWALPHA);
			}

			clipCheck = (cloudVertex0->clipInfo || cloudVertex2->clipInfo || cloudVertex3->clipInfo);
			if (clipCheck && ((cloudVertex0->pz < 1.0f) && (cloudVertex0->pz > 0.0f) &&
								(cloudVertex2->pz < 1.0f) && (cloudVertex2->pz > 0.0f) && 
								(cloudVertex3->pz < 1.0f) && (cloudVertex3->pz > 0.0f)))
			{
				//--------------------------
				//Bottom Triangle
				//
				// gVertex[0] same as above gVertex[0].
				// gVertex[1] is same as above gVertex[2].
				// gVertex[2] is calced from vertex[3].
				gVertex[0].x		= cloudVertex0->px;
				gVertex[0].y		= cloudVertex0->py;
				gVertex[0].z		= CLOUD_Z_VALUE; 
				gVertex[0].rhw		= cloudVertex0->pw;
				gVertex[0].u		= cloudVertex0->pu;
				gVertex[0].v		= cloudVertex0->pv;
				gVertex[0].argb		= cloudVertex0->fogRGB; 
				gVertex[0].frgb		= 0xff000000;
			
				gVertex[1].x		= cloudVertex2->px;
				gVertex[1].y		= cloudVertex2->py;
				gVertex[1].z		= CLOUD_Z_VALUE; 
				gVertex[1].rhw		= cloudVertex2->pw;
				gVertex[1].u		= cloudVertex2->pu;
				gVertex[1].v		= cloudVertex2->pv;
				gVertex[1].argb		= cloudVertex2->fogRGB; 
				gVertex[1].frgb		= 0xff000000;
				
				gVertex[2].x		= cloudVertex3->px;     
				gVertex[2].y		= cloudVertex3->py;     
				gVertex[2].z		= CLOUD_Z_VALUE; 
				gVertex[2].rhw		= cloudVertex3->pw;     
				gVertex[2].u		= cloudVertex3->pu;     
				gVertex[2].v		= cloudVertex3->pv;     
				gVertex[2].argb		= cloudVertex3->fogRGB;  
				gVertex[2].frgb		= 0xff000000;
				
				mcTextureManager->addVertices(mcTextureNodeIndex,gVertex,MC2_DRAWALPHA);
			}
		}
	}
}

//---------------------------------------------------------------------------
void Clouds::destroy (void)
{
	systemHeap->Free(cloudVertices);
	cloudVertices = NULL;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// Edit Log
//
//---------------------------------------------------------------------------


