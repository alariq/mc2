//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"gosfx.hpp"
#include"spinningcloud.hpp"
#include<mlr/mlr.hpp>

namespace MidLevelRenderer {class MLRCardCloud;}

namespace gosFX
{
	//############################################################################
	//########################  CardCloud__Specification  #############################
	//############################################################################

	class CardCloud__Specification:
		public SpinningCloud__Specification
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		CardCloud__Specification(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		CardCloud__Specification();

		static CardCloud__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);
		void
			Copy(CardCloud__Specification *spec);

		void
			Save(Stuff::MemoryStream *stream);

		void 
			BuildDefaults();
	
		virtual bool 
			IsDataValid(bool fix_data=false);

	//-------------------------------------------------------------------------
	// FCurves
	//
	public:
		SeededCurveOf<ComplexCurve, ComplexCurve,Curve::e_ComplexComplexType>
			m_halfHeight,
			m_aspectRatio;
		SeededCurveOf<ComplexCurve, SplineCurve,Curve::e_ComplexSplineType>
			m_pIndex;
		ConstantCurve
			m_UOffset,
			m_VOffset,
			m_USize,
			m_VSize;

		bool
			m_animated;
		BYTE
			m_width;

		void
			SetWidth();
	};

	//############################################################################
	//########################  SpinningCloud__Particle  #############################
	//############################################################################

	class CardCloud__Particle:
		public SpinningCloud__Particle
	{
	public:
		Stuff::Scalar
			m_halfX,
			m_halfY;
	};

//############################################################################
//#############################  CardCloud  #################################
//############################################################################

	class CardCloud : public SpinningCloud
	{
	//----------------------------------------------------------------------------
	// Class Registration Support
	//
	public:
		static void InitializeClass();
		static void	TerminateClass();

		typedef CardCloud__Specification Specification;
		typedef CardCloud__Particle Particle;

	public:
		enum {
			ParticleSize =
				sizeof(Particle)
				 + 4*sizeof(Stuff::Point3D)
				 + sizeof(Stuff::RGBAColor)
				 + 4*sizeof(Stuff::Vector2DOf<Stuff::Scalar>)
		};

	protected:
		MidLevelRenderer::MLRCardCloud * m_cloudImplementation;						// point to an MLR triangle cloud by Michael
		Stuff::Point3D							*m_P_vertices;
		Stuff::RGBAColor						*m_P_color;
		Stuff::Vector2DOf<Stuff::Scalar>	*m_P_uvs;

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	protected:
		CardCloud(
			Specification *spec,
			unsigned flags
		);

	public:
		~CardCloud();

		static CardCloud*
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
