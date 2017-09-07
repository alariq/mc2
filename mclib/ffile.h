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

#define FASTFILE_ENTRY_TABLE_START 8 //sebi


#pragma pack(1)
typedef struct 
{
    DWORD	offset;
    DWORD	size;						//LZ Compressed Size
	DWORD	realSize;					//Uncompressed Size
	DWORD	hash;						//Hash Compare to weed out stinky files faster then StrCmp
    char	name[MAX_FILENAME_SIZE];
} FILEENTRY;
#pragma pack()

typedef struct 
{
    DWORD       inuse;
    DWORD       pos;
    FILEENTRY	*pfe;
} FILE_HANDLE;

//---------------------------------------------------------------------------
// Class FastFile
class FastFile
{
	protected:
		DWORD		numFiles;
		FILE_HANDLE	*files;

		char 		*fileName;
		FILE		*handle;

		DWORD 		length;
		DWORD 		logicalPosition;

		bool		useLZCompress;

		// used when creating fast file
		int						numWrittenFiles;

		long writeVersion(FILE* handle);
		long writeNumFiles(FILE* handle, int num_files);
		long writeFileEntries(FILE* handle, FILE_HANDLE* files, int num_files, int offset);

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
		const FILE_HANDLE* getFilesInfo() const { return files; }

		const char* getFileName() { return fileName; }

		long openFast (DWORD hash, const char *fName);

		void closeFast (DWORD localHandle);

		long seekFast (DWORD fastFileHandle, DWORD off, DWORD from = SEEK_SET);
		long readFast (DWORD fastFileHandle, void *bfr, DWORD size);
		long readFastRAW (DWORD fastFileHandle, void *bfr, DWORD size);
		long tellFast (DWORD fastFileHandle);
		long sizeFast (DWORD fastFileHandle);
		long lzSizeFast (DWORD fastFileHandle);

		bool isLZCompressed (void)
		{
			return useLZCompress;
		}


		long create(const char* fName, bool compressed);
		long reserve(int num_files);
		long writeFast (const char* fastFileName, void* buffer, int nbytes);
};

//---------------------------------------------------------------------------
extern FastFile 	**fastFiles;
extern long 		numFastFiles;
extern long			maxFastFiles;
//---------------------------------------------------------------------------
#endif

