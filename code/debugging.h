//***************************************************************************
//
//	Debugging.h -- File contains the MechCommander Debugging Definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DEBUGGING_H
#define DEBUGGING_H

#include<stdio.h>
#include<gameos/gameos.hpp>
#include<gameos/toolos.hpp>

//---------------------------------------------------------------------------

#define	MAX_DEBUG_WINDOW_LINES		6
#define	MAX_DEBUG_WINDOW_LINELEN	256

class GameDebugWindow {

	public:

		bool				display;
		long				pos[2];
		char				textBuffer[MAX_DEBUG_WINDOW_LINES][MAX_DEBUG_WINDOW_LINELEN];
		long				linePos;
		long				numLines;

		static HGOSFONT3D	font;
		static long			fontHeight;

	public:

		void init (void) {
			display = false;
			pos[0] = 0;
			pos[1] = 0;
			linePos = 0;
			numLines = 0;
			for (long i = 0; i < MAX_DEBUG_WINDOW_LINES; i++)
				textBuffer[i][0] = NULL;
		}
		
		GameDebugWindow (void) {
			init();
		}

		void setPos (long x, long y) {
			pos[0] = x;
			pos[1] = y;
		}
		
		void open (long x = -1, long y = -1) {
			if ((x > -1) && (y > -1))
				setPos(x, y);
			display = true;
		}

		void close (void) {
			display = false;
		}

		void toggle (void) {
			if (display)
				close();
			else
				open();
		}

		~GameDebugWindow (void) {
			destroy();
		}
		
		virtual void destroy (void) {
		}
		
		void print (char* s);

		void render (void);

		void clear (void) {
			numLines = 0;
			for (long i = 0; i < MAX_DEBUG_WINDOW_LINES; i++)
				textBuffer[i][0] = NULL;
		}

		static void setFont (char* fontFile);
};

//***************************************************************************

#endif




