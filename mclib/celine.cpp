//---------------------------------------------------------------------------
//
// celine.cpp - This file contains the code for the VFX Line Element
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files

#ifndef CELINE_H
#include"celine.h"
#endif

#ifndef VPORT_H
#include"vport.h"
#endif

#ifndef TGL_H
#include"tgl.h"
#endif

#ifndef CLIP_H
#include"clip.h"
#endif

#include<gameos.hpp>

//---------------------------------------------------------------------------
// Static Globals

extern bool hasGuardBand;
//---------------------------------------------------------------------------
LineElement::LineElement (Stuff::Vector4D &pos1, Stuff::Vector4D &pos2, long clr, MemoryPtr fTable, long endClr) : Element(0L)
{
	startPos = pos1;;
	endPos = pos2;
	color = clr;
	fadeTable = fTable;
	endColor = endClr;
}
	
//---------------------------------------------------------------------------
void LineElement::draw (void)
{
	if (endColor == -1)
	{
		gos_VERTEX gVertex[2];
		
		gVertex[0].x 	= startPos.x;
		gVertex[0].y 	= startPos.y;
		gVertex[0].z 	= startPos.z;
		if (startPos.w > Stuff::SMALL)
			gVertex[0].rhw 	= 1.0f / startPos.w;
		else
			gVertex[0].rhw  = 0.00001f;

		gVertex[0].argb	= color;
		gVertex[0].frgb	= 0xff000000; 
		gVertex[0].u	= 0.0;
		gVertex[0].v	= 0.0;
		
		gVertex[1].x 	= endPos.x;
		gVertex[1].y 	= endPos.y;
		gVertex[1].z 	= endPos.z;
		if (endPos.w > Stuff::SMALL)
			gVertex[1].rhw 	= 1.0f / endPos.w;
		else
			gVertex[1].rhw  = 0.00001f;

		gVertex[1].argb	= color;
		gVertex[1].frgb	= 0xff000000;
		gVertex[1].u	= 0.0;
		gVertex[1].v	= 0.0;
		
		//--------------------------------
		//Set States for Software Renderer
		if (Environment.Renderer == 3)
		{
			gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
			gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
			gos_SetRenderState( gos_State_MonoEnable, 1);
			gos_SetRenderState( gos_State_Perspective, 0);
			gos_SetRenderState( gos_State_Clipping, 0);
			gos_SetRenderState( gos_State_AlphaTest, 0);
			gos_SetRenderState( gos_State_Specular, 0);
			gos_SetRenderState( gos_State_Dither, 0);
			gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulate);
			gos_SetRenderState( gos_State_Filter, gos_FilterNone);
//			gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
			gos_SetRenderState( gos_State_Texture, 0);
		}
		//--------------------------------
		//Set States for Hardware Renderer	
		else
		{
			gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
			gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
			gos_SetRenderState( gos_State_MonoEnable, 0);
			gos_SetRenderState( gos_State_Perspective, 1);
			gos_SetRenderState( gos_State_Clipping, 1);
			gos_SetRenderState( gos_State_AlphaTest, 1);
			gos_SetRenderState( gos_State_Specular, 0);
			gos_SetRenderState( gos_State_Dither, 1);
			gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulate);
			gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear);
//			gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
			gos_SetRenderState( gos_State_Texture, 0);
			if (!fadeTable)
			{
				gos_SetRenderState( gos_State_ZCompare, 1);
				gos_SetRenderState(	gos_State_ZWrite, 1);
			}
			else
			{
				gos_SetRenderState( gos_State_ZCompare, 0);
				gos_SetRenderState(	gos_State_ZWrite, 0);
			}
		}

		//--------------------------------------------------------------------------------
		// Reject Any triangle which has vertices off screeen in software for now.
		// Do real cliping in geometry layer for software and hardware that needs it!
		if ((gVertex[0].z >= 0.0f) &&
			(gVertex[0].z < 1.0f) &&
			(gVertex[1].z >= 0.0f) &&  
			(gVertex[1].z < 1.0f))
			{
				gos_DrawLines(gVertex, 2);
			}	
	}
	else
	{
		//---------------------------------
		// Draw from start to end color.
	}
}

//---------------------------------------------------------------------------
