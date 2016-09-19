//---------------------------------------------------------------------------
// 
// MechCommander 2 -- Copyright (c) 1998 FASA Interactive
//
// TGA file Specifics
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TGAINFO_H
#define TGAINFO_H

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

//---------------------------------------------------------------------------
enum TGAImageType
{
	NO_IMAGE = 0,
	UNC_PAL,
	UNC_TRUE,
	UNC_GRAY,
	RLE_PAL = 9,
	RLE_TRUE,
	RLE_GRAY
};

//---------------------------------------------------------------------------
#pragma pack(1)

struct TGAFileHeader
{
	byte image_id_len;
	byte color_map;
	byte image_type;

	short cm_first_entry;
	short cm_length;
	byte cm_entry_size;

	short x_origin;
	short y_origin;
	short width;
	short height;
	byte pixel_depth;
	byte image_descriptor;
};

#pragma pack()

void tgaDecomp (MemoryPtr dest, MemoryPtr source, TGAFileHeader *tga_header);

void loadTGATexture (FilePtr tgaFile, MemoryPtr ourRAM, long width, long height);
void loadTGAMask (FilePtr tgaFile, MemoryPtr ourRAM, long width, long height);

void flipTopToBottom (MemoryPtr buffer, BYTE depth, long width, long height);
//---------------------------------------------------------------------------
#endif
