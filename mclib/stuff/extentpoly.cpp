//===========================================================================//
// File:	extntbox.cc                                                      //
// Contents: Implementation details of bounding box class                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ExtentPoly::ExtentPoly(MemoryStream *stream)
{
	*stream >> minY;
	*stream >> maxY;
	*stream >> numberOfVertices;
	vertex = new Vector3D[numberOfVertices];
	Register_Object(vertex);
	for (int i=0;i<numberOfVertices;i++)
	{
		*stream >> vertex[i];
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ExtentPoly::ExtentPoly(
		FileStream *file_stream,
		char *page_name
		)
{
	Check_Object(this);
	Check_Object(file_stream);
	Check_Pointer(page_name);

	//
	//------------------
	// Parse fcurve page
	//------------------
	//
	char *assign_delim = "=";
	MString line_string;

	// Read page
	line_string = file_stream->ReadString();
	Verify(line_string == page_name);

	// Read in minY and maxY
	line_string = file_stream->ReadString();
	Verify(line_string.GetNthToken(0, assign_delim) == "MinY");
	minY = AtoF(line_string.GetNthToken(1, assign_delim));

	line_string = file_stream->ReadString();
	Verify(line_string.GetNthToken(0, assign_delim) == "MaxY");
	maxY = AtoF(line_string.GetNthToken(1, assign_delim));

	// Read numberOfVertices

	line_string = file_stream->ReadString();
	Verify(line_string.GetNthToken(0, assign_delim) == "NumberOfVertices");
	numberOfVertices = atoi(line_string.GetNthToken(1, assign_delim));
	vertex = new Vector3D[numberOfVertices];
	Register_Object(vertex);

	static char buffer[512];
	char *ptr;

	for (int i=0;i<numberOfVertices;i++)
	{
		file_stream->ReadLine(buffer, sizeof(buffer));
	   ptr = strchr(buffer, '=');
		Check_Pointer(ptr);
		ptr++;
		vertex[i].x = AtoF(ptr);

		vertex[i].y = 0.0f;

		file_stream->ReadLine(buffer, sizeof(buffer));
	   ptr = strchr(buffer, '=');
		Check_Pointer(ptr);
		ptr++;
		vertex[i].z = AtoF(ptr);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ExtentPoly::~ExtentPoly(void)
{
	Unregister_Object(vertex);
	delete vertex;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
ExtentPoly::Save(MemoryStream *stream)
{
	*stream << minY;
	*stream << maxY;
	*stream << numberOfVertices;
	for (int i=0;i<numberOfVertices;i++)
	{
		*stream << vertex[i];
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ExtentPoly&
	ExtentPoly::Intersect(
		const ExtentPoly &poly_1,
		const ExtentPoly &poly_2
	)
{
	Check_Pointer(this);
	Check_Object(&poly_1);
	Check_Object(&poly_2);
   return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ExtentPoly&
	ExtentPoly::Union(
		const ExtentPoly &poly_1,
		const ExtentPoly &poly_2
	)
{
	Check_Pointer(this);
	Check_Pointer(&poly_1);
	Check_Pointer(&poly_2);


	Check_Object(this);
   return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ExtentPoly&
	ExtentPoly::Union(
		const ExtentPoly &poly,
		const Vector3D &point
	)
{
	Check_Pointer(this);
	Check_Object(&poly);
	Check_Object(&point);

   return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Vector3D*
	ExtentPoly::ClosestPointOnLine(
	Vector3D *point,
	const Vector3D &p1,
	const Vector3D &p2
	)
{
	Scalar y = point->y;

	Vector3D v1;
	v1 = p1;
	v1 -= p2;
	v1.y = 0.0f;
	Scalar a = v1.GetLength();

	Vector3D v2;
	v2 = p1;
	v2 -= *point;
	v2.y = 0.0f;
	Scalar b_squared = v2.GetLengthSquared(); 

	Vector3D v3;
	v3 = p2;
	v3 -= *point;
	v3.y = 0.0f;
	Scalar c_squared = v3.GetLengthSquared();

	Scalar d = (c_squared + (a * a) - b_squared)/(2.0f*a);
	if (d > a)
	{
		*point = p1;
		point->y = y;
		return(point);
	}
	else if (d < 0.0f)
	{
		*point = p2;
		point->y = y;
		return(point);
	}
	else
	{
		point->Normalize(v1);
		*point *= d;
		*point += p2;
		point->y = y;
		return(point);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Scalar
	ExtentPoly::DistanceBetweenLineAndPoint(
	const Vector3D &point,
	const Vector3D &p1,
	const Vector3D &p2
	)
{
	Vector3D v1;
	v1 = p1;
	v1 -= p2;
	v1.y = 0.0f;
	Scalar a = v1.GetLength();

	Vector3D v2;
	v2 = p1;
	v2 -= point;
	v2.y = 0.0f;
	Scalar b = v2.GetLength(); 

	Vector3D v3;
	v3 = p2;
	v3 -= point;
	v3.y = 0.0f;
	Scalar c = v3.GetLength();

	Verify(!Small_Enough(a));

	Scalar d = ((c * c) + (a * a) - (b * b))/(2.0f*a);
	if (d > a)
	{
		return(b);
	}
	if (d < 0.0f)
	{
		return(c);
	}

	Scalar e = ((c * c) - (d * d));
	if (e < 0.0f)
	{
		return(0.0f);
	}
	else
	{
		return((Scalar)Sqrt(e));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Vector3D*
	ExtentPoly::Constrain(Vector3D *point) 
{
	Check_Object(this);
	Check_Object(point);
	

	if (point->y < minY)
	{
		point->y = minY;
	}
	if (point->y > maxY)
	{
		point->y = maxY;
	}

	if (!Contains(*point))
	{
		Scalar closest = 999999.9f;
		Scalar distance;
		Vector3D *point1 = NULL;
		Vector3D *point2 = NULL;

		for (int i=0;i<numberOfVertices - 1;i++)
		{
			distance = DistanceBetweenLineAndPoint(*point, vertex[i], vertex[i+1]);
			if (distance < closest)
			{
				closest = distance;
				point1 = &vertex[i];
				point2 = &vertex[i+1];
			}
		}
		distance = DistanceBetweenLineAndPoint(*point, vertex[numberOfVertices - 1], vertex[0]);
		if (distance < closest)
		{
			closest = distance;
			point1 = &vertex[numberOfVertices - 1];
			point2 = &vertex[0];
		}
		Verify(point1 && point2);
		ClosestPointOnLine(point, *point1, *point2);
	}

	return point;
}


int
	ExtentPoly::InfiniteLineTestWithXAxis(
	const Vector3D &point,
	const Vector3D &p1,
	const Vector3D &p2
	)
{
	// Translate everything to the origin
	Vector3D point1 = p1;
	point1 -= point;
	Vector3D point2 = p2;
	point2 -= point;
	// Are both endpoints left of the y-axis?
	if (point1.x <= 0 && point2.x <= 0)
	{
		return(0);
	}
	// Are both endpoints on the same side of the x-axis
	if (point2.z * point1.z > 0.0f)
	{
		return(0);
	}
	if (point1.x > 0.0f && point2.x > 0.0f)
	{
		return(1);
	}
	Scalar denom= point2.x - point1.x;
	Verify(!Small_Enough(denom));
	Scalar slope = (point2.z - point1.z) / denom; 
	Scalar y_intercept = slope*(-point1.x) + point1.z;
	// Is the line segment to the right of the origin
	if (slope*y_intercept<0.0f)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	ExtentPoly::Contains(const Vector3D &point) 
{
	Check_Object(this);
	Check_Object(&point);

	if (point.y < minY || point.y > maxY)
	{
		return(false);
	}
	int parity = 0;

	parity += InfiniteLineTestWithXAxis(point, vertex[numberOfVertices - 1], vertex[0]);
	for (int i=0;i<numberOfVertices - 1;i++)
	{
		parity += InfiniteLineTestWithXAxis(point, vertex[i], vertex[i+1]);
	}

	if (parity % 2 == 0)
	{
		return(false);
	}
	else
	{
		return(true);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	ExtentPoly::TestInstance() const
{
}
