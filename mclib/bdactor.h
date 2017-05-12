//---------------------------------------------------------------------------
//
//	dbactor.h - This file contains the header for the Static ground object appearance class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef BDACTOR_H
#define BDACTOR_H

//---------------------------------------------------------------------------
// Include files
#ifndef APPEAR_H
#include"appear.h"
#endif

#ifndef APPRTYPE_H
#include"apprtype.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif
#ifndef MSL_H
#include"msl.h"
#endif

#ifndef OBJECTAPPEARANCE_H
#include"objectappearance.h"
#endif

#include<gosfx/gosfxheaders.hpp>
//**************************************************************************************
#ifndef NO_ERR
#define NO_ERR						0
#endif

#define MAX_BD_ANIMATIONS			10
//***********************************************************************
//
// BldgAppearanceType
//
//***********************************************************************
class BldgAppearanceType : public AppearanceType
{
	public:
	
		TG_TypeMultiShapePtr		bldgShape[MAX_LODS];
		float						lodDistance[MAX_LODS];
		
		TG_TypeMultiShapePtr		bldgShadowShape;
		
		TG_TypeMultiShapePtr		bldgDmgShape;
		
		TG_TypeMultiShapePtr        bldgDmgShadowShape;
		
		TG_AnimateShapePtr			bdAnimData[MAX_BD_ANIMATIONS];
		bool						bdAnimLoop[MAX_BD_ANIMATIONS];
		bool						bdReverse[MAX_BD_ANIMATIONS];
		bool						bdRandom[MAX_BD_ANIMATIONS];
		long						bdStartF[MAX_BD_ANIMATIONS];
		
		char						rotationalNodeId[TG_NODE_ID];
		char						destructEffect[60];
		
		bool						spinMe;
		bool						isForestClump;
		
		DWORD						terrainLightRGB;
		float						terrainLightIntensity;
		float						terrainLightInnerRadius;
		float						terrainLightOuterRadius;
		
		long						numWeaponNodes;
		NodeData					*nodeData;
		
	public:
	
		void init (void)
		{
			long i=0;
			for (i=0;i<MAX_LODS;i++)
			{
				bldgShape[i] = NULL;
				lodDistance[i] = 0.0f;
			}

			bldgShadowShape = NULL;
			bldgDmgShape = NULL;
			bldgDmgShadowShape = NULL;
			
			for (i=0;i<MAX_BD_ANIMATIONS;i++)
            {
				bdAnimData[i] = NULL;

                //sebi: init so will not contain garbage
                bdAnimLoop[i] = false;
                bdReverse[i] = false;
                bdRandom[i] = false;
                bdStartF[i] = 0;
                //
            }
				
			destructEffect[0] = 0;
			
			spinMe = false;

            //sebi: init so will not contain garbage
            nodeData = NULL;
            numWeaponNodes = 0;
            isForestClump = false;
            MemSet(rotationalNodeId, 0);//sebi
            //
		}
	
		BldgAppearanceType (void)
		{
			init();
		}

		~BldgAppearanceType (void)
		{
			destroy();
		}

		void setAnimation (TG_MultiShapePtr shape, DWORD animationNum);
		
		long getNumFrames (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_BD_ANIMATIONS) && (bdAnimData[animationNum]))
				return bdAnimData[animationNum]->GetNumFrames();

			return 0.0f;
		}

		float getFrameRate (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_BD_ANIMATIONS) && (bdAnimData[animationNum]))
				return bdAnimData[animationNum]->GetFrameRate();

			return 0.0f;
		}

		void setFrameRate (long animationNum, float nFrameRate)
		{
			if ((animationNum >= 0) && (animationNum < MAX_BD_ANIMATIONS) && (bdAnimData[animationNum]))
				bdAnimData[animationNum]->SetFrameRate(nFrameRate);
		}

		bool isReversed (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_BD_ANIMATIONS) && (bdAnimData[animationNum]))
				return bdReverse[animationNum];

			return false;
		}
		
		bool isLooped (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_BD_ANIMATIONS) && (bdAnimData[animationNum]))
				return bdAnimLoop[animationNum];

			return false;
		}
		
		bool isRandom (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_BD_ANIMATIONS) && (bdAnimData[animationNum]))
				return bdRandom[animationNum];

			return false;
		}
		
  		virtual void init (const char *fileName);
		
		virtual void destroy (void);

};

//***********************************************************************
//
// BldgAppearance
//
//***********************************************************************
class BldgAppearance : public ObjectAppearance
{
	public:

		BldgAppearanceType*							appearType;
		TG_MultiShapePtr							bldgShape;
		TG_MultiShapePtr							bldgShadowShape;
		
		long										bdAnimationState;
		float										currentFrame;
		float										bdFrameRate;
		bool										isReversed;
		bool										isLooping;
		bool										setFirstFrame;
		bool										canTransition;
		
		float										turretYaw;
		float										turretPitch;
		
		float										hazeFactor;
		
		long										status;
		
   		gosFX::Effect								*destructFX;
   		gosFX::Effect								*activity;
   		gosFX::Effect								*activity1;
		bool										isActivitying;
		
		float										OBBRadius;
		float										highZ;
		
		float										SpinAngle;
		
		float										flashDuration;
		float										duration;
		float										currentFlash;
		bool										drawFlash;
		DWORD										flashColor;
		
		long										currentLOD;
		
 		long										*nodeUsed;				//Used to stagger the weapon nodes for firing.
		float										*nodeRecycle;			//Used for ripple fire to find out if the node has fired recently.
		
		TG_LightPtr									pointLight;
		DWORD										lightId;
		bool										forceLightsOut;
		bool										beenInView;
		
		bool										fogLightSet;
		DWORD										lightRGB;
		DWORD										fogRGB;

		long										rotationalNodeId;
		long										hitNodeId;
		long										activityNodeId;
		long										activityNode1Id;

 	public:

		virtual void init (AppearanceTypePtr tree = NULL, GameObjectPtr obj = NULL);

		virtual AppearanceTypePtr getAppearanceType (void)
		{
			return appearType;
		}

		BldgAppearance (void)
		{
			init();
		}

		virtual long update (bool animate = true);
		virtual long render (long depthFixup = 0);

		virtual long renderShadows (void);

		virtual void destroy (void);

		~BldgAppearance (void)
		{
			destroy();
		}

		virtual bool recalcBounds (void);
		
		virtual bool getInTransition (void)
		{
			return (canTransition == false);
		}

		void setFadeTable (MemoryPtr fTable)
		{
			fadeTable = fTable;
		}

		virtual void setGesture (unsigned long gestureId);
		
		virtual long getCurrentGestureId (void)
		{
			return bdAnimationState;
		}

		virtual unsigned long getAppearanceClass (void)
		{
			return BUILDING_APPR_TYPE;
		}
			
		virtual void setObjectNameId (long objId)
		{
			objectNameId = objId;
		}

		virtual bool isMouseOver (float px, float py);
		
		virtual void setObjectParameters (const Stuff::Vector3D &pos, float rot, long selected, long alignment, long homeRelations);
		
		virtual void setMoverParameters (float turretRot, float lArmRot = 0.0f, float rArmRot = 0.0f, bool isAirborne = false);
		
		virtual void setObjStatus (long oStatus);
		
		virtual long calcCellsCovered (Stuff::Vector3D& pos, short* cellList);

		virtual void markTerrain(_ScenarioMapCellInfo* pInfo, int type, int counter);
		
		virtual long markMoveMap (bool passable, long* lineOfSightRect, bool useheight = false, short* cellList = NULL);

		virtual void markLOS (bool clearIt = false);
		
 		void calcAdjCell (long& row, long& col);

		virtual void scale (float scaleFactor)
		{
			bldgShape->ScaleShape(scaleFactor);
		}
		
		virtual bool playDestruction (void);
		
		virtual float getRadius (void)
		{
			return OBBRadius;
		}
		
		virtual void flashBuilding (float duration, float flashDuration, DWORD color);

		virtual float getTopZ (void)
		{
			return highZ;
		}
		
		virtual void setWeaponNodeUsed (long nodeId);
		
		virtual long getWeaponNode (long weapontype);
		
		virtual float getWeaponNodeRecycle (long node);
		
		virtual Stuff::Vector3D getWeaponNodePosition (long node);

		virtual bool isSelectable()
		{
			return !appearType->spinMe;
		}

		virtual void setFilterState (bool state)
		{
			bldgShape->GetMultiType()->SetFilter(state);
		}
		
		virtual void setIsHudElement (void)
		{
			bldgShape->SetIsHudElement();
		}
		
		virtual bool getIsLit (void)
		{
			return (appearType->terrainLightRGB != 0xffffffff);
		}
		
		virtual void setLightsOut (bool lightFlag)
		{
			forceLightsOut = lightFlag;
		}

		virtual bool PerPolySelect (long mouseX, long mouseY);

		virtual bool isForestClump (void)
		{	
			return appearType->isForestClump;
		}
		
		virtual Stuff::Point3D getRootNodeCenter (void)
		{
			Stuff::Point3D result = bldgShape->GetRootNodeCenter();
			return result;
		}
		
		virtual Stuff::Vector3D getNodeNamePosition (const char *nodeName);
		
		virtual void startActivity (long effectId, bool loop);
		virtual void stopActivity (void);

		virtual Stuff::Vector3D getHitNode (void);

		virtual bool hasAnimationData (long gestureId)
		{
			return (appearType->bdAnimData[gestureId] != NULL);
		}

		virtual Stuff::Vector3D getNodeIdPosition (long nodeId);
};

//***************************************************************************

//***********************************************************************
//
// TreeAppearanceType
//
//***********************************************************************
class TreeAppearanceType : public AppearanceType
{
	public:
	
		TG_TypeMultiShapePtr		treeShape[MAX_LODS];
		float						lodDistance[MAX_LODS];
		
		TG_TypeMultiShapePtr		treeShadowShape;
		
		TG_TypeMultiShapePtr		treeDmgShape;
		
		TG_TypeMultiShapePtr        treeDmgShadowShape;
		
		TG_AnimateShapePtr			treeAnimData[MAX_BD_ANIMATIONS];
		bool						isForestClump;
		
	public:
	
		void init (void)
		{
			long i=0;
			for (i=0;i<MAX_LODS;i++)
			{
				treeShape[i] = NULL;
				lodDistance[i] = 0.0f;
			}

			for (i=0;i<MAX_BD_ANIMATIONS;i++)
				treeAnimData[i] = NULL;
				
			treeShadowShape = NULL;
			
			treeDmgShape = NULL;
			treeDmgShadowShape = NULL;
		}
	
		TreeAppearanceType (void)
		{
			init();
		}

		~TreeAppearanceType (void)
		{
			destroy();
		}

		virtual void init (const char *fileName);
		
		virtual void destroy (void);
};

//***********************************************************************
//
// TreeAppearance
//
//***********************************************************************
class TreeAppearance : public ObjectAppearance
{
	public:

		TreeAppearanceType*							appearType;
		TG_MultiShapePtr							treeShape;
		TG_MultiShapePtr							treeShadowShape;
												
		float										hazeFactor;
		float										pitch;
		float										yaw;
		long										status;
		
		float										OBBRadius;
		
		long										currentLOD;
		
		bool										forceLightsOut;
		bool										beenInView;

		bool										fogLightSet;
		DWORD										lightRGB;
		DWORD										fogRGB;

	public:

		virtual void init (AppearanceTypePtr tree = NULL, GameObjectPtr obj = NULL);

		virtual AppearanceTypePtr getAppearanceType (void)
		{
			return appearType;
		}

		TreeAppearance (void)
		{
			init();
		}

		virtual long update (bool animate = true);
		virtual long render (long depthFixup = 0);

		virtual long renderShadows (void);

		virtual void destroy (void);

		~TreeAppearance (void)
		{
			destroy();
		}

		virtual unsigned long getAppearanceClass (void)
		{
			return TREE_APPR_TYPE;
		}

		virtual bool recalcBounds (void);
		
		void setFadeTable (MemoryPtr fTable)
		{
			fadeTable = fTable;
		}
		
		virtual void setObjectNameId (long objId)
		{
			objectNameId = objId;
		}

		virtual bool isMouseOver (float px, float py);
		
		virtual void setObjectParameters (const Stuff::Vector3D &pos, float rot, long selected, long alignment, long homeRelations);
		
		virtual void setMoverParameters (float pitchAngle, float lArmRot = 0.0f, float rArmRot = 0.0f, bool isAirborne = false);
		
		virtual void setObjStatus (long oStatus);

		virtual void scale (float scaleFactor)
		{
			treeShape->ScaleShape(scaleFactor);
		}
		
		virtual float getRadius (void)
		{
			return OBBRadius;
		}
		
		virtual void setLightsOut (bool lightFlag)
		{
			forceLightsOut = lightFlag;
		}
		
		virtual bool isForestClump (void)
		{	
			return appearType->isForestClump;
		}
		
		virtual void markTerrain(_ScenarioMapCellInfo* pInfo, int type, int counter);

 		virtual Stuff::Point3D getRootNodeCenter (void)
		{
			Stuff::Point3D result = treeShape->GetRootNodeCenter();
			return result;
		}

		virtual void markLOS (bool clearIt = false);
};

//***************************************************************************


#endif


