#include "gameos.hpp"
#include "toolos.hpp"
#include "memorymanager.hpp" // goe_Heap
#include <stdio.h>
#include <time.h>
#include <stdlib.h> // rand
#include "strres.h"
#include <vector>

struct gosTextureInfo {
    int width_;
    int height_;
    gos_TextureFormat format_;
};

class gosTexture {
    public:
        gosTexture(gos_TextureFormat fmt, const char* fname, DWORD hints, BYTE* pdata, DWORD size)
        {
            format_ = fmt;
            if(fname) {
                filename_ = new char[strlen(fname)+1];
                strcpy(filename_, fname);
            } else {
                filename_ = 0;
            }
            hints_ = hints;
            
            size_ = size;
            pdata_ = new BYTE[size];
            memcpy(pdata_, pdata, size);

            is_locked_ = false;

            // TODO: correctly set width and height
            width_ = 16;
            height_ = 16;
            
        }
        ~gosTexture() {
            delete[] pdata_;
            delete[] filename_;
        }

        BYTE* Lock(int mipl_level, float is_read_only, int* pitch) {
            gosASSERT(is_locked_ == false);
            is_locked_ = true;
            // TODO:
            gosASSERT(pitch);
            *pitch = width_;
            return pdata_;
        }

        void Unlock() {
            gosASSERT(is_locked_ == true);
            is_locked_ = false;
        }

        void getTextureInfo(gosTextureInfo* texinfo) {
            gosASSERT(texinfo);
            texinfo->width_ = width_;
            texinfo->height_ = height_;
            texinfo->format_ = format_;
        }

    private:
        int width_;
        int height_;

        BYTE* pdata_;
        DWORD size_;

        gos_TextureFormat format_;
        char* filename_;
        DWORD hints_;

        bool is_locked_;
};

class gosRenderer {
    public:
        DWORD addTexture(gosTexture* texture) {
            gosASSERT(texture);
            textureList_.push_back(texture);
            return textureList_.size()-1;
        }

        gosTexture* getTexture(DWORD texture_id) {
            gosASSERT(textureList_.size() > texture_id);
            gosASSERT(textureList_[texture_id] != 0);
            return textureList_[texture_id];
        }

        void deleteTexture(DWORD texture_id) {
            // FIXME: bad use object list, with stable ids
            // to not waste space
            gosASSERT(textureList_.size() > texture_id);
            delete textureList_[texture_id];
            textureList_[texture_id] = 0;
        }

    private:
        std::vector<gosTexture*> textureList_;
};

static gosRenderer g_renderer;

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

void ExitGameOS()
{

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
// toolos.hpp
void __stdcall gos_FileSetReadWrite(char const* FileName)
{
}
DWORD __stdcall gos_FileSize(char const* FileName)
{
    return 0;
}
__int64 __stdcall gos_FileTimeStamp(char const* FileName)
{
    return 0;
}
// HGOSFILE = gosFileStream*
void __stdcall gos_OpenFile(gosFileStream**, char const* path, gosEnum_FileWriteStatus)
{
}
void __stdcall gos_CloseFile(gosFileStream* hfile)
{
}
bool __stdcall gos_DoesFileExist(char const* FileName)
{
    return false;
}
bool __stdcall gos_CreateDirectory(char const* FileName)
{
    return false;
}
char* __stdcall gos_FindDirectories(char const* DirectoryName)
{
    return NULL;
}
void __stdcall gos_FindDirectoriesClose()
{
}
char* __stdcall gos_FindDirectoriesNext()
{
    return NULL;
}
char* __stdcall gos_FindFiles(char const* PathFileName)
{
    return NULL;
}
void __stdcall gos_FindFilesClose()
{
}
char* __stdcall gos_FindFilesNext()
{
    return NULL;
}

void __stdcall gos_GetCurrentPath(char* Buffer, int buf_len)
{

}
DWORD __stdcall gos_ReadFile(HGOSFILE hfile, void* buf, DWORD size)
{
    return 0;
}

// gameos.hpp
void __stdcall gos_GetFile(char const* FileName, BYTE** MemoryImage, DWORD* Size)
{
}
DWORD __stdcall gos_WriteFile(HGOSFILE hfile, void const* buf, DWORD size)
{
    return 0;
}
////////////////////////////////////////////////////////////////////////////////

HSTRRES __stdcall gos_OpenResourceDLL(char const* FileName, const char** strings, int num)
{
    gos_StringRes* pstrres = new gos_StringRes();
    pstrres->strings = strings;
    pstrres->num_strings = num;
    return pstrres;
}
void __stdcall gos_CloseResourceDLL(HSTRRES handle)
{
    gos_StringRes* pstrres = new gos_StringRes();
    delete pstrres;
}
const char* __stdcall gos_GetResourceString(HSTRRES handle, DWORD id)
{
    static const char* dummy_string_res = "missing string res";
    gosASSERT(handle);
    gos_StringRes* pstrres = (gos_StringRes*)(handle);
    if(id > pstrres->num_strings) {
        fprintf(stderr, "Requested string id: %d not found, return dummy string\n", id);
        return dummy_string_res;
    }
    return pstrres->strings[id];
}

static const int g_hepsStackSize = 128;
static gos_Heap* g_heapsStack[g_hepsStackSize];
static int g_heapStackPointer = 0;

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
    g_heapsStack[g_heapStackPointer++] = Heap;
}
void __stdcall gos_PopCurrentHeap()
{
    gosASSERT(g_heapStackPointer > 0 && g_heapStackPointer < g_hepsStackSize);
    g_heapsStack[--g_heapStackPointer] = nullptr;
}

HGOSHEAP __stdcall gos_GetCurrentHeap()
{
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
DWORD __stdcall gos_GetKey()
{
    return 0;
}
void __stdcall gos_KeyboardFlush()
{

}
gosEnum_KeyStatus __stdcall gos_GetKeyStatus(gosEnum_KeyIndex index)
{
    return KEY_FREE;
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
void _stdcall gos_DrawLines(gos_VERTEX* Vertices, int NumVertices)
{
}
void _stdcall gos_DrawPoints(gos_VERTEX* Vertices, int NumVertices)
{
}
void _stdcall gos_DrawQuads(gos_VERTEX* Vertices, int NumVertices)
{
}
void _stdcall gos_DrawTriangles(gos_VERTEX* Vertices, int NumVertices)
{
}

////////////////////////////////////////////////////////////////////////////////

DWORD __stdcall gos_GetMachineInformation( MachineInfo mi, int Param1/*=0*/, int Param2/*=0*/, int Param3/*=0*/, int Param4/*=0*/)
{
    return 0;
}
////////////////////////////////////////////////////////////////////////////////

void __stdcall gos_GetMouseInfo( float* pXPosition, float* pYPosition, int* pXDelta, int* pYDelta, int* pWheelDelta, DWORD* pButtonsPressed )
{
}
////////////////////////////////////////////////////////////////////////////////

int __stdcall gos_rand()
{
    return rand();
}
////////////////////////////////////////////////////////////////////////////////

// graphics
//
void __stdcall gos_GetViewport( float* pViewportMulX, float* pViewportMulY, float* pViewportAddX, float* pViewportAddY )
{
}

HGOSFONT3D __stdcall gos_LoadFont( const char* FontFile, DWORD StartLine/* = 0*/, int CharCount/* = 256*/, DWORD TextureHandle/*=0*/)
{
    return NULL;
}
void __stdcall gos_DeleteFont( HGOSFONT3D Fonthandle )
{
}

DWORD __stdcall gos_NewEmptyTexture( gos_TextureFormat Format, const char* Name, DWORD HeightWidth, DWORD Hints/*=0*/, gos_RebuildFunction pFunc/*=0*/, void *pInstance/*=0*/)
{
    gosASSERT(0 && "Not implemented");
    return -1;
}
DWORD __stdcall gos_NewTextureFromMemory( gos_TextureFormat Format, const char* FileName, BYTE* pBitmap, DWORD Size, DWORD Hints/*=0*/, gos_RebuildFunction pFunc/*=0*/, void *pInstance/*=0*/)
{
    //gosASSERT(0 && "Not implemented");

    gosTexture* ptex = new gosTexture(Format, FileName, Hints, pBitmap, Size);
    return g_renderer.addTexture(ptex);
}
void __stdcall gos_DestroyTexture( DWORD Handle )
{
    //gosASSERT(0 && "Not implemented");
    g_renderer.deleteTexture(Handle);
}

void __stdcall gos_LockTexture( DWORD Handle, DWORD MipMapSize, bool ReadOnly, TEXTUREPTR* TextureInfo )
{
    // not implemented yet
    gosASSERT(MipMapSize == 0);
    int mip_level = 0; //func(MipMapSize);

    gosTextureInfo info;
    int pitch = 0;
    gosTexture* ptex = g_renderer.getTexture(Handle);
    ptex->getTextureInfo(&info);
    BYTE* pdata = ptex->Lock(mip_level, ReadOnly, &pitch);

    TextureInfo->pTexture = (DWORD*)pdata;
    TextureInfo->Width = info.width_;
    TextureInfo->Height = info.height_;
    TextureInfo->Pitch = pitch;
    TextureInfo->Type = info.format_;

    //gosASSERT(0 && "Not implemented");
}

void __stdcall gos_UnLockTexture( DWORD Handle )
{
    gosTexture* ptex = g_renderer.getTexture(Handle);
    ptex->Unlock();

    //gosASSERT(0 && "Not implemented");
}

void __stdcall gos_PushRenderStates()
{
}
void __stdcall gos_PopRenderStates()
{
}

void __stdcall gos_RenderIndexedArray( gos_VERTEX* pVertexArray, DWORD NumberVertices, WORD* lpwIndices, DWORD NumberIndices )
{
}

void __stdcall gos_RenderIndexedArray( gos_VERTEX_2UV* pVertexArray, DWORD NumberVertices, WORD* lpwIndices, DWORD NumberIndices )
{
}

void __stdcall gos_SetRenderState( gos_RenderState RenderState, int Value )
{
}
//void __stdcall gos_SetRenderState( gos_RenderState RenderState, DWORD Value ) // sebi
//{
//}

void __stdcall gos_SetScreenMode( DWORD Width, DWORD Height, DWORD bitDepth/*=16*/, DWORD Device/*=0*/, bool disableZBuffer/*=0*/, bool AntiAlias/*=0*/, bool RenderToVram/*=0*/, bool GotoFullScreen/*=0*/, int DirtyRectangle/*=0*/, bool GotoWindowMode/*=0*/, bool EnableStencil/*=0*/, DWORD Renderer/*=0*/)
{
}

void __stdcall gos_SetupViewport( bool FillZ, float ZBuffer, bool FillBG, DWORD BGColor, float top, float left, float bottom, float right, bool ClearStencil/*=0*/, DWORD StencilValue/*=0*/)
{
}

void __stdcall gos_TextDraw( const char *Message, ... )
{
}

void __stdcall gos_TextSetAttributes( HGOSFONT3D FontHandle, DWORD Foreground, float Size, bool WordWrap, bool Proportional, bool Bold, bool Italic, DWORD WrapType/*=0*/, bool DisableEmbeddedCodes/*=0*/)
{
}

void __stdcall gos_TextSetPosition( int XPosition, int YPosition )
{
}

void __stdcall gos_TextSetRegion( int Left, int Top, int Right, int Bottom )
{
}

void __stdcall gos_TextStringLength( DWORD* Width, DWORD* Height, const char *Message, ... )
{
}

// end graphics
////////////////////////////////////////////////////////////////////////////////

void __stdcall gos_TerminateApplication()
{
}
////////////////////////////////////////////////////////////////////////////////

// those are game specific callbacks (should move them outside to game)
int __cdecl InternalFunctionStop( const char* Message, ... )
{
    return 1;
}
int __cdecl InternalFunctionStop( const char* Message, const char* value)
{
    printf(Message, value);
    return 1;
}
int __cdecl InternalFunctionPause( const char* Message, ... )
{
    return 0;
}
void __cdecl InternalFunctionSpew( const char* Group, const char* Message, ... )
{
    fprintf(stderr, "Spew: %s", Group);
}
int __stdcall ErrorHandler( int Flags, const char* Text )
{
    fprintf(stderr, Text);
}
////////////////////////////////////////////////////////////////////////////////

bool mc2IsInDisplayBackBuffer = false;
bool mc2IsInMouseTimer = false;

////////////////////////////////////////////////////////////////////////////////

HGOSHEAP ParentClientHeap = NULL;

