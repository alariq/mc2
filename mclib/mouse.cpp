//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//-------------------------------------------------------------------
//
// This code will run in a separate callback to update the mouse
// positions at a fixed frame rate which is different from the game's
// 
// Essential for software and low frame rate situations
//
// Code is actually fairly simple BUT must work right ALL the time.
// As usual, debugging when a lock has been taken is often VERY hard.
//
// For each trip through the callback:
//	-if Screen has not been flipped since we were last here...
//		-Lock Rect Cursor WAS in.
//		-BLT old screen contents from mouseBuffer.
//		-Unlock Rect Cursor WAS in.
//	-Get Current Mouse X and Y.
//	-Lock Rect Cursor IS in.
//	-Copy Screen Contents of Rect to mouseBuffer.
// 	-BLT mouse cursor to Rect.
//	-Unlock Rect Cursor IS in.
//
// We should run this at a fixed rate of 10 to 20 Hz.
//
// This MUST block GameOS DisplayBackBuffer AND must NOT be called
// DURING GameOS DisplayBackBuffer.
//
// Cursors must be stored in a BLTable format for speed purposes.  32Bit TGA OK?
//
// Cursors must NOT be anything BUT keyed from now on.
//
// DisplayBackBuffer MUST do the following AFTER everything is done rendering
// and BEFORE it flips its buffers:
//	-Get Current Mouse X and Y.
//	-Lock Rect of BACKBUFFER Cursor IS in.
//	-Copy Screen Contents of Rect to mouseBuffer.
//	-BLT mouse cursor to Rect.
//	-Unlock Rect Cursor IS in.
//
//
//-----------------------------------------------------------------------

#include"dstd.h"
#include"userinput.h"
#include"platform_windows.h"
// sebi
#ifndef LINUX_BUILD
#include<mmsystem.h>
#include<ddraw.h>
#else
struct DDSURFACEDESC2 {
};
#endif

#include<gameos.hpp>

extern volatile bool mc2IsInDisplayBackBuffer;
extern volatile bool mc2IsInMouseTimer;
extern volatile bool mc2DisplayHasFlipped;
extern volatile bool mc2HasLostFocus;
volatile bool mc2MouseThreadStarted = false;
volatile bool mc2UseAsyncMouse = true;

MemoryPtr mouseBuffer = NULL;
#ifndef LINUX_BUILD
MMRESULT HTimer = 0;
#endif

RECT mouseWASInRect;

volatile char mc2MouseHotSpotX = 0;
volatile char mc2MouseHotSpotY = 0;
volatile char mc2MouseWidth = 32;
volatile char mc2MouseHeight = 32;

volatile MemoryPtr mc2MouseData = NULL;

//Timing in Hz to update mouse
long MOUSE_REFRESH_RATE = 30;

void (*AsynFunc)(RECT& WinRect,DDSURFACEDESC2& mouseSurfaceDesc ) = 0;
#ifndef LINUX_BUILD
void CALLBACK MouseTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);

//External GameOS stuff
extern IDirectDrawSurface7*	FrontBufferSurface;
extern HWND					hWindow;
extern POINT				clientToScreen;
extern DWORD 				MouseInWindow;
#endif // LINUX_BUILD

//
// Init the Mouse timer
//
void MouseTimerInit()
{
#ifndef LINUX_BUILD

	//Create mouseBuffer to hold screen contents under the mouse cursor.
	// This buffer MUST be at bit depth of screen!!
	// OR we could create a 32Bit buffer and convert during Blts both ways.
	// FASTEST would be created to screen Bit Depth BUT this means that 
	// the mouseBuffer must be recreated if we change to 32 bit rendering.
	// Should we even allow changing to 32-Bit Rendering?
	//
	// For now, create a 32Bit buffer and convert as needed.  If speed is an
	// issue on low end stuff, change it out.
	//
	mouseBuffer = (MemoryPtr)malloc(sizeof(DWORD) * MOUSE_WIDTH * MOUSE_WIDTH);
	memset(mouseBuffer,0,sizeof(DWORD) * MOUSE_WIDTH * MOUSE_WIDTH);
	
	if (!mc2MouseData)
	{
		mc2MouseData = (MemoryPtr)malloc(sizeof(DWORD) * MOUSE_WIDTH * MOUSE_WIDTH);
		memset(mc2MouseData,0,sizeof(DWORD) * MOUSE_WIDTH * MOUSE_WIDTH);
	}
	
	mc2MouseThreadStarted = true;
	mc2DisplayHasFlipped = true;

	mouseWASInRect.top = 0;
  	mouseWASInRect.left = 0;
	mouseWASInRect.right = 32;
	mouseWASInRect.bottom = 32;

	HTimer=timeSetEvent( 1000/MOUSE_REFRESH_RATE, 0, (LPTIMECALLBACK)MouseTimer, 0, TIME_PERIODIC );
#endif // LINUX_BUILD
}

//
// Clean up the mouse timer
//
void MouseTimerKill()
{
#ifndef LINUX_BUILD
	if(HTimer)
	{
		while (mc2IsInMouseTimer)
			;

		timeKillEvent(HTimer);
		
		while (mc2IsInMouseTimer)
			;
		
		HTimer=0;
	}
	
	mc2MouseThreadStarted = false;		//Keep us from calling silly stuff during startup and shutdown if we exception
	
	free(mouseBuffer);
	mouseBuffer = NULL;
	
	free(mc2MouseData);
	mc2MouseData = NULL;
#endif // LINUX_BUILD
}

//
// Returns the number of bits in a given mask.  Used to determine if we are in 555 mode vs 565 mode.
WORD GetNumberOfBits( DWORD dwMask )
{
    WORD wBits = 0;
    while( dwMask )
    {
        dwMask = dwMask & ( dwMask - 1 );  
        wBits++;
    }
    return wBits;
}
 

#ifndef LINUX_BUILD
//
// Actual Mouse Callback code here.
//
void CALLBACK MouseTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	HRESULT lockResult = -1;
	HRESULT unlockResult = -1;
	RECT WinRect;
	DDSURFACEDESC2 mouseSurfaceDesc;
	mouseSurfaceDesc.dwSize = sizeof(DDSURFACEDESC2);
	long screenX = Environment.screenWidth;
	long screenY = Environment.screenHeight;
	
	//Must immediately get out, We are NOT the main app anymore!!
	if (mc2HasLostFocus)
		return;
		
 	//Must immediately get out if are Flipping or Blting the display!!
	if (mc2IsInDisplayBackBuffer)
		return;
		
	//Called this while we are already in it, let it finish the last request first!!
	if (mc2IsInMouseTimer)
		return;
		
	//Thread hasn't started yet?  Should not be possible but insure safety.
	// These mouse thingies are NOTORIOUSLY hard to debug!!
	if (!mc2MouseThreadStarted)
		return;
	
	//For some unknown reason, our offscreen buffer is now NULL.  DO not allow this to run!!
	if (!mouseBuffer)
		return;

	//Warn everyone that we are updating the mouse now.
	// If we get to DisplayBackBuffer and this is true, WAIT until it goes false!!
	mc2IsInMouseTimer = true;
//
// Create a Rect with the actual screen dimensions in it.
//
	if( Environment.fullScreen )
	{
		GetWindowRect( hWindow, &WinRect );
	}
	else
	{
		WinRect.top 	= clientToScreen.y+1;
		WinRect.left 	= clientToScreen.x+1;
		WinRect.right	= clientToScreen.x + Environment.screenWidth - 1;
		WinRect.bottom	= clientToScreen.y + Environment.screenHeight - 1;
	}
	
	//	-if Screen has not been flipped since we were last here...
	if (!mc2DisplayHasFlipped)
	{
		//		-Lock Rect Cursor WAS in.
		if (FrontBufferSurface)
			lockResult = FrontBufferSurface->Lock(NULL,&mouseSurfaceDesc,DDLOCK_DONOTWAIT,NULL);
		else
			lockResult = -1;

		if (lockResult == DD_OK)
		{
			// call other functions first if any
			if ( AsynFunc )
				AsynFunc( WinRect, mouseSurfaceDesc );

			if (UserInput::drawMouse)
			{
				//Good.  Surface was ready to be locked, BLT our mouseBuffer to it.
				//		-BLT old screen contents from mouseBuffer.
				//
				// Get the pixel Format and use it to copy data from the mouseBuffer
				// to the screen.
				if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
				{
					MemoryPtr mBuffer = mouseBuffer;

					for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
					{
						MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
												((mouseWASInRect.left << 1) +
												(y * mouseSurfaceDesc.lPitch));

						for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
						{
							//Only copy bytes that are on screen.
							if ((x > WinRect.left) && (y > WinRect.top))
							{
								//Copy first byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;
	
								//Copy second byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;
							}
							else
							{
								//Just iterate across the screen and buffer
								//
								//Skip first byte
								mBuffer++;
								screenPos++;

								//Skip second byte
								mBuffer++;
								screenPos++;
							}
						}
					}
				}
				else if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 24) 
				{
					MemoryPtr mBuffer = mouseBuffer;
					for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
					{
						MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
												((mouseWASInRect.left * 3) +
												(y * mouseSurfaceDesc.lPitch));

						for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
						{
							//Only copy bytes that are on screen.
							if ((x > WinRect.left) && (y > WinRect.top))
							{
								//Copy first byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;

								//Copy second byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;

								//Copy third byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;
							}
							else
							{
								//Just iterate across the screen and buffer
								//
								//Skip first byte
								mBuffer++;
								screenPos++;

								//Skip second byte
								mBuffer++;
								screenPos++;

								//Skip third byte
								mBuffer++;
								screenPos++;
							}
						}
					}
				}
				else if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32) 
				{
					MemoryPtr mBuffer = mouseBuffer;
					for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
					{
						MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
												((mouseWASInRect.left << 2) +
												(y * mouseSurfaceDesc.lPitch));

						for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
						{
							//Only copy bytes that are on screen.
							if ((x > WinRect.left) && (y > WinRect.top))
							{
								//Copy first byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;

								//Copy second byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;

								//Copy third byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;

								//Copy fourth byte
								*screenPos = *mBuffer;
								mBuffer++;
								screenPos++;
							}
							else
							{
								//Just iterate across the screen and buffer
								//
								//Skip first byte
								mBuffer++;
								screenPos++;

								//Skip second byte
								mBuffer++;
								screenPos++;

								//Skip third byte
								mBuffer++;
								screenPos++;

								//Skip fourth byte
								mBuffer++;
								screenPos++;
							}
						}
					}
				}
				else
				{
					//We are in some WAY unknown video mode.  Just get out.
					unlockResult = FrontBufferSurface->Unlock(&mouseWASInRect);
					if (unlockResult != DD_OK)
					{
						//I have no idea how this could happen, but if it does, Just move on?
						// If the surface was not locked, something is really wrong.
						// We could LOSE the surface by switching focus but we lost the lock then anyway.
					}
					goto mouseTimerDone;
				}
			}
		}
		else
		{
			//ANY other situation means that for some reason the surface was unavailable.
			// We may have lost focus, GameOS is doing something stupid OR
			// some other unforseen event.  Just don't update it.
			// We do not need to unlock.  It was never locked.
			goto mouseTimerDone;
		}
		
		// Must now Unlock to the rect.
		//		-Unlock Rect Cursor WAS in.
		unlockResult = FrontBufferSurface->Unlock(&mouseWASInRect);
		if (unlockResult != DD_OK)
		{
			//I have no idea how this could happen, but if it does, Just move on?
			// If the surface was not locked, something is really wrong.
			// We could LOSE the surface by switching focus but we lost the lock then anyway.
		}
	}

	//Check to see if we should even bother drawing the mouse.  They may wanyt us OFF!!
	if (UserInput::drawMouse)
	{
		//	-Get Current Mouse X and Y.
		// Copy code from GameOS here.  DO NOT CALL GAMEOS FUNCTIONS!!!!!!
		// They are almost certainly not re-entrant!!
	//
	// Get absolute mouse position using window calls
	//
		POINT pt;
		GetCursorPos(&pt);
	//
	// Determine if cursor is out of WinRect.  If so, clamp it back in.
	//
		if (pt.x < WinRect.left)
		{
			pt.x = WinRect.left;
		}
		else if (pt.x >= WinRect.right)
		{
			pt.x = WinRect.right;
		}
		
		if (pt.y < WinRect.top)
		{
			pt.y = WinRect.top;
		}
		else if (pt.y >= WinRect.bottom)
		{
			pt.y = WinRect.bottom;
		}
		
		//At this point, pt.x and pt.y contain the relative coords of the mouse in
		// OUR window space.
		if (!Environment.fullScreen)
		{
			screenX = WinRect.right;
			screenY = WinRect.bottom;
		}
		
		//	-Lock Rect Cursor IS in.
		// In order to do this, you need the current mouse cursor hot spot.
		// the mouse HOTSPOT is at the pt coordinates.  The rect is relative to that.
		mouseWASInRect.left = pt.x - mc2MouseHotSpotX;
		mouseWASInRect.top = pt.y - mc2MouseHotSpotY;
		mouseWASInRect.right = mouseWASInRect.left + mc2MouseWidth;
		mouseWASInRect.bottom = mouseWASInRect.top + mc2MouseHeight;
		
		if (mouseWASInRect.left >= (screenX-1))
		{
			mouseWASInRect.left = screenX-1;
		}
		
		if (mouseWASInRect.top >= (screenY-1))
		{
			mouseWASInRect.top = screenY-1;
		}
		
		if (mouseWASInRect.right >= (screenX-1))
		{
			mouseWASInRect.right = screenX-1;
		}
		
		if (mouseWASInRect.bottom >= (screenY-1))
		{
			mouseWASInRect.bottom = screenY-1;
		}
		
		//Lock can wait because the above unlock might not have been done baking yet!
		if (FrontBufferSurface)
			lockResult = FrontBufferSurface->Lock(NULL,&mouseSurfaceDesc,DDLOCK_WAIT,NULL);
		else
			lockResult = -1;

		if (lockResult == DD_OK)
		{
			//Good.  Lock was OK.
			//	-Copy Screen Contents of Rect to mouseBuffer.
			// Get the pixel Format and use it to copy data from the screen
			// to the mouseBuffer.
			// 
			// Our new mouseRect may include negative numbers which are off screen.
			// Clip so that ALL numbers are greater then zero!!
			//
			if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
			{
				MemoryPtr mBuffer = mouseBuffer;
				for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
				{
					MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
											((mouseWASInRect.left << 1) +
											(y * mouseSurfaceDesc.lPitch));
											
					for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
					{
						//Only copy bytes that are on screen.
						if ((x > WinRect.left) && (y > WinRect.top))
						{
							//Copy first byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
							
							//Copy second byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
						}
						else
						{
							//Just iterate across the screen and buffer
							//
							//Skip first byte
							mBuffer++;
							screenPos++;
							
							//Skip second byte
							mBuffer++;
							screenPos++;
						}
					}
				}
			}
			else if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 24) 
			{
				MemoryPtr mBuffer = mouseBuffer;
				for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
				{
					MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
											((mouseWASInRect.left * 3) +
											(y * mouseSurfaceDesc.lPitch));
											
					for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
					{
						//Only copy bytes that are on screen.
						if ((x > WinRect.left) && (y > WinRect.top))
						{
							//Copy first byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
							
							//Copy second byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
							
							//Copy third byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
						}
						else
						{
							//Just iterate across the screen and buffer
							//
							//Skip first byte
							mBuffer++;
							screenPos++;
							
							//Skip second byte
							mBuffer++;
							screenPos++;
							
							//Skip third byte
							mBuffer++;
							screenPos++;
						}
					}
				}
			}
			else if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32) 
			{
				MemoryPtr mBuffer = mouseBuffer;
				for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
				{
					MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
											((mouseWASInRect.left << 2) +
											(y * mouseSurfaceDesc.lPitch));
											
					for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
					{
						//Only copy bytes that are on screen.
						if ((x > WinRect.left) && (y > WinRect.top))
						{
							//Copy first byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
							
							//Copy second byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
							
							//Copy third byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
							
							//Copy fourth byte
							*mBuffer = *screenPos;
							mBuffer++;
							screenPos++;
						}
						else
						{
							//Just iterate across the screen and buffer
							//
							//Skip first byte
							mBuffer++;
							screenPos++;
							
							//Skip second byte
							mBuffer++;
							screenPos++;
							
							//Skip third byte
							mBuffer++;
							screenPos++;
							
							//Skip fourth byte
							mBuffer++;
							screenPos++;
						}
					}
				}
			}
			else
			{
				//We are in some WAY unknown video mode.  Just get out.
				unlockResult = FrontBufferSurface->Unlock(&mouseWASInRect);
				if (unlockResult != DD_OK)
				{
					//I have no idea how this could happen, but if it does, Just move on?
					// If the surface was not locked, something is really wrong.
					// We could LOSE the surface by switching focus but we lost the lock then anyway.
				}
				goto mouseTimerDone;
			}
			
			// 	-BLT mouse cursor to Rect.
			// mc2MouseData should contain a 32-bit image of the mouse cursor.
			// BLT it to the mouseSurface.
			//
			// Again, the mouseRect may contain negative numbers.
			// This means do NOT BLT until numbers are greater then Zero
			// Keep iterating across the mouse shape, though!!
			if (mc2MouseData)
			{
				bool in555Mode = false;
				if (GetNumberOfBits(mouseSurfaceDesc.ddpfPixelFormat.dwGBitMask) == 5)
					in555Mode = true;

				if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
				{
					DWORD * mData = (DWORD *)mc2MouseData;
					for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
					{
						MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
												((mouseWASInRect.left << 1) +
												(y * mouseSurfaceDesc.lPitch));
												
						for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
						{
							//We are pointing at the top left corner of where 
							// the mouse cursor will draw.  mc2MouseData is a 32bit
							// bitmap with Alpha information.  Go!

							DWORD mColor = *mData;
							BYTE baseAlpha 		= 0;
							BYTE baseColorRed	= (mColor & 0x00ff0000)>>16;
							BYTE baseColorGreen	= (mColor & 0x0000ff00)>>8;
							BYTE baseColorBlue 	= (mColor & 0x000000ff);
							
							//Check for color key instead
							if ((baseColorRed != 0xff) || (baseColorGreen != 0x0) || (baseColorBlue != 0xff))
								baseAlpha = 0x1;
							
							// Check if our rect is off screen and do not BLT!!
							if (baseAlpha && (x > WinRect.left) && (y > WinRect.top))
							{
								//Create a 16Bit color value to jam into the screen.
								unsigned short clr = 0;
								if (!in555Mode)
								{
									clr = (baseColorRed >> 3) << 11;
									clr += (baseColorGreen >> 2) << 5;
									clr += (baseColorBlue >> 3);
								}
								else
								{
									clr = (baseColorRed >> 3) << 10;
									clr += (baseColorGreen >> 3) << 5;
									clr += (baseColorBlue >> 3);
								}
								
								*screenPos = clr & 0xff;
								screenPos++;
								*screenPos = clr >> 8;
								screenPos++;
							}
							else
							{
								screenPos++;
								screenPos++;
							}
							
							//Always iterate across shape, even if not drawn
							mData++;
						}
						
						//Skip portion of mouse which is off bottom or right.
						mData += mc2MouseWidth - (mouseWASInRect.right - mouseWASInRect.left);
					}
				}
				else if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 24) 
				{
					DWORD * mData = (DWORD *)mc2MouseData;
					for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
					{
						MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
												((mouseWASInRect.left * 3) +
												(y * mouseSurfaceDesc.lPitch));
												
						for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
						{
							//We are pointing at the top left corner of where 
							// the mouse cursor will draw.  mc2MouseData is a 32bit
							// bitmap with Alpha information.  Go!
							DWORD mColor = *mData;
							BYTE baseAlpha 		= 0;
							BYTE baseColorRed	= (mColor & 0x00ff0000)>>16;
							BYTE baseColorGreen	= (mColor & 0x0000ff00)>>8;
							BYTE baseColorBlue 	= (mColor & 0x000000ff);
							
							//Check for color key instead
							if ((baseColorRed != 0xff) || (baseColorGreen != 0x0) || (baseColorBlue != 0xff))
								baseAlpha = 0x1;
							
							// Check if our rect is off screen and do not BLT!!
							if (baseAlpha && (x > WinRect.left) && (y > WinRect.top))
							{
								//Just JAM colors into the screen
								// TRUE COLOR!!
								*screenPos = baseColorBlue;
								screenPos++;
								*screenPos = baseColorGreen;
								screenPos++;
								*screenPos = baseColorRed;
								screenPos++;
							}
							else
							{
								screenPos++;
								screenPos++;
								screenPos++;
							}
							
							//Always iterate across shape, even if not drawn
							mData++;
						}
						
						//Skip portion of mouse which is off bottom or right.
						mData += mc2MouseWidth - (mouseWASInRect.right - mouseWASInRect.left);
					}
				}
				else if (mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32) 
				{
					DWORD * mData = (DWORD *)mc2MouseData;
					for (long y=mouseWASInRect.top;y<mouseWASInRect.bottom;y++)
					{
						MemoryPtr screenPos = (MemoryPtr)mouseSurfaceDesc.lpSurface +
												((mouseWASInRect.left << 2) +
												(y * mouseSurfaceDesc.lPitch));
												
						for (long x=mouseWASInRect.left;x<mouseWASInRect.right;x++)
						{
							//We are pointing at the top left corner of where 
							// the mouse cursor will draw.  mc2MouseData is a 32bit
							// bitmap with Alpha information.  Go!
							DWORD mColor = *mData;
							BYTE baseAlpha 		= 0;
							BYTE baseColorRed	= (mColor & 0x00ff0000)>>16;
							BYTE baseColorGreen	= (mColor & 0x0000ff00)>>8;
							BYTE baseColorBlue 	= (mColor & 0x000000ff);
							
							//Check for color key instead
							if ((baseColorRed != 0xff) || (baseColorGreen != 0x0) || (baseColorBlue != 0xff))
								baseAlpha = 0x1;
							
							// Check if our rect is off screen and do not BLT!!
							if (baseAlpha && (x > WinRect.left) && (y > WinRect.top))
							{
								//Just JAM colors into the screen
								// TRUE COLOR!!
								*screenPos = baseColorBlue;
								screenPos++;
								*screenPos = baseColorGreen;
								screenPos++;
								*screenPos = baseColorRed;
								screenPos++;
								screenPos++;	//Jam one more for 32 Bits
							}
							else
							{
								screenPos++;
								screenPos++;
								screenPos++;
								screenPos++;	//Jam one more for 32 Bits
							}
							
							//Always iterate across shape, even if not drawn
							mData++;
						}
						
						//Skip portion of mouse which is off bottom or right.
						mData += mc2MouseWidth - (mouseWASInRect.right - mouseWASInRect.left);
					}
				}
			}
			
			//	-Unlock Rect Cursor IS in.
			unlockResult = FrontBufferSurface->Unlock(&mouseWASInRect);
			if (unlockResult != DD_OK)
			{
				//I have no idea how this could happen, but if it does, Just move on?
				// If the surface was not locked, something is really wrong.
				// We could LOSE the surface by switching focus but we lost the lock then anyway.
			}
		}
		else
		{
			//ANY other situation means that for some reason the surface was unavailable.
			// We may have lost focus, GameOS is doing something stupid OR
			// some other unforseen event.  Just don't update it.
			// We do not need to unlock.  It was never locked.
			goto mouseTimerDone;
		}	
	}


mouseTimerDone:
	
	//ALL done.  It is now safe to allow DisplayBackBuffer to run.
	mc2IsInMouseTimer = false;
	mc2DisplayHasFlipped = false;
}

#endif // LINUX_BUILD

//-------------------------------------------------------------------
