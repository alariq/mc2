//===========================================================================//
// File:	origin.cc                                                        //
// Contents: Implementation details for the position class                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

const Origin3D
	Origin3D::Identity(
		Point3D(0.0f, 0.0f, 0.0f),
		UnitQuaternion(0.0f, 0.0f, 0.0f, 1.0f)
	);

//
//###########################################################################
//###########################################################################
//
Origin3D&
	Origin3D::operator=(const Origin3D &origin)
{
	Check_Pointer(this);
	Check_Object(&origin);

	angularPosition = origin.angularPosition;
	linearPosition = origin.linearPosition;
	return *this;
}

//
//###########################################################################
//###########################################################################
//
Origin3D&
	Origin3D::operator=(const LinearMatrix4D &matrix)
{
	Check_Pointer(this);
	Check_Object(&matrix);

	angularPosition = matrix;
	linearPosition = matrix;
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	Stuff::Close_Enough(
		const Origin3D &a1,
		const Origin3D &a2,
		Scalar e
	)
{
	Check_Object(&a1);
	Check_Object(&a2);
	return
		Close_Enough(a1.linearPosition, a2.linearPosition, e)
		 && Close_Enough(a1.angularPosition, a2.angularPosition, e);
}

#if 0
//
//###########################################################################
//###########################################################################
//
Origin3D&
	Origin3D::AddScaled(
		const Origin3D& source,
		const Motion& delta,
		Scalar t
	)
{
	Check_Pointer(this);
	Check_Object(&source);
	Check_Object(&delta);
	Verify(t >= 0.0f);

	linearPosition.AddScaled(source.linearPosition, delta.linearMotion, t);
	angularPosition.AddScaled(source.angularPosition, delta.angularMotion, t);
	return *this;
}
#endif

//
//#############################################################################
//#############################################################################
//
Origin3D&
	Origin3D::Lerp(
		const Origin3D &start,
		const Origin3D &end,
		Scalar t
	)
{
	Check_Pointer(this);
	Check_Object(&start);
	Check_Object(&end);

	linearPosition.Lerp(start.linearPosition, end.linearPosition, t);
	angularPosition.Lerp(start.angularPosition, end.angularPosition, t);
	Check_Object(this);
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
			const Origin3D& origin
		)
	{
		SPEW((group, "{+"));
		Spew(group, origin.linearPosition);
		SPEW((group, ",+"));
		Spew(group, origin.angularPosition);
		SPEW((group, "}+"));
	}
#endif

//
//###########################################################################
//###########################################################################
//
void
	Origin3D::TestInstance() const
{
	Check_Object(&angularPosition);
}
