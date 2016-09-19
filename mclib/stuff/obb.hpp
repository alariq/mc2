//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"linearmatrix.hpp"
#include"sphere.hpp"

namespace Stuff {class OBB;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::OBB &box
		);
#endif

namespace Stuff {

	class Line3D;

	class OBB
	{
	public:
		void
			TestInstance() const;

		static OBB
			Identity;

		LinearMatrix4D
			localToParent;
		Vector3D
			axisExtents;
		Scalar
			sphereRadius;

		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const OBB &box
				);
		#endif

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Construction
	//
	public:
		OBB()
			{}
		OBB(const OBB& obb):
			localToParent(obb.localToParent),
			axisExtents(obb.axisExtents),
			sphereRadius(obb.sphereRadius)
				{}
		OBB(
			const LinearMatrix4D &origin,
			const Vector3D &extents
		):
			localToParent(origin),
			axisExtents(extents),
			sphereRadius(extents.GetLength())
				{}
		OBB(
			const LinearMatrix4D &origin,
			const Vector3D &extents,
			Scalar radius
		):
			localToParent(origin),
			axisExtents(extents),
			sphereRadius(radius)
				{}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assignment
	//
	public:
		OBB&
			operator =(const OBB& obb)
				{
					Check_Pointer(this); Check_Object(&obb);
					localToParent = obb.localToParent;
					axisExtents = obb.axisExtents;
					sphereRadius = obb.sphereRadius;
					return *this;
				}

		OBB&
			BuildSphere(const Sphere& sphere)
				{
					Check_Pointer(this); Check_Object(&sphere);
					sphereRadius = sphere.radius;
					localToParent.BuildTranslation(sphere.center);
					return *this;
				}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Transforms
	//
	public:
		OBB&
			Multiply(
				const OBB& obb,
				const LinearMatrix4D &matrix
			);
		inline OBB&
			MultiplySphereOnly(
				const OBB& obb,
				const LinearMatrix4D &matrix
			)
			{
				Check_Pointer(this);
				Check_Object(&obb);
				Check_Object(&matrix);

#if USE_ASSEMBLER_CODE

				Scalar *f = localToParent.entries;

				_asm {
					mov         edx, matrix
					push        esi
					mov         esi, obb.localToParent
					
					mov         eax, f

					fld         dword ptr [edx+4]		//	m[1][0]
					fmul        dword ptr [esi+01ch]	//	obb.localToParent(3,1)
					
					fld         dword ptr [edx+8]		//	m[2][0]
					fmul        dword ptr [esi+02Ch]	//	obb.localToParent(3,2)

					fxch		st(1)
					fadd        dword ptr [edx+0Ch]		//	m[3][0]
					
					fld         dword ptr [edx]			//	m[0][0]
					fmul        dword ptr [esi+0Ch]		//	obb.localToParent(3,0)

					fxch		st(2)
					faddp       st(1),st

					fld         dword ptr [edx+14h]		//	m[1][1]
					fmul        dword ptr [esi+01ch]	//	obb.localToParent(3,1)

					fxch		st(2)
					faddp       st(1),st

					fld         dword ptr [edx+18h]		//	m[2][1]
					fmul        dword ptr [esi+02ch]	//	obb.localToParent(3,2)
					
					fxch		st(1)
					fstp        dword ptr [eax+0ch]		//	localToParent(3,0)

					fadd        dword ptr [edx+1Ch]		//	m[3][1]

					fld         dword ptr [edx+10h]		//	m[0][1]
					fmul        dword ptr [esi+0ch]		//	obb.localToParent(3,0)

					fxch		st(2)
					faddp       st(1),st

					fld         dword ptr [edx+24h]		//	m[1][2]
					fmul        dword ptr [esi+01ch]	//	obb.localToParent(3,1)

					fxch		st(2)
					faddp       st(1),st

					fld         dword ptr [edx+28h]		//	m[2][2]
					fmul        dword ptr [esi+02ch]	//	obb.localToParent(3,2)
					
					fxch		st(1)
					fstp        dword ptr [eax+01ch]	//	localToParent(3,1)

					fadd        dword ptr [edx+2Ch]		//	m[3][2]
					
					fld         dword ptr [edx+20h]		//	m[0][2]
					fmul        dword ptr [esi+0ch]		//	obb.localToParent(3,0)

					fxch		st(2)
					faddp       st(1),st

					pop         esi

					faddp       st(1),st

					fstp        dword ptr [eax+02ch]	//	localToParent(3,2)
				}
#else
				localToParent(3,0) =
					obb.localToParent(3,0)*matrix(0,0)
					 + obb.localToParent(3,1)*matrix(1,0)
					 + obb.localToParent(3,2)*matrix(2,0)
					 + matrix(3,0);
				localToParent(3,1) =
					obb.localToParent(3,0)*matrix(0,1)
					 + obb.localToParent(3,1)*matrix(1,1)
					 + obb.localToParent(3,2)*matrix(2,1)
					 + matrix(3,1);
				localToParent(3,2) =
					obb.localToParent(3,0)*matrix(0,2)
					 + obb.localToParent(3,1)*matrix(1,2)
					 + obb.localToParent(3,2)*matrix(2,2)
					 + matrix(3,2);
#endif

				sphereRadius = obb.sphereRadius;
				return *this;
			}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Intersection functions
	//
	public:
		enum SeparatingAxis {
			NoSeparation = 0,
			A0,
			A1,
			A2,
			B0,
			B1,
			B2,
			A0xB0,
			A0xB1,
			A0xB2,
			A1xB0,
			A1xB1,
			A1xB2,
			A2xB0,
			A2xB1,
			A2xB2
		};

		SeparatingAxis
			FindSeparatingAxis(const OBB& box) const;
		//
		// Intersection functions
		//
		bool
			Contains(const Point3D &point) const;
		bool
			Intersects(const Plane &plane) const;

		void
			Union(
				const OBB &first,
				const OBB &second
			);
	};

	inline Sphere&
		Sphere::operator =(const OBB &obb)
	{
		Check_Pointer(this); Check_Object(&obb);
		center = obb.localToParent;
		radius = obb.sphereRadius;
		return *this;
	}

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::OBB *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::OBB *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
