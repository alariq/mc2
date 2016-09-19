#pragma once
//===========================================================================//
// File:	 Platform.cpp													 //
// Contents: Setup GameOS for running games, MFC or DLL's					 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//


typedef enum PlatformType
{
	Platform_Game,
	Platform_MFC,
	Platform_DLL
};



extern PlatformType Platform;

extern void _stdcall RunFromOtherApp( HINSTANCE hInstance, HWND hWnd, char* CommandLine );
extern int APIENTRY RunFromWinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow );
