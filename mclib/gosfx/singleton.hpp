//==========================================================================//
// File:	 gosFX_Singleton.hpp									    	//
// Contents: Base Singleton Particle									    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#pragma once

#include"gosfx.hpp"
#include"effect.hpp"

namespace gosFX
{
	//############################################################################
	//########################  Singleton__Specification  #############################
	//############################################################################

	class Singleton__Specification:
		public Effect__Specification
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		Singleton__Specification(
			Stuff::RegisteredClass::ClassID class_id,
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		Singleton__Specification(Stuff::RegisteredClass::ClassID class_id);

		void
			Copy(Singleton__Specification *spec);

		void
			Save(Stuff::MemoryStream *stream);

		void 
			BuildDefaults();

		bool 
			IsDataValid(bool fix_data=false);

	//-------------------------------------------------------------------------
	// FCurves
	//
	public:
		SeededCurveOf<ComplexCurve, LinearCurve,Curve::e_ComplexLinearType>
			m_red,
			m_green,
			m_blue,
			m_alpha;
		SeededCurveOf<ComplexCurve, ComplexCurve,Curve::e_ComplexComplexType>
			m_scale;

		bool
			m_alignZUsingX,
			m_alignZUsingY;
	};

	//############################################################################
	//##############################  Singleton  #############################
	//############################################################################

	class _declspec(novtable) Singleton:
		public Effect
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		static ClassData
			*DefaultData;

		typedef Singleton__Specification Specification;

	protected:
		Stuff::DynamicArrayOf<char>
			m_data;

		Singleton(
			ClassData *class_data,
			Specification *spec,
			unsigned flags
		);

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	public:
		Specification*
			GetSpecification()
				{
					Check_Object(this);
					return
						Cast_Object(Specification*, m_specification);
				}

	//----------------------------------------------------------------------------
	// API
	//
	public:
		bool
			Execute(ExecuteInfo *info);

	protected:
		Stuff::RGBAColor
			m_color;
		Stuff::Scalar
			m_radius,
			m_scale;

	//----------------------------------------------------------------------------
	// Testing
	//
	public:
		void
			TestInstance() const;
	};
}
