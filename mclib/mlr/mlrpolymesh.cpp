//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#include"owntrace.hpp"

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	BitTrace *MLRPolyMesh_Clip;
	#define SET_MLRPOLYMESH_CLIP() MLRPolyMesh_Clip->Set()
	#define CLEAR_MLRPOLYMESH_CLIP() MLRPolyMesh_Clip->Clear()
#else
	#define SET_MLRPOLYMESH_CLIP()
	#define CLEAR_MLRPOLYMESH_CLIP()
#endif

OwnTrace *clipTrace;

//#############################################################################
//###############################    MLRPolyMesh    ##################################
//#############################################################################

MLRPolyMesh::ClassData*
	MLRPolyMesh::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::InitializeClass()
{
	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			MLRPolyMeshClassID,
			"MidLevelRenderer::MLRPolyMesh",
			MLRPrimitive::DefaultData,
			(MLRPrimitive::Factory)&Make
		);
	Register_Object(DefaultData);

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		MLRPolyMesh_Clip = new BitTrace("MLRPolyMesh_Clip");
		Register_Object(MLRPolyMesh_Clip);
	#endif

	clipTrace = new OwnTrace("Clip Timing");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		Unregister_Object(MLRPolyMesh_Clip);
		delete MLRPolyMesh_Clip;
	#endif
	
	clipTrace->Print();
	delete clipTrace;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPolyMesh::MLRPolyMesh(
	MemoryStream *stream,
	int version
) :
	MLRPrimitive(DefaultData, stream, version)
{
	Check_Pointer(this);
	Check_Pointer(stream);

	facePlanes.SetLength(GetNumPrimitives());
	testList.SetLength(GetNumPrimitives());

	FindFacePlanes();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPolyMesh::MLRPolyMesh():
	MLRPrimitive(DefaultData)
{
	Check_Pointer(this);

	drawMode = SortData::TriList;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPolyMesh::~MLRPolyMesh()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPolyMesh*
	MLRPolyMesh::Make(
		MemoryStream *stream,
		int version
	)
{
	return new MLRPolyMesh(stream, version);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLRPrimitive::Save(stream);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::SetPrimitiveLength (unsigned char *data, int numPrimitives)
{
	Check_Object(this); 

	lengths.AssignData(data, numPrimitives);

	facePlanes.SetLength(numPrimitives);

	testList.SetLength(numPrimitives);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::GetPrimitiveLength (unsigned char **data, int *l)
{
	Check_Object(this); 
	*l = lengths.GetLength();
	*data = lengths.GetData();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::FindFacePlanes()
{
	Check_Object(this); 

	int i, j, stride, numPrimitives = GetNumPrimitives();
	Vector3D v;

	for(i=0,j=0;i<numPrimitives;i++)
	{
		stride = lengths[i];

		facePlanes[i].BuildPlane(
			coords[j],
			coords[j+1],
			coords[j+2]
		);
		j += stride;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRPolyMesh::FindBackFace(const Point3D& u)
{
	Check_Object(this);

	unsigned char *iPtr;
	int i, numPrimitives = GetNumPrimitives();
	int ret = 0, len = lengths.GetLength();
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
		for(i=0;i<numPrimitives;i++,p++,iPtr++)
		{
#if defined(_ARMOR)
			*iPtr = (p->DistanceTo(u) >= 0.0f) ? (unsigned char)1: (unsigned char)0;

			Verify (*iPtr != (unsigned char)0);
#else
			*iPtr = (unsigned char)1;
#endif
		}
		ret = (unsigned char)1;
	}
	else
	{
		for(i=0;i<numPrimitives;i++,p++,iPtr++)
		{
			*iPtr = (p->DistanceTo(u) >= 0.0f) ? (unsigned char)1: (unsigned char)0;
			
			ret += *iPtr;
		}

		visible = ret ? (unsigned char)1 : (unsigned char)0;
	}

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::ResetTestList()
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
void
	MLRPolyMesh::Transform(Matrix4D *mat)
{
	Check_Object(this);

	int i, j, k;
	int pLen, len = lengths.GetLength();

	for(i=0,j=0;i<len;i++)
	{
		pLen = lengths[i];

		if(testList[i] != 0)
		{
			for(k=j;k<pLen+j;k++)
			{
				transformedCoords[k].Multiply(coords[k], *mat);
			}
			
			#ifdef LAB_ONLY
				Statistics::MLR_TransformedVertices += pLen;
			#endif
		}

		j += pLen;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::TransformNoClip(Matrix4D *mat, GOSVertexPool *vt)
{
	Check_Object(this);

	SET_MLRPOLYMESH_CLIP();

	int i, j, k, stride;
	int len = lengths.GetLength();

	if(len <= 0)
	{
		visible = 0;
		return;
	}

	Check_Object(vt);
	gos_vertices = vt->GetActualVertexPool();
	numGOSVertices = 0;

	//
	//-------------------------------
	// Handle the non-indexed version
	//-------------------------------
	//

	for(i=0,j=0;i<len;i++)
	{
		stride = lengths[i];

		Verify(stride >= 3);

		if(testList[i] == 0)
		{
			j += stride;
			continue;
		}

		for(k=j;k<stride+j;k++)
		{
			transformedCoords[k].Multiply(coords[k], *mat);
		}
		
		#ifdef LAB_ONLY
			Statistics::MLR_TransformedVertices += stride;
		#endif

		//
		//--------------------------------------------------------
		// JM claims all vertices are in. lets check it. who knows
		//--------------------------------------------------------
		//
		#if defined(_ARMOR)
			for(k=j;k<j+stride;k++)
			{
				Verify(transformedCoords[k].x >= 0.0f && transformedCoords[k].x <= transformedCoords[k].w );
				Verify(transformedCoords[k].y >= 0.0f && transformedCoords[k].y <= transformedCoords[k].w );
				Verify(transformedCoords[k].z >= 0.0f && transformedCoords[k].z <= transformedCoords[k].w );
			}
		#endif
		for(k=1;k<stride-1;k++)
		{
			Verify((vt->GetLast() + 3 + numGOSVertices) < vt->GetLength());
			Verify((*actualColors).GetLength() == coords.GetLength());
			Verify(texCoords.GetLength() == coords.GetLength());

			GOSCopyTriangleData(
				&gos_vertices[numGOSVertices],
				transformedCoords.GetData(),
				actualColors->GetData(),
				texCoords.GetData(),
				j, j+k+1, j+k
			);


			numGOSVertices += 3;
		}

#ifdef LAB_ONLY
		Statistics::MLR_NonClippedVertices += stride;
#endif

		j += stride;
	}

	Check_Object(vt);
	vt->Increase(numGOSVertices);
	
	visible = numGOSVertices ? 1 : 0;

	CLEAR_MLRPOLYMESH_CLIP();
}

static MLRClippingState theAnd, theOr, theTest;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Now it gets serious
int
	MLRPolyMesh::Clip(MLRClippingState clippingFlags, GOSVertexPool *vt)
{
	clipTrace->Start();
	SET_MLRPOLYMESH_CLIP();

	Check_Object(this);

	static int clipTrick[6][2] = {
		{ 1, 1},
		{ 1, 0},
		{ 0, 1},
		{ 0, 0},
		{ 2, 0},
		{ 2, 1}
	};

	int i, j, k, l;
	int stride, end, len = lengths.GetLength();

	int mask, k1, ct=0, ret = 0;
	Scalar a=0.0f, bc0, bc1;
	
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

		CLEAR_MLRPOLYMESH_CLIP();
		clipTrace->Stop();

		return visible;
	}

	int	myNumberUsedClipVertex, myNumberUsedClipIndex, myNumberUsedClipLength;

	myNumberUsedClipVertex = 0;
	myNumberUsedClipIndex = 0;
	myNumberUsedClipLength = 0;

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

		Stuff::Vector4D *v4d = transformedCoords.GetData() + j;
		MLRClippingState *cs = clipPerVertex.GetData() + j;
		
		for(k=j;k<end;k++,v4d++,cs++)
		{
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

			theAnd &= *cs;

			theOr |= *cs;

			theAnd = theOr = 0;

#ifdef LAB_ONLY
			if(clipPerVertex[k]==0)
			{
				Statistics::MLR_NonClippedVertices++;
			}
			else
			{
				Statistics::MLR_ClippedVertices++;
			}
#endif
		}

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
		}
		else if (theOr == 0)
		{

			ret++;
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
				for(k=j;k<end;k++)
				{
					k1 = (k + 1 - j)%stride + j;

					//
					//----------------------------------------------------
					// If this vertex is inside the viewing space, copy it
					// directly to the clipping buffer
					//----------------------------------------------------
					//
					int clipped_index =
						myNumberUsedClipVertex + numberVerticesPerPolygon;
					theTest = clipPerVertex[k];
					if(theTest == 0)
					{
						clipExtraCoords[clipped_index] = transformedCoords[k];

						Verify((*actualColors).GetLength() > 0);
						clipExtraColors[clipped_index] = (*actualColors)[k];

						Verify(texCoords.GetLength() > 0);
						clipExtraTexCoords[clipped_index] = texCoords[k];

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
						Verify(clipPerVertex[k1] == clipPerVertex[k]);
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
							bc0 = GetBC(l, transformedCoords[k]);
							bc1 = GetBC(l, transformedCoords[k1]);

							//
							//-------------------------------------------
							// Find the clipping interval from bc0 to bc1
							//-------------------------------------------
							//
							Verify(!Close_Enough(bc0, bc1));
							a = bc0 / (bc0 - bc1);
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
						transformedCoords[k],
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
					if((*actualColors).GetLength() > 0)
					{
#if COLOR_AS_DWORD
						clipExtraColors[clipped_index] = Color_DWORD_Lerp (
							(*actualColors)[k],
							(*actualColors)[k1],
							a
						);
#else
						clipExtraColors[clipped_index].Lerp(
							(*actualColors)[k],
							(*actualColors)[k1],
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
					if(texCoords.GetLength() > 0)
					{
						clipExtraTexCoords[clipped_index].Lerp
							(
								texCoords[k],
								texCoords[k1],
								a
							);
					}

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
				ClipPolygon clipBuffer[2];
				ClipData srcPolygon, dstPolygon;
				int dstBuffer = 0;

				//
				//-----------------------------------------------------
				// Point the source polygon buffer at our original data
				//-----------------------------------------------------
				//
				srcPolygon.coords = &transformedCoords[j];
				srcPolygon.clipPerVertex = &clipPerVertex[j];							
				srcPolygon.flags = 0;

				if((*actualColors).GetLength() > 0)
				{
					srcPolygon.colors = &(*actualColors)[j];
					srcPolygon.flags |= 1;
				}
				else
				{
					srcPolygon.colors = NULL;
				}

				if(texCoords.GetLength() > 0)
				{
					srcPolygon.texCoords = &texCoords[j];
					srcPolygon.flags |= 2;
				}
				else
				{
					srcPolygon.texCoords = NULL;
				}

				srcPolygon.length = lengths[i];

				//
				//--------------------------------
				// Point to the destination buffer
				//--------------------------------
				//
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
							k1 = (k+1)%srcPolygon.length;

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
							bc0 = GetBC(l, srcPolygon.coords[k]);
							bc1 = GetBC(l, srcPolygon.coords[k1]);
							Verify(!Close_Enough(bc0, bc1));
							a = bc0 / (bc0 - bc1);
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

	gos_vertices = vt->GetActualVertexPool();
	numGOSVertices = 0;

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
			Verify((vt->GetLast() + 3 + numGOSVertices) < vt->GetLength());
			Verify((*actualColors).GetLength() == coords.GetLength());
			Verify(texCoords.GetLength() == coords.GetLength());

#if 1
			GOSCopyTriangleData(
				&gos_vertices[numGOSVertices],
				transformedCoords.GetData(),
				actualColors->GetData(),
				texCoords.GetData(),
				j, j+k+1, j+k
			);

#else
			gos_vertices[numGOSVertices] = transformedCoords[j];
			gos_vertices[numGOSVertices+1] = transformedCoords[j+k+1];
			gos_vertices[numGOSVertices+2] = transformedCoords[j+k];

			if((*actualColors).GetLength() == coords.GetLength())
			{
				gos_vertices[numGOSVertices] = (*actualColors)[j];
				gos_vertices[numGOSVertices+1] = (*actualColors)[j+k+1];
				gos_vertices[numGOSVertices+2] = (*actualColors)[j+k];
			}

			if(texCoords.GetLength() == coords.GetLength())
			{
				gos_vertices[numGOSVertices] = texCoords[j];
				gos_vertices[numGOSVertices+1] = texCoords[j+k+1];
				gos_vertices[numGOSVertices+2] = texCoords[j+k];
			}
#endif
			numGOSVertices += 3;
		}

		j += stride;
	}

//		very strange but it seems this line is wrong
//		vt->Increase(numGOSVertices);

	if(myNumberUsedClipLength > 0)
	{
		for(i=0,j=0;i<myNumberUsedClipLength;i++)
		{
			stride = clipExtraLength[i];

			for(k=1;k<stride-1;k++)
			{
				Verify((vt->GetLast() + 3 + numGOSVertices) < vt->GetLength());

#if 1
				GOSCopyTriangleData(
					&gos_vertices[numGOSVertices],
					clipExtraCoords.GetData(),
					clipExtraColors.GetData(),
					clipExtraTexCoords.GetData(),
					j, j+k+1, j+k
				);
#else

				gos_vertices[numGOSVertices] = clipExtraCoords[j];
				gos_vertices[numGOSVertices+1] = clipExtraCoords[j+k+1];
				gos_vertices[numGOSVertices+2] = clipExtraCoords[j+k];

				if((*actualColors).GetLength() == coords.GetLength())
				{
					gos_vertices[numGOSVertices] = clipExtraColors[j];
					gos_vertices[numGOSVertices+1] = clipExtraColors[j+k+1];
					gos_vertices[numGOSVertices+2] = clipExtraColors[j+k];
				}

				if(texCoords.GetLength() == coords.GetLength())
				{
					gos_vertices[numGOSVertices] = clipExtraTexCoords[j];
					gos_vertices[numGOSVertices+1] = clipExtraTexCoords[j+k+1];
					gos_vertices[numGOSVertices+2] = clipExtraTexCoords[j+k];
				}
#endif
				numGOSVertices += 3;
			}
			j += stride;
		}
	}
	vt->Increase(numGOSVertices);

	visible = numGOSVertices ? 1 : 0;

	if(visible)
	{
	}
	else
	{
	}

	CLEAR_MLRPOLYMESH_CLIP();
	clipTrace->Stop();

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRPolyMesh::Lighting (
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

			int i, j, k, l, stride, len = colors.GetLength();

			MLRVertexData vertexData;

#if COLOR_AS_DWORD
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

			len = lengths.GetLength();
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

				for(k=0,j=0;k<len;k++)
				{
					stride = lengths[k];

					if(testList[k] == 0)
					{
						j += stride;
						vertexData.point += stride;
						vertexData.color += stride;
						vertexData.normal += stride;
						continue;
					}

					for(l=j;l<j+stride;j++)
					{
						light->LightVertex(vertexData);

						vertexData.point++;
						vertexData.color++;
						vertexData.normal++;
					}
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
			STOP(("Lightmaps not implemented yet."));
		}
		break;
		case MLRState::LightingClassicAndLightMapMode:
		{
			Verify(state.GetAlphaMode() == MLRState::OneZeroMode);

			Verify(colors.GetLength() == litColors.GetLength());
			Verify(normals.GetLength() == colors.GetLength());
			Verify(coords.GetLength() == colors.GetLength());

			int i, j, k, l, stride, len = colors.GetLength();

			MLRVertexData vertexData;

#if COLOR_AS_DWORD
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

			len = lengths.GetLength();
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

				for(k=0,j=0;k<len;k++)
				{
					stride = lengths[k];

					if(testList[k] == 0)
					{
						j += stride;
						vertexData.point += stride;
						vertexData.color += stride;
						vertexData.normal += stride;
						continue;
					}

					for(l=j;l<j+stride;j++)
					{
						light->LightVertex(vertexData);

						vertexData.point++;
						vertexData.color++;
						vertexData.normal++;
					}
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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPolyMesh*
	MidLevelRenderer::CreateCube(
		Scalar half, 
		RGBAColor *eightColors, 
		Vector3D *eightNormals,
		MLRState *state
	)
{
	MLRPolyMesh *ret = new MLRPolyMesh;
	Register_Object(ret);

	Vector3D eightCoords[8] = {
		Vector3D( half, -half,  half),
		Vector3D(-half, -half,  half),
		Vector3D( half, -half, -half),
		Vector3D(-half, -half, -half),
		Vector3D(-half,  half,  half),
		Vector3D( half,  half,  half),
		Vector3D( half,  half, -half),
		Vector3D(-half,  half, -half)
	};

	unsigned char *lengths = new unsigned char [6];

	int i;

	for(i=0;i<6;i++)
	{
		lengths[i] = 4;
	}

	ret->SetPrimitiveLength(lengths, 6);

	Point3D *coords = new Point3D [6*4];

	coords[0] = eightCoords[0];
	coords[1] = eightCoords[2];
	coords[2] = eightCoords[6];
	coords[3] = eightCoords[5];

	coords[4] = eightCoords[0];
	coords[5] = eightCoords[5];
	coords[6] = eightCoords[4];
	coords[7] = eightCoords[1];

	coords[8] = eightCoords[5];
	coords[9] = eightCoords[6];
	coords[10] = eightCoords[7];
	coords[11] = eightCoords[4];

	coords[12] = eightCoords[2];
	coords[13] = eightCoords[3];
	coords[14] = eightCoords[7];
	coords[15] = eightCoords[6];

	coords[16] = eightCoords[1];
	coords[17] = eightCoords[4];
	coords[18] = eightCoords[7];
	coords[19] = eightCoords[3];

	coords[20] = eightCoords[0];
	coords[21] = eightCoords[1];
	coords[22] = eightCoords[3];
	coords[23] = eightCoords[2];


	ret->SetCoordData(coords, 6*4);
	ret->FindFacePlanes();

	if(eightColors!=NULL)
	{
#if COLOR_AS_DWORD
		DWORD *colors = new DWORD[6*4];

		colors[0] = GOSCopyColor(&eightColors[0]);
		colors[1] = GOSCopyColor(&eightColors[2]);
		colors[2] = GOSCopyColor(&eightColors[6]);
		colors[3] = GOSCopyColor(&eightColors[5]);

		colors[4] = GOSCopyColor(&eightColors[0]);
		colors[5] = GOSCopyColor(&eightColors[5]);
		colors[6] = GOSCopyColor(&eightColors[4]);
		colors[7] = GOSCopyColor(&eightColors[1]);

		colors[8] = GOSCopyColor(&eightColors[5]);
		colors[9] = GOSCopyColor(&eightColors[6]);
		colors[10] = GOSCopyColor(&eightColors[7]);
		colors[11] = GOSCopyColor(&eightColors[4]);

		colors[12] = GOSCopyColor(&eightColors[2]);
		colors[13] = GOSCopyColor(&eightColors[3]);
		colors[14] = GOSCopyColor(&eightColors[7]);
		colors[15] = GOSCopyColor(&eightColors[6]);

		colors[16] = GOSCopyColor(&eightColors[1]);
		colors[17] = GOSCopyColor(&eightColors[4]);
		colors[18] = GOSCopyColor(&eightColors[7]);
		colors[19] = GOSCopyColor(&eightColors[3]);

		colors[20] = GOSCopyColor(&eightColors[0]);
		colors[21] = GOSCopyColor(&eightColors[1]);
		colors[22] = GOSCopyColor(&eightColors[3]);
		colors[23] = GOSCopyColor(&eightColors[2]);
#else
		RGBAColor *colors = new RGBAColor[6*4];

		colors[0] = eightColors[0];
		colors[1] = eightColors[2];
		colors[2] = eightColors[6];
		colors[3] = eightColors[5];

		colors[4] = eightColors[0];
		colors[5] = eightColors[5];
		colors[6] = eightColors[4];
		colors[7] = eightColors[1];

		colors[8] = eightColors[5];
		colors[9] = eightColors[6];
		colors[10] = eightColors[7];
		colors[11] = eightColors[4];

		colors[12] = eightColors[2];
		colors[13] = eightColors[3];
		colors[14] = eightColors[7];
		colors[15] = eightColors[6];

		colors[16] = eightColors[1];
		colors[17] = eightColors[4];
		colors[18] = eightColors[7];
		colors[19] = eightColors[3];

		colors[20] = eightColors[0];
		colors[21] = eightColors[1];
		colors[22] = eightColors[3];
		colors[23] = eightColors[2];
#endif

		ret->SetColorData(colors, 6*4);
	}

	if(eightNormals!=NULL)
	{
		Vector3D *normals = new Vector3D[6*4];

		normals[0] = eightNormals[0];
		normals[1] = eightNormals[2];
		normals[2] = eightNormals[6];
		normals[3] = eightNormals[5];

		normals[4] = eightNormals[0];
		normals[5] = eightNormals[5];
		normals[6] = eightNormals[4];
		normals[7] = eightNormals[1];

		normals[8] = eightNormals[5];
		normals[9] = eightNormals[6];
		normals[10] = eightNormals[7];
		normals[11] = eightNormals[4];

		normals[12] = eightNormals[2];
		normals[13] = eightNormals[3];
		normals[14] = eightNormals[7];
		normals[15] = eightNormals[6];

		normals[16] = eightNormals[1];
		normals[17] = eightNormals[4];
		normals[18] = eightNormals[7];
		normals[19] = eightNormals[3];

		normals[20] = eightNormals[0];
		normals[21] = eightNormals[1];
		normals[22] = eightNormals[3];
		normals[23] = eightNormals[2];

		ret->SetNormalData(normals, 6*4);
	}

	if(state->GetTextureHandle() > 0)
	{
		Vector2DScalar *texCoords = new Vector2DScalar[6*4];

		texCoords[0] = Vector2DScalar(0.0f, 0.0f);
		texCoords[1] = Vector2DScalar(0.0f, 1.0f);
		texCoords[2] = Vector2DScalar(1.0f, 1.0f);
		texCoords[3] = Vector2DScalar(1.0f, 0.0f);

		texCoords[4] = Vector2DScalar(0.0f, 0.0f);
		texCoords[5] = Vector2DScalar(0.0f, 1.0f);
		texCoords[6] = Vector2DScalar(1.0f, 1.0f);
		texCoords[7] = Vector2DScalar(1.0f, 0.0f);

		texCoords[8] = Vector2DScalar(0.0f, 0.0f);
		texCoords[9] = Vector2DScalar(0.0f, 1.0f);
		texCoords[10] = Vector2DScalar(1.0f, 1.0f);
		texCoords[11] = Vector2DScalar(1.0f, 0.0f);

		texCoords[12] = Vector2DScalar(0.0f, 0.0f);
		texCoords[13] = Vector2DScalar(0.0f, 1.0f);
		texCoords[14] = Vector2DScalar(1.0f, 1.0f);
		texCoords[15] = Vector2DScalar(1.0f, 0.0f);

		texCoords[16] = Vector2DScalar(0.0f, 0.0f);
		texCoords[17] = Vector2DScalar(0.0f, 1.0f);
		texCoords[18] = Vector2DScalar(1.0f, 1.0f);
		texCoords[19] = Vector2DScalar(1.0f, 0.0f);

		texCoords[20] = Vector2DScalar(0.0f, 0.0f);
		texCoords[21] = Vector2DScalar(0.0f, 1.0f);
		texCoords[22] = Vector2DScalar(1.0f, 1.0f);
		texCoords[23] = Vector2DScalar(1.0f, 0.0f);

		ret->SetTexCoordData(texCoords, 6*4);
	}

	if(state != NULL)
	{
		ret->SetReferenceState(*state);
	}

	return ret;
}

