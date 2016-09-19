//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLSTD.CPP
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

//***************************************************************************

extern TokenCodeType	curToken;
extern char				wordString[];
extern TokenCodeType	followParmList[];
extern TokenCodeType	statementEndList[];
extern SymTableNodePtr	symTableDisplay[];
extern long				level;
extern TypePtr			IntegerTypePtr;
extern TypePtr			CharTypePtr;
extern TypePtr			RealTypePtr;
extern TypePtr			BooleanTypePtr;
extern Type				DummyType;

extern SymTableNodePtr	CurRoutineIdPtr;

bool   EnterStateSymbol = false;

//***************************************************************************
// STANDARD ROUTINE PARSING FUNCTIONS
//***************************************************************************

void stdReturn (void) {

	// RETURN function
	//
	//		PARAMS: <same as function return type>
	//
	//		RETURN: NONE

	if (curToken == TKN_LPAREN) {
		getToken();
		TypePtr paramType = expression();
		if (paramType != CurRoutineIdPtr->typePtr)
			syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
		ifTokenGetElseError(TKN_RPAREN, ABL_ERR_SYNTAX_MISSING_RPAREN);
		}
	else if (CurRoutineIdPtr->typePtr != NULL)
		syntaxError(ABL_ERR_SYNTAX_WRONG_NUMBER_OF_PARAMS);
}

//***************************************************************************

void stdPrint (void) {

	// PRINT function:
	//
	//		PARAMS:	integer or real or string
	//
	//		RETURN: NONE

	if (curToken == TKN_LPAREN)
		getToken();
	else
		syntaxError(ABL_ERR_SYNTAX_WRONG_NUMBER_OF_PARAMS);

	TypePtr paramType = expression();
	if ((paramType != IntegerTypePtr) &&
		(paramType != RealTypePtr) &&
		(paramType != CharTypePtr) &&
		((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != CharTypePtr)))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
	ifTokenGetElseError(TKN_RPAREN, ABL_ERR_SYNTAX_MISSING_RPAREN);
}

//***************************************************************************

TypePtr stdConcat (void) {

	// PRINT function:
	//
	//		PARAMS:	char array
	//
	//				integer, real or char array
	//
	//		RETURN: integer (resulting length, not including NULL)

	if (curToken == TKN_LPAREN)
		getToken();
	else
		syntaxError(ABL_ERR_SYNTAX_WRONG_NUMBER_OF_PARAMS);

	TypePtr paramType = expression();
	if ((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != CharTypePtr))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
	ifTokenGetElseError(TKN_COMMA, ABL_ERR_SYNTAX_MISSING_COMMA);

	paramType = expression();
	if ((paramType != IntegerTypePtr) &&
		(paramType != RealTypePtr) &&
		(paramType != CharTypePtr) &&
		(paramType != BooleanTypePtr) &&
		((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != CharTypePtr)))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
		
	ifTokenGetElseError(TKN_RPAREN, ABL_ERR_SYNTAX_MISSING_RPAREN);

	return(IntegerTypePtr);
}

//***************************************************************************

TypePtr stdGetStateHandle (void) {

	if (curToken == TKN_LPAREN)
		getToken();
	else
		syntaxError(ABL_ERR_SYNTAX_WRONG_NUMBER_OF_PARAMS);

	EnterStateSymbol = true;
	TypePtr paramType = expression();
	EnterStateSymbol = false;
	if ((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != CharTypePtr))
		syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);

	ifTokenGetElseError(TKN_RPAREN, ABL_ERR_SYNTAX_MISSING_RPAREN);

	return(IntegerTypePtr);
}

//***************************************************************************

TypePtr standardRoutineCall (SymTableNodePtr routineIdPtr) {

	long key = routineIdPtr->defn.info.routine.key;
	long numParams = FunctionInfoTable[key].numParams;
	switch (key) {
		case RTN_RETURN:
			stdReturn();
			return(NULL);
		case RTN_PRINT:
			stdPrint();
			return(NULL);
		case RTN_CONCAT:
			return(stdConcat());
		case RTN_GET_STATE_HANDLE:
			return(stdGetStateHandle());
		default:
			if (key >= NumStandardFunctions)
				syntaxError(ABL_ERR_SYNTAX_UNEXPECTED_TOKEN);
			if (numParams == 0) {
				if (curToken == TKN_LPAREN)
					syntaxError(ABL_ERR_SYNTAX_WRONG_NUMBER_OF_PARAMS);
				}
			else {
				if (curToken == TKN_LPAREN)
					getToken();
				else
					syntaxError(ABL_ERR_SYNTAX_WRONG_NUMBER_OF_PARAMS);
				for (long i = 0; i < numParams; i++) {
					TypePtr paramType = expression();
					switch (FunctionInfoTable[key].params[i]) {
						case PARAM_TYPE_ANYTHING:
							break;
						case PARAM_TYPE_CHAR:
							if (paramType != CharTypePtr)
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_INTEGER:
							if (paramType != IntegerTypePtr)
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_REAL:
							if (paramType != RealTypePtr)
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_BOOLEAN:
							if (paramType != BooleanTypePtr)
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_INTEGER_REAL:
							if ((paramType != IntegerTypePtr) && (paramType != RealTypePtr))
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_CHAR_ARRAY:
							if ((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != CharTypePtr))
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_INTEGER_ARRAY:
							if ((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != IntegerTypePtr))
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_REAL_ARRAY:
							if ((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != RealTypePtr))
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
						case PARAM_TYPE_BOOLEAN_ARRAY:
							if ((paramType->form != FRM_ARRAY) || (paramType->info.array.elementTypePtr != BooleanTypePtr))
								syntaxError(ABL_ERR_SYNTAX_INCOMPATIBLE_TYPES);
							break;
					}
					if (i == (numParams - 1))
						ifTokenGetElseError(TKN_RPAREN, ABL_ERR_SYNTAX_MISSING_RPAREN);
					else
						ifTokenGetElseError(TKN_COMMA, ABL_ERR_SYNTAX_MISSING_COMMA);
				}
			}
			switch (FunctionInfoTable[key].returnType) {
				case RETURN_TYPE_NONE:
					return(NULL);
				case RETURN_TYPE_INTEGER:
					return(IntegerTypePtr);
				case RETURN_TYPE_REAL:
					return(RealTypePtr);
				case RETURN_TYPE_BOOLEAN:
					return(BooleanTypePtr);
			}
	}
	return(NULL);
}

//***************************************************************************

