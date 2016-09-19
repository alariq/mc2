//===========================================================================//
// File:	matrix.cc                                                        //
// Contents: Implementation details for the matrix class                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

const Matrix4D
	Matrix4D::Identity(true);

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::BuildIdentity()
{
	(*this)(0,0) = 1.0f;
	(*this)(1,0) = 0.0f;
	(*this)(2,0) = 0.0f;
	(*this)(3,0) = 0.0f;

	(*this)(0,1) = 0.0f;
	(*this)(1,1) = 1.0f;
	(*this)(2,1) = 0.0f;
	(*this)(3,1) = 0.0f;

	(*this)(0,2) = 0.0f;
	(*this)(1,2) = 0.0f;
	(*this)(2,2) = 1.0f;
	(*this)(3,2) = 0.0f;

	(*this)(0,3) = 0.0f;
	(*this)(1,3) = 0.0f;
	(*this)(2,3) = 0.0f;
	(*this)(3,3) = 1.0f;

	return *this;
}

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::operator=(const AffineMatrix4D &m)
{
	Check_Pointer(this);
	Check_Object(&m);

	Mem_Copy(entries, m.entries, sizeof(m.entries), sizeof(entries));
	(*this)(0,3) = 0.0f;
	(*this)(1,3) = 0.0f;
	(*this)(2,3) = 0.0f;
	(*this)(3,3) = 1.0f;
	return *this;
}

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::operator=(const Origin3D& p)
{
	Check_Pointer(this);
	Check_Object(&p);

	BuildRotation(p.angularPosition);
	BuildTranslation(p.linearPosition);

	(*this)(0,3) = 0.0f;
	(*this)(1,3) = 0.0f;
	(*this)(2,3) = 0.0f;
	(*this)(3,3) = 1.0f;

	return *this;
}

//
//#############################################################################
//#############################################################################
//
Matrix4D&
	Matrix4D::BuildRotation(const EulerAngles &angles)
{
	Check_Pointer(this);
	Check_Object(&angles);

	SinCosPair
		x,
		y,
		z;

	x = angles.pitch;
	y = angles.yaw;
	z = angles.roll;

	(*this)(0,0) = y.cosine*z.cosine;
	(*this)(0,1) = y.cosine*z.sine;
	(*this)(0,2) = -y.sine;

	(*this)(1,0) = x.sine*y.sine*z.cosine - x.cosine*z.sine;
	(*this)(1,1) = x.sine*y.sine*z.sine + x.cosine*z.cosine;
	(*this)(1,2) = x.sine*y.cosine;

	(*this)(2,0) = x.cosine*y.sine*z.cosine + x.sine*z.sine;
	(*this)(2,1) = x.cosine*y.sine*z.sine - x.sine*z.cosine;
	(*this)(2,2) = x.cosine*y.cosine;

	Check_Object(this);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
Matrix4D&
	Matrix4D::operator=(const EulerAngles &angles)
{
	Check_Pointer(this);
	Check_Object(&angles);

	(*this)(3,0) = 0.0f;
	(*this)(3,1) = 0.0f;
	(*this)(3,2) = 0.0f;

	(*this)(0,3) = 0.0f;
	(*this)(1,3) = 0.0f;
	(*this)(2,3) = 0.0f;
	(*this)(3,3) = 1.0f;

	return BuildRotation(angles);
}

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::BuildRotation(const UnitQuaternion &q)
{
	Check_Pointer(this);
	Check_Object(&q);

	Scalar
		a = q.x*q.y,
		b = q.y*q.z,
		c = q.z*q.x,
		d = q.w*q.x,
		e = q.w*q.y,
		f = q.w*q.z,
		g = q.w*q.w,
		h = q.x*q.x,
		i = q.y*q.y,
		j = q.z*q.z;

	(*this)(0,0) = g + h - i - j;
	(*this)(1,0) = 2.0f*(a - f);
	(*this)(2,0) = 2.0f*(c + e);

	(*this)(0,1) = 2.0f*(f + a);
	(*this)(1,1) = g - h + i - j;
	(*this)(2,1) = 2.0f*(b - d);

	(*this)(0,2) = 2.0f*(c - e);
	(*this)(1,2) = 2.0f*(b + d);
	(*this)(2,2) = g - h - i + j;

	return *this;
}

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::operator=(const Point3D& p)
{
	Check_Pointer(this);
	Check_Object(&p);

	(*this)(0,0) = 1.0f;
	(*this)(1,0) = 0.0f;
	(*this)(2,0) = 0.0f;
	(*this)(3,0) = p.x;

	(*this)(0,1) = 0.0f;
	(*this)(1,1) = 1.0f;
	(*this)(2,1) = 0.0f;
	(*this)(3,1) = p.y;

	(*this)(0,2) = 0.0f;
	(*this)(1,2) = 0.0f;
	(*this)(2,2) = 1.0f;
	(*this)(3,2) = p.z;

	(*this)(0,3) = 0.0f;
	(*this)(1,3) = 0.0f;
	(*this)(2,3) = 0.0f;
	(*this)(3,3) = 1.0f;

	return *this;
}

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::BuildTranslation(const Point3D& p)
{
	Check_Pointer(this);
	Check_Object(&p);

	(*this)(3,0) = p.x;
	(*this)(3,1) = p.y;
	(*this)(3,2) = p.z;
	return *this;
}

//
//###########################################################################
//###########################################################################
//
bool
	Stuff::Close_Enough(
		const Matrix4D &m1,
		const Matrix4D &m2,
		Scalar e
	)
{
	Check_Object(&m2);
	Check_Object(&m1);

	for (size_t i=0; i<ELEMENTS(m1.entries); ++i)
	{
		if (!Close_Enough(m1.entries[i], m2.entries[i], e))
		{
			return false;
		}
	}
	return true;
}

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::Multiply(
		const Matrix4D &Source1,
		const Matrix4D &Source2
	)
{
	(*this)(0,0) =
		Source1(0,0)*Source2(0,0)
		 + Source1(0,1)*Source2(1,0)
		 + Source1(0,2)*Source2(2,0)
		 + Source1(0,3)*Source2(3,0);
	(*this)(1,0) =
		Source1(1,0)*Source2(0,0)
		 + Source1(1,1)*Source2(1,0)
		 + Source1(1,2)*Source2(2,0)
		 + Source1(1,3)*Source2(3,0);
	(*this)(2,0) =
		Source1(2,0)*Source2(0,0)
		 + Source1(2,1)*Source2(1,0)
		 + Source1(2,2)*Source2(2,0)
		 + Source1(2,3)*Source2(3,0);
	(*this)(3,0) =
		Source1(3,0)*Source2(0,0)
		 + Source1(3,1)*Source2(1,0)
		 + Source1(3,2)*Source2(2,0)
		 + Source1(3,3)*Source2(3,0);

	(*this)(0,1) =
		Source1(0,0)*Source2(0,1)
		 + Source1(0,1)*Source2(1,1)
		 + Source1(0,2)*Source2(2,1)
		 + Source1(0,3)*Source2(3,1);
	(*this)(1,1) =
		Source1(1,0)*Source2(0,1)
		 + Source1(1,1)*Source2(1,1)
		 + Source1(1,2)*Source2(2,1)
		 + Source1(1,3)*Source2(3,1);
	(*this)(2,1) =
		Source1(2,0)*Source2(0,1)
		 + Source1(2,1)*Source2(1,1)
		 + Source1(2,2)*Source2(2,1)
		 + Source1(2,3)*Source2(3,1);
	(*this)(3,1) =
		Source1(3,0)*Source2(0,1)
		 + Source1(3,1)*Source2(1,1)
		 + Source1(3,2)*Source2(2,1)
		 + Source1(3,3)*Source2(3,1);

	(*this)(0,2) =
		Source1(0,0)*Source2(0,2)
		 + Source1(0,1)*Source2(1,2)
		 + Source1(0,2)*Source2(2,2)
		 + Source1(0,3)*Source2(3,2);
	(*this)(1,2) =
		Source1(1,0)*Source2(0,2)
		 + Source1(1,1)*Source2(1,2)
		 + Source1(1,2)*Source2(2,2)
		 + Source1(1,3)*Source2(3,2);
	(*this)(2,2) =
		Source1(2,0)*Source2(0,2)
		 + Source1(2,1)*Source2(1,2)
		 + Source1(2,2)*Source2(2,2)
		 + Source1(2,3)*Source2(3,2);
	(*this)(3,2) =
		Source1(3,0)*Source2(0,2)
		 + Source1(3,1)*Source2(1,2)
		 + Source1(3,2)*Source2(2,2)
		 + Source1(3,3)*Source2(3,2);

	(*this)(0,3) =
		Source1(0,0)*Source2(0,3)
		 + Source1(0,1)*Source2(1,3)
		 + Source1(0,2)*Source2(2,3)
		 + Source1(0,3)*Source2(3,3);
	(*this)(1,3) =
		Source1(1,0)*Source2(0,3)
		 + Source1(1,1)*Source2(1,3)
		 + Source1(1,2)*Source2(2,3)
		 + Source1(1,3)*Source2(3,3);
	(*this)(2,3) =
		Source1(2,0)*Source2(0,3)
		 + Source1(2,1)*Source2(1,3)
		 + Source1(2,2)*Source2(2,3)
		 + Source1(2,3)*Source2(3,3);
	(*this)(3,3) =
		Source1(3,0)*Source2(0,3)
		 + Source1(3,1)*Source2(1,3)
		 + Source1(3,2)*Source2(2,3)
		 + Source1(3,3)*Source2(3,3);

	return *this;
}

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::Multiply(
		const Matrix4D &Source1,
		const AffineMatrix4D &Source2
	)
{
	(*this)(0,0) =
		Source1(0,0)*Source2(0,0)
		 + Source1(0,1)*Source2(1,0)
		 + Source1(0,2)*Source2(2,0)
		 + Source1(0,3)*Source2(3,0);
	(*this)(1,0) =
		Source1(1,0)*Source2(0,0)
		 + Source1(1,1)*Source2(1,0)
		 + Source1(1,2)*Source2(2,0)
		 + Source1(1,3)*Source2(3,0);
	(*this)(2,0) =
		Source1(2,0)*Source2(0,0)
		 + Source1(2,1)*Source2(1,0)
		 + Source1(2,2)*Source2(2,0)
		 + Source1(2,3)*Source2(3,0);
	(*this)(3,0) =
		Source1(3,0)*Source2(0,0)
		 + Source1(3,1)*Source2(1,0)
		 + Source1(3,2)*Source2(2,0)
		 + Source1(3,3)*Source2(3,0);

	(*this)(0,1) =
		Source1(0,0)*Source2(0,1)
		 + Source1(0,1)*Source2(1,1)
		 + Source1(0,2)*Source2(2,1)
		 + Source1(0,3)*Source2(3,1);
	(*this)(1,1) =
		Source1(1,0)*Source2(0,1)
		 + Source1(1,1)*Source2(1,1)
		 + Source1(1,2)*Source2(2,1)
		 + Source1(1,3)*Source2(3,1);
	(*this)(2,1) =
		Source1(2,0)*Source2(0,1)
		 + Source1(2,1)*Source2(1,1)
		 + Source1(2,2)*Source2(2,1)
		 + Source1(2,3)*Source2(3,1);
	(*this)(3,1) =
		Source1(3,0)*Source2(0,1)
		 + Source1(3,1)*Source2(1,1)
		 + Source1(3,2)*Source2(2,1)
		 + Source1(3,3)*Source2(3,1);

	(*this)(0,2) =
		Source1(0,0)*Source2(0,2)
		 + Source1(0,1)*Source2(1,2)
		 + Source1(0,2)*Source2(2,2)
		 + Source1(0,3)*Source2(3,2);
	(*this)(1,2) =
		Source1(1,0)*Source2(0,2)
		 + Source1(1,1)*Source2(1,2)
		 + Source1(1,2)*Source2(2,2)
		 + Source1(1,3)*Source2(3,2);
	(*this)(2,2) =
		Source1(2,0)*Source2(0,2)
		 + Source1(2,1)*Source2(1,2)
		 + Source1(2,2)*Source2(2,2)
		 + Source1(2,3)*Source2(3,2);
	(*this)(3,2) =
		Source1(3,0)*Source2(0,2)
		 + Source1(3,1)*Source2(1,2)
		 + Source1(3,2)*Source2(2,2)
		 + Source1(3,3)*Source2(3,2);

	(*this)(0,3) = Source1(0,3);
	(*this)(1,3) = Source1(1,3);
	(*this)(2,3) = Source1(2,3);
	(*this)(3,3) = Source1(3,3);

	return *this;
}

#if 0 // moved into the hpp-file
//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::Multiply(
		const AffineMatrix4D &Source1,
		const Matrix4D &Source2
	)
{
	Check_Pointer(this);
	Check_Object(&Source1);
	Check_Object(&Source2);

#if USE_ASSEMBLER_CODE
				Scalar *f = entries;
				_asm {
					mov         edx, Source1.entries
					push        esi
					mov         esi, Source2.entries
					
					mov         eax, f

					fld         dword ptr [edx]			//	s1[0][0]
					fmul        dword ptr [esi]			//	s2[0][0]	M0,1

					fld         dword ptr [edx+010h]	//	s1[0][1]
					fmul        dword ptr [esi+4]		//	s2[1][0]	M0,2
					
					fld         dword ptr [edx+020h]	//	s1[0][2]
					fmul        dword ptr [esi+8]		//	s2[2][0]	M0,3

					fxch		st(2)
					faddp       st(1),st				//				A0,1

					fld         dword ptr [edx+4]		//	s1[1][0]
					fmul        dword ptr [esi]			//	s2[0][0]	M1,1

					fxch		st(2)
					faddp       st(1),st				//				A0,2

					fld         dword ptr [edx+14h]		//	s1[1][1]
					fmul        dword ptr [esi+4]		//	s2[1][0]	M1,2
					
					fxch		st(1)
					fstp        dword ptr [eax]			//	[0][0]		S0

					fld         dword ptr [edx+24h]		//	s1[1][2]
					fmul        dword ptr [esi+8]		//	s2[2][0]	M1,3

					fxch		st(2)
					faddp       st(1),st				//				A1,1

					fld         dword ptr [edx+8]		//	s1[2][0]
					fmul        dword ptr [esi]			//	s2[0][0]	M2,1

					fxch		st(2)
					faddp       st(1),st				//				A1,2

					fld         dword ptr [edx+018h]	//	s1[2][1]
					fmul        dword ptr [esi+4]		//	s2[1][0]	M2,2

					fxch		st(1)
					fstp        dword ptr [eax+4]		//	[1][0]		S1

					fld         dword ptr [edx+28h]		//	s1[2][2]
					fmul        dword ptr [esi+8]		//	s2[2][0]	M2,3
					
					fxch		st(2)
					faddp       st(1),st				//				A2,1

					fld         dword ptr [edx+0ch]		//	s1[3][0]
					fmul        dword ptr [esi]			//	s2[0][0]	M3,1

					fxch		st(2)
					faddp       st(1),st				//				A2,2

					fld         dword ptr [edx+1ch]		//	s1[3][1]
					fmul        dword ptr [esi+4]		//	s2[1][0]	M3,2

					fxch		st(1)
					fstp        dword ptr [eax+8]		//	[2][0]		S2

					fld         dword ptr [edx+2ch]		//	s1[3][2]
					fmul        dword ptr [esi+8]		//	s2[2][0]	M3,3

					fxch		st(2)
					faddp       st(1),st				//				A3,1

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					fld         dword ptr [edx]			//	s1[0][0]
					fmul        dword ptr [esi+010h]	//	s2[0][1]	M0,1

					fxch		st(2)
					faddp		st(1),st				//				A3,2

					fld         dword ptr [edx+010h]	//	s1[0][1]
					fmul        dword ptr [esi+014h]	//	s2[1][1]	M0,2

					fxch		st(1)
					fadd        dword ptr [esi+0Ch]		//	s2[3][0]	A3,3
					
					fld         dword ptr [edx+020h]	//	s1[0][2]
					fmul        dword ptr [esi+018h]	//	s2[2][1]	M0,3

					fxch		st(1)
					fstp        dword ptr [eax+0Ch]		//	[3][0]		S3

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

					fxch		st(2)
					faddp       st(1),st				//				A0,1

					fld         dword ptr [edx+4]		//	s1[1][0]
					fmul        dword ptr [esi+010h]	//	s2[0][1]	M1,1

					fxch		st(2)
					faddp       st(1),st				//				A0,2

					fld         dword ptr [edx+014h]	//	s1[1][1]
					fmul        dword ptr [esi+014h]	//	s2[1][1]	M1,2
					
					fxch		st(1)
					fstp        dword ptr [eax+010h]	//	[0][1]		S0

					fld         dword ptr [edx+024h]	//	s1[1][2]
					fmul        dword ptr [esi+018h]	//	s2[2][1]	M1,3

					fxch		st(2)
					faddp       st(1),st				//				A1,1

					fld         dword ptr [edx+8]		//	s1[2][0]
					fmul        dword ptr [esi+010h]	//	s2[0][1]	M2,1

					fxch		st(2)
					faddp       st(1),st				//				A1,2

					fld         dword ptr [edx+018h]	//	s1[2][1]
					fmul        dword ptr [esi+014h]	//	s2[1][1]	M2,2

					fxch		st(1)
					fstp        dword ptr [eax+014h]	//	[1][1]		S1

					fld         dword ptr [edx+028h]	//	s1[2][2]
					fmul        dword ptr [esi+018h]	//	s2[2][1]	M2,3
					
					fxch		st(2)
					faddp       st(1),st				//				A2,1

					fld         dword ptr [edx+0ch]		//	s1[3][0]
					fmul        dword ptr [esi+010h]	//	s2[0][1]	M3,1

					fxch		st(2)
					faddp       st(1),st				//				A2,2

					fld         dword ptr [edx+01ch]	//	s1[3][1]
					fmul        dword ptr [esi+014h]	//	s2[1][1]	M3,2

					fxch		st(1)
					fstp        dword ptr [eax+018h]	//	[2][1]		S2

					fld         dword ptr [edx+02ch]	//	s1[3][2]
					fmul        dword ptr [esi+018h]	//	s2[2][1]	M3,3

					fxch		st(2)
					faddp       st(1),st				//				A3,1

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					fld         dword ptr [edx]			//	s1[0][0]
					fmul        dword ptr [esi+020h]	//	s2[0][2]	M0,1

					fxch		st(2)
					faddp		st(1),st				//				A3,2

					fld         dword ptr [edx+010h]	//	s1[0][1]
					fmul        dword ptr [esi+024h]	//	s2[1][2]	M0,2

					fxch		st(1)
					fadd        dword ptr [esi+01Ch]	//	s2[3][1]	A3,3
					
					fld         dword ptr [edx+020h]	//	s1[0][2]
					fmul        dword ptr [esi+028h]	//	s2[2][2]	M0,3

					fxch		st(1)
					fstp        dword ptr [eax+01Ch]	//	[3][1]		S3

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

					fxch		st(2)
					faddp       st(1),st				//				A0,1

					fld         dword ptr [edx+4]		//	s1[1][0]
					fmul        dword ptr [esi+020h]	//	s2[0][2]	M1,1

					fxch		st(2)
					faddp       st(1),st				//				A0,2

					fld         dword ptr [edx+014h]	//	s1[1][1]
					fmul        dword ptr [esi+024h]	//	s2[1][2]	M1,2
					
					fxch		st(1)
					fstp        dword ptr [eax+020h]	//	[0][2]		S0

					fld         dword ptr [edx+024h]	//	s1[1][2]
					fmul        dword ptr [esi+028h]	//	s2[2][2]	M1,3

					fxch		st(2)
					faddp       st(1),st				//				A1,1

					fld         dword ptr [edx+8]		//	s1[2][0]
					fmul        dword ptr [esi+020h]	//	s2[0][2]	M2,1

					fxch		st(2)
					faddp       st(1),st				//				A1,2

					fld         dword ptr [edx+018h]	//	s1[2][1]
					fmul        dword ptr [esi+024h]	//	s2[1][2]	M2,2

					fxch		st(1)
					fstp        dword ptr [eax+024h]	//	[1][2]		S1

					fld         dword ptr [edx+028h]	//	s1[2][2]
					fmul        dword ptr [esi+028h]	//	s2[2][2]	M2,3
					
					fxch		st(2)
					faddp       st(1),st				//				A2,1

					fld         dword ptr [edx+0ch]		//	s1[3][0]
					fmul        dword ptr [esi+020h]	//	s2[0][2]	M3,1

					fxch		st(2)
					faddp       st(1),st				//				A2,2

					fld         dword ptr [edx+01ch]	//	s1[3][1]
					fmul        dword ptr [esi+024h]	//	s2[1][2]	M3,2

					fxch		st(1)
					fstp        dword ptr [eax+028h]	//	[2][2]		S2

					fld         dword ptr [edx+02ch]	//	s1[3][2]
					fmul        dword ptr [esi+028h]	//	s2[2][2]	M3,3

					fxch		st(2)
					faddp       st(1),st				//				A3,1

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					fld         dword ptr [edx]			//	s1[0][0]
					fmul        dword ptr [esi+030h]	//	s2[0][3]	M0,1

					fxch		st(2)
					faddp		st(1),st				//				A3,2

					fld         dword ptr [edx+010h]	//	s1[0][1]
					fmul        dword ptr [esi+034h]	//	s2[1][3]	M0,2

					fxch		st(1)
					fadd        dword ptr [esi+02Ch]	//	s2[3][2]	A3,3
					
					fld         dword ptr [edx+020h]	//	s1[0][2]
					fmul        dword ptr [esi+038h]	//	s2[2][3]	M0,3

					fxch		st(1)
					fstp        dword ptr [eax+02Ch]	//	[3][2]		S3

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

					fxch		st(2)
					faddp       st(1),st				//				A0,1

					fld         dword ptr [edx+4]		//	s1[1][0]
					fmul        dword ptr [esi+030h]	//	s2[0][3]	M1,1

					fxch		st(2)
					faddp       st(1),st				//				A0,2

					fld         dword ptr [edx+014h]	//	s1[1][1]
					fmul        dword ptr [esi+034h]	//	s2[1][3]	M1,2
					
					fxch		st(1)
					fstp        dword ptr [eax+030h]	//	[0][3]		S0

					fld         dword ptr [edx+024h]	//	s1[1][2]
					fmul        dword ptr [esi+038h]	//	s2[2][3]	M1,3

					fxch		st(2)
					faddp       st(1),st				//				A1,1

					fld         dword ptr [edx+8]		//	s1[2][0]
					fmul        dword ptr [esi+030h]	//	s2[0][3]	M2,1

					fxch		st(2)
					faddp       st(1),st				//				A1,2

					fld         dword ptr [edx+018h]	//	s1[2][1]
					fmul        dword ptr [esi+034h]	//	s2[1][3]	M2,2

					fxch		st(1)
					fstp        dword ptr [eax+034h]	//	[1][3]		S1

					fld         dword ptr [edx+028h]	//	s1[2][2]
					fmul        dword ptr [esi+038h]	//	s2[2][3]	M2,3
					
					fxch		st(2)
					faddp       st(1),st				//				A2,1

					fld         dword ptr [edx+0ch]		//	s1[3][0]
					fmul        dword ptr [esi+030h]	//	s2[0][3]	M3,1

					fxch		st(2)
					faddp       st(1),st				//				A2,2

					fld         dword ptr [edx+01ch]	//	s1[3][1]
					fmul        dword ptr [esi+034h]	//	s2[1][3]	M3,2

					fxch		st(1)
					fstp        dword ptr [eax+038h]	//	[2][3]		S2

					fld         dword ptr [edx+02ch]	//	s1[3][2]
					fmul        dword ptr [esi+038h]	//	s2[2][3]	M3,3

					fxch		st(2)
					faddp       st(1),st				//				A3,1

					faddp		st(1),st				//				A3,2
					fadd        dword ptr [esi+03Ch]	//	s2[3][3]	A3,3
					
					fstp        dword ptr [eax+03Ch]	//	[3][3]		S3

					pop         esi
				}

#else
	(*this)(0,0) =
		Source1(0,0)*Source2(0,0)
		 + Source1(0,1)*Source2(1,0)
		 + Source1(0,2)*Source2(2,0);
	(*this)(1,0) =
		Source1(1,0)*Source2(0,0)
		 + Source1(1,1)*Source2(1,0)
		 + Source1(1,2)*Source2(2,0);
	(*this)(2,0) =
		Source1(2,0)*Source2(0,0)
		 + Source1(2,1)*Source2(1,0)
		 + Source1(2,2)*Source2(2,0);
	(*this)(3,0) =
		Source1(3,0)*Source2(0,0)
		 + Source1(3,1)*Source2(1,0)
		 + Source1(3,2)*Source2(2,0)
		 + Source2(3,0);

	(*this)(0,1) =
		Source1(0,0)*Source2(0,1)
		 + Source1(0,1)*Source2(1,1)
		 + Source1(0,2)*Source2(2,1);
	(*this)(1,1) =
		Source1(1,0)*Source2(0,1)
		 + Source1(1,1)*Source2(1,1)
		 + Source1(1,2)*Source2(2,1);
	(*this)(2,1) =
		Source1(2,0)*Source2(0,1)
		 + Source1(2,1)*Source2(1,1)
		 + Source1(2,2)*Source2(2,1);
	(*this)(3,1) =
		Source1(3,0)*Source2(0,1)
		 + Source1(3,1)*Source2(1,1)
		 + Source1(3,2)*Source2(2,1)
		 + Source2(3,1);

	(*this)(0,2) =
		Source1(0,0)*Source2(0,2)
		 + Source1(0,1)*Source2(1,2)
		 + Source1(0,2)*Source2(2,2);
	(*this)(1,2) =
		Source1(1,0)*Source2(0,2)
		 + Source1(1,1)*Source2(1,2)
		 + Source1(1,2)*Source2(2,2);
	(*this)(2,2) =
		Source1(2,0)*Source2(0,2)
		 + Source1(2,1)*Source2(1,2)
		 + Source1(2,2)*Source2(2,2);
	(*this)(3,2) =
		Source1(3,0)*Source2(0,2)
		 + Source1(3,1)*Source2(1,2)
		 + Source1(3,2)*Source2(2,2)
		 + Source2(3,2);

	(*this)(0,3) =
		Source1(0,0)*Source2(0,3)
		 + Source1(0,1)*Source2(1,3)
		 + Source1(0,2)*Source2(2,3);
	(*this)(1,3) =
		Source1(1,0)*Source2(0,3)
		 + Source1(1,1)*Source2(1,3)
		 + Source1(1,2)*Source2(2,3);
	(*this)(2,3) =
		Source1(2,0)*Source2(0,3)
		 + Source1(2,1)*Source2(1,3)
		 + Source1(2,2)*Source2(2,3);
	(*this)(3,3) =
		Source1(3,0)*Source2(0,3)
		 + Source1(3,1)*Source2(1,3)
		 + Source1(3,2)*Source2(2,3)
		 + Source2(3,3);
#endif

	return *this;
}
#endif

//
//###########################################################################
//###########################################################################
//
Matrix4D&
	Matrix4D::Multiply(
		const AffineMatrix4D &m1,
		const AffineMatrix4D &m2
	)
{
	PAUSE(("Not safe"));
	Cast_Pointer(AffineMatrix4D*, this)->Multiply(m1,m2);
	(*this)(0,3) = 0.0f;
	(*this)(1,3) = 0.0f;
	(*this)(2,3) = 0.0f;
	(*this)(3,3) = 1.0f;
	return *this;
}

Matrix4D&
	Matrix4D::Invert(const Matrix4D& Source)
{
	float m3344S3443 = Source(2,2) * Source(3,3) - Source(2,3) * Source(3,2);
	float m3244S3442 = Source(2,1) * Source(3,3) - Source(2,3) * Source(3,1);
	float m3243S3342 = Source(2,1) * Source(3,2) - Source(2,2) * Source(3,1);
	
	float m3144S3441 = Source(2,0) * Source(3,3) - Source(2,3) * Source(3,0);
	float m3143S3341 = Source(2,0) * Source(3,2) - Source(2,2) * Source(3,0);
	
	float m3142S3241 = Source(2,0) * Source(3,1) - Source(2,1) * Source(3,0);
	
	float m2344S2443 = Source(1,2) * Source(3,3) - Source(1,3) * Source(3,2);
	float m2244S2442 = Source(1,1) * Source(3,3) - Source(1,3) * Source(3,1);
	float m2243S2342 = Source(1,1) * Source(3,2) - Source(1,2) * Source(3,1);
	
	float m2144S2441 = Source(1,0) * Source(3,3) - Source(1,3) * Source(3,0);
	float m2143S2341 = Source(1,0) * Source(3,2) - Source(1,2) * Source(3,0);
	
	float m2142S2241 = Source(1,0) * Source(3,1) - Source(1,1) * Source(3,0);
	
	float m2334S2433 = Source(1,2) * Source(2,3) - Source(1,3) * Source(2,2);
	float m2234S2432 = Source(1,1) * Source(2,3) - Source(1,3) * Source(2,1);
	float m2233S2332 = Source(1,1) * Source(2,2) - Source(1,2) * Source(2,1);
	
	float m2134S2431 = Source(1,0) * Source(2,3) - Source(1,3) * Source(2,0);
	float m2133S2331 = Source(1,0) * Source(2,2) - Source(1,2) * Source(2,0);
	
	float m2132S2231 = Source(1,0) * Source(2,1) - Source(1,1) * Source(2,0);
	
	float A11 =   Source(1,1) * m3344S3443 - Source(1,2) * m3244S3442 + Source(1,3) * m3243S3342;
	float A12 = -(Source(1,0) * m3344S3443 - Source(1,2) * m3144S3441 + Source(1,3) * m3143S3341);
	float A13 =   Source(1,0) * m3244S3442 - Source(1,1) * m3144S3441 + Source(1,3) * m3142S3241;
	float A14 = -(Source(1,0) * m3243S3342 - Source(1,1) * m3143S3341 + Source(1,2) * m3142S3241);
	
	float A21 = -(Source(0,1) * m3344S3443 - Source(0,2) * m3244S3442 + Source(0,3) * m3243S3342);
	float A22 =   Source(0,0) * m3344S3443 - Source(0,2) * m3144S3441 + Source(0,3) * m3143S3341;
	float A23 = -(Source(0,0) * m3244S3442 - Source(0,1) * m3144S3441 + Source(0,3) * m3142S3241);
	float A24 =   Source(0,0) * m3243S3342 - Source(0,1) * m3143S3341 + Source(0,2) * m3142S3241;
	
	float A31 =   Source(0,1) * m2344S2443 - Source(0,2) * m2244S2442 + Source(0,3) * m2243S2342;
	float A32 = -(Source(0,0) * m2344S2443 - Source(0,2) * m2144S2441 + Source(0,3) * m2143S2341);
	float A33 =   Source(0,0) * m2244S2442 - Source(0,1) * m2144S2441 + Source(0,3) * m2142S2241;
	float A34 = -(Source(0,0) * m2243S2342 - Source(0,1) * m2143S2341 + Source(0,2) * m2142S2241);
	
	float A41 = -(Source(0,1) * m2334S2433 - Source(0,2) * m2234S2432 + Source(0,3) * m2233S2332);
	float A42 =   Source(0,0) * m2334S2433 - Source(0,2) * m2134S2431 + Source(0,3) * m2133S2331;
	float A43 = -(Source(0,0) * m2234S2432 - Source(0,1) * m2134S2431 + Source(0,3) * m2132S2231);
	float A44 =   Source(0,0) * m2233S2332 - Source(0,1) * m2133S2331 + Source(0,2) * m2132S2231;
	
	//Calc out the determinant.
	float detA = Source(0,0) * A11;
								
	float detB = Source(0,1) * A12;
								
	float detC = Source(0,2) * A13;
								
	float detD = Source(0,3) * A14;
							   
	float det = (detA + detB + detC + detD);

	float oneOverDet = 0.0f;
	if (det < Stuff::SMALL)
	{
#ifdef _DEBUG
		PAUSE(("Matrix4D is Singular."));
#endif
		oneOverDet = 1e30f;
	}
	else
		oneOverDet = 1.0f / det;
	
	(*this)(0,0) = A11 * oneOverDet;
	(*this)(1,0) = A12 * oneOverDet;
	(*this)(2,0) = A13 * oneOverDet;
	(*this)(3,0) = A14 * oneOverDet;
	
	(*this)(0,1) = A21 * oneOverDet;
	(*this)(1,1) = A22 * oneOverDet;
	(*this)(2,1) = A23 * oneOverDet;
	(*this)(3,1) = A24 * oneOverDet;
 
	(*this)(0,2) = A31 * oneOverDet;
	(*this)(1,2) = A32 * oneOverDet;
	(*this)(2,2) = A33 * oneOverDet;
	(*this)(3,2) = A34 * oneOverDet;
 
	(*this)(0,3) = A41 * oneOverDet;
	(*this)(1,3) = A42 * oneOverDet;
	(*this)(2,3) = A43 * oneOverDet;
	(*this)(3,3) = A44 * oneOverDet;
	
	return (*this);
}

//
//###########################################################################
//###########################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Matrix4D& matrix
		)
	{
		Check_Object(&matrix);
		SPEW((
			group,
			"\n\t| %9f, %9f, %9f, %9f |",
			matrix(0,0),
			matrix(0,1),
			matrix(0,2),
			matrix(0,3)
		));
		SPEW((
			group,
			"\t| %9f, %9f, %9f, %9f |",
			matrix(1,0),
			matrix(1,1),
			matrix(1,2),
			matrix(1,3)
		));
		SPEW((
			group,
			"\t| %9f, %9f, %9f, %9f |",
			matrix(2,0),
			matrix(2,1),
			matrix(2,2),
			matrix(2,3)
		));
		SPEW((
			group,
			"\t| %9f, %9f, %9f, %9f |+",
			matrix(3,0),
			matrix(3,1),
			matrix(3,2),
			matrix(3,3)
		));
	}
#endif


//
//###########################################################################
//###########################################################################
//
void
	Matrix4D::SetPerspective(
		Scalar near_clip,
		Scalar far_clip,
		Scalar left_clip,
		Scalar right_clip,
		Scalar top_clip,
		Scalar bottom_clip
	)
{
	Verify(far_clip - near_clip > SMALL);
	Verify(left_clip - right_clip > SMALL);
	Verify(top_clip - bottom_clip > SMALL);

	//
	//-------------------------------------------------------
	// Calculate the horizontal, vertical, and forward ranges
	//-------------------------------------------------------
	//
	Scalar horizontal_range = APPLY_LEFT_SIGN(1.0f) / (left_clip - right_clip);
	Scalar vertical_range = APPLY_UP_SIGN(1.0f) / (top_clip - bottom_clip);
	Scalar depth_range = APPLY_FORWARD_SIGN(1.0f) / (far_clip - near_clip);

	//
	//------------------------------------------------------------------------
	// Set up the camera to clip matrix.  This matrix takes camera space
	// coordinates and maps them into a homogeneous culling space where valid
	// X, Y, and Z axis values (when divided by W) will all be between 0 and 1
	//------------------------------------------------------------------------
	//
	(*this)(LEFT_AXIS, LEFT_AXIS) = near_clip * horizontal_range;
	(*this)(LEFT_AXIS, UP_AXIS) = 0.0f;
	(*this)(LEFT_AXIS, FORWARD_AXIS) = 0.0f;
	(*this)(LEFT_AXIS, 3) = 0.0f;

	(*this)(UP_AXIS, LEFT_AXIS) = 0.0f;
	(*this)(UP_AXIS, UP_AXIS) = near_clip * vertical_range;
	(*this)(UP_AXIS, FORWARD_AXIS) = 0.0f;
	(*this)(UP_AXIS, 3) = 0.0f;

	(*this)(FORWARD_AXIS, LEFT_AXIS) = -right_clip * horizontal_range;
	(*this)(FORWARD_AXIS, UP_AXIS) = -bottom_clip * vertical_range;
	(*this)(FORWARD_AXIS, FORWARD_AXIS) = far_clip * depth_range;
	(*this)(FORWARD_AXIS, 3) = 1.0f;

	(*this)(3, LEFT_AXIS) = 0.0f;
	(*this)(3, UP_AXIS) = 0.0f;
	(*this)(3, FORWARD_AXIS) = -far_clip * near_clip * depth_range;
	(*this)(3, 3) = 0.0f;
}

void
	Matrix4D::GetPerspective(
		Scalar *near_clip,
		Scalar *far_clip,
		Scalar *left_clip,
		Scalar *right_clip,
		Scalar *top_clip,
		Scalar *bottom_clip
	) const
{
	if(near_clip)
	{
		Verify(!Small_Enough((*this)(FORWARD_AXIS, FORWARD_AXIS)) );
		*near_clip = -(*this)(3, FORWARD_AXIS)/(*this)(FORWARD_AXIS, FORWARD_AXIS);
	}

	if(far_clip)
	{
		Verify(!Small_Enough((1.0f - (*this)(FORWARD_AXIS, FORWARD_AXIS))) );
		*far_clip = (*this)(3, FORWARD_AXIS)/(1.0f - (*this)(FORWARD_AXIS, FORWARD_AXIS));
	}

	if(left_clip)
	{
		Verify(!Small_Enough((*this)(FORWARD_AXIS, FORWARD_AXIS) * (*this)(LEFT_AXIS, LEFT_AXIS)) );
		*left_clip = 	( ((*this)(FORWARD_AXIS, LEFT_AXIS) - 1.0f) * (*this)(3, FORWARD_AXIS) )/
						( (*this)(FORWARD_AXIS, FORWARD_AXIS) * (*this)(LEFT_AXIS, LEFT_AXIS) );
	}

	if(right_clip)
	{
		Verify(!Small_Enough((*this)(FORWARD_AXIS, FORWARD_AXIS) * (*this)(LEFT_AXIS, LEFT_AXIS)) );
		*right_clip =	( (*this)(FORWARD_AXIS, LEFT_AXIS) * (*this)(3, FORWARD_AXIS) )/
						( (*this)(FORWARD_AXIS, FORWARD_AXIS) * (*this)(LEFT_AXIS, LEFT_AXIS) );
	}

	if(top_clip)
	{
		Verify(!Small_Enough((*this)(UP_AXIS, UP_AXIS) * (*this)(FORWARD_AXIS, FORWARD_AXIS)) );
		*top_clip = 	( ((*this)(FORWARD_AXIS, UP_AXIS) - 1.0f) * (*this)(3, FORWARD_AXIS) )/
						( (*this)(UP_AXIS, UP_AXIS) * (*this)(FORWARD_AXIS, FORWARD_AXIS) );
	}

	if(bottom_clip)
	{
		Verify(!Small_Enough((*this)(UP_AXIS, UP_AXIS) * (*this)(FORWARD_AXIS, FORWARD_AXIS)) );
		*bottom_clip =	( (*this)(FORWARD_AXIS, UP_AXIS) * (*this)(3, FORWARD_AXIS) )/
						( (*this)(UP_AXIS, UP_AXIS) * (*this)(FORWARD_AXIS, FORWARD_AXIS) );
	}
}

void
	Matrix4D::SetPerspective(
		Scalar near_clip,
		Scalar far_clip,
		const  Radian &horizontal_fov,
		Scalar height_to_width
	)
{
	Verify(far_clip - near_clip > SMALL);
	Verify(horizontal_fov > SMALL);
	Verify(height_to_width > SMALL);

	//
	//-------------------------------------------------------------
	// Calculate the near plane offsets to the side culling planes
	//-------------------------------------------------------------
	//
	Scalar width = (Scalar)(near_clip * tan(horizontal_fov*0.5f));
	Scalar height = width * height_to_width;
	SetPerspective(near_clip, far_clip, width, -width, height, -height);
}

void
	Matrix4D::GetPerspective(
		Scalar *nearClip,
		Scalar *farClip,
		Radian *horizontal_fov,
		Scalar *height_to_width
	) const
{
	Scalar near_clip, far_clip, left_clip, right_clip, top_clip, bottom_clip;

	GetPerspective(&near_clip, &far_clip, &left_clip, &right_clip, &top_clip, &bottom_clip);

	if(nearClip)
	{
		*nearClip = near_clip;
	}

	if(farClip)
	{
		*farClip = far_clip;
	}

	if(horizontal_fov)
	{
		Verify(!Small_Enough(near_clip) );
		horizontal_fov->angle = 2.0f * (Scalar)atan(left_clip / near_clip);
	}

	if(height_to_width)
	{
		Verify(!Small_Enough(right_clip) );
		*height_to_width = top_clip / left_clip;
	}
}
