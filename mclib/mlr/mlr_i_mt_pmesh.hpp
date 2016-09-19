//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_I_MT_PMESH_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLR_I_PMesh_HPP)
#include<mlr/mlr_i_pmesh.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//### MLRIndexedPolyMesh with no color no lighting multi texture layer  ####
	//##########################################################################


	class MLR_I_MT_PMesh:
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
		MLR_I_MT_PMesh(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_I_MT_PMesh();

	public:
		MLR_I_MT_PMesh(ClassData *class_data=MLR_I_MT_PMesh::DefaultData);

		static MLR_I_MT_PMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
		void Copy(MLR_I_PMesh*);

		virtual int	
			TransformAndClip(Stuff::Matrix4D *, MLRClippingState, GOSVertexPool*,bool=false);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false);

		void
			SetTexCoordData(const Vector2DScalar*,	int, int pass=0);
		void
			GetTexCoordData(const Vector2DScalar **data, int *dataSize,	int pass);

		virtual void
			SetReferenceState(const MLRState& _state, int pass=0)
				{
					Check_Object(this);
					Verify(pass>=0 && pass<Limits::Max_Number_Of_Multitextures);
					if(pass==0)
					{
						referenceState = _state;
					}

					multiReferenceState[pass] = _state;
				}

		virtual const MLRState&
			GetReferenceState(int pass=0) const
				{
					Check_Object(this); 
					Verify(pass>=0 && pass<Limits::Max_Number_Of_Multitextures);
					return multiReferenceState[pass];
				}
		virtual const MLRState&
			GetCurrentState(int pass=0) const
				{
					Check_Object(this);
					Verify(pass>=0 && pass<Limits::Max_Number_Of_Multitextures);
					return multiState[pass];
				}

		virtual void
			CombineStates (const MLRState& master)
				{
					Check_Object(this);

					state.Combine(master, referenceState); 
					for(int i=0;i<currentNrOfPasses;i++)
					{
						multiState[i].Combine(master, multiReferenceState[i]); 
					}
				};

		virtual GOSVertex*
			GetGOSVertices(int pass=0)
				{
					Check_Object(this); 
					Verify(pass>=0 && pass<Limits::Max_Number_Of_Multitextures);
					return gos_vertices + pass*numGOSVertices;
				}

		virtual	void
			InitializeDrawPrimitive(unsigned char, int=0);

		int
			GetNumPasses()
				{ Check_Object(this); return currentNrOfPasses; }

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
		void
			SetTexCoordDataPointer(const Vector2DScalar*);

		unsigned char currentNrOfPasses;

		Stuff::DynamicArrayOf<MLRState> multiState, multiReferenceState;

		Stuff::DynamicArrayOf<Stuff::DynamicArrayOf<Vector2DScalar>* > multiTexCoords;	// Max_Number_Vertices_Per_Mesh

		Stuff::DynamicArrayOf<const Vector2DScalar*> multiTexCoordsPointers;

		static Stuff::DynamicArrayOf<Stuff::DynamicArrayOf<Vector2DScalar> > *clipExtraMultiTexCoords;	// Max_Number_Vertices_Per_Mesh
		static Stuff::DynamicArrayOf<Stuff::DynamicArrayOf<Vector2DScalar> > *extraMultiTexCoords;	// Max_Number_Vertices_Per_Mesh
	};

	MLRShape*
		CreateIndexedIcosahedron_NoColor_NoLit_MultiTexture(
			IcoInfo&,
			Stuff::DynamicArrayOf<MLRState>*
		);
}
