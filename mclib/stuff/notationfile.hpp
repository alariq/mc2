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
#include"scalar.hpp"
#include"filestream.hpp"
#include"tree.hpp"

namespace Stuff {

	class NotationFile;
	class Page;
	class Note;

	class Macro;
	typedef TreeOf<Macro *, MString> MacroTree;

	class Macro:
		public Plug
	{
	public:
		MString m_macro;
		MString m_replacement;
		bool m_inUse;

		Macro(
			MString *macro,
			MString *replace
		);

		static void
			AddValue(
				MacroTree *macro_tree,
				const char* name,
				const char* value
			);

		static void
			ReplaceMacros(
				MacroTree *macro_tree,
				const char *buffer,
				char *new_buf,
				int new_buf_size
			);
	};

	//=======================================================================
	// Format of notation file:
	//
	// !include file1.txt
	// !include=file1a.txt
	// !include "file2.txt"
	//
	// !example_macro=Field2Data
	//
	// // comment
	//
	// [RecordName]	// comment
	// FieldName=FieldData
	// Field2Name=Field2Data
	// Field3Name=test.ini
	//
	// [Record2Name]
	// FieldName=FieldData
	// Field4Name=$(example_macro)
	// Field5Name={
	//  [Record3Name]
	//  Field6Name=Whatever
	// }
	//
	// /* [Page3Name]
	// Field7Name=Uhhm
	// Field8Name=Uhhh */
	// ...
	//
	//=======================================================================

	//##########################################################################
	//##############    NotationFile    ########################################
	//##########################################################################

	class NotationFile
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
		friend class Page;
		friend class Note;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor/Destructors
	//
	public:
		enum Type {
			Standard,
			NonEmpty,
			Raw
		};

		NotationFile(
			const char *file_name,
			Type type=Standard
		);
		NotationFile(
			MemoryStream *stream=NULL,
			MacroTree *macro_tree=NULL
		);

		~NotationFile();

		void
			TestInstance() const;
		static bool
			TestClass();

	protected:
		void
			CommonConstruction(
				MemoryStream *memory_stream,
				TreeOf<Macro*, MString> *macro_tree
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Stream access
	//
	public:
		const FileDependencies*
			GetFileDependencies() const
				{Check_Object(this); return &m_fileDependencies;}

		const char*
			GetFileName() const
				{Check_Object(this); return m_fileName;}

		void
			SaveAs(const char* file_name);
		void
			Save();

		void
			IgnoreChanges()
				{Check_Object(this); m_dirtyFlag = false;}

		bool
			IsChanged() const
				{Check_Object(this); return m_dirtyFlag;}

	protected:
		void
			Read(
				MemoryStream *stream,
				MacroTree *macro_tree,
				Page **page,
				bool nested
			);
		void
			Write(MemoryStream *stream);

		void
			ProcessLine(
				MemoryStream *stream,
				MacroTree *macro_tree,
				Page **notepage,
				char* buffer
			);

		void
			HandleBangStuff(
				char *buffer,
				MacroTree *macro_tree,
				Page **page
			);

		void
			SetDirty()
				{ Check_Object(this); m_dirtyFlag = true; }

		Stuff::MString
			m_fileName;
		bool
			m_dirtyFlag;
		Type
			m_type;
		FileDependencies
			m_fileDependencies;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Page access
	//
	public:
		bool
			IsEmpty()
				{Check_Object(this); return m_pages.IsEmpty();}

		bool
			DoesPageExist(const char *pagename)
				{Check_Object(this); return FindPage(pagename) != NULL;}
		Page*
			FindPage(const char* pagename);
		Page*
			GetPage(unsigned index);
		Page*
			GetPage(const char* pagename);

		typedef ChainIteratorOf<Page*> PageIterator;
		PageIterator*
			MakePageIterator()
				{Check_Object(this); return new PageIterator(&m_pages);}

		Page*
			AddPage(const char *pagename);
		Page*
			SetPage(const char *pagename);

		void
			DeletePage(const char *pagename);
		void
			DeleteAllPages();

	protected:
		ChainOf<Page*>
			m_pages;
	};

}
