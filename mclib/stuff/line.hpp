//=======================================================================//
// File:		line.hpp                                                 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"ray.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Line3D3D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Line3D:
		public Ray3D
	{
	 public:
		Scalar
			length;

		Line3D()
			{}
		Line3D(
			const Ray3D &ray,
			Scalar length
		):
			Ray3D(ray),
			length(length)
				{}
		Line3D(
			const Point3D &start,
			const UnitVector3D &direction,
			Scalar length
		):
			Ray3D(start,direction),
			length(length)
				{}

		//
		// Assignment operators
		//
		Line3D&
			SetDirection(const Vector3D &vector);

		void
			FindEnd(Point3D *result)
				{Check_Object(this); Check_Pointer(result); Ray3D::Project(length, result);}
			
		//
		// Intersection functions
		//
		Scalar
			GetDistanceTo(
				const Plane &plane,
				Scalar *product
			) const
				{Check_Object(this); return Ray3D::GetDistanceTo(plane, product);}
		Scalar
			GetDistanceTo(
				const Sphere &sphere,
				Scalar *penetration
			) const;
		Scalar
			GetDistanceTo(const OBB& box);
		Scalar
			GetDistanceTo(
				const OBB& box,
				int *axis
			);
	};

}
