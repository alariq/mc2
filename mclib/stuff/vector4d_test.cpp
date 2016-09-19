//===========================================================================//
// File:	vector4d.tst                                                     //
// Contents: Test_Assumption code for vector classes                         //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//###########################################################################
//###########################################################################
//
bool
	Vector4D::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Vector4D test..."));

	const Vector4D
		a(Identity);
	Vector4D
		b;
	const Vector4D
		c(1.0f,2.0f,3.0f,4.0f);
	Vector4D
		d(2.0f,3.0f,1.0f,4.0f);

	Test_Assumption(!a.x && !a.y && !a.z && !a.w);
#if 0
	Test_Assumption(c.x == 1.0f && c.y == 2.0f && c.z == 3.0f && c.w == 4.0f);

	Test_Assumption(c[2] == c.z);
	Test_Assumption(a[1] == a.y);
#endif

	Test_Assumption(Small_Enough(a));
	Test_Assumption(!a);

	b = c;
	Test_Assumption(Close_Enough(b,c));
	Test_Assumption(b == c);
	Test_Assumption(a != b);

	Point3D p(3.0f,2.0f,1.0f);
	b = p;
	Test_Assumption(b == Vector4D(p.x,p.y,p.z,1.0f));

	Vector3D v(-1.0f,3.0f,2.0f);
	b = v;
	Test_Assumption(b == Vector4D(v.x,v.y,v.z,0.0f));

	b.Negate(c);
	Test_Assumption(b == Vector4D(-c.x,-c.y,-c.z,-c.w));

	b.Add(c,d);
	Test_Assumption(b == Vector4D(c.x+d.x,c.y+d.y,c.z+d.z,c.w+d.w));
	b = c;
	b += d;
	Test_Assumption(b == Vector4D(c.x+d.x,c.y+d.y,c.z+d.z,c.w+d.w));

	b.Subtract(d,c);
	Test_Assumption(b == Vector4D(d.x-c.x,d.y-c.y,d.z-c.z,d.w-c.w));
	b = d;
	b -= c;
	Test_Assumption(b == Vector4D(d.x-c.x,d.y-c.y,d.z-c.z,d.w-c.w));

	Scalar f = c*d;
#if 0
	Test_Assumption(f == c.x*d.x + c.y*d.y + c.z*d.z + c.w*d.w);
#endif

	f = 2.0f;
	b.Multiply(c,f);
	Test_Assumption(b == Vector4D(f*c.x,f*c.y,f*c.z,f*c.w));
	b = c;
	b *= f;
	Test_Assumption(b == Vector4D(f*c.x,f*c.y,f*c.z,f*c.w));

	b.Multiply(c,d);
	Test_Assumption(b == Vector4D(c.x*d.x,c.y*d.y,c.z*d.z,c.w*d.w));
	b = c;
	b *= d;
	Test_Assumption(b == Vector4D(c.x*d.x,c.y*d.y,c.z*d.z,c.w*d.w));

	b.Divide(c,f);
	Test_Assumption(b == Vector4D(c.x/f,c.y/f,c.z/f,c.w/f));
	b = c;
	b /= f;
	Test_Assumption(b == Vector4D(c.x/f,c.y/f,c.z/f,c.w/f));

	b.Divide(c,d);
	Test_Assumption(b == Vector4D(c.x/d.x,c.y/d.y,c.z/d.z,c.w/d.w));
	b = c;
	b /= d;
	Test_Assumption(b == Vector4D(c.x/d.x,c.y/d.y,c.z/d.z,c.w/d.w));

	AffineMatrix4D m;
	EulerAngles r(Pi_Over_4,0.0f,0.0f);
	m.BuildRotation(r);
	m.BuildTranslation(p);
	b.Multiply(c,m);
	Test_Assumption(b == Vector4D(c.x+c.w*m(3,0),c.y*m(1,1)+c.z*m(2,1)+c.w*m(3,1),c.y*m(1,2)+c.z*m(2,2)+c.w*m(3,2),c.w));
	b = c;
	b *= m;
	Test_Assumption(b == Vector4D(c.x+c.w*m(3,0),c.y*m(1,1)+c.z*m(2,1)+c.w*m(3,1),c.y*m(1,2)+c.z*m(2,2)+c.w*m(3,2),c.w));

	Matrix4D m2;
	m2 = m;
	b.Multiply(c,m2);
	Test_Assumption(b == Vector4D(c.x+c.w*m2(3,0),c.y*m2(1,1)+c.z*m2(2,1)+c.w*m2(3,1),c.y*m2(1,2)+c.z*m2(2,2)+c.w*m2(3,2),c.w));
	b = c;
	b *= m2;
	Test_Assumption(b == Vector4D(c.x+c.w*m2(3,0),c.y*m2(1,1)+c.z*m2(2,1)+c.w*m2(3,1),c.y*m2(1,2)+c.z*m2(2,2)+c.w*m2(3,2),c.w));

	b.Multiply(v,m2);
	Test_Assumption(b == Vector4D(v.x,v.y*m2(1,1)+v.z*m2(2,1),v.y*m2(1,2)+v.z*m2(2,2),0.0f));
	b.Multiply(p,m2);
	Test_Assumption(b == Vector4D(p.x+m2(3,0),p.y*m2(1,1)+p.z*m2(2,1)+m2(3,1),p.y*m2(1,2)+p.z*m2(2,2)+m2(3,2),1.0f));

	Test_Assumption(!a.GetLengthSquared());
	f = c.GetLengthSquared();
	Test_Assumption(f == c.x*c.x + c.y*c.y + c.z*c.z + c.w*c.w);
	f = c.GetLength();
	Test_Assumption(Close_Enough(f,Sqrt(c.GetLengthSquared())));

	b.Combine(c,2.0f,d,2.0f);
	Test_Assumption(b == Vector4D(2.0f*(c.x+d.x),2.0f*(c.y+d.y),2.0f*(c.z+d.z),2.0f*(c.w+d.w)));

	b.Lerp(c,d,0.5f);
	Test_Assumption(b == Vector4D(0.5f*(c.x+d.x),0.5f*(c.y+d.y),0.5f*(c.z+d.z),0.5f*(c.w+d.w)));

	return true;
}

