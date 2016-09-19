#pragma once
//===========================================================================//
// File:	 Globals.hpp													 //
// Contents: global variables												 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//



//System Setup
extern DWORD TerminateGame;

extern DWORD gHardwareMouse;


//
// Processor information
//
extern int HasCPUID;			// 1 when at least a Pentium is being used.
extern int HasMMX;				// 1 when MMX instructions are present
extern int HasRDTSC;			// 1 when RDTSC instruction is present
extern int HasKatmai;			// 1 when Katmai instructions available
extern int Has3DNow;			// 1 when 3DNox instructions available
extern BYTE CPUID0[17];		// CPUID 0 information (+ terminating 0)
extern BYTE CPUID1[16];		// CPUID 1 information
extern BYTE CPUID2[16];		// CPUID 2 information

//
// Global window 
//
extern bool gActive;
extern bool gGotFocus;
extern DWORD MouseInWindow;		// 1=Mouse over window in a window mode



//
//
//
void CheckThreads();
void DestroyThreads();
