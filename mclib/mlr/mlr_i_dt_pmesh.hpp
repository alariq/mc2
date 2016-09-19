//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_I_DT_PMESH_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLR_I_PMesh_HPP)
#include<mlr/mlr_i_pmesh.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//#### MLRIndexedPolyMesh with no color no lighting two texture layer  #####
	//##########################################################################


	class MLR_I_DT_PMesh:
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
		MLR_I_DT_PMesh(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_I_DT_PMesh();

	public:
		MLR_I_DT_PMesh(ClassData *class_data=MLR_I_DT_PMesh::DefaultData);

		static MLR_I_DT_PMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
//		void Copy(MLRIndexedPolyMesh*);

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
		MLRState	state2, referenceState2;

		static Stuff::DynamicArrayOf<Vector2DScalar> *texCoords2;	// Max_Number_Vertices_Per_Mesh
		static Stuff::DynamicArrayOf<Vector2DScalar> *clipExtraTexCoords2;	// Max_Number_Vertices_Per_Mesh
	};

	MLR_I_DT_PMesh*
		CreateIndexedCube_NoColor_NoLit_2Tex(Stuff::Scalar, MLRState*, MLRState*);

	MLRShape*
		CreateIndexedIcosahedron_NoColor_NoLit_2Tex(
			IcoInfo&,
			MLRState*,
			MLRState*
		);

}
