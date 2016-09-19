//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_I_TMESH_HPP

#include<mlr/mlr.hpp>
#include<mlr/mlr_i_pmesh.hpp>

namespace MidLevelRenderer {

	//##########################################################################
	//#### MLRIndexedTriMesh with no color no lighting one texture layer  #####
	//##########################################################################


	class MLR_I_TMesh:
		public MLRIndexedPrimitiveBase
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
		MLR_I_TMesh(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_I_TMesh();

	public:
		MLR_I_TMesh(ClassData *class_data=MLR_I_TMesh::DefaultData);

		static MLR_I_TMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
		virtual	void	InitializeDrawPrimitive(unsigned char, int=0);

		virtual int
			GetNumPrimitives()
				{ Check_Object(this); return numOfTriangles; }

		virtual void
			SetSubprimitiveLengths(
				unsigned char *length_array,
				int subprimitive_count
				)
				{
					Check_Object(this);
					Verify(gos_GetCurrentHeap() == Heap);
					numOfTriangles = subprimitive_count;
					testList.SetLength(numOfTriangles);
					facePlanes.SetLength(numOfTriangles);
				}

		void	FindFacePlanes();

		virtual int	FindBackFace(const Stuff::Point3D&);

		const Stuff::Plane *GetTrianglePlane(int i)
			{
				Check_Object(this);
				Verify(i<facePlanes.GetLength());

				return &facePlanes[i];
			}

		virtual void	Lighting(MLRLight* const*, int nrLights);

		virtual void LightMapLighting(MLRLight*);

		virtual void
#if COLOR_AS_DWORD
			PaintMe(const DWORD *paintMe) {};
#else
			PaintMe(const Stuff::RGBAColor *paintMe) {};
#endif

		virtual int	TransformAndClip(Stuff::Matrix4D *, MLRClippingState, GOSVertexPool*,bool=false);

		bool
			CastRay(
				Stuff::Line3D *line,
				Stuff::Normal3D *normal
			);

//		void
//			Transform(Stuff::Matrix4D*);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false);

	//	Initializes the visibility test list
		void
			ResetTestList();

	//	find which vertices are visible which not - returns nr of visible vertices
	//	the result is stored in the visibleIndexedVertices array
		int
			FindVisibleVertices();

		bool
			Copy(MLR_I_PMesh*);

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
			int ret = MLRIndexedPrimitiveBase::GetSize();
			ret += testList.GetSize();
			ret += facePlanes.GetSize();

			return ret;
		}

	protected:
		int numOfTriangles;

		Stuff::DynamicArrayOf<unsigned char>	testList;
		
		Stuff::DynamicArrayOf<Stuff::Plane> facePlanes;
		
	};

	#define ICO_X 0.525731112119133606f
	#define ICO_Z 0.850650808352039932f

	extern float vdata[12][3];
	extern unsigned int tindices [20][3];

	extern long triDrawn;

	MLR_I_TMesh*
		CreateIndexedTriCube_NoColor_NoLit(Stuff::Scalar, MLRState*);
	MLRShape*
		CreateIndexedTriIcosahedron_NoColor_NoLit(
			IcoInfo&,
			MLRState*
		);

}
