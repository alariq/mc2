//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLENV.CPP
//
//***************************************************************************

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string_win.h>

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLERR_H
#include"ablerr.h"
#endif

#ifndef ABLSCAN_H
#include"ablscan.h"
#endif

#ifndef ABLEXEC_H
#include"ablexec.h"
#endif

#ifndef ABLENV_H
#include"ablenv.h"
#endif

#ifndef ABLDBUG_H
#include"abldbug.h"
#endif

//***************************************************************************
long ABLi_preProcess (const char* sourceFileName,
					  long* numErrors = NULL,
					  long* numLinesProcessed = NULL,
					  long* numFilesProcessed = NULL,
					  bool printLines = false);

ABLModulePtr ABLi_loadLibrary (const char* sourceFileName,
					   long* numErrors = NULL,
					   long* numLinesProcessed = NULL,
					   long* numFilesProcessed = NULL,
					   bool printLines = false,
					   bool createInstance = true);

//-------------------
// EXTERNAL variables
extern long				lineNumber;
extern long				errorCount;
extern int              execStatementCount;

extern TokenCodeType	curToken;
extern char				wordString[];
extern SymTableNodePtr	symTableDisplay[];
extern long				level;
extern bool				blockFlag;
extern BlockType		blockType;
extern bool				printFlag;
extern SymTableNodePtr	CurModuleIdPtr;
extern SymTableNodePtr	CurRoutineIdPtr;
extern long				CurModuleHandle;
extern bool				CallModuleInit;
extern long				FileNumber;

extern Type				DummyType;
extern char*			codeBuffer;
extern char*			codeBufferPtr;
extern StackItem*		stack;
//extern StackItem*		eternalStack;
extern StackItemPtr		tos;
extern StackItemPtr		stackFrameBasePtr;
extern long				eternalOffset;

extern TokenCodeType	statementStartList[];
extern TokenCodeType	statementEndList[];
extern TokenCodeType	declarationStartList[];

extern char				tokenString[MAXLEN_TOKENSTRING];

extern CharCodeType		charTable[256];
extern ABLFile*			sourceFile;

extern char				sourceBuffer[MAXLEN_SOURCELINE];
extern long				bufferOffset;
extern char*			bufferp;
extern char*			tokenp;

extern long				digitCount;
extern bool				countError;

extern bool				eofFlag;
extern long				pageNumber;

extern SymTableNodePtr	SymTableDisplay[MAX_NESTING_LEVEL];
extern TypePtr			IntegerTypePtr;
extern TypePtr			RealTypePtr;
extern TypePtr			BooleanTypePtr;

extern unsigned long*	OrderCompletionFlags;
extern StackItemPtr		StaticDataPtr;
extern StackItem		returnValue;

extern DebuggerPtr		debugger;
extern long*			EternalVariablesSizes;

//-----------------------
// CLASS static variables
long				NumModules = 0;

//-----------------
// GLOBAL variables
ModuleEntryPtr		ModuleRegistry = NULL;
ABLModulePtr*		ModuleInstanceRegistry = NULL;
long				MaxModules = 0;
long				NumModulesRegistered = 0;
long				NumModuleInstances = 0;
long				MaxWatchesPerModule = 20;
long				MaxBreakPointsPerModule = 20;
ABLModulePtr		CurModule = NULL;
ABLModulePtr		CurFSM = NULL;
ABLModulePtr		CurLibrary = NULL;
ABLModulePtr*		LibraryInstanceRegistry = NULL;
long				NumStateTransitions = 0;
long				MaxLibraries = 0;
bool				NewStateSet = false;
extern long			numLibrariesLoaded;

extern long			NumExecutions;
long				CallStackLevel = 0;

#define	MAX_PROFILE_LINELEN		128
#define MAX_PROFILE_LINES		256

long NumProfileLogLines = 0;
int TotalProfileLogLines = 0;
char ProfileLogBuffer[MAX_PROFILE_LINES][MAX_PROFILE_LINELEN];
ABLFile* ProfileLog = NULL;
long ProfileLogFunctionTimeLimit = 5;

UserFilePtr			UserFile::files[MAX_USER_FILES];

//***************************************************************************
// PROFILING LOG routines
//***************************************************************************

void DumpProfileLog (void) {

	//----------------
	// Dump to file...
	for (long i = 0; i < NumProfileLogLines; i++)
		ProfileLog->writeString(ProfileLogBuffer[i]);
	NumProfileLogLines = 0;
}

//---------------------------------------------------------------------------

void ABL_CloseProfileLog (void) {

	if (ProfileLog) 
	{
		DumpProfileLog();
		char s[512];
		sprintf(s, "\nNum Total Lines = %d\n", TotalProfileLogLines);
		ProfileLog->writeString(s);
		ProfileLog->close();
		delete ProfileLog;
		ProfileLog = NULL;
		NumProfileLogLines = 0;
		TotalProfileLogLines = 0;
	}
}

//---------------------------------------------------------------------------

void ABL_OpenProfileLog (void) {

	if (ProfileLog)
		ABL_CloseProfileLog();

	NumProfileLogLines = 0;
	ProfileLog = new ABLFile;
	if (!ProfileLog)
		ABL_Fatal(0, " unable to malloc ABL ProfileLog ");
	if (ProfileLog->create("abl.log") != ABL_NO_ERR)
		ABL_Fatal(0, " unable to create ABL ProfileLog ");
}

//---------------------------------------------------------------------------

void ABL_AddToProfileLog (char* profileString) {

	if (NumProfileLogLines == MAX_PROFILE_LINES)
		DumpProfileLog();

	strncpy(ProfileLogBuffer[NumProfileLogLines], profileString, MAX_PROFILE_LINELEN - 1);
	ProfileLogBuffer[NumProfileLogLines][MAX_PROFILE_LINELEN - 1] = '\0';
	NumProfileLogLines++;
	TotalProfileLogLines++;
}

//***************************************************************************
// USER FILE routines
//***************************************************************************

void* UserFile::operator new (size_t mySize) {

	void* result = NULL;
	
	result = ABLSystemMallocCallback(mySize);
	
	return(result);
}

//---------------------------------------------------------------------------

void UserFile::operator delete (void* us) {

	ABLSystemFreeCallback(us);
}

//---------------------------------------------------------------------------

void UserFile::dump (void) {

	//----------------
	// Dump to file...
	for (long i = 0; i < numLines; i++)
		filePtr->writeString(lines[i]);
	numLines = 0;
}

//---------------------------------------------------------------------------

void UserFile::close (void) {

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

long UserFile::open (const char* fileName) {

	numLines = 0;
	totalLines = 0;
	if (filePtr->create(fileName) != ABL_NO_ERR)
		return(-1);

	inUse = true;
	return(0);
}

//---------------------------------------------------------------------------

void UserFile::write (const char* s) {

	static char buffer[MAX_USER_FILE_LINELEN];
	
	if (numLines == MAX_USER_FILE_LINES)
		dump();
	
    //sebi !NB
	//if (strlen(s) > (MAX_USER_FILE_LINELEN - 1))
	//	s[MAX_USER_FILE_LINELEN - 1] = '\0';

	//sprintf(buffer, "%s\n", s);
	snprintf(buffer, MAX_USER_FILE_LINELEN - 1, "%s\n", s);
	
    //sebi 
    if (strlen(s) > (MAX_USER_FILE_LINELEN - 1))
        buffer[MAX_USER_FILE_LINELEN - 1] = '\0';
    //

	strncpy(lines[numLines], buffer, MAX_USER_FILE_LINELEN - 1);
	numLines++;
	totalLines++;
}

//---------------------------------------------------------------------------

UserFile* UserFile::getNewFile (void) {

	long fileHandle = -1;
    long i = 0;
	for (; i < MAX_USER_FILES; i++)
		if (!files[i]->inUse) {
			fileHandle = i;
			break;
		}
	return(files[i]);
}

//---------------------------------------------------------------------------

void UserFile::setup (void) {

	for (long i = 0; i < MAX_USER_FILES; i++) {
		files[i] = (UserFile*)ABLSystemMallocCallback(sizeof(UserFile));
		files[i]->init();
		files[i]->handle = i;
		files[i]->inUse = false;
		files[i]->filePtr = new ABLFile;
		if (!files[i]->filePtr)
			ABL_Fatal(0, " ABL: Unable to malloc UserFiles ");
	}
}

//---------------------------------------------------------------------------

void UserFile::cleanup (void) {

	for (long i = 0; i < MAX_USER_FILES; i++) 
	{
		if (files[i]) {
			if (files[i]->inUse)
				files[i]->close();
			//Should actually free the memory allocated above here!
			delete files[i]->filePtr;
			files[i]->filePtr = NULL;

			ABLSystemFreeCallback(files[i]);
			files[i] = NULL;
		}
	}
}

//***************************************************************************
// MODULE REGISTRY routines
//***************************************************************************

void initModuleRegistry (long maxModules) {

	//---------------------------------------------------------------------
	// First, set the max number of modules that may be loaded into the ABL
	// environment at a time...
	MaxModules = maxModules;

	//------------------------------
	// Create the module registry...
	ModuleRegistry = (ModuleEntryPtr)ABLStackMallocCallback(sizeof(ModuleEntry) * MaxModules);
	if (!ModuleRegistry)
		ABL_Fatal(0, " ABL: Unable to AblStackHeap->malloc Module Registry ");
	memset(ModuleRegistry, 0, sizeof(ModuleEntry) * MaxModules);

	//-------------------------------------------------
	// Create the active (ABLModule) module registry...
	ModuleInstanceRegistry = (ABLModulePtr*)ABLStackMallocCallback(sizeof(ABLModulePtr) * MaxModules);
	if (!ModuleInstanceRegistry)
		ABL_Fatal(0, " ABL: Unable to malloc AblStackHeap->Module Instance Registry ");
	memset(ModuleInstanceRegistry, 0, sizeof(ABLModulePtr) * MaxModules);
}

//***************************************************************************

void destroyModuleRegistry (void) {

	//-----------------------------------------------------------
	// First, go through the registry, free'n each module and its
	// associated data...
	for (long i = 0; i < NumModulesRegistered; i++) {
		ABLStackFreeCallback(ModuleRegistry[i].fileName);
		ModuleRegistry[i].fileName = NULL;
		ModuleRegistry[i].moduleIdPtr = NULL;
		for (long j = 0; j < ModuleRegistry[i].numSourceFiles; j++) {
			ABLStackFreeCallback(ModuleRegistry[i].sourceFiles[j]);
			ModuleRegistry[i].sourceFiles[j] = NULL;
		}
	}

	ABLStackFreeCallback(ModuleRegistry);	
	ModuleRegistry = NULL;

	ABLStackFreeCallback(ModuleInstanceRegistry);	
	ModuleInstanceRegistry = NULL;
}

//***************************************************************************
// LIBRARY REGISTRY routines
//***************************************************************************

void initLibraryRegistry (long maxLibraries) {

	//-----------------------------------------------------------------------
	// First, set the max number of libraries that may be loaded into the ABL
	// environment at a time...
	MaxLibraries = maxLibraries;

	//--------------------------------------------------
	// Create the active (ABLModule) library registry...
	LibraryInstanceRegistry = (ABLModulePtr*)ABLStackMallocCallback(sizeof(ABLModulePtr) * MaxLibraries);
	if (!LibraryInstanceRegistry)
		ABL_Fatal(0, " ABL: Unable to malloc AblStackHeap->Library Instance Registry ");
	memset(LibraryInstanceRegistry, 0, sizeof(ABLModulePtr) * MaxLibraries);
}

//***************************************************************************

void destroyLibraryRegistry (void) {

	//-----------------------------------------------------------------
	// Kinda need to do the same thing here as in the normal Registry.
	// Or leak o RAMA!!!!!!!
	// The actual data held by the pointer is removed in destroyModuleRegistry.
	// However, the classes holding the actual ABLModulePtr are then responsible
	// for deleting the ABLModulePtr.  Libraries have no owner class which, I'm guessing,
	// this class was supposed to do.  This class now does that!!!!
	// -fs		1/25/98
	for (long i=0;i<numLibrariesLoaded;i++)
	{
		delete LibraryInstanceRegistry[i];
		LibraryInstanceRegistry[i] = NULL;
	}

	ABLStackFreeCallback(LibraryInstanceRegistry);
	LibraryInstanceRegistry = NULL;
}

//***************************************************************************
// ABLMODULE class
//***************************************************************************

void* ABLModule::operator new (size_t mySize) {

	void* result = NULL;
	
	result = ABLSystemMallocCallback(mySize);
	
	return(result);
}

//---------------------------------------------------------------------------

void ABLModule::operator delete (void* us) {

	ABLSystemFreeCallback(us);
}

//---------------------------------------------------------------------------
long ABLModule::getRealId (void)
{
	//Scan through the ModuleInstanceRegistry and find the pointer that matches
	// this.  DO NOT COUNT ANY NULLs!!!!  These will go away when we reload a
	// QuickSave!!
	long actualCount = 0;
	bool foundBrain = false;

	for (long i=0;i<NumModuleInstances;i++)
	{
		if (this == ModuleInstanceRegistry[i])
		{
			foundBrain = true;
			break;
		}

		if (ModuleInstanceRegistry[i])
			actualCount++;
	}

	if (!foundBrain)
			ABL_Fatal(0,"Could not find this Brain in the ModuleInstanceRegistry");

	return actualCount;
}

//---------------------------------------------------------------------------

long ABLModule::init (long moduleHandle) {

	if (moduleHandle == -1) {
		//----------
		// Clean up!
		return(-1);
	}

	id = NumModules++;
	handle = moduleHandle;
	staticData = NULL;
	long numStatics = ModuleRegistry[handle].numStaticVars;
	if (numStatics) {
		staticData = (StackItemPtr)ABLStackMallocCallback(sizeof(StackItem) * numStatics);
		if (!staticData) {
			char err[255];
			sprintf(err, "ABL: Unable to AblStackHeap->malloc staticData [Module %d]", id);
			ABL_Fatal(0, err);
		}
		long* sizeList = ModuleRegistry[handle].sizeStaticVars;
		for (long i = 0; i < numStatics; i++)
			if (sizeList[i] > 0) {
				staticData[i].address = (char*)ABLStackMallocCallback(sizeList[i]);
				if (!staticData) {
					char err[255];
					sprintf(err, "ABL: Unable to AblStackHeap->malloc staticData address [Module %d]", id);
					ABL_Fatal(0, err);
				}
				}
			else
				staticData[i].integer = 0;
	}

	if (ModuleRegistry[handle].numOrderCalls) {
		long numLongs = 1 + ModuleRegistry[handle].numOrderCalls / 32;
		orderCallFlags = (unsigned long*)ABLStackMallocCallback(sizeof(unsigned long) * numLongs);
		if (!orderCallFlags) {
			char err[255];
			sprintf(err, "ABL: Unable to AblStackHeap->malloc orderCallFlags [Module %d]", id);
			ABL_Fatal(0, err);
		}
		for (long i = 0; i < numLongs; i++)
			orderCallFlags[i] = 0;
	}
	ModuleRegistry[handle].numInstances++;
	initCalled = false;

	//------------------------------------------------------
	// This Active Module is now on the instance registry...
	ModuleInstanceRegistry[NumModuleInstances++] = this;

	if (debugger) {
		watchManager = new WatchManager;
		if (!watchManager)
			ABL_Fatal(0, " Unable to AblStackHeap->malloc WatchManager ");
		long result = watchManager->init(MaxWatchesPerModule);
		if (result != ABL_NO_ERR)
			ABL_Fatal(0, " Unable to AblStackHeap->malloc WatchManager ");
		breakPointManager = new BreakPointManager;
		if (!breakPointManager)
			ABL_Fatal(0, " Unable to AblStackHeap->malloc BreakPointManager ");
		result = breakPointManager->init(MaxBreakPointsPerModule);
		if (result != ABL_NO_ERR)
			ABL_Fatal(0, " Unable to AblStackHeap->malloc BreakPointManager ");
	}

	if (ModuleRegistry[handle].moduleIdPtr->defn.info.routine.flags & ROUTINE_FLAG_FSM) {
		//--------------------------------
		// Always starts in START state...
		SymTableNodePtr startState = searchSymTable("start", ModuleRegistry[handle].moduleIdPtr->defn.info.routine.localSymTable);
		if (!startState) {
			char err[255];
			sprintf(err, "ABL: FSM has no Start state [%s]", CurModule->getName());
			ABL_Fatal(0, err);
		}
		prevState = NULL;
		state = startState;
	}

	//--------------------
	// Can this ever fail?
	return(ABL_NO_ERR);
}

//---------------------------------------------------------------------------

void ABLModule::write (ABLFile* moduleFile) {

	moduleFile->writeString(name);
	moduleFile->writeByte('\0');
	moduleFile->writeLong(handle);
	if (prevState == NULL)
		moduleFile->writeString("NULLPrevState");
	else
		moduleFile->writeString(prevState->name);
	moduleFile->writeByte('\0');
	if (state == NULL)
		moduleFile->writeString("NULLState");
	else
		moduleFile->writeString(state->name);
	moduleFile->writeByte('\0');
	moduleFile->writeLong(initCalled ? 1 : 0);
	long numStatics = ModuleRegistry[handle].numStaticVars;
	long* sizeList = ModuleRegistry[handle].sizeStaticVars;
	for (long i = 0; i < numStatics; i++) {
		if (sizeList[i] > 0)
			moduleFile->write((unsigned char*)staticData[i].address, sizeList[i]);
		else
			moduleFile->write((unsigned char*)&staticData[i], sizeof(StackItem));
	}
}

//---------------------------------------------------------------------------

void ABLModule::read (ABLFile* moduleFile) {

	//----------------------------------------------------------------------------
	// If this is called on a newly init'd module, then it will do all appropriate
	// memory alloc, etc. If it's being called on a module that's already been
	// setup (via a call to init(moduleHandle)), then it simply loads the
	// module's data...
	bool fresh = (id == -1);
	if (fresh) {
		id = NumModules++;
		moduleFile->readString((unsigned char*)name);
		handle = moduleFile->readLong();
		staticData = NULL;
		}
	else {
		char tempName[1024];
		moduleFile->readString((unsigned char*)tempName);
		long ignore = moduleFile->readLong();
	}

	char stateName[256];
	memset(stateName,0,256);
	moduleFile->readString((unsigned char*)stateName);
	prevState = NULL;
	if (strcmp(stateName, "NULLPrevState"))
		prevState = findState(stateName);
	
	memset(stateName,0,256);
	moduleFile->readString((unsigned char*)stateName);
	state = NULL;
	if (strcmp(stateName, "NULLState"))
		state = findState(stateName);

	bool savedInitCalled = (moduleFile->readLong() == 1);

	long numStatics = ModuleRegistry[handle].numStaticVars;
	if (numStatics) {
		if (fresh) {		
			staticData = (StackItemPtr)ABLStackMallocCallback(sizeof(StackItem) * numStatics);
			if (!staticData) {
				char err[255];
				sprintf(err, "ABL: Unable to AblStackHeap->malloc staticData [Module %d]", id);
				ABL_Fatal(0, err);
			}
		}
		long* sizeList = ModuleRegistry[handle].sizeStaticVars;
		for (long i = 0; i < numStatics; i++)
			if (sizeList[i] > 0) {
				if (fresh) {
					staticData[i].address = (char*)ABLStackMallocCallback(sizeList[i]);
					if (!staticData) {
						char err[255];
						sprintf(err, "ABL: Unable to AblStackHeap->malloc staticData address [Module %d]", id);
						ABL_Fatal(0, err);
					}
				}
				long result = moduleFile->read((unsigned char*)staticData[i].address, sizeList[i]);
				if (!result) {
					char err[255];
					sprintf(err, "ABL: Unable to read staticData.address [Module %d]", id);
					ABL_Fatal(0, err);
				}
				}
			else {
				staticData[i].integer = 0;
				long result = moduleFile->read((unsigned char*)&staticData[i], sizeof(StackItem));
				if (!result) {
					char err[255];
					sprintf(err, "ABL: Unable to read staticData [Module %d]", id);
					ABL_Fatal(0, err);
				}
			}
	}

	if (ModuleRegistry[handle].numOrderCalls) {
		long numLongs = 1 + ModuleRegistry[handle].numOrderCalls / 32;
		orderCallFlags = (unsigned long*)ABLStackMallocCallback(sizeof(unsigned long) * numLongs);
		if (!orderCallFlags) {
			char err[255];
			sprintf(err, "ABLModule.read: Unable to AblStackHeap->malloc orderCallFlags [Module %d]", id);
			ABL_Fatal(0, err);
		}
		for (long i = 0; i < numLongs; i++)
			orderCallFlags[i] = 0;
	}
	
	if (fresh) {
		ModuleRegistry[handle].numInstances++;
		initCalled = savedInitCalled;

		//------------------------------------------------------
		// This Active Module is now on the instance registry...
		ModuleInstanceRegistry[NumModuleInstances++] = this;

		if (debugger) {
			watchManager = new WatchManager;
			if (!watchManager)
				ABL_Fatal(0, " Unable to AblStackHeap->malloc WatchManager ");
			long result = watchManager->init(MaxWatchesPerModule);
			if (result != ABL_NO_ERR)
				ABL_Fatal(0, " Unable to AblStackHeap->malloc WatchManager ");
			breakPointManager = new BreakPointManager;
			if (!breakPointManager)
				ABL_Fatal(0, " Unable to AblStackHeap->malloc BreakPointManager ");
			result = breakPointManager->init(MaxBreakPointsPerModule);
			if (result != ABL_NO_ERR)
				ABL_Fatal(0, " Unable to AblStackHeap->malloc BreakPointManager ");
		}
	}
}

//---------------------------------------------------------------------------

char* ABLModule::getFileName (void) {

	return(ModuleRegistry[handle].fileName);
}

//---------------------------------------------------------------------------

void ABLModule::setName (const char* _name) {

	strncpy(name, _name, MAX_ABLMODULE_NAME);
	name[MAX_ABLMODULE_NAME-1] = '\0';
}

//---------------------------------------------------------------------------

bool ABLModule::isLibrary (void) {

	return(ModuleRegistry[handle].moduleIdPtr->library != NULL);
}

//---------------------------------------------------------------------------

void ABLModule::resetOrderCallFlags (void) {

	if (ModuleRegistry[handle].numOrderCalls == 0)
		return;

	long numLongs = 1 + ModuleRegistry[handle].numOrderCalls / 32;
	for (long i = 0; i < numLongs; i++)
		orderCallFlags[i] = 0;
}

//---------------------------------------------------------------------------

void ABLModule::setOrderCallFlag (unsigned char dword, unsigned char bit) {

	orderCallFlags[dword] |= (1 << bit);
}

//---------------------------------------------------------------------------

void ABLModule::clearOrderCallFlag(unsigned char dword, unsigned char bit) {

	orderCallFlags[dword] &= ((1 << bit) ^ 0xFFFFFFFF);
}

//---------------------------------------------------------------------------

long ABLModule::getPrevStateHandle (void) {

	if (!prevState)
		return(0);

	for (long i = 0; i < ModuleRegistry[handle].numStateHandles; i++)
		if (strcmp(prevState->name, ModuleRegistry[handle].stateHandles[i].name) == 0)
			return(i);

	return(0);
}

//---------------------------------------------------------------------------

long ABLModule::getStateHandle (void) {

	if (!state)
		return(0);

	for (long i = 0; i < ModuleRegistry[handle].numStateHandles; i++)
		if (strcmp(state->name, ModuleRegistry[handle].stateHandles[i].name) == 0)
			return(i);

	return(0);
}

//---------------------------------------------------------------------------

long ABLModule::execute (ABLParamPtr paramList) {

	CurModule = this;
	if (debugger)
		debugger->setModule(this);

	//--------------------------
	// Execute the ABL module...
	SymTableNodePtr moduleIdPtr = ModuleRegistry[handle].moduleIdPtr;
	if (moduleIdPtr->defn.info.routine.flags & ROUTINE_FLAG_FSM)
		CurFSM = this;
	else
		CurFSM = NULL;
	NumStateTransitions = 0;

	//--------------------------------------------
	// Point to this module's static data space...
	StaticDataPtr = staticData;
	OrderCompletionFlags = orderCallFlags;

	//---------------------------------
	// Init some important variables...
	CurModuleIdPtr = NULL;
	CurRoutineIdPtr = NULL;
	FileNumber = -1;
	errorCount = 0;
	execStatementCount = 0;
	NumExecutions++;

	//------------------
	// Init the stack...
	stackFrameBasePtr = tos = (stack + eternalOffset);

	//---------------------------------------
	// Initialize the module's stack frame...
	level = 1;
	CallStackLevel = 0;
	stackFrameBasePtr = tos + 1;
	
	//-------------------------
	// Function return value...
	pushInteger(0);
	//---------------
	// Static Link...
	pushAddress(NULL);
	//----------------
	// Dynamic Link...
	pushAddress(NULL);
	//------------------
	// Return Address...
	pushAddress(NULL);

	//initDebugger();

	//----------
	// Run it...

	if (paramList) {
		//------------------------------------------------------------------------------
		// NOTE: Currently, parameter passing of arrays is not functioning. This MUST be
		// done...
		long curParam = 0;
		for (SymTableNodePtr formalIdPtr = (SymTableNodePtr)(moduleIdPtr->defn.info.routine.params);
			 formalIdPtr != NULL;
			 formalIdPtr = formalIdPtr->next) {

			TypePtr formalTypePtr = (TypePtr)(formalIdPtr->typePtr);

			if (formalIdPtr->defn.key == DFN_VALPARAM) {

				if (formalTypePtr == RealTypePtr) {
					if (paramList[curParam].type == ABL_PARAM_INTEGER) {
						//---------------------------------------------
						// Real formal parameter, but integer actual...
						pushReal((float)(paramList[curParam].integer));
						}
					else if (paramList[curParam].type == ABL_PARAM_REAL)
						pushReal(paramList[curParam].real);
					}
				else if (formalTypePtr == IntegerTypePtr) {
					if (paramList[curParam].type== ABL_PARAM_INTEGER)
						pushInteger(paramList[curParam].integer);
					else
						return(0);
				}

				//----------------------------------------------------------
				// Formal parameter is an array or record, so make a copy...
				if (formalTypePtr->form == FRM_ARRAY/* || (formalTypePtr->form == FRM_RECORD)*/) {
					//------------------------------------------------------------------------------
					// The following is a little inefficient, but is kept this way to keep it clear.
					// Once it's verified to work, optimize...
					long size = formalTypePtr->size;
					char* dest = (char*)ABLStackMallocCallback((size_t)size);
					if (!dest) {
						char err[255];
						sprintf(err, "ABL: Unable to AblStackHeap->malloc array parameter [Module %d]", id);
						ABL_Fatal(0, err);
					}
					char* src = tos->address;
					char* savePtr = dest;
					memcpy(dest, src, size);
					tos->address = savePtr;
				}
				}
			else {
				//-------------------------------
				// pass by reference parameter...
				if (formalTypePtr == RealTypePtr)
					pushAddress((Address)&(paramList[curParam].real));
				else if (formalTypePtr == IntegerTypePtr)
					pushAddress((Address)&(paramList[curParam].integer));
				else
					return(0);
				//SymTableNodePtr idPtr = getCodeSymTableNodePtr();
				//execVariable(idPtr, USE_REFPARAM);
			}
			curParam++;
		}
	}

	CurModuleHandle = handle;

	//--------------------------------------------------------------------
	// No init function in FSM. Put all init stuff into the start state...
	CallModuleInit = !initCalled;
	initCalled = true;

	NewStateSet = false;
	::execute(moduleIdPtr);

	memcpy(&returnVal, &returnValue, sizeof(StackItem));

	//-----------
	// Summary...
	return(execStatementCount);	
}

//---------------------------------------------------------------------------

long ABLModule::execute (ABLParamPtr moduleParamList, SymTableNodePtr functionIdPtr) {

	CurModule = this;
	if (debugger)
		debugger->setModule(this);

	//--------------------------
	// Execute the ABL module...
	SymTableNodePtr moduleIdPtr = ModuleRegistry[handle].moduleIdPtr;
	if (moduleIdPtr->defn.info.routine.flags & ROUTINE_FLAG_FSM)
		CurFSM = this;
	else
		CurFSM = NULL;
	NumStateTransitions = 0;


	//--------------------------------------------
	// Point to this module's static data space...
	StaticDataPtr = staticData;
	OrderCompletionFlags = orderCallFlags;

	//---------------------------------
	// Init some important variables...
	CurModuleIdPtr = NULL;
	CurRoutineIdPtr = NULL;
	FileNumber = -1;
	errorCount = 0;
	execStatementCount = 0;
	NumExecutions++;
	NewStateSet = false;

	//------------------
	// Init the stack...
	stackFrameBasePtr = tos = (stack + eternalOffset);

	//---------------------------------------
	// Initialize the module's stack frame...
	level = 1;
	CallStackLevel = 0;
	stackFrameBasePtr = tos + 1;
	
	//-------------------------
	// Function return value...
	pushInteger(0);
	//---------------
	// Static Link...
	pushAddress(NULL);
	//----------------
	// Dynamic Link...
	pushAddress(NULL);
	//------------------
	// Return Address...
	pushAddress(NULL);

	//initDebugger();

	//----------
	// Run it...

	if (moduleParamList) {
		//------------------------------------------------------------------------------
		// NOTE: Currently, parameter passing of arrays is not functioning. This MUST be
		// done...
		long curParam = 0;
		for (SymTableNodePtr formalIdPtr = (SymTableNodePtr)(moduleIdPtr->defn.info.routine.params);
			 formalIdPtr != NULL;
			 formalIdPtr = formalIdPtr->next) {

			TypePtr formalTypePtr = (TypePtr)(formalIdPtr->typePtr);

			if (formalIdPtr->defn.key == DFN_VALPARAM) {

				if (formalTypePtr == RealTypePtr) {
					if (moduleParamList[curParam].type == ABL_PARAM_INTEGER) {
						//---------------------------------------------
						// Real formal parameter, but integer actual...
						pushReal((float)(moduleParamList[curParam].integer));
						}
					else if (moduleParamList[curParam].type == ABL_PARAM_REAL)
						pushReal(moduleParamList[curParam].real);
					}
				else if (formalTypePtr == IntegerTypePtr) {
					if (moduleParamList[curParam].type== ABL_PARAM_INTEGER)
						pushInteger(moduleParamList[curParam].integer);
					else
						return(0);
				}

				//----------------------------------------------------------
				// Formal parameter is an array or record, so make a copy...
				if (formalTypePtr->form == FRM_ARRAY/* || (formalTypePtr->form == FRM_RECORD)*/) {
					//------------------------------------------------------------------------------
					// The following is a little inefficient, but is kept this way to keep it clear.
					// Once it's verified to work, optimize...
					long size = formalTypePtr->size;
					char* dest = (char*)ABLStackMallocCallback((size_t)size);
					if (!dest) {
						char err[255];
						sprintf(err, "ABL: Unable to AblStackHeap->malloc array parameter [Module %d]", id);
						ABL_Fatal(0, err);
					}
					char* src = tos->address;
					char* savePtr = dest;
					memcpy(dest, src, size);
					tos->address = savePtr;
				}
				}
			else {
				//-------------------------------
				// pass by reference parameter...
				if (formalTypePtr == RealTypePtr)
					pushAddress((Address)&(moduleParamList[curParam].real));
				else if (formalTypePtr == IntegerTypePtr)
					pushAddress((Address)&(moduleParamList[curParam].integer));
				else
					return(0);
			}
			curParam++;
		}
	}

	CurModuleHandle = handle;

	CallModuleInit = !initCalled;
	initCalled = true;

	::executeChild(moduleIdPtr, functionIdPtr);

	memcpy(&returnVal, &returnValue, sizeof(StackItem));

	//-----------
	// Summary...
	return(execStatementCount);	
}

//---------------------------------------------------------------------------

SymTableNodePtr ABLModule::findSymbol (const char* symbolName, SymTableNodePtr curFunction, bool searchLibraries) {

	if (curFunction) {
        char* sn = strdup(symbolName);
		SymTableNodePtr symbol = searchSymTable(strlwr(sn), curFunction->defn.info.routine.localSymTable);
        free(sn);
		if (symbol)
			return(symbol);
	}

    char* sn = strdup(symbolName);
    sn = strlwr(sn);

	SymTableNodePtr symbol = searchSymTable(sn, ModuleRegistry[handle].moduleIdPtr->defn.info.routine.localSymTable);

	if (!symbol && searchLibraries) {
		for (long i = 0; i < ModuleRegistry[handle].numLibrariesUsed; i++) {
			symbol = searchSymTable(sn, ModuleRegistry[ModuleRegistry[handle].librariesUsed[i]->handle].moduleIdPtr->defn.info.routine.localSymTable);
			if (symbol)
				break;
		}
	}

    free(sn);

	return(symbol);
}

//---------------------------------------------------------------------------

SymTableNodePtr ABLModule::findFunction (const char* functionName, bool searchLibraries) {

	SymTableNodePtr symbol = searchSymTableForFunction(functionName, ModuleRegistry[handle].moduleIdPtr->defn.info.routine.localSymTable);

	if (!symbol && searchLibraries) 
    {
		for (long i = 0; i < ModuleRegistry[handle].numLibrariesUsed; i++) 
        {
            char temp[1024];
            memset(temp, 0, 1024 );
            strncpy( temp, functionName, (strlen(functionName) > 1020) ? 1020 : strlen(functionName) );

			symbol = searchSymTable(_strlwr(temp), ModuleRegistry[ModuleRegistry[handle].librariesUsed[i]->handle].moduleIdPtr->defn.info.routine.localSymTable);

			if (symbol)
				break;
		}
	}

	return(symbol);
}

//---------------------------------------------------------------------------

SymTableNodePtr ABLModule::findState (const char* stateName) {

	SymTableNodePtr symbol = searchSymTableForState(stateName, ModuleRegistry[handle].moduleIdPtr->defn.info.routine.localSymTable);
	return(symbol);
}

//---------------------------------------------------------------------------

long ABLModule::findStateHandle (const char* stateName) {

	for (long i = 1; i < ModuleRegistry[handle].numStateHandles; i++)
		if (strcmp(stateName, ModuleRegistry[handle].stateHandles[i].name) == 0)
			return(i);
	return(0);
}

//---------------------------------------------------------------------------

long ABLModule::setStaticInteger (char* name, long value) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(1);
	
	if (symbol->typePtr != IntegerTypePtr)
		return(2);

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(3);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	*((long*)dataPtr) = value;
	return(0);
}

//---------------------------------------------------------------------------

long ABLModule::getStaticInteger (char* name) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(0xFFFFFFFF);
	
	if (symbol->typePtr != IntegerTypePtr)
		return(0xFFFFFFFF);

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(0xFFFFFFFF);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	return(*((long*)dataPtr));
}

//---------------------------------------------------------------------------

long ABLModule::setStaticReal (char* name, float value) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(1);
	
	if (symbol->typePtr != RealTypePtr)
		return(2);

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(3);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	*((float*)dataPtr) = value;
	return(0);
}

//---------------------------------------------------------------------------

float ABLModule::getStaticReal (char* name) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(-999999.0);
	
	if (symbol->typePtr != RealTypePtr)
		return(-999999.0);

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(-999999.0);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	return(*((float*)dataPtr));
}

//---------------------------------------------------------------------------

long ABLModule::setStaticIntegerArray (char* name, long numValues, long* values) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(1);
	
	//--------------------------------------------------------------------------
	// NOTE: This function is not dummy-proof. Essentially, this routine copies
	// the values data into the array's data space WITHOUT checking to make sure
	// the array really is an array (single or mult-dimensional) of reals. User
	// beware!

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(3);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	memcpy(dataPtr->address, values, 4 * numValues);
	
	return(0);
}

//---------------------------------------------------------------------------

long ABLModule::getStaticIntegerArray (char* name, long numValues, long* values) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(0);
	
	//--------------------------------------------------------------------------
	// NOTE: This function is not dummy-proof. Essentially, this routine copies
	// the values data into the array's data space WITHOUT checking to make sure
	// the array really is an array (single or mult-dimensional) of reals. User
	// beware!

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(0);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	memcpy(values, dataPtr->address, 4 * numValues);
	return(1);
}

//---------------------------------------------------------------------------

long ABLModule::setStaticRealArray (char* name, long numValues, float* values) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(1);

	//--------------------------------------------------------------------------
	// NOTE: This function is not dummy-proof. Essentially, this routine copies
	// the values data into the array's data space WITHOUT checking to make sure
	// the array really is an array (single or mult-dimensional) of reals. User
	// beware!

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(3);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	memcpy(dataPtr->address, values, 4 * numValues);

	return(0);
}

//---------------------------------------------------------------------------

long ABLModule::getStaticRealArray (char* name, long numValues, float* values) {

	SymTableNodePtr symbol = findSymbol(name);
	if (!symbol)
		return(0);

	//--------------------------------------------------------------------------
	// NOTE: This function is not dummy-proof. Essentially, this routine copies
	// the values data into the array's data space WITHOUT checking to make sure
	// the array really is an array (single or mult-dimensional) of reals. User
	// beware!

	if (symbol->defn.info.data.varType != VAR_TYPE_STATIC)
		return(0);

	StackItemPtr dataPtr = staticData + symbol->defn.info.data.offset;
	memcpy(values, dataPtr->address, 4 * numValues);

	return(1);
}

//---------------------------------------------------------------------------

char* ABLModule::getSourceFile (long fileNumber) {

	return(ModuleRegistry[handle].sourceFiles[fileNumber]);
}

//---------------------------------------------------------------------------

char* ABLModule::getSourceDirectory (long fileNumber, char* directory) {

	char* fileName = ModuleRegistry[handle].sourceFiles[fileNumber];
	long curChar = strlen(fileName);
	while ((curChar > -1) && (fileName[curChar] != '\\'))
		curChar--;
	if (curChar == -1)
		return(NULL);
	strcpy(directory, fileName);
	directory[curChar + 1] = '\0';
	return(directory);
}

//---------------------------------------------------------------------------

void buildRoutineList (SymTableNodePtr curSymbol, ModuleInfo* moduleInfo) {

	if (curSymbol) {
		buildRoutineList(curSymbol->left, moduleInfo);
		if (curSymbol->defn.key == DFN_FUNCTION) {
			if (moduleInfo->numRoutines < 1024) {
				strcpy(moduleInfo->routineInfo[moduleInfo->numRoutines].name, curSymbol->name);
				moduleInfo->routineInfo[moduleInfo->numRoutines].codeSegmentSize = curSymbol->defn.info.routine.codeSegmentSize;
				moduleInfo->numRoutines++;
			}
		}
		buildRoutineList(curSymbol->right, moduleInfo);
	}
}

//---------------------------------------------------------------------------

void ABLModule::getInfo (ModuleInfo* moduleInfo) {

	strcpy(moduleInfo->name, name);
	strcpy(moduleInfo->fileName, ModuleRegistry[handle].fileName);

	moduleInfo->numRoutines = 0;
	buildRoutineList(ModuleRegistry[handle].moduleIdPtr->defn.info.routine.localSymTable, moduleInfo);
	for (long i = 0; i < moduleInfo->numRoutines; i++)
		moduleInfo->totalCodeSegmentSize += moduleInfo->routineInfo[i].codeSegmentSize;

	moduleInfo->numStaticVars = ModuleRegistry[handle].numStaticVars;
	moduleInfo->totalSizeStaticVars = ModuleRegistry[handle].totalSizeStaticVars;

	long largest = 0;
	for (int i = 0; i < moduleInfo->numStaticVars; i++) {
		if (ModuleRegistry[handle].sizeStaticVars[i] > ModuleRegistry[handle].sizeStaticVars[largest])
			largest = i;
	}
	moduleInfo->largestStaticVar.size = 0;
	if (ModuleRegistry[handle].sizeStaticVars)
		moduleInfo->largestStaticVar.size = ModuleRegistry[handle].sizeStaticVars[largest];
	moduleInfo->largestStaticVar.name[0] = '\0';
}

//---------------------------------------------------------------------------

void ABLModule::destroy (void) {

	if ((id > -1) && ModuleInstanceRegistry) {
		//-----------------------------------------------
		// It's on the active registry, so pull it off...
		for (long i = 0; i < NumModuleInstances; i++)
			if (ModuleInstanceRegistry[i] == this) {
				ModuleInstanceRegistry[i] = ModuleInstanceRegistry[NumModuleInstances - 1];
				ModuleInstanceRegistry[NumModuleInstances - 1] = NULL;
				NumModuleInstances--;
				NumModules--;
				break;
			}
	}

	if (watchManager) {
		delete watchManager;
		watchManager = NULL;
	}

	if (breakPointManager) {
		delete breakPointManager;
		breakPointManager = NULL;
	}

	if (staticData) {
		ABLStackFreeCallback(staticData);
		staticData = NULL;
	}
}

//***************************************************************************
// MISC routines
//***************************************************************************

void ABLi_saveEnvironment (ABLFile* ablFile) {

	ablFile->writeLong(numLibrariesLoaded);
	ablFile->writeLong(NumModulesRegistered);
	ablFile->writeLong(NumModules);
	for (long i = 0; i < NumModulesRegistered; i++) {
		ablFile->writeString(ModuleRegistry[i].fileName);
		ablFile->writeByte('\0');
	}
	ablFile->writeLong(999);
	for (int i = 0; i < eternalOffset; i++) {
		StackItemPtr dataPtr = (StackItemPtr)stack + i;
		if (EternalVariablesSizes[i] > 0)
			ablFile->write((unsigned char*)dataPtr->address, EternalVariablesSizes[i]);
		else
			ablFile->write((unsigned char*)dataPtr, sizeof(StackItem));
	}
	for (int i = 0; i < NumModules; i++)
	{
		if (ModuleInstanceRegistry[i])
			ModuleInstanceRegistry[i]->write(ablFile);
	}
}

//---------------------------------------------------------------------------

void ABLi_loadEnvironment (ABLFile* ablFile, bool malloc) {

	long numLibs = ablFile->readLong();
	long numModsRegistered = ablFile->readLong();
	long numMods = ablFile->readLong();

	for (int i = 0; i < numLibs; i++) {
		unsigned char fileName[1024];
		long result = ablFile->readString(fileName);
		if (!result) {
			char err[255];
			sprintf(err, "ABLi_loadEnvironment: Unable to read filename [Module %d]", i);
			ABL_Fatal(0, err);
		}
		if (malloc) {
			long numErrors, numLinesProcessed;
			ABLModulePtr library = ABLi_loadLibrary((const char*)fileName, &numErrors, &numLinesProcessed, NULL, false, false);
			if (!library) {
				char err[255];
				sprintf(err, "ABLi_loadEnvironment: Unable to load library [Module %d]", i);
				ABL_Fatal(0, err);
			}
		}
	}

	for (int i = 0; i < (numModsRegistered - numLibs); i++) {
		unsigned char fileName[1024];
		long result = ablFile->readString(fileName);
		if (!result) {
			char err[255];
			sprintf(err, "ABLi_loadEnvironment: Unable to read filename [Module %d]", i);
			ABL_Fatal(0, err);
		}
		long numErrors, numLinesProcessed;
		if (malloc) {
			long handle = ABLi_preProcess((const char*)fileName, &numErrors, &numLinesProcessed);
			if (handle < 0) {
				char err[255];
				sprintf(err, "ABLi_loadEnvironment: Unable to preprocess [Module %d]", i);
				ABL_Fatal(0, err);
			}
		}
	}
	long mark = ablFile->readLong();
	for (int i = 0; i < eternalOffset; i++) {
		StackItemPtr dataPtr = (StackItemPtr)stack + i;
		if (EternalVariablesSizes[i] > 0)
			ablFile->read((unsigned char*)dataPtr->address, EternalVariablesSizes[i]);
		else
			ablFile->read((unsigned char*)dataPtr, sizeof(StackItem));
	}
	for (int i = 0; i < numLibs; i++) {
		ABLModulePtr library = LibraryInstanceRegistry[i];
		library->read(ablFile);
	}

	for (int i = 0; i < (numMods - numLibs); i++) {
		ABLModulePtr module = NULL;
		if (malloc)
			module = new ABLModule;
		else
			module = ModuleInstanceRegistry[numLibs + i];
		module->read(ablFile);
	}
}

//***************************************************************************


