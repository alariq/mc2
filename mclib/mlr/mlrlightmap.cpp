//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#if !defined(MLR_MLRCLIPTRICK_HPP)
#include<mlr/mlrcliptrick.hpp>
#endif

//#############################################################################
//###########################    MLRLightMap    ###############################
//#############################################################################

MLRLightMap::ClassData*
	MLRLightMap::DefaultData = NULL;
Stuff::MemoryStream
	*MLRLightMap::stream;
GOSVertexPool*
	MLRLightMap::vertexPool;

DynamicArrayOf<Stuff::Vector4D>
	*transformedCoords, *clipExtraCoords;
DynamicArrayOf<RGBAColor>
	*clipExtraColors;
DynamicArrayOf<Vector2DScalar>
	*clipExtraTexCoords;
DynamicArrayOf<MLRClippingState>
	*clippingStates;
ClipPolygon2
	*MLRLightMap::clipBuffer;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLightMap::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRLightMapClassID,
			"MidLevelRenderer::MLRLightMap",
			RegisteredClass::DefaultData
		);
	Register_Object(DefaultData);

	unsigned char *ptr = new unsigned char [Limits::Max_Size_Of_LightMap_MemoryStream];
	Register_Pointer(ptr);

	stream = new MemoryStream(ptr, Limits::Max_Size_Of_LightMap_MemoryStream);
	Register_Object(stream);

	transformedCoords = new DynamicArrayOf<Stuff::Vector4D> (Limits::Max_Number_Vertices_Per_Polygon);
	Register_Object(transformedCoords);
	clipExtraCoords = new DynamicArrayOf<Stuff::Vector4D> (2*Limits::Max_Number_Vertices_Per_Polygon);
	Register_Object(clipExtraCoords);
	clipExtraColors = new DynamicArrayOf<RGBAColor> (2*Limits::Max_Number_Vertices_Per_Polygon);
	Register_Object(clipExtraColors);
	clipExtraTexCoords = new DynamicArrayOf<Vector2DScalar> (2*Limits::Max_Number_Vertices_Per_Polygon);
	Register_Object(clipExtraTexCoords);
	clippingStates = new DynamicArrayOf<MLRClippingState> (Limits::Max_Number_Vertices_Per_Polygon);
	Register_Object(clippingStates);

	clipBuffer = new ClipPolygon2 [2];
	Register_Pointer(clipBuffer);

	clipBuffer[0].Init(Limits::Max_Number_Of_Multitextures);
	clipBuffer[1].Init(Limits::Max_Number_Of_Multitextures);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLightMap::TerminateClass()
{
	clipBuffer[1].Destroy();
	clipBuffer[0].Destroy();
	Unregister_Pointer(clipBuffer);
	delete [] clipBuffer;

	Unregister_Object(transformedCoords);
	delete transformedCoords;
	Unregister_Object(clipExtraCoords);
	delete clipExtraCoords;
	Unregister_Object(clipExtraColors);
	delete clipExtraColors;
	Unregister_Object(clipExtraTexCoords);
	delete clipExtraTexCoords;
	Unregister_Object(clippingStates);
	delete clippingStates;

	stream->Rewind();
	unsigned char *ptr = (unsigned char *)stream->GetPointer();

	Unregister_Object(stream);
	delete stream;
	stream = NULL;

	Unregister_Pointer(ptr);
	delete [] ptr;

	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLightMap::MLRLightMap(MLRTexture *tex) :
	RegisteredClass(DefaultData)
{
	Verify(gos_GetCurrentHeap() == Heap);
	state.SetTextureHandle(tex->GetTextureHandle());
	state.SetRenderDeltaMask(MLRState::TextureMask);

//	state.SetFogData(0xffffffff, 0.0f, 1.0f, 100.0f);
#ifdef OLDFOG
	state.SetFogMode(MLRState::DisableFogMode);
#else
	state.SetFogMode(0);
#endif

	state.SetZBufferCompareOn();
	state.SetZBufferWriteOff();
	state.SetBackFaceOn();
	state.SetAlphaMode(MLRState::OneOneMode);

	state.SetFilterMode(MLRState::BiLinearFilterMode);
	state.SetTextureWrapMode(MLRState::TextureClamp);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLightMap::~MLRLightMap()
{
	Check_Object(this);
	Check_Object(stream);

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLightMap::SetDrawData
		(
			GOSVertexPool *vp,
			Stuff::Matrix4D *mat,
			MLRClippingState& clippingState,
			MLRState& _state
		)
{
	vertexPool = vp;

	stream->Rewind();

	*stream << (int)Matrix4D;
    // sebi !NB
	//*stream << reinterpret_cast<int>(mat);
	*stream << reinterpret_cast<size_t>(mat);

	*stream << (int)ClippingState;
	clippingState.Save(stream);

	*stream << (int)MasterRenderState;
	_state.Save(stream);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLightMap::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
	Check_Object(stream);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLightMap::DrawLightMaps(MLRSorter *sorter)
{
	Check_Object(stream);

	void *ptr, *end = stream->GetPointer();

	Stuff::Matrix4D *currentMatrix=NULL;
	MLRClippingState currentClippingState;
	
	MLRState currentState, masterState;

	unsigned short stride;

    // sebi !NB (because SetDrawData changed also)
	//int i, pointerValue;
	int i;
    size_t pointerValue;
    //
	Stuff::Point3D *coords = NULL;
	Stuff::RGBAColor color;
	Stuff::RGBAColor *colors = NULL;
	Vector2DScalar *texCoords = NULL;
	DWORD argb = 0xffffffff;

	Check_Object(vertexPool);
	GOSVertex* gos_vertices = vertexPool->GetActualVertexPool();
	int numGOSVertices = 0;

	int msd;
	MemoryStreamData type;

	stream->Rewind();

	ptr = stream->GetPointer();

	while(ptr < end)
	{
		*stream >> msd;

		type = static_cast<MemoryStreamData>(msd);

		switch(msd)
		{
			case Matrix4D:
				*stream >> pointerValue;
				currentMatrix = reinterpret_cast<Stuff::Matrix4D*>(pointerValue);
			break;
			case ClippingState:
				currentClippingState.Load(stream);
			break;
			case MasterRenderState:
				masterState.Load(stream, Current_MLR_Version);
			break;
			case LightMapRenderState:
			{
				MLRState lightmapState;
				lightmapState.Load(stream, Current_MLR_Version);

				lightmapState.Combine(masterState, lightmapState);

				if(numGOSVertices && (lightmapState != currentState))
				{
					vertexPool->Increase(numGOSVertices);

					SortData *sd = sorter->SetRawData
						(
							gos_vertices, 
							numGOSVertices,
							currentState,
							SortData::TriList
						);

					if(currentState.GetDrawNowMode()==MLRState::DrawNowOnMode)

					{
						SortData::DrawFunc drawFunc = sd->Draw[sd->type];
						(sd->*drawFunc)();
					}
					else
					{

						sorter->AddSortRawData(sd);
					}
					gos_vertices = vertexPool->GetActualVertexPool();
					numGOSVertices = 0;

				}
				currentState = lightmapState;
			}
			break;
			case Polygon:
			{
				*stream >> stride;
				Verify(stride<=Limits::Max_Number_Vertices_Per_Polygon);

				*stream >> color;

				argb = GOSCopyColor(&color);

				coords = (Stuff::Point3D *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Stuff::Point3D));
				texCoords = (Vector2DScalar *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Vector2DScalar));

				MLRClippingState theAnd(0x3f), theOr(0);
				MLRClippingState *cs = clippingStates->GetData();
				Vector4D *v4d = transformedCoords->GetData();
				for(i=0;i<stride;i++,v4d++,cs++)
				{
					v4d->Multiply(coords[i], *currentMatrix);

					if(currentClippingState!=0)
					{
						cs->Clip4dVertex(v4d);

						theAnd &= *cs;

						theOr |= *cs;
					}
#if defined(_ARMOR)
					else
					{
						Verify((*transformedCoords)[i].x >= 0.0f && (*transformedCoords)[i].x <= (*transformedCoords)[i].w );
						Verify((*transformedCoords)[i].y >= 0.0f && (*transformedCoords)[i].y <= (*transformedCoords)[i].w );
						Verify((*transformedCoords)[i].z >= 0.0f && (*transformedCoords)[i].z <= (*transformedCoords)[i].w );
					}
#endif
				}

				if(theOr == 0)
				{
					for(i=1;i<stride-1;i++)
					{
						Verify((vertexPool->GetLast() + 3 + numGOSVertices) < vertexPool->GetLength());

						GOSCopyTriangleData(
							&gos_vertices[numGOSVertices],
							transformedCoords->GetData(),
							texCoords,
							0, i + 1, i
						);

						gos_vertices[numGOSVertices].argb = argb;
						gos_vertices[numGOSVertices+1].argb = argb;
						gos_vertices[numGOSVertices+2].argb = argb;

						numGOSVertices+=3;
					}
				}
				else
				{
					if(theAnd != 0)
					{
						break;
					}
					else
					{
						int k, k0, k1, l, mask, ct = 0;
						Scalar a = 0.0f;

						int numberVerticesPerPolygon = 0;

						//
						//---------------------------------------------------------------
						// Handle the case of a single clipping plane by stepping through
						// the vertices and finding the edge it originates
						//---------------------------------------------------------------
						//
						bool firstIsIn;
						MLRClippingState theTest;

						if (theOr.GetNumberOfSetBits() == 1)
						{
#ifdef LAB_ONLY
							Set_Statistic(PolysClippedButOnePlane, PolysClippedButOnePlane+1);
#endif
							for(k=0;k<stride;k++)
							{
								int clipped_index = numberVerticesPerPolygon;

								k0 = k;
								k1 = (k+1) < stride ? k+1 : 0;

								//
								//----------------------------------------------------
								// If this vertex is inside the viewing space, copy it
								// directly to the clipping buffer
								//----------------------------------------------------
								//
								theTest = (*clippingStates)[k0];

								if(theTest == 0)
								{
									firstIsIn = true;
									(*clipExtraCoords)[clipped_index] = (*transformedCoords)[k0];
									Verify((*clipExtraCoords)[clipped_index].x >= 0.0f && (*clipExtraCoords)[clipped_index].x <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].y >= 0.0f && (*clipExtraCoords)[clipped_index].y <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].z >= 0.0f && (*clipExtraCoords)[clipped_index].z <= (*clipExtraCoords)[clipped_index].w );
									(*clipExtraTexCoords)[clipped_index] = texCoords[k0];

									numberVerticesPerPolygon++;
									clipped_index++;

									//
									//-------------------------------------------------------
									// We don't need to clip this edge if the next vertex is
									// also in the viewing space, so just move on to the next
									// vertex
									//-------------------------------------------------------
									//
									if((*clippingStates)[k1] == 0)
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
									if((*clippingStates)[k1] != 0)
									{
										Verify((*clippingStates)[k1] == (*clippingStates)[k0]);
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
								theTest |= (*clippingStates)[k1];

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
									Verify((*clipExtraCoords)[clipped_index].x >= 0.0f && (*clipExtraCoords)[clipped_index].x <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].y >= 0.0f && (*clipExtraCoords)[clipped_index].y <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].z >= 0.0f && (*clipExtraCoords)[clipped_index].z <= (*clipExtraCoords)[clipped_index].w );

									//
									//-----------------------------------------------------
									// If there are texture uv's, we need to lerp them in a
									// perspective correct manner
									//-----------------------------------------------------
									//
									(*clipExtraTexCoords)[clipped_index].Lerp
										(
											texCoords[k0],
											texCoords[k1],
											a
										);
								}
								else
								{
									(*clipExtraCoords)[clipped_index].Lerp(
										(*transformedCoords)[k1],
										(*transformedCoords)[k0],
										a
									);

									DoClipTrick((*clipExtraCoords)[clipped_index], ct);
									Verify((*clipExtraCoords)[clipped_index].x >= 0.0f && (*clipExtraCoords)[clipped_index].x <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].y >= 0.0f && (*clipExtraCoords)[clipped_index].y <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].z >= 0.0f && (*clipExtraCoords)[clipped_index].z <= (*clipExtraCoords)[clipped_index].w );
									//
									//-----------------------------------------------------
									// If there are texture uv's, we need to lerp them in a
									// perspective correct manner
									//-----------------------------------------------------
									//
									(*clipExtraTexCoords)[clipped_index].Lerp
										(
											texCoords[k1],
											texCoords[k0],
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
			#ifdef LAB_ONLY
							Set_Statistic(PolysClippedButGOnePlane, PolysClippedButGOnePlane+1);
			#endif


							ClipData2 srcPolygon, dstPolygon;
							int dstBuffer = 1;

							srcPolygon.coords = clipBuffer[dstBuffer].coords.GetData();

							srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
							srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();

							//
							//----------------------------------------------------------
							// unravel and copy the original data into the source buffer
							//----------------------------------------------------------
							//
							for(k=0;k<stride;k++)
							{
								srcPolygon.coords[k] = (*transformedCoords)[k];
								srcPolygon.texCoords[k] = texCoords[k];
								srcPolygon.clipPerVertex[k] = (*clippingStates)[k];
							}

							srcPolygon.length = stride;

							//
							//--------------------------------
							// Point to the destination buffer
							//--------------------------------
							//
							dstBuffer = 0;

							dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
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

											DoClipTrick(dstPolygon.coords[dstPolygon.length], l);

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

												DoClipTrick(dstPolygon.coords[dstPolygon.length], l);

												//
												//-----------------------------------------------------
												// If there are texture uv's, we need to lerp them in a
												// perspective correct manner
												//-----------------------------------------------------
												//
												dstPolygon.texCoords[dstPolygon.length].Lerp
													(
														srcPolygon.texCoords[k1],
														srcPolygon.texCoords[k],
														a
													);
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
										srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
										srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();	
										srcPolygon.length = dstPolygon.length;

										dstBuffer = !dstBuffer;

										dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
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

								theOr == theNewOr;
								loop++;
							} while (theNewOr != 0 && loop--);

							Verify(theNewOr == 0);
							//
							//--------------------------------------------------
							// Move the most recent polygon into the clip buffer
							//--------------------------------------------------
							//
							for(k=0;k<srcPolygon.length;k++)
							{
								(*clipExtraCoords)[k] = srcPolygon.coords[k];

								Verify((*clipExtraCoords)[k].x >= 0.0f && (*clipExtraCoords)[k].x <= (*clipExtraCoords)[k].w );
								Verify((*clipExtraCoords)[k].y >= 0.0f && (*clipExtraCoords)[k].y <= (*clipExtraCoords)[k].w );
								Verify((*clipExtraCoords)[k].z >= 0.0f && (*clipExtraCoords)[k].z <= (*clipExtraCoords)[k].w );

								(*clipExtraTexCoords)[k] = srcPolygon.texCoords[k];
							}

							numberVerticesPerPolygon = srcPolygon.length;
						}

//						clip

						for(i=1;i<numberVerticesPerPolygon-1;i++)
						{
							Verify((vertexPool->GetLast() + 3 + numGOSVertices) < vertexPool->GetLength());

							GOSCopyTriangleData(
								&gos_vertices[numGOSVertices],
								clipExtraCoords->GetData(),
								clipExtraTexCoords->GetData(),
								0, i + 1, i
							);

							gos_vertices[numGOSVertices].argb = argb;
							gos_vertices[numGOSVertices+1].argb = argb;
							gos_vertices[numGOSVertices+2].argb = argb;

							numGOSVertices+=3;
						}

					}
				}
			}
			break;
			case PolygonWithColor:
			{
				*stream >> stride;
				Verify(stride<=Limits::Max_Number_Vertices_Per_Polygon);

				coords = (Stuff::Point3D *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Stuff::Point3D));
				
				colors = (Stuff::RGBAColor *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Stuff::RGBAColor));

				texCoords = (Vector2DScalar *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Vector2DScalar));

				MLRClippingState theAnd(0x3f), theOr(0);
				MLRClippingState *cs = clippingStates->GetData();
				Vector4D *v4d = transformedCoords->GetData();
				for(i=0;i<stride;i++,v4d++,cs++)
				{
					v4d->Multiply(coords[i], *currentMatrix);

					if(currentClippingState!=0)
					{
						cs->Clip4dVertex(v4d);

						theAnd &= *cs;

						theOr |= *cs;
					}
#if defined(_ARMOR)
					else
					{
						Verify((*transformedCoords)[i].x >= 0.0f && (*transformedCoords)[i].x <= (*transformedCoords)[i].w );
						Verify((*transformedCoords)[i].y >= 0.0f && (*transformedCoords)[i].y <= (*transformedCoords)[i].w );
						Verify((*transformedCoords)[i].z >= 0.0f && (*transformedCoords)[i].z <= (*transformedCoords)[i].w );
					}
#endif
				}

				if(theOr == 0)
				{
					for(i=1;i<stride-1;i++)
					{
						Verify((vertexPool->GetLast() + 3 + numGOSVertices) < vertexPool->GetLength());

						GOSCopyTriangleData(
							&gos_vertices[numGOSVertices],
							transformedCoords->GetData(),
							colors,
							texCoords,
							0, i + 1, i
						);

						numGOSVertices+=3;
					}
				}
				else
				{
					if(theAnd != 0)
					{
						break;
					}
					else
					{
						int k, k0, k1, l, mask, ct = 0;
						Scalar a = 0.0f;

						int numberVerticesPerPolygon = 0;

						//
						//---------------------------------------------------------------
						// Handle the case of a single clipping plane by stepping through
						// the vertices and finding the edge it originates
						//---------------------------------------------------------------
						//
						bool firstIsIn;
						MLRClippingState theTest;

						if (theOr.GetNumberOfSetBits() == 1)
						{
#ifdef LAB_ONLY
							Set_Statistic(PolysClippedButOnePlane, PolysClippedButOnePlane+1);
#endif
							for(k=0;k<stride;k++)
							{
								int clipped_index = numberVerticesPerPolygon;

								k0 = k;
								k1 = (k+1) < stride ? k+1 : 0;

								//
								//----------------------------------------------------
								// If this vertex is inside the viewing space, copy it
								// directly to the clipping buffer
								//----------------------------------------------------
								//
								theTest = (*clippingStates)[k0];

								if(theTest == 0)
								{
									firstIsIn = true;
									(*clipExtraCoords)[clipped_index] = (*transformedCoords)[k0];
									Verify((*clipExtraCoords)[clipped_index].x >= 0.0f && (*clipExtraCoords)[clipped_index].x <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].y >= 0.0f && (*clipExtraCoords)[clipped_index].y <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].z >= 0.0f && (*clipExtraCoords)[clipped_index].z <= (*clipExtraCoords)[clipped_index].w );
									(*clipExtraColors)[clipped_index] = colors[k0];
									(*clipExtraTexCoords)[clipped_index] = texCoords[k0];

									numberVerticesPerPolygon++;
									clipped_index++;

									//
									//-------------------------------------------------------
									// We don't need to clip this edge if the next vertex is
									// also in the viewing space, so just move on to the next
									// vertex
									//-------------------------------------------------------
									//
									if((*clippingStates)[k1] == 0)
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
									if((*clippingStates)[k1] != 0)
									{
										Verify((*clippingStates)[k1] == (*clippingStates)[k0]);
										continue;
									}
								}

								//
								//--------------------------------------------------
								// We now find the distance along the edge where the
								// clipping plane will intersect
								//--------------------------------------------------
								//)
								mask = 1;
								theTest |= (*clippingStates)[k1];

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
									Verify((*clipExtraCoords)[clipped_index].x >= 0.0f && (*clipExtraCoords)[clipped_index].x <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].y >= 0.0f && (*clipExtraCoords)[clipped_index].y <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].z >= 0.0f && (*clipExtraCoords)[clipped_index].z <= (*clipExtraCoords)[clipped_index].w );

									//
									//-----------------------------------------------------
									// If there are texture uv's, we need to lerp them in a
									// perspective correct manner
									//-----------------------------------------------------
									//
									(*clipExtraTexCoords)[clipped_index].Lerp
										(
											texCoords[k0],
											texCoords[k1],
											a
										);

									(*clipExtraColors)[clipped_index].Lerp
										(
											colors[k0],
											colors[k1],
											a
										);
								}
								else
								{
									(*clipExtraCoords)[clipped_index].Lerp(
										(*transformedCoords)[k1],
										(*transformedCoords)[k0],
										a
									);

									DoClipTrick((*clipExtraCoords)[clipped_index], ct);
									Verify((*clipExtraCoords)[clipped_index].x >= 0.0f && (*clipExtraCoords)[clipped_index].x <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].y >= 0.0f && (*clipExtraCoords)[clipped_index].y <= (*clipExtraCoords)[clipped_index].w );
									Verify((*clipExtraCoords)[clipped_index].z >= 0.0f && (*clipExtraCoords)[clipped_index].z <= (*clipExtraCoords)[clipped_index].w );
									//
									//-----------------------------------------------------
									// If there are texture uv's, we need to lerp them in a
									// perspective correct manner
									//-----------------------------------------------------
									//
									(*clipExtraTexCoords)[clipped_index].Lerp
										(
											texCoords[k1],
											texCoords[k0],
											a
										);

									(*clipExtraColors)[clipped_index].Lerp
										(
											colors[k1],
											colors[k0],
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
			#ifdef LAB_ONLY
							Set_Statistic(PolysClippedButGOnePlane, PolysClippedButGOnePlane+1);
			#endif


							ClipData2 srcPolygon, dstPolygon;
							int dstBuffer = 1;

							srcPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
							srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
							srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
							srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();

							//
							//----------------------------------------------------------
							// unravel and copy the original data into the source buffer
							//----------------------------------------------------------
							//
							for(k=0;k<stride;k++)
							{
								srcPolygon.coords[k] = (*transformedCoords)[k];
								srcPolygon.texCoords[k] = texCoords[k];
								srcPolygon.colors[k] = colors[k];
								srcPolygon.clipPerVertex[k] = (*clippingStates)[k];
							}

							srcPolygon.length = stride;

							//
							//--------------------------------
							// Point to the destination buffer
							//--------------------------------
							//
							dstBuffer = 0;

							dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
							dstPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
							dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
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

												dstPolygon.clipPerVertex[dstPolygon.length] = 
													srcPolygon.clipPerVertex[k];
												dstPolygon.texCoords[dstPolygon.length] = 
													srcPolygon.texCoords[k];
												dstPolygon.colors[dstPolygon.length] = 
													srcPolygon.colors[k];
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

											DoClipTrick(dstPolygon.coords[dstPolygon.length], l);

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
											dstPolygon.colors[dstPolygon.length].Lerp
												(
													srcPolygon.colors[k],
													srcPolygon.colors[k1],
													a
												);
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

												DoClipTrick(dstPolygon.coords[dstPolygon.length], l);

												//
												//-----------------------------------------------------
												// If there are texture uv's, we need to lerp them in a
												// perspective correct manner
												//-----------------------------------------------------
												//
												dstPolygon.texCoords[dstPolygon.length].Lerp
													(
														srcPolygon.texCoords[k1],
														srcPolygon.texCoords[k],
														a
													);
												dstPolygon.colors[dstPolygon.length].Lerp
													(
														srcPolygon.colors[k1],
														srcPolygon.colors[k],
														a
													);
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
										srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
										srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
										srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();	
										srcPolygon.length = dstPolygon.length;

										dstBuffer = !dstBuffer;

										dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
										dstPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
										dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
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

								theOr == theNewOr;
								loop++;
							} while (theNewOr != 0 && loop--);

							Verify(theNewOr == 0);
							//
							//--------------------------------------------------
							// Move the most recent polygon into the clip buffer
							//--------------------------------------------------
							//
							for(k=0;k<srcPolygon.length;k++)
							{
								(*clipExtraCoords)[k] = srcPolygon.coords[k];

								Verify((*clipExtraCoords)[k].x >= 0.0f && (*clipExtraCoords)[k].x <= (*clipExtraCoords)[k].w );
								Verify((*clipExtraCoords)[k].y >= 0.0f && (*clipExtraCoords)[k].y <= (*clipExtraCoords)[k].w );
								Verify((*clipExtraCoords)[k].z >= 0.0f && (*clipExtraCoords)[k].z <= (*clipExtraCoords)[k].w );

								(*clipExtraTexCoords)[k] = srcPolygon.texCoords[k];

								(*clipExtraColors)[k] = srcPolygon.colors[k];
							}

							numberVerticesPerPolygon = srcPolygon.length;
						}

//						clip

						for(i=1;i<numberVerticesPerPolygon-1;i++)
						{
							Verify((vertexPool->GetLast() + 3 + numGOSVertices) < vertexPool->GetLength());

							GOSCopyTriangleData(
								&gos_vertices[numGOSVertices],
								clipExtraCoords->GetData(),
								clipExtraColors->GetData(),
								clipExtraTexCoords->GetData(),
								0, i + 1, i
							);

							numGOSVertices+=3;
						}

					}
				}
			}
			break;
		}
		ptr = stream->GetPointer();
	}

	if(numGOSVertices)
	{
		vertexPool->Increase(numGOSVertices);

		SortData *sd = sorter->SetRawData
			(
				gos_vertices, 
				numGOSVertices,
				currentState,
				SortData::TriList
			);

		if(currentState.GetDrawNowMode()==MLRState::DrawNowOnMode)
		{
			SortData::DrawFunc drawFunc = sd->Draw[sd->type];
			(sd->*drawFunc)();
		}
		else
		{

			sorter->AddSortRawData(sd);
		}
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape*
	MLRLightMap::CreateLightMapShape()
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLRShape *ret = new MLRShape(20);
	Register_Object(ret);
	MLR_I_C_TMesh *ctmesh = NULL;
	
	void *ptr, *end = stream->GetPointer();

	MLRClippingState currentClippingState;
	
	MLRState currentState, masterState;

	unsigned short stride;

	int i;
	Stuff::Point3D *coords = NULL;
	Stuff::RGBAColor color;
	Stuff::RGBAColor *colors = NULL;
	Vector2DScalar *texCoords = NULL;

	int numGOSVertices = 0;

	int msd;
	MemoryStreamData type;

	stream->Rewind();

	ptr = stream->GetPointer();

	while(ptr < end)
	{
		*stream >> msd;

		type = static_cast<MemoryStreamData>(msd);

		switch(msd)
		{
			case Matrix4D:
				//	not this time
				*stream >> i;
			break;
			case ClippingState:
				//	not this time
				*stream >> i;
			break;
			case MasterRenderState:
				//	not this time
				masterState.Load(stream, Current_MLR_Version);
			break;
			case LightMapRenderState:
			{
				MLRState state; 
				state.Load(stream, Current_MLR_Version);

				if(numGOSVertices && (state != currentState))
				{
					if(ctmesh!=NULL)
					{
						ctmesh->SetSubprimitiveLengths(NULL, numGOSVertices/3);

						ctmesh->FlashClipCoords(numGOSVertices);
						ctmesh->FlashClipTexCoords(numGOSVertices);
						ctmesh->FlashClipColors(numGOSVertices);
						ctmesh->SetReferenceState(currentState);

						ctmesh->TheIndexer(numGOSVertices);

						ctmesh->FindFacePlanes();

						ret->Add(ctmesh);

						ctmesh->DetachReference();
						
						ctmesh = NULL;
					}
				}
				currentState = state;
			}
			break;
			case Polygon:
			{
				if(ctmesh == NULL)
				{
					ctmesh = new MLR_I_C_TMesh;
					numGOSVertices = 0;
				}
				*stream >> stride;
				Verify(stride==3);

				*stream >> color;

#if COLOR_AS_DWORD
				DWORD argb = 0xffffffff;
				argb = GOSCopyColor(&color);
#endif

				coords = (Stuff::Point3D *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Stuff::Point3D));
				texCoords = (Vector2DScalar *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Vector2DScalar));

				for(i=0;i<stride;i++,numGOSVertices++)
				{
					ctmesh->SetClipCoord(coords[i], numGOSVertices);
					ctmesh->SetClipTexCoord(texCoords[i], numGOSVertices);
#if COLOR_AS_DWORD
					ctmesh->SetClipColor(argb, numGOSVertices);
#else
					ctmesh->SetClipColor(color, numGOSVertices);
#endif
				}
			}
			break;
			case PolygonWithColor:
			{
				if(ctmesh == NULL)
				{
					ctmesh = new MLR_I_C_TMesh;
					numGOSVertices = 0;
				}

				*stream >> stride;
				Verify(stride<=Limits::Max_Number_Vertices_Per_Polygon);

				coords = (Stuff::Point3D *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Stuff::Point3D));
				
				colors = (Stuff::RGBAColor *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Stuff::RGBAColor));

				texCoords = (Vector2DScalar *)stream->GetPointer();
				stream->AdvancePointer(stride*sizeof(Vector2DScalar));


				for(i=0;i<stride;i++,numGOSVertices++)
				{
					ctmesh->SetClipCoord(coords[i], numGOSVertices);
					ctmesh->SetClipTexCoord(texCoords[i], numGOSVertices);
#if COLOR_AS_DWORD
					#error not implemented yet
#else
					ctmesh->SetClipColor(colors[i], numGOSVertices);
#endif
				}
			}
			break;
		}
		ptr = stream->GetPointer();
	}

	if(ctmesh!=NULL)
	{
		ctmesh->SetSubprimitiveLengths(NULL, numGOSVertices/3);

		ctmesh->FlashClipCoords(numGOSVertices);
		ctmesh->FlashClipTexCoords(numGOSVertices);
		ctmesh->FlashClipColors(numGOSVertices);
		ctmesh->SetReferenceState(currentState);

		ctmesh->TheIndexer(numGOSVertices);

		ctmesh->FindFacePlanes();

		ret->Add(ctmesh);

		ctmesh->DetachReference();
	}
	gos_PopCurrentHeap();

	return ret;
}
