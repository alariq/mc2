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
size_t LZDecomp (MemoryPtr dest, MemoryPtr src, size_t srcLen);
size_t LZCompress (MemoryPtr dest,  MemoryPtr src, size_t len);

//---------------------------------------------------------------------------
#endif
