//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								SCANNER.H
//
//***************************************************************************

#ifndef ABLSCAN_H
#define	ABLSCAN_H

#ifndef ABLGEN_H
#include"ablgen.h"
#endif

#ifndef ABLERR_H
#include"ablerr.h"
#endif

#include<cstddef>
#include<stdint.h>

//***************************************************************************

#define	TAB_SIZE				4

//#define	MIN_INTEGER				-2147483648
#define	MAX_INTEGER				2147483647
#define	MAX_DIGIT_COUNT			20
#define	MAX_EXPONENT			20

//***************************************************************************

//------------
// TOKEN CODES

typedef enum {
	TKN_NONE,
	TKN_IDENTIFIER,
	TKN_NUMBER,
	TKN_TYPE,
	TKN_STRING,
	TKN_STAR,
	TKN_LPAREN,
	TKN_RPAREN,
	TKN_MINUS,
	TKN_PLUS,
	TKN_EQUAL,
	TKN_LBRACKET,
	TKN_RBRACKET,
	TKN_COLON,
	TKN_SEMICOLON,
	TKN_LT,
	TKN_GT,
	TKN_COMMA,
	TKN_PERIOD,
	TKN_FSLASH,
	TKN_EQUALEQUAL,
	TKN_LE,
	TKN_GE,
	TKN_NE,
	TKN_EOF,
	TKN_ERROR,
	TKN_CODE,
	TKN_ORDER,
	TKN_STATE,
	TKN_AND,
	TKN_SWITCH,
	TKN_CASE,
	TKN_CONST,
	TKN_DIV,
	TKN_DO,
	TKN_OF,
	TKN_ELSE,
	TKN_END_IF,
	TKN_END_WHILE,
	TKN_END_FOR,
	TKN_END_FUNCTION,
	TKN_END_ORDER,
	TKN_END_STATE,
	TKN_END_MODULE,
	TKN_END_FSM,
	TKN_END_LIBRARY,
	TKN_END_VAR,
	TKN_END_CODE,
	TKN_END_CASE,
	TKN_END_SWITCH,
	TKN_FOR,
	TKN_FUNCTION,
	TKN_IF,
	TKN_MOD,
	TKN_NOT,
	TKN_OR,
	TKN_REPEAT,
	TKN_THEN,
	TKN_TO,
	TKN_UNTIL,
	TKN_VAR,
	TKN_REF,
	TKN_WHILE,
	TKN_ELSIF,
	TKN_RETURN,
	TKN_MODULE,
	TKN_FSM,
	TKN_TRANS,
	TKN_TRANS_BACK,
	TKN_LIBRARY,
	TKN_ETERNAL,
	TKN_STATIC,
	TKN_POUND,
	TKN_UNEXPECTED_TOKEN,
	TKN_STATEMENT_MARKER,
	TKN_ADDRESS_MARKER,
	NUM_TOKENS
} TokenCodeType;

typedef enum {
	CHR_LETTER,
	CHR_DIGIT,
	CHR_DQUOTE,
	CHR_SPECIAL,
	CHR_EOF
} CharCodeType;

typedef struct {
	const char*			string;
	TokenCodeType	tokenCode;
} ReservedWord;

//***************************************************************************

//------------------
// LITERAL structure

typedef enum {
	LIT_INTEGER,
	LIT_REAL,
	LIT_STRING
} LiteralType;

typedef struct {
	LiteralType		type;
	struct {
		int         integer;
		float		real;
		char		string[MAXLEN_TOKENSTRING];
	} value;
} Literal;

//---------------------------------------------------------------------------

typedef struct CaseItem {
	int					labelValue;
	char*				branchLocation;
	struct CaseItem*	next;
} CaseItem;

typedef CaseItem* CaseItemPtr;

//***************************************************************************

class ABLFile {

    public:

		char*		fileName;
		void*		file;

		static long (*createCB) (void** file, const char* fName);
		static long (*openCB) (void** file, const char* fName);
		static long (*closeCB) (void** file);
		static bool (*eofCB) (void* file);
		static long (*readCB) (void* file, unsigned char* buffer, long length);
		static int32_t (*readIntCB) (void* file);
		static long (*readLongCB) (void* file);
		static long (*readStringCB) (void* file, unsigned char* buffer);
		static long (*readLineExCB) (void* file, unsigned char* buffer, long maxLength);
		static long (*writeCB) (void* file, unsigned char* buffer, long length);
		static long (*writeByteCB) (void* file, unsigned char byte);
		static long (*writeIntCB) (void* file, int32_t value);
		static long (*writeLongCB) (void* file, long value);
		static long (*writeStringCB) (void* file, const char* buffer);

	public:

		void* operator new (size_t ourSize);

		void operator delete (void *us);

		void init (void);
	
		ABLFile (void) {
			init();
		}

		void destroy (void);

		~ABLFile (void) {
			destroy();
		}
		
		void set (void* fPtr) {
			file = fPtr;
		}

		void* get (void) {
			return(file);
		}

		long create (const char* fileName);

		long open (const char* fileName);

		long close (void);

		bool eof (void);

		long read (unsigned char* buffer, long length);

		int32_t readInt (void);

		long readLong (void);

		long readString (unsigned char* buffer);

		long readLineEx (unsigned char* buffer, long maxLength);

		long write (unsigned char* buffer, long length);

		long writeByte (unsigned char val);

		long writeInt (int32_t val);

		long writeLong (long val);

		long writeString (const char* buffer);
};

//***************************************************************************

//----------
// FUNCTIONS

inline CharCodeType calcCharCode (long ch);
long isReservedWord (void);
void initScanner (char* fileName);
void quitScanner (void);
void skipComment (void);
void skipBlanks (void);
void getChar(void);
void crunchToken (void);
void downShiftWord (void);
void getToken(void);
void getWord (void);
void accumulateValue (float* valuePtr, SyntaxErrorType errCode);
void getNumber (void);
void getString (void);
void getSpecial (void);
bool tokenIn (TokenCodeType* tokenList);
void synchronize (TokenCodeType* tokenList1,
				  TokenCodeType* tokenList2,
				  TokenCodeType* tokenList3);
bool getSourceLine (void);
void printLine (char* line);
void initPageHeader (char* fileName);
void printPageHeader (void);

//----------
// VARIABLES

extern char			wordString[MAXLEN_TOKENSTRING];
extern void* (*ABLSystemMallocCallback) (unsigned long memSize);
extern void* (*ABLStackMallocCallback) (unsigned long memSize);
extern void* (*ABLCodeMallocCallback) (unsigned long memSize);
extern void* (*ABLSymbolMallocCallback) (unsigned long memSize);
extern void (*ABLSystemFreeCallback) (void* memBlock);
extern void (*ABLStackFreeCallback) (void* memBlock);
extern void (*ABLCodeFreeCallback) (void* memBlock);
extern void (*ABLSymbolFreeCallback) (void* memBlock);
extern void (*ABLDebugPrintCallback) (const char* s);
extern long (*ABLRandomCallback) (long range);
extern void (*ABLSeedRandomCallback) (unsigned long range);
extern unsigned long (*ABLGetTimeCallback) (void);
extern void (*ABLFatalCallback) (long code, const char* s);

//***************************************************************************
	
#endif


