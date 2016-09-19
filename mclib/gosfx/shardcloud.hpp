//==========================================================================//
// File:	 gosFX_ShardCloud.hpp											//
// Contents: ShardCloud Component											//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#pragma once

#include"gosfx.hpp"
#include"spinningcloud.hpp"
#include<mlr/mlr.hpp>

namespace MidLevelRenderer {class MLRTriangleCloud;}

namespace gosFX
{
	//############################################################################
	//########################  ShardCloud__Specification  #############################
	//############################################################################

	class ShardCloud__Specification:
		public SpinningCloud__Specification
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		ShardCloud__Specification(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		ShardCloud__Specification();

		static ShardCloud__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);
		void
			Copy(ShardCloud__Specification *spec);

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
			m_angularity;
	};

	//############################################################################
	//########################  SpinningCloud__Particle  #############################
	//############################################################################

	class ShardCloud__Particle:
		public SpinningCloud__Particle
	{
	public:
		Stuff::Scalar
			m_angle;
	};

//############################################################################
//#############################  ShardCloud  #################################
//############################################################################

	class ShardCloud : public SpinningCloud
	{
	//----------------------------------------------------------------------------
	// Class Registration Support
	//
	public:
		static void InitializeClass();
		static void	TerminateClass();

		typedef ShardCloud__Specification Specification;
		typedef ShardCloud__Particle Particle;

		enum {
			ParticleSize =
				sizeof(Particle)
				 + 3*sizeof(Stuff::Point3D)
				 + 3*sizeof(Stuff::RGBAColor)
		};

	protected:
		MidLevelRenderer::MLRTriangleCloud * m_cloudImplementation;						// point to an MLR triangle cloud by Michael
		Stuff::Point3D			*m_P_vertices;
		Stuff::RGBAColor		*m_P_color;

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	protected:
		ShardCloud(
			Specification *spec,
			unsigned flags
		);

	public:
		~ShardCloud();

		static ShardCloud*
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
