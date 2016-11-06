//---------------------------------------------------------------------------
//
//	actor.h - This file contains the header for the Actor class
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef ACTOR_H
#define ACTOR_H
//---------------------------------------------------------------------------
// Include files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef APPEAR_H
#include"appear.h"
#endif

#ifndef APPRTYPE_H
#include"apprtype.h"
#endif

#ifndef TGATXM_H
#include"tgatxm.h"
#endif


//***********************************************************************
// Macro Definitions

#ifndef NO_ERR
#define NO_ERR						0
#endif

#define MAX_TREES		20

enum ActorState
{
	ACTOR_STATE_INVALID = -1,
	ACTOR_STATE_NORMAL = 0,
	ACTOR_STATE_BLOWING_UP1,		//From Normal to Damaged 
	ACTOR_STATE_DAMAGED,
	ACTOR_STATE_BLOWING_UP2,		//From Damaged to Destroyed 
	ACTOR_STATE_DESTROYED,
	ACTOR_STATE_FALLEN_DMG,			//Used for Trees
	MAX_ACTOR_STATES
};

struct ActorData
{
	ActorState			state;
	unsigned char 		symmetrical;					// are second-half rotations flip versions of first half?
	unsigned char		numRotations;					// number of rotations (including flips)
	unsigned long		numFrames;						// number of frames for this gesture (if -1, does not exist)
	unsigned long		basePacketNumber;				// Where in packet file does this gesture start.
	float				frameRate;						// intended frame rate of playback
	long				textureSize;					// Length of one edge of texture.
	long				textureHS;						// Where the screen coord should go for texture.
};

//***********************************************************************
//
// VFXAppearanceType
//
//***********************************************************************

//-----------------------------------------------------------------------
class VFXAppearanceType : public AppearanceType
{
	public:
	
		ActorData			*actorStateData;
		TGATexturePtr		*textureList;				//These go NULL when a texture is cached out.
		long				numPackets;
		DWORD				textureMemoryHandle;
		unsigned char		numStates;

	public:
	
		void init (void)
		{
			actorStateData = NULL;
			textureList = NULL;
			numStates = 0;
			numPackets = 0;

			users = NULL;
			lastUser = NULL;
		}
	
		VFXAppearanceType (void)
		{
			init();
		}

		~VFXAppearanceType (void)
		{
			destroy();
		}

		void init (FilePtr appearFile, unsigned long fileSize);
		
		//----------------------------------------------
		// This routine is where the magic happens.
		TGATexturePtr getTexture (ActorState shapeId, long rot, long currFrame, float &frameRate, bool &mirror);

		long loadIniFile (FilePtr appearFile, unsigned long fileSize);

		void destroy (void);

		virtual void removeTexture (TGATexture *shape);

		long getNumFrames (ActorState typeId)
		{
			if (actorStateData)
				return actorStateData[typeId].numFrames;
				
			return 0;
		}
};

//***********************************************************************
//
// VFXAppearance
//
//***********************************************************************

//-----------------------------------------------------------------------
class VFXAppearance : public Appearance
{
	public:

		VFXAppearanceType*			appearType;
		TGATexturePtr				currentTexture;		//OK because we make sure each frame before we draw it.
		unsigned long				currentFrame;
		float						currentRotation;
		
		float						lastInView;						//Time since last in view (s)
		float						timeInFrame;
		float						frameInc;
		long						lastWholeFrame;
		
		unsigned long				startFrame;
		unsigned long				endFrame;
			
		ActorState					currentShapeTypeId;
		MemoryPtr 					fadeTable;
		bool 						realBuildingDamage;
		bool						changedTypeId;
		
		float						lightIntensity;
		float						topZ;
		
		Stuff::Vector2DOf<long>		shapeMin;
		Stuff::Vector2DOf<long>		shapeMax;

		Stuff::Vector3D				position;
		float						rotation;
		long						selected;
		long						alignment;
		
	public:

		virtual void init (AppearanceTypePtr tree = NULL, GameObjectPtr obj = NULL);

		VFXAppearance (void)
		{
			init();
		}

		virtual long update (void);
		virtual long render (long depthFixup = 0);

		virtual void destroy (void);

		~VFXAppearance (void)
		{
			destroy();
		}

		virtual AppearanceTypePtr getAppearanceType (void)
		{
			return appearType;
		}

		virtual bool recalcBounds (void);
		
		virtual void setTypeId (ActorState typeId)
		{
			if ((typeId < appearType->numStates) && (typeId >= 0))
			{
				currentShapeTypeId = typeId;
				currentFrame = -1;
				timeInFrame = 0.0;
				lastWholeFrame = 0;
			}
			
			changedTypeId = TRUE;
		}

		void loopFrames (long sFrame, long eFrame)
		{
			startFrame = sFrame;
			endFrame = eFrame;
		}
		
		virtual void setDamageLvl (unsigned long damage);
		
		void setFadeTable (MemoryPtr fTable)
		{
			fadeTable = fTable;
		}
		
		void setObjectParameters (const Stuff::Vector3D &pos, float rot, long selected);
		
		long stateExists (ActorState typeId);
		
		void debugUpdate (void);
		
		virtual bool isMouseOver (float px, float py);
};


//***********************************************************************
#endif


