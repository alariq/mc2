//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLXEXPR.CPP
//
//***************************************************************************

#include<string.h>

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

#ifndef ABLDBUG_H
#include"abldbug.h"
#endif

//***************************************************************************

//----------
// EXTERNALS

extern long				level;
extern char*			codeSegmentPtr;
extern TokenCodeType	codeToken;

extern StackItem*		stack;
extern StackItemPtr		tos;
extern StackItemPtr		stackFrameBasePtr;
extern StackItemPtr		StaticDataPtr;
extern SymTableNodePtr	CurRoutineIdPtr;
extern ABLModulePtr		CurModule;

extern TypePtr			IntegerTypePtr;
extern TypePtr			CharTypePtr;
extern TypePtr			RealTypePtr;
extern TypePtr			BooleanTypePtr;

extern DebuggerPtr		debugger;

//***************************************************************************

inline void promoteOperandsToReal (StackItemPtr operand1Ptr, TypePtr type1Ptr,
								   StackItemPtr operand2Ptr, TypePtr type2Ptr) {

	if (type1Ptr == IntegerTypePtr)
		operand1Ptr->real = (float)(operand1Ptr->integer);
	if (type2Ptr == IntegerTypePtr)
		operand2Ptr->real = (float)(operand2Ptr->integer);
}

//***************************************************************************

TypePtr execSubscripts (TypePtr typePtr) {

	//----------------------------------------
	// Loop to execute bracketed subscripts...

	while (codeToken == TKN_LBRACKET) {
		do {
			getCodeToken();
			execExpression();
			
			long subscriptValue = tos->integer;
			pop();

			//-------------------------
			// Range check the index...
			if ((subscriptValue < 0) || (subscriptValue >= typePtr->info.array.elementCount))
				runtimeError(ABL_ERR_RUNTIME_VALUE_OUT_OF_RANGE);

			tos->address += (subscriptValue * typePtr->info.array.elementTypePtr->size);

			if (codeToken == TKN_COMMA)
				typePtr = typePtr->info.array.elementTypePtr;
		} while (codeToken == TKN_COMMA);

		getCodeToken();
		if (codeToken == TKN_LBRACKET)
			typePtr = typePtr->info.array.elementTypePtr;
	}
	return(typePtr->info.array.elementTypePtr);
}

//***************************************************************************

TypePtr execConstant (SymTableNodePtr idPtr) {

	TypePtr typePtr = idPtr->typePtr;
	if ((typePtr == IntegerTypePtr) || (typePtr->form == FRM_ENUM))
		pushInteger(idPtr->defn.info.constant.value.integer);
	else if (typePtr == RealTypePtr)
		pushReal(idPtr->defn.info.constant.value.real);
	else if (typePtr == CharTypePtr)
		pushInteger(idPtr->defn.info.constant.value.character);
	else if (typePtr->form == FRM_ARRAY)
		pushAddress(idPtr->defn.info.constant.value.stringPtr);

	if (debugger)
		debugger->traceDataFetch(idPtr, typePtr, tos);

	getCodeToken();

	return(typePtr);
}

//***************************************************************************

TypePtr execVariable (SymTableNodePtr idPtr, UseType use) {

	TypePtr	typePtr = (TypePtr)(idPtr->typePtr);

	// First, point to the variable's stack item. If the variable's scope
	// level is less than the current scope level, follow the static links
	// to the proper stack frame base...
	StackItemPtr dataPtr = NULL;
	StackItem tempStackItem;
	switch (idPtr->defn.info.data.varType) {
		case VAR_TYPE_NORMAL: {
			StackFrameHeaderPtr headerPtr = (StackFrameHeaderPtr)stackFrameBasePtr;
			long delta = level - idPtr->level;
			while (delta-- > 0)
				headerPtr = (StackFrameHeaderPtr)headerPtr->staticLink.address;
			dataPtr = (StackItemPtr)headerPtr + idPtr->defn.info.data.offset;
			}
			break;
		case VAR_TYPE_ETERNAL:
			dataPtr = (StackItemPtr)stack + idPtr->defn.info.data.offset;
			break;
		case VAR_TYPE_STATIC:
			//---------------------------------------------------------
			// If we're referencing a library's static variable, we may
			// need to shift to its static data space temporarily...
			if (idPtr->library && (idPtr->library != CurModule))
				StaticDataPtr = idPtr->library->getStaticData();
			dataPtr = (StackItemPtr)StaticDataPtr + idPtr->defn.info.data.offset;
			if (idPtr->library && (idPtr->library != CurModule))
				StaticDataPtr = CurModule->getStaticData();
			break;
		case VAR_TYPE_REGISTERED:
			tempStackItem.address = (char*)idPtr->defn.info.data.registeredData;
			dataPtr = &tempStackItem;
			break;
	}

	//---------------------------------------------------------------
	// If it's a scalar or enumeration reference parameter, that item
	// points to the actual item...
	if (idPtr->defn.key == DFN_REFPARAM)
		if (typePtr->form != FRM_ARRAY)/* && (typePtr->form != FRM_RECORD)*/
			dataPtr = (StackItemPtr)dataPtr->address;

	ABL_Assert(dataPtr != NULL, 0, " ABL.execVariable(): dataPtr is NULL ");

	//-----------------------------------------------------
	// Now, push the address of the variable's data area...
	if ((typePtr->form == FRM_ARRAY) /*|| (typePtr->form == FRM_RECORD)*/) {
		//pushInteger(typePtr->size);
		pushAddress((Address)dataPtr->address);
		}
	else if (idPtr->defn.info.data.varType == VAR_TYPE_REGISTERED)
		pushAddress((Address)dataPtr->address);
	else
		pushAddress((Address)dataPtr);

	//-----------------------------------------------------------------------------------
	// If there is a subscript (or field identifier, if records are being used in ABL)
	// then modify the address to point to the proper element of the array (or record)...
	getCodeToken();
	while ((codeToken == TKN_LBRACKET) /*|| (codeTOken == TKN_PERIOD)*/) {
		//if (codeToken == TKN_LBRACKET)
			typePtr = execSubscripts(typePtr);
		//else if (codeToken == TKN_PERIOD)
		//	typePtr = execField(typePtr);
	}

	//------------------------------------------------------------
	// Leave the modified address on the top of the stack if:
	//		a) it's an assignment target;
	//		b) it reresents a parameter passed by reference;
	//		c) it's the address of an array or record;
	// Otherwise, replace the address with the value it points to.
	if ((use != USE_TARGET) && (use != USE_REFPARAM) &&	(typePtr->form != FRM_ARRAY)) {
		if ((typePtr == IntegerTypePtr) || (typePtr->form == FRM_ENUM)) {
			tos->integer = *((long*)tos->address);
			}
		else if (typePtr == CharTypePtr)
			tos->byte = *((char*)tos->address);
		else
			tos->real = *((float*)tos->address);
	}

	if (debugger) {
		if ((use != USE_TARGET) && (use != USE_REFPARAM)) {
			if (typePtr->form == FRM_ARRAY)
				debugger->traceDataFetch(idPtr, typePtr, (StackItemPtr)tos->address);
			else
				debugger->traceDataFetch(idPtr, typePtr, tos);
		}
	}

	return(typePtr);
}

//***************************************************************************

TypePtr execFactor (void) {

	TypePtr resultTypePtr = NULL;

   	switch (codeToken) {
		case TKN_IDENTIFIER: {
			SymTableNodePtr idPtr = getCodeSymTableNodePtr();
			if (idPtr->defn.key == DFN_FUNCTION) {
				SymTableNodePtr thisRoutineIdPtr = CurRoutineIdPtr;
				resultTypePtr = execRoutineCall(idPtr, false);
				CurRoutineIdPtr = thisRoutineIdPtr;
				}
			else if (idPtr->defn.key == DFN_CONST)
				resultTypePtr = execConstant(idPtr);
			else
				resultTypePtr = execVariable(idPtr, USE_EXPR);
			}
			break;
		case TKN_NUMBER: {
			SymTableNodePtr numberPtr = getCodeSymTableNodePtr();
			if (numberPtr->typePtr == IntegerTypePtr) {
				pushInteger(numberPtr->defn.info.constant.value.integer);
				resultTypePtr = IntegerTypePtr;
				}
			else {
				pushReal(numberPtr->defn.info.constant.value.real);
				resultTypePtr = RealTypePtr;
			}
			getCodeToken();
			}
			break;
		case TKN_STRING: {
			SymTableNodePtr nodePtr = getCodeSymTableNodePtr();
			long length = strlen(nodePtr->name);
			if (length > 1) {
				//-----------------------------------------------------------------------
				// Remember, the double quotes are on the back and front of the string...
				pushAddress(nodePtr->info);
				resultTypePtr = nodePtr->typePtr;
				}
			else {
				//----------------------------------------------
				// Just push the one character in this string...
				pushByte(nodePtr->name[0]);
				resultTypePtr = CharTypePtr;
			}
			getCodeToken();
			}
			break;
		case TKN_NOT:
			getCodeToken();
			resultTypePtr = execFactor();
			//--------------------------------------
			// Following flips 1 to 0, and 0 to 1...
			tos->integer = 1 - tos->integer;
			break;
		case TKN_LPAREN:
			getCodeToken();
			resultTypePtr = execExpression();
			getCodeToken();
			break;
	}
	return(resultTypePtr);
}

//***************************************************************************

TypePtr execTerm (void) {

	StackItemPtr operand1Ptr;
	StackItemPtr operand2Ptr;
	TypePtr type2Ptr;

	TokenCodeType op;

	TypePtr resultTypePtr = execFactor();

	//----------------------------------------------
	// Process the factors separated by operators...
	while ((codeToken == TKN_STAR) || (codeToken == TKN_FSLASH) ||
		   (codeToken == TKN_DIV) || (codeToken == TKN_MOD) ||
		   (codeToken == TKN_AND)) {

		op = codeToken;

		getCodeToken();
		type2Ptr = execFactor();

		operand1Ptr = tos - 1;
		operand2Ptr = tos;

		if (op == TKN_AND) {
			operand1Ptr->integer = operand1Ptr->integer && operand2Ptr->integer;
			resultTypePtr = BooleanTypePtr;
			}
		else
			switch (op) {
				case TKN_STAR:
					if ((resultTypePtr == IntegerTypePtr) && (type2Ptr == IntegerTypePtr)) {
						//-----------------------------
						// Both operands are integer...
						operand1Ptr->integer = operand1Ptr->integer * operand2Ptr->integer;
						resultTypePtr = IntegerTypePtr;
						}
					else {
						//----------------------------------------------------------------
						// Both operands are real, or one is real and the other integer...
						promoteOperandsToReal(operand1Ptr, resultTypePtr, operand2Ptr, type2Ptr);
						operand1Ptr->real = operand1Ptr->real * operand2Ptr->real;
						resultTypePtr = RealTypePtr;
					}
					break;
				case TKN_FSLASH:
					//--------------------------------------------------------------------
					// Both operands are real, or one is real and the other an integer. We
					// probably want this same token to be used for integers, as opposed
					// to using the DIV token...
					if ((resultTypePtr == IntegerTypePtr) && (type2Ptr == IntegerTypePtr)) {
						//-----------------------------
						// Both operands are integer...
						if (operand2Ptr->integer == 0) {
#ifdef _DEBUG
							runtimeError(ABL_ERR_RUNTIME_DIVISION_BY_ZERO);
#else
							//HACK!!!!!!!!!!!!
							operand1Ptr->integer = 0;
#endif
							}
						else
							operand1Ptr->integer = operand1Ptr->integer / operand2Ptr->integer;
						resultTypePtr = IntegerTypePtr;
						}
					else {
						//----------------------------------------------------------------
						// Both operands are real, or one is real and the other integer...
						promoteOperandsToReal(operand1Ptr, resultTypePtr, operand2Ptr, type2Ptr);
						if (operand2Ptr->real == 0.0)
#ifdef _DEBUG
							runtimeError(ABL_ERR_RUNTIME_DIVISION_BY_ZERO);
#else
							//HACK!!!!!!!!!!!!
							operand1Ptr->real = 0.0;
#endif
						else
							operand1Ptr->real = operand1Ptr->real / operand2Ptr->real;
						resultTypePtr = RealTypePtr;
					}
					break;
				case TKN_DIV:
				case TKN_MOD:
					//-----------------------------
					// Both operands are integer...
					if (operand2Ptr->integer == 0)
#ifdef _DEBUG
						runtimeError(ABL_ERR_RUNTIME_DIVISION_BY_ZERO);
#else
						//HACK!!!!!!!!!!!!
						operand1Ptr->integer = 0;
#endif
					else {
						if (op == TKN_DIV)
							operand1Ptr->integer = operand1Ptr->integer / operand2Ptr->integer;
						else
							operand1Ptr->integer = operand1Ptr->integer % operand2Ptr->integer;
					}
					resultTypePtr = IntegerTypePtr;
					break;
			}
			
		//------------------------------
		// Pop off the second operand...
		pop();
	}

	return(resultTypePtr);
}

//***************************************************************************

TypePtr execSimpleExpression (void) {

	TokenCodeType unaryOp = TKN_PLUS;
	//------------------------------------------------------
	// If there's a + or - before the expression, save it...
	if ((codeToken == TKN_PLUS) || (codeToken == TKN_MINUS)) {
		unaryOp = codeToken;
		getCodeToken();
	}

	TypePtr resultTypePtr = execTerm();

	//-------------------------------------------------------
	// If there was a unary -, negate the top of the stack...
	if (unaryOp == TKN_MINUS) {
		if (resultTypePtr == IntegerTypePtr)
			tos->integer = -(tos->integer);
		else
			tos->real = -(tos->real);
	}

	while ((codeToken == TKN_PLUS) || (codeToken == TKN_MINUS) || (codeToken == TKN_OR)) {
		TokenCodeType op = codeToken;
		getCodeToken();
		TypePtr type2Ptr = execTerm();

		StackItemPtr operand1Ptr = tos - 1;
		StackItemPtr operand2Ptr = tos;

		if (op == TKN_OR) {
			operand1Ptr->integer = operand1Ptr->integer || operand2Ptr->integer;
			resultTypePtr = BooleanTypePtr;
			}
		else if ((resultTypePtr == IntegerTypePtr) && (type2Ptr  == IntegerTypePtr)) {
			if (op == TKN_PLUS)
				operand1Ptr->integer = operand1Ptr->integer + operand2Ptr->integer;
			else
				operand1Ptr->integer = operand1Ptr->integer - operand2Ptr->integer;
			resultTypePtr = IntegerTypePtr;
			}
		else {
			//-------------------------------------------------------------------
			// Both operands are real, or one is real and the other is integer...
			promoteOperandsToReal(operand1Ptr, resultTypePtr, operand2Ptr, type2Ptr);
			if (op == TKN_PLUS)
				operand1Ptr->real = operand1Ptr->real + operand2Ptr->real;
			else
				operand1Ptr->real = operand1Ptr->real - operand2Ptr->real;
			resultTypePtr = RealTypePtr;
		}

		//--------------------------
		// Pop the second operand...
		pop();
	}

	return(resultTypePtr);
}

//***************************************************************************

TypePtr execExpression (void) {

	StackItemPtr		operand1Ptr;
	StackItemPtr		operand2Ptr;
	TokenCodeType		op;
	bool				result = false;

 	//-----------------------------------
	// Get the first simple expression...
	TypePtr resultTypePtr = execSimpleExpression();
	TypePtr type2Ptr = NULL;

 	//-----------------------------------------------------------
	// If there is a relational operator, save it and process the
	// second simple expression...

	if ((codeToken == TKN_EQUALEQUAL) || (codeToken == TKN_LT) ||
		(codeToken == TKN_GT) || (codeToken == TKN_NE) ||
		(codeToken == TKN_LE) || (codeToken == TKN_GE)) {
		op = codeToken;

		getCodeToken();

		//---------------------------------
		// Get the 2nd simple expression...
		type2Ptr = execSimpleExpression();

		operand1Ptr = tos - 1;
		operand2Ptr = tos;

		//-----------------------------------------------------
		// Both operands are integer, boolean or enumeration...
		if (((resultTypePtr == IntegerTypePtr) && (type2Ptr == IntegerTypePtr)) ||
			(resultTypePtr->form == FRM_ENUM)) {
			switch (op) {
				case TKN_EQUALEQUAL:
					result = operand1Ptr->integer == operand2Ptr->integer;
					break;
				case TKN_LT:
					result = operand1Ptr->integer < operand2Ptr->integer;
					break;
				case TKN_GT:
					result = operand1Ptr->integer > operand2Ptr->integer;
					break;
				case TKN_NE:
					result = operand1Ptr->integer != operand2Ptr->integer;
					break;
				case TKN_LE:
					result = operand1Ptr->integer <= operand2Ptr->integer;
					break;
				case TKN_GE:
					result = operand1Ptr->integer >= operand2Ptr->integer;
					break;
			}
			}
		else if (resultTypePtr == CharTypePtr) {
			switch (op) {
				case TKN_EQUALEQUAL:
					result = operand1Ptr->byte == operand2Ptr->byte;
					break;
				case TKN_LT:
					result = operand1Ptr->byte < operand2Ptr->byte;
					break;
				case TKN_GT:
					result = operand1Ptr->byte > operand2Ptr->byte;
					break;
				case TKN_NE:
					result = operand1Ptr->byte != operand2Ptr->byte;
					break;
				case TKN_LE:
					result = operand1Ptr->byte <= operand2Ptr->byte;
					break;
				case TKN_GE:
					result = operand1Ptr->byte >= operand2Ptr->byte;
					break;
			}
			}
		else if ((resultTypePtr->form == FRM_ARRAY) && (resultTypePtr->info.array.elementTypePtr == CharTypePtr)) {
			//----------------------------------------
			// Strings. For now, always return true...
			result = true;
			}
		else if ((resultTypePtr == RealTypePtr) || (type2Ptr == RealTypePtr)) {
			promoteOperandsToReal(operand1Ptr, resultTypePtr, operand2Ptr, type2Ptr);
			switch (op) {
				case TKN_EQUALEQUAL:
					result = operand1Ptr->real == operand2Ptr->real;
					break;
				case TKN_LT:
					result = operand1Ptr->real < operand2Ptr->real;
					break;
				case TKN_GT:
					result = operand1Ptr->real > operand2Ptr->real;
					break;
				case TKN_NE:
					result = operand1Ptr->real != operand2Ptr->real;
					break;
				case TKN_LE:
					result = operand1Ptr->real <= operand2Ptr->real;
					break;
				case TKN_GE:
					result = operand1Ptr->real >= operand2Ptr->real;
					break;
			}
		}
 
		//---------------------------------------------------------
		// Replace the two operands with the result on the stack...
		if (result)
			operand1Ptr->integer = 1;
		else
			operand1Ptr->integer = 0;
		pop();

		resultTypePtr = BooleanTypePtr;
	}
 	return(resultTypePtr);
}

//***************************************************************************




