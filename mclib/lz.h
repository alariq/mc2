//---------------------------------------------------------------------------
//
// LZ Compress/Decompress Headers
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef LZ_H
#define LZ_H

//---------------------------------------------------------------------------
typedef unsigned char* MemoryPtr;
long LZDecomp (MemoryPtr dest, MemoryPtr src, unsigned long srcLen);
long LZCompress (MemoryPtr dest,  MemoryPtr src, unsigned long len);

//---------------------------------------------------------------------------
#endif
