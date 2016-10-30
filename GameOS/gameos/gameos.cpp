#include "gameos.hpp"
#include "toolos.hpp"
#include "memorymanager.hpp" // goe_Heap
#include <stdio.h>
#include <time.h>
#include <stdlib.h> // rand
#include "strres.h"

////////////////////////////////////////////////////////////////////////////////
void AddDebuggerMenuItem(char const*, bool (*)(), void (*)(), bool (*)(), DWORD (*)(char const*, DWORD))
{
    // TODO: maybe use dconsole for this
}

void AddStatistic( const char* Name, const char* TypeName, gosType Type, void* Value, DWORD Flags )
{
    printf("STATISTICS: [%s : %s]\n", Name, TypeName);
}
void StatisticFormat(char const* s)
{
    printf("STATISTICS: [%s]\n", s);
}
////////////////////////////////////////////////////////////////////////////////

void* DecodeJPG( const char* FileName, BYTE* Data, DWORD DataSize, DWORD* TextureWidth, DWORD* TextureHeight, bool TextureLoad, void *pDestSurf )
{
    return NULL;
}

gosEnvironment Environment;

////////////////////////////////////////////////////////////////////////////////
void EnterFullScreenMode(void)
{
    Environment.fullScreen = true;
}

void EnterWindowMode(void)
{
    Environment.fullScreen = false;
}

static bool g_gos_exit_game_os = false;

void ExitGameOS()
{
    g_gos_exit_game_os = true;
}

bool gosExitGameOS() {
    return g_gos_exit_game_os;
}

float frameRate = 30.0f; // apparently tiny geometry needs this

__int64 __stdcall GetCycles()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    int64_t v = ts.tv_sec * 1e+9;
    v += ts.tv_nsec;
    return v;
}

////////////////////////////////////////////////////////////////////////////////
DWORD __stdcall gos_GetMachineInformation( MachineInfo mi, int Param1/*=0*/, int Param2/*=0*/, int Param3/*=0*/, int Param4/*=0*/)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Creates a resource to be played later
//
void __stdcall gosAudio_CreateResource( HGOSAUDIO* hgosaudio, enum gosAudio_ResourceType,  const char* file_name, gosAudio_Format* ga_wf, void* data, int size, bool only2D)
{
    
}

//////////////////////////////////////////////////////////////////////////////////
// Destroy a resource; any sounds currently playing using the ResourceID will be
//  stopped.
//  Any memory the SoundAPI associated with the resource will be freed.
//
void __stdcall gosAudio_DestroyResource( HGOSAUDIO* hgosaudio )
{
}

//////////////////////////////////////////////////////////////////////////////////
// This prepares the channel for a specific type of sound playback. Optimally,
// allocate only the properties that will need modification. Use a bitwise'd group
// of gosAudio_Properties to set what is needed.
//
void __stdcall gosAudio_AllocateChannelSliders( int Channel, DWORD properties)
{
}

//////////////////////////////////////////////////////////////////////////////////
// Prepare a channel to play a resource of any type.
//
void __stdcall gosAudio_AssignResourceToChannel( int Channel, HGOSAUDIO hgosaudio)
{
}

//////////////////////////////////////////////////////////////////////////////////
// Get and Set functions only operate if a channel has the property enabled
//  Channel number -1 used in SetVolume and SetPanning will alter the windows master
//  volume and balance
void __stdcall gosAudio_SetChannelSlider( int Channel, enum gosAudio_Properties, float value1, float value2, float value3)
{
}
void __stdcall gosAudio_GetChannelSlider( int Channel, enum gosAudio_Properties, float* value1, float* value2, float* value3)
{
}

//////////////////////////////////////////////////////////////////////////////////
// Play, Loop, Stop, Pause, or Continue a particular channel
//
void __stdcall gosAudio_SetChannelPlayMode( int Channel, enum gosAudio_PlayMode ga_pm )
{
}
//////////////////////////////////////////////////////////////////////////////////
// Determine the current play mode of a channel
//
gosAudio_PlayMode __stdcall gosAudio_GetChannelPlayMode( int Channel )
{
}

////////////////////////////////////////////////////////////////////////////////

static const int g_hepsStackSize = 128;
static gos_Heap* g_heapsStack[g_hepsStackSize];
static int g_heapStackPointer = -1;

////////////////////////////////////////////////////////////////////////////////
HGOSHEAP __stdcall gos_CreateMemoryHeap(char const* HeapName, DWORD MaximumSize/* = 0*/, HGOSHEAP parentHeap/* = ParentClientHeap*/)
{
    gos_Heap* pheap = new gos_Heap();
    memset(pheap, 0, sizeof(gos_Heap));
    pheap->pParent = parentHeap;
    pheap->Magic = (DWORD)(reinterpret_cast<size_t>(HeapName) & 0xffffffff);
    strncpy(pheap->Name, HeapName, sizeof(pheap->Name)-1);
    pheap->Name[sizeof(pheap->Name)-1] = '\0';
#ifdef LAB_ONLY
    pheap->MaximumSize = MaximumSize;
#endif

    return pheap;
}
void __stdcall gos_DestroyMemoryHeap(HGOSHEAP Heap, bool shouldBeEmpty/* = true*/)
{
    // shouldBeEmpty ?? should Instances be zero or what?
    delete Heap;
    Heap = nullptr;
}
void __stdcall gos_PushCurrentHeap(HGOSHEAP Heap)
{
    gosASSERT(g_heapStackPointer < g_hepsStackSize);
    g_heapsStack[++g_heapStackPointer] = Heap;
}
void __stdcall gos_PopCurrentHeap()
{
    gosASSERT(g_heapStackPointer >= 0 && g_heapStackPointer < g_hepsStackSize);
    g_heapsStack[g_heapStackPointer--] = nullptr;
}

HGOSHEAP __stdcall gos_GetCurrentHeap()
{
    if(g_heapStackPointer == -1)
        return NULL;
    return g_heapsStack[g_heapStackPointer];
}

void __stdcall gos_WalkMemoryHeap(HGOSHEAP pHeap, bool vociferous/* = false*/)
{

}

////////////////////////////////////////////////////////////////////////////////

void* __cdecl operator new[](size_t size, HGOSHEAP Heap)
{
    return malloc(size);
}

void* __stdcall gos_Malloc(size_t bytes, HGOSHEAP Heap/* = 0*/)
{
    return malloc(bytes);
}
void __stdcall gos_Free(void* ptr)
{
    free(ptr);
}
////////////////////////////////////////////////////////////////////////////////

int __stdcall gos_rand()
{
    return rand() % (1<<15);
}
////////////////////////////////////////////////////////////////////////////////
void __stdcall gos_EnableIME(bool enabledDisabled)
{
}
void _stdcall gos_ToggleIME(bool activeInactive)
{
}
void _stdcall gos_GetIMEAppearance(gosIME_Appearance* pia)
{
}
void _stdcall gos_SetIMEAppearance(const gosIME_Appearance* pia)
{
}
void _stdcall gos_PositionIME(DWORD x, DWORD y)
{
}
void _stdcall gos_SetIMELevel(DWORD dwImeLevel)
{
}

////////////////////////////////////////////////////////////////////////////////

void __stdcall gos_TerminateApplication()
{
    g_gos_exit_game_os = true;
}

void __stdcall gos_AbortTermination()
{
    g_gos_exit_game_os = false;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool mc2IsInDisplayBackBuffer = false;
bool mc2IsInMouseTimer = false;

////////////////////////////////////////////////////////////////////////////////

HGOSHEAP ParentClientHeap = NULL;

