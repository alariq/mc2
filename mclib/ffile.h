//---------------------------------------------------------------------------
//
// ffile.h - This file contains the class declaration for FastFile
//
// This class will catch a fileOpen and try to find the file in the FastFiles
// open before allowing the call to go through to the system
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef FFILE_H
#define FFILE_H
//---------------------------------------------------------------------------
// Include files

#ifndef DSTD_H
#include<dstd.h>
#endif

#ifndef FILE_H
#include<file.h>
#endif

#include<sys/types.h>
#include<sys/stat.h>
//---------------------------------------------------------------------------
#define MAX_FILENAME_SIZE		250

#define FASTFILE_VERSION_BYTESIZE   4 //sebi
#define FASTFILE_VERSION		0xCADDECAF
#define FASTFILE_VERSION_LZ		0xFADDECAF

#pragma pack(1)
typedef struct 
{
    long	offset;
    long	size;						//LZ Compressed Size
	long	realSize;					//Uncompressed Size
	DWORD	hash;						//Hash Compare to weed out stinky files faster then StrCmp
    char	name[MAX_FILENAME_SIZE];
} FILEENTRY;
#pragma pack()

typedef struct 
{
    long		inuse;
    long		pos;
    FILEENTRY	*pfe;
} FILE_HANDLE;

//---------------------------------------------------------------------------
// Class FastFile
class FastFile
{
	protected:
		long					numFiles;
		FILE_HANDLE				*files;

		char 					*fileName;
		FILE					*handle;

		unsigned long 			length;
		unsigned long 			logicalPosition;

		bool					useLZCompress;

	public:
		FastFile (void);
		~FastFile (void);

		void *operator new (size_t mySize);
		void operator delete (void *us);

		long open (const char* fName);
		void close (void);

		bool isOpen (void)
		{
			return (handle != NULL);
		}

		long fileSize (void)
		{
			if (isOpen() && (length == 0))
			{
				struct stat st;
				stat(fileName,&st);
				length = st.st_size;
			}

			return length;			
		}

		long getNumFiles (void)
		{
			return numFiles;
		}

		long openFast (DWORD hash, const char *fName);

		void closeFast (long localHandle);

		long seekFast (long fastFileHandle, long off, long from = SEEK_SET);
		long readFast (long fastFileHandle, void *bfr, long size);
		long readFastRAW (long fastFileHandle, void *bfr, long size);
		long tellFast (long fastFileHandle);
		long sizeFast (long fastFileHandle);
		long lzSizeFast (long fastFileHandle);

		bool isLZCompressed (void)
		{
			return useLZCompress;
		}
};

//---------------------------------------------------------------------------
extern FastFile 	**fastFiles;
extern long 		numFastFiles;
extern long			maxFastFiles;
//---------------------------------------------------------------------------
#endif

