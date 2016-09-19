#pragma once
//==========================================================================//
// File:	 Font3D.hpp														//
// Contents: Font3D drawing routines										//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//
// Support for .d3f fonts files
//
#pragma warning( push )
#pragma warning( disable : 4200 ) 
//
// Original FontEdit program
//
typedef struct {
	DWORD dwSig;				// 0x46443344
	DWORD dwWidth;
	DWORD dwFontHeight;
	DWORD dwHeight;
	DWORD dwX[256];
	DWORD dwY[256];
	DWORD dwWidths[256];
	int nA[256];
	int nC[256];
	BYTE bPixels[0];
} D3DFontData;

#pragma pack(push,1)

//
// FontEdit 4.0+
//
typedef struct 
{
	DWORD dwSig;				// 0x46443344
	char szFaceName[64];
	int iSize;					// point size of font used to create this font
	bool bItalic;				// italic on or off when this font was created
	int iWeight;				// weight of font created
	int iTextureCount; 
	DWORD dwFontHeight;			// height in pixels of the resulatant font
	BYTE bTexture[256];			// indicate which texture each character is on
	BYTE bX[256];				// indicates x position of each character on it's texture
	BYTE bY[256];				// indicates y position of each character on it's texture
	BYTE bW[256];				// indicates width of each character in the texture
	signed char cA[256];		// indicates amount of space to add before each character (may be -)
	signed char cC[256];		// indicates amount of space to add after each character (may be -)
} D3DFontData1;

typedef struct
{
	DWORD dwSize;
	BYTE bPixels[0];
} D3DFontTexture;

#pragma pack( pop )
#pragma warning( pop )




typedef struct _FontInfo
{
	DWORD		MagicNumber;			// Valid font check
	_FontInfo*	pNext;					// Pointer to next font
	DWORD		ReferenceCount;			// Reference count
	char		FontFile[MAX_PATH];		// Path name of font texture
	DWORD		StartLine;				// texture line where font starts
	int			CharCount;				// number of chars in font (valid range 33 to 256)
	DWORD		TextureSize;			// Width and Height of texture
	float		rhSize;					// 1.0 / Size
	DWORD		TexturePitch;			// Pitch of texture
	int			Width;					// Width of font grid
	int			Height;					// Height of font grid
	DWORD		Across;					// Number of characters across one line
	DWORD		Aliased;				// True if 4444 texture (may be aliased - else 1555 keyed)
	DWORD		FromTextureHandle;		// True is from a texture handle
	BYTE		BlankPixels[256-32];	// Empty pixels before character
	BYTE		UsedPixels[256-32];		// Width of character
	BYTE		TopU[256-32];
	BYTE		TopV[256-32];			// Position of character
	BYTE		TopOffset[256-32];		// Offset from top (number of blank lines)
	BYTE		RealHeight[256-32];		// Height of character
	BYTE		TextureHandle[256-32];	// Which texture handle to use
	DWORD		NumberOfTextures;		// Number of texture handles used (normally 1)
	HFONT		hFontTTF;				// handle to a GDI font
	DWORD		Texture[8];				// Texture handle array
} FontInfo;

void Init3DFont();
void Destroy3DFont();









