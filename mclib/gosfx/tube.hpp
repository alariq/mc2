//==========================================================================//
// File:	 gosFX_Tube.hpp										            //
// Contents: Base Tube Profile									            //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#pragma once

#include"gosfx.hpp"
#include"effect.hpp"

namespace MidLevelRenderer {class MLRIndexedTriangleCloud;}

namespace gosFX
{
	//############################################################################
	//########################  Tube__Specification  #############################
	//############################################################################

	class Tube__Specification:
		public Effect__Specification
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		Tube__Specification(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		Tube__Specification();

		static Tube__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		void
			Copy(Tube__Specification *spec);

		void
			Save(Stuff::MemoryStream *stream);
		
		void 
			BuildDefaults();

		bool 
			IsDataValid(bool fix_data=false);

		bool
			CalculateUBias(bool adjust);

	//-------------------------------------------------------------------------
	// FCurves
	//
	public:
		ConstantCurve
			m_profilesPerSecond,
			m_pLifeSpan;
		LinearCurve
			m_emitterSizeX,
			m_emitterSizeY,
			m_emitterSizeZ;
		ConstantCurve
			m_minimumDeviation;
		ComplexCurve
			m_maximumDeviation;
		SeededCurveOf<ComplexCurve, SplineCurve,Curve::e_ComplexSplineType>
			m_pDisplacement,
			m_pScale;
		SeededCurveOf<ComplexCurve, LinearCurve,Curve::e_ComplexLinearType>
			m_pRed,
			m_pGreen,
			m_pBlue,
			m_pAlpha;
		SeededCurveOf<ComplexCurve, SplineCurve,Curve::e_ComplexSplineType>
			m_pUOffset,
			m_pVOffset;
		SeededCurveOf<SplineCurve, SplineCurve,Curve::e_SplineSplineType>
			m_pUSize,
			m_pVSize;

	//----------------------------------------------------------------------
	// Data
	//
	public:
		int
			m_maxProfileCount;

		enum ProfileType {
			e_Ribbon,
			e_AlignedRibbon,
			e_Triangle,
			e_Square,
			e_Cross,
			e_Pentagon,
			e_Hexagon,
			e_VerticalRibbon
		}
			m_profileType;
		bool
			m_insideOut;

		Stuff::DynamicArrayOf<Stuff::Point3D>
			m_vertices;
		Stuff::DynamicArrayOf<Stuff::Vector2DOf<Stuff::Scalar> >
			m_uvs;

		Stuff::Scalar
			m_UBias;

		void
			BuildTemplate();
	};

	//############################################################################
	//########################  Tube__Profile  #############################
	//############################################################################

	class Tube__Profile
	{
	public:
		Stuff::LinearMatrix4D
			m_profileToWorld;
		Stuff::UnitVector3D
			m_direction;
		Stuff::Scalar
			m_age,
			m_ageRate,
			m_seed;

		void
			TestInstance() const
				{}
	};

	//############################################################################
	//##############################  Tube  #############################
	//############################################################################

	class Tube:
		public Effect
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		static ClassData
			*DefaultData;

		typedef Tube__Specification Specification;
		typedef Tube__Profile Profile;

	protected:
		int
			m_headProfile,
			m_tailProfile,
			m_activeProfileCount,
			m_triangleCount,
			m_vertexCount;
		Stuff::Scalar
			m_birthAccumulator;

		Stuff::DynamicArrayOf<Profile>
			m_profiles;
		Stuff::DynamicArrayOf<char>
			m_data;
		MidLevelRenderer::MLRIndexedTriangleCloud
			*m_mesh;
		Stuff::Point3D
			*m_P_vertices;
		Stuff::RGBAColor
			*m_P_colors;
		Stuff::Vector2DOf<Stuff::Scalar>
			*m_P_uvs;

		void
			BuildMesh(unsigned short *indices);

		Tube(
			Specification *spec,
			unsigned flags
		);

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	public:
		~Tube();

		static Tube*
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
		Profile*
			GetProfile(unsigned index)
				{
					Check_Object(this); Check_Object(GetSpecification());
					return &m_profiles[index];
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
		bool
			AnimateProfile(
				unsigned index,
				unsigned profile,
				const Stuff::LinearMatrix4D &world_to_new_local,
				Stuff::Time till,
				Stuff::Sphere *sphere
			);
		void
			CreateNewProfile(
				unsigned index,
				const Stuff::LinearMatrix4D &origin
			);
		void
			DestroyProfile(unsigned index);
		void
			ComputeNewLinearVelocity(
				Profile *particle,
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
	};
}
