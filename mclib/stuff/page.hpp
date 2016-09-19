//===========================================================================//
// File:     notation.hpp                                                    //
// Title:    Declaration of NotationFile classes.                            //
// Purpose:  Provide general purpose access to data stored in a formatted    //
//           text file.                                                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"notationfile.hpp"

namespace Stuff {

	//##########################################################################
	//##############    Page    ##############################
	//##########################################################################

	class Page:
		public Plug
	{
		friend class NotationFile;
		friend class Note;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor/Destructors
	//
	protected:
		Page(NotationFile *notation_file);
		~Page();

		NotationFile
			*m_notationFile;

	public:
		NotationFile*
			GetNotationFile()
				{Check_Object(this); return m_notationFile;}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Page functions
	//
	public:
		void
			SetName(const char *pagename)
				{Check_Object(this); m_name = pagename;}
		const char*
			GetName() const
				{ Check_Object(this); return m_name; }

		void
			WriteNotes(MemoryStream *stream);

	protected:
		MString
			m_name;

		void
			SetDirty()
				{Check_Object(this); Check_Object(m_notationFile); m_notationFile->SetDirty();}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Entry access
	//
	public:
		bool
			IsEmpty()
				{Check_Object(this); return m_notes.IsEmpty();}

		bool
			DoesNoteExist(const char *entryname)
				{Check_Object(this); return FindNote(entryname) != NULL;}
		Note*
			FindNote(const char *entryname);
		Note*
			GetNote(unsigned index);

		typedef ChainIteratorOf<Note*> NoteIterator;
		NoteIterator*
			MakeNoteIterator()
				{Check_Object(this); return new NoteIterator(&m_notes);}

		ChainOf<Note*>*
			MakeNoteChain(const char* prefix);

		Note*
			AddNote(const char* entryname);
		Note*
			SetNote(const char* entryname);

		void
			DeleteNote(const char *entryname);
		void
			DeleteAllNotes();

	protected:
		ChainOf<Note*>
			m_notes;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// string access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				const char **contents,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				const char *contents
			);
		void
			AppendEntry(
				const char *entryname,
				const char *contents
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// int access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				int *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				int value
			);
		void
			AppendEntry(
				const char *entryname,
				int value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// scalar access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				Scalar *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				Scalar value
			);
		void
			AppendEntry(
				const char *entryname,
				Scalar value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// bool access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				bool *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				bool value
			);
		void
			AppendEntry(
				const char *entryname,
				bool value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Vector3D access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				Vector3D *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				const Vector3D &value
			);
		void
			AppendEntry(
				const char *entryname,
				const Vector3D &value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// YawPitchRoll access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				YawPitchRoll *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				const YawPitchRoll &value
			);
		void
			AppendEntry(
				const char *entryname,
				const YawPitchRoll &value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// UnitQuaternion access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				UnitQuaternion *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				const UnitQuaternion &value
			);
		void
			AppendEntry(
				const char *entryname,
				const UnitQuaternion &value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Motion3D access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				Motion3D *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				const Motion3D &value
			);
		void
			AppendEntry(
				const char *entryname,
				const Motion3D &value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// RGBColor access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				RGBColor *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				const RGBColor &value
			);
		void
			AppendEntry(
				const char *entryname,
				const RGBColor &value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// RGBAColor access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				RGBAColor *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				const RGBAColor &value
			);
		void
			AppendEntry(
				const char *entryname,
				const RGBAColor &value
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// NotationFile access
	//
	public:
		bool
			GetEntry(
				const char *entryname,
				NotationFile *value,
				bool required=false
			);
		void
			SetEntry(
				const char *entryname,
				NotationFile *value
			);
		void
			AppendEntry(
				const char *entryname,
				NotationFile *value
			);

	public:
		void
			TestInstance() const;
	};

}
