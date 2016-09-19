//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//#####################    MLRIndexedPrimitive    #############################
//#############################################################################

MLRIndexedPrimitive::ClassData*
	MLRIndexedPrimitive::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::InitializeClass()
{
	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			MLRIndexedPrimitiveClassID,
			"MidLevelRenderer::MLRIndexedPrimitive",
			MLRPrimitive::DefaultData,
			NULL
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPrimitive::MLRIndexedPrimitive(
	ClassData *class_data,
	MemoryStream *stream,
	int version
):
	MLRPrimitive(class_data, stream, version)
{
	Check_Pointer(this);
	Check_Object(stream);

	MemoryStreamIO_Read(stream, &index);

	visibleIndexedVerticesKey = false;
	visibleIndexedVertices.SetLength(numVertices);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLRPrimitive::Save(stream);

	MemoryStreamIO_Write(stream, &index);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPrimitive::MLRIndexedPrimitive(ClassData *class_data):
	MLRPrimitive(class_data), index(0)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPrimitive::~MLRIndexedPrimitive()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::InitializeDrawPrimitive(int vis, int parameter)
{
	MLRPrimitive::InitializeDrawPrimitive(vis, parameter);

	gos_indices = NULL;
	numGOSIndices = -1;

	visibleIndexedVerticesKey = false;
	int i, len = visibleIndexedVertices.GetLength();

	for(i=0;i<len;i++)
	{
		visibleIndexedVertices[i] = 0;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::SetCoordData(
		const Point3D *data,
		int dataSize
	)
{
	Check_Object(this); 
	Check_Pointer(data);

	Verify(colors.GetLength() == 0 || dataSize == colors.GetLength());
	Verify(normals.GetLength() == 0 || dataSize == normals.GetLength());
	Verify(texCoords.GetLength() == 0 || dataSize == texCoords.GetLength());

#if defined (MAX_NUMBER_VERTICES)
	Verify(dataSize <= MAX_NUMBER_VERTICES);
#endif
	coords.AssignData(data, dataSize);
	transformedCoords.SetLength(dataSize);
	numVertices = dataSize;

	if(index.GetLength() > 0 && visibleIndexedVertices.GetLength() != dataSize)
	{
		visibleIndexedVertices.SetLength(dataSize);
	}


}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::SetIndexData(
		unsigned short *index_array,
		int index_count
	)
{
	Check_Object(this); 
	Check_Pointer(index_array);

	if(coords.GetLength() > 0)
	{
		visibleIndexedVertices.SetLength(coords.GetLength());
	}

	index.AssignData(index_array, index_count);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::GetIndexData(
		unsigned short **index_array,
		int *index_count
	)
{
	Check_Object(this); 

	*index_array = index.GetData();
	*index_count = index.GetLength();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::Transform(Matrix4D *mat)
{
	Check_Object(this);

	int i, len = coords.GetLength();

	for(i=0;i<len;i++)
	{
		transformedCoords[i].Multiply(coords[i], *mat);
	}
	
	#ifdef LAB_ONLY
		Statistics::MLR_TransformedVertices += len;
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitive::Lighting (
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

			int i, k, len = colors.GetLength();

			MLRVertexData vertexData;

#if COLOR_AS_DWORD
			TO_DO;
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

				for(k=0;k<len;k++)
				{
					light->LightVertex(vertexData);

					vertexData.point++;
					vertexData.color++;
					vertexData.normal++;
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

			int i, k, len = colors.GetLength();

			MLRVertexData vertexData;

#if COLOR_AS_DWORD
			TO_DO;
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

				for(k=0;k<len;k++)
				{
					light->LightVertex(vertexData);

					vertexData.point++;
					vertexData.color++;
					vertexData.normal++;
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
