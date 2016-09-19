//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRPOLYMESH_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLRPRIMITIVE_HPP)
#include<mlr/mlrprimitive.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//########################    MLRPolyMesh    ###############################
	//##########################################################################


	class MLRPolyMesh:
		public MLRPrimitive
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
		MLRPolyMesh(
			Stuff::MemoryStream *stream,
			int version
		);
		~MLRPolyMesh();

	public:
		MLRPolyMesh();

		static MLRPolyMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

		virtual void	SetPrimitiveLength(unsigned char *, int);
		virtual void	GetPrimitiveLength(unsigned char **, int*);

		void	FindFacePlanes();

		virtual int	FindBackFace(const Stuff::Point3D&);

		const Stuff::Plane *GetPolygonPlane(int i)
			{
				Check_Object(this);
				Verify(i<facePlanes.GetLength());

				return &facePlanes[i];
			}

		virtual void	Lighting(MLRLight**, int nrLights);

		virtual int	Clip(MLRClippingState, GOSVertexPool*);

		void
			Transform(Stuff::Matrix4D*);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*);

	//	Initializes the visibility test list
		void
			ResetTestList();

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
			int ret = MLRPrimitive::GetSize();
			ret += testList.GetSize();
			ret += facePlanes.GetSize();

			return ret;
		}

	protected:
		Stuff::DynamicArrayOf<unsigned char>	testList;
		
		Stuff::DynamicArrayOf<Stuff::Plane> facePlanes;
		
	};

	MLRPolyMesh*
		CreateCube(Stuff::Scalar, Stuff::RGBAColor*, Stuff::Vector3D*, MLRState*);

}
