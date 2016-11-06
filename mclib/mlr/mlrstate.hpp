//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRSTATE_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

namespace MidLevelRenderer {class MLRState;}

namespace GetHashFunctions {
	Stuff::IteratorPosition
		GetHashValue(const MidLevelRenderer::MLRState &value);
}

namespace MidLevelRenderer {

	class MLRSorter;
	class MLRTexturePool;

	//##########################################################################
	//########################    MLRState    ###############################
	//##########################################################################

	class MLRState
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
		friend class MLRSorter;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	protected:
		MLRState(
			Stuff::MemoryStream *stream,
			int version
		);

	public:
		MLRState();
		MLRState(const MLRState&);
		~MLRState() {};

		static MLRState*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);
		void
			Load(
				Stuff::MemoryStream *stream,
				int version
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Render state
	//
	public:
		enum {
			TextureNumberBit = 0,
			TextureNumberBits = Limits::Max_Number_Of_Texture_Bits,
			TextureMask =
				(0xFFFFFFFF >> (Stuff::INT_BITS - TextureNumberBits)) << TextureNumberBit,

			AlphaBit = TextureNumberBit + TextureNumberBits,
			AlphaBits = 3,
			AlphaMask =  (0xFFFFFFFF >> (Stuff::INT_BITS - AlphaBits)) << AlphaBit,

			FilterBit = AlphaBit + AlphaBits,
			FilterBits = 2,
			FilterMask = (0xFFFFFFFF >> (Stuff::INT_BITS - FilterBits)) << FilterBit,

			FogBit = FilterBit + FilterBits,
			FogBits = 2,
			FogMask = (0xFFFFFFFF >> (Stuff::INT_BITS - FogBits)) << FogBit,

			TextureWrapBit = FogBit + FogBits,
			TextureWrapMask = 1 << TextureWrapBit,

			DitherBit = TextureWrapBit+1,
			DitherMask = 1<<DitherBit,

			TextureCorrectionBit = DitherBit+1,
			TextureCorrectionMask = 1<<TextureCorrectionBit,

			WireFrameBit = TextureCorrectionBit + 1,
			WireFrameBits = 2,
			WireFrameMask =
				(0xFFFFFFFF >> (Stuff::INT_BITS - WireFrameBits)) << WireFrameBit,

			ZBufferWriteBit = WireFrameBit + WireFrameBits,
			ZBufferWriteMask = 1<<ZBufferWriteBit,

			ZBufferCompareBit = ZBufferWriteBit + 1,
			ZBufferCompareMask = 1<<ZBufferCompareBit,

			SpecularBit = ZBufferCompareBit + 1,
			SpecularMask = 1 << SpecularBit,

			FlatColoringBit = SpecularBit + 1,
			FlatColoringMask = 1 << FlatColoringBit,

			UsedRenderBits = FlatColoringBit + 1,
			UsedRenderMask = 0xFFFFFFFF >> (Stuff::INT_BITS - UsedRenderBits)
		};

		enum AlphaMode {
			OneZeroMode = 0,
			OneOneMode = 1 << AlphaBit,
			AlphaOneMode = 2 << AlphaBit,
			OneAlphaMode = 3 << AlphaBit,
			AlphaInvAlphaMode = 4 << AlphaBit,
			OneInvAlphaMode = 5 << AlphaBit,

			FirstAlphaAlphaMode = OneOneMode
		};

		enum FilterMode {
			NoFilterMode = 0,
			BiLinearFilterMode = 1 << FilterBit,
			TriLinearFilterMode = 2 << FilterBit
		};

		enum SpecularMode {
			SpecularOffMode = 0,
			SpecularOnMode = 1 << SpecularBit
		};

		enum TextureWrapMode {
			TextureWrap = 0,
			TextureClamp = 1 << TextureWrapBit
		};

		enum DitherMode {
			DitherOffMode = 0,
			DitherOnMode = 1 << DitherBit
		};

		enum TextureCorrectionMode {
			TextureCorrectionOffMode = 0,
			TextureCorrectionOnMode = 1 << TextureCorrectionBit
		};

		enum WireFrameMode {
			WireFrameOffMode = 0,
			WireFrameOnlyMode = 1 << WireFrameBit,
			WireFrameAddMode = 2 << WireFrameBit
		};

		enum ZBufferWriteMode {
			ZBufferWriteOffMode = 0,
			ZBufferWriteOnMode = 1 << ZBufferWriteBit
		};

		enum ZBufferCompareMode {
			ZBufferCompareOffMode = 0,
			ZBufferCompareOnMode = 1 << ZBufferCompareBit
		};

		enum FlatColoringMode {
			FlatColoringOffMode = 0,
			FlatColoringOnMode = 1 << FlatColoringBit
		};

// manipulate render state
		void
			SetTextureHandle(unsigned texture)
				{
					Check_Object(this); renderState &= ~TextureMask;
					renderDeltaMask |= TextureMask;
					Verify(texture <= TextureMask); renderState |= texture;
				}
		unsigned
			GetTextureHandle() const
				{Check_Object(this); return renderState & TextureMask;}

		void
			SetAlphaMode(AlphaMode alpha)
				{Check_Object(this); renderState &= ~AlphaMask; renderState |= alpha; renderDeltaMask |= AlphaMask;}
		AlphaMode
			GetAlphaMode() const
				{
					Check_Object(this);
					return static_cast<AlphaMode>(renderState & AlphaMask);
				}

		void
			SetFilterMode(FilterMode filter)
				{Check_Object(this); renderState &= ~FilterMask; renderState |= filter; renderDeltaMask |= FilterMask;}
		FilterMode
			GetFilterMode() const
				{
					Check_Object(this);
					return static_cast<FilterMode>(renderState & FilterMask);
				}

		void
			SetFogMode(int fog)
				{Check_Object(this); renderState &= ~FogMask; renderState |= (fog<<FogBit); renderDeltaMask |= FogMask;}
		int
			GetFogMode() const
				{Check_Object(this); return (renderState & FogMask)>>FogBit;}

		void
			SetSpecularOn()
				{Check_Object(this); renderState |= SpecularOnMode; renderDeltaMask |= SpecularMask;}
		void
			SetSpecularOff()
				{Check_Object(this); renderState &= ~SpecularOnMode; renderDeltaMask |= SpecularMask;}
		SpecularMode
			GetSpecularMode() const
				{Check_Object(this); return static_cast<SpecularMode>(renderState & SpecularMask);}

		void
			SetTextureWrapMode(TextureWrapMode TextureWrap)
				{Check_Object(this); renderState &= ~TextureWrapMask; renderState |= TextureWrap; renderDeltaMask |= TextureWrapMask;}
		TextureWrapMode
			GetTextureWrapMode() const
				{
					Check_Object(this);
					return static_cast<TextureWrapMode>(renderState & TextureWrapMask);
				}

		void
			SetDitherOn()
				{Check_Object(this); renderState |= DitherOnMode; renderDeltaMask |= DitherMask;}
		void
			SetDitherOff()
				{Check_Object(this); renderState &= ~DitherOnMode; renderDeltaMask |= DitherMask;}
		DitherMode
			GetDitherMode() const
				{Check_Object(this); return static_cast<DitherMode>(renderState & DitherMask);}

		void
			SetTextureCorrectionOn()
				{Check_Object(this); renderState |= TextureCorrectionOnMode; renderDeltaMask |= TextureCorrectionMask;}
		void
			SetTextureCorrectionOff()
				{Check_Object(this); renderState &= ~TextureCorrectionOnMode; renderDeltaMask |= TextureCorrectionMask;}
		TextureCorrectionMode
			GetTextureCorrectionMode() const
				{Check_Object(this); return static_cast<TextureCorrectionMode>(renderState & TextureCorrectionMask);}

		void
			SetWireFrameMode(WireFrameMode wire)
				{Check_Object(this); renderState &= ~WireFrameMask; renderState |= wire; renderDeltaMask |= WireFrameMask;}
		WireFrameMode
			GetWireFrameMode() const
				{
					Check_Object(this);
					return static_cast<WireFrameMode>(renderState & WireFrameMask);
				}

		void
			SetZBufferWriteOn()
				{Check_Object(this); renderState |= ZBufferWriteOnMode; renderDeltaMask |= ZBufferWriteMask;}
		void
			SetZBufferWriteOff()
				{Check_Object(this); renderState &= ~ZBufferWriteOnMode; renderDeltaMask |= ZBufferWriteMask;}
		ZBufferWriteMode
			GetZBufferWriteMode() const
				{Check_Object(this); return static_cast<ZBufferWriteMode>(renderState & ZBufferWriteMask);}

		void
			SetZBufferCompareOn()
				{Check_Object(this); renderState |= ZBufferCompareOnMode; renderDeltaMask |= ZBufferCompareMask;}
		void
			SetZBufferCompareOff()
				{Check_Object(this); renderState &= ~ZBufferCompareOnMode; renderDeltaMask |= ZBufferCompareMask;}
		ZBufferCompareMode
			GetZBufferCompareMode() const
				{Check_Object(this); return static_cast<ZBufferCompareMode>(renderState & ZBufferCompareMask);}

		void
			SetFlatColoringOn()
				{Check_Object(this); renderState |= FlatColoringOnMode; renderDeltaMask |= FlatColoringMask;}
		void
			SetFlatColoringOff()
				{Check_Object(this); renderState &= ~FlatColoringOnMode; renderDeltaMask |= FlatColoringMask;}
		FlatColoringMode
			GetFlatColoringMode() const
				{Check_Object(this); return static_cast<FlatColoringMode>(renderState & FlatColoringMask);}

		void
			SetRenderDeltaMask(int mask)
				{ Check_Object(this); renderDeltaMask = mask; }

		int
			GetRenderDeltaMask() const
				{ Check_Object(this); return renderDeltaMask; }

		void
			SetRenderPermissionMask(int mask)
				{ Check_Object(this); renderPermissionMask = mask; }

		int
			GetRenderPermissionMask() const
				{ Check_Object(this); return renderPermissionMask; }

		int
			GetRenderStateFlags() const
				{Check_Object(this); return renderState;}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Process state
	//
	public:
		enum {
			PriorityBit = 0,
			PriorityBits = 4,
			PriorityMask =
				(0xFFFFFFFF >> (Stuff::INT_BITS - PriorityBits)) << PriorityBit,

			BackFaceBit = PriorityBit + PriorityBits,
			BackFaceMask = 1<<BackFaceBit,

			LightingBit = BackFaceBit + 1,
			LightingBits = 5,
			LightingMask =
				(0xFFFFFFFF >> (Stuff::INT_BITS - LightingBits)) << LightingBit,

			MultitextureBit = LightingBit + LightingBits,
			MultitextureBits = 4,
			MultitextureMask =
				(0xFFFFFFFF >> (Stuff::INT_BITS - MultitextureBits)) << MultitextureBit,

			DrawNowBit = MultitextureBit + MultitextureBits,
			DrawNowMask = 1<<DrawNowBit,

			UsedProcessBits = DrawNowBit + 1,
			UsedProcessMask = 0xFFFFFFFF >> (Stuff::INT_BITS - UsedProcessBits)
		};

		enum BackFaceMode {
			BackFaceOffMode = 0,
			BackFaceOnMode = 1 << BackFaceBit,
		};

		enum {
			DefaultPriority = 0,
			AlphaPriority = 2,
			FirstApplicationPriority = 4,
			PriorityCount = 1<<PriorityBits,
		};

		enum LightingMode {
			LightingOffMode = 0,
			VertexLightingMode = 1 << LightingBit,
			LightMapLightingMode = 2 << LightingBit,
			LookupLightingMode = 4 << LightingBit,
			FaceLightingMode = 8 << LightingBit,
			TerrainLightingMode = 16 << LightingBit
		};

//	if cards are multi texture capable so use this mode
		enum MultiTextureMode {
			MultiTextureOffMode = 0,
			MultiTextureLightmapMode = 1 << MultitextureBit,
			MultiTextureSpecularMode = 2 << MultitextureBit,
		};

		enum DrawNowMode {
			DrawNowOffMode = 0,
			DrawNowOnMode = 1 << DrawNowBit,
		};

// manipulate process state
		void
			SetBackFaceOn()
				{Check_Object(this); processState |= BackFaceOnMode; processDeltaMask |= BackFaceMask;}
		void
			SetBackFaceOff()
				{Check_Object(this); processState &= ~BackFaceOnMode; processDeltaMask |= BackFaceMask;}
		BackFaceMode
			GetBackFaceMode() const
				{Check_Object(this); return static_cast<BackFaceMode>(processState & BackFaceOnMode);}

		void
			SetPriority(unsigned priority)
				{
					Check_Object(this); processState &= ~PriorityMask;
					processDeltaMask |= PriorityMask;
					Verify(priority < PriorityCount); processState |= priority;
				}
		unsigned
			GetPriority() const
				{Check_Object(this); return processState & PriorityMask;}

		void
			SetLightingMode(int lighting)
				{Check_Object(this); processState &= ~LightingMask; processState |= lighting; processDeltaMask |= LightingMask;}
		 
		int
			GetLightingMode() const
				{
					Check_Object(this);
					return static_cast<LightingMode>(processState & LightingMask);
				}

		void
			SetMultiTextureMode(MultiTextureMode multiTex)
				{Check_Object(this); processState &= ~MultitextureMask; processState |= multiTex; processDeltaMask |= MultitextureMask;}
		 
		MultiTextureMode
			GetMultiTextureMode() const
				{
					Check_Object(this);
					return static_cast<MultiTextureMode>(processState & MultitextureMask);
				}

		void
			SetDrawNowOn()
				{Check_Object(this); processState |= DrawNowOnMode; processDeltaMask |= DrawNowMask;}
		void
			SetDrawNowOff()
				{Check_Object(this); processState &= ~DrawNowOnMode; processDeltaMask |= DrawNowMask;}
		DrawNowMode
			GetDrawNowMode() const
				{Check_Object(this); return static_cast<DrawNowMode>(processState & DrawNowOnMode);}

		void
			SetProcessDeltaMask(int mask)
				{ Check_Object(this); processDeltaMask = mask; }

		int
			GetProcessDeltaMask() const
				{ Check_Object(this); return processDeltaMask; }

		void
			SetProcessPermissionMask(int mask)
				{ Check_Object(this); processPermissionMask = mask; }

		int
			GetProcessPermissionMask() const
				{ Check_Object(this); return processPermissionMask; }

		int
			GetProcessStateFlags() const
				{Check_Object(this); return processState;}

		void
			SetRendererState(MLRTexturePool*);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// System flags set at begin of every frame
	//

		enum {
			HasAGPAvailable = 1,				//	FALSE when no AGP memory available (assume a low end card)
			CanMultitextureLightMap = 2,		//	TRUE when single pass light mapping is available
			CanMultitextureSpecularMap = 4,		//	TRUE when single pass specular mapping is available
			HasMaxUVs = 8						//	TRUE if video card has certain limits on UVs
		};

		static void
			SetAGPAvailable(bool b)
				{ if(b==true) systemFlags |= HasAGPAvailable; else systemFlags &= ~HasAGPAvailable; }

		static void
			SetMultitextureLightMap(bool b)
				{ if(b==true) systemFlags |= CanMultitextureLightMap; else systemFlags &= ~CanMultitextureLightMap; }

		static void
			SetMultitextureSpecularMap(bool b)
				{ if(b==true) systemFlags |= CanMultitextureSpecularMap; else systemFlags &= ~CanMultitextureSpecularMap; }

		static void
			SetHasMaxUVs(bool b)
				{ if(b==true) systemFlags |= HasMaxUVs; else systemFlags &= ~HasMaxUVs; }

		static bool
			GetAGPAvailable()
				{ return (systemFlags & HasAGPAvailable)>0; }

		static bool
			GetMultitextureLightMap()
				{ return (systemFlags & CanMultitextureLightMap)>0; }

		static bool
			GetMultitextureSpecularMap()
				{ return (systemFlags & CanMultitextureSpecularMap)>0; }

		static bool
			GetHasMaxUVs()
				{ return (systemFlags & HasMaxUVs)>0; }

		static void
			SetMaxUV(float mUV)
				{ maxUV = mUV; SetHasMaxUVs(mUV > 0.0f); }

		static float
			GetMaxUV()
				{ return maxUV; }

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assignment operators
	//
		MLRState&
			operator=(const int &s)
				{Check_Pointer(this); renderState = s; return *this;}

		bool
			operator==(const MLRState &s) const
				{
					Check_Pointer(this);
					return
						renderState == s.renderState
						 && processState == s.processState;
				}

		bool
			operator>(const MLRState &s) const
				{
					Check_Pointer(this);
					return
						renderState > s.renderState
						 || (renderState == s.renderState
							 && processState > s.processState);
				}

		bool
			operator!=(const MLRState &s) const
				{
					Check_Pointer(this);
					return
						renderState != s.renderState
						 || processState != s.processState;
				}

		MLRState&
			Combine(
				const MLRState &master,
				const MLRState &slave
			);

		friend Stuff::IteratorPosition
			GetHashFunctions::GetHashValue(const MLRState &value);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const
				{}

	protected:
		int
			renderState,
			processState,
			renderDeltaMask,
			renderPermissionMask,
			processDeltaMask,
			processPermissionMask;

		static int
			systemFlags;
		static float
			maxUV;

#ifdef OLDFOG
		unsigned int
			fogColor;
		Stuff::Scalar
			fogDensity,
			nearFog,
			farFog;
#else
	public:
		static unsigned int
			fogColor;
#endif
	};
}
