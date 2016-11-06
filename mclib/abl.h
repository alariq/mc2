//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABL.H
//
//***************************************************************************

#ifndef ABL_H
#define	ABL_H

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLEXEC_H
#include"ablexec.h"
#endif

#ifndef ABLSYMT_H
#include"ablsymt.h"
#endif

#ifndef ABLENV_H
#include"ablenv.h"
#endif

#ifndef ABLDBUG_H
#include"abldbug.h"
#endif

//***************************************************************************

#define	DEBUGGING

void ABLi_init (unsigned long runtimeStackSize, // = 20480,
				unsigned long maxCodeBufferSize, // = 10240,
				unsigned long maxRegisteredModules, // = 200,
				unsigned long maxStaticVariables, // = 100,
				void* (*systemMallocCallback) (unsigned long memSize),
				void* (*stackMallocCallback) (unsigned long memSize),
				void* (*codeMallocCallback) (unsigned long memSize),
				void* (*symbolMallocCallback) (unsigned long memSize),
				void (*systemFreeCallback) (void* memBlock),
				void (*stackFreeCallback) (void* memBlock),
				void (*codeFreeCallback) (void* memBlock),
				void (*symbolFreeCallback) (void* memBlock),
				long (*fileCreateCB) (void** file, const char* fName),
				long (*fileOpenCB) (void** file, const char* fName),
				long (*fileCloseCB) (void** file),
				bool (*fileEofCB) (void* file),
				long (*fileReadCB) (void* file, unsigned char* buffer, long length),
				long (*fileReadLongCB) (void* file),
				long (*fileReadStringCB) (void* file, unsigned char* buffer),
				long (*fileReadLineExCB) (void* file, unsigned char* buffer, long maxLength),
				long (*fileWriteCB) (void* file, unsigned char* buffer, long length),
				long (*fileWriteByteCB) (void* file, unsigned char byte),
				long (*fileWriteLongCB) (void* file, long value),
				long (*fileWriteStringCB) (void* file, const char* buffer),
				void (*debuggerPrintCallback) (const char* s),
				void (*ablFatalCallback) (long code, const char* s),
				bool debugInfo = false,
				bool debug = false,
				bool profile = false);

ABLParamPtr ABLi_createParamList (long numParameters);

void ABLi_setIntegerParam (ABLParamPtr paramList, long index, long value);

void ABLi_setRealParam (ABLParamPtr paramList, long index, float value);

void ABLi_deleteParamList (ABLParamPtr paramList);

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

long ABLi_execute (SymTableNodePtr moduleIdPtr,
				   SymTableNodePtr functionIdPtr = NULL,
				   ABLParamPtr paramList = NULL,
				   StackItemPtr returnVal = NULL);

long ABLi_deleteModule (SymTableNodePtr moduleIdPtr);

ABLModulePtr ABLi_getModule (long id);

DebuggerPtr ABLi_getDebugger (void);

void ABLi_saveEnvironment (ABLFile* ablFile);

void ABLi_loadEnvironment (ABLFile* ablFile, bool malloc);

void ABLi_close (void);

bool ABLi_enabled (void);

void ABLi_addFunction (const char* name,
					   bool isOrder,
					   const char* paramList,
					   const char* returnType,
					   void (*codeCallback)(void));

void ABLi_setRandomCallbacks (void (*seedRandomCallback) (unsigned long seed),
							  long (*randomCallback) (long range));
void ABLi_setDebugPrintCallback (void (*ABLDebugPrintCallback) (const char* s));
void ABLi_setGetTimeCallback (unsigned long (*ABLGetTimeCallback) (void));

void ABLi_setEndlessStateCallback (void (*endlessStateCallback) (UserFile* log));

char ABLi_popChar (void);
int ABLi_popInteger (void);
float ABLi_popReal (void);
bool ABLi_popBoolean (void);
float ABLi_popIntegerReal (void);
long ABLi_popAnything (ABLStackItem* value);

char* ABLi_popCharPtr (void);
long* ABLi_popIntegerPtr (void);
float* ABLi_popRealPtr (void);
char* ABLi_popBooleanPtr (void);

void ABLi_pushInteger (long value);
void ABLi_pushReal (float value);
void ABLi_pushBoolean (bool value);

long ABLi_peekInteger (void);
float ABLi_peekReal (void);
bool ABLi_peekBoolean (void);

char* ABLi_peekCharPtr (void);
long* ABLi_peekIntegerPtr (void);
float* ABLi_peekRealPtr (void);

void ABLi_pokeChar (long val);
void ABLi_pokeInteger (long val);
void ABLi_pokeReal (float val);
void ABLi_pokeBoolean (bool val);

long ABLi_registerInteger (char* name, long* address, long numElements = 0);
long ABLi_registerReal (char* name, float* address, long numElements = 0);

bool ABLi_getSkipOrder (void);
void ABLi_resetOrders (void);
long ABLi_getCurrentState (void);
void ABLi_transState (long newState);

//***************************************************************************

#endif
