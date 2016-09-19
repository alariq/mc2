
//===========================================================================//
// File:	 MFCPlatform.hpp												 //
// Contents: Setup GameOS for running DLL's									 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#define WIN32_EXTRA_LEAN			// Reduce windows header size
#define VC_EXTRALEAN				// Reduce windows header size
#include<windows.h>
#include<platform.hpp>
#include<gameos.hpp>

long _stdcall ProcessException( EXCEPTION_POINTERS* ep );
void _stdcall InitExceptionHandler( char* CommandLine );
void _stdcall InitGameOS( HINSTANCE hInstance, HWND hWindow, char* CommandLine );
LRESULT CALLBACK GameOSWinProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
DWORD _stdcall RunGameOSLogic();
void _stdcall ExitGameOS();

