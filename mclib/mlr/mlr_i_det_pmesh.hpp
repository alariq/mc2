//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_I_DeT_PMESH_HPP

#include<mlr/mlr.hpp>
#include<mlr/mlr_i_pmesh.hpp>

namespace MidLevelRenderer {

	struct DetailTextureInfo {
		DetailTextureInfo()
		{
			name = NULL;
			xOff=0.0f; xFac=8.0f;
			zOff=0.0f; zFac=8.0f;
			dStart = 32.0f; dEnd = 64.0f;
			alphaMode = MLRState::AlphaInvAlphaMode;
		}
		const char *name;
		Stuff::Scalar xOff, zOff;
		Stuff::Scalar xFac, zFac;
		Stuff::Scalar dStart, dEnd;
		MLRState::AlphaMode alphaMode;
	};
	//##########################################################################
	//#### MLRIndexedPolyMesh with no color no lighting w/ detail texture  #####
	//##########################################################################


	class MLR_I_DeT_PMesh:
		public MLR_I_PMesh
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialization
	//
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	protected:
		MLR_I_DeT_PMesh(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_I_DeT_PMesh();

	public:
		MLR_I_DeT_PMesh(ClassData *class_data=MLR_I_DeT_PMesh::DefaultData);

		static MLR_I_DeT_PMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
		void
			Copy(
				MLR_I_PMesh*,
				MLRState detailState,
				Stuff::Scalar xOff,
				Stuff::Scalar yOff,
				Stuff::Scalar xFac,
				Stuff::Scalar yFac
			);

		void
			SetDetailData(
				Stuff::Scalar xOff,
				Stuff::Scalar yOff,
				Stuff::Scalar xFac,
				Stuff::Scalar yFac
			)
				{
					Check_Object(this);
					xOffset = xOff;
					yOffset = yOff;
					xScale = xFac;
					yScale = yFac;
				}

		void
			GetDetailData(
				Stuff::Scalar& xOff,
				Stuff::Scalar& yOff,
				Stuff::Scalar& xFac,
				Stuff::Scalar& yFac
			)
				{
					Check_Object(this);
					xOff = xOffset;
					yOff = yOffset;
					xFac = xScale;
					yFac = yScale;
				}

		virtual int	TransformAndClip(Stuff::Matrix4D *, MLRClippingState, GOSVertexPool*,bool=false);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false);

		virtual void
			SetReferenceState(const MLRState& _state, int pass=0)
				{
					Check_Object(this);
					Verify(pass>=0 && pass<2);
					if(pass==0)
					{
						referenceState = _state;
					}
					else
					{
						passes = 2;
						referenceState2 = _state;
					}
				}
		virtual const MLRState&
			GetReferenceState(int pass=0) const
				{
					Check_Object(this); 
					if(pass==0)
						return referenceState;
					else
						return referenceState2;
				}
		virtual const MLRState&
			GetCurrentState(int pass=0) const
				{
					Check_Object(this);
					if(pass==0)
						return state;
					else
						return state2;
				}

		virtual void
			CombineStates (const MLRState& master)
				{
					Check_Object(this);
					state.Combine(master, referenceState); 
					state2.Combine(master, referenceState2);
				};

		virtual GOSVertex*
			GetGOSVertices(int pass=0)
				{
					Check_Object(this); 
					if(pass==0)
						return gos_vertices;
					else
						return gos_vertices+numGOSVertices;
				}

		virtual int
			GetNumPasses();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
	public:
		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const;

	virtual int
			GetSize()
		{ 
			Check_Object(this);
			int ret = MLR_I_PMesh::GetSize();

			return ret;
		}

	protected:
		bool	detTextureVisible;

		MLRState	state2, referenceState2;

		Stuff::Scalar noDetailDistance;

		Stuff::Scalar xOffset, yOffset, xScale, yScale;
	};

	MLR_I_DeT_PMesh*
		CreateIndexedCube_NoColor_NoLit_DetTex(
			Stuff::Scalar radius,
			MLRState*,
			MLRState*,
			Stuff::Scalar xOff,
			Stuff::Scalar yOff,
			Stuff::Scalar xFac,
			Stuff::Scalar yFac
			);
	MLRShape*
		CreateIndexedIcosahedron_NoColor_NoLit_DetTex(
			IcoInfo&,
			MLRState*,
			MLRState*
		);
}
