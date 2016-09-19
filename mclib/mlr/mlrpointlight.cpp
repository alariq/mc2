//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//###########################    MLRPointLight    #############################
//#############################################################################

MLRPointLight::ClassData*
	MLRPointLight::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPointLight::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRPointLightClassID,
			"MidLevelRenderer::MLRPointLight",
			MLRInfiniteLightWithFalloff::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPointLight::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPointLight::MLRPointLight() :
	MLRInfiniteLightWithFalloff(DefaultData)
{
	Verify(gos_GetCurrentHeap() == Heap);
	lightMap = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPointLight::MLRPointLight(
	Stuff::MemoryStream *stream,
	int version
) :
	MLRInfiniteLightWithFalloff(DefaultData, stream, version)
{
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);
	lightMap = NULL;
	if (version > 7)
	{
		MString name;
		*stream >> name;
		if (name.GetLength() > 0)
		{
			Check_Object(MLRTexturePool::Instance);
			MLRTexture *texture = (*MLRTexturePool::Instance)(name, 0);
			if (!texture)
				texture = MLRTexturePool::Instance->Add(name, 0);
			Check_Object(texture);
			lightMap = new MLRLightMap(texture);
			Check_Object(lightMap);
			lightMask |= MLRState::LightMapLightingMode;
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPointLight::MLRPointLight(Stuff::Page *page):
	MLRInfiniteLightWithFalloff(DefaultData, page)
{
	Check_Object(page);
	Verify(gos_GetCurrentHeap() == Heap);
	lightMap = NULL;
	const char* lightmap;
	if (page->GetEntry("LightMap", &lightmap))
	{
		Check_Pointer(lightmap);
		Check_Object(MLRTexturePool::Instance);
		MLRTexture *texture = (*MLRTexturePool::Instance)(lightmap, 0);
		if (!texture)
			texture = MLRTexturePool::Instance->Add(lightmap, 0);
		Check_Object(texture);
		lightMap = new MLRLightMap(texture);
		Check_Object(lightMap);
		lightMask |= MLRState::LightMapLightingMode;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPointLight::~MLRPointLight()
{
	if (lightMap)
	{
		Check_Object(lightMap);
		delete lightMap;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPointLight::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLRInfiniteLightWithFalloff::Save(stream);
	if (lightMap)
	{
		Check_Object(lightMap);
		unsigned handle = lightMap->GetState().GetTextureHandle();
		MLRTexture *texture = (*MLRTexturePool::Instance)[handle];
		Check_Object(texture);
		MString name = texture->GetTextureName();
		*stream << name;
	}
	else
		*stream << MString("");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPointLight::Write(Stuff::Page *page)
{
	Check_Object(this);
	Check_Object(page);

	MLRInfiniteLightWithFalloff::Write(page);
	if (lightMap)
	{
		Check_Object(lightMap);
		unsigned handle = lightMap->GetState().GetTextureHandle();
		MLRTexture *texture = (*MLRTexturePool::Instance)[handle];
		Check_Object(texture);
		page->SetEntry("LightMap", texture->GetTextureName());
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPointLight::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPointLight::LightVertex(const MLRVertexData& vertexData)
{
	UnitVector3D light_z;
	RGBColor light_color(color);
	Point3D vertex_to_light;

	Verify(GetFalloffDistance(vertex_to_light.x, vertex_to_light.y));

	GetInShapePosition(vertex_to_light);

	vertex_to_light -= *vertexData.point;

	//
	//--------------------------------------------------------------
	// If the distance to the vertex is zero, the light will not
	// contribute to the vertex coloration.  Otherwise, decrease the
	// light level as appropriate to the distance
	//--------------------------------------------------------------
	//
	Scalar length = vertex_to_light.GetApproximateLength();

	Scalar falloff = 1.0f;
	if(GetFalloff(length, falloff))
	{
		light_color.red *= falloff;
		light_color.green *= falloff;
		light_color.blue *= falloff;
	}
	else
	{
		return;
	}

	length = -1.0f / length;
	light_z.Vector3D::Multiply(vertex_to_light, length);

	//
	//-------------------------------------------------------------------
	// Now we reduce the light level falling on the vertex based upon the
	// cosine of the angle between light and normal
	//-------------------------------------------------------------------
	//
	Scalar cosine = -(light_z * (*vertexData.normal)) * intensity;

#if COLOR_AS_DWORD
#else
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPointLight::SetLightMap(MLRLightMap *light_map)
{
	Check_Object(this);
	if (lightMap)
	{
		Check_Object(lightMap);
		delete lightMap;
	}
	lightMap = light_map;

	if (lightMap == NULL)
	{
		lightMask &= ~MLRState::LightMapLightingMode;
	}
	else
	{
		Check_Object(light_map);
		lightMask |= MLRState::LightMapLightingMode;
	}
}
