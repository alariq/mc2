//===========================================================================//
// File:	linmtrx.hh                                                       //
// Contents: Interface specification for the linear matrices                 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"affinematrix.hpp"
#include"unitvector.hpp"
#include"rotation.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ LinearMatrix4D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class LinearMatrix4D:
		public AffineMatrix4D
	{
	public:
		static const LinearMatrix4D
			Identity;

		//
		// Constructors
		//
		LinearMatrix4D()
			{}
		explicit LinearMatrix4D(int)
			{BuildIdentity();}
		LinearMatrix4D(const LinearMatrix4D &m)
			{*this = m;}
		explicit LinearMatrix4D(const Origin3D &p)
			{*this = p;}
		explicit LinearMatrix4D(const AffineMatrix4D &m)
			{*this = m;}
		explicit LinearMatrix4D(const Matrix4D &m)
			{*this = m;}
		explicit LinearMatrix4D(const EulerAngles &angles)
			{*this = angles;}
		explicit LinearMatrix4D(const YawPitchRoll &angles)
			{*this = angles;}
		explicit LinearMatrix4D(const UnitQuaternion &q)
			{*this = q;}
		explicit LinearMatrix4D(const Point3D &p)
			{*this = p;}

		//
		// Assignment Operators
		//
		LinearMatrix4D&
			operator=(const LinearMatrix4D &m)
				{AffineMatrix4D::operator=(m); Check_Object(this); return *this;}
		LinearMatrix4D&
			operator=(const Origin3D &p)
				{AffineMatrix4D::operator=(p); Check_Object(this); return *this;}
		LinearMatrix4D&
			operator=(const AffineMatrix4D &m)
				{AffineMatrix4D::operator=(m); Check_Object(this); return *this;}
		LinearMatrix4D&
			operator=(const Matrix4D &m)
				{AffineMatrix4D::operator=(m); Check_Object(this); return *this;}
		LinearMatrix4D&
			operator=(const EulerAngles &angles)
				{
					AffineMatrix4D::operator=(angles);
					Check_Object(this); return *this;
				}
		LinearMatrix4D&
			operator=(const YawPitchRoll &angles)
				{
					AffineMatrix4D::operator=(angles);
					Check_Object(this); return *this;
				}
		LinearMatrix4D&
			operator=(const UnitQuaternion &q)
				{AffineMatrix4D::operator=(q); Check_Object(this); return *this;}
		LinearMatrix4D&
			operator=(const Point3D &p)
				{AffineMatrix4D::operator=(p); Check_Object(this); return *this;}

		LinearMatrix4D&
			BuildRotation(const EulerAngles &angles)
				{AffineMatrix4D::BuildRotation(angles); return *this;}
		LinearMatrix4D&
			BuildRotation(const YawPitchRoll &angles)
				{AffineMatrix4D::BuildRotation(angles); return *this;}
		LinearMatrix4D&
			BuildRotation(const UnitQuaternion &q)
				{AffineMatrix4D::BuildRotation(q); return *this;}
		LinearMatrix4D&
			BuildRotation(const Vector3D &v)
				{AffineMatrix4D::BuildRotation(v); return *this;}
		LinearMatrix4D&
			BuildTranslation(const Point3D &p)
				{AffineMatrix4D::BuildTranslation(p); return *this;}

		//
		// Axis Manipulation
		//
		void
			GetLocalForwardInWorld(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetLocalForwardInWorld(v);
			Check_Object(v);
		}
		void
			GetWorldForwardInLocal(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetWorldForwardInLocal(v);
			Check_Object(v);
		}

		void
			GetLocalBackwardInWorld(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetLocalBackwardInWorld(v);
			Check_Object(v);
		}
		void
			GetWorldBackwardInLocal(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetWorldBackwardInLocal(v);
			Check_Object(v);
		}

		void
			GetLocalRightInWorld(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetLocalRightInWorld(v);
			Check_Object(v);
		}
		void
			GetWorldRightInLocal(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetWorldRightInLocal(v);
			Check_Object(v);
		}

		void
			GetLocalLeftInWorld(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetLocalLeftInWorld(v);
			Check_Object(v);
		}
		void
			GetWorldLeftInLocal(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetWorldLeftInLocal(v);
			Check_Object(v);
		}

		void
			GetLocalUpInWorld(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetLocalUpInWorld(v);
			Check_Object(v);
		}
		void
			GetWorldUpInLocal(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetWorldUpInLocal(v);
			Check_Object(v);
		}

		void
			GetLocalDownInWorld(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetLocalDownInWorld(v);
			Check_Object(v);
		}
		void
			GetWorldDownInLocal(UnitVector3D *v) const
		{
			Check_Object(this); Check_Pointer(v);
			AffineMatrix4D::GetWorldDownInLocal(v);
			Check_Object(v);
		}

		void
			AlignLocalAxisToWorldVector(
				const Vector3D &target,
				int pointing_axis,
				int rotating_axis,
				int minor_axis
			);

		//
		// Matrix4D Multiplication
		//
		LinearMatrix4D&
			Multiply(
				const LinearMatrix4D& m1,
				const LinearMatrix4D& m2
			)
				{AffineMatrix4D::Multiply(m1, m2); return *this;}
		LinearMatrix4D&
			operator *=(const LinearMatrix4D& M)
				{LinearMatrix4D src(*this); return Multiply(src, M);}

		//
		// Matrix4D Inversion
		//
		LinearMatrix4D&
			Invert(const LinearMatrix4D& Source);
		LinearMatrix4D&
			Invert()
				{LinearMatrix4D src(*this); return Invert(src);}

		//
		// Rotation and Translation
		//
		LinearMatrix4D&
			Multiply(const LinearMatrix4D &m,const UnitQuaternion &q)
				{
					Check_Pointer(this); Check_Object(&m); Check_Object(&q);
					AffineMatrix4D::Multiply(m,q); return *this;
				}
		LinearMatrix4D&
			operator*=(const UnitQuaternion &q)
				{Check_Object(this); LinearMatrix4D m(*this); return Multiply(m,q);}
		LinearMatrix4D&
			Multiply(const LinearMatrix4D &m,const Point3D &p)
				{
					Check_Pointer(this); Check_Object(&m); Check_Object(&p);
					AffineMatrix4D::Multiply(m,p); return *this;
				}
		LinearMatrix4D&
			operator*=(const Point3D& p)
				{Check_Object(this); LinearMatrix4D m(*this); return Multiply(m,p);}

		//
		// Support functions
		//
		LinearMatrix4D&
			Normalize();

		void
      	TestInstance() const;
		static bool
			TestClass();

	private:
		LinearMatrix4D& Solve();
	};

	inline UnitQuaternion&
		UnitQuaternion::operator*=(const LinearMatrix4D &m)
			{UnitQuaternion t(*this); return Multiply(t,m);}

}

// sebi, why this was not implemented and how it worked??? could affine matrix implicitly converted to affine?
namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream *stream,
			Stuff::LinearMatrix4D *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream *stream,
			const Stuff::LinearMatrix4D *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
