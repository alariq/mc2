//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

DWORD	gShowClippedPolys=0;
DWORD	gShowBirdView=0;
DWORD	gEnableDetailTexture=1;
DWORD	gEnableTextureSort=1;
DWORD	gEnableAlphaSort=1;
DWORD	gEnableMultiTexture=1;
DWORD	gEnableLightMaps=1;

// sebi
static uint32_t MLR_SIGNATURE = 0x4D4C5223; // MLR#

static bool __stdcall CheckDetailTexture()
{
	return gEnableDetailTexture!=0;
}
static bool __stdcall CheckTextureSort()
{
	return gEnableTextureSort!=0;
}
static bool __stdcall CheckAlphaSort()
{
	return gEnableAlphaSort!=0;
}
static bool __stdcall CheckMultiTexture()
{
	return gEnableMultiTexture!=0;
}
static bool __stdcall CheckLightMaps()
{
	return gEnableLightMaps!=0;
}

static void __stdcall EnableDetailTexture()
{
	gEnableDetailTexture=!gEnableDetailTexture;
}
static void __stdcall EnableTextureSort()
{
	gEnableTextureSort=!gEnableTextureSort;
}
static void __stdcall EnableAlphaSort()
{
	gEnableAlphaSort=!gEnableAlphaSort;
}
static void __stdcall EnableMultiTexture()
{
	gEnableMultiTexture=!gEnableMultiTexture;
}
static void __stdcall EnableLightMaps()
{
	gEnableLightMaps=!gEnableLightMaps;
}

extern DWORD gShowClippedPolys;
static bool __stdcall Check_ShowClippedPolys() {return gShowClippedPolys!=0;}
static void __stdcall Toggle_ShowClippedPolys() {gShowClippedPolys=!gShowClippedPolys;}

extern DWORD gShowBirdView;
static bool __stdcall Check_ShowBirdView() {return gShowBirdView!=0;}
static void __stdcall Toggle_ShowBirdView() {gShowBirdView=!gShowBirdView;}

unsigned
	Limits::Max_Number_Vertices_Per_Frame,
	Limits::Max_Number_Primitives_Per_Frame,
	Limits::Max_Number_ScreenQuads_Per_Frame,
	Limits::Max_Size_Of_LightMap_MemoryStream;

HGOSHEAP
	MidLevelRenderer::Heap = NULL,
	MidLevelRenderer::StaticHeap = NULL;

DEFINE_TIMER(MidLevelRenderer, Scene_Draw_Time);

DEFINE_TIMER(MidLevelRenderer, Transform_Time);
DEFINE_TIMER(MidLevelRenderer, Clipping_Time);
DEFINE_TIMER(MidLevelRenderer, GOS_Draw_Time);
DEFINE_TIMER(MidLevelRenderer, Vertex_Light_Time);
DEFINE_TIMER(MidLevelRenderer, LightMap_Light_Time);
DEFINE_TIMER(MidLevelRenderer, Texture_Sorting_Time);
DEFINE_TIMER(MidLevelRenderer, Alpha_Sorting_Time);
DEFINE_TIMER(MidLevelRenderer, Unlock_Texture_Time);

DWORD MidLevelRenderer::Number_Of_Primitives;
DWORD MidLevelRenderer::NumAllIndices;
DWORD MidLevelRenderer::NumAllVertices;
float MidLevelRenderer::Index_Over_Vertex_Ratio;
DWORD MidLevelRenderer::TransformedVertices;
DWORD MidLevelRenderer::NumberOfAlphaSortedTriangles;
DWORD MidLevelRenderer::LitVertices;
DWORD MidLevelRenderer::NonClippedVertices;
DWORD MidLevelRenderer::ClippedVertices;
DWORD MidLevelRenderer::PolysClippedButOutside;
DWORD MidLevelRenderer::PolysClippedButInside;
DWORD MidLevelRenderer::PolysClippedButOnePlane;
DWORD MidLevelRenderer::PolysClippedButGOnePlane;


bool MidLevelRenderer::ConvertToTriangleMeshes = true;
bool MidLevelRenderer::PerspectiveMode = true;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MidLevelRenderer::InitializeClasses(
		unsigned Max_Number_Vertices_Per_Frame,
		unsigned Max_Number_Primitives_Per_Frame,
		unsigned Max_Number_ScreenQuads_Per_Frame,
		unsigned Max_Size_Of_LightMap_MemoryStream,
		bool Convert_To_Triangle_Meshes
	)
{
	Verify(FirstFreeMLRClassID <= LastMLRClassID);

	Verify(!StaticHeap);
	StaticHeap = gos_CreateMemoryHeap("MLR Static");
	Check_Pointer(StaticHeap);
	gos_PushCurrentHeap(StaticHeap);

	Verify(!Heap);
	Heap = gos_CreateMemoryHeap("MLR");
	Check_Pointer(Heap);

	Limits::Max_Number_Vertices_Per_Frame = Max_Number_Vertices_Per_Frame;
	Limits::Max_Number_Primitives_Per_Frame = Max_Number_Primitives_Per_Frame;
	Limits::Max_Number_ScreenQuads_Per_Frame = Max_Number_ScreenQuads_Per_Frame;
	Limits::Max_Size_Of_LightMap_MemoryStream = Max_Size_Of_LightMap_MemoryStream;

	ConvertToTriangleMeshes = Convert_To_Triangle_Meshes;

	MLRLight::InitializeClass();
	MLRTexturePool::InitializeClass();
	MLRClipper::InitializeClass();
	MLRSorter::InitializeClass();
	MLRSortByOrder::InitializeClass();
	MLRShape::InitializeClass();
	MLREffect::InitializeClass();
	MLRPointCloud::InitializeClass();
	MLRTriangleCloud::InitializeClass();
	MLRNGonCloud::InitializeClass();
	MLRCardCloud::InitializeClass();
	MLRAmbientLight::InitializeClass();
	MLRInfiniteLight::InitializeClass();
	MLRInfiniteLightWithFalloff::InitializeClass();
	MLRPointLight::InitializeClass();
	MLRSpotLight::InitializeClass();
//	MLRLightMap::InitializeClass();

	MLRPrimitiveBase::InitializeClass();
	MLRIndexedPrimitiveBase::InitializeClass();
	MLR_I_PMesh::InitializeClass();
	MLR_I_C_PMesh::InitializeClass();
	MLR_I_L_PMesh::InitializeClass();
	MLR_I_DT_PMesh::InitializeClass();
	MLR_I_C_DT_PMesh::InitializeClass();
	MLR_I_L_DT_PMesh::InitializeClass();

	MLR_I_MT_PMesh::InitializeClass();

	MLR_I_DeT_PMesh::InitializeClass();
	MLR_I_C_DeT_PMesh::InitializeClass();
	MLR_I_L_DeT_PMesh::InitializeClass();

	MLR_I_TMesh::InitializeClass();
	MLR_I_DeT_TMesh::InitializeClass();
	MLR_I_C_TMesh::InitializeClass();
	MLR_I_L_TMesh::InitializeClass();

//	MLR_Terrain::InitializeClass();
//	MLR_Terrain2::InitializeClass();

	MLRLineCloud::InitializeClass();
	MLRIndexedTriangleCloud::InitializeClass();

	MLR_I_DT_TMesh::InitializeClass();
	MLR_I_C_DT_TMesh::InitializeClass();
	MLR_I_L_DT_TMesh::InitializeClass();

	MLR_I_C_DeT_TMesh::InitializeClass();
	MLR_I_L_DeT_TMesh::InitializeClass();

	MLRLookUpLight::InitializeClass();

#if FOG_HACK
//	for(int i=0;i<Limits::Max_Number_Of_FogStates;i++)
//	{
//		GOSVertex::SetFogTableEntry(i+1, 700.0f, 1000.0f, 0.0f);
//	}
#endif

	gos_PopCurrentHeap();

	//
	//-------------------------
	// Setup the debugger menus
	//-------------------------
	//
	AddDebuggerMenuItem(
		"Libraries\\MLR\\Show Clipped Polygons",
		Check_ShowClippedPolys,
		Toggle_ShowClippedPolys,
		NULL
	);
	AddDebuggerMenuItem(
		"Libraries\\MLR\\Show Bird View",
		Check_ShowBirdView,
		Toggle_ShowBirdView,
		NULL
	);

	AddDebuggerMenuItem("Libraries\\MLR\\Texture Sort", CheckTextureSort, EnableTextureSort, NULL );
	AddDebuggerMenuItem("Libraries\\MLR\\Enable Detail Texture", CheckDetailTexture, EnableDetailTexture, NULL );
	AddDebuggerMenuItem("Libraries\\MLR\\Alpha Sort", CheckAlphaSort, EnableAlphaSort, NULL );
	AddDebuggerMenuItem("Libraries\\MLR\\MultiTexture Enabled", CheckMultiTexture, EnableMultiTexture, NULL );
	AddDebuggerMenuItem("Libraries\\MLR\\LightMaps Enabled", CheckLightMaps, EnableLightMaps, NULL );

	//
	//---------------------
	// Setup the statistics
	//---------------------
	//
	StatisticFormat( "" );
	StatisticFormat( "Mid Level Renderer" );
	StatisticFormat( "==================" );
	StatisticFormat( "" );

	Initialize_Timer(Transform_Time, "Transform Time");
	Initialize_Timer(Clipping_Time, "Clipping Time");
	Initialize_Timer(GOS_Draw_Time, "GOS Draw Time");
	Initialize_Timer(Vertex_Light_Time, "Vertex Light Time");
	Initialize_Timer(LightMap_Light_Time, "LightMap Light Time");
	Initialize_Timer(Texture_Sorting_Time, "Texture Sorting Time");
	Initialize_Timer(Alpha_Sorting_Time, "Alpha Sorting Time");
	Initialize_Timer(Unlock_Texture_Time, "Unlock Texture Time");	

	AddStatistic( "MLR Primitives",			"prims",		gos_DWORD, &Number_Of_Primitives, Stat_AutoReset );
	AddStatistic( "Indices/Vertices",	"Ratio",	gos_float, &Index_Over_Vertex_Ratio, Stat_AutoReset+Stat_2DP );
	AddStatistic( "Transformed vertices",			"vertices",		gos_DWORD, &TransformedVertices, Stat_AutoReset );
	AddStatistic( "Number of alphasorted Tri",			"tri",		gos_DWORD, &NumberOfAlphaSortedTriangles, Stat_AutoReset );
	AddStatistic( "Lit vertices",			"vertices",		gos_DWORD, &LitVertices, Stat_AutoReset );
	AddStatistic( "Unclipped vertices",			"vertices",		gos_DWORD, &NonClippedVertices, Stat_AutoReset );
	AddStatistic( "Clipped vertices",			"vertices",		gos_DWORD, &ClippedVertices, Stat_AutoReset );

// Polygons in primitives which are clipped but polys are outside the viewing frustrum
	AddStatistic( "Clip: Offscreen",			"Poly",		gos_DWORD, &PolysClippedButOutside, Stat_AutoReset );
// Polygons in primitives which are clipped but polys are inside the viewing frustrum
	AddStatistic( "Clip: Onscreen",			"Poly",		gos_DWORD, &PolysClippedButInside, Stat_AutoReset );
// Polygons in primitives which are clipped, polys clipped against one plain
	AddStatistic( "Clip: One Plane",			"Poly",		gos_DWORD, &PolysClippedButOnePlane, Stat_AutoReset );
// Polygons in primitives which are clipped, polys clipped against more than one plain
	AddStatistic( "Clip: > One Plane",			"Poly",		gos_DWORD, &PolysClippedButGOnePlane, Stat_AutoReset );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MidLevelRenderer::TerminateClasses()
{
	if ( !MLRLookUpLight::DefaultData ) //  make sure there is something to termiante
		return;

	MLRLookUpLight::TerminateClass();

	MLR_I_L_DeT_TMesh::TerminateClass();
	MLR_I_C_DeT_TMesh::TerminateClass();

	MLR_I_L_DT_TMesh::TerminateClass();
	MLR_I_C_DT_TMesh::TerminateClass();
	MLR_I_DT_TMesh::TerminateClass();

	MLRIndexedTriangleCloud::TerminateClass();
	MLRLineCloud::TerminateClass();

//	MLR_Terrain2::TerminateClass();
//	MLR_Terrain::TerminateClass();

	MLR_I_L_TMesh::TerminateClass();
	MLR_I_C_TMesh::TerminateClass();
	MLR_I_DeT_TMesh::TerminateClass();
	MLR_I_TMesh::TerminateClass();

	MLR_I_L_DeT_PMesh::TerminateClass();
	MLR_I_C_DeT_PMesh::TerminateClass();
	MLR_I_DeT_PMesh::TerminateClass();

	MLR_I_MT_PMesh::TerminateClass();

	MLR_I_L_DT_PMesh::TerminateClass();
	MLR_I_C_DT_PMesh::TerminateClass();
	MLR_I_DT_PMesh::TerminateClass();
	MLR_I_L_PMesh::TerminateClass();
	MLR_I_C_PMesh::TerminateClass();
	MLR_I_PMesh::TerminateClass();
	MLRIndexedPrimitiveBase::TerminateClass();
	MLRPrimitiveBase::TerminateClass();

//	MLRLightMap::TerminateClass();
	MLRSpotLight::TerminateClass();
	MLRPointLight::TerminateClass();
	MLRInfiniteLightWithFalloff::TerminateClass();
	MLRInfiniteLight::TerminateClass();
	MLRAmbientLight::TerminateClass();
	MLRCardCloud::TerminateClass();
	MLRNGonCloud::TerminateClass();
	MLRTriangleCloud::TerminateClass();
	MLRPointCloud::TerminateClass();
	MLREffect::TerminateClass();
	MLRShape::TerminateClass();
	MLRSortByOrder::TerminateClass();
	MLRSorter::TerminateClass();
	MLRClipper::TerminateClass();
	MLRTexturePool::TerminateClass();
	MLRLight::TerminateClass();

	Check_Pointer(Heap);
	gos_DestroyMemoryHeap(Heap);
	Heap = NULL;

	Check_Pointer(StaticHeap);
	gos_DestroyMemoryHeap(StaticHeap);
	StaticHeap = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MidLevelRenderer::ReadMLRVersion(MemoryStream *erf_stream)
{
	Check_Object(erf_stream);

	//
	//------------------------------------------------------------------------
	// See if this file has an erf signature.  If so, the next int will be the
	// version number.  If not, assume it is version 1 and rewind the file
	//------------------------------------------------------------------------
	//
	int version = -1;
	unsigned int erf_signature;
	*erf_stream >> erf_signature;
	if (erf_signature == MLR_SIGNATURE)
		*erf_stream >> version;
	else
		erf_stream->RewindPointer(sizeof(erf_signature));
	if (version > Current_MLR_Version)
		STOP(("Application must be rebuilt to use this version of MLR data"));
	return version;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MidLevelRenderer::WriteMLRVersion(MemoryStream *erf_stream)
{
	Check_Object(erf_stream);
	*erf_stream << MLR_SIGNATURE << static_cast<int>(Current_MLR_Version);
}
