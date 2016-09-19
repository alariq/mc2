//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLDBUG.H
//
//***************************************************************************

 
#ifndef ABLDBUG_H
#define ABLDBUG_H

#ifndef DABLDBUG_H
#include"dabldbug.h"
#endif

#ifndef ABLENV_H
#include"ablenv.h"
#endif

//***************************************************************************

typedef enum {
	DEBUG_COMMAND_SET_MODULE,
	DEBUG_COMMAND_TRACE,
	DEBUG_COMMAND_STEP,
	DEBUG_COMMAND_BREAKPOINT_SET,
	DEBUG_COMMAND_BREAKPOINT_REMOVE,
	DEBUG_COMMAND_WATCH_SET,
	DEBUG_COMMAND_WATCH_REMOVE_ALL,
	DEBUG_COMMAND_PRINT,
	DEBUG_COMMAND_CONTINUE,
	DEBUG_COMMAND_HELP,
	DEBUG_COMMAND_INFO,
	NUM_DEBUG_COMMANDS
} DebugCommandCode;

//***************************************************************************

typedef struct _Watch {
	SymTableNodePtr		idPtr;
	bool				store;
	bool				breakOnStore;
	bool				fetch;
	bool				breakOnFetch;
} Watch;

typedef Watch* WatchPtr;

class WatchManager {

	protected:

		long			maxWatches;
		long			numWatches;
		WatchPtr		watches;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);

		void init (void) {
			maxWatches = 0;
			maxWatches = 0;
			watches = NULL;
		}

		long init (long max);

		void destroy (void);

		WatchManager (void) {
			init();
		}

		~WatchManager (void) {
			destroy();
		}

		WatchPtr add (SymTableNodePtr idPtr);

		long remove (SymTableNodePtr idPtr);

		long removeAll (void);
		
		long setStore (SymTableNodePtr idPtr, bool state, bool breakToDebug = false);
		
		long setFetch (SymTableNodePtr idPtr, bool state, bool breakToDebug = false);

		bool getStore (SymTableNodePtr idPtr);

		bool getFetch (SymTableNodePtr idPtr);

		void print (void);
};

//---------------------------------------------------------------------------

class BreakPointManager {

	protected:

		long			maxBreakPoints;
		long			numBreakPoints;
		long*			breakPoints;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);

		void init (void) {
			maxBreakPoints = 0;
			numBreakPoints = 0;
			breakPoints = NULL;
		}

		long init (long max);

		void destroy (void);

		BreakPointManager (void) {
			init();
		}

		~BreakPointManager (void) {
			destroy();
		}

		long add (long lineNumber);

		long remove (long lineNumber);

		long removeAll (void);

		bool isBreakPoint (long lineNumber);

		void print (void);
};

//---------------------------------------------------------------------------

#define	WATCH_STORE_OFF		1
#define	WATCH_STORE_ON		2
#define	WATCH_FETCH_OFF		4
#define	WATCH_FETCH_ON		8
#define	WATCH_BREAK			16

class Debugger {

	protected:

		ABLModulePtr			module;					// Current executing module
		WatchManagerPtr			watchManager;			// Current executing watch manager
		BreakPointManagerPtr	breakPointManager;		// Current executing breakpt manager

		ABLModulePtr			debugModule;			// Current module being debugged

		bool					enabled;
		bool					debugCommand;
		bool					halt;
		bool					trace;
		bool					step;
		bool					traceEntry;
		bool					traceExit;

		static char				message[512];

		void (*printCallback)(const char* s);

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);

		void init (void) {
			module = NULL;
			watchManager = NULL;
			breakPointManager = NULL;
			debugModule = module;
			enabled = false;
			debugCommand = false;
			halt = false;
			trace = false;
			step = false;
			traceEntry = false;
			traceExit = false;
			printCallback = NULL;
		}

		long init (void (*callback)(const char* s), ABLModulePtr _module);

		void destroy (void);

		Debugger (void) {
			init();
		}

		~Debugger (void) {
			destroy();
		}

		void enable (void) {
			enabled = true;
		}

		void disable (void) {
			enabled = false;
		}

		bool isEnabled (void) {
			return(enabled);
		}

		long print (const char* s);

		void setModule (ABLModulePtr _module);

		long setWatch (long states);

		long addBreakPoint (void);

		long removeBreakPoint (void);

		void sprintStatement (char* dest);

		void sprintLineNumber (char* dest);

		void sprintDataValue (char* dest, StackItemPtr data, TypePtr dataType);

		long sprintSimpleValue (char* dest, SymTableNodePtr symbol);

		long sprintArrayValue (char* dest, SymTableNodePtr symbol, char* subscriptString);

		long sprintValue (char* dest, char* exprString);

		long traceStatementExecution (void);

		long traceRoutineEntry (SymTableNodePtr idPtr);

		long traceRoutineExit (SymTableNodePtr idPtr);

		long traceDataStore (SymTableNodePtr id, TypePtr idType, StackItemPtr target, TypePtr targetType);

		long traceDataFetch (SymTableNodePtr id, TypePtr idType, StackItemPtr data);

		void showValue (void);

		void assignVariable (void);

		void displayModuleInstanceRegistry (void);

		void processCommand (long commandId, char* strParam1, long numParam1, ABLModulePtr moduleParam1);

		void debugMode (void);

		ABLModulePtr getDebugModule (void) {
			return(debugModule);
		}
};

//***************************************************************************

#endif

