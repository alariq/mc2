//===========================================================================//
// File:	vector.hh                                                        //
// Contents: Interface specification for vector classes                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"vector3d.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Vector2D ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class Vector2DOf;

	template <class T> bool
		Small_Enough(const Vector2DOf<T> &v,Scalar e = SMALL);

	template <class T> bool
		Close_Enough(
			const Vector2DOf<T> &v1,
			const Vector2DOf<T> &v2,
			Scalar e
		);
// sebi: forward declaration
#if !defined(Spew)
	template <class T> void
		Spew(
			const char* group,
			const Stuff::Vector2DOf<T> &vector
		);
#endif

	template <class T> class Vector2DOf
	{
	 public:
//		static const Vector2DOf<T>
//			Identity;

		T
			x,
			y;

		Vector2DOf()
			{}
		Vector2DOf(
			T x,
			T y
		):
			x(x),
			y(y)
				{}
		Vector2DOf(const Vector2DOf<T> &v)
			{x = v.x; y = v.y;}

		friend bool
			Small_Enough<T>(
				const Vector2DOf<T> &v,
				Scalar e
			);
		bool
			operator!() const
				{return Small_Enough(*this,SMALL);}

		friend bool
			Close_Enough<T>(
				const Vector2DOf<T> &v1,
				const Vector2DOf<T> &v2,
				Scalar e
			);
		bool
			operator==(const Vector2DOf<T>& v) const
				{return Close_Enough(*this,v,SMALL);}
		bool
			operator!=(const Vector2DOf<T>& v) const
				{return !Close_Enough(*this,v,SMALL);}

		const T&
			operator[](size_t index) const
				{
					Check_Object(this);
					Verify(static_cast<unsigned>(index) <= Y_Axis);
					return (&x)[index];
				}
		T&
			operator[](size_t index)
				{
					Check_Object(this);
					Verify(static_cast<unsigned>(index) <= Y_Axis);
					return (&x)[index];
				}

		//
		//-----------------------------------------------------------------------
		// The following operators all assume that this points to the destination
		// of the operation results
		//-----------------------------------------------------------------------
		//
		Vector2DOf<T>&
			Negate(const Vector2DOf<T> &v)
				{
					Check_Object(this); Check_Object(&v);
					x = -v.x; y = -v.y; return *this;
				}

		Vector2DOf<T>&
			Add(
				const Vector2DOf<T>& v1,
				const Vector2DOf<T>& v2
			)
				{
					Check_Object(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x + v2.x; y = v1.y + v2.y; return *this;
				}

		Vector2DOf<T>&
			operator+=(const Vector2DOf<T>& v)
				{return Add(*this,v);}

		Vector2DOf<T>&
			Subtract(
				const Vector2DOf<T>& v1,
				const Vector2DOf<T>& v2
			)
				{
					Check_Object(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x - v2.x; y = v1.y - v2.y; return *this;
				}

		Vector2DOf<T>&
			operator-=(const Vector2DOf<T>& v)
				{return Subtract(*this,v);}

		T
			operator*(const Vector2DOf<T>& v) const
				{Check_Object(this); Check_Object(&v); return x*v.x + y*v.y;}

		Vector2DOf<T>&
			Multiply(
				const Vector2DOf<T>& v,
				T scale
			)
				{
					Check_Object(this); Check_Object(&v);
					x = v.x * scale; y = v.y * scale; return *this;
				}

		Vector2DOf<T>&
			operator*=(Scalar v)
				{return Multiply(*this,v);}

		Vector2DOf<T>&
			Multiply(
				const Vector2DOf<T>& v1,
				const Vector2DOf<T>& v2
			)
				{
					Check_Object(this); Check_Object(&v1); Check_Object(&v2);
					x = v1.x * v2.x; y = v1.y * v2.y; return *this;
				}

		Vector2DOf<T>&
			operator*=(const Vector2DOf<T> &v)
				{return Multiply(*this,v);}

		Vector2DOf<T>&
			Divide(
				const Vector2DOf<T>& v,
				T scale
			)
				{
					Check_Object(this); Check_Object(&v);
					Verify(!Small_Enough(static_cast<Scalar>(scale)));
					x = v.x / scale; y = v.y / scale; return *this;
				}

		Vector2DOf<T>&
			operator/=(T v)
				{return Divide(*this,v);}

		Vector2DOf<T>&
			Divide(
				const Vector2DOf<T>& v1,
				const Vector2DOf<T>& v2
			)
				{
					Check_Object(this); Check_Object(&v1); Check_Object(&v2);
					Verify(!Small_Enough(static_cast<Scalar>(v2.x)));
					Verify(!Small_Enough(static_cast<Scalar>(v2.y)));
					x = v1.x / v2.x; y = v1.y / v2.y; return *this;
				}

		Vector2DOf<T>&
			operator/=(const Vector2DOf<T> &v)
				{return Divide(*this,v);}

		T
			GetLengthSquared() const
				{Check_Object(this); return operator*(*this);}
		Scalar
			GetLength() const
				{
					Check_Object(this);
					return Sqrt(static_cast<Scalar>(GetLengthSquared()));
				}

		Scalar
			GetApproximateLength() const
				{
					Check_Object(this);
					return SqrtApproximate(static_cast<Scalar>(GetLengthSquared()));
				}

		Vector2DOf<T>&
			Normalize(const Vector2DOf<T> &v)
				{
					Check_Pointer(this); Check_Object(&v);
					Scalar len = v.GetLength(); Verify(!Small_Enough(len));
					x = v.x/len; y = v.y/len; return *this;
				}

		Vector2DOf<T>&
			Combine(
				const Vector2DOf<T>& v1,
				Scalar t1,
				const Vector2DOf<T>& v2,
				Scalar t2
			)
				{
					Check_Object(this); Check_Object(&v1); Check_Object(&v2);
					x = static_cast<T>(v1.x*t1 + v2.x*t2);
					y = static_cast<T>(v1.y*t1 + v2.y*t2); return *this;
				}

		void Zero (void)
		{
			x = y = 0;
		}

		Vector2DOf<T>&
			Lerp(
				const Vector2DOf<T>& v1,
				const Vector2DOf<T>& v2,
				Scalar t
			)
				{
					Check_Object(this); Check_Object(&v1); Check_Object(&v2);
					x = static_cast<T>(v1.x + t*(v2.x-v1.x));
					y = static_cast<T>(v1.y + t*(v2.y-v1.y)); return *this;
				}

		#if !defined(Spew)
			friend void
				Spew<T>(
					const char* group,
					const Vector2DOf<T>& vector
				);
		#endif
		void
			TestInstance() const
				{}
	};

//	template <class T> const Vector2DOf<T>
//		Vector2DOf<T>::Identity(0.0f,0.0f);

	template <class T> bool
		Small_Enough(const Vector2DOf<T> &v,Scalar e)
	{
		Check_Object(&v);
		return
			Small_Enough(static_cast<Scalar>(v.x),e)
			 && Small_Enough(static_cast<Scalar>(v.y),e);
	}

	template <class T> bool
		Close_Enough(
			const Vector2DOf<T> &v1,
			const Vector2DOf<T> &v2,
			Scalar e
		)
	{
		Check_Object(&v1);
		Check_Object(&v2);
		return
			Close_Enough(static_cast<Scalar>(v1.x), static_cast<Scalar>(v2.x), e)
			 && Close_Enough(static_cast<Scalar>(v1.y), static_cast<Scalar>(v2.y), e);
	}

#if !defined(Spew)
	template <class T> void
		Spew(
			const char* group,
			const Stuff::Vector2DOf<T> &vector
		)
	{
	    //Check_Object(this);
		SPEW((group, "<+"));
		Spew(group, vector.x);
		SPEW((group, ",+"));
		Spew(group, vector.y);
		SPEW((group, ">+"));
	}
#endif

}
