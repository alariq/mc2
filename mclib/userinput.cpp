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
#include"userinput.h"
#endif

#ifndef TIMING_H
#include"timing.h"
#endif

#ifndef CIDENT_H
#include"cident.h"
#endif

#ifndef INIFILE_H
#include"inifile.h"
#endif

#ifndef PATHS_H
#include"paths.h"
#endif

#ifndef CLIP_H
#include"clip.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#include<windows.h>

#include<stuff/stuff.hpp>
#include<math.h>

//---------------------------------------------------------------------------
UserInput *userInput = NULL;
extern bool 			hasGuardBand;
volatile bool			UserInput::drawMouse = false;
extern volatile bool 	mc2IsInDisplayBackBuffer;
extern volatile bool 	mc2IsInMouseTimer;

void MouseTimerInit();
void MouseTimerKill();
//---------------------------------------------------------------------------
void MouseCursorData::initCursors (const char *cursorFileName)
{
	//New
	// add an "a" to the end of the cursorFileName IF we are running in 800x600 or less.
	// Loads different sized cursors.
	char realHackName[1024];
	strcpy(realHackName,cursorFileName);
	if (Environment.screenWidth <= 800)
		sprintf(realHackName,"%sa",cursorFileName);

	FullPathFileName cursorName;
	cursorName.init(artPath,realHackName,".fit");
	
	FitIniFile cursorFile;
	long result = cursorFile.open(cursorName);
	gosASSERT(result == NO_ERR);
	
	result = cursorFile.seekBlock("Main");
	gosASSERT(result == NO_ERR);

	result = cursorFile.readIdLong("NumCursors",numCursors);
	gosASSERT(result == NO_ERR);

	gosASSERT( numCursors < MAX_MOUSE_STATES );

	cursorInfos = new StaticInfo[numCursors];
		
	//----------------------------------------------
	// Each cursor is defined as a number of frames
	// and a TGA File Name which we use to create
	// the texture handle.
	char blockName[32];
	for (long i=0;i<numCursors;i++)
	{
		sprintf( blockName, "Cursor%ld", i );
		cursorInfos[i].init( cursorFile, blockName,0,0,0x1);
		cursorFile.readIdChar( "HotSpotX", mouseHS[i][0] );
		cursorFile.readIdChar( "HotSpotY", mouseHS[i][1] );
		cursorFile.readIdULong( "NumFrames", numFrames[i] );
		cursorFile.readIdFloat( "FrameLength", frameLengths[i] );

	}
	
	cursorFile.close();
}	

//---------------------------------------------------------------------------
void MouseCursorData::destroy (void)
{
	if (mc2UseAsyncMouse && mc2MouseThreadStarted)
		MouseTimerKill();

	if (numCursors)
	{
		userInput->mouseOff();
		
		if ( cursorInfos )
		{
			delete [] cursorInfos;
			cursorInfos = NULL;
		}

		numCursors = 0;
	}
}	

//---------------------------------------------------------------------------
void UserInput::mouseOn (void)				//Draw Mouse Cursor
{
	drawMouse = true;
}

void UserInput::mouseOff (void)				//Don't Draw Mouse Cursor
{
	drawMouse = false;
}

void UserInput::setMouseCursor (long state)
{
	if ((state < 0) || (state >= mState_NUMMOUSESTATES))
		return;

	//DEBUG!!!!!!!!!!!!!!!!!!!!!!
	if (state == mState_NORMAL)
		printf("Go Flash");

	mouseState = state;
}

//---------------------------------------------------------------------------
void UserInput::update (void)
{
	if (mc2UseAsyncMouse && !mc2MouseThreadStarted)
		MouseTimerInit();
		
	if (!mc2UseAsyncMouse && mc2MouseThreadStarted)
		MouseTimerKill();

	//-----------------------------
	// Save the last Mouse States
	lastLeftMouseButtonState = leftMouseButtonState;
	lastRightMouseButtonState = rightMouseButtonState;

	lastMouseXPosition = mouseXPosition;
	lastMouseYPosition = mouseYPosition;

	leftMouseJustUp = 0;
	rightMouseJustUp = 0;

	bool bWasDouble = leftDoubleClick;

	//------------------------------------------------------
	// Reset Frame dependant variables
	leftClick = rightClick = middleClick = false;
	leftDoubleClick = rightDoubleClick = middleDoubleClick = false;

	
	DWORD LEFT_MOUSE_CODE = VK_LBUTTON;
	DWORD RIGHT_MOUSE_CODE = VK_RBUTTON;
	if ( GetSystemMetrics(SM_SWAPBUTTON) )
	{
		RIGHT_MOUSE_CODE = VK_LBUTTON;
		LEFT_MOUSE_CODE = VK_RBUTTON;
	}

	//-----------------
	// Poll the mouse.
	DWORD buttonStates;
	gos_GetMouseInfo(&mouseXPosition,&mouseYPosition,(int *)&mouseXDelta,(int *)&mouseYDelta,(int *)&mouseWheelDelta,&buttonStates);
//	leftMouseButtonState = buttonStates & 1;
//	rightMouseButtonState = (buttonStates & 2) >> 1;
//	middleMouseButtonState = (buttonStates & 4) >> 2;

	SHORT code = GetAsyncKeyState(LEFT_MOUSE_CODE);
	SHORT rCode = GetAsyncKeyState( RIGHT_MOUSE_CODE );
	SHORT mCode = GetAsyncKeyState( VK_MBUTTON );
	leftMouseButtonState = code ? MC2_MOUSE_DOWN : MC2_MOUSE_UP;
	rightMouseButtonState = rCode ? MC2_MOUSE_DOWN : MC2_MOUSE_UP;
	middleMouseButtonState = mCode ? MC2_MOUSE_DOWN : MC2_MOUSE_UP;

	//---------------------------------------------------------
	// Adjust MouseWheelDelta to get old Broken Win2K values.
	mouseWheelDelta *= -100;

	//-------------------------------------
	// Determine drag, double click states
	if ((leftMouseButtonState == MC2_MOUSE_UP) && (lastLeftMouseButtonState == MC2_MOUSE_DOWN))
	{
		//--------------------------------------------------------------------
		// Just lifted the button.  Drags are OFF!  Double Click clock starts!
		wasLeftMouseDrag = leftMouseDrag;
		wasRightMouseDrag = rightMouseDrag;
		
		leftMouseDrag = false;
		mouseLeftUpTime += frameLength;
//		if ( !bWasDouble )
			leftMouseJustUp = 1;

		mouseLeftHeldTime = 0.f;
	}

	if ((leftMouseButtonState == MC2_MOUSE_UP) && (lastLeftMouseButtonState == MC2_MOUSE_UP))
	{
		//--------------------------------------------
		// We are still up.  Increment mouse up time.
		mouseLeftUpTime += frameLength;
		mouseLeftHeldTime = 0.f;
	}

	if ( gos_GetKeyStatus( KEY_LMOUSE ) == KEY_PRESSED /*code & 0x0001*/ ) // clicked
	{
		//-------------------------------------------------------------------------------
		// We just clicked down.  If mouseUpTime is < threshold, this was a double click
		if ( (mouseLeftUpTime > 0.0 ) && (mouseLeftUpTime < mouseDblClickThreshold) && !bWasDouble)
		{
			leftDoubleClick = true;
			mouseLeftUpTime = 0.f; // make sure if we hold it, we don't keep clicking
		}
		else  
		{
			leftClick = true;
			mouseLeftUpTime = 0.001f;			
		}

		mouseDragX = lastMouseXPosition;
		mouseDragY = lastMouseYPosition;

		leftMouseDrag = 0;
		

	}

	else if (gos_GetKeyStatus( KEY_LMOUSE ) == KEY_HELD /*code & 0x8000*/) // held
	{
		mouseLeftUpTime = 5.0;
		//-----------------------------------------------------------------------
		// Down and still down should indicate dragging.  Check threshold of 
		// Deltas to see if this is TRUE IF AND ONLY IF we are NOT yet DRAGGING!
		if (!leftMouseDrag)
		{
			if ((fabs(mouseDragX - mouseXPosition) > mouseDragThreshold) ||
				(fabs(mouseDragY - mouseYPosition) > mouseDragThreshold))
			{
				//------------------
				// We are dragging.
				leftMouseDrag = true;
			}
		}

		mouseLeftUpTime = 0;
		mouseLeftHeldTime += frameLength;

	}

	if ((rightMouseButtonState == MC2_MOUSE_UP) && (lastRightMouseButtonState == MC2_MOUSE_DOWN))
	{
		//--------------------------------------------------------------------
		// Just lifted the button.  Drags are OFF!  Double Click clock starts!
		wasRightMouseDrag = rightMouseDrag;
		rightMouseDrag = false;
		mouseRightUpTime = 0.001f;
		rightMouseJustUp = true;

		mouseRightHeldTime = 0.f;
	}

	if ((rightMouseButtonState == MC2_MOUSE_UP) && (lastRightMouseButtonState == MC2_MOUSE_UP))
	{
		//--------------------------------------------
		// We are still up.  Increment mouse up time.
		mouseRightUpTime += frameLength;
		rightMouseDrag = 0;
		mouseRightHeldTime = 0.f;
	}

	if (gos_GetKeyStatus( KEY_RMOUSE ) == KEY_PRESSED /*rCode & 0x0001*/)
	{
		//-------------------------------------------------------------------------------
		// We just clicked down.  If mouseUpTime is < threshold, this was a double click
		if (mouseRightUpTime && mouseRightUpTime < mouseDblClickThreshold)
			rightDoubleClick = true;
		else
			rightClick = true;

		mouseDragX = lastMouseXPosition;
		mouseDragY = lastMouseYPosition;

		rightMouseDrag = 0;
	}
	else if (gos_GetKeyStatus( KEY_RMOUSE ) == KEY_HELD /*rCode & 0x8000*/)
	{
		//-----------------------------------------------------------------------
		// Down and still down should indicate dragging.  Check threshold of 
		// Deltas to see if this is TRUE IF AND ONLY IF we are NOT yet DRAGGING!
		if (!rightMouseDrag)
		{
			if ((fabs(mouseDragX - mouseXPosition) > mouseDragThreshold) ||
				(fabs(mouseDragY - mouseYPosition) > mouseDragThreshold))
			{		 
				//------------------
				// We are dragging.
				rightMouseDrag = true;
			}
		}

		rightClick = true;

		mouseRightUpTime = 0;
		mouseRightHeldTime += frameLength;
	}

	if ((middleMouseButtonState == MC2_MOUSE_UP) && (lastMiddleMouseButtonState == MC2_MOUSE_DOWN))
	{
		//--------------------------------------------------------------------
		// Just lifted the button.  Drags are OFF!  Double Click clock starts!
		mouseMiddleUpTime = 0.0;
	}

	if ((middleMouseButtonState == MC2_MOUSE_UP) && (lastMiddleMouseButtonState == MC2_MOUSE_UP))
	{
		//--------------------------------------------
		// We are still up.  Increment mouse up time.
		mouseMiddleUpTime += frameLength;
	}

	if (gos_GetKeyStatus( KEY_MMOUSE ) == KEY_PRESSED /*mCode & 0x0001*/)
	{
		//-------------------------------------------------------------------------------
		// We just clicked down.  If mouseUpTime is < threshold, this was a double click
		if (mouseMiddleUpTime < mouseDblClickThreshold)
			middleDoubleClick = true;
		else
			middleClick = true;
	}

	if (cursors->getNumFrames( mouseState ) > 1 )
	{
		mouseFrameLength += frameLength;
		if (mouseFrameLength > cursors->frameLengths[mouseState] )
		{
			mouseFrame++;
			if (mouseFrame >= cursors->getNumFrames(mouseState))
			{
				mouseFrame = 0;
			}

			mouseFrameLength = 0.0;
		}
	}
	
	if (mc2UseAsyncMouse)
	{
		//Wait for thread to finish.  Otherwise, we may move its data buffer halfway through!!
		while (mc2IsInMouseTimer)
			;

		//ONLY set the mouse BLT data at the end of each update.  NO MORE FLICKERING THEN!!!
		// BLOCK THREAD WHILE THIS IS HAPPENING
		mc2IsInDisplayBackBuffer = true;

		if (!mc2MouseData)
		{
			mc2MouseData = (MemoryPtr)malloc(sizeof(DWORD) * MOUSE_WIDTH * MOUSE_WIDTH);
			memset(mc2MouseData,0,sizeof(DWORD) * MOUSE_WIDTH * MOUSE_WIDTH);
		}

		//Need to update the mouse in the mouse thread to inform it that the cursor
		// possibly changed size and shape.
		mc2MouseHotSpotX = cursors->getMouseHSX( mouseState );
		mc2MouseHotSpotY = cursors->getMouseHSY( mouseState );

		mc2MouseWidth = cursors->cursorInfos[mouseState].width();
		mc2MouseHeight = cursors->cursorInfos[mouseState].height();

		DWORD totalMouseFrames = cursors->getNumFrames(mouseState);
 		if ( totalMouseFrames > 1 )
		{
			long framesPerRow = cursors->cursorInfos[mouseState].textureWidth/cursors->cursorInfos[mouseState].width();
			int iIndex = mouseFrame % framesPerRow;
			int jIndex = mouseFrame / framesPerRow;

			float oldU = cursors->cursorInfos[mouseState].u;
			float oldV = cursors->cursorInfos[mouseState].v;

			float newU = (.1f + oldU)/cursors->cursorInfos[mouseState].textureWidth + ((float)iIndex * cursors->cursorInfos[mouseState].width()/cursors->cursorInfos[mouseState].textureWidth);
			float newV = (.1f + oldV)/cursors->cursorInfos[mouseState].textureWidth + (float)jIndex * cursors->cursorInfos[mouseState].height()/cursors->cursorInfos[mouseState].textureWidth;

			float newU2 = newU + (cursors->cursorInfos[mouseState].width() + .1)/cursors->cursorInfos[mouseState].textureWidth;
			float newV2 = newV + (cursors->cursorInfos[mouseState].height() + .1)/cursors->cursorInfos[mouseState].textureWidth;

			cursors->cursorInfos[mouseState].setNewUVs( newU, newV, newU2, newV2 );
			cursors->cursorInfos[mouseState].getData(mc2MouseData);
			cursors->cursorInfos[mouseState].u = oldU;
			cursors->cursorInfos[mouseState].v = oldV;
		}
		else if (totalMouseFrames)
		{
			cursors->cursorInfos[mouseState].getData(mc2MouseData);
		}

		//Unblock Thread
		mc2IsInDisplayBackBuffer = false;
	}
}

//---------------------------------------------------------------------------
void UserInput::initMouseCursors (const char *mouseFile)
{
	if (cursors)
	{
		cursors->destroy();
		delete cursors;
		cursors = NULL;
	}

	cursors = new MouseCursorData;
	gosASSERT(cursors != NULL);
	
	cursors->initCursors(mouseFile);

	mouseFrame = 0;
}
	
//---------------------------------------------------------------------------
float smallTextureTLUVX[4] = 
{
	0.00,
	0.50,
	0.00,
	0.50
};

float smallTextureTLUVY[4] = 
{
	0.00,
	0.00,
	0.50,
	0.50
};

float smallTextureBRUVX[4] = 
{
	0.50,
	1.00,
	0.50,
	1.00
};

float smallTextureBRUVY[4] = 
{
	0.50,
	0.50,
	1.00,
	1.00
};

//---------------------------------------------------------------------------
float mediumTextureTLUVX[16] = 
{
	0.00,
	0.25,
	0.50,
	0.75,
	0.00,
	0.25,
	0.50,
	0.75,
	0.00,
	0.25,
	0.50,
	0.75,
	0.00,
	0.25,
	0.50,
	0.75
};

float mediumTextureTLUVY[16] = 
{
	0.00,
	0.00,
	0.00,
	0.00,
	0.25,
	0.25,
	0.25,
	0.25,
	0.50,
	0.50,
	0.50,
	0.50,
	0.75,
	0.75,
	0.75,
	0.75
};

float mediumTextureBRUVX[16] = 
{
	0.25,
	0.50,
	0.75,
	1.00,
	0.25,
	0.50,
	0.75,
	1.00,
	0.25,
	0.50,
	0.75,
	1.00,
	0.25,
	0.50,
	0.75,
	1.00
};

float mediumTextureBRUVY[16] = 
{
	0.25,
	0.25,
	0.25,
	0.25,
	0.50,
	0.50,
	0.50,
	0.50,
	0.75,
	0.75,
	0.75,
	0.75,
	1.00,
	1.00,
	1.00,
	1.00
};

//---------------------------------------------------------------------------
float largeTextureTLUVX[64] = 
{
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	0.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875
};

float largeTextureTLUVY[64] = 
{
	0.00,
	0.00,
	0.00,
	0.00,
	0.00,
	0.00,
	0.00,
	0.00,
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875
};

float largeTextureBRUVX[64] = 
{
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00,
	0.125,
	0.25,
	0.375,
	0.50,
	0.625,
	0.75,
	0.875,
	1.00
};

float largeTextureBRUVY[64] = 
{
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.125,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.25,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.375,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.50,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.625,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.75,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875,
	0.875,
	1.00,
	1.00,
	1.00,
	1.00,
	1.00,
	1.00,
	1.00,
	1.00
};

//---------------------------------------------------------------------------
void UserInput::setMouseScale (float scaleFactor)
{
	if (scaleFactor > 0.0f)
		mouseScale = scaleFactor;
}

//---------------------------------------------------------------------------
void UserInput::render (void)						//Last thing rendered.  Draws Mouse.
{
	if (!mc2UseAsyncMouse)
	{
		if (drawMouse && mouseState != -1)
		{
			// figure out where to put the thing
			long mouseX = getMouseX();
			long mouseY = getMouseY();
	
			mouseX -= cursors->getMouseHSX( mouseState );
			mouseY -= cursors->getMouseHSY( mouseState );
	
			cursors->cursorInfos[mouseState].setLocation( mouseX, mouseY );
	
			long totalMouseFrames = cursors->getNumFrames(mouseState);
	
			if ( totalMouseFrames > 1 )
			{
				long framesPerRow = cursors->cursorInfos[mouseState].textureWidth/cursors->cursorInfos[mouseState].width();
				int iIndex = mouseFrame % framesPerRow;
				int jIndex = mouseFrame / framesPerRow;

				float oldU = cursors->cursorInfos[mouseState].u;
				float oldV = cursors->cursorInfos[mouseState].v;
	
				float newU = (.1f + oldU)/cursors->cursorInfos[mouseState].textureWidth + ((float)iIndex * cursors->cursorInfos[mouseState].width()/cursors->cursorInfos[mouseState].textureWidth);
				float newV = (.1f + oldV)/cursors->cursorInfos[mouseState].textureWidth + (float)jIndex * cursors->cursorInfos[mouseState].height()/cursors->cursorInfos[mouseState].textureWidth;
	
				float newU2 = newU + (cursors->cursorInfos[mouseState].width() + .1)/cursors->cursorInfos[mouseState].textureWidth;
				float newV2 = newV + (cursors->cursorInfos[mouseState].height() + .1)/cursors->cursorInfos[mouseState].textureWidth;
	
				cursors->cursorInfos[mouseState].setNewUVs( newU, newV, newU2, newV2 );
				cursors->cursorInfos[mouseState].render();
				cursors->cursorInfos[mouseState].u = oldU;
				cursors->cursorInfos[mouseState].v = oldV;



			}
			else
				cursors->cursorInfos[mouseState].render();
		}
	}
}	

//---------------------------------------------------------------------------
