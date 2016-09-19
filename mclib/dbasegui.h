//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//--------------------------------------------------------------------------------------
//
// MechCommander 2
//
// This header contains the base classes for the GUI
//
// GUI is now single update driven.  An event comes in, the manager decides who its for
// and passes the event down.  Eveything still know how to draw etc.
//
// All drawing is done through gos_drawQuad and drawTriangle
//
// Basic cycle is 
//		call gObject->update with this frame's events.
//			This will check the events to see if any pertain to me.
//			Draw anything to the texture plane that needs drawing.
//			Call any code which the events might have trigged.
//		call gObject->render with this frame's events.
//			This draws the object to the screen.
//			called in order of depth.
// 
//--------------------------------------------------------------------------------------

#ifndef DBASEGUI_H
#define DBASEGUI_H
//--------------------------------------------------------------------------------------
// Include Files
//--------------------------------------------------------------------------------------
#define gtINVALID			-1
#define gtBACKGROUND		0
#define gtBUTTON			1
#define gtTEXT				2
#define gtHOTTEXT			3
#define gtEDIT				4
#define gtCHECKBOX			5
#define gtSPINNER			6
#define gtLISTBOX			7
#define gtTABCTRL			8
#define gtDRAGOBJECT		9
#define gtMENU				10
#define gtDIALOG			11

#define aMSG_NOTHING		-1
#define aMSG_MOUSE_OVER		0
#define aMSG_BUTTON_DOWN	1
#define aMSG_BUTTON_UP		2
#define aMSG_BUTTON_GRAY	3
#define aMSG_MOUSE_MOVE		4
#define aMSG_SELCHANGE		5
#define aMSG_CANCEL			6
#define aMSG_OK				7

#define aBTN_NEWCAMPAIGN	0
#define aBTN_LOADMISSION	1
#define aBTN_SAVEMISSION	2
#define aBTN_MULTIPLAYER	3
#define aBTN_PREFERENCES	4
#define aBTN_OPENINGCIN		5
#define aBTN_QUIT			6
#define aBTN_RETURNGAME		7

// Status bar interface colors
#define SD_YELLOW	((0x00) + (0xff<<8) + (0xff<<16) + (0xff<<24))
#define SD_RED		((0x00) + (0x00<<8) + (0xff<<16) + (0xff<<24))
#define SD_GREEN	((0x00) + (0xff<<8) + (0x00<<16) + (0xff<<24))
#define SD_BLUE		((0xff) + (0x00<<8) + (0x00<<16) + (0xff<<24))
#define SD_ORANGE	((0x00) + (0x7f<<8) + (0xff<<16) + (0xff<<24))
#define SD_WHITE	((0xff) + (0xff<<8) + (0xff<<16) + (0xff<<24))
#define SD_GRAY		((0x3f) + (0x3f<<8) + (0x3f<<16) + (0xff<<24))
#define SD_BLACK	((0x00) + (0x00<<8) + (0x00<<16) + (0xff<<24))

#define SX_WHITE	((0xff) + (0xff<<8) + (0xff<<16) + (0xbf<<24))
#define SX_CYAN		((0xff) + (0xff<<8) + (0x00<<16) + (0xbf<<24))
#define SX_YELLOW	((0x00) + (0xff<<8) + (0xff<<16) + (0xbf<<24))
#define SX_RED		((0x00) + (0x00<<8) + (0xff<<16) + (0xbf<<24))
#define SX_GREEN	((0x00) + (0xff<<8) + (0x00<<16) + (0xbf<<24))
#define SX_BLUE		((0xff) + (0x00<<8) + (0x00<<16) + (0xbf<<24))
#define SX_ORANGE	((0x00) + (0x7f<<8) + (0xff<<16) + (0xbf<<24))
#define SX_GRAY		((0x3f) + (0x3f<<8) + (0x3f<<16) + (0xbf<<24))
#define SX_BLACK	((0x00) + (0x00<<8) + (0x00<<16) + (0xbf<<24))

#define SB_YELLOW	((0x00) + (0xff<<8) + (0xff<<16) + (0x6f<<24))
#define SB_RED		((0x00) + (0x00<<8) + (0xff<<16) + (0x6f<<24))
#define SB_GREEN	((0x00) + (0xff<<8) + (0x00<<16) + (0x6f<<24))
#define SB_BLUE		((0xff) + (0x00<<8) + (0x00<<16) + (0x6f<<24))
#define SB_ORANGE	((0x00) + (0x7f<<8) + (0xff<<16) + (0x6f<<24))
#define SB_WHITE	((0xff) + (0xff<<8) + (0xff<<16) + (0x6f<<24))
#define SB_GRAY		((0x3f) + (0x3f<<8) + (0x3f<<16) + (0x6f<<24))
#define SB_BLACK	((0x00) + (0x00<<8) + (0x00<<16) + (0x6f<<24))

#define XP_YELLOW	((0x00) + (0xff<<8) + (0xff<<16) + (0x3f<<24))
#define XP_RED		((0x00) + (0x00<<8) + (0xff<<16) + (0x3f<<24))
#define XP_GREEN	((0x00) + (0xff<<8) + (0x00<<16) + (0x3f<<24))
#define XP_BLUE		((0xff) + (0x00<<8) + (0x00<<16) + (0x3f<<24))
#define XP_ORANGE	((0x00) + (0x7f<<8) + (0xff<<16) + (0x3f<<24))
#define XP_WHITE	((0xff) + (0xff<<8) + (0xff<<16) + (0x3f<<24))
#define XP_GRAY		((0x3f) + (0x3f<<8) + (0x3f<<16) + (0x3f<<24))
#define XP_BLACK	((0x00) + (0x00<<8) + (0x00<<16) + (0x3f<<24))


//--------------------------------------------------------------------------------------
#endif