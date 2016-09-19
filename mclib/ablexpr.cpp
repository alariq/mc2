//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLEXPR.CPP
//
//***************************************************************************

#include<string.h>
#include<stdio.h>
#include<stdlib.h>

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

#ifndef ABLENV_H
#include"ablenv.h"
#endif

//***************************************************************************

extern TokenCodeType	curToken;
extern char				tokenString[];
extern char				wordString[];
extern Literal			curLiteral;

extern SymTableNodePtr	SymTableDisplay[];
extern long				level;

extern TypePtr			IntegerTypePtr, CharTypePtr, RealTypePtr, BooleanTypePtr;
extern Type				DummyType;

extern TokenCodeType	statementEndList[];

extern bool  EnterStateSymbol;
extern ABLModulePtr		CurFSM;
SymTableNodePtr forwardState (char* stateName);
extern SymTableNodePtr	CurModuleIdPtr;

//***************************************************************************

TokenCodeType relationalOperatorList[] = {
	TKN_LT,
	TKN_LE,
	TKN_EQUALEQUAL,
	TKN_NE,
	TKN_GE,
	TKN_GT,
	TKN_NONE
};

TokenCodeType addOperatorList[] = {
	TKN_PLUS,
	TKN_MINUS,
	TKN_OR,
	TKN_NONE
};

TokenCodeType multiplyOperatorList[] = {
	TKN_STAR,
	TKN_FSLASH,
	TKN_DIV,		// we'll probably want to make this covered with FSLASH
	TKN_MOD,
	TKN_AND,
	TKN_NONE
};

//***************************************************************************
// MISC
//***************************************************************************

inline bool integerOperands (TypePtr type1, TypePtr type2) {

	return((type1 == IntegerTypePtr) && (type2 == IntegerTypePtr));
}

//***************************************************************************

inline bool realOperands (TypePtr type1, TypePtr type2) {

	if (type1 == RealTypePtr)
		return((type2 == RealTypePtr) || (type2 == IntegerTypePtr));
	else if (type2 == RealTypePtr)
		return(type1 == IntegerTypePtr);
	else
		return(false);
}

//***************************************************************************

inline bool booleanOperands (TypePtr type1, TypePtr type2) {

	return((type1 == BooleanTypePtr) && (type2 == BooleanTypePtr));
}

//***************************************************************************

void checkRelationalOpTypes (TypePtr type1, TypePtr type2) {

	if (type1 && type2) {
		if ((type1 == type2) && ((type1->form == FRM_SCALAR) || (type1->form == FRM_ENUM)))
			return;
		if (((type1 == IntegerTypePtr) && (type2 == RealTypePtr)) ||
			((type2 == IntegerTypePtr) && (type1 == RealTypePtr)))
			return;
		if ((type1->form == FRM_ARRAY) && (type2->form == FRM_ARRAY) &&
			(type1->info.array.elementTypePtr == CharTypePtr) && (type2->info.array.elementTypePtr == CharTypePtr) &&
			(type1->info.array.elementCount == type2->info.array.elementCount))
			return;
	}
	syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
}

//***************************************************************************

long isAssignTypeCompatible (TypePtr type1, TypePtr type2) {

	if (type1 == type2)
		return(1);
		
	if ((type1 == RealTypePtr) && (type2 == IntegerTypePtr))
		return(1);

	if ((type1->form == FRM_ARRAY) && (type2->form == FRM_ARRAY) &&
		(type1->info.array.elementTypePtr == CharTypePtr) && (type2->info.array.elementTypePtr == CharTypePtr) &&
		(type1->info.array.elementCount >= type2->info.array.elementCount))
		return(1);

	return(0);
}

//***************************************************************************
// EXPRESSION routines
//***************************************************************************

TypePtr variable (SymTableNodePtr variableIdPtr) {

	TypePtr typePtr = (TypePtr)(variableIdPtr->typePtr);
	DefinitionType defnKey = variableIdPtr->defn.key;

	crunchSymTableNodePtr(variableIdPtr);

	switch (defnKey) {
		case DFN_VAR:
		case DFN_VALPARAM:
		case DFN_REFPARAM:
		case DFN_FUNCTION:
		case DFN_UNDEFINED:
			break;
		default:
			typePtr = &DummyType;
			syntaxError(ABL_ERR_SYNTAX_INVALID_IDENTIFIER_USAGE);
	}

	getToken();

	//---------------------------------------------------------------------
	// There should not be a parameter list. However, if there is, parse it
	// for error recovery...
	if (curToken == TKN_LPAREN) {
		syntaxError(ABL_ERR_SYNTAX_UNEXPECTED_TOKEN);
		actualParamList(variableIdPtr, 0);
		return(typePtr);
	}

	//-----------
	// Subscripts
	while (curToken == TKN_LBRACKET) {
		if (curToken == TKN_LBRACKET)
			typePtr = arraySubscriptList(typePtr);
	}

	return(typePtr);
}

//***************************************************************************

TypePtr arraySubscriptList (TypePtr typePtr) {

	TypePtr indexTypePtr = NULL;
	TypePtr elementTypePtr = NULL;
	TypePtr subscriptTypePtr = NULL;

	do {
		if (typePtr->form == FRM_ARRAY) {
			indexTypePtr = typePtr->info.array.indexTypePtr;
			elementTypePtr = typePtr->info.array.elementTypePtr;

			getToken();
			subscriptTypePtr = expression();

			//-------------------------------------------------------------
			// If the subscript expression isn't assignment type compatible
			// with its corresponding subscript type, we're screwed...
			if (!isAssignTypeCompatible(indexTypePtr, subscriptTypePtr))
				syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);

			typePtr = elementTypePtr;
			}
		else {
			syntaxError(ABL_ERR_SYNTAX_TOO_MANY_SUBSCRIPTS);
			while ((curToken != TKN_RBRACKET) && !tokenIn(statementEndList))
				getToken();
		}
	} while (curToken == TKN_COMMA);

	ifTokenGetElseError(TKN_RBRACKET, ABL_ERR_SYNTAX_MISSING_RBRACKET);
	return(typePtr);
}

//***************************************************************************

TypePtr factor (void) {

	TypePtr thisType = NULL;
	switch (curToken) {
		case TKN_IDENTIFIER: {
			SymTableNodePtr IdPtr = NULL;
			searchAndFindAllSymTables(IdPtr);
			switch (IdPtr->defn.key) {
				case DFN_FUNCTION:
					crunchSymTableNodePtr(IdPtr);
					getToken();
					thisType = routineCall(IdPtr, 1);
					break;
				case DFN_CONST:
					crunchSymTableNodePtr(IdPtr);
					getToken();
					thisType = (TypePtr)(IdPtr->typePtr);
					break;
				default:
					thisType = (TypePtr)variable(IdPtr);
					break;
			}
			}
			break;
		case TKN_NUMBER: {
			SymTableNodePtr thisNode = searchSymTable(tokenString, SymTableDisplay[1]);
			if (!thisNode)
				thisNode = enterSymTable(tokenString, &SymTableDisplay[1]);
			if (curLiteral.type == LIT_INTEGER) {
				thisNode->typePtr = IntegerTypePtr;
				thisType = (TypePtr)(thisNode->typePtr);
				thisNode->defn.info.constant.value.integer = curLiteral.value.integer;
				}
			else {
				thisNode->typePtr = RealTypePtr;
				thisType = (TypePtr)(thisNode->typePtr);
				thisNode->defn.info.constant.value.real = curLiteral.value.real;
			}
			crunchSymTableNodePtr(thisNode);
			getToken();
			}
			break;
		case TKN_STRING: {
			long length = strlen(curLiteral.value.string);
			if (EnterStateSymbol) {
				SymTableNodePtr stateSymbol = searchSymTableForState(curLiteral.value.string, SymTableDisplay[1]);
				if (!stateSymbol)
					forwardState(curLiteral.value.string);
			}
			SymTableNodePtr thisNode = searchSymTableForString(tokenString, SymTableDisplay[1]);
			if (!thisNode)// {
				thisNode = enterSymTable(tokenString, &SymTableDisplay[1]);
				if (length == 1) {
					thisNode->defn.info.constant.value.character = curLiteral.value.string[0];
					thisType = CharTypePtr;
					}
				else {
					thisNode->typePtr = thisType = makeStringType(length);
					thisNode->info = (char*)ABLSymbolMallocCallback(length + 1);
					if (!thisNode->info)
						ABL_Fatal(0, " ABL: Unable to AblSymTableHeap->malloc string literal ");
					strcpy(thisNode->info, curLiteral.value.string);
				}
			//}
			crunchSymTableNodePtr(thisNode);

			getToken();
			}
			break;
		case TKN_NOT:
			getToken();
			thisType = factor();
			break;
		case TKN_LPAREN:
			getToken();
			thisType = expression();
			ifTokenGetElseError(TKN_RPAREN, ABL_ERR_SYNTAX_MISSING_RPAREN);
			break;
		default:
			syntaxError(ABL_ERR_SYNTAX_INVALID_EXPRESSION);
			thisType = &DummyType;
			break;
	}
	return(thisType);
}

//***************************************************************************

TypePtr term (void) {

	//-------------------------
	// Grab the first factor...
	TypePtr resultType = factor();
	
	//------------------------------------------------------------------
	// Now, continue grabbing factors separated by multiply operators...
	while (tokenIn(multiplyOperatorList)) {
		TokenCodeType op = curToken;
		
		getToken();
		TypePtr secondType = factor();
		
		switch (op) {
			case TKN_STAR:
				if (integerOperands(resultType, secondType)) {
					//---------------------------------------------------
					// Both operands are integer, so result is integer...
					resultType = IntegerTypePtr;
					}
				else if (realOperands(resultType, secondType)) {
					//----------------------------------------------------
					// Both real operands, or mixed (real and integer)...
					resultType = RealTypePtr;
					}
				else {
					syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
					resultType = &DummyType;
				}
				break;
			case TKN_FSLASH:
				if (integerOperands(resultType, secondType)) {
					//---------------------------------------------------
					// Both operands are integer, so result is integer...
					resultType = IntegerTypePtr;
					}
				else if (realOperands(resultType, secondType)) {
					//----------------------------------------------------
					// Both real operands, or mixed (real and integer)...
					resultType = RealTypePtr;
					}
				else {
					syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
					resultType = &DummyType;
				}
				break;
			case TKN_DIV:
			case TKN_MOD:
				//----------------------------------------------------------
				// Both operands should be integer, and result is integer...
				if (!integerOperands(resultType, secondType))
					syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
				resultType = IntegerTypePtr;
				break;
			case TKN_AND:
				if (!booleanOperands(resultType, secondType))
					syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
				resultType = BooleanTypePtr;								
				break;
		}
	}
	return(resultType);
}

//***************************************************************************

TypePtr simpleExpression (void) {

	bool usedUnaryOp = false;
	TokenCodeType unaryOp = TKN_PLUS;
	
	if ((curToken == TKN_PLUS) || (curToken == TKN_MINUS)) {
		unaryOp = curToken;
		usedUnaryOp = true;
		getToken();
	}
	
	//------------------------------------------------
	// Grab the first term in the simple expression...
	TypePtr resultType = term();
	
	if (usedUnaryOp && (resultType != IntegerTypePtr) && (resultType != RealTypePtr))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
		
	//---------------------------------------------------
	// Continue to process all terms in the expression...
	while (tokenIn(addOperatorList)) {
		TokenCodeType op = curToken;
		
		getToken();
		TypePtr secondType = term();
		
		switch (op) {
			case TKN_PLUS:
			case TKN_MINUS:
				if (integerOperands(resultType, secondType)) {
					//---------------------------------------------------
					// Both operands are integer, so result is integer...
					resultType = IntegerTypePtr;
					}
				else if (realOperands(resultType, secondType)) {
					//----------------------------------------------------
					// Both real operands, or mixed (real and integer)...
					resultType = RealTypePtr;
					}
				else {
					syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
					resultType = &DummyType;
				}
				break;
			case TKN_OR:
				if (!booleanOperands(resultType, secondType))
					syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
				resultType = BooleanTypePtr;
				break;
		}
	}
	return(resultType);
}				

//***************************************************************************

TypePtr expression (void) {

	//------------------------------------
	// Grab the first simple expression...
	TypePtr resultType = simpleExpression();

	if (tokenIn(relationalOperatorList)) {
		//---------------------------------------
		// Snatch the second simple expression...
		getToken();
		TypePtr secondType = simpleExpression();

		checkRelationalOpTypes(resultType, secondType);
		resultType = BooleanTypePtr;
	}
	return(resultType);
}

//***************************************************************************

