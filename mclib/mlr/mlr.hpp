 //===========================================================================//
// File:	MLRStuff.hpp                                                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLR_HPP

#if !defined(STUFF_STUFF_HPP)
#include<stuff/stuff.hpp>
#endif

#ifdef LINUX_BUILD
#define _declspec(x)
#endif

namespace MidLevelRenderer
{

	//
	//--------------
	// Stuff classes
	//--------------
	//
	enum
	{
		MLRStateClassID = Stuff::FirstMLRClassID,
		MLRClippingStateClassID,
		MLRClipperClassID,
		MLRSorterClassID,
		MLRSortByOrderClassID,
		MLRLightClassID,
		MLRTexturePoolClassID,
		MLRPrimitiveClassID,
		MLRIndexedPrimitiveClassID,
		MLRPolyMeshClassID,
		MLRIndexedPolyMeshClassID,
		MLRShapeClassID,
		MLREffectClassID,
		MLRPointCloudClassID,
		MLRTriangleCloudClassID,
		MLRAmbientLightClassID,
		MLRInfiniteLightClassID,
		MLRInfiniteLightWithFalloffClassID,
		MLRPointLightClassID,
		MLRSpotLightClassID,
		MLRLightMapClassID,

		MLRPrimitiveBaseClassID,
		MLRIndexedPrimitiveBaseClassID,
		MLR_I_PMeshClassID,
		MLR_I_C_PMeshClassID,
		MLR_I_L_PMeshClassID,
		MLR_I_DT_PMeshClassID,
		MLR_I_C_DT_PMeshClassID,
		MLR_I_L_DT_PMeshClassID,
		MLRNGonCloudClassID,
		MLRCardCloudClassID,

		MLR_I_MT_PMeshClassID,

		MLR_I_DeT_PMeshClassID,
		MLR_I_C_DeT_PMeshClassID,
		MLR_I_L_DeT_PMeshClassID,

		MLR_I_TMeshClassID,
		MLR_I_DeT_TMeshClassID,
		MLR_I_C_TMeshClassID,
		MLR_I_L_TMeshClassID,

		MLR_TerrainClassID,
		MLR_Terrain2ClassID,

		MLRLineCloudClassID,

		MLRIndexedTriangleCloudClassID,

		MLR_I_DT_TMeshClassID,
		MLR_I_C_DT_TMeshClassID,
		MLR_I_L_DT_TMeshClassID,

		MLR_I_C_DeT_TMeshClassID,
		MLR_I_L_DeT_TMeshClassID,

		MLRLookUpLightClassID,

		FirstFreeMLRClassID
	};

	enum {
		Current_MLR_Version = 9
	};

	struct Limits {
		static unsigned
			Max_Number_Vertices_Per_Frame,
			Max_Number_Primitives_Per_Frame,
			Max_Number_ScreenQuads_Per_Frame,
			Max_Size_Of_LightMap_MemoryStream;

		enum {
			Max_Number_Vertices_Per_Mesh = 1024,
			Max_Number_Vertices_Per_Polygon = 32,
			Max_Number_Of_Texture_Bits = 14,
			Max_Number_Textures = 1 << Max_Number_Of_Texture_Bits,
			Max_Number_Of_NGon_Vertices = 9,
			Max_Number_Of_Multitextures	= 8,
			Max_Number_Of_Lights_Per_Primitive = 16,
			Max_Number_Of_FogStates = 4
		};
	};

	int
		ReadMLRVersion(Stuff::MemoryStream *erf_stream);
	void
		WriteMLRVersion(Stuff::MemoryStream *erf_stream);

	void InitializeClasses(
		unsigned Max_Number_Vertices_Per_Frame = 8192,
		unsigned Max_Number_Primitives_Per_Frame = 1024,
		unsigned Max_Number_ScreenQuads_Per_Frame = 512,
		unsigned Max_Size_Of_LightMap_MemoryStream = 32768,
		bool Convert_To_Triangle_Meshes = true
	);
	void TerminateClasses();

	extern HGOSHEAP Heap;
	extern HGOSHEAP StaticHeap;

	extern bool ConvertToTriangleMeshes;

	DECLARE_TIMER(extern, Scene_Draw_Time);

	DECLARE_TIMER(extern, Transform_Time);
	DECLARE_TIMER(extern, Clipping_Time);
	DECLARE_TIMER(extern, GOS_Draw_Time);
	DECLARE_TIMER(extern, Vertex_Light_Time);
	DECLARE_TIMER(extern, LightMap_Light_Time);
	DECLARE_TIMER(extern, Texture_Sorting_Time);
	DECLARE_TIMER(extern, Alpha_Sorting_Time);
	DECLARE_TIMER(extern, Unlock_Texture_Time);

	extern DWORD Number_Of_Primitives;
	extern DWORD NumAllIndices;
	extern DWORD NumAllVertices;
	extern float Index_Over_Vertex_Ratio;
	extern DWORD TransformedVertices;
	extern DWORD NumberOfAlphaSortedTriangles;
	extern DWORD LitVertices;
	extern DWORD NonClippedVertices;
	extern DWORD ClippedVertices;
	extern DWORD PolysClippedButOutside;
	extern DWORD PolysClippedButInside;
	extern DWORD PolysClippedButOnePlane;
	extern DWORD PolysClippedButGOnePlane;
	extern bool PerspectiveMode;
}

#define COLOR_AS_DWORD	0

#define	EFECT_CLIPPED	0

#define FOG_HACK		1

#define	TO_DO	Abort_Program("Here has work to be done !");
#undef	MLR_TRACE

#include<mlr/mlrstate.hpp>
#include<mlr/mlrtexture.hpp>
#include<mlr/mlrtexturepool.hpp>
#include<mlr/mlrclippingstate.hpp>
#include<mlr/mlrclipper.hpp>
#include<mlr/mlrsorter.hpp>
#include<mlr/mlrsortbyorder.hpp>
#include<mlr/mlrlight.hpp>
#include<mlr/mlrshape.hpp>
#include<mlr/mlreffect.hpp>
