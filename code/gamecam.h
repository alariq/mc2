//---------------------------------------------------------------------------
//
// GameCam.h -- File contains the Game camera class definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GAMECAM_H
#define GAMECAM_H
//---------------------------------------------------------------------------
// Include Files

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

//---------------------------------------------------------------------------
class GameCamera : public Camera
{
	//Data Members
	//-------------
	protected:
		GameObjectPtr			targetObject;
		AppearancePtr			compass;
		bool					drawCompass;
		
		float					lastShadowLightPitch;
		
	public:
	
		virtual void init (void)
		{
			Camera::init();
			targetObject = NULL;
			compass = NULL;
			lastShadowLightPitch = 0.0f;

			DWORD szData = sizeof(drawCompass);
			gos_LoadDataFromRegistry("CompassStatus", &drawCompass, &szData);

			//Check if data not in registry yet.  Goober!
			if (!szData)
				drawCompass = true;
		}
			
		~GameCamera (void)
		{
			destroy();
		}

		void destroy (void);

		virtual void render (void);
		virtual long activate (void);
		
		virtual long update (void);
		virtual void moveLeft(float amount)
		{
			targetObject = NULL;
			Camera::moveLeft(amount);
		}
		
		virtual void moveRight(float amount)
		{
			targetObject = NULL;
			Camera::moveRight(amount);
		}
		
		virtual void moveUp(float amount)
		{
			targetObject = NULL;
			Camera::moveUp(amount);
		}
		
		virtual void moveDown(float amount)
		{
			targetObject = NULL;
			Camera::moveDown(amount);
		}
			
		virtual void setTarget (GameObjectPtr target)
		{
			targetObject = target;
			if( target )
			{
				//float newRotation = targetObject->getRotation();
				//setCameraRotation(newRotation,newRotation);
			}
		}

		GameObject* getTarget()
		{
			return targetObject;
		}
		
		virtual void allTight (void)
		{
			long i=0;
			GameObjectPtr mvt = NULL;
			while (i<ObjectManager->getNumGoodMovers())
			{
				mvt = ObjectManager->getGoodMover(i);
				if (!mvt->isDestroyed() && !mvt->isDisabled())
					break;
				i++;
			}

			tiltNormal();
			setTarget(mvt);
			ZoomTight();
		}

		void toggleCompass()
		{
			drawCompass ^= true;

			//Save current Compass state to registry.
			// Saves us having to put it into the prefs files.
			DWORD szData = sizeof(drawCompass);
			gos_SaveDataToRegistry( "CompassStatus",  &drawCompass,  szData );
		}
};		

//---------------------------------------------------------------------------
extern CameraPtr eye;		//Global Instance of the current camera

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit log
//
//---------------------------------------------------------------------------
