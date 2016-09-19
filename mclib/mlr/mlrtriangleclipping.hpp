//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#if !defined(MLR_MLRCLIPTRICK_HPP)
#include<mlr/mlrcliptrick.hpp>
#endif

extern DWORD gEnableTextureSort, gShowClippedPolys, gEnableDetailTexture;
extern unsigned short *indexOffset;	// [MidLevelRenderer::Max_Number_Vertices_Per_Mesh]

#define HUNT_CLIP_ERROR 0

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	CLASSNAME::TransformNoClip(Matrix4D *mat, GOSVertexPool *vt, bool db)
{
	Check_Object(this);
	Check_Object(vt);
	Verify(index.GetLength() > 0);

	Start_Timer(Transform_Time);

//	unsigned short stride;
	int stride;

	bool textureAnimation = false;
	Scalar deltaU=0.0f, deltaV=0.0f;
	
	if(state.GetTextureHandle())
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[state.GetTextureHandle()];
		Check_Object(texture);

		textureAnimation = texture->GetAnimateTexture();
		if(textureAnimation)
		{
			Stuff::AffineMatrix4D &textureMatrix = texture->GetTextureMatrix();
			deltaU = textureMatrix(3, 0);
			deltaV = textureMatrix(3, 1);
		}
	}

	int i, j;

#ifdef I_SAY_YES_TO_TERRAIN
	Scalar terrainUV[2];
#endif	//	I_SAY_YES_TO_TERRAIN

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
	int m;
	gos_PushCurrentHeap(StaticHeap);
	int tex2count[Limits::Max_Number_Of_Multitextures];
	gos_PopCurrentHeap();

	for(m=0;m<currentNrOfPasses;m++)
	{
		tex2count[m] = 0;
	}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES

	int numVertices = GetNumVertices();
	gos_vertices = vt->GetActualVertexPool(db);

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	int tex2count = 0;
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES

#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
	Scalar detailTexCoords[2];
#endif	//	I_SAY_YES_TO_DETAIL_TEXTURES

#if defined(I_SAY_YES_TO_DUAL_TEXTURES) || defined(I_SAY_YES_TO_DETAIL_TEXTURES)
	gos_vertices2uv = vt->GetActualVertexPool2UV(db);
#endif

	numGOSVertices = 0;

	Verify(index.GetLength() > 0);
	Verify(coords.GetLength() == visibleIndexedVertices.GetLength());

	if(visibleIndexedVerticesKey == false)
	{
		FindVisibleVertices();
	}

#ifdef _ARMOR
	memset(indexOffset, 0xff, Limits::Max_Number_Vertices_Per_Mesh*sizeof(unsigned short));
#endif	//	_ARMOR

	for(j=0,stride=0;j<numVertices;j++)
	{
		if(visibleIndexedVertices[j] == 0)
		{
			stride++;
		}
		else
		{
#ifdef LAB_ONLY
			Set_Statistic(TransformedVertices, TransformedVertices+1);

			if(db==false)
			{
				Verify (vt->GetLast() + numGOSVertices < Limits::Max_Number_Vertices_Per_Frame);
			}
			else
			{
				Verify (numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
			}
#endif	//	LAB_ONLY

			*(int *)(&indexOffset[j]) = (j - stride);

#ifdef I_SAY_YES_TO_TERRAIN
			terrainUV[0] = borderPixelFun + (coords[j].x - minX)*xUVFac;
			terrainUV[1] = borderPixelFun + (coords[j].z - minZ)*zUVFac;
#endif	//	I_SAY_YES_TO_TERRAIN

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
			if(MLRState::GetMultitextureLightMap() == true && state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode)
			{
				gos_vertices2uv[numGOSVertices].GOSTransformNoClip(
					coords[j],
					*mat,
					&texCoords[j][0],
					&texCoords[numVertices + j][0]
	#if FOG_HACK
					,state.GetFogMode()
	#endif	//	FOG_HACK
				);
	#ifdef I_SAY_YES_TO_COLOR
		#if COLOR_AS_DWORD
				gos_vertices2uv[numGOSVertices].argb = (*actualColors)[j];
		#else	//	COLOR_AS_DWORD
				gos_vertices2uv[numGOSVertices].argb = GOSCopyColor(&(*actualColors)[j]);
		#endif	//	COLOR_AS_DWORD
	#else	//	I_SAY_YES_TO_COLOR
				gos_vertices2uv[numGOSVertices].argb = 0xffffffff;
	#endif	//	I_SAY_YES_TO_COLOR
			}
			else
			{
				gos_vertices[numGOSVertices].GOSTransformNoClip(
					coords[j],
					*mat,
					&texCoords[j][0]
	#if FOG_HACK
					,state.GetFogMode()
	#endif	//	FOG_HACK
				);
				(*texCoords2)[tex2count++] = texCoords[numVertices + j];

			
			}
#else //	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
			if(	MLRState::GetMultitextureLightMap() == true &&
				state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode &&
				gEnableDetailTexture==1
			)
			{
		#ifdef I_SAY_YES_TO_TERRAIN2
				detailTexCoords[0] = texCoords[j][0]*xScale;
				detailTexCoords[1] = texCoords[j][1]*yScale;
		#else
				detailTexCoords[0] = texCoords[j][0]*xScale + xOffset;
				detailTexCoords[1] = texCoords[j][1]*yScale + yOffset;
		#endif
				Verify(	MLRState::GetHasMaxUVs() ? (detailTexCoords[0]>=-MLRState::GetMaxUV() && detailTexCoords[0]<=MLRState::GetMaxUV()) : 1);
				Verify(	MLRState::GetHasMaxUVs() ? (detailTexCoords[1]>=-MLRState::GetMaxUV() && detailTexCoords[1]<=MLRState::GetMaxUV()) : 1);

				gos_vertices2uv[numGOSVertices].GOSTransformNoClip(
					coords[j],
					*mat,
					&texCoords[j][0],
					&detailTexCoords[0]
		#if FOG_HACK
					,state.GetFogMode()
		#endif	//	FOG_HACK
				);
		#ifdef I_SAY_YES_TO_COLOR
			#if COLOR_AS_DWORD
				gos_vertices2uv[numGOSVertices].argb = (*actualColors)[j];
			#else	//	COLOR_AS_DWORD
				gos_vertices2uv[numGOSVertices].argb = GOSCopyColor(&(*actualColors)[j]);
			#endif	//	COLOR_AS_DWORD
		#else	//	I_SAY_YES_TO_COLOR
				gos_vertices2uv[numGOSVertices].argb = 0xffffffff;
		#endif	//	I_SAY_YES_TO_COLOR

				if(gos_vertices[j].rhw < fadeDetailEnd)
				{
					gos_vertices[j].argb &= 0x00ffffff;
				}
				else
				{
					if(gos_vertices[j].rhw<fadeDetailStart)
					{
						gos_vertices[j].argb &= ((Stuff::Truncate_Float_To_Word((gos_vertices[j].rhw - fadeDetailEnd)*fadeMultiplicator)) << 24) | 0x00ffffff;
					}
				}
			}
			else
	#endif //	I_SAY_YES_TO_DETAIL_TEXTURES
			{
				gos_vertices[numGOSVertices].GOSTransformNoClip(
					coords[j],
					*mat,
	#ifdef I_SAY_YES_TO_TERRAIN
					terrainUV
	#else	//	I_SAY_YES_TO_TERRAIN
					&texCoords[j][0]
	#endif	//	I_SAY_YES_TO_TERRAIN
	#if FOG_HACK
					, state.GetFogMode()
	#endif	//	FOG_HACK
				);

	#ifdef I_SAY_YES_TO_COLOR
		#if COLOR_AS_DWORD
				gos_vertices[numGOSVertices].argb = (*actualColors)[j];
		#else	//	COLOR_AS_DWORD
				gos_vertices[numGOSVertices].argb = GOSCopyColor(&(*actualColors)[j]);
		#endif	//	COLOR_AS_DWORD
	#else	//	I_SAY_YES_TO_COLOR
				gos_vertices[numGOSVertices].argb = 0xffffffff;
	#endif	//	I_SAY_YES_TO_COLOR
			}
#endif //	I_SAY_YES_TO_DUAL_TEXTURES

			if(textureAnimation)
			{
				gos_vertices[numGOSVertices].u += deltaU;
				gos_vertices[numGOSVertices].v += deltaV;
			}

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
			for(m=1;m<currentNrOfPasses;m++)
			{
				extraMultiTexCoords[m][tex2count[m]++] = multiTexCoordsPointers[m][j];
			}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
			numGOSVertices++;
		}
	}

#ifdef LAB_ONLY
	Set_Statistic(NonClippedVertices, NonClippedVertices+numGOSVertices);
#endif	//	LAB_ONLY

	Check_Object(vt);

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	if(MLRState::GetMultitextureLightMap() == false || state.GetMultiTextureMode()==MLRState::MultiTextureOffMode)
	{
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
			Verify(texture);

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
	}
	else
	{
		if(db==false)
		{
			vt->Increase2UV(numGOSVertices);
		}
	}
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
		if(	gEnableDetailTexture==1)
		{			
			if(MLRState::GetMultitextureLightMap() == false || state.GetMultiTextureMode()==MLRState::MultiTextureOffMode )
			{
				if(db==false)
				{
					Verify (vt->GetLast() + 2*numGOSVertices < vt->GetLength());
				}
				else
				{
					Verify (2*numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh );
				}

				memcpy(gos_vertices + numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

				stride = 0;
				for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
				{
			#ifdef I_SAY_YES_TO_TERRAIN2
					gos_vertices[j].u = gos_vertices[i].u*xScale;
					gos_vertices[j].v = gos_vertices[i].v*yScale;
			#else
					gos_vertices[j].u = gos_vertices[i].u*xScale + xOffset;
					gos_vertices[j].v = gos_vertices[i].v*yScale + yOffset;
			#endif
					Verify(	MLRState::GetHasMaxUVs() ? (gos_vertices[j].u>=-MLRState::GetMaxUV() && gos_vertices[j].u<=MLRState::GetMaxUV()) : 1);
					Verify(	MLRState::GetHasMaxUVs() ? (gos_vertices[j].v>=-MLRState::GetMaxUV() && gos_vertices[j].v<=MLRState::GetMaxUV()) : 1);

					if(gos_vertices[j].rhw < fadeDetailEnd)
					{
						gos_vertices[j].argb &= 0x00ffffff;
					}
					else
					{
						stride++;
						if(gos_vertices[j].rhw<fadeDetailStart)
						{
							gos_vertices[j].argb &= ((Stuff::Truncate_Float_To_Word((gos_vertices[j].rhw - fadeDetailEnd)*fadeMultiplicator)) << 24) | 0x00ffffff;
						}
					}
				}

				if(stride==0)
				{
					detTextureVisible=false;
				}
				else
				{
					detTextureVisible=true;
				}

				if(db==false)
				{
					vt->Increase(2*numGOSVertices);
				}
			}
			else
			{
				if(db==false)
				{
					vt->Increase2UV(numGOSVertices);
				}
			}
		}
		else
		{
			if(db==false)
			{
				vt->Increase(numGOSVertices);
			}
		}
	#else	//	I_SAY_YES_TO_DETAIL_TEXTURES
		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	#endif	//	I_SAY_YES_TO_DETAIL_TEXTURES
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES

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
			gos_vertices[j].u = extraMultiTexCoords[m][i][0];
			gos_vertices[j].v = extraMultiTexCoords[m][i][1];
		}

		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES

	gos_indices = vt->GetActualIndexPool(db);
	numGOSIndices = 0;

	int ngi = 0;
	for(i=0,j=0;i<numOfTriangles;j+=3,++i)
	{
		if(testList[i] == 0)
		{
			continue;
		}

		if(db==false)
		{
			Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());
		}
		else
		{
			Verify(3 + numGOSIndices < 2*Limits::Max_Number_Vertices_Per_Mesh);
		}

		Verify(indexOffset[index[j]] < numGOSVertices);
		gos_indices[ngi++] = indexOffset[index[j]];
		Verify(indexOffset[index[j+2]] < numGOSVertices);
		gos_indices[ngi++] = indexOffset[index[j+2]];
		Verify(indexOffset[index[j+1]] < numGOSVertices);
		gos_indices[ngi++] = indexOffset[index[j+1]];
	}
	numGOSIndices = (unsigned short)ngi;

	Check_Object(vt);
	if(db==false)
	{
		vt->IncreaseIndex(numGOSIndices);
	}

	visible = numGOSVertices ? (unsigned char)1 : (unsigned char)0;

	Stop_Timer(Transform_Time);
}

static MLRClippingState theAnd, theOr, theTest;

//	extern void _stdcall CheckVertices( gos_VERTEX* pVertexArray, DWORD NumberVertices, bool PointsLines );
//	extern void _stdcall CheckVertices1( gos_VERTEX_2UV* pVertexArray, DWORD NumberVertices );

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Now it gets serious
int
	CLASSNAME::TransformAndClip(Matrix4D *mat, MLRClippingState clippingFlags, GOSVertexPool *vt, bool db)
{
	Check_Object(this);

	Start_Timer(Transform_Time);

	Verify(numOfTriangles == testList.GetLength());
	Verify(clippingFlags.GetClippingState() != 0);

	//
	//--------------------------------------
	// See if we don't have to draw anything
	//--------------------------------------
	//
	if(numOfTriangles <= 0)
	{
		visible = 0;

		Stop_Timer(Transform_Time);

		return visible;
	}

	Verify(index.GetLength() > 0);
	unsigned short l;

	int i, j, k, ret = 0;

	int len = coords.GetLength();

	if(visibleIndexedVerticesKey == false)
	{
		FindVisibleVertices();
	}

	Stuff::Vector4D *v4d = transformedCoords->GetData();
	Stuff::Point3D *p3d = coords.GetData();
	int *cs = (int *)clipPerVertex->GetData();
	unsigned char *viv = visibleIndexedVertices.GetData();

	for(i=0;i<len;i++,p3d++,v4d++,cs++,viv++)
	{
		if(*viv == 0)
		{
			continue;
		}
		*viv = 0;

		v4d->MultiplySetClip(*p3d, *mat, cs);
#ifdef LAB_ONLY
		Set_Statistic(TransformedVertices, TransformedVertices+1);
#endif	//	LAB_ONLY

//
//--------------------------------------------------------
// I claims all vertices are in. lets check it. who knows
//--------------------------------------------------------
//
#ifdef LAB_ONLY
		if( (*cs)==0)
		{
#if defined(_ARMOR)
			if(ArmorLevel > 3)
			{
//
//--------------------------------------------------------
// I claims all vertices are in. lets check it. who knows
//--------------------------------------------------------
//
				Verify(v4d->x >= 0.0f && v4d->x <= v4d->w );
				Verify(v4d->y >= 0.0f && v4d->y <= v4d->w );
				Verify(v4d->z >= 0.0f && v4d->z <= v4d->w );
			}
#endif	//	_ARMOR
			Set_Statistic(NonClippedVertices, NonClippedVertices+1);
		}
		else
		{
			Set_Statistic(ClippedVertices, ClippedVertices+1);
		}
#endif	//	LAB_ONLY
	}

	Stop_Timer(Transform_Time);
	Start_Timer(Clipping_Time);

	int mask, k0, k1, ct=0;
	Scalar a=0.0f;

	bool textureAnimation = false;
	Scalar deltaU=0.0f, deltaV=0.0f;
	
	if(state.GetTextureHandle())
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[state.GetTextureHandle()];
		Check_Object(texture);

		textureAnimation = texture->GetAnimateTexture();
		if(textureAnimation)
		{
			Stuff::AffineMatrix4D &textureMatrix = texture->GetTextureMatrix();
			deltaU = textureMatrix(3, 0);
			deltaV = textureMatrix(3, 1);
		}
	}

#if defined(I_SAY_YES_TO_DUAL_TEXTURES) || defined(I_SAY_YES_TO_DETAIL_TEXTURES)
	gos_vertices2uv = vt->GetActualVertexPool2UV(db);
	bool textureAnimation2 = false;
	Scalar deltaU2=0.0f, deltaV2=0.0f;

	if(state2.GetTextureHandle())
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[state2.GetTextureHandle()];
		Verify(texture);

		textureAnimation2 = texture->GetAnimateTexture();
		if(textureAnimation2)
		{
			Stuff::AffineMatrix4D &textureMatrix = texture->GetTextureMatrix();
			deltaU2 = textureMatrix(3, 0);
			deltaV2 = textureMatrix(3, 1);
		}
	}

#endif	//	I_SAY_YES_TO_DUAL_TEXTURES || I_SAY_YES_TO_DETAIL_TEXTURES

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	int tex2count = 0;
	int numVertices = GetNumVertices();
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
	int m;
	gos_PushCurrentHeap(StaticHeap);
	int tex2count[Limits::Max_Number_Of_Multitextures];
	gos_PopCurrentHeap();

	for(m=0;m<currentNrOfPasses;m++)
	{
		tex2count[m] = 0;
	}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES

	int	myNumberUsedClipVertex, myNumberUsedClipIndex, myNumberUsedClipLength;

	myNumberUsedClipVertex = 0;
	myNumberUsedClipIndex = 0;
	myNumberUsedClipLength = 0;

	Verify(index.GetLength() > 0);
	
	//
	//------------------------
	// Handle the indexed case
	//------------------------
	//

	//
	//-----------------------------------------------------------------
	// Step through each polygon, making sure that we don't try to clip
	// backfaced polygons
	//-----------------------------------------------------------------
	//
	cs = (int *)clipPerVertex->GetData();
	int index0, index1, index2;

	for(i=0,j=0;i<numOfTriangles;j+=3,++i)
	{
//		if(testList[i] == 0)
//		{
//			continue;
//		}

		index0 = index[j];
		index1 = index[j+1];
		index2 = index[j+2];
		//
		//---------------------------------------------------------------
		// Test each vertex of the polygon against the allowed clipping
		// planes, and accumulate status for which planes always clip and
		// which planes clipped at least once
		//---------------------------------------------------------------
		//
		theAnd = cs[index0];
		theOr = theAnd;

		theAnd &= (cs[index1] & cs[index2]);
		theOr |= (cs[index1] | cs[index2]);

		//
		//-------------------------------------------------------------------
		// If any bit is set for all vertices, then the polygon is completely
		// outside the viewing space and we don't have to draw it.  On the
		// other hand, if no bits at all were ever set, we can do a trivial
		// accept of the polygon
		//-------------------------------------------------------------------
		//
		theAnd = theOr = 0;		//ASSUME NO CLIPPING NEEDED FOR MC2.  Its just not done here!

		if (theAnd != 0)
		{
			testList[i] = 0;
#ifdef LAB_ONLY
			Set_Statistic(PolysClippedButOutside, PolysClippedButOutside+1);
#endif	//	LAB_ONLY
		}
		else if (theOr == 0)
		{
			ret++;

			visibleIndexedVertices[index0] = 1;
			visibleIndexedVertices[index1] = 1;
			visibleIndexedVertices[index2] = 1;

#ifdef I_SAY_YES_TO_TERRAIN
			(*clipTexCoords)[index0][0] = borderPixelFun + (coords[index0].x - minX)*xUVFac;
			(*clipTexCoords)[index0][1] = borderPixelFun + (coords[index0].z - minZ)*zUVFac;

			(*clipTexCoords)[index1][0] = borderPixelFun + (coords[index1].x - minX)*xUVFac;
			(*clipTexCoords)[index1][1] = borderPixelFun + (coords[index1].z - minZ)*zUVFac;

			(*clipTexCoords)[index2][0] = borderPixelFun + (coords[index2].x - minX)*xUVFac;
			(*clipTexCoords)[index2][1] = borderPixelFun + (coords[index2].z - minZ)*zUVFac;
#endif	//	I_SAY_YES_TO_TERRAIN
			
#ifdef LAB_ONLY
			Set_Statistic(PolysClippedButInside, PolysClippedButInside+1);
#endif	//	LAB_ONLY
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

#ifdef I_SAY_YES_TO_TERRAIN
			(*clipTexCoords)[index0][0] = borderPixelFun + (coords[index0].x - minX)*xUVFac;
			(*clipTexCoords)[index0][1] = borderPixelFun + (coords[index0].z - minZ)*zUVFac;

			(*clipTexCoords)[index1][0] = borderPixelFun + (coords[index1].x - minX)*xUVFac;
			(*clipTexCoords)[index1][1] = borderPixelFun + (coords[index1].z - minZ)*zUVFac;

			(*clipTexCoords)[index2][0] = borderPixelFun + (coords[index2].x - minX)*xUVFac;
			(*clipTexCoords)[index2][1] = borderPixelFun + (coords[index2].z - minZ)*zUVFac;
#endif	//	I_SAY_YES_TO_TERRAIN
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
#endif	//	LAB_ONLY
				for(k=j;k<j+3;k++)
				{
					k0 = index[k];
					k1 = index[(k+1) < j+3 ? k+1 : j];

					//
					//----------------------------------------------------
					// If this vertex is inside the viewing space, copy it
					// directly to the clipping buffer
					//----------------------------------------------------
					//
					int clipped_index =
						myNumberUsedClipVertex + numberVerticesPerPolygon;
					theTest = cs[k0];

					if(theTest == 0)
					{
						firstIsIn = true;
						(*clipExtraCoords)[clipped_index] = (*transformedCoords)[k0];

#ifdef I_SAY_YES_TO_COLOR
					Verify((*actualColors).GetLength() > 0);
					(*clipExtraColors)[clipped_index] = (*actualColors)[k0];
#endif	//	I_SAY_YES_TO_COLOR

#ifdef I_SAY_YES_TO_TERRAIN
						(*clipExtraTexCoords)[clipped_index] = (*clipTexCoords)[k0];
#else	//	I_SAY_YES_TO_TERRAIN
						Verify(texCoords.GetLength() > 0);
						(*clipExtraTexCoords)[clipped_index] = texCoords[k0];
#endif	// I_SAY_YES_TO_TERRAIN

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
						(*clipExtraTexCoords2)[clipped_index] = texCoords[k0+numVertices];
#endif						

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
						for(m=1;m<currentNrOfPasses;m++)
						{
							clipExtraMultiTexCoords[m][clipped_index] = multiTexCoordsPointers[m][k0];
						}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES				
						numberVerticesPerPolygon++;
						clipped_index++;

						//
						//-------------------------------------------------------
						// We don't need to clip this edge if the next vertex is
						// also in the viewing space, so just move on to the next
						// vertex
						//-------------------------------------------------------
						//
						if(cs[k1] == 0)
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
						if(cs[k1] != 0)
						{
							Verify(cs[k1] == cs[k0]);
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
					theTest |= cs[k1];

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
						Verify((*actualColors).GetLength() > 0);
	#if COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index] = Color_DWORD_Lerp (
							(*actualColors)[k0],
							(*actualColors)[k1],
							a
						);
	#else	//	COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index].Lerp(
							(*actualColors)[k0],
							(*actualColors)[k1],
							a
						);
	#endif	//	COLOR_AS_DWORD
#endif	//	I_SAY_YES_TO_COLOR
						//
						//-----------------------------------------------------
						// If there are texture uv's, we need to lerp them in a
						// perspective correct manner
						//-----------------------------------------------------
						//
#ifdef I_SAY_YES_TO_TERRAIN
						(*clipExtraTexCoords)[clipped_index].Lerp
							(
								(*clipTexCoords)[k0],
								(*clipTexCoords)[k1],
								a
							);
#else	//	I_SAY_YES_TO_TERRAIN
						Verify(texCoords.GetLength() > 0);
						(*clipExtraTexCoords)[clipped_index].Lerp
							(
								texCoords[k0],
								texCoords[k1],
								a
							);
#endif	//	I_SAY_YES_TO_TERRAIN

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
						(*clipExtraTexCoords2)[clipped_index].Lerp
							(
								texCoords[k0+numVertices],
								texCoords[k1+numVertices],
								a
							);
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES

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

#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
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
						Verify((*actualColors).GetLength() > 0);
		#if COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index] = Color_DWORD_Lerp (
							(*actualColors)[k1],
							(*actualColors)[k0],
							a
						);
		#else	//	COLOR_AS_DWORD
						(*clipExtraColors)[clipped_index].Lerp(
							(*actualColors)[k1],
							(*actualColors)[k0],
							a
						);
		#endif	//	COLOR_AS_DWORD
#endif	//	I_SAY_YES_TO_COLOR
						//
						//-----------------------------------------------------
						// If there are texture uv's, we need to lerp them in a
						// perspective correct manner
						//-----------------------------------------------------
						//
#ifdef I_SAY_YES_TO_TERRAIN
						(*clipExtraTexCoords)[clipped_index].Lerp
							(
								(*clipTexCoords)[k1],
								(*clipTexCoords)[k0],
								a
							);
#else	//	I_SAY_YES_TO_TERRAIN
						Verify(texCoords.GetLength() > 0);
						(*clipExtraTexCoords)[clipped_index].Lerp
							(
								texCoords[k1],
								texCoords[k0],
								a
							);
#endif	//	I_SAY_YES_TO_TERRAIN

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
						(*clipExtraTexCoords2)[clipped_index].Lerp
							(
								texCoords[k1+numVertices],
								texCoords[k0+numVertices],
								a
							);
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES

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

#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
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
#endif	//	_ARMOR
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
#endif	//	LAB_ONLY
				ClipData2 srcPolygon, dstPolygon;
				int dstBuffer = 1;

#ifndef I_SAY_YES_TO_TERRAIN
				Verify(texCoords.GetLength() > 0);
#endif	//	I_SAY_YES_TO_TERRAIN

				srcPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
#ifdef I_SAY_YES_TO_COLOR
				Verify((*actualColors).GetLength() > 0);
				srcPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
#endif	//	I_SAY_YES_TO_COLOR
				srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
				srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();

				//
				//----------------------------------------------------------
				// unravel and copy the original data into the source buffer
				//----------------------------------------------------------
				//
				for(k=j,l=0;k<j+3;k++,l++)
				{
					int indexK = index[k];

					srcPolygon.coords[l] = (*transformedCoords)[indexK];

#ifdef I_SAY_YES_TO_COLOR
					srcPolygon.colors[l] = (*actualColors)[indexK];
#endif	//	I_SAY_YES_TO_COLOR

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
					srcPolygon.texCoords[2*l] = texCoords[indexK];
					srcPolygon.texCoords[2*l+1] = texCoords[indexK + numVertices];
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_MULTI_TEXTURES
					for(m=0;m<currentNrOfPasses;m++)
					{
						srcPolygon.texCoords[currentNrOfPasses*l+m] = multiTexCoordsPointers[m][indexK];
					}
	#else	//	I_SAY_YES_TO_MULTI_TEXTURES

		#ifdef I_SAY_YES_TO_TERRAIN
					srcPolygon.texCoords[l] = (*clipTexCoords)[indexK];
		#else	//	I_SAY_YES_TO_TERRAIN
					srcPolygon.texCoords[l] = texCoords[indexK];
		#endif	//	I_SAY_YES_TO_TERRAIN

	#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES
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
#endif	//	I_SAY_YES_TO_COLOR
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

				DEBUG_STREAM << "TheOriginalOR: " << hex << theOr.GetClippingState() << dec << '\n';
#endif	//	HUNT_CLIP_ERROR

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
#endif	//	HUNT_CLIP_ERROR
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
#endif	//	HUNT_CLIP_ERROR


									dstPolygon.clipPerVertex[dstPolygon.length] = 
										srcPolygon.clipPerVertex[k];
#ifdef I_SAY_YES_TO_COLOR
									dstPolygon.colors[dstPolygon.length] = 
										srcPolygon.colors[k];
#endif	//	I_SAY_YES_TO_COLOR

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
									dstPolygon.texCoords[2*dstPolygon.length] = 
										srcPolygon.texCoords[2*k];

									dstPolygon.texCoords[2*dstPolygon.length+1] = 
										srcPolygon.texCoords[2*k+1];
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_MULTI_TEXTURES
									for(m=0;m<currentNrOfPasses;m++)
									{
										dstPolygon.texCoords[currentNrOfPasses*dstPolygon.length+m] = 
											srcPolygon.texCoords[currentNrOfPasses*k+m];
									}
	#else	//	I_SAY_YES_TO_MULTI_TEXTURES
									dstPolygon.texCoords[dstPolygon.length] = 
										srcPolygon.texCoords[k];
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES
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
#endif	//	HUNT_CLIP_ERROR
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
	#else	//	COLOR_AS_DWORD
									dstPolygon.colors[dstPolygon.length].Lerp(
										srcPolygon.colors[k],
										srcPolygon.colors[k1],
										a
									);
	#endif	//	COLOR_AS_DWORD
#endif	//	I_SAY_YES_TO_COLOR
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
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
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
	#else	//	I_SAY_YES_TO_MULTI_TEXTURES
									dstPolygon.texCoords[dstPolygon.length].Lerp
										(
											srcPolygon.texCoords[k],
											srcPolygon.texCoords[k1],
											a
										);
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES
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
#endif	//	HUNT_CLIP_ERROR

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
	#else	//	COLOR_AS_DWORD
								dstPolygon.colors[dstPolygon.length].Lerp(
									srcPolygon.colors[k1],
									srcPolygon.colors[k],
									a
								);
	#endif	//	COLOR_AS_DWORD
#endif	//	I_SAY_YES_TO_COLOR
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
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
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
	#else	//	I_SAY_YES_TO_MULTI_TEXTURES
								dstPolygon.texCoords[dstPolygon.length].Lerp
									(
										srcPolygon.texCoords[k1],
										srcPolygon.texCoords[k],
										a
									);
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES
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
#endif	//	I_SAY_YES_TO_COLOR
							srcPolygon.texCoords = clipBuffer[dstBuffer].texCoords.GetData();
							srcPolygon.clipPerVertex = clipBuffer[dstBuffer].clipPerVertex.GetData();	
							srcPolygon.length = dstPolygon.length;

							dstBuffer = !dstBuffer;

							dstPolygon.coords = clipBuffer[dstBuffer].coords.GetData();
#ifdef I_SAY_YES_TO_COLOR
							dstPolygon.colors = clipBuffer[dstBuffer].colors.GetData();
#endif	//	I_SAY_YES_TO_COLOR
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
#endif	//	HUNT_CLIP_ERROR

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
#if HUNT_CLIP_ERROR
				DEBUG_STREAM << "Final: " << srcPolygon.length << '\n';
#endif	//	HUNT_CLIP_ERROR
				for(k=0;k<srcPolygon.length;k++)
				{
					int clipped_index = myNumberUsedClipVertex + k;
#if HUNT_CLIP_ERROR
					DEBUG_STREAM << setiosflags( ios::scientific) << setprecision(20) 
						<< srcPolygon.coords[k].x << " "
						<< srcPolygon.coords[k].y << " "
						<< srcPolygon.coords[k].z << " "
						<< srcPolygon.coords[k].w << '\n';
#endif	//	HUNT_CLIP_ERROR

					(*clipExtraCoords)[clipped_index] = srcPolygon.coords[k];

#ifdef I_SAY_YES_TO_COLOR
					(*clipExtraColors)[clipped_index] = srcPolygon.colors[k];
#endif	//	I_SAY_YES_TO_COLOR

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
					(*clipExtraTexCoords)[clipped_index] = srcPolygon.texCoords[2*k];
					(*clipExtraTexCoords2)[clipped_index] = srcPolygon.texCoords[2*k+1];
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_MULTI_TEXTURES
					for(m=0;m<currentNrOfPasses;m++)
					{
						(*clipExtraMultiTexCoords)[m][clipped_index] = srcPolygon.texCoords[currentNrOfPasses*k+m];
					}
	#else	//	I_SAY_YES_TO_MULTI_TEXTURES
					(*clipExtraTexCoords)[clipped_index] = srcPolygon.texCoords[k];
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES
				}

				numberVerticesPerPolygon = srcPolygon.length;
#if HUNT_CLIP_ERROR
				DEBUG_STREAM << "---" << '\n';
#endif	//	HUNT_CLIP_ERROR
			
				(*clipExtraLength)[myNumberUsedClipLength] = numberVerticesPerPolygon;
#ifdef _ARMOR
				(*clipExtraLength)[myNumberUsedClipLength] |= 0x8000;
#endif	//	_ARMOR
			}
			myNumberUsedClipVertex += numberVerticesPerPolygon;
			myNumberUsedClipLength++;
			ret++;

//					clip

//					dont draw the original
			testList[i] = 0;
		}
	}

	Check_Object(vt);
	gos_vertices = vt->GetActualVertexPool(db);
	numGOSVertices = 0;
	gos_indices = vt->GetActualIndexPool(db);
	numGOSIndices = 0;

	k = visibleIndexedVertices.GetLength();

	unsigned short strideIndex;
	for(j=0,strideIndex=0;j<k;j++)
	{
		if(visibleIndexedVertices[j] == 0)
		{
			strideIndex++;
		}
		else
		{
			indexOffset[j] = static_cast<unsigned short>(j-strideIndex);

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
			if(MLRState::GetMultitextureLightMap() == true && state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode)
			{
				GOSCopyData(
					&gos_vertices2uv[numGOSVertices],
					transformedCoords->GetData(),
	#ifdef I_SAY_YES_TO_COLOR
					actualColors->GetData(),
	#endif	//	I_SAY_YES_TO_COLOR

	#ifdef I_SAY_YES_TO_TERRAIN
					clipTexCoords->GetData(),
	#else	//	I_SAY_YES_TO_TERRAIN
					texCoords.GetData(),
	#endif	//	I_SAY_YES_TO_TERRAIN
					texCoords.GetData() + numVertices,
					j
	#if FOG_HACK
					, state.GetFogMode()
	#endif	//	FOG_HACK

				);
//				CheckVertices1( gos_vertices2uv+numGOSVertices, 1 );
			}
			else
			{
				GOSCopyData(
					&gos_vertices[numGOSVertices],
					transformedCoords->GetData(),
	#ifdef I_SAY_YES_TO_COLOR
					actualColors->GetData(),
	#endif	//	I_SAY_YES_TO_COLOR

	#ifdef I_SAY_YES_TO_TERRAIN
					clipTexCoords->GetData(),
	#else	//	I_SAY_YES_TO_TERRAIN
					texCoords.GetData(),
	#endif	//	I_SAY_YES_TO_TERRAIN
					j
	#if FOG_HACK
					, state.GetFogMode()
	#endif	//	FOG_HACK

				);
				(*texCoords2)[tex2count++] = texCoords[numVertices + j];
			}
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
			if(	MLRState::GetMultitextureLightMap() == true &&
				state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode &&
				gEnableDetailTexture==1
			)
			{
				GOSCopyData(
					&gos_vertices2uv[numGOSVertices],
					transformedCoords->GetData(),
		#ifdef I_SAY_YES_TO_COLOR
					actualColors->GetData(),
		#endif	//	I_SAY_YES_TO_COLOR

		#ifdef I_SAY_YES_TO_TERRAIN
					clipTexCoords->GetData(),
		#else	//	I_SAY_YES_TO_TERRAIN
					texCoords.GetData(),
		#endif	//	I_SAY_YES_TO_TERRAIN
		#ifdef I_SAY_YES_TO_TERRAIN
					clipTexCoords->GetData(),
		#else	//	I_SAY_YES_TO_TERRAIN
					texCoords.GetData(),
		#endif	//	I_SAY_YES_TO_TERRAIN
					j
		#if FOG_HACK
					, state.GetFogMode()
		#endif	//	FOG_HACK
				);
			}
			else
	#endif	//	I_SAY_YES_TO_DETAIL_TEXTURES
			{
				GOSCopyData(
					&gos_vertices[numGOSVertices],
					transformedCoords->GetData(),
		#ifdef I_SAY_YES_TO_COLOR
					actualColors->GetData(),
		#endif	//	I_SAY_YES_TO_COLOR

		#ifdef I_SAY_YES_TO_TERRAIN
					clipTexCoords->GetData(),
		#else	//	I_SAY_YES_TO_TERRAIN
					texCoords.GetData(),
		#endif	//	I_SAY_YES_TO_TERRAIN
					j
		#if FOG_HACK
					, state.GetFogMode()
		#endif	//	FOG_HACK
				);
			}
#endif	//	I_SAY_YES_TO_DUAL_TEXTURES

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
#endif	//	LAB_ONLY

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
			for(m=1;m<currentNrOfPasses;m++)
			{
				extraMultiTexCoords[m][tex2count[m]++] = multiTexCoordsPointers[m][j];
			}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES
			numGOSVertices++;
		}
	}

	for(i=0,j=0;i<numOfTriangles;j+=3,++i)
	{
		if(testList[i] == 0)
		{
			continue;
		}

		if(db==false)
		{
			Verify((vt->GetLastIndex() + 3 + numGOSIndices) < vt->GetLength());
		}
		else
		{
			Verify(3 + numGOSIndices < 2*Limits::Max_Number_Vertices_Per_Mesh);
		}

		Verify(indexOffset[index[j]] < numGOSVertices);
		gos_indices[numGOSIndices] = indexOffset[index[j]];

		Verify(indexOffset[index[j+2]] < numGOSVertices);
		gos_indices[numGOSIndices+1] = indexOffset[index[j+2]];

		Verify(indexOffset[index[j+1]] < numGOSVertices);
		gos_indices[numGOSIndices+2] = indexOffset[index[j+1]];

		numGOSIndices += 3;
	}

	unsigned short stride;
	if(myNumberUsedClipLength > 0)
	{
		for(i=0,j=0;i<myNumberUsedClipLength;i++)
		{
#ifdef _ARMOR
			stride = static_cast<unsigned short>((*clipExtraLength)[i] & 0x7fff);
#else	//	_ARMOR
			stride = (*clipExtraLength)[i];
#endif	//	_ARMOR
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

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
				if(MLRState::GetMultitextureLightMap() == true && state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode)
				{
					GOSCopyTriangleData(
						&gos_vertices2uv[numGOSVertices],
						clipExtraCoords->GetData(),
	#ifdef I_SAY_YES_TO_COLOR
						clipExtraColors->GetData(),
	#endif	//	I_SAY_YES_TO_COLOR
						clipExtraTexCoords->GetData(),
						clipExtraTexCoords2->GetData(),
						j, j+k+1, j+k
	#if FOG_HACK
						, state.GetFogMode()
	#endif	//	FOG_HACK
					);
//					CheckVertices1( gos_vertices2uv+numGOSVertices, 3 );
				}
				else
				{
					GOSCopyTriangleData(
						&gos_vertices[numGOSVertices],
						clipExtraCoords->GetData(),
	#ifdef I_SAY_YES_TO_COLOR
						clipExtraColors->GetData(),
	#endif	//	I_SAY_YES_TO_COLOR
						clipExtraTexCoords->GetData(),
						j, j+k+1, j+k
	#if FOG_HACK
						, state.GetFogMode()
	#endif	//	FOG_HACK
					);

					(*texCoords2)[tex2count++] = (*clipExtraTexCoords2)[j];
					(*texCoords2)[tex2count++] = (*clipExtraTexCoords2)[j+k+1];
					(*texCoords2)[tex2count++] = (*clipExtraTexCoords2)[j+k];
				}
#else //	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
				if(	MLRState::GetMultitextureLightMap() == true &&
					state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode &&
					gEnableDetailTexture==1
				)
				{
					GOSCopyTriangleData(
						&gos_vertices2uv[numGOSVertices],
						clipExtraCoords->GetData(),
		#ifdef I_SAY_YES_TO_COLOR
						clipExtraColors->GetData(),
		#endif	//	I_SAY_YES_TO_COLOR
						clipExtraTexCoords->GetData(),
						clipExtraTexCoords->GetData(),
						j, j+k+1, j+k
		#if FOG_HACK
						, state.GetFogMode()
		#endif	//	FOG_HACK
					);
				}
				else
	#endif	//	I_SAY_YES_TO_DETAIL_TEXTURES
				{
					GOSCopyTriangleData(
						&gos_vertices[numGOSVertices],
						clipExtraCoords->GetData(),
		#ifdef I_SAY_YES_TO_COLOR
						clipExtraColors->GetData(),
		#endif	//	I_SAY_YES_TO_COLOR
						clipExtraTexCoords->GetData(),
						j, j+k+1, j+k
		#if FOG_HACK
						, state.GetFogMode()
		#endif	//	FOG_HACK
					);
				}
#endif //	I_SAY_YES_TO_DUAL_TEXTURES

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
#endif	//	LAB_ONLY

#ifdef I_SAY_YES_TO_MULTI_TEXTURES
				for(m=1;m<currentNrOfPasses;m++)
				{
					extraMultiTexCoords[m][tex2count[m]++] = clipExtraMultiTexCoords[m][j];
					extraMultiTexCoords[m][tex2count[m]++] = clipExtraMultiTexCoords[m][j+k+1];
					extraMultiTexCoords[m][tex2count[m]++] = clipExtraMultiTexCoords[m][j+k];
				}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES

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
#endif	//	HUNT_CLIP_ERROR

	}

#ifdef I_SAY_YES_TO_DUAL_TEXTURES
	if(MLRState::GetMultitextureLightMap() == false || state.GetMultiTextureMode()==MLRState::MultiTextureOffMode)
	{
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
	}
	else
	{
		if(db==false)
		{
			vt->Increase2UV(numGOSVertices);
		}
	}
#else	//	I_SAY_YES_TO_DUAL_TEXTURES
	#ifdef I_SAY_YES_TO_DETAIL_TEXTURES
		if(gEnableDetailTexture==1)
		{
			if(	MLRState::GetMultitextureLightMap() == true &&
				state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode 
			)
			{
				for(i=0;i<numGOSVertices;i++)
				{
			#ifdef I_SAY_YES_TO_TERRAIN2
					gos_vertices2uv[i].u2 = gos_vertices2uv[i].u1*xScale;
					gos_vertices2uv[i].v2 = gos_vertices2uv[i].v1*yScale;
			#else	//	I_SAY_YES_TO_TERRAIN2
					gos_vertices2uv[i].u2 = gos_vertices2uv[i].u1*xScale + xOffset + deltaU2;
					gos_vertices2uv[i].v2 = gos_vertices2uv[i].v1*yScale + yOffset + deltaV2;
			#endif	//	I_SAY_YES_TO_TERRAIN2
					Verify(	MLRState::GetHasMaxUVs() ? (gos_vertices2uv[i].u2>=-MLRState::GetMaxUV() && gos_vertices2uv[i].u2<=MLRState::GetMaxUV()) : 1);
					Verify(	MLRState::GetHasMaxUVs() ? (gos_vertices2uv[i].v2>=-MLRState::GetMaxUV() && gos_vertices2uv[i].v2<=MLRState::GetMaxUV()) : 1);

					if(gos_vertices2uv[i].rhw < fadeDetailEnd)
					{
						gos_vertices2uv[i].argb &= 0x00ffffff;
					}
					else
					{
						if(gos_vertices2uv[i].rhw<fadeDetailStart)
						{
							gos_vertices2uv[i].argb &= ((Stuff::Truncate_Float_To_Word((gos_vertices2uv[i].rhw - fadeDetailEnd)*fadeMultiplicator)) << 24) | 0x00ffffff;
						}
					}
				}

				if(db==false)
				{
					vt->Increase2UV(numGOSVertices);
				}
			}
			else
			{
				if(db==false)
				{
					Verify (vt->GetLast() + 2*numGOSVertices < vt->GetLength());
				}
				else
				{
					Verify (2*numGOSVertices < 2*Limits::Max_Number_Vertices_Per_Mesh);
				}

				memcpy(gos_vertices + numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

				stride = 0;
				for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
				{
			#ifdef I_SAY_YES_TO_TERRAIN2
					gos_vertices[j].u = gos_vertices[i].u*xScale;
					gos_vertices[j].v = gos_vertices[i].v*yScale;
			#else	//	I_SAY_YES_TO_TERRAIN2
					gos_vertices[j].u = gos_vertices[i].u*xScale + xOffset + deltaU2;
					gos_vertices[j].v = gos_vertices[i].v*yScale + yOffset + deltaV2;
			#endif	//	I_SAY_YES_TO_TERRAIN2
					Verify(	MLRState::GetHasMaxUVs() ? (gos_vertices[j].u>=-MLRState::GetMaxUV() && gos_vertices[j].u<=MLRState::GetMaxUV()) : 1);
					Verify(	MLRState::GetHasMaxUVs() ? (gos_vertices[j].v>=-MLRState::GetMaxUV() && gos_vertices[j].v<=MLRState::GetMaxUV()) : 1);

					if(gos_vertices[j].rhw < fadeDetailEnd)
					{
						gos_vertices[j].argb &= 0x00ffffff;
					}
					else
					{
						stride++;
						if(gos_vertices[j].rhw<fadeDetailStart)
						{
							gos_vertices[j].argb &= ((Stuff::Truncate_Float_To_Word((gos_vertices[j].rhw - fadeDetailEnd)*fadeMultiplicator)) << 24) | 0x00ffffff;
						}
					}
				}

				if(stride==0)
				{
					detTextureVisible=false;
				}
				else
				{
					detTextureVisible=true;
				}

				if(db==false)
				{
					vt->Increase(2*numGOSVertices);
				}
			}
		}
		else
		{
			if(db==false)
			{
				vt->Increase(numGOSVertices);
			}
		}
	#else	//	I_SAY_YES_TO_DETAIL_TEXTURES
		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	#endif	//	I_SAY_YES_TO_DETAIL_TEXTURES
#endif	//		//	I_SAY_YES_TO_DUAL_TEXTURES

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

		memcpy(gos_vertices + m*numGOSVertices, gos_vertices, numGOSVertices * sizeof(GOSVertex));

		for(i=0,j=numGOSVertices;i<numGOSVertices;i++,j++)
		{
			gos_vertices[j].u = extraMultiTexCoords[m][i][0];
			gos_vertices[j].v = extraMultiTexCoords[m][i][1];
		}

		if(db==false)
		{
			vt->Increase(numGOSVertices);
		}
	}
#endif	//	I_SAY_YES_TO_MULTI_TEXTURES

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


#if	0	//def	I_SAY_YES_TO_DUAL_TEXTURES
	if(MLRState::GetMultitextureLightMap() == true && state.GetMultiTextureMode()!=MLRState::MultiTextureOffMode)
	{
		CheckVertices1( gos_vertices2uv, numGOSVertices );
	}
	else
	{
		CheckVertices( gos_vertices, numGOSVertices, false );
	}
#endif	//		//	I_SAY_YES_TO_DUAL_TEXTURES


	Stop_Timer(Clipping_Time);

	return ret;
}
