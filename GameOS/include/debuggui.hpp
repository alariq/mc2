#pragma once
//===========================================================================//
// File:	 DebugGUI.hpp													 //
// Contents: Debugger user interface										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

void UpdateDebugMouse();
void UpdateDebugWindow();
void InitTextDisplay();
void DrawText( DWORD Color, char* String );
void DrawSquare( int TopX, int TopY, int Width, int Height, DWORD Color );
void DrawChr( char Chr );


//
// Debugger window variables
//
extern int DbTopX,DbTopY;
extern int DbMaxX,DbMaxY,DbMinX;
extern DWORD DebugDisplay;
extern DWORD TopStatistics;

//
// Debugger window constants
//
#define DbSizeX 489
#define DbSizeY 342


extern int CurrentX,CurrentY;		// Current pixel position
extern int DbChrX,DbChrY;			// Current character x,y
extern int StartX,StartY;			// Current start of line




