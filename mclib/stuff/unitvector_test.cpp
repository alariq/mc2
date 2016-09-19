//===========================================================================//
// File:	unitvec.tst                                                      //
// Contents: Implementation details for unit vector class                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//###########################################################################
//###########################################################################
//
bool
	UnitVector3D::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting UnitVector3D test..."));

	UnitVector3D
		b;
	const UnitVector3D
		c(0.6f,0.0f,0.8f);
	UnitVector3D
		d(0.8f,-0.6f,0.0f);

#if 0
	Test_Assumption(c.x == 0.6f && c.y == 0.0f && c.z == 0.8f);

	Test_Assumption(c[2] == c.z);
#endif

	b = c;
#if 0
	Test_Assumption(b.x == c.x && b.y == c.y && b.z == c.z);
#endif
	Test_Assumption(Close_Enough(b,c));
	Test_Assumption(b == c);

	b.Negate(c);
	Test_Assumption(b == UnitVector3D(-c.x,-c.y,-c.z));

	Scalar f = c*d;
	Test_Assumption(Close_Enough(f,c.x*d.x + c.y*d.y + c.z*d.z));

	LinearMatrix4D
		m;
	EulerAngles
		r(Pi_Over_4,0.0f,0.0f);
	m.BuildRotation(r);
	b.Multiply(c,m);
	Test_Assumption(b == UnitVector3D(c.x,c.y*m(1,1)+c.z*m(2,1),c.y*m(1,2)+c.z*m(2,2)));
	b = c;
	b *= m;
	Test_Assumption(b == UnitVector3D(c.x,c.y*m(1,1)+c.z*m(2,1),c.y*m(1,2)+c.z*m(2,2)));

	f = c.GetLengthSquared();
	Test_Assumption(Close_Enough(f, 1.0f));
	f = c.GetLength();
	Test_Assumption(Close_Enough(f, 1.0f));

	Vector3D v(0.0f,1.2f,1.6f);
	f = v.GetLength();
	b = v;
	Test_Assumption(b == UnitVector3D(v.x/f, v.y/f, v.z/f));

	return true;
}

