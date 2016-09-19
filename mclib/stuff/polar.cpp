//===========================================================================//
// File:	rotation.cc                                                      //
// Contents: Implementation details for rotation classes                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ YawPitchRange ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const YawPitchRange
	YawPitchRange::Identity(0.0f,0.0f,0.0f);

//
//#############################################################################
//#############################################################################
//
YawPitchRange&
	YawPitchRange::operator=(const Vector3D &vector)
{
	Check_Pointer(this);
	Check_Object(&vector);

	//
	//------------------------------------------------------------------------
	// See if we have a zero length vector.  If so, convert it to the identity
	//------------------------------------------------------------------------
	//
	Verify(
		Vector3D::Forward.z == 1.0f && Vector3D::Left.x == 1.0f && Vector3D::Up.y == 1.0f
	);
	Scalar sub_range = vector.x*vector.x + vector.z*vector.z;
	range = Sqrt(sub_range + vector.y*vector.y);
	if (Small_Enough(range))
	{
		yaw = 0.0f;
		pitch = 0.0f;
	}
	else
	{
		//
		//---------------------------------------------------------------------
		// Isolate the yaw element.  If the vector is vertical, yaw will simply
		// be zero.  If not, the yaw will indicate counter-clockwise deviation
		// from the negative Z axis
		//---------------------------------------------------------------------
		//
		sub_range = Sqrt(sub_range);
		if (Small_Enough(sub_range))
		{
			yaw = 0.0f;
			pitch = (vector.y > 0.0f) ? -Pi_Over_2 : Pi_Over_2;
		}
		else
		{
			yaw = Arctan(vector.x, vector.z);
			pitch = -Arctan(vector.y, sub_range);
		}
	}

	return *this;
}

//
//#############################################################################
//#############################################################################
//
bool
	Stuff::Small_Enough(
		const YawPitchRange& angles,
		Scalar e
	)
{
	Check_Object(&angles);

	return
		Small_Enough(angles.pitch,e)
		 && Small_Enough(angles.yaw,e)
		 && Small_Enough(angles.range,e);
}

//
//#############################################################################
//#############################################################################
//
bool
	Stuff::Close_Enough(
		const YawPitchRange& a1,
		const YawPitchRange& a2,
		Scalar e
	)
{
	Check_Object(&a1);
	Check_Object(&a2);

	return
		Close_Enough(a1.pitch,a2.pitch,e)
		 && Close_Enough(a1.yaw,a2.yaw,e)
		 && Close_Enough(a1.range,a2.range,e);
}

//
//#############################################################################
//#############################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const YawPitchRange &angle
		)
	{
		SPEW((group, "<+"));
		Spew(group, angle.yaw);
		SPEW((group, ",+"));
		Spew(group, angle.pitch);
		SPEW((group, "; %f+", angle.range));
	}
#endif
