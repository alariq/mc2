//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#if !defined(MLR_GOSPOINT_HPP)
	#define MLR_GOSPOINT_HPP

	#if !defined(MLR_MLRSTUFF_HPP)
#include<mlr/mlrstuff.hpp>
	#endif

	#if !defined(MLR_GOSVERTEX_HPP)
#include<mlr/gosvertex.hpp>
	#endif

	#if !defined(GAMEOS_HPP)
#include<gameos/gameos.hpp>
	#endif

namespace MidLevelRenderer {

	//##########################################################################
	//#########################    GOSPoint    #################################
	//##########################################################################

	class GOSPoint :
		public gos_POINT
	{
	public:
		GOSPoint();

		inline GOSPoint&
			operator=(const Stuff::Vector4D& v)
				{
					Check_Pointer(this);

					Verify(!Stuff::Small_Enough(v.w));

//					Tell_Value(v);

					w = 1.0f / v.w;
					Verify(x>=0.0f && x<=1.0f);

					x = v.x * w;
					Verify(x>=0.0f && x<=1.0f);

					y = v.y * w;
					Verify(y>=0.0f && y<=1.0f);

					z = v.z * w;
					Verify(z>=0.0f && z<1.0f);

					return *this;
				}

		inline GOSPoint&
			operator=(const Stuff::RGBAColor& c)
				{
					Check_Pointer(this);

//					DEBUG_STREAM << "c = <" << c.alpha << ", " << c.red << ", ";
//					DEBUG_STREAM << c.green << ", " << c.blue << ">" << endl;

					float f;
					f = c.alpha * 255.99f;
					Clamp(f, 0.0f, 255.f);
					argb = Stuff::Positive_Float_To_Byte (f);

					f = c.red * 255.99f;
					Clamp(f, 0.0f, 255.f);
					argb = (argb << 8) | Stuff::Positive_Float_To_Byte (f);

					f = c.green * 255.99f;
					Clamp(f, 0.0f, 255.f);
					argb = (argb << 8) | Stuff::Positive_Float_To_Byte (f);

					f = c.blue * 255.99f;
					Clamp(f, 0.0f, 255.f);
					argb = (argb << 8) | Stuff::Positive_Float_To_Byte (f);

//					DEBUG_STREAM << "0x" << hex << argb << dec << endl;

					return *this;
				}

	protected:

	};

	typedef Stuff::Vector2DOf<Stuff::Scalar> Vector2DScalar;
	
	#pragma warning (disable : 4725)

//	copies 3 vertex data into rasterizer format
	inline bool
		GOSCopyData
			(GOSPoint *gos_points, Stuff::Vector4D *coords, 
			 Stuff::RGBAColor *colors, int _offset)
	{
		Verify(coords[_offset].w > Stuff::SMALL);

#if USE_ASSEMBLER_CODE
		_asm {
;			gos_points[0].w = 1.0f/coords[offset].w;

			mov         ecx,dword ptr [coords]
			fld1

			mov         edi, _offset
			mov         eax,edi
			shl         eax,4
			fdiv        dword ptr [eax+ecx+0Ch]
			add         eax,ecx
			mov         edx,dword ptr [gos_points]
			fst			dword ptr [edx+0Ch]

;			gos_points[0].x = coords[offset].x * gos_points[0].w;
			fld         st(0)
			fmul        dword ptr [eax]
			fstp        dword ptr [edx]
;			gos_points[0].y = coords[offset].y * gos_points[0].w;
			fld         dword ptr [eax+4]
			fmul        st,st(1)
			fstp        dword ptr [edx+4]
;			gos_points[0].z = coords[offset].z * gos_points[0].w;
;			fld         dword ptr [eax+8]

			fmul        dword ptr [eax+8]
			fstp        dword ptr [edx+8]
;			fstp        st(0)
		}

#else
		gos_points[0].w = 1.0f/coords[_offset].w;

		gos_points[0].x = coords[_offset].x * gos_points[0].w;
		gos_points[0].y = coords[_offset].y * gos_points[0].w;
		gos_points[0].z = coords[_offset].z * gos_points[0].w;
#endif

		gos_points[0].x = gos_points[0].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_points[0].y = gos_points[0].y*ViewportScalars::MulY + ViewportScalars::AddY;

//		gos_points[0] = colors[_offset];

		Stuff::Scalar f;
#if USE_ASSEMBLER_CODE
		int argb;

		_asm {
			fld		float_cheat

			mov		esi, dword ptr [colors]
			mov		eax, _offset
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

;			fld		dword ptr [esi+8]

			mov		ecx, dword ptr [esi+8]

			fadd	dword ptr [esi+8]
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

		gos_points[0].argb = argb;
#else
		f = colors[_offset].alpha * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = Stuff::Positive_Float_To_Byte (f);

		f = colors[_offset].red * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = (gos_points[0].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[_offset].green * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = (gos_points[0].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[_offset].blue * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = (gos_points[0].argb << 8) | Stuff::Positive_Float_To_Byte (f);

#endif

		return true;
	}

//	copies 3 vertex data into rasterizer format
	inline bool
		GOSCopyTriangleData
			(GOSPoint *gos_points, Stuff::Vector4D *coords, 
			 Stuff::RGBAColor *colors,
			 int offset0, int offset1, int offset2)
	{
#if USE_ASSEMBLER_CODE

		_asm {
;			gos_points[0].w = 1.0f/coords[offset0].w;

			mov         ecx,dword ptr [coords]
			fld1

			mov         edi,dword ptr [offset0]
			mov         eax,edi
			shl         eax,4
			fdiv        dword ptr [eax+ecx+0Ch]
			add         eax,ecx
			mov         edx,dword ptr [gos_points]
			fst			dword ptr [edx+0Ch]

;			gos_points[0].x = coords[offset0].x * gos_points[0].w;
			fld         st(0)
			fmul        dword ptr [eax]
			fstp        dword ptr [edx]
;			gos_points[0].y = coords[offset0].y * gos_points[0].w;
			fld         dword ptr [eax+4]
			fmul        st,st(1)
			fstp        dword ptr [edx+4]
;			gos_points[0].z = coords[offset0].z * gos_points[0].w;
			fld         dword ptr [eax+8]

;			gos_points[1].w = 1.0f/coords[offset1].w;
			mov         eax,dword ptr [offset1]
			fmul        st,st(1)
			shl         eax,4
			add         eax,ecx
			fstp        dword ptr [edx+8]
			fstp        st(0)

			fld1
			fdiv        dword ptr [eax+0Ch]
			fst         dword ptr [edx+2Ch]


;			gos_points[1].x = coords[offset1].x * gos_points[1].w;
			fld         st(0)
			fmul        dword ptr [eax]
			fstp        dword ptr [edx+20h]
;			gos_points[1].y = coords[offset1].y * gos_points[1].w;
			fld         dword ptr [eax+4]
			fmul        st,st(1)
			fstp        dword ptr [edx+24h]
;			gos_points[1].z = coords[offset1].z * gos_points[1].w;
			fld         dword ptr [eax+8]

;			gos_points[2].w = 1.0f/coords[offset2].w;
			mov         eax,dword ptr [offset2]
			fmul        st,st(1)
			shl         eax,4
			add         eax,ecx
			fstp        dword ptr [edx+28h]
			fstp        st(0)
			fld1
			fdiv        dword ptr [eax+0Ch]
			fst         dword ptr [edx+4Ch]

;			gos_points[2].x = coords[offset2].x * gos_points[2].w;
			fld         st(0)
			fmul        dword ptr [eax]
			fstp        dword ptr [edx+40h]
;			gos_points[2].y = coords[offset2].y * gos_points[2].w;
			fld         dword ptr [eax+4]
			fmul        st,st(1)
			fstp        dword ptr [edx+44h]
;			gos_points[2].z = coords[offset2].z * gos_points[2].w;
			fld         dword ptr [eax+8]
			fmul        st,st(1)
			fstp        dword ptr [edx+48h]
			fstp        st(0)
		}

#else
		gos_points[0].w = 1.0f/coords[offset0].w;

		gos_points[0].x = coords[offset0].x * gos_points[0].w;
		gos_points[0].y = coords[offset0].y * gos_points[0].w;
		gos_points[0].z = coords[offset0].z * gos_points[0].w;

		gos_points[1].w = 1.0f/coords[offset1].w;


		gos_points[1].x = coords[offset1].x * gos_points[1].w;
		gos_points[1].y = coords[offset1].y * gos_points[1].w;
		gos_points[1].z = coords[offset1].z * gos_points[1].w;

		gos_points[2].w = 1.0f/coords[offset2].w;

		gos_points[2].x = coords[offset2].x * gos_points[2].w;
		gos_points[2].y = coords[offset2].y * gos_points[2].w;
		gos_points[2].z = coords[offset2].z * gos_points[2].w;
#endif

		gos_points[0].x = gos_points[0].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_points[0].y = gos_points[0].y*ViewportScalars::MulY + ViewportScalars::AddY;

		gos_points[1].x = gos_points[1].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_points[1].y = gos_points[1].y*ViewportScalars::MulY + ViewportScalars::AddY;
		
		gos_points[2].x = gos_points[2].x*ViewportScalars::MulX + ViewportScalars::AddX;
		gos_points[2].y = gos_points[2].y*ViewportScalars::MulY + ViewportScalars::AddY;

//		gos_points[0] = colors[offset0];
//		gos_points[1] = colors[offset1];
//		gos_points[2] = colors[offset2];

		Stuff::Scalar f;
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

		gos_points[0].argb = argb;

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

		gos_points[1].argb = argb;


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

		gos_points[2].argb = argb;

#else
		f = colors[offset0].alpha * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = Stuff::Positive_Float_To_Byte (f);

		f = colors[offset0].red * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = (gos_points[0].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset0].green * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = (gos_points[0].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset0].blue * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[0].argb = (gos_points[0].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset1].alpha * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[1].argb = Stuff::Positive_Float_To_Byte (f);

		f = colors[offset1].red * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[1].argb = (gos_points[1].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset1].green * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[1].argb = (gos_points[1].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset1].blue * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[1].argb = (gos_points[1].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset2].alpha * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[2].argb = Stuff::Positive_Float_To_Byte (f);

		f = colors[offset2].red * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[2].argb = (gos_points[2].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset2].green * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[2].argb = (gos_points[2].argb << 8) | Stuff::Positive_Float_To_Byte (f);

		f = colors[offset2].blue * 255.99f;
		Clamp(f, 0.0f, 255.f);
		gos_points[2].argb = (gos_points[2].argb << 8) | Stuff::Positive_Float_To_Byte (f);
#endif

		return true;
	}

}

#endif
