//===========================================================================//
// File:		line.cpp                                                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Line3D&
	Line3D::SetDirection(const Vector3D &vector)
{
	Check_Pointer(this);
	Check_Object(&vector);

	//
	//---------------------------------------
	// Make sure length of vector is non-zero
	//---------------------------------------
	//
	length = vector.GetLength();
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Scalar
	Line3D::GetDistanceTo(
		const Sphere &sphere,
		Scalar *penetration
	) const
{
	Check_Object(this);
	Check_Object(&sphere);
	Check_Pointer(penetration);

	//
	//-------------------------------------------------------------------
	// Determine if ray intersects bounding sphere of object.  If sphere
	// is (X-C)*(X-C) = R^2 and ray is X = t*D+L for t >= 0, then
	// intersection is obtained by plugging X into sphere equation to
	// get quadratic:  (D*D)t^2 + 2*(D*(L-C))t + (L-C)*(L-C) = 0
	// Define a = D*D = 1.0f, b = 2*(D*(L-C)), and c = (L-C)*(L-C).
	//-------------------------------------------------------------------
	//
	Vector3D diff;
	diff.Subtract(origin, sphere.center);
	Scalar b = (direction*diff) * 2.0f;
	Scalar c = (diff*diff) - sphere.radius*sphere.radius;

	//
	//-------------------------------------------------------------------------
	// If penetration is negative, we couldn't hit the sphere at all.  If it is
	// really small, it touches at only one place
	//-------------------------------------------------------------------------
	//
	*penetration = b*b - 4.0f*c;
	if (*penetration < -SMALL)
	{
		return -1.0f;
	}
	b *= -0.5f;
	if (*penetration<SMALL)
	{
		*penetration = 0.0f;
		Min_Clamp(b, 0.0f);
		return (b > length) ? -1.0f : b;
	}

	//
	//-------------------------------------------------------------
	// We know we hit the sphere, so figure out where it first hits
	//-------------------------------------------------------------
	//
	*penetration = 0.5f * Sqrt(*penetration);
	if (b + *penetration < -SMALL)
	{
		return -1.0f;
	}
	b -= *penetration;
	if (b > length)
	{
		return -1.0f;
	}
	Min_Clamp(b, 0.0f);
	return b;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Scalar
	Line3D::GetDistanceTo(const OBB& box)
{
	Check_Object(this);
	Check_Object(&box);

	//
	//------------------------------------------------------------------------
	// Get the vector from the line to the centerpoint of the OBB.  All planes
	// will be generated relative to this
	//------------------------------------------------------------------------
	//
	Point3D center;
	center = box.localToParent;
	Vector3D delta;
	delta.Subtract(center, origin);

	//
	//--------------------------------------------------
	// Set up the loop to examine each of the three axes
	//--------------------------------------------------
	//
	Scalar enters = -100.0f - length;
	Scalar leaves = length + 100.0f;
	for (int axis=X_Axis; axis <= Z_Axis; ++axis)
	{
		UnitVector3D
			normal(
				box.localToParent(axis, X_Axis),
				box.localToParent(axis, Y_Axis),
				box.localToParent(axis, Z_Axis)
			);

		//
		//----------------------------------------------------------------------
		// Now, we have to calculate how far the line moves along the normal per
		// unit traveled down the line.  If it is perpendicular to the normal,
		// then it will hit or miss based solely upon the origin location
		//----------------------------------------------------------------------
		//
		Scalar drift = direction * normal;
		Scalar distance;
		if (Small_Enough(drift))
		{
			distance = delta * normal;
			if (Fabs(distance) > box.axisExtents[axis])
				return -1.0f;
			else
				continue;
		}

		//
		//--------------------------------------------------------------------
		// We know the line is not parallel, so we will now calculate how long
		// the line will stay inside the box.  We also will calculate how far
		// from the origin to the centerplane of the OBB
		//--------------------------------------------------------------------
		//
		drift = 1.0f / drift;
		Scalar span = box.axisExtents[axis] * Fabs(drift);
		distance = (delta * normal) * drift;

		//
		//--------------------------------------------------------------------
		// Now adjust where the line can enter and leave the OBB, and if it is
		// no longer possible to hit, stop checking
		//--------------------------------------------------------------------
		//
		Scalar enter = distance - span;
		Scalar leave = distance + span;
		if (enter > enters)
			enters = enter;
		if (leave < leaves)
			leaves = leave;
		if (enters > leaves)
			return -1.0f;
	}

	//
	//-------------------------------------------------------------------------
	// If we got here, then the line in theory can hit the OBB, so now we check
	// to make sure it hits it within the allowed span of the line
	//-------------------------------------------------------------------------
	//
	if (leaves < 0.0f || enters > length)
		return -1.0f;
	Min_Clamp(enters, 0.0f);
	return enters;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
Scalar
	Line3D::GetDistanceTo(
		const OBB& box,
		int *first_axis
	)
{
	Check_Object(this);
	Check_Object(&box);
	Check_Pointer(first_axis);

	//
	//------------------------------------------------------------------------
	// Get the vector from the line to the centerpoint of the OBB.  All planes
	// will be generated relative to this
	//------------------------------------------------------------------------
	//
	Point3D center;
	center = box.localToParent;
	Vector3D delta;
	delta.Subtract(center, origin);

	//
	//--------------------------------------------------
	// Set up the loop to examine each of the three axes
	//--------------------------------------------------
	//
	Scalar enters = -100.0f - length;
	Scalar leaves = length + 100.0f;
	for (int axis=X_Axis; axis <= Z_Axis; ++axis)
	{
		UnitVector3D
			normal(
				box.localToParent(axis, X_Axis),
				box.localToParent(axis, Y_Axis),
				box.localToParent(axis, Z_Axis)
			);

		//
		//----------------------------------------------------------------------
		// Now, we have to calculate how far the line moves along the normal per
		// unit traveled down the line.  If it is perpendicular to the normal,
		// then it will hit or miss based solely upon the origin location
		//----------------------------------------------------------------------
		//
		Scalar drift = direction * normal;
		Scalar distance;
		if (Small_Enough(drift))
		{
			distance = delta * normal;
			if (Fabs(distance) > box.axisExtents[axis])
				return -1.0f;
			else
				continue;
		}

		//
		//--------------------------------------------------------------------
		// We know the line is not parallel, so we will now calculate how long
		// the line will stay inside the box.  We also will calculate how far
		// from the origin to the centerplane of the OBB
		//--------------------------------------------------------------------
		//
		drift = 1.0f / drift;
		Scalar span = box.axisExtents[axis] * Fabs(drift);
		distance = (delta * normal) * drift;

		//
		//--------------------------------------------------------------------
		// Now adjust where the line can enter and leave the OBB, and if it is
		// no longer possible to hit, stop checking
		//--------------------------------------------------------------------
		//
		Scalar enter = distance - span;
		Scalar leave = distance + span;
		if (enter > enters)
		{
			*first_axis = axis;
			enters = enter;
		}
		if (leave < leaves)
			leaves = leave;
		if (enters > leaves)
			return -1.0f;
	}

	//
	//-------------------------------------------------------------------------
	// If we got here, then the line in theory can hit the OBB, so now we check
	// to make sure it hits it within the allowed span of the line
	//-------------------------------------------------------------------------
	//
	if (leaves < 0.0f || enters > length)
		return -1.0f;
	Min_Clamp(enters, 0.0f);
	return enters;
}
