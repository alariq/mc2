//***************************************************************************
//
//	Debugging.cpp -- File contains the Game Debugging code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DEBUGGING_H
#include"debugging.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include<string.h>

extern UserHeapPtr systemHeap;

HGOSFONT3D GameDebugWindow::font = NULL;
long GameDebugWindow::fontHeight = 0;

//***************************************************************************
//	GAME DEBUG WINDOW class
//***************************************************************************

void* GameDebugWindow::operator new (size_t ourSize) {

	void* result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void GameDebugWindow::operator delete (void* us) {

	systemHeap->Free(us);
}	

//---------------------------------------------------------------------------

void GameDebugWindow::setFont (const char* fontFile) {

	if (font) {
		gos_DeleteFont(font);
		font = NULL;
	}
	if (fontFile) {
		font = gos_LoadFont(fontFile);
		gos_TextSetAttributes(font, 0xffffffff, 1.0, true, true, false, false);
	}
	DWORD height, width;
	gos_TextStringLength(&width, &height, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	fontHeight = height;
}

//---------------------------------------------------------------------------

void GameDebugWindow::print (const char* s) {
	
	if (numLines < MAX_DEBUG_WINDOW_LINES)
		strcpy(textBuffer[numLines++], s);
	else {
		numLines++;
		strcpy(textBuffer[linePos++], s);
		if (linePos == MAX_DEBUG_WINDOW_LINES)
			linePos = 0;
	}
}

//---------------------------------------------------------------------------

void GameDebugWindow::render (void) {

	if (!display)
		return;

	gos_TextSetAttributes(font, 0xffffffff, 1.0, true, true, false, false);
	gos_TextSetRegion( 0, 0, Environment.screenWidth, Environment.screenHeight );
	long curY = pos[1] + 5;
	for (long i = linePos; i < MAX_DEBUG_WINDOW_LINES; i++) {
		gos_TextSetPosition(pos[0] + 5, curY);
		curY += fontHeight;
		gos_TextDraw(textBuffer[i]);
	}
	for (int i = 0; i < linePos; i++) {
		gos_TextSetPosition(pos[0] + 5, curY);
		curY += fontHeight;
		gos_TextDraw(textBuffer[i]);
	}
}

//***************************************************************************
