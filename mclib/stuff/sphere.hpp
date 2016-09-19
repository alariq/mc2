//===========================================================================//
// File:		sphere.hpp                                                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
#pragma once

#include"stuff.hpp"
#include"point3d.hpp"

namespace Stuff {class Sphere;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Sphere& sphere
		);
#endif

namespace Stuff {

	class Plane;
	class OBB;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Sphere ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Sphere
	{
	 public:
		Point3D
			center;
		Scalar
			radius;

		Sphere()
			{}
		Sphere(
			const Point3D &A_Point,
			Scalar Radius
		):
			center(A_Point),
			radius(Radius)
				{}
		Sphere(
			Scalar X,
			Scalar Y,
			Scalar Z,
			Scalar Radius
		):
			center(X,Y,Z),
			radius(Radius)
				{}
		Sphere(const Sphere& sphere):
			center(sphere.center),
			radius(sphere.radius)
				{}
		explicit Sphere(const OBB& obb)
			{*this = obb;}

		Sphere&
			operator =(const Sphere &sphere)
		{
			Check_Pointer(this); Check_Object(&sphere);
			center = sphere.center;
			radius = sphere.radius;
			return *this;
		}
		Sphere&
			operator =(const OBB &obb);

		Sphere&
			Union(
				const Sphere& sphere1,
				const Sphere& sphere2
			);

		//
		// Intersection functions
		//
		bool
			Contains(const Point3D &point) const
				{
					Check_Object(this); Check_Object(&point);
					Vector3D diff;
					diff.Subtract(center, point);
					return radius*radius - diff.GetLengthSquared() > -SMALL;
				}
		bool
			Intersects(const Sphere &sphere) const
				{
					Check_Object(this); Check_Object(&sphere);
					Scalar r = radius + sphere.radius;
					Vector3D temp;
					temp.Subtract(center, sphere.center);
					return r*r - temp.GetLengthSquared() >= -SMALL;
				}
		bool
			Intersects(const Plane &plane) const;

		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Sphere& sphere
				);
		#endif
		void
			TestInstance() const
				{}
	};

}
