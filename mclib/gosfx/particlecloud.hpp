//==========================================================================//
// File:	 gosFX_ParticleCloud.hpp										//
// Contents: Base ParticleCloud Particle									//
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
	//########################  ParticleCloud__Specification  #############################
	//############################################################################

	class ParticleCloud__Specification:
		public Effect__Specification
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		ParticleCloud__Specification(
			Stuff::RegisteredClass::ClassID class_id,
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		ParticleCloud__Specification(Stuff::RegisteredClass::ClassID class_id);

		void
			Copy(ParticleCloud__Specification *spec);

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
		ConstantCurve
			m_startingPopulation;
		ComplexCurve
			m_particlesPerSecond;
		LinearCurve
			m_emitterSizeX,
			m_emitterSizeY,
			m_emitterSizeZ;
		ConstantCurve
			m_minimumDeviation;
		SplineCurve
			m_maximumDeviation;
		SeededCurveOf<ComplexCurve, ComplexCurve,Curve::e_ComplexComplexType>
			m_startingSpeed;
		SeededCurveOf<ComplexCurve, SplineCurve,Curve::e_ComplexSplineType>
			m_pLifeSpan;
		SeededCurveOf<ConstantCurve, LinearCurve,Curve::e_ConstantLinearType>
			m_pEtherVelocityX,
			m_pEtherVelocityY,
			m_pEtherVelocityZ;
		SeededCurveOf<SplineCurve, LinearCurve,Curve::e_SplineLinearType>
			m_pAccelerationX,
			m_pAccelerationY,
			m_pAccelerationZ;
		SeededCurveOf<ComplexCurve, ComplexCurve,Curve::e_ComplexComplexType>
			m_pDrag;
		SeededCurveOf<ComplexCurve, LinearCurve,Curve::e_ComplexLinearType>
			m_pRed,
			m_pGreen,
			m_pBlue,
			m_pAlpha;

	//----------------------------------------------------------------------
	// Data
	//
	public:
		int
			m_maxParticleCount,
			m_totalParticleSize,
			m_particleClassSize;
	};

	//############################################################################
	//########################  ParticleCloud__Particle  #############################
	//############################################################################

	class ParticleCloud__Particle
	{
	public:
		Stuff::Vector3D
			m_localLinearVelocity,
			m_worldLinearVelocity;
		Stuff::Scalar
			m_seed,
			m_age,
			m_ageRate;

		void
			TestInstance() const
				{}
	};

//############################################################################
//##############################  ParticleCloud  #############################
//############################################################################

	class _declspec(novtable) ParticleCloud:
		public Effect
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		static ClassData
			*DefaultData;

		typedef ParticleCloud__Specification Specification;
		typedef ParticleCloud__Particle Particle;

	protected:
		int
			m_activeParticleCount;
		Stuff::Scalar		
			m_birthAccumulator;

		Stuff::DynamicArrayOf<char>
			m_data;

		ParticleCloud(
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
		virtual bool
			AnimateParticle(
				unsigned index,
				const Stuff::LinearMatrix4D *world_to_new_local,
				Stuff::Time till
			)=0;
		virtual void
			CreateNewParticle(
				unsigned index,
				Stuff::Point3D *translation
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
	};
}
