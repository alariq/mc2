//===========================================================================//
// File:	origin.hh                                                        //
// Contents: Implementation details for the position class                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"point3d.hpp"
#include"rotation.hpp"

namespace Stuff {class Origin3D;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Origin3D& origin
		);
#endif

namespace Stuff {

	class Motion3D;

	bool Close_Enough(
		const Origin3D &a1,
		const Origin3D &a2,
		Scalar e = SMALL
	);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Origin3D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Origin3D
	{
	 public:
		Point3D
			linearPosition;
		UnitQuaternion
			angularPosition;

		static const Origin3D
			Identity;

		//
		// Constructors
		//
		Origin3D()
			{}
		Origin3D(
			const Point3D &t,
			const EulerAngles& q
		)
			{Check_Object(&t); Check_Object(&q); linearPosition = t; angularPosition = q;}
		Origin3D(
			const Point3D &t,
			const UnitQuaternion& q
		)
			{Check_Object(&t); Check_Object(&q); linearPosition = t; angularPosition = q;}
		Origin3D(const Origin3D &origin)
			{*this = origin;}
		explicit Origin3D(const LinearMatrix4D &matrix)
			{*this = matrix;}

		//
		// Assignment operators
		//
		Origin3D&
			operator=(const Origin3D& p);
		Origin3D&
			operator=(const LinearMatrix4D &m);

		Origin3D&
			BuildTranslation(const Point3D& p)
				{Check_Pointer(this); Check_Object(&p); linearPosition = p; return *this;}
		Origin3D&
			BuildRotation(const EulerAngles& q)
				{Check_Pointer(this); Check_Object(&q); angularPosition = q; return *this;}
		Origin3D&
			BuildRotation(const YawPitchRoll& q)
				{Check_Pointer(this); Check_Object(&q); angularPosition = q; return *this;}
		Origin3D&
			BuildRotation(const UnitQuaternion& q)
				{Check_Pointer(this); Check_Object(&q); angularPosition = q; return *this;}
		//
		// Equality operator
		//
		friend bool
			Close_Enough(
				const Origin3D &a1,
				const Origin3D &a2,
				Scalar e/*=SMALL*/
			);
		bool
			operator==(const Origin3D& a) const
				{return Close_Enough(*this,a,SMALL);}
		bool
			operator!=(const Origin3D& a) const
				{return !Close_Enough(*this,a,SMALL);}

		//
		// Origin3D motion
		//
#if 0
		Origin3D&
			AddScaled(
				const Origin3D& source,
				const Motion3D& delta,
				Scalar t
			);
#endif
		Origin3D&
			Lerp(
				const Origin3D& start,
				const Origin3D& end,
				Scalar t
			);

		//
		// Support functions
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Origin3D& origin
				);
		#endif
		void
			TestInstance() const;
		static bool
			TestClass();
	};

	inline Point3D&
		Point3D::operator=(const Origin3D& p)
			{return operator=(p.linearPosition);}

	inline EulerAngles&
		EulerAngles::operator=(const Origin3D& p)
			{return operator=(p.angularPosition);}

	inline UnitQuaternion&
		UnitQuaternion::operator=(const Origin3D& p)
			{return operator=(p.angularPosition);}

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Origin3D *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Origin3D *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
