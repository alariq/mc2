//===========================================================================//
// File:	motion.hh                                                        //
// Contents: Implementation details for the position class                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"vector3d.hpp"

namespace Stuff {class Motion3D;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Motion3D& motion
		);
#endif

namespace Stuff {

	bool Close_Enough(
		const Motion3D &a1,
		const Motion3D &a2,
		Scalar e = SMALL
	);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ Motion3D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Motion3D
	{
	 public:
		Vector3D
			linearMotion;
		Vector3D
			angularMotion;

		static const Motion3D
			Identity;

		//
		// Constructors
		//
		Motion3D()
			{}
		Motion3D(const Motion3D& motion);
		Motion3D(
			const Vector3D& t,
			const Vector3D& q
		)
			{Check_Object(&t); Check_Object(&q); linearMotion = t; angularMotion = q;}

		//
		// Assignment operators
		//
		Motion3D&
			operator=(const Motion3D& p);

		friend bool
			Close_Enough(
				const Motion3D &a1,
				const Motion3D &a2,
				Scalar e/*=SMALL*/
			);
		bool
			operator==(const Motion3D& a) const
				{return Close_Enough(*this,a,SMALL);}
		bool
			operator!=(const Motion3D& a) const
				{return !Close_Enough(*this,a,SMALL);}

		//
		// Origin3D motion
		//
		Motion3D&
			AddScaled(
				const Motion3D& source,
				const Motion3D& delta,
				Scalar t
			);

		//
		// Support functions
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Motion3D& motion
				);
		#endif
		void
			TestInstance() const;
		static bool
			TestClass();
	};

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Motion3D *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Motion3D *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
