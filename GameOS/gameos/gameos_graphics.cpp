#include "gameos.hpp"
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
// graphics
//
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

