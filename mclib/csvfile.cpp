//---------------------------------------------------------------------------
//
// csvfile.cpp - This file contains the class declaration for the CSV Files
//
//				The CSV file is an Excel csv style file.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files

#ifndef CSVFILE_H
#include"csvfile.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<ctype.h>

#ifndef _MBCS
#include<gameos.hpp>
#else
#include<assert.h>
#define gosASSERT assert
#define gos_Malloc malloc
#define gos_Free free
#endif

//---------------------------------------------------------------------------
// Static Globals

#undef isspace		//Macro Chokes under Intel Compiler!!

//---------------------------------------------------------------------------
// class CSVIniFile
CSVFile::CSVFile (void) : File()
{
	totalRows = totalCols = 0L;
}

//---------------------------------------------------------------------------
CSVFile::~CSVFile (void)
{
	close();
}

//---------------------------------------------------------------------------
long CSVFile::countRows (void)
{
	long count = 0;
	long oldPosition = logicalPosition;
	
	seek(0);		//Start at the top.
	char tmp[2048];
	
	readLine((MemoryPtr)tmp,2047);
	while (!eof())
	{
		count++;
		readLine((MemoryPtr)tmp,2047);
	}
	
	//----------------------------------
	// Move back to where we were.
	seek(oldPosition);
	
	return(count);
}

//---------------------------------------------------------------------------
long CSVFile::countCols (void)
{
	long count = 0, maxCols = 0;
	long oldPosition = logicalPosition;
	
	seek(0);		//Start at the top.
	char tmp[2048];
	char *currentChk = tmp;
	
	readLine((MemoryPtr)tmp,2047);
	currentChk = strstr(tmp,",");
	while (currentChk && (*currentChk != '\n') && (*currentChk != '\r'))
	{
		count++;
		currentChk++;
		currentChk = strstr(currentChk,",");
	}
	
	if (count > maxCols)
		maxCols = count;
		
	readLine((MemoryPtr)tmp,2047);
	
	//----------------------------------
	// Move back to where we were.
	seek(oldPosition);
	
	return(maxCols);
}

//---------------------------------------------------------------------------
long CSVFile::getNextWord (char *&line, char *buffer, unsigned long bufLen)
{
	//--------------------------------------------------
	// Check to see if we are at end of line
	if (*line == '\0')
		return(-1);
		
	//--------------------------------------------------
	// Check to see if the rest of the line is comments
	if (*line == '/')
		return(-1);

	if ( *line == ',' ) // empty column, move on
		return -1;
		
	//------------------------------------------
	// Find start of word from current location
	while ((*line != '\0') && ((*line == ' ') || (*line == '\t') || (*line == ',')))
	{
		line++;
	}
	
	//--------------------------------------------------
	// Check to see if we are at end of line
	if (*line == '\0')
		return(-1);
		
	//--------------------------------------------------
	// Check to see if the rest of the line is comments
	if (*line == '/')
		return(-1);
		
	//-------------------------------------------
	// Find length of word from current location
	char *startOfWord = line;
	unsigned long wordLength = 0;
	while ((*line != '\0') && ((*line != ',')))
	{
		line++;
		wordLength++;
	}
	
	if (wordLength > bufLen)
		return(-2);
		
	strncpy(buffer, startOfWord, wordLength);
	buffer[wordLength] = '\0';
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::afterOpen (void)
{
	//-------------------------------------------------------
	// Check if we opened this with CREATE and write the
	// FITini Header and position to Write Start.
	if (fileMode == CREATE && parent == NULL)
	{
		STOP(("Cannot write CSV files at present."));
	}
	else
	{
		//------------------------------------------------------
		// Find out how many Rows and cols we have
		totalRows = countRows();
		totalCols = countCols();
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------
void CSVFile::atClose (void)
{
	//------------------------------------------------------------
	// Check if we are in create mode and if so, write the footer
	if (fileMode == CREATE)
	{
		STOP(("Cannot write CSV files at present."));
	}

	totalRows = totalCols = 0;
}

//---------------------------------------------------------------------------
float CSVFile::textToFloat (char *num)
{
	float result = atof(num);
	return(result);
}

//---------------------------------------------------------------------------
long CSVFile::textToLong (char *num)
{
	long result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		long numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		long power = 0;
		for (long count = numDigits;count >= 0;count--,power++)
		{
			unsigned char currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}

	
	return(result);
}


//---------------------------------------------------------------------------
short CSVFile::textToShort (char *num)
{
	short result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		long numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		long power = 0;
		for (long count = numDigits;count >= 0;count--,power++)
		{
			unsigned char currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}


//---------------------------------------------------------------------------
char CSVFile::textToChar (char *num)
{
	char result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		long numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		long power = 0;
		for (long count = numDigits;count >= 0;count--,power++)
		{
			unsigned char currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}

//---------------------------------------------------------------------------
unsigned long CSVFile::textToULong (char *num)
{
	unsigned long result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		long numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		long power = 0;
		for (long count = numDigits;count >= 0;count--,power++)
		{
			unsigned char currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}


//---------------------------------------------------------------------------
unsigned short CSVFile::textToUShort (char *num)
{
	unsigned short result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		long numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		long power = 0;
		for (long count = numDigits;count >= 0;count--,power++)
		{
			unsigned char currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}


//---------------------------------------------------------------------------
unsigned char CSVFile::textToUChar (char *num)
{
	unsigned char result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		long numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		long power = 0;
		for (long count = numDigits;count >= 0;count--,power++)
		{
			unsigned char currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}

//---------------------------------------------------------------------------
bool CSVFile::booleanToLong (char *num)
{
	char testChar = 0;
	while (num[testChar] && isspace(num[testChar]))
		testChar++;

	// 'N' == NO if you can believe that
	if ((toupper(num[testChar]) == 'F') || (toupper(num[testChar]) == '0') || (toupper(num[testChar]) == 'N') )
		return FALSE;
	else
		return(TRUE);
}	

//---------------------------------------------------------------------------
long CSVFile::floatToText (char *result, float num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%f4",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(-2);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}			

//---------------------------------------------------------------------------
long CSVFile::longToTextDec (char *result, long num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(-2);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long CSVFile::longToTextHex (char *result, long num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(-2);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long CSVFile::shortToTextDec (char *result, short num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(-2);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long CSVFile::shortToTextHex (char *result, short num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(-2);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long CSVFile::byteToTextDec (char *result, byte num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(-2);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long CSVFile::byteToTextHex (char *result, byte num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(-2);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long CSVFile::open (const char* fName, FileMode _mode, long numChild)
{
	long result = File::open(fName,_mode,numChild);
	if (result != NO_ERR)
		return(result);
		
	seek(0);

	result = afterOpen();
	
	return(result);
}

//---------------------------------------------------------------------------
long CSVFile::open (FilePtr _parent, unsigned long fileSize, long numChild)
{
	numChild = -1;		//Force all parented CSVs to load from RAM.
	long result = File::open(_parent,fileSize,numChild);
	if (result != NO_ERR)
		return(result);
		
	result = afterOpen();
	
	return(result);
}

//---------------------------------------------------------------------------
long CSVFile::create (char* fName)
{
	fName;
	//STOP(("CSV file write is not supported %s",fName));
	return(-1);
}

//---------------------------------------------------------------------------
void CSVFile::close (void)
{
	if (isOpen())
	{
		atClose();
		File::close();
	}
}

//---------------------------------------------------------------------------
long CSVFile::seekRowCol (DWORD row, DWORD col)
{
	if ((row > totalRows) || (col > totalCols))
		return -1;
		
	DWORD rowCount = 0;
	
	seek(0);		//Start at the top.
	char tmp[2048];
	
	do
	{
		rowCount++;
		readLine((MemoryPtr)tmp,2047);
	} while (rowCount != row);  
	
	char *currentChk = tmp;
	if (col)
	{
		DWORD colCount = 1;
		
		while (currentChk && (colCount != col))
		{
			colCount++;
			currentChk = strstr(currentChk,",");
			if ( currentChk ) // if we increment to one, bad things happen
				currentChk++;

		}
	}
	
	//---------------------------------------------------
	// We are now pointing at the row and col specified.
	if (currentChk)
	{
		char *data = dataBuffer;
		return getNextWord(currentChk,data,2047);
	}
	else
	{
		return -1;
		// CAN'T do this, for some reason excel writes out empty rows.
		//STOP(("Unable to parse CSV %s, ROW %d, COL %D, ERROR: NULL",getFilename(),row,col));
	}
	
	//return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readFloat (DWORD row, DWORD col, float &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = textToFloat(dataBuffer);
	}
	else
		value = 0.0f;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readLong (DWORD row, DWORD col, long &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = textToLong(dataBuffer);
	}
	else
		value = 0.0f;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readBoolean (DWORD row, DWORD col, bool &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = booleanToLong(dataBuffer);
	}
	else
		value = 0;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readShort (DWORD row, DWORD col, short &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = textToShort(dataBuffer);
	}
	else
		value = 0.0f;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readChar (DWORD row, DWORD col, char &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = textToChar(dataBuffer);
	}
	else
		value = 0.0f;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readULong (DWORD row, DWORD col, unsigned long &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = textToULong(dataBuffer);
	}
	else
		value = 0.0f;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readUShort (DWORD row, DWORD col, unsigned short &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = textToUShort(dataBuffer);
	}
	else
		value = 0.0f;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readUChar (DWORD row, DWORD col, unsigned char &value)
{
	long result = seekRowCol(row,col);
	if (result == NO_ERR)
	{
		value = textToUChar(dataBuffer);
	}
	else
		value = 0.0f;
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::copyString (char *dest, char *src, unsigned long bufLen)
{
	unsigned long offset = 0;
	//---------------------------------------
	// Copy each character until close quote
	while (*src != '"' && *src != '\0' && offset < bufLen)
	{
		dest[offset] = *src;
		src++;
		offset++;
	}

	//----------------------------------------------------
	// If this string is longer than buffer, let em know.
	if (offset == bufLen)
	{
		return(-2);
	}

	//---------------------------------
	// otherwise, NULL term and return	
	dest[offset] = '\0';
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long CSVFile::readString (DWORD row, DWORD col, char *result, unsigned long bufferSize)
{
	long res = seekRowCol(row,col);
	if (res == NO_ERR)
	{
		long errorCode = copyString(result,dataBuffer,bufferSize);
		if (errorCode != NO_ERR)
			return(errorCode);
	}
	else
		return 1;	
	return(0); // gotta return some kind of error!
}

//---------------------------------------------------------------------------
//
// Edit log
//
//---------------------------------------------------------------------------

