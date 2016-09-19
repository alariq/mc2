//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRINDEXEDPRIMITIVEBASE_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//###################    MLRIndexedPrimitiveBase    ########################
	//##########################################################################

	class MLRIndexedPrimitiveBase:
		public MLRPrimitiveBase
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
		MLRIndexedPrimitiveBase(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		~MLRIndexedPrimitiveBase();

	public:
		MLRIndexedPrimitiveBase(ClassData *class_data);

		static MLRIndexedPrimitiveBase*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		virtual void
			Save(Stuff::MemoryStream *stream);

		virtual	void	InitializeDrawPrimitive(unsigned char, int=0);

		virtual void	Lighting(MLRLight* const*, int nrLights) = 0;

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

		virtual unsigned short*
			GetGOSIndices(int=0)
				{ Check_Object(this); return gos_indices; }

		int
			GetNumGOSIndices()
				{ Check_Object(this); return numGOSIndices; }

		virtual void
			Transform(Stuff::Matrix4D*);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false) = 0;

		void
			TheIndexer(int num)
				{
					Check_Object(this);
					index.SetLength(num);
					for(unsigned short i=0;i<num;i++)
					{
						index[i] = i;
					}
				}

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
			int ret = MLRPrimitiveBase::GetSize();
			ret += visibleIndexedVertices.GetSize();
			ret += index.GetSize();

			return ret;
		}

		bool
			CheckIndicies();

	protected:
		bool visibleIndexedVerticesKey;

		Stuff::DynamicArrayOf<unsigned char>	visibleIndexedVertices;

		Stuff::DynamicArrayOf<unsigned short>	index;	// List of color indexes 

		static Stuff::DynamicArrayOf<unsigned short> *clipExtraIndex; // , Max_Number_Vertices_Per_Mesh

		unsigned short *gos_indices;
		unsigned short	numGOSIndices;
	};

}
