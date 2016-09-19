//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	BitTrace *MLR_I_MT_PMesh_Clip;
#endif

//#############################################################################
//###### MLRIndexedPolyMesh with no color no lighting two texture layer  ######
//#############################################################################

MLR_I_MT_PMesh::ClassData*
	MLR_I_MT_PMesh::DefaultData = NULL;

DynamicArrayOf<DynamicArrayOf<Vector2DScalar> >
	*MLR_I_MT_PMesh::clipExtraMultiTexCoords;
DynamicArrayOf<DynamicArrayOf<Vector2DScalar> >
	*MLR_I_MT_PMesh::extraMultiTexCoords;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLR_I_MT_PMeshClassID,
			"MidLevelRenderer::MLR_I_MT_PMesh",
			MLR_I_PMesh::DefaultData,
			(MLRPrimitiveBase::Factory)&Make
		);
	Register_Object(DefaultData);
	
	clipExtraMultiTexCoords = new DynamicArrayOf<DynamicArrayOf<Vector2DScalar> >;
	Register_Object(clipExtraMultiTexCoords);
	clipExtraMultiTexCoords->SetLength(Limits::Max_Number_Of_Multitextures);
	
	extraMultiTexCoords = new DynamicArrayOf<DynamicArrayOf<Vector2DScalar> >;
	Register_Object(extraMultiTexCoords);
	extraMultiTexCoords->SetLength(Limits::Max_Number_Of_Multitextures);

	for(int i=0;i<clipExtraMultiTexCoords->GetLength();i++)
	{
		(*clipExtraMultiTexCoords)[i].SetLength(Limits::Max_Number_Vertices_Per_Mesh);
		(*extraMultiTexCoords)[i].SetLength(Limits::Max_Number_Vertices_Per_Mesh);
	}

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		MLR_I_MT_PMesh_Clip = new BitTrace("MLR_I_MT_PMesh_Clip");
		Register_Object(MLR_I_MT_PMesh_Clip);
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::TerminateClass()
{
	for(int i=0;i<clipExtraMultiTexCoords->GetLength();i++)
	{
		(*clipExtraMultiTexCoords)[i].SetLength(0);
		(*extraMultiTexCoords)[i].SetLength(0);
	}
	Unregister_Object(clipExtraMultiTexCoords);
	delete clipExtraMultiTexCoords;

	Unregister_Object(extraMultiTexCoords);
	delete extraMultiTexCoords;

	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		Unregister_Object(MLR_I_MT_PMesh_Clip);
		delete MLR_I_MT_PMesh_Clip;
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_MT_PMesh::MLR_I_MT_PMesh(
	ClassData *class_data,
	MemoryStream *stream,
	int version
):
	MLR_I_PMesh(class_data, stream, version)
{
	Check_Pointer(this);
	Check_Pointer(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	multiTexCoords.SetLength(Limits::Max_Number_Of_Multitextures);
	multiTexCoordsPointers.SetLength(Limits::Max_Number_Of_Multitextures);
	multiState.SetLength(Limits::Max_Number_Of_Multitextures);
	multiReferenceState.SetLength(Limits::Max_Number_Of_Multitextures);

	multiTexCoords[0] = &texCoords;
	multiTexCoordsPointers[0] = texCoords.GetData();

	multiReferenceState[0] = referenceState;

	*stream >> passes;

	for(int i=1;i<passes;i++)
	{
		multiReferenceState[i].Save(stream);

		multiTexCoords[i] = new DynamicArrayOf<Vector2DScalar>;
		Register_Object(multiTexCoords[i]);

		MemoryStreamIO_Read(stream, multiTexCoords[i]);

		multiTexCoordsPointers[i] = multiTexCoords[i]->GetData();
	}

	currentNrOfPasses = passes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_MT_PMesh::MLR_I_MT_PMesh(ClassData *class_data):
	MLR_I_PMesh(class_data)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);

	multiTexCoords.SetLength(Limits::Max_Number_Of_Multitextures);
	multiTexCoordsPointers.SetLength(Limits::Max_Number_Of_Multitextures);

	multiTexCoords[0] = &texCoords;
	multiTexCoordsPointers[0] = texCoords.GetData();

	multiState.SetLength(Limits::Max_Number_Of_Multitextures);
	multiReferenceState.SetLength(Limits::Max_Number_Of_Multitextures);

	passes = 1;

	currentNrOfPasses = passes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::Copy(MLR_I_PMesh *pMesh)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);

	int num;
	Point3D *points;

	referenceState = pMesh->GetReferenceState();
	multiReferenceState[0] = referenceState;

	pMesh->GetCoordData(&points, &num);
	SetCoordData(points, num);

	Vector2DScalar *tex;
	pMesh->GetTexCoordData(&tex, &num);
	SetTexCoordData(tex, num);

	multiTexCoords[0] = &texCoords;
	multiTexCoordsPointers[0] = texCoords.GetData();

	unsigned char *length_array;
	pMesh->GetSubprimitiveLengths(&length_array, &num);
	SetSubprimitiveLengths(length_array, num);

	drawMode = pMesh->GetSortDataMode();

	unsigned short *index_array;
	pMesh->GetIndexData(&index_array, &num);
	SetIndexData(index_array, num);

	facePlanes.SetLength(GetNumPrimitives());
	testList.SetLength(GetNumPrimitives());

	FindFacePlanes();

	passes = 1;

	currentNrOfPasses = passes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_MT_PMesh::~MLR_I_MT_PMesh()
{
	Check_Object(this);

	for(int i=1;i<passes;i++)
	{
		Unregister_Object(multiTexCoords[i]);
		delete multiTexCoords[i];
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_MT_PMesh*
	MLR_I_MT_PMesh::Make(
		MemoryStream *stream,
		int version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLR_I_MT_PMesh *mesh = new MLR_I_MT_PMesh(DefaultData, stream, version);
	gos_PopCurrentHeap();

	return mesh;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLR_I_PMesh::Save(stream);

	*stream << passes;

	for(int i=1;i<passes;i++)
	{
		multiReferenceState[i].Save(stream);

		MemoryStreamIO_Write(stream, multiTexCoords[i]);
	}
}
 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::InitializeDrawPrimitive(unsigned char vis, int parameter)
{
	MLR_I_PMesh::InitializeDrawPrimitive(vis, parameter);

	currentNrOfPasses = passes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::SetTexCoordData(
		const Vector2DScalar *data,
		int dataSize,
		int pass
	)
{
	Check_Object(this); 
	Check_Pointer(data);

	Verify(pass>=0 && pass<Limits::Max_Number_Of_Multitextures);
	Verify(coords.GetLength() == 0 || dataSize == coords.GetLength());

	if(pass == 0)
	{
		texCoords.AssignData((Vector2DScalar *)data, dataSize);

		multiTexCoordsPointers[0] = texCoords.GetData();
	}
	else
	{
		if(pass == passes)
		{
			Verify(dataSize == multiTexCoords[pass-1]->GetLength());

			gos_PushCurrentHeap(Heap);
			multiTexCoords[pass] = new DynamicArrayOf<Vector2DScalar>;
			Register_Object(multiTexCoords[pass]);
			multiTexCoords[pass]->AssignData((Vector2DScalar *)data, dataSize);
			gos_PopCurrentHeap();

			multiTexCoordsPointers[pass] = multiTexCoords[pass]->GetData();
			
			passes++;
			currentNrOfPasses++;
		}
		else
		{
			STOP(("Add texture layers only one by one !!!"));
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::GetTexCoordData(
		const Vector2DScalar **data,
		int *dataSize,
		int pass
	)
{
	Check_Object(this); 
	Check_Pointer(data);

	Verify(pass>=0 && pass<passes);

	*data = multiTexCoords[pass]->GetData();
	*dataSize = multiTexCoords[pass]->GetLength();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_MT_PMesh::SetTexCoordDataPointer(const Vector2DScalar *data)
{
	Check_Pointer(data);

	Verify(currentNrOfPasses>=passes);

	multiTexCoordsPointers[currentNrOfPasses++] = data;
}

#define I_SAY_YES_TO_MULTI_TEXTURES
#undef I_SAY_YES_TO_DUAL_TEXTURES
#undef I_SAY_YES_TO_COLOR
#undef I_SAY_YES_TO_LIGHTING

#define CLASSNAME MLR_I_MT_PMesh

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	#define SET_MLR_PMESH_CLIP() MLR_I_MT_PMesh_Clip->Set()
	#define CLEAR_MLR_PMESH_CLIP() MLR_I_MT_PMesh_Clip->Clear()
#else
	#define SET_MLR_PMESH_CLIP()
	#define CLEAR_MLR_PMESH_CLIP()
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	This include contains follwing functions:
//	void MLR_I_MT_PMesh::TransformNoClip(Matrix4D*, GOSVertexPool*);
//	int MLR_I_MT_PMesh::Clip(MLRClippingState, GOSVertexPool*);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include<mlr/mlrprimitiveclipping.hpp>

#undef I_SAY_YES_TO_MULTI_TEXTURES
#undef CLASSNAME

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape*
	MidLevelRenderer::CreateIndexedIcosahedron_NoColor_NoLit_MultiTexture(
		IcoInfo& icoInfo,
		DynamicArrayOf<MLRState> *states
	)
{
	gos_PushCurrentHeap(Heap);
	MLRShape *ret = new MLRShape(20);
	Register_Object(ret);

	int i, j, k;
	int nrOfPasses = states->GetLength();
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

	DynamicArrayOf<Vector2DScalar *> texCoords(nrOfPasses);
	for(i=0;i<nrOfPasses;i++)
	{
		texCoords[i] = new Vector2DScalar[nrTri*3];
		Register_Pointer(texCoords[i]);
	}

	unsigned short	*index = new unsigned short [nrTri*3];
	Register_Pointer(index);

	int uniquePoints = 0;
	for (k=0;k<20;k++)
	{
		triDrawn = 0;
		MLR_I_MT_PMesh *mesh = new MLR_I_MT_PMesh();
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

		for(j=0;j<nrOfPasses;j++)
		{
			if(icoInfo.indexed==true)
			{
				for(i=0;i<uniquePoints;i++)
				{
					texCoords[j][i] = 
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
					texCoords[j][3*i] = 
						Vector2DScalar(
							(1.0f + coords[3*i].x)/2.0f,
							(1.0f + coords[3*i].y)/2.0f
						);
					texCoords[j][3*i+1] = 
						Vector2DScalar(
							(1.0f + coords[3*i+1].x)/2.0f,
							(1.0f + coords[3*i+1].y)/2.0f
						);
					texCoords[j][3*i+2] = 
						Vector2DScalar(
							(1.0f + coords[3*i+2].x)/2.0f,
							(1.0f + coords[3*i+2].y)/2.0f
						);
				}
			}
			mesh->SetTexCoordData(texCoords[j], uniquePoints, j);
		}

		for(i=0;i<nrOfPasses;i++)
		{
			mesh->SetReferenceState((*states)[i], i);
		}

		ret->Add(mesh);
		mesh->DetachReference();
	}

	for(i=0;i<nrOfPasses;i++)
	{
		Unregister_Pointer(texCoords[i]);
		delete [] texCoords[i];
	}

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
