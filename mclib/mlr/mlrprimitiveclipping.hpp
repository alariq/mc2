//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#if !defined(MLR_MLRCLIPTRICK_HPP)
#include<mlr/mlrcliptrick.hpp>
#endif

extern DWORD gEnableTextureSort, gShowClippedPolys;
extern unsigned short *indexOffset;	// [MidLevelRenderer::Max_Number_Vertices_Per_Mesh]

#define HUNT_CLIP_ERROR 0

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	CLASSNAME::TransformNoClip(Matrix4D *mat, GOSVertexPool *vt,bool db)
{
	Check_Object(this);
	Check_Object(vt);

	Start_Timer(Transform_Time);

	Verify(index.GetLength() > 0);

	unsigned short stride;
	bool textureAnimation = false;
	Scalar deltaU=0.0f, deltaV=0.0f;
	
	if(state.GetTextureHandle())
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[state.GetTextureHandle()];
		textureAnimation = texture->GetAnimateTexture();
		if(textureAnimation)
		{
			Stuff::AffineMatrix4D &textureMatrix = texture->GetTextureMatrix();
			deltaU = textureMatrix(3, 0);
			deltaV = textureMatrix(3, 1);
		}
	}

	int i, j, k, len = lengths.GetLength();

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	int tex2count = 0;
#endif
#ifdef I_SAY_YES_TO_MULTI_TEXTURES
	int m;
//	gos_PushCurrentHeap(StaticHeap);
//	static DynamicArrayOf<int> *tex2count = new DynamicArrayOf<int>(Limits::Max_Number_Of_Multitextures);
	int tex2count[Limits::Max_Number_Of_Multitextures];
//	SPEW(("micgaert", "Michael!!! The new() on line 42 can cause memory leaks!"));
//	gos_PopCurrentHeap();
	for(m=0;m<currentNrOfPasses;m++)
	{
		tex2count[m] = 0;
	}
#endif

	int numVertices = GetNumVertices();
	gos_vertices = vt->GetActualVertexPool(db);
	numGOSVertices = 0;

	Verify(index.GetLength() > 0);
	Verify(coords.GetLength() == visibleIndexedVertices.GetLength());

	if(visibleIndexedVerticesKey == false)
	{
		FindVisibleVertices();
	}

	for(j=0,stride=0;j<numVertices;j++)
	{
		if(visibleIndexedVertices[j] == 0)
		{
			stride++;
		}
		else
		{
#ifdef LAB_ONLY
			TransformedVertices++;
#endif

			if(db==false)
			{
				Verify (vt->GetLast() + numGOSVertices < Limits::Max_Number_Vertices_Per_Frame);
			}
			else
			{
				Verify (numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
			}

			indexOffset[j] = (unsigned short)(j - stride);

			gos_vertices[numGOSVertices].GOSTransformNoClip(
				coords[j],
				*mat,
				&texCoords[j][0]
#if FOG_HACK
				, (state.GetFogMode() != 0)
#endif
			);

//			gos_vertices[numGOSVertices].u = texCoords[j][0];
//			gos_vertices[numGOSVertices].v = texCoords[j][1];

#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_LIGHTING
		#if COLOR_AS_DWORD
			gos_vertices[numGOSVertices].argb = (*actualColors)[j];
		#else
			gos_vertices[numGOSVertices].argb = GOSCopyColor(&(*actualColors)[j]);
		#endif
	#else
		#if COLOR_AS_DWORD
			gos_vertices[numGOSVertices].argb = colors[j];
		#else
			gos_vertices[numGOSVertices].argb = GOSCopyColor(&colors[j]);
		#endif
	#endif
#else
			gos_vertices[numGOSVertices].argb = 0xffffffff;
#endif

			if(textureAnimation)
			{
				gos_vertices[numGOSVertices].u += deltaU;
				gos_vertices[numGOSVertices].v += deltaV;
			}

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
			(*texCoords2)[tex2count++] = texCoords[numVertices + j];
#endif
#ifdef I_SAY_YES_TO_MULTI_TEXTURES
			for(m=1;m<currentNrOfPasses;m++)
			{
				(*extraMultiTexCoords)[m][tex2count[m]++] = multiTexCoordsPointers[m][j];
			}
#endif
			numGOSVertices++;
		}
	}

#ifdef LAB_ONLY
	Set_Statistic(NonClippedVertices, NonClippedVertices+numGOSVertices);
#endif

	Check_Object(vt);

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	Verify (tex2count == numGOSVertices);
	if(db==false)
	{
		Verify (vt->GetLast() + 2*numGOSVertices < vt->GetLength());
	}
	else
	{
		Verify (2*numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
	}

	if(state2.GetTextureHandle())
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[state.GetTextureHandle()];
		textureAnimation = texture->GetAnimateTexture();
		if(textureAnimation)
		{
			Stuff::AffineMatrix4D &textureMatrix = texture->GetTextureMatrix();
			deltaU = textureMatrix(3, 0);
			deltaV = textureMatrix(3, 1);
		}
	}

	memcpy(gos_vertices + numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

	if(textureAnimation)
	{
		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = (*texCoords2)[i][0] + deltaU;
			gos_vertices[j].v = (*texCoords2)[i][1] + deltaV;
		}
	}
	else
	{
		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = (*texCoords2)[i][0];
			gos_vertices[j].v = (*texCoords2)[i][1];
		}
	}

	if(db==false)
	{
		vt->Increase(2*numGOSVertices);
	}
#else
	#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
		if(db==false)
		{
			Verify (vt->GetLast() + 2*numGOSVertices < vt->GetLength());
		}
		else
		{
			Verify (2*numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
		}

		memcpy(gos_vertices + numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = gos_vertices[i].u*xScale + xOffset;
			gos_vertices[j].v = gos_vertices[i].v*yScale + yOffset;
		}

		if(db==false)
		{
			vt->Increase(2*numGOSVertices);
		}
	#else
		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	#endif
#endif

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
	for(m=1;m<currentNrOfPasses;m++)
	{
		Verify (tex2count[m] == numGOSVertices);
		if(db==false)
		{
			Verify (vt->GetLast() + numGOSVertices < vt->GetLength());
		}
		else
		{
			Verify (numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
		}

		memcpy(gos_vertices + numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = (*extraMultiTexCoords)[m][i][0];
			gos_vertices[j].v = (*extraMultiTexCoords)[m][i][1];
		}

		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	}
#endif

	gos_indices = vt->GetActualIndexPool(db);
	numGOSIndices = 0;

	int ngi = 0;
	for(i=0,j=0;i<len;j+=stride,i++)
	{
		stride = lengths[i];

		Verify(stride >= 3);

		if(testList[i] == 0)
		{
			continue;
		}

		for(k=1;k<stride-1;k++)
		{
			if(db==false)
			{
				Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());
			}
			else
			{
				Verify(3 + numGOSIndices < 2*Limits::Max_Number_Vertices_Per_Mesh);
			}

			gos_indices[ngi++] = indexOffset[index[j]];
			gos_indices[ngi++] = indexOffset[index[j+1+k]];
			gos_indices[ngi++] = indexOffset[index[j+k]];
		}
	}
	numGOSIndices = (unsigned short)ngi;

	Check_Object(vt);
	if(db==false)
	{
		vt->IncreaseIndex(numGOSIndices);
	}

	Stop_Timer(Transform_Time);

	visible = numGOSVertices ? (unsigned char)1 : (unsigned char)0;
}

static MLRClippingState theAnd, theOr, theTest;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Now it gets serious
int
	CLASSNAME::TransformAndClip(Matrix4D *mat, MLRClippingState clippingFlags, GOSVertexPool *vt, bool db)
{
	Transform(mat);

	Start_Timer(Clipping_Time);

	Check_Object(this);

	unsigned short l, stride;
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

		Stop_Timer(Clipping_Time);

		return visible;
	}

	int mask, end, k0, k1, ct=0;
	Scalar a=0.0f;
//	Scalar bc0=0.0f, bc1=0.0f;

	bool textureAnimation = false;
	Scalar deltaU=0.0f, deltaV=0.0f;
	
	if(state.GetTextureHandle())
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[state.GetTextureHandle()];
		textureAnimation = texture->GetAnimateTexture();
		if(textureAnimation)
		{
			Stuff::AffineMatrix4D &textureMatrix = texture->GetTextureMatrix();
			deltaU = textureMatrix(3, 0);
			deltaV = textureMatrix(3, 1);
		}
	}

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	bool textureAnimation2 = false;
	Scalar deltaU2=0.0f, deltaV2=0.0f;
	
	if(state2.GetTextureHandle())
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[state.GetTextureHandle()];
		textureAnimation2 = texture->GetAnimateTexture();
		if(textureAnimation2)
		{
			Stuff::AffineMatrix4D &textureMatrix = texture->GetTextureMatrix();
			deltaU2 = textureMatrix(3, 0);
			deltaV2 = textureMatrix(3, 1);
		}
	}
#endif

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	int tex2count = 0;
	int numVertices = GetNumVertices();
#endif

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
	int m;
	gos_PushCurrentHeap(StaticHeap);
	static DynamicArrayOf<int> *tex2count = new DynamicArrayOf<int>(Limits::Max_Number_Of_Multitextures);
	SPEW(("micgaert", "Michael!!! The new() on line 316 can cause memory leaks!"));
	gos_PopCurrentHeap();
	for(m=0;m<currentNrOfPasses;m++)
	{
		tex2count[m] = 0;
	}

#endif

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
			theAnd &= (*clipPerVertex)[index[k]];

			theOr |= (*clipPerVertex)[index[k]];
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
#ifdef LAB_ONLY
			Set_Statistic(PolysClippedButOutside, PolysClippedButOutside+1);
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
			Set_Statistic(PolysClippedButInside, PolysClippedButInside+1);
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
			bool firstIsIn;
			if (theOr.GetNumberOfSetBits() == 1)
			{
#ifdef LAB_ONLY
				Set_Statistic(PolysClippedButOnePlane, PolysClippedButOnePlane+1);
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
					theTest = (*clipPerVertex)[k0];

					if(theTest == 0)
					{
						firstIsIn = true;
						(*clipExtraCoords)[clipped_index] = (*transformedCoords)[k0];

#ifdef I_SAY_YES_TO_COLOR
#ifdef I_SAY_YES_TO_LIGHTING
						Verify((*actualColors).GetLength() > 0);
						(*clipExtraColors)[clipped_index] = (*actualColors)[k0];
#else
						Verify(colors.GetLength() > 0);
						(*clipExtraColors)[clipped_index] = colors[k0];
#endif
#endif

						Verify(texCoords.GetLength() > 0);
						(*clipExtraTexCoords)[clipped_index] = texCoords[k0];

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
						(*clipExtraTexCoords2)[clipped_index] = texCoords[k0+numVertices];
#endif						

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
						for(m=1;m<currentNrOfPasses;m++)
						{
							(*clipExtraMultiTexCoords)[m][clipped_index] = multiTexCoordsPointers[m][k0];
						}
#endif						
						numberVerticesPerPolygon++;
						clipped_index++;

						//
						//-------------------------------------------------------
						// We don't need to clip this edge if the next vertex is
						// also in the viewing space, so just move on to the next
						// vertex
						//-------------------------------------------------------
						//
						if((*clipPerVertex)[k1] == 0)
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
					else
					{
						firstIsIn = false;
						if((*clipPerVertex)[k1] != 0)
						{
							Verify((*clipPerVertex)[k1] == (*clipPerVertex)[k0]);
							continue;
						}
					}

					//
					//--------------------------------------------------
					// We now find the distance along the edge where the
					// clipping plane will intersect
					//--------------------------------------------------
					//
					mask = 1;
					theTest |= (*clipPerVertex)[k1];

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
							if(firstIsIn==true)
							{
								a = GetLerpFactor(l, (*transformedCoords)[k0], (*transformedCoords)[k1]);
							}
							else
							{
								a = GetLerpFactor(l, (*transformedCoords)[k1], (*transformedCoords)[k0]);
							}

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
					if(firstIsIn==true)
					{
						(*clipExtraCoords)[clipped_index].Lerp(
							(*transformedCoords)[k0],
							(*transformedCoords)[k1],
							a
						);

						DoClipTrick((*clipExtraCoords)[clipped_index], ct);

						//
						//----------------------------------------------------------
						// If there are colors, lerp them in screen space for now as
						// most cards do that anyway
						//----------------------------------------------------------
						//
#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_LIGHTING
						Verify((*actualColors).GetLength() > 0);
		#if COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index] = Color_DWORD_Lerp (
							(*actualColors)[k0],
							(*actualColors)[k1],
							a
						);
		#else
						(*clipExtraColors)[clipped_index].Lerp(
							(*actualColors)[k0],
							(*actualColors)[k1],
							a
						);
		#endif
	#else
						Verify(colors.GetLength() > 0);
		#if COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index] = Color_DWORD_Lerp (
							colors[k0],
							colors[k1],
							a
						);
		#else
						(*clipExtraColors)[clipped_index].Lerp(
							colors[k0],
							colors[k1],
							a
						);
		#endif
	#endif
#endif
						//
						//-----------------------------------------------------
						// If there are texture uv's, we need to lerp them in a
						// perspective correct manner
						//-----------------------------------------------------
						//
						Verify(texCoords.GetLength() > 0);
						(*clipExtraTexCoords)[clipped_index].Lerp
							(
								texCoords[k0],
								texCoords[k1],
								a
							);

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
						(*clipExtraTexCoords2)[clipped_index].Lerp
							(
								texCoords[k0+numVertices],
								texCoords[k1+numVertices],
								a
							);
#endif

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
						for(m=1;m<currentNrOfPasses;m++)
						{
							(*clipExtraMultiTexCoords)[m][clipped_index] .Lerp
								(
									multiTexCoordsPointers[m][k0],
									multiTexCoordsPointers[m][k1],
									a
								);
						}

#endif
					}
					else
					{
						(*clipExtraCoords)[clipped_index].Lerp(
							(*transformedCoords)[k1],
							(*transformedCoords)[k0],
							a
						);

						DoClipTrick((*clipExtraCoords)[clipped_index], ct);

						//
						//----------------------------------------------------------
						// If there are colors, lerp them in screen space for now as
						// most cards do that anyway
						//----------------------------------------------------------
						//
#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_LIGHTING
						Verify((*actualColors).GetLength() > 0);
		#if COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index] = Color_DWORD_Lerp (
							(*actualColors)[k1],
							(*actualColors)[k0],
							a
						);
		#else
						(*clipExtraColors)[clipped_index].Lerp(
							(*actualColors)[k1],
							(*actualColors)[k0],
							a
						);
		#endif
	#else
						Verify(colors.GetLength() > 0);
		#if COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index] = Color_DWORD_Lerp (
							colors[k1],
							colors[k0],
							a
						);
		#else
						(*clipExtraColors)[clipped_index].Lerp(
							colors[k1],
							colors[k0],
							a
						);
		#endif
	#endif
#endif
						//
						//-----------------------------------------------------
						// If there are texture uv's, we need to lerp them in a
						// perspective correct manner
						//-----------------------------------------------------
						//
						Verify(texCoords.GetLength() > 0);
						(*clipExtraTexCoords)[clipped_index].Lerp
							(
								texCoords[k1],
								texCoords[k0],
								a
							);

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
						(*clipExtraTexCoords2)[clipped_index].Lerp
							(
								texCoords[k1+numVertices],
								texCoords[k0+numVertices],
								a
							);
#endif

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
						for(m=1;m<currentNrOfPasses;m++)
						{
							(*clipExtraMultiTexCoords)[m][clipped_index] .Lerp
								(
									multiTexCoordsPointers[m][k1],
									multiTexCoordsPointers[m][k0],
									a
								);
						}

#endif
					}

					//
					//--------------------------------
					// Bump the polygon's vertex count
					//--------------------------------
					//
					numberVerticesPerPolygon++;
				}
				(*clipExtraLength)[myNumberUsedClipLength] = numberVerticesPerPolygon;
#ifdef _ARMOR
				(*clipExtraLength)[myNumberUsedClipLength] &= ~0x8000;
#endif
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
				Set_Statistic(PolysClippedButGOnePlane, PolysClippedButGOnePlane+1);
#endif
				ClipData2 srcPolygon, dstPolygon;
				int dstBuffer = 1;

				Verify(texCoords.GetLength() > 0);

				srcPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_LIGHTING
				Verify((*actualColors).GetLength() > 0);
	#else
				Verify(colors.GetLength() > 0);
	#endif
				srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
#endif
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

					srcPolygon.coords[l] = (*transformedCoords)[indexK];

#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_LIGHTING
					srcPolygon.colors[l] = (*actualColors)[indexK];
	#else
					srcPolygon.colors[l] = colors[indexK];
	#endif
#endif

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
					srcPolygon.texCoords[2*l] = texCoords[indexK];
					srcPolygon.texCoords[2*l+1] = texCoords[indexK + numVertices];
#else
#ifdef I_SAY_YES_TO_MULTI_TEXTURES
					for(m=0;m<currentNrOfPasses;m++)
					{
						srcPolygon.texCoords[currentNrOfPasses*l+m] = multiTexCoordsPointers[m][indexK];
					}
#else
					srcPolygon.texCoords[l] = texCoords[indexK];
#endif
#endif
					srcPolygon.clipPerVertex[l] = (*clipPerVertex)[indexK];
				}

				srcPolygon.length = l;

				//
				//--------------------------------
				// Point to the destination buffer
				//--------------------------------
				//
				dstBuffer = 0;

				dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
#ifdef I_SAY_YES_TO_COLOR
				dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
#endif
				dstPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
				dstPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();
				dstPolygon.length = 0;

				//
				//-----------------------------------------------------------
				// Spin through each plane that clipped the primitive and use
				// it to actually clip the primitive
				//-----------------------------------------------------------
				//
				mask = 1;
				MLRClippingState theNewOr(0);
				int loop = 4;

#if HUNT_CLIP_ERROR
				for(k=0;k<srcPolygon.length;k++)
				{
					DEBUG_STREAM << setiosflags( ios::scientific) << setprecision(20) 
						<< srcPolygon.coords[k].x << " "
						<< srcPolygon.coords[k].y << " "
						<< srcPolygon.coords[k].z << " "
						<< srcPolygon.coords[k].w << '\n';
				}
#endif
#if HUNT_CLIP_ERROR
					DEBUG_STREAM << "TheOriginalOR: " << hex << theOr.GetClippingState() << dec << '\n';
#endif

				do
				{
					for(l=0; l<MLRClippingState::NextBit; l++)
					{
						if(theOr.IsClipped(mask))
						{

							//
							//-----------------------------------
							// Clip each vertex against the plane
							//-----------------------------------
							//
	#if HUNT_CLIP_ERROR
							DEBUG_STREAM << l << ": " << '\n';
	#endif
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
									firstIsIn = true;

									dstPolygon.coords[dstPolygon.length] = 
										srcPolygon.coords[k];
	#if HUNT_CLIP_ERROR
									DEBUG_STREAM << k << " goes " << setiosflags( ios::scientific) << setprecision(20) 
										<< srcPolygon.coords[k].x << " "
										<< srcPolygon.coords[k].y << " "
										<< srcPolygon.coords[k].z << " "
										<< srcPolygon.coords[k].w << '\n';
	#endif


									dstPolygon.clipPerVertex[dstPolygon.length] = 
										srcPolygon.clipPerVertex[k];
	#ifdef I_SAY_YES_TO_COLOR
									dstPolygon.colors[dstPolygon.length] = 
										srcPolygon.colors[k];
	#endif

	#ifdef I_SAY_YES_TO_DUAL_TEXTURES
									dstPolygon.texCoords[2*dstPolygon.length] = 
										srcPolygon.texCoords[2*k];

									dstPolygon.texCoords[2*dstPolygon.length+1] = 
										srcPolygon.texCoords[2*k+1];
	#else
	#ifdef I_SAY_YES_TO_MULTI_TEXTURES
									for(m=0;m<currentNrOfPasses;m++)
									{
										dstPolygon.texCoords[currentNrOfPasses*dstPolygon.length+m] = 
											srcPolygon.texCoords[currentNrOfPasses*k+m];
									}
	#else
									dstPolygon.texCoords[dstPolygon.length] = 
										srcPolygon.texCoords[k];
	#endif
	#endif
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
								else 
								{
									firstIsIn = false;

									if(srcPolygon.clipPerVertex[k1].IsClipped(mask) != 0)
									{
										Verify(
											srcPolygon.clipPerVertex[k1].IsClipped(mask)
											== srcPolygon.clipPerVertex[k].IsClipped(mask)
										);
										continue;
									}
								}

								//
								//-------------------------------------------
								// Find the clipping interval from bc0 to bc1
								//-------------------------------------------
								//
								if(firstIsIn == true)
								{
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

	#if HUNT_CLIP_ERROR
									DEBUG_STREAM << "True " << a << " " << k << " " << k1 << " we get " << dstPolygon.length << '\n';
									DEBUG_STREAM << setiosflags( ios::scientific) << setprecision(20) 
										<< dstPolygon.coords[dstPolygon.length].x << " "
										<< dstPolygon.coords[dstPolygon.length].y << " "
										<< dstPolygon.coords[dstPolygon.length].z << " "
										<< dstPolygon.coords[dstPolygon.length].w << '\n';
	#endif
									DoClipTrick(dstPolygon.coords[dstPolygon.length], l);


									//
									//----------------------------------------------------------
									// If there are colors, lerp them in screen space for now as
									// most cards do that anyway
									//----------------------------------------------------------
									//
	#ifdef I_SAY_YES_TO_COLOR
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
	#endif
									//
									//-----------------------------------------------------
									// If there are texture uv's, we need to lerp them in a
									// perspective correct manner
									//-----------------------------------------------------
									//
	#ifdef I_SAY_YES_TO_DUAL_TEXTURES
									dstPolygon.texCoords[2*dstPolygon.length].Lerp
										(
											srcPolygon.texCoords[2*k],
											srcPolygon.texCoords[2*k1],
											a
										);

									dstPolygon.texCoords[2*dstPolygon.length+1].Lerp
										(
											srcPolygon.texCoords[2*k+1],
											srcPolygon.texCoords[2*k1+1],
											a
										);
	#else
	#ifdef I_SAY_YES_TO_MULTI_TEXTURES
									for(m=0;m<currentNrOfPasses;m++)
									{
										dstPolygon.texCoords[currentNrOfPasses*dstPolygon.length+m].Lerp
											(
												srcPolygon.texCoords[currentNrOfPasses*k+m],
												srcPolygon.texCoords[currentNrOfPasses*k1+m],
												a
											);
									}
	#else
									dstPolygon.texCoords[dstPolygon.length].Lerp
										(
											srcPolygon.texCoords[k],
											srcPolygon.texCoords[k1],
											a
										);
	#endif
	#endif
								}
								else
								{
									a = GetLerpFactor (l, srcPolygon.coords[k1], srcPolygon.coords[k]);
									Verify(a >= 0.0f && a <= 1.0f);

								//
								//------------------------------
								// Lerp the homogeneous position
								//------------------------------
								//
								dstPolygon.coords[dstPolygon.length].Lerp(
									srcPolygon.coords[k1],
									srcPolygon.coords[k],
									a
								);

	#if HUNT_CLIP_ERROR
									DEBUG_STREAM << "False " << a << " " << k << " " << k1 << " we get " << dstPolygon.length << '\n';
									DEBUG_STREAM << setiosflags( ios::scientific) << setprecision(20) 
										<< dstPolygon.coords[dstPolygon.length].x << " "
										<< dstPolygon.coords[dstPolygon.length].y << " "
										<< dstPolygon.coords[dstPolygon.length].z << " "
										<< dstPolygon.coords[dstPolygon.length].w << '\n';
	#endif

								DoClipTrick(dstPolygon.coords[dstPolygon.length], l);


								//
								//----------------------------------------------------------
								// If there are colors, lerp them in screen space for now as
								// most cards do that anyway
								//----------------------------------------------------------
								//
	#ifdef I_SAY_YES_TO_COLOR
		#if COLOR_AS_DWORD
								dstPolygon.colors[dstPolygon.length] = Color_DWORD_Lerp(
									srcPolygon.colors[k1],
									srcPolygon.colors[k],
									a
								);
		#else
								dstPolygon.colors[dstPolygon.length].Lerp(
									srcPolygon.colors[k1],
									srcPolygon.colors[k],
									a
								);
		#endif
	#endif
								//
								//-----------------------------------------------------
								// If there are texture uv's, we need to lerp them in a
								// perspective correct manner
								//-----------------------------------------------------
								//
	#ifdef I_SAY_YES_TO_DUAL_TEXTURES
								dstPolygon.texCoords[2*dstPolygon.length].Lerp
									(
										srcPolygon.texCoords[2*k1],
										srcPolygon.texCoords[2*k],
										a
									);

								dstPolygon.texCoords[2*dstPolygon.length+1].Lerp
									(
										srcPolygon.texCoords[2*k1+1],
										srcPolygon.texCoords[2*k+1],
										a
									);
	#else
	#ifdef I_SAY_YES_TO_MULTI_TEXTURES
								for(m=0;m<currentNrOfPasses;m++)
								{
									dstPolygon.texCoords[currentNrOfPasses*dstPolygon.length+m].Lerp
										(
											srcPolygon.texCoords[currentNrOfPasses*k1+m],
											srcPolygon.texCoords[currentNrOfPasses*k+m],
											a
										);
								}
	#else
								dstPolygon.texCoords[dstPolygon.length].Lerp
									(
										srcPolygon.texCoords[k1],
										srcPolygon.texCoords[k],
										a
									);
	#endif
	#endif
								}

								//
								//-------------------------------------
								// We have to generate a new clip state
								//-------------------------------------
								//
								dstPolygon.clipPerVertex[dstPolygon.length].Clip4dVertex(&dstPolygon.coords[dstPolygon.length]);

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
	#ifdef I_SAY_YES_TO_COLOR
							srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
	#endif
							srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
							srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();	
							srcPolygon.length = dstPolygon.length;

							dstBuffer = !dstBuffer;

							dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
	#ifdef I_SAY_YES_TO_COLOR
							dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
	#endif
							dstPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
							dstPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();
							dstPolygon.length = 0;

						}

						mask = mask << 1;
					}

					theNewOr = 0;
					for(k=0;k<srcPolygon.length;k++)
					{
						theNewOr |= srcPolygon.clipPerVertex[k];
					}
#if HUNT_CLIP_ERROR
					DEBUG_STREAM << "TheOR: " << hex << theNewOr.GetClippingState() << dec << '\n';
#endif
					theOr == theNewOr;
				} while (theNewOr != 0 && loop--);

				//
				//--------------------------------------------------
				// could not clip this rare case, just ignore it
				//--------------------------------------------------
				//
				if(theNewOr != 0)
				{
					testList[i] = 0;
					continue;
				}

				//
				//--------------------------------------------------
				// Move the most recent polygon into the clip buffer
				//--------------------------------------------------
				//
#if HUNT_CLIP_ERROR
				DEBUG_STREAM << "Final: " << srcPolygon.length << '\n';
#endif
				for(k=0;k<srcPolygon.length;k++)
				{
					int clipped_index = myNumberUsedClipVertex + k;
#if HUNT_CLIP_ERROR
					DEBUG_STREAM << setiosflags( ios::scientific) << setprecision(20) 
						<< srcPolygon.coords[k].x << " "
						<< srcPolygon.coords[k].y << " "
						<< srcPolygon.coords[k].z << " "
						<< srcPolygon.coords[k].w << '\n';
#endif

					(*clipExtraCoords)[clipped_index] = srcPolygon.coords[k];

#ifdef I_SAY_YES_TO_COLOR
					(*clipExtraColors)[clipped_index] = srcPolygon.colors[k];
#endif

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
					(*clipExtraTexCoords)[clipped_index] = srcPolygon.texCoords[2*k];
					(*clipExtraTexCoords2)[clipped_index] = srcPolygon.texCoords[2*k+1];
#else
#ifdef I_SAY_YES_TO_MULTI_TEXTURES
					for(m=0;m<currentNrOfPasses;m++)
					{
						(*clipExtraMultiTexCoords)[m][clipped_index] = srcPolygon.texCoords[currentNrOfPasses*k+m];
					}
#else
					(*clipExtraTexCoords)[clipped_index] = srcPolygon.texCoords[k];
#endif
#endif
				}

				numberVerticesPerPolygon = srcPolygon.length;
#if HUNT_CLIP_ERROR
				DEBUG_STREAM << "---" << '\n';
#endif

				(*clipExtraLength)[myNumberUsedClipLength] = numberVerticesPerPolygon;
#ifdef _ARMOR
				(*clipExtraLength)[myNumberUsedClipLength] |= 0x8000;
#endif
			}

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
	gos_vertices = vt->GetActualVertexPool(db);
	numGOSVertices = 0;
	gos_indices = vt->GetActualIndexPool(db);
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
			indexOffset[j] = stride;

#if FOG_HACK
			int fogEntry = state.GetFogMode();
			if(fogEntry)
			{
				*((BYTE *)&gos_vertices[numGOSVertices].frgb + 3) =
					GOSVertex::fogTable[fogEntry-1][Truncate_Float_To_Word((*transformedCoords)[j].w)];
			}
			else
			{
				*((BYTE *)&gos_vertices[numGOSVertices].frgb + 3) = 0xff;
			}
#endif

			GOSCopyData(
				&gos_vertices[numGOSVertices],
				transformedCoords->GetData(),
#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_LIGHTING
				actualColors->GetData(),
	#else
				colors.GetData(),
	#endif
#endif
				texCoords.GetData(),
				j
			);

			if(textureAnimation)
			{
				gos_vertices[numGOSVertices].u += deltaU;
				gos_vertices[numGOSVertices].v += deltaV;
			}

#ifdef LAB_ONLY
			if(gShowClippedPolys)
			{
				gos_vertices[numGOSVertices].argb = 0xff0000ff;
				gos_vertices[numGOSVertices].u = 0.0f;
				gos_vertices[numGOSVertices].v = 0.0f;
			}
#endif

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
			(*texCoords2)[tex2count++] = texCoords[numVertices + j];
#endif
#ifdef I_SAY_YES_TO_MULTI_TEXTURES
			for(m=1;m<currentNrOfPasses;m++)
			{
				(*extraMultiTexCoords)[m][(*tex2count)[m]++] = multiTexCoordsPointers[m][j];
			}
#endif
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
			if(db==false)
			{
				Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());
			}
			else
			{
				Verify(3 + numGOSIndices < 2*Limits::Max_Number_Vertices_Per_Mesh);
			}

			gos_indices[numGOSIndices] = (unsigned short)(index[j] - indexOffset[index[j]]);
			gos_indices[numGOSIndices+1] = (unsigned short)(index[j+k+1] - indexOffset[index[j+k+1]]);
			gos_indices[numGOSIndices+2] = (unsigned short)(index[j+k] - indexOffset[index[j+k]]);

			numGOSIndices += 3;
		}
		j += stride;
	}

	if(myNumberUsedClipLength > 0)
	{
		for(i=0,j=0;i<myNumberUsedClipLength;i++)
		{
#ifdef _ARMOR
			stride = static_cast<unsigned short>((*clipExtraLength)[i] & 0x7fff);
#else
			stride = (*clipExtraLength)[i];
#endif


#if 0
			for(k=j;k<stride+j;k++)
			{
				if(clipExtraCoords[k].x < 0.0f && clipExtraCoords[k].x > -SMALL)
				{
					clipExtraCoords[k].x = 0.0f;
				}

				if(clipExtraCoords[k].y < 0.0f && clipExtraCoords[k].y > -SMALL)
				{
					clipExtraCoords[k].y = 0.0f;
				}

				if(clipExtraCoords[k].z < 0.0f && clipExtraCoords[k].z > -SMALL)
				{
					clipExtraCoords[k].z = 0.0f;
				}

				if(clipExtraCoords[k].x > clipExtraCoords[k].w && clipExtraCoords[k].x < clipExtraCoords[k].w + SMALL)
				{
					clipExtraCoords[k].x = clipExtraCoords[k].w;
				}

				if(clipExtraCoords[k].y > clipExtraCoords[k].w && clipExtraCoords[k].y < clipExtraCoords[k].w + SMALL)
				{
					clipExtraCoords[k].y = clipExtraCoords[k].w;
				}

				if(clipExtraCoords[k].z >= clipExtraCoords[k].w && clipExtraCoords[k].z < clipExtraCoords[k].w + SMALL)
				{
					clipExtraCoords[k].z = clipExtraCoords[k].w - SMALL;
				}
			}
#endif
			for(k=1;k<stride-1;k++)
			{
				if(db==false)
				{
					Verify((vt->GetLast() + 3 + numGOSVertices) < vt->GetLength());
					Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());
				}
				else
				{
					Verify(3 + numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
					Verify(3 + numGOSIndices < 2*Limits::Max_Number_Vertices_Per_Mesh);
				}

#if FOG_HACK
			int fogEntry = state.GetFogMode();
			if(fogEntry)
			{
				fogEntry--;
				*((BYTE *)&gos_vertices[numGOSVertices].frgb + 3) =
					GOSVertex::fogTable[fogEntry][Truncate_Float_To_Word((*clipExtraCoords)[j].w)];
				*((BYTE *)&gos_vertices[numGOSVertices+1].frgb + 3) =
					GOSVertex::fogTable[fogEntry][Truncate_Float_To_Word((*clipExtraCoords)[j+k+1].w)];
				*((BYTE *)&gos_vertices[numGOSVertices+2].frgb + 3) =
					GOSVertex::fogTable[fogEntry][Truncate_Float_To_Word((*clipExtraCoords)[j+k].w)];
			}
			else
			{
				*((BYTE *)&gos_vertices[numGOSVertices].frgb + 3) = 0xff;
				*((BYTE *)&gos_vertices[numGOSVertices+1].frgb + 3) = 0xff;
				*((BYTE *)&gos_vertices[numGOSVertices+2].frgb + 3) = 0xff;
			}
#endif

				GOSCopyTriangleData(
					&gos_vertices[numGOSVertices],
					clipExtraCoords->GetData(),
#ifdef I_SAY_YES_TO_COLOR
					clipExtraColors->GetData(),
#endif
					clipExtraTexCoords->GetData(),
					j, j+k+1, j+k
					);

				if(textureAnimation)
				{
					gos_vertices[numGOSVertices].u += deltaU;
					gos_vertices[numGOSVertices].v += deltaV;

					gos_vertices[numGOSVertices+1].u += deltaU;
					gos_vertices[numGOSVertices+1].v += deltaV;

					gos_vertices[numGOSVertices+2].u += deltaU;
					gos_vertices[numGOSVertices+2].v += deltaV;
				}

#ifdef LAB_ONLY
				if(gShowClippedPolys)
				{
					if((*clipExtraLength)[i] & 0x8000)
					{
						gos_vertices[numGOSVertices].argb = 0xffff0000;
						gos_vertices[numGOSVertices].u = 0.0f;
						gos_vertices[numGOSVertices].v = 0.0f;

						gos_vertices[numGOSVertices+1].argb = 0xffff0000;
						gos_vertices[numGOSVertices+1].u = 0.0f;
						gos_vertices[numGOSVertices+1].v = 0.0f;

						gos_vertices[numGOSVertices+2].argb = 0xffff0000;
						gos_vertices[numGOSVertices+2].u = 0.0f;
						gos_vertices[numGOSVertices+2].v = 0.0f;
					}
					else
					{
						gos_vertices[numGOSVertices].argb = 0xffff9999;
						gos_vertices[numGOSVertices].u = 0.0f;
						gos_vertices[numGOSVertices].v = 0.0f;

						gos_vertices[numGOSVertices+1].argb = 0xffff9999;
						gos_vertices[numGOSVertices+1].u = 0.0f;
						gos_vertices[numGOSVertices+1].v = 0.0f;

						gos_vertices[numGOSVertices+2].argb = 0xffff9999;
						gos_vertices[numGOSVertices+2].u = 0.0f;
						gos_vertices[numGOSVertices+2].v = 0.0f;
					}
				}
#endif

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
				(*texCoords2)[tex2count++] = (*clipExtraTexCoords2)[j];
				(*texCoords2)[tex2count++] = (*clipExtraTexCoords2)[j+k+1];
				(*texCoords2)[tex2count++] = (*clipExtraTexCoords2)[j+k];
#endif
#ifdef I_SAY_YES_TO_MULTI_TEXTURES
				for(m=1;m<currentNrOfPasses;m++)
				{
					(*extraMultiTexCoords)[m][(*tex2count)[m]++] = (*clipExtraMultiTexCoords)[m][j];
					(*extraMultiTexCoords)[m][(*tex2count)[m]++] = (*clipExtraMultiTexCoords)[m][j+k+1];
					(*extraMultiTexCoords)[m][(*tex2count)[m]++] = (*clipExtraMultiTexCoords)[m][j+k];
				}
#endif

				if(db==false)
				{
					Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());
				}
				else
				{
					Verify(3 + numGOSIndices < 2*Limits::Max_Number_Vertices_Per_Mesh);
				}
				Verify(numGOSIndices%3 == 0);

				gos_indices[numGOSIndices] = numGOSVertices;
				gos_indices[numGOSIndices+1] = (unsigned short)(numGOSVertices + 1);
				gos_indices[numGOSIndices+2] = (unsigned short)(numGOSVertices + 2);

				numGOSVertices += 3;
				numGOSIndices += 3;
			}

			j += stride;
		}
#if HUNT_CLIP_ERROR
		DEBUG_STREAM << "***" << endl << endl;
#endif

	}



#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	Verify (tex2count == numGOSVertices);
	if(db==false)
	{
		Verify (vt->GetLast() + 2*numGOSVertices < vt->GetLength());
	}
	else
	{
		Verify (2*numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
	}

	memcpy(gos_vertices + numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

	if(textureAnimation2)
	{
		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = (*texCoords2)[i][0] + deltaU2;
			gos_vertices[j].v = (*texCoords2)[i][1] + deltaV2;
		}
	}
	else
	{
		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = (*texCoords2)[i][0];
			gos_vertices[j].v = (*texCoords2)[i][1];
		}
	}

	if(db==false)
	{
		vt->Increase(2*numGOSVertices);
	}
#else
	#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
		if(db==false)
		{
			Verify (vt->GetLast() + 2*numGOSVertices < vt->GetLength());
		}
		else
		{
			Verify (2*numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
		}

		memcpy(gos_vertices + numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = gos_vertices[i].u*xScale + xOffset;
			gos_vertices[j].v = gos_vertices[i].v*yScale + yOffset;
		}

		if(db==false)
		{
			vt->Increase(2*numGOSVertices);
		}
	#else
		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	#endif
#endif

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
	for(m=1;m<currentNrOfPasses;m++)
	{
		Verify ((*tex2count)[m] == numGOSVertices);
		if(db==false)
		{
			Verify (vt->GetLast() + numGOSVertices < vt->GetLength());
		}
		else
		{
			Verify (numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
		}

		memcpy(gos_vertices + m*numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = (*extraMultiTexCoords)[m][i][0];
			gos_vertices[j].v = (*extraMultiTexCoords)[m][i][1];
		}

		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	}
#endif

	if(db==false)
	{
		vt->IncreaseIndex(numGOSIndices);
	}

	visible = numGOSVertices ? (unsigned char)1 : (unsigned char)0;

	if(visible)
	{
	}
	else
	{
	}

	Stop_Timer(Clipping_Time);

	return ret;
}
