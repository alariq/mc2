//---------------------------------------------------------------------------
//
// file.h - This file contains the class declaration for File
//
//				The File class simply calls the Windows 32bit file functions.
//				It is purely a wrapper.
//
//				The mmFile Class is a wrapper for the Win32 Memory Mapped
//				file functionality.  It is used exactly the same as above class.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef FILE_H
#define FILE_H
//---------------------------------------------------------------------------
// Include files

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef DFILE_H
#include"dfile.h"
#endif

#ifndef DFFILE_H
#include"dffile.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>

//---------------------------------------------------------------------------
// Enums
enum FileMode
{
	NOMODE = 0,
	READ,
	CREATE,
	MC2_APPEND,
	WRITE,
	RDWRITE
};

enum FileClass
{
	BASEFILE = 0,
	INIFILE,
	PACKETFILE,
	CSVFILE
};

//---------------------------------------------------------------------------
//Returns 1 if file is on HardDrive and 2 if file is in FastFile
#define FILE_NOT_FOUND	0
#define FILE_ON_DISK	1
#define FILE_ON_FST		2

// Function Declarations
long fileExists(const char *fName, long destination_mask = FILE_ON_DISK|FILE_ON_FST);
long fileExistsOnCD(const char *fName);
bool file1OlderThan2(const char *file1, const char* file2);

//---------------------------------------------------------------------------
// Macro Definitions
#ifndef	NO_ERR
#define	NO_ERR					0x00000000
#endif

#define	DISK_FULL_ERR			0xBADF0001
#define	SHARE_ERR				0xBADF0002
#define	FILE_NOT_FOUND_ERR		0xBADF0003
#define	PACKET_OUT_OF_RANGE		0xBADF0004
#define	PACKET_WRONG_SIZE		0xBADF0005
#define	READ_ONLY_ERR			0xBADF0006
#define	TOO_MANY_FILES_ERR		0xBADF0007
#define	READ_PAST_EOF_ERR		0xBADF0008
#define	INVALID_SEEK_ERR		0xBADF0009
#define	BAD_WRITE_ERR			0xBADF000A
#define	BAD_PACKET_VERSION		0xBADF000B
#define	NO_RAM_FOR_SEEK_TABLE	0xBADF000C
#define	NO_RAM_FOR_FILENAME		0xBADF000D
#define	PARENT_NULL				0xBADF000E
#define	TOO_MANY_CHILDREN		0xBADF000F
#define	FILE_NOT_OPEN			0xBADF0010
#define	CANT_WRITE_TO_CHILD		0xBADF0011
#define	NO_RAM_FOR_CHILD_LIST	0xBADF0012
#define MAPPED_WRITE_NOT_SUPPORTED	0xBADF0013
#define COULD_NOT_MAP_FILE		0xBADF0014
#define FILE_ALREADY_OPEN       0xBADF0015  //sebi

//---------------------------------------------------------------------------
//									File
//---------------------------------------------------------------------------
class File
{
	// Data Members
	//--------------
	protected:
	
		char 					*fileName;
		FileMode				fileMode;
	
		int                     handle;

		FastFilePtr				fastFile;
		long					fastFileHandle;

		unsigned long 			length;
		unsigned long 			logicalPosition;

		unsigned long			bufferResult;

		FilePtr					*childList;
		unsigned long			numChildren;
		unsigned long			maxChildren;
				
		FilePtr					parent;
		unsigned long			parentOffset;
		unsigned long			physicalLength;

		bool					inRAM;
		MemoryPtr				fileImage;

	public:

		static bool				logFileTraffic;
		static unsigned long	lastError;

	// Member Functions
	//------------------
		protected:

			void setup (void);

		public:

			void *operator new (size_t mySize);
			void operator delete (void *us);
			
			File (void);
			~File (void);

			bool eof (void);

			virtual long open ( const char* fName, FileMode _mode = READ, long numChildren = 50, bool doNotLower = false);
			virtual long open( const char* buffer, int bufferLength ); // for streaming from memory

			virtual long create (const char* fName);
			virtual long createWithCase(const char* fName ); // don't strlwr for me please!

			virtual void close (void);

			virtual long open (File *_parent, unsigned long fileSize, long numChildren = 50);
			
			void deleteFile (void);

			long seek (long pos, long from = SEEK_SET);
			void seekEnd (void);
			void skip (long bytesToSkip);

			long read (unsigned long pos, MemoryPtr buffer, long length);
			long read (MemoryPtr buffer, long length);

			//Used to dig the LZ data directly out of the fastfiles.
			// For textures.
			//long readRAW (unsigned long * &buffer, UserHeapPtr heap);
			long readRAW (DWORD* &buffer, UserHeapPtr heap);

			unsigned char readByte (void);
			short readWord (void);
			short readShort (void);
			long readLong (void);
			float readFloat( void );
			int readInt( void );

			long readString (MemoryPtr buffer);
			long readLine (MemoryPtr buffer, long maxLength);
			long readLineEx (MemoryPtr buffer, long maxLength);

			long write (unsigned long pos, MemoryPtr buffer, long bytes);
			long write (MemoryPtr buffer, size_t bytes);

			long writeByte (unsigned char value);
			long writeWord (short value);
			long writeShort (short value);
			long writeInt(int value);
			long writeLong (long value);
			long writeFloat (float value);

			long writeString (const char *buffer);
			long writeLine (char *buffer);

			bool isOpen (void);

			virtual FileClass getFileClass (void)
			{
				return BASEFILE;
			}

			char* getFilename (void);

			unsigned long getLength (void);
			unsigned long fileSize (void);
			unsigned long getNumLines (void);

			unsigned long getLastError (void) {
				return(lastError);
			}

			unsigned long getLogicalPosition (void)
			{
				return logicalPosition;
			}
			
			FilePtr getParent (void)
			{
				return parent;
			}

			FileMode getFileMode (void)
			{
				return(fileMode);
			}
			
			int getFileHandle (void)
			{
				return(handle);
			}

			time_t getFileMTime (void);
			
			long addChild (FilePtr child);
			void removeChild (FilePtr child);

};

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//
//---------------------------------------------------------------------------
