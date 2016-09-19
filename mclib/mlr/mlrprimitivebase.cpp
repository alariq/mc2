//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

int clipTrick[6][2] = 
{
	{ 1, 1},
	{ 1, 0},
	{ 0, 1},
	{ 0, 0},
	{ 2, 0},
	{ 2, 1}
};


//#############################################################################
//#########################    ClipPolygon2    ############################
//#############################################################################

void ClipPolygon2::Init(int passes)
{
	Verify(gos_GetCurrentHeap() == StaticHeap);
	coords.SetLength(Limits::Max_Number_Vertices_Per_Polygon);
	colors.SetLength(Limits::Max_Number_Vertices_Per_Polygon);
	texCoords.SetLength(passes*Limits::Max_Number_Vertices_Per_Polygon);
	clipPerVertex.SetLength(Limits::Max_Number_Vertices_Per_Polygon);
}

void ClipPolygon2::Destroy()
{
	coords.SetLength(0);
	colors.SetLength(0);
	texCoords.SetLength(0);
	clipPerVertex.SetLength(0);
}

//#############################################################################
//#########################    MLRPrimitiveBase    ############################
//#############################################################################

MLRPrimitiveBase::ClassData*
	MLRPrimitiveBase::DefaultData = NULL;

DynamicArrayOf<Vector4D>
	*MLRPrimitiveBase::transformedCoords;

DynamicArrayOf<MLRClippingState>
	*MLRPrimitiveBase::clipPerVertex;
DynamicArrayOf<Vector4D>
	*MLRPrimitiveBase::clipExtraCoords;

DynamicArrayOf<Vector2DScalar>
	*MLRPrimitiveBase::clipExtraTexCoords;

#if COLOR_AS_DWORD
DynamicArrayOf<DWORD>
#else
DynamicArrayOf<RGBAColor>
#endif
    // sebi baseuse it is defined in base class!
	//*MLR_I_C_PMesh::clipExtraColors;
	*MLRPrimitiveBase::clipExtraColors;

DynamicArrayOf<unsigned short>
	*MLRPrimitiveBase::clipExtraLength;

ClipPolygon2
	*MLRPrimitiveBase::clipBuffer;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRPrimitiveBaseClassID,
			"MidLevelRenderer::MLRPrimitiveBase",
			RegisteredClass::DefaultData,
			NULL
		);
	Register_Object(DefaultData);

	transformedCoords = new DynamicArrayOf<Vector4D> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(transformedCoords);

	clipPerVertex = new DynamicArrayOf<MLRClippingState> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(clipPerVertex);
	clipExtraCoords = new DynamicArrayOf<Vector4D> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(clipExtraCoords);
	clipExtraTexCoords = new DynamicArrayOf<Vector2DScalar> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(clipExtraTexCoords);

	clipExtraColors = new DynamicArrayOf<
#if COLOR_AS_DWORD
		DWORD
#else
		RGBAColor
#endif
	> (Limits::Max_Number_Primitives_Per_Frame);
	Register_Object(clipExtraColors);

	clipExtraLength = new DynamicArrayOf<unsigned short> (Limits::Max_Number_Primitives_Per_Frame);
	Register_Object(clipExtraLength);

	clipBuffer = new ClipPolygon2 [2];
	Register_Pointer(clipBuffer);

	clipBuffer[0].Init(Limits::Max_Number_Of_Multitextures);
	clipBuffer[1].Init(Limits::Max_Number_Of_Multitextures);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::TerminateClass()
{
	clipBuffer[1].Destroy();
	clipBuffer[0].Destroy();
	Unregister_Pointer(clipBuffer);
	delete [] clipBuffer;

	Unregister_Object(transformedCoords);
	delete transformedCoords;

	Unregister_Object(clipPerVertex);
	delete clipPerVertex;
	Unregister_Object(clipExtraCoords);
	delete clipExtraCoords;
	Unregister_Object(clipExtraTexCoords);
	delete clipExtraTexCoords;
	Unregister_Object(clipExtraColors);
	delete clipExtraColors;
	Unregister_Object(clipExtraLength);
	delete clipExtraLength;


	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPrimitiveBase::MLRPrimitiveBase(
	ClassData *class_data,
	MemoryStream *stream,
	int version
):
	RegisteredClass(class_data)
{
	Check_Pointer(this);
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	switch(version)
	{
		case 1:
		case 2:
		{
			STOP(("This class got created only after version 2 !"));
		}
		break;
		default:
		{
			MemoryStreamIO_Read(stream, &coords);

			MemoryStreamIO_Read(stream, &texCoords);

			MemoryStreamIO_Read(stream, &lengths);

			*stream >> drawMode;

			referenceState.Load(stream, version);
		}
		break;
	}

	passes = 1;
	referenceCount = 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	*stream << GetClassID();

	MemoryStreamIO_Write(stream, &coords);

	MemoryStreamIO_Write(stream, &texCoords);

	MemoryStreamIO_Write(stream, &lengths);

	*stream << static_cast<int>(drawMode);

	referenceState.Save(stream);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPrimitiveBase::MLRPrimitiveBase(ClassData *class_data):
	RegisteredClass(class_data),
	lengths(0),	texCoords(0), coords(0)
{
	Verify(gos_GetCurrentHeap() == Heap);
	referenceState = 0;
	
	state = 0;

	passes = 1;

	referenceCount = 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPrimitiveBase::~MLRPrimitiveBase()
{
	Verify(referenceCount==0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::InitializeDraw()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::InitializeDrawPrimitive(unsigned char vis, int)
{
	gos_vertices = NULL;
	numGOSVertices = -1;

	visible = vis;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::SetSubprimitiveLengths(unsigned char *data, int l)
{
	Check_Object(this); 
	lengths.AssignData(data, l);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::GetSubprimitiveLengths(unsigned char **data, int *len)
{
	Check_Object(this); 
	*data = lengths.GetData();
	*len = lengths.GetLength();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRPrimitiveBase::GetSubprimitiveLength (int i) const
{ 
	Check_Object(this); 
	return (lengths.GetLength() > 0 ? abs(lengths[i]) : 1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::SetCoordData(
		const Point3D *data,
		int dataSize
	)
{
	Check_Object(this); 
	Check_Pointer(data);

	Verify(texCoords.GetLength() == 0 || dataSize == texCoords.GetLength());

#if defined (MAX_NUMBER_VERTICES)
	Verify(dataSize <= MAX_NUMBER_VERTICES);
#endif
	coords.AssignData(data, dataSize);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::GetCoordData(
		Point3D **data,
		int *dataSize
	)
{
	Check_Object(this);

	*data = coords.GetData();
	*dataSize = coords.GetLength();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::SetTexCoordData(
		const Vector2DScalar *data,
		int dataSize
	)
{
	Check_Object(this); 
	Check_Pointer(data);

	Verify(coords.GetLength() == 0 || dataSize == coords.GetLength());

	texCoords.AssignData((Vector2DScalar *)data, dataSize);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::GetTexCoordData(
		Vector2DScalar **data,
		int *dataSize
	)
{
	Check_Object(this); 
	*data = texCoords.GetData();
	*dataSize = texCoords.GetLength();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::Transform(Matrix4D *mat)
{
	Check_Object(this);

	int i, len = coords.GetLength();

	for(i=0;i<len;i++)
	{
		(*transformedCoords)[i].Multiply(coords[i], *mat);
	}

	#ifdef LAB_ONLY
		Set_Statistic(TransformedVertices, TransformedVertices+len);
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPrimitiveBase::GetExtend(Stuff::ExtentBox *box)
{
	Check_Object(this);
	Check_Object(box);

	if(coords.GetLength()==0)
	{
		return;
	}

	box->minX = box->maxX = coords[0].x;
	box->minY = box->maxY = coords[0].y;
	box->minZ = box->maxZ = coords[0].z;

	for(int i=0;i<coords.GetLength();i++)
	{
		if(coords[i].x < box->minX)
		{
			box->minX = coords[i].x;
		}
		if(coords[i].y < box->minY)
		{
			box->minY = coords[i].y;
		}
		if(coords[i].z < box->minZ)
		{
			box->minZ = coords[i].z;
		}
		if(coords[i].x > box->maxX)
		{
			box->maxX = coords[i].x;
		}
		if(coords[i].y > box->maxY)
		{
			box->maxY = coords[i].y;
		}
		if(coords[i].z > box->maxZ)
		{
			box->maxZ = coords[i].z;
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MLRPrimitiveBase::CastRay(
		Line3D *line,
		Normal3D *normal
	)
{
	Check_Object(this);
	Check_Object(line);
	Check_Pointer(normal);
	STOP(("Not implemented"));
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape*
	MidLevelRenderer::CreateIndexedIcosahedron(IcoInfo& icoInfo, Stuff::DynamicArrayOf<MLRState> *states)
{
	switch(icoInfo.type)
	{
		case MLR_I_PMeshClassID:
			return CreateIndexedIcosahedron_NoColor_NoLit(icoInfo, &(*states)[0]);
		break;
		case MLR_I_C_PMeshClassID:
			return CreateIndexedIcosahedron_Color_NoLit(icoInfo, &(*states)[0]);
		break;
		case MLR_I_L_PMeshClassID:
			return CreateIndexedIcosahedron_Color_Lit(icoInfo, &(*states)[0]);
		break;
		case MLR_I_DT_PMeshClassID:
			return CreateIndexedIcosahedron_NoColor_NoLit_2Tex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_C_DT_PMeshClassID:
			return CreateIndexedIcosahedron_Color_NoLit_2Tex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_L_DT_PMeshClassID:
			return CreateIndexedIcosahedron_Color_Lit_2Tex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_MT_PMeshClassID:
			return CreateIndexedIcosahedron_NoColor_NoLit_MultiTexture(icoInfo, states);
		break;
		case MLR_I_DeT_PMeshClassID:
			return CreateIndexedIcosahedron_NoColor_NoLit_DetTex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_C_DeT_PMeshClassID:
			return CreateIndexedIcosahedron_Color_NoLit_DetTex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_L_DeT_PMeshClassID:
			return CreateIndexedIcosahedron_Color_Lit_DetTex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_TMeshClassID:
			return CreateIndexedTriIcosahedron_NoColor_NoLit(icoInfo, &(*states)[0]);
		break;
		case MLR_I_C_TMeshClassID:
			return CreateIndexedTriIcosahedron_Color_NoLit(icoInfo, &(*states)[0]);
		break;
		case MLR_I_L_TMeshClassID:
			return CreateIndexedTriIcosahedron_Color_Lit(icoInfo, &(*states)[0]);
		break;
		case MLR_I_DeT_TMeshClassID:
			return CreateIndexedTriIcosahedron_NoColor_NoLit_DetTex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_C_DeT_TMeshClassID:
			return CreateIndexedTriIcosahedron_Color_NoLit_DetTex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_L_DeT_TMeshClassID:
			return CreateIndexedTriIcosahedron_Color_Lit_DetTex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_DT_TMeshClassID:
			return CreateIndexedTriIcosahedron_NoColor_NoLit_2Tex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_C_DT_TMeshClassID:
			return CreateIndexedTriIcosahedron_Color_NoLit_2Tex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_I_L_DT_TMeshClassID:
			return CreateIndexedTriIcosahedron_Color_Lit_2Tex(icoInfo, &(*states)[0], &(*states)[1]);
		break;
		case MLR_TerrainClassID:
			return CreateIndexedTriIcosahedron_TerrainTest(icoInfo, &(*states)[0], &(*states)[1]);
		break;
	}
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
const char *
	MidLevelRenderer::IcoInfo::GetTypeName()
{
	switch(type)
	{
		case MLR_I_PMeshClassID:
			return "not colored, unlit mesh";
		break;
		case MLR_I_C_PMeshClassID:
			return "colored, unlit mesh";
		break;
		case MLR_I_L_PMeshClassID:
			return "colored, lit mesh";
		break;
		case MLR_I_DT_PMeshClassID:
			return "not colored, unlit mesh w/ 2 Tex";
		break;
		case MLR_I_C_DT_PMeshClassID:
			return "colored, unlit mesh w/ 2 Tex";
		break;
		case MLR_I_L_DT_PMeshClassID:
			return "colored, lit mesh w/ 2 Tex";
		break;
		case MLR_I_MT_PMeshClassID:
			return "not colored, unlit mesh w/ 4 Tex";
		break;
		case MLR_I_DeT_PMeshClassID:
			return "not colored, unlit mesh w/ DetTex";
		break;
		case MLR_I_C_DeT_PMeshClassID:
			return "colored, unlit mesh w/ DetTex";
		break;
		case MLR_I_L_DeT_PMeshClassID:
			return "colored, lit mesh w/ DetTex";
		break;
		case MLR_I_TMeshClassID:
			return "not colored, unlit tri. mesh";
		break;
		case MLR_I_C_TMeshClassID:
			return "colored, unlit tri. mesh";
		break;
		case MLR_I_L_TMeshClassID:
			return "colored, lit tri. mesh";
		break;
		case MLR_I_DeT_TMeshClassID:
			return "not colored, unlit tri. mesh w/ DetTex";
		break;
		case MLR_I_C_DeT_TMeshClassID:
			return "colored, unlit tri. mesh w/ DetTex";
		break;
		case MLR_I_L_DeT_TMeshClassID:
			return "colored, lit tri. mesh w/ DetTex";
		break;
		case MLR_I_DT_TMeshClassID:
			return "not colored, unlit tri. mesh w/ 2 Tex";
		break;
		case MLR_I_C_DT_TMeshClassID:
			return "colored, unlit tri. mesh w/ 2 Tex";
		break;
		case MLR_I_L_DT_TMeshClassID:
			return "colored, lit tri. mesh w/ 2 Tex";
		break;
		case MLR_TerrainClassID:
			return "not colored, unlit terrain w/ DetTex";
		break;
	}
	return "mesh";
}

//#############################################################################
//#####################    MLRPrimitiveBase__ClassData    #####################
//#############################################################################

void
	MLRPrimitiveBase__ClassData::TestInstance()
{
	Verify(IsDerivedFrom(MLRPrimitiveBase::DefaultData));
}
