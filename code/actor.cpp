//---------------------------------------------------------------------------
//
//	actor.cpp - This file contains the code for the Actor class
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef ACTOR_H
#include"actor.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef SPRTMGR_H
#include"sprtmgr.h"
#endif

#ifndef DBASEGUI_H
#include"dbasegui.h"
#endif

//-----------------------------------------------------------------------------
void memclear(void *Dest,int Length);

//-----------------------------------------------------------------------------
// class VFXAppearanceType
void VFXAppearanceType::init (FilePtr apprFile, unsigned long fileSize)
{
	loadIniFile(apprFile,fileSize);

	//------------------------------------------------------------------------------------
	// Once the Ini File is loaded, we need to create the Shape set for this type.
	numPackets = spriteManager->getShapePackets(getAppearanceFileNum());

	textureList = (TGATexturePtr *)spriteManager->mallocDataRAM(sizeof(TGATexture *)*numPackets);
	gosASSERT(textureList != NULL);

	memclear(textureList,sizeof(TGATexture *)*numPackets);
}

//---------------------------------------------------------------------------
void VFXAppearanceType::removeTexture (TGATexturePtr texture)		//Cache texture out
{
	for (long i=0;i<numPackets;i++)
	{
		if (textureList[i] == texture)
		{
			textureList[i] = NULL;
		}
	}

	AppearanceRecord *ourUsers = users;
	while (ourUsers)
	{
		VFXAppearance *thisAppearance = (VFXAppearance *)ourUsers->appear;

		if (thisAppearance->currentTexture == texture)
		{
			thisAppearance->currentTexture = NULL;
		}

		ourUsers = ourUsers->next;
	}
}

//---------------------------------------------------------------------------
long VFXAppearanceType::loadIniFile (FilePtr apprFile, unsigned long fileSize)
{
	FitIniFile VFXAppearanceFile;
	long result = VFXAppearanceFile.open(apprFile,fileSize);
	gosASSERT(result == NO_ERR);

	//-----------------
	// States section
	result = VFXAppearanceFile.seekBlock("States");
	gosASSERT(result == NO_ERR);

	result = VFXAppearanceFile.readIdUChar("NumStates", numStates);
	gosASSERT(result == NO_ERR);

	actorStateData = (ActorData *)spriteManager->mallocDataRAM(sizeof(ActorData)*numStates);
	gosASSERT(actorStateData != NULL);
	memclear(actorStateData,sizeof(ActorData)*MAX_ACTOR_STATES);
	
	for (long curState = 0; curState < numStates; curState++) 
	{
		char stateBlockName[20];
		sprintf(stateBlockName,"State%d",curState);

		result = VFXAppearanceFile.seekBlock(stateBlockName);
		gosASSERT(result == NO_ERR);

		unsigned char tempState;
		result = VFXAppearanceFile.readIdUChar("State", tempState);
		gosASSERT(result == NO_ERR);
			
		actorStateData[curState].state = (ActorState)tempState;

		result = VFXAppearanceFile.readIdULong("NumFrames", actorStateData[curState].numFrames);
		gosASSERT(result == NO_ERR);

		result = VFXAppearanceFile.readIdFloat("FrameRate", actorStateData[curState].frameRate);
		gosASSERT(result == NO_ERR);
			
		result = VFXAppearanceFile.readIdULong("BasePacketNumber", actorStateData[curState].basePacketNumber);
		gosASSERT(result == NO_ERR);
			
		result = VFXAppearanceFile.readIdUChar("NumRotations", actorStateData[curState].numRotations);
		gosASSERT(result == NO_ERR);

		result = VFXAppearanceFile.readIdUChar("Symmetrical", actorStateData[curState].symmetrical);
		gosASSERT(result == NO_ERR);
			
		result = VFXAppearanceFile.readIdLong("TextureSize", actorStateData[curState].textureSize);
		gosASSERT(result == NO_ERR);
		
		result = VFXAppearanceFile.readIdLong("TextureHS", actorStateData[curState].textureHS);
		gosASSERT(result == NO_ERR);
	}

	VFXAppearanceFile.close();

	return(NO_ERR);
}

//-----------------------------------------------------------------------------
// This function is the meat and potatoes of this class.  The Mech actor class
// will use this function to find the gesture it is currently interested in
// drawing.  This function will handle ALL caching.
TGATexturePtr VFXAppearanceType::getTexture (ActorState shapeId, long rot, long currFrame, float &frameRate, bool &mirror)
{
	bool mirrorOn = FALSE;
	mirror = FALSE;
	
	//---------------------------------------------------------------------------------
	// If the NewShape does not exist for this sprite, it's frame count should be set to
	// zero.  This is probably bad, since the sprite will disappear from the screen.
	if (actorStateData[shapeId].numFrames == 0)
		return(NULL);

	if (rot < 0.0 && actorStateData[shapeId].symmetrical)
	{
		rot  = -rot;
		mirrorOn = TRUE;
	}
	else if (rot < 0.0 && !actorStateData[shapeId].symmetrical)
	{
		rot += 360;
	}

	mirror = mirrorOn;
	frameRate = actorStateData[shapeId].frameRate;
	//---------------------------------------------------------------------------------------
	// Remember: sprites use 0 degrees as facing camera, and degrees rotate counter-clockwise
//	long rotation = (long)(rot / (360.0 / (actorStateData[shapeId].numRotations)));
	long rotation = float2short( (1.0/360.0) * (rot * (actorStateData[shapeId].numRotations) ));

	//-------------------------------------------------------------------
	// Using the information provided, figure out which packet we really
	// want and check to see if the Gesture is cached.  If it is, return it
	// If not, check if there is still room in the cache.  If so, load the
	// gesture.  At this point, depending on processor/game load and available
	// memory, we may load some other gestures to help offset a future load.
	// If no memory is left in this Sprite's cache, mark as free any block not
	// used recently.  LRU cache.
	long packetNum = actorStateData[shapeId].basePacketNumber;
	packetNum += rotation * actorStateData[shapeId].numFrames;		//Each frame is its own packet NOW!
	packetNum += currFrame;
	
	//-------------------------------------------------------------------
	// There weren't enough packets in the shape file.  Give em nothing.
	if (packetNum >= numPackets)
		return(NULL);
		
	if (textureList[packetNum])
	{
		((TGATexturePtr)textureList[packetNum])->lastTurnUsed = turn;
		return((TGATexturePtr)textureList[packetNum]);
	}
	
	//-------------------------------------------------
	// We have to cache something in at this point.
	// Be sure the frame length knows about it.
	dynamicFrameTiming = FALSE;
	
	textureList[packetNum] = spriteManager->getTextureData(getAppearanceFileNum(),packetNum,turn,this);
	return((TGATexturePtr)(textureList[packetNum]));
}

//----------------------------------------------------------------------------
void VFXAppearanceType::destroy (void)
{
	//---------------------------------------------------------------------------------------------
	//-- If we are going away, inform ALL of the shapes in the cache that their owner is NULL now.
	//-- Next dumpLRU will purge these FIRST!!
	for (long i=0;i<numPackets;i++)
	{
		if (textureList[i])
		{
			textureList[i]->owner = NULL;
		}
	}

	spriteManager->freeDataRAM(users);
	users = NULL;

	spriteManager->freeDataRAM(textureList);
	textureList = NULL;
	
	spriteManager->freeDataRAM(actorStateData);
	actorStateData = NULL;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class VFXAppearance
void VFXAppearance::init (AppearanceTypePtr tree, GameObjectPtr obj)
{
	Appearance::init(tree,obj);
	appearType = (VFXAppearanceType *)tree;
	
	if (appearType)
		appearType->addUsers(this);

	currentTexture = NULL;
	currentFrame = -1;
	currentRotation = 0;

	inView = FALSE;
	lastInView = 0.0;

	timeInFrame = 0.0;
	lastWholeFrame = 0;

	fadeTable = NULL;
	currentShapeTypeId = ACTOR_STATE_NORMAL;
	
	startFrame = endFrame = -1;

	shapeMin.x = shapeMin.y = -15.0;
	shapeMax.x = shapeMax.y = 15.0;

	changedTypeId = TRUE;
}

//-----------------------------------------------------------------------------
bool VFXAppearance::isMouseOver (float px, float py)
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

#define SELECTED_COLOR 	225
#define TARGET_COLOR	207
extern float currentScaleFactor;
//-----------------------------------------------------------------------------
bool VFXAppearance::recalcBounds (void)
{
	Stuff::Vector4D tempPos;
	inView = FALSE;

	if (eye)
	{
		Stuff::Vector3D topPosition(position);
		topPosition.z += appearType->actorStateData[currentShapeTypeId].textureSize;

		eye->projectZ(topPosition,tempPos);
		topZ = tempPos.z;

		eye->projectZ(position,screenPos);
		tempPos = screenPos;
		
		float scale = eye->getScaleFactor();
		
		upperLeft.x = tempPos.x;
		upperLeft.y = tempPos.y;

		lowerRight.x = upperLeft.x;
		lowerRight.y = upperLeft.y;

		if (currentTexture && currentTexture->textureHandle != 0xffffffff)
		{
			if (!appearType->typeBoundExists())
			{
				//------------------------------------------------------------------------
				// Now, figure out what the upperLeft and lowerRight coordinates are and 
				// draw these as extents if we are selected.
				shapeMin.x = -(appearType->actorStateData[currentShapeTypeId].textureSize >> 1);
				shapeMin.y = shapeMin.x;
				shapeMax.x = -(shapeMin.x);
				shapeMax.y = shapeMax.x;
				
				upperLeft.x = tempPos.x + (shapeMin.x * scale);
				upperLeft.y = tempPos.y + (shapeMin.y * scale);
					
				lowerRight.x = upperLeft.x + (shapeMax.x * scale);
				lowerRight.y = upperLeft.y + (shapeMax.y * scale);
			}
			else
			{
				upperLeft.x = tempPos.x + (appearType->typeUpperLeftX * scale);
				upperLeft.y = tempPos.y + (appearType->typeUpperLeftY * scale);
					
				lowerRight.x = tempPos.x + (appearType->typeLowerRightX * scale);
				lowerRight.y = tempPos.y + (appearType->typeLowerRightY * scale);
			}
		}
			
		if ((lowerRight.x >= 0) && (lowerRight.y >= 0) &&
			(upperLeft.x <= eye->getScreenResX()) &&
			(upperLeft.y <= eye->getScreenResY()))
		{
			inView = TRUE;
		}
	}
	
	return(inView);
}

//-----------------------------------------------------------------------------
void VFXAppearance::setObjectParameters (Stuff::Vector3D &pos, float rot, long sel)
{
	position = pos;
	rotation = rot;
	selected = sel;
}	

extern long mechCmdr1PaletteLookup[];
//-----------------------------------------------------------------------------
long VFXAppearance::render (long depthFixup)
{
	//-------------------------------------------------------------------------
	// First step is figure out where we are and cache the appropriate shapes
	bool oldMirror = FALSE;
	float tFrameRate = 0.0;
	currentTexture = appearType->getTexture(currentShapeTypeId,rotation,currentFrame,tFrameRate,oldMirror);

	if (currentFrame < 0)
		currentFrame = 0;

	TextureElement newElement;
	bool shapesOK = FALSE;

	if (currentTexture && currentTexture->textureHandle != 0xffffffff)
	{
		//------------------------------------------------------------------------
		// Add this shape to element list for sorting.
		newElement.init(currentTexture->textureHandle,screenPos.x,screenPos.y,
						(appearType->actorStateData[currentShapeTypeId].textureHS>>16),
						(appearType->actorStateData[currentShapeTypeId].textureHS & 0x0000ffff),
						appearType->actorStateData[currentShapeTypeId].textureSize,topZ,screenPos.z);

		unsigned char lightr,lightg,lightb, visible, seen;
		lightIntensity = land->getTerrainLight(position, visible, seen);
					
		lightr = eye->getLightRed(lightIntensity,visible,seen);
		lightg = eye->getLightGreen(lightIntensity,visible,seen);
		lightb = eye->getLightBlue(lightIntensity,visible,seen);
					
		DWORD lightRGB = lightb + (lightr<<16) + (lightg << 8) + (0xff << 24);
			
		newElement.setLight(lightRGB);
	
		newElement.draw();
		shapesOK = TRUE;
	}

	gosASSERT(shapesOK);

	if (selected)
		drawBars();

 	//------------------------------------------------------------------------
	if (selected)
	{
		drawSelectBox(SB_BLUE);
	}

	selected = FALSE;

	return NO_ERR;
}

//-----------------------------------------------------------------------------
void VFXAppearance::setDamageLvl (unsigned long dmg)
{
	realBuildingDamage = TRUE;
	
	if (dmg)
	{
		unsigned long totalFrames1 = appearType->actorStateData[ACTOR_STATE_BLOWING_UP1].numFrames;
		unsigned long totalFrames2 = appearType->actorStateData[ACTOR_STATE_BLOWING_UP2].numFrames;
		
		if ((dmg >= (totalFrames1 + totalFrames2)))
		{
			setTypeId(ACTOR_STATE_DESTROYED);
		}
		else if ((dmg >= totalFrames1) && (dmg < (totalFrames1 + totalFrames2)))
		{
			setTypeId(ACTOR_STATE_BLOWING_UP2);
			currentFrame = dmg - totalFrames1;
		}
		else if (dmg < totalFrames1)
		{
			setTypeId(ACTOR_STATE_BLOWING_UP1);
			currentFrame = dmg;
		}
	}
	else
	{
		setTypeId(ACTOR_STATE_NORMAL);
	}
}

//-----------------------------------------------------------------------------
void VFXAppearance::debugUpdate (void)
{
	update();
	recalcBounds();
}

//-----------------------------------------------------------------------------
long VFXAppearance::update (void)
{
	//--------------------------------------------------------
	// Check if we are just starting out.  If so, set
	// the current frame to zero.
	bool startingAnimation = FALSE;
	if (currentFrame == -1)
	{
		currentFrame = 0;
		startingAnimation = TRUE;
	}
		
  	//--------------------------------------------------------------
	// Must update animation frame numbers, even if not visible!!
  	// Make sure animation runs no faster than frameRate fps.
	// Moved to here to make game work.
	unsigned long anyFrames = appearType->actorStateData[currentShapeTypeId].numFrames;

	if (anyFrames > 1)
	{
		long frameInc = 0;
		float frameRate = appearType->actorStateData[currentShapeTypeId].frameRate;
		//------------------------------------------------------
		// Make sure animation runs no faster than frameRate fps.
		float frameRateOverOne = (1.0 / frameRate);
		timeInFrame += frameLength;
		if (timeInFrame >= frameRateOverOne)
		{
			frameInc = timeInFrame * frameRate;
			float remainder = timeInFrame/frameInc - frameRateOverOne;
			timeInFrame = remainder;
		}

		if (frameInc)
		{
  			currentFrame += frameInc;
				
			unsigned long totalFrames = appearType->actorStateData[currentShapeTypeId].numFrames;
			unsigned char loop = 1;
			
			if ((currentFrame >= totalFrames) && (loop) && (endFrame == -1))
			{
				currentFrame %= totalFrames;
				return(FALSE);		//Let the object know we have completed a cycle.
			}
			else if ((currentFrame >= totalFrames) && (!loop))
			{
				currentFrame = totalFrames-1;
			}
			else if (currentFrame >= endFrame)
			{
				currentFrame = startFrame;
				return(FALSE);		//Let the object know we have completed a cycle.
			}
		}
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------
void VFXAppearance::destroy (void) 
{
	appearType->removeUsers(this);
	appearanceTypeList->removeAppearance(appearType);
}

//-----------------------------------------------------------------------------
long VFXAppearance::stateExists (ActorState typeId)
{
	if ((typeId < appearType->numStates) && (typeId >= 0))
		return (appearType->actorStateData[typeId].numFrames);

	return 0;
}

//*****************************************************************************
