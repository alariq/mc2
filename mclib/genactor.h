//---------------------------------------------------------------------------
//
//	Genactor.h - This file contains the header for the generic 3D appearance class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GENACTOR_H
#define GENACTOR_H

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

#include"memfunc.h"

//**************************************************************************************
#ifndef NO_ERR
#define NO_ERR						0
#endif

#define MAX_GEN_ANIMATIONS			5

//***********************************************************************
//
// GenericAppearanceType
//
//***********************************************************************
class GenericAppearanceType : public AppearanceType
{
	public:
	
		TG_TypeMultiShapePtr		genShape;
		TG_TypeMultiShapePtr		genDmgShape;
		
		TG_AnimateShapePtr			genAnimData[MAX_GEN_ANIMATIONS];
		bool						genAnimLoop[MAX_GEN_ANIMATIONS];
		bool						genReverse[MAX_GEN_ANIMATIONS];
		bool						genRandom[MAX_GEN_ANIMATIONS];
		long						genStartF[MAX_GEN_ANIMATIONS];
		
		char						rotationalNodeId[TG_NODE_ID];
		char						textureName[50];
		DWORD						dotRGB;
		
	public:
	
		void init (void)
		{
			genShape = NULL;

            //sebi: init so will not contain garbage
            genDmgShape = NULL; 
            MemSet(rotationalNodeId, 0);
            //

			for (long i=0;i<MAX_GEN_ANIMATIONS;i++)
            {
				genAnimData[i] = NULL;

                //sebi: init so will not contain garbage
		        genAnimLoop[i] = false;
		        genReverse[i] = false;
		        genRandom[i] = false;
		        genStartF[i] = 0;
                //
            }


			textureName[0] = 0;
			dotRGB = 0x00ffffff;
		}
	
		GenericAppearanceType (void)
		{
			init();
		}

		~GenericAppearanceType (void)
		{
			destroy();
		}

		void setAnimation (TG_MultiShapePtr shape, DWORD animationNum);
		
		long getNumFrames (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GEN_ANIMATIONS) && (genAnimData[animationNum]))
				return genAnimData[animationNum]->GetNumFrames();

			return 0.0f;
		}

		float getFrameRate (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GEN_ANIMATIONS) && (genAnimData[animationNum]))
				return genAnimData[animationNum]->GetFrameRate();

			return 0.0f;
		}

		bool isReversed (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GEN_ANIMATIONS) && (genAnimData[animationNum]))
				return genReverse[animationNum];

			return false;
		}
		
		bool isLooped (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GEN_ANIMATIONS) && (genAnimData[animationNum]))
				return genAnimLoop[animationNum];

			return false;
		}
		
		bool isRandom (long animationNum)
		{
			if ((animationNum >= 0) && (animationNum < MAX_GEN_ANIMATIONS) && (genAnimData[animationNum]))
				return genRandom[animationNum];

			return false;
		}
		
  		virtual void init (const char *fileName);
		
		virtual void destroy (void);
};

//***********************************************************************
//
// GenericAppearance
//
//***********************************************************************
class GenericAppearance : public ObjectAppearance
{
	public:

		GenericAppearanceType*						appearType;
		TG_MultiShapePtr							genShape;
		
		long										genAnimationState;
		float										currentFrame;
		float										genFrameRate;
		bool										isReversed;
		bool										isLooping;
		bool										setFirstFrame;
		bool										canTransition;
		
		float										hazeFactor;
		float										pitch;
		
		long										status;
			
		float										OBBRadius;
		
		long										skyNumber;
		
	public:

		virtual void init (AppearanceTypePtr tree = NULL, GameObjectPtr obj = NULL);

		virtual AppearanceTypePtr getAppearanceType (void)
		{
			return appearType;
		}

		void changeSkyToSkyNum (char *txmName, char *newName);
		
		GenericAppearance (void)
		{
			init();
		}

		virtual long update (bool animate = true);
		virtual long render (long depthFixup = 0);

		virtual long renderShadows (void);

		virtual void destroy (void);

		~GenericAppearance (void)
		{
			destroy();
		}

		virtual bool recalcBounds (void);
		
		virtual bool getInTransition (void)
		{
			return (canTransition == false);
		}

		virtual void setGesture (unsigned long gestureId);
		
		virtual long getCurrentGestureId (void)
		{
			return genAnimationState;
		}

		virtual unsigned long getAppearanceClass (void)
		{
			return GENERIC_APPR_TYPE;
		}
			
		virtual void setObjectNameId (long objId)
		{
			objectNameId = objId;
		}

		virtual bool isMouseOver (float px, float py);
		
		virtual void setObjectParameters (const Stuff::Vector3D &pos, float rot, long selected, long alignment, long homeRelations);
		
		virtual void setMoverParameters (float turretRot, float lArmRot = 0.0f, float rArmRot = 0.0f, bool isAirborne = false);
		
		virtual void setObjStatus (long oStatus);
		
		virtual void markTerrain (_ScenarioMapCellInfo* pInfo, int type, int counter);
		
		virtual void markMoveMap (bool passable);

		virtual void setIsHudElement (void)
		{
			genShape->SetIsHudElement();
		}
		
 		virtual void scale (float scaleFactor)
		{
			genShape->ScaleShape(scaleFactor);
		}
		
		virtual Stuff::Point3D getRootNodeCenter (void)
		{
			Stuff::Point3D result = genShape->GetRootNodeCenter();
			return result;
		}
		
		virtual void setSkyNumber (long skyNum);
};

//***************************************************************************

#endif



