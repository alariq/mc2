//###########################################################################
//
// $Workfile:   RAY.HPP  $
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"point3d.hpp"
#include"unitvector.hpp"

namespace Stuff {

	class Plane;
	class Sphere;
	class OBB;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Ray3D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Ray3D
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	 public:
		Point3D
			origin;
		UnitVector3D
			direction;

		Ray3D()
			{}
		Ray3D(
			const Point3D &origin,
			const UnitVector3D &direction
		):
			origin(origin),
			direction(direction)
				{}

		//
		// Ray3D projection functions
		//
		void
			Project(
				Scalar length,
				Point3D *result
			)
				{
					Check_Object(this); Check_Object(result);
					Vector3D temp; temp.Multiply(direction,length);
					result->Add(origin,temp);
				}
		Scalar
			GetLengthToClosestPointTo(const Point3D &point)
				{
					Check_Object(this); Check_Object(&point);
					Vector3D temp; temp.Subtract(point,origin);
					return temp*direction;
				}
		void
			GetClosestPointTo(
				const Point3D &point,
				Point3D *result
			)
				{
					Check_Object(this); Check_Object(result); Check_Object(&point);
					Project(GetLengthToClosestPointTo(point),result);
				}

		//
		// Assignment operators
		//
		Ray3D&
			SetDirection(const Vector3D &vector);
		Ray3D&
			SetOrigin(const Point3D &point)
				{Check_Pointer(this); origin = point; return *this;}

		//
		// Ray3D intersection functions
		//
		Scalar
			GetDistanceTo(
				const Plane &plane,
				Scalar *product
			) const;
		Scalar
			GetDistanceTo(
				const Sphere &sphere,
				Scalar *penetration
			) const;
		Scalar
			GetDistanceTo(const OBB &obb) const;

		//
		// Test support
		//
		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

	Scalar
		Find_Closest_Approach(
			const Point3D& origin1,
			const Vector3D& velocity1,
			Point3D *result1,
			const Point3D& origin2,
			const Vector3D& velocity2,
			Point3D *result2,
			Scalar *time,
			bool *constant
		);

}
