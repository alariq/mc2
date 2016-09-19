#pragma once
//==========================================================================//
// File:	 FileIO.hpp														//
// Contents: file i/o routines												//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//




void Init_FileSystem();
void Destory_FileSystem( bool NoErrors );




#ifdef LAB_ONLY

//
// Debugging information
//
extern char FileInfo[32][MAX_PATH+32];
extern DWORD CurrentFileInfo;
#endif



//
// Memory mapped file structure
//
typedef struct _MemoryMappedFiles
{

	DWORD Magic;						// Identify this structure
	_MemoryMappedFiles*	pNext;			// Pointer to next structure
	HANDLE hFile;						// File handle
	HANDLE hFileMapping;				// Mapping handle
	BYTE*  pFile;						// Pointer to start of data
	DWORD  Size;						// Size of data
	DWORD  RefCount;					// Reference count (number of times a file is opened)
	char   Name[MAX_PATH];				// Copy of the file name

} MemoryMappedFiles;



struct gosFileStream
{
	gosEnum_FileWriteStatus m_writeEnabled;
	HANDLE m_hFile;
	char m_Filename[MAX_PATH];
	gosFileStream* pNext;

	gosFileStream( const char *FileName, gosEnum_FileWriteStatus fwstatus );

	~gosFileStream();

	DWORD Seek( int where, gosEnum_FileSeekType from_end );

	DWORD Read( void *buffer, DWORD length );

	DWORD Write( const void *buffer, DWORD length );

	DWORD BytesTransfered;
};



extern MemoryMappedFiles*	ListofMemoryMappedFiles;
extern gosFileStream* ListOfFiles;
extern DWORD NumberMemoryMappedFilesOpen;
extern DWORD NumberFilesOpen;
