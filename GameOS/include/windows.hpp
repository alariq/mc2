#pragma once
//===========================================================================//
// File:	 Windows.hpp													 //
// Contents: Window-related routines										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"globals.hpp"

char* GetWindowsMessage( DWORD uMsg,WPARAM wParam,LPARAM lParam );
void InitializeWindows();
void DestroyWindows();
bool AlreadyRunning();
void SystemCheck();
void Update();

enum Status { BeforeInit,Uninitialized, GameInit, Running, Paused, AfterExit };

extern HDC			DesktopDC;
extern HWND			hWindow;
extern HINSTANCE	hInstance;
extern WNDCLASS		wndClass;
extern char*		CommandLine;
extern MSG			msg;
extern Status		status;
extern BOOL			fullScreen;
extern POINT		clientToScreen;
extern int			widthX;
extern int			heightY;
extern int			WindowStartX;
extern int			WindowStartY;
extern bool			WindowsNT;
extern DWORD		HitScrollLock;
extern DWORD		ForceReStart;
extern DWORD		DesktopBpp;
extern DWORD		DesktopRes;
extern bool			PerfCounters;
extern bool			WindowClosed;
//
// Libraries.cpp routines
//
void InitLibraries();		// Load all libraries GOS depends upon
void DestroyLibraries();	// Unload any libraries GOS loaded
char* GetDirectXVersion();	// Returns a string describing the current version of DirectX


//
// Functions only in Win98/2000
//
typedef BOOL (WINAPI* T_GetFileAttributesEx)( LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation );
typedef EXECUTION_STATE (WINAPI* T_SetThreadExecutionState)( EXECUTION_STATE esFlags );
typedef BOOL (WINAPI* T_GetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
extern T_SetThreadExecutionState _SetThreadExecutionState;
extern T_GetFileAttributesEx _GetFileAttributesEx;
extern T_GetDiskFreeSpaceEx _GetDiskFreeSpaceEx;

extern BOOL (WINAPI* _EmptyWorkingSet)(HANDLE);
extern BOOL (WINAPI* _InitializeProcessForWsWatch)(HANDLE);
extern BOOL (WINAPI* _GetWsChanges)(HANDLE,PPSAPI_WS_WATCH_INFORMATION,DWORD);
extern DWORD (WINAPI* _GetMappedFileName)(HANDLE,LPVOID,LPTSTR,DWORD);



extern HRESULT (WINAPI* _DirectDrawCreateEx)( GUID* lpGUID, void** lplpDD, REFIID iid, IUnknown* pUnkOuter ); 
extern HRESULT (WINAPI* _DirectDrawEnumerate)( LPDDENUMCALLBACK lpCallback, LPVOID lpContext );
extern HRESULT (WINAPI* _DirectDrawEnumerateEx)( LPDDENUMCALLBACKEX lpCallback, LPVOID lpContext, DWORD Flags );
extern HRESULT (WINAPI* _DirectInputCreate)( HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT * lplpDirectInput, LPUNKNOWN punkOuter );
extern HRESULT (WINAPI* _DirectSoundCreate)( LPGUID lpGuid, LPDIRECTSOUND * ppDS, LPUNKNOWN  pUnkOuter );
extern HRESULT (WINAPI* _DirectInputCreateEx)( HINSTANCE hinst, DWORD dwVersion, REFIID refID, void ** lplpDirectInput, LPUNKNOWN punkOuter );
extern HRESULT (WINAPI* _DirectSoundEnumerate)( LPDSENUMCALLBACK lpDSEnumCallback, LPVOID lpContext );
extern HWND (WINAPI* _CallmyHelp)( HWND hWindow, HINSTANCE hInst, bool Window, DWORD HelpItem );
extern void (WINAPI* _FreeHlpLib)();
extern DWORD (WINAPI* _AMGetErrorText)( HRESULT hr, TCHAR *pBuffer, DWORD MaxLen );


//
// WinMain.cpp
//
extern float frameRate;
extern char AssetsDirectory1[MAX_PATH];
extern char AssetsDirectory2[MAX_PATH];
extern char ImageHelpPath[MAX_PATH];
extern char ApplicationName[256];
extern void DoGame();
extern void RestartGameOS();
extern char RegistryKey[256];
extern void gos_UpdateDisplay( bool Everything );
extern DWORD ThreadId;
extern DWORD SkipRendering;
extern bool gNoBlade;



