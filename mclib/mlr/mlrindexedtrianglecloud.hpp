//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRINDEXEDTRIANGLECLOUD_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLRTRIANGLECLOUD_HPP)
#include<mlr/mlrtrianglecloud.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//##################    MLRIndexedTriangleCloud    #########################
	//##########################################################################


	class MLRIndexedTriangleCloud:
		public MLRTriangleCloud
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
	public:
		MLRIndexedTriangleCloud(int);
		~MLRIndexedTriangleCloud();

		void
			SetData(
				const int *tri_count,
				const int *point_count,
				const unsigned short *index_data,
				const Stuff::Point3D *point_data,
				const Stuff::RGBAColor *color_data,
				const Vector2DScalar *uv_data
			);

		void Draw (DrawEffectInformation*, GOSVertexPool*, MLRSorter*);

		int	Clip(MLRClippingState, GOSVertexPool*);		

		virtual unsigned short*
			GetGOSIndices(int=0)
				{ Check_Object(this); return gos_indices; }

		int
			GetNumGOSIndices()
				{ Check_Object(this); return numGOSIndices; }

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

	protected:
		const int *usedNrOfPoints;

		const unsigned short *index;
		const Vector2DScalar *texCoords;

		static Stuff::DynamicArrayOf<Vector2DScalar> *clipExtraTexCoords; // , Max_Number_Vertices_Per_Mesh

		static Stuff::DynamicArrayOf<unsigned short> *clipExtraIndex; // , Max_Number_Vertices_Per_Mesh

		static Stuff::DynamicArrayOf<unsigned char>	*visibleIndexedVertices;

		unsigned short *gos_indices;
		unsigned short	numGOSIndices;
	};
}
