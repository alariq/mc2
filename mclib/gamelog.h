//---------------------------------------------------------------------------
//
// gamelog.h - This file contains the class declaration for GameLog
//
//				The Log File class allows easy dumping to a log file
//				from your application. It handles the string buffer (to
//				prevent writing to file everytime you call it).
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GAMELOG_H
#define GAMELOG_H
//---------------------------------------------------------------------------
// Include files

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef DFILE_H
#include"dfile.h"
#endif

#ifndef DGAMELOG_H
#include"dgamelog.h"
#endif

//---------------------------------------------------------------------------

#define	MAX_GAMELOGS			6
#define MAX_GAMELOG_LINES		50
#define	MAX_GAMELOG_LINELEN		200
#define	MAXLEN_GAMELOG_NAME		128

class GameLog {

	public:

		long					handle;
		bool					inUse;
		char					fileName[MAXLEN_GAMELOG_NAME];
		File*					filePtr;
		long					numLines;
		long					totalLines;
		char					lines[MAX_GAMELOG_LINES][MAX_GAMELOG_LINELEN];

		static GameLogPtr		files[MAX_GAMELOGS];

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
			
		void init (void) {
			handle = -1;
			inUse = false;
			fileName[0] = '\0';
			filePtr = NULL;
			numLines = 0;
			totalLines = 0;
			for (long i = 0; i < MAX_GAMELOG_LINES; i++)
				lines[i][0] = '\0';
		}

		GameLog (void) {
			init();
		}

		void destroy (void);

		~GameLog (void) {
			destroy();
		}

		void dump (void);

		void close (void);

		long open (const char* fileName);

		void write (const char* s);

		static void setup (void);

		static void cleanup (void);

		static GameLogPtr getNewFile (void);
};


//---------------------------------------------------------------------------
#endif

