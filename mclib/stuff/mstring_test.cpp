//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

#define TEST_STRING "Test String"
#define TEST_STRING_2 "Test StringTest String"
#define TEST_STRING_3 "Test StringZ"

bool
	MString::TestClass()
{
#if 1
	MString string_a;

	//Verify(string_a.stringSize == 0);
	//Verify(string_a.stringLength == 0);
   //Verify(string_a.stringText == NULL);

   MString string_b(TEST_STRING);
   MString string_c(string_b);

   Verify(string_b.GetLength() == strlen(TEST_STRING));
   Verify(string_b.GetLength() == strlen(string_c));
   Verify(string_b == string_c);

   MString string_d(TEST_STRING_2);
   MString string_e;

   string_e = string_b + string_c;
   Verify(string_e == string_d);

   MString string_f(TEST_STRING_3);
   MString string_g;

   string_g = string_b + 'Z';
   Verify(string_g == string_f);

   MString string_h(TEST_STRING);

   string_h += string_b;
   Verify(string_h == string_d);

	MString string_i(TEST_STRING);

   string_i += 'Z';
   Verify(string_i == string_f);

	MString string_j("aaa");
	MString string_k("aab");
	MString string_l("abb");
	MString string_m("bbb");
	MString string_n("aaa");

	Verify(string_j < string_k);
	Verify(string_l > string_k);
	Verify(string_l <= string_m);
	Verify(string_m >= string_j);
	Verify(string_j == string_n);
	Verify(string_j != string_k);

	Verify(string_k.operator[](0) == 'a');
	Verify(string_k.operator[](1) == 'a');
	Verify(string_k.operator[](2) == 'b');

	MString string_o("0.1,0.2,0.3");
	MString string_p("0.1 0.2 0.3");
	MString string_q("0.1");
	MString string_r("0.2");
	MString string_s("0.3");

	Verify(string_o.GetNthToken(0) == string_q);
	Verify(string_o.GetNthToken(1) == string_r);
	Verify(string_o.GetNthToken(2) == string_s);

	Verify(string_p.GetNthToken(0) == string_q);
	Verify(string_p.GetNthToken(1) == string_r);
	Verify(string_p.GetNthToken(2) == string_s);
#endif

	return true;
}

