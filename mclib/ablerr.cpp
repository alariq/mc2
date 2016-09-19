//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLERR.CPP
//
//***************************************************************************

#include<stdio.h>
#include<stdlib.h>

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLSCAN_H
#include"ablscan.h"
#endif

#ifndef ABLERR_H
#include"ablerr.h"
#endif

#ifndef ABL_H
#include"abl.h"
#endif

//***************************************************************************

//----------
// EXTERNALS
extern char*		tokenp;
extern long			execLineNumber;
extern long			lineNumber;
extern long			FileNumber;
extern char			SourceFiles[MAX_SOURCE_FILES][MAXLEN_FILENAME];
extern ABLModulePtr	CurModule;
extern char			wordString[];

//---------------------------------------------------------------------------

//----------------------
// SYNTAX ERROR messages

const char* syntaxErrorMessages[] = {
	"No syntax error",										// 0
	"Syntax error",
	"Too many errors",
	"Cannot open source file",
	"Unexpected end-of-file",
	"Invalid number",
	"Invalid fraction",
	"Invalid exponent",
	"Too many digits",
	"Real out of range",
	"Integer out of range",									// 10
	"Missing right parenthesis",
	"Invalid expression",
	"Undefined identifier",
	"Redefined identifier",
	"Unexpected token",
	"Incompatible types",
	"Nesting too deep",
	"Code segment overflow",
	"Missing equal",
	"Missing semi-colon",									// 20
	"Invalid constant",
	"Not a constant identifier",
	"No record types",
	"Missing colon (ouch)",
	"Not a type identifier",
	"Invalid type",
	"Missing end",
	"Invalid identifier usage",
	"Too many subscripts",
	"Missing right bracket",								// 30
	"Incompatible assignment",
	"Missing until",
	"Missing then",
	"Invalid for control",
	"Missing identifier",
	"Missing to",
	"Missing period",
	"Missing module",
	"Missing library",
	"Already forwarded",									// 40
	"Invalid reference parameter",
	"Wrong number of parameters",
	"Missing begin",
	"Missing endvar",
	"No function nesting",
	"Missing code",
	"Missing endif",
	"Missing endwhile",
	"Missing endfor",
	"Missing endfunction",									// 50
	"Missing endmodule",
	"Missing endlibrary",
	"Missing do",
	"Invalid index type",
	"Missing comma",
	"Too many static variables",
	"Missing endcase",
	"Missing endswitch",
	"Missing constant",
	"Bad language directive parameter",						// 60
	"Unknown language directive",
	"Too Many Formal Parameters",
	"Too Many Local Variables",
	"Order must have Integer Return Type",
	"Too many orders",
	"Missing state for transition",
	"Missing endfsm"
};

//---------------------------------------------------------------------------

//-----------------------
// RUNTIME ERROR messages

const char* runtimeErrorMessages[] = {
	"Runtime stack overflow",
	"Infinite Loop",
	"Nested function call",
	"Unimplemented feature",
	"Value out of range",
	"Division by zero",
	"Invalid function argument",
	"Invalid case value",
	"Abort",
	"No Previous State"
};

//---------------------------------------------------------------------------

//--------
// GLOBALS

long	errorCount = 0;

extern DebuggerPtr debugger;

//***************************************************************************

void ABL_Fatal (long errCode, const char* s) {

	ABLFatalCallback(errCode, s);
}

//---------------------------------------------------------------------------

void ABL_Assert (bool test, long errCode, const char* s) {

#ifdef _DEBUG
	if (!test)
		ABLFatalCallback(errCode, s);
#endif
}

//***************************************************************************

void syntaxError (long errCode) {

	char errMessage[MAXLEN_ERROR_MESSAGE];
	sprintf(errMessage, "SYNTAX ERROR %s [line %d] - (type %d) %s \"%s\"\n", SourceFiles[FileNumber], lineNumber, errCode, syntaxErrorMessages[errCode], wordString);
	ABL_Fatal(0, errMessage);
	
	*tokenp = '\0';
	errorCount++;
	
	if (errorCount > MAX_SYNTAX_ERRORS) {
		sprintf(errMessage, "Way too many syntax errors. ABL aborted.\n");
		ABL_Fatal(0, errMessage);
	}
}

//---------------------------------------------------------------------------

void runtimeError (long errCode) {

	char message[512];

	if (debugger) {
		sprintf(message, "RUNTIME ERROR:  [%d] %s", errCode, runtimeErrorMessages[errCode]);
		debugger->print(message);
		sprintf(message, "MODULE %s", CurModule->getName());
		debugger->print(message);
		if (FileNumber > -1)
			sprintf(message, "FILE %s", CurModule->getSourceFile(FileNumber));
		else
			sprintf(message, "FILE %s: unavailable");
		debugger->print(message);
		sprintf(message, "LINE %d", execLineNumber);
		debugger->print(message);
		debugger->debugMode();
	}

	sprintf(message, "ABL RUNTIME ERROR %s [line %d] - (type %d) %s\n", (FileNumber > -1) ? CurModule->getSourceFile(FileNumber) : "unavailable", execLineNumber, errCode, runtimeErrorMessages[errCode]);
	ABL_Fatal(-ABL_ERR_RUNTIME_ABORT, message);
}

//***************************************************************************

