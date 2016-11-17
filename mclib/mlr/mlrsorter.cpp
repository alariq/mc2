//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#ifdef CalDraw
	GOSVertexPool *ToBeDrawnPrimitive::allVerticesToDraw;
#endif

MLRSorter::ClassData*
	MLRSorter::DefaultData = NULL;

bool dontSeeMe = true;

SortData::DrawFunc SortData::Draw[LastMode]	= 
{
	&SortData::DrawTriList,
	&SortData::DrawTriIndexedList,
	&SortData::DrawPointCloud,
	&SortData::DrawQuads,
	&SortData::DrawLineCloud
};

SortData::LoadSortAlphaFunc SortData::LoadSortAlpha[LastMode]	= 
{
	&SortData::LoadAlphaFromTriList,
	&SortData::LoadAlphaFromTriIndexedList,
	&SortData::LoadAlphaFromPointCloud,
	&SortData::LoadAlphaFromQuads,
	&SortData::LoadAlphaFromLineCloud
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	SortData::DrawTriList()
{
	Start_Timer(GOS_Draw_Time);

#ifdef LAB_ONLY
	if(dontSeeMe == true)
#endif
	{
		if(texture2==0)
		{
			GOSVertex *v = (GOSVertex *)vertices;
			if ((v[0].z >= 0.0f) &&
				(v[0].z < 1.0f) &&
				(v[1].z >= 0.0f) &&  
				(v[1].z < 1.0f) && 
				(v[2].z >= 0.0f) &&  
				(v[2].z < 1.0f))
			{
				gos_DrawTriangles( (GOSVertex *)vertices, numVertices);
			}
		}
		else
		{
			STOP(("GOS doesnt suppert gos_DrawTriangles for gos_VERTEX_2UV yet."));
//			gos_DrawTriangles( (GOSVertex2UV *)vertices, numVertices);
		}
	}
	Stop_Timer(GOS_Draw_Time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	SortData::DrawTriIndexedList()
{
	Start_Timer(GOS_Draw_Time);

#ifdef LAB_ONLY
	if(dontSeeMe == true)
#endif
	{
		WORD newIndicies[4096];
		long startIndex = 0;
		GOSVertex *v = (GOSVertex *)vertices;
		for (long i=0;i<numIndices;i+=3)
		{
			if (((v[indices[i]].z >= 0.0f) &&   (v[indices[i]].z < 1.0f)) &&
				((v[indices[i+1]].z >= 0.0f) && (v[indices[i+1]].z < 1.0f)) &&
				((v[indices[i+2]].z >= 0.0f) && (v[indices[i+2]].z < 1.0f)))
			{
				//Copy these indicies to new array.
				newIndicies[startIndex] = indices[i];
				newIndicies[startIndex+1] = indices[i+1];
				newIndicies[startIndex+2] = indices[i+2];
				startIndex += 3;
			}
		}

		if (startIndex)
		{
			if(texture2==0)
			{
				gos_RenderIndexedArray( (GOSVertex *)vertices, numVertices, newIndicies, startIndex);
			}
			else
			{
				gos_RenderIndexedArray( (GOSVertex2UV *)vertices, numVertices, indices, numIndices);
			}
		}
	}
	Stop_Timer(GOS_Draw_Time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	SortData::DrawPointCloud()
{
	Verify(texture2==0);
	Start_Timer(GOS_Draw_Time);

#ifdef LAB_ONLY
	if(dontSeeMe == true)
#endif
	{

		GOSVertex pArray[32*3];

		float size = (float)numIndices;
		
		if( size == 0 )
		{
			size = 2.4f;
		}
		else
		{
			size *= 2.4f;
		}

		int Triangle = 0, Vertex = 0;
	//
	// Warning! - These points need clipping!
	//
		for( int i=numVertices; i; i-- )
		{
			pArray[Triangle+0] = *((GOSVertex *)vertices + Vertex);
			pArray[Triangle+1] = *((GOSVertex *)vertices + Vertex);
			pArray[Triangle+2] = *((GOSVertex *)vertices + Vertex);

			pArray[Triangle+1].x += size;
			pArray[Triangle+2].y += size;

			Triangle +=3;
			Vertex++;

			if( Triangle==32*3 || i==1)
			{
				if ((pArray[0].z >= 0.0f) &&
					(pArray[0].z < 1.0f) &&
					(pArray[1].z >= 0.0f) &&  
					(pArray[1].z < 1.0f) && 
					(pArray[2].z >= 0.0f) &&  
					(pArray[2].z < 1.0f))
				{
					gos_DrawTriangles( pArray, Triangle );
				}

				Triangle=0;
			}
		}
	}
	Stop_Timer(GOS_Draw_Time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	SortData::DrawQuads()
{
	Start_Timer(GOS_Draw_Time);

#ifdef LAB_ONLY
	if(dontSeeMe == true)
#endif
	{
		if(texture2==0)
		{
//			gos_DrawTriangles( (GOSVertex *)vertices, numVertices);
			gos_DrawQuads( (GOSVertex *)vertices, numVertices);
		}
		else
		{
			STOP(("GOS doesnt suppert gos_DrawQuads for gos_VERTEX_2UV yet."));
//			gos_DrawQuads( (GOSVertex2UV *)vertices, numVertices);
		}

	}
	Stop_Timer(GOS_Draw_Time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	SortData::DrawLineCloud()
{
	Verify(texture2==0);
	Start_Timer(GOS_Draw_Time);

	for(int i=0;i<numVertices;i++)
	{
		if(((GOSVertex *)vertices)[i].x > Environment.screenWidth-1)
		{
			((GOSVertex *)vertices)[i].x = static_cast<Scalar>(Environment.screenWidth-1);
		}
		if(((GOSVertex *)vertices)[i].y > Environment.screenHeight-1)
		{
			((GOSVertex *)vertices)[i].y = static_cast<Scalar>(Environment.screenHeight-1);
		}
	}

#ifdef LAB_ONLY
	if(dontSeeMe == true)
#endif
	{
		gos_DrawLines( (GOSVertex *)vertices, numVertices);
	}
	Stop_Timer(GOS_Draw_Time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	SortData::LoadAlphaFromTriList(SortAlpha **alpha)
{
	Start_Timer(Alpha_Sorting_Time);
	int i, index = 0, end = (int)(numVertices*0.333333333333333333333333);
	Verify(texture2==0);

	for(i=0;i<end;i++)
	{
		alpha[i]->state = &state;

		alpha[i]->triangle[0] = ((GOSVertex *)vertices)[index++];
		alpha[i]->triangle[1] = ((GOSVertex *)vertices)[index++];
		alpha[i]->triangle[2] = ((GOSVertex *)vertices)[index++];

		alpha[i]->distance = alpha[i]->triangle[0].z;
		alpha[i]->distance += alpha[i]->triangle[1].z;
		alpha[i]->distance += alpha[i]->triangle[2].z;
	}
	Stop_Timer(Alpha_Sorting_Time);

	return i;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	SortData::LoadAlphaFromTriIndexedList(SortAlpha **alpha)
{
	Start_Timer(Alpha_Sorting_Time);
	int i, index = 0, end = numIndices/3;
	Verify(texture2==0);

	for(i=0;i<end;i++)
	{
		alpha[i]->state = &state;

		alpha[i]->triangle[0] = ((GOSVertex *)vertices)[indices[index++]];
		alpha[i]->triangle[1] = ((GOSVertex *)vertices)[indices[index++]];
		alpha[i]->triangle[2] = ((GOSVertex *)vertices)[indices[index++]];

		alpha[i]->distance = alpha[i]->triangle[0].z;
		alpha[i]->distance += alpha[i]->triangle[1].z;
		alpha[i]->distance += alpha[i]->triangle[2].z;
	}
	Stop_Timer(Alpha_Sorting_Time);

	return i;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	SortData::LoadAlphaFromPointCloud(SortAlpha**)
{
	Start_Timer(Alpha_Sorting_Time);
	STOP(("Not implemented"));
	Stop_Timer(Alpha_Sorting_Time);

	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	SortData::LoadAlphaFromQuads(SortAlpha**)
{
	Start_Timer(Alpha_Sorting_Time);
	STOP(("Not implemented"));
	Stop_Timer(Alpha_Sorting_Time);

	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	SortData::LoadAlphaFromLineCloud(SortAlpha**)
{
	Start_Timer(Alpha_Sorting_Time);
	STOP(("Not implemented"));
	Stop_Timer(Alpha_Sorting_Time);

	return 0;
}


#ifdef CalDraw
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
ToBeDrawnPrimitive::ToBeDrawnPrimitive()
{
	primitive = NULL;

	cameraPosition = Stuff::LinearMatrix4D::Identity;
	shapeToClipMatrix = Stuff::LinearMatrix4D::Identity;

	for(int i=0;i<Limits::Max_Number_Of_Lights_Per_Primitive;i++)
	{
		activeLights[i] = NULL;
	}
	nrOfActiveLights = 0;
}
#endif

//#############################################################################
//############################    MLRSorter    ################################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSorter::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRSorterClassID,
			"MidLevelRenderer::MLRSorter",
			RegisteredClass::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSorter::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSorter::MLRSorter(ClassData *class_data, MLRTexturePool *tp):
	RegisteredClass(class_data)
{
	Verify(gos_GetCurrentHeap() == Heap);
	texturePool = tp;

	gos_PushCurrentHeap(StaticHeap);
	rawDrawData.SetLength(Limits::Max_Number_Primitives_Per_Frame);

#ifdef CalDraw
	for(int i=0;i<MLRState::PriorityCount;i++)
	{
		lastUsedInBucketNotDrawn[i] = 0;
		priorityBucketsNotDrawn[i].SetLength(Limits::Max_Number_Primitives_Per_Frame + Limits::Max_Number_ScreenQuads_Per_Frame);
	}

	drawData.SetLength(Limits::Max_Number_Primitives_Per_Frame);
#endif
	gos_PopCurrentHeap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRSorter::~MLRSorter()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSorter::StartDraw(const MLRState &default_state)
{
	theCurrentState = default_state;
	theCurrentState.SetRendererState(texturePool);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSorter::Reset ()
{
	lastUsedRaw = 0;

#ifdef CalDraw
	for(int i=0;i<MLRState::PriorityCount;i++)
	{
		lastUsedInBucketNotDrawn[i] = 0;
	}

	lastUsedDraw = 0;
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSorter::DrawPrimitive(MLRPrimitiveBase *pt, int pass)
{
	Check_Object(this); 
	Check_Object(pt);

	SortData *sd = NULL;

	switch(pt->GetSortDataMode())
	{
		case SortData::TriList:
		case SortData::TriIndexedList:
		{
			sd = SetRawData(pt, pass);

			if(theCurrentState != sd->state)
			{
				SetDifferences(theCurrentState, sd->state);
				theCurrentState = sd->state;
			}

			if(sd->texture2>0)
			{
				gos_SetRenderState( 
					gos_State_Texture2, 
					(*texturePool)[sd->texture2]->GetImage(NULL)->GetHandle());

				switch(sd->state.GetMultiTextureMode())
				{
					case MLRState::MultiTextureLightmapMode:
						gos_SetRenderState( gos_State_Multitexture, gos_Multitexture_LightMap );
					break;
					case MLRState::MultiTextureSpecularMode:
						gos_SetRenderState( gos_State_Multitexture, gos_Multitexture_SpecularMap );
                        //sebi: handle no multitexture mode
                    case MLRState::MultiTextureOffMode:
						gos_SetRenderState( gos_State_Multitexture, gos_Multitexture_None);
					break;
				}
			}
			else
			{
			}

			SortData::DrawFunc drawFunc = sd->Draw[sd->type];

			(sd->*drawFunc)();

			if(sd->texture2>0)
			{
				gos_SetRenderState( gos_State_Multitexture, gos_Multitexture_None );
			}
		}
		break;
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
SortData *
	MLRSorter::SetRawData
		(
			void *vertices, 
			int numVertices, 
			const MLRState& state,
			const int& mode,
			int tex2
		)
{
	Check_Object(this);

	SortData *sd = rawDrawData.GetData();

	Verify (lastUsedRaw < Limits::Max_Number_Primitives_Per_Frame);
	Verify (vertices != NULL);
	Verify (numVertices > 0);

	(sd + lastUsedRaw)->vertices = vertices;
	(sd + lastUsedRaw)->indices = 0;

	(sd + lastUsedRaw)->state = state;

	(sd + lastUsedRaw)->numVertices = numVertices;
	(sd + lastUsedRaw)->numIndices = 0;

	(sd + lastUsedRaw)->type = mode;
	(sd + lastUsedRaw)->texture2 = tex2;

	lastUsedRaw++;

	return (sd + lastUsedRaw - 1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
SortData *
	MLRSorter::SetRawIndexedData
		(
			void *vertices, 
			int numVertices, 
			unsigned short *indices, 
			int numIndices, 
			const MLRState& state,
			const int& mode,
			int tex2
		)
{
	Check_Object(this);

	SortData *sd = rawDrawData.GetData();

	Verify (lastUsedRaw < Limits::Max_Number_Primitives_Per_Frame);
	Verify (vertices != NULL);
	Verify (numVertices > 0);

	(sd + lastUsedRaw)->vertices = vertices;
	(sd + lastUsedRaw)->indices = indices;

	(sd + lastUsedRaw)->state = state;

	(sd + lastUsedRaw)->numVertices = numVertices;
	(sd + lastUsedRaw)->numIndices = numIndices;

	(sd + lastUsedRaw)->type = mode;
	(sd + lastUsedRaw)->texture2 = tex2;
		
	lastUsedRaw++;

	return (sd + lastUsedRaw - 1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
SortData *
	MLRSorter::SetRawData ( MLRPrimitiveBase *pt, int pass)
{
	Check_Object(this);
	Check_Object(pt);
	
	int drawMode = pt->GetSortDataMode();
	
	switch(drawMode)
	{
		case SortData::TriIndexedList:
		{
			MLRIndexedPrimitiveBase *ipt = static_cast<MLRIndexedPrimitiveBase *>(pt);

			int tex2 = 0;
			void *vertices = ipt->GetGOSVertices(pass);
			int vertexCount = ipt->GetNumGOSVertices();

			if(pt->GetCurrentState(pass).GetMultiTextureMode()!=MLRState::MultiTextureOffMode && MLRState::GetMultitextureLightMap())
			{
				Verify(pass==0);
				tex2 = pt->GetCurrentState(1).GetTextureHandle();
				vertices = pt->GetGOSVertices2UV();
			}

			return SetRawIndexedData (
				vertices,
				vertexCount, 
				ipt->GetGOSIndices(pass),
				ipt->GetNumGOSIndices(), 
				ipt->GetCurrentState(pass),
				drawMode,
				tex2
			);
		}
		case SortData::TriList:
			return SetRawData (
				pt->GetGOSVertices(pass),
				pt->GetNumGOSVertices(), 
				pt->GetCurrentState(pass),
				drawMode
			);
	}

	return NULL;
}

#ifdef CalDraw
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSorter::IncreaseTBDPCounter()
{
	Check_Object(this);
	int priority = drawData[lastUsedDraw].primitive->GetReferenceState().GetPriority();
	priorityBucketsNotDrawn[priority][lastUsedInBucketNotDrawn[priority]++] = &drawData[lastUsedDraw];
	lastUsedDraw++;
}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MLRSorter::SetDifferences(const MLRState& original, const MLRState& newer)
{
	Verify(original != newer);

	int changed = (original.renderState ^ newer.renderState);

	if(changed)
	{
		if(changed & MLRState::TextureMask)
		{
			Check_Object(texturePool);

			if( newer.renderState & MLRState::TextureMask )
			{
				Verify((*texturePool)[newer.renderState & MLRState::TextureMask]);
				gos_SetRenderState( 
					gos_State_Texture, 
					(*texturePool)[newer.renderState & MLRState::TextureMask]->GetImage(NULL)->GetHandle()
				);
			}
			else
				gos_SetRenderState(gos_State_Texture, 0);
		}

		if(changed & MLRState::AlphaMask)
		{
			switch( newer.renderState&MLRState::AlphaMask )
			{
			case MLRState::OneZeroMode:
				gos_SetRenderState(gos_State_AlphaTest, 0);
				gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_OneZero);
				break;

			case MLRState::OneOneMode:
				gos_SetRenderState(gos_State_AlphaTest, 0);
				gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_OneOne);
				break;

			case MLRState::AlphaOneMode:
				SPEW(("micgaert", "MLRState::AlphaOneMode not available anymore"));
				break;

			case MLRState::OneAlphaMode:
				SPEW(("micgaert", "MLRState::OneAlphaMode not available anymore"));
				break;

			case MLRState::AlphaInvAlphaMode:
				gos_SetRenderState(gos_State_AlphaTest, 1);
				gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
				break;

			case MLRState::OneInvAlphaMode:
				gos_SetRenderState(gos_State_AlphaTest, 0);
				gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_OneInvAlpha);
				break;

			default:
				STOP(("Unknown alpha renderState"));
			}
		}

		if (changed & MLRState::FilterMask)
		{
			switch( newer.renderState&MLRState::FilterMask )
			{
				case MLRState::NoFilterMode:
					gos_SetRenderState(gos_State_Filter, gos_FilterNone);
					break;

				case MLRState::BiLinearFilterMode:
					gos_SetRenderState(gos_State_Filter, gos_FilterBiLinear);
					break;

				case MLRState::TriLinearFilterMode:
					gos_SetRenderState(gos_State_Filter, gos_FilterTriLinear);
					break;

				default:
					STOP(("Unknown filter state"));
			}
		}

		if(changed & MLRState::FogMask)
		{
			if(newer.renderState & MLRState::FogMask)
			{
                // sebi ????
				//gos_SetRenderState(	gos_State_Fog, (int)&newer.fogColor);
				gos_SetRenderState(	gos_State_Fog, (int)newer.fogColor);
			}
			else
			{
				gos_SetRenderState(	gos_State_Fog, 0);
			}
		}

		if(changed & MLRState::SpecularMask)
		{
			gos_SetRenderState(	gos_State_Specular, newer.renderState & MLRState::SpecularOnMode );
		}

		if(changed & MLRState::TextureWrapMask)
		{
			gos_SetRenderState(	
				gos_State_TextureAddress, 
				(newer.renderState & MLRState::TextureClamp) ? gos_TextureClamp : gos_TextureWrap
			);
		}

		if(changed & MLRState::DitherOnMode)
		{
			gos_SetRenderState(	gos_State_Dither, newer.renderState & MLRState::DitherOnMode );
		}

		if(changed & MLRState::TextureCorrectionOnMode)
		{
			gos_SetRenderState(	gos_State_Perspective, newer.renderState & MLRState::TextureCorrectionOnMode );
		}

		if(changed & MLRState::WireFrameMask)
		{
			if( newer.renderState & MLRState::WireFrameOnlyMode )
			{
				DWORD wfColor=0xffffff;
                // sebi , wtf???
				//gos_SetRenderState(	gos_State_WireframeMode, (DWORD)&wfColor );
				gos_SetRenderState(	gos_State_WireframeMode, wfColor );
			}
			else
			{
				gos_SetRenderState(	gos_State_WireframeMode, 0 );
			}
		}

		if(changed & MLRState::FlatColoringMask)
		{
			gos_SetRenderState(gos_State_ShadeMode, gos_ShadeGouraud);
		}

		if(changed & MLRState::ZBufferWriteMask)
		{
			gos_SetRenderState(gos_State_ZWrite, (newer.renderState & MLRState::ZBufferWriteMask) ? 1 : 0);
		}

		if(changed & MLRState::ZBufferCompareMask)
		{
			gos_SetRenderState( gos_State_ZCompare, (newer.renderState & MLRState::ZBufferCompareMask) ? 1 : 0);
		}
	}

	gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);

	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRSorter::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

