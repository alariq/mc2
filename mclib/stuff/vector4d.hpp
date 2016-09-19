//===========================================================================//
// File:	vector4d.hh                                                      //
// Contents: Interface specification for vector classes                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"point3d.hpp"

namespace Stuff {class Vector4D;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Vector4D& vector
		);
#endif

namespace Stuff {

	class AffineMatrix4D;
	class Matrix4D;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Vector4D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Vector4D
	{
	 public:
		Scalar
			x,
			y,
			z,
			w;

		static const Vector4D
			Identity;

		//
		// Constructors
		//
		Vector4D()
			{}
		Vector4D(
			Scalar X,
			Scalar Y,
			Scalar Z,
			Scalar W)
				{x=X; y=Y; z=Z; w=W;}
		Vector4D(const Vector4D &v)
			{x=v.x; y=v.y; z=v.z; w=v.w;}
		explicit Vector4D(const Vector3D &v)
			{*this = v;}
		explicit Vector4D(const Point3D &p)
			{*this = p;}

		//
		// Assignment operators
		//
		Vector4D&
			operator=(const Vector4D &v)
				{
					Check_Pointer(this); Check_Object(&v);
					x = v.x; y = v.y; z = v.z; w = v.w; return *this;
				}
		Vector4D&
			operator=(const Vector3D &v)
				{
					Check_Pointer(this); Check_Object(&v);
					x = v.x; y = v.y; z = v.z; w = 0.0f; return *this;
				}
		Vector4D&
			operator=(const Point3D &p)
				{
					Check_Pointer(this); Check_Object(&p);
					x = p.x; y = p.y; z = p.z; w = 1.0f; return *this;
				}

		//
		// Index operators
		//
		const Scalar&
			operator[](size_t index) const
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(index) <= W_Axis);
					return (&x)[index];
				}
		Scalar&
			operator[](size_t index)
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(index) <= W_Axis);
					return (&x)[index];
				}

		friend bool
			Small_Enough(const Vector4D &v,Scalar e=SMALL);
		bool
			operator!() const
				{return Small_Enough(*this);}

		//
		// "Close-enough" comparison operators
		//
		friend bool
			Close_Enough(
				const Vector4D &v1,
				const Vector4D &v2,
				Scalar e=SMALL
			);
		bool
			operator==(const Vector4D& v) const
				{return Close_Enough(*this,v);}
		bool
			operator!=(const Vector4D& v) const
				{return !Close_Enough(*this,v);}

		//
		// The following operators all assume that this points to the destination
		// of the operation results
		//
		Vector4D&
			Negate(const Vector4D &v)
				{
					Check_Pointer(this); Check_Object(&v);
					x = -v.x; y = -v.y; z = -v.z; w = -v.w; return *this;
				}

		Vector4D&
			Add(
				const Vector4D& v1,
				const Vector4D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x + v2.x; y = v1.y + v2.y; z = v1.z + v2.z;
					w = v1.w + v2.w; return *this;
				}
		Vector4D&
			operator+=(const Vector4D& v)
				{return Add(*this,v);}

		Vector4D&
			Subtract(
				const Vector4D& v1,
				const Vector4D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x - v2.x; y = v1.y - v2.y; z = v1.z - v2.z;
					w = v1.w - v2.w; return *this;
				}
		Vector4D&
			operator-=(const Vector4D& v)
				{return Subtract(*this,v);}

		Scalar
			operator*(const Vector4D& v) const
				{Check_Object(this); return x*v.x + y*v.y + z*v.z + w*v.w;}

		Vector4D&
			Multiply(
				const Vector4D& v,
				Scalar scale
			)
				{
					Check_Pointer(this); Check_Object(&v);
					x = v.x * scale; y = v.y * scale; z = v.z * scale;
					w = v.w * scale; return *this;
				}
		Vector4D&
			operator*=(Scalar v)
				{return Multiply(*this,v);}

		Vector4D&
			Multiply(
				const Vector4D& v1,
				const Vector4D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x * v2.x; y = v1.y * v2.y; z = v1.z * v2.z;
					w = v1.w * v2.w; return *this;
				}
		Vector4D&
			operator*=(const Vector4D &v)
				{return Multiply(*this,v);}

		Vector4D&
			Divide(
				const Vector4D& v,
				Scalar scale
			)
				{
					Check_Pointer(this); Check_Object(&v);
					Verify(!Small_Enough(scale)); scale = 1.0f / scale;
					x = v.x * scale; y = v.y * scale; z = v.z * scale;
					w = v.w * scale; return *this;
				}
		Vector4D&
			operator/=(Scalar v)
				{return Divide(*this,v);}

		Vector4D&
			Divide(
				const Vector4D& v1,
				const Vector4D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					Verify(!Small_Enough(v1.x)); Verify(!Small_Enough(v1.y));
					Verify(!Small_Enough(v1.z)); Verify(!Small_Enough(v1.w));
					x = v1.x / v2.x; y = v1.y / v2.y; z = v1.z / v2.z; 
					w = v1.w / v2.w; return *this;
				}
		Vector4D&
			operator/=(const Vector4D &v)
				{return Divide(*this,v);}

		//
		//  Transforms
		//
		Vector4D&
			Multiply(
				const Vector4D &v,
				const AffineMatrix4D &m
			);
		Vector4D&
			operator*=(const AffineMatrix4D &M)
				{Vector4D src(*this); return Multiply(src,M);}
		Vector4D& Multiply(
			const Vector4D &v,
			const Matrix4D &m
		);
		Vector4D&
			operator*=(const Matrix4D &m)
				{Vector4D src(*this); return Multiply(src,m);}
		Vector4D& Multiply(
			const Vector3D &v,
			const Matrix4D &m
		);

		Vector4D& Multiply(
			const Point3D &v,
			const Matrix4D &m
		)
			{
				Check_Pointer(this);
				Check_Object(&v);
				Check_Object(&m);

#if USE_ASSEMBLER_CODE
				Scalar *f = &x;
				_asm {
		mov         edx, m
		
		mov         eax, v

		fld			dword ptr [eax]			//	v.x
		fld			dword ptr [eax+4]		//	v.y
		fld			dword ptr [eax+8]		//	v.z

		mov         eax, f

		fld         dword ptr [edx+34h]		//	m[1][3]
		fmul        st, st(2)				//	v.y
		
		fld         dword ptr [edx+38h]		//	m[2][3]
		fmul        st, st(2)				//	v.z

		fxch		st(1)
		fadd        dword ptr [edx+3Ch]		//	m[3][3]
		
		fld         dword ptr [edx+30h]		//	m[0][3]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+14h]		//	m[1][1]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+18h]		//	m[2][1]
		fmul        st, st(3)				//	v.z
		
		fxch		st(1)
		fstp        dword ptr [eax+0Ch]		//	w

		fadd        dword ptr [edx+1Ch]		//	m[3][1]

		fld         dword ptr [edx+10h]		//	m[0][1]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+24h]		//	m[1][2]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+28h]		//	m[2][2]
		fmul        st, st(3)				//	v.z
		
		fxch		st(1)
		fstp        dword ptr [eax+4]		//	y

		fadd        dword ptr [edx+2Ch]		//	m[3][2]
		
		fld         dword ptr [edx+20h]		//	m[0][2]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+4]		//	m[1][0]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+8]		//	m[2][0]
		fmul        st, st(3)				//	v.z

		fxch		st(1)
		fstp        dword ptr [eax+8]		//	z

		fadd        dword ptr [edx+0Ch]		//	m[3][0]
		
		fld         dword ptr [edx]			//	m[0][0]
		fmul        st, st(5)				//	v.x
		
		fxch		st(2)
		faddp       st(1),st

		faddp       st(1),st

//	get rid of x, y, z
		fstp		st(1)
		fstp		st(1)
		fstp		st(1)

		fstp        dword ptr [eax]			//	x

	}
#else
				x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0) + m(3,0);
				y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1) + m(3,1);
				z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2) + m(3,2);
				w = v.x*m(0,3) + v.y*m(1,3) + v.z*m(2,3) + m(3,3);
#endif
				return *this;
			}

		Vector4D& MultiplySetClip(
			const Point3D &v,
			const Matrix4D &m,
			int *clipper
		);
/*
			{
				Check_Pointer(this);
				Check_Object(&v);
				Check_Object(&m);

#if USE_ASSEMBLER_CODE
				Scalar *f = &x;
				_asm {
		mov         edx, m
		
		mov         eax, v

		fld			dword ptr [eax]			//	v.x
		fld			dword ptr [eax+4]		//	v.y
		fld			dword ptr [eax+8]		//	v.z

		mov         eax, f

		fld         dword ptr [edx+34h]		//	m[1][3]
		fmul        st, st(2)				//	v.y
		
		fld         dword ptr [edx+38h]		//	m[2][3]
		fmul        st, st(2)				//	v.z

		fxch		st(1)
		fadd        dword ptr [edx+3Ch]		//	m[3][3]
		
		fld         dword ptr [edx+30h]		//	m[0][3]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+14h]		//	m[1][1]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+18h]		//	m[2][1]
		fmul        st, st(3)				//	v.z
		
		fxch		st(1)
		fstp        dword ptr [eax+0Ch]		//	w

		fadd        dword ptr [edx+1Ch]		//	m[3][1]

		fld         dword ptr [edx+10h]		//	m[0][1]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+24h]		//	m[1][2]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+28h]		//	m[2][2]
		fmul        st, st(3)				//	v.z
		
		fxch		st(1)
		fstp        dword ptr [eax+4]		//	y

		fadd        dword ptr [edx+2Ch]		//	m[3][2]
		
		fld         dword ptr [edx+20h]		//	m[0][2]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+4]		//	m[1][0]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+8]		//	m[2][0]
		fmul        st, st(3)				//	v.z

		fxch		st(1)
		fstp        dword ptr [eax+8]		//	z

		fadd        dword ptr [edx+0Ch]		//	m[3][0]
		
		fld         dword ptr [edx]			//	m[0][0]
		fmul        st, st(5)				//	v.x
		
		fxch		st(2)
		faddp       st(1),st

		faddp       st(1),st

//	get rid of x, y, z
		fstp		st(1)
		fstp		st(1)
		fstp		st(1)

		fstp        dword ptr [eax]			//	x

	}
#else
				x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0) + m(3,0);
				y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1) + m(3,1);
				z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2) + m(3,2);
				w = v.x*m(0,3) + v.y*m(1,3) + v.z*m(2,3) + m(3,3);
#endif

				clipper = 0;

				if(w <= z)
				{
					clipper |= 32;
				}

				if(z < 0.0f)
				{
					clipper |= 16;
				}

				if(x < 0.0f)
				{
					clipper |= 8;
				}

				if(w < x)
				{
					clipper |= 4;
				}

				if(y < 0.0f)
				{
					clipper |= 2;
				}

				if(w < y)
				{
					clipper |= 1;
				}

				return *this;
			}
*/
		//
		// Support functions
		//
		Scalar
			GetLengthSquared() const
				{return operator*(*this);}
		Scalar
			GetLength() const
				{return Sqrt(GetLengthSquared());}

		Scalar
			GetApproximateLength() const
				{return SqrtApproximate(GetLengthSquared());}

		Vector4D&
			Combine(
				const Vector4D& v1,
				Scalar t1,
				const Vector4D& v2,
				Scalar t2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x*t1 + v2.x*t2; y = v1.y*t1 + v2.y*t2;
					z = v1.z*t1 + v2.z*t2; w = v1.w*t1 + v2.w*t2;
					return *this;
				}

		Vector4D&
			Lerp(
				const Vector4D& v1,
				const Vector4D& v2,
				Scalar t
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x + t*(v2.x-v1.x); y = v1.y + t*(v2.y-v1.y);
					z = v1.z + t*(v2.z-v1.z); w = v1.w + t*(v2.w-v1.w);
					return *this;
				}

		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Vector4D& vector
				);
		#endif
		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

}
