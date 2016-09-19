//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRINDEXEDPRIMITIVE_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//###################    MLRIndexedPrimitive    ############################
	//##########################################################################

	class MLRIndexedPrimitive:
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
		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	protected:
		MLRIndexedPrimitive(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLRIndexedPrimitive();

	public:
		MLRIndexedPrimitive(ClassData *class_data);

		static MLRIndexedPrimitive*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		virtual void
			Save(Stuff::MemoryStream *stream);

		virtual	void	InitializeDrawPrimitive(int, int=0);

		virtual void	Lighting(MLRLight**, int nrLights);

		virtual void
			SetCoordData(
				const Stuff::Point3D *array,
				int point_count
			);

		virtual void
			SetIndexData(
				unsigned short *index_array,
				int index_count
			);

		virtual void
			GetIndexData(
				unsigned short **index_array,
				int *index_count
			);

		unsigned short*
			GetGOSIndices()
				{ Check_Object(this); return gos_indices; }

		int
			GetNumGOSIndices()
				{ Check_Object(this); return numGOSIndices; }

		virtual void
			Transform(Stuff::Matrix4D*);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*) = 0;

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
			ret += visibleIndexedVertices.GetSize();
			ret += index.GetSize();

			return ret;
		}

	protected:
		bool visibleIndexedVerticesKey;

		Stuff::DynamicArrayOf<unsigned short>	visibleIndexedVertices;

		Stuff::DynamicArrayOf<unsigned short>	index;	// List of color indexes 

		unsigned short *gos_indices;
		unsigned short	numGOSIndices;
	};

}
