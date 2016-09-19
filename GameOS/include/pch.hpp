#pragma once
#define DIRECTINPUT_VERSION 0x0700
//===========================================================================//
// File:	 pch.hpp														 //
// Contents: Precompiled header file - heads that don't change much			 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
#pragma pack(4)
//
// These are used to disable features of GameOS
//
//
//#define USEPNG			// When present .PNG files can be loaded

//
// Error groups for SPEW
//
#define GROUP_FILES				"GameOS_Files"			// File functions
#define GROUP_MEMORY			"GameOS_Memory"			// Leak notification only
#define GROUP_MEMORYVERBOSE		"GameOS_MemoryVerbose"	// Heap create / destroy functions
#define GROUP_TEXTURE			"GameOS_Texture"		// Texture enumeration and creation
#define GROUP_GAMEOS			"GameOS_Core"
#define GROUP_NETWORK			"GameOS_Network"		// Networking messages (DPlay layer is lower level)
#define GROUP_DIRECTX			"GameOS_DirectX"
#define GROUP_DIRECTDRAW		"GameOS_DirectDraw"
#define GROUP_DIRECT3D			"GameOS_Direct3D"
#define GROUP_DIRECTINPUT		"GameOS_DirectInput"
#define GROUP_DIRECTSOUND		"GameOS_DirectSound"
#define GROUP_DIRECTSHOW		"GameOS_DirectShow"
#define GROUP_DIRECTPLAY		"GameOS_DirectPlay"
#define GROUP_WINDOWSMESSAGES	"Windows_Messages"		// WM_ messages from windows
//
// Defines to alter windows headers
//
#define STRICT						// Make windows type checking more accurate
#define WIN32_EXTRA_LEAN			// Reduce windows header size
#define VC_EXTRALEAN				// Reduce windows header size
#define NOGDIOBJ
#define NOSCROLL
#define NOMDI						// windows - if you ever use a windows function
#define NOLOGERROR					// of define that does not work or link comment
#define NOPROFILER					// out all of these, and put them back until you
#define NOMEMMGR					// find the one causing it to be removed.
#define NOLFILEIO
#define NOOPENFILE
#define NOATOM
#define NOLANGUAGE					// mfc\include\Afxv_w32.h
#define NOLSTRING
#define NODBCS
#define NOKEYBOARDINFO
#define NOGDICAPMASKS
#define NOCOLOR
#define NOSCALABLEFONT
#define NOBITMAP
#define NOMETAFILE
#define NOSYSTEMPARAMSINFO
#define NOKEYSTATES
#define NOWH
#define NOMENUS
#define NOICONS
//
// Disable stupid warnings (So warning 4 can be used)
//
#pragma warning( disable: 4018 )	// signed/unsigned mismatches
#pragma warning( disable: 4097 )	// typedef synonyms
#pragma warning( disable: 4100 )	// unreferenced formal parameter
#pragma warning( disable: 4102 )	// unreferenced labels
#pragma warning( disable: 4127 )	// conditional expression is constant eg: While(1)
#pragma warning( disable: 4201 )	// nameless struct/union
#pragma warning( disable: 4245 )	// conversion from 'const int' to 'unsigned long', signed/unsigned mismatch
#pragma warning( disable: 4355 )	// this used in base initializers
#pragma warning( disable: 4511 )	// no copy constructor
#pragma warning( disable: 4512 )	// no assignment operator
#pragma warning( disable: 4514 )	// unreferenced inline function has been removed
#pragma warning( disable: 4702 )	// Unreachable code
#pragma warning( disable: 4710 )	// inline not expanded
#pragma warning( disable: 4711 )	// auto-inline selection
//
// Common synonyms
//
typedef	unsigned char	U8;
typedef signed char		S8;
typedef unsigned short	U16;
typedef signed short	S16;
typedef unsigned int	U32;
typedef signed int		S32;
typedef float			real;
//
// C / C++ headers
//
#include<stdio.h>
#include<math.h>
#include<mbstring.h>
//
// Windows headers
//
#include<windows.h>
//
// We have modified imagehlp.h so that it does not require a lot more platformsdk files.
//
#define WINTRUST_H
#define ULONG_PTR DWORD
#include"imagehlp.h"
//
// Debugging library (Win2K only - from the platform SDK)
//
#include"psapi.h"
//
// Check that DX7 SDK headers and includes are included first in your Tools\Options\Directories in Visual C
//
#include<ddraw.h>
#if DIRECTDRAW_VERSION !=0x0700
#error DirectX 7 SDK is required
#endif
#include<dinput.h>
#include<dsetup.h>
#include<dsound.h>
#include<d3d.h>
#include<dplay.h>
#include<dplobby.h>
//
// DirectShow
//
#pragma warning( disable: 4201 )	// nameless struct/union (again - something reenables this!)
#include<amstream.h>
#include<control.h>
#include<process.h>
#include<mmreg.h>
#include<msacm.h>
#ifndef __IDDrawExclModeVideo_FWD_DEFINED__
#error DirectX Media 6.0 SDK is required
#endif
//
// So the GameOS can call it's own functions, GameOS.hpp is included
//
#include"gameos.hpp"
#include"toolos.hpp"
#include"platform.hpp"
#include"eax.h"
#include"language.h"
//
// Reads time stamp into an __int64, End finds difference
//
#define GetTimeStampStart(x) { _asm push eax  _asm push edx _asm rdtsc __asm mov dword ptr x,eax __asm mov dword ptr x+4,edx _asm pop edx _asm pop eax}
#define GetTimeStampEnd(x) {  _asm push eax _asm push edx _asm rdtsc __asm sub eax,dword ptr x __asm sbb edx,dword ptr x+4 __asm mov dword ptr x,eax __asm mov dword ptr x+4,edx _asm pop edx _asm pop eax}
//
// Faster float to long  (Always rounds to nearest)  ie: -2.7 goes to 3
//
inline long float2long( float fval )
{
	fval += 3 << 22;
	return ((*(long*)&fval)&0x007fffff) - 0x00400000;
}

#include<winsock.h>
