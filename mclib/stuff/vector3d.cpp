//===========================================================================//
// File:	vector3d.cc                                                      //
// Contents: Implementation details for vector classes                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Vector3D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const Vector3D
	Vector3D::Identity(0.0f, 0.0f, 0.0f);
const Vector3D
	Vector3D::Forward(FORWARD_X, FORWARD_Y, FORWARD_Z);
const Vector3D
	Vector3D::Backward(BACKWARD_X, BACKWARD_Y, BACKWARD_Z);
const Vector3D
	Vector3D::Left(LEFT_X, LEFT_Y, LEFT_Z);
const Vector3D
	Vector3D::Right(RIGHT_X, RIGHT_Y, RIGHT_Z);
const Vector3D
	Vector3D::Up(UP_X, UP_Y, UP_Z);
const Vector3D
	Vector3D::Down(DOWN_X, DOWN_Y, DOWN_Z);

//
//###########################################################################
//###########################################################################
//
bool
	Stuff::Close_Enough(
		const Vector3D &V1,
		const Vector3D &V2,
		Scalar e
	)
{
	Check_Object(&V1);
	Check_Object(&V2);
	Vector3D v(V1.x-V2.x,V1.y-V2.y,V1.z-V2.z);
	return Small_Enough(v, e);
}

//
//###########################################################################
//###########################################################################
//
Vector3D&
	Vector3D::operator=(const UnitQuaternion &q)
{
	Check_Pointer(this);
	Check_Object(&q);

	Scalar sine_of_half = Sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
	if (Small_Enough(sine_of_half))
	{
		return *this = Identity;
	}

	SinCosPair half_angle(sine_of_half, q.w);
	Radian angle;
	angle = half_angle;
	Scalar len = angle * 2.0f / sine_of_half;
	x = q.x * len;
	y = q.y * len;
	z = q.z * len;
	return *this;
}

//
//#############################################################################
//#############################################################################
//
Vector3D&
	Vector3D::operator=(const YawPitchRange &polar)
{
	Check_Object(this);
	Check_Object(&polar);

	Verify(
		Vector3D::Forward.z == 1.0f
		 && Vector3D::Left.x == 1.0f
		 && Vector3D::Up.y == 1.0f
	);
	SinCosPair azimuth(polar.yaw);
	SinCosPair altitude(polar.pitch);
	y = -polar.range * altitude.sine;

	Scalar len = polar.range * altitude.cosine;
	x = len * azimuth.sine;
	z = len * azimuth.cosine;
	return *this;
}

//
//###########################################################################
//###########################################################################
//
Vector3D&
	Vector3D::Multiply(const Vector3D& v,const AffineMatrix4D& m)
{
	Check_Pointer(this);
	Check_Object(&v);
	Check_Object(&m);
	Verify(this != &v);

	x = v.x*m(0,0) + v.y*m(1,0) + v.z*m(2,0);
	y = v.x*m(0,1) + v.y*m(1,1) + v.z*m(2,1);
	z = v.x*m(0,2) + v.y*m(1,2) + v.z*m(2,2);
	return *this;
}

//
//###########################################################################
//###########################################################################
//
Vector3D&
	Vector3D::MultiplyByInverse(
		const Vector3D& v,
		const LinearMatrix4D& m
	)
{
	Check_Pointer(this);
	Check_Object(&v);
	Check_Object(&m);
	Verify(this != &v);

	x = v.x*m(0,0) + v.y*m(0,1) + v.z*m(0,2);
	y = v.x*m(1,0) + v.y*m(1,1) + v.z*m(1,2);
	z = v.x*m(2,0) + v.y*m(2,1) + v.z*m(2,2);
	return *this;
}

//
//###########################################################################
//###########################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Vector3D &vector
		)
	{
		Check_Object(&vector);
		SPEW((group, "<%4f,%4f,%4f>+", vector.x, vector.y, vector.z));
	}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~ Vector3D functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//###########################################################################
//###########################################################################
//
void
	Stuff::Convert_From_Ascii(
      const char *str,
      Vector3D *vector_3D
   )
{
	Check_Pointer(str);
   Check_Object(vector_3D);

   MString parse_string(str);

	const char* token = parse_string.GetNthToken(0);
   Check_Pointer(token);
   vector_3D->x = AtoF(token);

	token = parse_string.GetNthToken(1);
   Check_Pointer(token);
   vector_3D->y = AtoF(token);

	token = parse_string.GetNthToken(2);
   Check_Pointer(token);
   vector_3D->z = AtoF(token);

   Check_Object(vector_3D);
}
