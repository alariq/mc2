//===========================================================================//
// File:	plane.cc                                                         //
// Contents: Implementation details for the plane class                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//#############################################################################
//#############################################################################
//
void
	Plane::BuildPlane(
		const Point3D& p0,
		const Point3D& p1,
		const Point3D& p2
	)
{
	Vector3D v1,v2;
	v1.Subtract(p1, p0);
	v2.Subtract(p2, p1);
	
	Vector3D axis;
	axis.Cross(v1, v2);
	Verify(!Small_Enough(axis.GetLength()));
	normal = axis;
	offset = normal * p0;
	#if defined(_ARMOR)
		Scalar offset2 = normal * p1;
		Scalar offset3 = normal * p2;

		//
		//----------------------------------------------------------------
		// occasionaly the offset becomes large and the triangle is small
		// this way we still maintain proof up to 1e-4 of the offset value
		//----------------------------------------------------------------
		//
		Verify(Close_Enough(
			offset,
			offset2,
			Fabs(offset+offset2) > 200.0f
				? SMALL*Fabs(offset+offset2)/200.0f
				: SMALL
		));
		Verify(Close_Enough(
			offset,
			offset3,
			Fabs(offset+offset3) > 200.0f
				? SMALL*Fabs(offset+offset3)/200.0f
				: SMALL
		));
	#endif
}

//
//#############################################################################
//#############################################################################
//
Plane&
	Plane::Multiply(
		const Plane &p,
		const LinearMatrix4D &m
	)
{
	Check_Pointer(this);
	Check_Object(&p);
	Check_Object(&m);
	Verify(this != &p);

	normal.x = p.normal.x*m(0,0) + p.normal.y*m(1,0) + p.normal.z*m(2,0);
	normal.y = p.normal.x*m(0,1) + p.normal.y*m(1,1) + p.normal.z*m(2,1);
	normal.z = p.normal.x*m(0,2) + p.normal.y*m(1,2) + p.normal.z*m(2,2);
	offset =
		normal.x*m(3,0) + normal.y*m(3,1) + normal.z*m(3,2) + p.offset;

	return *this;
}

//
//#############################################################################
//#############################################################################
//
Scalar
	Plane::GetDistanceTo(const Sphere& sphere) const
{
	Check_Object(this);
	Check_Object(&sphere);

	//
	//-----------------------------------------------------------------------
	// Find out how far the centerpoint of the sphere is from us, then adjust
	// so that 0.0 means we are intersecting the sphere
	//-----------------------------------------------------------------------
	//
	Scalar distance = GetDistanceTo(sphere.center);
	if (distance > sphere.radius)
		return distance - sphere.radius;
	else if (distance < -sphere.radius)
		return distance + sphere.radius;
	return 0.0f;
}

//
//#############################################################################
//#############################################################################
//
Scalar
	Plane::GetDistanceTo(const OBB& box) const
{
	Check_Object(this);
	Check_Object(&box);

	//
	//-------------------------------------------------
	// Rotate the plane into the local space of the box
	//-------------------------------------------------
	//
	Plane local_plane;
	LinearMatrix4D parent_to_local;
	parent_to_local.Invert(box.localToParent);
	local_plane.Multiply(*this, parent_to_local);
	Point3D inside_extents(box.axisExtents);
	Point3D outside_extents(box.axisExtents);

	//
	//---------------------------------------------------------------------
	// Look at the resulting vector to find out which corners of the box we
	// should be testing
	//---------------------------------------------------------------------
	//
	if (local_plane.normal.x > 0.0f)
		inside_extents.x = -inside_extents.x;
	else
		outside_extents.x = -outside_extents.x;
	if (local_plane.normal.y > 0.0f)
		inside_extents.y = -inside_extents.y;
	else
		outside_extents.y = -outside_extents.y;
	if (local_plane.normal.z > 0.0f)
		inside_extents.z = -inside_extents.z;
	else
		outside_extents.z = -outside_extents.z;

	//
	//---------------------------
	// Test against the plane now
	//---------------------------
	//
	Scalar distance = local_plane.GetDistanceTo(outside_extents);
	if (distance < 0.0f)
		return distance;
	distance = local_plane.GetDistanceTo(inside_extents);
	if (distance > 0.0f)
		return distance;
	return 0.0f;
}

//
//#############################################################################
//#############################################################################
//
bool
	Plane::ContainsSomeOf(
		const Sphere &sphere,
		Scalar thickness
	) const
{
	return normal*sphere.center - offset <= sphere.radius - thickness;
}

//
//#############################################################################
//#############################################################################
//
bool
	Plane::ContainsAllOf(
		const Sphere &sphere,
		Scalar thickness
	) const
{
	return offset - normal*sphere.center >= sphere.radius + thickness;
}

//
//#############################################################################
//#############################################################################
//
bool
	Plane::ContainsSomeOf(
		const ExtentBox &box,
		Scalar thickness
	) const
{
	Check_Object(this);
	Check_Object(&box);

	Point3D test;
	test.x = (normal.x > 0.0f) ? box.minX : box.maxX;
	test.y = (normal.y > 0.0f) ? box.minY : box.maxY;
	test.z = (normal.z > 0.0f) ? box.minZ : box.maxZ;
	return normal * test < offset - thickness;
}

//
//#############################################################################
//#############################################################################
//
bool
	Plane::ContainsAllOf(
		const ExtentBox &box,
		Scalar thickness
	) const
{
	Check_Object(this);
	Check_Object(&box);

	Point3D test;
	test.x = (normal.x < 0.0f) ? box.minX : box.maxX;
	test.y = (normal.y < 0.0f) ? box.minY : box.maxY;
	test.z = (normal.z < 0.0f) ? box.minZ : box.maxZ;
	return normal * test <= offset + thickness;
}

//
//#############################################################################
//#############################################################################
//
bool
	Plane::Intersects(
		const Sphere &sphere,
		Scalar thickness
	) const
{
	Scalar dist = normal*sphere.center - offset;
	return Abs(dist) <= sphere.radius + thickness;
}

//
//#############################################################################
//#############################################################################
//
bool
	Plane::Intersects(
		const ExtentBox &box,
		Scalar thickness
	) const
{
	if (!ContainsSomeOf(box, thickness))
	{
		return false;
	}
	STOP(("What is this????"));
	Plane inverse(-normal.x, -normal.y, -normal.z, -offset);
	return false;
//	return inverse.ContainsSomeOf(box);
}

//
//#############################################################################
//#############################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Plane& plane
		)
	{
		Check_Object(&plane);
		SPEW((group, "\n\tPlane Normal: +"));
		Spew(group, plane.normal);
		SPEW((group, "\tOffset: %f+", plane.offset));
	}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Plane::ComputeBestDividingPlane(DynamicArrayOf<Point3D> &points)
{
	Check_Object(&points);

	//
	//-----------------
	// Compute the sums
	//-----------------
	//
	unsigned count = points.GetLength();
	Verify(count > 1);
	Point3D sum = Point3D::Identity;
	Scalar xx = 0.0f;
	Scalar xy = 0.0f;
	Scalar xz = 0.0f;
	Scalar yy = 0.0f;
	Scalar yz = 0.0f;
	Scalar zz = 0.0f;
	unsigned i;
	for (i=0; i<count; i++) 
	{
		sum += points[i];
		xx += points[i].x*points[i].x;
		xy += points[i].x*points[i].y;
		xz += points[i].x*points[i].z;
		yy += points[i].y*points[i].y;
		yz += points[i].y*points[i].z;
		zz += points[i].z*points[i].z;
	}

	//
	//--------------------------------------
	// Compute the least squares differences
	//--------------------------------------
	//
	Scalar t = 1.0f / static_cast<Scalar>(count);
	xx -= t*sum.x*sum.x;
	xy -= t*sum.x*sum.y;
	xz -= t*sum.x*sum.z;
	yy -= t*sum.y*sum.y;
	yz -= t*sum.y*sum.z;
	zz -= t*sum.z*sum.z;

	//
	//------------------------------------------------------------------
	// Find the largest axis component, and use that to orient the plane
	//------------------------------------------------------------------
	//
	int axis;
	if (xx > yy)
		axis = (xx > zz) ? X_Axis : Z_Axis;
	else
		axis = (yy > zz) ? Y_Axis : Z_Axis;
	Vector3D direction;
	switch (axis)
	{
	case X_Axis:
		if (xx < SMALL)
			return false;
		direction.x = 1.0f;
		direction.y = xy / xx;
		direction.z = xz / xx;
		break;

	case Y_Axis:
		if (yy < SMALL)
			return false;
		Verify(yy > SMALL);
		direction.x = xy / yy;
		direction.y = 1.0f;
		direction.z = yz / yy;
		break;

	case Z_Axis:
		if (zz < SMALL)
			return false;
		direction.x = xz / zz;
		direction.y = yz / zz;
		direction.z = 1.0f;
		break;

	default:
		direction = Vector3D::Identity;
		STOP(("Bad switch"));
	}


	//
	//-----------------------------------------------------------------------
	// The dividing plane will coincide with the mean point and have a normal
	// parallel to the variance vector
	//-----------------------------------------------------------------------
	//
	normal = direction;
	sum *= t;
	offset = normal * sum;
	Check_Object(this);

	//
	//-----------------------------------------------------------------------
	// Now make sure that a coincident set didn't get through due to rounding
	// errors
	//-----------------------------------------------------------------------
	//
	for (i=0; i<count; i++) 
	{
		if (points[i] != sum)
			return true;
	}
	return false;
}

