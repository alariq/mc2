//---------------------------------------------------------------------------
//
// MechCommander 2
//
// UserInput Class -- Polls the state of keyboard, mouse, joystick
//						for this frame and stores values.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef USERINPUT_H
#define USERINPUT_H

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef MATHFUNC_H
#include"mathfunc.h"
#endif

#ifndef UTILITIES_H
#include"utilities.h"
#endif

#include<string.h>
#include<gameos.hpp>

//---------------------------------------------------------------------------
#define MC2_MOUSE_UP		0
#define MC2_MOUSE_DOWN		1

#define MOUSE_WIDTH			32

#define MAX_MOUSE_STATES			256

#define mState_MOVE					0
#define mState_MOVE_LOS				1
#define mState_RUN					2
#define mState_RUN_LOS				3
#define mState_JUMP					4
#define mState_JUMP_LOS				5
#define mState_WALKWAYPT			6
#define mState_WALKWAYPT_LOS		7
#define mState_RUNWAYPT				8
#define mState_RUNWAYPT_LOS			9
#define mState_JUMPWAYPT			10
#define mState_JUMPWAYPT_LOS		11
#define mState_LINK					12
#define mState_GENERIC_ATTACK		13
#define mState_ATTACK_LOS			14
#define mState_CURPOS_ATTACK		15
#define mState_CURPOS_ATTACK_LOS	16
#define mState_ENERGY_WEAPONS		17
#define mState_ENERGY_WEAPONS_LOS	18
#define mState_AIMED_ATTACK			19
#define mState_AIMED_ATTACK_LOS		20
#define mState_NORMAL				21
#define mState_INFO					22
#define mState_LAYMINES				23
#define mState_ROTATE_CAMERA		24
#define mState_UNCERTAIN_AIRSTRIKE	25
#define mState_AIRSTRIKE			26
#define mState_SENSORSTRIKE			27
#define mState_VEHICLE				28
#define mState_CANTVEHICLE			29
#define mState_DONT					30
#define mState_REPAIR				31
#define mState_XREPAIR				32
#define mState_EJECT				33
#define mState_XEJECT				34
#define mState_CAPTURE				35
#define mState_XCAPTURE				36
#define mState_SALVAGE				37
#define mState_XSALVAGE				38
#define mState_GUARD				39
#define mState_GUARD_LOS			39
#define mState_GUARDTOWER			40
#define mState_GUARDTOWER_LOS		41
#define mState_XMINES				42
#define mState_LOGISTICS			43
#define mState_TUTORIALS			44
#define mState_LONGRNG_ATTACK		45
#define mState_SHRTRNG_ATTACK		46
#define mState_MEDRNG_ATTACK		47
#define mState_WALK1				48
#define mState_WALKLOS1				80
#define mState_RUN1					112
#define mState_RUNLOS1				144
#define mState_JUMP1				176
#define mState_JUMPLOS1				208
#define mState_LONGRNG_LOS			240
#define mState_MEDRNG_LOS			241
#define mState_SHRTRNG_LOS			242

#define mState_NUMMOUSESTATES		243		//Add states BETWEEN this one and above!!!
											//Used to keep from setting cursor to STUPID!!

#define mouseFrameRate		0.10

extern volatile char mc2MouseHotSpotX;
extern volatile char mc2MouseHotSpotY;
extern volatile char mc2MouseWidth;
extern volatile char mc2MouseHeight;

extern volatile bool mc2UseAsyncMouse;		//Should mouse draw and update in separate thread?
extern volatile bool mc2MouseThreadStarted;	//Has the thread starting running yet?

extern volatile MemoryPtr mc2MouseData;

//---------------------------------------------------------------------------
class MouseCursorData
{
	protected:
		//---------------------------------------------------------------
		// Mouse textures are assumed 32x32.  Animating mice are stored
		// on the smallest texture that will hold their shape.  Thus if
		// have a mouse with four frames of animation it will be stored
		// on a 64x64.
		
		long numCursors;
		
		StaticInfo*	cursorInfos;	// gotta be pointer, so destructor is called before texmgr quits

		char mouseHS[MAX_MOUSE_STATES][2];
		unsigned long numFrames[MAX_MOUSE_STATES];
		float frameLengths[MAX_MOUSE_STATES];

		friend class UserInput;
		
	public:
		MouseCursorData (void)
		{
			init();
		}
		
		~MouseCursorData (void)
		{
			destroy();
		}
		
		void destroy (void);
		
		void init (void)
		{
			numCursors = 0;
			cursorInfos = 0;
			memset( mouseHS, 0, sizeof( mouseHS ) );
			memset( numFrames, 0, sizeof( numFrames ) );
			memset( frameLengths, 0, sizeof( frameLengths ) );
		}
		
		
		long getNumFrames (long state)
		{
			if ((state >= 0) && (state < numCursors))
				return numFrames[state]; // no animating for now

			return 0;
		}

		char getMouseHSX (long state)
		{
			if ((state >= 0) && (state < numCursors))
				return mouseHS[state][0];
				
			return 0;
		}
				
		char getMouseHSY (long state)
		{
			if ((state >= 0) && (state < numCursors))
				return mouseHS[state][1];
				
			return 0;
		}
		
				
		void initCursors (const char *cursorFile);
};

//---------------------------------------------------------------------------
class UserInput
{
	protected:

		//--------------------
		// Mouse Information
		float mouseXPosition;					//Current Mouse X Position.
		float mouseYPosition;					//Current Mouse Y position.

		int mouseXDelta;						//Amount mouse has moved in x since last poll
		int mouseYDelta;						//Amount mouse has moved in y since last poll

		int mouseWheelDelta;					//Amount Mouse wheel has moved since last poll

		long leftMouseButtonState;				//Mouse Current Left Button State
		long rightMouseButtonState;				//Mouse Current Right Button State
		long middleMouseButtonState;			//Mouse Current Middle Button State

		bool leftClick;							//Mouse has been left clicked
		bool rightClick;						//Mouse has been right clicked
		bool leftDoubleClick;					//Mouse has been left double clicked
		bool rightDoubleClick;					//Mouse has been right double clicked
		bool middleClick;						//Mouse has been middle clicked
		bool middleDoubleClick;					//Mouse has been middle Double Clicked

		bool leftMouseDrag;						//Mouse is left Dragging
		bool rightMouseDrag;					//Mouse is right Dragging

		bool wasLeftMouseDrag;					//Mouse is left Dragging
		bool wasRightMouseDrag;					//Mouse is right Dragging

		float mouseDragX;						//Where we started Dragging
		float mouseDragY;						//Where we started Dragging;

		float mouseDragThreshold;				//Distance mouse MUST move before a drag is assumed.
		float mouseDblClickThreshold;			//Time between clicks maximum to assume double click.

		float mouseLeftUpTime;					//Time since last left mouse up.
		float mouseRightUpTime;					//Time since last right mouse up.
		float mouseMiddleUpTime;				//Time since last right mouse up.

		float mouseLeftHeldTime;					//How long the mouse has been down
		float mouseRightHeldTime;					//How long the mouse has been down




		long lastLeftMouseButtonState;			//Last Left Mouse Button State
		long lastRightMouseButtonState;			//Last Right Mouse Button State
		long lastMiddleMouseButtonState;		//Last Middle Mouse Button State

		float lastMouseXPosition;				//Last Mouse X Position.
		float lastMouseYPosition;				//Last Mouse Y Position.
		
		MouseCursorData *cursors;				//Stores the actual cursor data and Info
		
		bool drawTerrainPointer;				//Draw an inverse project cross
		long mouseState;						//Which cursor should I draw?
		long mouseFrame;						//current Mouse Frame
		DWORD mouseTextureHandle;				//current Mouse Texture handle.
		float mouseFrameLength;					//Time in Current Mouse Frame
		float mouseScale;						//Scale factor for cursor for depth cueing.

		float viewMulX;
		float viewAddX;
		float viewMulY;
		float viewAddY;

		bool leftMouseJustUp;					// leftButtonUp message equivalent
		bool rightMouseJustUp;					// right up equivalent
		
		DWORD	attilaIndex;					// Set to 0xffffffff if no Attila Present.
		
	public:

		//Need to have the mouse draw here
		static volatile bool drawMouse;			//Should I draw the Mouse Cursor?

		UserInput (void)
		{
			init();
		}

		~UserInput (void)
		{
			destroy();
		}

		void init (void)
		{
			mouseScale = 1.0f;
			mouseXPosition = mouseYPosition = 0.0;

			mouseXDelta = mouseYDelta = 
			mouseWheelDelta = 0;

			leftClick = rightClick = middleClick = false;
			leftMouseButtonState = rightMouseButtonState = middleMouseButtonState = MC2_MOUSE_UP;
			leftMouseDrag = rightMouseDrag = false;
			leftDoubleClick = rightDoubleClick = middleDoubleClick = false;

			mouseDragThreshold = 0.0166667f;
			mouseDblClickThreshold = 0.2f;

			mouseLeftUpTime = mouseRightUpTime = mouseMiddleUpTime = 0.0;
			mouseDragX = mouseDragY = 0.0;

			lastLeftMouseButtonState = lastRightMouseButtonState = lastMiddleMouseButtonState = MC2_MOUSE_UP;

			lastMouseYPosition = lastMouseXPosition = 0.0;
			
			drawTerrainPointer = false;
			mouseState = -1;
			mouseFrameLength = 0.0;

			viewMulX = viewMulY = viewAddX = viewAddY = 0.0f;
			leftMouseJustUp = 0;
			rightMouseJustUp = 0;
			
			attilaIndex = 0xffffffff;

			mouseLeftHeldTime = 0.f;
			mouseRightHeldTime = 0.f;

			cursors = NULL;
		}

		void initMouseCursors (const char *mouseFile);
		
		void destroy (void)
		{
			if ( cursors )
			{
				delete cursors;
				cursors = NULL;
			}

			init();
		}

		void setViewport(float mulX, float mulY, float addX, float addY)
		{
			viewMulX = mulX;
			viewMulY = mulY;
			viewAddX = addX;
			viewAddY = addY;
		}

		//------------------------
		// Keyboard Functionality
		bool getKeyDown (gosEnum_KeyIndex index)
		{
			if ((gos_GetKeyStatus(index) == KEY_HELD) || (gos_GetKeyStatus(index) == KEY_PRESSED))
				return true;

			return(false);
		}

		bool leftShift (void)
		{
			return (getKeyDown(KEY_LSHIFT));
		}

		bool rightShift (void)
		{
			return (getKeyDown(KEY_RSHIFT));
		}

		bool shift (void)
		{
			return (leftShift() || rightShift());
		}

		bool leftAlt (void)
		{
			return (getKeyDown(KEY_LALT));
		}

		bool rightAlt (void)
		{
			return (getKeyDown(KEY_RALT));
		}

		bool alt (void)
		{
			return (leftAlt() || rightAlt());
		}

		bool leftCtrl (void)
		{
			return (getKeyDown(KEY_LCONTROL));
		}

		bool rightCtrl (void)
		{
			return (getKeyDown(KEY_RCONTROL));
		}

		bool ctrl (void)
		{
			return (leftCtrl() || rightCtrl());
		}

		//---------------------------------
		// Mouse Functionality
		void setMouseDoubleClickThreshold (float time)
		{
			mouseDblClickThreshold = time;
		}

		void setMouseDragThreshold (float distance)
		{
			mouseDragThreshold = distance;
		}

		long getMouseX (void)
		{
			return float2long(mouseXPosition * viewMulX);
		}

		long getMouseY (void)
		{
			return float2long(mouseYPosition * viewMulY);
		}

		float realMouseX (void)
		{
			return (mouseXPosition * viewMulX);
		}
		
		float realMouseY (void)
		{
			return (mouseYPosition * viewMulY);
		}
		
		void setMousePos (float x, float y)
		{
			float xRes = 0.0f;
			float yRes = 0.0f;

			if ((fabs(viewMulX) > Stuff::SMALL) && 
				(fabs(viewMulY) > Stuff::SMALL))
			{
				xRes = x/viewMulX;
				yRes = y/viewMulY;
			}
			
			gos_SetMousePosition(xRes,yRes);
		}
		
		long getMouseXDelta (void)
		{
			return mouseXDelta;
		}

		long getMouseYDelta (void)
		{
			return mouseYDelta;
		}

		long getMouseWheelDelta (void)
		{
			return mouseWheelDelta;
		}

		long getMouseLeftButtonState (void)
		{
			return leftMouseButtonState;
		}

		long getMouseRightButtonState (void)
		{
			return rightMouseButtonState;
		}

		bool isLeftDrag (void)
		{
			return leftMouseDrag;
		}

		bool isRightDrag (void)
		{
			return rightMouseDrag;
		}

		bool wasLeftDrag (void)
		{
			return wasLeftMouseDrag;
		}

		bool wasRightDrag (void)
		{
			return wasRightMouseDrag;
		}

		float getMouseDragX (void)
		{
			return mouseDragX * viewMulX;
		}

		float getMouseDragY (void)
		{
			return mouseDragY * viewMulY;
		}

		bool isLeftClick (void) {
			return(leftClick);
		}

		bool isRightClick (void) {
			return(rightClick);
		}

//		bool isLeftHeld() { 
//			return isLeftClick && lastLeftMouseButtonState == MC2_MOUSE_DOWN;
//		}

		bool isLeftDoubleClick (void)
		{
			return leftDoubleClick;
		}

		bool isRightDoubleClick (void)
		{
			return rightDoubleClick;
		}

		bool isMiddleClick (void)
		{
			return middleClick;
		}
		
		bool isMiddleDoubleClick (void)
		{
			return middleDoubleClick;
		}
		
		void setMouseCursor (long state);

		long getMouseCursor (void)
		{
			return (mouseState);
		}

		void setMouseFrame (long frameNum)
		{
			mouseFrame = frameNum;
		}

		long getMouseFrame (void)
		{
			return (mouseFrame);
		}
				
		void mouseOn (void);				//Draw Mouse Cursor
		void mouseOff (void);				//Don't Draw Mouse Cursor
		
		void pointerOn (void)
		{
			drawTerrainPointer = true;
		}
		
		void pointerOff (void)
		{
			drawTerrainPointer = false;
		}

		bool leftMouseReleased()
		{
			return leftMouseJustUp;
		}

		bool rightMouseReleased()
		{
			return rightMouseJustUp;
		}
		
		//------------------------------------------------------------------------------------
		// Used to make mouse move off into distance in perspective to help depth perception
		void setMouseScale (float scaleFactor);
		
		//----------------------------------------------------------
		// Attila Functionality.
		void addAttila (DWORD joyIndex)
		{
			attilaIndex = joyIndex;
		}
		
		float getAttilaXAxis (void)		//Left/Right Scroll
		{
			float result = 0.0f;
			if (attilaIndex != 0xffffffff)
				result = gosJoystick_GetAxis(attilaIndex, JOY_XAXIS);
				
			return result;			
		}
		
		float getAttilaYAxis (void)		//Up/Down Scroll
		{
			float result = 0.0f;
			if (attilaIndex != 0xffffffff)
				result = gosJoystick_GetAxis(attilaIndex, JOY_YAXIS);
				
			return result;			
		}
		
		float getAttilaRAxis (void)	//Camera Rotation
		{
			float result = 0.0f;
			if (attilaIndex != 0xffffffff)
				result = gosJoystick_GetAxis(attilaIndex, JOY_RZAXIS);
				
			return result;			
		}

		float getMouseRightHeld() 
		{
			return mouseRightHeldTime;
		}

		float getMouseLeftHeld() 
		{
			return mouseLeftHeldTime;
		}
		
		//------------------------
		// Poller
		void update (void);
		
		void render (void);						//Last thing rendered.  Draws Mouse.
};

extern UserInput *userInput;
//---------------------------------------------------------------------------
#endif
