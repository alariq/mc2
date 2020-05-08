//---------------------------------------------------------------------------
//
//	gvactor.cpp - This file contains the code for the Ground Vehicle Actor class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GVACTOR_H
#include"gvactor.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef DBASEGUI_H
#include"dbasegui.h"
#endif

#ifndef MECH3D_H
#include"mech3d.h"
#endif

#ifndef OBJSTATUS_H
#include"objstatus.h"
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

#ifndef UTILITIES_H
#include"utilities.h"
#endif

#ifndef CEVFX_H
#include"cevfx.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#ifndef ERR_H
#include"err.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
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

extern bool 	useShadows;

extern int		ObjectTextureSize;
TG_TypeMultiShapePtr GVAppearanceType::SensorTriangleShape = NULL;
TG_TypeMultiShapePtr GVAppearanceType::SensorCircleShape = NULL;

extern bool reloadBounds;

#define SPIN_RATE		90.0f

#define EXPAND_FACTOR 		(1.25f)

#define MAX_SMOKE_NODES		2
#define MAX_WEAPON_NODES	4
#define MAX_FOOT_NODES		2

extern MidLevelRenderer::MLRClipper * theClipper;

extern bool MLRVertexLimitReached;
extern bool InEditor;
extern bool useNonWeaponEffects;
extern bool useHighObjectDetail;
//-----------------------------------------------------------------------------
// class GVAppearanceType
void GVAppearanceType::init (const char * fileName)
{
	AppearanceType::init(fileName);

	//----------------------------------------------
	FullPathFileName iniName;
	iniName.init(tglPath,fileName,".ini");

	FitIniFile iniFile;
	long result = iniFile.open(iniName);
	if (result != NO_ERR)
		Fatal(result,"Could not find vehicle appearance INI file");

	result = iniFile.seekBlock("TGLData");
	if (result != NO_ERR)
		Fatal(result,"Could not find block in vehicle appearance INI file");

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
				gvShape[i] = new TG_TypeMultiShape;
				gosASSERT(gvShape[i] != NULL);
			
				FullPathFileName gvName;
				gvName.init(tglPath,aseFileName,".ase");
			
				gvShape[i]->LoadTGMultiShapeFromASE(gvName);
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
		gvShape[0] = new TG_TypeMultiShape;
		gosASSERT(gvShape[0] != NULL);
	
		FullPathFileName gvName;
		gvName.init(tglPath,aseFileName,".ase");
	
		gvShape[0]->LoadTGMultiShapeFromASE(gvName);
	}

	result = iniFile.readIdString("ShadowName",aseFileName,511);
	if (result == NO_ERR)
	{
		//----------------------------------------------
		// Base Shadow shape.
		gvShadowShape = new TG_TypeMultiShape;
		gosASSERT(gvShadowShape != NULL);
	
		FullPathFileName gvName;
		gvName.init(tglPath,aseFileName,".ase");
	
		gvShadowShape->LoadTGMultiShapeFromASE(gvName);
	}

	result = iniFile.seekBlock("TGLDamage");
	if (result == NO_ERR)
	{
		result = iniFile.readIdString("FileName",aseFileName,511);
		if (result != NO_ERR)
			Fatal(result,"Could not find ASE FileName in building appearance INI file");
	
		FullPathFileName dmgName;
		dmgName.init(tglPath,aseFileName,".ase");
	
		gvDmgShape = new TG_TypeMultiShape;
		gosASSERT(gvDmgShape != NULL);
		gvDmgShape->LoadTGMultiShapeFromASE(dmgName);

		if (!gvDmgShape->GetNumShapes())
		{
			delete gvDmgShape;
			gvDmgShape = NULL;
		}
	}
	else
	{
		gvDmgShape = NULL;
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
	for (long i=0;i<MAX_GV_ANIMATIONS;i++)
	{
		char blockId[512];
		sprintf(blockId,"Animation:%d",i);
		
		result = iniFile.seekBlock(blockId);
		if (result == NO_ERR)
		{
			char animName[512];
			result = iniFile.readIdString("AnimationName",animName,511);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("LoopAnimation",gvAnimLoop[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("Reverse",gvReverse[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdBoolean("Random",gvRandom[i]);
			gosASSERT(result == NO_ERR);
			
			result = iniFile.readIdLong("StartFrame",gvStartF[i]);
			if (result != NO_ERR)
				gvStartF[i] = 0;
				
 			//-------------------------------
			// We have an animation to load.
			FullPathFileName animPath;
			animPath.init(tglPath,animName,".ase");

			FullPathFileName otherPath;
			otherPath.init(tglPath,animName,".agl");

			if (fileExists(animPath) || fileExists(otherPath))
			{
				gvAnimData[i] = new TG_AnimateShape;
				gosASSERT(gvAnimData[i] != NULL);
	
				//--------------------------------------------------------
				// If this animation does not exist, it is not a problem!
				// Building will simply freeze until animation is "over"
				gvAnimData[i]->LoadTGMultiShapeAnimationFromASE(animPath,gvShape[0]);
			}
			else
				gvAnimData[i] = NULL;
		}
		else
		{
			gvAnimData[i] = NULL;
		}
	}
	
 	//--------------------------------------------------------------------
	// We can also load the node to yaw for vehicles with turrets.
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

	//-----------------------------------------------
	// Load up the Node Data.
	// Automatic for vehicles now.
	numSmokeNodes = MAX_SMOKE_NODES;
	numWeaponNodes = MAX_WEAPON_NODES;
	numFootNodes = MAX_FOOT_NODES;
		
	nodeData = (NodeData *)AppearanceTypeList::appearanceHeap->Malloc(sizeof(NodeData)*(numWeaponNodes+numSmokeNodes+numFootNodes));
	gosASSERT(nodeData != NULL);
	memset(nodeData,0,sizeof(NodeData)*(numWeaponNodes+numSmokeNodes+numFootNodes));
		
	for (int i=1;i<=numSmokeNodes;i++)
	{
		char blockId[512];
		sprintf(blockId,"Smoke_%02d",i);
			
		nodeData[i-1].nodeId = (char *)AppearanceTypeList::appearanceHeap->Malloc(strlen(blockId)+1); 
		gosASSERT(nodeData[i-1].nodeId != NULL);
			
		strcpy(nodeData[i-1].nodeId,blockId);
		nodeData[i-1].weaponType = MECH3D_WEAPONTYPE_NONE;
	}
		
	for (int i=1;i<=numWeaponNodes;i++)
	{
		char blockId[512];
		sprintf(blockId,"Weapon_%02d",i);
		
		nodeData[(i-1)+numSmokeNodes].nodeId = (char *)AppearanceTypeList::appearanceHeap->Malloc(strlen(blockId)+1);
		gosASSERT(nodeData[(i-1)+numSmokeNodes].nodeId != NULL);
		
		strcpy(nodeData[(i-1)+numSmokeNodes].nodeId,blockId);
		nodeData[(i-1)+numSmokeNodes].weaponType = MECH3D_WEAPONTYPE_ANY;
		
	}
	
	for (int i=1;i<=numFootNodes;i++)
	{
		char blockId[512];
		sprintf(blockId,"FootNode_%02d",i);
			
		nodeData[(i-1)+numSmokeNodes+numWeaponNodes].nodeId = (char *)AppearanceTypeList::appearanceHeap->Malloc(strlen(blockId)+1);       
		gosASSERT(nodeData[(i-1)+numSmokeNodes+numWeaponNodes].nodeId != NULL);
			
		strcpy(nodeData[(i-1)+numSmokeNodes+numWeaponNodes].nodeId,blockId);
		nodeData[(i-1)+numSmokeNodes+numWeaponNodes].weaponType = MECH3D_WEAPONTYPE_NONE;
	}
 	
 	//----------------------------------------------
	// Load up sensor Shape if not yet defined.
	if (SensorCircleShape == NULL)
	{
		FullPathFileName sensorName;
		sensorName.init(tglPath,"circularcontact",".ase");
	
		SensorCircleShape = new TG_TypeMultiShape;
		gosASSERT(SensorCircleShape != NULL);
	
		SensorCircleShape->LoadTGMultiShapeFromASE(sensorName);
	}

	if (SensorTriangleShape == NULL)
	{
		FullPathFileName sensorName;
		sensorName.init(tglPath,"trianglecontact",".ase");
	
		SensorTriangleShape = new TG_TypeMultiShape;
		gosASSERT(SensorCircleShape != NULL);
	
		SensorTriangleShape->LoadTGMultiShapeFromASE(sensorName);
	}
}

//----------------------------------------------------------------------------
void GVAppearanceType::destroy (void)
{
	AppearanceType::destroy();

	for (long i=0;i<MAX_LODS;i++)
	{
		if (gvShape[i])
		{
			delete gvShape[i];
			gvShape[i] = NULL;
		}
	}

	if (gvShadowShape)
	{
		delete gvShadowShape;
		gvShadowShape = NULL;
	}

	if (gvDmgShape)
	{
		delete gvDmgShape;
		gvDmgShape = NULL;
	}

	for (int i=0;i<MAX_GV_ANIMATIONS;i++)
	{
		delete gvAnimData[i];
		gvAnimData[i] = NULL;
	}

	if (nodeData)
	{
		for (int i=0;i<getTotalNodes();i++)
		{
			AppearanceTypeList::appearanceHeap->Free(nodeData[i].nodeId);
			nodeData[i].nodeId = NULL;
		}
		
		AppearanceTypeList::appearanceHeap->Free(nodeData);
		nodeData = NULL;
	}
}

//-----------------------------------------------------------------------------
void GVAppearanceType::setAnimation (TG_MultiShapePtr shape, DWORD animationNum)
{
	gosASSERT(shape != NULL);
	gosASSERT(animationNum != 0xffffffff);
	gosASSERT(animationNum < MAX_GV_ANIMATIONS);

	if (gvAnimData[animationNum])
		gvAnimData[animationNum]->SetAnimationState(shape);
	else
		shape->ClearAnimation();
}

//-----------------------------------------------------------------------------

Stuff::Vector3D debugGVActorPosition;
float vehicleDebugAngle = 0.0;
float turretDebugAngle = 0.0;
float debugVelMag = 0.0;
#define BASE_NODE_RECYCLE_TIME		0.25f
//-----------------------------------------------------------------------------
// class GVAppearance
void GVAppearance::setWeaponNodeUsed (long weaponNode)
{
	weaponNode -= appearType->numSmokeNodes;
   	if ((weaponNode >= 0) && (weaponNode < appearType->numWeaponNodes))
	{
		nodeUsed[weaponNode]++;
		nodeRecycle[weaponNode] = BASE_NODE_RECYCLE_TIME;
	}
}

//-----------------------------------------------------------------------------
Stuff::Vector3D GVAppearance::getWeaponNodePosition (long nodeId)
{
	Stuff::Vector3D result = position;
	if ((nodeId < appearType->numSmokeNodes) || (nodeId >= (appearType->numSmokeNodes+appearType->numWeaponNodes)))
		return result;
	
	if (!inView)
		return result;

	//We already know we are using this node.  Do NOT increment recycle or nodeUsed!
	
   	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = position.z;
   	xlatPosition.z = position.y;
   
   	Stuff::UnitQuaternion torsoRot;
   	torsoRot = Stuff::EulerAngles(0.0f,(turretRotation * DEGREES_TO_RADS),0.0f);
	if (rotationalNodeIndex == -1)
	   	rotationalNodeIndex = gvShape->SetNodeRotation(appearType->rotationalNodeId,&torsoRot);

	gvShape->SetNodeRotation(rotationalNodeIndex,&torsoRot);

	if (weaponNodeId[nodeId - appearType->numSmokeNodes] == -1)
		weaponNodeId[nodeId - appearType->numSmokeNodes] = gvShape->GetNodeNameId(appearType->nodeData[nodeId].nodeId);

	//If its STILL -1, we don't really have four weapon nodes.
	// Mark it with -2 to let us know to ignore it from now on.
	if (weaponNodeId[nodeId - appearType->numSmokeNodes] == -1)
		weaponNodeId[nodeId - appearType->numSmokeNodes] = -2;

	if (weaponNodeId[nodeId - appearType->numSmokeNodes] >= 0)
		result = gvShape->GetTransformedNodePosition(&xlatPosition,&qRotation,weaponNodeId[nodeId - appearType->numSmokeNodes]);

	if ((result.x == 0.0f) &&
		(result.y == 0.0f) && 
		(result.z == 0.0f))
		result = position;
		
 	return result;
}

//-----------------------------------------------------------------------------
Stuff::Vector3D GVAppearance::getSmokeNodePosition (long nodeId)
{
	Stuff::Vector3D result = position;
	if ((nodeId < 0) || (nodeId >= appearType->numSmokeNodes))
		return result;
	
 	if (!inView)
		return result;

	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = position.z;
   	xlatPosition.z = position.y;
   
   	Stuff::UnitQuaternion torsoRot;
   	torsoRot = Stuff::EulerAngles(0.0f,(turretRotation * DEGREES_TO_RADS),0.0f);
	if (rotationalNodeIndex == -1)
	   	rotationalNodeIndex = gvShape->SetNodeRotation(appearType->rotationalNodeId,&torsoRot);

	gvShape->SetNodeRotation(rotationalNodeIndex,&torsoRot);
   
	result = gvShape->GetTransformedNodePosition(&xlatPosition,&qRotation,appearType->nodeData[nodeId].nodeId);

	return result;
}

//-----------------------------------------------------------------------------
Stuff::Vector3D GVAppearance::getDustNodePosition (long nodeId)
{
	Stuff::Vector3D result = position;
	if ((nodeId < 0) || (nodeId >= appearType->numFootNodes))
		return result;
	
	if (!inView)
		return result;

   	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = position.z;
   	xlatPosition.z = position.y;
   
   	Stuff::UnitQuaternion torsoRot;
   	torsoRot = Stuff::EulerAngles(0.0f,(turretRotation * DEGREES_TO_RADS),0.0f);
	if (rotationalNodeIndex == -1)
	   	rotationalNodeIndex = gvShape->SetNodeRotation(appearType->rotationalNodeId,&torsoRot);

	gvShape->SetNodeRotation(rotationalNodeIndex,&torsoRot);
   
	result = gvShape->GetTransformedNodePosition(&xlatPosition,&qRotation,appearType->nodeData[nodeId+appearType->numWeaponNodes+appearType->numSmokeNodes].nodeId);

	return result;
}

//-----------------------------------------------------------------------------
long GVAppearance::getLowestWeaponNode (void)
{
	//------------------------------------------------
	// Scan all weapon nodes and find least used one.
	long bestNode = -1;
	float lowestPosZ;
	long numSmokeNodes = appearType->numSmokeNodes;
	lowestPosZ = 9999999999999.0f;
	for (long i=0;i<appearType->numWeaponNodes;i++)
	{
		Stuff::Vector3D nodePosition = getWeaponNodePosition(i+numSmokeNodes);
		if (((nodePosition.z - position.z) < lowestPosZ) && (nodePosition.z != position.z))
		{
			lowestPosZ = nodePosition.z - position.z;
			bestNode = i+numSmokeNodes;
		}
	}
		
   	if ((lowestPosZ == 0.0f) || (bestNode < 0) || (bestNode >= appearType->getTotalNodes()))
   		return -1;

 	return bestNode;
}

//-----------------------------------------------------------------------------
long GVAppearance::getWeaponNode (long weaponType)
{
	//------------------------------------------------
	// Scan all weapon nodes and find least used one.
	long leastUsed = 999999999;
	long bestNode = -1;
	long numSmokeNodes = appearType->numSmokeNodes;
	for (long i=0;i<appearType->numWeaponNodes;i++)
	{
		//This weaponNode does not exist if its -2
		if (weaponNodeId[i] == -2)
			continue;

		switch (weaponType)
		{
			case MECH3D_WEAPONTYPE_MISSILE:
				if ((appearType->nodeData[numSmokeNodes+i].weaponType == weaponType) || 
					(appearType->nodeData[numSmokeNodes+i].weaponType == MECH3D_WEAPONTYPE_ANY) || 
					(appearType->nodeData[numSmokeNodes+i].weaponType == MECH3D_WEAPONTYPE_NONENERGY))
				{
//					if (getWeaponNodePosition(i + numSmokeNodes).z != position.z)
//					{
						if (nodeUsed[i] < leastUsed)
						{
							leastUsed = nodeUsed[i];
							bestNode = i + numSmokeNodes;
						}
//					}
				}
			break;
			
			case MECH3D_WEAPONTYPE_BALLISTIC:
				if ((appearType->nodeData[numSmokeNodes+i].weaponType == weaponType) || 
					(appearType->nodeData[numSmokeNodes+i].weaponType == MECH3D_WEAPONTYPE_ANY) || 
					(appearType->nodeData[numSmokeNodes+i].weaponType == MECH3D_WEAPONTYPE_NONENERGY))
				{
//					if (getWeaponNodePosition(i + numSmokeNodes).z != position.z)
//					{
	 					if (nodeUsed[i] < leastUsed)
						{
							leastUsed = nodeUsed[i];
							bestNode = i + numSmokeNodes;
						}
//					}
				}
			break;
			
			case MECH3D_WEAPONTYPE_ENERGY:
				if ((appearType->nodeData[numSmokeNodes+i].weaponType == weaponType) || 
					(appearType->nodeData[numSmokeNodes+i].weaponType == MECH3D_WEAPONTYPE_ANY))
				{
//					if (getWeaponNodePosition(i + numSmokeNodes).z != position.z)
//					{
	 					if (nodeUsed[i] < leastUsed)
						{
							leastUsed = nodeUsed[i];
							bestNode = i + numSmokeNodes;
						}
//					}
				}
			break;
			
			case MECH3D_WEAPONTYPE_ANY:
//				if (getWeaponNodePosition(i + numSmokeNodes).z != position.z)
//				{
	 				if (nodeUsed[i] < leastUsed)
					{
						leastUsed = nodeUsed[i];
						bestNode = i + numSmokeNodes;
					}
//				}
			break;

			default:
				STOP(("Sent down a bad weapon type %d",weaponType));
		}
	}
		
   	if ((bestNode < 0) || (bestNode >= appearType->getTotalNodes()))
   		return -1;

 	return bestNode;
}
		
//-----------------------------------------------------------------------------
float GVAppearance::getWeaponNodeRecycle (long node)
{
	node -= appearType->numSmokeNodes;
	
 	if ((node >=0) && (node < appearType->numWeaponNodes))
		return nodeRecycle[node];
		
	return 9999.0f;		//NOT a weapon node.  Never recycled!!
}

//-----------------------------------------------------------------------------
void GVAppearance::init (AppearanceTypePtr tree, GameObjectPtr obj)
{
	Appearance::init(tree,obj);
	appearType = (GVAppearanceType *)tree;

	shapeMin.x = shapeMin.y = -25;
	shapeMax.x = shapeMax.y = 50;
	
	paintScheme = -1;
	objectNameId = 30862;
	pilotNameID = -1;
	hazeFactor = 0.0f;

	currentFlash = duration = flashDuration = 0.0f;
	flashColor = 0x00000000;
	drawFlash = false;
	
	rotationalNodeIndex = -1;
	dustNodeIndex = activityNodeIndex = hitNodeId = weaponNodeId[0] = weaponNodeId[1] = weaponNodeId[2] = weaponNodeId[3] = -1;

	screenPos.x = screenPos.y = screenPos.z = screenPos.w = -999.0f;
	position.Zero();
	rotation = 0.0;;
	turretRotation= 0.0;
	selected = 0;
	teamId = -1;
	homeTeamRelationship = 0;
	actualRotation = rotation;
	actualTurretRotation = turretRotation;
	inDebugMoveMode = false;

	sensorLevel = 0;
	
	destructFX = NULL;
	
	status = 0;
	roll = pitch = 0.0f;

	currentLOD = 0;
	
	OBBRadius = 0.0f;

	sensorSpin = 0.0f;
	
	gvAnimationState =-1;
	currentFrame = 0.0f;
	gvFrameRate = 0.0f;
	isReversed = false;
	isLooping = false;
	setFirstFrame = false;
	canTransition = true;
 
	localTextureHandle = 0xffffffff;
	
	nodeUsed = NULL;
	nodeRecycle = NULL;
	
	waterWake = NULL;
	dustCloud = NULL;
	activity = NULL;
	isWaking = false;
	movedThisFrame = false;
	dustCloudStart = false;
	isActivitying = false;

	if (appearType)
	{
		gvShape = appearType->gvShape[0]->CreateFrom();
	
		//-------------------------------------------------
		// Load the texture and store its handle.
		for (long i=0;i<gvShape->GetNumTextures();i++)
		{
			char txmName[1024];
			gvShape->GetTextureName(i,txmName,256);

			char texturePath[1024];
			sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
	
			FullPathFileName textureName;
			textureName.init(texturePath,txmName,"");

			if (fileExists(textureName))
			{
				if (S_strnicmp(txmName,"a_",2) == 0)
				{
					DWORD gosTextureHandle = 0;
					
					if (!i)
					{
						localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink,true);
						gvShape->SetTextureHandle(i,localTextureHandle);
						gvShape->SetTextureAlpha(i,true);
					}
					else
					{
						gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gvShape->SetTextureHandle(i,gosTextureHandle);
						gvShape->SetTextureAlpha(i,true);
					}
					
				}
				else
				{
					DWORD gosTextureHandle = 0;
					
					if (!i)
					{
						localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink,true);
						gvShape->SetTextureHandle(i,localTextureHandle);
						gvShape->SetTextureAlpha(i,false);
					}
					else
					{
						gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gvShape->SetTextureHandle(i,gosTextureHandle);
						gvShape->SetTextureAlpha(i,false);
					}
				}
			}
			else
			{
				//PAUSE(("Warning: %s texture name not found",textureName));
				gvShape->SetTextureHandle(i,0xffffffff);
			}
		}
		
		if (appearType->gvShadowShape)
		{
			gvShadowShape = appearType->gvShadowShape->CreateFrom();
	
			//-------------------------------------------------
			// Load the texture and store its handle.
			for (long i=0;i<gvShadowShape->GetNumTextures();i++)
			{
				char txmName[1024];
				gvShadowShape->GetTextureName(i,txmName,256);
		
				char texturePath[1024];
				sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
		
				FullPathFileName textureName;
				textureName.init(texturePath,txmName,"");
		
				if (fileExists(textureName))
				{
					if (S_strnicmp(txmName,"a_",2) == 0)
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						gvShadowShape->SetTextureHandle(i,gosTextureHandle);
						gvShadowShape->SetTextureAlpha(i,true);
					}
					else
					{
						DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gosASSERT(gosTextureHandle != 0xffffffff);
						gvShadowShape->SetTextureHandle(i,gosTextureHandle);
						gvShadowShape->SetTextureAlpha(i,false);
					}
				}
				else
				{
					gvShadowShape->SetTextureHandle(i,0xffffffff);
				}
			}
		}
		else
		{
			gvShadowShape = NULL;
		}
 		
		sensorTriangleShape = GVAppearanceType::SensorTriangleShape->CreateFrom();
		//-------------------------------------------------
		// Load the texture and store its handle.
		for (int i=0;i<sensorTriangleShape->GetNumTextures();i++)
		{
			char txmName[1024];
			sensorTriangleShape->GetTextureName(i,txmName,256);

			char texturePath[1024];
			sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
	
			FullPathFileName textureName;
			textureName.init(texturePath,txmName,"");
	
			if (fileExists(textureName))
			{
				if (S_strnicmp(txmName,"a_",2) == 0)
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
					sensorTriangleShape->SetTextureHandle(i,gosTextureHandle);
					sensorTriangleShape->SetTextureAlpha(i,true);
				}
				else
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
					sensorTriangleShape->SetTextureHandle(i,gosTextureHandle);
					sensorTriangleShape->SetTextureAlpha(i,false);
				}
			}
			else
			{
				//PAUSE(("Warning: %s texture name not found",textureName));
				sensorTriangleShape->SetTextureHandle(i,0xffffffff);
			}
		}
 		
		sensorCircleShape = GVAppearanceType::SensorCircleShape->CreateFrom();
 		//-------------------------------------------------
		// Load the texture and store its handle.
		for (int i=0;i<sensorCircleShape->GetNumTextures();i++)
		{
			char txmName[1024];
			sensorCircleShape->GetTextureName(i,txmName,256);

			char texturePath[1024];
			sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
	
			FullPathFileName textureName;
			textureName.init(texturePath,txmName,"");
	
			if (fileExists(textureName))
			{
				if (S_strnicmp(txmName,"a_",2) == 0)
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
					sensorCircleShape->SetTextureHandle(i,gosTextureHandle);
					sensorCircleShape->SetTextureAlpha(i,true);
				}
				else
				{
					DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
					sensorCircleShape->SetTextureHandle(i,gosTextureHandle);
					sensorCircleShape->SetTextureAlpha(i,false);
				}
			}
			else
			{
				//PAUSE(("Warning: %s texture name not found",textureName));
				sensorCircleShape->SetTextureHandle(i,0xffffffff);
			}
		}
    		 
 		Stuff::Vector3D boxCoords[8];
		Stuff::Vector3D nodeCenter = gvShape->GetRootNodeCenter();

		boxCoords[0].x = position.x + gvShape->GetMinBox().x + nodeCenter.x;
		boxCoords[0].y = position.y + gvShape->GetMinBox().z + nodeCenter.z;
		boxCoords[0].z = position.z + gvShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[1].x = position.x + gvShape->GetMinBox().x + nodeCenter.x;
		boxCoords[1].y = position.y + gvShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[1].z = position.z + gvShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[2].x = position.x + gvShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[2].y = position.y + gvShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[2].z = position.z + gvShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[3].x = position.x + gvShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[3].y = position.y + gvShape->GetMinBox().z + nodeCenter.z;
		boxCoords[3].z = position.z + gvShape->GetMaxBox().y + nodeCenter.y;
		
		boxCoords[4].x = position.x + gvShape->GetMinBox().x + nodeCenter.x;
		boxCoords[4].y = position.y + gvShape->GetMinBox().z + nodeCenter.z;
		boxCoords[4].z = position.z + gvShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[5].x = position.x + gvShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[5].y = position.y + gvShape->GetMinBox().z + nodeCenter.z;
		boxCoords[5].z = position.z + gvShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[6].x = position.x + gvShape->GetMaxBox().x + nodeCenter.x;
		boxCoords[6].y = position.y + gvShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[6].z = position.z + gvShape->GetMinBox().y + nodeCenter.y;
		
		boxCoords[7].x = position.x + gvShape->GetMinBox().x + nodeCenter.x;
		boxCoords[7].y = position.y + gvShape->GetMaxBox().z + nodeCenter.z;
		boxCoords[7].z = position.z + gvShape->GetMinBox().y + nodeCenter.y;
		
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
			appearType->typeUpperLeft = gvShape->GetMinBox();
			appearType->typeLowerRight = gvShape->GetMaxBox();

			//Now expand box by some percentage to make selection easier.
			appearType->typeUpperLeft.x *= EXPAND_FACTOR;
			appearType->typeUpperLeft.y *= EXPAND_FACTOR;
			appearType->typeUpperLeft.z *= EXPAND_FACTOR;

			appearType->typeLowerRight.x *= EXPAND_FACTOR;
			appearType->typeLowerRight.y *= EXPAND_FACTOR;
			appearType->typeLowerRight.z *= EXPAND_FACTOR;
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
			
		if (!InEditor)
		{
			if (!dustCloud && useNonWeaponEffects)
			{
				if (strcmp(weaponEffects->GetEffectName(VEHICLE_DUST_CLOUD),"NONE") != 0)
				{
					//--------------------------------------------
					// Yes, load it on up.
					unsigned flags = gosFX::Effect::ExecuteFlag|gosFX::Effect::LoopFlag;
		
					Check_Object(gosFX::EffectLibrary::Instance);
					gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(VEHICLE_DUST_CLOUD));
					
					if (gosEffectSpec)
					{
						dustCloud = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
						gosASSERT(dustCloud != NULL);
						
						MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void GVAppearance::setObjStatus (long oStatus)
{
	if (status != oStatus)
	{
		if ((oStatus == OBJECT_STATUS_DESTROYED) || (oStatus == OBJECT_STATUS_DISABLED))
		{
			if (appearType->gvDmgShape)
			{
				gvShape->ClearAnimation();
				delete gvShape;
				gvShape = NULL;
				
				gvShape = appearType->gvDmgShape->CreateFrom();
			}

			currentLOD = 0;
		}
		
		if (oStatus == OBJECT_STATUS_NORMAL)
		{
			if (appearType->gvShape[0])
			{
				delete gvShape;
				gvShape = NULL;
				
				gvShape = appearType->gvShape[0]->CreateFrom();
			}

			currentLOD = 0;
		}
		
		//-------------------------------------------------
		// Load the texture and store its handle.
		for (long i=0;i<gvShape->GetNumTextures();i++)
		{
			char txmName[1024];
			gvShape->GetTextureName(i,txmName,256);

			char texturePath[1024];
			sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
	
			FullPathFileName textureName;
			textureName.init(texturePath,txmName,"");
	
			if (fileExists(textureName))
			{
				if (S_strnicmp(txmName,"a_",2) == 0)
				{
					DWORD gosTextureHandle = 0;
					
					if (!i)
					{
						localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink,true);
						gvShape->SetTextureHandle(i,localTextureHandle);
						gvShape->SetTextureAlpha(i,true);
					}
					else
					{
						gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
						gvShape->SetTextureHandle(i,gosTextureHandle);
						gvShape->SetTextureAlpha(i,true);
					}
					
				}
				else
				{
					DWORD gosTextureHandle = 0;
					
					if (!i)
					{
						localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink,true);
						gvShape->SetTextureHandle(i,localTextureHandle);
						gvShape->SetTextureAlpha(i,false);
					}
					else
					{
						gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
						gvShape->SetTextureHandle(i,gosTextureHandle);
						gvShape->SetTextureAlpha(i,false);
					}
				}
			}
			else
			{
				//PAUSE(("Warning: %s texture name not found",textureName));
				gvShape->SetTextureHandle(i,0xffffffff);
			}
		}
		
		DWORD r, g, b;
		getPaintScheme(r, g, b);
		resetPaintScheme(r, g, b);
	}
	
	status = oStatus;
}

//-----------------------------------------------------------------------------
void GVAppearance::setPaintScheme (void)
{
	//----------------------------------------------------------------------------
	// Simple really.  Get the texture memory, apply the paint scheme, let it go!
	DWORD gosHandle = mcTextureManager->get_gosTextureHandle(gvShape->GetTextureHandle(0));

	if (gosHandle && gosHandle != 0xffffffff)
	{
		//-------------------
		// Lock the texture.
		TEXTUREPTR textureData;
		gos_LockTexture(gosHandle, 0, 0, &textureData);

		//-------------------------------------------------------
		DWORD *textureMemory = textureData.pTexture;
		for (long i=0;i<textureData.Height;i++)
		{
			for (long j=0;j<textureData.Height;j++)
			{
				//---------------------------------------------
				// Make Color from PaintScheme.
				DWORD baseColor = *textureMemory;
				BYTE baseColorAlpha = ((baseColor & 0xff000000)>>24);
				float baseColorRed = float((baseColor & 0x00ff0000)>>16);
				float baseColorGreen = float((baseColor & 0x0000ff00)>>8);
				float baseColorBlue = float(baseColor & 0x000000ff);

				DWORD newColor = *textureMemory;	//Black by default.
				if ((!baseColorGreen) && (!baseColorBlue))
				{
					baseColorRed *= 0.00390625f;		//Divide by 256;
					baseColorRed = 1.0 - baseColorRed;
					baseColorRed *= baseColorRed;	//Log colors
					baseColorRed = 1.0 - baseColorRed;

					float newColorRed = float((psRed & 0x00ff0000)>>16);
					newColorRed  *= baseColorRed;
					unsigned char red = (unsigned char)newColorRed;

					float newColorGreen = float((psRed & 0x0000ff00)>>8);
					newColorGreen  *= baseColorRed;
					unsigned char green = (unsigned char)newColorGreen;

					float newColorBlue = float(psRed & 0x000000ff);
					newColorBlue  *= baseColorRed;
					unsigned char blue = (unsigned char)newColorBlue;

					newColor = (baseColorAlpha<<24) + (red<<16) + (green<<8) + (blue);
				}
				else if ((!baseColorRed) && (!baseColorBlue))
				{
					baseColorGreen *= 0.00390625f;		//Divide by 256;
					baseColorGreen = 1.0 - baseColorGreen;
					baseColorGreen *= baseColorGreen;	//Log colors
					baseColorGreen = 1.0 - baseColorGreen;

					float newColorRed = float((psGreen & 0x00ff0000)>>16);
					newColorRed  *= baseColorGreen;
					unsigned char red = (unsigned char)newColorRed;

					float newColorGreen = float((psGreen & 0x0000ff00)>>8);
					newColorGreen  *= baseColorGreen;
					unsigned char green = (unsigned char)newColorGreen;

					float newColorBlue = float(psGreen & 0x000000ff);
					newColorBlue  *= baseColorGreen;
					unsigned char blue = (unsigned char)newColorBlue;

					newColor = (baseColorAlpha<<24) + (red<<16) + (green<<8) + (blue);
				}
				else if ((!baseColorRed) && (!baseColorGreen))
				{
					baseColorBlue *= 0.00390625f;		//Divide by 256;
					baseColorBlue = 1.0 - baseColorBlue;
					baseColorBlue *= baseColorBlue;		//Log colors
					baseColorBlue = 1.0 - baseColorBlue;

					float newColorRed = float((psBlue & 0x00ff0000)>>16);
					newColorRed  *= baseColorBlue;
					unsigned char red = (unsigned char)newColorRed;

					float newColorGreen = float((psBlue & 0x0000ff00)>>8);
					newColorGreen  *= baseColorBlue;
					unsigned char green = (unsigned char)newColorGreen;

					float newColorBlue = float(psBlue & 0x000000ff);
					newColorBlue  *= baseColorBlue;
					unsigned char blue = (unsigned char)newColorBlue;

					newColor = (baseColorAlpha<<24) + (red<<16) + (green<<8) + (blue);
				}

				*textureMemory = newColor;
				textureMemory++;
			}
		}

		//------------------------
		// Unlock the texture
		gos_UnLockTexture(gosHandle);
	}
}	

//---------------------------------------------------------------------------
DWORD bgrTorgb (DWORD frontRGB);

//-----------------------------------------------------------------------------
void GVAppearance::setPaintScheme (DWORD mcRed, DWORD mcGreen, DWORD mcBlue)
{
#ifdef BGR
	// These come into here bgr instead of RGB.  CONVERT!
	psRed = bgrTorgb(mcRed);
	psBlue = bgrTorgb(mcBlue);
	psGreen = bgrTorgb(mcGreen);
#else /*BGR*/
	psRed = mcRed;
	psBlue = mcBlue;
	psGreen = mcGreen;
#endif /*BGR*/

	setPaintScheme();	
}	

//-----------------------------------------------------------------------------
void GVAppearance::getPaintScheme( DWORD& red, DWORD& green, DWORD& blue )
{
#ifdef BGR
	red = bgrTorgb(psRed);
	blue = bgrTorgb(psBlue);
	green = bgrTorgb(psGreen);
#else /*BGR*/
	red = psRed;
	blue = psBlue;
	green = psGreen;
#endif /*BGR*/
}

//-----------------------------------------------------------------------------
void GVAppearance::resetPaintScheme (DWORD red, DWORD green, DWORD blue)
{
	//---------------------------------------------------------------------------------
	// Simple really.  Toss the current texture, reload the RGB and reapply the colors
	
	DWORD gosHandle = mcTextureManager->get_gosTextureHandle(localTextureHandle);
	mcTextureManager->removeTexture(gosHandle);
	
	//-------------------------------------------------
	// Load the texture and store its handle.
	char txmName[1024];
	gvShape->GetTextureName(0,txmName,256);

   	char texturePath[1024];
   	sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);

   	FullPathFileName textureName;
   	textureName.init(texturePath,txmName,"");

	//DWORD paintInstance = (red << 16) + (green << 8) + (blue);
	/* The texture manager asks for a unique 32bit identifier for every texture instance.
	However, it requires 72 bits to fully describe a mech texture (the base color (stored in
	the variable "red"), highlight color1 (blue), and highlight color2 (green), each of which
	is a 24bit number made up of 8bit r, g, and b components). Instead of creating a
	mapping between the mech textures used (probably less than 2^32 of them) and
	32bit identifiers, for the sake of expediency I'm just taking the 3 most significant bits of
	the 9 rgb components to make a 27 bit identifier. This means that two mech textures
	that are close in color (i.e. all of the 3 most significant bits of the 9 rgb components are
	the same) will be treated as the same texture, which is not necessarily a bad thing in
	our case.  */
	DWORD ccbase = ((red >> 5) & 7) + (((red >> 13) & 7) << 3) + (((red >> 21) & 7) << 6);
	DWORD cchighlight1 = ((green >> 5) & 7) + (((green >> 13) & 7) << 3) + (((green >> 21) & 7) << 6);
	DWORD cchighlight2 = ((blue >> 5) & 7) + (((blue >> 13) & 7) << 3) + (((blue >> 21) & 7) << 6);
	DWORD paintInstance = (ccbase << 18) + (cchighlight1 << 9) + (cchighlight2);
	
	if (fileExists(textureName))
	{
		if (S_strnicmp(txmName,"a_",2) == 0)
		{
			DWORD textureInstanceAlreadyExists = mcTextureManager->textureInstanceExists(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink,paintInstance);
			if (!textureInstanceAlreadyExists)
				localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink,paintInstance);
			else
				localTextureHandle = textureInstanceAlreadyExists;
				
			gvShape->SetTextureHandle(0,localTextureHandle);
			gvShape->SetTextureAlpha(0,true);
			
			if (textureInstanceAlreadyExists)
			{
				/* In this case, the texture returned should already have the paint scheme
				applied. */
	
				//Still need to store psRed/psGreen/psBlue!!!!
				psRed = red;
				psGreen = green;
				psBlue = blue;
				return;
			}
		}
		else
		{
			DWORD textureInstanceAlreadyExists = mcTextureManager->textureInstanceExists(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink,paintInstance);
			if (!textureInstanceAlreadyExists)
				localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink,paintInstance);
			else
				localTextureHandle = textureInstanceAlreadyExists;
				
 			gvShape->SetTextureHandle(0,localTextureHandle);
			gvShape->SetTextureAlpha(0,false);
			if (textureInstanceAlreadyExists)
			{
				/* In this case, the texture returned should already have the paint scheme
				applied. */
	
				//Still need to store psRed/psGreen/psBlue!!!!
				psRed = red;
				psGreen = green;
				psBlue = blue;
				return;
			}
		}
	}
	else
	{
		//PAUSE(("Warning: %s texture name not found",textureName));
		gvShape->SetTextureHandle(0,0xffffffff);
	}
	
	setPaintScheme(red,green,blue);
}	

//-----------------------------------------------------------------------------
void GVAppearance::setGesture (unsigned long gestureId)
{
	//------------------------------------------------------------
	// Check if state is possible.
	if (gestureId >= MAX_GV_ANIMATIONS)
		return;

	//------------------------------------------------------------
	// Check if object destroyed.  If so, no animation!
	if ((status == OBJECT_STATUS_DESTROYED) || (status == OBJECT_STATUS_DISABLED))
		return;
		
	//----------------------------------------------------------------------
	// If state is OK, set animation data, set first frame, set loop and 
	// reverse flag, and start it going until you hear otherwise.
	appearType->setAnimation(gvShape,gestureId);
	gvAnimationState = gestureId;
	currentFrame = 0.0f;
	if (appearType->gvStartF[gestureId])
		currentFrame = appearType->gvStartF[gestureId];
		
	isReversed = false;
	
	if (appearType->isReversed(gvAnimationState))
	{
		currentFrame = appearType->getNumFrames(gvAnimationState)-1;
		isReversed = true;
	}
	
	if (appearType->isRandom(gvAnimationState))
	{
		currentFrame = RandomNumber(appearType->getNumFrames(gvAnimationState)-1);
	}
	
	isLooping = appearType->isLooped(gvAnimationState);
	
	gvFrameRate = appearType->getFrameRate(gvAnimationState);
	
	setFirstFrame = true;
	canTransition = false;
}

//-----------------------------------------------------------------------------
Stuff::Vector3D GVAppearance::getHitNode (void)
{
	if (hitNodeId == -1)
		hitNodeId = gvShape->GetNodeNameId("hitnode");

	Stuff::Vector3D result = getNodeIdPosition(hitNodeId);
 	return result;
}

//-----------------------------------------------------------------------------
void GVAppearance::setObjectParameters (const Stuff::Vector3D &pos, float Rot, long sel, long team, long homeRelations)
{
	movedThisFrame = false;
	if ((rotation != Rot) || (pos != position))
		movedThisFrame = true;
	
	rotation = Rot;
	turretRotation = Rot;

	position = pos;

	selected = sel;

	actualRotation = Rot;

	teamId = team;
	homeTeamRelationship = homeRelations;
}

//-----------------------------------------------------------------------------
void GVAppearance::setMoverParameters (float turretRot, float lArmRot, float rArmRot, bool isAirborne)
{
	turretRotation = turretRot;
	pitch = lArmRot;
	roll = rArmRot;
	isInfantry = isAirborne;
}	

//-----------------------------------------------------------------------------
void GVAppearance::debugUpdate (void)
{
	if (!inDebugMoveMode)
		return;

	//----------------------------------------
	// Adjust mechDebugAngle based on Input
	if (userInput->getKeyDown(KEY_LEFT) && userInput->ctrl())
	{
		vehicleDebugAngle += 11.25;
		if (vehicleDebugAngle > 180.0)
			vehicleDebugAngle -= 360;
	}

	if (userInput->getKeyDown(KEY_RIGHT) && userInput->ctrl())
	{
		vehicleDebugAngle -= 11.25;
		if (vehicleDebugAngle < -180.0)
			vehicleDebugAngle += 360.0;
	}

	//----------------------------------------
	// Adjust torsoDebugAngle based on Input
	if (userInput->getKeyDown(KEY_UP) && userInput->ctrl())
	{
		turretDebugAngle += 11.25;
		if (turretDebugAngle > 180.0)
			turretDebugAngle -= 360;
	}

	if (userInput->getKeyDown(KEY_DOWN) && userInput->ctrl())
	{
		turretDebugAngle -= 11.25;
		if (turretDebugAngle < -180.0)
			turretDebugAngle += 360.0;
	}

	//------------------------------------------
	// Adjust Speed Based on Input
	if (userInput->getKeyDown(KEY_EQUALS) && userInput->ctrl())
	{
		debugVelMag += 10.0;
	}
						
	if (userInput->getKeyDown(KEY_MINUS) && userInput->ctrl())
	{
		debugVelMag -= 10.0;
	}

	if (userInput->getKeyDown(KEY_0) && userInput->ctrl())
	{
		debugVelMag = 0.0;
	}

	//------------------------------------------------------------------
	// Adjust position based on vehicle Velocity which is based on gesture
	Stuff::Vector3D velocity;
	velocity.x = 0.7071f;
	velocity.z = 0.0;
	velocity.y = -0.7071f;

	Rotate(velocity,-vehicleDebugAngle);

	float velMag = debugVelMag;

	//-----------------------------------------
	// Take slope being walked on into account.
	// Use for ground vehicles for sure.
	Stuff::Vector3D currentNormal = land->getTerrainNormal(debugGVActorPosition);
	float angle = angle_from(velocity,currentNormal);
	if (angle != 90.0)
	{
		float hillFactor = cos(angle * DEGREES_TO_RADS) * velMag;
		velMag += hillFactor;
	}

	velocity *= velMag * worldUnitsPerMeter;

	velocity *= frameLength;

	debugGVActorPosition += velocity;
	debugGVActorPosition.z = land->getTerrainElevation(debugGVActorPosition);

	setObjectParameters(debugGVActorPosition,vehicleDebugAngle,true,0, 0);
	update();
	recalcBounds();
}

//-----------------------------------------------------------------------------
bool GVAppearance::isMouseOver (float px, float py)
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
bool GVAppearance::recalcBounds (void)
{
	Stuff::Vector4D tempPos;
	inView = false;

	float eyeDistance = 0.0f;

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
			Stuff::Point3D objPosition;
			Stuff::Point3D eyePosition(eye->getCameraOrigin());
			objPosition.x = -position.x;
			objPosition.y = position.z;
			objPosition.z = position.y;
	
			Distance.Subtract(objPosition,eyePosition);
			eyeDistance = Distance.GetApproximateLength();
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
								if (appearType->gvShape[i] && (eyeDistance > appearType->lodDistance[i]))
								{
									baseLOD = false;
									selectLOD = i;
								}
							}
						}
						else	//We always want to use the lowest LOD!!
						{
							if (appearType->gvShape[1])
							{
								baseLOD = false;
								selectLOD = 1;
							}
						}
						
						// we are at this LOD level.
						if (selectLOD != currentLOD)
						{
							currentLOD = selectLOD;

							BYTE alphaValue = gvShape->GetAlphaValue();
							gvShape->ClearAnimation();
							delete gvShape;
							gvShape = NULL;

							gvShape = appearType->gvShape[currentLOD]->CreateFrom();
							if (gvAnimationState != -1)
								appearType->setAnimation(gvShape,gvAnimationState);

							gvShape->SetAlphaValue(alphaValue);

							//-------------------------------------------------
							// Load the texture and store its handle.
							for (long j=0;j<gvShape->GetNumTextures();j++)
							{
								char txmName[1024];
								gvShape->GetTextureName(j,txmName,256);

								char texturePath[1024];
								sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);

								FullPathFileName textureName;
								textureName.init(texturePath,txmName,"");

								if (fileExists(textureName))
								{
									if (S_strnicmp(txmName,"a_",2) == 0)
									{
										DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink);
										gosASSERT(gosTextureHandle != 0xffffffff);
										gvShape->SetTextureHandle(j,gosTextureHandle);
										gvShape->SetTextureAlpha(j,true);
									}
									else
									{
										DWORD gosTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink);
										gosASSERT(gosTextureHandle != 0xffffffff);
										gvShape->SetTextureHandle(j,gosTextureHandle);
										gvShape->SetTextureAlpha(j,false);
									}
								}
								else
								{
									//PAUSE(("Warning: %s texture name not found",textureName));
									gvShape->SetTextureHandle(j,0xffffffff);
								}
							}
							DWORD r, g, b;
							getPaintScheme(r, g, b);
							resetPaintScheme(r,g,b);
						}

						//ONLY change if we need
						if (currentLOD && baseLOD)
						{
						// we are at the Base LOD level.
							currentLOD = 0;
							
							BYTE alphaValue = gvShape->GetAlphaValue();
							gvShape->ClearAnimation();
							delete gvShape;
							gvShape = NULL;
							
							gvShape = appearType->gvShape[currentLOD]->CreateFrom();
							gvShape->SetAlphaValue(alphaValue);
							
							//-------------------------------------------------
							// Load the texture and store its handle.
							for (long i=0;i<gvShape->GetNumTextures();i++)
							{
								char txmName[1024];
								gvShape->GetTextureName(i,txmName,256);
										
								char texturePath[1024];
								sprintf(texturePath,"%s%d" PATH_SEPARATOR, tglPath, ObjectTextureSize);
						
								FullPathFileName textureName;
								textureName.init(texturePath,txmName,"");
										
								if (fileExists(textureName))
								{
									if (S_strnicmp(txmName,"a_",2) == 0)
									{
										localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap | gosHint_DontShrink,true);
										gosASSERT(localTextureHandle != 0xffffffff);
										gvShape->SetTextureHandle(i,localTextureHandle);
										gvShape->SetTextureAlpha(i,true);
									}
									else
									{
										localTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Solid,gosHint_DisableMipmap | gosHint_DontShrink,true);
										gosASSERT(localTextureHandle != 0xffffffff);
										gvShape->SetTextureHandle(i,localTextureHandle);
										gvShape->SetTextureAlpha(i,false);
									}
								}
								else
								{
									//PAUSE(("Warning: %s texture name not found",textureName));
									gvShape->SetTextureHandle(i,0xffffffff);
								}
							}

							DWORD r, g, b;
							getPaintScheme(r, g, b);
							resetPaintScheme(r,g,b);
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
bool GVAppearance::playDestruction (void)
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
			
			tPosition.x = -position.x;
			tPosition.y = position.z;
			tPosition.z = position.y;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
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
Stuff::Vector3D GVAppearance::getNodeNamePosition (const char *nodeName)
{
	Stuff::Vector3D result = position;
	
	if (!inView)
		return result;

   	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = position.z;
   	xlatPosition.z = position.y;
   
   	Stuff::UnitQuaternion torsoRot;
   	torsoRot = Stuff::EulerAngles(0.0f,(turretRotation * DEGREES_TO_RADS),0.0f);
	if (rotationalNodeIndex == -1)
	   	rotationalNodeIndex = gvShape->SetNodeRotation(appearType->rotationalNodeId,&torsoRot);

	gvShape->SetNodeRotation(rotationalNodeIndex,&torsoRot);
   
	result = gvShape->GetTransformedNodePosition(&xlatPosition,&qRotation,nodeName);

	if ((result.x == 0.0f) &&
		(result.y == 0.0f) && 
		(result.z == 0.0f))
		result = position;
		
	return result;
}

//-----------------------------------------------------------------------------
Stuff::Vector3D GVAppearance::getNodeIdPosition (long nodeId)
{
	Stuff::Vector3D result = position;
	
	if (!inView)
		return result;

   	//-------------------------------------------
   	// Create Matrix to conform to.
   	Stuff::UnitQuaternion qRotation;
   	float yaw = rotation * DEGREES_TO_RADS;
   	qRotation = Stuff::EulerAngles(0.0f, yaw, 0.0f);
   
   	Stuff::Point3D xlatPosition;
   	xlatPosition.x = -position.x;
   	xlatPosition.y = position.z;
   	xlatPosition.z = position.y;
   
   	Stuff::UnitQuaternion torsoRot;
   	torsoRot = Stuff::EulerAngles(0.0f,(turretRotation * DEGREES_TO_RADS),0.0f);
	if (rotationalNodeIndex == -1)
	   	rotationalNodeIndex = gvShape->SetNodeRotation(appearType->rotationalNodeId,&torsoRot);

	gvShape->SetNodeRotation(rotationalNodeIndex,&torsoRot);
   
	result = gvShape->GetTransformedNodePosition(&xlatPosition,&qRotation,nodeId);

	if ((result.x == 0.0f) &&
		(result.y == 0.0f) && 
		(result.z == 0.0f))
		result = position;
		
	return result;
}

//-----------------------------------------------------------------------------
long GVAppearance::renderShadows (void)
{
	gvShape->SetTextureHandle(0,localTextureHandle);
	
	if (inView && visible)
	{
		//---------------------------------------------
		// Call Multi-shape render stuff here.
		if (gvShadowShape)
			gvShadowShape->RenderShadows(true);
		else
			gvShape->RenderShadows(true);
	}
	return NO_ERR;
}

//-----------------------------------------------------------------------------
long GVAppearance::render (long depthFixup)
{
	gvShape->SetTextureHandle(0,localTextureHandle);

	if (inView)
	{
		uint32_t color = SD_BLUE;
		uint32_t highLight = 0x007f7f7f;
		if ((teamId > -1) && (teamId < 8)) {
			static uint32_t highLightTable[3] = {0x00007f00, 0x0000007f, 0x007f0000};
			static uint32_t colorTable[3] = {SB_GREEN | 0xff000000, SB_BLUE| 0xff000000, SB_RED | 0xff000000};
			color = colorTable[homeTeamRelationship];
			highLight = highLightTable[homeTeamRelationship];
		}

		if (visible)
		{
			if (selected & DRAW_COLORED && duration <= 0 )
			{
				gvShape->SetARGBHighLight(highLight);
			}
			else
			{
				gvShape->SetARGBHighLight(highlightColor);
			}

			Camera::HazeFactor = hazeFactor;

			if (drawFlash)
			{
				gvShape->SetARGBHighLight(flashColor);
			}

			//---------------------------------------------
			// Call Multi-shape render stuff here.
			// Force textures to reload due to unique instance.
			gvShape->Render(true);

			if (selected & DRAW_TEXT)
			{
				gvShape->SetARGBHighLight(highLight);
			}
			else
			{
				gvShape->SetARGBHighLight(0x0);
			}

				
			if (selected & DRAW_BARS)
			{
				drawBars();
			}

			if ( selected & DRAW_BRACKETS )
			{
				drawSelectBrackets(color);
			}

			if ( selected & DRAW_TEXT && !sensorLevel )
			{
 
				if (objectNameId != -1)
				{
					char tmpString[255];
					cLoadString(objectNameId, tmpString, 254);					
					drawTextHelp(tmpString, color);
					
				}
				if ( strlen( pilotName ) )
				{
					drawPilotName( pilotName, color );
				}

			}
		
			
			//selected = FALSE;
			
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
				
				tPosition.x = -position.x;
				tPosition.y = position.z;
				tPosition.z = position.y;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(tPosition);
				
				drawInfo.m_parentToWorld = &shapeOrigin;
				
				if (!MLRVertexLimitReached)
					destructFX->Draw(&drawInfo);
			}
			
			if (!sensorLevel)
			{
				if (waterWake && isWaking)
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
							
					Stuff::Point3D wakePos;
					wakePos.x = -position.x;
					wakePos.y = Terrain::waterElevation;
					wakePos.z = position.y;
					
					shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
					shapeOrigin.BuildTranslation(wakePos);
							
					Stuff::UnitQuaternion effectRot;
					effectRot = Stuff::EulerAngles(90.0f * DEGREES_TO_RADS,rotation * DEGREES_TO_RADS,0.0f);
					localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
					localResult.Multiply(localToWorld,shapeOrigin);
		
					drawInfo.m_parentToWorld = &localResult;
					if (!MLRVertexLimitReached)
						waterWake->Draw(&drawInfo);
				}
				else if (dustCloud)
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
							
					if (dustNodeIndex == -1)
						dustNodeIndex = gvShape->GetNodeNameId("dust_body");

					Stuff::Vector3D dustPos = getNodeIdPosition(dustNodeIndex);

					Stuff::Point3D wakePos;
					wakePos.x = -dustPos.x;
					wakePos.y = dustPos.z;
					wakePos.z = dustPos.y;
					
					shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
					shapeOrigin.BuildTranslation(wakePos);
							
					Stuff::UnitQuaternion effectRot;
					effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
					localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
					localResult.Multiply(localToWorld,shapeOrigin);
		
					drawInfo.m_parentToWorld = &localResult;
					if (!MLRVertexLimitReached)
						dustCloud->Draw(&drawInfo);
				}
				
				if (activity && isActivitying)
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
							
					if (activityNodeIndex == -1)
						activityNodeIndex = gvShape->GetNodeNameId("activity_node");

					Stuff::Vector3D dustPos = getNodeIdPosition(activityNodeIndex);
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
				}

			}
		}
		
		if ((sensorLevel > 0) && (sensorLevel < 5))
		{
			//---------------------------------------
			// Draw Sensor Contact here.
			if (sensorLevel > 1)
			{
				sensorCircleShape->Render();
			}
			else
			{
				sensorTriangleShape->Render();
			}
		}
	}
	return NO_ERR;
}

//-----------------------------------------------------------------------------
void GVAppearance::updateGeometry (void)
{
	//if (visible)
	//{
		if (rotation > 180)
			rotation -= 360;
	
		if (rotation < -180)
			rotation += 360;
	
		if (turretRotation > 180)
			turretRotation -= 360;
	
		if (turretRotation < -180)
			turretRotation += 360;
	
		float pitchAngle = pitch;
		float rollAngle = roll;

		if ((status == OBJECT_STATUS_NORMAL || status == OBJECT_STATUS_SHUTDOWN) && land)  //Are we still oK?
		{
			int cellR, cellC;
			land->worldToCell(position,cellR, cellC);
			if (GameMap && !GameMap->getDeepWater(cellR, cellC) && !GameMap->getShallowWater(cellR, cellC))
			{
				Stuff::Vector3D worldK;
				land->getTerrainAngle(position,&worldK);

				//--------------------------------------------------
				// Rotate Angle obtained into Vehicle Rotation.
				Rotate(worldK,rotation);

				//------------------------------------------------
				// Find Pitch first.
				Stuff::Vector3D pitchK;
				pitchK = worldK;
				pitchK.x = 0.0f;
				pitchK.Normalize(pitchK);

				Stuff::Vector3D rollK;
				rollK = worldK;
				rollK.y = 0.0f;
				rollK.Normalize(rollK);

				Stuff::Vector3D up;
				up.x = up.y = 0.0f;
				up.z = 1.0f;

				pitchAngle = up * pitchK;
				pitchAngle = acos(pitchAngle) * RADS_TO_DEGREES;
				if (pitchK.y < 0.0f)
					pitchAngle = -pitchAngle;

				rollAngle = up * rollK;
				rollAngle = acos(rollAngle) * RADS_TO_DEGREES;
				if (rollK.x < 0.0f)
					rollAngle = -rollAngle;
			}
		}
		
		//-------------------------------------------
		// Does math necessary to draw Vehicle
		Stuff::UnitQuaternion rot, pitchRoll;
		float yaw = rotation * DEGREES_TO_RADS;
		pitchAngle *= DEGREES_TO_RADS;
		rollAngle *= DEGREES_TO_RADS;
	
		Stuff::UnitQuaternion totalRotation;
		rot = Stuff::EulerAngles(0.0f, yaw, 0.0f);
	
		pitchRoll = Stuff::EulerAngles(pitchAngle, 0.0f, rollAngle);
		totalRotation.Multiply(pitchRoll,rot);
	
		unsigned char lightr,lightg,lightb;
		float lightIntensity = 1.0f;
		if (land)
			lightIntensity = land->getTerrainLight(position);
							
		lightr = eye->getLightRed(lightIntensity);
		lightg = eye->getLightGreen(lightIntensity);
		lightb = eye->getLightBlue(lightIntensity);
	
		DWORD lightRGB = (lightr<<16) + (lightg<<8) + lightb;
		
		eye->setLightColor(0,lightRGB);
		eye->setLightIntensity(0,1.0);
	
		DWORD fogRGB = 0xff<<24;
		float fogStart = eye->fogStart;
		float fogFull = eye->fogFull;
	
		Stuff::Point3D xlatPosition;
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
	
				unsigned char fogResult = fogFactor;
				fogRGB = fogResult << 24;
			}
		}
		else
		{
			fogRGB = 0xff<<24;
		}
	
		if (useFog)
			gvShape->SetFogRGB(fogRGB);
		else
			gvShape->SetFogRGB(0xffffffff);
	
		Stuff::UnitQuaternion turretRot;
		turretRot = Stuff::EulerAngles(0.0f,(turretRotation * DEGREES_TO_RADS),0.0f);
		if (rotationalNodeIndex == -1)
		   	rotationalNodeIndex = gvShape->SetNodeRotation(appearType->rotationalNodeId,&turretRot);

		gvShape->SetNodeRotation(rotationalNodeIndex,&turretRot);
				 
		if (gvShadowShape)
			gvShape->SetUseShadow(false);
			
		if (gvShadowShape && useShadows)
		{
			gvShadowShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
			gvShadowShape->TransformMultiShape (&xlatPosition,&rot);
		}
		
//		Camera::HazeFactor = hazeFactor;
		gvShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
		gvShape->TransformMultiShape (&xlatPosition,&totalRotation);
	//}
	
	//------------------------------------------------
	// Update GOSFX
	if (destructFX && destructFX->IsExecuted())
	{
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D 	localToWorld;
		Stuff::Point3D			tPosition;
			
		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(tPosition);

		tPosition.x = -position.x;
		tPosition.y = position.z;
		tPosition.z = position.y;
		
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
	
	if (waterWake && isWaking)
	{
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D	localToWorld;
		Stuff::LinearMatrix4D	localResult;
				
		Stuff::Point3D wakePos;
		wakePos.x = -position.x;
		wakePos.y = Terrain::waterElevation;
		wakePos.z = position.y;
		
 		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(wakePos);
				
		Stuff::UnitQuaternion effectRot;
		effectRot = Stuff::EulerAngles(90.0f * DEGREES_TO_RADS,rotation * DEGREES_TO_RADS,0.0f);
		localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
		localResult.Multiply(localToWorld,shapeOrigin);
		
 		Stuff::OBB boundingBox;
		gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,&boundingBox);
		
		waterWake->Execute(&info);
	}
	else if (dustCloud)
	{
		if (movedThisFrame && !isInfantry)
		{
			dustCloud->SetLoopOn();
			dustCloud->SetExecuteOn();
		}
		else
		{
			dustCloud->SetLoopOff();
			dustCloud->SetExecuteOn();
		}
		
		if (!dustCloudStart)
		{
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;
					
			if (dustNodeIndex == -1)
				dustNodeIndex = gvShape->GetNodeNameId("dust_body");

			Stuff::Vector3D dustPos = getNodeIdPosition(dustNodeIndex);

			Stuff::Point3D wakePos;
			wakePos.x = -dustPos.x;
			wakePos.y = dustPos.z;
			wakePos.z = dustPos.y;
 			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(wakePos);
					
			Stuff::UnitQuaternion effectRot;
			effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
			localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
				
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,NULL);
	
			dustCloud->Start(&info);
			dustCloudStart = true;
		}
	
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D	localToWorld;
		Stuff::LinearMatrix4D	localResult;
				
		if (dustNodeIndex == -1)
			dustNodeIndex = gvShape->GetNodeNameId("dust_body");

		Stuff::Vector3D dustPos = getNodeIdPosition(dustNodeIndex);

		Stuff::Point3D wakePos;
		wakePos.x = -dustPos.x;
		wakePos.y = dustPos.z;
		wakePos.z = dustPos.y;
		
 		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(wakePos);
				
		Stuff::UnitQuaternion effectRot;
		effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
		localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
		localResult.Multiply(localToWorld,shapeOrigin);
		
 		Stuff::OBB boundingBox;
		gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,&boundingBox);
		
		dustCloud->Execute(&info);
	}
	
	if (activity && isActivitying)
	{
 		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D	localToWorld;
		Stuff::LinearMatrix4D	localResult;
				
		if (activityNodeIndex == -1)
			activityNodeIndex = gvShape->GetNodeNameId("activity_node");

		Stuff::Vector3D dustPos = getNodeIdPosition(activityNodeIndex);

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
	}
	
	sensorSpin += SPIN_RATE * frameLength;
	if (sensorSpin > 180)
		sensorSpin -= 360;

	if (sensorSpin < -180)
		sensorSpin += 360;

	totalRotation = Stuff::EulerAngles(0.0f,sensorSpin * DEGREES_TO_RADS,0.0f);
	//----------------------------------------------
	// Do geometry here to draw sensor contact
	sensorTriangleShape->SetFogRGB(0xffffffff);
	sensorTriangleShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
	sensorTriangleShape->TransformMultiShape (&xlatPosition,&totalRotation);
		
	//----------------------------------------------
	// Do geometry here to draw sensor contact
	sensorCircleShape->SetFogRGB(0xffffffff);
	sensorCircleShape->SetLightList(eye->getWorldLights(),eye->getNumLights());
	sensorCircleShape->TransformMultiShape (&xlatPosition,&totalRotation);
}

//-----------------------------------------------------------------------------
long GVAppearance::update (bool animate) 
{
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

	//Always override with our local instance.
	gvShape->SetTextureHandle(0,localTextureHandle);
 
	if ((status == OBJECT_STATUS_DESTROYED) || (status == OBJECT_STATUS_DISABLED))
	{
		gvShape->SetLightsOut(true);
	}
	
 	if (animate && gvFrameRate != 0.0f)
	{
		//--------------------------------------------------------
		// Make sure animation runs no faster than bdFrameRate fps.
		float frameInc = gvFrameRate * frameLength;
		
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
			if (currentFrame >= appearType->getNumFrames(gvAnimationState))
			{
				if (isLooping)
					currentFrame -= appearType->getNumFrames(gvAnimationState);
				else
					currentFrame = appearType->getNumFrames(gvAnimationState) - 1;
					
				canTransition = true;		//Whenever we have completed one cycle or at last frame, OK to move on!
			}
			
	
			//--------------------------------------
			//Check negative overflow of gesture
			if (currentFrame < 0)
			{
				if (isLooping)
					currentFrame += appearType->getNumFrames(gvAnimationState); 
				else
					currentFrame = 0.0f; 
					
				canTransition = true;		//Whenever we have completed one cycle or at last frame, OK to move on!
			}
		}
		
		gvShape->SetFrameNum(currentFrame);
	}

	if ((turn < 3) || inView)
		updateGeometry();
		
	return TRUE;
}

//-----------------------------------------------------------------------------
void GVAppearance::destroy (void)
{
	if (gvShape)
	{
		delete gvShape;
		gvShape = NULL;
	}
	
	if (gvShadowShape)
	{
		delete gvShadowShape;
		gvShadowShape = NULL;
	}
	
	if (sensorCircleShape)
	{
		delete sensorCircleShape;
		sensorCircleShape = NULL;	
	}
	   
	if (sensorTriangleShape)
	{
		delete sensorTriangleShape;
		sensorTriangleShape = NULL;	
	}

	if (destructFX)
	{
		destructFX->Kill();
		delete destructFX;
		destructFX = NULL;
	}

	if (waterWake)
	{
		waterWake->Kill();
		delete waterWake;
		waterWake = NULL;
	}

	if (dustCloud)
	{
		dustCloud->Kill();
		delete dustCloud;
		dustCloud = NULL;
	}

	if (activity)
	{
		activity->Kill();
		delete activity;
		activity = NULL;
	}

	appearanceTypeList->removeAppearance(appearType);
}

//-----------------------------------------------------------------------------
void GVAppearance::startWaterWake (void)
{
	//Check if we are already playing one.  If not, wake city.
	
	//First, check if its even loaded.
	// can easily preload this.  Should we?  Memory?
	if (!waterWake && useNonWeaponEffects)
	{
   		if (strcmp(weaponEffects->GetEffectName(VEHICLE_WATER_WAKE),"NONE") != 0)
   		{
			//--------------------------------------------
			// Yes, load it on up.
			unsigned flags = gosFX::Effect::ExecuteFlag|gosFX::Effect::LoopFlag;

			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(VEHICLE_WATER_WAKE));
			
			if (gosEffectSpec)
			{
				waterWake = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
				gosASSERT(waterWake != NULL);
				
  				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			}
		}
	}
	
	if (waterWake && !isWaking)		//Start the effect if we are not running it yet!!
	{
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D	localToWorld;
		Stuff::LinearMatrix4D	localResult;
				
		Stuff::Point3D wakePos;
		wakePos.x = -position.x;
		wakePos.y = Terrain::waterElevation;	//Wake is at Water Level!
		wakePos.z = position.y;
		
 		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(wakePos);
				
		Stuff::UnitQuaternion effectRot;
		effectRot = Stuff::EulerAngles(90.0f * DEGREES_TO_RADS,rotation * DEGREES_TO_RADS,0.0f);
		localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
		localResult.Multiply(localToWorld,shapeOrigin);
			
 		gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,NULL);

		waterWake->Start(&info);

		isWaking = true;
	}
}

//-----------------------------------------------------------------------------
void GVAppearance::stopWaterWake (void)
{
	if (waterWake && isWaking)		//Stop the effect if we are running it!!
	{
		waterWake->Kill();
	}
	
	isWaking = false;
}

//-----------------------------------------------------------------------------
void GVAppearance::startActivity (long effectId, bool loop)
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
				
  				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			}
		}
	}
	
	if (!isActivitying && activity)		//Start the effect if we are not running it yet!!
	{
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D	localToWorld;
		Stuff::LinearMatrix4D	localResult;
		
		if (activityNodeIndex == -1)
			activityNodeIndex = gvShape->GetNodeNameId("activity_node");

		Stuff::Vector3D nodePos = getNodeIdPosition(activityNodeIndex);

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
		
		isActivitying = true;
	}
}

//-----------------------------------------------------------------------------
void GVAppearance::stopActivity (void)
{
	if (activity && isActivitying)		//Stop the effect if we are running it!!
	{
		activity->Kill();
	}
	
	isActivitying = false;
}

//-----------------------------------------------------------------------------
bool GVAppearance::PerPolySelect (long mouseX, long mouseY)
{
	return gvShape->PerPolySelect(mouseX,mouseY);
}

//-----------------------------------------------------------------------------
void GVAppearance::flashBuilding (float dur, float fDuration, DWORD color)
{
	duration = dur;
	flashDuration = fDuration;
	flashColor = color;
	drawFlash = true;
	currentFlash = flashDuration;
}


//*****************************************************************************
