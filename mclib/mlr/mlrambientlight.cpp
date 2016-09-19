//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//###########################    MLRAmbientLight    ###########################
//#############################################################################

MLRAmbientLight::ClassData*
	MLRAmbientLight::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRAmbientLight::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRAmbientLightClassID,
			"MidLevelRenderer::MLRAmbientLight",
			MLRLight::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRAmbientLight::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRAmbientLight::MLRAmbientLight() :
	MLRLight(DefaultData)
{
	Verify(gos_GetCurrentHeap() == Heap);
	lightMask = MLRState::FaceLightingMode|MLRState::VertexLightingMode;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRAmbientLight::MLRAmbientLight(
	Stuff::MemoryStream *stream,
	int version
) :
	MLRLight(DefaultData, stream, version)
{
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);
	lightMask = MLRState::FaceLightingMode|MLRState::VertexLightingMode;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRAmbientLight::MLRAmbientLight(Stuff::Page *page) :
	MLRLight(DefaultData, page)
{
	Check_Object(page);
	Verify(gos_GetCurrentHeap() == Heap);
	lightMask = MLRState::FaceLightingMode|MLRState::VertexLightingMode;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRAmbientLight::~MLRAmbientLight()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRAmbientLight::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRAmbientLight::LightVertex(const MLRVertexData& vertexData)
{
#if COLOR_AS_DWORD
	TO_DO;
#else
	vertexData.color->red += (color.red*intensity);
	vertexData.color->green += (color.green*intensity);
	vertexData.color->blue += (color.blue*intensity);
#endif
}