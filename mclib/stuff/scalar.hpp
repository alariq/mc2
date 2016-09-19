//===========================================================================//
// File:	scalar.hpp                                                       //
// Contents: Base information used by all MUNGA source files                 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"memorystream.hpp"

namespace Stuff {

	#if !defined(M_PI)
		#define M_PI 3.14159265358979323846
	#endif

	typedef float Scalar;
	const Scalar Pi=static_cast<Scalar>(M_PI);
	const Scalar Pi_Over_2=static_cast<Scalar>(M_PI/2.0);
	const Scalar Pi_Over_3=static_cast<Scalar>(M_PI/3.0);
	const Scalar Pi_Over_4=static_cast<Scalar>(M_PI/4.0);
	const Scalar Pi_Over_6=static_cast<Scalar>(M_PI/6.0);
	const Scalar Pi_Over_8=static_cast<Scalar>(M_PI/8.0);
	const Scalar Two_Pi=static_cast<Scalar>(2.0*M_PI);
	const Scalar Degrees_Per_Radian=static_cast<Scalar>(180.0/M_PI);
	const Scalar Radians_Per_Degree=static_cast<Scalar>(M_PI/180.0);
	const int FP_ONE_BITS = 0x3F800000;


	inline Scalar
		Fabs(Scalar a)
			{return static_cast<Scalar>(fabs(a));}
	inline Scalar
		Lerp(Scalar a, Scalar b, Scalar t)
			{return a*(1.0f - t) + b*t;}
	inline bool
		Small_Enough(Scalar x,Scalar e=SMALL)
			{return Fabs(x) <= e;}
	inline bool
		Close_Enough(Scalar x,Scalar y,Scalar e=SMALL)
			{return Fabs(x-y) <= e;}
	inline bool
		Close_Enough(int x,int y,Scalar e=SMALL)
			{return Fabs(static_cast<Scalar>(x-y)) <= e;}

	inline WORD
		Round_Float_To_Word(float in)
	{
		Verify(in >= 0.0f && in < 65536.0f);
		in += 12582912.0f;
		return *Cast_Pointer(WORD*, &in);
	}

	inline BYTE
		Round_Float_To_Byte(float in)
	{
		Verify(in >= 0.0f && in < 256.0f);
		in += 12582912.0f;
		return *Cast_Pointer(BYTE*, &in);
	}

    // sebi !NB
	inline BYTE
		Positive_Float_To_Byte(float in)
	{
		Verify(in >= 0.0f && in < 256.0f);
		return (BYTE)in;
	}

	inline WORD
		Truncate_Float_To_Word(float in)
	{
		Verify(in >= 0.0f && in < 65536.0f);
		in -= 0.5f;
		in += 12582912.0f;
		return *Cast_Pointer(WORD*, &in);
	}

	inline BYTE
		Truncate_Float_To_Byte(float in)
	{
		Verify(in >= 0.0f && in < 256.0f);
		in -= 0.5f;
		in += 12582912.0f;
		return *Cast_Pointer(BYTE*, &in);
	}

	DWORD
		Scaled_Float_To_Bits(float in, float min, float max, int bits);
	
	float
		Scaled_Float_From_Bits(DWORD in, float min, float max, int bits);

	DWORD
		Scaled_Int_To_Bits(int in, int min, int max, int bits);

	int
		Scaled_Int_From_Bits(DWORD in, int min, int max, int bits);

	

	int
		Round(Scalar value);

	// mg: I made some statistic test and came up with an max error of 6%
	static inline float
		SqrtApproximate(float f)
			{
				Verify(f >= 0.0f);

#if USE_ASSEMBLER_CODE
				float temp;
				_asm {
			// __int32 i = (AsInt32(f) >> 1) + (INT32_FLOAT_ONE >> 1);
					mov         eax, f
					sar         eax, 1
					add         eax, 1FC00000h
					mov			temp,eax
				}
				return temp;
#else
				return static_cast<Scalar>(sqrt(f));
#endif
			}

	static inline float
		OneOverApproximate(float f)
			{
				Verify(f != 0.0f);

#if USE_ASSEMBLER_CODE
				float temp;

				int _i = 2 * FP_ONE_BITS - *(int *)&(f);
				temp = *(float *)&_i;
				temp = temp * (2.0f - (f) * temp);

				return temp;
#else
				return 1.0f/f;
#endif
			}

	void
		Find_Roots(
			Scalar a,		// a*x*x + b*x + c = 0
			Scalar b,
			Scalar c,
			Scalar *center,
			Scalar *range
		);

	inline Scalar
		Sqrt(Scalar value)
	{
		Verify(value >= 0.0f);
		return static_cast<Scalar>(sqrt(value));
	}

	inline Scalar
		Arctan(
			Scalar y,
			Scalar x
		)
	{
		Verify(!Small_Enough(y) || !Small_Enough(x));
		return static_cast<Scalar>(atan2(y,x));
	}

	inline Scalar
		Arccos(Scalar x)
	{
		Verify(x>=-1.0f && x<=1.0f);
		return static_cast<Scalar>(acos(x));
	}

	inline Scalar
		Arcsin(Scalar x)
	{
		Verify(x>=-1.0f && x<=1.0f);
		return static_cast<Scalar>(asin(x));
	}

	inline Scalar
		Power(
			Scalar x,
			Scalar y
		)
	{
		Verify(x >= 0);
		return static_cast<Scalar>(
			pow(static_cast<double>(x), static_cast<double>(y))
		);
	}

	inline Scalar
		Exp(Scalar v)
	{
		return static_cast<Scalar>(exp(v));
	}

	inline Scalar
		Sin(Scalar v)
	{
		return static_cast<Scalar>(sin(v));
	}

	inline Scalar
		Cos(Scalar v)
	{
		return static_cast<Scalar>(cos(v));
	}

	inline Scalar
		Tan(Scalar v)
	{
		return static_cast<Scalar>(tan(v));
	}

	inline Scalar
		AtoF(const char* v)
	{
		return static_cast<Scalar>(atof(v));
	}

	typedef double Time;
}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Scalar *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Scalar *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
