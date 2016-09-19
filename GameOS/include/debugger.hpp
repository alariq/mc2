#pragma once
//===========================================================================//
// File:	 Debugger.hpp													 //
// Contents: Debugger														 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
// Size of scroll back buffer for spew
//
#define DebuggerBufferSize 128*1024

extern gos_VERTEX pTextVertices[80];
extern DWORD TextVertex;
extern DWORD WantAA;
extern DWORD Want32;
extern DWORD Want32T;
extern DWORD WantRes;
extern DWORD WantRVRAM;
extern DWORD WantSW;
extern DWORD WantSW1;
extern DWORD WantHW;
extern DWORD FrameAdvance;							// Used to step or fast forward through scripts
extern DWORD OldFreeze;								// Old value of gFreezeLogic
extern DWORD OldRender;								// Old value of gStopRendering;
extern DWORD PerfCounterSelected;
extern int PerfYStart,PerfYEnd;
extern DWORD DoUpdateWindow;
extern DWORD DoingAdvance;
extern DWORD OldgStopSystem;
extern DWORD OldgGameLogicActive;
extern bool MenuActive;
extern bool SubMenuActive;
extern bool	DebounceMenu;
extern DWORD OldStop;
extern DWORD OldRendering;
extern int AreaL, AreaR, AreaT, AreaB;
extern DWORD PolygonsInArea;
extern float DebuggerTexelOffset;
extern int DBWheelDelta;
extern DWORD gShowMemoryUseage;						// 2=Enable at next frame, 1=show data, 0=Do not show data
extern DWORD gShowWSUseage;							// 2=Enable at next frame, 1=show data, 0=Do not show data
extern DWORD gShowFTOLUseage;						// 2=Enable at next frame, 1=show data, 0=Do not show data
extern DWORD gShowALLUseage;
extern gos_VERTEX Graph[512];
extern DWORD gScreenBMP;

void WalkStack( DWORD* RoutineAddresses, DWORD NumberOfLevels, DWORD IgnoreLevels );
char* DecodeAddress( DWORD Address, bool brief = true );
void DrawLines( int X1, int Y1, int X2, int Y2, DWORD Color );

typedef struct _MenuItem
{
	_MenuItem*	pNext;
	_MenuItem* pSubMenu;							// Pointer to sub menu list, or NULL
	DWORD(__stdcall *Callback)(char* Name, DWORD MenuFunction);
	void (*Routine)();								// Sub menu list pointer
	bool (*Greyed)();
	bool (*CheckMark)();							// 0 When no check mark routine, 1 when sub menu
	char* FullName;
	char* Name;
} MenuItem;



extern MenuItem*	pDebuggerMenu;
extern MenuItem*	pRasterizerMenu;
extern MenuItem*	pLibrariesMenu;
extern MenuItem*	pGameMenu;
//
extern MenuItem*	CurrentMenu;
extern MenuItem*	MenuHighlight;
extern MenuItem**	pCurrentMenu;
extern void CleanUpDebugger();
extern void InitDebuggerMenus();

#ifdef LAB_ONLY
//
// Pie chart data
//
	extern gos_VERTEX PieData[103];	
#endif


//
// Array of textures in texture heap display
//
typedef struct _TextureHeap
{
	_TextureHeap*	pNext;
	int				X1;
	int				Y1;
	int				X2;
	int				Y2;
	DWORD			Handle;

} TextureHeap;

extern TextureHeap* pTextureHeap;
extern DWORD SelectedHandle;
extern DWORD TextureToUnload;
bool CheckWindow();

//
// Main Debugger Screens
//

enum EDbgScreen
{
	DbgS_Stat,					// 0
	DbgS_Spew,					// 1
	DbgS_Texture,				// 2
	DbgS_Network,				// 3
	DbgS_Sound,					// 4
	DbgS_Control,				// 5
	DbgS_UsedTextures,			// 6
	DbgS_UnusedTextures,		// 7
	DbgS_Memory,				// 8
	DbgS_CPUStats,				// 9
	DbgS_PlayHistory,			// 10
	DbgS_SoundResource,			// 11
	DbgS_NetworkX,				// 12
	DbgS_File,					// 13
	DbgS_SoundSpatial,			// 14
	DbgS_LoadedTextures,		// 15
	DbgS_AllTextures,			// 16
	DbgS_UsedTextureDetail,		// 17
	DbgS_UnusedTextureDetail,	// 18
	DbgS_LoadedTextureDetail,	// 19
	DbgS_AllTextureDetail,		// 20
	DbgS_AllTextureNoUpload,	// 21
	DbgS_TextureLog,			// 22
};
extern  EDbgScreen DebugDisplay;


//
// Framegraph information
//
typedef enum {
	Graph_Chart=0,
	Graph_30,
	Graph_Pie30,
	Graph_60,
	Graph_Pie60,
} FrameGraphMode;

extern FrameGraphMode GraphMode;
extern DWORD ShowColorInfo;




//
// True when the debugger window is visible on the display
//
extern bool DebuggerActive;
extern bool InDebugger;						// During debugger rendering
extern bool ProcessMemorySize;				// When true will calculate each processes memory size in the exception handler (can take about 1 second!)
extern int ZoomMode;						// Zoom screen around cursor
extern int BaseZoom;
extern DWORD gForceNoFlip;					// Force blt instead of flip in fullscreen
extern DWORD gShowAverage;
extern DWORD gShowGraphsAsTime;
extern DWORD gShowGraphBackground;
extern DWORD gNoGraph;
extern DWORD WhichImage;					// Original / Special mode image?
//
// Scroll back buffer for Dumplog (128K)
//
extern char DebuggerBuffer[DebuggerBufferSize];
extern DWORD CurrentDebugSpewPos;		// Current position in buffer
extern DWORD CurrentDebugTopOfScreen;	// Current top of screen
//
// Alpha values to added to debugger window
//
extern DWORD DebuggerAlpha;
//
// Mouse position
//
extern int DBMouseX,DBMouseY;
extern DWORD DBMouseMoved;
extern int ExMouseX,ExMouseY;
extern DWORD DBButtons;

extern DWORD gEnableMulti;
extern DWORD gForceMono;
extern DWORD gForceFlat;
extern DWORD gForceBlend;
extern DWORD gForceGouraud;
extern DWORD gForcePerspective;
extern DWORD gForceNoPerspective;
extern DWORD gForceSpecular;
extern DWORD gForceNoSpecular;
extern DWORD gForceAlpha;
extern DWORD gShowVertexData;
extern DWORD gForceNoFog;
extern DWORD gForceNoTextures;
extern DWORD gForceBiLinear;
extern DWORD gForceTriLinear;
extern int	 gForceMipBias;
extern DWORD gForceNoFiltering;
extern DWORD gForceNoDithering;
extern DWORD gForceDithering;
extern DWORD gForceChessTexture;
extern DWORD gForceNoAlphaTest;
extern DWORD gForceAlphaTest;
extern DWORD ChessTexture;				// The chess texture
extern DWORD MipColorTexture;			// The mipmap colored texture
extern DWORD gForceAlphaBlending;
extern DWORD gUseGameSpew;
extern DWORD gControlsActive;					// Allow controls for the game during the debugger
extern DWORD gGameLogicActive;					// Allow game logic to run when debugger up
extern DWORD gStopSystem;						// Stop whole system in debugger
extern DWORD gFrameGraph;
extern DWORD gStopGameRendering;
extern DWORD gStopRendering;					// Stop rendering
extern DWORD gFreezeLogic;						// Stop rendering
extern DWORD gShowLFControls;
extern bool NoDebuggerStats;						// When 0 Stats and spews are active during GameOS functions like debugger
extern DWORD gEnableRS;
extern DWORD gTextureOverrun;
extern DWORD gDisableLighting;
extern DWORD gDisableNormalizing;
extern DWORD gDisableHardwareTandL;
extern DWORD gDisableVertexBlending;

//
// Debugger window variables
//
extern int DbTopX,DbTopY;
extern int DbMaxX,DbMaxY,DbMinX;

extern DWORD TopStatistics;

//
// Debugger window constants
//
#define DbSizeX 489
#define DbSizeY 342


extern int CurrentX,CurrentY;		// Current pixel position
extern int DbChrX,DbChrY;			// Current character x,y
extern int StartX,StartY;			// Current start of line



void UpdateDebugger();
void InitDebugger();
void EndRenderMode();
void DestroyDebugger();
void SpewToDebugger( char* Message );
int GetMipmapUsed( DWORD Handle, gos_VERTEX* Pickv1, gos_VERTEX* Pickv2, gos_VERTEX* Pickv3 );
void ShowFrameGraphs();
void UpdateDebugMouse();
void UpdateDebugWindow();
void InitTextDisplay();
void DrawText( DWORD Color, char* String );
void DrawSquare( int TopX, int TopY, int Width, int Height, DWORD Color );
void DrawChr( char Chr );














