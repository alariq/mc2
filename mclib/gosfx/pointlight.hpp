//==========================================================================//
// File:	 gosFX_PointLight.hpp									    	//
// Contents: Base PointLight Particle									    //
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
	//############################  LightManager  ################################
	//############################################################################

	class Light;
	class LightManager
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		static LightManager* Instance;

		virtual Light*
			MakePointLight(const char* light_map=NULL);

		struct Info {
			Stuff::RGBColor
				m_color;
			Stuff::LinearMatrix4D
				m_origin;
			Stuff::Scalar
				m_intensity,
				m_inner,
				m_outer;
			Stuff::Radian
				m_spread;
		};

		virtual void
			ChangeLight(
				Light *light,
				Info *info
			);

		virtual void
			DeleteLight(Light *light);

		void
			TestInstance() const
				{}
	};

	//############################################################################
	//####################  PointLight__Specification  #########################
	//############################################################################

	class PointLight__Specification:
		public Effect__Specification
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		PointLight__Specification(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		PointLight__Specification();

		void
			Copy(PointLight__Specification *spec);

		void
			Save(Stuff::MemoryStream *stream);

		void 
			BuildDefaults();

		bool 
			IsDataValid(bool fix_data=false);

		static PointLight__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

	//-------------------------------------------------------------------------
	// FCurves
	//
	public:
		ComplexCurve
			m_red,
			m_green,
			m_blue,
			m_intensity;
		SplineCurve
			m_innerRadius,
			m_outerRadius;

		bool
			m_twoSided;
		Stuff::MString
			m_lightMap;
	};

	//############################################################################
	//##############################  PointLight  #############################
	//############################################################################

	class PointLight:
		public Effect
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		static ClassData
			*DefaultData;

		typedef PointLight__Specification Specification;

		static PointLight*
			Make(
				Specification *spec,
				unsigned flags
			);

		~PointLight();

	protected:
		PointLight(
			Specification *spec,
			unsigned flags
		);

		Light
			*m_light;

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
		void
			Start(ExecuteInfo *info);
		bool
			Execute(ExecuteInfo *info);
		void
			Kill();

	//----------------------------------------------------------------------------
	// Testing
	//
	public:
		void
			TestInstance() const;
	};
}
