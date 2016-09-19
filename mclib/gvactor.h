//---------------------------------------------------------------------------
//
//	gvactor.h - This file contains the header for the Ground Vehicle Actor class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GVACTOR_H
#define GVACTOR_H

//---------------------------------------------------------------------------
// Include files
#ifndef APPEAR_H
#include"appear.h"
#endif

#ifndef APPRTYPE_H
#include"apprtype.h"
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

#define	GV_PART_BODY				0
#define	GV_PART_TURRET				1
#define	NUM_GV_PARTS				2

#define MAX_GV_ANIMATIONS			10
//***********************************************************************
//
// GVAppearanceType
//
//***********************************************************************
class GVAppearanceType : public AppearanceType
{
	public:
	
		TG_TypeMultiShapePtr		gvShape[MAX_LODS];
		float						lodDistance[MAX_LODS];
		
		TG_TypeMultiShapePtr		gvShadowShape;
		
 		TG_TypeMultiShapePtr		gvDmgShape;
		
		char						destructEffect[60];
		
		TG_AnimateShapePtr			gvAnimData[MAX_GV_ANIMATIONS];
		bool						gvAnimLoop[MAX_GV_ANIMATIONS];
		bool						gvReverse[MAX_GV_ANIMATIONS];
		bool						gvRandom[MAX_GV_ANIMATIONS];
		long						gvStartF[MAX_GV_ANIMATIONS];
 		
		char						rotationalNodeId[TG_NODE_ID];
		
		long						numSmokeNodes;		//Where damage smoke comes from.
		long						numWeaponNodes;		//Where weapons fire from.
		long						numFootNodes;		//Where dust trail, contrail comes out of.
		NodeData					*nodeData;
		
		static TG_TypeMultiShapePtr	SensorTriangleShape;
		static TG_TypeMultiShapePtr	SensorCircleShape;
		
	public:
	
		void init (void)
		{
			long i=0;
			for (i=0;i<MAX_LODS;i++)
			{
				gvShape[i] = NULL;
				lodDistance[i] = 0.0f;
			}
			
			gvShadowShape = NULL;
			
			gvDmgShape = NULL;

			for (i=0;i<MAX_GV_ANIMATIONS;i++)
			{
				gvAnimData[i] = NULL;
				gvAnimLoop[i] = false;
				gvReverse[i] = false;
				gvRandom[i] = false;
				gvStartF[i] = 0;			
			}
				
			destructEffect[0] = 0;
		}
	
		GVAppearanceType (void)
		{
			init();
		}

		~GVAppearanceType (void)
		{
			destroy();
		}

		virtual void init (char *fileName);
		
		virtual void destroy (void);
		
		void setAnimation (TG_MultiShapePtr shape, DWORD animationNum);
		
		long getNumFrames (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GV_ANIMATIONS) && (gvAnimData[animationNum]))
				return gvAnimData[animationNum]->GetNumFrames();

			return 0.0f;
		}

		float getFrameRate (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GV_ANIMATIONS) && (gvAnimData[animationNum]))
				return gvAnimData[animationNum]->GetFrameRate();

			return 0.0f;
		}

		bool isReversed (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GV_ANIMATIONS) && (gvAnimData[animationNum]))
				return gvReverse[animationNum];

			return false;
		}
		
		bool isLooped (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GV_ANIMATIONS) && (gvAnimData[animationNum]))
				return gvAnimLoop[animationNum];

			return false;
		}
		
		bool isRandom (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GV_ANIMATIONS) && (gvAnimData[animationNum]))
				return gvRandom[animationNum];

			return false;
		}
		
		long getTotalNodes (void)
		{
			return numSmokeNodes + numWeaponNodes + numFootNodes;
		}

};

//***********************************************************************
//
// GVAppearance
//
//***********************************************************************
class GVAppearance : public ObjectAppearance
{
	public:

		GVAppearanceType*							appearType;
		
		TG_MultiShapePtr							gvShape;
		TG_MultiShapePtr							gvShadowShape;
		
		TG_MultiShapePtr							sensorCircleShape;
		TG_MultiShapePtr							sensorTriangleShape;
		float										sensorSpin;
									
		long										objectNameId;
	
		float										turretRotation;
		float										pitch;
		float										roll;
		float										actualTurretRotation;
		float										velocityMagnitude;
		bool										inDebugMoveMode;
		long										sensorLevel;
		float										hazeFactor;
		long										status;
			
   		gosFX::Effect								*destructFX;
   		gosFX::Effect								*waterWake;
   		gosFX::Effect								*dustCloud;
		gosFX::Effect								*activity;
		bool										isWaking;
		bool										isActivitying;
		bool										movedThisFrame;
		bool										dustCloudStart;
		
		float										OBBRadius;
		
		long										gvAnimationState;
		float										currentFrame;
		float										gvFrameRate;
		bool										isReversed;
		bool										isLooping;
		bool										setFirstFrame;
		bool										canTransition;
		bool										isInfantry;
		
		long										currentLOD;
		
 		long										*nodeUsed;				//Used to stagger the weapon nodes for firing.
		float										*nodeRecycle;			//Used for ripple fire to find out if the node has fired recently.
		
		DWORD										localTextureHandle;
		
		DWORD										psRed;
		DWORD										psBlue;
		DWORD										psGreen;

		float										flashDuration;
		float										duration;
		float										currentFlash;
		bool										drawFlash;
		DWORD										flashColor;
		
		long										rotationalNodeIndex;
		long										dustNodeIndex;
		long										activityNodeIndex;
		long										hitNodeId;
		long										weaponNodeId[4];

 	public:

		virtual void init (AppearanceTypePtr tree = NULL, GameObjectPtr obj = NULL);

		virtual AppearanceTypePtr getAppearanceType (void)
		{
			return appearType;
		}

		GVAppearance (void)
		{
			init();
		}

		virtual long update (bool animate = true);
		void updateGeometry (void);
		
		virtual long render (long depthFixup = 0);

		virtual long renderShadows (void);
		
		virtual void destroy (void);

		virtual unsigned long getAppearanceClass (void)
		{
			return VEHICLE_APPR_TYPE;
		}
	
		~GVAppearance (void)
		{
			destroy();
		}

		virtual void setPaintScheme (void);

		virtual void setPaintScheme (DWORD red, DWORD green, DWORD blue);

		virtual void getPaintScheme (DWORD &red, DWORD &green, DWORD &blue);

		virtual void resetPaintScheme (DWORD red, DWORD green, DWORD blue);
		
 		virtual bool recalcBounds (void);

		virtual void flashBuilding (float duration, float flashDuration, DWORD color);

		
		void setFadeTable (MemoryPtr fTable)
		{
			fadeTable = fTable;
		}
		
		virtual void setObjectNameId (long objId)
		{
			objectNameId = objId;
		}

		virtual bool isMouseOver (float px, float py);
		
		virtual void setObjectParameters (Stuff::Vector3D &pos, float rot, long selected, long team, long homeRelations);

		virtual void setMoverParameters (float turretRot, float lArmRot = 0.0f, float rArmRot = 0.0f, bool isAirborne = false);

		void debugUpdate (void);

		virtual void setSensorLevel (long lvl)
		{
			sensorLevel = lvl;
		}
		
		virtual void setObjStatus (long oStatus);
		
		virtual bool playDestruction (void);
		
		virtual bool getInTransition (void)
		{
			return (canTransition == false);
		}

		virtual void setGesture (unsigned long gestureId);
		
		virtual long getCurrentGestureId (void)
		{
			return gvAnimationState;
		}

		//--------------------------------------------
		// Once site Objects are in place, go get 'em
		virtual void setWeaponNodeUsed (long nodeId);
		
		virtual Stuff::Vector3D getWeaponNodePosition (long nodeId);
		
		virtual Stuff::Vector3D getSmokeNodePosition (long nodeId);
		
		virtual Stuff::Vector3D getDustNodePosition (long nodeId);
		
		virtual long getWeaponNode (long weapontype);
		
		virtual float getWeaponNodeRecycle (long node);

		virtual long getLowestWeaponNode (void);
		
		virtual Stuff::Vector3D getNodeNamePosition (const char *nodeName);
		
 		virtual bool PerPolySelect (long mouseX, long mouseY);
		
		virtual Stuff::Point3D getRootNodeCenter (void)
		{
			Stuff::Point3D result = gvShape->GetRootNodeCenter();
			return result;
		}
		
		virtual void setAlphaValue (BYTE aVal)
		{
			gvShape->SetAlphaValue(aVal);
			
			//Sensor shape fades in opposite direction from mover
			sensorCircleShape->SetAlphaValue(0xff - aVal);
			sensorTriangleShape->SetAlphaValue(0xff - aVal);
		}

		virtual void scale (float scaleFactor)
		{
			gvShape->ScaleShape(scaleFactor);
		}
		
		virtual void startWaterWake (void);
		virtual void stopWaterWake (void);
		
		virtual void startActivity (long effectId, bool loop);
		virtual void stopActivity (void);

		virtual Stuff::Vector3D getNodeIdPosition (long nodeId);

		virtual Stuff::Vector3D getHitNode (void);
};

//***************************************************************************

#endif


