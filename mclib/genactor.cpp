//---------------------------------------------------------------------------
//
//	genactor.cpp - This file contains the code for the generic appearance classes
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GENACTOR_H
#include"genactor.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
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

#ifndef OBJSTATUS_H
#include"objstatus.h"
#endif

#ifndef UTILITIES_H
#include"utilities.h"
#endif

#ifndef INIFILE_H
#include"inifile.h"
#endif

#ifndef ERR_H
#include"err.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#ifndef TIMING_H
#include"timing.h"
#endif

#ifndef CELINE_H
#include"celine.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

//******************************************************************************************
extern float	worldUnitsPerMeter;
extern bool 	drawTerrainGrid;
extern bool		useFog;

extern long 	mechRGBLookup[];
extern long 	mechRGBLookup2[];

extern long		ObjectTextureSize;

extern bool reloadBounds;
//-----------------------------------------------------------------------------
// class GenericAppearanceType
void GenericAppearanceType::init (const char * fileName)
{
	AppearanceType::init(fileName);

	//----------------------------------------------
	// Base shape.  In stand Pose by default.
	genShape = new TG_TypeMultiShape;
	gosASSERT(genShape != NULL);

	FullPathFileName iniName;
	iniName.init(tglPath,fileName,".ini");

	FitIniFile iniFile;
	long result = iniFile.open(iniName);
	if (result != NO_ERR)
		Fatal(result,"Could not find building appearance INI file");

	result = iniFile.seekBlock("TGLData");
	if (result != NO_ERR)
		Fatal(result,"Could not find block in building appearance INI file");

	char aseFileName[512];
	result = iniFile.readIdString("FileName",aseFileName,511);
	if (result != NO_ERR)
		Fatal(result,"Could not find ASE FileName in building appearance INI file");

	FullPathFileName genName;
	genName.init(tglPath,aseFileName,".ase");

	genShape->LoadTGMultiShapeFromASE(genName);

	result = iniFile.readIdString("TextureName",textureName,49);
	if (result != NO_ERR)
		textureName[0] = 0;

	result = iniFile.readIdULong("RGB",dotRGB);
	if (result != NO_ERR)
		dotRGB = 0x00ffffff;

	result = iniFile.seekBlock("TGLDamage");
	if (result == NO_ERR)
	{
		result = iniFile.readIdString("FileName",aseFileName,511);
		if (result != NO_ERR)
			Fatal(result,"Could not find ASE FileName in building appearance INI file");
	
		FullPathFileName dmgName;
		dmgName.init(tglPath,aseFileName,".ase");
	
		genDmgShape = new TG_TypeMultiShape;
		gosASSERT(genDmgShape != NULL);
		genDmgShape->LoadTGMultiShapeFromASE(dmgName);

		if (!genDmgShape->GetNumShapes())
		{
			delete genDmgShape;
			genDmgShape = NULL;
		}
	}
	else
	{
		genDmgShape = NULL;
	}

		
	//--------------------------------------------------------------------
	// Load Animation Information.
	// We can load up to 10 Animation States.
	for (long i=0;i<MAX_BD_ANIMATIONS;i++)
	{
		char blockId[512];
		sprintf(blockId,"Animation:%d",i);
		
		result = iniFile.seekBlock(blockId);
		if (result == NO_ERR)
		{
			char animName[512];
			result = iniFile.readIdString("AnimationName",animName,511);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("LoopAnimation",genAnimLoop[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("Reverse",genReverse[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("Random",genRandom[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdLong("StartFrame",genStartF[i]);
			if (result != NO_ERR)
				genStartF[i] = 0;
				
 			//-------------------------------
			// We have an animation to load.
			FullPathFileName animPath;
			animPath.init(tglPath,animName,".ase");

			FullPathFileName otherPath;
			otherPath.init(tglPath,animName,".agl");

			if (fileExists(animPath) || fileExists(otherPath))
			{
				genAnimData[i] = new TG_AnimateShape;
				gosASSERT(genAnimData[i] != NULL);
	
				//--------------------------------------------------------
				// If this animation does not exist, it is not a problem!
				// Building will simply freeze until animation is "over"
				genAnimData[i]->LoadTGMultiShapeAnimationFromASE(animPath,genShape);
			}
			else
				genAnimData[i] = NULL;
		}
		else
		{
			genAnimData[i] = NULL;
		}
	}
	
	//--------------------------------------------------------------------
	// We can also load the node to pitch and yaw for spotlights/turrets.
	result = iniFile.seekBlock("AnimationNode");
	if (result == NO_ERR)
	{
		result = iniFile.readIdString("AnimationNodeId",rotationalNodeId,24);
		gosASSERT(result == NO_ERR);
	}
	else
	{
		strcpy(rotationalNodeId,"NONE");
	}

}

//----------------------------------------------------------------------------
void GenericAppearanceType::destroy (void)
{
	AppearanceType::destroy();

	if (genShape)
	{
		delete genShape;
		genShape = NULL;
	}

	if (genDmgShape)
	{
		delete genDmgShape;
		genDmgShape = NULL;
	}
}

//-----------------------------------------------------------------------------
void GenericAppearanceType::setAnimation (TG_MultiShapePtr shape, DWORD animationNum)
{
	gosASSERT(shape != NULL);
	gosASSERT(animationNum != 0xffffffff);
	gosASSERT(animationNum < MAX_GEN_ANIMATIONS);

	if (genAnimData[animationNum])
		genAnimData[animationNum]->SetAnimationState(shape);
	else
		shape->ClearAnimation();
}

//-----------------------------------------------------------------------------
// class GenericAppearance
void GenericAppearance::init (AppearanceTypePtr tree, GameObjectPtr obj)
{
	Appearance::init(tree,obj);
	appearType = (GenericAppearanceType *)tree;

	shapeMin.x = shapeMin.y = -25;
	shapeMax.x = shapeMax.y = 50;

	genAnimationState =-1;
	currentFrame = 0.0f;
	genFrameRate = 0.0f;
	isReversed = false;
	isLooping = false;
	setFirstFrame = false;
	canTransition = true;
	
	paintScheme = -1;
	objectNameId = 30469;
	hazeFactor = 0.0f;
	skyNumber = 0;

	screenPos.x = screenPos.y = screenPos.z = screenPos.w = -999.0f;
	position.Zero();
	rotation = 0.0f;
	pitch = 0.0f;
	selected = 0;
	teamId = -1;
	homeTeamRelationship = 0;
	actualRotation = rotation;

	OBBRadius = -1.0f;
	
	if (appearType)
	{
		genShape = appearType->genShape->CreateFrom();

		//-------------------------------------------------
		// Load the texture and store its handle.
		for (long i=0;i<genShape->GetNumTextures();i++)
		{
			char txmName[1024];
			if ((i == 0) && (appearType->textureName[0]))
				strcpy(txmName,appearType->textureName);
			else
				genShape->GetTextureName(i,txmName,256);

			char texturePath[1024];
			sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
	
			FullPathFileName textureName;
			textureName.init(texturePath,txmName,"");
	
			if (fileExists(textureName))
			{
				if (strnicmp(txmName,"a_",2) == 0)
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
					genShape->SetTextureHandle(i,gosTextureHandle);
					genShape->SetTextureAlpha(i,true);
				}
				else
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
					genShape->SetTextureHandle(i,gosTextureHandle);
					genShape->SetTextureAlpha(i,false);
				}
			}
			else
			{
				//PAUSE(("Warning: %s texture name not found",textureName));
				genShape->SetTextureHandle(i,0xffffffff);
			}
		}

		Stuff::Vector3D boxCoords[8];
		Stuff::Vector3D nodeCenter = genShape->GetRootNodeCenter();

		boxCoords[0].x = position.x + genShape->GetMinBox().x + nodeCenter.x;
		boxCoords[0].y = position.y + genShape->GetMinBox().z + nodeCenter.z;
		boxCoords[0].z = position.z + genShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[1].x = position.x + genShape->GetMinBox().x + nodeCenter.x;
		boxCoords[1].y = position.y + genShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[1].z = position.z + genShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[2].x = position.x + genShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[2].y = position.y + genShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[2].z = position.z + genShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[3].x = position.x + genShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[3].y = position.y + genShape->GetMinBox().z + nodeCenter.z;
		boxCoords[3].z = position.z + genShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[4].x = position.x + genShape->GetMinBox().x + nodeCenter.x;
		boxCoords[4].y = position.y + genShape->GetMinBox().z + nodeCenter.z;
		boxCoords[4].z = position.z + genShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[5].x = position.x + genShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[5].y = position.y + genShape->GetMinBox().z + nodeCenter.z;
		boxCoords[5].z = position.z + genShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[6].x = position.x + genShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[6].y = position.y + genShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[6].z = position.z + genShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[7].x = position.x + genShape->GetMinBox().x + nodeCenter.x;
		boxCoords[7].y = position.y + genShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[7].z = position.z + genShape->GetMinBox().y + nodeCenter.y;
		
		float testRadius = 0.0;
		
		for (int i=0;i<8;i++)
		{
			testRadius = boxCoords[i].GetLength();
			if (OBBRadius < testRadius)
				OBBRadius = testRadius;
		}

		appearType->boundsUpperLeftX = (-OBBRadius * 2.0);
		appearType->boundsUpperLeftY = (-OBBRadius * 2.0);
		   					 
		appearType->boundsLowerRightX = (OBBRadius * 2.0);
		appearType->boundsLowerRightY = (OBBRadius);
		
 		if (!appearType->getDesignerTypeBounds())
		{
			appearType->typeUpperLeft = genShape->GetMinBox();
			appearType->typeLowerRight = genShape->GetMaxBox();
		}
	}
}

//-----------------------------------------------------------------------------
void GenericAppearance::setObjStatus (long oStatus)
{
	if (status != oStatus)
	{
		if ((oStatus == OBJECT_STATUS_DESTROYED) || (oStatus == OBJECT_STATUS_DISABLED))
		{
			if (appearType->genDmgShape)
			{
				genShape->ClearAnimation();
				delete genShape;
				genShape = NULL;
				
				genShape = appearType->genDmgShape->CreateFrom();
				
				//-------------------------------------------------
				// Load the texture and store its handle.
				for (long i=0;i<genShape->GetNumTextures();i++)
				{
					char txmName[1024];
					genShape->GetTextureName(i,txmName,256);
					
					char texturePath[1024];
					sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
			
					FullPathFileName textureName;
					textureName.init(texturePath,txmName,"");
					
					if (fileExists(textureName))
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						genShape->SetTextureHandle(i,gosTextureHandle);
					}
					else
					{
						//PAUSE(("Warning: %s texture name not found",textureName));
						genShape->SetTextureHandle(i,0xffffffff);
					}
				}
			}
		}
	}
	
	status = oStatus;
}

//-----------------------------------------------------------------------------
void GenericAppearance::setGesture (unsigned long gestureId)
{
	//------------------------------------------------------------
	// Check if state is possible.
	if (gestureId >= MAX_GEN_ANIMATIONS)
		return;

	//------------------------------------------------------------
	// Check if object destroyed.  If so, no animation!
	if ((status == OBJECT_STATUS_DESTROYED) || (status == OBJECT_STATUS_DISABLED))
		return;
		
	//----------------------------------------------------------------------
	// If state is OK, set animation data, set first frame, set loop and 
	// reverse flag, and start it going until you hear otherwise.
	appearType->setAnimation(genShape,gestureId);
	genAnimationState = gestureId;
	currentFrame = 0.0f;
	if (appearType->genStartF[gestureId])
		currentFrame = appearType->genStartF[gestureId];
		
	isReversed = false;
	
	if (appearType->isReversed(genAnimationState))
	{
		currentFrame = appearType->getNumFrames(genAnimationState)-1;
		isReversed = true;
	}
	
	if (appearType->isRandom(genAnimationState))
	{
		currentFrame = RandomNumber(appearType->getNumFrames(genAnimationState)-1);
	}
	
	isLooping = appearType->isLooped(genAnimationState);
	
	genFrameRate = appearType->getFrameRate(genAnimationState);
	
	setFirstFrame = true;
	canTransition = false;
}

//-----------------------------------------------------------------------------
void GenericAppearance::setMoverParameters (float turretRot, float lArmRot, float rArmRot, bool isAirborne)
{
	pitch = turretRot;
}

//-----------------------------------------------------------------------------
void GenericAppearance::setObjectParameters (const Stuff::Vector3D &pos, float Rot, long sel, long team, long homeRelations)
{
	rotation = Rot;

	position = pos;

	selected = sel;

	actualRotation = Rot;

	teamId = team;
	homeTeamRelationship = homeRelations;
}

//-----------------------------------------------------------------------------
void GenericAppearance::changeSkyToSkyNum (char *txmName, char *newName)
{
	if (strnicmp(txmName,"sky",3) != 0)
	{
		strcpy(newName,txmName);
	}
	else
	{
		strcpy(newName,"Sky");
		sprintf(newName,"Sky%02d%s",skyNumber,&txmName[3]);
	}
	
	return;
}

//-----------------------------------------------------------------------------
void GenericAppearance::setSkyNumber (long skyNum)
{
	//-------------------------------------------------
	// Load the texture and store its handle.
	for (long i=0;i<genShape->GetNumTextures();i++)
	{
		char txmName[1024];
		char newName[1024];
		if ((i == 0) && (appearType->textureName[0]))
			strcpy(txmName,appearType->textureName);
		else
			genShape->GetTextureName(i,txmName,256);

		char texturePath[1024];
		sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);

		//Make txmName into a SKY%02d texture and load it!!
		skyNumber = skyNum;
		
		changeSkyToSkyNum(txmName,newName);
		
		FullPathFileName textureName;
		textureName.init(texturePath,newName,"");

		if (fileExists(textureName))
		{
			if (strnicmp(newName,"a_",2) == 0)
			{
				DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
				genShape->SetTextureHandle(i,gosTextureHandle);
				genShape->SetTextureAlpha(i,true);
			}
			else
			{
				DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
				genShape->SetTextureHandle(i,gosTextureHandle);
				genShape->SetTextureAlpha(i,false);
			}
		}
		else
		{
			//PAUSE(("Warning: %s texture name not found",textureName));
			genShape->SetTextureHandle(i,0xffffffff);
		}
	}
}

//-----------------------------------------------------------------------------
bool GenericAppearance::isMouseOver (float px, float py)
{
	if (inView)
	{
		if ((px <= lowerRight.x) && (py <= lowerRight.y) &&
			(px >= upperLeft.x) &&
			(py >= upperLeft.y))
		{
			return inView;
		}
		else
		{
			return FALSE;
		}
	}
	
	return(inView);
}	

//-----------------------------------------------------------------------------
bool GenericAppearance::recalcBounds (void)
{
	Stuff::Vector4D tempPos;
	inView = false;

	if (eye)
	{
		//ALWAYS need to do this or select is YAYA
		eye->projectZ(position,screenPos);
		
 		//--------------------------------------------------
		// First, if we are using perspective, figure out
		// if object too far from camera.  Far Clip Plane.
		if (eye->usePerspective)
		{
			Stuff::Point3D Distance;
			Stuff::Point3D eyePosition(eye->getPosition());
			Stuff::Point3D objPosition(position);
	
			Distance.Subtract(objPosition,eyePosition);
			float eyeDistance = Distance.GetApproximateLength();
			if (eyeDistance > Camera::MaxClipDistance)
			{
				hazeFactor = 1.0f;
				inView = false;
			}
			else if (eyeDistance > Camera::MinHazeDistance)
			{
				Camera::HazeFactor = (eyeDistance - Camera::MinHazeDistance) * Camera::DistanceFactor;
				inView = true;
			}
			else
			{
				Camera::HazeFactor = 0.0f;
				inView = true;
			}
			
			//-----------------------------------------------------------------
			// If inside farClip plane, check if behind camera.
			// Find angle between lookVector of Camera and vector from camPos
			// to Target.  If angle is less then halfFOV, object is visible.
			if (inView)
			{
				Stuff::Vector3D Distance;
				Stuff::Point3D objPosition;
				Stuff::Point3D eyePosition(eye->getCameraOrigin());
				objPosition.x = -position.x;
				objPosition.y = position.z;
				objPosition.z = position.y;
		
				Distance.Subtract(objPosition,eyePosition);
				Distance.Normalize(Distance);
				
				float cosine = Distance * eye->getLookVector();
 				if (cosine > eye->cosHalfFOV)
					inView = true;
				else
					inView = false;
			}
		}
		else
		{
			Camera::HazeFactor = 0.0f;
			inView = true;
		}
		
		if (inView)
		{
			if (reloadBounds)
				appearType->reinit();

			appearType->boundsLowerRightY = (OBBRadius * eye->getTiltFactor() * 2.0f);
			
			if (screenPos.z > 0.999999f)
				screenPos.z = 0.999999f;
			//-------------------------------------------------------------------------
			// do a rough check if on screen.  If no where near, do NOT do the below.
			// Mighty mighty slow!!!!
			// Use the original check done before all this 3D madness.  Dig out sourceSafe tomorrow!
			tempPos = screenPos;
			upperLeft.x = tempPos.x;
			upperLeft.y = tempPos.y;
			
			lowerRight.x = tempPos.x;
			lowerRight.y = tempPos.y;
			
			upperLeft.x += (appearType->boundsUpperLeftX * eye->getScaleFactor());
			upperLeft.y += (appearType->boundsUpperLeftY * eye->getScaleFactor());
	
			lowerRight.x += (appearType->boundsLowerRightX * eye->getScaleFactor());
			lowerRight.y += (appearType->boundsLowerRightY * eye->getScaleFactor());

			if ((lowerRight.x >= 0) && (lowerRight.y >= 0) &&
				(upperLeft.x <= eye->getScreenResX()) &&
				(upperLeft.y <= eye->getScreenResY()))
			{
				//We are on screen.  Figure out selection box.
				Stuff::Vector3D boxCoords[8];
				Stuff::Vector4D bcsp[8];
	
				boxCoords[0].x = position.x + appearType->typeUpperLeft.x;
				boxCoords[0].y = position.y + appearType->typeUpperLeft.y;
				boxCoords[0].z = position.z + appearType->typeUpperLeft.z;
	
				boxCoords[1].x = position.x + appearType->typeUpperLeft.x;
				boxCoords[1].y = position.y + appearType->typeLowerRight.y;
				boxCoords[1].z = position.z + appearType->typeUpperLeft.z;
	
				boxCoords[2].x = position.x + appearType->typeLowerRight.x;
				boxCoords[2].y = position.y + appearType->typeUpperLeft.y;
				boxCoords[2].z = position.z + appearType->typeUpperLeft.z;
	
				boxCoords[3].x = position.x + appearType->typeLowerRight.x;
				boxCoords[3].y = position.y + appearType->typeLowerRight.y;
				boxCoords[3].z = position.z + appearType->typeUpperLeft.z;
	
				boxCoords[4].x = position.x + appearType->typeLowerRight.x;
				boxCoords[4].y = position.y + appearType->typeLowerRight.y;
				boxCoords[4].z = position.z + appearType->typeLowerRight.z;
	
				boxCoords[5].x = position.x + appearType->typeLowerRight.x;
				boxCoords[5].y = position.y + appearType->typeUpperLeft.y;
				boxCoords[5].z = position.z + appearType->typeLowerRight.z;
	
				boxCoords[6].x = position.x + appearType->typeUpperLeft.x;
				boxCoords[6].y = position.y + appearType->typeLowerRight.y;
				boxCoords[6].z = position.z + appearType->typeLowerRight.z;
	
				boxCoords[7].x = position.x + appearType->typeUpperLeft.x;
				boxCoords[7].y = position.y + appearType->typeUpperLeft.y;
				boxCoords[7].z = position.z + appearType->typeLowerRight.z;
	
				float maxX = 0.0f, maxY = 0.0f;
				float minX = 0.0f, minY = 0.0f;

				for (long i=0;i<8;i++)
				{
					eye->projectZ(boxCoords[i],bcsp[i]);
					if (!i)
					{
						maxX = minX = bcsp[i].x;
						maxY = minY = bcsp[i].y;
					}
					
					if (i)
					{
						if (bcsp[i].x > maxX)
							maxX = bcsp[i].x;
						
						if (bcsp[i].x < minX)
							minX = bcsp[i].x;
							
						if (bcsp[i].y > maxY)
							maxY = bcsp[i].y;
						
						if (bcsp[i].y < minY)
							minY = bcsp[i].y;
					}
				}
		
				upperLeft.x = minX;
				upperLeft.y = minY;
				lowerRight.x = maxX;
				lowerRight.y = maxY;
				
				if ((lowerRight.x >= 0) && (lowerRight.y >= 0) &&
					(upperLeft.x <= eye->getScreenResX()) &&
					(upperLeft.y <= eye->getScreenResY()))
				{
					inView = true;
				}
				else
				{
					inView = false;		//Did alot of extra work checking this, but WHY draw and insult to injury?
				}
			}
			else
			{
				inView = false;
			}
		}
	}

	
	return(inView);
}

//-----------------------------------------------------------------------------
long GenericAppearance::render (long depthFixup)
{
	if (inView)
	{
		long color = SD_BLUE;
		unsigned long highLight = 0x007f7f7f;
		if ((teamId > -1) && (teamId < 8)) {
			static unsigned long highLightTable[3] = {0x00007f00, 0x0000007f, 0x007f0000};
			static long colorTable[3] = {SB_GREEN | 0xff000000, SB_BLUE | 0xff000000, SB_RED | 0xff000000};
			color = colorTable[homeTeamRelationship];
			highLight = highLightTable[homeTeamRelationship];
		}
		if (selected & DRAW_COLORED)
		{
			genShape->SetARGBHighLight(highLight);
		}
		else
		{
			genShape->SetARGBHighLight(highlightColor);
		}

		//---------------------------------------------
		// Call Multi-shape render stuff here.
		if (visible)
		{
			if (depthFixup)
			{
				genShape->Render(false,0.99999f);	//Sky or something like it.  Push to back!
			}
			else
				genShape->Render();
		}
		else
		{
			gos_VERTEX planetPoint;

			planetPoint.x = screenPos.x;
			planetPoint.y = screenPos.y;
			planetPoint.z = screenPos.z;
			planetPoint.rhw = screenPos.w;
			planetPoint.argb = appearType->dotRGB;
			planetPoint.frgb = 0xff000000;
			planetPoint.u = planetPoint.v = 0.0;
	
			gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
			gos_SetRenderState( gos_State_ShadeMode, gos_ShadeFlat);
			gos_SetRenderState( gos_State_MonoEnable, 0);
			gos_SetRenderState( gos_State_Perspective, 1);
			gos_SetRenderState( gos_State_Clipping, 2);
			gos_SetRenderState( gos_State_AlphaTest, 0);
			gos_SetRenderState( gos_State_Specular, 0);
			gos_SetRenderState( gos_State_Dither, 0);
			gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendDecal);
			gos_SetRenderState( gos_State_Filter, gos_FilterNone);
			gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
			gos_SetRenderState( gos_State_ZCompare, 1);
			gos_SetRenderState(	gos_State_ZWrite, 0);
			gos_SetRenderState(gos_State_Texture,0);
	
			gos_DrawPoints(&planetPoint,1);
		}

		if (selected & DRAW_BARS)
		{
			drawBars();
		}

		if ( selected & DRAW_BRACKETS )
			drawSelectBrackets(color);
			
		if (selected & DRAW_TEXT && objectNameId != -1)
		{	
			char tmpString[255];
			cLoadString(objectNameId, tmpString, 254);

			drawTextHelp(tmpString, color);
	
		}
		
//		selected = FALSE;
//#define DRAW_BOX
#ifdef DRAW_BOX
		//---------------------------------------------------------
		// Render the Bounding Box to see if it is OK.
		Stuff::Vector3D nodeCenter = bldgShape->GetRootNodeCenter();
		Stuff::Vector3D boxStart;
		Stuff::Vector3D boxEnd;
		boxStart.x = -(bldgShape->GetMinBox().x + nodeCenter.x);
		boxStart.z = bldgShape->GetMinBox().y + nodeCenter.y;
		boxStart.y = bldgShape->GetMinBox().z + nodeCenter.z;
		
		boxEnd.x = -(bldgShape->GetMaxBox().x + nodeCenter.x);
		boxEnd.z = bldgShape->GetMaxBox().y + nodeCenter.y;
		boxEnd.y = bldgShape->GetMaxBox().z + nodeCenter.z;
		
 		Stuff::Vector3D boxCoords[8];
		Stuff::Vector3D addCoords;
		
		addCoords.x = boxStart.x;
		addCoords.y = boxStart.y;
		addCoords.z = boxEnd.z;
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[0].Add(position,addCoords);

		addCoords.x = boxStart.x;
		addCoords.y = boxEnd.y;  
		addCoords.z = boxEnd.z;  		
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[1].Add(position,addCoords);

 		addCoords.x = boxEnd.x; 
		addCoords.y = boxEnd.y; 
		addCoords.z = boxEnd.z; 		
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[2].Add(position,addCoords);
		
 		addCoords.x = boxEnd.x;   
		addCoords.y = boxStart.y; 
		addCoords.z = boxEnd.z;   		
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[3].Add(position,addCoords);
		
 		addCoords.x = boxStart.x;
		addCoords.y = boxStart.y; 
		addCoords.z = boxStart.z; 		
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[4].Add(position,addCoords);
 					  
 		addCoords.x = boxEnd.x;   
		addCoords.y = boxStart.y;   
		addCoords.z = boxStart.z; 
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[5].Add(position,addCoords);
		
 		addCoords.x = boxEnd.x;   
		addCoords.y = boxEnd.y;   
		addCoords.z = boxStart.z; 
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[6].Add(position,addCoords);
		
 		addCoords.x = boxStart.x; 
		addCoords.y = boxEnd.y;   
		addCoords.z = boxStart.z; 
		if (rotation != 0.0f)
			Rotate(addCoords,-rotation);
 		
		boxCoords[7].Add(position,addCoords);
		
		Stuff::Vector4D screenPos[8];
		for (long i=0;i<8;i++)
		{
			eye->projectZ(boxCoords[i],screenPos[i]);
		}
		
		{
			LineElement newElement(screenPos[0],screenPos[1],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[0],screenPos[4],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[0],screenPos[3],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[5],screenPos[4],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[5],screenPos[6],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[5],screenPos[3],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[2],screenPos[3],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[2],screenPos[6],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[2],screenPos[1],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[7],screenPos[1],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[7],screenPos[6],XP_WHITE,NULL,-1);
			newElement.draw();
		}
		
		{
			LineElement newElement(screenPos[7],screenPos[4],XP_WHITE,NULL,-1);
			newElement.draw();
		}
#endif
#undef DRAW_BOX
	}
	return NO_ERR;
}

//-----------------------------------------------------------------------------
long GenericAppearance::renderShadows (void)
{
	if (inView && visible)
	{
		//---------------------------------------------
		// Call Multi-shape render stuff here.
		genShape->RenderShadows();
	}
	return NO_ERR;
}

//-----------------------------------------------------------------------------
long GenericAppearance::update (bool animate) 
{
	Stuff::Point3D xlatPosition;
	Stuff::UnitQuaternion rot;
	Stuff::UnitQuaternion yawAngle;
	Stuff::UnitQuaternion pitchAngle;

	if (inView)
	{
		if (rotation > 180)
			rotation -= 360;
	
		if (rotation < -180)
			rotation += 360;
	
		//-------------------------------------------
		// Does math necessary to draw Tree
		float yaw = rotation * DEGREES_TO_RADS;
		float p = pitch * DEGREES_TO_RADS;
		pitchAngle = Stuff::EulerAngles(p, 0.0f, 0.0f);
		yawAngle = Stuff::EulerAngles(0.0f,yaw,0.0f); 
		rot = pitchAngle;
		rot.Multiply(pitchAngle,yawAngle);
	
		unsigned char lightr,lightg,lightb;
		float lightIntensity = 1.0f;
		if (land)
			land->getTerrainLight(position);
							
		lightr = eye->getLightRed(lightIntensity);
		lightg = eye->getLightGreen(lightIntensity);
		lightb = eye->getLightBlue(lightIntensity);
	
		DWORD lightRGB = (lightr<<16) + (lightg<<8) + lightb;
		
		eye->setLightColor(0,lightRGB);
		eye->setLightIntensity(0,1.0);
	
		DWORD fogRGB = 0xff<<24;
		float fogStart = eye->fogStart;
		float fogFull = eye->fogFull;
	
		xlatPosition.x = -position.x;
		xlatPosition.y = position.z;
		xlatPosition.z = position.y;
	
		if (xlatPosition.y < fogStart)
		{
			float fogFactor = fogStart - xlatPosition.y;
			if (fogFactor < 0.0)
				fogRGB = 0xff<<24;
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
	
				unsigned char fogResult = float2long(fogFactor);
				fogRGB = fogResult << 24;
			}
		}
		else
		{
			fogRGB = 0xff<<24;
		}
	
		if (useFog)
			genShape->SetFogRGB(fogRGB);
		else
			genShape->SetFogRGB(0xffffffff);
	}

	if (animate && genFrameRate != 0.0f)
	{
		//--------------------------------------------------------
		// Make sure animation runs no faster than bdFrameRate fps.
		float frameInc = genFrameRate * frameLength;
		
		//---------------------------------------
		// Increment Frames -- Everything else!
		if (frameInc != 0.0f)
		{
			if (!setFirstFrame)		//DO NOT ANIMATE ON FIRST FRAME!  Wait a bit!
			{
				if (isReversed)
					currentFrame -= frameInc;
				else
					currentFrame += frameInc;
			}
			else
			{
				setFirstFrame = false;
			}
	
			//--------------------------------------
			//Check Positive overflow of Animation
			if (currentFrame >= appearType->getNumFrames(genAnimationState))
			{
				if (isLooping)
					currentFrame -= appearType->getNumFrames(genAnimationState);
				else
					currentFrame = appearType->getNumFrames(genAnimationState) - 1;
					
				canTransition = true;		//Whenever we have completed one cycle or at last frame, OK to move on!
			}
			
	
			//--------------------------------------
			//Check negative overflow of gesture
			if (currentFrame < 0)
			{
				if (isLooping)
					currentFrame += appearType->getNumFrames(genAnimationState); 
				else
					currentFrame = 0.0f; 
					
				canTransition = true;		//Whenever we have completed one cycle or at last frame, OK to move on!
			}
		}
		
		genShape->SetFrameNum(currentFrame);
	}

	if (inView)
	{
		genShape->SetIsClamped(true);
		genShape->SetLightList(NULL,0);
		genShape->TransformMultiShape (&xlatPosition,&rot);
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------
void GenericAppearance::destroy (void)
{
	if ( genShape )
	{
		delete genShape;
		genShape = NULL;
	}

	appearanceTypeList->removeAppearance(appearType);
}

#define HEIGHT_THRESHOLD 10.0f
//-----------------------------------------------------------------------------
void GenericAppearance::markTerrain (_ScenarioMapCellInfo* pInfo, int type, int counter)
{
}

//-----------------------------------------------------------------------------
void GenericAppearance::markMoveMap (bool passable)
{
}

///////////////////////////////////////////////////////////////////////////////////////
