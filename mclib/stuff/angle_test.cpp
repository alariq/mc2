//===========================================================================//
// File:	angle.tst                                                        //
// Contents: Tests for angle classes                                         //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//#############################################################################
//#############################################################################
//
bool
	Radian::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Radian Test..."));

	const Radian a(1.25f);
	Radian
		b,c;

	Test_Assumption(a);

	c = 0.0f;
	Test_Assumption(!c);

	Test_Assumption(Normalize(3.1f) == 3.1f);
	Test_Assumption(Normalize(-3.1f) == -3.1f);
	Scalar f = Normalize(Pi+Pi_Over_2);
	Test_Assumption(Close_Enough(f,Pi_Over_2 - Pi));
	f = Normalize(-Pi-Pi_Over_2);
	Test_Assumption(Close_Enough(f,Pi - Pi_Over_2));

	c = a;
#if 0
	Test_Assumption(c			== a);
	Test_Assumption(c.angle	== a);
	Test_Assumption(c			== a.angle);
#endif

	b.Negate(c);
#if 0
	Test_Assumption(b			== -c.angle);
#endif
	#if 0
	b = -c;
	d.Add(b,c);
	Test_Assumption(d			== b.angle + c.angle);
	Test_Assumption(a+b			== a.angle + b.angle);
	Test_Assumption(a+c			== a.angle + c.angle);
	Test_Assumption(a+1.25f	== a.angle+1.25f);
	Test_Assumption(1.25f+c	== 1.25f+c.angle);

	c = 1.5f;
	d.Subtract(c,a);
	Test_Assumption(d			== c.angle - a.angle);
	Test_Assumption(c-a			== c.angle - a.angle);
	Test_Assumption(c-1.25f	== c.angle - 1.25f);
	Test_Assumption(1.5f-a		== 1.5f - a.angle);
	Test_Assumption(c-b			== c.angle - b.angle);

	c = 2.5f;
	d.Multiply(a,c);
	Test_Assumption(d			== a.angle * c.angle);
	Test_Assumption(a*c			== a.angle * c.angle);
	Test_Assumption(1.25f*c	== 1.25f * c.angle);
	Test_Assumption(a*2.5f		== a.angle * 2.5f);
	Test_Assumption(a*b			== a.angle * b.angle);

	c = 2.0f;
	d.Divide(a,c);
	Test_Assumption(d			== a.angle / c.angle);
	Test_Assumption(a/c			== a.angle / c.angle);
	Test_Assumption(1.25f/c	== 1.25f / c.angle);
	Test_Assumption(a/2.0f		== a.angle / 2.0f);
	Test_Assumption(a/b			== a.angle / b.angle);

	b = a;
	b += c;
	b.Normalize();
	Test_Assumption(b == 3.25f - TWO_PI);
	b += 2.0f;
	b.Normalize();
	Test_Assumption(b == 5.25f - TWO_PI);

	b -= c;
	b.Normalize();
	Test_Assumption(b == 3.25f - TWO_PI);
	b -= 2.0f;
	b.Normalize();
	Test_Assumption(b == 1.25f);

	b *= c;
	b.Normalize();
	Test_Assumption(b == 1.25f*2.0f);
	b *= 2.0f;
	b.Normalize();
	Test_Assumption(b == 1.25f*2.0f*2.0f - TWO_PI);

	b = a*c;
	b.Normalize();
	Test_Assumption(b == 1.25f*2.0f);
	b /= 2.0f;
	b.Normalize();
	Test_Assumption(b == 1.25f);

	b = -3.0f*PI_OVER_4;
	c = 3.0f*PI_OVER_4;
	Test_Assumption(Lerp(b,c,0.25f) < b);
	Test_Assumption(Normalize(Lerp(b,c,0.75f)) > c);
	#endif

	return true;
}

//
//#############################################################################
//#############################################################################
//
bool
	Degree::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Degree test..."));

	const Degree a(Degrees_Per_Radian);
	Degree
		b,c;

	Radian
		r(1.0f),s;

	s = a;
	Test_Assumption(r == s);
	b = r;
	s = b;
#if 0
	Test_Assumption(r == s);
#endif
	c = Degrees_Per_Radian;
	s = c;
#if 0
	Test_Assumption(r == s);
#endif
	b = c;
	s = b;
#if 0
	Test_Assumption(r == s);
#endif

	return true;
}

//
//#############################################################################
//#############################################################################
//
bool
	SinCosPair::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting SinCos test..."));
	Radian
		s,r(Pi_Over_2);

	SinCosPair a;
	a = r;

	Test_Assumption(Close_Enough(a.sine,1.0f));
	Test_Assumption(Small_Enough(a.cosine));

	s = a;
	Test_Assumption(s == r);

	return true;
}
