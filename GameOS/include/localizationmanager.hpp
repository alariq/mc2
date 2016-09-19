#pragma once
//==========================================================================//
// File:	 LocalizationManager.hpp										//
// Contents: Localizer routines												//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
		



struct hResource
{
	hResource*	pNext;
	
	HINSTANCE	hInstance;
	char		Name[256];
	DWORD		ReferenceCount;

} ;

struct gos_CaretInfo
{
	DWORD		fontX;
	DWORD		fontY;
	float		fontSize;
	HGOSFONT3D	fontHandle;
	DWORD		fontColorComp;
	DWORD		fontColorCand;
	bool		fontBold;
	bool		fontItalic;
	bool		fontProportional;
	RECT		margins ;
} ;

void TextDrawDBCS(  char *Message );

void InitLocalization();
void DestroyLocalization();

void LocalizationSetPurgeFlags();
void LocalizationPurge();
void LocalizationShutdown();
void RenderIME();

extern DWORD gEnableLoc;
