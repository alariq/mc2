//===========================================================================//
// File:	plane.hh                                                         //
// Contents: Interface specification of the plane class                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"normal.hpp"
#include"point3d.hpp"

namespace Stuff {class Plane;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Plane& plane
		);
#endif

namespace Stuff {

	class Sphere;
	class ExtentBox;
	class OBB;
	class Line3D;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Plane ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Plane
	{
	 public:
		//
		// The plane equation is P*normal = offset, where P is a homogeneous
		//
		Normal3D
			normal;
		Scalar
			offset;

		//
		// Constructors
		//
		Plane()
			{}
		Plane(
			Scalar x,
			Scalar y,
			Scalar z,
			Scalar offset
		):
			normal(x,y,z),
			offset(offset)
				{}
		Plane(
			const Normal3D& n,
			Scalar offset
		):
			normal(n),
			offset(offset)
				{}
		Plane(
			const Point3D& p0,
			const Point3D& p1,
			const Point3D& p2
		)
			{BuildPlane(p0, p1, p2);}

		void
			BuildPlane(
				const Point3D& p0,
				const Point3D& p1,
				const Point3D& p2
			);

		//
		// Transform functions
		//
		Plane&
			Multiply(
				const Plane &p,
				const LinearMatrix4D &m
			);
		Plane&
			operator*=(const LinearMatrix4D &m)
				{Check_Object(this); Plane t(*this); return Multiply(t,m);}

		//
		// half-space division functions
		//
		bool
			IsSeenBy(const Vector3D &A_Vector) const
				{return normal * A_Vector < 0.0;}
		bool
			IsSeenBy(const Point3D &A_Point) const
				{return normal * A_Point > offset;}
		Scalar
			GetDistanceTo(const Point3D& A_Point) const
				{return normal * A_Point - offset;}
		Scalar
			GetDistanceTo(const Sphere& sphere) const;
		Scalar
			GetDistanceTo(const OBB& box) const;

		//
		// half-space containment functions
		//
		bool
			Contains(
				const Point3D &point,
				Scalar thickness=SMALL
			) const
				{Check_Object(this); return normal * point <= offset + thickness;}
		bool
			ContainsSomeOf(
				const Sphere &sphere,
				Scalar thickness=SMALL
			) const;
		bool
			ContainsAllOf(
				const Sphere &sphere,
				Scalar thickness=SMALL
			) const;
		bool
			ContainsSomeOf(
				const ExtentBox &box,
				Scalar thickness=SMALL
			) const;
		bool
			ContainsAllOf(
				const ExtentBox &box,
				Scalar thickness=SMALL
			) const;
		bool
			ContainsSomeOf(
				const OBB &box,
				Scalar thickness=SMALL
			) const;
		bool
			ContainsAllOf(
				const OBB &box,
				Scalar thickness=SMALL
			) const;

		//
		// plane surface intersection functions
		//
		bool
			Intersects(
				const Sphere &sphere,
				Scalar thickness=SMALL
			) const;
		bool
			Intersects(
				const ExtentBox &box,
				Scalar thickness=SMALL
			) const;
		bool
			Intersects(
				const OBB &box,
				Scalar thickness=SMALL
			) const;

		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Plane& plane
				);
		#endif
		void
			TestInstance() const
				{}
		static bool
			TestClass();
			
		//
		// Equation solutions
		//
		Scalar
			CalculateX(Scalar y, Scalar z)
				{
					Check_Object(this); Verify(!Small_Enough(normal.x));
					Scalar result = (offset - y*normal.y - z*normal.z) / normal.x;
					return result;
				}
		Scalar
			CalculateY(Scalar x, Scalar z)
				{
					Check_Object(this); Verify(!Small_Enough(normal.y));
					Scalar result = (offset - x*normal.x - z*normal.z) / normal.y;
					return result;
				}
		Scalar
			CalculateZ(Scalar x, Scalar y)
				{
					Check_Object(this); Verify(!Small_Enough(normal.z));
					Scalar result = (offset - x*normal.x - y*normal.y) / normal.z;
					return result;
				}

		//
		// Reflection code
		//
		void
			Reflect(Vector3D *vector)
				{
					Check_Object(this);
					vector->AddScaled(
						*vector,
						normal,
						-2.0f * ((*vector) * normal)
					);
				}
		bool
			ComputeBestDividingPlane(
				Stuff::DynamicArrayOf<Stuff::Point3D> &points
			);
	};

}
