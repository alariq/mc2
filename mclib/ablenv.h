//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ENVIRON.H
//
//***************************************************************************

#ifndef ABLENV_H
#define ABLENV_H

#include<stdio.h>

#ifndef DABLENV_H
#include"dablenv.h"
#endif

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLSYMT_H
#include"ablsymt.h"
#endif

#ifndef ABLEXEC_H
#include"ablexec.h"
#endif

#ifndef DABLDBUG_H
#include"dabldbug.h"
#endif

#ifndef ABLSCAN_H
#include"ablscan.h"
#endif

//***************************************************************************

typedef struct _SourceFile {
	char					fileName[MAXLEN_FILENAME];
	unsigned char			fileNumber;
	ABLFile*				filePtr;
	long					lineNumber;
} SourceFile;

//---------------------------------------------------------------------------

#define	MAX_USER_FILES			6
#define MAX_USER_FILE_LINES		50
#define	MAX_USER_FILE_LINELEN	200

class UserFile {

	public:

		long					handle;
		bool					inUse;
		char					fileName[MAXLEN_FILENAME];
		ABLFile*				filePtr;
		long					numLines;
		int                     totalLines;
		char					lines[MAX_USER_FILE_LINES][MAX_USER_FILE_LINELEN];

		static UserFilePtr		files[MAX_USER_FILES];

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
			for (long i = 0; i < MAX_USER_FILE_LINES; i++)
				lines[i][0] = '\0';
		}

		UserFile (void) {
			init();
		}

		void destroy (void);

		~UserFile (void) {
			destroy();
		}

		void dump (void);

		void close (void);

		long open (const char* fileName);

		void write (const char* s);

		static void setup (void);

		static void cleanup (void);

		static UserFilePtr getNewFile (void);
};

//---------------------------------------------------------------------------

#define	MAX_ABLMODULE_NAME				5
#define	MAX_SOURCE_FILES				256			// per module
#define	MAX_LIBRARIES_USED				25			// per module
#define	MAX_STATE_HANDLES_PER_MODULE	10

typedef struct {
	char					name[128];
	int                     size;
} VariableInfo;

typedef struct {
	char					name[128];
	long					codeSegmentSize;
} RoutineInfo;

typedef struct {
	char					name[128];
	char					fileName[128];
	int                     numStaticVars;
	int                     totalSizeStaticVars;
	VariableInfo			largestStaticVar;
	long					totalCodeSegmentSize;
	long					numRoutines;
	RoutineInfo				routineInfo[128];
} ModuleInfo;

typedef struct {
	char*					fileName;
	SymTableNodePtr			moduleIdPtr;
	long					numSourceFiles;
	char**					sourceFiles;
	long					numLibrariesUsed;
	ABLModulePtr*			librariesUsed;
	int                     numStaticVars;
	long					numOrderCalls;
	long					numStateHandles;
	StateHandleInfoPtr		stateHandles;
	long*					sizeStaticVars;
	long					totalSizeStaticVars;
	long					numInstances;
} ModuleEntry;

typedef ModuleEntry* ModuleEntryPtr;

class ABLModule {

	private:

		int                     id;
		char					name[MAX_ABLMODULE_NAME];
		int32_t                 handle;
		StackItemPtr			staticData;
		unsigned long*			orderCallFlags;
		StackItem				returnVal;
		bool					initCalled;
		SymTableNodePtr			prevState;
		SymTableNodePtr			state;
		WatchManagerPtr			watchManager;
		BreakPointManagerPtr	breakPointManager;
		bool					trace;
		bool					step;
		bool					traceEntry;
		bool					traceExit;

		//static long				numModules;

	public:

		void* operator new (size_t mySize);
		void operator delete (void* us);
			
		void init (void) {
			id = -1;
			name[0] = '\0';
			handle = -1;
			staticData = NULL;
			returnVal.integer = 0;
			initCalled = false;
			prevState = NULL;
			state = NULL;
			watchManager = NULL;
			breakPointManager = NULL;
			trace = false;
			step = false;
			traceEntry = false;
			traceExit = false;
		}


		ABLModule (void) {
			init();
		}

		long init (int moduleHandle);
		
		void write (ABLFile* moduleFile);
		
		void read (ABLFile* moduleFile);

		int getId (void) {
			return(id);
		}

		long getRealId (void);

		int getHandle (void) {
			return(handle);
		}

		StackItemPtr getStaticData (void) {
			return(staticData);
		}

		void setInitCalled (bool called) {
			initCalled = called;
		}

		bool getInitCalled (void) {
			return(initCalled);
		}

		char* getFileName (void);

		char* getName (void) {
			return(name);
		}

		void setName (const char* _name);

		unsigned long* getOrderCallFlags (void) {
			return(orderCallFlags);
		}

		void setPrevState (SymTableNodePtr stateSym) {
			prevState = stateSym;
		}

		SymTableNodePtr getPrevState (void) {
			return(prevState);
		}

		int getPrevStateHandle (void);

		void setState (SymTableNodePtr stateSym) {
			state = stateSym;
		}

		SymTableNodePtr getState (void) {
			return(state);
		}

		int getStateHandle (void);

		bool isLibrary (void);

		void resetOrderCallFlags (void);

		void setOrderCallFlag (unsigned char dword, unsigned char bit);

		void clearOrderCallFlag (unsigned char orderDWord, unsigned char orderBitMask);

		bool getOrderCallFlag (unsigned char dword, unsigned char bit) {
			return((orderCallFlags[dword] & (1 << bit)) != 0);
		}

		WatchManagerPtr getWatchManager (void) {
			return(watchManager);
		}

		BreakPointManagerPtr getBreakPointManager (void) {
			return(breakPointManager);
		}

		void setTrace (bool _trace) {
			trace = _trace;
			traceEntry = _trace;
			traceExit = _trace;
		}

		bool getTrace (void) {
			return(trace);
		}

		void setStep (bool _step) {
			step = _step;
		}

		bool getStep (void) {
			return(step);
		}

		long execute (ABLParamPtr paramList = NULL);
		long execute (ABLParamPtr moduleParamList, SymTableNodePtr functionIdPtr);

		SymTableNodePtr findSymbol (const char* symbolName, SymTableNodePtr curFunction = NULL, bool searchLibraries = false);

		SymTableNodePtr findFunction (const char* functionName, bool searchLibraries = false);

		SymTableNodePtr findState (const char* stateName);

		int findStateHandle (const char* stateName);

		char* getSourceFile (int fileNumber);

		char* getSourceDirectory (int fileNumber, char* directory);

		void getInfo (ModuleInfo* moduleInfo);

		float getReal (void) {
			return(returnVal.real);
		}
		
		int getInteger (void) {
			return(returnVal.integer);
		}

		int setStaticInteger (char* name, int value);

		int getStaticInteger (char* name);

		int setStaticReal (char* name, float value);
		
		float getStaticReal (char* name);

		int setStaticIntegerArray (char* name, int size, int* values);

		int getStaticIntegerArray (char* name, int size, int* values);
		
		int setStaticRealArray (char* name, int size, float* values);

		int getStaticRealArray (char* name, int size, float* values);

		void destroy (void);

		~ABLModule (void) {

			destroy();
		}

};

//*************************************************************************

void initModuleRegistry (long maxModules);
void destroyModuleRegistry (void);
void initLibraryRegistry (long maxLibraries);
void destroyLibraryRegistry (void);

//***************************************************************************

#endif
