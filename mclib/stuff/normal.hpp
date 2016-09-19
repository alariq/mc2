//===========================================================================//
// File:	normal.hh                                                        //
// Contents: Implementation details for the normal class                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"unitvector.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Normal3D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Normal3D:
		public UnitVector3D
	{
	public:
		//
		// Constructors
		//
		Normal3D()
			{}
		Normal3D(
			Scalar x,
			Scalar y,
			Scalar z
		):
			UnitVector3D(x,y,z)
				{}
		Normal3D(const Normal3D &n):
			UnitVector3D(n)
				{}
		Normal3D(const UnitVector3D& v):
			UnitVector3D(v)
				{}
		explicit Normal3D(const Vector3D &v)
			{*this = v;}

		static const Normal3D
			Forward;
		static const Normal3D
			Backward;
		static const Normal3D
			Left;
		static const Normal3D
			Right;
		static const Normal3D
			Up;
		static const Normal3D
			Down;

		//
		// Assignment operators
		//
		Normal3D&
			operator=(const Normal3D& n)
				{UnitVector3D::operator=(n); return *this;}
		Normal3D&
			operator=(const UnitVector3D& v)
				{UnitVector3D::operator=(v); return *this;}
		Normal3D&
			operator=(const Vector3D& v)
				{Normalize(v); return *this;}

		//
		// Math operators
		//
		Normal3D&
			Negate(const Normal3D &v)
				{Vector3D::Negate(v); return *this;}

		Scalar
			operator*(const Vector3D& v) const
				{return Vector3D::operator*(v);}

		Normal3D& Multiply(
			const Normal3D &n,
			const LinearMatrix4D &m
		)
			{UnitVector3D::Multiply(n,m); return *this;}
		Normal3D&
			operator*=(const LinearMatrix4D &M)
				{Normal3D src(*this); return Multiply(src,M);}

		//
		// These functions will cause the vector to lose its unit length, thus
		// cause any downstream verifies to fail.  We have to be able to only
		// normalize the normal once after all transformations if these are to
		// be of any benefit, so don't use them for now
		Normal3D&
			Multiply_Inverse(
				const Normal3D &Source,
				const AffineMatrix4D &M
			);
		Normal3D&
			Multiply(
				const Normal3D &Source,
				const AffineMatrix4D &M
			);

		//
		// Support functions
		//
		static bool
			TestClass();

	private:
		static const Normal3D identity;
		Normal3D& Negate(const Vector3D &V);
		Normal3D& Add(const Vector3D& V1,const Vector3D& V2);
		Normal3D& operator+=(const Vector3D& V);
		Normal3D& Subtract(const Vector3D& V1,const Vector3D& V2);
		Normal3D& operator-=(const Vector3D& V);
		Normal3D& Cross(const Vector3D& V1,const Vector3D& V2);
		Normal3D& Multiply(const Vector3D& V,Scalar Scale);
		Normal3D& operator*=(Scalar Value);
		Normal3D& Multiply(const Vector3D& V1,const Vector3D& V2);
		Normal3D& operator*=(const Vector3D &V);
		Normal3D& Multiply(const Vector3D &Source, const AffineMatrix4D &M);
		Normal3D& operator*=(const AffineMatrix4D &M);
		Normal3D& Divide(const Vector3D& V,Scalar Scale);
		Normal3D& operator/=(Scalar Value);
		Normal3D& Combine(const Vector3D& V1,Scalar t1,const Vector3D& V2,Scalar t2);
	};

}
