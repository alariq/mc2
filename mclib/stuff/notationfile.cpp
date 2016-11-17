//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

#define MAX_LINE_SIZE 512

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Macro::Macro(MString *macro, MString *replace):
	Plug(DefaultData)
{
	m_macro = *macro;
	m_replacement = *replace;
	m_inUse = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Macro::AddValue(
		MacroTree *macro_tree,
		const char* name,
		const char* value
	)
{
	Check_Object(macro_tree);
	Check_Pointer(name);
	Check_Pointer(value);

	//
	//----------------------------------------------------
	// Create MStrings for all this, and create a new plug
	//----------------------------------------------------
	//
	MString ms_define(name);
	MString ms_replace(value);
	Macro *mr_new = new Macro(&ms_define, &ms_replace);
	Check_Object(mr_new);

	//
	//---------------------------------------------------------
	// See if a previous definition is there.  If so, remove it
	//---------------------------------------------------------
	//
	Macro *mr_exists = NULL;
	mr_exists = macro_tree->Find(ms_define);
	if (mr_exists)
	{
		Check_Object(mr_exists);
		macro_tree->Remove(mr_exists);
	}
	macro_tree->AddValue(mr_new, ms_define);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Macro::ReplaceMacros(
		MacroTree *macro_tree,
		const char *buffer,
		char *new_buf,
		int new_buf_size
	)
{
	//
	//----------------------------------------------------
	// Copy the characters one by one until we find the $(
	//----------------------------------------------------
	//
	while (1)
	{
		while (*buffer && (*buffer != '$' || buffer[1] != '('))
		{
			*new_buf++ = *buffer++;
			--new_buf_size;
			Verify(new_buf_size > 0);
		}

		//
		//--------------------------------------
		// If we are out of buffer, search again
		//--------------------------------------
		//
		if (!*buffer)
		{
			Verify(new_buf_size > 0);
			*new_buf = 0;
			return;
		}

		//
		//-------------------------------
		// Find the end of the macro name
		//-------------------------------
		//
		const char *p = strchr(buffer, ')');
		if (!p)
		{
			Verify(new_buf_size > 0);
			*new_buf = '\0';
			return;
		}

		//
		//-------------------
		// Isolate macro name
		//-------------------
		//
		int len = p-buffer - 2;
		MString macro_name;
		macro_name.AllocateLength(len+1);
		char *t = macro_name;
		size_t i=0;
		for (p = buffer+2; *p != ')'; ++p,++i)
		{
			Verify(i<len);
			t[i] = *p;
		}
		t[len] = '\0';

		//
		//--------------------------------------------------------------------
		// Find it in the tree, and if it exists, copy it into the buffer.  If
		// not, leave the buffer alone
		//--------------------------------------------------------------------
		//
		buffer = p+1;
		Macro *macro = macro_tree->Find(macro_name);
		if (macro && !macro->m_inUse)
		{
			macro->m_inUse = true;
			ReplaceMacros(
				macro_tree,
				macro->m_replacement,
				new_buf,
				new_buf_size
			);
			len = strlen(new_buf);
			new_buf_size -= len;
			new_buf += len;
			macro->m_inUse = false;
		}
	}
}

//#############################################################################
//##############    NotationFile    ###########################################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
NotationFile::NotationFile(
	const char *file_name,
	Type type
):
	m_pages(NULL)
{
	Check_Pointer(this);
	Check_Pointer(file_name);

	//
	//-----------------------------------------
	// See if we need to read stuff from a file
	//-----------------------------------------
	//
	FileStream file_stream;
	if (gos_DoesFileExist(file_name))
	{
		file_stream.Open(file_name);
		m_fileName = file_stream.GetFileName();
		m_fileDependencies.AddDependency(&file_stream);
	}
	else
		m_fileName = file_name;

	//
	//---------------------------------------------------------------
	// Set up the file dependencies, and finish constructing the file
	//---------------------------------------------------------------
	//
	m_type = type;
	CommonConstruction(&file_stream, NULL);

	//
	//-----------------------------------------------------------
	// Make sure that we don't allow a non-empty file to be empty
	//-----------------------------------------------------------
	//
	if (m_type==NonEmpty && IsEmpty())
		STOP(("%s cannot be empty!", file_name));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
NotationFile::NotationFile(
	MemoryStream *memory_stream,
	MacroTree *macro_tree
):
	m_pages(NULL)
{
	Check_Pointer(this);

	//
	//--------------------------------------------------------------------
	// If we were given a stream and it was a file, get the dependency and
	// filename info from it
	//--------------------------------------------------------------------
	//
	if (memory_stream)
	{
		Check_Object(memory_stream);
		if (memory_stream->IsDerivedFrom(FileStream::DefaultData))
		{
			FileStream* file_stream = Cast_Object(FileStream*, memory_stream);
			m_fileDependencies.AddDependency(file_stream);
			m_fileName = file_stream->GetFileName();
		}
	}

	//
	//-------------------
	// Finish building it
	//-------------------
	//
	CommonConstruction(memory_stream, macro_tree);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
NotationFile::~NotationFile()
{
	Check_Object(this);

	//
	//------------------------
	// Clear out the notepages
	//------------------------
	//
	Verify(!IsChanged());
	PageIterator pages(&m_pages);
	pages.DeletePlugs();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::TestInstance() const
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::CommonConstruction(
		MemoryStream *memory_stream,
		MacroTree *macro_tree
	)
{
	Check_Pointer(this);

	//
	//-------------------
	// Read the stream in
	//-------------------
	//
	if (memory_stream)
	{
		Check_Object(memory_stream);
		Read(memory_stream, macro_tree, NULL, false);
	}
	m_dirtyFlag = false;
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::Read(
		MemoryStream *stream,
		MacroTree *macro_tree,
		Page **page,
		bool nested
	)
{
	Check_Object(this);
	Check_Pointer(stream);

	//
	//------------------------------------------------------------------
	// If aren't already reading a page, make a pointer to hold the info
	//------------------------------------------------------------------
	//
	Page *temp_page = NULL;
	if (!page)
		page = &temp_page;

	//
	//------------------------------------------------
	// If we don't have a macro tree yet, make one now
	//------------------------------------------------
	//
	MacroTree *orig_tree = macro_tree;
	if (!macro_tree)
	{
		macro_tree = new MacroTree(NULL, true);
		Check_Object(macro_tree);
	}

	//
	//--------------------------------
	// Read each line till we are done
	//--------------------------------
	//
	bool comment_mode = false;
	char buffer[MAX_LINE_SIZE];
	while (stream->ReadLine(buffer, sizeof(buffer)))
	{		
		char *p = buffer;

		//
		//----------------------------------------------------------------
		// Deal with any comments - C++ style are the easiest to deal with
		//----------------------------------------------------------------
		//
		char *comment_start = NULL;
		while ((p = strchr(p, '/')) != NULL)
		{
			if (p[1] == '/')
				break;

			//
			//--------------------------------------------------------------
			// If we are ending a C style comment, move the memory to where
			// the comment started on this line.  If it didn't start on this
			// line, move it to the beginning of the buffer
			//--------------------------------------------------------------
			//
			else if (p[-1] == '*')
			{
				if (!comment_mode)
				{
					strcpy(p-1, p+1);
					--p;
				}
				else if (!comment_start)
				{
					strcpy(buffer, p+1);
					p = buffer;
				}
				else
				{
					strcpy(comment_start, p+1);
					p = comment_start;
				}
				comment_start = NULL;
				comment_mode = false;
				continue;
			}

			//
			//------------------------------------------------------------
			// If we are starting a C style comment, remember where we are
			//------------------------------------------------------------
			//
			else if (p[1] == '*')
			{
				comment_mode = true;
				comment_start = p;
			}
			++p;
		}

		//
		//--------------------------------------------------------------------
		// If we started a comment on this line, clip the line there.  If not,
		// and we are within a C comment block, skip the whole line
		//--------------------------------------------------------------------
		//
		if (comment_start)
			p = comment_start;
		else if (comment_mode)
			continue;

		//
		//---------------------
		// Clip off the comment
		//---------------------
		//
		if (p)
			*p = '\0';

		//
		//-----------------------
		// Trim ending whitespace
		//-----------------------
		//
		p = buffer + strlen(buffer) - 1;
		while (p >= buffer && (*p==' ' || *p=='\t'))
			*p-- = '\0';
		p = buffer;
		#if defined(_ARMOR)
			bool macroed=false;
		#endif

		//
		//--------------------------
		// Ignore leading whitespace
		//--------------------------
		//
Parse:
		while (*p==' ' || *p=='\t')
			++p;
		if(!*p)
			continue;

		//
		//--------------------------------------------------------------------
		// If the line consists of a nested block end, handle it appropriately
		//--------------------------------------------------------------------
		//
		if (!_stricmp(p, "}"))
		{
			if (nested)
			{
				m_dirtyFlag = false;
				return;
			}
			else
				continue;
		}

		//
		//---------------------------------------------------------------
		// If we aren't in raw mode, look for our preprocessor directives
		//---------------------------------------------------------------
		//
		if (m_type != Raw)
		{
			char *q;
			if (*p == '!')
				HandleBangStuff(p+1, macro_tree, page);

			//
			//-------------------------------------------
			// Look to see if we need to deal with macros
			//-------------------------------------------
			//
			else if((q = strchr(p, '$')) != NULL && q[1]=='(')
			{
				#if defined(_ARMOR)
					Verify(!macroed);
					macroed = true;
				#endif
				char new_buf[MAX_LINE_SIZE];
				Macro::ReplaceMacros(macro_tree, buffer, new_buf, sizeof(new_buf));
				p = new_buf;
				goto Parse;
			}
			else
				ProcessLine(stream, macro_tree, page, p);
		}

		//
		//------------------------------------------------------------
		// We are in raw mode, so just process each line as it happens
		//------------------------------------------------------------
		//
		else
			ProcessLine(stream, macro_tree, page, p);
	}
	if (comment_mode)
		PAUSE(("There is a missing */ in %s", (const char*)m_fileName));
	if (nested)
		PAUSE(("There is a missing } in %s", (const char*)m_fileName));

	//
	//------------------------------------------------------------------------
	// If we allocated the macro tree, delete it now.  The extra braces are to
	// remove the iterator needed for deletion
	//------------------------------------------------------------------------
	//
	if (!orig_tree)
	{
		{
			TreeIteratorOf<Macro*, MString> macros(macro_tree);
			macros.DeletePlugs();
		}
		Check_Object(macro_tree);
		delete macro_tree;		
	}

	//
	//---------------------
	// Nothing to save, yet
	//---------------------
	//
	m_dirtyFlag = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::Write(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	//
	//----------------------------------
	// Write each page out to the stream
	//----------------------------------
	//
	PageIterator pages(&m_pages);
	Page *page;
	while ((page = pages.ReadAndNext()) != NULL)
	{
		Check_Object(page);
		page->WriteNotes(stream);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::Save()
{
	Check_Object(this);

	//
	//------------------------------------------------------
	// If the file is dirty and has a filename, write it out
	//------------------------------------------------------
	//
	if (m_dirtyFlag && m_fileName.GetLength()>0)
	{
		FileStream output(m_fileName, FileStream::WriteOnly);
		Write(&output);
	}
	IgnoreChanges();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::SaveAs(const char* file_name)
{
	Check_Object(this);

	//
	//------------------------------------------------------
	// If the file is dirty and has a filename, write it out
	//------------------------------------------------------
	//
	FileStream output(file_name, FileStream::WriteOnly);
	m_fileName = output.GetFileName();
	Write(&output);
	IgnoreChanges();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::ProcessLine(
		MemoryStream *stream,
		MacroTree *macro_tree,
		Page **notepage,
		char *buffer
	)
{
	Check_Object(this);
	Check_Object(stream);
	Check_Object(macro_tree);
	Check_Pointer(buffer);
	Check_Pointer(notepage);

	//
	//--------------------------------
	// find first non-blank character
	//--------------------------------
	//
	char *p = buffer;
	while (*p == ' ' || *p == '\t')
		++p;

	//
	//----------------------------------------------------------------------
	// If this is a page name, isolate the name, then only add it if it is a
	// new page
	//----------------------------------------------------------------------
	//
	char *token;
	if (*p == '[')
	{
		token = p+1;
		if ((p = strchr(token, ']')) != NULL)
			*p = '\0';
		*notepage = SetPage(token);
		return;
	}

	//
	//---------------------------------------------
	// If this is a comment or empty line, stop now
	//---------------------------------------------
	//
	if (!*p)
		return;

	//
	//--------------------------------------------------------------
	// Look for an entry, and if we find one, clear out the trailing
	// whitespace on the name
	//--------------------------------------------------------------
	//
	token = p;
	char *entry = NULL;
	if ((p = strchr(token, '=')) != NULL)
	{
		*p = '\0';
		entry = p + 1;
		--p;
		while (*p == ' ' || *p == '\t')
		{
			*p = '\0';
			if (--p < token)
				break;
		}
	}

	//
	//----------------------------------------------------------------
	// If a notepage structure does not exist yet, make the empty page
	//----------------------------------------------------------------
	//
	if (!*notepage)
		*notepage = AddPage("");
	Check_Object(*notepage);

	//
	//--------------------------------------------------
	// Create the new entry if it is not a complex entry
	//--------------------------------------------------
	//
	Note *notation = (*notepage)->AddNote(token);
	Check_Object(notation);
	if (entry)
	{
		if (_stricmp(entry, "{"))
			notation->SetEntry(entry);

		//
		//----------------------------------------------------------------------
		// Now we create a new notation file to read the next part of the stream
		// into
		//----------------------------------------------------------------------
		//
		else
		{
			NotationFile nested_file;
			nested_file.Read(stream, macro_tree, NULL, true);
			DynamicMemoryStream file_buffer(5);
			file_buffer << "{\r\n";
			nested_file.Write(&file_buffer);
			file_buffer << "}" << '\0';
			file_buffer.Rewind();
			notation->m_text = static_cast<const char*>(file_buffer.GetPointer());
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::HandleBangStuff(
		char *buffer,
		MacroTree *macro_tree,
		Page **page
	)
{
	Check_Pointer(buffer);
	Check_Pointer(page);
	Check_Object(macro_tree);

    PAUSE(("")); // sebi

	//
	//-------------------------
	// Move past any whitespace
	//-------------------------
	//
	while (*buffer == ' ' || *buffer == '\t')
		++buffer;
	if (!*buffer)
		return;

	//
	//----------------------------
	// Handle an include directive
	//----------------------------
	//
	char *p;
	if (!strnicmp(buffer, "include", 7))
	{
		p = buffer+7;

		//
		//---------------------------------------
		// Look for the beginning of the filename
		//---------------------------------------
		//
		while (*p == ' ' || *p == '\t')
			++p;
		if (*p == '=')
		{
			++p;
			while (*p == ' ' || *p == '\t')
				++p;
		}

		//
		//--------------------------------------------------
		// If the filename starts with a quote, strip it off
		//--------------------------------------------------
		//
		char *file_name;
		if (*p == '"')
		{
			file_name = ++p;
			p = strrchr(p, '"');
			if (p)
				*p = '\0';
		}
		else
			file_name = p;

		//
		//---------------------------------------------------------------
		// If there is a pathname in the file, it is searched for by gos,
		// otherwise we assume it is relative to us
		//---------------------------------------------------------------
		//
		char path[MAX_LINE_SIZE];
		if (!strchr(file_name, '\\'))
		{
			Str_Copy(path, GetFileName(), sizeof(path));
			p = strrchr(path, '\\');
			if (p)
			{
				Str_Copy(p+1, file_name, sizeof(path) - (p - path) - 1);
				file_name = path;
			}
		}

		//
		//---------------
		// Now open it up
		//---------------
		//
		FileStream input(file_name);
		m_fileDependencies.AddDependency(&input);
		Read(&input, macro_tree, page, false);
	}

	//
	//------------------------------------------
	// Otherwise, we must have ourselves a macro
	//------------------------------------------
	//
	else
	{
		char *entry = NULL;

		//
		//--------------------------------
		// trim the spaces off of the name
		//--------------------------------
		//
		if ((p = strchr(buffer, '=')) != NULL)
		{
			*p = '\0';
			entry = p + 1;
			--p;
			while (*p == ' ' || *p == '\t')
			{
				*p = '\0';
				if (--p < buffer)
					break;
			}
		}

		//
		//---------------------------------------------------------
		// Make sure the name is decent and that the entry is valid
		//---------------------------------------------------------
		//
		if (!*buffer || !entry || !*entry)
			return;

		Macro::AddValue(macro_tree, buffer, entry);
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Page*
	NotationFile::FindPage(const char *pagename)
{
	Check_Object(this);
	Check_Pointer(pagename);

	PageIterator pages(&m_pages);
	Page *page;
	while ((page = pages.ReadAndNext()) != NULL)
	{
		Check_Object(page);
		const char* name = page->m_name;
		if (name && !_stricmp(name, pagename))
			return page;
	}
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Page*
	NotationFile::GetPage(const char *pagename)
{
	Page *page = FindPage(pagename);
	if (!page)
		STOP((
			"%s: [%s] is a required page!",
			GetFileName(),
			pagename
		));
	return page;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Page*
	NotationFile::SetPage(const char *pagename)
{
	Check_Object(this);
	Check_Pointer(pagename);

	Page *page = FindPage(pagename);
	if (!page)
		page = AddPage(pagename);
	Check_Object(page);
	return page;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Page*
	NotationFile::AddPage(const char *pagename)
{
	Check_Object(this);
	Check_Pointer(pagename);

	SetDirty();
	Page *page = new Page(this);
	Check_Object(page);
	page->SetName(pagename);
	m_pages.Add(page);

	return page;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::DeletePage(const char *pagename)
{
	Check_Object(this);
	Check_Pointer(pagename);

	Page *page = FindPage(pagename);
	if (page)
	{
		Check_Object(page);
		delete page;
		SetDirty();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	NotationFile::DeleteAllPages()
{
	Check_Object(this);

	PageIterator pages(&m_pages);
	Page *page;
	while ((page = pages.ReadAndNext()) != NULL)
	{
		Check_Object(page);
		delete page;
	}
}
