//===========================================================================//
// File:	affnmtrx.hh                                                      //
// Contents: Interface specifications for Affine matrices                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"point3d.hpp"

namespace Stuff {class AffineMatrix4D;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::AffineMatrix4D& matrix
		);
#endif

namespace Stuff {

	class Origin3D;
	class EulerAngles;
	class UnitQuaternion;
	class YawPitchRoll;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ AffineMatrix4D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class AffineMatrix4D
	{
	 public:
		static const AffineMatrix4D
			Identity;

		Scalar
			entries[12];

		//
		// Constructors
		//
		AffineMatrix4D()
			{}
		AffineMatrix4D&
			BuildIdentity();
		explicit AffineMatrix4D(int)
			{BuildIdentity();}
		AffineMatrix4D(const AffineMatrix4D &m)
			{*this = m;}
		explicit AffineMatrix4D(const Origin3D &p)
			{*this = p;}
		explicit AffineMatrix4D(const Matrix4D &m)
			{*this = m;}
		explicit AffineMatrix4D(const EulerAngles &angles)
			{*this = angles;}
		explicit AffineMatrix4D(const YawPitchRoll &angles)
			{*this = angles;}
		explicit AffineMatrix4D(const UnitQuaternion &q)
			{*this = q;}
		explicit AffineMatrix4D(const Point3D &p)
			{*this = p;}

		//
		// Assignment Operators
		//
		AffineMatrix4D&
			operator=(const AffineMatrix4D &m)
				{
					Check_Pointer(this); Check_Object(&m);
					memcpy(entries, m.entries, sizeof(m.entries)); return *this;
				}
		AffineMatrix4D&
			operator=(const Origin3D &p);
		AffineMatrix4D&
			operator=(const Matrix4D &m);
		AffineMatrix4D&
			operator=(const EulerAngles &angles);
		AffineMatrix4D&
			operator=(const YawPitchRoll &angles);
		AffineMatrix4D&
			operator=(const UnitQuaternion &q);
		AffineMatrix4D&
			operator=(const Point3D &p);

		AffineMatrix4D&
			BuildRotation(const EulerAngles &angles);
		AffineMatrix4D&
			BuildRotation(const YawPitchRoll &angles);
		AffineMatrix4D&
			BuildRotation(const UnitQuaternion &q);
		AffineMatrix4D&
			BuildRotation(const Vector3D &angles);
		AffineMatrix4D&
			BuildTranslation(const Point3D &p)
				{
					Check_Pointer(this); Check_Object(&p);
					(*this)(W_Axis, X_Axis) = p.x;
					(*this)(W_Axis, Y_Axis) = p.y;
					(*this)(W_Axis, Z_Axis) = p.z;
					return *this;
				}

		//
		// Comparison operators
		//
		friend bool
			Close_Enough(
				const AffineMatrix4D &m1,
				const AffineMatrix4D &m2,
				Scalar e=SMALL
			);
		bool
			operator==(const AffineMatrix4D& a) const
				{return Close_Enough(*this,a,SMALL);}
		bool
			operator!=(const AffineMatrix4D& a) const
				{return !Close_Enough(*this,a,SMALL);}

		//
		// Index operators
		//
		Scalar&
			operator()(size_t row,size_t column)
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(row) <= W_Axis);
					Verify(static_cast<unsigned>(column) <= Z_Axis);
					return entries[(column<<2)+row];
				}
		const Scalar&
			operator ()(size_t row,size_t column) const
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(row) <= W_Axis);
					Verify(static_cast<unsigned>(column) <= Z_Axis);
					return entries[(column<<2)+row];
				}

		//
		// Axis Manipulation functions
		//
		void
			GetLocalForwardInWorld(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_FORWARD_SIGN((*this)(FORWARD_AXIS, X_Axis));
			v->y = APPLY_FORWARD_SIGN((*this)(FORWARD_AXIS, Y_Axis));
			v->z = APPLY_FORWARD_SIGN((*this)(FORWARD_AXIS, Z_Axis));
		}
		void
			GetWorldForwardInLocal(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_FORWARD_SIGN((*this)(X_Axis, FORWARD_AXIS));
			v->y = APPLY_FORWARD_SIGN((*this)(Y_Axis, FORWARD_AXIS));
			v->z = APPLY_FORWARD_SIGN((*this)(Z_Axis, FORWARD_AXIS));
		}

		void
			GetLocalBackwardInWorld(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_BACKWARD_SIGN((*this)(BACKWARD_AXIS, X_Axis));
			v->y = APPLY_BACKWARD_SIGN((*this)(BACKWARD_AXIS, Y_Axis));
			v->z = APPLY_BACKWARD_SIGN((*this)(BACKWARD_AXIS, Z_Axis));
		}
		void
			GetWorldBackwardInLocal(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_BACKWARD_SIGN((*this)(X_Axis, BACKWARD_AXIS));
			v->y = APPLY_BACKWARD_SIGN((*this)(Y_Axis, BACKWARD_AXIS));
			v->z = APPLY_BACKWARD_SIGN((*this)(Z_Axis, BACKWARD_AXIS));
		}

		void
			GetLocalRightInWorld(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_RIGHT_SIGN((*this)(RIGHT_AXIS, X_Axis));
			v->y = APPLY_RIGHT_SIGN((*this)(RIGHT_AXIS, Y_Axis));
			v->z = APPLY_RIGHT_SIGN((*this)(RIGHT_AXIS, Z_Axis));
		}
		void
			GetWorldRightInLocal(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_RIGHT_SIGN((*this)(X_Axis, RIGHT_AXIS));
			v->y = APPLY_RIGHT_SIGN((*this)(Y_Axis, RIGHT_AXIS));
			v->z = APPLY_RIGHT_SIGN((*this)(Z_Axis, RIGHT_AXIS));
		}

		void
			GetLocalLeftInWorld(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_LEFT_SIGN((*this)(LEFT_AXIS, X_Axis));
			v->y = APPLY_LEFT_SIGN((*this)(LEFT_AXIS, Y_Axis));
			v->z = APPLY_LEFT_SIGN((*this)(LEFT_AXIS, Z_Axis));
		}
		void
			GetWorldLeftInLocal(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_LEFT_SIGN((*this)(X_Axis, LEFT_AXIS));
			v->y = APPLY_LEFT_SIGN((*this)(Y_Axis, LEFT_AXIS));
			v->z = APPLY_LEFT_SIGN((*this)(Z_Axis, LEFT_AXIS));
		}

		void
			GetLocalUpInWorld(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_UP_SIGN((*this)(UP_AXIS, X_Axis));
			v->y = APPLY_UP_SIGN((*this)(UP_AXIS, Y_Axis));
			v->z = APPLY_UP_SIGN((*this)(UP_AXIS, Z_Axis));
		}
		void
			GetWorldUpInLocal(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_UP_SIGN((*this)(X_Axis, UP_AXIS));
			v->y = APPLY_UP_SIGN((*this)(Y_Axis, UP_AXIS));
			v->z = APPLY_UP_SIGN((*this)(Z_Axis, UP_AXIS));
		}

		void
			GetLocalDownInWorld(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_DOWN_SIGN((*this)(DOWN_AXIS, X_Axis));
			v->y = APPLY_DOWN_SIGN((*this)(DOWN_AXIS, Y_Axis));
			v->z = APPLY_DOWN_SIGN((*this)(DOWN_AXIS, Z_Axis));
		}
		void
			GetWorldDownInLocal(Vector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			v->x = APPLY_DOWN_SIGN((*this)(X_Axis, DOWN_AXIS));
			v->y = APPLY_DOWN_SIGN((*this)(Y_Axis, DOWN_AXIS));
			v->z = APPLY_DOWN_SIGN((*this)(Z_Axis, DOWN_AXIS));
		}

		//
		// Matrix Multiplication
		//
		inline AffineMatrix4D&
			Multiply(
				const AffineMatrix4D& Source1,
				const AffineMatrix4D& Source2
			)
		{
			Check_Pointer(this);
			Check_Object(&Source1);
			Check_Object(&Source2);
			Verify(this != &Source1);
			Verify(this != &Source2);

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

					faddp		st(1),st				//				A3,2
					fadd        dword ptr [esi+02Ch]	//	s2[3][2]	A3,3
					
					fstp        dword ptr [eax+02Ch]	//	[3][2]		S3

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
#endif

			return *this;
		};

		AffineMatrix4D&
			operator*=(const AffineMatrix4D& m)
				{AffineMatrix4D temp(*this); return Multiply(temp,m);}

		//
		// Matrix Inversion
		//
		AffineMatrix4D&
			Invert(const AffineMatrix4D& Source);
		AffineMatrix4D&
			Invert()
				{AffineMatrix4D src(*this); return Invert(src);}

		//
		// Scaling, Rotation and Translation
		//
		AffineMatrix4D&
			Multiply(const AffineMatrix4D &m,const Vector3D &v);
		AffineMatrix4D&
			operator*=(const Vector3D &v)
				{AffineMatrix4D m(*this); return Multiply(m,v);}
		AffineMatrix4D&
			Multiply(const AffineMatrix4D &m,const UnitQuaternion &q);
		AffineMatrix4D&
			operator*=(const UnitQuaternion &q)
				{AffineMatrix4D m(*this); return Multiply(m,q);}
		AffineMatrix4D&
			Multiply(const AffineMatrix4D &m,const Point3D &p);
		AffineMatrix4D&
			operator*=(const Point3D& p)
				{AffineMatrix4D m(*this); return Multiply(m,p);}

		//
		// Miscellaneous Functions
		//
		Scalar
			Determinant() const;
		AffineMatrix4D&
			Solve();

		//
		// Support functions
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const AffineMatrix4D& matrix
				);
		#endif

		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

	inline Point3D&
		Point3D::operator=(const AffineMatrix4D& m)
			{
				Check_Pointer(this); Check_Object(&m);
				x = m(W_Axis, X_Axis);
				y = m(W_Axis, Y_Axis);
				z = m(W_Axis, Z_Axis);
				Check_Object(this);
				return *this;
			}

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream *stream,
			Stuff::AffineMatrix4D *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream *stream,
			const Stuff::AffineMatrix4D *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
