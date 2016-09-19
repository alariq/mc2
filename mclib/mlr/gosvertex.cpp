//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

Scalar
	GOSVertex::farClipReciprocal;

Scalar
	ViewportScalars::MulX,
	ViewportScalars::MulY,
	ViewportScalars::AddX,
	ViewportScalars::AddY;

#if FOG_HACK
	BYTE GOSVertex::fogTable[Limits::Max_Number_Of_FogStates][1024];
#endif

//#############################################################################
//############################    GOSVertex   #################################
//#############################################################################

GOSVertex::GOSVertex()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	rhw = 1.0f;

	argb = 0xffffffff;

	u = 0.0f;
	v = 0.0f;

	frgb = 0xffffffff;
}

void
	GOSVertex::SetFogTableEntry(int entry, Scalar nearFog, Scalar farFog, Scalar fogDensity)
{
	float Fog;

	Verify(farFog > nearFog);

	entry--;

	GOSVertex::fogTable[entry][0] = 0;

	for( int t1=0; t1<1024; t1++ )
	{
		if( 0.0f == fogDensity )
		{
			Fog=(farFog-t1)/(farFog-nearFog);													// 0.0 = Linear fog table (from Start to End)
		}
		else
		{
			if( fogDensity<1.0f )
			{
				Fog=(float)exp(-fogDensity*t1);											// 0.0->1.0 = FOG_EXP
			}
			else
			{
				Fog=(float)exp(-((fogDensity-1.0f)*t1)*((fogDensity-1.0f)*t1));		// 1.0->2.0 = FOG_EXP2
			}
		}
		
		if( Fog<0.0f )
			Fog=0.0f;
		if( Fog>1.0f )
			Fog=1.0f;

		GOSVertex::fogTable[entry][t1]=(BYTE)(255.9f*Fog);
	}
}

#if 0

bool
	GOSCopyTriangleData
		(	GOSVertex *gos_vertices,
			Vector4D *coords,
			RGBAColor *colors,
			Vector2DScalar *texCoords,
			int offset0,
			int offset1,
			int offset2)
{
#if USE_ASSEMBLER_CODE

		_asm {
;			gos_vertices[0].w = 1.0f/coords[offset0].w;

			mov         ecx,dword ptr [coords]
			fld1

			mov         edi,dword ptr [offset0]
			mov         eax,edi
			shl         eax,4
			fdiv        dword ptr [eax+ecx+0Ch]
			add         eax,ecx
			mov         edx,dword ptr [gos_vertices]
			fst			dword ptr [edx+0Ch]

;			gos_vertices[0].x = coords[offset0].x * gos_vertices[0].w;
			fld         st(0)
			fmul        dword ptr [eax]
			fstp        dword ptr [edx]
;			gos_vertices[0].y = coords[offset0].y * gos_vertices[0].w;
			fld         dword ptr [eax+4]
			fmul        st,st(1)
			fstp        dword ptr [edx+4]
;			gos_vertices[0].z = coords[offset0].z * gos_vertices[0].w;
			fld         dword ptr [eax+8]

;			gos_vertices[1].w = 1.0f/coords[offset1].w;
			mov         eax,dword ptr [offset1]
			fmul        st,st(1)
			shl         eax,4
			add         eax,ecx
			fstp        dword ptr [edx+8]
			fstp        st(0)

			fld1
			fdiv        dword ptr [eax+0Ch]
			fst         dword ptr [edx+2Ch]


;			gos_vertices[1].x = coords[offset1].x * gos_vertices[1].w;
			fld         st(0)
			fmul        dword ptr [eax]
			fstp        dword ptr [edx+20h]
;			gos_vertices[1].y = coords[offset1].y * gos_vertices[1].w;
			fld         dword ptr [eax+4]
			fmul        st,st(1)
			fstp        dword ptr [edx+24h]
;			gos_vertices[1].z = coords[offset1].z * gos_vertices[1].w;
			fld         dword ptr [eax+8]

;			gos_vertices[2].w = 1.0f/coords[offset2].w;
			mov         eax,dword ptr [offset2]
			fmul        st,st(1)
			shl         eax,4
			add         eax,ecx
			fstp        dword ptr [edx+28h]
			fstp        st(0)
			fld1
			fdiv        dword ptr [eax+0Ch]
			fst         dword ptr [edx+4Ch]

;			gos_vertices[2].x = coords[offset2].x * gos_vertices[2].w;
			fld         st(0)
			fmul        dword ptr [eax]
			fstp        dword ptr [edx+40h]
;			gos_vertices[2].y = coords[offset2].y * gos_vertices[2].w;
			fld         dword ptr [eax+4]
			fmul        st,st(1)
			fstp        dword ptr [edx+44h]
;			gos_vertices[2].z = coords[offset2].z * gos_vertices[2].w;
			fld         dword ptr [eax+8]
			fmul        st,st(1)
			fstp        dword ptr [edx+48h]
			fstp        st(0)
		}

#else
		gos_vertices[0].w = 1.0f/coords[offset0].w;

		gos_vertices[0].x = coords[offset0].x * gos_vertices[0].w;
		gos_vertices[0].y = coords[offset0].y * gos_vertices[0].w;
		gos_vertices[0].z = coords[offset0].z * gos_vertices[0].w;

		gos_vertices[1].w = 1.0f/coords[offset1].w;


		gos_vertices[1].x = coords[offset1].x * gos_vertices[1].w;
		gos_vertices[1].y = coords[offset1].y * gos_vertices[1].w;
		gos_vertices[1].z = coords[offset1].z * gos_vertices[1].w;

		gos_vertices[2].w = 1.0f/coords[offset2].w;

		gos_vertices[2].x = coords[offset2].x * gos_vertices[2].w;
		gos_vertices[2].y = coords[offset2].y * gos_vertices[2].w;
		gos_vertices[2].z = coords[offset2].z * gos_vertices[2].w;
#endif

//		gos_vertices[0] = colors[offset0];
//		gos_vertices[1] = colors[offset1];
//		gos_vertices[2] = colors[offset2];

		Scalar f;
#if USE_ASSEMBLER_CODE
		int argb;

		_asm {
			fld		float_cheat

			mov		esi, dword ptr [colors]
			mov		eax, offset0
			shl		eax, 4
			add		esi, eax

			fld		dword ptr [esi + 0Ch]

			mov		ecx, dword ptr [esi + 0Ch]

			fadd	st, st(1)

			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		argb, ebx

			fld		dword ptr [esi]

			mov		ecx, dword ptr [esi]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx

			fld		dword ptr [esi+4]

			mov		ecx, dword ptr [esi+4]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx

			fld		dword ptr [esi+8]

			mov		ecx, dword ptr [esi+8]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx
		}

		gos_vertices[0].argb = argb;

		_asm {
			mov		esi, dword ptr [colors]
			mov		eax, offset1
			shl		eax, 4
			add		esi, eax

			fld		dword ptr [esi + 0Ch]

			mov		ecx, dword ptr [esi + 0Ch]

			fadd	st, st(1)

			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		argb, ebx

			fld		dword ptr [esi]

			mov		ecx, dword ptr [esi]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx

			fld		dword ptr [esi+4]

			mov		ecx, dword ptr [esi+4]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx

			fld		dword ptr [esi+8]

			mov		ecx, dword ptr [esi+8]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx
		}

		gos_vertices[1].argb = argb;


		_asm {
			mov		esi, dword ptr [colors]
			mov		eax, offset2
			shl		eax, 4
			add		esi, eax

			fld		dword ptr [esi + 0Ch]

			mov		ecx, dword ptr [esi + 0Ch]

			fadd	st, st(1)

			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		argb, ebx

			fld		dword ptr [esi]

			mov		ecx, dword ptr [esi]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx

			fld		dword ptr [esi+4]

			mov		ecx, dword ptr [esi+4]

			fadd	st, st(1)
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx

			fld		dword ptr [esi+8]

			mov		ecx, dword ptr [esi+8]

			faddp	st(1), st
			rcl		ecx, 1
			sbb		eax, eax
			xor		eax, -1

			fstp	f
			xor		ecx, ecx

			mov		ebx, f
			and		ebx, eax

			test	ebx, 0000ff00h

			seta	cl

			xor		eax, eax
			sub		eax, ecx

			or		ebx, eax
			and		ebx, 000000ffh

			mov		ecx, argb
			shl		ecx, 8

			or		ecx, ebx
			mov		argb, ecx
		}

		gos_vertices[2].argb = argb;

#else
		f = colors[offset0].alpha * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[0].argb = Positive_Float_To_Byte (f);

		f = colors[offset0].red * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[0].argb = (gos_vertices[0].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset0].green * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[0].argb = (gos_vertices[0].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset0].blue * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[0].argb = (gos_vertices[0].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset1].alpha * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[1].argb = Positive_Float_To_Byte (f);

		f = colors[offset1].red * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[1].argb = (gos_vertices[1].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset1].green * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[1].argb = (gos_vertices[1].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset1].blue * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[1].argb = (gos_vertices[1].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset2].alpha * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[2].argb = Positive_Float_To_Byte (f);

		f = colors[offset2].red * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[2].argb = (gos_vertices[2].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset2].green * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[2].argb = (gos_vertices[2].argb << 8) | Positive_Float_To_Byte (f);

		f = colors[offset2].blue * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_vertices[2].argb = (gos_vertices[2].argb << 8) | Positive_Float_To_Byte (f);
#endif

		gos_vertices[0].u = texCoords[offset0][0];
		gos_vertices[0].v = texCoords[offset0][1];
		gos_vertices[1].u = texCoords[offset1][0];
		gos_vertices[1].v = texCoords[offset1][1];
		gos_vertices[2].u = texCoords[offset2][0];
		gos_vertices[2].v = texCoords[offset2][1];

		return true;
	}
#endif