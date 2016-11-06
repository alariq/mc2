//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLXSTD.CPP
//
//***************************************************************************

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string_win.h>
#include<math.h>
#include<time.h>

#ifndef ABL_H
#include"abl.h"
#endif

inline signed int double2long(double _in)
{
	_in+=6755399441055744.0;
	return(*(signed int*)&_in);
}

//***************************************************************************

//----------
// EXTERNALS

extern long				level;
extern int              execLineNumber;
extern long				FileNumber;
extern char*			codeSegmentPtr;
extern TokenCodeType	codeToken;
extern StackItem*		stack;
extern StackItemPtr		tos;
extern StackItemPtr		stackFrameBasePtr;
extern SymTableNodePtr	CurRoutineIdPtr;
extern long				CurModuleHandle;
extern TypePtr			IntegerTypePtr;
extern TypePtr			RealTypePtr;
extern TypePtr			BooleanTypePtr;
extern TypePtr			CharTypePtr;
extern ABLModulePtr		CurModule;
extern ABLModulePtr		CurFSM;
extern long	MaxLoopIterations;
extern DebuggerPtr		debugger;
extern bool				NewStateSet;

//--------
// GLOBALS

StackItem				returnValue;
bool					eofFlag = false;
bool					ExitWithReturn = false;
bool					ExitFromTacOrder = false;
bool					SkipOrder = false;
TokenCodeType			ExitRoutineCodeSegment[2] = {TKN_END_FUNCTION,
													 TKN_SEMICOLON};
TokenCodeType			ExitOrderCodeSegment[2] = {TKN_END_ORDER,
												   TKN_SEMICOLON};
TokenCodeType			ExitStateCodeSegment[2] = {TKN_END_STATE,
												   TKN_SEMICOLON};

//***************************************************************************
// USEFUL ABL HELP ROUTINES
//***************************************************************************

char ABLi_popChar (void) {

	getCodeToken();
	execExpression();
	char val = (char)tos->integer;
	pop();
	
	return(val);
}

//---------------------------------------------------------------------------

int ABLi_popInteger (void) {

	getCodeToken();
	execExpression();
	int val = tos->integer;
	pop();
	
	return(val);
}

//---------------------------------------------------------------------------

float ABLi_popReal (void) {

	getCodeToken();
	execExpression();
	float val = tos->real;
	pop();

	return(val);
}

//---------------------------------------------------------------------------

float ABLi_popIntegerReal (void) {

	getCodeToken();
	TypePtr paramTypePtr = execExpression();
	float val = 0.0;
	if (paramTypePtr == IntegerTypePtr)
		val = (float)tos->integer;
	else
		val = tos->real;
	pop();

	return(val);
}

//---------------------------------------------------------------------------

bool ABLi_popBoolean (void) {

	getCodeToken();
	execExpression();
	long val = tos->integer;
	pop();
	
	return(val == 1);
}

//---------------------------------------------------------------------------

char* ABLi_popCharPtr (void) {

	//--------------------------
	// Get destination string...
	getCodeToken();
	execExpression();
	char* charPtr = (char*)tos->address;
	pop();

	return(charPtr);
}

//---------------------------------------------------------------------------

long* ABLi_popIntegerPtr (void) {

	getCodeToken();
	SymTableNodePtr idPtr = getCodeSymTableNodePtr();
	execVariable(idPtr, USE_REFPARAM);
	long* integerPtr = (long*)(&((StackItemPtr)tos->address)->integer);
	pop();

	return(integerPtr);
}

//---------------------------------------------------------------------------

float* ABLi_popRealPtr (void) {

	getCodeToken();
	SymTableNodePtr idPtr = getCodeSymTableNodePtr();
	execVariable(idPtr, USE_REFPARAM);
	float* realPtr = (float*)(&((StackItemPtr)tos->address)->real);
	pop();

	return(realPtr);
}

//---------------------------------------------------------------------------

char* ABLi_popBooleanPtr (void) {

	//--------------------------
	// Get destination string...
	getCodeToken();
	execExpression();
	char* charPtr = (char*)tos->address;
	pop();

	return(charPtr);
}

//---------------------------------------------------------------------------

long ABLi_popAnything (ABLStackItem* value) {

	getCodeToken();
	TypePtr paramTypePtr = execExpression();

	long type = -1;
	if (paramTypePtr == IntegerTypePtr) {
		value->type = type = ABL_STACKITEM_INTEGER;
		value->data.integer = tos->integer;
		}
	else if (paramTypePtr == BooleanTypePtr) {
		value->type = type = ABL_STACKITEM_BOOLEAN;
		value->data.boolean = (tos->integer ? true : false);
		}
	else if (paramTypePtr == CharTypePtr) {
		value->type = type = ABL_STACKITEM_CHAR;
		value->data.character  = tos->byte;
		}
	else if (paramTypePtr == RealTypePtr) {
		value->type = type = ABL_STACKITEM_REAL;
		value->data.real = tos->real;
		}
	else if (paramTypePtr->form == FRM_ARRAY) {
		if (paramTypePtr->info.array.elementTypePtr == CharTypePtr) {
			value->type = type = ABL_STACKITEM_CHAR_PTR;
			value->data.characterPtr = (char*)tos->address;
			}
		else if (paramTypePtr->info.array.elementTypePtr == IntegerTypePtr) {
			value->type = type = ABL_STACKITEM_INTEGER_PTR;
			value->data.integerPtr = (long*)tos->address;
			}
		else if (paramTypePtr->info.array.elementTypePtr == RealTypePtr) {
			value->type = type = ABL_STACKITEM_REAL_PTR;
			value->data.realPtr = (float*)tos->address;
			}
		else if (paramTypePtr->info.array.elementTypePtr == BooleanTypePtr) {
			value->type = type = ABL_STACKITEM_BOOLEAN_PTR;
			value->data.booleanPtr = (bool*)tos->address;
		}
	}
	pop();
	return(type);
}

//---------------------------------------------------------------------------

void ABLi_pushBoolean (bool value) {

	StackItemPtr valuePtr = ++tos;

	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->integer = value ? 1 : 0;
}

//---------------------------------------------------------------------------

void ABLi_pushInteger (long value) {

	StackItemPtr valuePtr = ++tos;

	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->integer = value;
}

//---------------------------------------------------------------------------

void ABLi_pushReal (float value) {

	StackItemPtr valuePtr = ++tos;

	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->real = value;
}

//---------------------------------------------------------------------------

void ABLi_pushChar (char value) {

	StackItemPtr valuePtr = ++tos;

	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->integer = value;
}

//---------------------------------------------------------------------------

long ABLi_peekInteger (void) {

	getCodeToken();
	execExpression();
	return(tos->integer);
}

//---------------------------------------------------------------------------

float ABLi_peekReal (void) {

	getCodeToken();
	execExpression();
	return(tos->real);
}

//---------------------------------------------------------------------------

bool ABLi_peekBoolean (void) {

	getCodeToken();
	execExpression();
	return(tos->integer == 1);
}

//---------------------------------------------------------------------------

char* ABLi_peekCharPtr (void) {

	getCodeToken();
	execExpression();
	return((char*)tos->address);
}

//---------------------------------------------------------------------------

long* ABLi_peekIntegerPtr (void) {

	getCodeToken();
	SymTableNodePtr idPtr = getCodeSymTableNodePtr();
	execVariable(idPtr, USE_REFPARAM);
	return((long*)(&((StackItemPtr)tos->address)->integer));
}

//---------------------------------------------------------------------------

float* ABLi_peekRealPtr (void) {

	getCodeToken();
	SymTableNodePtr idPtr = getCodeSymTableNodePtr();
	execVariable(idPtr, USE_REFPARAM);
	return((float*)(&((StackItemPtr)tos->address)->real));
}

//---------------------------------------------------------------------------

void ABLi_pokeChar (long val) {

	tos->integer = val;
}

//---------------------------------------------------------------------------

void ABLi_pokeInteger (long val) {

	tos->integer = val;
}

//---------------------------------------------------------------------------

void ABLi_pokeReal (float val) {

	tos->real = val;
}

//---------------------------------------------------------------------------

void ABLi_pokeBoolean (bool val) {

	tos->integer = val ? 1 : 0;
}

//***************************************************************************

void execOrderReturn (long returnVal) {

	//-----------------------------
	// Assignment to function id...
	StackFrameHeaderPtr headerPtr = (StackFrameHeaderPtr)stackFrameBasePtr;
	long delta = level - CurRoutineIdPtr->level - 1;
	while (delta-- > 0)
		headerPtr = (StackFrameHeaderPtr)headerPtr->staticLink.address;

	if (CurRoutineIdPtr->defn.info.routine.flags & ROUTINE_FLAG_STATE) {
		//----------------------------------
		// Return in a state function, so...
		if (debugger)
			debugger->traceDataStore(CurRoutineIdPtr, CurRoutineIdPtr->typePtr, (StackItemPtr)headerPtr, CurRoutineIdPtr->typePtr);
	
		ExitWithReturn = true;
		ExitFromTacOrder = true;

		if (returnVal == 0) {
			//----------------------------------------------------------
			// Use the "eject" code only if called for a failed Order...
			codeSegmentPtr = (char*)ExitStateCodeSegment;
			getCodeToken();
		}
		
		}
	else {
		//-------------------------------------------------------------------------
		// All Order functions (TacticalOrder/GeneralOrder/ActionOrder) must return
		// an integer error code, so we assume the return type is IntegerTypePtr...
		StackItemPtr targetPtr = (StackItemPtr)headerPtr;
		targetPtr->integer = returnVal;

		//----------------------------------------------------------------------
		// Preserve the return value, in case we need it for the calling user...
		memcpy(&returnValue, targetPtr, sizeof(StackItem));

		if (debugger)
			debugger->traceDataStore(CurRoutineIdPtr, CurRoutineIdPtr->typePtr, (StackItemPtr)headerPtr, CurRoutineIdPtr->typePtr);
	
		ExitWithReturn = true;
		ExitFromTacOrder = true;

		if (returnVal == 0) {
			//----------------------------------------------------------
			// Use the "eject" code only if called for a failed Order...
			codeSegmentPtr = (char*)ExitOrderCodeSegment;
			getCodeToken();
		}
	}
}

//***************************************************************************

void execStdReturn (void) {

	memset(&returnValue, 0, sizeof(StackItem));
	if (CurRoutineIdPtr->typePtr) {
		//-----------------------------
		// Assignment to function id...
		StackFrameHeaderPtr headerPtr = (StackFrameHeaderPtr)stackFrameBasePtr;
		long delta = level - CurRoutineIdPtr->level - 1;
		while (delta-- > 0)
			headerPtr = (StackFrameHeaderPtr)headerPtr->staticLink.address;

		StackItemPtr targetPtr = (StackItemPtr)headerPtr;
		TypePtr targetTypePtr = (TypePtr)(CurRoutineIdPtr->typePtr);
		getCodeToken();

		//---------------------------------------------------------------
		// Routine execExpression() leaves the expression value on top of
		// stack...
		getCodeToken();
		TypePtr expressionTypePtr = execExpression();

		//--------------------------
		// Now, do the assignment...
		if ((targetTypePtr == RealTypePtr) && (expressionTypePtr == IntegerTypePtr)) {
			//-------------------------
			// integer assigned to real
			targetPtr->real = (float)(tos->integer);
			}
		else if (targetTypePtr->form == FRM_ARRAY) {
			//-------------------------
			// Copy the array/record...
			char* dest = (char*)targetPtr;
			char* src = tos->address;
			long size = targetTypePtr->size;
			memcpy(dest, src, size);
			}
		else if ((targetTypePtr == IntegerTypePtr) || (targetTypePtr->form == FRM_ENUM)) {
			//------------------------------------------------------
			// Range check assignment to integer or enum subrange...
			targetPtr->integer = tos->integer;
			}
		else {
			//-----------------------
			// Assign real to real...
			targetPtr->real = tos->real;
		}

		//-----------------------------
		// Grab the expression value...
		pop();

		//----------------------------------------------------------------------
		// Preserve the return value, in case we need it for the calling user...
		memcpy(&returnValue, targetPtr, sizeof(StackItem));
		
		if (debugger)
			debugger->traceDataStore(CurRoutineIdPtr, CurRoutineIdPtr->typePtr, targetPtr, targetTypePtr);
	}

	//-----------------------
	// Grab the semi-colon...
	getCodeToken();

	if (CurRoutineIdPtr->defn.info.routine.flags & ROUTINE_FLAG_ORDER)
		codeSegmentPtr = (char*)ExitOrderCodeSegment;
	else if (CurRoutineIdPtr->defn.info.routine.flags & ROUTINE_FLAG_STATE)
		codeSegmentPtr = (char*)ExitStateCodeSegment;
	else
		codeSegmentPtr = (char*)ExitRoutineCodeSegment;
	ExitWithReturn = true;

	getCodeToken();
}

//***************************************************************************

void execStdPrint (void) {

	//---------------------------
	// Grab the opening LPAREN...
	getCodeToken();

	//----------------------------
	// Get parameter expression...
	getCodeToken();
	TypePtr paramTypePtr = execExpression();

	char buffer[20];
	char* s = buffer;
	if (paramTypePtr == IntegerTypePtr)
		sprintf(buffer, "%d", tos->integer);
	else if (paramTypePtr == BooleanTypePtr)
		sprintf(buffer, "%s", tos->integer ? "true" : "false");
	else if (paramTypePtr == CharTypePtr)
		sprintf(buffer, "%c", tos->byte);
	else if (paramTypePtr == RealTypePtr)
		sprintf(buffer, "%.4f", tos->real);
	else if ((paramTypePtr->form == FRM_ARRAY) && (paramTypePtr->info.array.elementTypePtr == CharTypePtr))
		s = (char*)tos->address;
	pop();

	if (debugger) {
		char message[512];
		sprintf(message, "PRINT:  \"%s\"", s);
		debugger->print(message);
		sprintf(message, "   MODULE %s", CurModule->getName());
		debugger->print(message);
		sprintf(message, "   FILE %s", CurModule->getSourceFile(FileNumber));
		debugger->print(message);
		sprintf(message, "   LINE %d", execLineNumber);
		debugger->print(message);
		}
/*	else if (TACMAP) {
		aChatWindow* chatWin = TACMAP->getChatWindow();
		if (chatWin)
			chatWin->processChatString(0, s, -1);
		else {
#ifdef _DEBUG
			OutputDebugString(s);
#endif
		}
		}
*/	else {
#ifdef _DEBUG
		ABLDebugPrintCallback(s);
#endif
	}
	//-----------------------
	// Grab closing RPAREN...
	getCodeToken();
}

//***************************************************************************

TypePtr execStdConcat (void) {

	//-------------------
	// Grab the LPAREN...
	getCodeToken();

	//--------------------------
	// Get destination string...
	getCodeToken();
	execExpression();
	char* dest = (char*)tos->address;
	pop();

	//----------------------
	// Get item to append...
	getCodeToken();
	TypePtr paramTypePtr = execExpression();
	char buffer[20];
	if (paramTypePtr == IntegerTypePtr) {
		sprintf(buffer, "%d", tos->integer);
		strcat(dest, buffer);
		}
	else if (paramTypePtr == CharTypePtr) {
		sprintf(buffer, "%c", tos->byte);
		strcat(dest, buffer);
		}
	else if (paramTypePtr == RealTypePtr) {
		sprintf(buffer, "%.2f", tos->real);
		strcat(dest, buffer);
		}
	else if (paramTypePtr == BooleanTypePtr) {
		sprintf(buffer, "%s", tos->integer ? "true" : "false");
		strcat(dest, buffer);
		}
	else if ((paramTypePtr->form == FRM_ARRAY) && (paramTypePtr->info.array.elementTypePtr == CharTypePtr))
		strcat(dest, (char*)tos->address);

	tos->integer = 0;

	getCodeToken();
	return(IntegerTypePtr);
}

//***************************************************************************

void execStdAbs (void) {

	float val = ABLi_popIntegerReal();

	if (val < 0.0)
		val = -val;

	ABLi_pushReal(val);
}

//*****************************************************************************

void execStdRound (void) {

	float val = ABLi_popReal();

	if (val > 0.0)
		ABLi_pushInteger((long)(val + 0.5));
	else
		ABLi_pushInteger((long)(val - 0.5));
}

//***************************************************************************

void execStdSqrt (void) {

	float val = ABLi_popIntegerReal();

	if (val < 0.0)
		runtimeError(ABL_ERR_RUNTIME_INVALID_FUNCTION_ARGUMENT);
	else
		ABLi_pushReal((float)sqrt(val));
}

//***************************************************************************

void execStdTrunc (void) {

	float val = ABLi_popReal();
	ABLi_pushInteger((long)val);
}

//***************************************************************************

void execStdFileOpen (void) {

	char* fileName = ABLi_popCharPtr();

	long fileHandle = -1;
	UserFile* userFile = UserFile::getNewFile();
	if (userFile) {
		long err = userFile->open(fileName);
		if (!err)
			fileHandle = userFile->handle;
	}

	ABLi_pushInteger(fileHandle);
}

//---------------------------------------------------------------------------

void execStdFileWrite (void) {

	int fileHandle = ABLi_popInteger();
	char* string = ABLi_popCharPtr();

	UserFile* userFile = UserFile::files[fileHandle];
	if (userFile->inUse)
		userFile->write(string);
}

//---------------------------------------------------------------------------

void execStdFileClose (void) {

	int fileHandle = ABLi_popInteger();

	UserFile* userFile = UserFile::files[fileHandle];
	if (userFile->inUse)
		userFile->close();
}

//***************************************************************************

void execStdGetModule (void) {

	//----------------------------------------------------------
	// Return the handle of the current module being executed...
	char* curBuffer = ABLi_popCharPtr();
	char* fsmBuffer = ABLi_popCharPtr();
	strcpy(curBuffer, CurModule->getFileName());
	strcpy(fsmBuffer, CurFSM ? CurFSM->getFileName() : "none");
	ABLi_pushInteger(CurModuleHandle);
}

//***************************************************************************

void execStdSetMaxLoops (void) {

	//----------------------------------------------------------------------
	//
	//	SET MAX LOOPS function:
	//
	//		Sets the max number of loops that may occur (in a for, while or
	//		repeat loop) before an infinite loop run-time error occurs.
	//
	//		PARAMS:	integer
	//
	//		RETURN: none
	//
	//----------------------------------------------------------------------

	MaxLoopIterations = ABLi_popInteger() + 1;
}

//---------------------------------------------------------------------------

void execStdFatal (void) {

	//----------------------------------------------------------------------
	//
	//	FATAL function:
	//
	//		If the debugger is active, this immediately jumps into debug mode.
	//		Otherwise, it causes a fatal and exits the game (displaying the
	//		string passed in).
	//
	//		PARAMS:	integer							fatal code to display
	//
	//				char[]							message
	//
	//		RETURN: none
	//
	//----------------------------------------------------------------------

	int code = ABLi_popInteger();
	char* s = ABLi_popCharPtr();

	char message[512];
	if (debugger) {
		sprintf(message, "FATAL:  [%d] \"%s\"", code, s);
		debugger->print(message);
		sprintf(message, "   MODULE (%d) %s", CurModule->getId(), CurModule->getName());
		debugger->print(message);
		sprintf(message, "   FILE %s", CurModule->getSourceFile(FileNumber));
		debugger->print(message);
		sprintf(message, "   LINE %d", execLineNumber);
		debugger->print(message);
		debugger->debugMode();
		}
	else {
		sprintf(message, "ABL FATAL: [%d] %s", code, s);
		ABL_Fatal(0, s);
	}
}

//---------------------------------------------------------------------------

void execStdAssert (void) {

	//----------------------------------------------------------------------
	//
	//	ASSERT function:
	//
	//		If the debugger is active, this immediately jumps into debug mode
	//		if expression is FALSE. Otherwise, the assert statement is ignored
	//		unless the #debug directive has been issued in the module. If
	//		so, a fatal occurs and exits the game (displaying the
	//		string passed in).
	//
	//		PARAMS:	boolean							expression
	//
	//				integer							assert code to display
	//
	//				char[]							message
	//
	//		RETURN: none
	//
	//----------------------------------------------------------------------

	int expression = ABLi_popInteger();
	int code = ABLi_popInteger();
	char* s = ABLi_popCharPtr();

	if (!expression) {
		char message[512];
		if (debugger) {
			sprintf(message, "ASSERT:  [%d] \"%s\"", code, s);
			debugger->print(message);
			sprintf(message, "   MODULE (%d) %s", CurModule->getId(), CurModule->getName());
			debugger->print(message);
			sprintf(message, "   FILE %s", CurModule->getSourceFile(FileNumber));
			debugger->print(message);
			sprintf(message, "   LINE %d", execLineNumber);
			debugger->print(message);
			debugger->debugMode();
			}
		else {
			sprintf(message, "ABL ASSERT: [%d] %s", code, s);
			ABL_Fatal(0, message);
		}
	}
}

//-----------------------------------------------------------------------------

void execStdRandom (void) {

	long n = ABLi_peekInteger();
	//---------------------------------------------------------------------
	// This is, like, a really bad number generator. But, you get the idea.
	// Once we know which pseudo-random number algorithm we want to use, we
	// should plug it in here. The range for the random number (r), given a
	// param of (n), should be: 0 <= r <= (n-1).
	ABLi_pokeInteger(ABLRandomCallback(n));
}

//-----------------------------------------------------------------------------

void execStdSeedRandom (void) {

	long seed  = ABLi_popInteger();

	if (seed == -1)
		ABLSeedRandomCallback((unsigned int)time(NULL));
	else
		ABLSeedRandomCallback((unsigned int)seed);
}

//-----------------------------------------------------------------------------

void execStdResetOrders (void) {

	long scope = ABLi_popInteger();

	if (scope == 0)
		CurModule->resetOrderCallFlags();
	else if (scope == 1) {
		long startIndex = CurRoutineIdPtr->defn.info.routine.orderCallIndex;
		long endIndex = startIndex + CurRoutineIdPtr->defn.info.routine.numOrderCalls;
		for (long i = startIndex; i < endIndex; i++) {
			unsigned char orderDWord = (unsigned char)(i / 32);
			unsigned char orderBitMask = (unsigned char)(i % 32);
			CurModule->clearOrderCallFlag(orderDWord, orderBitMask);
		}
	}
}

//---------------------------------------------------------------------------

void execStdGetStateHandle (void) {

	char* name = ABLi_popCharPtr();

	long stateHandle = CurFSM->findStateHandle(_strlwr(name));
	ABLi_pushInteger(stateHandle);
}

//---------------------------------------------------------------------------

void execStdGetCurrentStateHandle (void) {

	long stateHandle = CurFSM->getStateHandle();
	ABLi_pushInteger(stateHandle);
}

//---------------------------------------------------------------------------

extern ModuleEntryPtr ModuleRegistry;

extern char	SetStateDebugStr[256];

void execStdSetState (void) {

	unsigned long stateHandle = ABLi_popInteger();

	if (stateHandle > 0) {
		SymTableNodePtr stateFunction = ModuleRegistry[CurFSM->getHandle()].stateHandles[stateHandle].state;
		CurFSM->setPrevState(CurFSM->getState());
		CurFSM->setState(stateFunction);
		sprintf(SetStateDebugStr, "%s:%s, line %d", CurFSM->getFileName(), stateFunction->name, execLineNumber);
		NewStateSet = true;
	}
}

//---------------------------------------------------------------------------

void execStdGetFunctionHandle (void) {

	char* name = ABLi_popCharPtr();

	SymTableNodePtr function = CurModule->findFunction(name, false);
	if (function)
		ABLi_pushInteger((unsigned long)function);
	else
		ABLi_pushInteger(0);
}

//---------------------------------------------------------------------------

void execStdSetFlag (void) {

	unsigned long bits = (unsigned long)ABLi_popInteger();
	unsigned long flag = (unsigned long)ABLi_popInteger();
	bool set = ABLi_popBoolean();

	bits &= (flag ^ 0xFFFFFFFF);
	if (set)
		bits |= flag;

	ABLi_pushInteger(bits);
}

//---------------------------------------------------------------------------

void execStdGetFlag (void) {

	unsigned long bits = (unsigned long)ABLi_popInteger();
	unsigned long flag = (unsigned long)ABLi_popInteger();

	bool set = ((bits & flag) != 0);

	ABLi_pushInteger(set);
}

//---------------------------------------------------------------------------

void execStdCallFunction (void) {

	unsigned long address = ABLi_popInteger();

	if (address) {
//GLENN: Not functional, yet...
	}
}

//***************************************************************************

void initStandardRoutines (void) {

	//-------------------------------------------------------------
	// Fatal and Assert will have hardcoded keys so we can look for
	// 'em in the rest of the ABL code (example: ignore asserts if
	// the assert_off option has been set).
	enterStandardRoutine("fatal", RTN_FATAL, false, "iC", NULL, execStdFatal);
	enterStandardRoutine("assert", RTN_ASSERT, false, "biC", NULL, execStdAssert);
	enterStandardRoutine("getstatehandle", RTN_GET_STATE_HANDLE, false, "C", "i", execStdGetStateHandle);

	enterStandardRoutine("getcurrentstatehandle", -1, false, NULL, "i", execStdGetCurrentStateHandle);
	enterStandardRoutine("abs", -1, false, "*", "r", execStdAbs);
	enterStandardRoutine("sqrt", -1, false, "*", "r", execStdSqrt);
	enterStandardRoutine("round", -1, false, "r", "i", execStdRound);
	enterStandardRoutine("trunc", -1, false, "r", "i", execStdTrunc);
	enterStandardRoutine("random", -1, false, "i", "i", execStdRandom);
	enterStandardRoutine("seedrandom", -1, false, "i", NULL, execStdSeedRandom);
	enterStandardRoutine("setmaxloops", -1, false, "i", NULL, execStdSetMaxLoops);
	enterStandardRoutine("fileopen", -1, false, "C", "i", execStdFileOpen);
	enterStandardRoutine("filewrite", -1, false, "iC", NULL, execStdFileWrite);
	enterStandardRoutine("fileclose", -1, false, "i", NULL, execStdFileClose);
	enterStandardRoutine("getmodule", -1, false, "CC", "i", execStdGetModule);
	enterStandardRoutine("resetorders", -1, false, "i", NULL, execStdResetOrders);
	enterStandardRoutine("setstate", -1, false, "i", NULL, execStdSetState);
	enterStandardRoutine("getfunctionhandle", -1, false, "C", "i", execStdGetFunctionHandle);
	enterStandardRoutine("callfunction", -1, false, "i", NULL, execStdCallFunction);
	enterStandardRoutine("setflag", -1, false, "iib", "i", execStdSetFlag);
	enterStandardRoutine("getflag", -1, false, "ii", "b", execStdGetFlag);

}

//-----------------------------------------------------------------------------

TypePtr execStandardRoutineCall (SymTableNodePtr routineIdPtr, bool skipOrder) {

	int key = routineIdPtr->defn.info.routine.key;
	switch (key) {
		case RTN_RETURN:
			execStdReturn();
			return(NULL);
		case RTN_PRINT:
			execStdPrint();
			return(NULL);
		case RTN_CONCAT:
			return(execStdConcat());
		default: {
			if (key >= NumStandardFunctions) {
				char err[255];
				sprintf(err, " ABL: Undefined ABL RoutineKey in %s:%d", CurModule->getName(), execLineNumber);
				ABL_Fatal(0, err);
			}
			if (FunctionInfoTable[key].numParams > 0)
				getCodeToken();
			SkipOrder = skipOrder;
			if (FunctionCallbackTable[key])
				(*FunctionCallbackTable[key])();
			else
			{
				char err[255];
				sprintf(err, " ABL: Undefined ABL RoutineKey %d in %s:%d", key, CurModule->getName(), execLineNumber);
				ABL_Fatal(key,err);
			}

			getCodeToken();
			switch (FunctionInfoTable[key].returnType) {
				case RETURN_TYPE_NONE:
					return(NULL);
				case RETURN_TYPE_INTEGER:
					return(IntegerTypePtr);
				case RETURN_TYPE_REAL:
					return(RealTypePtr);
				case RETURN_TYPE_BOOLEAN:
					return(BooleanTypePtr);
                default:;
			}
		}
	}
	return(NULL);
}

//***************************************************************************
