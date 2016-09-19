//###########################################################################
//
// $Workfile:   RAY.CPP  $
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Ray3D&
	Ray3D::SetDirection(const Vector3D &vector)
{
	Check_Pointer(this);
	Check_Object(&vector);

	//
	//---------------------------------------
	// Make sure length of vector is non-zero
	//---------------------------------------
	//
	Scalar length = vector.GetLength();
	Verify(!Small_Enough(length));
	length = 1.0f / length;

	//
	//----------------------------------------------
	// Normalize the vector and put it into the line
	//----------------------------------------------
	//
	direction.x = vector.x*length;
	direction.y = vector.y*length;
	direction.z = vector.z*length;
	return *this;
}

//
//#############################################################################
//#############################################################################
//
Scalar
	Ray3D::GetDistanceTo(
		const Plane &plane,
		Scalar *product
	) const
{
	Check_Object(this);
	Check_Object(&plane);
	Check_Pointer(product);

	*product = direction * plane.normal;
	if (Small_Enough(*product))
	{
		return -1.0f;
	}
	Scalar result = -plane.GetDistanceTo(origin) / *product;
	return result;
}

//
//#############################################################################
//#############################################################################
//
Scalar
	Ray3D::GetDistanceTo(
		const Sphere &sphere,
		Scalar *penetration
	) const
{
	Scalar
		b,c;
	Vector3D
		temp;

	//
	//-------------------------------------------------------------------------
	// Set up to solve a quadratic equation for the intersection of the ray and
	// sphere.  The solution is based on finding the closest point on the line
	// to the sphere, and then calculating the interval between the entry and
	// exit points of the ray
	//-------------------------------------------------------------------------
	//
	temp.Subtract(origin,sphere.center);
	b = 2.0f * (direction * temp);
	c = temp.GetLengthSquared() - sphere.radius*sphere.radius;

	//
	//--------------------------------------------------------------------------
	// Compute the squared interval to use for the solution.  If it is negative,
	// then the ray misses the sphere
	//--------------------------------------------------------------------------
	//
	*penetration = b*b - 4.0f*c;
	if (*penetration<SMALL)
		return 0.0f;

	//
	//-------------------------------------------------------------------------
	// Otherwise, find the linear distance along the line of the entry point by
	// subtracting half the interval between entry and exit points from the
	// distance to the closest point on the sphere
	//-------------------------------------------------------------------------
	//
	else
	{
		*penetration = Sqrt(*penetration);
		return -0.5f*(b+*penetration);
	}
}

//
//#############################################################################
//#############################################################################
//
Scalar
	Stuff::Find_Closest_Approach(
		const Point3D& origin1,
		const Vector3D& velocity1,
		Point3D *result1,
		const Point3D& origin2,
		const Vector3D& velocity2,
		Point3D *result2,
		Scalar *time,
		bool *constant
	)
{
	Vector3D a,b;
	a.Subtract(origin1, origin2);
	b.Subtract(velocity1, velocity2);

	//
	//--------------------------------------------------------------------
	// If the velocities are identical, any point will do for the test, so
	// simply return the difference between the starting points
	//--------------------------------------------------------------------
	//
	Scalar d = b.GetLengthSquared();
	if (Small_Enough(d))
	{
		*constant = true;
		d = a.GetLength();
		return d;
	}

	//
	//-------------------------------------------------------------------------
	// The velocities are not parallel, so figure out when the closest approach
	// is via the derivative
	//-------------------------------------------------------------------------
	//	
	*constant = false;
	*time = (a * b) / -d;

	//
	//------------------------------------------------------
	// Now, plot the resultant points of both line equations
	//------------------------------------------------------
	//
	Vector3D closest;
	closest.AddScaled(a, b, *time);
	result1->AddScaled(origin1, velocity1, *time);
	result2->AddScaled(origin2, velocity2, *time);
	d = closest.GetLength();
	return d;
}
