//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								EXECUTOR.H
//
//***************************************************************************

#ifndef ABLEXEC_H
#define ABLEXEC_H

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLSYMT_H
#include"ablsymt.h"
#endif

#ifndef ABLPARS_H
#include"ablparse.h"
#endif

//***************************************************************************

#define	STATEMENT_MARKER	0x70
#define	ADDRESS_MARKER		0x71

//***************************************************************************

//--------------
// ABL Parameter

#define	ABL_PARAM_VOID			0
#define	ABL_PARAM_INTEGER		1
#define	ABL_PARAM_REAL			2

typedef enum {
	ABL_STACKITEM_CHAR,
	ABL_STACKITEM_INTEGER,
	ABL_STACKITEM_REAL,
	ABL_STACKITEM_BOOLEAN,
	ABL_STACKITEM_CHAR_PTR,
	ABL_STACKITEM_INTEGER_PTR,
	ABL_STACKITEM_REAL_PTR,
	ABL_STACKITEM_BOOLEAN_PTR,
	NUM_ABL_STACKITEMS
} ABLStackItemType;

typedef struct {
	char		type;
	int         integer;
	float		real;
} ABLParam;

typedef ABLParam* ABLParamPtr;

//---------------
// RUN-TIME STACK

typedef union {
	int             integer;
	float			real;
	unsigned char	byte;
	Address			address;
} StackItem;

typedef StackItem* StackItemPtr;

typedef struct {
	long			type;
	union {
		long		integer;
		float		real;
		bool		boolean;
		char		character;
		long*		integerPtr;
		float*		realPtr;
		bool*		booleanPtr;
		char*		characterPtr;
	} data;
} ABLStackItem;

typedef ABLStackItem* ABLStackItemPtr;

typedef struct {
	StackItem	functionValue;
	StackItem	staticLink;
	StackItem	dynamicLink;
	StackItem	returnAddress;
} StackFrameHeader;

typedef StackFrameHeader* StackFrameHeaderPtr;

//***************************************************************************

#if 0

//class ABLmodule {

//	public:

//		SymTableNodePtr		moduleIdPtr;


//	public:


//};

#endif

//***************************************************************************

extern char*			codeBuffer;
extern char*			codeBufferPtr;
extern char*			codeSegmentPtr;
extern char*			codeSegmentLimit;
extern char*			statementStartPtr;

extern TokenCodeType	codeToken;
extern int              execLineNumber;
extern int              execStatementCount;

extern StackItem*		stack;
extern StackItemPtr		tos;
extern StackItemPtr		stackFrameBasePtr;

//***************************************************************************

//----------
// FUNCTIONS

SymTableNodePtr getSymTableCodePtr (void);
TypePtr execRoutineCall (void);
TypePtr execExpression (void);
TypePtr execVariable (void);

//*************************
// CRUNCH/DECRUNCH routines
//*************************

void crunchToken (void);
void crunchSymTableNodePtr (SymTableNodePtr nodePtr);
void crunchStatementMarker (void);
void uncrunchStatementMarker (void);
char* crunchAddressMarker (Address address);
char* fixupAddressMarker (Address address);
void crunchInteger (long value);
void crunchByte (unsigned char value);
void crunchOffset (Address address);
char* createCodeSegment (long& codeSegmentSize);
SymTableNodePtr getCodeSymTableNodePtr (void);
long getCodeStatementMarker (void);
char* getCodeAddressMarker (void);
long getCodeInteger (void);
unsigned char getCodeByte (void);
char* getCodeAddress (void);

//***************
// STACK routines
//***************

void pop (void);
void getCodeToken (void);
void pushInteger (long value);
void pushReal (float value);
void pushByte (char value);
void pushAddress (Address address);
void pushBoolean (bool value);
void pushStackFrameHeader (long oldLevel, long newLevel);
void allocLocal (TypePtr typePtr);
void freeData (SymTableNodePtr idPtr);

//*****************************
// FUNCTION ENTRY/EXIT routines
//*****************************

void routineEntry (SymTableNodePtr routineIdPtr);
void routineExit (SymTableNodePtr routineIdPtr);
void execute (SymTableNodePtr routineIdPtr);
void executeChild (SymTableNodePtr routineIdPtr, SymTableNodePtr childRoutineIdPtr);

//******************
// EXECSTMT routines
//******************

void execStatement (void);
void execAssignmentStatement (SymTableNodePtr idPtr);
TypePtr execRoutineCall (SymTableNodePtr routineIdPtr, bool skipOrder);
TypePtr execDeclaredRoutineCall (SymTableNodePtr routineIdPtr, bool skipOrder);
void execActualParams (SymTableNodePtr routineIdPtr);
void execCompoundStatement (void);
void execCaseStatement (void);
void execForStatement (void);
void execIfStatement (void);
void execRepeatStatement (void);
void execWhileStatement (void);
void execSwitchStatement (void);
void execTransStatement (void);
void execTransBackStatement (void);

//******************
// EXECEXPR routines
//******************

TypePtr execField (void);
TypePtr execSubscripts (TypePtr typePtr);
TypePtr execConstant (SymTableNodePtr idPtr);
TypePtr execVariable (SymTableNodePtr idPtr, UseType use);
TypePtr execFactor (void);
TypePtr execTerm (void);
TypePtr execSimpleExpression (void);
TypePtr execExpression (void);

//*****************
// EXECSTD routines
//*****************

TypePtr execStandardRoutineCall (SymTableNodePtr routineIdPtr, bool skipOrder);

//***************************************************************************

#endif
