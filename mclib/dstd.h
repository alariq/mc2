//---------------------------------------------------------------------------
//
// dstd.h - This file contains the standard definitions
//
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DSTD_H
#define DSTD_H
//---------------------------------------------------------------------------
// Include files

//---------------------------------------------------------------------------
// Type Definitions
typedef unsigned char* MemoryPtr;
typedef unsigned char byte;
//sebi
//typedef unsigned size_t;

#ifndef NULL
#define NULL	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define RADS_TO_DEGREES	57.2957795132f		// Used to convert results in Rad to Deg
#define DEGREES_TO_RADS 0.0174532925199f 	// Used to convert results in Deg to Rad
#define COS_SIN_45		0.70710678f		 	// Used to convert results in Deg to Rad
#define PI				3.1415926535897932384626433832795

// !NB sebi
#ifndef PLATFORM_WINDOWS
typedef unsigned int DWORD;
typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef void *PVOID;
#else
typedef unsigned long DWORD; // sizeof(long) is 32bit on both 32bit and 64 bit Visual C++ compilers 
typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef void *PVOID;
#endif

//--------------------------------------------------------------------------
// Macro Definitions
#define NONE -1

//--------------------------------------------------------------------------
// Pragmas
#pragma warning(disable:4244)
#pragma warning(disable:4514)		// Unused Inline Functions
#pragma warning(disable:4800)
#pragma warning(disable:4244)

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//
//---------------------------------------------------------------------------
