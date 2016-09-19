#pragma once
//===========================================================================//
// File:	 DirectXDebugging.hpp											 //
// Contents: DirectX Debug routines											 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//



//
// DirectX Debugging support
//
char* GetSBStatus( DWORD Status );
char* GetQSupport( ULONG Support );
char* GetLightInfo( LPD3DLIGHT7 lpLight );
char* GetMaterialInfo( LPD3DMATERIAL7 lpMaterial );
char* GetTransformType( D3DTRANSFORMSTATETYPE dtstTransformStateType );
char* GetApplyFlag( DWORD flag );
char* GetPropertyItem( REFGUID rguid, LONG Item );
char* GetPropertySet( REFGUID rguid );
char* GetSurfaceCaps( LPDDSCAPS2 lpDDSCaps2 );
char* GetInputDevice( DWORD Size );
char* GetEnumPlayersInfo( LPGUID lpguidInstance, DWORD Flags );
char* GetConnectionInfo( void* ConnectionInfo );
char* GetNetworkMessage( NetPacket* pMessage );
char* GetSendExInfo( DPID idFrom, DPID idTo, DWORD dwFlags, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID );
char* GetSendInfo( DPID idFrom, DPID idTo, DWORD dwFlags, DWORD dwDataSize );
char* GetReceivedData( LPDPID lpidFrom, LPVOID lpData, LPDWORD lpdwDataSize );
char* GetReceiveFlags( DWORD dwFlags, LPDPID lpidFrom, LPDPID lpidTo );
char* GetPlayerName( LPDPNAME lpPlayerName );
char* GetCreatePlayerFlags( DWORD dwFlags );
char* GetEnumSessionFlags( DWORD Timeout, DWORD Flags );
char* GetDPSessionDesc2( LPDPSESSIONDESC2 desc );
char* GetDPOpenFLags( DWORD Flags );
char* GetVertexType( char* Buffer, DWORD VertexType );
char* GetChanProp( DWORD Prop );
char* GetWaveFormat( char* Buffer, LPCWAVEFORMATEX lpcfxFormat );
char* GetDSoundCoopLevel( DWORD Level );
char* GetFlipFlags( DWORD Flags );
char* GetStageSet( D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue );
char* GetRectangle( char* Buffer, LPRECT lp );
char* GetDIDevice( REFGUID rguid );
char* GetRenderState( DWORD RenderState, DWORD Value );
char* GetDrawPrimitiveData( D3DPRIMITIVETYPE dptPrimitiveType, DWORD dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags );
char* GetDrawIndexedPrimitiveData( D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags );
char* GetClearArea( DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags );
char* GetSurfaceDescription( LPDDSURFACEDESC2 pSD );
char* GetSurfaceDescriptionOld( LPDDSURFACEDESC pSD );
char* GetGosPixelFormat( LPDDPIXELFORMAT pf );
char* GetSetCooperativeLevelFlags( DWORD dwFlags );
char* GetCreatePaletteFlags( DWORD dwFlags );
char* GetEnumDisplayModeFlags( DWORD dwFlags );
char* GetSetColorKeyFlags( DWORD dwFlags );
char* GetLockFlags( DWORD dwFlags );
char* Get3DDevice( REFCLSID rclsid );
char* GetReturnInterface( REFIID riid );
char* GetANSIfromUNICODE( const LPCWSTR Unicode );
char* GetBltInformation( IDirectDrawSurface7* lpdd7, LPRECT lpDestRect, IDirectDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx );
char* GetBltFastInformation( IDirectDrawSurface7* lpdd7, DWORD dwX, DWORD dwY, IDirectDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans );
char* GetDIEnumFlags(DWORD flags);
char* GetDICoopFlags(DWORD flags);
char* GetDIGetDeviceDataFlags(DWORD dwFlags);
char* GetDIInputFormat( LPCDIDATAFORMAT Format ); 
char* GetDIDevType( DWORD DevType );
char* DSGetStreamType(STREAM_TYPE StreamType);
char* DSGetInitFlags(DWORD dwFlags);
char* DSGetUpdateFlags(DWORD dwFlags);
char* GetDSSpeakerConfig(DWORD dsFlags);
char* GetDSBDFlags(DWORD dsFlags);
char* GetDShowSeekType(DWORD seekFlag);
char* GetEffectFlags(DWORD flags);
