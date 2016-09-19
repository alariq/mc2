//---------------------------------------------------------------------------
//
// cepane.h - This file contains the class declarations for the VFX Pane Element
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CEPANE_H
#define CEPANE_H
//---------------------------------------------------------------------------
// Include files

#ifndef CELEMENT_H
#include"celement.h"
#endif

#ifndef VFX_H
#include"vfx.h"
#endif

//---------------------------------------------------------------------------
class PaneElement : public Element
{
	public:
	
		PANE *	shapePane;
		long	x;
		long	y;
		long	midx,midy,SizeX,SizeY;


	PaneElement (void)
	{
		shapePane = NULL;
		x = y = 0;
	}

	PaneElement (PANE *_shapePane, long _x, long _y, long _midx, long _midy, long _SizeX, long _SizeY);

	virtual void draw (void);
};

//---------------------------------------------------------------------------
class DeltaElement : public Element
{
	public:
	
		MemoryPtr		shapeTable;
		long			frameNum;
		long			x,y;
		bool			reverse;
		MemoryPtr		fadeTable;
		bool			noScaleDraw;
		bool			scaleUp;

	DeltaElement (void)
	{
		shapeTable = NULL;
		frameNum = 0;
		x = y = 0;
		reverse = FALSE;
		
		fadeTable = NULL;
		
		noScaleDraw = FALSE;
		scaleUp = FALSE;
	}

	DeltaElement (MemoryPtr _shape, long _x, long _y, long frame, bool rev, MemoryPtr fTable = NULL, bool noScale = FALSE, bool scaleUp = FALSE);

	virtual void draw (void);
};

//---------------------------------------------------------------------------
#endif
