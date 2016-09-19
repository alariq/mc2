//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

extern DWORD gEnableTextureSort, gEnableAlphaSort, gEnableLightMaps;


MLRSortByOrder::ClassData*
	MLRSortByOrder::DefaultData = NULL;

//#############################################################################
//############################    MLRSortByOrder    ################################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRSortByOrderClassID,
			"MidLevelRenderer::MLRSortByOrder",
			MLRSorter::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSortByOrder::MLRSortByOrder(MLRTexturePool *tp) :
	MLRSorter(DefaultData, tp)
{
	Verify(gos_GetCurrentHeap() == Heap);
	int i;

	gos_PushCurrentHeap(StaticHeap);
	for(i=0;i<MLRState::PriorityCount;i++)
	{
		lastUsedInBucket[i] = 0;
		priorityBuckets[i].SetLength(Limits::Max_Number_Primitives_Per_Frame + Limits::Max_Number_ScreenQuads_Per_Frame);
	}
	alphaSort.SetLength(2*Limits::Max_Number_Vertices_Per_Frame);
	for(i=0;i<alphaSort.GetLength();i++)
	{
		alphaSort[i] = new SortAlpha;
	}
	gos_PopCurrentHeap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSortByOrder::~MLRSortByOrder()
{
	int i;

	gos_PushCurrentHeap(StaticHeap);
	for(i=0;i<alphaSort.GetLength();i++)
	{
		delete alphaSort[i];
	}
	alphaSort.SetLength(0);
	gos_PopCurrentHeap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::Reset ()
{
	Check_Object(this);

	int i;

	for(i=0;i<MLRState::PriorityCount;i++)
	{
		lastUsedInBucket[i] = 0;
	}

	MLRSorter::Reset();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::AddPrimitive(MLRPrimitiveBase *pt, int pass)
{
	Check_Object(this); 
	Check_Object(pt);

	SortData *sd = NULL;

	switch(pt->GetSortDataMode())
	{
		case SortData::TriList:
		case SortData::TriIndexedList:
			sd = SetRawData(pt, pass);
		break;
	}

	unsigned priority = pt->GetCurrentState(pass).GetPriority();
	if(sd != NULL)
	{
		priorityBuckets[priority][lastUsedInBucket[priority]++] = sd;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::AddEffect(MLREffect *ef, const MLRState& state)
{
	Check_Object(this); 
	Check_Object(ef);

	SortData *sd = NULL;

	switch(ef->GetSortDataMode())
	{
		case SortData::PointCloud:
			sd = SetRawData(
					ef->GetGOSVertices(),
					ef->GetNumGOSVertices(), 
					state,
					SortData::PointCloud
					);
			sd->numIndices = ef->GetType(0);
		break;
		case SortData::LineCloud:
			sd = SetRawData(
					ef->GetGOSVertices(),
					ef->GetNumGOSVertices(), 
					state,
					SortData::LineCloud
					);
			sd->numIndices = ef->GetType(0);
		break;
		case SortData::TriList:
			sd = SetRawData(
					ef->GetGOSVertices(),
					ef->GetNumGOSVertices(), 
					state,
					SortData::TriList
					);
		break;
		case SortData::TriIndexedList:
		{
			MLRIndexedTriangleCloud *itc = Cast_Object(MLRIndexedTriangleCloud*, ef);
			sd = SetRawIndexedData(
					itc->GetGOSVertices(),
					itc->GetNumGOSVertices(), 
					itc->GetGOSIndices(),
					itc->GetNumGOSIndices(), 
					state,
					SortData::TriIndexedList
					);
		}
		break;
	}

	unsigned priority = state.GetPriority();
	if(sd != NULL)
	{
		priorityBuckets[priority][lastUsedInBucket[priority]++] = sd;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::AddScreenQuads
		(
			GOSVertex *vertices, 
			const DrawScreenQuadsInformation *dInfo
		)
{
	Verify(dInfo->currentNrOfQuads != 0 && (dInfo->currentNrOfQuads&3) == 0);

	SortData *sd = SetRawData
		(
			vertices,
			dInfo->currentNrOfQuads,
			dInfo->state,
			SortData::Quads
		);

	unsigned priority = dInfo->state.GetPriority();
	if(sd != NULL)
	{
		priorityBuckets[priority][lastUsedInBucket[priority]++] = sd;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::AddSortRawData(SortData *sd)
{
	Check_Object(this); 
	
	if(sd==NULL)
	{
		return;
	}

	unsigned priority = sd->state.GetPriority();
	priorityBuckets[priority][lastUsedInBucket[priority]++] = sd;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::RenderNow ()
{
	Check_Object(this);
//
// So GameOS knows how long the transform and clip and lighting took of update renderers
//

	Stuff::DynamicArrayOf<SortData*>
		*priorityBucket;
	Stuff::DynamicArrayOf<ToBeDrawnPrimitive*>
		*priorityBucketNotDrawn;

	GOSVertex::farClipReciprocal = farClipReciprocal;

	int i, j, k;
//	Limits::Max_Number_Primitives_Per_Frame + Max_Number_ScreenQuads_Per_Frame

	MLRPrimitiveBase *primitive;
	for(i=0;i<MLRState::PriorityCount;i++)
	{
#ifdef CalDraw
		ToBeDrawnPrimitive *tbdp;
		int alphaToSort = 0;

		if(lastUsedInBucketNotDrawn[i])
		{
			Verify(lastUsedInBucketNotDrawn[i]<=Limits::Max_Number_Primitives_Per_Frame + Limits::Max_Number_ScreenQuads_Per_Frame);

			if( gEnableTextureSort && i != MLRState::AlphaPriority)
			{
				Start_Timer(Texture_Sorting_Time);
				// do a shell sort

				int ii, jj, hh;

				ToBeDrawnPrimitive *tempSortData;
				priorityBucketNotDrawn = &priorityBucketsNotDrawn[i];

				for(hh=1;hh<lastUsedInBucketNotDrawn[i]/9;hh=3*hh+1);

				for(;hh>0;hh/=3)
				{
					for(ii=hh;ii<lastUsedInBucketNotDrawn[i];ii++)
					{
						tempSortData = (*priorityBucketNotDrawn)[ii];

						jj = ii;
						while(jj>=hh && (*priorityBucketNotDrawn)[jj-hh]->state.GetTextureHandle() > tempSortData->state.GetTextureHandle())
						{
							(*priorityBucketNotDrawn)[jj] = (*priorityBucketNotDrawn)[jj-hh];
							jj -= hh;
						}
						(*priorityBucketNotDrawn)[jj] = tempSortData;
					}
				}
				Stop_Timer(Texture_Sorting_Time);
			}

			if(i != MLRState::AlphaPriority)
			{
				for(j=0;j<lastUsedInBucketNotDrawn[i];j++)
				{
					tbdp = priorityBucketsNotDrawn[i][j];
					Check_Pointer(tbdp);
					primitive = tbdp->primitive;
					Check_Object(primitive);

					int	nrOfLightMaps = 0;

					for(k=0;k<tbdp->nrOfActiveLights;k++)
					{
						Check_Object(tbdp->activeLights[k]);
						nrOfLightMaps += (tbdp->activeLights[k]->GetLightMap() != NULL) ? 1 : 0;
						tbdp->activeLights[k]->SetLightToShapeMatrix(tbdp->worldToShape);
					}

					if(!gEnableLightMaps)
					{
						nrOfLightMaps = 0;
					}

					if(nrOfLightMaps)
					{
						MLRLightMap::SetDrawData
							(
								ToBeDrawnPrimitive::allVerticesToDraw,
								&tbdp->shapeToClipMatrix,
								tbdp->clippingFlags,
								tbdp->state
							);
					}

					if(primitive->FindBackFace(tbdp->cameraPosition))
					{
						primitive->Lighting(tbdp->activeLights, tbdp->nrOfActiveLights);

						if(tbdp->clippingFlags.GetClippingState() != 0)
						{
							if(primitive->TransformAndClip(
								&tbdp->shapeToClipMatrix,
								tbdp->clippingFlags,
								ToBeDrawnPrimitive::allVerticesToDraw,
								true)
							)
							{
								if(primitive->GetVisible())
								{
									for(k=0;k<primitive->GetNumPasses();k++)
									{
										DrawPrimitive(primitive, k);
									}
								}
							}
						}
						else
						{
							primitive->TransformNoClip(
								&tbdp->shapeToClipMatrix,
								ToBeDrawnPrimitive::allVerticesToDraw,
								true);

							for(k=0;k<primitive->GetNumPasses();k++)
							{
								DrawPrimitive(primitive, k);
							}
						}
#ifdef LAB_ONLY
						Set_Statistic(Number_Of_Primitives, Number_Of_Primitives+1);

						if(primitive->IsDerivedFrom(MLRIndexedPrimitiveBase::DefaultData))
						{
							Point3D *coords;
							unsigned short *indices;
							int nr;

							(Cast_Pointer(MLRIndexedPrimitiveBase*, primitive))->GetIndexData(&indices, &nr);
							Set_Statistic(NumAllIndices, NumAllIndices+nr);

							primitive->GetCoordData(&coords, &nr);
							Set_Statistic(NumAllVertices, NumAllVertices + nr);

							Set_Statistic(Index_Over_Vertex_Ratio, (Scalar)NumAllIndices/(Scalar)NumAllVertices);
						}
#endif

					}

					if(nrOfLightMaps)
					{
						MLRLightMap::DrawLightMaps(this);
					}
				}
			}
			else
			{
				SortData *sd = NULL;

				for(j=0;j<lastUsedInBucketNotDrawn[i];j++)
				{
					tbdp = priorityBucketsNotDrawn[i][j];
					primitive = tbdp->primitive;

					if(primitive->FindBackFace(tbdp->cameraPosition))
					{
						primitive->Lighting(tbdp->activeLights, tbdp->nrOfActiveLights);

						if(tbdp->clippingFlags.GetClippingState() != 0)
						{
							if(primitive->TransformAndClip(
								&tbdp->shapeToClipMatrix,
								tbdp->clippingFlags,
								ToBeDrawnPrimitive::allVerticesToDraw,
								true)
							)
							{
								if(!primitive->GetVisible())
								{
									continue;
								}
							}
						}
						else
						{
							primitive->TransformNoClip(
								&tbdp->shapeToClipMatrix,
								ToBeDrawnPrimitive::allVerticesToDraw,
								true);
						}
#ifdef LAB_ONLY
						Set_Statistic(Number_Of_Primitives, Number_Of_Primitives+1);

						if(primitive->IsDerivedFrom(MLRIndexedPrimitiveBase::DefaultData))
						{
							Point3D *coords;
							unsigned short *indices;
							int nr;

							(Cast_Pointer(MLRIndexedPrimitiveBase*, primitive))->GetIndexData(&indices, &nr);
							Set_Statistic(NumAllIndices, NumAllIndices+nr);

							primitive->GetCoordData(&coords, &nr);
							Set_Statistic(NumAllVertices, NumAllVertices+nr);

							Set_Statistic(Index_Over_Vertex_Ratio, (Scalar)NumAllIndices/(Scalar)NumAllVertices);
						}
#endif
					}
					else
					{
						continue;
					}

					if(primitive->GetNumGOSVertices() > 0)
						for(k=0;k<primitive->GetNumPasses();k++)
					{

						sd = SetRawData(primitive, k);
						Check_Pointer(sd);

						if( gEnableAlphaSort && (sd->type == SortData::TriList || sd->type == SortData::TriIndexedList) )
						{
							SortData::LoadSortAlphaFunc alphaFunc = sd->LoadSortAlpha[sd->type];

							Verify(alphaToSort+sd->numVertices/3 < 2*Limits::Max_Number_Vertices_Per_Frame);
							alphaToSort += (sd->*alphaFunc)(alphaSort.GetData() + alphaToSort);
						}
						else
						{
							if(theCurrentState != sd->state)
							{
								SetDifferences(theCurrentState, sd->state);
								theCurrentState = sd->state;
							}
							SortData::DrawFunc drawFunc = sd->Draw[sd->type];

							(sd->*drawFunc)();
						}
					}

				}
			}
		}

#endif
		if(lastUsedInBucket[i])
		{
			Verify(lastUsedInBucket[i]<=Limits::Max_Number_Primitives_Per_Frame + Limits::Max_Number_ScreenQuads_Per_Frame);

			if( gEnableTextureSort && i != MLRState::AlphaPriority)
			{
				Start_Timer(Texture_Sorting_Time);
				// do a shell sort

				int ii, jj, hh;

				SortData *tempSortData;
				priorityBucket = &priorityBuckets[i];

				for(hh=1;hh<lastUsedInBucket[i]/9;hh=3*hh+1);

				for(;hh>0;hh/=3)
				{
					for(ii=hh;ii<lastUsedInBucket[i];ii++)
					{
						tempSortData = (*priorityBucket)[ii];

						jj = ii;

						while(jj>=hh && (*priorityBucket)[jj-hh]->state.GetTextureHandle() > tempSortData->state.GetTextureHandle())
						{
							(*priorityBucket)[jj] = (*priorityBucket)[jj-hh];
							jj -= hh;
						}
						(*priorityBucket)[jj] = tempSortData;
					}
				}
				Stop_Timer(Texture_Sorting_Time);
			}

			if(i != MLRState::AlphaPriority)
			{
				for(j=0;j<lastUsedInBucket[i];j++)
				{
					SortData *sd = priorityBuckets[i][j];
					Check_Pointer(sd);

					if(theCurrentState != sd->state)
					{
						SetDifferences(theCurrentState, sd->state);
						theCurrentState = sd->state;
					}
					SortData::DrawFunc drawFunc = sd->Draw[sd->type];

	//				_clear87();
					(sd->*drawFunc)();
	//				_clear87();
				}
			}
			else
			{
				for(j=0;j<lastUsedInBucket[i];j++)
				{
					SortData *sd = priorityBuckets[i][j];
					Check_Pointer(sd);

					if( gEnableAlphaSort && (sd->type == SortData::TriList || sd->type == SortData::TriIndexedList) )
					{
						SortData::LoadSortAlphaFunc alphaFunc = sd->LoadSortAlpha[sd->type];

						Verify(alphaToSort+sd->numVertices/3 < 2*Limits::Max_Number_Vertices_Per_Frame);
						alphaToSort += (sd->*alphaFunc)(alphaSort.GetData() + alphaToSort);
					}
					else
					{
						if(theCurrentState != sd->state)
						{
							SetDifferences(theCurrentState, sd->state);
							theCurrentState = sd->state;
						}
						SortData::DrawFunc drawFunc = sd->Draw[sd->type];

						(sd->*drawFunc)();
					}
				}
			}
		}

		if(alphaToSort > 0)
		{
			Start_Timer(Alpha_Sorting_Time);
			// do a shell sort

			int ii, jj, hh;

			SortAlpha *tempSortAlpha;
			Stuff::DynamicArrayOf<SortAlpha*>
				*alphaArray;

			alphaArray = &alphaSort;

			for(hh=1;hh<alphaToSort/9;hh=3*hh+1);

			for(;hh>0;hh/=3)
			{
				for(ii=hh;ii<alphaToSort;ii++)
				{
					tempSortAlpha = (*alphaArray)[ii];

					jj = ii;

					while(jj>=hh && (*alphaArray)[jj-hh]->distance < tempSortAlpha->distance)
					{
						(*alphaArray)[jj] = (*alphaArray)[jj-hh];
						jj -= hh;

					}
					(*alphaArray)[jj] = tempSortAlpha;
				}
			}

			Stop_Timer(Alpha_Sorting_Time);

			for(ii=0;ii<alphaToSort;ii++)
			{
				if(theCurrentState != *alphaSort[ii]->state)
				{
					SetDifferences(theCurrentState, *alphaSort[ii]->state);
					theCurrentState = *alphaSort[ii]->state;
				}
				Start_Timer(GOS_Draw_Time);
	
				if ((alphaSort[ii]->triangle[0].z >= 0.0f) &&
					(alphaSort[ii]->triangle[0].z < 1.0f) &&
					(alphaSort[ii]->triangle[1].z >= 0.0f) &&  
					(alphaSort[ii]->triangle[1].z < 1.0f) && 
					(alphaSort[ii]->triangle[2].z >= 0.0f) &&  
					(alphaSort[ii]->triangle[2].z < 1.0f))
				{
					gos_DrawTriangles( &(alphaSort[ii]->triangle[0]), 3);
				}

				Stop_Timer(GOS_Draw_Time);
			}
			Set_Statistic(NumberOfAlphaSortedTriangles, NumberOfAlphaSortedTriangles+alphaToSort);
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSortByOrder::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

