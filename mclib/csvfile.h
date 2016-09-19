//---------------------------------------------------------------------------
//
// csvfile.h - This file contains the class declaration for the CSV Files
//
//				The CSV file is an Excel csv style file.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CSVFILE_H
#define CSVFILE_H
//---------------------------------------------------------------------------
// Include files

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef DCSVFILE_H
#include"dcsvfile.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions

//---------------------------------------------------------------------------
// Enums

//---------------------------------------------------------------------------
// Structs

//---------------------------------------------------------------------------
//									CSVFile
class CSVFile : public File
{
	// Data Members
	//--------------
	protected:
		DWORD totalRows;			//Number of ROWS CSV file has.
		DWORD totalCols;			//NUmber of COLS CSV file has.
		
		char dataBuffer[2048];

	// Member Functions
	//------------------
	protected:
		long afterOpen (void);
		void atClose (void);
		
		long countRows (void);
		long countCols (void);
		
		long getNextWord (char *&line, char *buffer, unsigned long bufLen);

		float textToFloat (char *num);
		
		long textToLong (char *num);
		unsigned long textToULong (char *num);
		
		short textToShort (char *num);
		unsigned short textToUShort (char *num);
		
		char textToChar (char *num);
		unsigned char textToUChar (char *num);

		bool booleanToLong (char *num);

		long floatToText (char *result, float num, unsigned long bufLen);
		
		long longToTextDec (char *result, long num, unsigned long bufLen);
		long longToTextHex (char *result, long num, unsigned long bufLen);

		long shortToTextDec (char *result, short num, unsigned long bufLen);
		long shortToTextHex (char *result, short num, unsigned long bufLen);

		long byteToTextDec (char *result, byte num, unsigned long bufLen);	
		long byteToTextHex (char *result, byte num, unsigned long bufLen);
		
		long copyString (char* dest, char *src, unsigned long bufLen);

	public:
		CSVFile (void);
		~CSVFile (void);

		virtual long open (const char* fName, FileMode _mode = READ, long numChildren = 50);
		virtual long open (FilePtr _parent, unsigned long fileSize, long numChildren = 50);
		
		virtual long create (char* fName);

		virtual void close (void);

		virtual FileClass getFileClass (void)
		{
			return CSVFILE;
		}

		long seekRowCol (DWORD row, DWORD col);
		
		long readFloat (DWORD row, DWORD col, float &value);
		
		long readBoolean (DWORD row, DWORD col, bool &value);
		long readLong (DWORD row, DWORD col, long &value);
		long readULong (DWORD row, DWORD col, unsigned long &value);
		
		long readShort (DWORD row, DWORD col, short &value);
		long readUShort (DWORD row, DWORD col, unsigned short &value);
		
		long readChar (DWORD row, DWORD col, char &value);
		long readUChar (DWORD row, DWORD col, unsigned char &value);
		
		long readString (DWORD row, DWORD col, char *result, unsigned long bufferSize);
};

//---------------------------------------------------------------------------
#endif
