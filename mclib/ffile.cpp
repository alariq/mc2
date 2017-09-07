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

#ifndef FASTFILE_H
#include "fastfile.h"
#endif

#ifndef UTILITIES_H
#include"utilities.h"
#endif

#include"lz.h"

#include<zlib.h>

#include"platform_windows.h"
#include"platform_io.h"

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

	numWrittenFiles = 0;
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

long FastFile::writeVersion(FILE* handle)
{
	fseek(handle, 0, SEEK_SET);
	int version = useLZCompress ? FASTFILE_VERSION_LZ : FASTFILE_VERSION;
	int result = fwrite(&version, 1, 4, handle);  
	logicalPosition += result;

	if (result != 4)
	{
		long lastError = errno;
		return lastError;
	}

	return NO_ERR;
}

long FastFile::writeNumFiles(FILE* handle, int num_files)
{
	fseek(handle, 4, SEEK_SET);
	int result = fwrite(&numFiles, 1, 4, handle);
	if (result != 4)
	{
		long lastError = errno;
		return lastError;
	}

	return NO_ERR;
}

long FastFile::writeFileEntries(FILE* handle, FILE_HANDLE* files, int num_files, int offset)
{
	fseek(handle, FASTFILE_ENTRY_TABLE_START + offset * sizeof(FILEENTRY), SEEK_SET);
	int result  = 0;
	for (int i=0;i<num_files;i++)
	{
		result += fwrite(files[i].pfe, 1, sizeof(FILEENTRY), handle);
	}

	return result == sizeof(FILEENTRY) * num_files ? NO_ERR : BAD_WRITE_ERR;
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

	return (0);
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
long FastFile::create(const char* fName, bool compressed)
{
	if(handle)
		return FILE_ALREADY_OPEN;

	//-------------------------------------------------------------
	long fNameLength = strlen(fName);
	fileName = new char [fNameLength+1];

	if (!fileName) {
        delete[] fileName;
		return(NO_RAM_FOR_FILENAME);
    }
		
	strncpy(fileName,fName,fNameLength+1);

	handle = fopen(fileName, "wb");
	if (handle != NULL)
	{
		logicalPosition = 0;
		fileSize();				//Sets Length
	}

	useLZCompress = compressed;

	long res = writeVersion(handle);
	if(NO_ERR != res) {
        delete[] fileName;
		return res;
    }

	res = writeNumFiles(handle, 0);

	logicalPosition = ftell(handle);
		
	return NO_ERR;
}

long FastFile::reserve(int num_files)
{
	numWrittenFiles = 0;

	if(files) {

		for (int i=0; i<numFiles; i++) {
			free(files[i].pfe);
		}
		free(files);
	}
	numFiles = num_files;

	long res = writeNumFiles(handle, numFiles);
	if(NO_ERR != res)
		return res;

	files = (FILE_HANDLE*)malloc(sizeof(FILE_HANDLE) * numFiles);

	for (long i=0;i<numFiles;i++)
	{
		files[i].pfe = (FILEENTRY *)calloc(1, sizeof(FILEENTRY));
		files[i].inuse = FALSE;
		files[i].pos = 0;
	}

	res = writeFileEntries(handle, files, numFiles, 0);
	if(NO_ERR != res)
		return res;

	logicalPosition = ftell(handle);

	return NO_ERR;
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

				//sebi: second cndiotion to handle zero-length files
				if (result != files[fastFileHandle].pfe->size && files[fastFileHandle].pfe->size>0)
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
long FastFile::writeFast (const char* fastFileName, void* buffer, int nbytes)
{
	if(handle == NULL)
		return FILE_NOT_OPEN;
	
	if (numWrittenFiles >= numFiles)
		return TOO_MANY_CHILDREN;

	const int fastFileHandle = numWrittenFiles;

	fseek(handle, 0, SEEK_END);
	const int file_pos = ftell(handle);

	files[fastFileHandle].inuse = true;
	files[fastFileHandle].pos = 0;
	files[fastFileHandle].pfe->offset = file_pos;

	strncpy(files[fastFileHandle].pfe->name, fastFileName, MAX_FILENAME_SIZE-1);
	files[fastFileHandle].pfe->name[MAX_FILENAME_SIZE-1] = '\0';
	files[fastFileHandle].pfe->hash = elfHash(files[fastFileHandle].pfe->name);
	files[fastFileHandle].pfe->realSize = nbytes;

	fseek(handle, file_pos, SEEK_SET);

	if(useLZCompress)
	{
		unsigned long workBufferSize = (nbytes << 1);
		workBufferSize = workBufferSize < 4096 ? 4096 : workBufferSize;

		if (!LZPacketBuffer)
		{
			LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
			if (!LZPacketBuffer)
				return 0;
		}
				
		if (LZPacketBufferSize < workBufferSize)
		{
			LZPacketBufferSize = workBufferSize;
				
			free(LZPacketBuffer);
			LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
			if (!LZPacketBuffer)
				return 0;
		}

		gosASSERT(LZPacketBuffer);

		size_t compressedSize = LZCompress(LZPacketBuffer, (Bytef*)buffer, nbytes);
		size_t uncompressedSize = LZDecomp((Bytef*)buffer, LZPacketBuffer, compressedSize);
		if (nbytes != uncompressedSize)
			STOP(("fast File size changed after compression.  Was %d is now %d", nbytes, uncompressedSize));

		files[fastFileHandle].pfe->size = compressedSize;
	
		// write file itself
		int result = fwrite(LZPacketBuffer, compressedSize, 1, handle);
		if(result != 1)
			return BAD_WRITE_ERR;

		if(0)
		{
			int file = creat("d:/ffile.bin", S_IWRITE);
			//FILE* file = fopen("d:/ffile.bin", "wb");
			int result = write(file, LZPacketBuffer, compressedSize);
			_close(file);

			char* fbc = new char[compressedSize];
			char* fb = new char[nbytes];

			file = _open("d:/ffile.bin", _O_RDONLY | _O_BINARY);
			result = _read(file, fbc, compressedSize);
			_close(file);

			gosASSERT(memcmp(LZPacketBuffer, fbc, compressedSize) == 0);

			size_t uncompressedSize = LZDecomp((Bytef*)fb, (Bytef*)fbc, compressedSize);
			gosASSERT(nbytes == uncompressedSize);
		}
	}
	else
	{
		files[fastFileHandle].pfe->size = nbytes;
	
		// write file itself
		int result = fwrite(buffer, nbytes, 1, handle);
		if(result != 1 && nbytes > 0) // to handle case when input file has zero length
			return BAD_WRITE_ERR;
	}

	long res = writeFileEntries(handle, &files[fastFileHandle], 1, fastFileHandle);
	if(NO_ERR != res)
		return res;

	numWrittenFiles++;

	return NO_ERR;
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
