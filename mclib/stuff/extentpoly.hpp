//===========================================================================//
// File:	ExtentPoly.hpp                                                   //
// Contents:																 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#if !defined(STUFF_EXTENTPOLY_HPP)
	#define STUFF_EXTENTPOLY_HPP

	#if !defined(STUFF_STUFF_HPP)
#include<stuff/stuff.hpp>
	#endif

	#if !defined(STUFF_SCALAR_HPP)
#include<stuff/scalar.hpp>
	#endif

	#if !defined(STUFF_MARRAY_HPP)
#include<stuff/marray.hpp>
	#endif

	#if !defined(STUFF_PLANE_HPP)
#include<stuff/plane.hpp>
	#endif

namespace Stuff {

	class Vector3D;
	class Point3D;
	class NotationFile;

	//~~~~~~~~~~~~~~~~~~~~~~~~~ ExtentPoly ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class ExtentPoly
	{
	 public:
		Scalar
			minY,
			maxY;
			int numberOfVertices;
			Vector3D *vertex;

		ExtentPoly() {}
		ExtentPoly(MemoryStream *stream);
		ExtentPoly(FileStream *file_stream,	char *page_name);
		~ExtentPoly(void);
		void
			Save(MemoryStream *stream);

		ExtentPoly&
			Intersect(
				const ExtentPoly &poly_1,
				const ExtentPoly &poly_2
			);
		ExtentPoly&
			Union(
				const ExtentPoly &poly_1,
				const ExtentPoly &poly_2
			);
		ExtentPoly&
			Union(
				const ExtentPoly &poly_1,
				const Vector3D &point
			);

		Vector3D*
			ClosestPointOnLine(
			Vector3D *point,
			const Vector3D &p1,
			const Vector3D &p2
			);

		Scalar
			DistanceBetweenLineAndPoint(
			const Vector3D &point,
			const Vector3D &p1,
			const Vector3D &p2
			);
		Vector3D*
			Constrain(Vector3D *point);
		int
			InfiniteLineTestWithXAxis(
				const Vector3D &point,
				const Vector3D &p1,
				const Vector3D &p2
				);
		bool
			Contains(const Vector3D &point);
		void
			TestInstance() const;
		static bool
			TestClass();
	};

}

#endif
