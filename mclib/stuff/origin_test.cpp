//===========================================================================//
// File:	origin.tst                                                       //
// Contents: Implementation details for the position class                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//###########################################################################
//###########################################################################
//
bool
	Origin3D::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Origin3D test..."));

	Point3D p(1.0f,2.0f,3.0f);
	UnitQuaternion q(0.0f, 1.0f, 0.0f, 0.0f);

	UnitQuaternion r;
	r = q;
	const Origin3D
		a(p,r);
	Origin3D
		b;

	UnitQuaternion t;
	t = a.angularPosition;
	Test_Assumption(
		a.linearPosition == p
		 && t.x == q.x
		 && t.w == q.w
	);

	LinearMatrix4D
		m;

	m = a;
	b = m;
	t = b;
	Test_Assumption(
		a.linearPosition == b.linearPosition
		 && t.y == q.y
	);

#if 0
	b = Point3D(3.0f,2.0f,1.0f);
	p = b;
	Test_Assumption(p == Point3D(3.0f,2.0f,1.0f));
#endif

	return true;
}

