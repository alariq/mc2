//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//###########################    MLRLight    ##################################
//#############################################################################

MLRLight::ClassData*
	MLRLight::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRLightClassID,
			"MidLevelRenderer::MLRLight",
			RegisteredClass::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLight::MLRLight(ClassData *class_data) :
	RegisteredClass(class_data)
{
	Verify(gos_GetCurrentHeap() == Heap);
	intensity = 1.0f;

	lightToWorld = LinearMatrix4D::Identity;
	lightToShape = LinearMatrix4D::Identity;

	color = RGBColor(0.0f, 0.0f, 0.0f);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLight::MLRLight(
	ClassData *class_data,
	Stuff::MemoryStream *stream,
	int version
) :
	RegisteredClass(class_data)
{
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	LinearMatrix4D matrix;
	*stream >> intensity >> color >> matrix;

	if(version>=9)
	{
		*stream >> lightName;
	}
	else
	{
		lightName = "Light";
	}

	SetLightToWorldMatrix(matrix);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLight::MLRLight(
	ClassData *class_data,
	Stuff::Page *page
) :
	RegisteredClass(class_data)
{
	Check_Object(page);
	Verify(gos_GetCurrentHeap() == Heap);

	lightName = page->GetName();

	intensity = 1.0f;
	page->GetEntry("Intensity", &intensity);

	color = RGBColor(0.0f, 0.0f, 0.0f);
	page->GetEntry("Color", &color);

	LinearMatrix4D matrix(true);
	Point3D position = Point3D::Identity;
	page->GetEntry("Position", &position);
	matrix.BuildTranslation(position);

	Vector3D direction(0.0f, 0.0f, 1.0f);
	page->GetEntry("Direction", &direction);
	matrix.AlignLocalAxisToWorldVector(direction, Z_Axis, Y_Axis, X_Axis);

	SetLightToWorldMatrix(matrix);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLight::~MLRLight()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLight*
	MLRLight::Make(
		Stuff::MemoryStream *stream,
		int version
	)
{
	gos_PushCurrentHeap(Heap);
	int type;
	MLRLight *light = NULL;
	*stream >> type;
	switch (type)
	{
	case AmbientLight:
		light = new MLRAmbientLight(stream, version);
		break;
	case InfiniteLight:
		light = new MLRInfiniteLight(MLRInfiniteLight::DefaultData, stream, version);
		break;
	case PointLight:
		light = new MLRPointLight(stream, version);
		break;
	case SpotLight:
		light = new MLRSpotLight(stream, version);
		break;
	case LookUpLight:
		light = new MLRLookUpLight(stream, version);
		break;
	default:
		STOP(("Bad light type"));
	}
	gos_PopCurrentHeap();
	return light;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLight*
	MLRLight::Make(Stuff::Page *page)
{
	gos_PushCurrentHeap(Heap);
	const char* type;
	
	page->GetEntry("LightType", &type, true);
	MLRLight *light = NULL;
	if (!_stricmp(type, "Ambient"))
		light = new MLRAmbientLight(page);
	else if (!_stricmp(type, "Infinite"))
		light = new MLRInfiniteLight(MLRInfiniteLight::DefaultData, page);
	else if (!_stricmp(type, "Point"))
		light = new MLRPointLight(page);
	else if (!_stricmp(type, "Spot"))
		light = new MLRSpotLight(page);
	else if (!_stricmp(type, "LookUp"))
		light = new MLRLookUpLight(page);
	else
		STOP(("Bad light type"));
	gos_PopCurrentHeap();
	return light;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	*stream << static_cast<int>(GetLightType());
	*stream << intensity << color << lightToWorld;
	*stream << lightName;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::Write(Stuff::Page *page)
{
	Check_Object(this);
	Check_Object(page);

	switch (GetLightType())
	{
		case AmbientLight:
			page->SetEntry("LightType", "Ambient");
		break;

		case InfiniteLight:
			page->SetEntry("LightType", "Infinite");
		break;

		case PointLight:
			page->SetEntry("LightType", "Point");
		break;

		case SpotLight:
			page->SetEntry("LightType", "Spot");
		break;

		case LookUpLight:
			page->SetEntry("LightType", "LookUp");
		break;
	}

	page->SetEntry("Intensity", intensity);
	page->SetEntry("Color", color);

	Point3D position(lightToWorld);
	page->SetEntry("Position", position);

	UnitVector3D direction;
	lightToWorld.GetLocalForwardInWorld(&direction);
	page->SetEntry("Direction", direction);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::SetLightToShapeMatrix(const LinearMatrix4D& worldToShape)
{
	Check_Object(this);

	lightToShape.Multiply(lightToWorld, worldToShape);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::SetLightToWorldMatrix(const LinearMatrix4D& matrix)
{
	Check_Object(this);

	lightToWorld = matrix;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::SetColor (Scalar r, Scalar g, Scalar b)
{
	SetColor(RGBColor(r, b, b));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLight::GetColor (Scalar& r, Scalar& g, Scalar& b)
{
	Check_Object(this);

	r = color.red;
	g = color.green;
	b = color.blue;
}
