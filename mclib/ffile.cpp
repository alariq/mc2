//---------------------------------------------------------------------------
//
// ffile.cpp - This file contains the class functions for FastFile
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files
#ifndef FFILE_H
#include"ffile.h"
#endif

#ifndef UTILITIES_H
#include"utilities.h"
#endif

#include"lz.h"

#include<zlib/zlib.h>



#include<windows.h>

#include<string.h>
#include"platform_str.h"
#include<gameos.hpp>

MemoryPtr 		LZPacketBuffer = NULL;
unsigned int	LZPacketBufferSize = 512000;

extern char CDInstallPath[];
void EnterWindowMode();
void EnterFullScreenMode();
void __stdcall ExitGameOS();

char FileMissingString[512];
char CDMissingString[1024];
char MissingTitleString[256];
//---------------------------------------------------------------------------
//	class FastFile member functions
//---------------------------------------------------------------------------
void *FastFile::operator new (size_t mySize)
{
	void *result = NULL;
	result = malloc(mySize);
	
	return(result);
}

//---------------------------------------------------------------------------
void FastFile::operator delete (void *us)
{
	::free(us);
}

//---------------------------------------------------------------------------
FastFile::FastFile (void)
{
	files = NULL;
	numFiles = 0;

	fileName = NULL;
	handle = NULL;
	length = 0;
	logicalPosition = 0;

	useLZCompress = false;
}
			
//---------------------------------------------------------------------------
FastFile::~FastFile (void)
{
	close();

	if (LZPacketBuffer)
	{
		free(LZPacketBuffer);
		LZPacketBuffer = NULL;
	}
}

//---------------------------------------------------------------------------
long FastFile::open (const char* fName)
{
	//-------------------------------------------------------------
	long fNameLength = strlen(fName);
	fileName = new char [fNameLength+1];

	if (!fileName)
		return(NO_RAM_FOR_FILENAME);
		
	strncpy(fileName,fName,fNameLength+1);

	handle = fopen(fileName,"rb");
	if (handle != NULL)
	{
		logicalPosition = 0;
		fileSize();				//Sets Length
	}
	else
	{
		//Check on CD.  They may not have been installed.
		char actualPath[2048];
		strcpy(actualPath,CDInstallPath);
		strcat(actualPath,fileName);
		handle = fopen(actualPath,"r");
		if (handle == NULL)
		{
			//OPEN Error.  Maybe the CD is missing?
			bool openFailed = false;
			bool alreadyFullScreen = (Environment.fullScreen != 0);
			while (handle == NULL)
			{
				openFailed = true;
				EnterWindowMode();

				char data[2048];
				sprintf(data,FileMissingString,fileName,CDMissingString);
				DWORD result1 = MessageBox(NULL,data,MissingTitleString,MB_OKCANCEL | MB_ICONWARNING);
				if (result1 == IDCANCEL)
				{
					ExitGameOS();
					return (2);		//File not found.  Never returns though!
				}

				handle = fopen(actualPath,"r");
			}

			if (openFailed && (Environment.fullScreen == 0) && alreadyFullScreen)
				EnterFullScreenMode();
		}
		else
		{
			logicalPosition = 0;
			// sebi
			// NB!: this is incorrect, because if we opened file fromCD we need to change its name(path), otherwise, size wil be incorrect!
			fileSize();				//Sets Length
		}
	}

	//---------------------------------------------
	//-- Read in ALL of the file names and offsets
	logicalPosition = fseek(handle,0,SEEK_SET);

	//---------------------------------------------
	//-- First Long is Version Number of FastFile
	unsigned long result = 0;
	unsigned long version = 0;

    // sebi
	//result = fread((&version),1,sizeof(long),handle);
    //logicalPosition += sizeof(long);
	//if (result != sizeof(long))
	result = fread((&version),1, FASTFILE_VERSION_BYTESIZE,handle);
    logicalPosition += FASTFILE_VERSION_BYTESIZE;

	if (result != FASTFILE_VERSION_BYTESIZE)
	{
		long lastError = errno;
		return lastError;
	}

	if (version != FASTFILE_VERSION && version != FASTFILE_VERSION_LZ)
		return FASTFILE_VERSION;

	if (version == FASTFILE_VERSION_LZ)
		useLZCompress = true;

	//---------------------------------------------
	//-- Second Long is number of filenames present.
	result = fread((&numFiles),1, 4,handle);

	logicalPosition += 4;

	if (result != 4)
	{
		long lastError = errno;
		return lastError;
	}

	files = (FILE_HANDLE*)malloc(sizeof(FILE_HANDLE) * numFiles);

	for (DWORD i=0;i<numFiles;i++)
	{
		files[i].pfe = (FILEENTRY *)malloc(sizeof(FILEENTRY));
		memset(files[i].pfe,0,sizeof(FILEENTRY));

		unsigned long result = 0;
		result = fread(files[i].pfe,1,sizeof(FILEENTRY),handle);

		files[i].inuse = FALSE;
		files[i].pos = 0;
	}

	return (result);
}
		
//---------------------------------------------------------------------------
void FastFile::close (void)
{
	//------------------------------------------------------------------------
	// The actual stored filename is also in the parent.  Everyone else just has
	// pointer and, as such, only the parent frees the memory.
	if (fileName)
		delete [] fileName;	//	this was free, which didn't match the new allocation.
							//	neither new nor free were overridden. Should they have been?
	fileName = NULL;
	length = 0;

	if (isOpen())
	{
		fclose(handle);
		handle = NULL;
	}

	//---------------------------------------------
	//-- First Long is number of filenames present.
	for (DWORD i=0;i<numFiles;i++)
	{
		free(files[i].pfe);
	}

	free(files);

	files = NULL;
	numFiles = 0;
}

//---------------------------------------------------------------------------
long FastFile::openFast (DWORD hash, const char *fName)
{
	//------------------------------------------------------------------
	//-- In order to use this, the file name must be part of the index.
	for (DWORD i=0;i<numFiles;i++)
	{
		if ((hash == files[i].pfe->hash) && (S_stricmp(files[i].pfe->name,fName) == 0))
		{
			files[i].inuse = TRUE;
			files[i].pos = 0;
			return i;
		}
	}

	return -1;
}

//---------------------------------------------------------------------------
void FastFile::closeFast (DWORD fastFileHandle)
{
	if ((fastFileHandle >= 0) && (fastFileHandle < numFiles) && files[fastFileHandle].inuse)
	{
		files[fastFileHandle].inuse = FALSE;
		files[fastFileHandle].pos = 0;
	}
}

//---------------------------------------------------------------------------
long FastFile::seekFast (DWORD fastFileHandle, DWORD off, DWORD how)
{
	if ((fastFileHandle >= 0) && (fastFileHandle < numFiles) && files[fastFileHandle].inuse)
	{
		switch (how)
		{
			case SEEK_SET:
				if (off > files[fastFileHandle].pfe->size)
				{
					return READ_PAST_EOF_ERR;
				}
				break;
	
			case SEEK_END:
				if (((DWORD)abs((long)off) > files[fastFileHandle].pfe->size) || (off > 0))
				{
					return READ_PAST_EOF_ERR;
				}
				break;
	
			case SEEK_CUR:
				if (off+files[fastFileHandle].pos > files[fastFileHandle].pfe->size)
				{
					return READ_PAST_EOF_ERR;
				}
				break;
		}

		long newPosition = 0;
		switch (how)
		{
			case SEEK_SET:
				newPosition = off;
			break;

			case SEEK_END:
				newPosition = files[fastFileHandle].pfe->size;
				newPosition += off;
			break;

			case SEEK_CUR:
				newPosition = off+files[fastFileHandle].pos;
			break;
		}
			
		if (newPosition == -1)
		{
			return (INVALID_SEEK_ERR);
		}

		files[fastFileHandle].pos = newPosition;

		//-----------------------------------
		//-- Now macro seek the entire file.
		if (fseek(handle,files[fastFileHandle].pos + files[fastFileHandle].pfe->offset,SEEK_SET) == 0)
			logicalPosition = ftell(handle);

		return (files[fastFileHandle].pos);
	}

	return (FILE_NOT_OPEN);
}

//---------------------------------------------------------------------------
long FastFile::readFast (DWORD fastFileHandle, void *bfr, DWORD size)
{
	size;

	long result = 0;

	if ((fastFileHandle >= 0) && (fastFileHandle < numFiles) && files[fastFileHandle].inuse)
	{
		logicalPosition = fseek(handle,files[fastFileHandle].pos + files[fastFileHandle].pfe->offset,SEEK_SET);

		//ALL files in the fast file are now zLib compressed. NO EXCEPTIONS!!
		// This fixes a bug where the zLib Compressed version is the same length
		// as the raw version.  Yikes but this is rare.  Finally happened though!
		// -fs
		/*
		if (files[fastFileHandle].pfe->size == files[fastFileHandle].pfe->realSize)
		{
			result = fread(bfr,1,size,handle);
			logicalPosition += size;

			if (result != size)
			{
				long lastError = errno;
				return lastError;
			}
		}
		else			//File is NOW zLib Compressed.  Read In Appropriately
		*/
		{
			if (!LZPacketBuffer)
			{
				LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
				if (!LZPacketBuffer)
					return 0;
			}
				
			if ((DWORD)LZPacketBufferSize < files[fastFileHandle].pfe->size)
			{
				LZPacketBufferSize = files[fastFileHandle].pfe->size;
				
				free(LZPacketBuffer);
				LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
				if (!LZPacketBuffer)
					return 0;
			}
			
			if (LZPacketBuffer)
			{
				result = fread(LZPacketBuffer,1,files[fastFileHandle].pfe->size,handle);
				logicalPosition += files[fastFileHandle].pfe->size;

				if (result != files[fastFileHandle].pfe->size)
				{
					//READ Error.  Maybe the CD is missing?
					bool openFailed = false;
					bool alreadyFullScreen = (Environment.fullScreen != 0);
					while (result != files[fastFileHandle].pfe->size)
					{
						openFailed = true;
						EnterWindowMode();
		
						char data[2048];
						sprintf(data,FileMissingString,fileName,CDMissingString);
						DWORD result1 = MessageBox(NULL,data,MissingTitleString,MB_OKCANCEL | MB_ICONWARNING);
						if (result1 == IDCANCEL)
						{
							ExitGameOS();
							return (2);		//File not found.  Never returns though!
						}
		
						logicalPosition = fseek(handle,files[fastFileHandle].pos + files[fastFileHandle].pfe->offset,SEEK_SET);
						result = fread(LZPacketBuffer,1,files[fastFileHandle].pfe->size,handle);
						logicalPosition += files[fastFileHandle].pfe->size;
					}
		
					if (openFailed && (Environment.fullScreen == 0) && alreadyFullScreen)
						EnterFullScreenMode();
				}

				//--------------------------------------------------------
				//USED to LZ Compress here.  It is NOW zLib Compression.
				//  We should not try to use old fastfiles becuase version check above should fail when trying to open!!
				unsigned long decompLength = 0;
				if (useLZCompress)
				{
					decompLength = LZDecomp((MemoryPtr)bfr,LZPacketBuffer,files[fastFileHandle].pfe->size);
				}
				else
				{
					decompLength = files[fastFileHandle].pfe->realSize;
					long error = uncompress((MemoryPtr)bfr,&decompLength,LZPacketBuffer,files[fastFileHandle].pfe->size);
					if (error != Z_OK)
						STOP(("Error %d UnCompressing File %s from FastFile %s",error,files[fastFileHandle].pfe->name,fileName));
				}

				if ((long)decompLength != files[fastFileHandle].pfe->realSize)
					result = 0;
				else
					result = decompLength;
			}
		}

		return result;
	}

	return FILE_NOT_OPEN;
}

//---------------------------------------------------------------------------
// This function pulls the raw compressed data out of the file and sticks it in the buffer
// passed in.  This way, we can load the textures directly from file to RAM and not
// have to decompress them!!
long FastFile::readFastRAW (DWORD fastFileHandle, void *bfr, DWORD size)
{
	size;

	long result = 0;

	if ((fastFileHandle >= 0) && (fastFileHandle < numFiles) && files[fastFileHandle].inuse)
	{
		logicalPosition = fseek(handle,files[fastFileHandle].pos + files[fastFileHandle].pfe->offset,SEEK_SET);

		if (size >= files[fastFileHandle].pfe->size)
		{
			result = fread(bfr,1,files[fastFileHandle].pfe->size,handle);
			logicalPosition += files[fastFileHandle].pfe->size;
		}

		if (result != files[fastFileHandle].pfe->size)
		{
			//READ Error.  Maybe the CD is missing?
			bool openFailed = false;
			bool alreadyFullScreen = (Environment.fullScreen != 0);
			while (result != files[fastFileHandle].pfe->size)
			{
				openFailed = true;
				EnterWindowMode();

				char data[2048];
				sprintf(data,FileMissingString,fileName,CDMissingString);
				DWORD result1 = MessageBox(NULL,data,MissingTitleString,MB_OKCANCEL | MB_ICONWARNING);
				if (result1 == IDCANCEL)
				{
					ExitGameOS();
					return (2);		//File not found.  Never returns though!
				}

				logicalPosition = fseek(handle,files[fastFileHandle].pos + files[fastFileHandle].pfe->offset,SEEK_SET);
				result = fread(bfr,1,files[fastFileHandle].pfe->size,handle);
				logicalPosition += files[fastFileHandle].pfe->size;
			}

			if (openFailed && (Environment.fullScreen == 0) && alreadyFullScreen)
				EnterFullScreenMode();
		}

		return result;
	}

	return FILE_NOT_OPEN;
}

//---------------------------------------------------------------------------
long FastFile::tellFast (DWORD fastFileHandle)
{
	if ((fastFileHandle >= 0) && (fastFileHandle < numFiles) && files[fastFileHandle].inuse)
		return files[fastFileHandle].pos;

	return -1;
}

//---------------------------------------------------------------------------
long FastFile::sizeFast (DWORD fastFileHandle)
{
	if ((fastFileHandle >= 0) && (fastFileHandle < numFiles) && files[fastFileHandle].inuse)
		return files[fastFileHandle].pfe->realSize;

	return -1;
}

//---------------------------------------------------------------------------
long FastFile::lzSizeFast (DWORD fastFileHandle)
{
	if ((fastFileHandle >= 0) && (fastFileHandle < numFiles) && files[fastFileHandle].inuse)
		return files[fastFileHandle].pfe->size;

	return -1;
}

//---------------------------------------------------------------------------
//
// Edit Log
//
//
//---------------------------------------------------------------------------
