//==========================================================================//
// File:	 gosFX_DebrisCloud.hpp											//
// Contents: DebrisCloud Component											//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#pragma once

#include"gosfx.hpp"
#include"spinningcloud.hpp"
#include<mlr/mlr.hpp>

namespace MidLevelRenderer {class MLRShape;}

namespace gosFX
{
	//############################################################################
	//#######################  DebrisCloud__Specification  #######################
	//############################################################################

	class DebrisCloud;

	class DebrisCloud__Specification:
		public Effect__Specification
	{
		friend class DebrisCloud;

	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		DebrisCloud__Specification(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		DebrisCloud__Specification();
		~DebrisCloud__Specification();

		void
			Save(Stuff::MemoryStream *stream);

		static DebrisCloud__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		void
			Copy(DebrisCloud__Specification *spec);

		void
			Load(Stuff::MemoryStream *stream);

		void
			LoadGeometry(Stuff::MemoryStream *stream);

		void 
			BuildDefaults();
	
		virtual bool 
			IsDataValid(bool fix_data=false);

	//-------------------------------------------------------------------------
	// FCurves
	//
	public:
		ConstantCurve
			m_minimumDeviation;
		SplineCurve
			m_maximumDeviation;
		SeededCurveOf<ComplexCurve, ComplexCurve,Curve::e_ComplexComplexType>
			m_startingSpeed;
		SeededCurveOf<ComplexCurve, SplineCurve,Curve::e_ComplexSplineType>
			m_pLifeSpan;
		SeededCurveOf<ConstantCurve, LinearCurve,Curve::e_ConstantLinearType>
			m_pEtherVelocityY;
		SeededCurveOf<SplineCurve, LinearCurve,Curve::e_SplineLinearType>
			m_pAccelerationY;
		SeededCurveOf<ComplexCurve, ComplexCurve,Curve::e_ComplexComplexType>
			m_pDrag;
		SeededCurveOf<ComplexCurve, LinearCurve,Curve::e_ComplexLinearType>
			m_pAlpha;
		SeededCurveOf<ConstantCurve, LinearCurve,Curve::e_ConstantLinearType>
			m_pSpin;

	//-------------------------------------------------------------------------
	// Data
	//
		Stuff::Point3D
			centerOfForce;

		Stuff::DynamicArrayOf<MidLevelRenderer::MLRShape*>
			debrisPieces;
		Stuff::DynamicArrayOf<Stuff::Point3D>
			debrisPositions;
		Stuff::DynamicArrayOf<Stuff::Sphere>
			debrisSpheres;
		Stuff::DynamicArrayOf<Stuff::Scalar>
			debrisSeed;
	};

	//############################################################################
	//#########################  DebrisCloud__Particle  #########################
	//############################################################################

	class DebrisCloud__Particle
	{
	public:
		Stuff::LinearMatrix4D
			m_localToParent;

		Stuff::Scalar
			m_age,
			m_ageRate,
			m_seed;

		Stuff::Vector3D
			m_angularVelocity,
			m_linearVelocity;

		Stuff::Scalar
			m_alpha;

		void
			TestInstance() const
				{}
	};

//############################################################################
//#############################  DebrisCloud  ################################
//############################################################################

	class DebrisCloud : public Effect
	{
	//----------------------------------------------------------------------------
	// Class Registration Support
	//
	public:
		static void InitializeClass();
		static void	TerminateClass();

		typedef DebrisCloud__Specification Specification;
		typedef DebrisCloud__Particle Particle;

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	protected:
		DebrisCloud(
			Specification *spec,
			unsigned flags
		);

	public:
		static DebrisCloud*
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
							&debrisPieces[index]
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
			Execute(ExecuteInfo *info);
		bool
			AnimateParticle(
				unsigned index,
				const Stuff::LinearMatrix4D *world_to_new_local,
				Stuff::Time till
			);
		virtual void
			DestroyParticle(unsigned index);
		void
			ComputeNewLinearVelocity(
				Particle *particle,
				Stuff::Scalar time_slice
			);

	public:
		void
			Start(ExecuteInfo *info);
		void
			Kill();
		bool
			HasFinished();
		void
			Draw(DrawInfo *info);

	protected:
		Stuff::DynamicArrayOf<DebrisCloud__Particle>
			debrisPieces;
	};
}
