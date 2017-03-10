//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------------------
// Global Fast File Code.
//

#include"heap.h"
#include"fastfile.h"
#include<ctype.h>

long ffLastError = 0;

#define NO_ERR		0
//-----------------------------------------------------------------------------------
bool FastFileInit (const char *fname)
{
	if (numFastFiles == maxFastFiles)
	{
		ffLastError = -1;
		return FALSE;
	}

	//-----------------------------------------------------------------------------
	//-- Open this fast file, add it to the list O pointers and return TRUE if OK!
	fastFiles[numFastFiles] = new FastFile;
	long result = fastFiles[numFastFiles]->open(fname);
	if (result == FASTFILE_VERSION)
	{
		ffLastError = result;
		return FALSE;
	}

	numFastFiles++;

	return TRUE;
}

//-----------------------------------------------------------------------------------
void FastFileFini (void)
{
	if (fastFiles)
	{
		long currentFastFile = 0;
		while (currentFastFile < maxFastFiles)
		{
			if (fastFiles[currentFastFile])
			{
				fastFiles[currentFastFile]->close();
	
				delete fastFiles[currentFastFile];
				fastFiles[currentFastFile] = NULL;
			}
	
			currentFastFile++;
		}
	}

	free(fastFiles);
	fastFiles = NULL;
	numFastFiles= 0;
}

//-----------------------------------------------------------------------------------
FastFile *FastFileFind (const char *fname, long &fastFileHandle)
{
	if (fastFiles)
	{
		DWORD thisHash = elfHash(fname);
		long currentFastFile = 0;
		long tempHandle = -1;
		while (currentFastFile < numFastFiles)
		{
			tempHandle = fastFiles[currentFastFile]->openFast(thisHash,fname);
			if (tempHandle != -1)
			{
				fastFileHandle = tempHandle;
				return fastFiles[currentFastFile];
			}

			currentFastFile++;
		}

        // sebi NB!
        // if we are here file was not found, try with backslashes
        char* fname2 = _strdup(fname);
        int len = (int)strlen(fname2);
        for(int i=0;i<len;++i) {
            if(fname2[i]=='/')
                fname2[i] = '\\';
        }

		thisHash = elfHash(fname2);
		currentFastFile = 0;
		tempHandle = -1;
		while (currentFastFile < numFastFiles)
		{
			tempHandle = fastFiles[currentFastFile]->openFast(thisHash,fname2);
			if (tempHandle != -1)
			{
				fastFileHandle = tempHandle;
				return fastFiles[currentFastFile];
			}

			currentFastFile++;
		}


	}


	return NULL;
}

//------------------------------------------------------------------
DWORD elfHash (const char *name)
{
    unsigned long   h = 0, g;
    while ( *name )
    {
        h = ( h << 4 ) + *name++;
        if ( (g = h & 0xF0000000) )
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

//-----------------------------------------------------------------------------------




