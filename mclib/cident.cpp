//---------------------------------------------------------------------------
//
// cident.cpp - This file contains some misc string classes
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files

#ifndef CIDENT_H
#include"cident.h"
#endif

#include"heap.h"
#include<ctype.h>

#include"windows.h"

#ifndef _MBCS
#include<gameos.hpp>
#else
#include<assert.h>
#define gosASSERT assert
#define gos_Malloc malloc
#define gos_Free free
#endif

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//								FullPathFileName
//---------------------------------------------------------------------------
FullPathFileName::~FullPathFileName (void)
{
	destroy();
}

//---------------------------------------------------------------------------
void FullPathFileName::destroy (void)
{
	if (fullName)
		systemHeap->Free(fullName);

	fullName = NULL;
}

//---------------------------------------------------------------------------
void FullPathFileName::init (const char * dir_path, const char * name, const char * ext)
{
	destroy();

	long total_length = strlen(dir_path);
	total_length += strlen(name);
	total_length += strlen(ext);
	total_length++;


	fullName = (char *)systemHeap->Malloc(total_length);
	gosASSERT(fullName != NULL);
	fullName[0] = 0;

	if ( strstr( name, dir_path ) != name )
		strcpy(fullName,dir_path);
	strcat(fullName,name);

	// don't append if its already there
	if (ext && stricmp( fullName + strlen( fullName ) - strlen( ext ), ext ) != 0)
		strcat(fullName,ext);

	CharLower(fullName);
}

void FullPathFileName::changeExt (const char *from, const char *to)
{
	if (strlen(from) != strlen(to))
		return;

	char *ext = strstr(fullName,from);
	if (ext)
	{
		for (unsigned long i=0;i<strlen(to);i++)
			ext[i] = to[i];
	}
}
//---------------------------------------------------------------------------
//
// Edit Log
//
// $Log$
//
//---------------------------------------------------------------------------
