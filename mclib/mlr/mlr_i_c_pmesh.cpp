//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	BitTrace *MLR_I_C_PMesh_Clip;
#endif

//#############################################################################
//###### MLR_I_C_PMesh with color but no lighting one texture layer  #####
//#############################################################################

MLR_I_C_PMesh::ClassData*
	MLR_I_C_PMesh::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_C_PMesh::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLR_I_C_PMeshClassID,
			"MidLevelRenderer::MLR_I_C_PMesh",
			MLR_I_PMesh::DefaultData,
			(MLRPrimitiveBase::Factory)&Make
		);
	Register_Object(DefaultData);

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		MLR_I_C_PMesh_Clip = new BitTrace("MLR_I_C_PMesh_Clip");
		Register_Object(MLR_I_C_PMesh_Clip);
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_C_PMesh::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		Unregister_Object(MLR_I_C_PMesh_Clip);
		delete MLR_I_C_PMesh_Clip;
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_C_PMesh::MLR_I_C_PMesh(
	ClassData *class_data,
	MemoryStream *stream,
	int version
):
	MLR_I_PMesh(class_data, stream, version)
{
	Verify(gos_GetCurrentHeap() == Heap);
	Check_Pointer(this);
	Check_Pointer(stream);

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
#if COLOR_AS_DWORD
			MemoryStreamIO_Read(stream, &colors);
#else
			Stuff::DynamicArrayOf<DWORD> smallColors;

			MemoryStreamIO_Read(stream, &smallColors);
		
			int i, len = smallColors.GetLength();

			colors.SetLength(len);

			DWORD theColor;

			for(i=0;i<len;i++)
			{
				theColor = smallColors[i];

				colors[i].blue = (theColor & 0xff) * One_Over_256;

				theColor = theColor>>8;

				colors[i].green = (theColor & 0xff) * One_Over_256;

				theColor = theColor>>8;

				colors[i].red = (theColor & 0xff) * One_Over_256;

				theColor = theColor>>8;

				colors[i].alpha = (theColor & 0xff) * One_Over_256;
			}
#endif
		}
		break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_C_PMesh::MLR_I_C_PMesh(ClassData *class_data):
	MLR_I_PMesh(class_data), colors(0)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
void
	MLR_I_C_PMesh::Copy(MLRIndexedPolyMesh *polyMesh)
{
	Check_Pointer(this);

	int len;
#if COLOR_AS_DWORD
	DWORD *_colors;
#else
	RGBAColor *_colors;
#endif

	MLR_I_PMesh::Copy(polyMesh);

	polyMesh->GetColorData(&_colors, &len);
	SetColorData(_colors, len);
}
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_C_PMesh::~MLR_I_C_PMesh()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_C_PMesh*
	MLR_I_C_PMesh::Make(
		MemoryStream *stream,
		int version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLR_I_C_PMesh *mesh = new MLR_I_C_PMesh(DefaultData, stream, version);
	gos_PopCurrentHeap();

	return mesh;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_C_PMesh::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLR_I_PMesh::Save(stream);

#if COLOR_AS_DWORD
	MemoryStreamIO_Write(stream, &colors);
#else
	Stuff::DynamicArrayOf<DWORD> smallColors;
	int i, len = colors.GetLength();

	const Stuff::RGBAColor *data = colors.GetData();

	smallColors.SetLength(len);

	for(i=0;i<len;i++)
	{
		smallColors[i] = GOSCopyColor(data+i);
	}

	MemoryStreamIO_Write(stream, &smallColors);
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_C_PMesh::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_C_PMesh::SetColorData(
#if COLOR_AS_DWORD
		const DWORD *data,
#else
		const RGBAColor *data,
#endif
		int dataSize
	)
{
	Check_Object(this); 
	Check_Pointer(data);

	Verify(coords.GetLength() == 0 || dataSize == coords.GetLength());
	Verify(texCoords.GetLength() == 0 || dataSize == texCoords.GetLength());

	colors.AssignData(data, dataSize);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_C_PMesh::GetColorData(
#if COLOR_AS_DWORD
		DWORD **data,
#else
		RGBAColor **data,
#endif
		int *dataSize
	)
{
	Check_Object(this); 
	*data = colors.GetData();
	*dataSize = colors.GetLength();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_C_PMesh::PaintMe(
#if COLOR_AS_DWORD
		const DWORD *paintMe
#else
		const RGBAColor *paintMe
#endif
		
	)
{
	Check_Object(this);
	// original color is lost !!!;

	int k, len = colors.GetLength();

#if COLOR_AS_DWORD
	DWORD argb = GOSCopyColor(paintMe);

	for(k=0;k<len;k++)
	{
		colors[k] = argb;
	}
#else
	for(k=0;k<len;k++)
	{
		colors[k] = *paintMe;
	}
#endif
}

#undef I_SAY_YES_TO_DUAL_TEXTURES
#define I_SAY_YES_TO_COLOR
#undef I_SAY_YES_TO_LIGHTING

#define CLASSNAME MLR_I_C_PMesh

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	#define SET_MLR_PMESH_CLIP() MLR_I_C_PMesh_Clip->Set()
	#define CLEAR_MLR_PMESH_CLIP() MLR_I_C_PMesh_Clip->Clear()
#else
	#define SET_MLR_PMESH_CLIP()
	#define CLEAR_MLR_PMESH_CLIP()
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	This include contains follwing functions:
//	void MLR_I_C_PMesh::TransformNoClip(Matrix4D*, GOSVertexPool*);
//	int MLR_I_C_PMesh::Clip(MLRClippingState, GOSVertexPool*);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include<mlr/mlrprimitiveclipping.hpp>

#undef I_SAY_YES_TO_COLOR
#undef CLASSNAME

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_C_PMesh*
	MidLevelRenderer::CreateIndexedCube_Color_NoLit(
		Scalar half, 
		RGBAColor *eightColors, 
		MLRState *state
	)
{
	gos_PushCurrentHeap(Heap);
	MLR_I_C_PMesh *ret = new MLR_I_C_PMesh;
	Register_Object(ret);

	Point3D *coords = new Point3D [8];
	Register_Pointer(coords);

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
	Unregister_Pointer(lengths);
	delete [] lengths;

	ret->SetCoordData(coords, 8);
	Unregister_Pointer(coords);
	delete [] coords;

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
	Unregister_Pointer(index);
	delete [] index;

	ret->FindFacePlanes();

	if(eightColors!=NULL)
	{
#if COLOR_AS_DWORD
		DWORD *dwColor = new DWORD [8];
		Register_Pointer(dwColor);

		for(i=0;i<8;i++)
		{
			dwColor[i] = GOSCopyColor(eightColors+i);
		}

		ret->SetColorData(dwColor, 8);
		Unregister_Pointer(dwColor);
		delete [] dwColor;
#else
		ret->SetColorData(eightColors, 8);
#endif
	}

	Vector2DScalar *texCoords = new Vector2DScalar[8];
	Register_Pointer(texCoords);

	texCoords[0] = Vector2DScalar(0.0f, 0.0f);
	texCoords[1] = Vector2DScalar(0.0f, 0.0f);
	texCoords[2] = Vector2DScalar(0.0f, 0.0f);
	texCoords[3] = Vector2DScalar(0.0f, 0.0f);

	texCoords[4] = Vector2DScalar(0.0f, 0.0f);
	texCoords[5] = Vector2DScalar(0.0f, 0.0f);
	texCoords[6] = Vector2DScalar(0.0f, 0.0f);
	texCoords[7] = Vector2DScalar(0.0f, 0.0f);

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
	ret->SetTexCoordData(texCoords, 8);
	Unregister_Pointer(texCoords);
	delete [] texCoords;

	gos_PopCurrentHeap();
	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_PMesh*
	MidLevelRenderer::CreateIndexedViewFrustrum_Color_NoLit(
		Scalar near_clip,
		Scalar far_clip,
		Scalar left_clip,
		Scalar right_clip,
		Scalar top_clip,
		Scalar bottom_clip,
		RGBAColor& color,
		MLRState *state
	)
{
	gos_PushCurrentHeap(Heap);
	MLR_I_C_PMesh *ret = new MLR_I_C_PMesh();
	Register_Object(ret);

	Point3D *coords = new Point3D [8];
	Register_Pointer(coords);

	Scalar right_far, left_far, top_far, bottom_far;
	Scalar fac = (near_clip + far_clip) / near_clip;

	right_far = right_clip*fac;
	left_far = left_clip*fac;
	top_far = top_clip*fac;
	bottom_far = bottom_clip*fac;

	coords[0] = Point3D(left_far, bottom_far, far_clip);
	coords[1] = Point3D(right_far, bottom_far, far_clip);
	coords[2] = Point3D(left_clip, bottom_clip, near_clip);
	coords[3] = Point3D(right_clip, bottom_clip, near_clip);
	coords[4] = Point3D(right_far, top_far, far_clip);
	coords[5] = Point3D(left_far, top_far, far_clip);
	coords[6] = Point3D(left_clip, top_clip, near_clip);
	coords[7] = Point3D(right_clip, top_clip, near_clip);

	unsigned char *lengths = new unsigned char [6];
	Register_Pointer(lengths);

	int i;

	for(i=0;i<6;i++)
	{
		lengths[i] = 4;
	}

//	ret->SetSubprimitiveLengths(lengths, 6);
	ret->SetSubprimitiveLengths(lengths, 5);

	ret->SetCoordData(coords, 8);

	unsigned short	*index = new unsigned short [6*4];
	Register_Pointer(index);

	index[0] = 0;
	index[1] = 5;
	index[2] = 4;
	index[3] = 1;

	index[4] = 0;
	index[5] = 1;
	index[6] = 3;
	index[7] = 2;

	index[8] = 0;
	index[9] = 2;
	index[10] = 6;
	index[11] = 5;

	index[12] = 1;
	index[13] = 4;
	index[14] = 7;
	index[15] = 3;

	index[16] = 2;
	index[17] = 3;
	index[18] = 7;
	index[19] = 6;

//	index[20] = 5;
//	index[21] = 6;
//	index[22] = 7;
//	index[23] = 4;

//	ret->SetIndexData(index, 6*4);
	ret->SetIndexData(index, 5*4);

	ret->FindFacePlanes();

#if COLOR_AS_DWORD
	DWORD *colors = new DWORD [8];
	Register_Pointer(colors);

	colors[0] = GOSCopyColor(color);
	for(i=1;i<8;i++)
	{
		colors[i] = colors[0];
	}

#else
	RGBAColor *colors = new RGBAColor[8];
	Register_Pointer(colors);

	for(i=0;i<8;i++)
	{
		colors[i] = color;
	}

	for(i=0;i<4;i++)
	{
		colors[i].red *= 0.8f;;
		colors[i].green *= 0.8f;;
		colors[i].blue *= 0.8f;;

		colors[i+4].red *= 1.2f;;
		colors[i+4].green *= 1.2f;;
		colors[i+4].blue *= 1.2f;;
	}


#endif
	ret->SetColorData(colors, 8);

	Vector2DScalar *texCoords = new Vector2DScalar[8];
	Register_Pointer(texCoords);

	texCoords[0] = Vector2DScalar(0.0f, 0.0f);
	texCoords[1] = Vector2DScalar(0.0f, 0.0f);
	texCoords[2] = Vector2DScalar(0.0f, 0.0f);
	texCoords[3] = Vector2DScalar(0.0f, 0.0f);

	texCoords[4] = Vector2DScalar(0.0f, 0.0f);
	texCoords[5] = Vector2DScalar(0.0f, 0.0f);
	texCoords[6] = Vector2DScalar(0.0f, 0.0f);
	texCoords[7] = Vector2DScalar(0.0f, 0.0f);

	ret->SetTexCoordData(texCoords, 8);

	if(state != NULL)
	{
		ret->SetReferenceState(*state);
	}

	Unregister_Pointer(texCoords);
	delete [] texCoords;

	Unregister_Pointer(colors);
	delete [] colors;

	Unregister_Pointer(index);
	delete [] index;

	Unregister_Pointer(lengths);
	delete [] lengths;

	Unregister_Pointer(coords);
	delete [] coords;

	gos_PopCurrentHeap();
	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape*
	MidLevelRenderer::CreateIndexedIcosahedron_Color_NoLit(
		IcoInfo& icoInfo,
		MLRState *state
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
	RGBAColor *colors = new RGBAColor[nrTri*3];
	Register_Pointer(colors);

	int uniquePoints = 0;
	for (k=0;k<20;k++)
	{
		MidLevelRenderer::triDrawn = 0;
		MLR_I_C_PMesh *mesh = new MLR_I_C_PMesh();
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

		if(icoInfo.indexed==true)
		{
			for(i=0;i<uniquePoints;i++)
			{
				colors[i] = 
					RGBAColor(
						(1.0f + collapsedCoords[i].x)/2.0f,
						(1.0f + collapsedCoords[i].y)/2.0f,
						(1.0f + collapsedCoords[i].z)/2.0f,
						1.0f
					);
			}
		}
		else
		{
			for(i=0;i<uniquePoints;i++)
			{
				colors[i] = 
					RGBAColor(
						(1.0f + coords[i].x)/2.0f,
						(1.0f + coords[i].y)/2.0f,
						(1.0f + coords[i].z)/2.0f,
						1.0f
					);
			}
		}
		mesh->SetColorData(colors, uniquePoints);

		ret->Add(mesh);
		mesh->DetachReference();
	}

	Unregister_Pointer(colors);
	delete [] colors;

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
