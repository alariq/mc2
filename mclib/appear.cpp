//---------------------------------------------------------------------------
//
// Appear.cpp -- File contains the Basic Game Appearance operator overrides
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files
#ifndef APPEAR_H
#include"appear.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef APPRTYPE_H
#include"apprtype.h"
#endif

#ifndef DBASEGUI_H
#include"dbasegui.h"
#endif

#ifndef CELINE_H
#include"celine.h"
#endif

#ifndef CEVFX_H
#include"cevfx.h"
#endif

//#ifndef FONT_H
//#include"font.h"
//#endif

extern bool useFog;

//---------------------------------------------------------------------------
// class Appearance
void * Appearance::operator new (size_t mySize)
{
	void* result = NULL;
	if (AppearanceTypeList::appearanceHeap && AppearanceTypeList::appearanceHeap->heapReady())
	{
		result = AppearanceTypeList::appearanceHeap->Malloc(mySize);
	}
		
	return(result);
}

//---------------------------------------------------------------------------
void Appearance::operator delete (void * us)
{
	long result;
	if (AppearanceTypeList::appearanceHeap && AppearanceTypeList::appearanceHeap->heapReady())
	{
		result = AppearanceTypeList::appearanceHeap->Free(us);
	}
}		

//---------------------------------------------------------------------------
void Appearance::drawTextHelp (char *text, unsigned long color)
{
	DWORD width, height;
	Stuff::Vector4D moveHere;
	moveHere = screenPos;


	gos_TextSetAttributes (gosFontHandle, 0, gosFontScale, false, true, false, false);
	gos_TextStringLength(&width,&height,text);

	moveHere.y = lowerRight.y + 10.0f;
	moveHere.x -= width / 2;
	moveHere.z = width;
	moveHere.w = height;

	globalFloatHelp->setFloatHelp(text,moveHere,color,SD_BLACK,1.0f,true,false,false,false);
}

void Appearance::drawTextHelp( char* text )
{
	drawTextHelp( text, SD_GREEN );
}
void Appearance::drawPilotName(char *text, unsigned long color )
{
	DWORD width, height;
	Stuff::Vector4D moveHere;
	moveHere = screenPos;


	gos_TextSetAttributes (gosFontHandle, 0, gosFontScale, false, true, false, false);
	gos_TextStringLength(&width,&height,text);

	moveHere.y = lowerRight.y + 10.0f + height;
	moveHere.x -= width / 2;
	moveHere.z = width;
	moveHere.w = height;

	globalFloatHelp->setFloatHelp(text,moveHere,color,SD_BLACK,1.0f,true,false,false,false);
}

//---------------------------------------------------------------------------
void Appearance::drawSelectBox (DWORD color)
{
	Stuff::Vector4D				ul, br, pos1, pos2;
	float						offsets;

	AppearanceType *appearType = getAppearanceType();
	if (appearType && appearType->typeBoundExists())
	{		   
		eye->projectZ(appearType->typeUpperLeft,ul);
		ul.z = HUD_DEPTH;
		
		eye->projectZ(appearType->typeLowerRight,br);
		br.z = HUD_DEPTH;
	}
	else
	{
		ul.x = upperLeft.x;
		ul.y = upperLeft.y;
		ul.z = HUD_DEPTH;

		br.x = lowerRight.x;
		br.y = lowerRight.y;
		br.z = HUD_DEPTH;
	}

	//-----------------------------------------------------
	// Must scale the magic numbers for select bracket
	offsets = 5.0f * eye->getScaleFactor();

	pos1.x = float(ul.x-offsets);
	pos1.y = float(ul.y-offsets);
	pos1.z = ul.z;

	pos2.x = float(ul.x-offsets);
	pos2.y = float(ul.y);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = float(ul.x-offsets);
	pos1.y = float(ul.y-offsets);
	pos1.z = ul.z;

	pos2.x = float(ul.x);
	pos2.y = float(ul.y-offsets);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = float(br.x+offsets);
	pos1.y = float(ul.y-offsets);
	pos1.z = ul.z;

	pos2.x = float(br.x+offsets);
	pos2.y = float(ul.y);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = float(br.x+offsets);
	pos1.y = float(ul.y-offsets);
	pos1.z = ul.z;

	pos2.x = float(br.x);
	pos2.y = float(ul.y-offsets);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = float(br.x+offsets);
	pos1.y = float(br.y+offsets);
	pos1.z = ul.z;

	pos2.x = float(br.x+offsets);
	pos2.y = float(br.y);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = float(br.x+offsets);
	pos1.y = float(br.y+offsets);
	pos1.z = ul.z;

	pos2.x = float(br.x);
	pos2.y = float(br.y+offsets);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = float(ul.x-offsets);
	pos1.y = float(br.y+offsets);
	pos1.z = ul.z;

	pos2.x = float(ul.x-offsets);
	pos2.y = float(br.y);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = float(ul.x-offsets);
	pos1.y = float(br.y+offsets);
	pos1.z = ul.z;

	pos2.x = float(ul.x);
	pos2.y = float(br.y+offsets);
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
}
	
//---------------------------------------------------------------------------
void Appearance::drawSelectBrackets (DWORD color)
{
	float					offsets = 5.0 * eye->getScaleFactor();
	Stuff::Vector4D			pos1;
	Stuff::Vector4D			pos2;
	Stuff::Vector4D			ul, br;

	ul.x = upperLeft.x;
	ul.y = upperLeft.y;
	ul.z = HUD_DEPTH;

	br.x = lowerRight.x;
	br.y = lowerRight.y;
	br.z = HUD_DEPTH;

	pos1.x = ul.x;
	pos1.y = ul.y;
	pos1.z = ul.z;

	pos2.x = ul.x+offsets;
	pos2.y = ul.y;
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}

	pos1.x = ul.x;
	pos1.y = ul.y;
	pos1.z = ul.z;

	pos2.x = ul.x;
	pos2.y = ul.y+offsets;
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = br.x;
	pos1.y = ul.y;
	pos1.z = ul.z;

	pos2.x = br.x;
	pos2.y = ul.y+offsets;
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = br.x;
	pos1.y = ul.y;
	pos1.z = ul.z;

	pos2.x = br.x-offsets;
	pos2.y = ul.y;
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}

	pos1.x = ul.x;
	pos1.y = br.y;
	pos1.z = br.z;

	pos2.x = ul.x;
	pos2.y = br.y-offsets;
	pos2.z = br.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	pos1.x = ul.x;
	pos1.y = br.y;
	pos1.z = br.z;

	pos2.x = ul.x+offsets;
	pos2.y = br.y;
	pos2.z = br.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
	
 	pos1.x = br.x;
	pos1.y = br.y;
	pos1.z = br.z;

	pos2.x = br.x;
	pos2.y = br.y-offsets;
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
	
 	pos1.x = br.x;
	pos1.y = br.y;
	pos1.z = br.z;

	pos2.x = br.x-offsets;
	pos2.y = br.y;
	pos2.z = ul.z;
	pos2.w = pos1.w = 1.0f;
	
	{
		LineElement newElement(pos1,pos2,color,NULL,-1);
		newElement.draw();
	}
}
	
//-----------------------------------------------------------------------------
void DrawBox(float l, float t, float r, float b)
{
	Stuff::Vector4D		p1, p2;

	p1.x = l;
	p1.y = t;
	p1.z = HUD_DEPTH;
	p1.w = 1.0;

	p2.x = r;
	p2.y = t;
	p2.z = HUD_DEPTH;
	p2.w = 1.0;

	{
		LineElement newElement(p1,p2,SD_BLACK,NULL,-1);
		newElement.draw();
	}

	p1.x = r;
	p1.y = t;

	p2.x = r;
	p2.y = b;

	{
		LineElement newElement(p1,p2,SD_BLACK,NULL,-1);
		newElement.draw();
	}

	p1.x = r;
	p1.y = b;

	p2.x = l;
	p2.y = b;

	{
		LineElement newElement(p1,p2,SD_BLACK,NULL,-1);
		newElement.draw();
	}

	p1.x = l;
	p1.y = t;

	p2.x = l;
	p2.y = b;

	{
		LineElement newElement(p1,p2,SD_BLACK,NULL,-1);
		newElement.draw();
	}
}

void Appearance::drawIcon( unsigned long bmpHandle, unsigned long bmpWidth,
						  unsigned long bmpHeight, unsigned long color, 
						  unsigned long where )
{
	// ignoring where for now
	float			offset = 8.0 * eye->getScaleFactor();
	float			trueHeight = HEIGHT * eye->getScaleFactor();
	float			Y = upperLeft.y - offset - trueHeight -  2 * bmpHeight;
	float			X = (upperLeft.x + lowerRight.x)/2.f - bmpWidth/2.f;

	gos_VERTEX v[4]; 

	for ( int i = 0; i < 4; i ++ )
	{
		v[i].argb = color;
		v[i].frgb = 0;
		v[i].z = 0.f;
		v[i].rhw = .5f;
		v[i].x = X;
		v[i].y = Y;
		v[i].u = .25; // might want to pass these in too....
		v[i].v = 0.f;
	}

	v[2].x = v[3].x = X + bmpWidth;
	v[1].y = v[2].y = Y + bmpHeight;
	v[2].u = v[3].u = 9.f/16.f;
	v[1].v = v[2].v = 5.f/16.f;

	gos_VERTEX v1[3];
	v1[0] = v[0];
	v1[1] = v[2];
	v1[2] = v[3];
	mcTextureManager->addVertices(bmpHandle,v,MC2_ISHUDLMNT);
	mcTextureManager->addVertices(bmpHandle,v1,MC2_ISHUDLMNT); 
}
	
//---------------------------------------------------------------------------
void Appearance::drawBars (void)
{
	//-----------------------------------------
	// Change to GOS DrawQuad code for HWare!!
	float			offset = 8.0 * eye->getScaleFactor();		//Remember, EVEN numbers!!!
	float			trueWidth = WIDTH * eye->getScaleFactor() * 2;
	float			trueHeight = HEIGHT * eye->getScaleFactor();
	float			topY = upperLeft.y - offset - trueHeight;
	float			leftX = floor((upperLeft.x + lowerRight.x)/2.f - trueWidth / 2);
	
	DWORD					color;
	
	if (barStatus > 1.0f)
		barStatus = 1.0f;

	if (!barColor)
	{
		if (barStatus >= 0.5)
			color = SB_GREEN;
		else if (barStatus > 0.2)
			color = SB_YELLOW;
		else if ( barStatus )
			color = SB_RED;
		else
			color = 0;
	}
	else
		color = barColor;
		
	float barLength = trueWidth * barStatus;

	gos_VERTEX vertices[4];
	
	vertices[0].x 		= leftX - 1.0;
	vertices[0].y 		= topY - 1.0;
	vertices[0].z		= HUD_DEPTH;
	vertices[0].rhw		= 0.5;
	vertices[0].u		= 0.0;
	vertices[0].v		= 0.0;
	vertices[0].argb	= color | 0xff000000;	  //Factor out the alpha color!!
	vertices[0].frgb	= 0x00000000; 
	
	vertices[1].x 		= leftX + barLength + 1.0;
	vertices[1].y 		= topY - 1.0;
	vertices[1].z		= HUD_DEPTH;
	vertices[1].rhw		= 0.5;
	vertices[1].u		= 0.0;
	vertices[1].v		= 0.0;
	vertices[1].argb	= color | 0xff000000;     //Factor out the alpha color!! 
	vertices[1].frgb	= 0x00000000; 
	
	vertices[2].x 		= leftX + barLength + 1.0;
	vertices[2].y 		= topY + trueHeight + 1.0;
	vertices[2].z		= HUD_DEPTH;
	vertices[2].rhw		= 0.5;
	vertices[2].u		= 0.0;
	vertices[2].v		= 0.0;
	vertices[2].argb	= color | 0xff000000;     //Factor out the alpha color!! 
	vertices[2].frgb	= 0x00000000; 
	
	vertices[3].x 		= leftX - 1.0;
	vertices[3].y 		= topY + trueHeight + 1.0;
	vertices[3].z		= HUD_DEPTH;
	vertices[3].rhw		= 0.5;
	vertices[3].u		= 0.0;
	vertices[3].v		= 0.0;
	vertices[3].argb	= color | 0xff000000;     //Factor out the alpha color!! 
	vertices[3].frgb	= 0x00000000; 
		
	PolygonQuadElement newElement;
	newElement.init(vertices);

	gos_SetRenderState( gos_State_Fog, 0);

	newElement.draw();
	
	DrawBox(vertices[0].x,vertices[0].y,(leftX + trueWidth + 1.0),vertices[2].y);

	DWORD fogColor = eye->fogColor;

	//-----------------------------------------------------
	// FOG time.  Set Render state to FOG on!
	if (useFog)
	{
		gos_SetRenderState( gos_State_Fog, fogColor);
	}
}

//---------------------------------------------------------------------------
