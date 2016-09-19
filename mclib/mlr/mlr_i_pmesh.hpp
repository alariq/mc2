//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_I_PMESH_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLRINDEXEDPRIMITIVEBASE_HPP)
#include<mlr/mlrindexedprimitivebase.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//#### MLRIndexedPolyMesh with no color no lighting one texture layer  #####
	//##########################################################################


	class MLR_I_PMesh:
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
		MLR_I_PMesh(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_I_PMesh();

	public:
		MLR_I_PMesh(ClassData *class_data=MLR_I_PMesh::DefaultData);

		static MLR_I_PMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
//		void Copy(MLRIndexedPolyMesh*);

		virtual	void	InitializeDrawPrimitive(unsigned char, int=0);

		virtual void	SetSubprimitiveLengths(unsigned char *, int);
		virtual void	GetSubprimitiveLengths(unsigned char **, int*);

		void	FindFacePlanes();

		virtual int	FindBackFace(const Stuff::Point3D&);

		const Stuff::Plane *GetPolygonPlane(int i)
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

		virtual int	TransformAndClip(Stuff::Matrix4D*, MLRClippingState, GOSVertexPool*,bool=false);

		bool
			CastRay(
				Stuff::Line3D *line,
				Stuff::Normal3D *normal
			);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false);

	//	Initializes the visibility test list
		void
			ResetTestList();

	//	find which vertices are visible which not - returns nr of visible vertices
	//	the result is stored in the visibleIndexedVertices array
		int
			FindVisibleVertices();

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
		void
			Transform(Stuff::Matrix4D*);

		Stuff::DynamicArrayOf<unsigned char>	testList;
		
		Stuff::DynamicArrayOf<Stuff::Plane> facePlanes;
		
	};

	#define ICO_X 0.525731112119133606f
	#define ICO_Z 0.850650808352039932f

	extern float vdata[12][3];
	extern unsigned int tindices [20][3];

	extern long triDrawn;

	void
		subdivide (
			Stuff::Point3D *coords,
			Stuff::Point3D& v1,
			Stuff::Point3D& v2,
			Stuff::Point3D& v3,
			long depth,
			long tri2draw, 
			float rad = 1.0f
		);

	MLR_I_PMesh*
		CreateIndexedCube_NoColor_NoLit(Stuff::Scalar, MLRState*);
	MLRShape*
		CreateIndexedIcosahedron_NoColor_NoLit(
			IcoInfo&,
			MLRState*
		);

}
