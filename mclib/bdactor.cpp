//---------------------------------------------------------------------------
//
//	bdactor.cpp - This file contains the code for the building and tree appearance classes
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef BDACTOR_H
#include"bdactor.h"
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

#include "../code/unitdesg.h" /* just for definition of MIN_TERRAIN_PART_ID and MAX_MAP_CELL_WIDTH */
//******************************************************************************************
extern float	worldUnitsPerMeter;
extern bool 	drawTerrainGrid;
extern bool		useFog;

extern long 	mechRGBLookup[];
extern long 	mechRGBLookup2[];

extern long		ObjectTextureSize;

extern bool		reloadBounds;
extern float	metersPerWorldUnit;
extern bool		useShadows;

extern MidLevelRenderer::MLRClipper * theClipper;

extern bool useNonWeaponEffects;
extern bool useHighObjectDetail;
extern bool MLRVertexLimitReached;

#define SPINRATE					90.0f
#define BASE_NODE_RECYCLE_TIME		0.25f
#define MAX_WEAPON_NODES			4
//-----------------------------------------------------------------------------
// class BldgAppearanceType
void BldgAppearanceType::init (const char * fileName)
{
	AppearanceType::init(fileName);

	//----------------------------------------------
	FullPathFileName iniName;
	iniName.init(tglPath,fileName,".ini");

	FitIniFile iniFile;
	long result = iniFile.open(iniName);
	if (result != NO_ERR)
		STOP(("Could not find building appearance INI file %s",iniName));

	result = iniFile.seekBlock("TGLData");
	if (result != NO_ERR)
		Fatal(result,"Could not find block in building appearance INI file");

	result = iniFile.readIdBoolean("SpinMe",spinMe);
	if (result != NO_ERR)
		spinMe = false;
		
	float nFrameRate = 0.0f;
	result = iniFile.readIdFloat("FrameRate",nFrameRate);
	if (result != NO_ERR)
		nFrameRate = 0.0f;

	result = iniFile.readIdBoolean("ForestClump",isForestClump);
	if (result != NO_ERR)
		isForestClump = false;
	   
	DWORD hotPinkRGB, hotGreenRGB, hotYellowRGB;
	result = iniFile.readIdULong("HotPinkRGB",hotPinkRGB);
	if (result != NO_ERR)
		hotPinkRGB = 0xffff00ff;
		
	result = iniFile.readIdULong("HotGreenRGB",hotGreenRGB);
	if (result != NO_ERR)
		hotGreenRGB = 0xff00ff00;
		
	result = iniFile.readIdULong("HotYellowRGB",hotYellowRGB);
	if (result != NO_ERR)
		hotYellowRGB = 0xffffff00;

	result = iniFile.readIdULong("TerrainLightRGB",terrainLightRGB);
	if (result != NO_ERR)
	{
		terrainLightRGB = 0xffffffff;
	}
	else
	{
		result = iniFile.readIdFloat("TerrainLightIntensity",terrainLightIntensity);
		if (result != NO_ERR)
			terrainLightIntensity = 0.5f;
			
		result = iniFile.readIdFloat("TerrainLightInnerRadius",terrainLightInnerRadius);
		if (result != NO_ERR)
			terrainLightInnerRadius = 100.0f;
			
		result = iniFile.readIdFloat("TerrainLightOuterRadius",terrainLightOuterRadius);
		if (result != NO_ERR)
			terrainLightOuterRadius = 250.0f;
	}
	
	char aseFileName[512];
	result = iniFile.readIdString("FileName",aseFileName,511);
	if (result != NO_ERR)
	{
		//Check for LOD filenames instead
		for (int i=0;i<MAX_LODS;i++)
		{
			char baseName[256];
			char baseLODDist[256];
			sprintf(baseName,"FileName%d",i);
			sprintf(baseLODDist,"Distance%d",i);
			
			result = iniFile.readIdString(baseName,aseFileName,511);
			if (result == NO_ERR)
			{
				result = iniFile.readIdFloat(baseLODDist,lodDistance[i]);
				if (result != NO_ERR)
					STOP(("LOD %d has no distance value in file %s",i,fileName));
					
				//----------------------------------------------
				// Base LOD shape.  In stand Pose by default.
				bldgShape[i] = new TG_TypeMultiShape;
				gosASSERT(bldgShape[i] != NULL);
			
				FullPathFileName bldgName;
				bldgName.init(tglPath,aseFileName,".ase");
			
				bldgShape[i]->LoadTGMultiShapeFromASE(bldgName);
			}
			else if (!i)
			{
				STOP(("No base LOD for shape %s",fileName));
			}
		}
	}
	else
	{
		//----------------------------------------------
		// Base shape.  In stand Pose by default.
		bldgShape[0] = new TG_TypeMultiShape;
		gosASSERT(bldgShape[0] != NULL);
	
		FullPathFileName bldgName;
		bldgName.init(tglPath,aseFileName,".ase");
	
		bldgShape[0]->LoadTGMultiShapeFromASE(bldgName);
	}

	result = iniFile.readIdString("ShadowName",aseFileName,511);
	if (result == NO_ERR)
	{
		//----------------------------------------------
		// Base Shadow shape.
		bldgShadowShape = new TG_TypeMultiShape;
		gosASSERT(bldgShadowShape != NULL);
	
		FullPathFileName bldgName;
		bldgName.init(tglPath,aseFileName,".ase");
	
		bldgShadowShape->LoadTGMultiShapeFromASE(bldgName);
	}
 
	//destroyed state.
	result = iniFile.seekBlock("TGLDamage");
	if (result == NO_ERR)
	{
		result = iniFile.readIdString("FileName",aseFileName,511);
		if (result != NO_ERR)
			Fatal(result,"Could not find ASE FileName in building appearance INI file");
	
		FullPathFileName dmgName;
		dmgName.init(tglPath,aseFileName,".ase");
	
		bldgDmgShape = new TG_TypeMultiShape;
		gosASSERT(bldgDmgShape != NULL);
		bldgDmgShape->LoadTGMultiShapeFromASE(dmgName);

		if (!bldgDmgShape->GetNumShapes())
		{
			delete bldgDmgShape;
			bldgDmgShape = NULL;
		}
		
		//Shadow for destroyed state.
		result = iniFile.readIdString("ShadowName",aseFileName,511);
		if (result == NO_ERR)
		{
			//----------------------------------------------
			// Base Shadow shape.
			bldgDmgShadowShape = new TG_TypeMultiShape;
			gosASSERT(bldgDmgShadowShape != NULL);
		
			FullPathFileName bldgName;
			bldgName.init(tglPath,aseFileName,".ase");
		
			bldgDmgShadowShape->LoadTGMultiShapeFromASE(bldgName);
			if (!bldgDmgShadowShape->GetNumShapes())
			{
				delete bldgDmgShadowShape;
				bldgDmgShadowShape = NULL;
			}
		}
	}
	else
	{
		bldgDmgShape = NULL;
		bldgDmgShadowShape = NULL;
	}

	result = iniFile.seekBlock("TGLDestructEffect");
	if (result == NO_ERR)
	{
		result = iniFile.readIdString("FileName",destructEffect,59);
		if (result != NO_ERR)
			STOP(("Could not Find DestructEffectName in building appearance INI file"));
	
	}
	else
	{
		destructEffect[0] = 0;
	}

	//--------------------------------------------------------------------
	// Load Animation Information.
	// We can load up to 10 Animation States.
	for (int i=0;i<MAX_BD_ANIMATIONS;i++)
	{
		char blockId[512];
		sprintf(blockId,"Animation:%d",i);
		
		result = iniFile.seekBlock(blockId);
		if (result == NO_ERR)
		{
			char animName[512];
			result = iniFile.readIdString("AnimationName",animName,511);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("LoopAnimation",bdAnimLoop[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("Reverse",bdReverse[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("Random",bdRandom[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdLong("StartFrame",bdStartF[i]);
			if (result != NO_ERR)
				bdStartF[i] = 0;
				
 			//-------------------------------
			// We have an animation to load.
			FullPathFileName animPath;
			animPath.init(tglPath,animName,".ase");

			FullPathFileName otherPath;
			otherPath.init(tglPath,animName,".agl");

			if (fileExists(animPath) || fileExists(otherPath))
			{
				bdAnimData[i] = new TG_AnimateShape;
				gosASSERT(bdAnimData[i] != NULL);
	
				//--------------------------------------------------------
				// If this animation does not exist, it is not a problem!
				// Building will simply freeze until animation is "over"
				bdAnimData[i]->LoadTGMultiShapeAnimationFromASE(animPath,bldgShape[0]);
			}
			else
				bdAnimData[i] = NULL;
		}
		else
		{
			bdAnimData[i] = NULL;
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
	
	if (nFrameRate != 0.0f)
	{
		for (long i=0;i<MAX_BD_ANIMATIONS;i++)
			setFrameRate(i,nFrameRate);
	}

	//-----------------------------------------------
	// Load up the Weapon Node Data.
	numWeaponNodes = 0;
	nodeData = NULL;
	result = iniFile.seekBlock("WeaponNode");
	if (result == NO_ERR)
	{
		nodeData = (NodeData *)AppearanceTypeList::appearanceHeap->Malloc(sizeof(NodeData)*(MAX_WEAPON_NODES));
		gosASSERT(nodeData != NULL);
		
		for (int i=0;i<MAX_WEAPON_NODES;i++)
		{
			char blockId[512];
			sprintf(blockId,"WeaponNodeId%d",i);
			
			char weaponName[512];
			result = iniFile.readIdString(blockId,weaponName,511);
			if (result != NO_ERR)
			{
				strcpy(weaponName,"NONE");
			}
			
			nodeData[i].nodeId = (char *)AppearanceTypeList::appearanceHeap->Malloc(strlen(weaponName)+1);
			gosASSERT(nodeData[i].nodeId != NULL);
				
			strcpy(nodeData[i].nodeId,weaponName);
			nodeData[i].weaponType = 0;
			numWeaponNodes++;
 		}
	}

	for (int i=0;i<MAX_LODS;i++)
	{
		if (bldgShape[i])
			bldgShape[i]->SetLightRGBs(hotPinkRGB, hotGreenRGB, hotYellowRGB);
	}
}

//----------------------------------------------------------------------------
void BldgAppearanceType::destroy (void)
{
	AppearanceType::destroy();

	for (long i=0;i<MAX_LODS;i++)
	{
		if (bldgShape[i])
		{
			delete bldgShape[i];
			bldgShape[i] = NULL;
		}
	}

	if (bldgShadowShape)
	{
		delete bldgShadowShape;
		bldgShadowShape = NULL;
	}
	
 	if (bldgDmgShape)
	{
		delete bldgDmgShape;
		bldgDmgShape = NULL;
	}
	
	if (bldgDmgShadowShape)
	{
		delete bldgDmgShadowShape;
		bldgDmgShadowShape = NULL;
	}
	
 	for (int i=0;i<MAX_BD_ANIMATIONS;i++)
	{
		if (bdAnimData[i])
		{
			delete bdAnimData[i];
			bdAnimData[i] = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
void BldgAppearanceType::setAnimation (TG_MultiShapePtr shape, DWORD animationNum)
{
	gosASSERT(shape != NULL);
	gosASSERT(animationNum != 0xffffffff);
	gosASSERT(animationNum < MAX_BD_ANIMATIONS);

	if (bdAnimData[animationNum])
		bdAnimData[animationNum]->SetAnimationState(shape);
	else
		shape->ClearAnimation();
}

//-----------------------------------------------------------------------------
// class BldgAppearance
void BldgAppearance::setWeaponNodeUsed (long weaponNode)
{
	weaponNode -= appearType->numWeaponNodes;
   	if ((weaponNode >= 0) && (weaponNode < appearType->numWeaponNodes))
	{
		nodeUsed[weaponNode]++;
		nodeRecycle[weaponNode] = BASE_NODE_RECYCLE_TIME;
	}
}

//-----------------------------------------------------------------------------
Stuff::Vector3D BldgAppearance::getWeaponNodePosition (long nodeId)
{
	Stuff::Vector3D result = position;
	if ((nodeId < 0) || (nodeId >= appearType->numWeaponNodes))
		return result;
	
	//We already know we are using this node.  Do NOT increment recycle or nodeUsed!
		
   	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = land->getTerrainElevation(position);
   	xlatPosition.z = position.y;
   
   	Stuff::UnitQuaternion torsoRot;
   	torsoRot = Stuff::EulerAngles(0.0f,(turretYaw * DEGREES_TO_RADS),0.0f);
	if (rotationalNodeId == -1)
	{
		if (stricmp(appearType->rotationalNodeId,"NONE") != 0)
			rotationalNodeId = bldgShape->GetNodeNameId(appearType->rotationalNodeId);
		else
			rotationalNodeId = -2;
	}
   
	if (rotationalNodeId >= 0)
	   	bldgShape->SetNodeRotation(rotationalNodeId,&torsoRot);

	result = bldgShape->GetTransformedNodePosition(&xlatPosition,&qRotation,appearType->nodeData[nodeId].nodeId);

	if ((result.x == 0.0f) &&
		(result.y == 0.0f) && 
		(result.z == 0.0f))
		result = position;
		
 	return result;
}

//-----------------------------------------------------------------------------
Stuff::Vector3D BldgAppearance::getHitNode (void)
{
	if (hitNodeId == -1)
		hitNodeId = bldgShape->GetNodeNameId("hitnode");

	Stuff::Vector3D result = getNodeIdPosition(hitNodeId);
 	return result;
}

//-----------------------------------------------------------------------------
long BldgAppearance::getWeaponNode (long weaponType)
{
	//------------------------------------------------
	// Scan all weapon nodes and find least used one.
	long leastUsed = 999999999;
	long bestNode = -1;
	for (long i=0;i<appearType->numWeaponNodes;i++)
	{
		if (nodeUsed[i] < leastUsed)
		{
			leastUsed = nodeUsed[i];
			bestNode = i;
		}
	}
		
   	if ((bestNode < 0) || (bestNode >= appearType->numWeaponNodes))
   		return -1;

 	return bestNode;
}
		
//-----------------------------------------------------------------------------
float BldgAppearance::getWeaponNodeRecycle (long node)
{
	if ((node >= 0) && (node < appearType->numWeaponNodes))
		return nodeRecycle[node];
		
	return 0.0f;
}

//-----------------------------------------------------------------------------
void BldgAppearance::init (AppearanceTypePtr tree, GameObjectPtr obj)
{
	Appearance::init(tree,obj);
	appearType = (BldgAppearanceType *)tree;

	shapeMin.x = shapeMin.y = -25;
	shapeMax.x = shapeMax.y = 50;

	bdAnimationState =-1;
	currentFrame = 0.0f;
	bdFrameRate = 0.0f;
	isReversed = false;
	isLooping = false;
	setFirstFrame = false;
	canTransition = true;
	
	paintScheme = -1;
	objectNameId = 30469;
	hazeFactor = 0.0f;

	rotationalNodeId = -1;
	hitNodeId = activityNodeId = activityNode1Id = -1;

	currentFlash = duration = flashDuration = 0.0f;
	flashColor = 0x00000000;
	drawFlash = false;

	pointLight = NULL;
	lightId = 0xffffffff;
	forceLightsOut = false;
	
	screenPos.x = screenPos.y = screenPos.z = screenPos.w = -999.0f;
	position.Zero();
	rotation = 0.0f;
	selected = 0;
	teamId = -1;
	homeTeamRelationship = 0;
	actualRotation = rotation;

	currentLOD = 0;
 	
	turretYaw = turretPitch = 0.0f;
	
	destructFX = NULL;
	activity = NULL;
	activity1 = NULL;
	isActivitying = false;

	OBBRadius = -1.0f;
	highZ = -1.0f;
	
	nodeUsed = NULL;
	nodeRecycle = NULL;
	
	beenInView = false;

	fogLightSet = false;
	if (appearType)
	{
		bldgShape = appearType->bldgShape[0]->CreateFrom();

		//-------------------------------------------------
		// Load the texture and store its handle.
		for (int i=0;i<bldgShape->GetNumTextures();i++)
		{
			char txmName[1024];
			bldgShape->GetTextureName(i,txmName,256);

			char texturePath[1024];
			sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
	
			FullPathFileName textureName;
			textureName.init(texturePath,txmName,"");
	
			if (fileExists(textureName))
			{
				if (strnicmp(txmName,"a_",2) == 0)
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
					gosASSERT(gosTextureHandle != 0xffffffff);
					bldgShape->SetTextureHandle(i,gosTextureHandle);
					bldgShape->SetTextureAlpha(i,true);
				}
				else
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
					gosASSERT(gosTextureHandle != 0xffffffff);
					bldgShape->SetTextureHandle(i,gosTextureHandle);
					bldgShape->SetTextureAlpha(i,false);
				}
			}
			else
			{
				//PAUSE(("Warning: %s texture name not found",textureName));
				bldgShape->SetTextureHandle(i,0xffffffff);
			}
		}
		
		if (appearType->bldgShadowShape)
		{
			bldgShadowShape = appearType->bldgShadowShape->CreateFrom();
	
			//-------------------------------------------------
			// Load the texture and store its handle.
			for (long i=0;i<bldgShadowShape->GetNumTextures();i++)
			{
				char txmName[1024];
				bldgShadowShape->GetTextureName(i,txmName,256);
		
				char texturePath[1024];
				sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
		
				FullPathFileName textureName;
				textureName.init(texturePath,txmName,"");
		
				if (fileExists(textureName))
				{
					if (strnicmp(txmName,"a_",2) == 0)
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						bldgShadowShape->SetTextureHandle(i,gosTextureHandle);
						bldgShadowShape->SetTextureAlpha(i,true);
					}
					else
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						bldgShadowShape->SetTextureHandle(i,gosTextureHandle);
						bldgShadowShape->SetTextureAlpha(i,false);
					}
				}
				else
				{
					bldgShadowShape->SetTextureHandle(i,0xffffffff);
				}
			}
		}
		else
		{
			bldgShadowShape = NULL;
		}
 		
		Stuff::Vector3D boxCoords[8];
		Stuff::Vector3D nodeCenter = bldgShape->GetRootNodeCenter();

		boxCoords[0].x = position.x + bldgShape->GetMinBox().x + nodeCenter.x;
		boxCoords[0].y = position.y + bldgShape->GetMinBox().z + nodeCenter.z;
		boxCoords[0].z = position.z + bldgShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[1].x = position.x + bldgShape->GetMinBox().x + nodeCenter.x;
		boxCoords[1].y = position.y + bldgShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[1].z = position.z + bldgShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[2].x = position.x + bldgShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[2].y = position.y + bldgShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[2].z = position.z + bldgShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[3].x = position.x + bldgShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[3].y = position.y + bldgShape->GetMinBox().z + nodeCenter.z;
		boxCoords[3].z = position.z + bldgShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[4].x = position.x + bldgShape->GetMinBox().x + nodeCenter.x;
		boxCoords[4].y = position.y + bldgShape->GetMinBox().z + nodeCenter.z;
		boxCoords[4].z = position.z + bldgShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[5].x = position.x + bldgShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[5].y = position.y + bldgShape->GetMinBox().z + nodeCenter.z;
		boxCoords[5].z = position.z + bldgShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[6].x = position.x + bldgShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[6].y = position.y + bldgShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[6].z = position.z + bldgShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[7].x = position.x + bldgShape->GetMinBox().x + nodeCenter.x;
		boxCoords[7].y = position.y + bldgShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[7].z = position.z + bldgShape->GetMinBox().y + nodeCenter.y;
		
 		float testRadius = 0.0;
		
		for (int i=0;i<8;i++)
		{
			testRadius = boxCoords[i].GetLength();
			if (OBBRadius < testRadius)
				OBBRadius = testRadius;

			if (boxCoords[i].z > highZ)
				highZ = boxCoords[i].z;
		}
		
		appearType->boundsUpperLeftX = (-OBBRadius * 2.0);
		appearType->boundsUpperLeftY = (-OBBRadius * 2.0);
		   					 
		appearType->boundsLowerRightX = (OBBRadius * 2.0);
		appearType->boundsLowerRightY = (OBBRadius);
		
		if (!appearType->getDesignerTypeBounds())
		{
			Stuff::Vector3D nodeCenter = bldgShape->GetRootNodeCenter();
			appearType->typeUpperLeft.Add(bldgShape->GetMinBox(),nodeCenter);
			appearType->typeLowerRight.Add(bldgShape->GetMaxBox(),nodeCenter);
		}
		
 		if (appearType->numWeaponNodes)
		{
			nodeUsed = (long *)AppearanceTypeList::appearanceHeap->Malloc(sizeof(long) * appearType->numWeaponNodes);
			gosASSERT(nodeUsed != NULL);
			memset(nodeUsed,0,sizeof(long) * appearType->numWeaponNodes);
			
			nodeRecycle = (float *)AppearanceTypeList::appearanceHeap->Malloc(sizeof(float) * appearType->numWeaponNodes);
			gosASSERT(nodeRecycle != NULL);
			
			for (long i=0;i<appearType->numWeaponNodes;i++)
				nodeRecycle[i] = 0.0f;
		}
	}
}

//-----------------------------------------------------------------------------
void BldgAppearance::setObjStatus (long oStatus)
{
	if (status != oStatus)
	{
		if ((oStatus == OBJECT_STATUS_DESTROYED) || (oStatus == OBJECT_STATUS_DISABLED))
		{
			if (appearType->bldgDmgShape)
			{
				if (bldgShape)
				{
					bldgShape->ClearAnimation();
					delete bldgShape;
					bldgShape = NULL;
				}
				
				bldgShape = appearType->bldgDmgShape->CreateFrom();
				if (bdAnimationState != -1)
					appearType->setAnimation(bldgShape,bdAnimationState);
				
				beenInView = false; 
				currentLOD = 0;
			}
			
			if (appearType->bldgDmgShadowShape)
			{
				if (bldgShadowShape)
				{
					bldgShadowShape->ClearAnimation();
					delete bldgShadowShape;
					bldgShadowShape = NULL;
				}
				
				bldgShadowShape = appearType->bldgDmgShadowShape->CreateFrom();
				
				//Do shadows need to animate??
				//if (bdAnimationState != -1)
					//appearType->setAnimation(bldgShadowShape,bdAnimationState);
				
				beenInView = false; 
			}

			stopActivity();
		}
		
		if (oStatus == OBJECT_STATUS_NORMAL)
		{
			if (appearType->bldgShape[0])
			{
				if (bldgShape)
				{
					bldgShape->ClearAnimation();
					delete bldgShape;
					bldgShape = NULL;
				}
				
				bldgShape = appearType->bldgShape[0]->CreateFrom();
				if (bdAnimationState != -1)
					appearType->setAnimation(bldgShape,bdAnimationState);
				
				beenInView = false; 
			}
			
			if (appearType->bldgShadowShape)
			{
				if (bldgShadowShape)
				{
					bldgShadowShape->ClearAnimation();
					delete bldgShadowShape;
					bldgShadowShape = NULL;
				}
				
				bldgShadowShape = appearType->bldgShadowShape->CreateFrom();
				
				//Do shadows need to animate??
//				if (bdAnimationState != -1)
					//appearType->setAnimation(bldgShadowShape,bdAnimationState);
				
				beenInView = false; 
			}
		}
		
		if (bldgShape)
		{
			//-------------------------------------------------
			// Load the texture and store its handle.
			for (long i=0;i<bldgShape->GetNumTextures();i++)
			{
				char txmName[1024];
				bldgShape->GetTextureName(i,txmName,256);
	
				char texturePath[1024];
				sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
		
				FullPathFileName textureName;
				textureName.init(texturePath,txmName,"");
		
				if (fileExists(textureName))
				{
					if (strnicmp(txmName,"a_",2) == 0)
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						bldgShape->SetTextureHandle(i,gosTextureHandle);
						bldgShape->SetTextureAlpha(i,true);
					}
					else
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						bldgShape->SetTextureHandle(i,gosTextureHandle);
						bldgShape->SetTextureAlpha(i,false);
					}
				}
				else
				{
					//PAUSE(("Warning: %s texture name not found",textureName));
					bldgShape->SetTextureHandle(i,0xffffffff);
				}
			}
		}

		if (bldgShadowShape)
		{
			//-------------------------------------------------
			// Load the texture for the shadow and store its handle.
			for (long i=0;i<bldgShadowShape->GetNumTextures();i++)
			{
				char txmName[1024];
				bldgShadowShape->GetTextureName(i,txmName,256);
	
				char texturePath[1024];
				sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
		
				FullPathFileName textureName;
				textureName.init(texturePath,txmName,"");
		
				if (fileExists(textureName))
				{
					if (strnicmp(txmName,"a_",2) == 0)
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						bldgShadowShape->SetTextureHandle(i,gosTextureHandle);
						bldgShadowShape->SetTextureAlpha(i,true);
					}
					else
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						bldgShadowShape->SetTextureHandle(i,gosTextureHandle);
						bldgShadowShape->SetTextureAlpha(i,false);
					}
				}
				else
				{
					bldgShadowShape->SetTextureHandle(i,0xffffffff);
				}
			}
		}
	}
	
	status = oStatus;
}

//-----------------------------------------------------------------------------
Stuff::Vector3D BldgAppearance::getNodeNamePosition (const char *nodeName)
{
	Stuff::Vector3D result = position;
	
   	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = position.z;
   	xlatPosition.z = position.y;
   
	result = bldgShape->GetTransformedNodePosition(&xlatPosition,&qRotation,nodeName);

	if ((result.x == 0.0f) &&
		(result.y == 0.0f) && 
		(result.z == 0.0f))
		result = position;
		
	return result;
}

//-----------------------------------------------------------------------------
Stuff::Vector3D BldgAppearance::getNodeIdPosition (long nodeId)
{
	Stuff::Vector3D result = position;
	
   	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = position.z;
   	xlatPosition.z = position.y;
   
	result = bldgShape->GetTransformedNodePosition(&xlatPosition,&qRotation,nodeId);

	if ((result.x == 0.0f) &&
		(result.y == 0.0f) && 
		(result.z == 0.0f))
		result = position;
		
	return result;
}

//-----------------------------------------------------------------------------
bool BldgAppearance::PerPolySelect (long mouseX, long mouseY)
{
	return bldgShape->PerPolySelect(mouseX,mouseY);
}

//-----------------------------------------------------------------------------
void BldgAppearance::setGesture (unsigned long gestureId)
{
	//------------------------------------------------------------
	// Check if state is possible.
	if (gestureId >= MAX_BD_ANIMATIONS)
		return;

	//------------------------------------------------------------
	// Check if object destroyed.  If so, no animation!
	if ((status == OBJECT_STATUS_DESTROYED) || (status == OBJECT_STATUS_DISABLED))
		return;
		
	if (gestureId == bdAnimationState)
		return;

	//----------------------------------------------------------------------
	// If state is OK, set animation data, set first frame, set loop and 
	// reverse flag, and start it going until you hear otherwise.
	appearType->setAnimation(bldgShape,gestureId);
	bdAnimationState = gestureId;
	currentFrame = 0.0f;
	if (appearType->bdStartF[gestureId])
		currentFrame = appearType->bdStartF[gestureId];
		
	isReversed = false;
	
	if (appearType->isReversed(bdAnimationState))
	{
		currentFrame = appearType->getNumFrames(bdAnimationState)-1;
		isReversed = true;
	}
	
	if (appearType->isRandom(bdAnimationState))
	{
		currentFrame = RandomNumber(appearType->getNumFrames(bdAnimationState)-1);
	}
	
	isLooping = appearType->isLooped(bdAnimationState);
	
	bdFrameRate = appearType->getFrameRate(bdAnimationState);
	
	setFirstFrame = true;
	if (bdFrameRate > Stuff::SMALL)
		canTransition = false;
	else
		canTransition = true;		//We can change immediately to another animation because we have no animation for this state!
}

//-----------------------------------------------------------------------------
void BldgAppearance::setMoverParameters (float turretRot, float lArmRot, float rArmRot, bool isAirborne)
{
	turretYaw = turretRot;
	turretPitch = lArmRot;
}

//-----------------------------------------------------------------------------
void BldgAppearance::setObjectParameters (const Stuff::Vector3D &pos, float Rot, long sel, long team, long homeRelations)
{
	rotation = Rot;

	position = pos;

	selected = sel;

	actualRotation = Rot;

	teamId = team;
	homeTeamRelationship = homeRelations;
}

//-----------------------------------------------------------------------------
bool BldgAppearance::isMouseOver (float px, float py)
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
bool BldgAppearance::recalcBounds (void)
{
	Stuff::Vector4D tempPos;
	inView = false;

	float distanceToEye = 0.0f;
	
	if (eye)
	{
		//-------------------------------------------------------------------
		//NEW METHOD from the WAY BACK Days
		inView = true;
		
		if (eye->usePerspective)
		{
			Stuff::Vector3D cameraPos;
			cameraPos.x = -eye->getCameraOrigin().x;
			cameraPos.y = eye->getCameraOrigin().z;
			cameraPos.z = eye->getCameraOrigin().y;
			float vClipConstant = eye->verticalSphereClipConstant;
			float hClipConstant = eye->horizontalSphereClipConstant; 
	
			Stuff::Vector3D objectCenter;
			objectCenter.Subtract(position,cameraPos);
			Camera::cameraFrame.trans_to_frame(objectCenter);
			distanceToEye = objectCenter.GetApproximateLength();
			float clip_distance = fabs(1.0f / objectCenter.y);
			
			//Is vertex on Screen OR close enough to screen that its triangle MAY be visible?
			// WE have removed the atans here by simply taking the tan of the angle we want above.
			float object_angle = fabs(objectCenter.z) * clip_distance;
			float extent_angle = bldgShape->GetExtentRadius() / distanceToEye;
			if (object_angle > (vClipConstant + extent_angle))
			{
				//In theory, we would return here.  Object is NOT on screen.
				inView = false;
			}
			else
			{
				object_angle = fabs(objectCenter.x) * clip_distance;
				if (object_angle > (hClipConstant + extent_angle))
				{
					//In theory, we would return here.  Object is NOT on screen.
					inView = false;
				}
			}
		}
		
		//Can we be seen at all?
		// If yes, check if we are behind fog plane.
		if (inView)
		{
			//ALWAYS need to do this or select is YAYA
			// But now inView is correct!!
			eye->projectZ(position,screenPos);
			
			if (eye->usePerspective)
			{
				if (distanceToEye > Camera::MaxClipDistance)
				{
					hazeFactor = 1.0f;
					inView = false;
				}
				else if (distanceToEye > Camera::MinHazeDistance)
				{
					Camera::HazeFactor = (distanceToEye - Camera::MinHazeDistance) * Camera::DistanceFactor;
					inView = true;
				}
				else
				{
					Camera::HazeFactor = 0.0f;
					inView = true;
				}
			}
			else
			{
				Camera::HazeFactor = 0.0f;
				inView = true;
			}
		}
		
		//If we were not behind fog plane, do a bunch O math we need later!!
		if (inView)
		{
			if (reloadBounds)
				appearType->reinit();

			appearType->boundsLowerRightY = (OBBRadius * eye->getTiltFactor() * 2.0f);
			
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

				Stuff::Vector3D boxStart;
				boxStart.x = -appearType->typeUpperLeft.x;
				boxStart.y = appearType->typeUpperLeft.z;
				boxStart.z = appearType->typeUpperLeft.y;

				Stuff::Vector3D boxEnd;
				boxEnd.x = -appearType->typeLowerRight.x;
				boxEnd.y = appearType->typeLowerRight.z;
				boxEnd.z = appearType->typeLowerRight.y;
				
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

					if ((status != OBJECT_STATUS_DESTROYED) && (status != OBJECT_STATUS_DISABLED))
					{
						//-------------------------------------------------------------------------------
						//Set LOD of Model here because we have the distance and we KNOW we can see it!
						bool baseLOD = true;
						DWORD selectLOD = 0;
						if (useHighObjectDetail)
						{
							for (long i=1;i<MAX_LODS;i++)
							{
								if (appearType->bldgShape[i] && (distanceToEye > appearType->lodDistance[i]))
								{
									baseLOD = false;
									selectLOD = i;
								}
							}
						}
						else	//We always want to use the lowest LOD!!
						{
							if (appearType->bldgShape[1])
							{
								baseLOD = false;
								selectLOD = 1;
							}
						}
						
						// we are at this LOD level.
						if (selectLOD != currentLOD)
						{
							currentLOD = selectLOD;

							bldgShape->ClearAnimation();
							delete bldgShape;
							bldgShape = NULL;

							bldgShape = appearType->bldgShape[currentLOD]->CreateFrom();
							if (bdAnimationState != -1)
								appearType->setAnimation(bldgShape,bdAnimationState);

							//-------------------------------------------------
							// Load the texture and store its handle.
							for (long j=0;j<bldgShape->GetNumTextures();j++)
							{
								char txmName[1024];
								bldgShape->GetTextureName(j,txmName,256);

								char texturePath[1024];
								sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);

								FullPathFileName textureName;
								textureName.init(texturePath,txmName,"");

								if (fileExists(textureName))
								{
									if (strnicmp(txmName,"a_",2) == 0)
									{
										DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
										gosASSERT(gosTextureHandle != 0xffffffff);
										bldgShape->SetTextureHandle(j,gosTextureHandle);
										bldgShape->SetTextureAlpha(j,true);
									}
									else
									{
										DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
										gosASSERT(gosTextureHandle != 0xffffffff);
										bldgShape->SetTextureHandle(j,gosTextureHandle);
										bldgShape->SetTextureAlpha(j,false);
									}
								}
								else
								{
									//PAUSE(("Warning: %s texture name not found",textureName));
									bldgShape->SetTextureHandle(j,0xffffffff);
								}
							}
						}

						//ONLY change if we need
						if (currentLOD && baseLOD)
						{
						// we are at the Base LOD level.
							currentLOD = 0;
							
							bldgShape->ClearAnimation();
							delete bldgShape;
							bldgShape = NULL;
							
							bldgShape = appearType->bldgShape[currentLOD]->CreateFrom();
							if (bdAnimationState != -1)
								appearType->setAnimation(bldgShape,bdAnimationState);
							
							//-------------------------------------------------
							// Load the texture and store its handle.
							for (long i=0;i<bldgShape->GetNumTextures();i++)
							{
								char txmName[1024];
								bldgShape->GetTextureName(i,txmName,256);
										
								char texturePath[1024];
								sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
						
								FullPathFileName textureName;
								textureName.init(texturePath,txmName,"");
										
								if (fileExists(textureName))
								{
									if (strnicmp(txmName,"a_",2) == 0)
									{
										DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
										gosASSERT(gosTextureHandle != 0xffffffff);
										bldgShape->SetTextureHandle(i,gosTextureHandle);
										bldgShape->SetTextureAlpha(i,true);
									}
									else
									{
										DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
										gosASSERT(gosTextureHandle != 0xffffffff);
										bldgShape->SetTextureHandle(i,gosTextureHandle);
										bldgShape->SetTextureAlpha(i,false);
									}
								}
								else
								{
									//PAUSE(("Warning: %s texture name not found",textureName));
									bldgShape->SetTextureHandle(i,0xffffffff);
								}
							}
						}
					}
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
bool BldgAppearance::playDestruction (void)
{
	//Check if there is a Destruct FX
	if (appearType->destructEffect[0])
	{
		//--------------------------------------------
		// Yes, load it on up.
		unsigned flags = gosFX::Effect::ExecuteFlag;

		Check_Object(gosFX::EffectLibrary::Instance);
		gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(appearType->destructEffect);
		
		if (gosEffectSpec)
		{
			destructFX = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
			gosASSERT(destructFX != NULL);
		
			MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
		
			Stuff::Point3D			tPosition;
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			
            //Stuff::Vector3D offsetPosition;
			//offsetPosition.x = Terrain::worldUnitsPerVertex / 3.0f;
			//offsetPosition.y = -(Terrain::worldUnitsPerVertex / 3.0f);
			//offsetPosition.z = 0.0f;

			//OppRotate(offsetPosition,rotation);

			Stuff::Vector3D actualPosition = position;
			//actualPosition.Add(position,offsetPosition);

			tPosition.x = -actualPosition.x;
			tPosition.y = actualPosition.z;
			tPosition.z = actualPosition.y;

			float yaw = (180.0f + rotation) * DEGREES_TO_RADS;
			Stuff::UnitQuaternion rot;
			rot = Stuff::EulerAngles(0.0f, yaw, 0.0f);

			shapeOrigin.BuildRotation(rot);
			shapeOrigin.BuildTranslation(tPosition);
			
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
			destructFX->Start(&info);
			
			return true;
		}
		
		return false;
	}
	
	return false;		//We didn't have a destruct effect.  Tell the object to play its default.
}

//-----------------------------------------------------------------------------
long BldgAppearance::render (long depthFixup)
{
	if (inView)
	{
		long color = SD_BLUE;
		unsigned long highLight = 0x007f7f7f;
		if ((teamId > -1) && (teamId < 8)) {
			static unsigned long highLightTable[3] = {0x00007f00, 0x0000007f, 0x007f0000};
			static long colorTable[3] = {SB_GREEN | 0xff000000, SB_BLUE | 0xff000000, SB_RED| 0xff000000};
			color = colorTable[homeTeamRelationship];
			highLight = highLightTable[homeTeamRelationship];
		}

		if (selected & DRAW_COLORED)
		{
			bldgShape->SetARGBHighLight(highLight);
		}
		else
		{
			bldgShape->SetARGBHighLight(highlightColor);
		}
		
		if (drawFlash)
		{
			bldgShape->SetARGBHighLight(flashColor);
		}
		
		//---------------------------------------------
		// Call Multi-shape render stuff here.
		if (appearType->spinMe)
			bldgShape->Render(false,0.00001f);
		else if (!depthFixup)
			bldgShape->Render();
		else if (depthFixup > 0)
			bldgShape->Render(false,0.9999999f);
		else if (depthFixup < 0)
			bldgShape->Render(false,0.00001f);

		if (selected & DRAW_BARS)
		{
			if (!appearType->spinMe)
			{
				drawBars();

				//drawSelectBrackets(color);
			}
		}

		if ( selected & DRAW_TEXT )
		{
			if (objectNameId != -1)
			{
				char tmpString[255];
				cLoadString(objectNameId, tmpString, 254);

				drawTextHelp(tmpString, color);
			}
		}
		
		//------------------------------------------
		// Render GOS FX if necessary
		if (destructFX && destructFX->IsExecuted())
		{
			gosFX::Effect::DrawInfo drawInfo;
			drawInfo.m_clipper = theClipper;
			
			MidLevelRenderer::MLRState mlrState;
			mlrState.SetDitherOn();
			mlrState.SetTextureCorrectionOn();
			mlrState.SetZBufferCompareOn();
			mlrState.SetZBufferWriteOn();
	
			drawInfo.m_state = mlrState;
			drawInfo.m_clippingFlags = 0x0;

 			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::Point3D			tPosition;
			
			//Stuff::Vector3D offsetPosition;
			//offsetPosition.x = Terrain::worldUnitsPerVertex / 3.0f;
			//offsetPosition.y = -(Terrain::worldUnitsPerVertex / 3.0f);
			//offsetPosition.z = 0.0f;

			//OppRotate(offsetPosition,rotation);

			Stuff::Vector3D actualPosition = position;
			//actualPosition.Add(position,offsetPosition);

			tPosition.x = -actualPosition.x;
			tPosition.y = actualPosition.z;
			tPosition.z = actualPosition.y;

			float yaw = (180.0f + rotation) * DEGREES_TO_RADS;
			Stuff::UnitQuaternion rot;
			rot = Stuff::EulerAngles(0.0f, yaw, 0.0f);
 			shapeOrigin.BuildRotation(rot);
			shapeOrigin.BuildTranslation(tPosition);
			
			drawInfo.m_parentToWorld = &shapeOrigin;
			
			if (!MLRVertexLimitReached)
				destructFX->Draw(&drawInfo);
		}
		
		if (isActivitying)
		{
			gosFX::Effect::DrawInfo drawInfo;
			drawInfo.m_clipper = theClipper;

			MidLevelRenderer::MLRState mlrState;
			mlrState.SetDitherOn();
			mlrState.SetTextureCorrectionOn();
			mlrState.SetZBufferCompareOn();
			mlrState.SetZBufferWriteOn();
	
			drawInfo.m_state = mlrState;
			drawInfo.m_clippingFlags = 0x0;

			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;

			if (activityNodeId == -1)
				activityNodeId = bldgShape->GetNodeNameId("activity_node");
			Stuff::Vector3D dustPos = getNodeIdPosition(activityNodeId);

			if (rotationalNodeId == -1)
			{
				if (stricmp(appearType->rotationalNodeId,"NONE") != 0)
	   				rotationalNodeId = bldgShape->GetNodeNameId(appearType->rotationalNodeId);
				else
					rotationalNodeId = -2;
			}

			if (rotationalNodeId >= 0)
				dustPos = getNodeIdPosition(rotationalNodeId);
				
			Stuff::Point3D wakePos;
			wakePos.x = -dustPos.x;
			wakePos.y = dustPos.z;
			wakePos.z = dustPos.y;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(wakePos);
					
			/*
			Stuff::UnitQuaternion effectRot;
			effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
			localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
			*/

			drawInfo.m_parentToWorld = &shapeOrigin;
			if (!MLRVertexLimitReached)
				activity->Draw(&drawInfo);
			
			if (activity1)
			{
				if (activityNodeId == -1)
					activityNodeId = bldgShape->GetNodeNameId("activity_node");
				Stuff::Vector3D dustPos = getNodeIdPosition(activityNodeId);
	
				if (rotationalNodeId == -1)
				{
					if (stricmp(appearType->rotationalNodeId,"NONE") != 0)
		   				rotationalNodeId = bldgShape->GetNodeNameId(appearType->rotationalNodeId);
					else
						rotationalNodeId = -2;
				}
	
				if (rotationalNodeId >= 0)
					dustPos = getNodeIdPosition(rotationalNodeId);
					
				Stuff::Point3D wakePos;
				wakePos.x = -dustPos.x;
				wakePos.y = dustPos.z;
				wakePos.z = dustPos.y;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(wakePos);
						
				/*
				Stuff::UnitQuaternion effectRot;
				effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
				localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
				localResult.Multiply(localToWorld,shapeOrigin);
				*/
	
				drawInfo.m_parentToWorld = &shapeOrigin;
				if (!MLRVertexLimitReached)
					activity1->Draw(&drawInfo);
			}
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
long BldgAppearance::renderShadows (void)
{
	if (inView && visible && !appearType->spinMe)
	{
		//---------------------------------------------
		// Call Multi-shape render stuff here.
		if (bldgShadowShape)
			bldgShadowShape->RenderShadows();
		else
			bldgShape->RenderShadows();
	}
	return NO_ERR;
}

//-----------------------------------------------------------------------------
long BldgAppearance::update (bool animate) 
{
	Stuff::Point3D xlatPosition;
	Stuff::UnitQuaternion rot;

	//----------------------------------------
	// Recycle the weapon Nodes
	if (nodeRecycle)
	{
		for (long i=0;i<appearType->numWeaponNodes;i++)
		{
			if (nodeRecycle[i] > 0.0f)
			{
				nodeRecycle[i] -= frameLength;
				if (nodeRecycle[i] < 0.0f)
					nodeRecycle[i] = 0.0f;
			}
		}
	}

   	if (appearType->terrainLightRGB != 0xffffffff && (eye->nightFactor > 0.0f) && !forceLightsOut)
   	{
   		if (!pointLight)
   		{
   			pointLight = (TG_LightPtr)malloc(sizeof(TG_Light));
   			pointLight->init(TG_LIGHT_TERRAIN);
   			lightId = eye->addWorldLight(pointLight);
   	
   			pointLight->SetaRGB(appearType->terrainLightRGB);
   			pointLight->SetIntensity(appearType->terrainLightIntensity);
   			pointLight->SetFalloffDistances(appearType->terrainLightInnerRadius, appearType->terrainLightOuterRadius);
   		}
		
		if (pointLight)
		{
			Stuff::Point3D ourPosition;
			ourPosition.x = -position.x;
			ourPosition.y = position.z;
			ourPosition.z = position.y;
	
			pointLight->direction = ourPosition;
	
			Stuff::LinearMatrix4D lightToWorldMatrix;
			lightToWorldMatrix.BuildTranslation(ourPosition);
			lightToWorldMatrix.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			pointLight->SetLightToWorld(&lightToWorldMatrix);
			pointLight->SetPosition(&position);
			pointLight->SetIntensity(appearType->terrainLightIntensity * eye->getNightFactor());
		}
   	}
	else
	{
		//Turn the lights off!
		//Need to kill the light source here too!
		if (pointLight)
		{
			eye->removeWorldLight(lightId,pointLight);
			free(pointLight);
			pointLight = NULL;
		}
	}

	if (forceLightsOut)
		bldgShape->SetLightsOut(true);
		
	//Update flashing regardless of view!!!
	if (duration > 0.0f)
	{
		duration -= frameLength;
		currentFlash -= frameLength;
		if (currentFlash < 0.0f)
		{
			drawFlash ^= true;
			currentFlash = flashDuration;
		}
	}
	else
	{
		drawFlash = false;
	}

	if (inView)
	{
		if (appearType->spinMe)
			rotation += SPINRATE * frameLength;
		
 		if (rotation > 180)
			rotation -= 360;
	
		if (rotation < -180)
			rotation += 360;
	
		//-------------------------------------------
		// Does math necessary to draw Tree
		float yaw = rotation * DEGREES_TO_RADS;
		rot = Stuff::EulerAngles(0.0f, yaw, 0.0f);
	
		if (appearType->spinMe && land)
		{
			//Make sure we are above the water level
			if (position.z < Terrain::waterElevation)
				position.z = Terrain::waterElevation;
		}

		xlatPosition.x = -position.x;
		xlatPosition.y = position.z;
		xlatPosition.z = position.y;

		if (!fogLightSet)
		{
			unsigned char lightr,lightg,lightb;
			float lightIntensity = 1.0f;
			if (land)
				lightIntensity = land->getTerrainLight(position);

			lightr = eye->getLightRed(lightIntensity);
			lightg = eye->getLightGreen(lightIntensity);
			lightb = eye->getLightBlue(lightIntensity);

			lightRGB = (lightr<<16) + (lightg<<8) + lightb;

			fogRGB = 0xff<<24;
			float fogStart = eye->fogStart;
			float fogFull = eye->fogFull;

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

			fogLightSet = true;
		}
	
		eye->setLightColor(0,lightRGB);
		eye->setLightIntensity(0,1.0);

		if (useFog)
			bldgShape->SetFogRGB(fogRGB);
		else
			bldgShape->SetFogRGB(0xffffffff);
	
		Stuff::UnitQuaternion turretRot;
		turretRot = Stuff::EulerAngles((turretPitch * DEGREES_TO_RADS),(turretYaw * DEGREES_TO_RADS),0.0f);
		if (rotationalNodeId == -1)
	   		rotationalNodeId = bldgShape->SetNodeRotation(appearType->rotationalNodeId,&turretRot);
   
	   	bldgShape->SetNodeRotation(rotationalNodeId,&turretRot);
	}

	float oldFrame = currentFrame;
	if (animate && bdFrameRate != 0.0f)
	{
		//--------------------------------------------------------
		// Make sure animation runs no faster than bdFrameRate fps.
		float frameInc = bdFrameRate * frameLength;
		
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
			if (currentFrame >= appearType->getNumFrames(bdAnimationState))
			{
				if (isLooping)
					currentFrame -= appearType->getNumFrames(bdAnimationState);
				else
					currentFrame = appearType->getNumFrames(bdAnimationState) - 1;
					
				canTransition = true;		//Whenever we have completed one cycle or at last frame, OK to move on!
			}
			
	
			//--------------------------------------
			//Check negative overflow of gesture
			if (currentFrame < 0)
			{
				if (isLooping)
					currentFrame += appearType->getNumFrames(bdAnimationState); 
				else
					currentFrame = 0.0f; 
					
				canTransition = true;		//Whenever we have completed one cycle or at last frame, OK to move on!
			}
		}
		
		bldgShape->SetFrameNum(currentFrame);
	}

	if (inView)
	{
		bool checkShadows = ((!beenInView) || (appearType->spinMe) || (eye->forceShadowRecalc) || (currentFrame != oldFrame));
		if (bldgShadowShape)
			bldgShape->SetUseShadow(false);
		else
			bldgShape->SetRecalcShadows(checkShadows);
			
		bldgShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
		bldgShape->TransformMultiShape (&xlatPosition,&rot);
		
		if (bldgShadowShape && useShadows)
		{
			bldgShadowShape->SetRecalcShadows(checkShadows);
			bldgShadowShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
			bldgShadowShape->TransformMultiShape (&xlatPosition,&rot);
		}
 		
		if ((turn > 3) && useShadows)
			beenInView = true;
			
		//------------------------------------------------
		// Update GOSFX
		if (destructFX && destructFX->IsExecuted())
		{
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D 	localToWorld;
			Stuff::Point3D			tPosition;
				
			//Stuff::Vector3D offsetPosition;
			//offsetPosition.x = Terrain::worldUnitsPerVertex / 3.0f;
			//offsetPosition.y = -(Terrain::worldUnitsPerVertex / 3.0f);
			//offsetPosition.z = 0.0f;

			//OppRotate(offsetPosition,rotation);

			Stuff::Vector3D actualPosition = position;
			//actualPosition.Add(position,offsetPosition);

			tPosition.x = -actualPosition.x;
			tPosition.y = actualPosition.z;
			tPosition.z = actualPosition.y;

			float yaw = (180.0f + rotation) * DEGREES_TO_RADS;
			Stuff::UnitQuaternion rot;
			rot = Stuff::EulerAngles(0.0f, yaw, 0.0f);
 			shapeOrigin.BuildRotation(rot);
			shapeOrigin.BuildTranslation(tPosition);

	 		Stuff::OBB boundingBox;
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
	
			bool result = destructFX->Execute(&info);
			if (!result)
			{
				destructFX->Kill();
				delete destructFX;
				destructFX = NULL;
			}
		}
		
		if (isActivitying)
		{
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;
					
			if (activityNodeId == -1)
				activityNodeId = bldgShape->GetNodeNameId("activity_node");
			Stuff::Vector3D dustPos = getNodeIdPosition(activityNodeId);

			if (rotationalNodeId == -1)
			{
				if (stricmp(appearType->rotationalNodeId,"NONE") != 0)
	   				rotationalNodeId = bldgShape->GetNodeNameId(appearType->rotationalNodeId);
				else
					rotationalNodeId = -2;
			}

			if (rotationalNodeId >= 0)
				dustPos = getNodeIdPosition(rotationalNodeId);
 			
			Stuff::Point3D wakePos;
			wakePos.x = -dustPos.x;
			wakePos.y = dustPos.z;
			wakePos.z = dustPos.y;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(wakePos);
					
			/*
			Stuff::UnitQuaternion effectRot;
			effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
			localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
			*/
			
			Stuff::OBB boundingBox;
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
			
			activity->Execute(&info);
			
			if (activity1)
			{
				if (activityNodeId == -1)
					activityNodeId = bldgShape->GetNodeNameId("activity_node");
				Stuff::Vector3D dustPos = getNodeIdPosition(activityNodeId);
	
				if (rotationalNodeId == -1)
				{
					if (stricmp(appearType->rotationalNodeId,"NONE") != 0)
		   				rotationalNodeId = bldgShape->GetNodeNameId(appearType->rotationalNodeId);
					else
						rotationalNodeId = -2;
				}
	
				if (rotationalNodeId >= 0)
					dustPos = getNodeIdPosition(rotationalNodeId);
				
				Stuff::Point3D wakePos;
				wakePos.x = -dustPos.x;
				wakePos.y = dustPos.z;
				wakePos.z = dustPos.y;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(wakePos);
						
				/*
				Stuff::UnitQuaternion effectRot;
				effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
				localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
				localResult.Multiply(localToWorld,shapeOrigin);
				*/
				
				Stuff::OBB boundingBox;
				gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
				
				activity1->Execute(&info);
			}
		}
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------
void BldgAppearance::startActivity (long effectId, bool loop)
{
	//Check if we are already playing one.  If not, be active!
	
	//First, check if its even loaded.
	// can easily preload this.  Should we?  NO.  We don't know what will be passed in.
	if (!activity && useNonWeaponEffects)
	{
   		if (strcmp(weaponEffects->GetEffectName(effectId),"NONE") != 0)
   		{
			//--------------------------------------------
			// Yes, load it on up.
			unsigned flags = gosFX::Effect::ExecuteFlag|gosFX::Effect::LoopFlag;
			if (!loop)
				flags = gosFX::Effect::ExecuteFlag;

			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(effectId));
			
			if (gosEffectSpec)
			{
				activity = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
				gosASSERT(activity != NULL);
				
				Stuff::Vector3D testPos = getNodeNamePosition("activity_node1");
				if (testPos != position)
				{
					activity1 = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
					gosASSERT(activity != NULL);
				}

  				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			}
		}
	}
	
	if (!isActivitying && activity)		//Start the effect if we are not running it yet!!
	{
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D	localToWorld;
		Stuff::LinearMatrix4D	localResult;
		
		if (activityNodeId == -1)
   			activityNodeId = bldgShape->GetNodeNameId("activity_node");
   		Stuff::Vector3D nodePos = getNodeIdPosition(activityNodeId);

   		if (rotationalNodeId == -1)
   		{
   			if (stricmp(appearType->rotationalNodeId,"NONE") != 0)
      				rotationalNodeId = bldgShape->GetNodeNameId(appearType->rotationalNodeId);
   			else
   				rotationalNodeId = -2;
   		}

   		if (rotationalNodeId >= 0)
   			nodePos = getNodeIdPosition(rotationalNodeId);

 		Stuff::Point3D wakePos;
		wakePos.x = -nodePos.x;
		wakePos.y = nodePos.z;	//Wake is at Water Level!
		wakePos.z = nodePos.y;
		
 		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(wakePos);
				
		/*
		Stuff::UnitQuaternion effectRot;
		effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
		localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
		localResult.Multiply(localToWorld,shapeOrigin);
		*/
			
 		gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);

		activity->Start(&info);
		
		if (activity1)
		{
			if (activityNode1Id == -1)
				activityNode1Id = bldgShape->GetNodeNameId("activity_node1");
			Stuff::Vector3D nodePos = getNodeIdPosition(activityNode1Id);

			if (rotationalNodeId == -1)
			{
				if (stricmp(appearType->rotationalNodeId,"NONE") != 0)
	   				rotationalNodeId = bldgShape->GetNodeNameId(appearType->rotationalNodeId);
				else
					rotationalNodeId = -2;
			}

			if (rotationalNodeId >= 0)
				nodePos = getNodeIdPosition(rotationalNodeId);
			
 			Stuff::Point3D wakePos;
			wakePos.x = -nodePos.x;
			wakePos.y = nodePos.z;	//Wake is at Water Level!
			wakePos.z = nodePos.y;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(wakePos);
					
			/*
			Stuff::UnitQuaternion effectRot;
			effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
			localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
			*/
				
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
	
			activity1->Start(&info);
		}
		
		isActivitying = true;
	}
}

//-----------------------------------------------------------------------------
void BldgAppearance::stopActivity (void)
{
	if (isActivitying)		//Stop the effect if we are running it!!
	{
		activity->Kill();
		if (activity1)
			activity1->Kill();
	}
	
	isActivitying = false;
}

//-----------------------------------------------------------------------------
void BldgAppearance::flashBuilding (float dur, float fDuration, DWORD color)
{
	duration = dur;
	flashDuration = fDuration;
	flashColor = color;
	drawFlash = true;
	currentFlash = flashDuration;
}

//-----------------------------------------------------------------------------
void BldgAppearance::destroy (void)
{
	if ( bldgShape )
	{
		delete bldgShape;
		bldgShape = NULL;
	}

	if (bldgShadowShape)
	{
		delete bldgShadowShape;
		bldgShadowShape = NULL;
	}

	if (destructFX)
	{
		destructFX->Kill();
		delete destructFX;
		destructFX = NULL;
	}
	
	//Turn the lights off!
	//Need to kill the light source here too!
	if (pointLight)
	{
		if (eye)
			eye->removeWorldLight(lightId,pointLight);

		free(pointLight);
		pointLight = NULL;
	}

	appearanceTypeList->removeAppearance(appearType);
}

#define HEIGHT_THRESHOLD 10.0f

//-----------------------------------------------------------------------------

long BldgAppearance::calcCellsCovered (Stuff::Vector3D& pos, short* cellList) {

	gosASSERT((Terrain::realVerticesMapSide * MAPCELL_DIM) == GameMap->width);
	long numCoords = 0;
	long maxCoords = cellList[0];

	//MUST force building to HIGHEST LOD!!!  IMpassability data is only valid at this LOD!!
	// Building will reset its LOD on next draw!!
	if (currentLOD)
	{
		currentLOD = 0;
	
		bldgShape->ClearAnimation();
		delete bldgShape;
		bldgShape = NULL;
	
		bldgShape = appearType->bldgShape[currentLOD]->CreateFrom();
		if (bdAnimationState != -1)
			appearType->setAnimation(bldgShape,bdAnimationState);
	}

	//-------------------------------------------------------------
	// New way.  For each vertex in each shape, translate to world
	for (int i=0;i<bldgShape->GetNumShapes();i++)
	{
		//Check if the artists meant for this piece to NOT block passability!!
		if (strnicmp(bldgShape->GetNodeId(i),"_PAB",4) != 0)
		{
			for (int j=0;j<bldgShape->GetNumVerticesInShape(i);j++) 
			{
				Stuff::Vector3D vertexPos, worldPos;
				vertexPos = bldgShape->GetShapeVertexInEditor(i,j,-rotation);
				worldPos.Add(pos,vertexPos);
	
				bool recordCell = false;
				if (appearType->isForestClump)
					recordCell = (vertexPos.z <= 1.0f);
				else
					recordCell = (vertexPos.z >= 1.0f);
				if (recordCell) 
				{
					int cellR, cellC;
					land->worldToCell(worldPos,cellR,cellC);
					if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR)
						|| (0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
					{
						//gosASSERT(false);
						continue;
					}
	//				if (GameMap->inBounds(cellR, cellC)) {
						//-------------------
						// Record the cell...
						if (numCoords > (maxCoords - 2))
							Fatal(numCoords, "BldgAppearance.markMoveMap: too many coords for cellList ");
							
						cellList[numCoords++] = (short)cellR;
						cellList[numCoords++] = (short)cellC;
	//				}
				}
			}
		}
	}
	
	return(numCoords);
}

//-----------------------------------------------------------------------------

void BldgAppearance::markTerrain (_ScenarioMapCellInfo* pInfo, int type, int counter)
{
	if (appearType->spinMe)			//We are a marker
		return;						//Do not mark impassable
		
	//MUST force building to HIGHEST LOD!!!  IMpassability data is only valid at this LOD!!
	// Building will reset its LOD on next draw!!
	if (currentLOD)
	{
		currentLOD = 0;
	
		bldgShape->ClearAnimation();
		delete bldgShape;
		bldgShape = NULL;
	
		bldgShape = appearType->bldgShape[currentLOD]->CreateFrom();
		if (bdAnimationState != -1)
			appearType->setAnimation(bldgShape,bdAnimationState);
	}

	int cellR, cellC;
	land->worldToCell(position, cellR, cellC);
	if (appearType->isForestClump)
	{
		//-------------------------------------------------------------
		// New way.  For each vertex in each shape, translate to world
		for (int i=0;i<bldgShape->GetNumShapes();i++)
		{
			//Check if the artists meant for this piece to NOT block passability!!
			if (strnicmp(bldgShape->GetNodeId(i),"_PAB",4) != 0)
			{
				for (int j=0;j<bldgShape->GetNumVerticesInShape(i);j++)
				{
					Stuff::Vector3D vertexPos, worldPos;
					vertexPos = bldgShape->GetShapeVertexInEditor(i,j,-rotation);
					worldPos.Add(position,vertexPos);
		
					int cellR, cellC;
					land->worldToCell(worldPos,cellR,cellC);
					if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
						(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
					{
						continue;
					}
					
					_ScenarioMapCellInfo*pTmp = &(pInfo[cellR * Terrain::realVerticesMapSide * MAPCELL_DIM + cellC]);
	
					if (vertexPos.z <= 1.0f)
					{
						pTmp->passable = true;
						pTmp->gate = false;
						pTmp->forest = true;
						//pTmp->specialType = type;
						//pTmp->specialID = counter;
					}
					
					float cellLocalHeight = vertexPos.z * metersPerWorldUnit * 0.25f;
					if (cellLocalHeight > 15.0f)
						cellLocalHeight = 15.0f;
						
					//ONLY mark LOS on cells that are impassable with forests.  Maybe everything?
					if (pTmp->passable && (pTmp->lineOfSight < cellLocalHeight))
						pTmp->lineOfSight = cellLocalHeight+0.5f;
				}
			}
		}
	}
	else
	{
		if ((type == SPECIAL_GATE) || (type == SPECIAL_WALL))
		{
			if (appearType->bldgShape[0])
			{
				bldgShape->ClearAnimation();
				delete bldgShape;
				bldgShape = NULL;
					
				bldgShape = appearType->bldgShape[0]->CreateFrom();
				if (bdAnimationState != -1)
					appearType->setAnimation(bldgShape,bdAnimationState);
			}
		}

		if (type == SPECIAL_LAND_BRIDGE)
		{
			if (appearType->bldgDmgShape)
			{
				bldgShape->ClearAnimation();
				delete bldgShape;
				bldgShape = NULL;
					
				bldgShape = appearType->bldgDmgShape->CreateFrom();
				if (bdAnimationState != -1)
					appearType->setAnimation(bldgShape,bdAnimationState);
			}
		}

		//-------------------------------------------------------------
		// New way.  For each vertex in each shape, translate to world
		for (int i=0;i<bldgShape->GetNumShapes();i++)
		{
			//Check if the artists meant for this piece to NOT block passability!!
			if (strnicmp(bldgShape->GetNodeId(i),"_PAB",4) != 0)
			{
				for (int j=0;j<bldgShape->GetNumVerticesInShape(i);j++)
				{
					Stuff::Vector3D vertexPos, worldPos;
					vertexPos = bldgShape->GetShapeVertexInEditor(i,j,-rotation);
					worldPos.Add(position,vertexPos);
		
					int cellR, cellC;
					land->worldToCell(worldPos,cellR,cellC);
					if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
						(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
					{
						continue;
					}
					_ScenarioMapCellInfo*pTmp = &(pInfo[cellR * Terrain::realVerticesMapSide * MAPCELL_DIM + cellC]);
	
					if (vertexPos.z >= 1.0f)
					{
						pTmp->passable = false;
						
						if (((type == SPECIAL_GATE) || (type == SPECIAL_WALL)))
						{
							pTmp->passable = true;
							pTmp->specialID = counter;
							pTmp->specialType = type;
							if (type == SPECIAL_GATE)
								pTmp->gate = true;
						}
						else if (type == SPECIAL_LAND_BRIDGE)
						{
							pTmp->passable = true;
							pTmp->specialID = counter;
							pTmp->specialType = type;
						}
						else if (type == 18)
						{
							pTmp->specialID = 0;
							pTmp->specialType = SPECIAL_NONE;
							pTmp->passable = true;
						}
						else
						{
							pTmp->specialID = 0;
							pTmp->specialType = SPECIAL_NONE;
						}
							
						if (type != 18)
						{
							float cellLocalHeight = vertexPos.z * metersPerWorldUnit * 0.25f;
							if (cellLocalHeight > 15.0f)
								cellLocalHeight = 15.0f;
								
							if (pTmp->lineOfSight < cellLocalHeight)
								pTmp->lineOfSight = cellLocalHeight+0.5f;
						}
					}
				}
			}
		}
		
 		//Switch to destroyed state to mark impassable.  The destroyed impassability will NEVER change!!
		// When a gate opens or a wall or gate is destroyed, we only want to mark stuff that is going 
		// away passable and long range capable.
		if ((type == SPECIAL_GATE) || (type == SPECIAL_WALL))
		{
			if (appearType->bldgDmgShape)
			{
				bldgShape->ClearAnimation();
				delete bldgShape;
				bldgShape = NULL;
					
				bldgShape = appearType->bldgDmgShape->CreateFrom();
				if (bdAnimationState != -1)
					appearType->setAnimation(bldgShape,bdAnimationState);
			}
		}

		if (type == SPECIAL_LAND_BRIDGE)
		{
			if (appearType->bldgShape[0])
			{
				bldgShape->ClearAnimation();
				delete bldgShape;
				bldgShape = NULL;
					
				bldgShape = appearType->bldgShape[0]->CreateFrom();
				if (bdAnimationState != -1)
					appearType->setAnimation(bldgShape,bdAnimationState);
			}
		}

			
		//-------------------------------------------------------------
		// New way.  For each vertex in each shape, translate to world
		for (int i=0;i<bldgShape->GetNumShapes();i++)
		{
			//Check if the artists meant for this piece to NOT block passability!!
			if (strnicmp(bldgShape->GetNodeId(i),"_PAB",4) != 0)
			{
				for (int j=0;j<bldgShape->GetNumVerticesInShape(i);j++)
				{
					Stuff::Vector3D vertexPos, worldPos;
					vertexPos = bldgShape->GetShapeVertexInEditor(i,j,-rotation);
					worldPos.Add(position,vertexPos);
		
					int cellR, cellC;
					land->worldToCell(worldPos,cellR,cellC);
					if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
						(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
					{
						continue;
					}
					_ScenarioMapCellInfo*pTmp = &(pInfo[cellR * Terrain::realVerticesMapSide * MAPCELL_DIM + cellC]);
	
					if (vertexPos.z >= 1.0f)
					{
						if (type == 18)
						{
							pTmp->passable = true;
							pTmp->specialID = 0;
							pTmp->specialType = SPECIAL_NONE;
						}
						else
						{
							pTmp->passable = false;
							pTmp->gate = false;			//Perfectly OK to mark these again,  They are no longer special!!
							pTmp->specialID = 0;
							pTmp->specialType = SPECIAL_NONE;
						}
					}
				}
			}
		}
			
		if ((status != OBJECT_STATUS_DESTROYED) && appearType->bldgShape[0])
		{
			bldgShape->ClearAnimation();
			delete bldgShape;
			bldgShape = NULL;
						
			bldgShape = appearType->bldgShape[0]->CreateFrom();
			if (bdAnimationState != -1)
				appearType->setAnimation(bldgShape,bdAnimationState);
		}
		else if ((status == OBJECT_STATUS_DESTROYED) && appearType->bldgDmgShape)
		{
			bldgShape->ClearAnimation();
			delete bldgShape;
			bldgShape = NULL;
					
			bldgShape = appearType->bldgDmgShape->CreateFrom();
			if (bdAnimationState != -1)
				appearType->setAnimation(bldgShape,bdAnimationState);
		}
	}
}

//-----------------------------------------------------------------------------

long BldgAppearance::markMoveMap (bool passable, long* lineOfSightRect, bool useHeight, short* cellList)
{
	int minRow = 9999;
	int maxRow = 0;
	int minCol = 9999;
	int maxCol = 0;

	//MUST force building to HIGHEST LOD!!!  IMpassability data is only valid at this LOD!!
	// Building will reset its LOD on next draw!!
	TG_MultiShapePtr tempBldgShape = bldgShape;

	if (currentLOD)
	{
		tempBldgShape = appearType->bldgShape[currentLOD]->CreateFrom();
		if (bdAnimationState != -1)
			appearType->setAnimation(tempBldgShape,bdAnimationState);
	}

	int numCoords = 0;
	if (cellList) {
		gosASSERT(!useHeight);
		//----------------------------------------------------------------------------------
		// Store the max number of coords allowed in the first cell. Can overwrite it now...
		int maxCoords = cellList[0];
		//-------------------------------------------------------------
		// New way.  For each vertex in each shape, translate to world
		for (int i = 0; i < tempBldgShape->GetNumShapes(); i++) 
		{
			//Check if the artists meant for this piece to NOT block passability!!
			if (strnicmp(tempBldgShape->GetNodeId(i),"_PAB",4) != 0)
			{
				for (int j=0;j<tempBldgShape->GetNumVerticesInShape(i);j++) 
				{
					Stuff::Vector3D vertexPos, worldPos;
					vertexPos = tempBldgShape->GetShapeVertexInWorld(i,j,-rotation);
					worldPos.Add(position,vertexPos);
	
					int cellR, cellC;
					land->worldToCell(worldPos,cellR,cellC);
					if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
						(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
					{
						continue;
					}
					
					//----------------------------
					// Building lineOfSightRect...
					if (cellR < minRow)
						minRow = cellR;
					if (cellR > maxRow)
						maxRow = cellR;
					if (cellC < minCol)
						minCol = cellC;
					if (cellC > maxCol)
						maxCol = cellC;
						
					//-------------------
					// Record the cell...
					if (numCoords > (maxCoords - 2))
						Fatal(numCoords, "BldgAppearance.markMoveMap: too many coords for cellList ");
					cellList[numCoords++] = (short)cellR;
					cellList[numCoords++] = (short)cellC;
				}
			}
		}
	}
	else 
	{
		//-------------------------------------------------------------
		// New way.  For each vertex in each shape, translate to world
		for (int i=0;i<tempBldgShape->GetNumShapes();i++)
		{
			//Check if the artists meant for this piece to NOT block passability!!
			if (strnicmp(tempBldgShape->GetNodeId(i),"_PAB",4) != 0)
			{
				for (int j=0;j<tempBldgShape->GetNumVerticesInShape(i);j++)
				{
					Stuff::Vector3D vertexPos, worldPos;
					vertexPos = tempBldgShape->GetShapeVertexInWorld(i,j,-rotation);
					worldPos.Add(position,vertexPos);
	
					int cellR, cellC;
					land->worldToCell(worldPos,cellR,cellC);
					if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
						(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
					{
						continue;
					}
					
					//----------------------------
					// Building lineOfSightRect...
					if (cellR < minRow)
						minRow = cellR;
					if (cellR > maxRow)
						maxRow = cellR;
					if (cellC < minCol)
						minCol = cellC;
					if (cellC > maxCol)
						maxCol = cellC;
						
					//----------------
					// Mark the map...
					MapCellPtr curCell = GameMap->getCell(cellR, cellC);
					if (appearType->isForestClump) {
						if (vertexPos.z <= 1.0f)
							curCell->setPassable(passable);
						}
					else {
						if (vertexPos.z >= 1.0f)
							curCell->setPassable(passable);
					}
				}
			}
		}
	}
	
	if (lineOfSightRect) {
		lineOfSightRect[0] = minRow;
		lineOfSightRect[1] = minCol;
		lineOfSightRect[2] = maxRow;
		lineOfSightRect[3] = maxCol;
	}

	if (tempBldgShape != bldgShape)
	{
		tempBldgShape->ClearAnimation();
		delete tempBldgShape;
		tempBldgShape = NULL;
	}

	return(numCoords/2);
}

//-----------------------------------------------------------------------------

void BldgAppearance::markLOS (bool clearIt)
{
	//MUST force building to HIGHEST LOD!!!  IMpassability data is only valid at this LOD!!
	// Building will reset its LOD on next draw!!
	TG_MultiShapePtr tempBldgShape = bldgShape;
	if (currentLOD)
	{
		tempBldgShape = appearType->bldgShape[0]->CreateFrom();
		if (bdAnimationState != -1)
			appearType->setAnimation(tempBldgShape,bdAnimationState);
	}

	//-------------------------------------------------------------
	// New way.  For each vertex in each shape, translate to world
	for (int i=0;i<tempBldgShape->GetNumShapes();i++)
	{
		//Check if the artists meant for this piece to NOT block LOS!!
		// Probably should check for light cones,too!
		
		if ((strnicmp(tempBldgShape->GetNodeId(i),"LOS_",4) != 0) &&
			(strnicmp(tempBldgShape->GetNodeId(i),"SpotLight_",10) != 0))
		{
			for (int j=0;j<tempBldgShape->GetNumVerticesInShape(i);j++)
			{
				Stuff::Vector3D vertexPos, worldPos;
				vertexPos = tempBldgShape->GetShapeVertexInEditor(i,j,-rotation);
//				vertexPos = tempBldgShape->GetShapeVertexInWorld(i,j,-rotation);
				worldPos.Add(position,vertexPos);
	
				int cellR, cellC;
				land->worldToCell(worldPos,cellR,cellC);
				if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
					(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
				{
					continue;
				}
				
				//----------------
				// Mark the map...
				MapCellPtr curCell = GameMap->getCell(cellR, cellC);

				if (!clearIt)
				{
					float currentCellHeight = curCell->getLocalHeight();
					
					float cellLocalHeight = vertexPos.z * metersPerWorldUnit * 0.25f;
					if (cellLocalHeight > 15.0f)
						cellLocalHeight = 15.0f;

					if (cellLocalHeight > currentCellHeight)
						curCell->setLocalHeight(cellLocalHeight+0.5f);
				}
				else	//We want to clear all LOS height INFO.  We're about to change shape!!
				{
					curCell->setLocalHeight(0.0f);
				}
			}
		}
	}

	if (tempBldgShape != bldgShape)
	{
		tempBldgShape->ClearAnimation();
		delete tempBldgShape;
		tempBldgShape = NULL;
	}
}

//-----------------------------------------------------------------------------

void BldgAppearance::calcAdjCell (long& row, long& col)
{
	//MUST force building to HIGHEST LOD!!!  IMpassability data is only valid at this LOD!!
	// Building will reset its LOD on next draw!!
	if (currentLOD)
	{
		currentLOD = 0;
	
		bldgShape->ClearAnimation();
		delete bldgShape;
		bldgShape = NULL;
	
		bldgShape = appearType->bldgShape[currentLOD]->CreateFrom();
		if (bdAnimationState != -1)
			appearType->setAnimation(bldgShape,bdAnimationState);
	}

	//-------------------------------------------------------------
	// New way.  For each vertex in each shape, translate to world
	int numVert = 0;
	for (int i=0;i<bldgShape->GetNumShapes();i++)
	{
		for (int j=0;j<bldgShape->GetNumVerticesInShape(i);j++)
		{
			Stuff::Vector3D vertexPos, worldPos;
			vertexPos = bldgShape->GetShapeVertexInWorld(i,j,-rotation);
			worldPos.Add(position,vertexPos);

			{
				numVert++;
				int cellR, cellC;
				land->worldToCell(worldPos,cellR,cellC);
				
				//MapCellPtr curCell = GameMap->getCell(cellR, cellC);
				//curCell->setPassable(passable);	
			}
		}
	}
}

//-----------------------------------------------------------------------------
// class TreeAppearanceType
void TreeAppearanceType::init (const char * fileName)
{
	AppearanceType::init(fileName);

	FullPathFileName iniName;
	iniName.init(tglPath,fileName,".ini");

	FitIniFile iniFile;
	
	long result = iniFile.open(iniName);
	if (result != NO_ERR)
		Fatal(result,"Could not find building appearance INI file");

	result = iniFile.seekBlock("TGLData");
	if (result != NO_ERR)
		Fatal(result,"Could not find block in building appearance INI file");

	result = iniFile.readIdBoolean("ForestClump",isForestClump);
	if (result != NO_ERR)
		isForestClump = false;
		
 	char aseFileName[512];
	result = iniFile.readIdString("FileName",aseFileName,511);
	if (result != NO_ERR)
	{
		//Check for LOD filenames instead
		for (long i=0;i<MAX_LODS;i++)
		{
			char baseName[256];
			char baseLODDist[256];
			sprintf(baseName,"FileName%d",i);
			sprintf(baseLODDist,"Distance%d",i);
			
			result = iniFile.readIdString(baseName,aseFileName,511);
			if (result == NO_ERR)
			{
				result = iniFile.readIdFloat(baseLODDist,lodDistance[i]);
				if (result != NO_ERR)
					STOP(("LOD %d has no distance value in file %s",i,fileName));
					
				//----------------------------------------------
				// Base LOD shape.  In stand Pose by default.
				treeShape[i] = new TG_TypeMultiShape;
				gosASSERT(treeShape[i] != NULL);
			
				FullPathFileName treeName;
				treeName.init(tglPath,aseFileName,".ase");
			
				treeShape[i]->LoadTGMultiShapeFromASE(treeName);
				
				//---------------------------------------------------------
				// Should only be necessary for trees.  Easy to data drive
				treeShape[i]->SetAlphaTest(true);
				treeShape[i]->SetFilter(true);
			}
			else if (!i)
			{
				STOP(("No base LOD for shape %s",fileName));
			}
		}
	}
	else
	{
		//----------------------------------------------
		// Base shape.  In stand Pose by default.
		treeShape[0] = new TG_TypeMultiShape;
		gosASSERT(treeShape[0] != NULL);
	
		FullPathFileName treeName;
		treeName.init(tglPath,aseFileName,".ase");
	
		treeShape[0]->LoadTGMultiShapeFromASE(treeName);
		
		//---------------------------------------------------------
		// Should only be necessary for trees.  Easy to data drive
		treeShape[0]->SetAlphaTest(true);
		treeShape[0]->SetFilter(true);
	}

	result = iniFile.readIdString("ShadowName",aseFileName,511);
	if (result == NO_ERR)
	{
		//----------------------------------------------
		// Base Shadow shape.
		treeShadowShape = new TG_TypeMultiShape;
		gosASSERT(treeShadowShape != NULL);
	
		FullPathFileName treeName;
		treeName.init(tglPath,aseFileName,".ase");
	
		treeShadowShape->LoadTGMultiShapeFromASE(treeName);
		
		//---------------------------------------------------------
		// Should only be necessary for trees.  Easy to data drive
		treeShadowShape->SetAlphaTest(true);
		treeShadowShape->SetFilter(true);
	}
	
	result = iniFile.seekBlock("TGLDamage");
	if (result == NO_ERR)
	{
		result = iniFile.readIdString("FileName",aseFileName,511);
		if (result != NO_ERR)
			Fatal(result,"Could not find ASE FileName in building appearance INI file");
	
		FullPathFileName dmgName;
		dmgName.init(tglPath,aseFileName,".ase");
	
		treeDmgShape = new TG_TypeMultiShape;
		gosASSERT(treeDmgShape != NULL);
		treeDmgShape->LoadTGMultiShapeFromASE(dmgName);

		if (!treeDmgShape->GetNumShapes())
		{
			delete treeDmgShape;
			treeDmgShape = NULL;
		}
		
		//Shadow for destroyed state.
		result = iniFile.readIdString("ShadowName",aseFileName,511);
		if (result == NO_ERR)
		{
			//----------------------------------------------
			// Base Shadow shape.
			treeDmgShadowShape = new TG_TypeMultiShape;
			gosASSERT(treeDmgShadowShape != NULL);
		
			FullPathFileName treeName;
			treeName.init(tglPath,aseFileName,".ase");
		
			treeDmgShadowShape->LoadTGMultiShapeFromASE(treeName);
			if (!treeDmgShadowShape->GetNumShapes())
			{
				delete treeDmgShadowShape;
				treeDmgShadowShape = NULL;
			}
		}
	}
	else
	{
		treeDmgShape = NULL;
		treeDmgShadowShape = NULL;
	}

 	//No Animations at present.
}

//----------------------------------------------------------------------------
void TreeAppearanceType::destroy (void)
{
	AppearanceType::destroy();

	for (long i=0;i<MAX_LODS;i++)
	{
		if (treeShape[i])
		{
			delete treeShape[i];
			treeShape[i] = NULL;
		}
	}

	if (treeDmgShape)
	{
		delete treeDmgShape;
		treeDmgShape = NULL;
	}
	
	if (treeDmgShadowShape)
	{
		delete treeDmgShadowShape;
		treeDmgShadowShape = NULL;
	}
	
 	if (treeShadowShape)
	{
		delete treeShadowShape;
		treeShadowShape = NULL;
	}
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class TreeAppearance
void TreeAppearance::init (AppearanceTypePtr tree, GameObjectPtr obj)
{
	Appearance::init(tree,obj);
	appearType = (TreeAppearanceType *)tree;

	shapeMin.x = shapeMin.y = -25;
	shapeMax.x = shapeMax.y = 50;
	
	paintScheme = -1;
	objectNameId = 30862;
	
	hazeFactor = 0.0f;

	screenPos.x = screenPos.y = screenPos.z = screenPos.w = -999.0f;
	position.Zero();
	rotation = 0.0;;
	selected = 0;
	teamId = 0;
	homeTeamRelationship = 0;
	actualRotation = rotation;

	OBBRadius = -1.0f;

	currentLOD = 0;
	
	beenInView = false;
	
	fogLightSet = false;
	lightRGB = fogRGB = 0xffffffff;

	if (appearType)
	{
		treeShape = appearType->treeShape[0]->CreateFrom();

		//-------------------------------------------------
		// Load the texture and store its handle.
		for (long i=0;i<treeShape->GetNumTextures();i++)
		{
			char txmName[1024];
			treeShape->GetTextureName(i,txmName,256);
	
			char texturePath[1024];
			sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
	
			FullPathFileName textureName;
			textureName.init(texturePath,txmName,"");
	
			if (fileExists(textureName))
			{
				if (strnicmp(txmName,"a_",2) == 0)
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
					gosASSERT(gosTextureHandle != 0xffffffff);
					treeShape->SetTextureHandle(i,gosTextureHandle);
					treeShape->SetTextureAlpha(i,true);
				}
				else
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
					gosASSERT(gosTextureHandle != 0xffffffff);
					treeShape->SetTextureHandle(i,gosTextureHandle);
					treeShape->SetTextureAlpha(i,false);
				}
			}
			else
			{
				//PAUSE(("Warning: %s texture name not found",textureName));
				treeShape->SetTextureHandle(i,0xffffffff);
			}
		}
		
		if (appearType->treeShadowShape)
		{
			treeShadowShape = appearType->treeShadowShape->CreateFrom();
	
			//-------------------------------------------------
			// Load the texture and store its handle.
			for (long i=0;i<treeShadowShape->GetNumTextures();i++)
			{
				char txmName[1024];
				treeShadowShape->GetTextureName(i,txmName,256);
		
				char texturePath[1024];
				sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
		
				FullPathFileName textureName;
				textureName.init(texturePath,txmName,"");
		
				if (fileExists(textureName))
				{
					if (strnicmp(txmName,"a_",2) == 0)
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						treeShadowShape->SetTextureHandle(i,gosTextureHandle);
						treeShadowShape->SetTextureAlpha(i,true);
					}
					else
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						treeShadowShape->SetTextureHandle(i,gosTextureHandle);
						treeShadowShape->SetTextureAlpha(i,false);
					}
				}
				else
				{
					treeShadowShape->SetTextureHandle(i,0xffffffff);
				}
			}
		}
		else
		{
			treeShadowShape = NULL;
		}
		
		Stuff::Vector3D boxCoords[8];
		Stuff::Vector3D nodeCenter = treeShape->GetRootNodeCenter();

		boxCoords[0].x = position.x + treeShape->GetMinBox().x + nodeCenter.x;
		boxCoords[0].y = position.y + treeShape->GetMinBox().z + nodeCenter.z;
		boxCoords[0].z = position.z + treeShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[1].x = position.x + treeShape->GetMinBox().x + nodeCenter.x;
		boxCoords[1].y = position.y + treeShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[1].z = position.z + treeShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[2].x = position.x + treeShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[2].y = position.y + treeShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[2].z = position.z + treeShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[3].x = position.x + treeShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[3].y = position.y + treeShape->GetMinBox().z + nodeCenter.z;
		boxCoords[3].z = position.z + treeShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[4].x = position.x + treeShape->GetMinBox().x + nodeCenter.x;
		boxCoords[4].y = position.y + treeShape->GetMinBox().z + nodeCenter.z;
		boxCoords[4].z = position.z + treeShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[5].x = position.x + treeShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[5].y = position.y + treeShape->GetMinBox().z + nodeCenter.z;
		boxCoords[5].z = position.z + treeShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[6].x = position.x + treeShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[6].y = position.y + treeShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[6].z = position.z + treeShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[7].x = position.x + treeShape->GetMinBox().x + nodeCenter.x;
		boxCoords[7].y = position.y + treeShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[7].z = position.z + treeShape->GetMinBox().y + nodeCenter.y;
		
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
			appearType->typeUpperLeft = treeShape->GetMinBox();
			appearType->typeLowerRight = treeShape->GetMaxBox();
		}
	}
	
	pitch = yaw = 0.0f;
}

//-----------------------------------------------------------------------------
void TreeAppearance::setObjStatus (long oStatus)
{
	if (status != oStatus)
	{
		if ((oStatus == OBJECT_STATUS_DESTROYED) || (oStatus == OBJECT_STATUS_DISABLED))
		{
			if (appearType->treeDmgShape)
			{
				if (treeShape)
				{
					treeShape->ClearAnimation();
					delete treeShape;
					treeShape = NULL;
				}
				
				treeShape = appearType->treeDmgShape->CreateFrom();
				beenInView = false; 
			}
			
			if (appearType->treeDmgShadowShape)
			{
				if (treeShadowShape)
				{
					treeShadowShape->ClearAnimation();
					delete treeShadowShape;
					treeShadowShape = NULL;
				}
				
				treeShadowShape = appearType->treeDmgShadowShape->CreateFrom();
				
				beenInView = false; 
			}
		}
		
		if (oStatus == OBJECT_STATUS_NORMAL)
		{
			if (appearType->treeShape[0])
			{
				if (treeShape)
				{
					treeShape->ClearAnimation();
					delete treeShape;
					treeShape = NULL;
				}
				
				treeShape = appearType->treeShape[0]->CreateFrom();
				beenInView = false; 
			}
			
			if (appearType->treeShadowShape)
			{
				if (treeShadowShape)
				{
					treeShadowShape->ClearAnimation();
					delete treeShadowShape;
					treeShadowShape = NULL;
				}
				
				treeShadowShape = appearType->treeShadowShape->CreateFrom();
				
				beenInView = false;
			}
		}
		
		//-------------------------------------------------
		// Load the texture and store its handle.
		if (treeShape)
		{
			for (long i=0;i<treeShape->GetNumTextures();i++)
			{
				char txmName[1024];
				treeShape->GetTextureName(i,txmName,256);
		
				char texturePath[1024];
				sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
		
				FullPathFileName textureName;
				textureName.init(texturePath,txmName,"");
		
				if (fileExists(textureName))
				{
					if (strnicmp(txmName,"a_",2) == 0)
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						treeShape->SetTextureHandle(i,gosTextureHandle);
						treeShape->SetTextureAlpha(i,true);
					}
					else
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						treeShape->SetTextureHandle(i,gosTextureHandle);
						treeShape->SetTextureAlpha(i,false);
					}
				}
				else
				{
					//PAUSE(("Warning: %s texture name not found",textureName));
					treeShape->SetTextureHandle(i,0xffffffff);
				}
			}
		}

		if (treeShadowShape)
		{
			//-------------------------------------------------
			// Load the texture and store its handle.
			for (long i=0;i<treeShadowShape->GetNumTextures();i++)
			{
				char txmName[1024];
				treeShadowShape->GetTextureName(i,txmName,256);
		
				char texturePath[1024];
				sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
		
				FullPathFileName textureName;
				textureName.init(texturePath,txmName,"");
		
				if (fileExists(textureName))
				{
					if (strnicmp(txmName,"a_",2) == 0)
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						treeShadowShape->SetTextureHandle(i,gosTextureHandle);
						treeShadowShape->SetTextureAlpha(i,true);
					}
					else
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						treeShadowShape->SetTextureHandle(i,gosTextureHandle);
						treeShadowShape->SetTextureAlpha(i,false);
					}
				}
				else
				{
					//PAUSE(("Warning: %s texture name not found",textureName));
					treeShadowShape->SetTextureHandle(i,0xffffffff);
				}
			}
		}
	}
	
	status = oStatus;
}

//-----------------------------------------------------------------------------
void TreeAppearance::setObjectParameters (const Stuff::Vector3D &pos, float Rot, long sel, long team, long homeRelations)
{
	rotation = Rot;

	position = pos;

	selected = sel;

	actualRotation = Rot;

	teamId = team;
	homeTeamRelationship = homeRelations;
}

//-----------------------------------------------------------------------------
void TreeAppearance::setMoverParameters (float pitchAngle, float lArmRot, float rArmRot, bool isAirborne)
{
	pitch = pitchAngle;
}
		
//-----------------------------------------------------------------------------
bool TreeAppearance::isMouseOver (float px, float py)
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
bool TreeAppearance::recalcBounds (void)
{
	Stuff::Vector4D tempPos;
	inView = false;
	
	float distanceToEye = 0.0f;

	if (eye)
	{
		//-------------------------------------------------------------------
		//NEW METHOD from the WAY BACK Days
		inView = true;
		if (eye->usePerspective)
		{
			Stuff::Vector3D cameraPos;
			cameraPos.x = -eye->getCameraOrigin().x;
			cameraPos.y = eye->getCameraOrigin().z;
			cameraPos.z = eye->getCameraOrigin().y;
			float vClipConstant = eye->verticalSphereClipConstant;
			float hClipConstant = eye->horizontalSphereClipConstant; 
	
			Stuff::Vector3D objectCenter;
			objectCenter.Subtract(position,cameraPos);
			Camera::cameraFrame.trans_to_frame(objectCenter);
			distanceToEye = objectCenter.GetApproximateLength();
			float clip_distance = fabs(1.0f / objectCenter.y);
			
			//Is vertex on Screen OR close enough to screen that its triangle MAY be visible?
			// WE have removed the atans here by simply taking the tan of the angle we want above.
			float object_angle = fabs(objectCenter.z) * clip_distance;
			float extent_angle = treeShape->GetExtentRadius() / distanceToEye;
			if (object_angle > (vClipConstant + extent_angle))
			{
				//In theory, we would return here.  Object is NOT on screen.
				inView = false;
			}
			else
			{
				object_angle = fabs(objectCenter.x) * clip_distance;
				if (object_angle > (hClipConstant + extent_angle))
				{
					//In theory, we would return here.  Object is NOT on screen.
					inView = false;
				}
			}
		}
		
		//Can we be seen at all?
		// If yes, check if we are behind fog plane.
		if (inView)
		{
			//ALWAYS need to do this or select is YAYA
			// But now inView is correct.
			eye->projectZ(position,screenPos);
		
			if (eye->usePerspective)
			{
				if (distanceToEye > Camera::MaxClipDistance)
				{
					hazeFactor = 1.0f;
					inView = false;
				}
				else if (distanceToEye > Camera::MinHazeDistance)
				{
					Camera::HazeFactor = (distanceToEye - Camera::MinHazeDistance) * Camera::DistanceFactor;
					inView = true;
				}
				else
				{
					Camera::HazeFactor = 0.0f;
					inView = true;
				}
			
			}
			else
			{
				Camera::HazeFactor = 0.0f;
				inView = true;
			}
		}
		
		//If we were not behind fog plane, do a bunch O math we need later!!
		if (inView)
		{
			//We are on screen.  Figure out selection box.
			Stuff::Vector3D boxCoords[8];
			Stuff::Vector4D bcsp[8];

			Stuff::Vector3D minBox;
			minBox.x = -appearType->typeUpperLeft.x;
			minBox.y = appearType->typeUpperLeft.z;
			minBox.z = appearType->typeUpperLeft.y;

			Stuff::Vector3D maxBox;
			maxBox.x = -appearType->typeLowerRight.x;
			maxBox.y = appearType->typeLowerRight.z;
			maxBox.z = appearType->typeLowerRight.y;

			if (rotation != 0.0f)
				Rotate(minBox,-rotation);

			if (rotation != 0.0f)
				Rotate(maxBox,-rotation);

			boxCoords[0].x = position.x + minBox.x;
			boxCoords[0].y = position.y + minBox.y;
			boxCoords[0].z = position.z + minBox.z;

			boxCoords[1].x = position.x + minBox.x;
			boxCoords[1].y = position.y + maxBox.y;
			boxCoords[1].z = position.z + minBox.z;

			boxCoords[2].x = position.x + maxBox.x;
			boxCoords[2].y = position.y + minBox.y;
			boxCoords[2].z = position.z + minBox.z;

			boxCoords[3].x = position.x + maxBox.x;
			boxCoords[3].y = position.y + maxBox.y;
			boxCoords[3].z = position.z + minBox.z;

			boxCoords[4].x = position.x + maxBox.x;
			boxCoords[4].y = position.y + maxBox.y;
			boxCoords[4].z = position.z + maxBox.z;

			boxCoords[5].x = position.x + maxBox.x;
			boxCoords[5].y = position.y + minBox.y;
			boxCoords[5].z = position.z + maxBox.z;

			boxCoords[6].x = position.x + minBox.x;
			boxCoords[6].y = position.y + maxBox.y;
			boxCoords[6].z = position.z + maxBox.z;

			boxCoords[7].x = position.x + minBox.x;
			boxCoords[7].y = position.y + minBox.y;
			boxCoords[7].z = position.z + maxBox.z;

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
		
				if ((status != OBJECT_STATUS_DESTROYED) && (status != OBJECT_STATUS_DISABLED))
				{
					//-------------------------------------------------------------------------------
					//Set LOD of Model here because we have the distance and we KNOW we can see it!
					bool baseLOD = true;
					DWORD selectLOD = 0;
					if (useHighObjectDetail)
					{
						for (long i=1;i<MAX_LODS;i++)
						{
							if (appearType->treeShape[i] && (distanceToEye > appearType->lodDistance[i]))
							{
								baseLOD = false;
								selectLOD = i;
							}
						}
					}
					else	//We always want to use the lowest LOD!!
					{
						if (appearType->treeShape[1])
						{
							baseLOD = false;
							selectLOD = 1;
						}
					}
					
					// we are at this LOD level.
					if (selectLOD != currentLOD)
					{
						currentLOD = selectLOD;

						treeShape->ClearAnimation();
						delete treeShape;
						treeShape = NULL;

						treeShape = appearType->treeShape[currentLOD]->CreateFrom();
						//-------------------------------------------------
						// Load the texture and store its handle.
						for (long j=0;j<treeShape->GetNumTextures();j++)
						{
							char txmName[1024];
							treeShape->GetTextureName(j,txmName,256);

							char texturePath[1024];
							sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);

							FullPathFileName textureName;
							textureName.init(texturePath,txmName,"");

							if (fileExists(textureName))
							{
								if (strnicmp(txmName,"a_",2) == 0)
								{
									DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
									gosASSERT(gosTextureHandle != 0xffffffff);
									treeShape->SetTextureHandle(j,gosTextureHandle);
									treeShape->SetTextureAlpha(j,true);
								}
								else
								{
									DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
									gosASSERT(gosTextureHandle != 0xffffffff);
									treeShape->SetTextureHandle(j,gosTextureHandle);
									treeShape->SetTextureAlpha(j,false);
								}
							}
							else
							{
								//PAUSE(("Warning: %s texture name not found",textureName));
								treeShape->SetTextureHandle(j,0xffffffff);
							}
						}
					}
						
					//ONLY change if we need
					if (currentLOD && baseLOD)
					{
					// we are at the Base LOD level.
						currentLOD = 0;
						
						treeShape->ClearAnimation();
						delete treeShape;
						treeShape = NULL;
						
						treeShape = appearType->treeShape[currentLOD]->CreateFrom();
						
						//-------------------------------------------------
						// Load the texture and store its handle.
						for (long i=0;i<treeShape->GetNumTextures();i++)
						{
							char txmName[1024];
							treeShape->GetTextureName(i,txmName,256);
									
							char texturePath[1024];
							sprintf(texturePath,"%s%d" PATH_SEPARATOR,tglPath,ObjectTextureSize);
					
							FullPathFileName textureName;
							textureName.init(texturePath,txmName,"");
									
							if (fileExists(textureName))
							{
								if (strnicmp(txmName,"a_",2) == 0)
								{
									DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
									gosASSERT(gosTextureHandle != 0xffffffff);
									treeShape->SetTextureHandle(i,gosTextureHandle);
									treeShape->SetTextureAlpha(i,true);
								}
								else
								{
									DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
									gosASSERT(gosTextureHandle != 0xffffffff);
									treeShape->SetTextureHandle(i,gosTextureHandle);
									treeShape->SetTextureAlpha(i,false);
								}
							}
							else
							{
								//PAUSE(("Warning: %s texture name not found",textureName));
								treeShape->SetTextureHandle(i,0xffffffff);
							}
						}
					}
				}
			}
			else
			{
				inView = false;		//Did alot of extra work checking this, but WHY draw and insult to injury?
			}
		}
	}
	
	return(inView);
}

//-----------------------------------------------------------------------------
long TreeAppearance::render (long depthFixup)
{
	if (inView)
	{
		long color = SD_BLUE;
		//unsigned long highLight = 0x007f7f7f;
		if ((teamId > -1) && (teamId < 8)) {
			//static unsigned long highLightTable[3] = {0x00007f00, 0x0000007f, 0x007f0000};
			static long colorTable[3] = {SB_GREEN | 0xff000000, SB_BLUE | 0xff000000, SB_RED | 0xff000000};
			color = colorTable[homeTeamRelationship];
			//highLight = highLightTable[homeTeamRelationship];
		}
		//---------------------------------------------
		// Call Multi-shape render stuff here.
		treeShape->Render();

		if (selected & DRAW_BARS)
		{
			drawBars();
		}

		if ( selected & DRAW_BRACKETS )
		{
			drawSelectBrackets(color);
		}

		if ( selected & DRAW_TEXT )
		{

			if (objectNameId != -1)
			{
				char tmpString[255];
				cLoadString(objectNameId, tmpString, 254);

				drawTextHelp(tmpString, color);
			}
		}

		// I don't want my selection reset each time I draw HKG
//		selected = FALSE;

		//---------------------------------------------------------
		// Render the Bounding Box to see if it is OK.
#ifdef DRAW_BOX
		Stuff::Vector3D nodeCenter = treeShape->GetRootNodeCenter();

		boxCoords[0].x = position.x + treeShape->minBox.x + nodeCenter.x;
		boxCoords[0].y = position.y + treeShape->minBox.z + nodeCenter.z;
		boxCoords[0].z = position.z + treeShape->maxBox.y + nodeCenter.y;
		
		boxCoords[1].x = position.x + treeShape->minBox.x + nodeCenter.x;
		boxCoords[1].y = position.y + treeShape->maxBox.z + nodeCenter.z;
		boxCoords[1].z = position.z + treeShape->maxBox.y + nodeCenter.y;
		
		boxCoords[2].x = position.x + treeShape->maxBox.x + nodeCenter.x;
		boxCoords[2].y = position.y + treeShape->maxBox.z + nodeCenter.z;
		boxCoords[2].z = position.z + treeShape->maxBox.y + nodeCenter.y;
		
		boxCoords[3].x = position.x + treeShape->maxBox.x + nodeCenter.x;
		boxCoords[3].y = position.y + treeShape->minBox.z + nodeCenter.z;
		boxCoords[3].z = position.z + treeShape->maxBox.y + nodeCenter.y;
		
		boxCoords[4].x = position.x + treeShape->minBox.x + nodeCenter.x;
		boxCoords[4].y = position.y + treeShape->minBox.z + nodeCenter.z;
		boxCoords[4].z = position.z + treeShape->minBox.y + nodeCenter.y;
		
		boxCoords[5].x = position.x + treeShape->maxBox.x + nodeCenter.x;
		boxCoords[5].y = position.y + treeShape->minBox.z + nodeCenter.z;
		boxCoords[5].z = position.z + treeShape->minBox.y + nodeCenter.y;
		
		boxCoords[6].x = position.x + treeShape->maxBox.x + nodeCenter.x;
		boxCoords[6].y = position.y + treeShape->maxBox.z + nodeCenter.z;
		boxCoords[6].z = position.z + treeShape->minBox.y + nodeCenter.y;
		
		boxCoords[7].x = position.x + treeShape->minBox.x + nodeCenter.x;
		boxCoords[7].y = position.y + treeShape->maxBox.z + nodeCenter.z;
		boxCoords[7].z = position.z + treeShape->minBox.y + nodeCenter.y;
		
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
	}
	return NO_ERR;
}

//-----------------------------------------------------------------------------
long TreeAppearance::renderShadows (void)
{
	if (inView && visible)
	{
		//---------------------------------------------
		// Call Multi-shape render stuff here.
		if (treeShadowShape)
			treeShadowShape->RenderShadows();
		else
			treeShape->RenderShadows();
	}
	
	return NO_ERR;
}

//-----------------------------------------------------------------------------
long TreeAppearance::update (bool animate) 
{
	if (rotation > 180)
		rotation -= 360;

	if (rotation < -180)
		rotation += 360;

	//-------------------------------------------
	// Does math necessary to draw Tree
	Stuff::UnitQuaternion rot;
	float yawAngle = (rotation * DEGREES_TO_RADS) + (yaw * DEGREES_TO_RADS);
	float pitchAngle = (pitch * DEGREES_TO_RADS);
	rot = Stuff::EulerAngles(pitchAngle, yawAngle, 0.0f);

	Stuff::Point3D xlatPosition;
	xlatPosition.x = -position.x;
	xlatPosition.y = position.z;
	xlatPosition.z = position.y;

	if (!fogLightSet)
	{
		unsigned char lightr,lightg,lightb;
		float lightIntensity = 1.0f;
		if (land)
			lightIntensity = land->getTerrainLight(position);

		lightr = eye->getLightRed(lightIntensity);
		lightg = eye->getLightGreen(lightIntensity);
		lightb = eye->getLightBlue(lightIntensity);

		lightRGB = (lightr<<16) + (lightg<<8) + lightb;

		fogRGB = 0xff<<24;
		float fogStart = eye->fogStart;
		float fogFull = eye->fogFull;

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

		fogLightSet = true;
	}

	if (useFog)
		treeShape->SetFogRGB(fogRGB);
	else
		treeShape->SetFogRGB(0xffffffff);

	DWORD oldRGB = eye->getLightColor(1);

	eye->setLightColor(1,lightRGB);
	eye->setLightIntensity(1,1.0);

	if (forceLightsOut)
		treeShape->SetLightsOut(true);

	if (inView)
	{
		bool checkShadows = ((!beenInView) || (eye->forceShadowRecalc));
		
		if (treeShadowShape)
			treeShape->SetUseShadow(false);
		else
			treeShape->SetRecalcShadows(checkShadows);
			
		TG_LightPtr light = eye->getWorldLight(0);
		light->active = false;

		treeShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
		treeShape->TransformMultiShape (&xlatPosition,&rot);
		
		light->active = true;

		if (treeShadowShape && useShadows)
		{
			treeShadowShape->SetRecalcShadows(checkShadows);
			treeShadowShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
			treeShadowShape->TransformMultiShape (&xlatPosition,&rot);
		}
		
		if ((turn > 3) && useShadows)
			beenInView = true;
	}
	
	//Set Ambient back to normal color.
	eye->setLightColor(1,oldRGB);

	return TRUE;
}

//-----------------------------------------------------------------------------

void TreeAppearance::markTerrain (_ScenarioMapCellInfo* pInfo, int type, int counter)
{
	//MUST force tree to HIGHEST LOD!!!  Impassability data is only valid at this LOD!!
	// Tree will reset its LOD on next draw!!
	if (currentLOD)
	{
		currentLOD = 0;
	
		treeShape->ClearAnimation();
		delete treeShape;
		treeShape = NULL;
	
		treeShape = appearType->treeShape[currentLOD]->CreateFrom();
	}

	//-------------------------------------------------------------
	// New way.  For each vertex in each shape, translate to world
	for (int i=0;i<treeShape->GetNumShapes();i++)
	{
		//Check if the artists meant for this piece to NOT block passability!!
		if (strnicmp(treeShape->GetNodeId(i),"_PAB",4) != 0)
		{
			for (int j=0;j<treeShape->GetNumVerticesInShape(i);j++)
			{
				Stuff::Vector3D vertexPos, worldPos;
				vertexPos = treeShape->GetShapeVertexInEditor(i,j,-rotation);
				worldPos.Add(position,vertexPos);
	
				int cellR, cellC;
				land->worldToCell(worldPos,cellR,cellC);
				if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
					(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
				{
					continue;
				}
				_ScenarioMapCellInfo*pTmp = &(pInfo[cellR * Terrain::realVerticesMapSide * MAPCELL_DIM + cellC]);

				if (vertexPos.z >= 1.0f)
				{
					pTmp->forest = true;
						
					float cellLocalHeight = vertexPos.z * metersPerWorldUnit * 0.25f;
					if (cellLocalHeight > 15.0f)
						cellLocalHeight = 15.0f;
						
					if (pTmp->lineOfSight < cellLocalHeight)
						pTmp->lineOfSight = cellLocalHeight+0.5f;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TreeAppearance::markLOS (bool clearIt)
{
	//MUST force building to HIGHEST LOD!!!  IMpassability data is only valid at this LOD!!
	// Building will reset its LOD on next draw!!
	if (currentLOD)
	{
		currentLOD = 0;
	
		treeShape->ClearAnimation();
		delete treeShape;
		treeShape = NULL;
	
		treeShape = appearType->treeShape[currentLOD]->CreateFrom();
	}

	//-------------------------------------------------------------
	// New way.  For each vertex in each shape, translate to world
	for (int i=0;i<treeShape->GetNumShapes();i++)
	{
		//Check if the artists meant for this piece to NOT block LOS!!
		// Probably should check for light cones,too!
		
		if ((strnicmp(treeShape->GetNodeId(i),"LOS_",4) != 0) &&
			(strnicmp(treeShape->GetNodeId(i),"SpotLight_",10) != 0))
		{
			for (int j=0;j<treeShape->GetNumVerticesInShape(i);j++)
			{
				Stuff::Vector3D vertexPos, worldPos;
				vertexPos = treeShape->GetShapeVertexInEditor(i,j,-rotation);
//				vertexPos = treeShape->GetShapeVertexInWorld(i,j,-rotation);
				worldPos.Add(position,vertexPos);
	
				int cellR, cellC;
				land->worldToCell(worldPos,cellR,cellC);
				if ((0 > cellR) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellR) || 
					(0 > cellC) || (Terrain::realVerticesMapSide * MAPCELL_DIM <= cellC))
				{
					continue;
				}
				
				//----------------
				// Mark the map...
				MapCellPtr curCell = GameMap->getCell(cellR, cellC);

				float currentCellHeight = curCell->getLocalHeight();
				
				float cellLocalHeight = vertexPos.z * metersPerWorldUnit * 0.25f;
				if (cellLocalHeight > 15.0f)
					cellLocalHeight = 15.0f;
				
				if (!clearIt)
				{
					if (cellLocalHeight > currentCellHeight)
						curCell->setLocalHeight(cellLocalHeight+0.5f);
				}
				else	//We want to clear all LOS height INFO.  We're about to change shape!!
				{
					curCell->setLocalHeight(0.0f);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void TreeAppearance::destroy (void)
{
	if ( treeShape )
	{
		delete treeShape;
		treeShape = NULL;
	}

	appearanceTypeList->removeAppearance(appearType);
}


//*****************************************************************************
