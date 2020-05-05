//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

// stuff I need all over the place
#ifndef UTILITIES_H
#define UTILITIES_H

#include<gameos.hpp>
#include<string.h>

//#include<stdio.h>

class FitIniFile;


class StaticInfo
{
public:

	void init( FitIniFile& file, const char* blockName, long hiResOffsetX = 0, long hiResOffsetY = 0, DWORD neverFlush = 0 );
	void render();
	bool isInside( int mouseX, int mouseY );

	void setLocation( float  newX, float newY );
	void move( float deltaX, float deltaY );

	void setNewUVs( float uLeft, float vTop, float uRight, float vBottom );

	float width(){ return location[2].x - location[0].x; }
	float height(){ return location[2].y - location[0].y; }
	
	void getData(unsigned char * buffer);

	void showGUIWindow( bool bShow );

	void setColor( long newColor );

	StaticInfo(){}
	~StaticInfo();

	unsigned long textureHandle;
	gos_VERTEX	location[4];
	long u, v, uWidth, vHeight;

	unsigned long textureWidth; // textures are square
};



typedef struct _GUI_RECTd
{
	long left;
	long top;
	long right;
	long bottom;
} GUI_RECT;


void drawEmptyRect( const GUI_RECT& rect, unsigned long leftBorderColor = 0xffffffff,
	 unsigned long rightBorderColor = 0xff000000 );


void drawRect( const GUI_RECT& rect, DWORD color );

void drawShadowText( long colorTop, long colorShadow, HGOSFONT3D font, 
					long left, long top, bool proportional, const char* text, bool bBold, float scale );

void drawShadowText( long colorTop, long colorShadow, HGOSFONT3D font, 
					long left, long top, bool proportional, const char* text, bool bold, float scale,
					long xOffset, long yOffset);

void drawShadowText( long colorTop, long colorShadow, HGOSFONT3D font, 
					long left, long top, long right, long bottom, bool proportional, const char* text, bool bold, float scale,
					long xOffset, long yOffset);



long interpolateColor( long color1, long color2, float percent );

inline long reverseRGB( long oldVal ) 
{
	return ( (oldVal & 0xff000000) | ((oldVal & 0xff0000) >> 16) | (oldVal & 0xff00) | ((oldVal & 0xff) << 16) );
}


//-------------------------------------------------------
// Replaces the WINDOWS version because it is a lie!
// There are cases where windows does NOT append a null.
// THANKS!
//
// Replace with GOS String Resource get when available
// Replaced.  Andy wants us to call everytime.  Will try and see if practical.
extern HSTRRES gosResourceHandle;




#if 1
inline int cLoadString(
  unsigned int uID,             // resource identifier
  char* lpBuffer,      // pointer to buffer for resource
  int nBufferMax,        // size of buffer
  HSTRRES handle = gosResourceHandle
  )
{
	memset(lpBuffer,0,nBufferMax);
	const char * tmpBuffer = gos_GetResourceString(handle, uID);
	int stringLength = (int)strlen(tmpBuffer);
	if (stringLength >= nBufferMax)
		STOP(("String too long for buffer.  String Id %u, bufferLen %d, StringLen %l",uID,nBufferMax,stringLength));
	memcpy(lpBuffer,tmpBuffer,stringLength);
	return stringLength;
}

#else

inline char * cLoadString (unsigned int uID)
{
	return gos_GetResourceString(gosResourceHandle, uID);
}
#endif

#endif
