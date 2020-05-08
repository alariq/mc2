//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

#if !defined(TOOLOS_HPP)
#include<toolos.hpp>
#endif

//#############################################################################
//#############################    Directory    ###############################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Directory::Directory(
	char *find_files,
	bool directories
):
	fileEntries(NULL,false),
	folderEntries(NULL,false)
{
	Check_Pointer(find_files);

	//
	//------------------------------------------------
	// Isolate the search path that we are looking for
	//------------------------------------------------
	//
	MString new_directory_string = find_files;

	//
	//---------------------------------------------------------
	// Now look for the files and add each entry into the chain
	//---------------------------------------------------------
	//
	const char* file_name = gos_FindFiles(new_directory_string);
	while (file_name)
	{
		MString file_name_string = file_name;
		DirectoryEntry *entry = new DirectoryEntry(file_name_string);
		Check_Object(entry);
		fileEntries.AddValue(entry,file_name_string);
		file_name = gos_FindFilesNext();
	}
	gos_FindFilesClose();
	fileWalker = new SortedChainIteratorOf<DirectoryEntry*,MString>(&fileEntries);
	Check_Object(fileWalker);

	//
	//---------------------------------------------------
	// Look through the directories if we are supposed to
	//---------------------------------------------------
	//
	folderWalker = NULL;
	if (directories)
	{
		file_name = gos_FindDirectories(new_directory_string);
		while (file_name)
		{
			MString file_name_string = file_name;
			DirectoryFolder *entry = new DirectoryFolder(file_name_string);
			Check_Object(entry);
			folderEntries.AddValue(entry,file_name_string);
			file_name = gos_FindDirectoriesNext();
		}
		gos_FindDirectoriesClose();
		folderWalker = new SortedChainIteratorOf<DirectoryFolder*,MString>(&folderEntries);
		Check_Object(folderWalker);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Directory::~Directory(void)
{
	Check_Object(this);
	Check_Object(fileWalker);
	fileWalker->DeletePlugs();
	Check_Object(fileWalker);
	delete fileWalker;
	if (folderWalker != NULL)
	{
		Check_Object(folderWalker);
		folderWalker->DeletePlugs();
		Check_Object(folderWalker);
		delete folderWalker;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
char*
	Directory::GetCurrentFileName(void)
{
	Check_Object(this);
	Check_Object(fileWalker);

	DirectoryEntry *entry;
	
	if ((entry = fileWalker->GetCurrent()) != NULL)
	{
		Check_Object(entry);
		return entry->GetItem();
	}
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Directory::AdvanceCurrentFile(void)
{
	Check_Object(this);
	Check_Object(fileWalker);
	fileWalker->Next();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
char * Directory::GetCurrentFolderName( void )
{
	Check_Object(this);
	if (folderWalker == NULL)
		STOP(("Directory class was instantiated without directory support:\n           Set the <directories> parameter to true to enable."));
	Check_Object(folderWalker);

	DirectoryFolder *entry;
	
	if ((entry = folderWalker->GetCurrent()) != NULL)
	{
		Check_Object(entry);
		return entry->GetItem();
	}
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void Directory::AdvanceCurrentFolder( void )
{
	Check_Object(this);
	if (folderWalker == NULL)
		STOP(("Directory class was instantiated without directory support:\n           Set the <directories> parameter to true to enable."));
	Check_Object(folderWalker);
	folderWalker->Next();
}

//#############################################################################
//###########################    FileStream    ################################
//#############################################################################

FileStream::ClassData*
	FileStream::DefaultData = NULL;

const char *
	FileStream::WhiteSpace = " \t\n";

char
	FileStream::RedirectedName[256];
bool
	FileStream::IsRedirected=false,
	FileStream::IgnoreReadOnlyFlag=false;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileStream::InitializeClass()
{
	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			FileStreamClassID,
			"Stuff::FileStream",
			MemoryStream::DefaultData
		);
	Check_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileStream::TerminateClass()
{
	Check_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileStream::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileStream::FileStream():
	MemoryStream(DefaultData, NULL, 0)
{
	fileName = NULL;
	isOpen = false;
	fileHandle = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileStream::FileStream(
	const char* file_name,
	WriteStatus writable
):
	MemoryStream(DefaultData, NULL, 0)
{
	fileName = NULL;
	isOpen = false;
	Open(file_name, writable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileStream::~FileStream()
{
	Close();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileStream::Open(
		const char* file_name,
		WriteStatus writable
	)
{
	Check_Object(this);
	Check_Pointer(file_name);

	writeEnabled = writable;
	currentPosition = streamStart = NULL;
	streamSize = 0;
	fileHandle = NULL;

	//
	//------------------------------------------------
	// If this is a readonly file, have gos read it in
	//------------------------------------------------
	//
	if (writeEnabled == ReadOnly)
	{
		if (IsRedirected)
		{
			void (__stdcall *old_hook)(const char*, BYTE**, DWORD*) = Environment.HookGetFile;
			Environment.HookGetFile = NULL;
			gos_GetFile(RedirectedName, &streamStart, &streamSize);
			Environment.HookGetFile = old_hook;
			fileName = _strdup(RedirectedName);
		}
		else
		{
			gos_GetFile(file_name, &streamStart, &streamSize);
			if (IsRedirected)
				fileName = _strdup(RedirectedName);
			else
				fileName = _strdup(file_name);
		}
		isOpen = true;
		currentPosition = streamStart;
	}

	//
	//-----------------------------------------------
	// Write only flags go through standard file open
	//-----------------------------------------------
	//
	else
	{
		writeEnabled = writable;
		if (IgnoreReadOnlyFlag)
		{
			bool (__stdcall *old_hook)(const char*) = Environment.HookDoesFileExist;
			Environment.HookDoesFileExist = NULL;
			if (gos_DoesFileExist(file_name))
				gos_FileSetReadWrite(file_name);
			Environment.HookDoesFileExist = old_hook;
		}
		gos_OpenFile(
			&fileHandle,
			file_name,
			READWRITE
		);
		isOpen = true;
		streamSize = 0xFFFFFFFFU;
		fileName = _strdup(file_name);
	}

	//
	//--------------------------
	// Try and open the filename
	//--------------------------
	//
	IsRedirected = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileStream::Close()
{
	Check_Object(this);

	//
	//---------------------------------
	// If the file was opened, close it
	//---------------------------------
	//
	if (fileHandle)
		gos_CloseFile(fileHandle);
	else if (streamStart != NULL)
		gos_Free(streamStart);

	//
	//----------------
	// Delete the name
	//----------------
	//
	isOpen = false;
	if (fileName)
	{
		Check_Pointer(fileName);
		free(fileName);
		fileName = NULL;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileStream::SetPointer(DWORD index)
{
	Check_Object(this);
	Verify(IsFileOpened());
	Verify(writeEnabled == ReadOnly);
	MemoryStream::SetPointer(index);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	FileStream::AdvancePointer(DWORD index)
{
	Check_Object(this);
	Verify(IsFileOpened());
	Verify(writeEnabled == ReadOnly);
	return MemoryStream::AdvancePointer(index);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	FileStream::RewindPointer(DWORD index)
{
	Check_Object(this);
	Verify(IsFileOpened());
	Verify(writeEnabled == ReadOnly);
	return MemoryStream::RewindPointer(index);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	FileStream::ReadBytes(
		void *ptr,
		DWORD number_of_bytes
	)
{
	Check_Object(this);
	Verify(IsFileOpened());
	Verify(writeEnabled == ReadOnly);
	return MemoryStream::ReadBytes(ptr, number_of_bytes);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	FileStream::WriteBytes(
		const void *ptr,
		DWORD number_of_bytes
	)
{
	Check_Object(this);
	Verify(IsFileOpened());
	Verify(writeEnabled == WriteOnly);
	DWORD written =
		gos_WriteFile(
			fileHandle,
			Cast_Pointer(BYTE*, const_cast<void*>(ptr)),
			number_of_bytes
		);
	Verify(written == number_of_bytes);
	currentPosition += written;
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	FileStream::IsFileOpened()
{
	Check_Object(this);
	return isOpen;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Stuff::CreateDirectories(const char *directory_path)
{

    PAUSE(("")); // sebi
	
	if (directory_path == NULL)
		return false;

	Check_Pointer(directory_path);


	const char *start_position = directory_path;
	const char *current_position = directory_path;

	if (*current_position != '\\')
	{
	
		if ((current_position + 1) != NULL)
		{
			if((current_position + 2) != NULL)
			{
				if ( *(current_position + 1) == ':' && *(current_position + 2) == '\\')
				{
					current_position += 2;
				}
			}
		}
	}

	if (*current_position != '\\')
		return false;


	while((current_position != NULL))
	{
		//make a substring with the path...
		const char *next_slash;
		next_slash = strchr( current_position + 1, '\\' );

		char *new_string = NULL;

		if (next_slash == NULL)
		{
			//copy the whole string
			size_t length = strlen(start_position)+1;
			new_string = new char[length];
			Check_Pointer(new_string);

			Str_Copy(new_string, start_position, length);
		}
		else
		{

			if (next_slash - current_position == 0)
			{
				return false;
			}
			if (next_slash - current_position == 1)
			{
				return false;
			}

			//copy the sub string
			size_t length = next_slash - start_position;
			new_string = new char[length+1];
			Check_Pointer(new_string);

			strncpy(new_string, start_position, length);

			new_string[length] = '\0';

		}

		Verify(new_string != NULL);

		
		if (!gos_DoesFileExist(new_string))
		{
			gos_CreateDirectory(new_string);
		}



		Check_Pointer(new_string);
		delete new_string;

		current_position = next_slash;
	}




	return true;
	
}
