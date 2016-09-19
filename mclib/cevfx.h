//---------------------------------------------------------------------------
//
// cevfx.h - This file contains the class declarations for the VFX Shape Element
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CEVFX_H
#define CEVFX_H
//---------------------------------------------------------------------------
// Include files

#ifndef CELEMENT_H
#include"celement.h"
#endif

#include<float.h>
#include<gameos.hpp>
//---------------------------------------------------------------------------
class VFXElement : public Element
{
	public:
	
		MemoryPtr		shapeTable;
		long			frameNum;
		long			x,y;
		bool			reverse;
		MemoryPtr		fadeTable;
		bool			noScaleDraw;
		bool			scaleUp;

	VFXElement (void)
	{
		shapeTable = NULL;
		frameNum = 0;
		x = y = 0;
		reverse = FALSE;
		
		fadeTable = NULL;
		
		noScaleDraw = FALSE;
		scaleUp = FALSE;
	}

	VFXElement (MemoryPtr _shape, long _x, long _y, long frame, bool rev, MemoryPtr fTable = NULL, bool noScale = FALSE, bool scaleUp = FALSE);
	VFXElement (MemoryPtr _shape, float _x, float _y, long frame, bool rev, MemoryPtr fTable = NULL, bool noScale = FALSE, bool scaleUp = FALSE);

	virtual void draw (void);
};

#define MAX_ELEMENT_SHAPES		4
//---------------------------------------------------------------------------
class VFXShapeElement : public Element
{
	public:
	
		//---------------------------------------
		// This new element class combines all of
		// the shapes into a single Texture for
		// rendering.  Can be used for vehicles
		// and mechs in MechCmdr 2.
		MemoryPtr		shapeTable[MAX_ELEMENT_SHAPES];
		long			frameNum[MAX_ELEMENT_SHAPES];
		bool			reverse[MAX_ELEMENT_SHAPES];
		long			x,y,xHS,yHS;
		unsigned long	*fadeTable;
		DWORD			textureMemoryHandle;
		long			actualHeight;
		float			textureFactor;
		DWORD			lightRGB;
		DWORD			fogRGB;
		float			z,topZ;

	VFXShapeElement (void)
	{
		shapeTable[0] = shapeTable[1] = shapeTable[2] = shapeTable[3] = NULL;
		frameNum[0] = frameNum[1] = frameNum[2] = frameNum[3] = 0;

		x = y = 0;

		reverse[0] = reverse[1] = reverse[2] = reverse[3] = FALSE;
		
		fadeTable = NULL;

		textureMemoryHandle = 0xffffffff;

		actualHeight = -1;
		
		lightRGB = 0xffffffff;		//Fully Lit
		fogRGB = 0xffffffff;		//NO Fog
	}

	void init (MemoryPtr _shape, long _x, long _y, long frame, bool rev, unsigned long *fTable = NULL, float _z = 0.0, float tZ = 0.0);

	long getTextureHandle (long height = -1);					//Return the block of memory so I store it for this mech/vehicle,etc.
	void setTextureHandle (DWORD handle, long height = -1);

	void setLight (DWORD light)
	{
		lightRGB = light;
	}

	void setFog (DWORD fog)
	{
		fogRGB = fog;
	}

	void drawShape (void);
		
	virtual void draw (void);
};

//---------------------------------------------------------------------------
class TextureElement : public Element
{
	public:
	
		//-----------------------------------------
		// This just draws a textured face.
		// Texture is passed in when inited.
		// For use with Cards.  Not real 3D faces!
		long			x, y, xHS, yHS;
		float			tWidth;
		float			z,topZ;
		DWORD			textureMemoryHandle;
		DWORD			lightRGB;
		DWORD			fogRGB;

	TextureElement (void)
	{
		x = y = 0;
		topZ = z = 0.0;
		textureMemoryHandle = 0xffffffff;
		lightRGB = 0xffffffff;
	}

	void setLight (DWORD light)
	{
		lightRGB = light;
	}

	void setFog (DWORD fog)
	{
		fogRGB = fog;
	}
		
	void init (DWORD textureHandle, long _x, long _y, long hsx, long hsy, float tWidth, float _z, float tZ);
	virtual void draw (void);
};

//---------------------------------------------------------------------------
class PolygonQuadElement : public Element
{
	public:
		
		//--------------------------------
		// This draws any untextured face.
		// Useful for status bars, etc.
		gos_VERTEX		vertices[4];
		
	PolygonQuadElement (void)
	{
	}
	
	void init (gos_VERTEX *v);
	
	virtual void draw (void);

};

//---------------------------------------------------------------------------
class PolygonTriElement : public Element
{
	public:
		
		//--------------------------------
		// This draws any untextured face.
		// Useful for status bars, etc.
		gos_VERTEX		vertices[3];
		
	PolygonTriElement (void)
	{
	}
	
	void init (gos_VERTEX *v);
	
	virtual void draw (void);

};

//---------------------------------------------------------------------------
class TexturedPolygonQuadElement : public PolygonQuadElement
{
	public:
		
		//--------------------------------
		// This draws any untextured face.
		// Useful everywhere
		DWORD		textureHandle;
		bool		zWrite;
		bool		zComp;
		
	TexturedPolygonQuadElement (void)
	{
	}
	
	void init (gos_VERTEX *v, DWORD tHandle, bool writeZ = true, bool compZ = true);
	
	virtual void draw (void);

};

//---------------------------------------------------------------------------
class TexturedPolygonTriElement : public PolygonTriElement
{
	public:
		
		//--------------------------------
		// This draws any textured face.
		// Useful everywhere.
		DWORD		textureHandle;
		
	TexturedPolygonTriElement (void)
	{
	}
	
	void init (gos_VERTEX *v, DWORD tHandle);
	
	virtual void draw (void);

};

//---------------------------------------------------------------------------
#endif
