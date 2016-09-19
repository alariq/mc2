//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"tgl.h"

extern bool InEditor;
//------------------------------------------------------------------------------------------------------------
// This function checks each visible triangle against the mouse position and returns if we are over ANY poly!
bool TG_Shape::PerPolySelect (float mouseX, float mouseY)
{
	if (!InEditor)
	{
		if (!listOfVertices ||
			!listOfColors ||
			!listOfShadowTVertices ||
			!listOfTriangles ||
			!listOfVisibleFaces ||
			!listOfVisibleShadows ||
			((lastTurnTransformed != (turn-1)) /*&& (lastTurnTransformed != turn)*/))
			return false;
	}
	else
	{
		if (!listOfVertices ||
			!listOfColors ||
			!listOfShadowTVertices ||
			!listOfTriangles ||
			!listOfVisibleFaces ||
			!listOfVisibleShadows ||
			(/*(lastTurnTransformed != (turn-1)) &&*/ (lastTurnTransformed != turn)))
			return false;
	}

	TG_TypeShapePtr theShape = (TG_TypeShapePtr)myType;
 	for (long j=0;j<numVisibleFaces;j++)
	{
		if (listOfVisibleFaces[j] != 0xffffffff)
		{
			TG_TypeTriangle triType = theShape->listOfTypeTriangles[listOfVisibleFaces[j]];
			Stuff::Point3D v0,v1,v2;
			v0.x = listOfVertices[triType.Vertices[0]].x;
			v0.y = listOfVertices[triType.Vertices[0]].y;
			
			v1.x = listOfVertices[triType.Vertices[1]].x;
			v1.y = listOfVertices[triType.Vertices[1]].y;
			
			v2.x = listOfVertices[triType.Vertices[2]].x;
			v2.y = listOfVertices[triType.Vertices[2]].y;
			v0.z = v1.z = v2.z = 0.0f;
			
			//Using the above vertex Data, determine if the mouse is over this poly!
			//
			//Theorem:
			// Given the sides of the triangle defined by the lines v0v1, v1v2 and v2v0
			// in the form Ax + By + C = 0
			//
			// the point mousex, mousey lies inside the triangle if and only if
			//
			//  	A0 * mouseX + B0 * mouseY + C0 = D0
			//		A1 * mouseX * B1 * mouseY + c1 = D1
			//		A2 * mouseX + B2 * mouseY + c2 = D2
			//
			// Dx is the same sign for each line as the correct sign for clockwise or counterclockwise vertices!
			//
			Stuff::Vector3D line1;
			Stuff::Vector3D line2;
			line1.Subtract(v0,v1);
			line2.Subtract(v1,v2);
			
			float order = line2.x * line1.y - line1.x * line2.y;
			order = sign(order);
			
			float A0 = -(v0.y - v1.y);
			float B0 = (v0.x - v1.x);
			float C0 = -B0*(v0.y) - A0*(v0.x);
			float D0 = A0 * mouseX + B0 * mouseY + C0;
			
			float A1 = -(v1.y - v2.y);
			float B1 = (v1.x - v2.x);
			float C1 = -B1*(v1.y) - A1*(v1.x);
			float D1 = A1 * mouseX + B1 * mouseY + C1;
 			
 			float A2 = -(v2.y - v0.y);
			float B2 = (v2.x - v0.x);
			float C2 = -B2*(v2.y) - A2*(v2.x);
			float D2 = A2 * mouseX + B2 * mouseY + C2;
			
			if ((sign(D0) == order) && (sign(D0) == sign(D1)) && (sign(D0) == sign(D2)))
				return true;
    	}
	}
	
	return false;
}


