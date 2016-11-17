//===========================================================================//
// File:	filestrmmgr.cpp                                                  //
// Contents: Implementation Details of resource management                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

//#############################################################################
//##########################    FileDependencies    ###########################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileDependencies::FileDependencies():
	Plug(DefaultData)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileDependencies::~FileDependencies()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileDependencies&
	FileDependencies::operator=(const FileDependencies &dependencies)
{
	Check_Pointer(this);
	Check_Object(&dependencies);

	m_fileNameStream.Rewind();
	int len = dependencies.m_fileNameStream.GetBytesUsed();
	if (len)
	{
		MemoryStream scanner(
			static_cast<BYTE*>(dependencies.m_fileNameStream.GetPointer())-len,
			len
		);
		m_fileNameStream.AllocateBytes(len);
		m_fileNameStream << scanner;
	}
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileDependencies::AddDependency(FileStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	//
	//---------------------
	// Get the new filename
	//---------------------
	//
	const char* new_file = stream->GetFileName();
	Check_Pointer(new_file);

	//
	//---------------------------------------------------
	// Make a new memorystream that wraps our current one
	//---------------------------------------------------
	//
	BYTE *end = Cast_Pointer(BYTE*, m_fileNameStream.GetPointer());
	int len = m_fileNameStream.GetBytesUsed();
	MemoryStream scanner(end-len, len);

	//
	//--------------------------------------
	// See if the new file is already inside
	//--------------------------------------
	//
	while (scanner.GetBytesRemaining() > 0)
	{
		const char* old_name = Cast_Pointer(const char*, scanner.GetPointer());
		int len = strlen(old_name);
		if (!_stricmp(new_file, old_name))
			return;
		scanner.AdvancePointer(len+1);
	}
	m_fileNameStream.WriteBytes(new_file, strlen(new_file)+1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileDependencies::AddDependencies(MemoryStream *dependencies)
{
	Check_Object(this);
	Check_Object(dependencies);

	int old_len = m_fileNameStream.GetBytesUsed();

	dependencies->Rewind();
	while (dependencies->GetBytesRemaining() > 0)
	{
		const char* new_name =
			Cast_Pointer(const char*, dependencies->GetPointer());
		int new_len = strlen(new_name);

		//
		//---------------------------------------------------
		// Make a new memorystream that wraps our current one
		//---------------------------------------------------
		//
		BYTE *end = static_cast<BYTE*>(m_fileNameStream.GetPointer());
		MemoryStream scanner(end-m_fileNameStream.GetBytesUsed(), old_len);

		//
		//--------------------------------------
		// See if the new file is already inside
		//--------------------------------------
		//
		while (scanner.GetBytesRemaining() > 0)
		{
			const char* old_name = Cast_Pointer(const char*, scanner.GetPointer());
			int len = strlen(old_name);
			if (!_stricmp(old_name, new_name))
				break;
			scanner.AdvancePointer(len+1);
		}
		if (!scanner.GetBytesRemaining())
			m_fileNameStream.WriteBytes(new_name, new_len+1);
		dependencies->AdvancePointer(new_len+1);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileDependencies::AddDependencies(const FileDependencies *dependencies)
{
	Check_Object(this);
	Check_Object(dependencies);

	AddDependencies((MemoryStream*)&dependencies->m_fileNameStream);
}

//#############################################################################
//#########################    FileStreamManager    ###########################
//#############################################################################

FileStreamManager*
	FileStreamManager::Instance = NULL;
	
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileStreamManager::FileStreamManager():
	compareCache(NULL, true)
{	
}	

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
FileStreamManager::~FileStreamManager()
{
	PurgeFileCompareCache();
}	

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	FileStreamManager::CompareModificationDate(
		const MString &file_name,
		__int64 time_stamp
	)
{
	Check_Object(this);
	Check_Object(&file_name);

	//
	//------------------------------
	// Check the compare cache first
	//------------------------------
	//
	FileStatPlug *stat_plug;
	
	if ((stat_plug = compareCache.Find(file_name)) != NULL)
	{
		Check_Object(stat_plug);
		Check_Pointer(stat_plug->GetPointer());
		if (*stat_plug->GetPointer() > time_stamp)
			return true;
		return false;
	}

	//
	//-------------------------------------------------------------
	// Get the statistics about the file.  If the file isn't there, 
	// return false == "file older than time stamp"
	//-------------------------------------------------------------
	//
	__int64 file_stats = gos_FileTimeStamp(file_name);
	if (file_stats == -1)
		return false;

	//
	//-------------
	// Add to cache
	//-------------
	//
	stat_plug = new FileStatPlug(file_stats);
	Register_Object(stat_plug);
	compareCache.AddValue(stat_plug, file_name);

	//
	// Return, "is file newer than time stamp"?
	//
	if (file_stats > time_stamp)
		return true;
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	FileStreamManager::PurgeFileCompareCache()
{
	Check_Object(this);
	TreeIteratorOf<FileStatPlug*, MString> cache(&compareCache);
	cache.DeletePlugs();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MString*
	Stuff::StripExtension(MString* file_name)
{
	Check_Object(file_name);

	if(file_name->GetLength() == 0)
	{
		return file_name;
	}

	char *p = strrchr(*file_name, '.');
	if (p)
	{
		*p = '\0';
		file_name->SetLength(p - (char *)*file_name);
	}

	Check_Object(file_name);
	return file_name;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MString*
	Stuff::IsolateDirectory(MString* file_name)
{
	Check_Object(file_name);

    PAUSE(("")); // sebi

	if(file_name->GetLength() == 0)
	{
		return file_name;
	}

	char *p = strrchr(*file_name, '\\');
	if (p)
	{
		*++p = '\0';
	}
	else
	{
		p = *file_name;
		*p = '\0';
	}
	file_name->SetLength(p - (char *)*file_name);
	Check_Object(file_name);
	return file_name;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MString*
	Stuff::StripDirectory(MString* file_name)
{

    PAUSE(("")); // sebi

	Check_Object(file_name);
	char *p = strrchr(*file_name, '\\');
	if (p)
	{
		char *q = *file_name;
		do
		{
			*q++ = *++p;
		}
		while (*p);
		// The following does not handle all cases...
		// file_name->SetLength(p - (char *)*file_name);
		file_name->SetLength(strlen(*file_name));
	}
	Check_Object(file_name);
	return file_name;
}
