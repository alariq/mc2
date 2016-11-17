//---------------------------------------------------------------------------
//
// cellip.cpp - This file contains the code for the VFX Ellipse Element
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files

#ifndef CELLIP_H
#include"cellip.h"
#endif

#ifndef VPORT_H
#include"vport.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

unsigned long EllipseElement::s_textureHandle = 0;
//---------------------------------------------------------------------------
// Static Globals

//---------------------------------------------------------------------------
EllipseElement::EllipseElement (Stuff::Vector2DOf<long> &cntr, Stuff::Vector2DOf<long> &ortho, long clr, long depth) : Element(depth)
{
	for ( int i = 0; i < 5; ++i )
	{
		location[i].argb = clr;
		location[i].frgb = 0;

		location[i].u = 0.f;
		location[i].v = 0.f;
		location[i].rhw = .5;
		location[i].z  =0.f;
	}

	location[0].x = location[1].x = cntr.x - ortho.x/2;
	location[2].x = location[3].x = cntr.x + ortho.x/2;
	location[0].y = location[3].y = cntr.y - ortho.y/2;
	location[1].y = location[2].y = cntr.y + ortho.y/2;

	location[3].u = location[2].u = 1.0f;
	location[1].v = location[2].v = 1.0f;

	location[4] = location[0];

}
	
//---------------------------------------------------------------------------
void EllipseElement::draw (void)
{
	gos_SetRenderState( gos_State_Filter, gos_FilterNone );
	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );

	gos_SetRenderState( gos_State_AlphaTest, true );
			
	DWORD gosTextureHandle = mcTextureManager->get_gosTextureHandle (s_textureHandle);
	gos_SetRenderState( gos_State_Texture, gosTextureHandle );
	gos_SetRenderState( gos_State_Clipping, 2);
	gos_SetRenderState( gos_State_Specular, 0);
	gos_SetRenderState( gos_State_Fog, 0);

	
	if ( clip.left != 0 || clip.right != 0 || clip.top != 0 || clip.bottom != 0 )
	{
		gos_VERTEX newLocation[4];
		for ( int i = 0; i < 4; ++i )
		{
			newLocation[i] = location[i];
		}
		if ( location[0].x > clip.right )
			return;
		if ( location[2].x < clip.left )
			return;
		if ( location[0].x < clip.left )
		{
			newLocation[0].u = newLocation[1].u = ((float)clip.left-location[0].x)/(location[2].x - location[0].x);
			newLocation[0].x = newLocation[1].x = (float)clip.left;
		}
		if ( location[2].x > clip.right )
		{
			newLocation[2].u = newLocation[3].u = ((float)clip.right-location[0].x)/(location[2].x-location[0].x);
			newLocation[2].x = newLocation[3].x = (float)clip.right;
		}

		if ( location[2].y < clip.top )
			return;

		if ( location[0].y > clip.bottom )
			return;

		if ( location[0].y < clip.top )
		{
			newLocation[0].v = newLocation[3].v = ((float)clip.top - location[0].y )/(location[1].y - location[0].y);
			newLocation[0].y = newLocation[3].y = (float)clip.top;
		}

		if ( location[2].y > clip.bottom )
		{
			newLocation[1].v = newLocation[2].v = ((float)clip.bottom - location[0].y)/(location[2].y-location[0].y);
			newLocation[1].y = newLocation[2].y = (float)clip.bottom;
		}

		gos_DrawQuads( newLocation, 4 );
	}
	else
	{


		gos_DrawTriangles( location, 3 );
		gos_DrawTriangles( &location[2], 3 );
	}

	
}

void EllipseElement::init()
{
	if ( !s_textureHandle )
	{
		s_textureHandle = mcTextureManager->loadTexture("data/art/ring.tga", gos_Texture_Alpha, 0 );
	}
}

void EllipseElement::setClip( const GUI_RECT& rect )
{
	clip = rect;
}

void EllipseElement::removeTextureHandle (void)
{
	if (s_textureHandle)
		mcTextureManager->removeTextureNode(s_textureHandle);

	s_textureHandle = 0;
}

//---------------------------------------------------------------------------
