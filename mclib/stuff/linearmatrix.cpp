//===========================================================================//
// File:	linmtrx.cc                                                       //
// Contents: Implementation details for the linear matrices                  //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

const LinearMatrix4D
	LinearMatrix4D::Identity(true);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	LinearMatrix4D::AlignLocalAxisToWorldVector(
		const Vector3D &world_target,
		int pointing_axis,
		int rotating_axis,
		int minor_axis
	)
{
	Check_Object(this);
	Check_Object(&world_target);
	Verify(static_cast<unsigned>(pointing_axis) <= Z_Axis);
	Verify(static_cast<unsigned>(rotating_axis) <= Z_Axis);
	Verify(rotating_axis != pointing_axis);

	//
	//------------------------------------------------------------------
	// These are the variables that the alignment algorithm must fill in
	//------------------------------------------------------------------
	//
	UnitVector3D
		rotation_vector,
		pointing_vector,
		minor_vector;

	//
	//------------------------------------------------------------------
	// Extract the current target axis direction, then cross it with the
	// plane target to find the minor axis direction (unsigned)
	//------------------------------------------------------------------
	//
	if (Small_Enough(world_target.GetLengthSquared()))
		return;
	rotation_vector.x = (*this)(rotating_axis, X_Axis);
	rotation_vector.y = (*this)(rotating_axis, Y_Axis);
	rotation_vector.z = (*this)(rotating_axis, Z_Axis);
	Check_Object(&rotation_vector);
	Vector3D temp;
	temp.Cross(rotation_vector, world_target);

	//
	//----------------------------------------------------------------------
	// First check to see if we are rotating around a frozen axis.  If so,
	// if the axes specified are in the right-handed configuration, simply
	// generate the new pointing axis values, otherwise negate the minor
	// axis and generate the pointing vector appropriately
	//----------------------------------------------------------------------
	//
	if (minor_axis == -1)
	{
		minor_axis = 3 - pointing_axis - rotating_axis;
		minor_vector.Normalize(temp);
		if ((rotating_axis+1)%3 == pointing_axis)
			pointing_vector.Vector3D::Cross(minor_vector, rotation_vector);
		else
		{
			minor_vector.Vector3D::Negate(minor_vector);
			pointing_vector.Vector3D::Cross(rotation_vector, minor_vector);
		}
		Check_Object(&pointing_vector);
	}

	//
	//------------------------------------------------------------------------
	// The next case to check is non-frozen rotation.  In this case, maximum
	// effort is taken to preserve the rotating matrix, but it will be rotated
	// around the minor axis so that the pointing axis is exactly aligned with
	// the target vector
	//------------------------------------------------------------------------
	//
	else
	{

		//
		//--------------------------------------------------------------------
		// If the resultant vector is zero, it means the rotating axis is
		// parallel to the target vector, and thus a correct orthogonal set of
		// axis vectors can already be found in the matrix
		//--------------------------------------------------------------------
		//
		Verify(minor_axis == 3 - pointing_axis - rotating_axis);
		if (Small_Enough(temp.GetLengthSquared()))
		{
			if (world_target*rotation_vector > 0.0f)
			{
				pointing_vector.x = (*this)(rotating_axis, X_Axis);
				pointing_vector.y = (*this)(rotating_axis, Y_Axis);
				pointing_vector.z = (*this)(rotating_axis, Z_Axis);
				rotation_vector.x = -(*this)(pointing_axis, X_Axis);
				rotation_vector.y = -(*this)(pointing_axis, Y_Axis);
				rotation_vector.z = -(*this)(pointing_axis, Z_Axis);
			}
			else
			{
				pointing_vector.x = -(*this)(rotating_axis, X_Axis);
				pointing_vector.y = -(*this)(rotating_axis, Y_Axis);
				pointing_vector.z = -(*this)(rotating_axis, Z_Axis);
				rotation_vector.x = (*this)(pointing_axis, X_Axis);
				rotation_vector.y = (*this)(pointing_axis, Y_Axis);
				rotation_vector.z = (*this)(pointing_axis, Z_Axis);
			}
			minor_vector.x = (*this)(minor_axis, X_Axis);
			minor_vector.y = (*this)(minor_axis, Y_Axis);
			minor_vector.z = (*this)(minor_axis, Z_Axis);
		}

		//
		//---------------------------------------------------------------------
		// We have a non-trivial minor vector, so use it to generate the real
		// minor axis, then calculate the new rotation axis.  If the axes
		// specified are in the right-handed configuration, simply generate the
		// new pointing axis values, otherwise negate the minor axis and
		// generate the pointing vector appropriately
		//---------------------------------------------------------------------
		//
		else
		{
			pointing_vector.Normalize(world_target);
			minor_vector.Normalize(temp);
			if ((rotating_axis+1)%3 == pointing_axis)
				rotation_vector.Vector3D::Cross(pointing_vector, minor_vector);
			else
			{
				minor_vector.Vector3D::Negate(minor_vector);
				rotation_vector.Vector3D::Cross(minor_vector, pointing_vector);
			}
			Check_Object(&rotation_vector);
		}
	}

	//
	//------------------------------------------------
	// Now stuff the unit vectors back into the matrix
	//------------------------------------------------
	//
	Check_Object(&pointing_vector);
	(*this)(pointing_axis, X_Axis) = pointing_vector.x;
	(*this)(pointing_axis, Y_Axis) = pointing_vector.y;
	(*this)(pointing_axis, Z_Axis) = pointing_vector.z;

	Check_Object(&rotation_vector);
	(*this)(rotating_axis, X_Axis) = rotation_vector.x;
	(*this)(rotating_axis, Y_Axis) = rotation_vector.y;
	(*this)(rotating_axis, Z_Axis) = rotation_vector.z;

	Check_Object(&minor_vector);
	(*this)(minor_axis, X_Axis) = minor_vector.x;
	(*this)(minor_axis, Y_Axis) = minor_vector.y;
	(*this)(minor_axis, Z_Axis) = minor_vector.z;

	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
LinearMatrix4D&
	LinearMatrix4D::Invert(const LinearMatrix4D& m)
{
	Check_Pointer(this);
	Check_Object(&m);
	Verify(this != &m);

	//
	//-----------------------------------------
	// First, transpose the 3x3 rotation matrix
	//-----------------------------------------
	//
	(*this)(0,0) = m(0,0);
	(*this)(0,1) = m(1,0);
	(*this)(0,2) = m(2,0);
	(*this)(1,0) = m(0,1);
	(*this)(1,1) = m(1,1);
	(*this)(1,2) = m(2,1);
	(*this)(2,0) = m(0,2);
	(*this)(2,1) = m(1,2);
	(*this)(2,2) = m(2,2);

	//
	//----------------------------
	// Now run the offsets through
	//----------------------------
	//
	(*this)(3,0) = -m(3,0)*m(0,0) - m(3,1)*m(0,1) - m(3,2)*m(0,2);
	(*this)(3,1) = -m(3,0)*m(1,0) - m(3,1)*m(1,1) - m(3,2)*m(1,2);
	(*this)(3,2) = -m(3,0)*m(2,0) - m(3,1)*m(2,1) - m(3,2)*m(2,2);
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
LinearMatrix4D&
	LinearMatrix4D::Normalize()
{
	Check_Pointer(this);

	#if defined(LEFT_HANDED_COORDINATES)
		#error Right handed coordinate dependancy!
	#endif
	(*this)(0,2) = (*this)(1,0)*(*this)(2,1) - (*this)(1,1)*(*this)(2,0);
	(*this)(1,2) = (*this)(2,0)*(*this)(0,1) - (*this)(2,1)*(*this)(0,0);
	(*this)(2,2) = (*this)(0,0)*(*this)(1,1) - (*this)(0,1)*(*this)(1,0);
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	LinearMatrix4D::TestInstance() const
{
	UnitVector3D v1;
	v1.x = (*this)(0,0);
	v1.y = (*this)(0,1);
	v1.z = (*this)(0,2);
	Check_Object(&v1);

	UnitVector3D v2;
	v2.x = (*this)(1,0);
	v2.y = (*this)(1,1);
	v2.z = (*this)(1,2);
	Check_Object(&v2);

	UnitVector3D v3;
	v3.Vector3D::Cross(v1,v2);
	#if defined(LEFT_HANDED_COORDINATES)
		#error Right handed coordinate depenancy!
	#endif


	Verify(Close_Enough(v3.x, (*this)(2,0)));
	Verify(Close_Enough(v3.y, (*this)(2,1)));
	Verify(Close_Enough(v3.z, (*this)(2,2)));


}
