//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

#if !defined(MLR_MLRCLIPTRICK_HPP)
#include<mlr/mlrcliptrick.hpp>
#endif

//#############################################################################
//#########################    MLRLineCloud    ###############################
//#############################################################################

MLRLineCloud::ClassData*
	MLRLineCloud::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLineCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRLineCloudClassID,
			"MidLevelRenderer::MLRLineCloud",
			MLREffect::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLineCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLineCloud::MLRLineCloud(int nr, int _type) :
	MLREffect(2*nr, DefaultData), type(_type)
{
	Verify(gos_GetCurrentHeap() == Heap);
	usedNrOfVertices = 0;

	Check_Pointer(this);
	
	drawMode = SortData::LineCloud;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLineCloud::~MLRLineCloud()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::SetData(
		const int *count,
		const Stuff::Point3D *point_data,
		const Stuff::RGBAColor *color_data
	)
{
	Check_Pointer(this);

	usedNrOfVertices = count;
	Verify(*usedNrOfVertices <= maxNrOf);
	points = point_data;
	colors = color_data;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::Draw (DrawEffectInformation *dInfo, GOSVertexPool *allVerticesToDraw, MLRSorter *sorter)
{
	Check_Object(this);

	worldToEffect.Invert(*dInfo->effectToWorld);

	Transform(*usedNrOfVertices, 1);

#if 0
	Lighting(*shape->worldToShape, dInfo->activeLights, dInfo->nrOfActiveLights);
#endif

	if( Clip(dInfo->clippingFlags, allVerticesToDraw) )
	{
		sorter->AddEffect(this, dInfo->state);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::Transform(int, int)
{
	Check_Object(this);

	Start_Timer(Transform_Time);
	int i;

	for(i=0;i<*usedNrOfVertices;i++)
	{
		if(IsOn(i) == false)
		{
			continue;
		}

		(*transformedCoords)[i].Multiply(points[i], effectToClipMatrix);
	}
	Stop_Timer(Transform_Time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int 
	MLRLineCloud::Clip(MLRClippingState clippingFlags, GOSVertexPool *vt)
{
	//--------------------------------------
	// See if we don't have to draw anything
	//--------------------------------------
	//
	int i;
	numGOSVertices = 0;

	if(clippingFlags.GetClippingState() == 0 || usedNrOfVertices <= 0)
	{
		if(usedNrOfVertices <= 0)
		{
			visible = 0;
		}
		else
		{
			Check_Object(vt);
			gos_vertices = vt->GetActualVertexPool();

			for(i=0;i<*usedNrOfVertices;i++)
			{
				if(IsOn(i) == false)
				{
					continue;
				}

				GOSCopyData(
					&gos_vertices[numGOSVertices],
					transformedCoords->GetData(),
					colors,
 					i
				);

				numGOSVertices++;
			}

			Check_Object(vt);
			vt->Increase(numGOSVertices);
			
			visible = numGOSVertices ? 1 : 0;
		}
		return visible;
	}

	Check_Object(vt);
	gos_vertices = vt->GetActualVertexPool();

	Stuff::Vector4D *v4d = transformedCoords->GetData();

	MLRClippingState theAnd, theOr, startClip, endClip;

	for(i=0;i<*usedNrOfVertices;i+=2)
	{
//		if(IsOn(i) == false)
//		{
//			continue;
//		}

		startClip.Clip4dVertex(v4d+i);
		endClip.Clip4dVertex(v4d+i+1);

		theAnd = *(int *)&startClip & *(int *)&endClip;
		theOr = *(int *)&startClip | *(int *)&endClip;

		theAnd = theOr = 0;		//ASSUME NO CLIPPING NEEDED FOR MC2.  Its just not done here!

		if(theAnd!=0)
		{
			continue;
		}

		if(theOr==0)
		{
			GOSCopyData(
				&gos_vertices[numGOSVertices],
				v4d,
				colors,
				i
			);
			numGOSVertices++;

			GOSCopyData(
				&gos_vertices[numGOSVertices],
				v4d,
				colors,
				i+1
			);
			numGOSVertices++;

			continue;
		}

		Scalar a=0.0f;
		int l, ct=0, mask;

		if(theOr.GetNumberOfSetBits()==1)
		{
			Stuff::Vector4D p4d;
			RGBAColor cc;
			mask = 1;
			ct = 0;
			a = 0.0f;
			for (l=0; l<MLRClippingState::NextBit; l++)
			{
				if(theOr.IsClipped(mask))
				{
					//
					//-------------------------------------------
					// Find the clipping interval from bc0 to bc1
					//-------------------------------------------
					//
					a = GetLerpFactor(l, (*transformedCoords)[i], (*transformedCoords)[i+1]);

					Verify(a >= 0.0f && a <= 1.0f);

					ct = l;

					break;
				}
				mask <<= 1;
			}
			p4d.Lerp(
				(*transformedCoords)[i],
				(*transformedCoords)[i+1],
				a
			);

			DoClipTrick(p4d, ct);

			cc.Lerp(
				colors[i],
				colors[i+1],
				a
			);

			if(startClip==0)
			{
				GOSCopyData(
					&gos_vertices[numGOSVertices],
					transformedCoords->GetData(),
					colors,
					i
				);
				numGOSVertices++;

				GOSCopyData(
					&gos_vertices[numGOSVertices],
					&p4d,
					&cc,
					0
				);
				numGOSVertices++;
			}
			else
			{
				GOSCopyData(
					&gos_vertices[numGOSVertices],
					&p4d,
					&cc,
					0
				);
				numGOSVertices++;

				GOSCopyData(
					&gos_vertices[numGOSVertices],
					transformedCoords->GetData(),
					colors,
					i+1
				);
				numGOSVertices++;
			}
		}
		else
		{
			Stuff::Vector4D p4d0, p4d1;
			RGBAColor cc0, cc1;

			p4d0 = (*transformedCoords)[i];
			p4d1 = (*transformedCoords)[i+1];

			cc0 = colors[i];
			cc1 = colors[i+1];

			mask = 1;
			ct = 0;
			a = 0.0f;

			for(l=0; l<MLRClippingState::NextBit; l++)
			{
				if(theOr.IsClipped(mask))
				{
				//
				//-----------------------------------
				// Clip each vertex against the plane
				//-----------------------------------
				//
					if(startClip.IsClipped(mask))
					{
						a = GetLerpFactor(l, p4d0, p4d1);

						Verify(a >= 0.0f && a <= 1.0f);

						p4d0.Lerp(
							p4d0,
							p4d1,
							a
						);

						DoClipTrick(p4d0, l);

						startClip.Clip4dVertex(&p4d0);

						cc0.Lerp(
							cc0,
							cc1,
							a
						);
					}


					if(endClip.IsClipped(mask))
					{
						a = GetLerpFactor(l, p4d0, p4d1);

						Verify(a >= 0.0f && a <= 1.0f);

						p4d1.Lerp(
							p4d0,
							p4d1,
							a
						);

						DoClipTrick(p4d1, l);

						endClip.Clip4dVertex(&p4d1);

						cc1.Lerp(
							cc0,
							cc1,
							a
						);
					}
				}

				if(*(int *)&startClip & *(int *)&endClip)
				{
					break;
				}
				mask = mask << 1;
			}

			if(l==MLRClippingState::NextBit)
			{
				GOSCopyData(
					&gos_vertices[numGOSVertices],
					&p4d0,
					&cc0,
					0
				);
				numGOSVertices++;

				GOSCopyData(
					&gos_vertices[numGOSVertices],
					&p4d1,
					&cc1,
					0
				);
				numGOSVertices++;
			}
		}
	}
	vt->Increase(numGOSVertices);

	visible = numGOSVertices ? 1 : 0;

	return visible;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::TestInstance() const
{
	if (usedNrOfVertices)
	{
		Verify(*usedNrOfVertices >= 0);
		Verify(*usedNrOfVertices <= maxNrOf);
	}
}

