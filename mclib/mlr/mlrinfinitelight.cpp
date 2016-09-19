//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//###############################    MLRInfiniteLight    ##################################
//#############################################################################

MLRInfiniteLight::ClassData*
	MLRInfiniteLight::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRInfiniteLight::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRInfiniteLightClassID,
			"MidLevelRenderer::MLRInfiniteLight",
			MLRLight::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRInfiniteLight::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRInfiniteLight::MLRInfiniteLight(ClassData *class_data) :
	MLRLight(class_data)
{
	Verify(gos_GetCurrentHeap() == Heap);
	lightMask = MLRState::FaceLightingMode|MLRState::VertexLightingMode;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRInfiniteLight::MLRInfiniteLight(
	ClassData *class_data,
	Stuff::MemoryStream *stream,
	int version
) :
	MLRLight(class_data, stream, version)
{
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);
	lightMask = MLRState::FaceLightingMode|MLRState::VertexLightingMode;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRInfiniteLight::MLRInfiniteLight(
	ClassData *class_data,
	Stuff::Page *page
) :
	MLRLight(class_data, page)
{
	Check_Object(page);
	Verify(gos_GetCurrentHeap() == Heap);
	lightMask = MLRState::FaceLightingMode|MLRState::VertexLightingMode;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRInfiniteLight::~MLRInfiniteLight()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRInfiniteLight::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRInfiniteLight::LightVertex(const MLRVertexData& vertexData)
{
	UnitVector3D light_z;

	GetInShapeDirection(light_z);

	//
	//-------------------------------------------------------------------
	// Now we reduce the light level falling on the vertex based upon the
	// cosine of the angle between light and normal
	//-------------------------------------------------------------------
	//
	Scalar cosine = -(light_z * (*vertexData.normal)) * intensity;

#if COLOR_AS_DWORD
	TO_DO;
#else
	RGBColor light_color(color);

	if (cosine > SMALL)
	{
		light_color.red *= cosine;
		light_color.green *= cosine;
		light_color.blue *= cosine;
		vertexData.color->red += light_color.red;
		vertexData.color->green += light_color.green;
		vertexData.color->blue += light_color.blue;
	}
#endif
}