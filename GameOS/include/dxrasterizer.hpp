#pragma once
//===========================================================================//
// File:	 DXRasterizer.hpp												 //
// Contents: DirectDraw Manager routines									 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//




//
// VideoCard.cpp functions
//
void FindVideoCards();


void CreateCopyBuffers();
void RestoreAreas();
void DestroyDirtyRectangles();
void InvalidateVertexBuffers();
void ReCreateVertexBuffers();

//
// IME displays directly to backbuffer, that's why it's in DXRasterizer.hpp
//
void RenderIMEToBackBuffer(int POS_X, int POS_Y, float FONTHEIGHT);

//
// Variables for gamma correction
//
extern BYTE GammaTable[256];
extern float GammaSetting;
extern bool UseGammaCorrection;
extern DWORD GlobalWidth;
extern DWORD GlobalHeight;
extern bool GlobalZBuffer;
extern bool GlobalFullScreen;
HRESULT CALLBACK CheckEnumProc( DDPIXELFORMAT* ddpfPixelFormat, LPVOID lpContext );
extern DWORD ValidTextures;
extern void CopyBackBuffer( IDirectDrawSurface7* Dest, IDirectDrawSurface7* Source );
extern bool ModeChanged;
extern bool DebuggerUsed;
extern DDSURFACEDESC2 BackBufferddsd;
extern DWORD AllowBrightness;
extern DWORD AllowContrast;
//
// Compatibility flags for different video cards
//
enum
{
	SceneRenderer=1,				// Scene based renderer (PowerVR, Kyro etc...)
	NoAlphaTest=1<<1,				// Disable alpha test (conflicts with other modes)
	NoTriLinear=1<<2,				// Do not attempt trilinear filtering
	NoTripleBuffer=1<<3,			// Do not attempt triple buffering
	DefaultBltNotFlip=1<<4,			// In full screen mode, use BLT
	NoFog=1<<5,						// Do not attempt fog
	TextureOffset=1<<6,				// Riva 128 needs a texture offset for fonts
	BrokenAlphaTest=1<<7,			// ATI Rage cards cannot do Alpha test without ALPHABLEND being enabled
	RenderEqualZ=1<<8,				// Render bit depth must equal Z bit depth (nVidia cards)
	NoParallelism=1<<9,				// Lock the back buffer after rendering (Voodoo3 - queueing up too many blits)
	BadMouse=1<<10,					// Card can never used GDI mouse
	LowEndCard=1<<11,				// This is a 'low end' card - the application can request this is not supported in hardware (Environment.DisableLowEndCard)
	NoMipMap=1<<12,					// Disable mipmaps
	Bad4444=1<<13,					// Disable 4444 textures  (Chromatic Mpact)
	BadAGP=1<<14,					// Disable AGP textures   (Chromatic Mpact)
	NoDisablePerspective=1<<15,		// Disable perspective correct disable.
	LimitTextureSize=1<<16,			// Limit texture size to 256*256 to correct bad caps
	ForceSquareTextures=1<<17,		// Do not allow non-square textures
	NoStretchTexture=1<<18,			// Force system memory stretch blit to square up textures
	AllowLowMemoryCard=1<<19,		// Allow card to bypass low texture memory test (Voodoo2)
	NoMultitexture=1<<20,			// Disable multitexture blending modes
	Disable1024x768=1<<21,			// Disable 1024*768 mode
	DisableTextureDeletes=1<<22,	// Try not to delete textures
	SlowDisplayModeChange=1<<23,    // Prevent video drivers that take a while before sending the WM_DISPLAYCHANGE msg from getting stuck in an endless loop of display mode changes
	DisableColorControl=1<<24,		// Disable gamma and color controls
	Limit500VidTextures=1<<25,		// Limit video memory textures to 500
	Bad8888=1<<26,					// Disable 32 bit textures
	Disable32BitRender=1<<27,		// Disable 32 bit rendering
};

//
// This is the current cards compatiblity flags
//
extern DWORD Compatibility3D;

//
// Used to store all known video cards
//
typedef struct
{
	DWORD VendorID;						// Vendor ID, ie: S3=0x5333
	DWORD DeviceID;						// Device ID, ie: ATI Rage=0x4750

	DWORD DriverH;						// High and Low dword of driver version number (Windows 9x)
	DWORD DriverL;

	DWORD NTDriverH;					// High and Low dword of driver version number (Windows 2000)
	DWORD NTDriverL;

	char* VendorName;					// Friendly name for Vendor

	char* CardName;						// Friendly name for card type

	DWORD CompatibilityFlags;			// Flags that may alter how GameOS behaves

} CardInfo;

extern CardInfo *KnownCards;

//
// Used to store information about video cards in machine
//
typedef struct
{
	GUID				DeviceGUID;

	DDDEVICEIDENTIFIER2	DDid;					// Use DDid.szDescription for the readable name

	DWORD				Empty;					// Bug in GetDeviceIdentifier - it overwrites the next 4 bytes!
	
	DDCAPS				DDCaps;

	D3DDEVICEDESC7		D3DCaps;

	char				FourCC[16*4];			// First 16 FourCC codes

	DDPIXELFORMAT		TextureFormats[16];		// First 16 texture formats (ends in 0)

	WORD				Modes16[16*2];			// First 16, 16 bit screen modes above 640*480
	WORD				Refresh16[16];			// Refresh rates for above modes

	WORD				Modes32[16*2];			// First 16, 32 bit screen modes above 640*480
	WORD				Refresh32[16];			// Refresh rates for above modes

	DDPIXELFORMAT		ZFormats[16];			// First 16 Z Buffer formats

	DWORD				TotalVid;
	DWORD				TotalLocalTex;
	DWORD				TotalTex;

	DWORD				MultitextureLightMap;			// Lightmap support
	DWORD				MultitextureSpecularMap;		// Specular support
	DWORD				MultitextureDetail;				// Detail support
	DWORD				MultitextureLightMapFilter;		// Lightmap support (1=Disable trilinear)
	DWORD				MultitextureSpecularMapFilter;	// Specular support (1=Disable trilinear)
	DWORD				MultitextureDetailFilter;		// Detail support (1=Disable trilinear)

	DWORD				CurrentCard;			// Which entry in the KnownCard array. (-1=Unknown)

	bool				Failed60Hz;				// Set to only try full screen 60Hz once
	bool				LinearMemory;			// Set if linear memory, not rectangular
	int					MaxRefreshRate;			// Maximum refresh rate

	char*				CurrentVendor;			// If vendor known, point to name, else 0

} DeviceInfo;

extern DeviceInfo	DeviceArray[8];
extern int			HardwareRenderer;			// Set when using a hardware renderer
extern char DisplayInfoText[128];

extern DWORD TripleBuffer;						// Copied from Environment for current card (so can be disabled on 3Dfx)
extern DWORD StencilActive;
extern DDSURFACEDESC2 BBddsd;
extern DDSURFACEDESC2 ZBddsd;

extern IDirectDraw7*			DDobject;
extern IDirectDraw7*			CurrentDDobject;
extern IDirectDrawSurface7*		FrontBufferSurface;
extern IDirectDrawSurface7*		BackBufferSurface;
extern IDirectDrawSurface7*		ZBufferSurface;
extern IDirectDrawSurface7*		g_RefBackBuffer;
extern IDirectDrawSurface7*		g_CopyBackBuffer;
extern IDirectDrawClipper*		g_lpDDClipper;
extern IDirectDrawSurface7*		CopyTarget;
extern IDirectDrawSurface7*		CopyZBuffer;
extern IDirectDrawGammaControl*	GammaControlInterface;

extern IDirect3D7*				d3d7;
extern IDirect3DDevice7*		d3dDevice7;				// Current D3D Device
extern IDirect3DDevice7*		Refd3dDevice7;			// Reference D3D Device
extern IDirect3DDevice7*		Maind3dDevice7;			// Selected D3D Device
extern IDirect3DDevice7*		RenderDevice;
extern bool						InsideBeginScene;
extern bool						NeedToInitRenderStates;
extern int						HardwareRenderer;
extern DWORD					BGColor;
extern D3DDEVICEDESC7			CapsDirect3D;

extern D3DVIEWPORT7				viewData;
extern float					gosViewportMulX,gosViewportAddX,gosViewportMulY,gosViewportAddY;
extern DWORD					NumDevices;
extern DWORD					NumHWDevices;
extern DWORD					NumMonitors;
extern DWORD gosColorTable[];
extern DWORD DepthResults[8];
extern DWORD DepthPixels;

extern void InitRenderer();
extern char* GetDirectXVersion();

//
// MipMapping flags
//
extern DWORD HasBiLinear;
extern DWORD HasTriLinear;
extern int HasTLHAL;
//
// Card specific changes triggered by these values
//
extern DWORD HasVertexBlending;
extern DWORD HasClamp;
extern DWORD HasFog;
extern DWORD HasAlphaTest;
extern DWORD HasAlphaModes;
extern DWORD HasMultitextureLightmap;
extern DWORD HasMultitextureSpecularmap;
extern DWORD HasMultitextureDetailTexture;
extern DWORD HasMultitextureLightmapFilter;
extern DWORD HasMultitextureSpecularmapFilter;
extern DWORD HasMultitextureDetailTextureFilter;

void DisplayBackBuffer();
void End3DScene();

void RenderWithReferenceRasterizer( DWORD Type );
void DirectDrawInstall();
void ShowFrameGraphs();
void DirectDrawUninstall();
void DirectDrawCreateDDObject();
BOOL CALLBACK DirectDrawEnumerateCallback( GUID* lpGUID, LPSTR, LPSTR, LPVOID lpContext );
HRESULT CALLBACK DirectDrawEnumDisplayModesCallback( LPDDSURFACEDESC2 pddsd, LPVOID lpContext );
void DirectDrawCreateAllBuffers();
void EnterWindowMode();
void EnterFullScreenMode();
void RenderIMEtoScreen();


