//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include<windows.h>
#include<stdio.h>
#include"msodw.h"

#include"utilities.h"
#include "..\resource.h"

#include<gameos.hpp>

long _stdcall ProcessException( EXCEPTION_POINTERS* ep );
extern HWND			hWindow;
void EnterWindowMode();

#define EXTERNAL
//--------------------------------------------------------------------------------------------
//DW Exception Code for MC2.
//
// Simple, really.  Setup the exception filter by calling InitDW.
// After DW is inited, ANY exception which occurs in ANY thread will be handed
// to DW first.  Any problem with running DW OR when DW ends, the GameOS exception handler
// will triggered to give even more data to the user.
//
//--------------------------------------------------------------------------------------------

char WatsonCrashMessage[4096];

//Will be allocated below.
WCHAR *WatsonCrashMessageUnicode = NULL;

/*
 * AnsiToUnicode converts the ANSI string pszA to a Unicode string
 * and returns the Unicode string through ppszW. Space for the
 * the converted string is allocated by AnsiToUnicode.
 */ 

HRESULT __fastcall AnsiToUnicode(LPCSTR pszA, LPOLESTR* ppszW)
{
    ULONG cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (NULL == pszA)
    {
        *ppszW = NULL;
        return NOERROR;
    }

    // Determine number of wide characters to be allocated for the
    // Unicode string.
    cCharacters =  strlen(pszA)+1;

    // Use of the OLE allocator is required if the resultant Unicode
    // string will be passed to another COM component and if that
    // component will free it. Otherwise you can use your own allocator.
    *ppszW = (LPOLESTR)malloc(cCharacters*2);
    if (NULL == *ppszW)
        return E_OUTOFMEMORY;

    // Covert to Unicode.
    if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,
                  *ppszW, cCharacters))
    {
        dwError = GetLastError();
        free(*ppszW);
        *ppszW = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }

    return NOERROR;
}

//----------------------------------------------------------------------------
//	ORIGINAL source from testCrash.cpp.
//	%%Function: DwHandleException
//	%%Contact: MRuhlen

//	Calls DW in the event of an exception
//	Calls GameOS exception handler AFTER DW has had a crack at it.
//----------------------------------------------------------------------------

LONG WINAPI DwExceptionFilter(LPEXCEPTION_POINTERS pep)
{
	EXCEPTION_RECORD *per;
	HANDLE hFileMap;
	DWSharedMem *pdwsm;
	SECURITY_ATTRIBUTES  sa;
	
	//------------------------------------------------------------------------------------------------------------
	// we keep local copies of these in case another thread is trashing memory
	// it much more likely to trash the heap than our stack
	HANDLE hEventDone;          // event DW signals when done
	HANDLE hEventAlive;         // heartbeat event DW signals per EVENT_TIMEOUT
	HANDLE hMutex;              // to protect the signaling of EventDone  
	
	char szCommandLine[MAX_PATH * 2];
	
	DWORD dw;
	BOOL fDwRunning;  
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	//------------------------------------------------------------------------------------------------------------
	// init - Check if we just hit a breakpoint.  If so, continue execution.  Debugger will take care of itself.
	per = pep->ExceptionRecord;
	if (EXCEPTION_BREAKPOINT == per->ExceptionCode)
		return 0;

	//------------------------------------------------------------------------------------------------------------
	// create shared memory
	memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	
	hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, sizeof(DWSharedMem), NULL);
	if (hFileMap == NULL)
	{
		//At this point, call the GameOS exception handler and convert the pep to the data they need!
		ProcessException(pep);
		return 1;
	}
		
	pdwsm = (DWSharedMem *) MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (pdwsm == NULL)
	{
		//At this point, call the GameOS exception handler and convert the pep to the data they need!
		ProcessException(pep);
		return 1;
	}

	memset(pdwsm, 0, sizeof(DWSharedMem));

	hEventAlive = CreateEvent(&sa, FALSE, FALSE, NULL);
	hEventDone = CreateEvent(&sa, FALSE, FALSE, NULL);
	hMutex = CreateMutex(&sa, FALSE, NULL);

	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &pdwsm->hProc, PROCESS_ALL_ACCESS, TRUE, 0))
	{
		//At this point, call the GameOS exception handler and convert the pep to the data they need!
		ProcessException(pep);
		return 1;
	}

	if (hEventAlive == NULL || hEventDone == NULL || hMutex == NULL || pdwsm->hProc == NULL)
	{
		//At this point, call the GameOS exception handler and convert the pep to the data they need!
		ProcessException(pep);
		return 1;
	}

	//------------------------------------------------------------------------------------------------------------
	// setup interface structure
	pdwsm->pid = GetCurrentProcessId();
	pdwsm->tid = GetCurrentThreadId();
	pdwsm->hEventAlive = hEventAlive;
	pdwsm->hEventDone = hEventDone;
	pdwsm->hMutex = hMutex;
	pdwsm->dwSize = sizeof(DWSharedMem);
	pdwsm->pep = pep;
	pdwsm->eip = (DWORD) pep->ExceptionRecord->ExceptionAddress;
	pdwsm->bfmsoctdsOffer = msoctdsQuit;
	pdwsm->bfmsoctdsLetRun = msoctdsQuit;
	pdwsm->bfDWBehaviorFlags = fDwCheckSig;

	strcpy(pdwsm->szFormalAppName, Environment.applicationName);
	strcpy(pdwsm->szInformalAppName, "MechCommander 2");

	strcpy(pdwsm->szRegSubPath, "Software\\Microsoft\\Microsoft Games\\");
	strcat(pdwsm->szRegSubPath, Environment.applicationName);

//	strcpy(pdwsm->szLCIDKeyValue, "");
//	strcpy(pdwsm->szPIDRegKey, "HKLM\\Software\\Microsoft\\Internet Explorer\\Registration\\DigitalProductID");

#if defined(FINAL) || defined(EXTERNAL)
	strcpy(pdwsm->szServer, "watson.microsoft.com"); 
#else
	strcpy(pdwsm->szServer, "officewatson");
#endif

	wcscpy(pdwsm->wzErrorMessage, WatsonCrashMessageUnicode);

	//Leave this alone?  No idea what it does.  Not in Docs.
	// OK, I kinda know now.  These are DLLs that Watson can check for goodness at crash time.
	// COOL, because the end user might have mucked with the EXE or data and this allows us to
	// report that information back to the server.
	memcpy(pdwsm->wzDotDataDlls, L"mc2res.dll\0editores.dll\0", 24 * sizeof(WCHAR));

	GetModuleFileNameA(NULL, pdwsm->szModuleFileName, DW_MAX_PATH);

	//Additional Files for MechCommander?  Should there be any?  Log files, etc.

	// ok, now we don't want to accidently change this
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	wsprintfA(szCommandLine, "dw -x -s %u", (DWORD) hFileMap); 

	//Check if we are in fullScreen mode.  If so, switch back to WindowMode to insure DW screen comes up.
	if(Environment.fullScreen && hWindow )
		EnterWindowMode();

	if (CreateProcessA(NULL, szCommandLine, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		fDwRunning = TRUE;
		while (fDwRunning)
		{
			if (WaitForSingleObject(hEventAlive, DW_TIMEOUT_VALUE * 100) == WAIT_OBJECT_0)
			{
				if (WaitForSingleObject(hEventDone, 1) == WAIT_OBJECT_0)
				{
					fDwRunning = FALSE;
				}

				continue;
			}
				
			 // we timed-out waiting for DW to respond, try to quit
			dw = WaitForSingleObject(hMutex, DW_TIMEOUT_VALUE);
			if (dw == WAIT_TIMEOUT)
			{
				fDwRunning = FALSE; // either DW's hung or crashed, we must carry on  
			}
			else if (dw == WAIT_ABANDONED)
			{
				fDwRunning = FALSE;
				ReleaseMutex(hMutex);
			}
			else
			{
				// DW has not woken up?
				if (WaitForSingleObject(hEventAlive, 1) != WAIT_OBJECT_0)
				// tell DW we're through waiting for it's sorry self
				{
					SetEvent(hEventDone);
					fDwRunning = FALSE;
				}
				else
				{
					// are we done
					if (WaitForSingleObject(hEventDone, 1) == WAIT_OBJECT_0)
						fDwRunning = FALSE;
				}

				ReleaseMutex(hMutex);
			}
		}

#if 0		
		// did we get attached?
		// Again, do NOT term the current APP.  Late GameOS have its shot at the exception.
		if (WaitForSingleObject(hEventDBAttach, 1) == WAIT_OBJECT_0)
		{
			// yes, die
			MessageBox(NULL, "DB Attach ", "out", MB_OK);
			CloseHandle(hEventAlive);
			CloseHandle(hEventDone);
			CloseHandle(hMutex);
			TerminateProcess(GetCurrentProcess(), 0);
		}
#endif		
		// no, clean up
		CloseHandle(hEventAlive);
		CloseHandle(hEventDone);
		CloseHandle(hMutex);
	} // end if CreateProcess succeeded
	
	UnmapViewOfFile(pdwsm);
	CloseHandle(hFileMap);

	//At this point, call the GameOS exception handler and convert the pep to the data they need!
	ProcessException(pep);
	return 1;
}

//----------------------------------------------------------------------------
void InitDW (void)
{
	cLoadString(IDS_WATSON_CRASH_MSG,WatsonCrashMessage,4095);
	AnsiToUnicode(WatsonCrashMessage,&WatsonCrashMessageUnicode);

	SetUnhandledExceptionFilter(DwExceptionFilter);
}

//----------------------------------------------------------------------------

