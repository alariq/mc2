//===========================================================================//
// File:		sphere.cpp                                                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Sphere&
	Sphere::Union(
		const Sphere& sphere1,
		const Sphere& sphere2
	)
{
	Check_Object(this);
	Check_Object(&sphere1);
	Check_Object(&sphere2);

	//
	//--------------------------------------------------
	// Calculate the length between the sphere midpoints
	//--------------------------------------------------
	//
	Vector3D dist;
	dist.Subtract(sphere1.center, sphere2.center);
	Scalar len = dist.GetLength();

	//
	//------------------------------------------------------
	// If the sphere is contained in the old sphere, move on
	//------------------------------------------------------
	//
	if (len + sphere1.radius <= sphere2.radius)
	{
		*this = sphere2;
		return *this;
	}

	//
	//----------------------------------------------------------
	// If the new sphere contains the old sphere, use it instead
	//----------------------------------------------------------
	//
	if (len + sphere2.radius <= sphere1.radius)
	{
		*this = sphere1;
		return *this;
	}

	//
	//------------------------------
	// Calculate the new centerpoint
	//------------------------------
	//
	len += sphere1.radius + sphere2.radius;
	UnitVector3D direction;
	direction.Normalize(dist);
	len *= 0.5f;
	center.AddScaled(
		sphere2.center,
		direction,
		len - sphere2.radius
	);
	radius = len;
	return *this;
}

//
//###########################################################################
//###########################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Sphere& sphere
		)
	{
		Check_Object(&sphere);

		SPEW((group, "\n\tSphere Centerpoint: +"));
		Spew(group, sphere.center);
		SPEW((group, "\tRadius: %f", sphere.radius));
	}
#endif