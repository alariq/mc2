//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								PARSER.H
//
//***************************************************************************

#ifndef ABLPARSE_H
#define	ABLPARSE_H

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLSYMT_H
#include"ablsymt.h"
#endif

#ifndef ABLERR_H
#include"ablerr.h"
#endif

//***************************************************************************

typedef enum {
	USE_EXPR,
	USE_TARGET,
	USE_REFPARAM
} UseType;

typedef enum {
	BLOCK_MODULE,
	BLOCK_ROUTINE
} BlockType;

//***************************************************************************

//----------
// FUNCTIONS
TypePtr expression (void);
TypePtr variable (SymTableNodePtr variableIdPtr);
TypePtr arraySubscriptList (TypePtr typePtr);
//TypePtr routineCall (SymTableNodePtr routineIdPtr, BOOL parmCheckFlag);
void checkRelationalOpTypes (TypePtr type1, TypePtr type2);
long isAssignTypeCompatible (TypePtr type1, TypePtr type2);
void ifTokenGet (TokenCodeType tokenCode);
void ifTokenGetElseError (TokenCodeType tokenCode, SyntaxErrorType errCode);

// DECL routines
void declarations (SymTableNodePtr routineIdPtr, bool allowFunctions);
void constDefinitions (void);
void doConst (SymTableNodePtr constantIdPtr);
void varDeclarations (SymTableNodePtr routineIdPtr);
void varOrFieldDeclarations (SymTableNodePtr routineIdPtr, long offset);
void typeDefinitions (void);
TypePtr doType (void);
TypePtr identifierType (SymTableNodePtr idPtr);
TypePtr enumerationType (void);
TypePtr subrangeType (void);
TypePtr arrayType (void);
long arraySize (TypePtr typePtr);
TypePtr makeStringType (long length);

// ROUTINE functions
void module (void);
SymTableNodePtr moduleHeader (void);
void routine (void);
SymTableNodePtr functionHeader (void);
SymTableNodePtr formalParamList (long* count, long* totalSize);
TypePtr routineCall (SymTableNodePtr routineIdPtr, long paramCheckFlag);
TypePtr declaredRoutineCall (SymTableNodePtr routineIdPtr, long paramCheckFlag);
void actualParamList (SymTableNodePtr routineIdPtr, long paramCheckFlag);
void block (SymTableNodePtr routineIdPtr);

// STATEMNT routines
void compoundStatement (void);
void assignmentStatement (SymTableNodePtr varIdPtr);
void repeatStatement (void);
void whileStatement (void);
void ifStatement (void);
void forStatement (void);
void switchStatement (void);
void transStatement (void);
void statement (void);

// STANDARD routines
void stdPrint (void);
TypePtr stdAbs (void);
TypePtr stdRound (void);
TypePtr stdTrunc (void);
TypePtr stdSqrt (void);
TypePtr stdRandom (void);
TypePtr standardRoutineCall (SymTableNodePtr routineIdPtr);

// FILE routines
long openSourceFile (const char* sourceFileName);
long closeSourceFile (void);

#if !ANALYZE_ON
#define	analyzeConstDefn(idPtr)
#define	analyzeVarDecl(idPtr)
#define	analyzeTypeDefn(idPtr)
#define	analyzeRoutineHeader(idPtr)
#define	analyzeBlock(idPtr)
#endif

//***************************************************************************

#endif

