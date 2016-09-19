//========================================================================
// PERF.h
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

//========================================================================
// types
//========================================================================
typedef HRESULT (WINAPI * PERFMONCALLBACK)(char *szDescription, DWORD dwFlags, DWORD dwId);

//========================================================================
// external functions
//========================================================================
extern HRESULT WINAPI InitPerformanceMonitorLibrary(void);
extern HRESULT WINAPI TermPerformanceMonitorLibrary(void);

extern HRESULT WINAPI EnumeratePerformanceMonitors(PERFMONCALLBACK lpfCallback);
extern HRESULT WINAPI SetPerformanceMonitor(DWORD dwId);
extern double DecodePerformanceMonitors( __int64 *Registers );
extern BOOL WINAPI ReadPerformanceMonitorRegisters(__int64 *result);


//========================================================================
// errors
//========================================================================
#define PERF_ERROR_ALREADYINITIALIZED			-1
#define PERF_ERROR_OWNED_BY_ANOTHER_APP			-2
#define PERF_ERROR_CANTCREATEMUTEX				-3
#define PERF_ERROR_NOTINITIALIZED				-4
#define PERF_ERROR_INVALID_CPU					-5
#define PERF_ERROR_CANT_LOAD_VXD				-6
#define PERF_ERROR_SET_FAILED					-7

