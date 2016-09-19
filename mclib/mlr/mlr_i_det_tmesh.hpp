//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_I_DeT_TMESH_HPP

#include<mlr/mlr.hpp>
#include<mlr/mlr_i_tmesh.hpp>
#include<mlr/mlr_i_det_pmesh.hpp>

extern DWORD gEnableDetailTexture;

namespace MidLevelRenderer {

	//##########################################################################
	//#### MLRIndexedPolyMesh with no color no lighting w/ detail texture  #####
	//##########################################################################


	class MLR_I_DeT_TMesh:
		public MLR_I_TMesh
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
		MLR_I_DeT_TMesh(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_I_DeT_TMesh();

	public:
		MLR_I_DeT_TMesh(ClassData *class_data=MLR_I_DeT_TMesh::DefaultData);

		static MLR_I_DeT_TMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
		void
			Copy(
				MLR_I_TMesh*,
				MLRState detailState,
				Stuff::Scalar xOff,
				Stuff::Scalar yOff,
				Stuff::Scalar xFac,
				Stuff::Scalar yFac
			);

		bool
			Copy(MLR_I_DeT_PMesh *pmesh)
				{ Check_Object(pmesh); STOP(("Not implemented yet !")); return false; }

		void
			SetDetailData(
				Stuff::Scalar xOff,
				Stuff::Scalar yOff,
				Stuff::Scalar xFac,
				Stuff::Scalar yFac,
				Stuff::Scalar detailStart,
				Stuff::Scalar detailEnd
			);

		void
			GetDetailData(
				Stuff::Scalar& xOff,
				Stuff::Scalar& yOff,
				Stuff::Scalar& xFac,
				Stuff::Scalar& yFac,
				Stuff::Scalar& dStart,
				Stuff::Scalar& dEnd
			)
				{
					Check_Object(this);
					xOff = xOffset;
					yOff = yOffset;
					xFac = xScale;
					yFac = yScale;
					dStart = fadeDetailStart;
					dEnd = fadeDetailEnd;
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
					if(gEnableDetailTexture==0)
					{
						state.SetMultiTextureMode(MLRState::MultiTextureOffMode);
					}
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

		GOSVertex2UV*
			GetGOSVertices2UV(int=0)
				{ Check_Object(this); return gos_vertices2uv; }

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
			int ret = MLR_I_TMesh::GetSize();

			return ret;
		}

	protected:
		bool	detTextureVisible;

		MLRState	state2, referenceState2;

		Stuff::Scalar fadeDetailStart, fadeDetailEnd, fadeMultiplicator;

		Stuff::Scalar xOffset, yOffset, xScale, yScale;

		GOSVertex2UV *gos_vertices2uv;
	};

	MLR_I_DeT_TMesh*
		CreateIndexedTriCube_NoColor_NoLit_DetTex(Stuff::Scalar, MLRState*, MLRState*);
	MLRShape*
		CreateIndexedTriIcosahedron_NoColor_NoLit_DetTex(
			IcoInfo&,
			MLRState*,
			MLRState*
		);

}
