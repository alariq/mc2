//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#include"owntrace.hpp"

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	BitTrace *MLRIndexedPolyMesh_Clip;
	#define SET_MLRIndexedPolyMesh_CLIP() MLRIndexedPolyMesh_Clip->Set()
	#define CLEAR_MLRIndexedPolyMesh_CLIP() MLRIndexedPolyMesh_Clip->Clear()
#else
	#define SET_MLRIndexedPolyMesh_CLIP()
	#define CLEAR_MLRIndexedPolyMesh_CLIP()
#endif

//#############################################################################
//###############################    MLRIndexedPolyMesh    ##################################
//#############################################################################

MLRIndexedPolyMesh::ClassData*
	MLRIndexedPolyMesh::DefaultData = NULL;
MemoryBlock*
	MLRIndexedPolyMesh::AllocatedMemory = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::InitializeClass()
{
	Verify(!AllocatedMemory);
	AllocatedMemory =
		new MemoryBlock(
			sizeof(MLRIndexedPolyMesh),
			100,
			100,
			"MLRIndexedPolyMesh"
		);
	Register_Object(AllocatedMemory);

	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			MLRIndexedPolyMeshClassID,
			"MidLevelRenderer::MLRIndexedPolyMesh",
			MLRIndexedPrimitive::DefaultData,
			(MLRPrimitive::Factory)&Make
		);
	Register_Object(DefaultData);

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		MLRIndexedPolyMesh_Clip = new BitTrace("MLRIndexedPolyMesh_Clip");
		Register_Object(MLRIndexedPolyMesh_Clip);
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;

	Unregister_Object(AllocatedMemory);
	delete AllocatedMemory;
	AllocatedMemory = NULL;

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		Unregister_Object(MLRIndexedPolyMesh_Clip);
		delete MLRIndexedPolyMesh_Clip;
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPolyMesh::MLRIndexedPolyMesh(
	MemoryStream *stream,
	int version
):
	MLRIndexedPrimitive(DefaultData, stream, version)
{
	Check_Pointer(this);
	Check_Pointer(stream);

	facePlanes.SetLength(GetNumPrimitives());
	testList.SetLength(GetNumPrimitives());

	FindFacePlanes();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPolyMesh::MLRIndexedPolyMesh():
	MLRIndexedPrimitive(DefaultData)
{
	Check_Pointer(this);

	drawMode = SortData::TriIndexedList;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPolyMesh::~MLRIndexedPolyMesh()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPolyMesh*
	MLRIndexedPolyMesh::Make(
		MemoryStream *stream,
		int version
	)
{
	return new MLRIndexedPolyMesh(stream, version);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLRIndexedPrimitive::Save(stream);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::InitializeDrawPrimitive(int vis, int parameter)
{
	MLRIndexedPrimitive::InitializeDrawPrimitive(vis, parameter);

	if(parameter & 1)
	{
		ResetTestList();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::SetPrimitiveLength (unsigned char *data, int numPrimitives)
{
	Check_Object(this); 

	lengths.AssignData(data, numPrimitives);

	facePlanes.SetLength(numPrimitives);

	testList.SetLength(numPrimitives);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::GetPrimitiveLength (unsigned char **data, int *l)
{
	Check_Object(this);
	*l = lengths.GetLength();
	*data = lengths.GetData();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::FindFacePlanes()
{
	Check_Object(this); 

	int i, j, stride, numPrimitives = GetNumPrimitives();
	Vector3D v;

	Verify(index.GetLength() > 0);

	for(i=0,j=0;i<numPrimitives;i++)
	{
		stride = lengths[i];

		facePlanes[i].BuildPlane(
			coords[index[j]],
			coords[index[j+1]],
			coords[index[j+2]]
		);

		j += stride;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRIndexedPolyMesh::FindBackFace(const Point3D& u)
{
	Check_Object(this);

	int i, numPrimitives = GetNumPrimitives();
	int ret = 0, len = lengths.GetLength();
	unsigned char *iPtr;
	Plane *p;

	if(len <= 0)
	{
		visible = 0;

		return 0;
	}

	p = &facePlanes[0];
	iPtr = &testList[0];

	if(state.GetBackFaceMode() == MLRState::BackFaceOffMode)
	{
		ResetTestList();
		ret = 1;
	}
	else
	{
		for(i=0;i<numPrimitives;i++,p++,iPtr++)
		{
//			Scalar s = p->DistanceTo(u);

//			*iPtr = !Get_Sign_Bit(s);

			*iPtr = (p->DistanceTo(u) >= 0.0f) ? (unsigned char)1: (unsigned char)0;
			
			ret += *iPtr;
		}

		visible = ret ? (unsigned char)1 : (unsigned char)0;
	}

	visible = ret ? (unsigned char)1 : (unsigned char)0;

	FindVisibleVertices();

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::ResetTestList()
{
	int i, numPrimitives = GetNumPrimitives();
	unsigned char *iPtr = &testList[0];

	for(i=0;i<numPrimitives;i++,iPtr++)
	{
		*iPtr = 1;
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRIndexedPolyMesh::FindVisibleVertices()
{
	Check_Object(this);
	Verify(index.GetLength() > 0);

	int ret, i, j, k, stride, len = lengths.GetLength();

	Verify(index.GetLength() > 0);
	
	for(i=0,j=0,ret=0;i<len;i++)
	{
		stride = lengths[i];

		Verify(stride >= 3);

		if(testList[i] == 0)
		{
			j += stride;
			continue;
		}

		for(k=j;k<j+stride;k++)
		{
			visibleIndexedVertices[index[k]] = 1;
			ret++;
		}

		j += stride;
	}
	visibleIndexedVerticesKey = true;

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::Transform(Matrix4D *mat)
{
	Check_Object(this);

	Verify(index.GetLength() > 0);
	int i, len = coords.GetLength();

	if(visibleIndexedVerticesKey == false)
	{
		FindVisibleVertices();
	}

	for(i=0;i<len;i++)
	{
		if(visibleIndexedVertices[i] != 0)
		{
			transformedCoords[i].Multiply(coords[i], *mat);
#ifdef LAB_ONLY
			Statistics::MLR_TransformedVertices++;
#endif
		}
	}
	
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::TransformNoClip(Matrix4D *mat, GOSVertexPool *vt)
{
	Check_Object(this);
	Check_Object(vt);
	Verify(index.GetLength() > 0);

	SET_MLRIndexedPolyMesh_CLIP();

	unsigned short stride;
	int i, j, k, len = lengths.GetLength();
	gos_vertices = vt->GetActualVertexPool();
	numGOSVertices = 0;

	Verify(index.GetLength() > 0);
	
	k = visibleIndexedVertices.GetLength();

	if(visibleIndexedVerticesKey == false)
	{
		FindVisibleVertices();
	}

	for(j=0,stride=0;j<k;j++)
	{
		if(visibleIndexedVertices[j] == 0)
		{
			stride++;
		}
		else
		{
			visibleIndexedVertices[j] = stride;

			transformedCoords[j].Multiply(coords[j], *mat);

#ifdef LAB_ONLY
			Statistics::MLR_TransformedVertices++;
#endif

//
//--------------------------------------------------------
// JM claims all vertices are in. lets check it. who knows
//--------------------------------------------------------
//
			Verify(transformedCoords[j].x >= 0.0f && transformedCoords[j].x <= transformedCoords[j].w );
			Verify(transformedCoords[j].y >= 0.0f && transformedCoords[j].y <= transformedCoords[j].w );
			Verify(transformedCoords[j].z >= 0.0f && transformedCoords[j].z <= transformedCoords[j].w );
			GOSCopyData(
				&gos_vertices[numGOSVertices],
				transformedCoords.GetData(),
				actualColors->GetData(),
				texCoords.GetData(),
				j
			);
			numGOSVertices++;
		}
	}

	#ifdef LAB_ONLY
		Statistics::MLR_NonClippedVertices += numGOSVertices;
	#endif

	Check_Object(vt);
	vt->Increase(numGOSVertices);

	gos_indices = vt->GetActualIndexPool();
	numGOSIndices = 0;

	for(i=0,j=0;i<len;i++)
	{
		stride = lengths[i];

		Verify(stride >= 3);

		if(testList[i] == 0)
		{
			j += stride;
			continue;
		}

		for(k=1;k<stride-1;k++)
		{
			Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());

			gos_indices[numGOSIndices] = (unsigned short)(index[j] - visibleIndexedVertices[index[j]]);
			gos_indices[numGOSIndices+1] = (unsigned short)(index[j+k+1] - visibleIndexedVertices[index[j+k+1]]);
			gos_indices[numGOSIndices+2] = (unsigned short)(index[j+k] - visibleIndexedVertices[index[j+k]]);

			numGOSIndices += 3;
		}
		j += stride;
	}
	Check_Object(vt);
	vt->IncreaseIndex(numGOSIndices);

	visible = numGOSVertices ? 1 : 0;
	CLEAR_MLRIndexedPolyMesh_CLIP();
}

static MLRClippingState theAnd, theOr, theTest;
static int clipTrick[6][2] = {
	{ 1, 1},
	{ 1, 0},
	{ 0, 1},
	{ 0, 0},
	{ 2, 0},
	{ 2, 1}
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Now it gets serious
int
	MLRIndexedPolyMesh::Clip(MLRClippingState clippingFlags, GOSVertexPool *vt)
{
	StartCycleTiming( &Statistics::MLR_ClipTime );
	SET_MLRIndexedPolyMesh_CLIP();

	Check_Object(this);

	unsigned short stride, l;
	int i, j, k, ret = 0;
	int len = lengths.GetLength();

	Verify(len == testList.GetLength());
	Verify(clippingFlags.GetClippingState() != 0);

	//
	//--------------------------------------
	// See if we don't have to draw anything
	//--------------------------------------
	//
	if(len <= 0)
	{
		visible = 0;

		CLEAR_MLRIndexedPolyMesh_CLIP();
		EndCycleTiming( &Statistics::MLR_ClipTime );

		return visible;
	}

	int mask, end, k0, k1, ct=0;
	Scalar a=0.0f;
//	Scalar bc0=0.0f, bc1=0.0f;

	int	myNumberUsedClipVertex, myNumberUsedClipIndex, myNumberUsedClipLength;

	myNumberUsedClipVertex = 0;
	myNumberUsedClipIndex = 0;
	myNumberUsedClipLength = 0;

	Verify(index.GetLength() > 0);
	
	if(visibleIndexedVerticesKey == false)
	{
		FindVisibleVertices();
	}

	//
	//------------------------
	// Handle the indexed case
	//------------------------
	//

	//
	//-----------------------------------------------------------------
	// Step through each vertex and check visibility
	// backfaced polygons
	//-----------------------------------------------------------------
	//
	k = transformedCoords.GetLength();
	Stuff::Vector4D *v4d = transformedCoords.GetData();
	MLRClippingState *cs = clipPerVertex.GetData();

 	for(i=0;i<k;i++,v4d++,cs++)
	{
		if(visibleIndexedVertices[i] == 0)
		{
			continue;
		}

		cs->SetClippingState(0);

		if( clippingFlags.IsFarClipped() && v4d->w < v4d->z)
		{
			cs->SetFarClip();
		}

		if( clippingFlags.IsNearClipped() && v4d->z < 0.0f)
		{
			cs->SetNearClip();
		}

		if( clippingFlags.IsRightClipped() && v4d->x < 0.0f)
		{
			cs->SetRightClip();
		}

		if( clippingFlags.IsLeftClipped() && v4d->w < v4d->x)
		{
			cs->SetLeftClip();
		}

		if( clippingFlags.IsBottomClipped() && v4d->y < 0.0f)
		{
			cs->SetBottomClip();
		}

		if(clippingFlags.IsTopClipped() && v4d->w < v4d->y)
		{
			cs->SetTopClip();
		}

#ifdef LAB_ONLY
		if( (*cs)==0)
		{
			Statistics::MLR_NonClippedVertices++;
		}
		else
		{
			Statistics::MLR_ClippedVertices++;
		}
#endif
	}

	//	initialize visibleIndexedVertices
	memset(visibleIndexedVertices.GetData(), 0, visibleIndexedVertices.GetSize());

	//
	//-----------------------------------------------------------------
	// Step through each polygon, making sure that we don't try to clip
	// backfaced polygons
	//-----------------------------------------------------------------
	//
	for(i=0,j=0;i<len;i++)
	{
		stride = lengths[i];

		if(testList[i] == 0)
		{
			j += stride;
			continue;
		}

		//
		//---------------------------------------------------------------
		// Test each vertex of the polygon against the allowed clipping
		// planes, and accumulate status for which planes always clip and
		// which planes clipped at least once
		//---------------------------------------------------------------
		//
		theAnd.SetClippingState(0x3f);
		theOr.SetClippingState(0);
		end = j+stride;
		
		for(k=j;k<end;k++)
		{
			theAnd &= clipPerVertex[index[k]];

			theOr |= clipPerVertex[index[k]];
		}

		theAnd = theOr = 0;		//ASSUME NO CLIPPING NEEDED FOR MC2.  Its just not done here!

		//
		//-------------------------------------------------------------------
		// If any bit is set for all vertices, then the polygon is completely
		// outside the viewing space and we don't have to draw it.  On the
		// other hand, if no bits at all were ever set, we can do a trivial
		// accept of the polygon
		//-------------------------------------------------------------------
		//
		if (theAnd != 0)
		{
			testList[i] = 0;
#ifdef LAB_ONLY
			Statistics::MLR_PolysClippedButOutside++;
#endif
		}
		else if (theOr == 0)
		{
			ret++;
			for(k=j;k<end;k++)
			{
				visibleIndexedVertices[index[k]] = 1;
			}
#ifdef LAB_ONLY
			Statistics::MLR_PolysClippedButInside++;
#endif
		}

		//
		//-----------------------------------------------------------------
		// It is not a trivial case, so we must now do real clipping on the
		// polygon
		//-----------------------------------------------------------------
		//
		else
		{
			unsigned short numberVerticesPerPolygon = 0;

			//
			//---------------------------------------------------------------
			// Handle the case of a single clipping plane by stepping through
			// the vertices and finding the edge it originates
			//---------------------------------------------------------------
			//
			if (theOr.GetNumberOfSetBits() == 1)
			{
#ifdef LAB_ONLY
				Statistics::MLR_PolysClippedButOnePlane++;
#endif
				for(k=j;k<end;k++)
				{
					k0 = index[k];
					k1 = index[(k+1) < end ? k+1 : j];

					//
					//----------------------------------------------------
					// If this vertex is inside the viewing space, copy it
					// directly to the clipping buffer
					//----------------------------------------------------
					//
					int clipped_index =
						myNumberUsedClipVertex + numberVerticesPerPolygon;
					theTest = clipPerVertex[k0];
					if(theTest == 0)
					{
						clipExtraCoords[clipped_index] = transformedCoords[k0];

						Verify((*actualColors).GetLength() > 0);
						clipExtraColors[clipped_index] = (*actualColors)[k0];

						Verify(texCoords.GetLength() > 0);
						clipExtraTexCoords[clipped_index] = texCoords[k0];

						numberVerticesPerPolygon++;
						clipped_index++;

						//
						//-------------------------------------------------------
						// We don't need to clip this edge if the next vertex is
						// also in the viewing space, so just move on to the next
						// vertex
						//-------------------------------------------------------
						//
						if(clipPerVertex[k1] == 0)
						{
							continue;
						}
					}

					//
					//---------------------------------------------------------
					// This vertex is outside the viewing space, so if the next
					// vertex is also outside the viewing space, no clipping is
					// needed and we throw this vertex away.  Since only one
					// clipping plane is involved, it must be in the same space
					// as the first vertex
					//---------------------------------------------------------
					//
					else if(clipPerVertex[k1] != 0)
					{
						Verify(clipPerVertex[k1] == clipPerVertex[k0]);
						continue;
					}

					//
					//--------------------------------------------------
					// We now find the distance along the edge where the
					// clipping plane will intersect
					//--------------------------------------------------
					//
					mask = 1;
					theTest |= clipPerVertex[k1];

					//
					//-----------------------------------------------------
					// Find the boundary conditions that match our clipping
					// plane
					//-----------------------------------------------------
					//
					for (l=0; l<MLRClippingState::NextBit; l++)
					{
						if(theTest.IsClipped(mask))
						{
//							GetDoubleBC(l, bc0, bc1, transformedCoords[k0], transformedCoords[k1]);

							//
							//-------------------------------------------
							// Find the clipping interval from bc0 to bc1
							//-------------------------------------------
							//
//							Verify(!Close_Enough(bc0, bc1));
//							a = bc0 / (bc0 - bc1);
							a = GetLerpFactor(l, transformedCoords[k0], transformedCoords[k1]);
							Verify(a >= 0.0f && a <= 1.0f);

							ct = l;

							break;
						}
						mask <<= 1;
					}

					//
					//------------------------------
					// Lerp the homogeneous position
					//------------------------------
					//
					clipExtraCoords[clipped_index].Lerp(
						transformedCoords[k0],
						transformedCoords[k1],
						a
					);

					clipExtraCoords[clipped_index][clipTrick[ct][0]] = clipTrick[ct][1] ? clipExtraCoords[clipped_index].w : 0.0f;

					//
					//----------------------------------------------------------
					// If there are colors, lerp them in screen space for now as
					// most cards do that anyway
					//----------------------------------------------------------
					//
					Verify((*actualColors).GetLength() > 0);
#if COLOR_AS_DWORD
					clipExtraColors[clipped_index] = Color_DWORD_Lerp (
						(*actualColors)[k0],
						(*actualColors)[k1],
						a
					);
#else
					clipExtraColors[clipped_index].Lerp(
						(*actualColors)[k0],
						(*actualColors)[k1],
						a
					);
#endif

					//
					//-----------------------------------------------------
					// If there are texture uv's, we need to lerp them in a
					// perspective correct manner
					//-----------------------------------------------------
					//
					Verify(texCoords.GetLength() > 0);
					clipExtraTexCoords[clipped_index].Lerp
						(
							texCoords[k0],
							texCoords[k1],
							a
						);

					//
					//--------------------------------
					// Bump the polygon's vertex count
					//--------------------------------
					//
					numberVerticesPerPolygon++;
				}
			}

			//
			//---------------------------------------------------------------
			// We have to handle multiple planes.  We do this by creating two
			// buffers and we switch between them as we clip plane by plane
			//---------------------------------------------------------------
			//
			else
			{
#ifdef LAB_ONLY
				Statistics::MLR_PolysClippedButGOnePlane++;
#endif
				ClipPolygon clipBuffer[2];
				ClipData srcPolygon, dstPolygon;
				int dstBuffer = 1;

				Verify((*actualColors).GetLength() > 0);
				Verify(texCoords.GetLength() > 0);

				srcPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
				srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
				srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
				srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();

				//
				//----------------------------------------------------------
				// unravel and copy the original data into the source buffer
				//----------------------------------------------------------
				//
				for(k=j,l=0;k<end;k++,l++)
				{
					int indexK = index[k];

					srcPolygon.coords[l] = transformedCoords[indexK];
					srcPolygon.colors[l] = (*actualColors)[indexK];
					srcPolygon.texCoords[l] = texCoords[indexK];
					srcPolygon.clipPerVertex[l] = clipPerVertex[indexK];
				}

				srcPolygon.length = l;
				srcPolygon.flags |= 3;


				//
				//--------------------------------
				// Point to the destination buffer
				//--------------------------------
				//
				dstBuffer = 0;

				dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
				dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
				dstPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
				dstPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();
				dstPolygon.flags = srcPolygon.flags;
				dstPolygon.length = 0;

				//
				//-----------------------------------------------------------
				// Spin through each plane that clipped the primitive and use
				// it to actually clip the primitive
				//-----------------------------------------------------------
				//
				mask = 1;

				for(l=0; l<MLRClippingState::NextBit; l++)
				{
					if(theOr.IsClipped(mask))
					{

						//
						//-----------------------------------
						// Clip each vertex against the plane
						//-----------------------------------
						//
						for(k=0;k<srcPolygon.length;k++)
						{
							k1 = (k+1) < srcPolygon.length ? k+1 : 0;

							theTest = srcPolygon.clipPerVertex[k];

							//
							//----------------------------------------------------
							// If this vertex is inside the viewing space, copy it
							// directly to the clipping buffer
							//----------------------------------------------------
							//
							if(theTest.IsClipped(mask) == 0)
							{
								dstPolygon.coords[dstPolygon.length] = 
									srcPolygon.coords[k];

								dstPolygon.clipPerVertex[dstPolygon.length] = 
									srcPolygon.clipPerVertex[k];

								if(srcPolygon.flags & 0x1)
								{
									dstPolygon.colors[dstPolygon.length] = 
										srcPolygon.colors[k];
								}

								if(srcPolygon.flags & 0x2)
								{
									dstPolygon.texCoords[dstPolygon.length] = 
										srcPolygon.texCoords[k];
								}

								dstPolygon.length++;

								//
								//-------------------------------------------------------
								// We don't need to clip this edge if the next vertex is
								// also in the viewing space, so just move on to the next
								// vertex
								//-------------------------------------------------------
								//
								if(srcPolygon.clipPerVertex[k1].IsClipped(mask) == 0)
								{
									continue;
								}
							}

							//
							//---------------------------------------------------------
							// This vertex is outside the viewing space, so if the next
							// vertex is also outside the viewing space, no clipping is
							// needed and we throw this vertex away.  Since only one
							// clipping plane is involved, it must be in the same space
							// as the first vertex
							//---------------------------------------------------------
							//
							else if(srcPolygon.clipPerVertex[k1].IsClipped(mask) != 0)
							{
								Verify(
									srcPolygon.clipPerVertex[k1].IsClipped(mask)
									== srcPolygon.clipPerVertex[k].IsClipped(mask)
								);
								continue;
							}

							//
							//-------------------------------------------
							// Find the clipping interval from bc0 to bc1
							//-------------------------------------------
							//

//							GetDoubleBC(l, bc0, bc1, srcPolygon.coords[k], srcPolygon.coords[k1]);

//							Verify(!Close_Enough(bc0, bc1));
//							a = bc0 / (bc0 - bc1);
							a = GetLerpFactor (l, srcPolygon.coords[k], srcPolygon.coords[k1]);
							Verify(a >= 0.0f && a <= 1.0f);

							//
							//------------------------------
							// Lerp the homogeneous position
							//------------------------------
							//
							dstPolygon.coords[dstPolygon.length].Lerp(
								srcPolygon.coords[k],
								srcPolygon.coords[k1],
								a
							);

							dstPolygon.coords[dstPolygon.length][clipTrick[l][0]] = clipTrick[l][1] ? dstPolygon.coords[dstPolygon.length].w : 0.0f;


							//
							//----------------------------------------------------------
							// If there are colors, lerp them in screen space for now as
							// most cards do that anyway
							//----------------------------------------------------------
							//
							if(srcPolygon.flags & 1)
							{
#if COLOR_AS_DWORD
								dstPolygon.colors[dstPolygon.length] = Color_DWORD_Lerp(
									srcPolygon.colors[k],
									srcPolygon.colors[k1],
									a
								);
#else
								dstPolygon.colors[dstPolygon.length].Lerp(
									srcPolygon.colors[k],
									srcPolygon.colors[k1],
									a
								);
#endif
							}

							//
							//-----------------------------------------------------
							// If there are texture uv's, we need to lerp them in a
							// perspective correct manner
							//-----------------------------------------------------
							//
							if(srcPolygon.flags & 2)
							{
								dstPolygon.texCoords[dstPolygon.length].Lerp
									(
										srcPolygon.texCoords[k],
										srcPolygon.texCoords[k1],
										a
									);
							}

							//
							//-------------------------------------
							// We have to generate a new clip state
							//-------------------------------------
							//
							dstPolygon.clipPerVertex[dstPolygon.length].SetClippingState(0);
							switch (l)
							{
							case 0:
								if(clippingFlags.IsTopClipped())
								{
									if(dstPolygon.coords[dstPolygon.length].w < dstPolygon.coords[dstPolygon.length].y)
									{
										dstPolygon.clipPerVertex[dstPolygon.length].SetTopClip();
									}
								}
							case 1:
								if(clippingFlags.IsBottomClipped())
								{
									if(dstPolygon.coords[dstPolygon.length].y < 0.0f)
									{
										dstPolygon.clipPerVertex[dstPolygon.length].SetBottomClip();
									}
								}
							case 2:
								if(clippingFlags.IsLeftClipped())
								{
									if(dstPolygon.coords[dstPolygon.length].w < dstPolygon.coords[dstPolygon.length].x)
									{
										dstPolygon.clipPerVertex[dstPolygon.length].SetLeftClip();
									}
								}
							case 3:
								if(clippingFlags.IsRightClipped())
								{
									if(dstPolygon.coords[dstPolygon.length].x < 0.0f)
									{
										dstPolygon.clipPerVertex[dstPolygon.length].SetRightClip();
									}
								}
							case 4:
								if(clippingFlags.IsNearClipped())
								{
									if(dstPolygon.coords[dstPolygon.length].z < 0.0f)
									{
										dstPolygon.clipPerVertex[dstPolygon.length].SetNearClip();
									}
								}
							case 5:
								if(clippingFlags.IsFarClipped())
								{
									if(dstPolygon.coords[dstPolygon.length].w < dstPolygon.coords[dstPolygon.length].z)
									{
										dstPolygon.clipPerVertex[dstPolygon.length].SetFarClip();
									}
								}
							}

							//
							//----------------------------------
							// Bump the new polygon vertex count
							//----------------------------------
							//
							dstPolygon.length++;
						}

						//
						//-----------------------------------------------
						// Swap source and destination buffer pointers in
						// preparation for the next plane test
						//-----------------------------------------------
						//
						srcPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
						srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
						srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
						srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();	
						srcPolygon.length = dstPolygon.length;

						dstBuffer = !dstBuffer;

						dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
						dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
						dstPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
						dstPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();
						dstPolygon.length = 0;

					}

					mask = mask << 1;
				}

				//
				//--------------------------------------------------
				// Move the most recent polygon into the clip buffer
				//--------------------------------------------------
				//
				for(k=0;k<srcPolygon.length;k++)
				{
					int clipped_index = myNumberUsedClipVertex + k;
					clipExtraCoords[clipped_index] = srcPolygon.coords[k];

					if(srcPolygon.flags & 0x1)
					{
						clipExtraColors[clipped_index] = srcPolygon.colors[k];
					}

					if(srcPolygon.flags & 0x2)
					{
						clipExtraTexCoords[clipped_index] = srcPolygon.texCoords[k];
					}
				}

				numberVerticesPerPolygon = srcPolygon.length;
			}

			clipExtraLength[myNumberUsedClipLength] = numberVerticesPerPolygon;
			myNumberUsedClipVertex += numberVerticesPerPolygon;
			myNumberUsedClipLength++;
			ret++;

//					clip

//					dont draw the original
			testList[i] = 0;
		}

		j += stride;
	}

	Check_Object(vt);
	gos_vertices = vt->GetActualVertexPool();
	numGOSVertices = 0;
	gos_indices = vt->GetActualIndexPool();
	numGOSIndices = 0;

	k = visibleIndexedVertices.GetLength();

	for(j=0,stride=0;j<k;j++)
	{
		if(visibleIndexedVertices[j] == 0)
		{
			stride++;
		}
		else
		{
			visibleIndexedVertices[j] = stride;

		//
		//--------------------------------------------------------
		// JM claims all vertices are in. lets check it. who knows
		//--------------------------------------------------------
		//
			Verify(transformedCoords[j].x >= 0.0f && transformedCoords[j].x <= transformedCoords[j].w );
			Verify(transformedCoords[j].y >= 0.0f && transformedCoords[j].y <= transformedCoords[j].w );
			Verify(transformedCoords[j].z >= 0.0f && transformedCoords[j].z <= transformedCoords[j].w );
			

			GOSCopyData(
				&gos_vertices[numGOSVertices],
				transformedCoords.GetData(),
				actualColors->GetData(),
				texCoords.GetData(),
				j
			);
			numGOSVertices++;
		}
	}

	for(i=0,j=0;i<len;i++)
	{
		stride = lengths[i];

		Verify(stride >= 3);

		if(testList[i] == 0)
		{
			j += stride;
			continue;
		}

		for(k=1;k<stride-1;k++)
		{
			Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());

			gos_indices[numGOSIndices] = (unsigned short)(index[j] - visibleIndexedVertices[index[j]]);
			gos_indices[numGOSIndices+1] = (unsigned short)(index[j+k+1] - visibleIndexedVertices[index[j+k+1]]);
			gos_indices[numGOSIndices+2] = (unsigned short)(index[j+k] - visibleIndexedVertices[index[j+k]]);

			numGOSIndices += 3;
		}
		j += stride;
	}

	if(myNumberUsedClipLength > 0)
	{
		for(i=0,j=0;i<myNumberUsedClipLength;i++)
		{
			stride = clipExtraLength[i];

			for(k=1;k<stride-1;k++)
			{
				Verify((vt->GetLast() + 3 + numGOSVertices) < vt->GetLength());
				Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());

				GOSCopyTriangleData(
					&gos_vertices[numGOSVertices],
					clipExtraCoords.GetData(),
					clipExtraColors.GetData(),
					clipExtraTexCoords.GetData(),
					j, j+k+1, j+k
				);

				Verify(numGOSIndices%3 == 0);
				gos_indices[numGOSIndices] = numGOSVertices;
				gos_indices[numGOSIndices+1] = (unsigned short)(numGOSVertices + 1);
				gos_indices[numGOSIndices+2] = (unsigned short)(numGOSVertices + 2);

				numGOSVertices += 3;
				numGOSIndices += 3;
			}

			j += stride;
		}
	}

	vt->IncreaseIndex(numGOSIndices);
	vt->Increase(numGOSVertices);

	visible = numGOSVertices ? 1 : 0;

	if(visible)
	{
	}
	else
	{
	}

	CLEAR_MLRIndexedPolyMesh_CLIP();
	EndCycleTiming( &Statistics::MLR_ClipTime );

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPolyMesh::Lighting (
		MLRLight **lights,
		int nrLights
	)
{
	Check_Object(this);

	// set the to use colors to the original colors ...
	// only lighting could overwrite this;
	actualColors = &colors;

	if(nrLights == 0)
	{
		return;
	}

	if(normals.GetLength() == 0)
	{
		return;
	}

	if(lights == NULL)
	{
		return;
	}

	switch (GetCurrentState().GetLightingMode())
	{
		case MLRState::LightingOffMode:
			return;
		case MLRState::LightingClassicOnlyMode:
		{
			Verify(colors.GetLength() == litColors.GetLength());
			Verify(normals.GetLength() == colors.GetLength());
			Verify(coords.GetLength() == colors.GetLength());

			int i, k, len = colors.GetLength();

			MLRVertexData vertexData;

#if COLOR_AS_DWORD
			TO_DO;
#else
			RGBAColor *color = &colors[0];
			RGBAColor *litColor = &litColors[0];

#if USE_ASSEMBLER_CODE
			_asm {
				push		esi
				push		edi

				mov			esi, color

				mov			edi, litColor
				mov         ecx, len

			_loop1:

				mov			eax, dword ptr [esi]
				mov			ebx, dword ptr [esi+4]

				mov			dword ptr [edi], eax
				mov			dword ptr [edi+ 4], ebx

				mov			eax, dword ptr [esi + 8]
				mov			ebx, dword ptr [esi + 12]

				mov			dword ptr [edi + 8], eax
				mov			dword ptr [edi + 12], ebx

				add			esi,16
				add			edi,16

				dec			ecx
				jnz			_loop1

				pop			edi
				pop			esi
			}
#else	// it doesnt know that ...
			memcpy(litColor, color, (len<<2)*sizeof(Scalar));
#endif
#endif
			//
			//-----------------------------------
			// Test each light against the vertex
			//-----------------------------------
			//
			for (i=0;i<nrLights;i++)
			{
				MLRLight *light = lights[i];

				Check_Object(light);


				vertexData.point = &coords[0];
				vertexData.color = &litColors[0];
				vertexData.normal = &normals[0];

				for(k=0;k<len;k++)
				{
					if(visibleIndexedVertices[k] != 0)
					{
						light->LightVertex(vertexData);
					}

					vertexData.point++;
					vertexData.color++;
					vertexData.normal++;
				}
			}

			#ifdef LAB_ONLY
				Statistics::MLR_LitVertices += len*nrLights;
			#endif

			// set the to use colors to the original colors ...
			// only lighting could overwrite this;
			actualColors = &litColors;
		}
		break;
		case MLRState::LightingLightMapOnlyMode:
		{
			Verify(state.GetAlphaMode() == MLRState::OneZeroMode);

			int i;

			for (i=0;i<nrLights;i++)
			{
				LightMapLighting(lights[i]);
			}
		}
		break;
		case MLRState::LightingClassicAndLightMapMode:
		{
			Verify(state.GetAlphaMode() == MLRState::OneZeroMode);

			Verify(colors.GetLength() == litColors.GetLength());
			Verify(normals.GetLength() == colors.GetLength());
			Verify(coords.GetLength() == colors.GetLength());

			int i, k, len = colors.GetLength();

			MLRVertexData vertexData;

#if COLOR_AS_DWORD
			TO_DO;
#else
			RGBAColor *color = &colors[0];
			RGBAColor *litColor = &litColors[0];

#if USE_ASSEMBLER_CODE
			_asm {
				push		esi
				push		edi

				mov			esi, color

				mov			edi, litColor
				mov         ecx, len

			_loop2:

				mov			eax, dword ptr [esi]
				mov			ebx, dword ptr [esi+4]

				mov			dword ptr [edi], eax
				mov			dword ptr [edi+ 4], ebx

				mov			eax, dword ptr [esi + 8]
				mov			ebx, dword ptr [esi + 12]

				mov			dword ptr [edi + 8], eax
				mov			dword ptr [edi + 12], ebx

				add			esi,16
				add			edi,16

				dec			ecx
				jnz			_loop2

				pop			edi
				pop			esi
			}
#else	// it doesnt know that ...
			memcpy(litColor, color, (len<<2)*sizeof(Scalar));
#endif
#endif
			//
			//-----------------------------------
			// Test each light against the vertex
			//-----------------------------------
			//
			for (i=0;i<nrLights;i++)
			{
				MLRLight *light = lights[i];

				Check_Object(light);


				vertexData.point = &coords[0];
				vertexData.color = &litColors[0];
				vertexData.normal = &normals[0];

				for(k=0;k<len;k++)
				{
					light->LightVertex(vertexData);

					vertexData.point++;
					vertexData.color++;
					vertexData.normal++;
				}
			}

			#ifdef LAB_ONLY
				Statistics::MLR_LitVertices += len*nrLights;
			#endif

			// set the to use colors to the original colors ...
			// only lighting could overwrite this;
			actualColors = &litColors;

			STOP(("Lightmaps not implemented yet."));
		}
		break;
	}
}

//---------------------------------------------------------------------------
//
MLRPrimitive *
	MLRIndexedPolyMesh::LightMapLighting(MLRLight *light)
{
	int i, j, k, stride, len = lengths.GetLength();
	LinearMatrix4D matrix = LinearMatrix4D::Identity;
	Point3D lightPos, hitPoint;

	UnitVector3D up, right;
	Scalar f, u, v;

	light->GetInShapePosition(lightPos);

	for(i=0,j=0,k=0;i<len;i++)
	{
		stride = lengths[i];

		if(testList[i] == 0)
		{
			j += stride;
			continue;
		}

		f = facePlanes[i].DistanceTo(lightPos);
		if(f>0.0f)
		{
			hitPoint.Multiply(facePlanes[i].normal, -f);
			hitPoint+=lightPos;

			matrix.AlignAxis(facePlanes[i].normal, Z_Axis, X_Axis, Y_Axis);
			matrix.GetWorldRightInLocal(&right);
			matrix.GetWorldUpInLocal(&up);

			for(k=j;k<stride+j;k++)
			{
				Vector3D vec(coords[index[k]]);
				vec-=hitPoint;

				u = (right*vec)/f;
				v = (up*vec)/f;
			}
			testList[i] = 0;
		}

		j += stride;
	}

	return NULL;
}

//---------------------------------------------------------------------------
//
bool
	MLRIndexedPolyMesh::CastRay(
		Line3D *line,
		Normal3D *normal
	)
{
	Check_Object(this);
	Check_Object(line);
	Check_Pointer(normal);

	//
	//---------------------------------------------------------------------
	// We have to spin through each of the polygons stored in the shape and
	// collide the ray against each
	//---------------------------------------------------------------------
	//
	int poly_start = 0, numPrimitives = GetNumPrimitives();
	bool hit = false;
	for (int polygon=0; polygon<numPrimitives; ++polygon)
	{
		int stride = lengths[polygon];
		Verify(stride>2);

		//
		//---------------------------------
		// See if the line misses the plane
		//---------------------------------
		//
		Scalar product;
		const Plane *plane = &facePlanes[polygon];
		Check_Object(plane);
		Scalar distance = line->DistanceTo(*plane, &product);
		if (distance < 0.0f || distance > line->length)
		{
			poly_start += stride;
			continue;
		}
		bool negate = false;
		if (product > -SMALL)
		{
			if (GetCurrentState().GetBackFaceMode() == MLRState::BackFaceOnMode)
			{
				poly_start += stride;
				continue;
			}
			negate = true;
		}

		//
		//-------------------------------------------
		// Figure out where on the plane the line hit
		//-------------------------------------------
		//
		Point3D impact;
		line->Project(distance, &impact);

		//
		//-------------------------------------------------------------------
		// We now need to find out which cardinal plane we should project the
		// triangle onto
		//-------------------------------------------------------------------
		//
		int s,t;
		Scalar nx = Abs(plane->normal.x);
		Scalar ny = Abs(plane->normal.y);
		Scalar nz = Abs(plane->normal.z);
		if (nx > ny)
		{
			if (nx > nz)
			{
				s = Y_Axis;
				t = Z_Axis;
			}
			else
			{
				s = X_Axis;
				t = Y_Axis;
			}
		}
		else if (ny > nz)
		{
			s = Z_Axis;
			t = X_Axis;
		}
		else
		{
			s = X_Axis;
			t = Y_Axis;
		}

		//
		//----------------------------------------
		// Initialize the vertex and leg variables
		//----------------------------------------
		//
		Point3D *v1, *v2, *v3;
		v1 = &coords[index[poly_start]];
		v2 = &coords[index[poly_start+1]];
		v3 = &coords[index[poly_start+2]];

		//
		//---------------------------------------
		// Get the projection of the impact point
		//---------------------------------------
		//
		Scalar s0 = impact[s] - (*v1)[s];
		Scalar t0 = impact[t] - (*v1)[t];
		Scalar s1 = (*v2)[s] - (*v1)[s];
		Scalar t1 = (*v2)[t] - (*v1)[t];

		//
		//------------------------------------------------------------
		// For each triangle, figure out what the second leg should be
		//------------------------------------------------------------
		//
		bool local_hit = false;
		int next_v = 3;
Test_Triangle:
		Check_Pointer(v3);
		Scalar s2 = (*v3)[s] - (*v1)[s];
		Scalar t2 = (*v3)[t] - (*v1)[t];

		//
		//--------------------------------
		// Now, see if we hit the triangle
		//--------------------------------
		//
		if (Small_Enough(s1))
		{
			Verify(!Small_Enough(s2));
			Scalar beta = s0 / s2;
			if (beta >= 0.0f && beta < 1.0f)
			{
				Verify(!Small_Enough(t1));
				Scalar alpha = (t0 - beta*t2) / t1;
				local_hit = (alpha >= 0.0f && alpha+beta <= 1.0f);
			}
		}
		else
		{
			Scalar beta = (t0*s1 - s0*t1);
			Scalar alpha = (t2*s1 - s2*t1);
			beta /= alpha;
			if (beta >= 0.0f && beta <= 1.0f)
			{
				alpha = (s0 - beta*s2) / s1;
				local_hit = (alpha >= 0.0f && alpha+beta <= 1.0f);
			}
		}

		//
		//-----------------------------
		// Set up for the next triangle
		//-----------------------------
		//
		if (next_v < stride && !local_hit)
		{
			v2 = v3;
			v3 = &coords[index[poly_start+next_v++]];
			s1 = s2;
			t1 = t2;
			goto Test_Triangle;
		}

		//
		//----------------------------------------------------
		// Handle the hit status, and move to the next polygon
		//----------------------------------------------------
		//
		if (local_hit)
		{
			hit = true;
			line->length = distance;
			if (negate)
				normal->Negate(plane->normal);
			else
				*normal = plane->normal;
			Verify(*normal * line->direction <= -SMALL);
		}
		poly_start += stride;
	}

	//
	//----------------------
	// Return the hit status
	//----------------------
	//
	return hit;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPolyMesh*
	MidLevelRenderer::CreateIndexedCube(
		Scalar half, 
		RGBAColor *eightColors, 
		Vector3D *eightNormals,
		MLRState *state
	)
{
	MLRIndexedPolyMesh *ret = new MLRIndexedPolyMesh;
	Register_Object(ret);

	Point3D *coords = new Point3D [8];

	coords[0] = Point3D( half, -half,  half);
	coords[1] = Point3D(-half, -half,  half);
	coords[2] = Point3D( half, -half, -half);
	coords[3] = Point3D(-half, -half, -half);
	coords[4] = Point3D(-half,  half,  half);
	coords[5] = Point3D( half,  half,  half);
	coords[6] = Point3D( half,  half, -half);
	coords[7] = Point3D(-half,  half, -half);

	unsigned char *lengths = new unsigned char [6];

	int i;

	for(i=0;i<6;i++)
	{
		lengths[i] = 4;
	}

	ret->SetPrimitiveLength(lengths, 6);

	ret->SetCoordData(coords, 8);

	unsigned short	*index = new unsigned short [6*4];

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

	if(eightColors!=NULL)
	{
#if COLOR_AS_DWORD
		DWORD *dwColor = new DWORD [8];

		for(i=0;i<8;i++)
		{
			dwColor[i] = GOSCopyColor(eightColors+i);
		}

		ret->SetColorData(dwColor, 8);
#else
		ret->SetColorData(eightColors, 8);
#endif
	}

	if(eightNormals!=NULL)
	{
		ret->SetNormalData(eightNormals, 8);
	}

	Vector2DScalar *texCoords = new Vector2DScalar[8];

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

	return ret;
}
