//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_TERRAIN_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLR_I_DeT_TMESH_HPP)
#include<mlr/mlr_i_det_tmesh.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//#### MLRIndexedPolyMesh with no color no lighting w/ detail texture  #####
	//##########################################################################


	class MLR_Terrain:
		public MLR_I_DeT_TMesh
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
		MLR_Terrain(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLR_Terrain();

	public:
		MLR_Terrain(ClassData *class_data=MLR_Terrain::DefaultData);

		static MLR_Terrain*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

	public:
		virtual int
			TransformAndClip(Stuff::Matrix4D *, MLRClippingState, GOSVertexPool*,bool=false);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false);

		void
			SetUVData(Stuff::Scalar, Stuff::Scalar, Stuff::Scalar, Stuff::Scalar, Stuff::Scalar);

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
			int ret = MLR_I_DeT_TMesh::GetSize();

			return ret;
		}

	protected:
		Stuff::Scalar borderPixelFun;
		Stuff::Scalar xUVFac, zUVFac, minX, minZ;

		static Stuff::DynamicArrayOf<Vector2DScalar> *clipTexCoords;  // , Max_Number_Vertices_Per_Mesh
	};

	MLRShape*
		CreateIndexedTriIcosahedron_TerrainTest(
			IcoInfo&,
			MLRState*,
			MLRState*
		);

}
