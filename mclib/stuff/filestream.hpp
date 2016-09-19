//===========================================================================//
// File:	filestrm.hpp                                                     //
// Contents: Implementation Details of resource management                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"memorystream.hpp"

namespace Stuff {

	class FileStreamManager;
	class FileStream;

	//##########################################################################
	//##########################    Directory    ###############################
	//##########################################################################

	class Directory
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		Directory(
			char *find_files,
			bool find_directories=false
		);
		~Directory();
		
		void
			TestInstance() const
				{}

		char*
			GetCurrentFileName();
		void
			AdvanceCurrentFile();
		char*
			GetCurrentFolderName();
		void
			AdvanceCurrentFolder();

	public:
		int 
			fileOk;

		Stuff::MString
			currentDirectoryString;

		typedef Stuff::PlugOf<Stuff::MString>
			DirectoryEntry;
		typedef Stuff::PlugOf<Stuff::MString>
			DirectoryFolder;


		Stuff::SortedChainOf<DirectoryFolder*, Stuff::MString>
			folderEntries;
		Stuff::SortedChainOf<DirectoryEntry*, Stuff::MString>
			fileEntries;
		Stuff::SortedChainIteratorOf<DirectoryFolder*, Stuff::MString>
			*folderWalker;
		Stuff::SortedChainIteratorOf<DirectoryEntry*, Stuff::MString>
			*fileWalker;
	};

	//##########################################################################
	//########################    FileStream    ################################
	//##########################################################################

	class FileStream:
		public MemoryStream
	{
		friend class FileStreamManager;

	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		static ClassData
			*DefaultData;

		static const char
			*WhiteSpace;

		void
			TestInstance() const;
		static bool
			TestClass();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors
	//
	public:
		enum WriteStatus {
			ReadOnly,
			WriteOnly
		};

		FileStream();
		explicit FileStream(
			const char* file_name,
			WriteStatus writable = ReadOnly
		);
		~FileStream();

		void
			Open(
				const char* file_name = NULL,
				WriteStatus writable = ReadOnly
			);
		void
	      	Close();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Status functions
	//
	public:
		void
			SetPointer(void *)
				{
					STOP((
					 "No implementation possible for FileStream::SetPointer(void*)"
					));
				}
		void
			SetPointer(DWORD index);

		MemoryStream&
			AdvancePointer(DWORD count);

		MemoryStream&
			RewindPointer(DWORD count);

		MemoryStream&
			ReadBytes(
				void *ptr,
				DWORD number_of_bytes
			);
		MemoryStream&
			WriteBytes(
				const void *ptr,
				DWORD number_of_bytes
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// File functions
	//
	public:
		virtual bool
			IsFileOpened();
		const char*
			GetFileName()
				{Check_Object(this); return fileName;}

		enum SeekType {
			FromBeginning,
			FromEnd
		};

		static char
			RedirectedName[256];
		static bool
			IsRedirected;

		static void
			IgnoreReadOnly(bool flag)
				{IgnoreReadOnlyFlag = flag;}
				

	protected:
		WriteStatus
			writeEnabled;
		char *
			fileName;
		HGOSFILE
			fileHandle;
		bool
			isOpen;
		static bool
			IgnoreReadOnlyFlag;
	};

	MString*
		StripExtension(MString *file_name);
	MString*
		IsolateDirectory(MString *file_name);
	MString*
		StripDirectory(MString *file_name);

	bool
		CreateDirectories(const char *directories);
}
