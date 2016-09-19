//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"


//#############################################################################
//############################   MLREffect    #################################
//#############################################################################

void EffectClipPolygon::Init()
{
	Verify(gos_GetCurrentHeap() == StaticHeap);
	coords.SetLength(Limits::Max_Number_Vertices_Per_Polygon);
	colors.SetLength(Limits::Max_Number_Vertices_Per_Polygon);
	texCoords.SetLength(Limits::Max_Number_Vertices_Per_Polygon);
	clipPerVertex.SetLength(Limits::Max_Number_Vertices_Per_Polygon);
}

void EffectClipPolygon::Destroy()
{
	coords.SetLength(0);
	colors.SetLength(0);
	texCoords.SetLength(0);
	clipPerVertex.SetLength(0);
}

//#############################################################################
//############################   MLREffect    #################################
//#############################################################################


MLREffect::ClassData*
	MLREffect::DefaultData = NULL;
EffectClipPolygon
	*MLREffect::clipBuffer;

DynamicArrayOf<Vector4D>
	*MLREffect::transformedCoords;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLREffect::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLREffectClassID,
			"MidLevelRenderer::MLREffect",
			RegisteredClass::DefaultData
		);
	Register_Object(DefaultData);
	
	transformedCoords = new DynamicArrayOf<Vector4D> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(transformedCoords);

	clipBuffer = new EffectClipPolygon [2];
	Register_Pointer(clipBuffer);

	clipBuffer[0].Init();
	clipBuffer[1].Init();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLREffect::TerminateClass()
{
	clipBuffer[1].Destroy();
	clipBuffer[0].Destroy();
	Unregister_Pointer(clipBuffer);
	delete [] clipBuffer;

	Unregister_Object(transformedCoords);
	delete transformedCoords;

	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLREffect::MLREffect(int nr, ClassData *class_data):
	RegisteredClass(class_data)
{
	Verify(gos_GetCurrentHeap() == Heap);
	visible = 0;
	maxNrOf = nr;

	testList.SetLength(maxNrOf);
	for(int i=0; i < maxNrOf; i++)
	{
		testList[i] = 0;
	}
	
	TurnAllOff();
	TurnAllVisible();

	worldToEffect = LinearMatrix4D::Identity;

	gos_vertices = NULL;
	numGOSVertices = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLREffect::~MLREffect()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLREffect::Transform(int nrOfUsedEffects, int nrOfVertices)
{
	Check_Object(this);

	Start_Timer(Transform_Time);
	int i, j, k;

	for(i=0,j=0;i<nrOfUsedEffects;i++,j+=nrOfVertices)
	{
		if(IsOn(i) == false)
		{
			continue;
		}

		for(k=j;k<j+nrOfVertices;k++)
		{
			(*transformedCoords)[k].Multiply(points[k], effectToClipMatrix);
		}
	}
	Stop_Timer(Transform_Time);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLREffect::TurnAllOn()
{
	Check_Object(this);
	int i;

	for(i=0;i<maxNrOf;i++)
	{
		testList[i] |= 2;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLREffect::TurnAllOff()
{
	Check_Object(this);
	int i;

	for(i=0;i<maxNrOf;i++)
	{
		testList[i] &= ~2;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLREffect::TurnAllVisible()
{
	Check_Object(this);
	int i;

	for(i=0;i<maxNrOf;i++)
	{
		testList[i] |= 1;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLREffect::TurnAllInVisible()
{
	Check_Object(this);
	int i;

	for(i=0;i<maxNrOf;i++)
	{
		testList[i] &= ~1;
	}
}
