//==========================================================================//
// File:	 gosFX_PertCloud.hpp											//
// Contents: PertCloud Component											//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#pragma once

#include"gosfx.hpp"
#include"spinningcloud.hpp"
#include<mlr/mlr.hpp>

namespace MidLevelRenderer {class MLRNGonCloud;}

namespace gosFX
{
	//############################################################################
	//########################  PertCloud__Specification  #############################
	//############################################################################

	class PertCloud__Specification:
		public SpinningCloud__Specification
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		PertCloud__Specification(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		PertCloud__Specification(unsigned sides);

		static PertCloud__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		void
			Copy(PertCloud__Specification *spec);

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
		SeededCurveOf<ComplexCurve, SplineCurve,Curve::e_ComplexSplineType>
			m_size;
		SeededCurveOf<ConstantCurve, SplineCurve,Curve::e_ConstantSplineType>
			m_perturbation;
		SeededCurveOf<ComplexCurve, LinearCurve,Curve::e_ComplexLinearType>
			m_pCenterRed,
			m_pCenterGreen,
			m_pCenterBlue,
			m_pCenterAlpha;

		unsigned
			m_vertices;
	};


	//############################################################################
	//########################  SpinningCloud__Particle  #############################
	//############################################################################

	class PertCloud__Particle:
		public SpinningCloud__Particle
	{
	public:
		Stuff::Point3D
			m_vertices[MidLevelRenderer::Limits::Max_Number_Of_NGon_Vertices];
	};

//############################################################################
//#############################  PertCloud  ##################################
//############################################################################

	class PertCloud : public SpinningCloud
	{
	//----------------------------------------------------------------------------
	// Class Registration Support
	//
	public:
		static void InitializeClass();
		static void	TerminateClass();

		typedef PertCloud__Specification Specification;
		typedef PertCloud__Particle Particle;

	protected:
		MidLevelRenderer::MLRNGonCloud * m_cloudImplementation;						// Pert to an MLR Pert cloud by Jim
		Stuff::Point3D		*m_P_vertices;
		Stuff::RGBAColor	*m_P_color;

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	protected:
		PertCloud(
			Specification *spec,
			unsigned flags
		);

	public:
		~PertCloud();

		static PertCloud*
			Make(
				Specification *spec,
				unsigned flags
			);

		Specification*
			GetSpecification()
				{
					Check_Object(this);
					return
						Cast_Object(Specification*, m_specification);
				}
		Particle*
			GetParticle(unsigned index)
				{
					Check_Object(this); Check_Object(GetSpecification());
					return
						Cast_Pointer(
							Particle*,
							&m_data[index*GetSpecification()->m_particleClassSize]
						);
				}

		static ClassData
			*DefaultData;

	//----------------------------------------------------------------------------
	// Testing
	//
	public:
		void
			TestInstance() const;

	//----------------------------------------------------------------------------
	// API
	//
	protected:
		bool
			AnimateParticle(
				unsigned index,
				const Stuff::LinearMatrix4D *world_to_new_local,
				Stuff::Time till
			);
		void
			CreateNewParticle(
				unsigned index,
				Stuff::Point3D *translation
			);
		void
			DestroyParticle(unsigned index);

	public:
		void
			Draw(DrawInfo *info);
	};
}
