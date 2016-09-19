//===========================================================================//
// File:     namelist.cpp                                                    //
// Title:    Definition of NameList classes.                                 //
// Purpose:  Maintains an unsorted list of strings with (void *) to          //
//           anything the client needs to associate with the string.         //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include"namelist.hpp"

//#############################################################################
//##############    ObjectNameList    #########################################
//#############################################################################

ObjectNameList::ObjectNameList()
{
	Check_Pointer(this);
	firstEntry = lastEntry = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ObjectNameList::~ObjectNameList()
{
	Check_Object(this);

	Entry
		*next;

	while (firstEntry)
	{
		Check_Pointer(firstEntry);
		next = firstEntry->nextEntry;
		Unregister_Pointer(firstEntry);
		delete firstEntry;
		firstEntry = next;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
const char*
	ObjectNameList::AddEntry(
		const char *name,
		void *data
	)
{
	Check_Object(this);
	Check_Pointer(name);

	Entry
		*entry;

	entry = Cast_Pointer(Entry *, new char[sizeof(Entry) + strlen(name) + 1]);
	Register_Object(entry);

	entry->dataReference = data;
	entry->nextEntry = NULL;

	if (firstEntry)
	{
		Check_Pointer(lastEntry);
		lastEntry->nextEntry = entry;
	}
	else
	{
		firstEntry = entry;
	}
	lastEntry = entry;

	entry->SetName(name);

	return entry->GetName();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void*
	ObjectNameList::FindObject(const char *name)
{
	Check_Object(this);
	Check_Pointer(name);

	Entry
		*entry;

	for (entry = firstEntry; entry; entry = entry->nextEntry)
	{
		Check_Pointer(entry);
		if (!strcmp(entry->GetName(), name))
		{
			break;
		}
	}
	return (entry)?entry->dataReference:NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	ObjectNameList::DeleteEntry(const char *name)
{
	Check_Object(this);
	Check_Pointer(name);

	Entry
		*cur,
		*prev,
		*entry;

	//----------------------------------------------------
	// ***** DANGEROUS!!! see notice in namelist.hh *****
	//----------------------------------------------------
	entry = Cast_Pointer(Entry *, const_cast<char*>(name - sizeof(Entry)));
	prev = NULL;
	for (cur=firstEntry; cur && cur != entry; cur = cur->nextEntry)
	{
		Check_Pointer(cur);
		prev = cur;
	}
	Verify(cur == entry);
	if (!prev)
	{
		firstEntry = entry->nextEntry;
	}
	else
	{
		prev->nextEntry = entry->nextEntry;
	}
	if (entry == lastEntry)
	{
		lastEntry = prev;
	}
	Unregister_Pointer(entry);
	delete entry;
	return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	ObjectNameList::GetEntryCount() const
{
	Check_Object(this);

	Entry
		*entry = firstEntry;
	int
		count = 0;

	while (entry)
	{
		Check_Pointer(entry);
		count++;
		entry = entry->nextEntry;
	}
	return count;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	ObjectNameList::BuildSubList(
		const ObjectNameList &source_list,
		const char *prefix
	)
{
	Check_Object(this);
	Check_Object(&source_list);
	Check_Pointer(prefix);

   const int
		length = strlen(prefix);
	Entry
		*entry = source_list.firstEntry;
	const char
		*name;
	int
		count = 0;

	//------------------------------------------------------
	// add entries to SubList whose name begins with prefix
	//------------------------------------------------------
	while (entry)
	{
		Check_Pointer(entry);
		name = entry->GetName();
		Check_Pointer(name);
		if (!strncmp(name, prefix, length))
		{
			count++;
			AddEntry(name, entry->dataReference);
		}
		entry = entry->nextEntry;
	}
	//-------------------------------------------
	// return number of entries added to SubList
	//-------------------------------------------
	return count;
}

//#############################################################################
//##############    ObjectNameList::Entry    ##################################
//#############################################################################

void
	ObjectNameList::Entry::SetName(const char *name)
{
	Check_Pointer(this);
	Check_Pointer(name);
	strcpy(
		Cast_Pointer(char*, this) + sizeof(ObjectNameList::Entry),
		name
	);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	ObjectNameList::Entry::IsName(const char *name) const
{
	Check_Object(this);
	//do not check name here

	if (name)
	{
		Check_Pointer(name);
		return strcmp(GetName(), name) == 0;
	}
	else
	{
		return GetName() == name;
	}
}

//#############################################################################
//##############    NameList    ###############################################
//#############################################################################

NameList::NameList()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
NameList::~NameList()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
const char*
	NameList::FindName(void *data)
{
	Check_Object(this);

	Entry
		*entry;

	for (entry = firstEntry; entry; entry = entry->nextEntry)
	{
		Check_Pointer(entry);
		if (entry->dataReference == data)
		{
			break;
		}
	}
	return (entry)?entry->GetName():NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
NameList::Entry*
	NameList::FindEntry(const char *name)
{
	Check_Object(this);
	Check_Pointer(name);

	Entry
		*entry;

	for (entry = firstEntry; entry; entry = entry->nextEntry)
	{
		Check_Pointer(entry);
		if (!_stricmp(entry->GetName(), name))
		{
			break;
		}
	}
	return entry;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	NameList::FindEntryIndex(const char *name)
{
	Check_Object(this);
	Check_Pointer(name);

	Entry
		*entry;
	int
		result = -1;

	for (entry = firstEntry; entry; entry = entry->nextEntry)
	{
		++result;
		Check_Pointer(entry);
		if (!strcmp(entry->GetName(), name))
		{
			return result;
		}
	}
	return -1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
NameList::Entry
	*NameList::FindEntry(void *data)
{
	Check_Object(this);

	Entry
		*entry;

	for (entry = firstEntry; entry; entry = entry->nextEntry)
	{
		Check_Pointer(entry);
		if (entry->dataReference == data)
		{
			break;
		}
	}
	return entry;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NameList::DeleteEntry(const char *name)
{
	Check_Object(this);
	Check_Pointer(name);

	Entry
		*prev = NULL,
		*entry;

	for (entry = firstEntry; entry; entry = entry->nextEntry)
	{
		Check_Pointer(entry);
		if (!strcmp(entry->GetName(), name))
		{
			break;
		}
		prev = entry;
	}
	if (entry)
	{
		if (!prev)
		{
			firstEntry = entry->nextEntry;
		}
		else
		{
			prev->nextEntry = entry->nextEntry;
		}
		if (entry == lastEntry)
		{
			lastEntry = prev;
		}
		Unregister_Pointer(entry);
		delete entry;
	}
	return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStreamIO::Write(
		MemoryStream *stream,
		const NameList *names
	)
{
	Check_Object(names);

	//
	//------------------------------
	// Write out the number of names
	//------------------------------
	//
	int i = 0;
	const NameList::Entry *entry = names->GetFirstEntry();
	while (entry)
	{
		Check_Object(entry);
		++i;
		entry = entry->GetNextEntry();
	}
	*stream << i;

	//
	//--------------------
	// Write out the names
	//--------------------
	//
	entry = names->GetFirstEntry();
	while (entry)
	{
		Check_Object(entry);
		const char* name = entry->GetName();
		Check_Pointer(name);
		stream->WriteBytes(name, strlen(name)+1);
		entry = entry->GetNextEntry();
	}

	return *stream;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStreamIO::Read(
		MemoryStream *stream,
		NameList *names
	)
{
	//
	//---------------------------------------------------------------------
	// Read in the number of strings to add, then loop through and add them
	//---------------------------------------------------------------------
	//
	int count;
	*stream >> count;
	while (count-- > 0)
	{
		const char* name = Cast_Pointer(const char*, stream->GetPointer());
		names->AddEntry(name, NULL);
		stream->AdvancePointer(strlen(name)+1);
	}

	return *stream;
}

//#############################################################################
//##############    AlphaNameList    ##########################################
//#############################################################################

AlphaNameList::AlphaNameList()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
AlphaNameList::~AlphaNameList()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
const char*
	AlphaNameList::AddEntry(
		const char *name,
		void *data
	)
{
	Check_Object(this);
	Check_Pointer(name);

	Entry
		*entry,
		*next = firstEntry,
		*prev = NULL;

	entry = Cast_Pointer(Entry *, new char[sizeof(Entry) + strlen(name) + 1]);
	Register_Object(entry);

	//-----------------------------------
	// find location to insert new entry
	//-----------------------------------
	while (next)
	{
		Check_Pointer(next);
		if (strcmp(name, next->GetName()) < 0)
		{
			break;
		}
		prev = next;
		next = next->nextEntry;
	}
	//---------------------------------------------
	// insert new entry after prev and before next
	//---------------------------------------------
	if (!next)
	{
		lastEntry = entry;
	}
	if (!prev)
	{
		firstEntry = entry;
	}
	else
	{
		prev->nextEntry = entry;
	}
	entry->dataReference = data;
	entry->nextEntry = next;
	entry->SetName(name);

	return entry->GetName();
}

//#############################################################################
//#############################################################################
