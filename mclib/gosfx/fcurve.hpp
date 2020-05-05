//===========================================================================//
// File:	fcurve.hpp                                                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"gosfx.hpp"

namespace gosFX {

	class Curve
	{
	public:
		enum CurveType
		{
			e_ConstantType,
			e_LinearType,
			e_SplineType,
			e_ComplexType,
			e_ComplexLinearType,
			e_ComplexComplexType,
			e_ComplexSplineType,
			e_ConstantComplexType,
			e_ConstantLinearType,
			e_ConstantSplineType,
			e_SplineLinearType,
			e_SplineSplineType
		};

		const CurveType
			m_type;

		Curve(CurveType type):
			m_type(type)
				{}

		Stuff::Scalar 
			ExpensiveCompute(Stuff::Scalar tme,int curvenum=0);

		void 
			ExpensiveComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi,
				int curvenum
				);

		void 
			ExpensiveComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi
				);

		void
			Save(Stuff::MemoryStream *stream);
		void
			Load(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		Stuff::Scalar 
			Mid(int curvenum=0);

		bool
			IsComplex();
		
		void 
			SetSeedFlagIfComplex(bool vflag);

		int 
			GetSeedFlagIfComplex();

		void 
			TranslateTo(Stuff::Scalar pos,int curvenum=0);

		void 
			TranslateBy(Stuff::Scalar delta,int curvenum=0);

		void 
			LocalScale(Stuff::Scalar sfactor,int curvenum=0);

		void 
			AxisScale(Stuff::Scalar sfactor,Stuff::Scalar axis=0.0f,int curvenum=0);

		Curve * 
			GetSubCurve(int curvenum);
		
		void
			TestInstance() const
				{}
	};

	//######################################################################
	//####################    ConstantCurve    #############################
	//######################################################################

	class ConstantCurve:
		public Curve
	{
	public:
		Stuff::Scalar
			m_value;

		ConstantCurve(CurveType type=e_ConstantType):
			Curve(type)
				{}

		ConstantCurve&
			operator=(const ConstantCurve& curve)
				{m_value = curve.m_value; return *this;}

		void
			Save(Stuff::MemoryStream *stream);
		void
			Load(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		//------------------------------------------------------
		// for Set...Key(), true=math good, false=math unstable
		//------------------------------------------------------
		bool
			SetCurve(Stuff::Scalar v)
				{Check_Object(this); m_value = v; return true;}

		Stuff::Scalar
			ComputeValue(Stuff::Scalar, Stuff::Scalar)
				{Check_Object(this); return m_value;}
		Stuff::Scalar
			ComputeSlope(Stuff::Scalar)
				{Check_Object(this); return 0.0f;}

		void
			ComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi
			)
				{
					Check_Object(this); Check_Pointer(low); Check_Pointer(hi);
					*hi = *low = m_value;
				}
	};

	//######################################################################
	//#####################    LinearCurve    ##############################
	//######################################################################

	class LinearCurve:
		public ConstantCurve
	{
	public:
		Stuff::Scalar
			m_slope;

		LinearCurve(CurveType type=e_LinearType):
			ConstantCurve(type)
				{}

		LinearCurve&
			operator=(const LinearCurve& curve)
				{m_slope = curve.m_slope; m_value = curve.m_value; return *this;}

		void
			Save(Stuff::MemoryStream *stream);
		void
			Load(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		//------------------------------------------------------
		// for Set...Key(), true=math good, false=math unstable
		//------------------------------------------------------
		bool
			SetCurve(Stuff::Scalar v);
		bool
			SetCurve(
				Stuff::Scalar v0,
				Stuff::Scalar v1
			);

		Stuff::Scalar
			ComputeValue(Stuff::Scalar t, Stuff::Scalar)
				{Check_Object(this); return m_slope*t + m_value;}
		Stuff::Scalar
			ComputeSlope(Stuff::Scalar)
				{Check_Object(this); return m_slope;}

		void
			ComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi
			);
	};

	//######################################################################
	//######################    SplineCurve    ##############################
	//######################################################################

	class SplineCurve:
		public LinearCurve
	{
	public:
		Stuff::Scalar
			m_a,
			m_b;

		SplineCurve():
			LinearCurve(e_SplineType)
				{}

		SplineCurve&
			operator=(const SplineCurve& curve);

		void
			Save(Stuff::MemoryStream *stream);
		void
			Load(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		//------------------------------------------------------
		// for Set...Key(), true=math good, false=math unstable
		//------------------------------------------------------
		bool
			SetCurve(Stuff::Scalar v);
		bool
			SetCurve(
				Stuff::Scalar v0,
				Stuff::Scalar v1
			);
		bool
			SetCurve(
				Stuff::Scalar v0,
				Stuff::Scalar s0,
				Stuff::Scalar v1,
				Stuff::Scalar s1
			);

		Stuff::Scalar
			ComputeValue(Stuff::Scalar t, Stuff::Scalar)
				{Check_Object(this); return ((m_a*t + m_b)*t + m_slope)*t + m_value;}
		Stuff::Scalar
			ComputeSlope(Stuff::Scalar t)
				{Check_Object(this); return (3.0f*m_a*t + 2.0f*m_b)*t + m_slope;}

		void
			ComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi
			);
	};

	//######################################################################
	//######################    CurveKey    ###############################
	//######################################################################

	class CurveKey
	{
	public:
		Stuff::Scalar
			m_time,
			m_slope,
			m_value;

		//------------------------------------------------------
		// for Set...Key(), true=math good, false=math unstable
		//------------------------------------------------------
		bool
			SetConstantKey(
				Stuff::Scalar key_time,
				Stuff::Scalar v
			);
		bool
			SetLinearKey(
				Stuff::Scalar key_time,
				Stuff::Scalar v0,
				Stuff::Scalar v1,
				Stuff::Scalar dt
			);

		Stuff::Scalar
			ComputeValue(Stuff::Scalar t)
				{Check_Object(this); return m_slope*t + m_value;}
		Stuff::Scalar
			ComputeSlope(Stuff::Scalar)
				{Check_Object(this); return m_slope;}

		void
			ComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi,
				Stuff::Scalar dt
			);

		void
			TestInstance() const
				{}
	};

	//######################################################################
	//########################    ComplexCurve    ################################
	//######################################################################

	class ComplexCurve:
		public Curve
	{
	protected:
		Stuff::DynamicArrayOf<CurveKey>
			m_keys;

	public:
		ComplexCurve();
		ComplexCurve(const ComplexCurve &fcurve);
		ComplexCurve(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

		ComplexCurve&
			operator=(const ComplexCurve &fcurve);

		void
			Save(Stuff::MemoryStream *stream);
		void
			Load(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		//-----------------------------------------------------------------
		// Warning:  both index of and pointer to CurveKey's are volitile
		//           client code should store neither between transactions
		//-----------------------------------------------------------------

		CurveKey&
			operator[](int index)
				{Check_Object(this); return m_keys[index];}
		int
			GetKeyCount()
				{Check_Object(this); return (int)m_keys.GetLength();}
		int
			GetKeyIndex(Stuff::Scalar time)
				{
					Check_Object(this);
					int i = 0;
					const int len = (int)m_keys.GetLength();
					for (; i<len; ++i)
						if (m_keys[i].m_time > time)
							break;
					return --i;
				}

		int
			InsertKey(Stuff::Scalar time);
		void
			DeleteKey(int index);

		void
			SetCurve(Stuff::Scalar v);
		void
			SetCurve(
				Stuff::Scalar starting_value,
				Stuff::Scalar ending_value
			);

		Stuff::Scalar
			ComputeValue(Stuff::Scalar time, Stuff::Scalar)
				{
					Check_Object(this);
					CurveKey &key = (*this)[GetKeyIndex(time)];
					return key.ComputeValue(time - key.m_time);
				}
		Stuff::Scalar
			ComputeSlope(Stuff::Scalar time)
				{
					Check_Object(this);
					CurveKey &key = (*this)[GetKeyIndex(time)];
					return key.ComputeSlope(time - key.m_time);
				}

		void
			ComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi
			);
	};

	//######################################################################
	//#####################    SeededCurveOf    ############################
	//######################################################################

	template <class C, class S,Curve::CurveType type> class SeededCurveOf:
		public Curve
	{
	public:
		C
			m_ageCurve;
		S
			m_seedCurve;
		bool
			m_seeded;

		SeededCurveOf():
			Curve(type)
				{}

		SeededCurveOf<C,S,type>&
			operator=(const SeededCurveOf<C,S,type>& curve);
		void
			Save(Stuff::MemoryStream *stream);
		void
			Load(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		Stuff::Scalar
			ComputeValue(
				Stuff::Scalar age,
				Stuff::Scalar seed
			)
				{
					Check_Object(this);
					Stuff::Scalar result = m_ageCurve.ComputeValue(age, 0.0f);
					if (m_seeded)
						result *= m_seedCurve.ComputeValue(seed, 0.0f);
					return result;
				}
		void
			ComputeRange(
				Stuff::Scalar *low,
				Stuff::Scalar *hi
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const
				{}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class C, class S,Curve::CurveType type> SeededCurveOf<C,S,type>&
		SeededCurveOf<C,S,type>::operator=(const SeededCurveOf<C,S,type>& curve)
	{
		m_ageCurve = curve.m_ageCurve;
		m_seedCurve = curve.m_seedCurve;
		m_seeded = curve.m_seeded;
		return *this;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class C, class S,Curve::CurveType type> void
		SeededCurveOf<C,S,type>::Save(Stuff::MemoryStream *stream)
	{
		m_ageCurve.Save(stream);
		m_seedCurve.Save(stream);
		*stream << m_seeded;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class C, class S,Curve::CurveType type> void
		SeededCurveOf<C,S,type>::Load(
			Stuff::MemoryStream *stream,
			int gfx_version
		)
	{
		m_ageCurve.Load(stream, gfx_version);
		m_seedCurve.Load(stream, gfx_version);
		*stream >> m_seeded;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class C, class S,Curve::CurveType type> void
		SeededCurveOf<C,S,type>::ComputeRange(
			Stuff::Scalar *low,
			Stuff::Scalar *hi
		)
	{
		Stuff::Scalar low_age, hi_age;
		m_ageCurve.ComputeRange(&low_age, &hi_age);
		Stuff::Scalar low_seed, hi_seed;
		m_seedCurve.ComputeRange(&low_seed, &hi_seed);
		*low = *hi = low_age*low_seed;
		Stuff::Scalar temp = low_age*hi_seed;
		if (temp<*low)
			*low = temp;
		else if (temp>*hi)
			*hi = temp;
		temp = hi_age*low_seed;
		if (temp<*low)
			*low = temp;
		else if (temp>*hi)
			*hi = temp;
		temp = hi_age*hi_seed;
		if (temp<*low)
			*low = temp;
		else if (temp>*hi)
			*hi = temp;
	}
}
