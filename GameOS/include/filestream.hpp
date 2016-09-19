//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// FileStream.h: interface for the FileStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FILESTREAM_HPP)
#define FILESTREAM_HPP

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/*
#pragma warning(push,3)
#include<fstream.h>
#pragma warning(pop)

//#ifdef _ARMOR
//#if 0
//#define DebugFileStream ofstream
//
//#else

class DebugFileStream
{
public:
	DebugFileStream(char *)
	{}

	DebugFileStream()
	{}

	inline  DebugFileStream& operator<<(ostream& (__cdecl * _f)(ostream&)){return *this;}
	inline  DebugFileStream& operator<<(ios& (__cdecl * _f)(ios&)){return *this;}
	inline  DebugFileStream& operator<<(const char *){return *this;}
	inline  DebugFileStream& operator<<(const unsigned char *){return *this;}
	inline  DebugFileStream& operator<<(const signed char *){return *this;}
	inline  DebugFileStream& operator<<(char){return *this;}
	inline  DebugFileStream& operator<<(unsigned char){return *this;}
	inline  DebugFileStream& operator<<(signed char){return *this;}
	inline  DebugFileStream& operator<<(short){return *this;}
	inline  DebugFileStream& operator<<(unsigned short){return *this;}
	inline  DebugFileStream& operator<<(int){return *this;}
	inline  DebugFileStream& operator<<(unsigned int){return *this;}
	inline  DebugFileStream& operator<<(long){return *this;}
	inline  DebugFileStream& operator<<(unsigned long){return *this;}
	inline  DebugFileStream& operator<<(float){return *this;}
	inline	DebugFileStream& operator<<(double){return *this;}
	inline  DebugFileStream& operator<<(long double){return *this;}
	inline  DebugFileStream& operator<<(const void *){return *this;}
	inline  DebugFileStream& operator<<(streambuf*){return *this;}

};

//#endif

*/

#endif

