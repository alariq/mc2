#pragma once
//===========================================================================//
// File:	 3DRasterizer.hpp												 //
// Contents: Low level 3D rasterizer										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//




typedef struct _SAVESTATE
{
	_SAVESTATE* pNext;
	DWORD		SaveState[gos_MaxState];
} SAVESTATE;

extern SAVESTATE*	pStateStack;
extern DWORD		StackDepth;




//
// Structure to hold information about vertex buffers created
//
typedef struct _VertexBuffer
{
	DWORD					Magic;					// Magic number to verify handle valid
	gosVERTEXTYPE			VertexType;				// Type of vertex the buffer contains
	_VertexBuffer*			pNext;
	IDirect3DVertexBuffer7*	vBuffer;				// D3D handle
	#ifdef LAB_ONLY
	IDirect3DVertexBuffer7*	DebugvBuffer;			// D3D handle to a system memory vertex buffer for debugging D3DTLVERTEX data
	IDirect3DVertexBuffer7*	CopyvBuffer;			// D3D handle to a system memory vertex buffer for debugging (Readable copy of D3DVERTEX data)
	#endif
	DWORD					NumberVertices;			// Number of vertices when created
	bool					Locked;					// True when locked
	bool					WantOptimize;			// True to optimize after the next unlock
	bool					Optimized;				// True once buffer is optimized
	bool					Lost;					// True when mode changed and buffer invalid
	void*					Pointer;				// Pointer to start of data when locked

} VertexBuffer;



//
// Variables that effect rendering
//
extern WORD QuadIndex[192];
extern DWORD AlphaInvAlpha;			// Set when alpha blend mode is AlphaInvAlpha

extern bool ViewPortChanged;			// Set when game changes viewport
extern DWORD InUpdateRenderers;		// True when in 'Update Renderers'
extern DWORD DrawingPolys;			// Current polygon
extern DWORD gCulledTriangles;		// Number of culled triangles
extern DWORD CulledMax;
extern DWORD DrawingMax;			// Maximum polys to draw
extern DWORD DrawingHighest;
extern DWORD gForceNoClear;
extern DWORD VertexBuffersLocked;
extern DWORD gDisableLinesPoints;
extern DWORD gDisablePrimitives;


//
// Render to texture variables
//
void InitRenderToTexture();
void DestroyRenderToTexture();



//
// RenderStates.cpp
//
void FlushRenderStates();
extern bool DirtyStates;
extern BYTE UpdatedState[gos_MaxState];


//
// 3DRasterizer.hpp
//
void InitRenderStates();
void InitRenderer();
void Save3DState();
void Restore3DState();
void ReInit3D();
void Destroy3D();
void CheckVertices( gos_VERTEX* pVertexArray, DWORD NumberVertices, bool PointsLines=0 );
void CheckVertices2( gos_VERTEX_2UV* pVertexArray, DWORD NumberVertices );
void CheckVertices3( gos_VERTEX_3UV* pVertexArray, DWORD NumberVertices );
void DebugTriangle( gos_VERTEX* v1, gos_VERTEX* v2, gos_VERTEX* v3 );
void DebugTriangle_2UV( gos_VERTEX_2UV* v1, gos_VERTEX_2UV* v2, gos_VERTEX_2UV* v3 );
void DebugTriangle_3UV( gos_VERTEX_3UV* v1, gos_VERTEX_3UV* v2, gos_VERTEX_3UV* v3 );
DWORD GetMipmapColor( int Mipmap );
void gos_ClipDrawQuad( gos_VERTEX* pVertices );


//
// Statistics
//
extern DWORD	NumSpecular;
extern DWORD	NumPerspective;
extern DWORD	NumAlpha;
extern DWORD	NumTextured;
extern DWORD	PrimitivesRendered;
extern DWORD	PointsRendered;
extern DWORD	LinesRendered;
extern DWORD	TrianglesRendered;
extern DWORD	QuadsRendered;
extern DWORD	IndexedTriangleCalls;
extern float	IndexedTriangleLength;
extern DWORD	IndexedVBTriangleCalls;
extern float	IndexedVBTriangleLength;
extern DWORD	LastSpecularPrim;
extern DWORD	LastPerspectivePrim;
extern DWORD	LastAlphaPrim;
extern DWORD	LastTexturePrim;
extern DWORD	LastFilterPrim;
extern DWORD	NumPointSampled;
extern DWORD	NumBilinear;
extern DWORD	NumTrilinear;
extern DWORD	LastZComparePrim;
extern DWORD	NumZCompare;
extern DWORD	LastZWritePrim;
extern DWORD	NumZWrite;
extern DWORD	LastAlphaTestPrim;
extern DWORD	NumAlphaTest;
extern DWORD	LastDitheredPrim;
extern DWORD	NumDithered;
extern DWORD	LastCulledPrim;
extern DWORD	NumCulled;
extern DWORD	LastTextureAddressPrim;
extern DWORD	NumWrapped;
extern DWORD	NumClamped;
extern DWORD	LastShadePrim;
extern DWORD	NumFlat;
extern DWORD	NumGouraud;
extern DWORD	LastBlendPrim;
extern DWORD	NumDecal;
extern DWORD	NumModulate;
extern DWORD	NumModulateAlpha;
extern DWORD	LastMonoPrim;
extern DWORD	NumMono;
extern DWORD	LastFogPrim;
extern DWORD	NumFog;
extern DWORD	LastClipped;
extern DWORD	NumGuardBandClipped;
extern DWORD	NumClipped;




extern gos_VERTEX PickZoom[3];


//
// Capabilities of the mode
//
extern int HasGuardBandClipping;
extern float MinGuardBandClip;		// Minimum guard band clip
extern float MaxGuardBandClip;		// Maximum guard band clip
extern int HasAGP;					// AGP memory available?
extern int HasSpecular;				// Specular available
extern int HasDither;				// Dithering available
extern int HasAntiAlias;			// Can do sort independant antialias
extern int HasMipLodBias;			// MipMap LOD bias can be altered
extern int HasDecal;				// Can DECAL
extern int HasModulateAlpha;		// Can MODULATEALPHA
extern int HasAdd;					// Can ADD
extern int HasMipMap;				// Can do mip mapping

extern int	 HasMaxUV;				// There is a maximum UV coord for this render mode (and texture)
extern float MaxUCoord;			// The maximum u coord (for current texture)//
extern float MinUCoord;			// The minimum u coord (for current texture)// Various renderstates based on the capabilities of the current video card
extern float MaxVCoord;			// The maximum v coord (for current texture)//
extern float MinVCoord;			// The minimum v coord (for current texture)// Various renderstates based on the capabilities of the current video card

//
// Various modes the renderer can be in
//
typedef enum
{
	Normal=0,						// Normal rendering
	WireframeGlobalColor,			// Only wireframe, no texture, global wireframe color
	WireframeGlobalTexture,			// Global wireframe color ontop of texture
	ExamineScene,					// Examine, zoom into scene mode
	ExamineMipmaps,					// Examine Mipmaps mode
	ExamineRenderer,				// Compare with reference rasterizer
	ExamineDepth,					// Examine depth complexity
	ExamineTriangleSize,			// Examine triangle size
	ExamineTriangleSize1,			// Examine triangle size and only draw 1 or 2 pixel triangles
	ExamineArea,					// Examine an area of the screen
	ExamineDrawing,					// Examine scnene being drawn
	ShowTriangleColors,				// Show each triangle as a different color
	ExamineColor,					// Examine scene, show color information
	ExamineMipmaps2,				// Examine mipmaps using colored mipmap (alter u,v)
} RenderModeType;

extern RenderModeType RenderMode;	// In a special debugger mode



extern DWORD RenderStates[gos_MaxState];			// Current User settings
extern DWORD PreviousStates[gos_MaxState];			// Previously set renderstates

//
// Various renderstates based on the capabilities of the current video card
//
extern DWORD	ModeMagBiLinear;		// MAG mode for Bilinear
extern DWORD	ModeMinNone;			// MIN mode for non filtering
extern DWORD	ModeMinBiLinear;		// MIN mode for bilinear
extern DWORD	ModeMinTriLinear;		// MIN mode for trilinear



