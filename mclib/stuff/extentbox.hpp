//===========================================================================//
// File:	extntbox.hh                                                      //
// Contents: Interface specification of bounding box class                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"scalar.hpp"
#include"marray.hpp"
#include"plane.hpp"

namespace Stuff {class ExtentBox;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::ExtentBox& box
		);
#endif

namespace Stuff {

	class Vector3D;
	class Point3D;
	class NotationFile;
	class OBB;

	//~~~~~~~~~~~~~~~~~~~~~~~~~ ExtentBox ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class ExtentBox
	{
	 public:
		Scalar
			minX,
			maxX,
			minY,
			maxY,
			minZ,
			maxZ;

		ExtentBox() {}
		ExtentBox(
			const Vector3D &min,
			const Vector3D &max
		);
		ExtentBox(const ExtentBox &box);
		explicit ExtentBox(const OBB& obb);

		const Scalar&
			operator[](int index) const
				{Check_Object(this); return (&minX)[index];}
		Scalar&
			operator[](int index)
				{Check_Object(this); return (&minX)[index];}

		ExtentBox&
			Intersect(
				const ExtentBox &box_1,
				const ExtentBox &box_2
			);
		ExtentBox&
			Union(
				const ExtentBox &box_1,
				const ExtentBox &box_2
			);
		ExtentBox&
			Union(
				const ExtentBox &box_1,
				const Vector3D &point
			);
		Vector3D*
			Constrain(Vector3D *point) const;

		bool
			Contains(const Vector3D &point) const;
		bool
			Contains(const ExtentBox &box) const;
		bool
			Intersects(const ExtentBox &box) const;

		void
			GetCenterpoint(Point3D *point) const;

		void
			TestInstance() const;
		static bool
			TestClass();
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const ExtentBox& box
				);
		#endif
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~ ExtentBox functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	void
   	Convert_From_Ascii(
      	const char *str,
         ExtentBox *extent_box
		);

	void
		Use_Scalar_In_Sorted_Array(
			DynamicArrayOf<Scalar> *values,
			Scalar value,
			unsigned *max_index,
			unsigned block_size,
			Scalar threshold = SMALL
		);

	void
		Find_Planes_Of_Boxes(
			DynamicArrayOf<Plane> *planes,
			const DynamicArrayOf<ExtentBox> &boxes
		);

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::ExtentBox *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::ExtentBox *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
