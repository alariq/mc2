//==========================================================================//
// File:	 gosFX_SpinningCloud.hpp										//
// Contents: SpinningCloud Component										//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#pragma once

#include"gosfx.hpp"
#include"particlecloud.hpp"

namespace gosFX
{
	//############################################################################
	//########################  SpinningCloud__Specification  #############################
	//############################################################################

	class SpinningCloud;

	class SpinningCloud__Specification:
		public ParticleCloud__Specification
	{
		friend class SpinningCloud;

	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		SpinningCloud__Specification(
			Stuff::RegisteredClass::ClassID class_id,
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		SpinningCloud__Specification(Stuff::RegisteredClass::ClassID class_id);

		void
			Copy(SpinningCloud__Specification *spec);

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
		SeededCurveOf<ConstantCurve, LinearCurve,Curve::e_ConstantLinearType>
			m_pSpin;
		SeededCurveOf<ComplexCurve, ComplexCurve,Curve::e_ComplexComplexType>
			m_pScale;

		bool
			m_randomStartingRotation,
			m_alignYUsingVelocity,
			m_alignZUsingX,
			m_alignZUsingY;
	};

	//############################################################################
	//########################  ParticleCloud__Particle  #############################
	//############################################################################

	class SpinningCloud__Particle:
		public ParticleCloud__Particle
	{
	public:
		Stuff::Vector3D
			m_angularVelocity;
		Stuff::Point3D
			m_localTranslation,
			m_worldTranslation;
		Stuff::UnitQuaternion
			m_localRotation,
			m_worldRotation;
		Stuff::Scalar
			m_radius,
			m_scale;
	};

	//############################################################################
	//#############################  SpinningCloud  #################################
	//############################################################################

	class _declspec(novtable) SpinningCloud:
		public ParticleCloud
	{
	//----------------------------------------------------------------------------
	// Class Registration Support
	//
	public:
		static void InitializeClass();
		static void	TerminateClass();

		static ClassData
			*DefaultData;

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	public:
		typedef SpinningCloud__Specification Specification;
		typedef SpinningCloud__Particle Particle;

	protected:
		SpinningCloud(
			ClassData *class_data,
			Specification *spec,
			unsigned flags
		);

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

	public:
		bool
			Execute(ExecuteInfo *info);
	};
}
