//===========================================================================//
// File:	point.tst                                                        //
// Contents: test code for point class                                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//###########################################################################
//###########################################################################
//
bool
	Point3D::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Point3D test...\n"));

	const Point3D
		a(Identity);
	Point3D
		b;
	const Point3D
		c(1.0f,2.0f,3.0f);
	Point3D
		d(2.0f,3.0f,1.0f);

	Test_Assumption(!a.x && !a.y && !a.z);
#if 0
	Test_Assumption(c.x == 1.0f && c.y == 2.0f && c.z == 3.0f);

	Test_Assumption(c[2] == c.z);
	Test_Assumption(a[1] == a.y);
#endif

	Test_Assumption(Small_Enough(a));
//	Test_Assumption(!a);

	b = c;
#if 0
	Test_Assumption(b.x == c.x && b.y == c.y && b.z == c.z);
#endif
	Test_Assumption(Close_Enough(b,c));
	Test_Assumption(b == c);
	Test_Assumption(a != b);

	b.Negate(c);
	Test_Assumption(b == Point3D(-c.x,-c.y,-c.z));

	b.Add(c,d);
	Test_Assumption(b == Point3D(c.x+d.x,c.y+d.y,c.z+d.z));
	b = c;
	b += d;
	Test_Assumption(b == Point3D(c.x+d.x,c.y+d.y,c.z+d.z));

	b.Subtract(d,c);
	Test_Assumption(b == Point3D(d.x-c.x,d.y-c.y,d.z-c.z));
	b = d;
	b -= c;
	Test_Assumption(b == Point3D(d.x-c.x,d.y-c.y,d.z-c.z));

	Scalar f = c*d;
#if 0
	Test_Assumption(f == c.x*d.x + c.y*d.y + c.z*d.z);
#endif

	b.Cross(c,d);
	Test_Assumption(b == Point3D(c.y*d.z-c.z*d.y,c.z*d.x-c.x*d.z,c.x*d.y-c.y*d.x));

	f = 2.0f;
	b.Multiply(c,f);
	Test_Assumption(b == Point3D(f*c.x,f*c.y,f*c.z));
	b = c;
	b *= f;
	Test_Assumption(b == Point3D(f*c.x,f*c.y,f*c.z));

	b.Multiply(c,d);
	Test_Assumption(b == Point3D(c.x*d.x,c.y*d.y,c.z*d.z));
	b = c;
	b *= d;
	Test_Assumption(b == Point3D(c.x*d.x,c.y*d.y,c.z*d.z));

	b.Divide(c,f);
	Test_Assumption(b == Point3D(c.x/f,c.y/f,c.z/f));
	b = c;
	b /= f;
	Test_Assumption(b == Point3D(c.x/f,c.y/f,c.z/f));

	b.Divide(c,d);
	Test_Assumption(b == Point3D(c.x/d.x,c.y/d.y,c.z/d.z));
	b = c;
	b /= d;
	Test_Assumption(b == Point3D(c.x/d.x,c.y/d.y,c.z/d.z));

	AffineMatrix4D
		m;
	EulerAngles
		r(Pi_Over_4,0.0f,0.0f);
	m.BuildRotation(r);
	m.BuildTranslation(c);
	b = m;
	Test_Assumption(b == c);
	b.Multiply(c,m);
	Test_Assumption(b == Point3D(c.x+m(3,0),c.y*m(1,1)+c.z*m(2,1)+m(3,1),c.y*m(1,2)+c.z*m(2,2)+m(3,2)));
	b = c;
	b *= m;
	Test_Assumption(b == Point3D(c.x+m(3,0),c.y*m(1,1)+c.z*m(2,1)+m(3,1),c.y*m(1,2)+c.z*m(2,2)+m(3,2)));

	Test_Assumption(!a.GetLengthSquared());
	f = c.GetLengthSquared();
	Test_Assumption(f == c.x*c.x + c.y*c.y + c.z*c.z);
	f = c.GetLength();
	Test_Assumption(Close_Enough(f,Sqrt(c.x*c.x + c.y*c.y + c.z*c.z)));
	b.Normalize(c);
	Test_Assumption(b == Point3D(c.x/f, c.y/f, c.z/f));

	b.Combine(c,2.0f,d,2.0f);
	Test_Assumption(b == Point3D(2.0f*(c.x+d.x),2.0f*(c.y+d.y),2.0f*(c.z+d.z)));

	b.Lerp(c,d,0.5f);
	Test_Assumption(b == Point3D(0.5f*(c.x+d.x),0.5f*(c.y+d.y),0.5f*(c.z+d.z)));

	Vector4D v(2.0f,4.0f,6.0f,2.0f);
	b = v;
	Test_Assumption(b == Point3D(v.x/v.w, v.y/v.w, v.z/v.w));

	Origin3D t;
	t.linearPosition = Point3D(2.0f,3.0f,4.0f);
	t.angularPosition = EulerAngles(0.0f,0.0f,0.0f);
	b = t;
	Test_Assumption(b == Point3D(t.linearPosition.x,t.linearPosition.y,t.linearPosition.z));

	return true;
}
