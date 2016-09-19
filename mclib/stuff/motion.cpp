//===========================================================================//
// File:	motion.cc                                                        //
// Contents: Implementation details for the position class                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

const Motion3D
	Motion3D::Identity(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(0.0f, 0.0f, 0.0f));

Motion3D::Motion3D(const Motion3D& motion)
{
	Check_Pointer(this);
	Check_Object(&motion);

	angularMotion = motion.angularMotion;
	linearMotion = motion.linearMotion;
}

//
//###########################################################################
//###########################################################################
//
Motion3D&
	Motion3D::operator=(const Motion3D &motion)
{
	Check_Pointer(this);
	Check_Object(&motion);

	angularMotion = motion.angularMotion;
	linearMotion = motion.linearMotion;
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Stuff::Close_Enough(
		const Motion3D &a1,
		const Motion3D &a2,
		Scalar e
	)
{
	Check_Object(&a1);
	Check_Object(&a2);
	return
		Close_Enough(a1.linearMotion, a2.linearMotion, e)
		 && Close_Enough(a1.angularMotion, a2.angularMotion, e);
}

//
//###########################################################################
//###########################################################################
//
Motion3D&
	Motion3D::AddScaled(
		const Motion3D& source,
		const Motion3D& delta,
		Scalar t
	)
{
	Check_Pointer(this);
	Check_Object(&source);
	Check_Object(&delta);
	Verify(t >= 0.0f);

	linearMotion.AddScaled(source.linearMotion, delta.linearMotion, t);
	angularMotion.AddScaled(source.angularMotion, delta.angularMotion, t);
	return *this;
}

//
//###########################################################################
//###########################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char *group,
			const Motion3D& motion
		)
	{
		Check_Object(&motion);

		SPEW((group, "{+"));
		Spew(group, motion.linearMotion);
		SPEW((group, ",+"));
		Spew(group, motion.angularMotion);
		SPEW((group, "}+"));
	}
#endif

//
//###########################################################################
//###########################################################################
//
void
	Motion3D::TestInstance() const
{
	Check_Object(&angularMotion);
}

