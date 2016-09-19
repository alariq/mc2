//===========================================================================//
// File:	angle.hh                                                         //
// Contents: Interface specification for angle classes                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"scalar.hpp"

namespace Stuff {
	class Radian;
	class Degree;
	class SinCosPair;
}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Radian& angle
		);
	void
		Spew(
			const char* group,
			const Stuff::Degree& angle
		);
	void
		Spew(
			const char* group,
			const Stuff::SinCosPair& angle
		);
#endif

namespace Stuff {

	class Degree;
	class SinCosPair;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Radian ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Radian
	{
	public:
		Scalar
			angle;

		//
		// Constructors
		//
		Radian()
			{}
		Radian(Scalar a)
			{angle = a;}
		Radian(const Radian &a)
			{angle = a.angle;}
		explicit Radian(const Degree &degree)
			{*this = degree;}
		explicit Radian(const SinCosPair &pair)
			{*this = pair;}

		//
		// Assignment operators
		//
		Radian&
			operator=(Scalar angle)
				{
					Check_Pointer(this);
					this->angle = angle; return *this;
				}
		Radian&
			operator=(const Radian &radian)
				{
					Check_Pointer(this); Check_Object(&radian);
					angle = radian.angle; return *this;
				}
		Radian&
			operator=(const Degree &degree);
		Radian&
			operator=(const SinCosPair &pair);

		//
		// Casting
		//
		operator Scalar() const
			{Check_Object(this); return angle;}

		//
		// These comparator functions are not designed to make exact comparisons
		// of Scalaring point numbers, but rather to compare them to within some
		// specified error threshold
		//
		bool
			operator!() const
				{Check_Object(this); return Small_Enough(angle);}

		bool
			operator==(const Radian &r) const
				{Check_Object(this); Check_Object(&r); return Close_Enough(angle,r.angle);}
		bool
			operator==(float r) const
				{Check_Object(this); return Close_Enough(angle,r);}

		bool
			operator!=(const Radian &r) const
				{Check_Object(this); Check_Object(&r); return !Close_Enough(angle,r.angle);}
		bool
			operator!=(float r) const
				{Check_Object(this); return !Close_Enough(angle,r);}

		//
		// Math operators
		//
		Radian&
			Negate(Scalar r)
				{Check_Pointer(this); angle = -r; return *this;}

		Radian&
			Add(Scalar r1,Scalar r2)
				{Check_Pointer(this); angle = r1 + r2; return *this;}
		Radian&
			operator+=(Scalar r)
				{Check_Object(this); angle += r; return *this;}

		Radian&
			Subtract(Scalar r1,Scalar r2)
				{Check_Pointer(this); angle = r1 - r2; return *this;}
		Radian&
			operator-=(Scalar r)
				{Check_Object(this); angle -= r; return *this;}

		Radian&
			Multiply(Scalar r1,Scalar r2)
				{Check_Pointer(this); angle = r1 * r2; return *this;}
		Radian&
			operator*=(Scalar r)
				{Check_Object(this); angle *= r; return *this;}

		Radian&
			Divide(Scalar r1,Scalar r2)
				{
					Check_Pointer(this); Verify(!Small_Enough(r2));
					angle = r1 / r2; return *this;
				}
		Radian&
			operator/=(Scalar r)
				{Check_Object(this); Verify(!Small_Enough(r)); angle /= r; return *this;}

		//
		// Template support
		//
		Radian&
			Lerp(
				const Radian &a,
				const Radian &b,
				Scalar t
			);

		//
		// Support functions
		//
		static Scalar
			Normalize(Scalar Value);
		Radian&
			Normalize();

		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Radian& angle
				);
		#endif
		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Degree ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Degree
	{
	 public:
		Scalar
			angle;

		//
		// constructors
		//
		Degree()
			{}
		Degree(Scalar a)
			{angle = a;}
		Degree(const Degree &a)
			{angle = a.angle;}
		explicit Degree(const Radian &radian)
			{*this = radian;}

		//
		// Assignment operators
		//
		Degree&
			operator=(const Degree &degree)
				{Check_Object(this); Check_Object(&degree); angle = degree.angle; return *this;}
		Degree&
			operator=(Scalar angle)
				{Check_Object(this); this->angle = angle; return *this;}
		Degree&
			operator=(const Radian &radian)
				{
					Check_Object(this); Check_Object(&radian);
					angle = radian.angle * Degrees_Per_Radian; return *this;
				}

		//
		// Support functions
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Degree& angle
				);
		#endif
		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

	inline Radian&
		Radian::operator=(const Degree& degree)
			{
				Check_Pointer(this); Check_Object(&degree);
				angle = degree.angle * Radians_Per_Degree; return *this;
			}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SinCosPair ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SinCosPair
	{
	 public:
		Scalar
			sine,
			cosine;

		//
		// Constructors
		//
		SinCosPair()
			{}
		SinCosPair(Scalar sin, Scalar cos)
			{Check_Pointer(this); sine = sin; cosine = cos; Check_Object(this);}
		SinCosPair(const SinCosPair &pair)
			{
				Check_Pointer(this); Check_Object(&pair);
				sine = pair.sine; cosine = pair.cosine;
			}
		explicit SinCosPair(const Radian &radian)
			{*this = radian;}

		//
		// Assignment operators
		//
		SinCosPair&
			operator=(const SinCosPair &pair)
				{
					Check_Pointer(this); Check_Object(&pair);
					sine = pair.sine; cosine = pair.cosine; return *this;
				}
		SinCosPair&
			operator=(const Radian &radian)
				{
					Check_Pointer(this);
					Check_Object(&radian);

				#if USE_ASSEMBLER_CODE
					Scalar *f = &sine;
					_asm {
						push	ebx
						push	edx

						mov		ebx, f
						mov		edx, radian.angle

						fld		dword ptr [edx]
						fsincos
						fstp	dword ptr [ebx + 4]
						fstp	dword ptr [ebx]

						pop		edx
						pop		ebx
					}

				#else
					cosine = cos(radian);
					sine = sin(radian);
				#endif
					
					Check_Object(this);
					
					return *this;
				}

		//
		// Support functions
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const SinCosPair& angle
				);
		#endif
		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

	inline Radian&
		Radian::operator=(const SinCosPair& pair)
			{
				Check_Pointer(this); Check_Object(&pair);
				angle = Arctan(pair.sine, pair.cosine); return *this;
			}

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Radian *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Radian *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
