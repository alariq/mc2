//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	BitTrace *MLR_I_PMesh_Clip;
#endif

#define UV_TEST 0

extern DWORD gEnableLightMaps;

//#############################################################################
//###### MLRIndexedPolyMesh with no color no lighting one texture layer  ######
//#############################################################################

MLR_I_PMesh::ClassData*
	MLR_I_PMesh::DefaultData = NULL;

DynamicArrayOf<Vector2DScalar> *lightMapUVs;
DynamicArrayOf<Scalar> *lightMapSqFalloffs;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLR_I_PMeshClassID,
			"MidLevelRenderer::MLR_I_PMesh",
			MLRIndexedPrimitiveBase::DefaultData,
			(MLRPrimitiveBase::Factory)&Make
		);
	Register_Object(DefaultData);

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		MLR_I_PMesh_Clip = new BitTrace("MLR_I_PMesh_Clip");
		Register_Object(MLR_I_PMesh_Clip);
	#endif

	lightMapUVs = new DynamicArrayOf<Vector2DScalar>(Limits::Max_Number_Vertices_Per_Polygon);
	Register_Object(lightMapUVs);

	lightMapSqFalloffs = new DynamicArrayOf<Scalar> (Limits::Max_Number_Vertices_Per_Polygon);
	Register_Object(lightMapSqFalloffs);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::TerminateClass()
{
	Unregister_Object(lightMapSqFalloffs);
	delete lightMapSqFalloffs;

	Unregister_Object(lightMapUVs);
	delete lightMapUVs;

	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;

	#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
		Unregister_Object(MLR_I_PMesh_Clip);
		delete MLR_I_PMesh_Clip;
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_PMesh::MLR_I_PMesh(
	ClassData *class_data,
	MemoryStream *stream,
	int version
):
	MLRIndexedPrimitiveBase(class_data, stream, version)
{
	Check_Pointer(this);
	Check_Pointer(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	facePlanes.SetLength(GetNumPrimitives());
	testList.SetLength(GetNumPrimitives());

	FindFacePlanes();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_PMesh::MLR_I_PMesh(ClassData *class_data):
	MLRIndexedPrimitiveBase(class_data)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);

	drawMode = SortData::TriIndexedList;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
void
	MLR_I_PMesh::Copy(MLRIndexedPolyMesh *polyMesh)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);

	int len;
	unsigned short *_index;
	Point3D *_coords;
	Vector2DScalar *_texCoords;
	unsigned char *_lengths;

	polyMesh->GetCoordData(&_coords, &len);
	SetCoordData(_coords, len);

	polyMesh->GetIndexData(&_index, &len);
	SetIndexData(_index, len);

	polyMesh->GetSubprimitiveLengths(&_lengths);
	SetSubprimitiveLengths(_lengths, polyMesh->GetNumPrimitives());

	facePlanes.SetLength(GetNumPrimitives());
	testList.SetLength(GetNumPrimitives());

	FindFacePlanes();

	polyMesh->GetTexCoordData(&_texCoords, &len);
	SetTexCoordData(_texCoords, len);

	referenceState = polyMesh->GetReferenceState();

	visibleIndexedVerticesKey = false;
	visibleIndexedVertices.SetLength(coords.GetLength());
}
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_PMesh::~MLR_I_PMesh()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLR_I_PMesh*
	MLR_I_PMesh::Make(
		MemoryStream *stream,
		int version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLR_I_PMesh *mesh = new MLR_I_PMesh(DefaultData, stream, version);
	gos_PopCurrentHeap();

	return mesh;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLRIndexedPrimitiveBase::Save(stream);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::InitializeDrawPrimitive(unsigned char vis, int parameter)
{
	MLRIndexedPrimitiveBase::InitializeDrawPrimitive(vis, parameter);

	if(parameter & 1)
	{
		ResetTestList();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::SetSubprimitiveLengths (unsigned char *data, int numPrimitives)
{
	Check_Object(this); 
	Verify(gos_GetCurrentHeap() == Heap);

	lengths.AssignData(data, numPrimitives);

	facePlanes.SetLength(numPrimitives);

	testList.SetLength(numPrimitives);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::GetSubprimitiveLengths (unsigned char **data, int *l)
{
	Check_Object(this);
	*l = lengths.GetLength();
	*data = lengths.GetData();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::FindFacePlanes()
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
	MLR_I_PMesh::FindBackFace(const Point3D& u)
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
			*iPtr = (p->GetDistanceTo(u) >= 0.0f) ? (unsigned char)0xff : (unsigned char)0;
			
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
	MLR_I_PMesh::ResetTestList()
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
	MLR_I_PMesh::FindVisibleVertices()
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

extern DWORD gEnableTextureSort, gEnableAlphaSort;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLR_I_PMesh::Transform(Matrix4D *mat)
{
	Check_Object(this);

	Start_Timer(Transform_Time);

	Verify(index.GetLength() > 0);
	int i, len = coords.GetLength();

	if(visibleIndexedVerticesKey == false)
	{
		FindVisibleVertices();
	}

	Stuff::Vector4D *v4d = transformedCoords->GetData();
	Stuff::Point3D *p3d = coords.GetData();
	MLRClippingState *cs = clipPerVertex->GetData();
	unsigned char *viv = visibleIndexedVertices.GetData();

	for(i=0;i<len;i++,p3d++,v4d++,cs++,viv++)
	{
		if(*viv == 0)
		{
			continue;
		}
		
		v4d->Multiply(*p3d, *mat);
#ifdef LAB_ONLY
		Set_Statistic(TransformedVertices, TransformedVertices+1);
#endif

		cs->Clip4dVertex(v4d);
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
#endif
			Set_Statistic(NonClippedVertices, NonClippedVertices+1);
		}
		else
		{
			Set_Statistic(ClippedVertices, ClippedVertices+1);
		}
#endif
	}
	Stop_Timer(Transform_Time);
}

#undef I_SAY_YES_TO_DUAL_TEXTURES
#undef I_SAY_YES_TO_COLOR
#undef I_SAY_YES_TO_LIGHTING

#define CLASSNAME MLR_I_PMesh

#if defined(TRACE_ENABLED) && defined(MLR_TRACE)
	#define SET_MLR_PMESH_CLIP() MLR_I_PMesh_Clip->Set()
	#define CLEAR_MLR_PMESH_CLIP() MLR_I_PMesh_Clip->Clear()
#else
	#define SET_MLR_PMESH_CLIP()
	#define CLEAR_MLR_PMESH_CLIP()
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	This include contains follwing functions:
//	void MLR_I_PMesh::TransformNoClip(Matrix4D*, GOSVertexPool*);
//	int MLR_I_PMesh::Clip(MLRClippingState, GOSVertexPool*);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include<mlr/mlrprimitiveclipping.hpp>

#undef CLASSNAME

//---------------------------------------------------------------------------
//
void
	MLR_I_PMesh::Lighting(MLRLight* const* lights, int nrLights)
{
	int i;
	MLRLightMap *lightMap;

	for(i=0;i<nrLights;i++)
	{
		lightMap = lights[i]->GetLightMap();

		if(lightMap!=NULL)
		{
			LightMapLighting(lights[i]);
		}
	}

}

RGBAColor errorColor;

//---------------------------------------------------------------------------
//
bool
	CheckForBigTriangles(DynamicArrayOf<Vector2DScalar> *lightMapUVs, int stride)
{
	int k1, k0;
	Scalar u1v0, u0v1;

	int up=0, down=0, right=0, left=0;

	for(k0=0;k0<stride;k0++)
	{
		if((*lightMapUVs)[k0][1] > 0.5f)
		{
			up++;
		}
		if((*lightMapUVs)[k0][1] < -0.5f)
		{
			down++;
		}
		if((*lightMapUVs)[k0][0] > 0.5f)
		{
			right++;
		}
		if((*lightMapUVs)[k0][0] < -0.5f)
		{
			left++;
		}
	}

	if(up==stride || down==stride || left==stride || right==stride)
	{
		errorColor = RGBAColor(0.5f, 0.5f, 0.0f, 1.0f);
		return false;
	}

#if 1
	Scalar sign = -1.0f;
	if( 
		((*lightMapUVs)[1][0]-(*lightMapUVs)[0][0]) * ((*lightMapUVs)[stride-1][1]-(*lightMapUVs)[0][1]) >
		((*lightMapUVs)[1][1]-(*lightMapUVs)[0][1]) * ((*lightMapUVs)[stride-1][0]-(*lightMapUVs)[0][0])
		)
	{
		sign = 1.0f;
	}
#endif

	for(k0=0;k0<stride;k0++)
	{
		k1 = (k0+1<stride) ? k0+1 : 0;

		u1v0 = 	(*lightMapUVs)[k1][0] * (*lightMapUVs)[k0][1];
		u0v1 = 	(*lightMapUVs)[k0][0] * (*lightMapUVs)[k1][1];

		if( sign*(u1v0 - u0v1) < 0.0f )
//		if( (u1v0 - u0v1) < 0.0f )
		{
			continue;
		}

		if(
			(
				((*lightMapUVs)[k1][0]*(*lightMapUVs)[k1][0] - 2.0f*(*lightMapUVs)[k1][0]*(*lightMapUVs)[k0][0] + (*lightMapUVs)[k0][0]*(*lightMapUVs)[k0][0]) +
				((*lightMapUVs)[k1][1]*(*lightMapUVs)[k1][1] - 2.0f*(*lightMapUVs)[k1][1]*(*lightMapUVs)[k0][1] + (*lightMapUVs)[k0][1]*(*lightMapUVs)[k0][1])
			) 	<	(2.0f*(u1v0*u1v0 - 2.0f*u1v0*u0v1 + u0v1*u0v1))
		)
		{
#if UV_TEST
			DEBUG_STREAM << k0 << endl;
			for(int i=0;i<stride;i++)
			{
				DEBUG_STREAM << (*lightMapUVs)[i][0] << " " << (*lightMapUVs)[i][1] << endl;
			}
			DEBUG_STREAM << endl;
#endif
			errorColor = RGBAColor(0.0f, 0.5f, 0.0f, 1.0f);
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------
//
void
	MLR_I_PMesh::LightMapLighting(MLRLight *light)
{
	if(!gEnableLightMaps)
	{
		return;
	}

	unsigned short stride;
	int i, j, k, len = lengths.GetLength();
	LinearMatrix4D matrix = LinearMatrix4D::Identity;
	Point3D lightPosInShape, hitPoint;
	UnitVector3D up, left, forward;
	bool lm;
	Scalar f, rhf, falloff = 1.0f, distance;

	MLRLightMap *lightMap = light->GetLightMap();

	if( (!lightMap)  || (!gEnableLightMaps) )
	{
		return;
	}

	switch(light->GetLightType())
	{
		case MLRLight::PointLight:
		{
			lightMap->AddState(referenceState.GetPriority()+1);
			
			light->GetInShapePosition(lightPosInShape);

			for(i=0,j=0,k=0;i<len;i++,j += stride)
			{
				stride = lengths[i];

				if(testList[i] == 0)
				{
					continue;
				}

				f = facePlanes[i].GetDistanceTo(lightPosInShape);
				
				lm = false;

				if(f>0.0f && Cast_Object(MLRInfiniteLightWithFalloff*, light)->GetFalloff(f, falloff) == true)
				{
					rhf = 1.0f/f;

					matrix = LinearMatrix4D::Identity;
					matrix.AlignLocalAxisToWorldVector(facePlanes[i].normal, Z_Axis, Y_Axis, X_Axis);
					matrix.GetWorldLeftInLocal(&left);
					matrix.GetWorldUpInLocal(&up);
					matrix.GetWorldForwardInLocal(&forward);
					Verify(Small_Enough(up*left));

		#if defined(_ARMOR)
					Scalar diff = forward*left;
					Verify(Small_Enough(diff));
					diff = up*forward;
					Verify(Small_Enough(diff));
		#endif

					Check_Object(&forward);
					hitPoint.Multiply(forward, -f);
					hitPoint+=lightPosInShape;

					for(k=0;k<stride;k++)
					{
						Vector3D vec(coords[index[k+j]]);
						vec-=hitPoint;

						(*lightMapUVs)[k][0] = -(left*vec)*rhf;
						(*lightMapUVs)[k][1] = -(up*vec)*rhf;

						if(
							(*lightMapUVs)[k][0] >= -0.5f && (*lightMapUVs)[k][0] <= 0.5f &&
							(*lightMapUVs)[k][1] >= -0.5f && (*lightMapUVs)[k][1] <= 0.5f

						) 
						{
							lm = true;
						}
					}
				}
				else
				{
					continue;
				}

				if(lm == true || CheckForBigTriangles(lightMapUVs, stride) == true)
				{
					lightMap->SetPolygonMarker(0);
					lightMap->AddUShort(stride);

					Scalar sq_falloff
						= falloff*falloff*light->GetIntensity();
					RGBAColor color(sq_falloff, sq_falloff, sq_falloff, 1.0f);

					lightMap->AddColor(color);
					for(k=0;k<stride;k++)
					{
						lightMap->AddCoord(coords[index[k+j]]);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddUVs((*lightMapUVs)[k][0]+0.5f, (*lightMapUVs)[k][1]+0.5f);
		//				DEBUG_STREAM << k << " " << lightMapUVs[k][0] << " " << lightMapUVs[k][0] << "\n";
					}
				}
			}
		}
		break;
		case MLRLight::SpotLight:
		{
			int behindCount = 0, falloffCount = 0;

			lightMap->AddState(referenceState.GetPriority()+1);
			
			light->GetInShapePosition(matrix);
			lightPosInShape = matrix;

			Scalar tanSpeadAngle = Cast_Object(MLRSpotLight*, light)->GetTanSpreadAngle();

			matrix.GetLocalLeftInWorld(&left);
			matrix.GetLocalUpInWorld(&up);
			matrix.GetLocalForwardInWorld(&forward);

			Verify(Small_Enough(up*left));

			for(i=0,j=0,k=0;i<len;i++,j += stride)
			{
				behindCount = 0;
				falloffCount = 0;
				stride = lengths[i];

				if(testList[i] == 0)
				{
					continue;
				}

				lm = false;

				if(!facePlanes[i].IsSeenBy(lightPosInShape))
				{
					continue;
				}

				int tooBig = 0;
				for(k=0;k<stride;k++)
				{
					Vector3D vec;
					vec.Subtract(coords[index[k+j]], lightPosInShape);

					distance = (vec*forward);

					if(distance > SMALL)
					{
						if(Cast_Object(MLRInfiniteLightWithFalloff*, light)->GetFalloff(distance, falloff) == false)
						{
							falloffCount++;
						}
						(*lightMapSqFalloffs)[k] = falloff*falloff*light->GetIntensity();
					}
					else
					{
						behindCount++;
						break;
					}

					(*lightMapUVs)[k][0] = (left*vec)/(2.0f*distance*tanSpeadAngle);
					(*lightMapUVs)[k][1] = -(up*vec)/(2.0f*distance*tanSpeadAngle);

					if(
						(*lightMapUVs)[k][0] >= -0.5f && (*lightMapUVs)[k][0] <= 0.5f &&
						(*lightMapUVs)[k][1] >= -0.5f && (*lightMapUVs)[k][1] <= 0.5f

					) 
					{
						lm = true;
					}
                    //sebi:  what was the idea? why no ()?
					if(
						(*lightMapUVs)[k][0] < -1.5f || (*lightMapUVs)[k][0] > 1.5f ||
						(*lightMapUVs)[k][1] < -1.5f && (*lightMapUVs)[k][1] > 1.5f

					) 
					{
						tooBig++;
					}
				}

#if 1
				if(	
					tooBig == 0 &&
					behindCount == 0 && 
					falloffCount < stride &&
					(lm == true || CheckForBigTriangles(lightMapUVs, stride) == true)
					)
				{
					lightMap->SetPolygonMarker(1);
					lightMap->AddUShort(stride);

					for(k=0;k<stride;k++)
					{
						lightMap->AddCoord(coords[index[k+j]]);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddColor((*lightMapSqFalloffs)[k], (*lightMapSqFalloffs)[k], (*lightMapSqFalloffs)[k], 1.0f);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddUVs((*lightMapUVs)[k][0]+0.5f, (*lightMapUVs)[k][1]+0.5f);
		//				DEBUG_STREAM << k << " " << lightMapUVs[k][0] << " " << lightMapUVs[k][0] << "\n";
					}
				}
#else
				if(tooBig != 0)
				{
					lightMap->SetPolygonMarker(1);
					lightMap->AddUShort(stride);

					for(k=0;k<stride;k++)
					{
						lightMap->AddCoord(coords[index[k+j]]);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddColor(RGBAColor(0.0f, 0.0f, 0.5f, 1.0f));
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddUVs(0.5f, 0.5f);
		//				DEBUG_STREAM << k << " " << lightMapUVs[k][0] << " " << lightMapUVs[k][0] << "\n";
					}
				} 
				else if(behindCount != 0)
				{
					lightMap->SetPolygonMarker(1);
					lightMap->AddUShort(stride);

					for(k=0;k<stride;k++)
					{
						lightMap->AddCoord(coords[index[k+j]]);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddColor(RGBAColor(0.5f, 0.0f, 0.0f, 1.0f));
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddUVs(0.5f, 0.5f);
		//				DEBUG_STREAM << k << " " << lightMapUVs[k][0] << " " << lightMapUVs[k][0] << "\n";
					}
				} 
				else	if(behindCount == 0 && (lm == true || CheckForBigTriangles(&lightMapUVs, stride) == true) )
				{
					lightMap->SetPolygonMarker(1);
					lightMap->AddUShort(stride);

					for(k=0;k<stride;k++)
					{
						lightMap->AddCoord(coords[index[k+j]]);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddColor(lightMapSqFalloffs[k], lightMapSqFalloffs[k], lightMapSqFalloffs[k], 1.0f);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddUVs(lightMapUVs[k][0]+0.5f, lightMapUVs[k][1]+0.5f);
		//				DEBUG_STREAM << k << " " << lightMapUVs[k][0] << " " << lightMapUVs[k][0] << "\n";
					}

				}
				else if(CheckForBigTriangles(&lightMapUVs, stride) == false)
				{
					lightMap->SetPolygonMarker(1);
					lightMap->AddUShort(stride);

					for(k=0;k<stride;k++)
					{
						lightMap->AddCoord(coords[index[k+j]]);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddColor(errorColor);
					}
					for(k=0;k<stride;k++)
					{
						lightMap->AddUVs(0.5f, 0.5f);
		//				DEBUG_STREAM << k << " " << lightMapUVs[k][0] << " " << lightMapUVs[k][0] << "\n";
					}
				}
#endif
			}
		}
		break;
		default:
			STOP(("MLR_I_PMesh::LightMapLighting: What you want me to do ?"));
		break;
	}
}

//---------------------------------------------------------------------------
//
bool
	MLR_I_PMesh::CastRay(
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
		Scalar distance = line->GetDistanceTo(*plane, &product);
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
MLR_I_PMesh*
	MidLevelRenderer::CreateIndexedCube_NoColor_NoLit(
		Scalar half, 
		MLRState *state
	)
{
	gos_PushCurrentHeap(Heap);
	MLR_I_PMesh *ret = new MLR_I_PMesh();
	Register_Object(ret);

	Point3D *coords = new Point3D [8];
	Register_Object(coords);

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

	ret->SetCoordData(coords, 8);

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

	ret->FindFacePlanes();

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

	Unregister_Object(texCoords);
	delete [] texCoords;

	Unregister_Pointer(index);
	delete [] index;

	Unregister_Pointer(lengths);
	delete [] lengths;

	Unregister_Object(coords);
	delete [] coords;

	gos_PopCurrentHeap();
	return ret;
}

long MidLevelRenderer::triDrawn = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MidLevelRenderer::subdivide (
		Point3D *coords,
		Point3D& v1,
		Point3D& v2,
		Point3D& v3,
		long depth,
		long tri2draw, 
        float rad
	)
{
	Point3D v12(0.0f, 0.0f, 0.0f), v23(0.0f, 0.0f, 0.0f), v31(0.0f, 0.0f, 0.0f);
	int i;

	if (depth == 0)
	{
		if (tri2draw > triDrawn)
		{
			coords[3*triDrawn].Multiply(v1, rad);
			coords[3*triDrawn+1].Multiply(v3, rad);
			coords[3*triDrawn+2].Multiply(v2, rad);
		}
		triDrawn++;
		return;
	}

	for (i=0;i<3;i++)
	{
		v12[i] = v1[i] + v2[i];
		v23[i] = v2[i] + v3[i];
		v31[i] = v3[i] + v1[i];
	}
	v12.Normalize(v12);
	v23.Normalize(v23);
	v31.Normalize(v31);

	MidLevelRenderer::subdivide (coords, v1, v12, v31, depth-1, tri2draw, rad);
	MidLevelRenderer::subdivide (coords, v2, v23, v12, depth-1, tri2draw, rad);
	MidLevelRenderer::subdivide (coords, v3, v31, v23, depth-1, tri2draw, rad);
	MidLevelRenderer::subdivide (coords, v12, v23, v31, depth-1, tri2draw, rad);
}

float MidLevelRenderer::vdata[12][3] = {
  {-ICO_X, 0.0f, ICO_Z}, {ICO_X, 0.0f, ICO_Z}, {-ICO_X, 0.0f, -ICO_Z}, {ICO_X, 0.0f, -ICO_Z},
  {0.0f, ICO_Z, ICO_X}, {0.0f, ICO_Z, -ICO_X}, {0.0f, -ICO_Z, ICO_X}, {0.0f, -ICO_Z, -ICO_X},
  {ICO_Z, ICO_X, 0.0f}, {-ICO_Z, ICO_X, 0.0f}, {ICO_Z, -ICO_X, 0.0f}, {-ICO_Z, -ICO_X, 0.0f}
};

unsigned int MidLevelRenderer::tindices [20][3] = {
  {0, 4, 1}, {0, 9, 4}, {9, 5, 4}, {4, 5, 8}, {4, 8, 1},
  {8, 10, 1}, {8, 3, 10}, {5, 3, 8}, {5, 2, 3}, {2, 7, 3},
  {7, 10, 3}, {7, 6, 10}, {7, 11, 6}, {11, 0, 6}, {0, 1, 6},
  {6, 1, 10}, {9, 0, 11}, {9, 11, 2}, {9, 2, 5}, {7, 2, 11}
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape*
	MidLevelRenderer::CreateIndexedIcosahedron_NoColor_NoLit(
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

	int uniquePoints = 0;
	for (k=0;k<20;k++)
	{
		triDrawn = 0;
		MLR_I_PMesh *mesh = new MLR_I_PMesh();
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

		ret->Add(mesh);
		mesh->DetachReference();
	}

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
