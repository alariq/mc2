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
void
	Note::WriteNotation(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	if (m_name)
		*stream << (const char*)m_name;
	if (m_text)
		*stream << '=' << (const char*)m_text;
	*stream << "\r\n";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(int *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	*value = atoi(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(int value)
{
	Check_Object(this);

	char contents[12];
	_itoa(value, contents, 10);
	Verify(strlen(contents) < sizeof(contents));

	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(Scalar *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	*value = AtoF(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(Scalar value)
{
	Check_Object(this);

	char contents[32];
	sprintf(contents, "%f", value);
	Verify(strlen(contents) < sizeof(contents));

	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(bool *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	*value = (!S_stricmp(contents, "true") || !S_stricmp(contents, "yes") || atoi(contents) != 0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(bool value)
{
	Check_Object(this);

	SetEntry((value)?"true":"false");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(Vector3D *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	int count =
		sscanf(
			contents,
			"%f %f %f",
			&value->x,
			&value->y,
			&value->z
		);
	if (count != 3)
	{
		Page *page = m_page;
		Check_Object(page);
		NotationFile *file = page->m_notationFile;
		Check_Object(file);
		STOP((
			"%s: {[%s]%s=%s} is not a Vector!",
			file->GetFileName(),
			(const char*)page->m_name,
			(const char*)m_name,
			contents
		));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(const Vector3D &value)
{
	Check_Object(this);

	static char contents[64];
	int temp;
	temp = sprintf(
		contents,
		"%f %f %f",
		value.x,
		value.y,
		value.z
	);
	Verify (temp <sizeof(contents));
	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(YawPitchRoll *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	int count =
		sscanf(
			contents,
			"%f %f %f",
			&value->yaw,
			&value->pitch,
			&value->roll
		);
	if (count != 3)
	{
		Page *page = m_page;
		Check_Object(page);
		NotationFile *file = page->m_notationFile;
		Check_Object(file);
		STOP((
			"%s: {[%s]%s=%s} is not a YawPitchRoll!",
			file->GetFileName(),
			(const char*)page->m_name,
			(const char*)m_name,
			contents
		));
	}
	value->yaw *= Radians_Per_Degree;
	value->pitch *= Radians_Per_Degree;
	value->roll *= Radians_Per_Degree;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(const YawPitchRoll &value)
{
	Check_Object(this);

	static char
		contents[32];
	sprintf(
		contents,
		"%f %f %f",
		value.yaw * Degrees_Per_Radian,
		value.pitch * Degrees_Per_Radian,
		value.roll * Degrees_Per_Radian
	);
	Verify(strlen(contents) < sizeof(contents));
	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(UnitQuaternion *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	YawPitchRoll ypr;
	int count =
		sscanf(
			contents,
			"%f %f %f",
			&ypr.yaw,
			&ypr.pitch,
			&ypr.roll
		);
	if (count != 3)
	{
		Page *page = m_page;
		Check_Object(page);
		NotationFile *file = page->m_notationFile;
		Check_Object(file);
		STOP((
			"%s: {[%s]%s=%s} is not a UnitQuaternion!",
			file->GetFileName(),
			(const char*)page->m_name,
			(const char*)m_name,
			contents
		));
	}
	ypr.yaw *= Radians_Per_Degree;
	ypr.pitch *= Radians_Per_Degree;
	ypr.roll *= Radians_Per_Degree;
	*value = ypr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(const UnitQuaternion &value)
{
	Check_Object(this);

	static char
		contents[32];
	YawPitchRoll ypr(value);
	Verify(strlen(contents) < sizeof(contents));
	sprintf(
		contents,
		"%f %f %f",
		ypr.yaw * Degrees_Per_Radian,
		ypr.pitch * Degrees_Per_Radian,
		ypr.roll * Degrees_Per_Radian
	);
	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(Motion3D *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	int count =
		sscanf(
			contents,
			"%f %f %f %f %f %f",
			&value->linearMotion.x,
			&value->linearMotion.y,
			&value->linearMotion.z,
			&value->angularMotion.x,
			&value->angularMotion.y,
			&value->angularMotion.z
		);
	if (count != 6)
	{
		Page *page = m_page;
		Check_Object(page);
		NotationFile *file = page->m_notationFile;
		Check_Object(file);
		STOP((
			"%s: {[%s]%s=%s} is not a Motion!",
			file->GetFileName(),
			(const char*)page->m_name,
			(const char*)m_name,
			contents
		));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(const Motion3D &value)
{
	Check_Object(this);

	static char
		contents[64];
	sprintf(
		contents,
		"%f %f %f %f %f %f",
		value.linearMotion.x,
		value.linearMotion.y,
		value.linearMotion.z,
		value.angularMotion.x,
		value.angularMotion.y,
		value.angularMotion.z
	);
	Verify(strlen(contents) < sizeof(contents));
	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(RGBColor *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	int count =
		sscanf(
			contents,
			"%f %f %f",
			&value->red,
			&value->green,
			&value->blue
		);
	if (count != 3)
	{
		Page *page = m_page;
		Check_Object(page);
		NotationFile *file = page->m_notationFile;
		Check_Object(file);
		STOP((
			"%s: {[%s]%s=%s} is not an RGBColor!",
			file->GetFileName(),
			(const char*)page->m_name,
			(const char*)m_name,
			contents
		));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(const RGBColor &value)
{
	Check_Object(this);

	static char
		contents[32];
	sprintf(
		contents,
		"%f %f %f",
		value.red,
		value.green,
		value.blue
	);
	Verify(strlen(contents) < sizeof(contents));
	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(RGBAColor *value)
{
	Check_Object(this);
	Check_Pointer(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);
	int count =
		sscanf(
			contents,
			"%f %f %f %f",
			&value->red,
			&value->green,
			&value->blue,
			&value->alpha
		);
	if (count == 3)
		value->alpha = 1.0f;
	if (count < 3)
	{
		Page *page = m_page;
		Check_Object(page);
		NotationFile *file = page->m_notationFile;
		Check_Object(file);
		STOP((
			"%s: {[%s]%s=%s} is not an RGBAColor!",
			file->GetFileName(),
			(const char*)page->m_name,
			(const char*)m_name,
			contents
		));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(const RGBAColor &value)
{
	Check_Object(this);

	static char
		contents[48];
	sprintf(
		contents,
		"%f %f %f %f",
		value.red,
		value.green,
		value.blue,
		value.alpha
	);
	Verify(strlen(contents) < sizeof(contents));
	SetEntry(contents);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::GetEntry(NotationFile *value)
{
	Check_Object(this);
	Check_Object(value);

	const char *contents = NULL;
	GetEntry(&contents);
	Check_Pointer(contents);

	//
	//-------------------------------------------------------
	// If this is a file reference, open the file and read it
	//-------------------------------------------------------
	//
	if (strncmp(contents, "{\r\n", 3))
	{
		FileStream file(contents);
		value->m_fileName = file.GetFileName();
		value->m_fileDependencies.AddDependency(&file);
		value->Read(&file, NULL, NULL, false);
	}

	//
	//--------------------------------------------------------------------
	// Otherwise, we are reading a nested file, so set up the dependencies
	//--------------------------------------------------------------------
	//
	else
	{
		MemoryStream stream(const_cast<char*>(contents+3), strlen(contents)-3);
		NotationFile *parent_file = m_page->GetNotationFile();
		Check_Object(parent_file);
		value->m_fileDependencies.AddDependencies(
			parent_file->GetFileDependencies()
		);

		//
		//-----------------------------------
		// Figure out the name of the subfile
		//-----------------------------------
		//
		if (parent_file->GetFileName())
		{
			MString name(parent_file->GetFileName());
			name += '[';
			name += GetName();
			name += ']';
			value->m_fileName = name;
		}

		//
		//--------------------
		// Read the string now
		//--------------------
		//
		value->Read(&stream, NULL, NULL, true);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::SetEntry(NotationFile *value)
{
	Check_Object(this);
	Check_Object(value);

	//
	//----------------------------------------------------
	// If this isn't a nested file, write out the filename
	//----------------------------------------------------
	//
	const char* name = value->GetFileName();
	if (name && name[strlen(name)-1] != ']')
		SetEntry(name);

	//
	//-------------------------------------------------------------------------
	// Otherwise, we need to write the notation file out to a memory stream and
	// assign that to the entry
	//-------------------------------------------------------------------------
	//
	else
	{
		DynamicMemoryStream file_buffer(5);
		file_buffer << "{\r\n";
		value->Write(&file_buffer);
		file_buffer << "}" << '\0';
		file_buffer.Rewind();
		SetEntry(static_cast<const char*>(file_buffer.GetPointer()));
		value->IgnoreChanges();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Note::TestInstance() const
{
}
