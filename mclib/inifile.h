//---------------------------------------------------------------------------
//
// inifile.h - This file contains the class declaration for the FitIni Files
//
//				The FitIni file is a ini style file.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef INIFILE_H
#define INIFILE_H
//---------------------------------------------------------------------------
// Include files

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef DINIFILE_H
#include"dinifile.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions
#ifndef 	NO_ERR
#define	NO_ERR						0x00000000
#endif

#define BLOCK_NOT_FOUND						0xFADA0000
#define ID_NOT_FOUND							0xFADA0001
#define DATA_NOT_CORRECT_TYPE				0xFADA0002
#define BUFFER_TOO_SMALL					0xFADA0003
#define NOT_A_FITINIFILE					0xFADA0004
#define NO_RAM_FOR_INI_BLOCKS				0xFADA0005
#define NO_MORE_BLOCKS						0xFADA0006
#define TOO_MANY_BLOCKS						0xFADA0007
#define NOT_ENOUGH_BLOCKS					0xFADA0008
#define VARIABLE_NOT_FOUND					0xFADA0009
#define SYNTAX_ERROR							0xFADA000A
#define NOT_ENOUGH_ELEMENTS_FOR_ARRAY	0xFADA000B
#define GET_NEXT_LINE						0xFADA000C
#define USER_ARRAY_TOO_SMALL				0xFADA000D
#define TOO_MANY_ELEMENTS					0xFADA000E

//---------------------------------------------------------------------------
// Enums

//---------------------------------------------------------------------------
// Structs
struct IniBlockNode
{
	char blockId[50];
	unsigned long blockOffset;
};

//---------------------------------------------------------------------------
//									FitIniFile
class FitIniFile : public File
{
	// Data Members
	//--------------
	protected:
		unsigned long 	totalBlocks;					//Total number of blocks in file
		IniBlockNode 	*fileBlocks;					//Data for blocks to speed up file
		char 				*currentBlockId;				//Id of current block
		unsigned long 	currentBlockOffset;			//Offset into file of block start
		unsigned long 	currentBlockSize;				//Length of current block

	// Member Functions
	//------------------
	protected:
		long afterOpen (void);
		void atClose (void);
		
		long findNextBlockStart (char *line = NULL, unsigned long lineLen = 0);
		long countBlocks (void);
		
		long getNextWord (char *&line, char *buffer, unsigned long bufLen);

		float textToFloat (const char *num);
		double textToDouble (const char *num);
		
		long textToLong (const char *num);
		unsigned long textToULong (const char *num);
		
		short textToShort (const char *num);
		unsigned short textToUShort (const char *num);
		
		char textToChar (const char *num);
		unsigned char textToUChar (const char *num);

		bool booleanToLong (const char *num);

		float mathToFloat (const char *num);
		
		long mathToLong (const char *num);
		unsigned long mathToULong (const char *num);
		
		short mathToShort (const char *num);
		unsigned short mathToUShort (const char *num);
		
		char mathToChar (const char *num);
		unsigned char mathToUChar (const char *num);

		long floatToText (char *result, float num, unsigned long bufLen);
		
		long longToTextDec (char *result, long num, unsigned long bufLen);
		long longToTextHex (char *result, long num, unsigned long bufLen);

		long shortToTextDec (char *result, short num, unsigned long bufLen);
		long shortToTextHex (char *result, short num, unsigned long bufLen);

		long byteToTextDec (char *result, byte num, unsigned long bufLen);	
		long byteToTextHex (char *result, byte num, unsigned long bufLen);
		
		long copyString (char* dest, char *src, unsigned long bufLen);

	public:
		FitIniFile (void);
		~FitIniFile (void);

		virtual long open (const char* fName, FileMode _mode = READ, long numChildren = 50);
		virtual long open (FilePtr _parent, unsigned long fileSize, long numChildren = 50);
		
		virtual long create (const char* fName);
		virtual long createWithCase(const char* fName );


		virtual void close (void);

		virtual FileClass getFileClass (void)
		{
			return INIFILE;
		}

		long seekBlock (const char *blockId);

		long readIdFloat (const char *varName, float &value);
		long readIdDouble (const char *varName, double &value);
		
		long readIdBoolean (const char *varName, bool &value);
		long readIdInt (const char *varName, int &value);
		long readIdLong (const char *varName, long &value);
		long readIdULong (const char *varName, unsigned long &value);
        // sebi
		long readIdULong (const char *varName, DWORD &value);
		
		long readIdShort (const char *varName, short &value);
		long readIdUShort (const char *varName, unsigned short &value);
		
		long readIdChar (const char *varName, char &value);
		long readIdUChar (const char *varName, unsigned char &value);
		
		long readIdString (const char *varName, char *result, unsigned long bufferSize);

		long getIdStringLength (const char *varName);
		
		long readIdFloatArray (const char *varName, float *result, unsigned long numElements);
		
		long readIdLongArray (const char *varName, long *result, unsigned long numElements);
		long readIdULongArray (const char *varName, unsigned long *result, unsigned long numElements);
		
		long readIdShortArray (const char *varName, short *result, unsigned long numElements);
		long readIdUShortArray (const char *varName, unsigned short *result, unsigned long numElements);
		
		long readIdCharArray (const char *varName, char *result, unsigned long numElements);
		long readIdUCharArray (const char *varName, unsigned char *result, unsigned long numElements);
		
		unsigned long getIdFloatArrayElements (const char *varName);
		
		unsigned long getIdLongArrayElements (const char *varName);
		unsigned long getIdULongArrayElements (const char *varName);
											 
		unsigned long getIdShortArrayElements (const char *varName);
		unsigned long getIdUShortArrayElements (const char *varName);
		
		unsigned long getIdCharArrayElements (const char *varName);
		unsigned long getIdUCharArrayElements (const char *varName);
		
		long writeBlock (const char *blockId);

		long writeIdFloat (const char *varName, float value);
		
		long writeIdBoolean (const char *varName, bool value);
		long writeIdLong (const char *varName, long value);
		long writeIdULong (const char *varName, unsigned long value);
		
		long writeIdShort (const char *varName, short value);
		long writeIdUShort (const char *varName, unsigned short value);
		
		long writeIdChar (const char *varName, char value);
		long writeIdUChar (const char *varName, unsigned char value);
		
		long writeIdString (const char *varName, const char *result);

		long writeIdFloatArray (const char *varName, float *array, unsigned long numElements);
		long writeIdLongArray (const char *varName, long *array, unsigned long numElements);
		long writeIdUShortArray (const char *varName, unsigned short *array, unsigned long numElements);
		long writeIdUCharArray (const char *varName, unsigned char *array, unsigned long numElements);
};

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//
//---------------------------------------------------------------------------
