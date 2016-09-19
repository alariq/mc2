//===========================================================================//
// File:     notation.cpp                                                    //
// Title:    Definition of NotationFile classes.                             //
// Purpose:  Provide general purpose access to data stored in a formatted    //
//           text file.                                                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

#define MAX_LINE_SIZE 512

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Page::Page(NotationFile *notation_file):
	Plug(DefaultData),
	m_notes(NULL)
{
	Check_Pointer(this);
	Check_Object(notation_file);
	m_notationFile = notation_file;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Page::~Page()
{
	Check_Object(this);
	NoteIterator notes(&m_notes);
	notes.DeletePlugs();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::WriteNotes(MemoryStream *stream)
{
	Check_Object(this);

	const char* name = m_name;
	if (name)
		*stream << '[' << name << "]\r\n";
	NoteIterator notes(&m_notes);
	Note *note;
	while ((note = notes.ReadAndNext()) != NULL)
	{
		Check_Object(note);
		note->WriteNotation(stream);
	}
	*stream << "\r\n";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Note*
	Page::FindNote(const char *entryname)
{
	Check_Object(this);
	Check_Pointer(entryname);

	NoteIterator notes(&m_notes);
	Note *note;
	while ((note = notes.ReadAndNext()) != NULL)
	{
		Check_Object(note);
		if (!_stricmp(note->m_name, entryname))
			return note;
	}
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Note*
	Page::GetNote(unsigned i)
{
	Check_Object(this);

	NoteIterator notes(&m_notes);
	return notes.GetNth(i);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ChainOf<Note*>*
	Page::MakeNoteChain(const char *entryname)
{
	Check_Object(this);
	Check_Pointer(entryname);

	NoteIterator notes(&m_notes);
	Note *note;
	ChainOf<Note*>* chain = new ChainOf<Note*>(NULL);
	Check_Object(chain);
	int len = strlen(entryname);
	while ((note = notes.ReadAndNext()) != NULL)
	{
		Check_Object(note);
		if (!_strnicmp(note->GetName(), entryname, len))
			chain->Add(note);
	}
	return chain;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Note*
	Page::AddNote(const char *entryname)
{
	Check_Object(this);
	Check_Pointer(entryname);

	SetDirty();
	Note *note = new Note(this);
	Check_Object(note);
	note->SetName(entryname);
	m_notes.Add(note);

	return note;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::DeleteNote(const char *entryname)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		delete note;
		SetDirty();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::DeleteAllNotes()
{
	Check_Object(this);

	NoteIterator notes(&m_notes);
	Note *note;
	while ((note = notes.ReadAndNext()) != NULL)
	{
		Check_Object(note);
		delete note;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		const char **contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
            (const char*)m_name,
            entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		const char *contents
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		const char *contents
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		int *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		int contents
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		int contents
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		Scalar *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		Scalar value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		Scalar value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		bool *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		bool value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		bool value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		Vector3D *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		const Vector3D &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		const Vector3D &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		YawPitchRoll *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		const YawPitchRoll &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		const YawPitchRoll &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		UnitQuaternion *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		const UnitQuaternion &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		const UnitQuaternion &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		Motion3D *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
            (const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		const Motion3D &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		const Motion3D &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		RGBColor *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		const RGBColor &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		const RGBColor &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		RGBAColor *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		const RGBAColor &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		const RGBAColor &value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Page::GetEntry(
		const char *entryname,
		NotationFile *contents,
		bool required
	)
{
	Check_Object(this);
	Check_Pointer(entryname);
	Check_Pointer(contents);

	Note *note = FindNote(entryname);
	if (note)
	{
		Check_Object(note);
		note->GetEntry(contents);
		return true;
	}

	if (required)
		STOP((
			"%s: [%s]%s is a required entry!",
			m_notationFile->GetFileName(),
			(const char*)m_name,
			entryname
		));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::SetEntry(
		const char *entryname,
		NotationFile *value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = FindNote(entryname);
	if (!note)
		note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::AppendEntry(
		const char *entryname,
		NotationFile *value
	)
{
	Check_Object(this);
	Check_Pointer(entryname);

	Note *note = AddNote(entryname);
	Check_Object(note);
	note->SetEntry(value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Page::TestInstance() const
{
}
