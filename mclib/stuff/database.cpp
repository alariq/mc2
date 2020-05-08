#include"stuffheaders.hpp"
#include"database.hpp"
#include<toolos.hpp>
#include <stddef.h> // ptrdiff_t

//===========================================================================//
// File:	Database.cpp                                                     //
// Contents: Database functionality											 //												 
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//



//
// A pointer to this structure is the handle to the database
//
class Stuff::Database
{
public:
	enum {
		e_DataBlockSize=1021,
		e_Tag = 0x23564244, // '\x44\x42\x56\x23', // 'DBV#',
		e_Version = 2
	};

	Database();

	DWORD
		m_tag,							// Magic number to identity file
		m_version;						// Version number
	DWORD
		m_numberOfRecords,				// Number of used records in database
		m_nextRecordID;					// Sequential index assigned in ADD
	size_t
		m_idOffsets[e_DataBlockSize],	// Offsets to DatabaseRecords ( sorted by index )
		m_nameOffsets[e_DataBlockSize];	// Offsets to DatabaseRecords ( sorted by HASH )

	void
		TestInstance() const
			{Verify(m_tag == e_Tag && m_version <= e_Version);}

	static int
		FilesOpened;
};

//
// Each database record has this header
//
class Stuff::Record
{
public:
	Record(
		const RecordHandle *handle,
		DWORD record_hash,
		DWORD name_length
	);
	void
		Unhook(const RecordHandle *handle);

	__int64
		m_lastModified;				// Time record was last modifyed

	size_t
		m_nextIDRecord,				// offset to chain of records that share the same hash
		m_nextNameRecord;			// offset to chain of records that share the same hash
	DWORD
		m_ID,						// ID
		m_hash,						// Hash value
		m_nameLength,
		m_length;					// If this is zero, the record has been deleted (used to signify gaps before compressing)

	bool
		m_mustFree;					// When 1 gos_Free must be called on the block
	char
		m_name[1];
	BYTE
		m_data[1];

	void
		TestInstance() const
			{}
};

static HGOSHEAP
	Database_Heap = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
static DWORD
	GenerateHash(const char* name)
{
	DWORD hash=0;

	while (*name)
	{
		hash=(hash<<4) + (*name&15);
		if (hash&0xf0000000)
			hash=hash^(hash>>28);
		name++;
	}
	return hash|0x80000000;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Record::Record(
	const RecordHandle *handle,
	DWORD record_hash,
	DWORD name_length
)
{
	Check_Pointer(this);
	Check_Object(handle);
	DatabaseHandle *db_handle = handle->m_databaseHandle;
	Check_Object(db_handle);
	Database *db = db_handle->m_dataBase;
	Check_Object(db);

	//
	//------------------
	// Set up the record
	//------------------
	//
	m_mustFree = true;
	m_ID = handle->m_ID;
	m_hash = record_hash;
	m_nameLength = name_length;
	m_length = handle->m_length;
	m_lastModified = gos_GetTimeDate();
	memcpy(&m_name[name_length+1], handle->m_data, m_length);

	//
	//------------------
	// Store in database
	//------------------
	//
	DWORD index=m_ID % Database::e_DataBlockSize;
    size_t offset = (size_t)this - db_handle->m_baseAddress;
	//DWORD offset = (DWORD)this - db_handle->m_baseAddress;
	m_nextIDRecord = db->m_idOffsets[index];
	db->m_idOffsets[index] = offset;
	Check_Pointer(handle->m_name);
	memcpy(m_name, handle->m_name, name_length+1);
	index = record_hash % Database::e_DataBlockSize;
	m_nextNameRecord = db->m_nameOffsets[index];
	db->m_nameOffsets[index] = offset;

	//
	//----------------------------
	// Increase the database count
	//----------------------------
	//
	db->m_numberOfRecords++;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Record::Unhook(const RecordHandle *handle)
{
	Check_Pointer(this);
	Check_Object(handle);
	DatabaseHandle *db_handle = handle->m_databaseHandle;
	Check_Object(db_handle);
	Database *db = db_handle->m_dataBase;
	Check_Object(db);

	//
	//--------------------------------------------
	// Make sure that we can find the record by ID
	//--------------------------------------------
	//
	Record* record;
	DWORD index = m_ID % Database::e_DataBlockSize;
	#ifdef _ARMOR
		record = reinterpret_cast<Record*>(db->m_idOffsets[index]);
		while (record)
		{
			record = reinterpret_cast<Record*>((size_t)record + db_handle->m_baseAddress);
			Check_Object(record);
			if (record->m_ID == m_ID)
				break;
			record =
				reinterpret_cast<Record*>(record->m_nextIDRecord);
		}	
		Verify(this == record);
	#endif

	//
	//-------------------------------------------
	// Now remove the record from the index database
	//-------------------------------------------
	//
	record = reinterpret_cast<Record*>(db->m_idOffsets[index] + db_handle->m_baseAddress);
	if (record == this)
		db->m_idOffsets[index] = m_nextIDRecord;
	else
	{
		record = reinterpret_cast<Record*>(db->m_idOffsets[index]);
		while (record)
		{
			record = reinterpret_cast<Record*>((size_t)record + db_handle->m_baseAddress);
			Check_Object(record);
			if ((DWORD)record->m_nextIDRecord+db_handle->m_baseAddress == (size_t)this)
			{
				record->m_nextIDRecord = m_nextIDRecord;
				break;
			}
			record = reinterpret_cast<Record*>(record->m_nextIDRecord);
		}	
	}

	//
	//---------------------------------
	// Now remove from hash index database
	//---------------------------------
	//
	index = m_hash % Database::e_DataBlockSize;
	record = reinterpret_cast<Record*>(db->m_nameOffsets[index] + db_handle->m_baseAddress);
	if (record == this)
		db->m_nameOffsets[index] = m_nextNameRecord;
	else
	{
		record = reinterpret_cast<Record*>(db->m_nameOffsets[index]);
		while (record)
		{
			record = reinterpret_cast<Record*>((size_t)record + db_handle->m_baseAddress);
			Check_Object(record);
			if ((DWORD)record->m_nextNameRecord+db_handle->m_baseAddress == (size_t)this)
			{
				record->m_nextNameRecord = m_nextNameRecord;
				break;
			}
			record = reinterpret_cast<Record*>(record->m_nextNameRecord);
		}	
	}
	db->m_numberOfRecords--;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	RecordHandle::Add()
{
	Check_Object(this);
	gos_PushCurrentHeap(Database_Heap);

	//
	//------------------
	// Find our database
	//------------------
	//
	Check_Object(m_databaseHandle);
	Verify(!m_databaseHandle->m_readOnly);
	m_databaseHandle->m_dirtyFlag = true;
	Database* db = m_databaseHandle->m_dataBase;
	Check_Object(db);

	//
	//-------------------------------------------
	// Make sure the record doesn't already exist
	//-------------------------------------------
	//
	Verify(!m_record);
	#ifdef _ARMOR
		if (m_name)
		{
			RecordHandle dup_check = *this;
			if (dup_check.FindName())
				STOP(( "Duplicate Record" ));
		}
	#endif
	
	//
	//------------------
	// Set the record index
	//------------------
	//
	m_ID=db->m_nextRecordID++;

	//
	//---------------------------------------------------
	// Figure out how long the name is and its hash value
	//---------------------------------------------------
	//
	DWORD record_hash, name_length;
	if (m_name)
	{
		record_hash = GenerateHash(m_name);
		name_length = (DWORD)strlen(m_name);
		Verify(name_length > 0);
	}
	else
	{
		record_hash = 0;
		name_length = 0;
	}

	//
	//------------------
	// Set up the record
	//------------------
	//
	Verify(!m_record);
	Record *data =
		new(new BYTE[sizeof(*m_record) + m_length + name_length])
			Record(this, record_hash, name_length);
	Check_Object(data);
	m_data = &data->m_name[name_length+1];
	if (m_name)
		m_name = data->m_name;
	m_timeStamp = data->m_lastModified;

	//
	//------------------
	// Update statistics
	//------------------
	//
	m_record = data;
	gos_PopCurrentHeap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	RecordHandle::Replace()
{
	Check_Object(this);
	gos_PushCurrentHeap(Database_Heap);

	//
	//------------------
	// Find our database
	//------------------
	//
	Check_Object(m_databaseHandle);
	Verify(!m_databaseHandle->m_readOnly);
	m_databaseHandle->m_dirtyFlag = true;
	
	//
	//----------------------
	// Unhook the old record
	//----------------------
	//
	Check_Object(m_record);
	const_cast<Record*>(m_record)->Unhook(this);

	//
	//---------------------------------------------------
	// Figure out how long the name is and its hash value
	//---------------------------------------------------
	//
	DWORD record_hash, name_length;
	if (m_name)
	{
		record_hash = GenerateHash(m_name);
		name_length = (DWORD)strlen(m_name);
		Verify(name_length > 0);
	}
	else
	{
		record_hash = 0;
		name_length = 0;
	}

	//
	//------------------
	// Set up the record
	//------------------
	//
	Record *data =
		new(new BYTE[sizeof(*m_record) + m_length + name_length])
			Record(this, record_hash, name_length);
	Check_Object(data);
	m_data = &data->m_name[name_length+1];
	if (m_name)
		m_name = data->m_name;
	m_timeStamp = data->m_lastModified;

	//
	//------------------
	// Update statistics
	//------------------
	//
	if (m_record->m_mustFree)
		delete const_cast<Record*>(m_record);
	m_record = data;
	gos_PopCurrentHeap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	RecordHandle::Delete()
{
	Check_Object(this);

	//
	//------------------
	// Find our database
	//------------------
	//
	Check_Object(m_databaseHandle);
	Verify(!m_databaseHandle->m_readOnly);
	m_databaseHandle->m_dirtyFlag = true;
	
	//
	//----------------------
	// Delete the old record
	//----------------------
	//
	Check_Object(m_record);
	const_cast<Record*>(m_record)->Unhook(this);
	if (m_record->m_mustFree)
		delete const_cast<Record*>(m_record);
	m_record = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	RecordHandle::FindID()
{
	Check_Object(this);
	Check_Object(m_databaseHandle);
	Database* db = m_databaseHandle->m_dataBase;
	Check_Object(db);

	DWORD index=m_ID % Database::e_DataBlockSize;
	Record* record = reinterpret_cast<Record*>(db->m_idOffsets[index]);
	while (record)
	{
		record =
			reinterpret_cast<Record*>(
				(size_t)record + m_databaseHandle->m_baseAddress
			);
		Check_Object(record);
		if (record->m_ID==m_ID)
		{
			m_length = record->m_length;
			m_data = &record->m_data[record->m_nameLength];
			m_name = record->m_name;
			m_record = record;
			m_timeStamp = record->m_lastModified;
			return true;
		}
		record = reinterpret_cast<Record*>(record->m_nextIDRecord);
	}	

	m_record = NULL;
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	RecordHandle::FindName()
{
	Check_Object(this);
	Check_Object(m_databaseHandle);
	Database* db = m_databaseHandle->m_dataBase;
	Check_Object(db);

	Check_Pointer(m_name);
	DWORD hash = GenerateHash(m_name);
	DWORD index = hash % Database::e_DataBlockSize;

	Record* record = reinterpret_cast<Record*>(db->m_nameOffsets[index]);
	while (record)
	{
		record = reinterpret_cast<Record*>(
			(size_t)record + m_databaseHandle->m_baseAddress
		);
		Check_Object(record);
		if (record->m_hash == hash && !S_stricmp(m_name,record->m_name))
		{
			m_length = record->m_length;
			m_data = &record->m_data[record->m_nameLength];
			m_ID = record->m_ID;
			m_name = record->m_name;
			m_record = record;
			m_timeStamp = record->m_lastModified;
			return true;
		}
		record = reinterpret_cast<Record*>(record->m_nextNameRecord);
	}	

	m_record = NULL;
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	RecordHandle::ReadAndNext()
{
	Check_Object(this);
	Check_Object(m_databaseHandle);
	Database* db = m_databaseHandle->m_dataBase;
	Check_Object(db);

	Record* data;
	if (!m_databaseHandle->m_currentPointer)
		return false;

	Check_Object(m_databaseHandle->m_currentPointer);
	m_ID = m_databaseHandle->m_currentPointer->m_ID;
	m_length = m_databaseHandle->m_currentPointer->m_length;
	m_data = (&m_databaseHandle->m_currentPointer->m_data[m_databaseHandle->m_currentPointer->m_nameLength]);
	m_name = m_databaseHandle->m_currentPointer->m_name;
	m_record = m_databaseHandle->m_currentPointer;
	m_timeStamp = m_databaseHandle->m_currentPointer->m_lastModified;

	m_databaseHandle->m_currentPointer = 
		reinterpret_cast<Record*>(m_databaseHandle->m_currentPointer->m_nextIDRecord);
	if (!m_databaseHandle->m_currentPointer)
	{
		while (++m_databaseHandle->m_currentRecord < Database::e_DataBlockSize)
		{
			data = reinterpret_cast<Record*>(
				db->m_idOffsets[m_databaseHandle->m_currentRecord]
			);
			if( data )
			{
				m_databaseHandle->m_currentPointer =
					reinterpret_cast<Record*>(
						(size_t)data + m_databaseHandle->m_baseAddress
					);
				return true;
			}
		}
		m_databaseHandle->m_currentPointer = NULL;
		return true;
	}

	m_databaseHandle->m_currentPointer = 
		reinterpret_cast<Record*>(
			(size_t)m_databaseHandle->m_currentPointer + m_databaseHandle->m_baseAddress
		);
	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	RecordHandle::TestInstance() const
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	Database::FilesOpened = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Database::Database()
{
	m_tag = e_Tag;
	m_version = e_Version;
	m_nextRecordID = 1;
	m_numberOfRecords = 0;
	memset(m_idOffsets, 0, sizeof(m_idOffsets));
	memset(m_nameOffsets, 0, sizeof(m_nameOffsets));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
DatabaseHandle::DatabaseHandle(
	const char* filename,
	bool read_only
)
{
	//
	//-----------------------------------------------------
	// Create the database heap if it doesn't already exist
	//-----------------------------------------------------
	//
	m_fileName = filename;
	if (!Database::FilesOpened++)
		Database_Heap = gos_CreateMemoryHeap("Database", 0);
	gos_PushCurrentHeap(Database_Heap);
	m_currentRecord = 0;
	m_currentPointer = NULL;
	m_dirtyFlag = false;

	//
	//--------------------------------------------
	// If the file is read only, our job is simple
	//--------------------------------------------
	//
	m_readOnly = read_only;
	if (m_readOnly)
	{
		if (gos_DoesFileExist(filename))
		{
			FileStream::IsRedirected = false;
			DWORD size;
			m_handle =
				gos_OpenMemoryMappedFile(
					filename,
					reinterpret_cast<BYTE**>(&m_dataBase),
					&size
				);
			if (m_dataBase->m_tag != Database::e_Tag)
				STOP(("Invalid database file \"%s\"", filename));
			if (m_dataBase->m_version > Database::e_Version)
				STOP(("Application must be recompiled to use database \"%s\"", filename));
			m_baseAddress = reinterpret_cast<size_t>(m_dataBase);
			Check_Object(m_dataBase);
		}
		else
			STOP(("Database \"%s\" does not exist", filename));
	}

	//
	//-------------------------------------------------------------
	// The file is not read only, so see if we load it or create it
	//-------------------------------------------------------------
	//
	else
	{
		m_handle = 0;
		if (gos_DoesFileExist(filename))
		{
			size_t size;
			gos_GetFile(
				filename,
				reinterpret_cast<BYTE**>(&m_dataBase),
				&size
			);
			FileStream::IsRedirected = false;
			Check_Pointer(m_dataBase);
			if (m_dataBase->m_tag != Database::e_Tag || m_dataBase->m_version > Database::e_Version || m_dataBase->m_version == 1)
			{
				gos_Free(m_dataBase);
				PAUSE(("Bad database file!  Press 'Continue' to rebuild", filename));
				m_dataBase = new(gos_Malloc(sizeof(Database), Database_Heap)) Database;
				m_baseAddress = 0;
			}
			else
			{
				m_baseAddress = reinterpret_cast<size_t>(m_dataBase);
				Check_Object(m_dataBase);
			}
		}

		//
		//------------------------------------------
		// The file doesn't exist, so create it here
		//------------------------------------------
		//
		else
		{
			m_dataBase = new(gos_Malloc(sizeof(Database), Database_Heap)) Database;
			m_baseAddress = 0;
		}
	}
	gos_PopCurrentHeap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
DatabaseHandle::~DatabaseHandle()
{
	Check_Object(this);

	//
	//---------------------------------------------------------------
	// If we are closing a read only file, release the memmap handle,
	// otherwise delete the database object
	//---------------------------------------------------------------
	//
	if (m_readOnly)
		gos_CloseMemoryMappedFile(m_handle);
	else
	{
		Save();
		Check_Object(m_dataBase);

		//
		//---------------------------------------------------------------------
		// Now we need to go through and delete all the records in the database
		//---------------------------------------------------------------------
		//
		for (DWORD i=0; i<Database::e_DataBlockSize; ++i)
		{
			Record* record = reinterpret_cast<Record*>(m_dataBase->m_idOffsets[i]);
			if (record)
			{
				while (record)
				{
					record = reinterpret_cast<Record*>((size_t)record + m_baseAddress);
					Check_Object(record);
					Record* this_record = record;

					record = reinterpret_cast<Record*>(record->m_nextIDRecord);
					if (this_record->m_mustFree)
						delete this_record;
				}
			}
		}

		//
		//--------------------------------
		// Now free up the database itself
		//--------------------------------
		//
		gos_Free(m_dataBase);
	}

	//
	//----------------------------------------------------
	// Delete the memory heap if this is the last database
	//----------------------------------------------------
	//
	if (--Database::FilesOpened == 0)
		gos_DestroyMemoryHeap(Database_Heap);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	DatabaseHandle::Save()
{
	Check_Object(this);

	//
	//--------------------------------------
	// If there were no changes, we are done
	//--------------------------------------
	//
	if (m_readOnly || !m_dirtyFlag)
		return;

	//
	//----------------------------------------------------------------------
	// We will need to adjust pointers as we write this thing out, so make a
	// copy of the database object now
	//----------------------------------------------------------------------
	//
	Database output_db;
	output_db.m_numberOfRecords = m_dataBase->m_numberOfRecords;
	output_db.m_nextRecordID = m_dataBase->m_nextRecordID;

	//
	//----------------------------------------------------
	// Build a table to deal with rethreading the database
	//----------------------------------------------------
	//
	struct OutputRecord
	{
		DWORD
			m_ID,
			m_offset,
			m_nextIDRecord,
			m_nextNameRecord;
		Record
			*m_data;
	};
	OutputRecord *new_records = new OutputRecord[m_dataBase->m_numberOfRecords];
	Check_Pointer(new_records);
	size_t new_id_index[Database::e_DataBlockSize];
	memset(new_id_index, 0, sizeof(new_id_index));

	//
	//---------------------------------------------------------
	// Start filling in the block information from the database
	//---------------------------------------------------------
	//
	DWORD offset = sizeof(output_db);
	OutputRecord* new_record = new_records;
	for (DWORD i=0; i<Database::e_DataBlockSize; ++i)
	{
		Record* old_record = reinterpret_cast<Record*>(m_dataBase->m_idOffsets[i]);
		if (old_record)
		{
			output_db.m_idOffsets[i] = offset;
			new_id_index[i] = new_record - new_records;
			while (old_record)
			{
				old_record = reinterpret_cast<Record*>((size_t)old_record + m_baseAddress);
				Check_Object(old_record);

				new_record->m_data = old_record;
				new_record->m_ID = old_record->m_ID;
				new_record->m_offset = offset;
				new_record->m_nextIDRecord = 0;
				new_record->m_nextNameRecord = 0;

				offset +=
					sizeof(*old_record) + old_record->m_length + old_record->m_nameLength;

				old_record = reinterpret_cast<Record*>(old_record->m_nextIDRecord);
				if (old_record)
					new_record->m_nextIDRecord = offset;
				++new_record;
			}
		}
	}

	//
	//-----------------------------------------
	// Make sure Hash index table is up to date
	//-----------------------------------------
	//
	for (int i=0; i<Database::e_DataBlockSize; ++i)
	{
		Record* old_record = reinterpret_cast<Record*>(m_dataBase->m_nameOffsets[i]);
		if (old_record)
		{
			old_record = reinterpret_cast<Record*>((size_t)old_record + m_baseAddress);
			Check_Object(old_record);

			//
			//---------------------------------
			// Find this record in our new data
			//---------------------------------
			//
			DWORD index = old_record->m_ID % Database::e_DataBlockSize;
			size_t j = new_id_index[index];
			OutputRecord *new_record = &new_records[j];
			for (; j<m_dataBase->m_numberOfRecords; ++j, ++new_record)
			{
				if (new_record->m_ID == old_record->m_ID)
					break;
			}
			Verify(j<m_dataBase->m_numberOfRecords);

			//
			//----------------------
			// Set up the hash chain
			//----------------------
			//
			output_db.m_nameOffsets[i] = new_record->m_offset;
			while (old_record)
			{
				Check_Object(old_record);

				//
				//-----------------------------------------------------------
				// Find the next record, and find where it is in our new data
				//-----------------------------------------------------------
				//
				old_record = reinterpret_cast<Record*>(old_record->m_nextNameRecord);
				Verify(j<m_dataBase->m_numberOfRecords);
				if (old_record)
				{
					old_record = reinterpret_cast<Record*>((size_t)old_record + m_baseAddress);
					Check_Object(old_record);
					index = old_record->m_ID % Database::e_DataBlockSize;
					j = new_id_index[index];
					OutputRecord *next_record = &new_records[j];
					for (; j<m_dataBase->m_numberOfRecords; ++j, ++next_record)
					{
						if (next_record->m_ID == old_record->m_ID)
							break;
					}
					Verify(j<m_dataBase->m_numberOfRecords);
					new_record->m_nextNameRecord = next_record->m_offset;
					new_record = next_record;
				}
			}
		}
	}

	//
	//------------------------------------------------
	// This file was read/write, so write it out again
	//------------------------------------------------
	//
	FileStream db_file(m_fileName, FileStream::WriteOnly);
	db_file.WriteBytes(&output_db, sizeof(output_db));

	//
	//----------------------
	// Write out each record
	//----------------------
	//
	new_record=new_records;
	for (int i=0; i<m_dataBase->m_numberOfRecords; ++i, ++new_record)
	{
		Record* old_record = new_record->m_data;

		//
		//------------------------------------------------------------
		// Save the old connection info, then replace it with the data
		// calculated in the new records
		//------------------------------------------------------------
		//
		bool free_block = old_record->m_mustFree;
		size_t next_id = old_record->m_nextIDRecord;
		size_t next_name = old_record->m_nextNameRecord;
		old_record->m_mustFree = false;
		old_record->m_nextIDRecord = new_record->m_nextIDRecord;
		old_record->m_nextNameRecord = new_record->m_nextNameRecord;

		//
		//------------------------------------------
		// Write out the info, then restore the data
		//------------------------------------------
		//
		db_file.WriteBytes(
			old_record,
			sizeof(*old_record) + old_record->m_length + old_record->m_nameLength
		);
		old_record->m_mustFree = free_block;
		old_record->m_nextIDRecord = next_id;
		old_record->m_nextNameRecord = next_name;
	}

	//
	//---------
	// Clean up
	//---------
	//
	delete[] new_records;
	m_dirtyFlag = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
DWORD
	DatabaseHandle::GetNumberOfRecords()
{
	Check_Object(this);
	return m_dataBase->m_numberOfRecords;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	DatabaseHandle::First()
{
	Check_Object(this);

	m_currentRecord = 0;
	m_currentPointer = NULL;

	for (DWORD i=0; i<Database::e_DataBlockSize; i++)
	{
		Record* data =
			reinterpret_cast<Record*>(
				m_dataBase->m_idOffsets[i] + m_baseAddress
			);
		if( data )
		{
			m_currentRecord = i;
			m_currentPointer = data;
			break;
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	DatabaseHandle::TestInstance() const
{
}
