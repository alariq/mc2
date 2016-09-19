//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//###############################    MLRState    ##################################
//#############################################################################

int MLRState::systemFlags;
float MLRState::maxUV;

#ifndef OLDFOG
	unsigned int
		MLRState::fogColor;
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRState::MLRState(
	MemoryStream *stream,
	int version
)
{
	Verify(gos_GetCurrentHeap() == Heap);
	Check_Pointer(this);
	Check_Object(stream);
	Load(stream, version);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRState::MLRState()
{
	renderState = renderDeltaMask = 0;
	renderPermissionMask = 0xFFFFFFFF;

	processState = processDeltaMask = 0;
	processPermissionMask = 0xFFFFFFFF;

#ifdef OLDFOG
	fogColor = 0xffffffff;
	fogDensity = 0.0f;
	nearFog = 1.0f;
	farFog = 100.0f;
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRState::MLRState(const MLRState& mState)
{
	Check_Object(&mState);

	renderState = mState.renderState;
	renderDeltaMask = mState.renderDeltaMask;
	renderPermissionMask = mState.renderPermissionMask;

	processState = mState.processState;
	processDeltaMask = mState.processDeltaMask;
	processPermissionMask = mState.processPermissionMask;

#ifdef OLDFOG
	nearFog = mState.nearFog;
	farFog = mState.farFog;
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRState*
	MLRState::Make(
		MemoryStream *stream,
		int version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLRState *state = new MLRState(stream, version);
	gos_PopCurrentHeap();

	return state;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRState::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	//
	//-----------------------------
	// Save the renderState to the stream
	//-----------------------------
	//
	*stream << renderState << renderDeltaMask << renderPermissionMask;
	*stream << processState << processDeltaMask << processPermissionMask;

#if OLDFOG
	*stream << nearFog << farFog << fogDensity << fogColor;
#endif

	if (renderState&TextureMask)
	{
		MLRTexture *texture = (*MLRTexturePool::Instance)[this];
		Check_Object(texture);
		MString name = texture->GetTextureName();

		int hint = texture->GetHint();
		hint <<= 4;
		hint |= texture->GetTextureInstance();

		*stream << name << hint;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRState::Load(
		MemoryStream *stream,
		int version
	)
{
	Check_Object(this);
	Check_Object(stream);

	//
	//-------------------------------------
	// Load the renderState from the stream
	//-------------------------------------
	//
	*stream >> renderState >> renderDeltaMask >> renderPermissionMask;
	*stream >> processState >> processDeltaMask >> processPermissionMask;
	

#ifdef OLDFOG
		*stream >> nearFog >> farFog;
#else
	if(version < 6)
	{
		Scalar dummy;
		*stream >> dummy >> dummy;
	}
#endif

	if (version > 3)
	{
#ifdef OLDFOG
		*stream >> fogDensity >> fogColor;
#else
		if(version < 6)
		{
			int dummy1;
			Scalar dummy2;

			*stream >> dummy2 >> dummy1;
		}
#endif
	}
	else
	{
#ifdef OLDFOG
		fogDensity = 0.0f;
		fogColor = 0;
#endif
		if (renderDeltaMask & FogMask)
			renderDeltaMask |= FogMask;
		renderState &= ~(TextureWrapMask|SpecularMask);
		renderDeltaMask &= ~(TextureWrapMask|SpecularMask);
		renderPermissionMask |= TextureWrapMask|SpecularMask;
	}

	if (renderState & TextureMask)
	{
		MString name;
		int instance;

		*stream >> name;
		*stream >> instance;

		Check_Object(MLRTexturePool::Instance);
		MLRTexture *texture = (*MLRTexturePool::Instance)(name, instance);
		if (!texture)
		{
			texture = MLRTexturePool::Instance->Add(name, instance);
		}
		Check_Object(texture);

		texture->SetHint(instance>>4);
		instance &= 0xf;

		SetTextureHandle(texture->GetTextureHandle());
		Verify((GetTextureHandle()&7) == 1);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRState&
	MLRState::Combine(
		const MLRState &master,
		const MLRState &slave
	)
{
	Check_Pointer(this);
	Check_Object(&master);
	Check_Object(&slave);

	renderPermissionMask = master.renderPermissionMask & slave.renderPermissionMask;
	renderDeltaMask = slave.renderDeltaMask & master.renderPermissionMask;
	renderState =
		(master.renderState&~renderDeltaMask) | (slave.renderState&renderDeltaMask);

	processPermissionMask = master.processPermissionMask & slave.processPermissionMask;
	processDeltaMask = slave.processDeltaMask & master.processPermissionMask;
	processState = (master.processState&~processDeltaMask) | (slave.processState&processDeltaMask);

#ifdef OLDFOG
	unsigned fog_mode = renderDeltaMask & FogMask;
	if (fog_mode)
	{
		fogDensity = slave.fogDensity;
		nearFog = slave.nearFog;
		farFog = slave.farFog;
		
		if ((renderState&fog_mode) == OverrideFogAndColorMode)
		{
			fogColor = slave.fogColor;
		}
		else
		{
			nearFog = master.nearFog;
			farFog = master.farFog;
			fogColor = master.fogColor;
		}
	}
	else
	{
		fogColor = master.fogColor;
		fogDensity = master.fogDensity;
		nearFog = master.nearFog;
		farFog = master.farFog;
	}
#endif

	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
void
	MLRState::SetFogData (
		unsigned fog_color,
		Scalar fog_density,
		Scalar near_fog,
		Scalar far_fog
	)
{
	fogColor = fog_color;
	fogDensity = fog_density;
	nearFog = near_fog;
	farFog = far_fog;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRState::SetFogData (
		RGBAColor fog_color,
		Scalar fog_density,
		Scalar near_fog,
		Scalar far_fog
	)
{
	SetFogData(GOSCopyColor(&fog_color), fog_density, near_fog, far_fog);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRState::GetFogData (
		RGBAColor *fog_color,
		Scalar *fog_density,
		Scalar *near_fog,
		Scalar *far_fog
	) const
{
	*fog_color =
		RGBAColor(
			((fogColor>>16)&0xFF)/255.0f,
			((fogColor>>8)&0xFF)/255.0f,
			(fogColor&0xFF)/255.0f,
			((fogColor>>24)&0xFF)/255.0f
		);
	*fog_density = fogDensity;
	*near_fog = nearFog;
	*far_fog = farFog;
}
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRState::SetRendererState(MLRTexturePool *texturePool)
{
	Check_Object(this);

	Check_Object(texturePool);

	if( renderState & MLRState::TextureMask )
		gos_SetRenderState( 
			gos_State_Texture, 
			(*texturePool)[renderState & MLRState::TextureMask]->GetImage(NULL)->GetHandle()
		);
	else
		gos_SetRenderState( gos_State_Texture, 0 );

	switch( renderState&MLRState::AlphaMask )
	{
		case MLRState::OneZeroMode:
			gos_SetRenderState(gos_State_AlphaTest, 0);
			gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_OneZero);
			break;

		case MLRState::OneOneMode:
			gos_SetRenderState(gos_State_AlphaTest, 0);
			gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_OneOne);
			break;

		case MLRState::AlphaOneMode:
			SPEW(("micgaert", "MLRState::AlphaOneMode not available anymore"));
			break;

		case MLRState::OneAlphaMode:
			SPEW(("micgaert", "MLRState::OneAlphaMode not available anymore"));
			break;

		case MLRState::AlphaInvAlphaMode:
			gos_SetRenderState(gos_State_AlphaTest, 1);
			gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
			break;

		case MLRState::OneInvAlphaMode:
			gos_SetRenderState(gos_State_AlphaTest, 0);
			gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_OneInvAlpha);
			break;

		default:
			STOP(("Unknown alpha state"));
	}

	switch( renderState&MLRState::FilterMask )
	{
		case MLRState::NoFilterMode:
			gos_SetRenderState(gos_State_Filter, gos_FilterNone);
			break;

		case MLRState::BiLinearFilterMode:
			gos_SetRenderState(gos_State_Filter, gos_FilterBiLinear);
			break;

		case MLRState::TriLinearFilterMode:
			gos_SetRenderState(gos_State_Filter, gos_FilterTriLinear);
			break;

		default:
			STOP(("Unknown filter state"));
	}

	gos_SetRenderState(	gos_State_Dither, renderState & MLRState::DitherOnMode );

	if( renderState & MLRState::WireFrameOnlyMode )
	{
		DWORD wfColor=0xffffff;
        // sebi wtf???
		//gos_SetRenderState(	gos_State_WireframeMode, (DWORD)&wfColor );
		gos_SetRenderState(	gos_State_WireframeMode, wfColor );
	}
	else
	{
		gos_SetRenderState(	gos_State_WireframeMode, 0 );
	}

	gos_SetRenderState(	gos_State_Perspective, renderState & MLRState::TextureCorrectionOnMode );

	if (renderState & MLRState::FogMask)
    {
        // sebi ???
		//gos_SetRenderState(	gos_State_Fog, (int)&fogColor);
		gos_SetRenderState(	gos_State_Fog, (int)fogColor);
	}
	else
	{
		gos_SetRenderState(	gos_State_Fog, 0);
	}

	gos_SetRenderState(
		gos_State_ShadeMode,
		gos_ShadeGouraud
	);

	gos_SetRenderState(
		gos_State_ZWrite,
		(renderState & MLRState::ZBufferWriteMask) ? 1 : 0
	);

	gos_SetRenderState(	gos_State_Specular, renderState & MLRState::SpecularOnMode );

	gos_SetRenderState(	
		gos_State_TextureAddress, 
		(renderState & MLRState::TextureClamp) ? gos_TextureClamp : gos_TextureWrap
	);

	gos_SetRenderState( gos_State_ZCompare,(renderState & MLRState::ZBufferCompareMask) ? 1 : 0 );

	gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);
}

		Stuff::IteratorPosition
			GetHashFunctions::GetHashValue(const MLRState &value)
				{
					Verify(sizeof(Stuff::IteratorPosition) == sizeof(DWORD));
					return
						(
							((value.processState & MidLevelRenderer::MLRState::UsedProcessMask) << MidLevelRenderer::MLRState::UsedRenderBits)
							 | (value.renderState & MidLevelRenderer::MLRState::UsedRenderMask)
						) & 0x7FFFFFFF;
				}
