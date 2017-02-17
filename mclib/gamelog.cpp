//---------------------------------------------------------------------------
//
// gamelog.cpp - This file contains the class functions for GameLog
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files
#ifndef FILE_H
#include<file.h>
#endif

#ifndef GAMELOG_H
#include"gamelog.h"
#endif

#ifndef HEAP_H
#include<heap.h>
#endif

#include<malloc.h>
#include<stdlib.h>
#include<stdio.h>
#include"platform_str.h"
#include<gameos.hpp>

bool isSetup = false;

//*****************************************************************************

GameLogPtr GameLog::files[MAX_GAMELOGS] = {NULL, NULL, NULL, NULL, NULL, NULL};

void* GameLog::operator new (size_t mySize) {

	void* result = NULL;
	
	result = gos_Malloc(mySize);
	
	return(result);
}

//---------------------------------------------------------------------------

void GameLog::operator delete (void* us) {

	gos_Free(us);
}

//---------------------------------------------------------------------------

void GameLog::dump (void) {

	//----------------
	// Dump to file...
	for (long i = 0; i < numLines; i++)
		filePtr->writeString(lines[i]);
	numLines = 0;
}

//---------------------------------------------------------------------------

void GameLog::close (void) {

	if (filePtr && inUse) {
		dump();
		char s[512];
		sprintf(s, "\nNum Total Lines = %d\n", totalLines);
		filePtr->writeString(s);
		filePtr->close();
		inUse = false;
		numLines = 0;
		totalLines = 0;
	}
}

//---------------------------------------------------------------------------

void GameLog::destroy (void) {

	close();
}

//---------------------------------------------------------------------------

long GameLog::open (const char* fileName) {

	numLines = 0;
	totalLines = 0;
	if (filePtr->create(fileName) != NO_ERR)
		return(-1);

	inUse = true;
	return(0);
}

//---------------------------------------------------------------------------

void GameLog::write (const char* s) {

	static char buffer[MAX_GAMELOG_LINELEN];
	
	if (numLines == MAX_GAMELOG_LINES)
		dump();
	//sebi
	//if (strlen(s) > (MAX_GAMELOG_LINELEN - 1))
	//	s[MAX_GAMELOG_LINELEN - 1] = '\0';

	//sprintf(buffer, "%s\n", s);
	S_snprintf(buffer, MAX_GAMELOG_LINES-1, "%s\n", s);

    // sebi
	if (strlen(s) > (MAX_GAMELOG_LINELEN - 1))
		buffer[MAX_GAMELOG_LINELEN - 1] = '\0';
    //
	strncpy(lines[numLines], buffer, MAX_GAMELOG_LINELEN - 1);
	lines[numLines][MAX_GAMELOG_LINELEN - 1] = '\0';

	numLines++;
	totalLines++;
}

//---------------------------------------------------------------------------

GameLog* GameLog::getNewFile (void) {

	if (!isSetup)
		setup();

	long fileHandle = -1;
    long i = 0;
	for (; i < MAX_GAMELOGS; i++)
		if (!files[i]->inUse) {
			fileHandle = i;
			break;
		}
	return(files[i]);
}

//---------------------------------------------------------------------------

void GameLog::setup (void) {

	if (isSetup)
		return;

	isSetup = true;
	for (long i = 0; i < MAX_GAMELOGS; i++) {
		files[i] = new GameLog;
		files[i]->init();
		files[i]->handle = i;
		files[i]->inUse = false;
		files[i]->filePtr = new File;
		gosASSERT(files[i]->filePtr != NULL);
	}
}

//---------------------------------------------------------------------------

void GameLog::cleanup (void) {

	if (!isSetup)
		return;

	for (long i = 0; i < MAX_GAMELOGS; i++) 
	{
		if (files[i] && files[i]->inUse)
		{
			files[i]->close();
			files[i]->filePtr->close();
			delete files[i]->filePtr;
			files[i]->filePtr = NULL;
			delete files[i];
			files[i] = NULL;
		}
	}
}

