//---------------------------------------------------------------------------
//
// cepoly.h - This file contains the class declarations for the polygon element
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CEPOLY_H
#define CEPOLY_H
//---------------------------------------------------------------------------
// Include files

#ifndef CELEMENT_H
#include"celement.h"
#endif

#ifndef VFX_H
#include"vfx.h"
#endif

//---------------------------------------------------------------------------
struct PolyElementData
{
	long		numVertices;			//Actually num_vertices + extras for clipping
	SCRNVERTEX	vertices[6]; 			//Draw Vertices

	bool		correctTexture;			//Should we Correctly Texture?
	bool		isClipped;				//Set by zclip to let me know
	bool		drawTranslucent;		//Used for FX
	bool		StatusBar;				//Is this a status bar?
	long		BarWidth;				//Width of status bar
	int			BarColor;				//Color of status bar

	MemoryPtr	textureMap;				//Pointer to Bitmap for Texture
	long	    width;					//Width of texture
	long		height;					//Height of texture
	char		*hazePalette;			//Translucency/haze table


	void init (void)
	{
		numVertices = 0;

		correctTexture = FALSE;
		isClipped = FALSE;

		StatusBar=FALSE;
		textureMap = 0;
		width = 0;
		height = 0;
		hazePalette = NULL;
		drawTranslucent = FALSE;
	}
		
	PolyElementData (void)
	{
		init();
	}
};

typedef PolyElementData *PolyElementDataPtr;

//---------------------------------------------------------------------------
class PolygonElement : public Element
{
	public:

		PolyElementData		polyData;
			
	PolygonElement (void)
	{
		polyData.init();
	}

	PolygonElement (PolyElementDataPtr pData, long depth);

	virtual void draw (void);
};

//---------------------------------------------------------------------------
#endif
