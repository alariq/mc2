//---------------------------------------------------------------------------
//
// Appear.h -- File contains the Basic Game Appearance Declaration
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef APPEAR_H
#define APPEAR_H
//---------------------------------------------------------------------------
// Include Files
#ifndef DAPPEAR_H
#include"dappear.h"
#endif

#ifndef DAPRTYPE_H
#include"daprtype.h"
#endif

#ifndef FLOATHELP_H
#include"floathelp.h"
#endif

#include<stuff/stuff.hpp>
//---------------------------------------------------------------------------
// Macro definitions
#ifndef MAX_ULONG
#define MAX_ULONG		0x70000000		//Must be able to square this and still get big number!!
#endif

#ifndef NO_ERR
#define NO_ERR			0
#endif

#define HUD_DEPTH		0.0001f			//HUD Objects draw over everything else.

#define WIDTH			19				// really half the width...
#define HEIGHT			4

#define	BARTEST			0.001f

extern FloatHelpPtr globalFloatHelp;
extern unsigned long currentFloatHelp;


// would have loved to make object flags, but it looks like we're out...
#define DRAW_NORMAL						0x00
#define DRAW_BARS						0x01
#define DRAW_TEXT						0x02
#define DRAW_BRACKETS					0x04
#define DRAW_COLORED					0x08


struct _ScenarioMapCellInfo;
struct _GameObjectFootPrint;
//---------------------------------------------------------------------------
// Class definitions
class Appearance
{
	//Data Members
	//-------------
	protected:
		Stuff::Vector4D				screenPos;		//Where am I on Screen?  INCLUDES correct Z now!

		bool						visible;		//Current FOW status to help draw
		bool						seen;			//Current FOW status to help draw
		
	public:

		bool						inView;			//Can I be Seen?
		Stuff::Vector4D				upperLeft;		//used to draw select boxes.  Can be 3D Now!
		Stuff::Vector4D				lowerRight;		//used to draw select boxes.
		
		float						barStatus;		//Status Bar Length.
		DWORD						barColor;		//Status Bar Color.
	
	//Member Functions
	//-----------------
	public:

		void * operator new (size_t mySize);
		void operator delete (void * us);
			
		Appearance (void)
		{
			inView = FALSE;
			screenPos.x = screenPos.y = screenPos.z = screenPos.w = -999.0f;
			upperLeft.x = upperLeft.y = upperLeft.z = upperLeft.w = -999.0f;
			lowerRight.x = lowerRight.y = lowerRight.z = lowerRight.w = -999.0f;
			barStatus = 1.0;
			barColor = 0x0;

			visible = seen = false;
		}
		
		virtual void init (AppearanceTypePtr tree = NULL, GameObjectPtr obj = NULL)
		{
			inView = FALSE;
			screenPos.x = screenPos.y = screenPos.z = screenPos.w = -999.0f;
			upperLeft.x = upperLeft.y = upperLeft.z = upperLeft.w = -999.0f;
			lowerRight.x = lowerRight.y = lowerRight.z = lowerRight.w = -999.0f;
			barStatus = 1.0;
			barColor = 0x0;

			visible = seen = false;
		}
		
		virtual void initFX (void)
		{
		}

		virtual void destroy (void)
		{
			init();
		}
		
		virtual ~Appearance (void)
		{
			destroy();
		}
		
		virtual long update (bool animate = true)
		{
			//Perform any frame by frame tasks.  Animations, etc.
			return NO_ERR;
		}
		
		virtual long render (long depthFixup = 0)
		{
			//Decide whether or not I can be seen and add me to render list.
			return NO_ERR;
		}

		virtual long renderShadows (void)
		{
			return NO_ERR;
		}

		virtual AppearanceTypePtr getAppearanceType (void)
		{
			return NULL;
		}

		bool canBeSeen (void)
		{
			return(inView);
		}
		
		void setInView (bool viewStatus)
		{
			inView = viewStatus;
		}
				
		Stuff::Vector4D getScreenPos (void)
		{
			return screenPos;
		}

		virtual bool isMouseOver (float px, float py)
		{
			return FALSE;		//Never over a base appearance
		}
				
		virtual void drawSelectBox (DWORD color);

		virtual void drawSelectBrackets (DWORD color);

		virtual void drawBars (void);

		void drawTextHelp (char *text);
		void drawTextHelp (char *text, unsigned long color);

		void drawPilotName(char *text, unsigned long color ); // next line below drawTextHelp

		
		virtual bool recalcBounds (void)
		{
			//-------------------------------------------------------
			// returns TRUE is this appearance is Visible this frame
			inView = FALSE;
			return inView;
		}

		virtual void setGesture (unsigned long gestureId)
		{
		}

		virtual long setGestureGoal (long gestureId)
		{
			return 0;
		}
		
		virtual void setVelocityPercentage (float percent)
		{
		}
		
		virtual long getFrameNumber (void)
		{
			return 0;
		}

		virtual unsigned long getAppearanceClass (void)
		{
			return BASE_APPEARANCE;
		}
		
		virtual void setBarStatus (float stat)
		{
			barStatus = stat;
			if (barStatus > 1.0f)
				barStatus = 1.0f;
		}

		virtual void setBarColor (DWORD clr)
		{
			barColor = clr;
		}

		virtual bool getInTransition (void)
		{
			return false;
		}

		virtual long getNumFramesInGesture (long gestureId)
		{
			return 0;
		}

		virtual long getOldGestureGoal (void)
		{
			return -1;
		}
		
		virtual long getCurrentGestureGoal (void)
		{
			return -1;
		}

		virtual long getCurrentGestureId (void)
		{
			return 0;
		}

		virtual bool isInJump (void)
		{
			return false;
		}

		virtual bool isJumpSetup (void)
		{
			return false;
		}

		virtual bool isJumpAirborne (void)
		{
			return false;
		}

		virtual bool haveFallen (void)
		{
			return false;
		}

		virtual void setObjectNameId (long objId)
		{
		}

		virtual bool setJumpParameters (Stuff::Vector3D &jumpGoal)
		{
			return false;
		}

		virtual void setWeaponNodeUsed (long nodeId)
		{
		}

		virtual long getLowestWeaponNode (void)
		{
			return 0;
		}
		
		virtual long getWeaponNode (long weapontype)
		{
			return 0;
		}
		
		virtual float getWeaponNodeRecycle (long node)
		{
			return 0.0f;
		}

		virtual void resetWeaponNodes (void)
		{
		}

		virtual void setWeaponNodeRecycle(long nodeId, float time)
		{
		}
		
		virtual Stuff::Vector3D getSmokeNodePosition (long nodeId)
		{
			Stuff::Vector3D position;
			position.x = position.y = position.z = 0.0f;

			return position;
		}
		
		virtual Stuff::Vector3D getDustNodePosition (long nodeId)
		{
			Stuff::Vector3D position;
			position.x = position.y = position.z = 0.0f;

			return position;
		}
		
 		virtual Stuff::Vector3D getWeaponNodePosition (long node)
		{
			Stuff::Vector3D position;
			position.x = position.y = position.z = 0.0f;

			return position;
		}

		virtual Stuff::Vector3D getNodePosition (long nodeId)
		{
			Stuff::Vector3D position;
			position.x = position.y = position.z = 0.0f;

			return position;
		}
		
		virtual Stuff::Vector3D getNodeNamePosition (const char *nodeName)
		{
			Stuff::Vector3D position;
			position.x = position.y = position.z = 0.0f;

			return position;
		}
		
		virtual Stuff::Vector3D getNodeIdPosition (long nodeId)
		{
			Stuff::Vector3D position;
			position.x = position.y = position.z = 0.0f;

			return position;
		}

		virtual void setCombatMode (bool combatMode)
		{
		}

		virtual float getVelocityMagnitude (void)
		{
			return 0.0f;
		}

		virtual float getVelocityOfGesture (long gestureId)
		{
			return 0.0f;
		}


		virtual void setBrake (bool brake)
		{
		}

		virtual void setObjectParameters (const Stuff::Vector3D &pos, float rot, long selected, long team, long homeRelations) = 0;

		virtual void setMoverParameters (float turretRot, float lArmRot = 0.0f, float rArmRot = 0.0f, bool isAirborne = false)
		{
		}

		virtual void updateFootprints (void)
		{
		}

		virtual void setPaintScheme (void)
		{
		}

		virtual void setPaintScheme (DWORD red, DWORD green, DWORD blue)
		{
		}

		virtual void getPaintScheme (DWORD &red, DWORD &green, DWORD &blue)
		{
		}

		virtual void resetPaintScheme (DWORD red, DWORD green, DWORD blue)
		{
		}
		
		virtual void setDebugMoveMode (void)
		{
		}

		virtual void setSingleStepMode (void)
		{
		}
		
		virtual void setPrevFrame (void)
		{
		}
		
		virtual void setNextFrame (void)
		{
		}

		virtual void setVisibility (bool vis, bool sen)
		{
			visible = vis;
			seen = sen;
		}

		virtual void setSensorLevel (long lvl)
		{
		}
		
		virtual void hitFront (void)
		{
		}
	
		virtual void hitBack (void)
		{
		}
		
		virtual void hitLeft (void)
		{
		}

		virtual void hitRight (void)
		{
		}
		
		virtual void setObjStatus (long oStatus)
		{
		}
		
		virtual long calcCellsCovered (Stuff::Vector3D& pos, short* cellList) {
			return(0);
		}

		virtual void markTerrain (_ScenarioMapCellInfo* pInfo, int type, int counter)
		{
		}
		
		virtual long markMoveMap (bool passable, long* lineOfSightRect, bool useheight = false, short* cellList = NULL)
		{
			return(0);
		}

		virtual void markLOS (bool clearIt = false)
		{
		
		}
		
		virtual void scale (float scaleFactor)
		{
		}
		
		virtual bool playDestruction (void)
		{
			return false;
		}
		
		virtual float getRadius (void)
		{
			return 0.0f;
		}
		
		virtual void flashBuilding (float duration, float flashDuration, DWORD color)
		{
		}
		
		virtual void setHighlightColor( long argb )
		{
		}
	
		virtual float getTopZ (void)
		{
			return 0.0f;
		}
		
		virtual void blowLeftArm (void)
		{
		}
		
		virtual void blowRightArm (void)
		{
		}

		virtual void setFilterState (bool state)
		{
		}
		
		virtual Stuff::Vector3D getVelocity (void)
		{
			Stuff::Vector3D result;
			result.Zero();
			return result;
		}

		virtual bool isSelectable()
		{
			return true;
		}
		
		virtual void setIsHudElement (void)
		{
		}

		virtual long getObjectNameId ()
		{
			return -1;
		}

		virtual bool getIsLit (void)
		{
			return false;
		}
		
		virtual void setLightsOut (bool lightFlag)
		{
		
		}

		virtual bool PerPolySelect (long mouseX, long mouseY)
		{
			return true;
		}

		virtual bool isForestClump (void)
		{	
			return false;
		}
		
		virtual Stuff::Point3D getRootNodeCenter (void)
		{
			Stuff::Point3D result;
			result.Zero();
			return result;
		}
		
		virtual void setAlphaValue (BYTE aVal)
		{
		}

		void drawIcon(  unsigned long bmpHandle, unsigned long bmpWidth,
						  unsigned long bmpHeight, unsigned long color, 
						  unsigned long where = 0 );
						  
		virtual void setSkyNumber (long skyNum)
		{
		}

		virtual void setMechName( const char* pName ){}
		
		virtual void startSmoking (long smokeLvl)
		{
		}
		
		virtual void startWaterWake (void)
		{
		}
		
		virtual void stopWaterWake (void)
		{
		}
		
		virtual void playEjection (void)
		{
		}
		
		virtual void startActivity (long effectId, bool loop)
		{
		}
		
		virtual void stopActivity (void)
		{
		}

		virtual Stuff::Vector3D getHitNode (void)
		{
			Stuff::Point3D result;
			result.Zero();
			return result;
		}

		virtual Stuff::Vector3D getHitNodeLeft (void)
		{
			Stuff::Point3D result;
			result.Zero();
			return result;
		}

		virtual Stuff::Vector3D getHitNodeRight (void)
		{
			Stuff::Point3D result;
			result.Zero();
			return result;
		}

		virtual bool getRightArmOff (void)
		{
			return false;
		}

		virtual bool getLeftArmOff (void)
		{
			return false;
		}

		virtual bool hasAnimationData (long gestureId)
		{
			return false;
		}
};		

//---------------------------------------------------------------------------
#endif
