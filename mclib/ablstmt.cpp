//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLSTMT.CPP
//
//***************************************************************************

#include<stdio.h>

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLERR_H
#include"ablerr.h"
#endif

#ifndef ABLSCAN_H
#include"ablscan.h"
#endif

#ifndef ABLSYMT_H
#include"ablsymt.h"
#endif

#ifndef ABLPARSE_H
#include"ablparse.h"
#endif

#ifndef ABLEXEC_H
#include"ablexec.h"
#endif

//***************************************************************************

extern TokenCodeType	curToken;
extern char				tokenString[];
extern char				wordString[];
extern Literal			curLiteral;
extern TokenCodeType	statementStartList[];
extern TokenCodeType	statementEndList[];
extern SymTableNodePtr	symTableDisplay[];
extern long				level;
extern char*			codeBuffer;
extern TypePtr			IntegerTypePtr;
extern TypePtr			RealTypePtr;
extern TypePtr			BooleanTypePtr;
extern TypePtr			CharTypePtr;
extern Type				DummyType;
extern SymTableNodePtr	CurRoutineIdPtr;
extern SymTableNodePtr	SymTableDisplay[MAX_NESTING_LEVEL];
extern bool				AssertEnabled;
extern bool				PrintEnabled;
extern bool				StringFunctionsEnabled;
extern bool				Crunch;
extern long				NumOrderCalls;

//--------
// GLOBALS
TokenCodeType	FollowSwitchExpressionList[] = {
					TKN_CASE,
					TKN_SEMICOLON,
					TKN_NONE
				};
TokenCodeType	FollowCaseLabelList[] = {
					TKN_COLON,
					TKN_SEMICOLON,
					TKN_NONE
				};
TokenCodeType	CaseLabelStartList[] = {
					TKN_IDENTIFIER,
					TKN_NUMBER,
					TKN_PLUS,
					TKN_MINUS,
					TKN_STRING,
					TKN_NONE
				};

SymTableNodePtr forwardState (char* stateName);

//***************************************************************************

void assignmentStatement (SymTableNodePtr varIdPtr) {

	//-----------------------------------
	// Grab the variable we're setting...
	TypePtr varType = variable(varIdPtr);
	ifTokenGetElseError(TKN_EQUAL, ABL_ERR_SYNTAX_MISSING_EQUAL);

	//---------------------------------------------------------
	// Now, get the expression we're setting the variable to...	
	TypePtr exprType = expression();
	
	//----------------------------------------
	// They better be assignment compatible...
	if (!isAssignTypeCompatible(varType, exprType))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_ASSIGNMENT);
}

//***************************************************************************

void repeatStatement (void) {

	getToken();
	
	if (curToken != TKN_UNTIL) {
		do {
			statement();
			while (curToken == TKN_SEMICOLON)
				getToken();
			if (curToken == TKN_UNTIL)
				break;
		} while (tokenIn(statementStartList));
	}
	
	ifTokenGetElseError(TKN_UNTIL, ABL_ERR_SYNTAX_MISSING_UNTIL);
	
	TypePtr exprType = expression();
	if (exprType != BooleanTypePtr)
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
}

//***************************************************************************

void whileStatement (void) {

	// NEW STYLE, using endwhile keyword...
	getToken();
	char* loopEndLocation = crunchAddressMarker(NULL);
	
	TypePtr exprType = expression();
	if (exprType != BooleanTypePtr)
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);

	//---------------------------------------
	// Let's not use a DO keyword, for now...
	ifTokenGetElseError(TKN_DO, ABL_ERR_SYNTAX_MISSING_DO);

	if (curToken != TKN_END_WHILE)
		do {
			statement();
			while (curToken == TKN_SEMICOLON)
				getToken();
			if (curToken == TKN_END_WHILE)
				break;
		} while (tokenIn(statementStartList));
	
	ifTokenGetElseError(TKN_END_WHILE, ABL_ERR_SYNTAX_MISSING_END_WHILE);

	fixupAddressMarker(loopEndLocation);
}

//***************************************************************************

void ifStatement (void) {

	getToken();
	char* falseLocation = crunchAddressMarker(NULL);
	
	TypePtr exprType = expression();
	if (exprType != BooleanTypePtr)
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
		
	ifTokenGetElseError(TKN_THEN, ABL_ERR_SYNTAX_MISSING_THEN);

	if ((curToken != TKN_END_IF) && (curToken != TKN_ELSE))
		do {
			statement();
			while (curToken == TKN_SEMICOLON)
				getToken();
			if ((curToken == TKN_END_IF) || (curToken == TKN_ELSE))
				break;
		} while (tokenIn(statementStartList));
	
	fixupAddressMarker(falseLocation);
	
	//-----------------------------
	// ELSE branch, if necessary...
	if (curToken == TKN_ELSE) {
		getToken();
		char* ifEndLocation = crunchAddressMarker(NULL);

		if (curToken != TKN_END_IF)
			do {
				statement();
				while (curToken == TKN_SEMICOLON)
					getToken();
				if (curToken == TKN_END_IF)
					break;
			} while (tokenIn(statementStartList));
		
		fixupAddressMarker(ifEndLocation);
	}

	ifTokenGetElseError(TKN_END_IF, ABL_ERR_SYNTAX_MISSING_END_IF);
}

//***************************************************************************

void forStatement (void) {

	getToken();
	char* loopEndLocation = crunchAddressMarker(NULL);
	
	TypePtr forType = NULL;
	if (curToken == TKN_IDENTIFIER) {
		SymTableNodePtr forIdPtr = NULL;
		searchAndFindAllSymTables(forIdPtr);
		crunchSymTableNodePtr(forIdPtr);
		if (/*(forIdPtr->level != level) ||*/ (forIdPtr->defn.key != DFN_VAR))
			syntaxError(ABL_ERR_SYNTAX_INVALID_FOR_CONTROL);
		
		forType = forIdPtr->typePtr;
		getToken();
		
		//------------------------------------------------------------------
		// If we end up adding a CHAR type, this line needs to be changed...
		if ((forType != IntegerTypePtr) && /*(forType != CharTypePtr) &&*/ (forType->form != FRM_ENUM))
			syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
	
		}
	else {
		syntaxError(ABL_ERR_SYNTAX_MISSING_IDENTIFIER);
		forType = &DummyType;
	}
	
	ifTokenGetElseError(TKN_EQUAL, ABL_ERR_SYNTAX_MISSING_EQUAL);
	
	TypePtr exprType = expression();
	if (!isAssignTypeCompatible(forType, exprType))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
		
	if (curToken == TKN_TO)
		getToken();
	else
		syntaxError(ABL_ERR_SYNTAX_MISSING_TO);

	exprType = expression();
	if (!isAssignTypeCompatible(forType, exprType))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);

	//-----------------------------------------
	// For now, let's use the DO keyword...
	ifTokenGetElseError(TKN_DO, ABL_ERR_SYNTAX_MISSING_DO);		

	if (curToken != TKN_END_FOR)
		do {
			statement();
			while (curToken == TKN_SEMICOLON)
				getToken();
			if (curToken == TKN_END_FOR)
				break;
		} while (tokenIn(statementStartList));

	ifTokenGetElseError(TKN_END_FOR, ABL_ERR_SYNTAX_MISSING_END_FOR);

	fixupAddressMarker(loopEndLocation);
}

//***************************************************************************

TypePtr caseLabel (CaseItemPtr& caseItemHead, CaseItemPtr& caseItemTail, long& caseLabelCount) {

	CaseItemPtr newCaseItem = (CaseItemPtr)ABLStackMallocCallback(sizeof(CaseItem));
	if (!newCaseItem)
		ABL_Fatal(0, " ABL: Unable to AblStackHeap->malloc case item ");
	if (caseItemHead) {
		caseItemTail->next = newCaseItem;
		caseItemTail = newCaseItem;
		}
	else
		caseItemHead = caseItemTail = newCaseItem;
	newCaseItem->next = NULL;
	caseLabelCount++;

	TokenCodeType sign = TKN_PLUS;
	bool sawSign = false;
	if ((curToken == TKN_PLUS) || (curToken == TKN_MINUS)) {
		sign = curToken;
		sawSign = true;
		getToken();
	}

	if (curToken == TKN_NUMBER) {
		SymTableNodePtr thisNode = searchSymTable(tokenString, SymTableDisplay[1]);
		if (!thisNode)
			thisNode = enterSymTable(tokenString, &SymTableDisplay[1]);
		crunchSymTableNodePtr(thisNode);
		if (curLiteral.type == LIT_INTEGER)
			newCaseItem->labelValue = (sign == TKN_PLUS) ? curLiteral.value.integer : -curLiteral.value.integer;
		else
			syntaxError(ABL_ERR_SYNTAX_INVALID_CONSTANT);
		return(IntegerTypePtr);
		}
	else if (curToken == TKN_IDENTIFIER) {
		SymTableNodePtr idPtr;
		searchAllSymTables(idPtr);
		crunchSymTableNodePtr(idPtr);
		if (!idPtr) {
			syntaxError(ABL_ERR_SYNTAX_UNDEFINED_IDENTIFIER);
			return(&DummyType);
			}
		else if (idPtr->defn.key != DFN_CONST) {
			syntaxError(ABL_ERR_SYNTAX_NOT_A_CONSTANT_IDENTIFIER);
			return(&DummyType);
			}
		else if (idPtr->typePtr == IntegerTypePtr) {
			newCaseItem->labelValue = (sign == TKN_PLUS ? idPtr->defn.info.constant.value.integer : -idPtr->defn.info.constant.value.integer);
			return(IntegerTypePtr);
			}
		else if (idPtr->typePtr == CharTypePtr) {
			if (sawSign)
				syntaxError(ABL_ERR_SYNTAX_INVALID_CONSTANT);
			newCaseItem->labelValue = idPtr->defn.info.constant.value.character;
			return(CharTypePtr);
			}
		else if (idPtr->typePtr->form == FRM_ENUM) {
			if (sawSign)
				syntaxError(ABL_ERR_SYNTAX_INVALID_CONSTANT);
			newCaseItem->labelValue = idPtr->defn.info.constant.value.integer;
			return(idPtr->typePtr);
			}
		else
			return(&DummyType);
		}
	else if (curToken == TKN_STRING) {
		// STRING/CHAR TYPE...
		}
	else {
		syntaxError(ABL_ERR_SYNTAX_INVALID_CONSTANT);
		return(&DummyType);
	}
	return(&DummyType);
}

//---------------------------------------------------------------------------

void caseBranch (CaseItemPtr& caseItemHead, CaseItemPtr& caseItemTail, long& caseLabelCount, TypePtr expressionType) {

	//static CaseItemPtr oldCaseItemTail = NULL;
	CaseItemPtr oldCaseItemTail = caseItemTail;

	bool anotherLabel;
	do {
		TypePtr labelType = caseLabel(caseItemHead, caseItemTail, caseLabelCount);
		if (expressionType != labelType)
			syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);

		getToken();
		if (curToken == TKN_COMMA) {
			getToken();
			if (tokenIn(CaseLabelStartList))
				anotherLabel = true;
			else {
				syntaxError(ABL_ERR_SYNTAX_MISSING_CONSTANT);
				anotherLabel = false;
			}
			}
		else
			anotherLabel = false;
	} while (anotherLabel);

	//--------------
	// Error sync...
	synchronize(FollowCaseLabelList, statementStartList, NULL);
	ifTokenGetElseError(TKN_COLON, ABL_ERR_SYNTAX_MISSING_COLON);

	//-----------------------------------------------------------------
	// Fill in the branch location for each CaseItem for this branch...
	CaseItemPtr caseItem = (!oldCaseItemTail ? caseItemHead : oldCaseItemTail->next);
	//oldCaseItemTail = CaseItemTail;
	while (caseItem) {
		caseItem->branchLocation = codeBufferPtr;
		caseItem = caseItem->next;
	}

	if (curToken != TKN_END_CASE)
		do {
			statement();
			while (curToken == TKN_SEMICOLON)
				getToken();
			if (curToken == TKN_END_CASE)
				break;
		} while (tokenIn(statementStartList));

	ifTokenGetElseError(TKN_END_CASE, ABL_ERR_SYNTAX_MISSING_END_CASE);
	ifTokenGetElseError(TKN_SEMICOLON, ABL_ERR_SYNTAX_MISSING_SEMICOLON);
}

//---------------------------------------------------------------------------

void switchStatement (void) {

	//-------------------------
	// Init the branch table...
	getToken();
	char* branchTableLocation = crunchAddressMarker(NULL);

CaseItemPtr	caseItemHead = NULL;
CaseItemPtr	caseItemTail = NULL;
long caseLabelCount = 0;
	
//CaseItemHead = CaseItemTail = NULL;
//CaseLabelCount = 0;

	TypePtr expressionType = expression();

	//-----------------------------------------------------------------------------
	// NOTE: If we have subranges in ABL, we'll have to check in the following line
	// for a subrange, as well...
	if (((expressionType->form != FRM_SCALAR) && (expressionType->form != FRM_ENUM)) || (expressionType == RealTypePtr))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);

	synchronize(FollowSwitchExpressionList, NULL, NULL);

	//----------------------------
	// Process each CASE branch...
	bool moreBranches = (curToken == TKN_CASE);
	char* caseEndChain = NULL;
	while (moreBranches) {
		getToken();
		if (tokenIn(CaseLabelStartList))
			caseBranch(caseItemHead, caseItemTail, caseLabelCount, expressionType);

		//---------------------------------------------------
		// Link another address marker at the end of the CASE
		// branch to point to the end of the CASE block...
		caseEndChain = crunchAddressMarker(caseEndChain);

		moreBranches = (curToken == TKN_CASE);
	}

	//if (curToken == TKN_DEFAULT) {
	//}

	//-------------------------
	// Emit the branch table...
	fixupAddressMarker(branchTableLocation);
	crunchInteger(caseLabelCount);
	CaseItemPtr caseItem = caseItemHead;
	while (caseItem) {
		crunchInteger(caseItem->labelValue);
		crunchOffset(caseItem->branchLocation);
		CaseItemPtr nextCaseItem = caseItem->next;
		ABLStackFreeCallback(caseItem);
		caseItem = nextCaseItem;
	}

	ifTokenGetElseError(TKN_END_SWITCH, ABL_ERR_SYNTAX_MISSING_END_SWITCH);

	//--------------------------------------------
	// Patch up the case branch address markers...
	while (caseEndChain)
		caseEndChain = fixupAddressMarker(caseEndChain);

}

//***************************************************************************

void transStatement (void) {

	getToken();
	ifTokenGetElseError(TKN_IDENTIFIER, ABL_ERR_MISSING_STATE_IDENTIFIER);

	SymTableNodePtr IdPtr = searchSymTableForState(wordString, SymTableDisplay[1]);
	if (!IdPtr) {
		// New symbol, so let's assume it's a state defined later. We'll make it
		IdPtr = forwardState(wordString);
	}
	if (!IdPtr || (IdPtr->defn.key != DFN_FUNCTION) || ((IdPtr->defn.info.routine.flags & ROUTINE_FLAG_STATE) == 0))
		syntaxError(ABL_ERR_MISSING_STATE_IDENTIFIER);
	crunchSymTableNodePtr(IdPtr);
//	getToken();
}

//***************************************************************************

void transBackStatement (void) {

	getToken();
}

//***************************************************************************

void statement (void) {

	//-------------------------------------------------------------------
	// NOTE: Since we currently don't support generic BEGIN/END (compound
	// statement) blocks...
	if ((curToken != TKN_CODE) /*&& (curToken != TKN_BEGIN)*/)
			crunchStatementMarker();
	
	switch (curToken) {
		case TKN_IDENTIFIER: {
			SymTableNodePtr IdPtr = NULL;
			
			//--------------------------------------------------------------
			// First, do we have an assignment statement or a function call?		
			searchAndFindAllSymTables(IdPtr);
		
			if ((IdPtr->defn.key == DFN_FUNCTION)/* || (IdPtr->defn.key == DFN_MODULE)*/) {
				RoutineKey key = IdPtr->defn.info.routine.key;
				if ((key == RTN_ASSERT) || (key == RTN_PRINT) || (key == RTN_CONCAT)) {
					bool uncrunch = ((key == RTN_ASSERT) && !AssertEnabled) ||
									((key == RTN_PRINT) && !PrintEnabled) ||
									((key == RTN_CONCAT) && !StringFunctionsEnabled);
					if (uncrunch) {
						uncrunchStatementMarker();
						Crunch = false;
					}
				}
				crunchSymTableNodePtr(IdPtr);
				if (IdPtr->defn.info.routine.flags & ROUTINE_FLAG_ORDER) {
					if (NumOrderCalls == MAX_ORDERS)
						syntaxError(ABL_ERR_SYNTAX_TOO_MANY_ORDERS);
					crunchByte(NumOrderCalls / 32);
					crunchByte(NumOrderCalls % 32);
					NumOrderCalls++;
				}
				getToken();
				SymTableNodePtr thisRoutineIdPtr = CurRoutineIdPtr;
				routineCall(IdPtr, 1);
				CurRoutineIdPtr = thisRoutineIdPtr;
				Crunch = true;
				}
			else
				assignmentStatement(IdPtr);
			}
			break;
		case TKN_REPEAT:
			repeatStatement();
			break;
		case TKN_WHILE:
			whileStatement();
			break;
		case TKN_IF:
			ifStatement();
			break;
		case TKN_FOR:
			forStatement();
			break;
		case TKN_SWITCH:
			switchStatement();
			break;
		case TKN_TRANS:
			transStatement();
			break;
		case TKN_TRANS_BACK:
			transBackStatement();
			break;
	}

	//---------------------------------------------------------------------
	// Now, make sure the statement is closed off with the proper block end
	// statement, if necessary (which is usually the case :).
	synchronize(statementEndList, NULL, NULL);
	if (tokenIn(statementStartList))
		syntaxError(ABL_ERR_SYNTAX_MISSING_SEMICOLON);
}
		
//***************************************************************************

