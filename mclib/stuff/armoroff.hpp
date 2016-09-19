//===========================================================================//
// File:    debugoff.hpp                                                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#undef Verify
#undef Warn
#undef Check_Signature
#undef Check_Pointer
#undef Mem_Copy
#undef Str_Copy
#undef Str_Cat
#undef Check
#undef Cast_Pointer
#undef Cast_Object
#undef Spew

#undef USE_TIME_ANALYSIS			// trace time statistics
#undef USE_TRACE_LOG					// logging functions
#undef USE_EVENT_STATISTICS      // event statistics

#if defined(LAB_ONLY) && !defined(NO_ANALYSIS)
	#define USE_TIME_ANALYSIS
	#define USE_TRACE_LOG
	#define USE_EVENT_STATISTICS
#endif

#define Verify(c) ((void)0)
#define Warn(c) ((void)0)
#define Check_Pointer(p) ((void)0)
#define Mem_Copy(destination, source, length, available)\
	memcpy(destination, source, length)
#define Str_Copy(destination, source, available)\
	strcpy(destination, source)
#define Str_Cat(destination, source, available)\
	strcat(destination, source)

#define Check_Object(p) ((void)0)
#define Check_Signature(p) ((void)0)
#define Cast_Pointer(type, ptr) reinterpret_cast<type>(ptr)
#define Cast_Object(type, ptr) static_cast<type>(ptr)
#define Spew(x,y) ((void)0)