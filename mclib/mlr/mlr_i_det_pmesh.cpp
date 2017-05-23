//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

extern DWORD gEnableDetailTexture;

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	BitTrace *MLR_I_DeT_PMesh_Clip;
#endif

//#############################################################################
//###### MLRIndexedPolyMesh with no color no lighting two texture layer  ######
//#############################################################################

MLR_I_DeT_PMesh::ClassData*
	MLR_I_DeT_PMesh::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_DeT_PMesh::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLR_I_DeT_PMeshClassID,
			"MidLevelRenderer::MLR_I_DeT_PMesh",
			MLR_I_PMesh::DefaultData,
			(MLRPrimitiveBase::Factory)&Make
		);
	Register_Object(DefaultData);
	
	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		MLR_I_DeT_PMesh_Clip = new BitTrace("MLR_I_DeT_PMesh_Clip");
		Register_Object(MLR_I_DeT_PMesh_Clip);
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_DeT_PMesh::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		Unregister_Object(MLR_I_DeT_PMesh_Clip);
		delete MLR_I_DeT_PMesh_Clip;
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_DeT_PMesh::MLR_I_DeT_PMesh(
	ClassData *class_data,
	MemoryStream *stream,
	int version
):
	MLR_I_PMesh(class_data, stream, version)
{
	Check_Pointer(this);
	Check_Pointer(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	referenceState2.Load(stream, version);

	*stream >> xOffset >> yOffset >> xScale >> yScale;

	noDetailDistance = 2*xScale;

	referenceState2.SetZBufferWriteOff();

	referenceState.SetPriority(MLRState::DefaultPriority);
	referenceState2.SetPriority(MLRState::DefaultPriority);

#if 0
	referenceState.SetPriority(MLRState::DefaultPriority+1);

	referenceState2.SetAlphaMode(MLRState::OneZeroMode);
	referenceState.SetAlphaMode(MLRState::AlphaInvAlphaMode);
#endif

	detTextureVisible = true;

	passes = 2;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_DeT_PMesh::MLR_I_DeT_PMesh(ClassData *class_data):
	MLR_I_PMesh(class_data)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);

	noDetailDistance = -1.0f;
	detTextureVisible = true;
	passes = 2;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_DeT_PMesh::Copy(
		MLR_I_PMesh*,
		MLRState detailState,
		Stuff::Scalar xOff,
		Stuff::Scalar yOff,
		Stuff::Scalar xFac,
		Stuff::Scalar yFac
	)
{
	Check_Pointer(this);

	detTextureVisible = true;

	STOP(("No conversion implemented yet !?"));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_DeT_PMesh::~MLR_I_DeT_PMesh()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_DeT_PMesh*
	MLR_I_DeT_PMesh::Make(
		MemoryStream *stream,
		int version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLR_I_DeT_PMesh *mesh = new MLR_I_DeT_PMesh(DefaultData, stream, version);
	gos_PopCurrentHeap();

	return mesh;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_DeT_PMesh::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLR_I_PMesh::Save(stream);

	referenceState2.Save(stream);

	*stream << xOffset << yOffset << xScale << yScale;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLR_I_DeT_PMesh::GetNumPasses()
{
	Check_Object(this);
	if(gEnableDetailTexture==0 || detTextureVisible==false)
		return 1;
	
	return passes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_DeT_PMesh::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

#define I_SAY_YES_TO_DETAIL_TEXTURES
#undef I_SAY_YES_TO_DUAL_TEXTURES
#undef I_SAY_YES_TO_COLOR
#undef I_SAY_YES_TO_LIGHTING

#define CLASSNAME MLR_I_DeT_PMesh

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	#define SET_MLR_PMESH_CLIP() MLR_I_DeT_PMesh_Clip->Set()
	#define CLEAR_MLR_PMESH_CLIP() MLR_I_DeT_PMesh_Clip->Clear()
#else
	#define SET_MLR_PMESH_CLIP()
	#define CLEAR_MLR_PMESH_CLIP()
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	This include contains follwing functions:
//	void MLR_I_DeT_PMesh::TransformNoClip(Matrix4D*, GOSVertexPool*);
//	int MLR_I_DeT_PMesh::Clip(MLRClippingState, GOSVertexPool*);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include<mlr/mlrprimitiveclipping.hpp>

#undef I_SAY_YES_TO_DETAIL_TEXTURES
#undef CLASSNAME

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_DeT_PMesh*
	MidLevelRenderer::CreateIndexedCube_NoColor_NoLit_DetTex(
		Scalar half,
		MLRState *state,
		MLRState *state2,
		Scalar xOff,
		Scalar yOff,
		Scalar xFac,
		Scalar yFac
	)
{
	gos_PushCurrentHeap(Heap);
	MLR_I_DeT_PMesh *ret = new MLR_I_DeT_PMesh();
	Register_Object(ret);

	ret->SetDetailData(xOff, yOff, xFac, yFac);

	Point3D *coords = new Point3D [8];
	Register_Object(coords);

	coords[0] = Point3D( half, -half,  half);
	coords[1] = Point3D(-half, -half,  half);
	coords[2] = Point3D( half, -half, -half);
	coords[3] = Point3D(-half, -half, -half);
	coords[4] = Point3D(-half,  half,  half);
	coords[5] = Point3D( half,  half,  half);
	coords[6] = Point3D( half,  half, -half);
	coords[7] = Point3D(-half,  half, -half);

	unsigned char *lengths = new unsigned char [6];
	Register_Pointer(lengths);

	int i;

	for(i=0;i<6;i++)
	{
		lengths[i] = 4;
	}

	ret->SetSubprimitiveLengths(lengths, 6);

	ret->SetCoordData(coords, 8);

	unsigned short	*index = new unsigned short [6*4];
	Register_Pointer(index);

	index[0] = 0;
	index[1] = 2;
	index[2] = 6;
	index[3] = 5;

	index[4] = 0;
	index[5] = 5;
	index[6] = 4;
	index[7] = 1;

	index[8] = 5;
	index[9] = 6;
	index[10] = 7;
	index[11] = 4;

	index[12] = 2;
	index[13] = 3;
	index[14] = 7;
	index[15] = 6;

	index[16] = 1;
	index[17] = 4;
	index[18] = 7;
	index[19] = 3;

	index[20] = 0;
	index[21] = 1;
	index[22] = 3;
	index[23] = 2;

	ret->SetIndexData(index, 6*4);

	ret->FindFacePlanes();

	Vector2DScalar *texCoords = new Vector2DScalar[16];
	Register_Object(texCoords);

	texCoords[0] = Vector2DScalar(0.0f, 0.0f);
	texCoords[1] = Vector2DScalar(0.0f, 0.0f);
	texCoords[2] = Vector2DScalar(0.0f, 0.0f);
	texCoords[3] = Vector2DScalar(0.0f, 0.0f);

	texCoords[4] = Vector2DScalar(0.0f, 0.0f);
	texCoords[5] = Vector2DScalar(0.0f, 0.0f);
	texCoords[6] = Vector2DScalar(0.0f, 0.0f);
	texCoords[7] = Vector2DScalar(0.0f, 0.0f);

	texCoords[8] = Vector2DScalar(0.0f, 0.0f);
	texCoords[9] = Vector2DScalar(0.0f, 0.0f);
	texCoords[10] = Vector2DScalar(0.0f, 0.0f);
	texCoords[12] = Vector2DScalar(0.0f, 0.0f);

	texCoords[13] = Vector2DScalar(0.0f, 0.0f);
	texCoords[14] = Vector2DScalar(0.0f, 0.0f);
	texCoords[15] = Vector2DScalar(0.0f, 0.0f);
	// sebi: aray size is only 16! ORIG BUG FIX
	//texCoords[16] = Vector2DScalar(0.0f, 0.0f);

	if(state != NULL)
	{
		ret->SetReferenceState(*state);
		if(state->GetTextureHandle() > 0)
		{
			texCoords[0] = Vector2DScalar(0.0f, 0.0f);
			texCoords[1] = Vector2DScalar(1.0f, 0.0f);
			texCoords[2] = Vector2DScalar(0.25f, 0.25f);
			texCoords[3] = Vector2DScalar(0.75f, 0.25f);

			texCoords[4] = Vector2DScalar(1.0f, 1.0f);
			texCoords[5] = Vector2DScalar(0.0f, 1.0f);
			texCoords[6] = Vector2DScalar(0.25f, 0.75f);
			texCoords[7] = Vector2DScalar(0.75f, 0.75f);
		}
	}

	if(state2 != NULL)
	{
		ret->SetReferenceState(*state, 1);
		if(state2->GetTextureHandle() > 0)
		{
			texCoords[8] = Vector2DScalar(0.0f, 0.0f);
			texCoords[9] = Vector2DScalar(1.0f, 0.0f);
			texCoords[10] = Vector2DScalar(0.25f, 0.25f);
			texCoords[11] = Vector2DScalar(0.75f, 0.25f);

			texCoords[12] = Vector2DScalar(1.0f, 1.0f);
			texCoords[13] = Vector2DScalar(0.0f, 1.0f);
			texCoords[14] = Vector2DScalar(0.25f, 0.75f);
			texCoords[15] = Vector2DScalar(0.75f, 0.75f);
		}
	}

	ret->SetTexCoordData(texCoords, 16);

	Unregister_Object(texCoords);
	delete [] texCoords;

	Unregister_Pointer(index);
	delete [] index;

	Unregister_Pointer(lengths);
	delete [] lengths;

	Unregister_Object(coords);
	delete [] coords;

	gos_PopCurrentHeap();
	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape*
	MidLevelRenderer::CreateIndexedIcosahedron_NoColor_NoLit_DetTex(
		IcoInfo& icoInfo,
		MLRState *state,
		MLRState *stateDet
	)
{
	gos_PushCurrentHeap(Heap);
	MLRShape *ret = new MLRShape(20);
	Register_Object(ret);

	int i, j, k;
	long    nrTri = (long) ceil (icoInfo.all * pow (4.0f, icoInfo.depth));
	Point3D v[3];

	if(3*nrTri >= Limits::Max_Number_Vertices_Per_Mesh)
	{
		nrTri = Limits::Max_Number_Vertices_Per_Mesh/3;
	}

	unsigned char *lengths = new unsigned char [nrTri];
	Register_Pointer(lengths);

	for(i=0;i<nrTri;i++)
	{
		lengths[i] = 3;
	}

	Point3D *coords = new Point3D [nrTri*3];
	Register_Pointer(coords);
	
	Point3D *collapsedCoords = NULL;
	if(icoInfo.indexed==true)
	{
		collapsedCoords = new Point3D [nrTri*3];
		Register_Pointer(collapsedCoords);
	}

	unsigned short	*index = new unsigned short [nrTri*3];
	Register_Pointer(index);
	Vector2DScalar *texCoords = new Vector2DScalar[nrTri*3];
	Register_Pointer(texCoords);

	int uniquePoints = 0;
	for (k=0;k<20;k++)
	{
		triDrawn = 0;
		MLR_I_DeT_PMesh *mesh = new MLR_I_DeT_PMesh();
		Register_Object(mesh);

// setup vertex position information
	    for (j=0;j<3;j++)
		{
			v[j].x = vdata[tindices[k][j]][0];
			v[j].y = vdata[tindices[k][j]][1];
			v[j].z = vdata[tindices[k][j]][2];
		}
		subdivide (coords, v[0], v[1], v[2], icoInfo.depth, nrTri, icoInfo.radius);

		mesh->SetSubprimitiveLengths(lengths, nrTri);

		if(icoInfo.indexed==true)
		{
			uniquePoints = 1;
			collapsedCoords[0] = coords[0];
			index[0] = 0;

			for(i=1;i<nrTri*3;i++)
			{
				for(j=0;j<uniquePoints;j++)
				{
					if(coords[i] == collapsedCoords[j])
					{
						break;
					}
				}
				if(j==uniquePoints)
				{
					collapsedCoords[uniquePoints++] = coords[i];
				}
				index[i] = static_cast<unsigned short>(j);
			}
			mesh->SetCoordData(collapsedCoords, uniquePoints);
		}
		else
		{
			uniquePoints = nrTri*3;
			for(i=0;i<nrTri*3;i++)
			{
				index[i] = static_cast<unsigned short>(i);
			}
			mesh->SetCoordData(coords, nrTri*3);
		}

		mesh->SetIndexData(index, nrTri*3);

		mesh->FindFacePlanes();

		if(state == NULL)
		{
			for(i=0;i<uniquePoints;i++)
			{
				texCoords[i] = Vector2DScalar(0.0f, 0.0f);
			}
		}
		else
		{
			mesh->SetReferenceState(*state);
			if(state->GetTextureHandle() > 0)
			{
				if(icoInfo.indexed==true)
				{
					for(i=0;i<uniquePoints;i++)
					{
						texCoords[i] = 
							Vector2DScalar(
								(1.0f + collapsedCoords[i].x)/2.0f,
								(1.0f + collapsedCoords[i].y)/2.0f
							);
					}
				}
				else
				{
					for(i=0;i<nrTri;i++)
					{
						texCoords[3*i] = 
							Vector2DScalar(
								(1.0f + coords[3*i].x)/2.0f,
								(1.0f + coords[3*i].y)/2.0f
							);
						texCoords[3*i+1] = 
							Vector2DScalar(
								(1.0f + coords[3*i+1].x)/2.0f,
								(1.0f + coords[3*i+1].y)/2.0f
							);
						texCoords[3*i+2] = 
							Vector2DScalar(
								(1.0f + coords[3*i+2].x)/2.0f,
								(1.0f + coords[3*i+2].y)/2.0f
							);
					}
				}
			}
			else
			{
				for(i=0;i<uniquePoints;i++)
				{
					texCoords[i] = Vector2DScalar(0.0f, 0.0f);
				}
			}
		}

		mesh->SetTexCoordData(texCoords, uniquePoints);

		mesh->SetDetailData(0.0f, 0.0f, 16.0f, 16.0f);
		mesh->SetReferenceState(*stateDet, 1);

		ret->Add(mesh);
		mesh->DetachReference();
	}

	Unregister_Pointer(texCoords);
	delete [] texCoords;

	Unregister_Pointer(index);
	delete [] index;

	if(icoInfo.indexed==true)
	{
		Unregister_Pointer(collapsedCoords);
		delete [] collapsedCoords;
	}
	
	Unregister_Pointer(coords);
	delete [] coords;
	
	Unregister_Pointer(lengths);
	delete [] lengths;

	gos_PopCurrentHeap();
	return ret;
}
