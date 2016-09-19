//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"gosfxheaders.hpp"
#include<mlr/mlrpointlight.hpp>

gosFX::LightManager
	*gosFX::LightManager::Instance = NULL;

gosFX::Light*
	gosFX::LightManager::MakePointLight(const char* light_map)
{
	return reinterpret_cast<Light*>(this);
}

void
	gosFX::LightManager::ChangeLight(
		Light *light,
		Info *info
	)
{
}

void
	gosFX::LightManager::DeleteLight(Light *light)
{
}

//############################################################################
//########################  gosFX::PointLight__Specification  #############################
//############################################################################

//------------------------------------------------------------------------------
//
gosFX::PointLight__Specification::PointLight__Specification(
	Stuff::MemoryStream *stream,
	int gfx_version
):
	Effect__Specification(PointLightClassID, stream, gfx_version)
{
	Check_Pointer(this);
	Verify(m_class == PointLightClassID);
	Verify(gos_GetCurrentHeap() == Heap);

	m_red.Load(stream, gfx_version);
	m_green.Load(stream, gfx_version);
	m_blue.Load(stream, gfx_version);
	m_intensity.Load(stream, gfx_version);
	if (gfx_version>15)
	{
		m_innerRadius.Load(stream, gfx_version);
		m_outerRadius.Load(stream, gfx_version);
		if (gfx_version>16)
			*stream >> m_lightMap;
	}
	else
	{
		m_innerRadius.SetCurve(0.0f);
		m_outerRadius.SetCurve(25.0f);
	}

	*stream >> m_twoSided;
}

//------------------------------------------------------------------------------
//
gosFX::PointLight__Specification::PointLight__Specification():
	Effect__Specification(PointLightClassID)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
	m_twoSided = false;
}

//------------------------------------------------------------------------------
//
gosFX::PointLight__Specification*
	gosFX::PointLight__Specification::Make(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	PointLight__Specification *spec =
		new gosFX::PointLight__Specification(stream, gfx_version);
	gos_PopCurrentHeap();

	return spec;
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointLight__Specification::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);
	Effect__Specification::Save(stream);

	m_red.Save(stream);
	m_green.Save(stream);
	m_blue.Save(stream);
	m_intensity.Save(stream);
	m_innerRadius.Save(stream);
	m_outerRadius.Save(stream);

	*stream << m_lightMap << m_twoSided;
}

//------------------------------------------------------------------------------
//
void 
	gosFX::PointLight__Specification::BuildDefaults()
{

	Check_Object(this);
	Effect__Specification::BuildDefaults();

	m_red.SetCurve(1.0f);
	m_green.SetCurve(1.0f);
	m_blue.SetCurve(1.0f);
	m_intensity.SetCurve(1.0f);

	m_innerRadius.SetCurve(0.0f);
	m_outerRadius.SetCurve(10.0f);

	m_twoSided = false;
}



//------------------------------------------------------------------------------
//
bool 
	gosFX::PointLight__Specification::IsDataValid(bool fix_data)
{

	Check_Object(this);
return	Effect__Specification::IsDataValid(fix_data);
/*
	m_red.SetCurve(1.0f);
	m_green.SetCurve(1.0f);
	m_blue.SetCurve(1.0f);
	m_intensity.SetCurve(1.0f);

	m_innerRadius.SetCurve(0.0f);
	m_outerRadius.SetCurve(10.0f);

	m_twoSided = false;
*/
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointLight__Specification::Copy(PointLight__Specification *spec)
{
	Check_Object(this);
	Check_Object(spec);

	Effect__Specification::Copy(spec);

	gos_PushCurrentHeap(Heap);
	m_red = spec->m_red;
	m_green = spec->m_green;
	m_blue = spec->m_blue;
	m_intensity = spec->m_intensity;
	m_twoSided = spec->m_twoSided;
	m_lightMap = spec->m_lightMap;
	gos_PopCurrentHeap();
}

//############################################################################
//##############################  gosFX::PointLight  ################################
//############################################################################

gosFX::PointLight::ClassData*
	gosFX::PointLight::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::PointLight::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == Heap);
	DefaultData =
		new ClassData(
			PointLightClassID,
			"gosFX::PointLight",
			Effect::DefaultData,
			(Effect::Factory)&Make,
			(Specification::Factory)&Specification::Make
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointLight::TerminateClass()
{
	if ( DefaultData )
	{
		Unregister_Object(DefaultData);
		delete DefaultData;
		DefaultData = NULL;
	}
}

//------------------------------------------------------------------------------
//
gosFX::PointLight::PointLight(
	Specification *spec,
	unsigned flags
):
	Effect(DefaultData, spec, flags)
{
	Check_Object(spec);
	Verify(gos_GetCurrentHeap() == Heap);
}

//------------------------------------------------------------------------------
//
gosFX::PointLight::~PointLight()
{
	if (m_light)
	{
		Check_Pointer(m_light);
		LightManager::Instance->DeleteLight(m_light);
	}
}

//------------------------------------------------------------------------------
//
gosFX::PointLight*
	gosFX::PointLight::Make(
		Specification *spec,
		unsigned flags
	)
{
	Check_Object(spec);

	gos_PushCurrentHeap(Heap);
	PointLight *obj = new gosFX::PointLight(spec, flags);
	gos_PopCurrentHeap();

	return obj;
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointLight::Start(ExecuteInfo *info)
{
	Check_Object(this);
	Check_Object(info);
	Effect::Start(info);

	Specification *spec = GetSpecification();
	Check_Object(spec);
	Check_Object(LightManager::Instance);
	m_light = LightManager::Instance->MakePointLight(spec->m_lightMap);
	Check_Pointer(m_light);
}

//------------------------------------------------------------------------------
//
bool
	gosFX::PointLight::Execute(ExecuteInfo *info)
{
	Check_Object(this);
	Check_Object(info);

	if (!IsExecuted())
		return false;

	//
	//------------------------
	// Do the effect animation
	//------------------------
	//
	if (!Effect::Execute(info))
		return false;

	//
	//-----------------------------------------
	// Animate the parent then get our pointers
	//-----------------------------------------
	//
	Specification *spec = GetSpecification();
	Check_Object(spec);

	//
	//--------------------------------
	// Animate the color and intensity
	//--------------------------------
	//
	Stuff::LinearMatrix4D local_to_world;

	//
	//-----------------------------------------------------------
	// Construct the info object and send it to the light manager
	//-----------------------------------------------------------
	//
	LightManager::Info light_info;
	light_info.m_color.red = spec->m_red.ComputeValue(m_age, m_seed);
	light_info.m_color.green = spec->m_green.ComputeValue(m_age, m_seed);
	light_info.m_color.blue = spec->m_blue.ComputeValue(m_age, m_seed);
	light_info.m_origin.Multiply(m_localToParent, *info->m_parentToWorld);
	light_info.m_intensity = spec->m_intensity.ComputeValue(m_age, m_seed);
	light_info.m_inner = spec->m_innerRadius.ComputeValue(m_age, m_seed);
	light_info.m_outer = spec->m_outerRadius.ComputeValue(m_age, m_seed);
	Check_Pointer(m_light);
	Check_Object(LightManager::Instance);
	LightManager::Instance->ChangeLight(m_light, &light_info);
	return true;
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointLight::Kill()
{
	Check_Object(this);

	Effect::Kill();
	Check_Pointer(m_light);
	LightManager::Instance->DeleteLight(m_light);
	m_light = NULL;
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointLight::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}
