//---------------------------------------------------------------------------
//
// inifile.cpp - This file contains the class declaration for the FitIni Files
//
//				The FitIni file is a ini style file.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files

#ifndef INIFILE_H
#include"inifile.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<string_win.h>
#include<ctype.h>

#include"err.h"

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
char fitIniHeader[7] = "FITini";
char fitIniFooter[7] = "FITend";

#undef isspace		//Macro Chokes under Intel Compiler!!

//---------------------------------------------------------------------------
// class FitIniFile
FitIniFile::FitIniFile (void) : File()
{
	totalBlocks = 0;
	fileBlocks = NULL;
	
	currentBlockId = NULL;
	currentBlockOffset = 0;
	currentBlockSize = 0;
}

//---------------------------------------------------------------------------
FitIniFile::~FitIniFile (void)
{
	close();
}

//---------------------------------------------------------------------------
long FitIniFile::findNextBlockStart (char *line, unsigned long lineLen)
{
	char thisLine[255];
	char *common = NULL;
	
	do
	{
		if (line)
		{
			readLine((MemoryPtr)line,lineLen);
			common = line;
		}
		else
		{
			readLine((MemoryPtr)thisLine,254);
			common = thisLine;
		}
	}
	while(!eof() && (common[0] != '['));
	
	if (eof())
	{
		return(NO_MORE_BLOCKS);
	}

	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long FitIniFile::countBlocks (void)
{
	long count = 0;
	long oldPosition = logicalPosition;
	
	while (findNextBlockStart() != NO_MORE_BLOCKS)
	{
		count++;
	}
	
	//----------------------------------
	// Move back to where we were.
	seek(oldPosition);
	
	return(count);
}

//---------------------------------------------------------------------------
long FitIniFile::getNextWord (char *&line, char *buffer, unsigned long bufLen)
{
	//--------------------------------------------------
	// Check to see if we are at end of line
	if (*line == '\0')
		return(GET_NEXT_LINE);
		
	//--------------------------------------------------
	// Check to see if the rest of the line is comments
	if (*line == '/')
		return(GET_NEXT_LINE);
		
	//------------------------------------------
	// Find start of word from current location
	while ((*line != '\0') && ((*line == ' ') || (*line == '\t') || (*line == ',')))
	{
		line++;
	}
	
	//--------------------------------------------------
	// Check to see if we are at end of line
	if (*line == '\0')
		return(GET_NEXT_LINE);
		
	//--------------------------------------------------
	// Check to see if the rest of the line is comments
	if (*line == '/')
		return(GET_NEXT_LINE);
		
	//-------------------------------------------
	// Find length of word from current location
	char *startOfWord = line;
	unsigned long wordLength = 0;
	while ((*line != '\0') && ((*line != ' ') && (*line != '\t') && (*line != ',')))
	{
		line++;
		wordLength++;
	}
	
	if (wordLength > bufLen)
		return(BUFFER_TOO_SMALL);
		
	strncpy(buffer, startOfWord, wordLength);
	buffer[wordLength] = '\0';
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::afterOpen (void)
{
	char line[255];
	
	//-------------------------------------------------------
	// Check if we opened this with CREATE and write the
	// FITini Header and position to Write Start.
	if (fileMode == CREATE && parent == NULL)
	{
		sprintf(line,"%s \r\n",fitIniHeader);
		write((MemoryPtr)line,strlen(line));

		totalBlocks = 0;
	}
	else
	{
		//------------------------------------------------------
		// Check if this is a FitIniFile by looking for header.
		char chkHeader[12];
	
		readLine((MemoryPtr)chkHeader,11);
		if (strstr(chkHeader,fitIniHeader) == NULL)
			return(NOT_A_FITINIFILE);

				//------------------------------------------------------
		// Find out how many blocks we have	
		totalBlocks = countBlocks();
		
		//--------------------------------------------------------------------------
		// Allocate RAM for the BlockInfoNodes.  Check if system Heap is available
		fileBlocks = (IniBlockNode *)systemHeap->Malloc(sizeof(IniBlockNode) * totalBlocks);
		
		gosASSERT(fileBlocks != NULL);

		memset(fileBlocks,0,sizeof(IniBlockNode) * totalBlocks);
		
		//--------------------------------------------------------------------------
		// Put Info into fileBlocks.
		
		unsigned long currentBlockNum = 0;
		while (findNextBlockStart(line,254) != NO_MORE_BLOCKS)
		{
			//----------------------------------------------------
			// If we write too many fileBlocks, we will trash RAM
			// Shouldn't be able to happen but...
			if (currentBlockNum == totalBlocks)
				return(TOO_MANY_BLOCKS);

			long count = 1;
			while (line[count] != ']' && line[count] != '\n')
			{
				fileBlocks[currentBlockNum].blockId[count-1] = line[count];
				count++;
			}
			if (count >= 49)
				STOP(("BlockId To large in Fit File %s",fileName));

			if (line[count] == '\n')
			{
				char error[256];
				sprintf( error, "couldn't resolve block %s in file %s", line, getFilename() );
				Assert( 0, 0, error );
				return SYNTAX_ERROR;
			}
			
			fileBlocks[currentBlockNum].blockId[count-1] = '\0';
			
			//----------------------------------------------------------------------
			// Since we just read all of last line, we now point to start of data
			fileBlocks[currentBlockNum].blockOffset = logicalPosition;
			currentBlockNum++;
		}

		//------------------------------------------------------
		// If we didn't read in enough, CD-ROM error?
		if (currentBlockNum != totalBlocks)
			return(NOT_ENOUGH_BLOCKS);
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------
void FitIniFile::atClose (void)
{
	//------------------------------------------------------------
	// Check if we are in create mode and if so, write the footer
	char line[200];
	if (fileMode == CREATE)
	{
		seek(0,SEEK_END);
		sprintf(line,"%s \r\n",fitIniFooter);
		write((MemoryPtr)line,strlen(line));
	}

	//-----------------------------
	// Free up the fileBlocks
	systemHeap->Free(fileBlocks);
	fileBlocks = NULL;
}

//---------------------------------------------------------------------------
float FitIniFile::textToFloat (const char *num)
{
	float result = atof(num);
	return(result);
}

//---------------------------------------------------------------------------
double FitIniFile::textToDouble (const char *num)
{
	double result = atof(num);
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::textToLong (const char *num)
{
	long result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = (char *)strstr(num,"0x");
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
short FitIniFile::textToShort (const char *num)
{
	short result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = (char *)strstr(num,"0x");
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
char FitIniFile::textToChar (const char *num)
{
	char result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = (char *)strstr(num,"0x");
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
unsigned long FitIniFile::textToULong (const char *num)
{
	unsigned long result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = (char *)strstr(num,"0x");
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
unsigned short FitIniFile::textToUShort (const char *num)
{
	unsigned short result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = (char *)strstr(num,"0x");
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
unsigned char FitIniFile::textToUChar (const char *num)
{
	unsigned char result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = (char *)strstr(num,"0x");
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
bool FitIniFile::booleanToLong (const char *num)
{
char testChar = 0;
	while (num[testChar] && isspace(num[testChar]))
		testChar++;

	if ((toupper(num[testChar]) == 'F') || (toupper(num[testChar]) == '0'))
		return FALSE;
	else
		return(TRUE);
}	

//---------------------------------------------------------------------------
long FitIniFile::floatToText (char *result, float num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%f4",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}			

//---------------------------------------------------------------------------
long FitIniFile::longToTextDec (char *result, long num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long FitIniFile::longToTextHex (char *result, long num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long FitIniFile::shortToTextDec (char *result, short num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long FitIniFile::shortToTextHex (char *result, short num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long FitIniFile::byteToTextDec (char *result, byte num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long FitIniFile::byteToTextHex (char *result, byte num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//---------------------------------------------------------------------------
long FitIniFile::open (const char* fName, FileMode _mode, long numChild, bool doNotLower)
{
	long result = File::open(fName,_mode,numChild, doNotLower);
	if (result != NO_ERR)
		return(result);
		
	seek(0);

	result = afterOpen();
	
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::open (FilePtr _parent, unsigned long fileSize, long numChild)
{
	numChild = -1;		//Force all parented FitINIs to load from RAM.
	long result = File::open(_parent,fileSize,numChild);
	if (result != NO_ERR)
		return(result);
		
	result = afterOpen();
	
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::create (const char* fName)
{
	long result = File::create(fName);
	afterOpen();
	return(result);
}

long FitIniFile::createWithCase(const char* fName )
{
	long result = File::createWithCase( fName );
	afterOpen();
	return result;
}

//---------------------------------------------------------------------------
void FitIniFile::close (void)
{
	if (isOpen())
	{
		atClose();
		File::close();
	}
}

//---------------------------------------------------------------------------
long FitIniFile::seekBlock (const char *blockId)
{
	unsigned long blockNum = 0;
	
	while ((blockNum < totalBlocks) && (strcmp(fileBlocks[blockNum].blockId,blockId) != 0))
	{
		blockNum++;
	}
	
	if (blockNum == totalBlocks)
	{
		return(BLOCK_NOT_FOUND);
	}

	//----------------------------------------
	// Block was found, seek to that position
	seek(fileBlocks[blockNum].blockOffset);
	
	//----------------------------------------
	// Setup all current Block Info
	currentBlockId = fileBlocks[blockNum].blockId;
	currentBlockOffset = fileBlocks[blockNum].blockOffset;
	
	blockNum++;
	if (blockNum == totalBlocks)
	{
		currentBlockSize = getLength() - currentBlockOffset;
	}
	else
	{
		currentBlockSize = fileBlocks[blockNum].blockOffset - currentBlockOffset;
	}
		
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdFloat (const char *varName, float &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"f %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0.0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToFloat(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdDouble (const char *varName, double &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"f %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0.0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToDouble(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

long FitIniFile::readIdInt(const char *varName, int &value)
{
    long tmp;
    long rv = readIdLong (varName, tmp);
    value = tmp;
    return rv;
}

//---------------------------------------------------------------------------
long FitIniFile::readIdLong (const char *varName, long &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"l %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToLong(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdBoolean (const char *varName, bool &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"b %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = booleanToLong(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdShort (const char *varName, short &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"s %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToShort(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdChar (const char *varName, char &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"c %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToChar(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

long FitIniFile::readIdULong (const char *varName, DWORD &value)
{
    unsigned long tmp;
    long rv = readIdULong (varName, tmp);
    value = (DWORD)tmp;
    return rv;
}


//---------------------------------------------------------------------------
long FitIniFile::readIdULong (const char *varName, unsigned long &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"ul %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
			char* tc = &line[strlen(searchString)];
			
			while (isspace(*tc))
				tc++;
				
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToULong(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdUShort (const char *varName, unsigned short &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"us %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToUShort(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdUChar (const char *varName, unsigned char &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"uc %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToUChar(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::copyString (char *dest, char *src, unsigned long bufLen)
{
	unsigned long offset = 0;
	//---------------------
	// Find starting Quote
	while (*src != '"')
	{
		src++;
	}
	
	src++;		//One more to get to first character past quote.
	
	//---------------------------------------
	// Copy each character until close quote
	while (*src != '"' && offset < bufLen)
	{
		dest[offset] = *src;
		src++;
		offset++;
	}

	//----------------------------------------------------
	// If this string is longer than buffer, let em know.
	if (offset == bufLen)
	{
		return(BUFFER_TOO_SMALL);
	}

	//---------------------------------
	// otherwise, NULL term and return	
	dest[offset] = '\0';
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdString (const char *varName, char *result, unsigned long bufferSize)
{
	char line[2048];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"st %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,2047);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;

		

		char* pFound = NULL;
		char* pFirstEqual = strstr( equalSign, "\"" );
		// strings can span more than one line, make sure there is another equal sign
		if ( pFirstEqual && !strstr( pFirstEqual+1, "\"" ) ) 
		{
			long curLen = strlen( equalSign );
			char tmpLine[2047];
			do
			{
				readLine((MemoryPtr)tmpLine,2047);
				pFound = strstr( tmpLine, "\"" );

				long addedLen = strlen( tmpLine ) + 1;
				if ( curLen + addedLen < 2048 )
				{
					strcat( equalSign, tmpLine );
					strcat( equalSign, "\n" );
					curLen += addedLen;
				}
				else
					break;

			}while( !pFound && (logicalPosition < endOfBlock) );
		}
		
		long errorCode = copyString(result,equalSign,bufferSize);
		if (errorCode != NO_ERR)
			return(errorCode);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::getIdStringLength (const char *varName)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"st %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((MemoryPtr)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		char* tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}
	
	char *equalSign = strchr(line, '"');
	if (equalSign)
	{
		equalSign++;
		char* end = equalSign;
		while (*end != '"' && *end != 0)
			end++;
		if (*end)
			return (end - equalSign + 1);	// + for terminator
		else
			return(SYNTAX_ERROR);
	}

	return(SYNTAX_ERROR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdFloatArray (const char *varName, float *result, unsigned long numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"f[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	char *equalSign = strstr(line, "=");
	unsigned long elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			long errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((MemoryPtr)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToFloat(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERR);
}
		
//---------------------------------------------------------------------------
long FitIniFile::readIdLongArray (const char *varName, long *result, unsigned long numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"l[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	char *equalSign = strstr(line, "=");
	unsigned long elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			long errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((MemoryPtr)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToLong(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdULongArray (const char *varName, unsigned long *result, unsigned long numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"ul[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	char *equalSign = strstr(line, "=");
	unsigned long elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			long errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((MemoryPtr)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToULong(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERR);
}
		
//---------------------------------------------------------------------------
long FitIniFile::readIdShortArray (const char *varName, short *result, unsigned long numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"s[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	char *equalSign = strstr(line, "=");
	unsigned long elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			long errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((MemoryPtr)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToShort(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdUShortArray (const char *varName, unsigned short *result, unsigned long numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"us[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	char *equalSign = strstr(line, "=");
	unsigned long elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			long errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((MemoryPtr)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToUShort(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERR);
}
		
//---------------------------------------------------------------------------
long FitIniFile::readIdCharArray (const char *varName, char *result, unsigned long numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"c[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	char *equalSign = strstr(line, "=");
	unsigned long elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			long errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((MemoryPtr)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToChar(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERR);
}

//---------------------------------------------------------------------------
long FitIniFile::readIdUCharArray (const char *varName, unsigned char *result, unsigned long numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"uc[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	char *equalSign = strstr(line, "=");
	unsigned long elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			long errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((MemoryPtr)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToUChar(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERR);
}
		
//---------------------------------------------------------------------------
unsigned long FitIniFile::getIdFloatArrayElements (const char *varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"f[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}	
	
//---------------------------------------------------------------------------
unsigned long FitIniFile::getIdLongArrayElements (const char *varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"l[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
unsigned long FitIniFile::getIdULongArrayElements (const char *varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"ul[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}
											 
//---------------------------------------------------------------------------
unsigned long FitIniFile::getIdShortArrayElements (const char *varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"s[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
unsigned long FitIniFile::getIdUShortArrayElements (const char *varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"us[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
unsigned long FitIniFile::getIdCharArrayElements (const char *varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"c[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
unsigned long FitIniFile::getIdUCharArrayElements (const char *varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	unsigned long endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"uc[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	char *fSearch = NULL;
	char *bSearch = NULL;
	
	do
	{
		readLine((MemoryPtr)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	unsigned long actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	long numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}
	
//---------------------------------------------------------------------------
long FitIniFile::writeBlock (const char *blockId)
{
	char thisLine[255];
	sprintf(thisLine,"\r\n[%s]\r\n",blockId);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdFloat (const char *varName, float value)
{
	char thisLine[255];
	sprintf(thisLine,"f %s = %f\r\n",varName,value);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdBoolean (const char *varName, bool value)
{
	char thisLine[255];
	if (value)
		sprintf(thisLine,"b %s = %s\r\n",varName,"TRUE");
	else
		sprintf(thisLine,"b %s = %s\r\n",varName,"FALSE");
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdLong (const char *varName, long value)
{
	char thisLine[255];
	sprintf(thisLine,"l %s = %d\r\n",varName,value);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdShort (const char *varName, short value)
{
	char thisLine[255];
	sprintf(thisLine,"s %s = %d\r\n",varName,value);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdChar (const char *varName, char value)
{
	char thisLine[255];
	sprintf(thisLine,"c %s = %d\r\n",varName,value);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdULong (const char *varName, unsigned long value)
{
	char thisLine[255];
	sprintf(thisLine,"ul %s = %d\r\n",varName,value);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdUShort (const char *varName, unsigned short value)
{
	char thisLine[255];
	sprintf(thisLine,"us %s = %d\r\n",varName,value);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdUChar (const char *varName, unsigned char value)
{
	char thisLine[255];
	sprintf(thisLine,"uc %s = %d\r\n",varName,value);
	
	long result = write((MemoryPtr)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdString (const char *varName, const char *result)
{
	char thisLine[4096];
	char tmpString[4000];
	memset(tmpString,0,4000);
	strncpy(tmpString,result,3999);

	if (strlen(result) >= 4000)
		PAUSE(("String passed to WriteIdString is longer then 4000 characters"));

	sprintf(thisLine,"st %s = \"%s\"\r\n",varName,result);
	
	long bytesWritten = write((MemoryPtr)thisLine,strlen(thisLine));
	return(bytesWritten);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdUShortArray (const char *varName, unsigned short *array, unsigned long numElements)
{
	char thisLine[255];
	sprintf(thisLine,"us[%d] %s = %d,",numElements,varName,array[0]);
	
	long bytesWritten = write((MemoryPtr)thisLine,strlen(thisLine));

	for (long i=1;i<(long)numElements;i++)
	{
		sprintf(thisLine,"%d,",array[i]);
		bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdLongArray (const char *varName, long *array, unsigned long numElements)
{
	char thisLine[255];
	sprintf(thisLine,"l[%d] %s = %d,",numElements,varName,array[0]);
	
	long bytesWritten = write((MemoryPtr)thisLine,strlen(thisLine));

	for (long i=1;i<(long)numElements;i++)
	{
		sprintf(thisLine,"%d,",array[i]);
		bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdFloatArray (const char *varName, float *array, unsigned long numElements)
{
	char thisLine[255];
	sprintf(thisLine,"f[%d] %s = %.2f,",numElements,varName,array[0]);
	
	long bytesWritten = write((MemoryPtr)thisLine,strlen(thisLine));

	for (long i=1;i<(long)numElements;i++)
	{
		sprintf(thisLine," %.2f,",array[i]);
		bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
long FitIniFile::writeIdUCharArray (const char *varName, unsigned char *array, unsigned long numElements)
{
	char thisLine[255];
	sprintf(thisLine,"uc[%d] %s = %d,",numElements,varName,array[0]);
	
	long bytesWritten = write((MemoryPtr)thisLine,strlen(thisLine));

	for (long i=1;i<(long)numElements;i++)
	{
		sprintf(thisLine," %d,",array[i]);
		bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((MemoryPtr)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
//
// Edit log
//
//---------------------------------------------------------------------------
