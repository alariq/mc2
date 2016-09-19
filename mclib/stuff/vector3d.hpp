//===========================================================================//
// File:	vector3d.hh                                                      //
// Contents: Interface specification for vector classes                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"memorystream.hpp"
#include"scalar.hpp"

namespace Stuff {class Vector3D;}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::Vector3D &vector
		);
#endif

namespace Stuff {

	class AffineMatrix4D;
	class LinearMatrix4D;
	class Matrix4D;
	class Point3D;
	class UnitQuaternion;
	class YawPitchRange;

	enum Axes {
		X_Axis,
		Y_Axis,
		Z_Axis,
		W_Axis
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// The following macros define MUNGAs base coordinate system
	//
	// Note that Yaw is always about the up/down axis, pitch is always about
	// the left/right axis, and Roll is always about the forward/back axis.  In
	// order to rotate about negative axes, the appropriate macros should be
	// uncommented
	//
//	#define LEFT_HANDED_COORDINATES

	#define UP_AXIS Y_Axis
	#define DOWN_AXIS Y_Axis
	#define FORWARD_AXIS Z_Axis
	#define BACKWARD_AXIS Z_Axis
	#define RIGHT_AXIS X_Axis
	#define LEFT_AXIS X_Axis

	#define PRIMARY_AXIS LEFT_AXIS
	#define SECONDARY_AXIS UP_AXIS
	#define TERTIARY_AXIS FORWARD_AXIS

//	#define YAW_ABOUT_DOWN			// Default Yaw is about Up
// #define PITCH_ABOUT_LEFT		// Default Pitch is about Right
//	#define ROLL_ABOUT_BACKWARD	// Default Roll is about Forward

	#define APPLY_UP_SIGN(y) (y)
	#define UP_X (0.0f)
	#define UP_Y (1.0f)
	#define UP_Z (0.0f)

	#define APPLY_FORWARD_SIGN(z) (z)
	#define FORWARD_X (0.0f)
	#define FORWARD_Y (0.0f)
	#define FORWARD_Z (1.0f)

	#define APPLY_RIGHT_SIGN(x) (-(x))
	#define RIGHT_X (-1.0f)
	#define RIGHT_Y (0.0f)
	#define RIGHT_Z (0.0f)

	#define APPLY_DOWN_SIGN(y) (-(y))
	#define DOWN_X (-UP_X)
	#define DOWN_Y (-UP_Y)
	#define DOWN_Z (-UP_Z)

	#define APPLY_BACKWARD_SIGN(z) (-(z))
	#define BACKWARD_X (-FORWARD_X)
	#define BACKWARD_Y (-FORWARD_Y)
	#define BACKWARD_Z (-FORWARD_Z)

	#define APPLY_LEFT_SIGN(x) (x)
	#define LEFT_X (-RIGHT_X)
	#define LEFT_Y (-RIGHT_Y)
	#define LEFT_Z (-RIGHT_Z)

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Vector3D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Vector3D
	{
	public:
		Scalar
			x,
			y,
			z;

		static const Vector3D
			Identity;
		static const Vector3D
			Forward;
		static const Vector3D
			Backward;
		static const Vector3D
			Up;
		static const Vector3D
			Down;
		static const Vector3D
			Left;
		static const Vector3D
			Right;

		//
		// Constructors
		//
		Vector3D()
		{  x = 0.f; y = 0.f; z = 0.f; }
		Vector3D(
			Scalar x,
			Scalar y,
			Scalar z
		):
			x(x),
			y(y),
			z(z)
				{}
		Vector3D(const Vector3D &v):
			x(v.x),
			y(v.y),
			z(v.z)
				{}
		explicit Vector3D(const UnitQuaternion &q)
			{*this = q;}
		explicit Vector3D(const YawPitchRange &p)
			{*this = p;}

		//
		// Assignment operators
		//
		Vector3D&
			operator=(const Vector3D &vector)
				{
					Check_Pointer(this); Check_Object(&vector);
					x = vector.x; y = vector.y; z = vector.z;
					return *this;
				}
		Vector3D&
			operator=(const UnitQuaternion &q);
		Vector3D&
			operator=(const YawPitchRange &p);

		//
		// "Close-enough" comparators
		//
		friend bool
			Small_Enough(
				const Vector3D &v,
				Scalar e=SMALL
			)
				{Check_Object(&v); return v.GetLengthSquared() <= e;}
		bool
			operator!() const
				{return Small_Enough(*this, SMALL);}

		friend bool
			Close_Enough(
				const Vector3D &v1,
				const Vector3D &v2,
				Scalar e=SMALL
			);
		bool
			operator==(const Vector3D& v) const
				{return Close_Enough(*this,v,SMALL);}
		bool
			operator!=(const Vector3D& v) const
				{return !Close_Enough(*this,v,SMALL);}

		void
			TestInstance() const
				{}

		//
		// Indexing operations
		//

		static int GetMemberCount(void)
			{
				return 3;
			}

		const Scalar&
			operator[](size_t index) const
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(index) <= Z_Axis);
					return (&x)[index];
				}
		Scalar&
			operator[](size_t index)
				{
					Check_Pointer(this);
					Verify(static_cast<unsigned>(index) <= Z_Axis);
					return (&x)[index];
				}

		Scalar
			GetForwardComponent()
				{Check_Object(this); return APPLY_FORWARD_SIGN((*this)[FORWARD_AXIS]);}
		Scalar
			GetBackwardComponent()
				{Check_Object(this); return APPLY_BACKWARD_SIGN((*this)[BACKWARD_AXIS]);}
		Scalar
			GetUpComponent()
				{Check_Object(this); return APPLY_UP_SIGN((*this)[UP_AXIS]);}
		Scalar
			GetDownComponent()
				{Check_Object(this); return APPLY_DOWN_SIGN((*this)[DOWN_AXIS]);}
		Scalar
			GetRightComponent()
				{Check_Object(this); return APPLY_RIGHT_SIGN((*this)[RIGHT_AXIS]);}
		Scalar
			GetLeftComponent()
				{Check_Object(this); return APPLY_LEFT_SIGN((*this)[LEFT_AXIS]);}

		//
		// The following operators all assume that this points to the destination
		// of the operation results
		//
		Vector3D&
			Negate(const Vector3D &v)
				{
					Check_Pointer(this); Check_Object(&v);
					x = -v.x; y = -v.y; z = -v.z; return *this;
				}

		Vector3D&
			Add(
				const Vector3D& v1,
				const Vector3D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x + v2.x; y = v1.y + v2.y; z = v1.z + v2.z; return *this;
				}
		Vector3D&
			AddScaled(
				const Vector3D& v1,
				const Vector3D& v2,
				Scalar scale
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x + v2.x*scale; y = v1.y + v2.y*scale;
					z = v1.z + v2.z*scale; return *this;
				}
		Vector3D&
			operator+=(const Vector3D& v)
				{return Add(*this,v);}

		Vector3D&
			Subtract(
				const Vector3D& v1,
				const Vector3D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x - v2.x; y = v1.y - v2.y; z = v1.z - v2.z; return *this;
				}
		Vector3D&
			operator-=(const Vector3D& v)
				{return Subtract(*this,v);}

		Scalar
			operator*(const Vector3D& v) const
         	{
					Check_Object(this); Check_Object(&v);
					return x*v.x + y*v.y + z*v.z;
				}

		Vector3D&
			Cross(
				const Vector3D& v1,
				const Vector3D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					Verify(this != &v1); Verify(this != &v2);
					#if defined(LEFT_HANDED_COORDINATES)
						x = v2.y*v1.z - v2.z*v1.y; y = v2.z*v1.x - v2.x*v1.z;
						z = v2.x*v1.y - v2.y*v1.x; return *this;
					#else
						x = v1.y*v2.z - v1.z*v2.y; y = v1.z*v2.x - v1.x*v2.z;
						z = v1.x*v2.y - v1.y*v2.x; return *this;
					#endif
				}

		Vector3D&
			Cross(
				const Vector3D& p1,	//	v1 = p1 - p2
				const Vector3D& p2,	//	v2 = p3 - p2
				const Vector3D& p3
			)
				{
					Check_Pointer(this); 
					Check_Object(&p1); Check_Object(&p2); Check_Object(&p3);
					Verify(this != &p1); Verify(this != &p2); Verify(this != &p3);
					#if defined(LEFT_HANDED_COORDINATES)
						x = (p3.y - p2.y)*(p1.z - p2.z) - (p3.z - p2.z)*(p1.y - p2.y);
						y = (p3.z - p2.z)*(p1.x - p2.x) - (p3.x - p2.x)*(p1.z - p2.z);
						z = (p3.x - p2.x)*(p1.y - p2.y) - (p3.y - p2.y)*(p1.x - p2.x);
						return *this;
					#else
						x = (p1.y - p2.y)*(p3.z - p2.z) - (p1.z - p2.z)*(p3.y - p2.y);
						y = (p1.z - p2.z)*(p3.x - p2.x) - (p1.x - p2.x)*(p3.z - p2.z);
						z = (p1.x - p2.x)*(p3.y - p2.y) - (p1.y - p2.y)*(p3.x - p2.x);
						return *this;
					#endif
				}

		Vector3D&
			Multiply(
				const Vector3D& v,
				Scalar scale
			)
				{
					Check_Pointer(this); Check_Object(&v);
					x = v.x*scale; y = v.y*scale; z = v.z*scale; return *this;
				}
		Vector3D&
			operator*=(Scalar v)
				{return Multiply(*this,v);}

		Vector3D&
			Multiply(
				const Vector3D& v1,
				const Vector3D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x*v2.x; y = v1.y*v2.y; z = v1.z*v2.z; return *this;
				}
		Vector3D&
			operator*=(const Vector3D &v)
				{return Multiply(*this,v);}

		Vector3D&
			Divide(
				const Vector3D& v,
				Scalar scale
			)
				{
					Check_Pointer(this); Check_Object(&v);
					Verify(!Small_Enough(scale));
					scale = 1.0f / scale;
					x = v.x*scale; y = v.y*scale; z = v.z*scale; return *this;
				}
		Vector3D&
			operator/=(Scalar v)
				{return Divide(*this,v);}

		Vector3D&
			Divide(
				const Vector3D& v1,
				const Vector3D& v2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					Verify(!Small_Enough(v2.x)); Verify(!Small_Enough(v2.y));
					Verify(!Small_Enough(v2.z));
					x = v1.x/v2.x; y = v1.y/v2.y; z = v1.z/v2.z; return *this;
				}
		Vector3D&
			operator/=(const Vector3D &v)
				{return Divide(*this,v);}

		//
		// Transform operators
		//
		Vector3D&
			Multiply(
				const Vector3D &v,
				const AffineMatrix4D &m
			);
		Vector3D&
			operator*=(const AffineMatrix4D &m)
				{Vector3D src(*this); return Multiply(src,m);}
		Vector3D&
			MultiplyByInverse(
				const Vector3D &v,
				const LinearMatrix4D &m
			);

		//
		// Miscellaneous functions
		//
		Scalar
			GetLengthSquared() const
				{Check_Object(this); return x*x + y*y + z*z;}
		Scalar
			GetLength() const
				{return Sqrt(GetLengthSquared());}

		Scalar
			GetApproximateLength() const
				{return SqrtApproximate(GetLengthSquared());}

		Vector3D&
			Normalize(const Vector3D &v)
				{
					Check_Object(this); Check_Object(&v);
					Scalar len = v.GetLength();
					Verify(!Small_Enough(len)); len = 1.0f / len;
					x = v.x*len; y = v.y*len; z = v.z*len; return *this;
				}

		Vector3D&
			Combine(
				const Vector3D& v1,
				Scalar t1,
				const Vector3D& v2,
				Scalar t2
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = t1*v1.x + t2*v2.x; y = t1*v1.y + t2*v2.y;
					z = t1*v1.z + t2*v2.z; return *this;
				}

		void Zero (void)
		{
			x = y = z = 0.0;
		}

		//
		// Template support
		//
		Vector3D&
			Lerp(
				const Vector3D& v1,
				const Vector3D& v2,
				Scalar t
			)
				{
					Check_Pointer(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x + t*(v2.x-v1.x); y = v1.y + t*(v2.y-v1.y);
					z = v1.z + t*(v2.z-v1.z); return *this;
				}

		//
		// Support functions
		//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const Vector3D &vector
				);
		#endif
		static bool
			TestClass();
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ Vector3D functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   void
   	Convert_From_Ascii(
      	const char *str,
         Vector3D *vector_3D
      );

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Vector3D *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Vector3D *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

}
