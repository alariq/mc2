//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------------------------------
// Screen Clip code for use with video cards without guard bands and BLADE.
// 
// Generally, we will only need one or two of the below routines per triangle.
//
// If a triangle is completely on screen, none of the below should be called.
//
// Each triangle can be easily checked to see which of the below should be 
// called in its case.  An easy, low-end machine optimization would be to
// NOT call any of the below for TGL/MSL shapes which are partly off screen
// and simply not to draw the shape.  Profiling will indicate if this is
// necessary.  
//
// Color space clipping is HUGELY painful and we are cheating until it 
// becomes necessary to do otherwise.  
//
// Triangles will be converted into polygons with as many as five sides.
// It will be necessary in the draw code to hand the vertices down to
// GOS/BLADE in triangles to avoid any weirdness in their code!
//
// These routines are easily optimized and assembled if and when it becomes
// necessary to do so.  Routines should be inlined for speed!
//---------------------------------------------------------------------------------------------------
#ifndef CLIP_H
#define CLIP_H
#include<gameos.hpp>

//---------------------------------------------------------------------------------------------------
inline void xLeftEdgeclip (gos_VERTEX *vertices, long &numVertices, float leftEdge)
{
	//------------------------------------------
	//make local copy of all point data involved
	gos_VERTEX points[6];

	memcpy(points,vertices,sizeof(gos_VERTEX)*numVertices);
	//------------------------------------------
	long first				= numVertices-1;
	long second				= 0;

	//----------------------------------------------
	// Index to current vertex in clipped polygon.
	//----------------------------------------------
	long work = 0;

	//------------------------------
	//final number of polygon points
	//------------------------------
	long n_points= 0;

	//----------------------------
	//process points in edge order
	//----------------------------
	for (long s=0; s<numVertices; s++, second++)
	{
		float clipEdgeTest = points[first].x;

		//----------------------------------------------------------------
		// if this point is not beyond the clipping Edge, store it.
		//----------------------------------------------------------------
		if (clipEdgeTest >= leftEdge)
		{
			//------------------------------------------------------------
			gos_VERTEX* workv = &(vertices[work]);
			memcpy(workv,&(points[first]),sizeof(gos_VERTEX));
			//------------------------------------------------------------
		
			work++;
			n_points++;
		}
		
		//------------------------------------------------------------
		//if the line defined by first and second intersects the 
		//clipping plane, generate a new point and calculate its info.
		//------------------------------------------------------------
		
		if (((clipEdgeTest < leftEdge) && (points[second].x >= leftEdge)) ||
			((clipEdgeTest >= leftEdge) && (points[second].x < leftEdge)))
		{
			gos_VERTEX* workv = &(vertices[work]);
		
			float sry = points[second].y;
			float srx = points[second].x;
			float srz = points[second].z;
			float srw = points[second].rhw;
			float sru = points[second].u;
			float srv = points[second].v;
			
			float fry = points[first].y;
			float frx = points[first].x;
			float frz = points[first].z;
			float frw = points[first].rhw;
			float fru = points[first].u;
			float frv = points[first].v;
				
			float mu = (leftEdge - srx)/(frx - srx);
			
			workv->x = leftEdge;
			workv->y = ((fry - sry) * mu) + sry;
			workv->z = ((frz - srz) * mu) + srz;

			// Clipping color is HUGELY painful at this point.  Try cheating first!
			if (clipEdgeTest < leftEdge)
			{
				workv->argb = points[first].argb;
				workv->frgb = points[first].frgb;
			}
			else
			{
				workv->argb = points[second].argb;
				workv->frgb = points[second].frgb;
			}

			workv->rhw = ((frw - srw) * mu) + srw;

			workv->u = ((fru - sru) * mu) + sru;
			workv->v = ((frv - srv) * mu) + srv;

			work++;
			n_points++;
		}
	
		first = second;
	}

	numVertices = n_points;
}

//---------------------------------------------------------------------------------------------------
inline void xRightEdgeclip (gos_VERTEX *vertices, long &numVertices, float rightEdge)
{
	//------------------------------------------
	//make local copy of all point data involved
	gos_VERTEX points[6];

	memcpy(points,vertices,sizeof(gos_VERTEX)*numVertices);
	//------------------------------------------
	long first				= numVertices-1;
	long second				= 0;

	//----------------------------------------------
	// Index to current vertex in clipped polygon.
	//----------------------------------------------
	long work = 0;

	//------------------------------
	//final number of polygon points
	//------------------------------
	long n_points= 0;

	//----------------------------
	//process points in edge order
	//----------------------------
	for (long s=0; s<numVertices; s++, second++)
	{
		float clipEdgeTest = points[first].x;

		//----------------------------------------------------------------
		// if this point is not beyond the clipping Edge, store it.
		//----------------------------------------------------------------
		if (clipEdgeTest < rightEdge)
		{
			//------------------------------------------------------------
			gos_VERTEX* workv = &(vertices[work]);
			memcpy(workv,&(points[first]),sizeof(gos_VERTEX));
			//------------------------------------------------------------
		
			work++;
			n_points++;
		}
		
		//------------------------------------------------------------
		//if the line defined by first and second intersects the 
		//clipping plane, generate a new point and calculate its info.
		//------------------------------------------------------------
		
		if (((clipEdgeTest >= rightEdge) && (points[second].x < rightEdge)) ||
			((clipEdgeTest < rightEdge) && (points[second].x >= rightEdge)))
		{
			gos_VERTEX* workv = &(vertices[work]);
		
			float sry = points[second].y;
			float srx = points[second].x;
			float srz = points[second].z;
			float srw = points[second].rhw;
			float sru = points[second].u;
			float srv = points[second].v;
			
			float fry = points[first].y;
			float frx = points[first].x;
			float frz = points[first].z;
			float frw = points[first].rhw;
			float fru = points[first].u;
			float frv = points[first].v;
				
			float mu = (rightEdge - srx)/(frx - srx);
			
			workv->x = rightEdge - 1.0f;
			workv->y = ((fry - sry) * mu) + sry;
			workv->z = ((frz - srz) * mu) + srz;

			// Clipping color is HUGELY painful at this point.  Try cheating first!
			if (clipEdgeTest >= rightEdge)
			{
				workv->argb = points[first].argb;
				workv->frgb = points[first].frgb;
			}
			else
			{
				workv->argb = points[second].argb;
				workv->frgb = points[second].frgb;
			}

			workv->rhw = ((frw - srw) * mu) + srw;

			workv->u = ((fru - sru) * mu) + sru;
			workv->v = ((frv - srv) * mu) + srv;

			work++;
			n_points++;
		}
	
		first = second;
	}

	numVertices = n_points;
}

//---------------------------------------------------------------------------------------------------
inline void yTopEdgeclip (gos_VERTEX *vertices, long &numVertices, float topEdge)
{
	//------------------------------------------
	//make local copy of all point data involved
	gos_VERTEX points[6];

	memcpy(points,vertices,sizeof(gos_VERTEX)*numVertices);
	//------------------------------------------
	long first				= numVertices-1;
	long second				= 0;

	//----------------------------------------------
	// Index to current vertex in clipped polygon.
	//----------------------------------------------
	long work = 0;

	//------------------------------
	//final number of polygon points
	//------------------------------
	long n_points= 0;

	//----------------------------
	//process points in edge order
	//----------------------------
	for (long s=0; s<numVertices; s++, second++)
	{
		float clipEdgeTest = points[first].y;

		//----------------------------------------------------------------
		// if this point is not beyond the clipping Edge, store it.
		//----------------------------------------------------------------
		if (clipEdgeTest >= topEdge)
		{
			//------------------------------------------------------------
			gos_VERTEX* workv = &(vertices[work]);
			memcpy(workv,&(points[first]),sizeof(gos_VERTEX));
			//------------------------------------------------------------
		
			work++;
			n_points++;
		}
		
		//------------------------------------------------------------
		//if the line defined by first and second intersects the 
		//clipping plane, generate a new point and calculate its info.
		//------------------------------------------------------------
		
		if (((clipEdgeTest < topEdge) && (points[second].y >= topEdge)) ||
			((clipEdgeTest >= topEdge) && (points[second].y < topEdge)))
		{
			gos_VERTEX* workv = &(vertices[work]);
		
			float sry = points[second].y;
			float srx = points[second].x;
			float srz = points[second].z;
			float srw = points[second].rhw;
			float sru = points[second].u;
			float srv = points[second].v;
			
			float fry = points[first].y;
			float frx = points[first].x;
			float frz = points[first].z;
			float frw = points[first].rhw;
			float fru = points[first].u;
			float frv = points[first].v;
				
			float mu = (topEdge - sry)/(fry - sry);
			
			workv->y = topEdge;
			workv->x = ((frx - srx) * mu) + srx;
			workv->z = ((frz - srz) * mu) + srz;

			// Clipping color is HUGELY painful at this point.  Try cheating first!
			if (clipEdgeTest < topEdge)
			{
				workv->argb = points[first].argb;
				workv->frgb = points[first].frgb;
			}
			else
			{
				workv->argb = points[second].argb;
				workv->frgb = points[second].frgb;
			}

			workv->rhw = ((frw - srw) * mu) + srw;

			workv->u = ((fru - sru) * mu) + sru;
			workv->v = ((frv - srv) * mu) + srv;

			work++;
			n_points++;
		}
	
		first = second;
	}

	numVertices = n_points;
}

//---------------------------------------------------------------------------------------------------
inline void yBottomEdgeclip (gos_VERTEX *vertices, long &numVertices, float bottomEdge)
{
	//------------------------------------------
	//make local copy of all point data involved
	gos_VERTEX points[6];

	memcpy(points,vertices,sizeof(gos_VERTEX)*numVertices);
	//------------------------------------------
	long first				= numVertices-1;
	long second				= 0;

	//----------------------------------------------
	// Index to current vertex in clipped polygon.
	//----------------------------------------------
	long work = 0;

	//------------------------------
	//final number of polygon points
	//------------------------------
	long n_points= 0;

	//----------------------------
	//process points in edge order
	//----------------------------
	for (long s=0; s<numVertices; s++, second++)
	{
		float clipEdgeTest = points[first].y;

		//----------------------------------------------------------------
		// if this point is not beyond the clipping Edge, store it.
		//----------------------------------------------------------------
		if (clipEdgeTest < bottomEdge)
		{
			//------------------------------------------------------------
			gos_VERTEX* workv = &(vertices[work]);
			memcpy(workv,&(points[first]),sizeof(gos_VERTEX));
			//------------------------------------------------------------
		
			work++;
			n_points++;
		}
		
		//------------------------------------------------------------
		//if the line defined by first and second intersects the 
		//clipping plane, generate a new point and calculate its info.
		//------------------------------------------------------------
		
		if (((clipEdgeTest >= bottomEdge) && (points[second].y < bottomEdge)) ||
			((clipEdgeTest < bottomEdge) && (points[second].y >= bottomEdge)))
		{
			gos_VERTEX* workv = &(vertices[work]);
		
			float sry = points[second].y;
			float srx = points[second].x;
			float srz = points[second].z;
			float srw = points[second].rhw;
			float sru = points[second].u;
			float srv = points[second].v;
			
			float fry = points[first].y;
			float frx = points[first].x;
			float frz = points[first].z;
			float frw = points[first].rhw;
			float fru = points[first].u;
			float frv = points[first].v;
				
			float mu = (bottomEdge - sry)/(fry - sry);
			
			workv->y = bottomEdge - 1.0f;
			workv->x = ((frx - srx) * mu) + srx;
			workv->z = ((frz - srz) * mu) + srz;

			// Clipping color is HUGELY painful at this point.  Try cheating first!
			if (clipEdgeTest >= bottomEdge)
			{
				workv->argb = points[first].argb;
				workv->frgb = points[first].frgb;
			}
			else
			{
				workv->argb = points[second].argb;
				workv->frgb = points[second].frgb;
			}

			workv->rhw = ((frw - srw) * mu) + srw;

			workv->u = ((fru - sru) * mu) + sru;
			workv->v = ((frv - srv) * mu) + srv;

			work++;
			n_points++;
		}
	
		first = second;
	}

	numVertices = n_points;
}

//---------------------------------------------------------------------------------------------------
#endif