//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								SYMTABLE.H
//
//***************************************************************************

#ifndef ABLSYMT_H
#define ABLSYMT_H

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLSCAN_H
#include"ablscan.h"
#endif

#ifndef DABLENV_H
#include"dablenv.h"
#endif

//***************************************************************************

//---------------------
// DEFINITION structure

typedef union {
	int         integer;
	char		character;
	float		real;
	char*		stringPtr;
} Value;

typedef enum {
	VAR_TYPE_NORMAL,
	VAR_TYPE_STATIC,
	VAR_TYPE_ETERNAL,
	VAR_TYPE_REGISTERED
} VariableType;

typedef enum {
	DFN_UNDEFINED,
	DFN_CONST,
	DFN_TYPE,
	DFN_VAR,
	DFN_VALPARAM,
	DFN_REFPARAM,
	DFN_MODULE,
	DFN_PROCEDURE,
	DFN_FUNCTION
} DefinitionType;

typedef enum {
	RTN_DECLARED,
	RTN_FORWARD,
	RTN_RETURN,
	RTN_PRINT,
	RTN_CONCAT,
	RTN_GET_STATE_HANDLE,
	RTN_ASSERT,
	RTN_FATAL,
	NUM_ABL_ROUTINES
} RoutineKey;
// NOTE: NUMBER OF ROUTINEKEYS IS LIMITED TO 256!

typedef struct _SymTableNode*	SymTableNodePtr;
typedef	struct _Type*			TypePtr;

typedef struct {
	Value					value;
} Constant;

typedef struct {
	char					name[128];
	SymTableNodePtr			state;
} StateHandleInfo;

typedef StateHandleInfo* StateHandleInfoPtr;

#define	ROUTINE_FLAG_ORDER	1
#define	ROUTINE_FLAG_FSM	2
#define	ROUTINE_FLAG_STATE	4

typedef struct _Routine {
	RoutineKey				key;
	unsigned char			flags;
	unsigned short			orderCallIndex;
	unsigned short			numOrderCalls;
	unsigned char			paramCount;
	unsigned char			totalParamSize;
	unsigned short			totalLocalSize;
	SymTableNodePtr			params;
	SymTableNodePtr			locals;
	SymTableNodePtr			localSymTable;
	char*					codeSegment;
	long					codeSegmentSize;
} Routine;

typedef struct {
	VariableType			varType;
	long					offset;
	void*					registeredData;
	//SymTableNodePtr			recordIdPtr;		// Currently not implementing record structures...
} Data;

typedef union {
	Constant				constant;
	Routine					routine;
	Data					data;
} DefinitionInfo;

typedef struct {
	DefinitionType			key;
	DefinitionInfo			info;
} Definition;

//***************************************************************************

//------------------
// SYMBOL TABLE node	

typedef struct _SymTableNode {
	SymTableNodePtr		left;
	SymTableNodePtr		parent;
	SymTableNodePtr		right;
	SymTableNodePtr		next;
	char*				name;
	char*				info;
	Definition			defn;
	TypePtr				typePtr;
	ABLModulePtr		library;
	unsigned char		level;
	long				labelIndex;		// really for compiling only...
} SymTableNode;

typedef enum {
	PARAM_TYPE_ANYTHING,
	PARAM_TYPE_CHAR,
	PARAM_TYPE_INTEGER,
	PARAM_TYPE_REAL,
	PARAM_TYPE_BOOLEAN,
	PARAM_TYPE_INTEGER_REAL,
	PARAM_TYPE_CHAR_ARRAY,
	PARAM_TYPE_INTEGER_ARRAY,
	PARAM_TYPE_REAL_ARRAY,
	PARAM_TYPE_BOOLEAN_ARRAY,
	NUM_PARAM_TYPES
} FunctionParamType;

typedef enum {
	RETURN_TYPE_NONE,
	RETURN_TYPE_INTEGER,
	RETURN_TYPE_REAL,
	RETURN_TYPE_BOOLEAN,
	NUM_RETURN_TYPES
} FunctionReturnType;

#define	MAX_STANDARD_FUNCTIONS	256
#define	MAX_FUNCTION_PARAMS		20

typedef struct {
	SymTableNodePtr			symbol;
	long					numParams;
	FunctionParamType		params[MAX_FUNCTION_PARAMS];
	FunctionReturnType		returnType;
} StandardFunctionInfo;

//***************************************************************************

//---------------
// TYPE structure

typedef enum {
	FRM_NONE,
	FRM_SCALAR,
	FRM_ENUM,
	FRM_ARRAY
	//FRM_RECORD
} FormType;

//---------------------------------------------------------------------
// Currently, we are only supporting the basic types: arrays and simple
// variables.

typedef struct _Type {
	long							numInstances;
	FormType						form;
	long							size;
	SymTableNodePtr					typeIdPtr;
	union {
		struct {
			SymTableNodePtr			constIdPtr;
			long					max;
		} enumeration;
		struct {
			TypePtr					indexTypePtr;		// should be Integer
			TypePtr					elementTypePtr;		// should be Real, Integer, Char or array
			long					elementCount;
		} array;
		// Not currently implementing record structures...
		//struct {
		//	SymTableNodePtr		fieldSymTable;
		//} record;
	} info;
} Type;	


//***************************************************************************

void searchLocalSymTable (SymTableNodePtr& IdPtr);
SymTableNodePtr searchLocalSymTableForFunction (const char* name);
void searchThisSymTable (SymTableNodePtr& IdPtr, SymTableNodePtr thisTable);
void searchAllSymTables (SymTableNodePtr& IdPtr);
void enterLocalSymTable (SymTableNodePtr& IdPtr);
void enterNameLocalSymTable (SymTableNodePtr& IdPtr, const char* name);
void searchAndFindAllSymTables (SymTableNodePtr& IdPtr);
void searchAndEnterLocalSymTable (SymTableNodePtr& IdPtr);
/*inline*/ void searchAndEnterThisTable (SymTableNodePtr& IdPtr, SymTableNodePtr thisTable);
inline SymTableNodePtr symTableSuccessor (SymTableNodePtr nodeX);

SymTableNodePtr searchSymTable (const char* name, SymTableNodePtr nodePtr);
SymTableNodePtr searchSymTableForFunction (const char* name, SymTableNodePtr nodePtr);
SymTableNodePtr searchSymTableForState (const char* name, SymTableNodePtr nodePtr);
SymTableNodePtr searchSymTableForString (const char* name, SymTableNodePtr nodePtr);
SymTableNodePtr searchSymTableDisplay (const char* name);
SymTableNodePtr enterSymTable (const char* name, SymTableNodePtr* ptrToNodePtr);
SymTableNodePtr insertSymTable (SymTableNodePtr* tableRoot, SymTableNodePtr newNode);
SymTableNodePtr extractSymTable (SymTableNodePtr* tableRoot, SymTableNodePtr nodeKill);
void enterStandardRoutine (const char* name, long routineKey, bool isOrder, const char* paramList, const char* returnType, void (*callback)(void));
void enterScope (SymTableNodePtr symTableRoot);
SymTableNodePtr exitScope (void);
void initSymTable (void);

TypePtr createType (void);
TypePtr setType (TypePtr type);
void clearType (TypePtr& type);

//***************************************************************************

extern StandardFunctionInfo		FunctionInfoTable[MAX_STANDARD_FUNCTIONS];
//extern void*					FunctionCallbackTable[MAX_STANDARD_FUNCTIONS];
extern void						(*FunctionCallbackTable[MAX_STANDARD_FUNCTIONS])(void);
extern long						NumStandardFunctions;

#endif

