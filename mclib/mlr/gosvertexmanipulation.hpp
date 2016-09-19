//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//#if !defined(MLR_GOSVERTEXMANIPULATION_HPP)

extern bool PerspectiveMode;

#define FCR_TRICK 0

#ifdef I_SAY_YES_TO_MULTI_TEXTURE
	#define VERTEX_STRUCT_SIZE	0x28	//	(sizeof(GOSVertex2UV))
	#define VERTEX_STRUCT_SIZEx2	0x50	//	(sizeof(GOSVertex2UV))
#else
	#define VERTEX_STRUCT_SIZE	0x20	//	(sizeof(GOSVertex))
	#define VERTEX_STRUCT_SIZEx2	0x40	//(sizeof(GOSVertex))
#endif

//	copies vertex data into rasterizer format
inline bool GOSCopyData (
#ifdef I_SAY_YES_TO_MULTI_TEXTURE
					GOSVertex2UV *gos_vertices,
#else
					GOSVertex *gos_vertices,
#endif
					const Stuff::Vector4D *coords,
#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_DWORD_COLOR
					const DWORD *colors,
	#else	//	I_SAY_YES_TO_DWORD_COLOR
					const Stuff::RGBAColor *colors,
	#endif	//	I_SAY_YES_TO_DWORD_COLOR
#endif	//	I_SAY_YES_TO_COLOR
#ifdef I_SAY_YES_TO_TEXTURE
	#ifdef I_SAY_YES_TO_MULTI_TEXTURE
					const Vector2DScalar *texCoords1,
					const Vector2DScalar *texCoords2,
	#else
					const Vector2DScalar *texCoords,
	#endif
#endif
					int _offset
#if FOG_HACK
				 , int foggy = 1
#endif	//	FOG_HACK
						)
{
	//Changed for MC2  due to Perspective and Parallel mode BOTH supported
	if (PerspectiveMode)
	{
		gos_vertices[0].rhw = 1.0f;
		if (fabs(coords[_offset].w) > Stuff::SMALL)
			gos_vertices[0].rhw = 1.0f / coords[_offset].w;
			
		gos_vertices[0].x = coords[_offset].x * gos_vertices[0].rhw;
		gos_vertices[0].y = coords[_offset].y * gos_vertices[0].rhw;
		gos_vertices[0].z = coords[_offset].z * gos_vertices[0].rhw; 
		gos_vertices[0].rhw = (float)fabs(gos_vertices[0].rhw);
	
		gos_vertices[0].x = gos_vertices[0].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[0].y = gos_vertices[0].y*ViewportScalars::MulY + ViewportScalars::AddY;
	}
	else
	{
		gos_vertices[0].rhw = 0.000001f;
	
		gos_vertices[0].x = (1.0f - coords[_offset].x);
		gos_vertices[0].y = (1.0f - coords[_offset].y);
		gos_vertices[0].z = coords[_offset].z;
	
		gos_vertices[0].x = gos_vertices[0].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[0].y = gos_vertices[0].y*ViewportScalars::MulY + ViewportScalars::AddY;
	}

#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_DWORD_COLOR
			gos_vertices[0].argb = colors[_offset];
	#else	//	I_SAY_YES_TO_DWORD_COLOR
			gos_vertices[0].argb = GOSCopyColor(&colors[_offset]);
	#endif	//	I_SAY_YES_TO_DWORD_COLOR
#else	//	I_SAY_YES_TO_COLOR
			gos_vertices[0].argb = 0xffffffff;
#endif	//	I_SAY_YES_TO_COLOR

	*((BYTE *)&gos_vertices[0].frgb + 3) = 0xff;

#ifdef I_SAY_YES_TO_TEXTURE
	#ifdef I_SAY_YES_TO_MULTI_TEXTURE
			gos_vertices[0].u1 = texCoords1[_offset][0];
			gos_vertices[0].v1 = texCoords1[_offset][1];
			gos_vertices[0].u2 = texCoords2[_offset][0];
			gos_vertices[0].v2 = texCoords2[_offset][1];
	#else	//	I_SAY_YES_TO_MULTI_TEXTURE
			gos_vertices[0].u = texCoords[_offset][0];
			gos_vertices[0].v = texCoords[_offset][1];
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURE
#else	//	I_SAY_YES_TO_TEXTURE
	#ifdef I_SAY_YES_TO_MULTI_TEXTURE
			gos_vertices[0].u1 = 0.0f;
			gos_vertices[0].v1 = 0.0f;
			gos_vertices[0].u2 = 0.0f;
			gos_vertices[0].v2 = 0.0f;
	#else	//	I_SAY_YES_TO_MULTI_TEXTURE
			gos_vertices[0].u = 0.0f;
			gos_vertices[0].v = 0.0f;
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURE
#endif	//	I_SAY_YES_TO_TEXTURE

	return true;
}

//	copies 3 vertex data into rasterizer format
inline bool GOSCopyTriangleData (
#ifdef I_SAY_YES_TO_MULTI_TEXTURE
			GOSVertex2UV *gos_vertices,
#else	//	I_SAY_YES_TO_MULTI_TEXTURE
			GOSVertex *gos_vertices,
#endif	//	I_SAY_YES_TO_MULTI_TEXTURE
			 const Stuff::Vector4D *coords, 
#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_DWORD_COLOR
			 const DWORD *colors,
	#else	//	I_SAY_YES_TO_DWORD_COLOR
			 const Stuff::RGBAColor *colors,
	#endif	//	I_SAY_YES_TO_DWORD_COLOR
#endif	//	I_SAY_YES_TO_COLOR
#ifdef I_SAY_YES_TO_TEXTURE
	#ifdef I_SAY_YES_TO_MULTI_TEXTURE
			 const Vector2DScalar *texCoords1,
			 const Vector2DScalar *texCoords2,
	#else	//	I_SAY_YES_TO_MULTI_TEXTURE
			 const Vector2DScalar *texCoords,
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURE
#endif	//	I_SAY_YES_TO_TEXTURE
			 int offset0, int offset1, int offset2
#if FOG_HACK
			 , int foggy = 1
#endif	//	FOG_HACK
							 )
{
	//Changed for MC2  due to Perspective and Parallel mode BOTH supported
	if (PerspectiveMode)
	{

		gos_vertices[0].rhw = 1.0f;
		if (fabs(coords[offset0].w) > Stuff::SMALL)
			gos_vertices[0].rhw = 1.0f / coords[offset0].w;
			
		gos_vertices[0].x = coords[offset0].x * gos_vertices[0].rhw;
		gos_vertices[0].y = coords[offset0].y * gos_vertices[0].rhw;
		gos_vertices[0].z = coords[offset0].z * gos_vertices[0].rhw; 
		gos_vertices[0].rhw = (float)fabs(gos_vertices[0].rhw);
	
		gos_vertices[1].rhw = 1.0f;
		if (fabs(coords[offset1].w) > Stuff::SMALL)
			gos_vertices[1].rhw = 1.0f / coords[offset1].w;
			
		gos_vertices[1].x = coords[offset1].x * gos_vertices[1].rhw;
		gos_vertices[1].y = coords[offset1].y * gos_vertices[1].rhw;
		gos_vertices[1].z = coords[offset1].z * gos_vertices[1].rhw; 
		gos_vertices[1].rhw = (float)fabs(gos_vertices[1].rhw);
	
		gos_vertices[2].rhw = 1.0f;
		if (fabs(coords[offset2].w) > Stuff::SMALL)
			gos_vertices[2].rhw = 1.0f / coords[offset2].w;
			
		gos_vertices[2].x = coords[offset2].x * gos_vertices[2].rhw;
		gos_vertices[2].y = coords[offset2].y * gos_vertices[2].rhw;
		gos_vertices[2].z = coords[offset2].z * gos_vertices[2].rhw; 
		gos_vertices[2].rhw = (float)fabs(gos_vertices[2].rhw);
	
		gos_vertices[0].x = gos_vertices[0].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[0].y = gos_vertices[0].y*ViewportScalars::MulY + ViewportScalars::AddY;

		gos_vertices[1].x = gos_vertices[1].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[1].y = gos_vertices[1].y*ViewportScalars::MulY + ViewportScalars::AddY;
		
		gos_vertices[2].x = gos_vertices[2].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[2].y = gos_vertices[2].y*ViewportScalars::MulY + ViewportScalars::AddY;
	}
	else
	{
		gos_vertices[0].rhw = gos_vertices[1].rhw = gos_vertices[2].rhw = 0.000001f;

		gos_vertices[0].x = 1.0f - coords[offset0].x;
		gos_vertices[0].y = 1.0f - coords[offset0].y;
		gos_vertices[0].z = coords[offset0].z;

		gos_vertices[1].x = 1.0f - coords[offset1].x;
		gos_vertices[1].y = 1.0f - coords[offset1].y;
		gos_vertices[1].z = coords[offset0].z;

		gos_vertices[2].x = 1.0f - coords[offset2].x;
		gos_vertices[2].y = 1.0f - coords[offset2].y;
		gos_vertices[2].z = coords[offset0].z;

		gos_vertices[0].x = gos_vertices[0].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[0].y = gos_vertices[0].y*ViewportScalars::MulY + ViewportScalars::AddY;

		gos_vertices[1].x = gos_vertices[1].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[1].y = gos_vertices[1].y*ViewportScalars::MulY + ViewportScalars::AddY;
		
		gos_vertices[2].x = gos_vertices[2].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_vertices[2].y = gos_vertices[2].y*ViewportScalars::MulY + ViewportScalars::AddY;
	}

#ifdef I_SAY_YES_TO_COLOR
	#ifdef I_SAY_YES_TO_DWORD_COLOR
		gos_vertices[0].argb = colors[offset0];
		gos_vertices[1].argb = colors[offset1];
		gos_vertices[2].argb = colors[offset2];
	#else	//	I_SAY_YES_TO_DWORD_COLOR
		gos_vertices[0].argb = GOSCopyColor(&colors[offset0]);
		gos_vertices[1].argb = GOSCopyColor(&colors[offset1]);
		gos_vertices[2].argb = GOSCopyColor(&colors[offset2]);
	#endif	//	I_SAY_YES_TO_DWORD_COLOR
#else
		gos_vertices[0].argb = 0xffffffff;
		gos_vertices[1].argb = 0xffffffff;
		gos_vertices[2].argb = 0xffffffff;
#endif	//	I_SAY_YES_TO_COLOR

		*((BYTE *)&gos_vertices[0].frgb + 3) = 0xff;
		*((BYTE *)&gos_vertices[1].frgb + 3) = 0xff;
		*((BYTE *)&gos_vertices[2].frgb + 3) = 0xff;

#ifdef I_SAY_YES_TO_TEXTURE
	#ifdef I_SAY_YES_TO_MULTI_TEXTURE
		gos_vertices[0].u1 = texCoords1[offset0][0];
		gos_vertices[0].v1 = texCoords1[offset0][1];
		gos_vertices[1].u1 = texCoords1[offset1][0];
		gos_vertices[1].v1 = texCoords1[offset1][1];
		gos_vertices[2].u1 = texCoords1[offset2][0];
		gos_vertices[2].v1 = texCoords1[offset2][1];

		gos_vertices[0].u2 = texCoords2[offset0][0];
		gos_vertices[0].v2 = texCoords2[offset0][1];
		gos_vertices[1].u2 = texCoords2[offset1][0];
		gos_vertices[1].v2 = texCoords2[offset1][1];
		gos_vertices[2].u2 = texCoords2[offset2][0];
		gos_vertices[2].v2 = texCoords2[offset2][1];
	#else	//	I_SAY_YES_TO_MULTI_TEXTURE
		gos_vertices[0].u = texCoords[offset0][0];
		gos_vertices[0].v = texCoords[offset0][1];
		gos_vertices[1].u = texCoords[offset1][0];
		gos_vertices[1].v = texCoords[offset1][1];
		gos_vertices[2].u = texCoords[offset2][0];
		gos_vertices[2].v = texCoords[offset2][1];
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURE
#else	//	I_SAY_YES_TO_TEXTURE
	#ifdef I_SAY_YES_TO_MULTI_TEXTURE
		gos_vertices1[0].u = 0.0f;
		gos_vertices1[0].v = 0.0f;
		gos_vertices1[1].u = 0.0f;
		gos_vertices1[1].v = 0.0f;
		gos_vertices1[2].u = 0.0f;
		gos_vertices1[2].v = 0.0f;

		gos_vertices2[0].u = 0.0f;
		gos_vertices2[0].v = 0.0f;
		gos_vertices2[1].u = 0.0f;
		gos_vertices2[1].v = 0.0f;
		gos_vertices2[2].u = 0.0f;
		gos_vertices2[2].v = 0.0f;
	#else	//	I_SAY_YES_TO_MULTI_TEXTURE
		gos_vertices[0].u = 0.0f;
		gos_vertices[0].v = 0.0f;
		gos_vertices[1].u = 0.0f;
		gos_vertices[1].v = 0.0f;
		gos_vertices[2].u = 0.0f;
		gos_vertices[2].v = 0.0f;
	#endif	//	I_SAY_YES_TO_MULTI_TEXTURE
#endif	//	I_SAY_YES_TO_TEXTURE

	return true;
}

#undef VERTEX_STRUCT_SIZE
#undef VERTEX_STRUCT_SIZEx2
//#endif
