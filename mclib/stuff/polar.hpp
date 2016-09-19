//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"rotation.hpp"

namespace Stuff {class YawPitchRange;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::YawPitchRange &polar
		);
#endif

namespace Stuff {

	class YawPitchRange
	{
	public:
		Radian
			yaw,
			pitch;
		Scalar
			range;

		static const YawPitchRange
			Identity;

		//
		// Constructors
		//
		YawPitchRange()
			{}
		YawPitchRange(
			const Radian &y,
			const Radian &p,
			const Scalar r
		)
			{Check_Pointer(this); pitch = p; yaw = y; range = r;}
		YawPitchRange(const YawPitchRange &polar)
			{
				Check_Pointer(this);
				pitch = polar.pitch; yaw = polar.yaw; range = polar.range;
			}
		explicit YawPitchRange(const Vector3D &vector)
			{*this = vector;}

		//
		// Assignment operators
		//
		YawPitchRange&
			operator=(const YawPitchRange &angles)
				{
					Check_Pointer(this); Check_Object(&angles);
					pitch = angles.pitch; yaw = angles.yaw; range = angles.range;
					return *this;
				}
		YawPitchRange&
			operator=(const Vector3D &vector);

		//
		// "Close-enough" comparators
		//
		friend bool
			Small_Enough(
				const YawPitchRange &a,
				Scalar e=SMALL
			);
		bool
			operator!() const
				{return Small_Enough(*this);}

		friend bool
			Close_Enough(
				const YawPitchRange &a1,
				const YawPitchRange &a2,
				Scalar e=SMALL
			);
		bool
			operator==(const YawPitchRange& a) const
				{return Close_Enough(*this,a,SMALL);}
		bool
			operator!=(const YawPitchRange& a) const
				{return !Close_Enough(*this,a,SMALL);}

		//
		// Support functions
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const YawPitchRange &polar
				);
		#endif

		//
		// Test functions
		//
		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

}
