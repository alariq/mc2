//---------------------------------------------------------------------------
//
// vfxshape.h - This file contains the header for the old shape format
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef VFXSHAPE_H
#define VFXSHAPE_H
//---------------------------------------------------------------------------
// Include files

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DAPRTYPE_H
#include"daprtype.h"
#endif

//---------------------------------------------------------------------------------
class Shape
{
	public:

		MemoryPtr			frameList;		//This is binary image of VFX file
		MemoryPtr			stupidHeader;
		unsigned long		lastTurnUsed;
		Shape				*next;
		AppearanceTypePtr	owner;

	public:

		void init (void) 
		{
			frameList = stupidHeader = NULL;
			lastTurnUsed = 0;
			next = NULL;
			owner = NULL;
		}

		Shape (void) 
		{
			init();
		}

		long init (MemoryPtr fileBlock, AppearanceTypePtr, long shapeSize);

		void destroy (void);

		~Shape (void) 
		{
			destroy();
		}
};

typedef Shape *ShapePtr;
//---------------------------------------------------------------------------------
#endif
