#pragma once
//===========================================================================//
// File:	 DirectX.hpp													 //
// Contents: Interface to DirectSetup										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//



extern bool	AllowFail;
extern volatile WORD FPUControl;	// Current FPU control word
extern WORD FPUDefault;				// Default FPU control word
char* ErrorNumberToMessage( int hResult );


//
// Blade GUID
//
DEFINE_GUID( IID_IDirect3DBladeDevice,    0x706e9520, 0x6db1, 0x11d2, 0xb9, 0x1, 0x0, 0x10, 0x4b, 0x36, 0xae, 0x4b);


//
// Used to save and restore FPU control word around DirectX functions
//
#if LAB_ONLY
#define SafeFPU()		_asm fldcw FPUDefault
#define GameOSFPU()		_asm fninit _asm fldcw FPUControl
#else
#define SafeFPU()
#define GameOSFPU()
#endif




//
// General DirectX
//
HRESULT wCoCreateInstance(REFCLSID rclsid,LPUNKNOWN pUnkOuter,DWORD dwClsContext,REFIID riid,LPVOID* ppv);
HRESULT wQueryInterface( IUnknown *iun, REFIID riid, LPVOID* obp );
ULONG wRelease( IUnknown *iun );
ULONG wAddRef( IUnknown *iun );

//
// Direct Setup
//
void wDirectXSetupGetVersion( DWORD* dwVersion,DWORD* dwRevision );

//
// IDirect3DDevice7
//
#ifdef LAB_ONLY
HRESULT wDrawPrimitive( IDirect3DDevice7* d3dDevice7, D3DPRIMITIVETYPE dptPrimitiveType, DWORD dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags );
HRESULT wDrawIndexedPrimitive( IDirect3DDevice7* d3dDevice7, D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags );
#else
#define wDrawPrimitive(d3dDevice7,dptPrimitiveType,dvtVertexType,lpvVertices,dwVertexCount,dwFlags ) d3dDevice7->DrawPrimitive( dptPrimitiveType,dvtVertexType,lpvVertices,dwVertexCount,dwFlags )
#define wDrawIndexedPrimitive(d3dDevice7,d3dptPrimitiveType,dwVertexTypeDesc,lpvVertices,dwVertexCount,lpwIndices,dwIndexCount,dwFlags ) d3dDevice7->DrawIndexedPrimitive(d3dptPrimitiveType,dwVertexTypeDesc,lpvVertices,dwVertexCount,lpwIndices,dwIndexCount,dwFlags )
#endif
HRESULT wDrawPrimitiveVB( IDirect3DDevice7* d3dDevice7, D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, DWORD dwFlags );
HRESULT wDrawIndexedPrimitiveVB( IDirect3DDevice7* d3dDevice7, D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags );
HRESULT wSetMaterial( IDirect3DDevice7* d3dDevice7, LPD3DMATERIAL7 lpMaterial );
HRESULT wSetLight( IDirect3DDevice7* d3dDevice7, DWORD dwLightIndex, LPD3DLIGHT7 lpLight );
HRESULT wLightEnable( IDirect3DDevice7* d3dDevice7, DWORD dwLightIndex, bool Enable );
HRESULT wMultiplyTransform( IDirect3DDevice7* d3dDevice7, D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix );
HRESULT wSetTransform( IDirect3DDevice7* d3dDevice7, D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix );
HRESULT wBeginScene( IDirect3DDevice7* d3dDevice7 );
HRESULT wEndScene( IDirect3DDevice7* d3dDevice7 );
HRESULT wGetCaps( IDirect3DDevice7* d3dDevice7, LPD3DDEVICEDESC7 lpD3DHWDevDesc );
HRESULT wSetRenderState( IDirect3DDevice7* d3dDevice7, D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState );
HRESULT wEnumTextureFormats( IDirect3DDevice7* d3dDevice7, LPD3DENUMPIXELFORMATSCALLBACK lpd3dEnumPixelProc, void* lpArg );
HRESULT wSetTexture( IDirect3DDevice7* d3dDevice7, DWORD dwStage, IDirectDrawSurface7* lpTexture );
HRESULT wSetTextureStageState( IDirect3DDevice7* d3dDevice7, DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue );
HRESULT wValidateDevice( IDirect3DDevice7* d3dDevice7, DWORD* ExtraPasses );
HRESULT wLoad( IDirect3DDevice7* d3dDevice7, IDirectDrawSurface7* lpDestTex, LPPOINT lpDestPoint, IDirectDrawSurface7* lpSrcTex, LPRECT lprcSrcRect, DWORD dwFlags );
HRESULT wSetViewport( IDirect3DDevice7* d3dDevice7, LPD3DVIEWPORT7 lpViewport );
HRESULT wClear( IDirect3DDevice7* d3dDevice7, DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, DWORD dwColor, D3DVALUE dvZ, DWORD dwStencil );

//
// IDirect3D7
//
HRESULT wCreateDevice( IDirect3D7* d3d7, REFCLSID rclsid, IDirectDrawSurface7* lpDDS, LPDIRECT3DDEVICE7* lplpD3DDevice7 );
HRESULT wEnumZBufferFormats( IDirect3D7* d3d7, REFCLSID riidDevice, LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback, void* lpContext );
HRESULT wEnumDevices( IDirect3D7* d3d7, LPD3DENUMDEVICESCALLBACK7 lpEnumDevicesCallback, LPVOID lpUserArg );
HRESULT wCreateVertexBuffer( IDirect3D7* d3d7, LPD3DVERTEXBUFFERDESC lpVBDesc, LPDIRECT3DVERTEXBUFFER7* lplpD3DVertexBuffer, DWORD dwFlags );

//
// IDirect3DVertexBuffer7
//
HRESULT wLock( IDirect3DVertexBuffer7* D3Dvb7, DWORD dwFlags, LPVOID* lplpData, LPDWORD lpdwSize );
HRESULT wUnlock( IDirect3DVertexBuffer7* D3Dvb7 );
HRESULT wOptimize( IDirect3DVertexBuffer7* D3Dvb7, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags );
HRESULT wProcessVertices( IDirect3DVertexBuffer7* D3Dvb7, DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags );

//
// Direct Draw
//
HRESULT wDirectDrawCreateEx( GUID* lpGUID, void** lplpDD, REFIID iid, IUnknown* pUnkOuter );
HRESULT wDirectDrawEnumerate( LPDDENUMCALLBACK lpCallback,LPDDENUMCALLBACKEX lpCallbackEx,LPVOID lpContext );

HRESULT wSetHWnd( LPDIRECTDRAWCLIPPER lpddc, DWORD dwFlags, HWND hWnd );

HRESULT wIsLost( IDirectDrawSurface7* lpdds7 );
HRESULT wGetDC( IDirectDrawSurface7* lpdds7, HDC* lphDC );
HRESULT wReleaseDC( IDirectDrawSurface7* lpdds7, HDC hDC );
HRESULT wSetColorKey( IDirectDrawSurface7* lpdd3, DWORD dwFlags, LPDDCOLORKEY lpDDColorKey );
HRESULT wLock( IDirectDrawSurface7* lpdd7, LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent );
HRESULT wUnlock( IDirectDrawSurface7* lpdds7, LPRECT lprect );
HRESULT wLock( IDirectDrawSurface* lpdds, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent );
HRESULT wUnlock( IDirectDrawSurface* lpdds, LPRECT lprect );
HRESULT wSetPalette( IDirectDrawSurface7* lpdd7, LPDIRECTDRAWPALETTE lpDDPalette );
HRESULT wRestore( IDirectDrawSurface7* lpdd7);
HRESULT wBlt( IDirectDrawSurface7* lpdd7, LPRECT lpDestRect, IDirectDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx );
HRESULT wBltFast( IDirectDrawSurface7* lpdd7, DWORD dwX, DWORD dwY, IDirectDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans );
HRESULT wFlip( IDirectDrawSurface7* lpdd7, IDirectDrawSurface7* lpDDSurfaceTargetOverride, DWORD dwFlags );
HRESULT wGetSurfaceDesc( IDirectDrawSurface7* lpdd7, LPDDSURFACEDESC2 lpDDSurfaceDesc );
HRESULT wGetSurfaceDesc( IDirectDrawSurface* lpdds, LPDDSURFACEDESC lpDDSurfaceDesc );
HRESULT wGetAttachedSurface( IDirectDrawSurface7* lpdds7, LPDDSCAPS2 lpDDSCaps2, IDirectDrawSurface7** lplpDDAttachedSurface );
HRESULT wGetPixelFormat( IDirectDrawSurface7* lpdds7, LPDDPIXELFORMAT lpDDPixelFormat );
HRESULT wAddAttachedSurface( IDirectDrawSurface7* lpdd7, IDirectDrawSurface7* lpDDSAttachedSurface );
HRESULT wDeleteAttachedSurface( IDirectDrawSurface7* lpdds7, DWORD dwFlags, IDirectDrawSurface7* lpDDSAttachedSurface );
HRESULT wSetClipper( IDirectDrawSurface7* lpdd7, LPDIRECTDRAWCLIPPER lpDDClipper );

HRESULT wGetDeviceIdentifier( IDirectDraw7* lpdd7, LPDDDEVICEIDENTIFIER2 lpdddi2, DWORD dwFlags );
HRESULT wGetCaps( IDirectDraw7* lpdd7, LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps );
HRESULT wCreatePalette( IDirectDraw7* lpdd7,DWORD dwFlags,LPPALETTEENTRY lpColorTable,LPDIRECTDRAWPALETTE* lplpDDPalette, IUnknown* pUnkOuter );
HRESULT wCreateSurface( IDirectDraw7* lpdd7, LPDDSURFACEDESC2 lpDDSurfaceDesc, IDirectDrawSurface7** lplpDDSurface7, IUnknown* pUnkOuter );
HRESULT wSetCooperativeLevel( IDirectDraw7* lpdd7, HWND hWnd, DWORD dwFlags );
HRESULT wEnumDisplayModes( IDirectDraw7* lpdd7,DWORD dwFlags,LPDDSURFACEDESC2 lpDDSurfaceDesc,LPVOID lpContext,LPDDENUMMODESCALLBACK2 lpEnumModesCallback );
HRESULT wRestoreDisplayMode( IDirectDraw7* lpdd7 );
HRESULT wGetDisplayMode( IDirectDraw7* lpdd7, LPDDSURFACEDESC2 lpDDSurfaceDesc );
HRESULT wCreateClipper( IDirectDraw7* lpdd7, DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter );
HRESULT wSetDisplayMode(IDirectDraw7* lpdd7,DWORD dwWidth,DWORD dwHeight,DWORD dwBPP,DWORD dwRefreshRate,DWORD dwFlags );
HRESULT wGetAvailableVidMem( IDirectDraw7* lpdd7, LPDDSCAPS2 lpDDSCaps2, LPDWORD lpdwTotal, LPDWORD lpdwFree );
HRESULT wGetFourCCCodes( IDirectDraw7* lpdd7, LPDWORD lpNumCodes, LPDWORD lpCodes );

//
// Direct Sound
//
HRESULT wDirectSoundEnumerate( LPDSENUMCALLBACK cb, void* vp );
HRESULT wDirectSoundCreate( LPGUID lpGuid, LPDIRECTSOUND* ppDS, IUnknown* pUnkOuter );
HRESULT wGetCaps( LPDIRECTSOUND ppDS, DSCAPS* dsc );
HRESULT	wSetSpeakerConfig( LPDIRECTSOUND ppDS, DWORD dsflags );
HRESULT	wGetSpeakerConfig( LPDIRECTSOUND ppDS, DWORD *dsflags );
HRESULT wLock( IDirectSoundBuffer* lpdsb, DWORD dwWriteCursor, DWORD dwWriteBytes, void** lplpvAudioPtr1, LPDWORD lpdwAudioBytes1, void** lplpvAudioPtr2, LPDWORD lpdwAudioBytes2, DWORD dwFlags );
HRESULT wUnlock( IDirectSoundBuffer* lpdsb, void* lplpvAudioPtr1, DWORD lpdwAudioBytes1, void* lplpvAudioPtr2, DWORD lpdwAudioBytes2 );
HRESULT wGetVolume( IDirectSoundBuffer* lpdsb, LONG* vol );
HRESULT wGetFrequency( IDirectSoundBuffer* lpdsb, DWORD* freq);
HRESULT wGetPan( IDirectSoundBuffer* lpdsb, LONG* pan);
HRESULT wSetVolume( IDirectSoundBuffer* lpdsb, LONG vol );
HRESULT wSetFrequency( IDirectSoundBuffer* lpdsb, DWORD freq);
HRESULT wSetPan( IDirectSoundBuffer* lpdsb, LONG pan);
HRESULT wPlay( IDirectSoundBuffer* lpdsb, DWORD res1, DWORD res2, DWORD flags);
HRESULT wStop( IDirectSoundBuffer* lpdsb);
HRESULT wGetStatus( IDirectSoundBuffer* lpdsb, LPDWORD status);
HRESULT wSetCurrentPosition( IDirectSoundBuffer* lpdsb, DWORD pos);
HRESULT wGetCurrentPosition( IDirectSoundBuffer* lpdsb, LPDWORD lpdwCurrentPlayCursor, LPDWORD lpdwCurrentWriteCursor );
HRESULT wCreateSoundBuffer( IDirectSound* lpds, LPCDSBUFFERDESC lpcDSBufferDesc, LPLPDIRECTSOUNDBUFFER lplpDirectSoundBuffer, IUnknown* pUnkOuter );
HRESULT wSetFormat( IDirectSoundBuffer* lpdsb, LPCWAVEFORMATEX lpcfxFormat );
HRESULT wGetCaps( LPDIRECTSOUNDBUFFER ppDSB, DSBCAPS* dsc );
HRESULT wSetCooperativeLevel( IDirectSound* lpds, HWND hwnd, DWORD dwLevel );
HRESULT wPlay( IDirectSoundBuffer* lpdsb, DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags );
HRESULT wSetRolloffFactor( IDirectSound3DListener* lpdsb, float dist, DWORD flag );
HRESULT wSetDopplerFactor( IDirectSound3DListener* lpdsb, float dist, DWORD flag );
HRESULT wSetDistanceFactor( IDirectSound3DListener* lpdsb, float dist, DWORD flag );
HRESULT wSetOrientation( IDirectSound3DListener* lpdsb, float fx, float fy, float fz, float tx, float ty, float tz, DWORD flag );
HRESULT wSetPosition( IDirectSound3DBuffer* lpdsb, float x, float y, float z, DWORD flag );
HRESULT wSetPosition( IDirectSound3DListener* lpdsb, float x, float y, float z, DWORD flag );
HRESULT wSetVelocity( IDirectSound3DBuffer* lpdsb, float x, float y, float z, DWORD flag );
HRESULT wSetVelocity( IDirectSound3DListener* lpdsb, float x, float y, float z, DWORD flag );
HRESULT wSetMinDistance( IDirectSound3DBuffer* lpdsb, float x, DWORD flag );
HRESULT wSetMaxDistance( IDirectSound3DBuffer* lpdsb, float x, DWORD flag );
HRESULT	wSetConeAngles( LPDIRECTSOUND3DBUFFER ppDSB3D, DWORD min, DWORD max, DWORD flag );
HRESULT	wSetConeOrientation( LPDIRECTSOUND3DBUFFER ppDSB3D, float x, float y, float z, DWORD flag );
HRESULT wCommitDeferredSettings( LPDIRECTSOUND3DLISTENER lpList );
HRESULT wQuerySupport( LPKSPROPERTYSET lpPS, REFGUID rGUID, ULONG uLong, ULONG* support );
HRESULT wSet( LPKSPROPERTYSET lpPS, REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData, ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength );
HRESULT wGetFormat( LPDIRECTSOUNDBUFFER lpdsb, LPWAVEFORMATEX lpwf, DWORD sizeIn, DWORD *sizeOut );
HRESULT wSetAllParameters(IDirectSound3DBuffer * lpdsb, DS3DBUFFER * ds3d, DWORD flag);
HRESULT wSetAllParameters(IDirectSound3DListener * lpdsb, DS3DLISTENER * ds3d, DWORD flag);
HRESULT wDuplicateSoundBuffer(IDirectSound* lpds, LPDIRECTSOUNDBUFFER src, LPDIRECTSOUNDBUFFER * dst );

//
// Direct Show
//
HRESULT wSetState( IMultiMediaStream* imms, STREAM_STATE NewState );
HRESULT wGetState( IMultiMediaStream* imms, STREAM_STATE * NewState );
HRESULT wSeek( IMultiMediaStream* imms, STREAM_TIME SeekTime );
HRESULT wGetTime( IMultiMediaStream* imms, STREAM_TIME* SeekTime );
HRESULT wGetDuration( IMultiMediaStream* imms, STREAM_TIME *theTime );
HRESULT wGetMediaStream( IMultiMediaStream* imms, REFMSPID idPurpose, IMediaStream** pMediaStream );

HRESULT wInitialize( IAMMultiMediaStream* iammms, STREAM_TYPE StreamType, DWORD dwFlags, IGraphBuilder* pFilterGraph );
HRESULT wAddMediaStream( IAMMultiMediaStream* iammms, IUnknown* pStreamObject, const MSPID *pPurposeID, DWORD dwFlags, IMediaStream** ppNewStream );
HRESULT wOpenFile( IAMMultiMediaStream* iammms, LPCWSTR pszFileName, DWORD dwFlags );

HRESULT wCreateSample( IDirectDrawMediaStream* iddms, IDirectDrawSurface* pSurface, const RECT *pRect, DWORD dwFlags, IDirectDrawStreamSample** ppSample );
HRESULT wGetSurface( IDirectDrawStreamSample* iddss, IDirectDrawSurface ** ppDirectDrawSurface, RECT* pRect );

HRESULT wSetFormat( IDirectDrawMediaStream* iddss, DDSURFACEDESC* ddsd );
HRESULT wGetFormat( IDirectDrawMediaStream* iddss, DDSURFACEDESC* ddsd_current, DDSURFACEDESC* ddsd_desired, DWORD* flags );
HRESULT wSetSampleTimes( IStreamSample* iddss, STREAM_TIME* st1, STREAM_TIME* st2 );
HRESULT wUpdate( IStreamSample* iss, DWORD flags, HANDLE event, PAPCFUNC papcfunc, DWORD apc );
HRESULT wGetFilterGraph( IAMMultiMediaStream* iamms, IGraphBuilder** igb );
HRESULT wSetPositions( IMediaSeeking* ims, STREAM_TIME* st, DWORD stflags, STREAM_TIME* end, DWORD eflags );
HRESULT wGetStopPosition( IMediaSeeking* ims, STREAM_TIME* st );
HRESULT wGetSampleTimes( IDirectDrawStreamSample* iddss, STREAM_TIME* st, STREAM_TIME* end, STREAM_TIME* cur );

HRESULT wPut_CurrentPosition( IMediaPosition* iba, REFTIME value );
HRESULT wGet_CurrentPosition( IMediaPosition* iba, REFTIME *value );
HRESULT wCanSeekForward( IMediaPosition* iba, LONG * canDo );
HRESULT wGet_Duration( IMediaPosition* iba, REFTIME *value );
HRESULT wStop( IMediaControl* iba );
HRESULT wPause( IMediaControl* iba );
HRESULT wRun( IMediaControl* iba );
HRESULT wPut_Volume( IBasicAudio* iba, LONG value );
HRESULT wPut_Balance( IBasicAudio* iba, LONG value );
HRESULT wGetEndOfStreamEventHandle(IMultiMediaStream* imms, HANDLE * ph);
HRESULT wGetInformation( IMultiMediaStream* imms, DWORD * flags, STREAM_TYPE *theType );

//
// Direct Input
//
HRESULT wDirectInputCreateEx( HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, void** ppvOut, LPUNKNOWN punkOuter );
HRESULT wCreateDeviceEx( IDirectInput7* lpdi, REFGUID rguid, REFIID riid, void** lplpDirectInputDevice, LPUNKNOWN pUnkOuter );
HRESULT wSetDataFormat( IDirectInputDevice7* dev, LPCDIDATAFORMAT lpdf );
HRESULT wSetCooperativeLevel( IDirectInputDevice7* dev, HWND hwnd, DWORD dwFlags );
HRESULT wUnacquire( IDirectInputDevice7* dev );
HRESULT wAcquire( IDirectInputDevice7* dev );
HRESULT wGetDeviceState( IDirectInputDevice7* dev, DWORD cbData, LPVOID lpvData );
HRESULT wEnumDevices( IDirectInput7* di, DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags );
HRESULT wSetProperty( IDirectInputDevice7* dev, REFGUID rguidProp, LPCDIPROPHEADER pdiph );
HRESULT wGetDeviceInfo( IDirectInputDevice7* dev, LPDIDEVICEINSTANCE pdidi );
HRESULT wGetCapabilities( IDirectInputDevice7* dev, LPDIDEVCAPS lpDIDevCaps );
HRESULT wGetDeviceData( IDirectInputDevice7* dev, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags );
HRESULT wPoll( IDirectInputDevice7* lpdid2 );
HRESULT wCreateEffect( IDirectInputDevice7* lpdid2, REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter );
HRESULT wStart( IDirectInputEffect* lpeff, DWORD iterations, DWORD flags );
HRESULT wStop( IDirectInputEffect* lpeff );
HRESULT wGetEffectStatus( IDirectInputEffect* lpeff, DWORD * status );
HRESULT wDownload( IDirectInputEffect* lpeff );
HRESULT wSetParameters( IDirectInputEffect* lpeff, LPCDIEFFECT lpdi, DWORD flags );



//
// DirectPlay
//
HRESULT wAddPlayerToGroup( IDirectPlay4A* dplay4, DPID idGroup, DPID idPlayer );
HRESULT wCancelMessage( IDirectPlay4A* dplay4, DWORD dwMsgID, DWORD dwFlags );
HRESULT wCancelPriority( IDirectPlay4A* dplay4, DWORD dwMinPriority, DWORD dwMaxPriority, DWORD dwFlags );
HRESULT wClose( IDirectPlay4A* dplay4 );
HRESULT wCreateGroup( IDirectPlay4A* dplay4, LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags );
HRESULT wCreateGroupInGroup( IDirectPlay4A* dplay4, DPID idParentGroup, LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags );
HRESULT wCreatePlayer( IDirectPlay4A* dplay4, LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags );
HRESULT wDeleteGroupFromGroup( IDirectPlay4A* dplay4, DPID idParentGroup, DPID idGroup );
HRESULT wDeletePlayerFromGroup( IDirectPlay4A* dplay4, DPID idGroup, DPID idPlayer );
HRESULT wDestroyGroup( IDirectPlay4A* dplay4, DPID idGroup );
HRESULT wDestroyPlayer( IDirectPlay4A* dplay4, DPID idPlayer );
HRESULT wEnumConnections( IDirectPlay4A* dplay4, LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags );
HRESULT wEnumGroupPlayers( IDirectPlay4A* dplay4, DPID idGroup, LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumPlayersCallback2, LPVOID lpContext, DWORD dwFlags );
HRESULT wEnumGroups( IDirectPlay4A* dplay4, LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumPlayersCallback2, LPVOID lpContext, DWORD dwFlags );
HRESULT wEnumGroupsInGroup( IDirectPlay4A* dplay4, DPID idGroup, LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumCallback, LPVOID lpContext, DWORD dwFlags );
HRESULT wEnumPlayers( IDirectPlay4A* dplay4, LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumPlayersCallback2, LPVOID lpContext, DWORD dwFlags );
HRESULT wEnumSessions( IDirectPlay4A* dplay4, LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags );
HRESULT wGetCaps( IDirectPlay4A* dplay4, LPDPCAPS lpDPCaps, DWORD dwFlags );
HRESULT wGetGroupConnectionSettings( IDirectPlay4A* dplay4, DWORD dwFlags, DPID idGroup, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wGetGroupData( IDirectPlay4A* dplay4, DPID idGroup, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags );
HRESULT wGetGroupFlags( IDirectPlay4A* dplay4, DPID idGroup, LPDWORD lpdwFlags );
HRESULT wGetGroupName( IDirectPlay4A* dplay4, DPID idGroup, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wGetGroupOwner( IDirectPlay4A* dplay4, DPID idGroup, LPDPID lpidOwner );
HRESULT wGetGroupParent( IDirectPlay4A* dplay4, DPID idGroup, LPDPID lpidParent );
HRESULT wGetMessageCount( IDirectPlay4A* dplay4, DPID idPlayer, LPDWORD lpdwCount );
HRESULT wGetMessageQueue( IDirectPlay4A* dplay4, DPID idFrom, DPID idTo, DWORD dwFlags, LPDWORD lpdwNumMsgs, LPDWORD lpdwNumBytes );
HRESULT wGetPlayerAccount( IDirectPlay4A* dplay4, DPID idPlayer, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wGetPlayerAddress( IDirectPlay4A* dplay4, DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wGetPlayerCaps( IDirectPlay4A* dplay4, DPID idPlayer, LPDPCAPS lpPlayerCaps, DWORD dwFlags );
HRESULT wGetPlayerData( IDirectPlay4A* dplay4, DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags );
HRESULT wGetPlayerFlags( IDirectPlay4A* dplay4, DPID idPlayer, LPDWORD lpdwFlags );
HRESULT wGetPlayerName( IDirectPlay4A* dplay4, DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wGetSessionDesc( IDirectPlay4A* dplay4, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wInitialize( IDirectPlay4A* dplay4, LPGUID lpGUID );
HRESULT wInitializeConnection( IDirectPlay4A* dplay4, LPVOID lpConnection, DWORD dwFlags );
HRESULT wOpen( IDirectPlay4A* dplay4, LPDPSESSIONDESC2 lpsd, DWORD dwFlags );
HRESULT wReceive( IDirectPlay4A* dplay4, LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wSecureOpen( IDirectPlay4A* dplay4, LPCDPSESSIONDESC2 lpsd, DWORD dwFlags, LPCDPSECURITYDESC lpSecurity, LPCDPCREDENTIALS lpCredentials );
HRESULT wSendChatMessage( IDirectPlay4A* dplay4, DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage );
HRESULT wSend( IDirectPlay4A* dplay4, DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize );
HRESULT wSendEx( IDirectPlay4A* dplay4, DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID );
HRESULT wSetGroupConnectionSettings( IDirectPlay4A* dplay4, DWORD dwFlags, DPID idGroup, LPDPLCONNECTION lpConnection );
HRESULT wSetGroupData( IDirectPlay4A* dplay4, DPID idGroup, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags );
HRESULT wSetGroupName( IDirectPlay4A* dplay4, DPID idGroup, LPDPNAME lpGroupName, DWORD dwFlags );
HRESULT wSetGroupOwner( IDirectPlay4A* dplay4, DPID idGroup, DPID idOwner );
HRESULT wSetPlayerData( IDirectPlay4A* dplay4, DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags );
HRESULT wSetPlayerName( IDirectPlay4A* dplay4, DPID idPlayer, LPDPNAME lpPlayerName, DWORD dwFlags );
HRESULT wSetSessionDesc( IDirectPlay4A* dplay4, LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags );
HRESULT wStartSession( IDirectPlay4A* dplay4, DWORD dwFlags, DPID idGroup );



// DirectPlay Lobby functions

HRESULT wCreateAddress( IDirectPlayLobby3A* dplobby3, REFGUID guidSP, REFGUID guidDataType, LPCVOID lpData, DWORD dwDataSize, LPVOID lpAddress, LPDWORD lpdwAddressSize );
HRESULT wCreateCompoundAddress( IDirectPlayLobby3A* dplobby3, LPDPCOMPOUNDADDRESSELEMENT lpElements, DWORD dwElementCount, LPVOID lpAddress, LPDWORD lpdwAddressSize );
HRESULT wEnumAddress( IDirectPlayLobby3A* dplobby3, LPDPENUMADDRESSCALLBACK lpEnumAddressCallback, LPCVOID lpAddress, DWORD dwAddressSize, LPVOID lpContext );
HRESULT wGetConnectionSettings( IDirectPlayLobby3A* dplobby3, DWORD dwAppID, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wReceiveLobbyMessage( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, DWORD dwAppID, LPDWORD lpdwMessageFlags, LPVOID lpData, LPDWORD lpdwDataSize );
HRESULT wRegisterApplication( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, LPDPAPPLICATIONDESC lpAppDesc );
HRESULT wRunApplication( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, LPDWORD lpdwAppID, LPDPLCONNECTION lpConn, HANDLE hReceiveEvent );
HRESULT wSendLobbyMessage( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, DWORD dwAppID, LPVOID lpData, DWORD dwDataSize );
HRESULT wSetConnectionSettings( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, DWORD dwAppID, LPDPLCONNECTION lpConn );
HRESULT wSetLobbyMessageEvent( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, DWORD dwAppID, HANDLE hReceiveEvent );
HRESULT wUnregisterApplication( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, REFGUID guidApplication );
HRESULT wWaitForConnectionSettings( IDirectPlayLobby3A* dplobby3, DWORD dwFlags );
HRESULT wConnectEx( IDirectPlayLobby3A* dplobby3, DWORD dwFlags, REFIID riid, LPVOID *lplpDP, IUnknown* pUnk );
