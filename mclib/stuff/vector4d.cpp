//===========================================================================//
// File:	vector4d.cc                                                      //
// Contents: Implementation details for vector classes                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Vector4D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const Vector4D
	Vector4D::Identity(0.0f,0.0f,0.0f,0.0f);

//
//###########################################################################
//###########################################################################
//
bool
	Stuff::Small_Enough(const Vector4D &V,Scalar e)
{
	Check_Object(&V);
	return V.x*V.x + V.y*V.y + V.z*V.z + V.w*V.w <= e;
}

//
//###########################################################################
//###########################################################################
//
bool
	Stuff::Close_Enough(const Vector4D &V1, const Vector4D &V2, Scalar e)
{
	Check_Object(&V1);
	Check_Object(&V2);

	Vector4D v(V1.x-V2.x, V1.y-V2.y, V1.z-V2.z, V1.w-V2.w);
	return Stuff::Small_Enough(v, e);
}

//
//###########################################################################
//###########################################################################
//
Vector4D&
	Vector4D::Multiply(
		const Vector4D& v,
		const AffineMatrix4D& m
	)
{
	Check_Pointer(this);
	Check_Object(&v);
	Check_Object(&m);
	Verify(this != &v);

	x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0) + v.w*m(3,0);
	y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1) + v.w*m(3,1);
	z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2) + v.w*m(3,2);
	w = v.w;
	return *this;
}

//
//###########################################################################
//###########################################################################
//
Vector4D&
	Vector4D::Multiply(
		const Vector4D& v,
		const Matrix4D& m
	)
{
	Check_Pointer(this);
	Check_Object(&v);
	Check_Object(&m);
	Verify(this != &v);

	x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0) + v.w*m(3,0);
	y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1) + v.w*m(3,1);
	z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2) + v.w*m(3,2);
	w = v.x*m(0,3) + v.y*m(1,3) + v.z*m(2,3) + v.w*m(3,3);
	return *this;
}

//
//###########################################################################
//###########################################################################
//
Vector4D&
	Vector4D::Multiply(
		const Vector3D& v,
		const Matrix4D& m
	)
{
	Check_Pointer(this);
	Check_Object(&v);
	Check_Object(&m);

	x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0);
	y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1);
	z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2);
	w = v.x*m(0,3) + v.y*m(1,3) + v.z*m(2,3);
	return *this;
}

#if 0
// it is now inline likely this has to happen to most of the other functions
//###########################################################################
//###########################################################################
//
Vector4D&
	Vector4D::Multiply(
		const Point3D& v,
		const Matrix4D& m
	)
{
	Check_Pointer(this);
	Check_Object(&v);
	Check_Object(&m);

	x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0) + m(3,0);
	y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1) + m(3,1);
	z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2) + m(3,2);
	w = v.x*m(0,3) + v.y*m(1,3) + v.z*m(2,3) + m(3,3);
	return *this;
}
#endif

//
//###########################################################################
//###########################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Vector4D &vector
		)
	{
		Check_Object(&vector);
		SPEW((group, "<%4f,%4f,%4f,%4f>", vector.x, vector.y, vector.z, vector.w));
	}
#endif

Vector4D&
	Vector4D::MultiplySetClip(
		const Point3D &v,
		const Matrix4D &m,
		int *clipper
	)
{
	Check_Pointer(this);
	Check_Object(&v);
	Check_Object(&m);

#if USE_ASSEMBLER_CODE
	Scalar *f = &x;
	_asm {
		mov         edi, v

		fld			dword ptr [edi]			//	v.x

		mov         esi, m

		fld			dword ptr [edi+4]		//	v.y
		fld			dword ptr [edi+8]		//	v.z

		mov         edi, f

		fld         dword ptr [esi+34h]		//	m[1][3]
		fmul        st, st(2)				//	v.y

		fld         dword ptr [esi+38h]		//	m[2][3]
		fmul        st, st(2)				//	v.z

		fxch		st(1)
		fadd        dword ptr [esi+3Ch]		//	m[3][3]

		fld         dword ptr [esi+30h]		//	m[0][3]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [esi+14h]		//	m[1][1]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [esi+18h]		//	m[2][1]
		fmul        st, st(3)				//	v.z

		fxch		st(1)
		fstp        dword ptr [edi+0Ch]		//	w

		fadd        dword ptr [esi+1Ch]		//	m[3][1]

		fld         dword ptr [esi+10h]		//	m[0][1]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [esi+24h]		//	m[1][2]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [esi+28h]		//	m[2][2]
		fmul        st, st(3)				//	v.z

		fxch		st(1)
		fstp        dword ptr [edi+4]		//	y

		fadd        dword ptr [esi+2Ch]		//	m[3][2]

		fld         dword ptr [esi+20h]		//	m[0][2]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [esi+4]		//	m[1][0]
		fmul        st, st(4)				//	v.y

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [esi+8]		//	m[2][0]
		fmul        st, st(3)				//	v.z

		fxch		st(1)
		fstp        dword ptr [edi+8]		//	z

		fadd        dword ptr [esi+0Ch]		//	m[3][0]

		fld         dword ptr [esi]			//	m[0][0]
		fmul        st, st(5)				//	v.x

		fxch		st(2)
		faddp       st(1),st

		faddp       st(1),st

		//	get rid of x, y, z
		fstp		st(1)
		fstp		st(1)
		fstp		st(1)

		fstp        dword ptr [edi]			//	x

	}
#else
	x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0) + m(3,0);
	y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1) + m(3,1);
	z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2) + m(3,2);
	w = v.x*m(0,3) + v.y*m(1,3) + v.z*m(2,3) + m(3,3);
#endif

	*clipper = 0;

	if(w <= z)
	{
		*clipper |= 32;
	}

	if(z < 0.0f)
	{
		*clipper |= 16;
	}

	if(x < 0.0f)
	{
		*clipper |= 8;
	}

	if(w < x)
	{
		*clipper |= 4;
	}

	if(y < 0.0f)
	{
		*clipper |= 2;
	}

	if(w < y)
	{
		*clipper |= 1;
	}

	return *this;
}
