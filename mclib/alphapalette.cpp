//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include<stdio.h>

#ifndef FILE_H
#include"file.h"
#endif

#include"vfx.h"

#include<string.h>
#include<gameos.hpp>

unsigned char FindClosest( VFX_RGB* Palette, int r, int g, int b );


//
//
// Number of colors listed in the AlphaPal.ini file (>255 are special colors)
//
//
#define NUM_ALPHACOLORS 280			// (One+last color listed)

//
// Table used for alpha effects
//  Low byte  = Background
//  High byte = Sprite
//
char AlphaTable[NUM_ALPHACOLORS*256];
//
// Flag set to 1 when an alpha pixel is used
//
char SpecialColor[NUM_ALPHACOLORS];

enum { R, G, B, SourceAlpha, DestAlpha };

char *g_logistic_fadetable = &AlphaTable[256*256];
char *g_logistic_bluefade = &AlphaTable[267*256];
char *g_logistic_dlgfade = &AlphaTable[268*256];

void InitAlphaLookup( VFX_RGB* Palette)
{
	char* pAlphaTable = AlphaTable;
	int r,g,b,i;
	File *IniFile;
	char Line[256];
	float AlphaIni[NUM_ALPHACOLORS][5];
	int LineNumber;
//
// Clear the AlphaPal.ini array
//
	memset(AlphaIni,0,sizeof(AlphaIni));
	memset(SpecialColor,0,sizeof(SpecialColor));
//
// Read in and parse the AlphaPal.ini file containing all the color information
//
	IniFile = new File;
#ifdef _DEBUG
	long AlphapalOpenResult = 
#endif
		IniFile->open("AlphaPal.ini");
	gosASSERT(AlphapalOpenResult == NO_ERR);

	LineNumber=0;

	while (!IniFile->eof())
	{
		IniFile->readLine((MemoryPtr)&Line[0], 256);
		LineNumber++;
	
		if (Line[0]!='#' && Line[0]!=';' && Line[0]!=0xa && Line[0]!=0)
		{
			if( EOF == sscanf( Line, "%d", &i ))
				break;

			gosASSERT(i>=0 && i<NUM_ALPHACOLORS);

			SpecialColor[i]=1;

			gosASSERT(EOF != sscanf( Line, "%f %f %f %f %f",
													&AlphaIni[i][R],
													&AlphaIni[i][G],
													&AlphaIni[i][B],
													&AlphaIni[i][SourceAlpha],
													&AlphaIni[i][DestAlpha] ));

			gosASSERT(( AlphaIni[i][SourceAlpha]!=0.0 || AlphaIni[i][DestAlpha]!=0.0 ) &&
				( AlphaIni[i][R]!=255 && AlphaIni[i][G]!=255 && AlphaIni[i][B]!=255 ) );
		}
	}

	IniFile->close();
	delete IniFile;
	IniFile = NULL;
//
// Now generate the 64K alpha value lookup table
//
	for( int source=0; source<NUM_ALPHACOLORS; source++ )
	{
		for( int dest=0; dest<256; dest++ )
		{
			if( source==255 || source==0 )		// Color 255 and 0 - dest color remains the same (totally transparent)
			{
				*pAlphaTable++= dest;
			}
			else if( SpecialColor[source]==0 )	// If not specified, make a solid color
			{
				*pAlphaTable++= source;
			}
			else if( dest<10 || dest>245 )
			{
				*pAlphaTable++= char(-1);			// Is dest is ever a bad pixel, make white
			}
			else
			{
				if( AlphaIni[source][SourceAlpha]==0.0 && AlphaIni[source][DestAlpha]==0.0 )
				{
					r = (int) (((float)(Palette[dest].r<<2) * 255 ) / ( 255 - AlphaIni[source][R] ) );
					g = (int) (((float)(Palette[dest].g<<2) * 255 ) / ( 255 - AlphaIni[source][G] ) );
					b = (int) (((float)(Palette[dest].b<<2) * 255 ) / ( 255 - AlphaIni[source][B] ) );
				}
				else
				{
					r = (int)(AlphaIni[source][SourceAlpha] * AlphaIni[source][R] + AlphaIni[source][DestAlpha] * (float)(Palette[dest].r<<2));
					g = (int)(AlphaIni[source][SourceAlpha] * AlphaIni[source][G] + AlphaIni[source][DestAlpha] * (float)(Palette[dest].g<<2));
					b = (int)(AlphaIni[source][SourceAlpha] * AlphaIni[source][B] + AlphaIni[source][DestAlpha] * (float)(Palette[dest].b<<2));
				}

				if( r<0 )
					r=0;
				else if( r>255 )
					r=255;

				if( g<0 )
					g=0;
				else if( g>255 )
					g=255;

				if( b<0 )
					b=0;
				else if( b>255 )
					b=255;

				*pAlphaTable++ = FindClosest( Palette,r>>2,g>>2,b>>2 );
			}
		}
	}
}


//
//
// 51% Green
// 39% Red
// 10% Blue
//
// Returns the closest matching color in a palette, dose not check windows colors
//
unsigned char FindClosest( VFX_RGB* Palette, int r, int g, int b )
{
	unsigned char Closest = 10;
	int Distance = 255*255*255;
	int tempR,tempG,tempB,tdist;

	for( int t1=10; t1<246; t1++ )
	{
		tempR = r-Palette[t1].r;
		tempG = g-Palette[t1].g;
		tempB = b-Palette[t1].b;

		tdist = (39*tempR*tempR) + (51*tempG*tempG) + (10*tempB*tempB);

		if( Distance > tdist )
		{
			Distance = tdist;
			Closest = t1;
			if( tdist==0 )
				break;
		}
	}
	return Closest;
}


