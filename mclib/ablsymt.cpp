//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLSYMT.CPP
//
//***************************************************************************

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLERR_H
#include"ablerr.h"
#endif

#ifndef ABLSYMT_H
#include"ablsymt.h"
#endif

#ifndef ABLSCAN_H
#include"ablscan.h"
#endif

#ifndef ABLENV_H
#include"ablenv.h"
#endif

//***************************************************************************

//----------
// EXTERNALS

extern int32_t      level;		// current nesting/scope level

//--------
// GLOBALS

SymTableNodePtr		SymTableDisplay[MAX_NESTING_LEVEL];

ABLModulePtr		LibrariesUsed[MAX_LIBRARIES_USED];
long				NumLibrariesUsed = 0;

//------------------
// Pre-defined types
TypePtr				IntegerTypePtr;
TypePtr				CharTypePtr;
TypePtr				RealTypePtr;
TypePtr				BooleanTypePtr;

Type DummyType = {		// for erroneous type definitions
	0,
	FRM_NONE,
	0,
	NULL
};

StandardFunctionInfo		FunctionInfoTable[MAX_STANDARD_FUNCTIONS];
//void*						FunctionCallbackTable[MAX_STANDARD_FUNCTIONS];
void						(*FunctionCallbackTable[MAX_STANDARD_FUNCTIONS])(void);
long						NumStandardFunctions = NUM_ABL_ROUTINES;

void execStdRandom (void);

//***************************************************************************
// MISC. (initially were macros)
//***************************************************************************

void searchLocalSymTable (SymTableNodePtr& IdPtr) {

	IdPtr = searchSymTable(wordString, SymTableDisplay[level]);
}

//***************************************************************************

inline void searchThisSymTable (SymTableNodePtr& IdPtr, SymTableNodePtr thisTable) {

	IdPtr = searchSymTable(wordString, thisTable);
}

//***************************************************************************

void searchAllSymTables (SymTableNodePtr& IdPtr) {

	IdPtr = searchSymTableDisplay(wordString);
}

//***************************************************************************

void enterLocalSymTable (SymTableNodePtr& IdPtr) {

	IdPtr = enterSymTable(wordString, &SymTableDisplay[level]);
}

//***************************************************************************

inline void enterNameLocalSymTable (SymTableNodePtr& IdPtr, const char* name) {

	IdPtr = enterSymTable(name, &SymTableDisplay[level]);
}

//***************************************************************************

void searchAndFindAllSymTables (SymTableNodePtr& IdPtr) {

	if ((IdPtr = searchSymTableDisplay(wordString)) == NULL) {
		syntaxError(ABL_ERR_SYNTAX_UNDEFINED_IDENTIFIER);
		IdPtr = enterSymTable(wordString, &SymTableDisplay[level]);
		IdPtr->defn.key = DFN_UNDEFINED;
		IdPtr->typePtr = &DummyType;
	}
}

//***************************************************************************

void searchAndEnterLocalSymTable (SymTableNodePtr& IdPtr) {

	if ((IdPtr = searchSymTable(wordString, SymTableDisplay[level])) == NULL)
		IdPtr = enterSymTable(wordString, &SymTableDisplay[level]);
	else
		syntaxError(ABL_ERR_SYNTAX_REDEFINED_IDENTIFIER);
}

//***************************************************************************

void searchAndEnterThisTable (SymTableNodePtr& IdPtr, SymTableNodePtr thisTable) {

	if ((IdPtr = searchSymTable(wordString, thisTable)) == NULL)
		IdPtr = enterSymTable(wordString, &thisTable);
	else
		syntaxError(ABL_ERR_SYNTAX_REDEFINED_IDENTIFIER);
}

//***************************************************************************

inline SymTableNodePtr symTableSuccessor (SymTableNodePtr nodeX) {

	if (nodeX->right) {
		// return the treeMin...
		while (nodeX->left)
			nodeX = nodeX->left;
		return(nodeX);
	}
	
	SymTableNodePtr nodeY = nodeX->parent;
	while (nodeY && (nodeX == nodeY->right)) {
		nodeX = nodeY;
		nodeY = nodeY->parent;
	}
	return(nodeY);
}

//***************************************************************************
// TYPE management routines
//***************************************************************************

TypePtr createType (void) {

	TypePtr newType = (TypePtr)ABLSymbolMallocCallback(sizeof(Type));
	if (!newType)
		ABL_Fatal(0, " ABL: Unable to AblStackHeap->malloc newType ");
	newType->numInstances = 1;
	newType->form = FRM_NONE;
	newType->size = 0;
	newType->typeIdPtr = NULL;

	return(newType);
}

//---------------------------------------------------------------------------

TypePtr setType (TypePtr type) {

	if (type)
		type->numInstances++;

	return(type);
}

//---------------------------------------------------------------------------

void clearType (TypePtr& type) {

	if (type) {
		type->numInstances--;
		if (type->numInstances == 0) {
			ABLSymbolFreeCallback(type);
			type = NULL;
		}
	}
}

//***************************************************************************
// SYMBOL TABLE routines
//***************************************************************************

void recordLibraryUsed (SymTableNodePtr memberNodePtr) {

	//------------------------------------------------------------------
	// If the library already on our list, then don't bother. Otherwise,
	// add it to our list...
	ABLModulePtr library = memberNodePtr->library;
	for (long i = 0; i < NumLibrariesUsed; i++)
		if (LibrariesUsed[i] == library)
			return;

	//---------------------------------------------------
	// New library, so record it if we still have room...
	if (NumLibrariesUsed > MAX_LIBRARIES_USED)
		ABL_Fatal(0, " ABL: Too many libraries referenced from module ");

	LibrariesUsed[NumLibrariesUsed++] = library;
}

//***************************************************************************

SymTableNodePtr searchSymTable (const char* name, SymTableNodePtr nodePtr) {

	while (nodePtr) {
		long compareResult = strcmp(name, nodePtr->name);
		if (compareResult == 0)
			return(nodePtr);
		if (compareResult < 0)
			nodePtr = nodePtr->left;
		else
			nodePtr = nodePtr->right;
	}
	return(NULL);
}

//***************************************************************************

SymTableNodePtr searchSymTableForFunction (const char* name, SymTableNodePtr nodePtr) {

	while (nodePtr) {
		long compareResult = strcmp(name, nodePtr->name);
		if (compareResult == 0) {
			if (nodePtr->typePtr == NULL)
				if (nodePtr->defn.key == DFN_FUNCTION)
					return(nodePtr);
			nodePtr = nodePtr->right;
			}
		else if (compareResult < 0)
			nodePtr = nodePtr->left;
		else
			nodePtr = nodePtr->right;
	}
	return(NULL);
}

//***************************************************************************

SymTableNodePtr searchSymTableForState (const char* name, SymTableNodePtr nodePtr) {

	while (nodePtr) {
		long compareResult = strcmp(name, nodePtr->name);
		if (compareResult == 0) {
			if (nodePtr->typePtr == NULL)
				if (nodePtr->defn.key == DFN_FUNCTION)
					if (nodePtr->defn.info.routine.flags & ROUTINE_FLAG_STATE)
						return(nodePtr);
			nodePtr = nodePtr->right;
			}
		else if (compareResult < 0)
			nodePtr = nodePtr->left;
		else
			nodePtr = nodePtr->right;
	}
	return(NULL);
}

//***************************************************************************

SymTableNodePtr searchSymTableForString (const char* name, SymTableNodePtr nodePtr) {

	while (nodePtr) {
		long compareResult = strcmp(name, nodePtr->name);
		if (compareResult == 0) {
			if (nodePtr->typePtr)
				if (nodePtr->typePtr->form == FRM_ARRAY)
					if (nodePtr->typePtr->info.array.elementTypePtr == CharTypePtr)
						return(nodePtr);
			nodePtr = nodePtr->right;
			}
		else if (compareResult < 0)
			nodePtr = nodePtr->left;
		else
			nodePtr = nodePtr->right;
	}
	return(NULL);
}

//***************************************************************************

SymTableNodePtr searchLibrarySymTable (const char* name, SymTableNodePtr nodePtr) {

	//-------------------------------------------------------------
	// Since all libraries are at the symbol display 0-level, we'll
	// check the local symbol table of all libraries. WARNING: This
	// will find the FIRST instance of a symbol with that name,
	// so don't load two libraries with a similarly named function
	// or variable, otherwise you may not get the one you want
	// unless you explicitly reference the library you want
	// (e.g. testLib.fudge, rather than just fudge). This is WAY
	// inefficient compared to simply knowing the library we want,
	// so any ABL programmer that causes this function to be called
	// should be shot --gd 9/29/97

	if (nodePtr) {
		long compareResult = strcmp(name, nodePtr->name);
		if (compareResult == 0)
			return(nodePtr);
		else {
			if (nodePtr->library && (nodePtr->defn.key == DFN_MODULE)) {
				SymTableNodePtr memberNodePtr = searchSymTable(name, nodePtr->defn.info.routine.localSymTable);
				if (memberNodePtr)
					return(memberNodePtr);
			}
			SymTableNodePtr nodeFoundPtr = searchLibrarySymTable(name, nodePtr->left);
			if (nodeFoundPtr)
				return(nodeFoundPtr);
			nodeFoundPtr = searchLibrarySymTable(name, nodePtr->right);
			if (nodeFoundPtr)
				return(nodeFoundPtr);
		}
	}
	return(NULL);
}

//***************************************************************************

SymTableNodePtr searchLibrarySymTableDisplay (const char* name) {

	SymTableNodePtr nodePtr = searchLibrarySymTable(name, SymTableDisplay[0]);
	return(nodePtr);
}

//***************************************************************************

SymTableNodePtr searchSymTableDisplay (const char* name) {

    char* dup_name = strdup(name);
	//---------------------------------------------------------------------
	// First check if this is an explicit library reference. If so, we need
	// to determine which library and which identifier in that library...
	char* separator = strchr(dup_name, '.');
	SymTableNodePtr nodePtr = NULL;

	if (separator) {
		*separator = '\0';
		SymTableNodePtr libraryNodePtr = searchSymTable(dup_name, SymTableDisplay[0]);
		if (!libraryNodePtr)
			return(NULL);
		//-------------------------------------
		// Now, search for the member symbol...
		char* memberName = separator + 1;
		SymTableNodePtr memberNodePtr = searchSymTable(memberName, libraryNodePtr->defn.info.routine.localSymTable);
		if (memberNodePtr)
			recordLibraryUsed(memberNodePtr);
		return(memberNodePtr);
		}
	else {	
		for (long i = level; i >= 0; i--) {
			SymTableNodePtr nodePtr = searchSymTable(dup_name, SymTableDisplay[i]);
			if (nodePtr)
            {
                free(dup_name);
				return(nodePtr);
            }
		}
		//------------------------------------------------------------
		// We haven't found it, so maybe it's from a library but just
		// is not explicitly called with the library name. Since all
		// libraries are at the symbol table's 0-level, we'll check
		// the local symbol table of all libraries. WARNING: This
		// will find the FIRST instance of a symbol with that name,
		// so don't load two libraries with a similarly named function
		// or variable, otherwise you may not get the one you want
		// unless you explicitly reference the library you want
		// (e.g. testLib.fudge, rather than just fudge)...
		nodePtr = searchLibrarySymTableDisplay(dup_name);
		if (nodePtr)
			recordLibraryUsed(nodePtr);
	}

    free(dup_name);
	return(nodePtr);
}

//***************************************************************************

SymTableNodePtr enterSymTable (const char* name, SymTableNodePtr* ptrToNodePtr) {

	//-------------------------------------
	// First, create the new symbol node...
	SymTableNodePtr newNode = (SymTableNodePtr)ABLSymbolMallocCallback(sizeof(SymTableNode));
	if (!newNode)
		ABL_Fatal(0, " ABL: Unable to AblSymTableHeap->malloc symbol ");

	newNode->name = (char*)ABLSymbolMallocCallback(strlen(name) + 1);
	if (!newNode->name)
		ABL_Fatal(0, " ABL: Unable to AblSymTableHeap->malloc symbol name ");
	strcpy(newNode->name, name);
	newNode->left = NULL;
	newNode->parent = NULL;
	newNode->right = NULL;
	newNode->next = NULL;
	newNode->info = NULL;
	newNode->defn.key = DFN_UNDEFINED;
	newNode->defn.info.data.varType = VAR_TYPE_NORMAL;
	newNode->defn.info.data.offset = 0;
	newNode->typePtr = NULL;
	newNode->level = (unsigned char)level;
	newNode->labelIndex = 0;

	//-------------------------------------
	// Find where to put this new symbol...
	SymTableNodePtr curNode = *ptrToNodePtr;
	SymTableNodePtr parentNode = NULL;
	while (curNode) {
		if (strcmp(name, curNode->name) < 0)
			ptrToNodePtr = &(curNode->left);
		else
			ptrToNodePtr = &(curNode->right);
		parentNode = curNode;
		curNode = *ptrToNodePtr;
	}
	newNode->parent = parentNode;
	*ptrToNodePtr = newNode;

	return(newNode);
}

//***************************************************************************

SymTableNodePtr insertSymTable (SymTableNodePtr* tableRoot, SymTableNodePtr newNode) {

	newNode->left = NULL;
	newNode->parent = NULL;
	newNode->right = NULL;

	//------------------------------------
	// Find where to insert this symbol...
	SymTableNodePtr curNode = *tableRoot;
	SymTableNodePtr parentNode = NULL;
	while (curNode) {
		if (strcmp(newNode->name, curNode->name) < 0)
			tableRoot = &(curNode->left);
		else
			tableRoot = &(curNode->right);
		parentNode = curNode;
		curNode = *tableRoot;
	}
	newNode->parent = parentNode;
	*tableRoot = newNode;

	return(newNode);
}

//***************************************************************************

SymTableNodePtr extractSymTable (SymTableNodePtr* tableRoot, SymTableNodePtr nodeKill) {

	//------------------------------------------------------------------------
	// NOTE: While this routine extracts a node from the symbol table,
	// it does not account for other nodes in the table that may be pointing
	// to the now-extracted node. Currently, this is not a problem as this
	// routine is really just used to extract the module's Identifier at level
	// 0 in the SymTable Display. Do we want to eliminate the use of the
	// parent pointer, and just hardcode something that may be more efficient
	// for this level-0 special case?

	SymTableNodePtr nodeX = NULL;
	SymTableNodePtr nodeY = NULL;

	if ((nodeKill->left == NULL) || (nodeKill->right == NULL))
		nodeY = nodeKill;
	else
		nodeY = symTableSuccessor(nodeKill);
	
	if (nodeY->left)
		nodeX = nodeY->left;
	else
		nodeX = nodeY->right;

	if (nodeX)
		nodeX->parent = nodeY->parent;

	if (nodeY->parent == NULL)
		*tableRoot = nodeX;
	else if (nodeY == nodeY->parent->left)
		nodeY->parent->left = nodeX;
	else
		nodeY->parent->right = nodeX;

	if (nodeY != nodeKill) {
		//--------------------------------------
		// Copy y data to nodeKill's position...
		nodeKill->next = nodeY->next;
		nodeKill->name = nodeY->name;
		nodeKill->info = nodeY->info;
		memcpy(&nodeKill->defn, &nodeY->defn, sizeof(Definition));
		nodeKill->typePtr = nodeY->typePtr;
		nodeKill->level = nodeY->level;
		nodeKill->labelIndex = nodeY->labelIndex;
	}

	return(nodeY);
}

//***************************************************************************

void enterStandardRoutine (const char* name, long routineKey, bool isOrder, const char* paramList, const char* returnType, void (*callback)(void)) {

	long tableIndex = routineKey;
	if (tableIndex == -1) {
		if (NumStandardFunctions == MAX_STANDARD_FUNCTIONS)
			ABL_Fatal(0, " ABL.enterStandardRoutine: Too Many Standard Functions ");
		tableIndex = NumStandardFunctions++;
	}

	SymTableNodePtr routineIdPtr;
	enterNameLocalSymTable(routineIdPtr, name);
	
	routineIdPtr->defn.key = DFN_FUNCTION;
	routineIdPtr->defn.info.routine.key = (RoutineKey)tableIndex;
	routineIdPtr->defn.info.routine.flags = isOrder ? ROUTINE_FLAG_ORDER : 0;
	routineIdPtr->defn.info.routine.params = NULL;
	routineIdPtr->defn.info.routine.localSymTable = NULL;
	routineIdPtr->library = NULL;
	routineIdPtr->typePtr = NULL;

	FunctionInfoTable[tableIndex].numParams = 0;
	if (paramList) {
		FunctionInfoTable[tableIndex].numParams = strlen(paramList);
		if (FunctionInfoTable[tableIndex].numParams > MAX_FUNCTION_PARAMS)
			ABL_Fatal(0, " ABL.enterStandardRoutine: Too Many Standard Function Params ");
		for (long i = 0; i < FunctionInfoTable[tableIndex].numParams; i++)
			switch (paramList[i]) {
				case '?':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_ANYTHING;
					break;
				case 'c':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_CHAR;
					break;
				case 'i':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_INTEGER;
					break;
				case 'r':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_REAL;
					break;
				case 'b':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_BOOLEAN;
					break;
				case '*':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_INTEGER_REAL;
					break;
				case 'C':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_CHAR_ARRAY;
					break;
				case 'I':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_INTEGER_ARRAY;
					break;
				case 'R':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_REAL_ARRAY;
					break;
				case 'B':
					FunctionInfoTable[tableIndex].params[i] = PARAM_TYPE_BOOLEAN_ARRAY;
					break;
				default: {
					char err[255];
					sprintf(err, " ABL.enterStandardRoutine: bad param type (%c) for (%s)", paramList[i], name);
					ABL_Fatal(0, err);
				}
			}
	}
	
	FunctionInfoTable[tableIndex].returnType = RETURN_TYPE_NONE;
	if (returnType)
		switch (returnType[0]) {
			//case 'c':
			//	FunctionInfoTable[NumStandardFunctions].returnType = RETURN_TYPE_CHAR;
			//	break;
			case 'i':
				FunctionInfoTable[tableIndex].returnType = RETURN_TYPE_INTEGER;
				break;
			case 'r':
				FunctionInfoTable[tableIndex].returnType = RETURN_TYPE_REAL;
				break;
			case 'b':
				FunctionInfoTable[tableIndex].returnType = RETURN_TYPE_BOOLEAN;
				break;
			default: {
				char err[255];
				sprintf(err, " ABL.enterStandardRoutine: bad return type for (%s)", name);
				ABL_Fatal(0, err);
			}
		}
	FunctionCallbackTable[tableIndex] = callback;
}

//***************************************************************************

void enterScope (SymTableNodePtr symTableRoot) {

	if (++level >= MAX_NESTING_LEVEL)
		syntaxError(ABL_ERR_SYNTAX_NESTING_TOO_DEEP);
	SymTableDisplay[level] = symTableRoot;
}

//***************************************************************************

SymTableNodePtr exitScope (void) {

	return(SymTableDisplay[level--]);
}
 
//***************************************************************************
void initStandardRoutines (void);

void initSymTable (void) {

	//---------------------------------
	// Init the level-0 symbol table...
	SymTableDisplay[0] = NULL;
	
	//----------------------------------------------------------------------
	// Set up the basic variable types as identifiers in the symbol table...
	SymTableNodePtr integerIdPtr;
	enterNameLocalSymTable(integerIdPtr, "integer");
	SymTableNodePtr charIdPtr;
	enterNameLocalSymTable(charIdPtr, "char");
	SymTableNodePtr realIdPtr;
	enterNameLocalSymTable(realIdPtr, "real");
	SymTableNodePtr booleanIdPtr;
	enterNameLocalSymTable(booleanIdPtr, "boolean");
	SymTableNodePtr falseIdPtr;
	enterNameLocalSymTable(falseIdPtr, "false");
	SymTableNodePtr trueIdPtr;
	enterNameLocalSymTable(trueIdPtr, "true");
	
	//------------------------------------------------------------------
	// Now, create the basic variable TYPEs, and point their identifiers
	// to their proper type definition...
	IntegerTypePtr = createType();
	if (!IntegerTypePtr)
		ABL_Fatal(0, " ABL: Unable to AblSymTableHeap->malloc Integer Type ");
	CharTypePtr = createType();
	if (!CharTypePtr)
		ABL_Fatal(0, " ABL: Unable to AblSymTableHeap->malloc Char Type ");
	RealTypePtr = createType();
	if (!RealTypePtr)
		ABL_Fatal(0, " ABL: Unable to AblSymTableHeap->malloc Real Type ");
	BooleanTypePtr = createType();
	if (!BooleanTypePtr)
		ABL_Fatal(0, " ABL: Unable to AblSymTableHeap->malloc Boolean Type ");

	integerIdPtr->defn.key = DFN_TYPE;
	integerIdPtr->typePtr = IntegerTypePtr;
	IntegerTypePtr->form = FRM_SCALAR;
	IntegerTypePtr->size = sizeof(int);
	IntegerTypePtr->typeIdPtr = integerIdPtr;

	charIdPtr->defn.key = DFN_TYPE;
	charIdPtr->typePtr = CharTypePtr;
	CharTypePtr->form = FRM_SCALAR;
	CharTypePtr->size = sizeof(char);
	CharTypePtr->typeIdPtr = charIdPtr;
	
	realIdPtr->defn.key = DFN_TYPE;
	realIdPtr->typePtr = RealTypePtr;
	RealTypePtr->form = FRM_SCALAR;
	RealTypePtr->size = sizeof(float);
	RealTypePtr->typeIdPtr = realIdPtr;

	booleanIdPtr->defn.key = DFN_TYPE;
	booleanIdPtr->typePtr = BooleanTypePtr;
	BooleanTypePtr->form = FRM_ENUM;
	BooleanTypePtr->size = sizeof(int);
	BooleanTypePtr->typeIdPtr = booleanIdPtr;

	//----------------------------------------------------
	// Set up the FALSE identifier for the boolean type...
	BooleanTypePtr->info.enumeration.max = 1;
	((TypePtr)(booleanIdPtr->typePtr))->info.enumeration.constIdPtr = falseIdPtr;
	falseIdPtr->defn.key = DFN_CONST;
	falseIdPtr->defn.info.constant.value.integer = 0;
	falseIdPtr->typePtr = BooleanTypePtr;		

	//----------------------------------------------------
	// Set up the TRUE identifier for the boolean type...
	falseIdPtr->next = trueIdPtr;
	trueIdPtr->defn.key = DFN_CONST;
	trueIdPtr->defn.info.constant.value.integer = 1;
	trueIdPtr->typePtr = BooleanTypePtr;		

	//-------------------------------------------
	// Set up the standard, built-in functions...
//(char* name, long routineKey, bool isOrder, char* paramList, char* returnType, void* callback);
	enterStandardRoutine("return", RTN_RETURN, false, NULL, NULL, NULL);
	enterStandardRoutine("print", RTN_PRINT, false, NULL, NULL, NULL);
	enterStandardRoutine("concat", RTN_CONCAT, false, NULL, NULL, NULL);
	enterStandardRoutine("getstatehandle", RTN_GET_STATE_HANDLE, false, NULL, NULL, NULL);

	//-----------------------------------
	// Honor Bound-specific extensions...
	//-----------------------------------

	initStandardRoutines();
}

//***************************************************************************

