//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//###############################    MLRShape    ##################################
//#############################################################################

MLRShape::ClassData*
	MLRShape::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRShape::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRShapeClassID,
			"MidLevelRenderer::MLRShape",
			Plug::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRShape::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape::MLRShape(
	MemoryStream *stream,
	int version
):
	Plug(DefaultData)
{ 
	Check_Pointer(this);
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	*stream >> numPrimitives;
	int i;

	allPrimitives.SetLength(numPrimitives);

	for(i=0;i<numPrimitives;i++)
	{
		MLRPrimitiveBase *pt = NULL;
		switch(version)
		{
			case 1:
			case 2:
			{
				STOP(("Lower than version 3 is not supported anymore !"));
/*
				MLRPrimitive *pt_old;

				RegisteredClass::ClassID class_id;
				*stream >> class_id;
				MLRPrimitive::ClassData* class_data =
					Cast_Object(MLRPrimitive::ClassData*, FindClassData(class_id));
				pt_old = (*class_data->primitiveFactory)(stream, version);
				Register_Object(pt_old);

				int i, nr, test;

				if(pt_old->GetReferenceState().GetLightingMode() == MLRState::LightingOffMode)
				{
					test = 0;
				}
				else
				{
					test = 2;
				}

#if COLOR_AS_DWORD
				DWORD* colors;
#else
				RGBAColor *colors, testColor(1.0f, 1.0f, 1.0f, 1.0f);
#endif
				Cast_Pointer(MLRIndexedPolyMesh*, pt_old)->GetColorData(&colors, &nr);

				for(i=0;i<nr;i++)
				{
#if COLOR_AS_DWORD
					if(colors[i] != 0xffffffff)
#else
					if(colors[i] != testColor)
#endif
					{
						break;
					}
				}
				if(i<nr)
				{
					test |= 1;
				}

				switch (test)
				{
					case 0:
						pt = new MLR_I_PMesh;

						Cast_Pointer(MLR_I_PMesh*, pt)->Copy(Cast_Pointer(MLRIndexedPolyMesh*, pt_old));
					break;
					case 1:
						pt = new MLR_I_C_PMesh;

						Cast_Pointer(MLR_I_C_PMesh*, pt)->Copy(Cast_Pointer(MLRIndexedPolyMesh*, pt_old));
					break;
					default:
						pt = new MLR_I_L_PMesh;

						Cast_Pointer(MLR_I_L_PMesh*, pt)->Copy(Cast_Pointer(MLRIndexedPolyMesh*, pt_old));
					break;
				}
				pt_old->DetachReference();
*/
			}
			break;
			default:
			{


				RegisteredClass::ClassID class_id;
				*stream >> class_id;
				MLRPrimitiveBase::ClassData* class_data =
					Cast_Pointer(MLRPrimitiveBase::ClassData*, FindClassData(class_id));
				Check_Object(class_data);
				pt = (*class_data->primitiveFactory)(stream, version);
				Register_Object(pt);

				if(
					ConvertToTriangleMeshes == true && (
						pt->IsDerivedFrom(MLR_I_PMesh::DefaultData) ||
						pt->IsDerivedFrom(MLR_I_C_PMesh::DefaultData) ||
						pt->IsDerivedFrom(MLR_I_L_PMesh::DefaultData)
					)
				)
				{
					unsigned char *length;
					int i, num, threes, nonThrees;

					pt->GetSubprimitiveLengths(&length, &num);

					for(i=0,threes=0,nonThrees=0;i<num;i++)
					{
						if(length[i]==3)
						{
							threes++;
						}
						else
						{
							nonThrees++;
						}
					}
					if(threes>0 && nonThrees==0)	//threes > 9*nonThrees)
					{
						MLRPrimitiveBase *pt_old = pt;
				
						if(pt->GetClassData() == MLR_I_L_PMesh::DefaultData)
						{
							pt = new MLR_I_L_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_L_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_L_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_C_PMesh::DefaultData)
						{
							pt = new MLR_I_C_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_C_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_C_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_PMesh::DefaultData)
						{
							pt = new MLR_I_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_L_DT_PMesh::DefaultData)
						{
							pt = new MLR_I_L_DT_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_L_DT_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_L_DT_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_C_DT_PMesh::DefaultData)
						{
							pt = new MLR_I_C_DT_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_C_DT_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_C_DT_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_DT_PMesh::DefaultData)
						{
							pt = new MLR_I_DT_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_DT_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_DT_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_L_DeT_PMesh::DefaultData)
						{
							pt = new MLR_I_L_DeT_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_L_DeT_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_L_DeT_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_C_DeT_PMesh::DefaultData)
						{
							pt = new MLR_I_C_DeT_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_C_DeT_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_C_DeT_PMesh*, pt_old));
						}
						else if(pt->GetClassData() == MLR_I_DeT_PMesh::DefaultData)
						{
							pt = new MLR_I_DeT_TMesh;
							Register_Object(pt);

							Cast_Pointer(MLR_I_DeT_TMesh*, pt)->Copy(Cast_Pointer(MLR_I_DeT_PMesh*, pt_old));
						}
						pt_old->DetachReference();
					}
				}
			}
			break;
		}

		allPrimitives[i] = pt;
	}

	referenceCount = 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape::MLRShape(int nr):
	Plug(DefaultData), allPrimitives(nr ? nr : 4)
{ 
	Verify(gos_GetCurrentHeap() == Heap);
	numPrimitives = 0;
	referenceCount = 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape::~MLRShape()
{
	int i;
	MLRPrimitiveBase *pt;

	for(i=numPrimitives-1;i>=0;i--)
	{
		pt = allPrimitives[i];
		allPrimitives[i] = NULL;

		pt->DetachReference();

	}

	Verify(referenceCount==0);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRShape*
	MLRShape::Make(
		MemoryStream *stream,
		int version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLRShape *shape = new MLRShape(stream, version);
	gos_PopCurrentHeap();

	return shape;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRShape::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	*stream << numPrimitives;

	int i;

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);
		allPrimitives[i]->Save(stream);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRShape::Add (MLRPrimitiveBase *p)
{
	Check_Object(this); 

	if(numPrimitives >= allPrimitives.GetLength() || allPrimitives.GetLength() == 0)
	{
		gos_PushCurrentHeap(Heap);
		allPrimitives.SetLength(numPrimitives + 4);
		gos_PopCurrentHeap();
	}

	allPrimitives[numPrimitives] = p;
	p->AttachReference();

	numPrimitives++;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRShape::GetNumPrimitives()
{
	int i, ret = 0;

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);
		ret += allPrimitives[i]->GetNumPrimitives();
	}

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRShape::GetNumDrawnTriangles()
{
	int i, j, ret = 0;

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);
		if(allPrimitives[i]->IsDerivedFrom(MLRIndexedPrimitiveBase::DefaultData))
		{
			j = (Cast_Pointer(MLRIndexedPrimitiveBase*, allPrimitives[i]))->GetNumGOSIndices();
		}
		else
		{
			j = allPrimitives[i]->GetNumGOSVertices();
		}
		if(j!=0xffff)
		{
			ret += j;
		}
	}

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPrimitiveBase*
	MLRShape::Find (int i)
{
	Check_Object(this); 

	Verify(i<numPrimitives);

	return allPrimitives[i];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRShape::Find (MLRPrimitiveBase *p)
{
	Check_Object(this); 
	Check_Object(p);

	int i;

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]); 

		if(allPrimitives[i] == p)
		{
			return i;
		}
	}

	return -1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MLRShape::Replace (MLRPrimitiveBase *pout, MLRPrimitiveBase *pin)
{
	Check_Object(this); 
	Check_Object(pout);
	Check_Object(pin);

	int num = Find (pout);

	if(num>=0)
	{
		pout->DetachReference();
		allPrimitives[num] = pin;
		pin->AttachReference();
	}
	else
	{
		return false;
	}

	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPrimitiveBase*
	MLRShape::Remove(MLRPrimitiveBase *p)
{
	Check_Object(this); 
	Check_Object(p);

	int i, nr = Find(p);

	if(nr < 0)
	{
		return NULL;
	}

	for(i=nr;i<numPrimitives-1;i++)
	{
		allPrimitives[i] = allPrimitives[i+1];
	}

	allPrimitives[i] = NULL;

	numPrimitives--;
	p->DetachReference();

	return p;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRPrimitiveBase*
	MLRShape::Remove(int nr)
{
	Check_Object(this); 

	int i;
	MLRPrimitiveBase *p = Find(nr);

	if(nr < 0 || nr >= numPrimitives)
	{
		return NULL;
	}

	for(i=nr;i<numPrimitives-1;i++)
	{
		allPrimitives[i] = allPrimitives[i+1];
	}

	allPrimitives[i] = NULL;

	numPrimitives--;
	p->DetachReference();

	return p;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
 	MLRShape::Insert(MLRPrimitiveBase *p, int nr)
{
	Check_Object(this); 

	if(nr >= numPrimitives)
	{
		Add(p);

		return numPrimitives;
	}

	if(numPrimitives >= allPrimitives.GetLength() || allPrimitives.GetLength() == 0)
	{
		gos_PushCurrentHeap(Heap);
		allPrimitives.SetLength(numPrimitives + 4);
		gos_PopCurrentHeap();
	}


	int i;

	for(i=numPrimitives;i>nr;i--)
	{
		allPrimitives[i] = allPrimitives[i-1];
	}

	allPrimitives[i] = p;
	p->AttachReference();

	numPrimitives++;

	return numPrimitives;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRShape::InitializePrimitives(unsigned char vis, const MLRState& master, int parameter)
{
	Check_Object(this); 

	int i;

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);

		allPrimitives[i]->InitializeDrawPrimitive(vis, parameter);
		
		allPrimitives[i]->CombineStates(master);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// p is the eye point
void
	MLRShape::HurtMe(const Stuff::LinearMatrix4D& pain, Stuff::Scalar radius)
{
	for(int i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);

		allPrimitives[i]->HurtMe(pain, radius);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// p is the eye point
int
	MLRShape::FindBackFace(const Point3D& p)
{
	Check_Object(this); 

	int i, ret = 0;

	Point3D sp;
	
	sp.Multiply(p, *worldToShape);

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);

		ret += allPrimitives[i]->FindBackFace(sp);
	}

	return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
void
	MLRShape::Transform(Matrix4D *mat)
{
	Check_Object(this); 

	 int i;

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);

		allPrimitives[i]->Transform(mat);
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRShape::Transform()
{
	Check_Object(this); 

	 int i;

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);

		allPrimitives[i]->Transform(&shapeToClipMatrix);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRShape::Clip(MLRClippingState clippingFlags, GOSVertexPool *vp)
{
	Check_Object(this); 

	int i, ret = 0;

	gos_GetViewport( &ViewportScalars::MulX, &ViewportScalars::MulY, &ViewportScalars::AddX, &ViewportScalars::AddY );

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);

		ret += allPrimitives[i]->Clip(clippingFlags, vp);
	}

	return ret;
}
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRShape::Lighting (
		const LinearMatrix4D& WorldToShape, 
		MLRLight* const* lights,
		int nrLights
	)
{
	Check_Object(this);
	Check_Object(&WorldToShape);

	if(nrLights == 0)
	{
		return;
	}

	int i;

	for(i=0;i<nrLights;i++)
	{
		lights[i]->SetLightToShapeMatrix(WorldToShape);
	}

	for(i=0;i<numPrimitives;i++)
	{
		Check_Object(allPrimitives[i]);

		allPrimitives[i]->Lighting(lights, nrLights);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MLRShape::CastRay (
		Line3D *line,
		Normal3D *normal
	)
{
	Check_Object(this);
	Check_Object(line);
	Check_Pointer(normal);

	bool result = false;
	for(int i=0; i<numPrimitives; i++)
	{
		Check_Object(allPrimitives[i]);
		if (allPrimitives[i]->CastRay(line, normal))
			result = true;
	}
	return result;
}
