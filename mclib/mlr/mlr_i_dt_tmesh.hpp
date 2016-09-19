//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_I_DT_TMESH_HPP

#include<mlr/mlr.hpp>
#include<mlr/mlr_i_tmesh.hpp>
#include<mlr/gosvertex2uv.hpp>

namespace MidLevelRenderer {

	//##########################################################################
	//## MLRIndexedTriangleMesh with no color no lighting two texture layer  ###
	//##########################################################################


	class MLR_I_DT_TMesh:
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
		MLR_I_DT_TMesh(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_I_DT_TMesh();

	public:
		MLR_I_DT_TMesh(ClassData *class_data=MLR_I_DT_TMesh::DefaultData);

		static MLR_I_DT_TMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
		bool
			Copy(MLR_I_DT_PMesh *pmesh);

		virtual int	TransformAndClip(Stuff::Matrix4D *, MLRClippingState, GOSVertexPool*,bool=false);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false);

		void
			SetTexCoordData(const Vector2DScalar*,	int);

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
#if 0
		virtual unsigned short*
			GetGOSIndices(int pass=0)
				{
					Check_Object(this);
					if(pass==0)
						return gos_indices;
					else
						return gos_indices2;
				}
#endif
		GOSVertex2UV*
			GetGOSVertices2UV(int=0)
				{ Check_Object(this); return gos_vertices2uv; }

//		int
//			GetNumGOSVertices2UV()
//				{ Check_Object(this); return numGOSVertices2uv; }


		int
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
			int ret = MLR_I_TMesh::GetSize();

			return ret;
		}

	protected:
		MLRState	state2, referenceState2;

		static Stuff::DynamicArrayOf<Vector2DScalar> *texCoords2;	// Max_Number_Vertices_Per_Mesh
		static Stuff::DynamicArrayOf<Vector2DScalar> *clipExtraTexCoords2;	// Max_Number_Vertices_Per_Mesh

		GOSVertex2UV *gos_vertices2uv;
//		unsigned short	numGOSVertices2uv;
	};

	MLR_I_DT_TMesh*
		CreateIndexedTriCube_NoColor_NoLit_2Tex(Stuff::Scalar, MLRState*, MLRState*);

	MLRShape*
		CreateIndexedTriIcosahedron_NoColor_NoLit_2Tex(
			IcoInfo&,
			MLRState*,
			MLRState*
		);

}
