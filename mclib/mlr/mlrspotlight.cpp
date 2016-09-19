//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//#########################    MLRSpotLight    ################################
//#############################################################################

MLRSpotLight::ClassData*
	MLRSpotLight::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSpotLight::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRSpotLightClassID,
			"MidLevelRenderer::MLRSpotLight",
			MLRInfiniteLightWithFalloff::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSpotLight::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSpotLight::MLRSpotLight() :
	MLRInfiniteLightWithFalloff(DefaultData)
{
	Verify(gos_GetCurrentHeap() == Heap);
	lightMap = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSpotLight::MLRSpotLight(
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
	Radian angle;
	*stream >> angle;
	SetSpreadAngle(angle);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSpotLight::MLRSpotLight(Stuff::Page *page):
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
	Degree angle=45.0;
	page->GetEntry("Spread", &angle.angle);
	SetSpreadAngle(angle);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSpotLight::~MLRSpotLight()
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
	MLRSpotLight::Save(Stuff::MemoryStream *stream)
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
	*stream << spreadAngle;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSpotLight::Write(Stuff::Page *page)
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
	page->SetEntry("Spread", spreadAngle * Degrees_Per_Radian);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSpotLight::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSpotLight::SetSpreadAngle(const Radian &radian)
{
	Check_Object(this);

	spreadAngle = radian;

	tanSpreadAngle = (Scalar)tan(spreadAngle);
	cosSpreadAngle = (Scalar)cos(spreadAngle);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSpotLight::SetSpreadAngle(const Degree &degree)
{
	Check_Object(this);

	spreadAngle = degree;

	tanSpreadAngle = (Scalar)tan(spreadAngle);
	cosSpreadAngle = (Scalar)cos(spreadAngle);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MLRSpotLight::GetSpreadAngle(Radian *angle)
{
	Check_Object(this);

	*angle = spreadAngle;

	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSpotLight::LightVertex(const MLRVertexData& vertexData)
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

	Scalar cos_spread_angle = GetCosSpreadAngle();

	GetInWorldDirection(light_z);

	length = -1.0f / length;
	vertex_to_light *= length;
	
	Scalar t = vertex_to_light * light_z;
	
	if (t <= cos_spread_angle)
	{
		return;
	}

/*	Verify(!Close_Enough(cos_spread_angle, 1.0f));
	
	spread = 1.0f - ((1.0f - t) / (1.0f - cos_spread_angle));
	
	light_color.red *= cos_spread_angle;
	light_color.green *= cos_spread_angle;
	light_color.blue *= cos_spread_angle;
*/
	light_z.x = vertex_to_light.x;
	light_z.y = vertex_to_light.y;
	light_z.z = vertex_to_light.z;

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
	MLRSpotLight::SetLightMap(MLRLightMap *light_map)
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
