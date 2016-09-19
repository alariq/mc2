//===========================================================================//
// File:	normal.cc                                                        //
// Contents: Implementation details for the normal class                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

const Normal3D
	Normal3D::Forward(FORWARD_X, FORWARD_Y, FORWARD_Z);
const Normal3D
	Normal3D::Backward(BACKWARD_X, BACKWARD_Y, BACKWARD_Z);
const Normal3D
	Normal3D::Left(LEFT_X, LEFT_Y, LEFT_Z);
const Normal3D
	Normal3D::Right(RIGHT_X, RIGHT_Y, RIGHT_Z);
const Normal3D
	Normal3D::Up(UP_X, UP_Y, UP_Z);
const Normal3D
	Normal3D::Down(DOWN_X, DOWN_Y, DOWN_Z);

//
//#############################################################################
//#############################################################################
//
Normal3D&
	Normal3D::Multiply_Inverse(
		const Normal3D &Source,
		const AffineMatrix4D &M
	)
{
	Check_Object(this);
	Check_Object(&Source);
	Check_Object(&M);

	x = Source.x*M(0,0) + Source.y*M(0,1) + Source.z*M(0,2);
	y = Source.x*M(1,0) + Source.y*M(1,1) + Source.z*M(1,2);
	z = Source.x*M(2,0) + Source.y*M(2,1) + Source.z*M(2,2);

	return *this;
}

//
//#############################################################################
//#############################################################################
//
Normal3D&
	Normal3D::Multiply(
		const Normal3D &n,
		const AffineMatrix4D &m
	)
{
	Check_Object(this);
	Check_Object(&n);
	Check_Object(&n);
	Verify(this != &n);

	AffineMatrix4D m2;
	m2.Invert(m);
	return Multiply_Inverse(n,m);
}
