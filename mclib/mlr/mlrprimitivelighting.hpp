//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	CLASSNAME::Lighting (
		MLRLight* const* lights,
		int nrLights
	)
{
	Check_Object(this);

	//
	//----------------------------------------------------------------------
	// If no lights or normals are specified, use the original vertex colors
	//----------------------------------------------------------------------
	//
	actualColors = &colors;
	int state_mask = GetCurrentState().GetLightingMode();
	if (nrLights == 0 || normals.GetLength() == 0 || state_mask == MLRState::LightingOffMode)
		return;
	Check_Pointer(lights);

	//
	//-------------------------------
	// See if we need vertex lighting
	//-------------------------------
	//
	if (state_mask & MLRState::VertexLightingMode)
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
		#endif

		//
		//--------------------------------
		// Now light the array of vertices
		//--------------------------------
		//
		vertexData.point = &coords[0];
		vertexData.color = &litColors[0];
		vertexData.normal = &normals[0];
		for(k=0;k<len;k++)
		{
			if(visibleIndexedVertices[k] != 0)
			{
				vertexData.color->red = 0.0f;
				vertexData.color->green = 0.0f;
				vertexData.color->blue = 0.0f;
				vertexData.color->alpha = color->alpha;
				for (i=0;i<nrLights;i++)
				{
					MLRLight *light = lights[i];
					Check_Object(light);
					int mask = state_mask & light->GetLightMask();
					if (!mask)
						continue;
					if (mask&MLRState::VertexLightingMode)
					{
						if (
							GetCurrentState().GetBackFaceMode() != MLRState::BackFaceOffMode
							 || light->GetLightType() == MLRLight::AmbientLight
						)
						{
							light->LightVertex(vertexData);
							Set_Statistic(LitVertices, LitVertices+1);
						}
					}
				}
			}
			vertexData.point++;
			vertexData.color++;
			vertexData.normal++;
			color++;
		}
		actualColors = &litColors;
	}
}

