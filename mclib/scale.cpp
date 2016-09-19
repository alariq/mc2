//---------------------------------------------------------------------------//
// scale.cpp - This file contains definitions of the scaleFactor for scaled draws
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files
#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#ifndef VPORT_H
#include"vport.h"
#endif

#ifndef VFX_H
#include"vfx.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#include<math.h>
#include<gameos.hpp>

extern void AG_shape_translate_transform(PANE *globalPane, void *shapeTable,LONG frameNum, LONG hotX, LONG hotY,void *tempBuffer,LONG reverse, LONG scaleUp);
extern void AG_shape_transform(PANE *globalPane, void *shapeTable,LONG frameNum, LONG hotX, LONG hotY, void *tempBuffer,LONG reverse, LONG scaleUp);
extern void AG_shape_lookaside( UBYTE *table );
extern void AG_shape_draw (PANE *pane, void *shape_table,LONG shape_number, LONG hotX, LONG hotY);
extern void AG_shape_translate_draw (PANE *pane, void *shape_table,LONG shape_number, LONG hotX, LONG hotY);

//---------------------------------------------------------------------------
#define MAX_X		360
#define MAX_Y		360

MemoryPtr tempBuffer = NULL;
//---------------------------------------------------------------------------
//
long scaleDraw (MemoryPtr shapeTable, unsigned long frameNum, long sx, long sy, bool reverse, MemoryPtr fadeTable, bool scaleUp)
{
	//--------------------------------------------------------------
	// Check GlobalPane and GlobalWindow for validity.
	if( Camera::globalScaleFactor==1.0 )
		scaleUp=1;
	else
		scaleUp=0;

	//----------------------------------------------------------------
	// Check if shape is actually valid.
	if ((*(int*)shapeTable!=*(int*)"1.10"))
		return(TRUE);
		
	long result = VFX_shape_count(shapeTable);
	if (result <= (long)frameNum)
	{
		frameNum = result-1;
	}
	
	result = VFX_shape_bounds(shapeTable,frameNum);
	long xMax = result>>16;
	long yMax = result & 0x0000ffff;

	if ((xMax == 0) || (yMax == 0))
		return -1;

	if (abs(xMax - yMax) > 256)
	{
		return -1;
	}

	if ((xMax > 400) || (yMax > 400))
	{
		return -1;
	}

	if (!tempBuffer)
		tempBuffer = (MemoryPtr)systemHeap->Malloc(MAX_X * MAX_Y);
	
	gosASSERT((yMax * xMax) < (MAX_Y * MAX_X));
	
	if (fadeTable)
	{
		AG_shape_lookaside(fadeTable);
		if(scaleUp && !reverse)
			AG_shape_translate_draw(globalPane,shapeTable,frameNum,sx,sy);
		else
			AG_shape_translate_transform(globalPane,shapeTable,frameNum,sx,sy,tempBuffer,reverse,scaleUp);
	}
	else
	{
		if(scaleUp && !reverse)
			AG_shape_draw(globalPane,shapeTable,frameNum,sx,sy);
		else
			AG_shape_transform(globalPane,shapeTable,frameNum,sx,sy,tempBuffer,reverse,scaleUp);
	}
	
	return(result);
}

//---------------------------------------------------------------------------
