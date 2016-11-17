//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#if !defined(MLR_MLRCLIPTRICK_HPP)
#include<mlr/mlrcliptrick.hpp>
#endif

extern DWORD gShowClippedPolys;

//#############################################################################
//#########################    MLRCardCloud    ################################
//#############################################################################

DynamicArrayOf<MLRClippingState>
	*MLRCardCloud::clipPerVertex;
DynamicArrayOf<Vector4D>
	*MLRCardCloud::clipExtraCoords;
DynamicArrayOf<RGBAColor>
	*MLRCardCloud::clipExtraColors;
DynamicArrayOf<Vector2DScalar>
	*MLRCardCloud::clipExtraTexCoords;

DynamicArrayOf<int>
	*MLRCardCloud::clipExtraLength;

MLRCardCloud::ClassData*
	MLRCardCloud::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRCardCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRCardCloudClassID,
			"MidLevelRenderer::MLRCardCloud",
			MLREffect::DefaultData
		);
	Register_Object(DefaultData);
	
	clipPerVertex = new DynamicArrayOf<MLRClippingState> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(clipPerVertex);
	clipExtraCoords = new DynamicArrayOf<Vector4D> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(clipExtraCoords);
	clipExtraColors = new DynamicArrayOf<RGBAColor> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(clipExtraColors);
	clipExtraTexCoords = new DynamicArrayOf<Vector2DScalar> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Object(clipExtraTexCoords);
	
	clipExtraLength = new DynamicArrayOf<int> (Limits::Max_Number_Primitives_Per_Frame);
	Register_Object(clipExtraLength);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRCardCloud::TerminateClass()
{
	Unregister_Object(clipPerVertex);
	delete clipPerVertex;
	Unregister_Object(clipExtraCoords);
	delete clipExtraCoords;
	Unregister_Object(clipExtraColors);
	delete clipExtraColors;
	Unregister_Object(clipExtraTexCoords);
	delete clipExtraTexCoords;
	
	Unregister_Object(clipExtraLength);
	delete clipExtraLength;

	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRCardCloud::MLRCardCloud(int nr) :
	MLREffect(nr, DefaultData)
{
	Verify(gos_GetCurrentHeap() == Heap);
	usedNrOfCards = NULL;

	Check_Pointer(this);
	
	drawMode = SortData::TriList;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRCardCloud::~MLRCardCloud()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRCardCloud::SetData
	(
		const int *count,
		const Stuff::Point3D *point_data,
		const Stuff::RGBAColor *color_data
	)
{
	Check_Pointer(this);

	usedNrOfCards = count;
	Verify(*usedNrOfCards <= maxNrOf);
	points = point_data;
	colors = color_data;
	Verify(texCoords != NULL);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRCardCloud::SetData
	(
		const int *count,
		const Stuff::Point3D *point_data,
		const Stuff::RGBAColor *color_data,
		const Vector2DScalar *uv_data
	)
{
	Check_Pointer(this);

	usedNrOfCards = count;
	Verify(*usedNrOfCards <= maxNrOf);
	texCoords = uv_data;

	SetData(count, point_data, color_data);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRCardCloud::Draw (DrawEffectInformation *dInfo, GOSVertexPool *allVerticesToDraw, MLRSorter *sorter)
{
	Check_Object(this);

	worldToEffect.Invert(*dInfo->effectToWorld);

	Transform(*usedNrOfCards, 4);

#if 0
	Lighting(*shape->worldToShape, dInfo->activeLights, dInfo->nrOfActiveLights);
#endif

	if( Clip(dInfo->clippingFlags, allVerticesToDraw) )
	{
		sorter->AddEffect(this, dInfo->state);
	}
}

static MLRClippingState theAnd, theOr, theTest;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int 
	MLRCardCloud::Clip(MLRClippingState clippingFlags, GOSVertexPool *vt)
{
	Check_Object(this);

	int i, j, k, l;
	int end, len = *usedNrOfCards;

	int mask, k1, ct=0, ret = 0;
	Scalar a=0.0f, bc0, bc1;
	

	Check_Object(vt);

	gos_vertices = vt->GetActualVertexPool();
	Check_Pointer(gos_vertices);
	numGOSVertices = 0;

	//
	//--------------------------------------
	// See if we don't have to draw anything
	//--------------------------------------
	//
	if(clippingFlags.GetClippingState() == 0 || len <= 0)
	{
		if(len <= 0)
		{
			visible = 0;
		}
		else
		{
			//
			//-------------------------------
			// Handle the non-indexed version
			//-------------------------------
			//

			for(i=0,j=0;i<len;i++,j+=4)
			{
				if(IsOn(i) == false)
				{
					continue;
				}

				GOSCopyTriangleData(
					&gos_vertices[numGOSVertices],
					transformedCoords->GetData(),
					texCoords,
					j, j+1, j+2,
					true
				);

				DWORD tmpColor = GOSCopyColor(&colors[i]);

				for(k=numGOSVertices;k<numGOSVertices+3;k++)
				{
					gos_vertices[k].argb = tmpColor;
				}

				gos_vertices[numGOSVertices + 3] = gos_vertices[numGOSVertices];
				gos_vertices[numGOSVertices + 4] = gos_vertices[numGOSVertices + 2];

				GOSCopyData(
					&gos_vertices[numGOSVertices + 5],
					transformedCoords->GetData(),
					texCoords,
					j+3,
					true
				);
				gos_vertices[numGOSVertices + 5].argb = tmpColor;

				numGOSVertices += 6;
			}

			Check_Object(vt);
			vt->Increase(numGOSVertices);
			
			visible = numGOSVertices ? 1 : 0;
		}

		return visible;
	}

	int	myNumberUsedClipVertex, myNumberUsedClipIndex, myNumberUsedClipLength;

	myNumberUsedClipVertex = 0;
	myNumberUsedClipIndex = 0;
	myNumberUsedClipLength = 0;

	//
	//-------------------------------
	// Handle the non-indexed version
	//-------------------------------
	//
	//
	//-----------------------------------------------------------------
	// Step through each polygon, making sure that we don't try to clip
	// backfaced polygons
	//-----------------------------------------------------------------
	//
	for(i=0,j=0;i<len;i++,j+=4)
	{
//		if(IsOn(i) == false)
//		{
//			continue;
//		}

		TurnVisible(i);

		//
		//---------------------------------------------------------------
		// Test each vertex of the polygon against the allowed clipping
		// planes, and accumulate status for which planes always clip and
		// which planes clipped at least once
		//---------------------------------------------------------------
		//
		theAnd.SetClippingState(0x3f);
		theOr.SetClippingState(0);
		end = j+4;

		Stuff::Vector4D *v4d = transformedCoords->GetData() + j;
		MLRClippingState *cs = clipPerVertex->GetData() + j;

		for(k=j;k<end;k++,v4d++,cs++)
		{
			cs->Clip4dVertex(v4d);;
			
			theAnd &= (*clipPerVertex)[k];

			theOr |= (*clipPerVertex)[k];

#ifdef LAB_ONLY
			if(*cs==0)
			{
				Set_Statistic(NonClippedVertices, NonClippedVertices+1);
			}
			else
			{
				Set_Statistic(ClippedVertices, ClippedVertices+1);
			}
#endif
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
			TurnInVisible(i);
		}
		else if (theOr == 0)
		{
			TurnVisible(i);

			GOSCopyTriangleData(
				&gos_vertices[numGOSVertices],
				transformedCoords->GetData(),
				texCoords,
				j, j+1, j+2,
				true
			);


			DWORD tmpColor = GOSCopyColor(&colors[i]);

			gos_vertices[numGOSVertices].argb = tmpColor;
			gos_vertices[numGOSVertices + 1].argb = tmpColor;
			gos_vertices[numGOSVertices + 2].argb = tmpColor;

			gos_vertices[numGOSVertices + 3] = gos_vertices[numGOSVertices];
			gos_vertices[numGOSVertices + 4] = gos_vertices[numGOSVertices + 2];

			GOSCopyData(
				&gos_vertices[numGOSVertices + 5],
				transformedCoords->GetData(),
				texCoords,
				j+3,
				true
			);
			gos_vertices[numGOSVertices + 5].argb = tmpColor;

#ifdef LAB_ONLY
			if(gShowClippedPolys)
			{
				for(l=0;l<6;l++)
				{
					gos_vertices[numGOSVertices+l].argb = 0xff0000ff;
					gos_vertices[numGOSVertices+l].u = 0.0f;
					gos_vertices[numGOSVertices+l].v = 0.0f;
				}
			}
#endif
			numGOSVertices += 6;

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
			// ultra small triangles clipped at farclip cause problems

			if(theOr.IsFarClipped() == true)
			{
				continue;
			}


			int numberVerticesPerPolygon = 0;

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
					k1 = (k+1 < end) ? k + 1 : j;

					//
					//----------------------------------------------------
					// If this vertex is inside the viewing space, copy it
					// directly to the clipping buffer
					//----------------------------------------------------
					//
					int clipped_index =
						myNumberUsedClipVertex + numberVerticesPerPolygon;
					theTest = (*clipPerVertex)[k];
					if(theTest == 0)
					{
						(*clipExtraCoords)[clipped_index] = (*transformedCoords)[k];

						(*clipExtraTexCoords)[clipped_index] = texCoords[k];

						(*clipExtraColors)[clipped_index] = colors[i];

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
					else if((*clipPerVertex)[k1] != 0)
					{
						Verify((*clipPerVertex)[k1] == (*clipPerVertex)[k]);
						continue;
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
							//
							//-------------------------------------------
							// Find the clipping interval from bc0 to bc1
							//-------------------------------------------
							//
							a = GetLerpFactor(l, (*transformedCoords)[k], (*transformedCoords)[k1]);

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
					(*clipExtraCoords)[clipped_index].Lerp(
						(*transformedCoords)[k],
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
					(*clipExtraTexCoords)[clipped_index].Lerp(
						texCoords[k],
						texCoords[k1],
						a
					);

					(*clipExtraColors)[clipped_index] = colors[i];

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
				EffectClipData srcPolygon, dstPolygon;
				int dstBuffer = 0;

				//
				//-----------------------------------------------------
				// Point the source polygon buffer at our original data
				//-----------------------------------------------------
				//
				srcPolygon.coords = &((*transformedCoords)[j]);
				srcPolygon.clipPerVertex = &((*clipPerVertex)[j]);							
				srcPolygon.flags = 0;


//				srcPolygon.colors = const_cast<RGBAColor*>(&colors[j]);

				srcPolygon.flags |= 2;

				srcPolygon.texCoords = const_cast<Vector2DScalar*>(&texCoords[j]);

				srcPolygon.length = 4;

				//
				//--------------------------------
				// Point to the destination buffer
				//--------------------------------
				//
				dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
//				dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
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
				MLRClippingState theNewOr(0);
				int loop = 4;

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
							for(k=0;k<srcPolygon.length;k++)
							{
								k1 = (k+1 < srcPolygon.length) ? k+1 : 0;

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

									dstPolygon.texCoords[dstPolygon.length] = 
										srcPolygon.texCoords[k];

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

								DoCleanClipTrick(dstPolygon.coords[dstPolygon.length], l);

								//
								//-----------------------------------------------------
								// If there are texture uv's, we need to lerp them in a
								// perspective correct manner
								//-----------------------------------------------------
								//
								dstPolygon.texCoords[dstPolygon.length].Lerp
									(
										srcPolygon.texCoords[k],
										srcPolygon.texCoords[k1],
										a
									);

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
	//						srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
							srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
							srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();	
							srcPolygon.length = dstPolygon.length;

							dstBuffer = !dstBuffer;

							dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
	//						dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
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
                    // sebi: WTF? thanks clang
					//theOr == theNewOr;
					theOr = theNewOr;
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
				for(k=0;k<srcPolygon.length;k++)
				{
					int clipped_index = myNumberUsedClipVertex + k;

					if(srcPolygon.coords[k].z == srcPolygon.coords[k].w)
					{
						srcPolygon.coords[k].z -= SMALL;
					}
					(*clipExtraCoords)[clipped_index] = srcPolygon.coords[k];

					(*clipExtraTexCoords)[clipped_index] = srcPolygon.texCoords[k];

					(*clipExtraColors)[clipped_index] = colors[i];
				}

				numberVerticesPerPolygon = srcPolygon.length;
			}

			(*clipExtraLength)[myNumberUsedClipLength] = numberVerticesPerPolygon;
			myNumberUsedClipVertex += numberVerticesPerPolygon;
			myNumberUsedClipLength++;
			ret++;

//					clip

//					dont draw the original
			TurnInVisible(i);
		}
	}

	if(myNumberUsedClipLength > 0)
	{
		for(i=0,j=0;i<myNumberUsedClipLength;i++)
		{
			int stride = (*clipExtraLength)[i];

			for(k=1;k<stride-1;k++)
			{
				Verify((vt->GetLast() + 3 + numGOSVertices) < vt->GetLength());

				GOSCopyTriangleData(
					&gos_vertices[numGOSVertices],
					clipExtraCoords->GetData(),
					clipExtraColors->GetData(),
					clipExtraTexCoords->GetData(),
					j, j+k+1, j+k,
					true
				);
#ifdef LAB_ONLY
				if(gShowClippedPolys)
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

	return visible;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRCardCloud::TestInstance() const
{
	if (usedNrOfCards)
	{
		Check_Pointer(usedNrOfCards);
		Verify(*usedNrOfCards >= 0);
		Verify(*usedNrOfCards <= maxNrOf);
	}
}

