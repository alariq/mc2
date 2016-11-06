//===========================================================================//
// File:	matrix.hh                                                        //
// Contents: Interface specification for the matrix class                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"vector3d.hpp"

namespace Stuff {class Matrix4D;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Matrix4D &matrix
		);
#endif

namespace Stuff {

	class AffineMatrix4D;
	class Origin3D;
	class EulerAngles;
	class YawPitchRoll;
	class Point3D;
	class UnitQuaternion;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Matrix4D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Matrix4D
	{
	public:
		static const Matrix4D
			Identity;

		Scalar
			entries[16];

		//
		// Constructors
		//
		Matrix4D()
			{}
		Matrix4D&
			BuildIdentity();
		explicit Matrix4D(int)
			{BuildIdentity();}
		Matrix4D(const Matrix4D &matrix)
			{
				Check_Pointer(this); Check_Object(&matrix);
				memcpy(entries, matrix.entries, sizeof(entries));
			}
		explicit Matrix4D(const AffineMatrix4D &m)
			{*this = m;}
		explicit Matrix4D(const Origin3D &p)
			{*this = p;}
		explicit Matrix4D(const EulerAngles &angles)
			{*this = angles;}
		explicit Matrix4D(const YawPitchRoll &angles)
			{*this = angles;}
		explicit Matrix4D(const UnitQuaternion &q)
			{*this = q;}
		explicit Matrix4D(const Point3D &p)
			{*this = p;}

		//
		// Assignment operators
		//
		Matrix4D&
			operator=(const Matrix4D &m)
				{
					Check_Pointer(this); Check_Object(&m);
					memcpy(entries, m.entries, sizeof(m.entries));
					return *this;
				}
		Matrix4D&
			operator=(const AffineMatrix4D &m);
		Matrix4D&
			operator=(const Origin3D &p);
		Matrix4D&
			operator=(const EulerAngles &angles);
		Matrix4D&
			operator=(const YawPitchRoll &angles);
		Matrix4D&
			operator=(const UnitQuaternion &q);
		Matrix4D&
			operator=(const Point3D &p);

		Matrix4D&
			BuildRotation(const EulerAngles &angles);
		Matrix4D&
			BuildRotation(const YawPitchRoll &angles);
		Matrix4D&
			BuildRotation(const UnitQuaternion &q);
		Matrix4D&
			BuildTranslation(const Point3D &p);

		//
		// Comparison operators
		//
		friend bool
			Close_Enough(
				const Matrix4D &m1,
				const Matrix4D &m2,
				Scalar e/*=SMALL*/
			);
		bool
			operator==(const Matrix4D& a) const
				{return Close_Enough(*this,a,SMALL);}
		bool
			operator!=(const Matrix4D& a) const
				{return !Close_Enough(*this,a,SMALL);}

		//
		// Index operators
		//
		Scalar&
			operator ()(size_t row,size_t column)
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(row) <= W_Axis);
					Verify(static_cast<unsigned>(column) <= W_Axis);
					return entries[(column<<2)+row];
				}
		const Scalar&
			operator ()(size_t row,size_t column) const
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(row) <= W_Axis);
					Verify(static_cast<unsigned>(column) <= W_Axis);
					return entries[(column<<2)+row];
				}

		//
		// Matrix4D Multiplication
		//
		Matrix4D&
			Multiply(
				const Matrix4D& Source1,
				const Matrix4D& Source2
			);
		Matrix4D&
			operator *=(const Matrix4D& m)
				{Matrix4D temp(*this); return Multiply(temp,m);}
		Matrix4D&
			Multiply(
				const Matrix4D& Source1,
				const AffineMatrix4D &Source2
			);
		Matrix4D&
			operator *=(const AffineMatrix4D& m)
				{Matrix4D temp(*this); return Multiply(temp,m);}
		inline Matrix4D&
			Multiply(
				const AffineMatrix4D &Source1,
				const Matrix4D& Source2
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
		};

		Matrix4D&
			Multiply(
				const AffineMatrix4D &Source1,
				const AffineMatrix4D &Source2
			);

		//
		// Matrix4D Inversion
		//
		Matrix4D&
			Invert(const Matrix4D& Source);
		Matrix4D&
			Invert()
				{Matrix4D src(*this); return Invert(src);}

		//
		// Viewpoint Calculation
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Matrix4D &matrix
				);
		#endif
		static bool
			TestClass();
		void
			TestInstance() const
				{}

		//
		// Creating perspective Matrix4D
		//
		void
			SetPerspective(
				Scalar near_clip,
				Scalar far_clip,
				Scalar left_clip,
				Scalar right_clip,
				Scalar top_clip,
				Scalar bottom_clip
			);

		void
			GetPerspective(
				Scalar *near_clip,
				Scalar *far_clip,
				Scalar *left_clip,
				Scalar *right_clip,
				Scalar *top_clip,
				Scalar *bottom_clip
			) const;

		void
			SetPerspective(
				Scalar near_clip,
				Scalar far_clip,
				const Radian &horizontal_fov,
				Scalar height_to_width
			);
		void
			GetPerspective(
				Scalar *near_clip,
				Scalar *far_clip,
				Radian *horizontal_fov,
				Scalar *height_to_width
			) const;


	};

}
