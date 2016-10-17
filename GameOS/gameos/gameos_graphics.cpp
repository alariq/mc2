#include "gameos.hpp"
#include "font3d.hpp"
#include <vector>
#include <map>

#ifdef LINUX_BUILD
#include <cstdarg>
#endif

#include "utils/shader_builder.h"
#include "utils/gl_utils.h"
#include "utils/Image.h"

class gosRenderer;

struct gosTextureInfo {
    int width_;
    int height_;
    gos_TextureFormat format_;
};

class gosShaderMaterial {
    public:
        static gosShaderMaterial* load(const char* shader) {
            gosASSERT(shader);
            gosShaderMaterial* pmat = new gosShaderMaterial();
            char vs[256];
            char ps[256];
            snprintf(vs, 255, "data/shaders/%s.vert", shader);
            snprintf(ps, 255, "data/shaders/%s.frag", shader);
            pmat->program_ = glsl_program::makeProgram(shader, vs, ps);
            if(!pmat->program_) {
                SPEW(("SHADERS", "Failed to create %s material\n", shader));
                delete pmat;
                return NULL;
            }
            
            pmat->pos_loc = pmat->program_->getAttribLocation("pos");
            pmat->color_loc = pmat->program_->getAttribLocation("color");
            pmat->texcoord_loc = pmat->program_->getAttribLocation("texcoord");

            return pmat;
        }

        void applyVertexDeclaration() {

            const int stride = sizeof(gos_VERTEX);
            
            // gos_VERTEX structure
	        //float x,y;
	        //float z;
	        //float rhw;
	        //DWORD argb;
	        //DWORD frgb;
	        //float u,v;	

            gosASSERT(pos_loc >= 0);
            glEnableVertexAttribArray(pos_loc);
            glVertexAttribPointer(pos_loc, 4, GL_FLOAT, GL_FALSE, stride, (void*)0);

            if(color_loc != -1) {
                glEnableVertexAttribArray(color_loc);
                glVertexAttribPointer(color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, 
                        BUFFER_OFFSET(4*sizeof(float)));
            }

            if(texcoord_loc != -1) {
                glEnableVertexAttribArray(texcoord_loc);
                glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, stride, 
                        BUFFER_OFFSET(4*sizeof(float) + 2*sizeof(uint32_t)));
            }
        }

        bool setSamplerUnit(const char* sampler_name, uint32_t unit) {
            gosASSERT(sampler_name);
            // TODO: may also check that current program is equal to our program
            if(program_->samplers_.count(sampler_name)) {
                glUniform1i(program_->samplers_[sampler_name]->index_, 0);
            }
        }

        void apply() {
            gosASSERT(program_);
            program_->apply();
        }

        void end() {

            glDisableVertexAttribArray(pos_loc);

            if(color_loc != -1) {
                glDisableVertexAttribArray(color_loc);
            }

            if(texcoord_loc != -1) {
                glDisableVertexAttribArray(texcoord_loc);
            }
        }

    private:
        gosShaderMaterial():
            program_(NULL)
            , pos_loc(-1)
            , color_loc(-1)
            , texcoord_loc(-1)
        {
        }

        glsl_program* program_;
        GLint pos_loc;
        GLint color_loc;
        GLint texcoord_loc;
};

class gosMesh {
    public:
        static gosMesh* makeMesh(gosPRIMITIVETYPE prim_type, int vertex_capacity) {
            GLuint vb = makeBuffer(GL_ARRAY_BUFFER, 0, sizeof(gos_VERTEX)*vertex_capacity, GL_DYNAMIC_DRAW);
            if(vb < 0)
                return NULL;

            gosMesh* mesh = new gosMesh(prim_type, vertex_capacity);
            mesh->vb_ = vb;
            mesh->pdata_ = new gos_VERTEX[vertex_capacity];
            return mesh;
        }

        bool addVertices(gos_VERTEX* vertices, int count) {
            if(num_vertices_ + count <= capacity_) {
                memcpy(pdata_ + num_vertices_, vertices, sizeof(gos_VERTEX)*count);
                num_vertices_ += count;
                return true;
            }
            return false;
        }

        int getCapacity() const { return capacity_; }
        int getNumVertices() const { return num_vertices_; }
        const gos_VERTEX* getVertices() const { return pdata_; }

        void rewind() { num_vertices_ = 0; }

        void draw(gosShaderMaterial* material) const;

    private:

        gosMesh(gosPRIMITIVETYPE prim_type, int vertex_capacity)
            : capacity_(vertex_capacity)
            , num_vertices_(0)
            , pdata_(NULL)    
            , prim_type_(prim_type)
         {
         }

        int capacity_;
        int num_vertices_;
        gos_VERTEX* pdata_;
        gosPRIMITIVETYPE prim_type_;

        GLuint vb_;
};

void gosMesh::draw(gosShaderMaterial* material) const
{
    gosASSERT(material);

    if(num_vertices_ == 0)
        return;

    updateBuffer(vb_, GL_ARRAY_BUFFER, pdata_, num_vertices_*sizeof(gos_VERTEX), GL_DYNAMIC_DRAW);

    material->apply();

    material->setSamplerUnit("tex1", 0);

	glBindBuffer(GL_ARRAY_BUFFER, vb_);

    material->applyVertexDeclaration();
    CHECK_GL_ERROR;

    GLenum pt = GL_TRIANGLES;
    switch(prim_type_) {
        case PRIMITIVE_POINTLIST:
            pt = GL_POINTS;
            break;
        case PRIMITIVE_LINELIST:
            pt = GL_LINES;
            break;
        case PRIMITIVE_TRIANGLELIST:
            pt = GL_TRIANGLES;
            break;
        default:
            gosASSERT(0 && "Wrong primitive type");
    }

    glDrawArrays(pt, 0, num_vertices_);

    material->end();

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

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
        }

        bool createHardwareTexture();

        ~gosTexture() {
            delete[] pdata_;
            delete[] filename_;
        }

        uint32_t getTextureId() { return tex_.id; }

        BYTE* Lock(int mipl_level, float is_read_only, int* pitch) {
            gosASSERT(is_locked_ == false);
            is_locked_ = true;
            // TODO:
            gosASSERT(pitch);
            *pitch = tex_.w;
            return pdata_;
        }

        void Unlock() {
            gosASSERT(is_locked_ == true);
            is_locked_ = false;
        }

        void getTextureInfo(gosTextureInfo* texinfo) {
            gosASSERT(texinfo);
            texinfo->width_ = tex_.w;
            texinfo->height_ = tex_.h;
            texinfo->format_ = format_;
        }

    private:
        BYTE* pdata_;
        DWORD size_;
        Texture tex_;

        gos_TextureFormat format_;
        char* filename_;
        DWORD hints_;

        bool is_locked_;
};

struct gosTextAttribs {
    HGOSFONT3D FontHandle;
    DWORD Foreground;
    float Size;
    bool WordWrap;
    bool Proportional;
    bool Bold;
    bool Italic;
    DWORD WrapType;
    bool DisableEmbeddedCodes;
};

bool gosTexture::createHardwareTexture() {

    gosASSERT(filename_);

    Image img;
    if(!img.loadFromFile(filename_)) {
        return false;
    }

    FORMAT img_fmt = img.getFormat();
    if(img_fmt != FORMAT_RGB8 && img_fmt != FORMAT_RGBA8) {
        STOP(("Unsupported texture format when loading %s\n", filename_));
    }

    TexFormat tf = img_fmt == FORMAT_RGB8 ? TF_RGB8 : TF_RGBA8;

    tex_ = create2DTexture(img.getWidth(), img.getHeight(), tf, img.getPixels());

    return tex_.isValid();
}



class gosRenderer {

    typedef std::map<gos_RenderState, unsigned int> StatePair;
    typedef std::vector<StatePair> StateList;

    public:
        gosRenderer(int w, int h) {
            width_ = w;
            height_ = h;
        }

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

        gosTextAttribs& getTextAttributes() { return curTextAttribs_; }
        void setTextPos(int x, int y) { curTextPosX_ = x; curTextPosY_ = y; }
        void setTextRegion(int Left, int Top, int Right, int Bottom) {
            curTextLeft_ = Left;
            curTextTop_ = Top;
            curTextRight_ = Right;
            curTextBottom_ = Bottom;
        }

        void setupViewport(bool FillZ, float ZBuffer, bool FillBG, DWORD BGColor, float top, float left, float bottom, float right, bool ClearStencil = 0, DWORD StencilValue = 0) {

            clearDepth_ = FillZ;
            clearDepthValue_ = ZBuffer;
            clearColor_ = FillBG;
            clearColorValue_ = BGColor;
            clearStencil_ = ClearStencil;
            clearStencilValue_ = StencilValue;
            viewportTop_ = top;
            viewportLeft_ = left;
            viewportBottom_ = bottom;
            viewportRight_ = right;
        }

        void getViewportTransform(float* viewMulX, float* viewMulY, float* viewAddX, float* viewAddY) {
            gosASSERT(viewMulX && viewMulY && viewAddX && viewAddY);
            *viewMulX = (viewportRight_ - viewportLeft_)*width_;
            *viewMulY = (viewportBottom_ - viewportTop_)*height_;
            *viewAddX = viewportLeft_ * width_;
            *viewAddY = viewportTop_ * height_;
        }

        void setRenderState(gos_RenderState RenderState, int Value) {
            renderStates_[RenderState] = Value;
        }

        void applyRenderStates();

        void drawQuads(gos_VERTEX* vertices, int count);
        void drawLines(gos_VERTEX* vertices, int count);
        void drawPoints(gos_VERTEX* vertices, int count);
        void drawTris(gos_VERTEX* vertices, int count);

        void init();
        void flush();

    private:

        // render target size
        int width_;
        int height_;

        void initRenderStates();

        std::vector<gosTexture*> textureList_;

        // states data
        unsigned int curStates_[gos_MaxState];
        unsigned int renderStates_[gos_MaxState];

        int statesStackPointer;
        StateList statesStack_[16];
        //

        // text data
        gosTextAttribs curTextAttribs_;

        int curTextPosX_;
        int curTextPosY_;

        int curTextLeft_;
        int curTextTop_;
        int curTextRight_;
        int curTextBottom_;
        //
        
        // viewport config
        bool clearDepth_;
        float clearDepthValue_;
        bool clearColor_;
        DWORD clearColorValue_;
        bool clearStencil_;
        DWORD clearStencilValue_;
        float viewportTop_;
        float viewportLeft_;
        float viewportBottom_;
        float viewportRight_;
        //
        
        gosMesh* quads_;
        gosMesh* tris_;
        gosMesh* lines_;
        gosMesh* points_;
        gosShaderMaterial* basic_material_;
        gosShaderMaterial* basic_tex_material_;


};

void gosRenderer::init() {
    initRenderStates();

    // setup viewport
    setupViewport(true, 1.0f, true, 0, 0.0f, 0.0f, 1.0f, 1.0f);

    quads_ = gosMesh::makeMesh(PRIMITIVE_TRIANGLELIST, 1024);
    tris_ = gosMesh::makeMesh(PRIMITIVE_TRIANGLELIST, 1024);
    lines_ = gosMesh::makeMesh(PRIMITIVE_LINELIST, 1024);
    points_= gosMesh::makeMesh(PRIMITIVE_POINTLIST, 1024);
    basic_material_ = gosShaderMaterial::load("gos_vertex");
    basic_tex_material_ = gosShaderMaterial::load("gos_tex_vertex");
}

void gosRenderer::initRenderStates() {

	renderStates_[gos_State_Texture] = 0;
	renderStates_[gos_State_Texture2] = 0;
    renderStates_[gos_State_Texture3] = 0;
	renderStates_[gos_State_Filter] = gos_FilterNone;
	renderStates_[gos_State_ZCompare] = 1; 
    renderStates_[gos_State_ZWrite] = 1;
	renderStates_[gos_State_AlphaTest] = 0;
	renderStates_[gos_State_Perspective] = 1;
	renderStates_[gos_State_Specular] = 0;
	renderStates_[gos_State_Dither] = 0;
	renderStates_[gos_State_Clipping] = 0;	
	renderStates_[gos_State_WireframeMode] = 0;
	renderStates_[gos_State_AlphaMode] = gos_Alpha_OneZero;
	renderStates_[gos_State_TextureAddress] = gos_TextureWrap;
	renderStates_[gos_State_ShadeMode] = gos_ShadeGouraud;
	renderStates_[gos_State_TextureMapBlend] = gos_BlendModulateAlpha;
	renderStates_[gos_State_MipMapBias] = 0;
	renderStates_[gos_State_Fog]= 0;
	renderStates_[gos_State_MonoEnable] = 0;
	renderStates_[gos_State_Culling] = gos_Cull_None;
	renderStates_[gos_State_StencilEnable] = 0;
	renderStates_[gos_State_StencilFunc] = gos_Cmp_Never;
	renderStates_[gos_State_StencilRef] = 0;
	renderStates_[gos_State_StencilMask] = 0xffffffff;
	renderStates_[gos_State_StencilZFail] = gos_Stencil_Keep;
	renderStates_[gos_State_StencilFail] = gos_Stencil_Keep;
	renderStates_[gos_State_StencilPass] = gos_Stencil_Keep;
	renderStates_[gos_State_Multitexture] = gos_Multitexture_None;
	renderStates_[gos_State_Ambient] = 0xffffff;
	renderStates_[gos_State_Lighting] = 0;
	renderStates_[gos_State_NormalizeNormals] = 0;
	renderStates_[gos_State_VertexBlend] = 0;

    applyRenderStates();
}

void gosRenderer::applyRenderStates() {

   ////////////////////////////////////////////////////////////////////////////////
   if(0 == renderStates_[gos_State_ZCompare]) {
       glDisable(GL_DEPTH_TEST);
   } else {
       glEnable(GL_DEPTH_TEST);
   }
   switch(renderStates_[gos_State_ZCompare]) {
       case 0: glDepthFunc(GL_ALWAYS); break;
       case 1: glDepthFunc(GL_LEQUAL); break;
       case 2: glDepthFunc(GL_LESS); break;
       default: gosASSERT(0 && "Wrong depth test value");
   }
   curStates_[gos_State_ZCompare] = renderStates_[gos_State_ZCompare];

   ////////////////////////////////////////////////////////////////////////////////
   switch(renderStates_[gos_State_ZWrite]) {
       case 0: glDepthMask(GL_FALSE); break;
       case 1: glDepthMask(GL_TRUE); break;
       default: gosASSERT(0 && "Wrong depth write value");
   }
   curStates_[gos_State_ZWrite] = renderStates_[gos_State_ZWrite];

   ////////////////////////////////////////////////////////////////////////////////
   bool disable_blending = renderStates_[gos_State_AlphaMode] == gos_Alpha_OneZero;
   if(disable_blending) {
       glDisable(GL_BLEND);
   } else {
       glEnable(GL_BLEND);
   }
   switch(renderStates_[gos_State_AlphaMode]) {
       case gos_Alpha_OneZero:          glBlendFunc(GL_ONE, GL_ZERO); break;
       case gos_Alpha_OneOne:           glBlendFunc(GL_ONE, GL_ONE); break;
       case gos_Alpha_AlphaInvAlpha:    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
       case gos_Alpha_OneInvAlpha:      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); break;
       case gos_Alpha_AlphaOne:         glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
       default: gosASSERT(0 && "Wrong alpha mode value");
   }
   curStates_[gos_State_AlphaMode] = renderStates_[gos_State_AlphaMode];

   ////////////////////////////////////////////////////////////////////////////////
   uint32_t tex_states[] = { gos_State_Texture, gos_State_Texture2, gos_State_Texture3 };
   for(int i=0; i<sizeof(tex_states) / sizeof(tex_states[0]); ++i) {
       DWORD gosTextureHandle = renderStates_[tex_states[i]];

       glActiveTexture(GL_TEXTURE0 + i);
       if(gosTextureHandle != 0) {
           gosTexture* tex = this->getTexture(gosTextureHandle);
           gosASSERT(tex);

           glBindTexture(GL_TEXTURE_2D, tex->getTextureId());
       } else {
           glBindTexture(GL_TEXTURE_2D, 0);
       }
       curStates_[tex_states[i]] = gosTextureHandle;
   }

}

void gosRenderer::drawQuads(gos_VERTEX* vertices, int count) {
    gosASSERT(vertices);

    int num_quads = count / 4;
    int num_vertices = num_quads * 6;


    if(quads_->getNumVertices() + num_vertices > quads_->getCapacity()) {
        applyRenderStates();
        gosShaderMaterial* mat = 
            curStates_[gos_State_Texture]!=0 ? basic_tex_material_ : basic_material_;
        quads_->draw(mat);
        quads_->rewind();
    } 

    gosASSERT(quads_->getNumVertices() + num_vertices <= quads_->getCapacity());
    for(int i=0; i<count;i+=4) {

        quads_->addVertices(vertices + 4*i + 0, 1);
        quads_->addVertices(vertices + 4*i + 1, 1);
        quads_->addVertices(vertices + 4*i + 2, 1);

        quads_->addVertices(vertices + 4*i + 0, 1);
        quads_->addVertices(vertices + 4*i + 2, 1);
        quads_->addVertices(vertices + 4*i + 3, 1);
    }

    // for now draw anyway because no render state saved for draw calls
    applyRenderStates();
    gosShaderMaterial* mat = 
        curStates_[gos_State_Texture]!=0 ? basic_tex_material_ : basic_material_;
    quads_->draw(mat);
    quads_->rewind();
}

void gosRenderer::drawLines(gos_VERTEX* vertices, int count) {
    gosASSERT(vertices);

    if(lines_->getNumVertices() + count > lines_->getCapacity()) {
        applyRenderStates();
        lines_->draw(basic_material_);
        lines_->rewind();
    }

    gosASSERT(lines_->getNumVertices() + count <= lines_->getCapacity());
    lines_->addVertices(vertices, count);

    // for now draw anyway because no render state saved for draw calls
    applyRenderStates();
    lines_->draw(basic_material_);
    lines_->rewind();
}

void gosRenderer::drawPoints(gos_VERTEX* vertices, int count) {
    gosASSERT(vertices);

    if(points_->getNumVertices() + count > points_->getCapacity()) {
        applyRenderStates();
        points_->draw(basic_material_);
        points_->rewind();
    } 

    gosASSERT(points_->getNumVertices() + count <= points_->getCapacity());
    points_->addVertices(vertices, count);

    // for now draw anyway because no render state saved for draw calls
    applyRenderStates();
    points_->draw(basic_material_);
    points_->rewind();
}

void gosRenderer::drawTris(gos_VERTEX* vertices, int count) {
    gosASSERT(vertices);

    gosASSERT((count % 3) == 0);

    if(tris_->getNumVertices() + count > tris_->getCapacity()) {
        applyRenderStates();
        gosShaderMaterial* mat = 
            curStates_[gos_State_Texture]!=0 ? basic_tex_material_ : basic_material_;
        tris_->draw(mat);
        tris_->rewind();
    } 

    gosASSERT(tris_->getNumVertices() + count <= tris_->getCapacity());
    tris_->addVertices(vertices, count);

    // for now draw anyway because no render state saved for draw calls
    applyRenderStates();
    gosShaderMaterial* mat = 
        curStates_[gos_State_Texture]!=0 ? basic_tex_material_ : basic_material_;
    tris_->draw(mat);
    tris_->rewind();
}

void gosRenderer::flush()
{
    quads_->draw(basic_material_);
    quads_->rewind();
}

static gosRenderer* g_gos_renderer = NULL;

void gos_CreateRenderer(int w, int h) {

    g_gos_renderer = new gosRenderer(w, h);
    g_gos_renderer->init();
}

void gos_RendererEndFrame() {
    gosASSERT(g_gos_renderer);
    g_gos_renderer->flush();
}

////////////////////////////////////////////////////////////////////////////////
// graphics
//
void _stdcall gos_DrawLines(gos_VERTEX* Vertices, int NumVertices)
{
    //gosASSERT(0 && "Not implemented");
    gosASSERT(g_gos_renderer);
    g_gos_renderer->drawLines(Vertices, NumVertices);
}
void _stdcall gos_DrawPoints(gos_VERTEX* Vertices, int NumVertices)
{
 //   gosASSERT(0 && "Not implemented");
    gosASSERT(g_gos_renderer);
    g_gos_renderer->drawPoints(Vertices, NumVertices);
}

bool g_disable_quads = true;
void _stdcall gos_DrawQuads(gos_VERTEX* Vertices, int NumVertices)
{
    //gosASSERT(0 && "Not implemented");
    gosASSERT(g_gos_renderer);
    if(g_disable_quads == false )
        g_gos_renderer->drawQuads(Vertices, NumVertices);
}
void _stdcall gos_DrawTriangles(gos_VERTEX* Vertices, int NumVertices)
{
  //  gosASSERT(0 && "Not implemented");
    gosASSERT(g_gos_renderer);
    g_gos_renderer->drawTris(Vertices, NumVertices);
}

void __stdcall gos_GetViewport( float* pViewportMulX, float* pViewportMulY, float* pViewportAddX, float* pViewportAddY )
{
    gosASSERT(g_gos_renderer);
    g_gos_renderer->getViewportTransform(pViewportMulX, pViewportMulY, pViewportAddX, pViewportAddY);
}
/*
typedef struct _FontInfo
	DWORD		MagicNumber;			// Valid font check
	_FontInfo*	pNext;					// Pointer to next font
	DWORD		ReferenceCount;			// Reference count
	char		FontFile[MAX_PATH];		// Path name of font texture
	DWORD		StartLine;				// texture line where font starts
	int			CharCount;				// number of chars in font (valid range 33 to 256)
	DWORD		TextureSize;			// Width and Height of texture
	float		rhSize;					// 1.0 / Size
	DWORD		TexturePitch;			// Pitch of texture
	int			Width;					// Width of font grid
	int			Height;					// Height of font grid
	DWORD		Across;					// Number of characters across one line
	DWORD		Aliased;				// True if 4444 texture (may be aliased - else 1555 keyed)
	DWORD		FromTextureHandle;		// True is from a texture handle
	BYTE		BlankPixels[256-32];	// Empty pixels before character
	BYTE		UsedPixels[256-32];		// Width of character
	BYTE		TopU[256-32];
	BYTE		TopV[256-32];			// Position of character
	BYTE		TopOffset[256-32];		// Offset from top (number of blank lines)
	BYTE		RealHeight[256-32];		// Height of character
	BYTE		TextureHandle[256-32];	// Which texture handle to use
	DWORD		NumberOfTextures;		// Number of texture handles used (normally 1)
	HFONT		hFontTTF;				// handle to a GDI font
	DWORD		Texture[8];				// Texture handle array
*/
HGOSFONT3D __stdcall gos_LoadFont( const char* FontFile, DWORD StartLine/* = 0*/, int CharCount/* = 256*/, DWORD TextureHandle/*=0*/)
{
    gosASSERT(FontFile);
    _FontInfo* fi = new _FontInfo();
    memset(fi, 0, sizeof(fi));
    strncpy(fi->FontFile, FontFile, sizeof(fi->FontFile));
    fi->StartLine = StartLine;
    fi->CharCount = CharCount;
    fi->NumberOfTextures = TextureHandle!=0 ? 1 : 0;
    fi->TextureHandle[0] = TextureHandle;

    // TODO: actually load some font
    fi->Width = 16;
    fi->Height = 16;

    //gosASSERT(0 && "Not implemented");
    return fi;
}

void __stdcall gos_DeleteFont( HGOSFONT3D Fonthandle )
{
    gosASSERT(Fonthandle);
    delete (_FontInfo*)Fonthandle;
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
    if(!ptex->createHardwareTexture()) {
        STOP(("Failed to create texture\n"));
    }

    return g_gos_renderer->addTexture(ptex);
}
void __stdcall gos_DestroyTexture( DWORD Handle )
{
    //gosASSERT(0 && "Not implemented");
    g_gos_renderer->deleteTexture(Handle);
}

void __stdcall gos_LockTexture( DWORD Handle, DWORD MipMapSize, bool ReadOnly, TEXTUREPTR* TextureInfo )
{
    // TODO: does not really locks texture
    
    // not implemented yet
    gosASSERT(MipMapSize == 0);
    int mip_level = 0; //func(MipMapSize);

    gosTextureInfo info;
    int pitch = 0;
    gosTexture* ptex = g_gos_renderer->getTexture(Handle);
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
    gosTexture* ptex = g_gos_renderer->getTexture(Handle);
    ptex->Unlock();

    //gosASSERT(0 && "Not implemented");
}

void __stdcall gos_PushRenderStates()
{
    //gosASSERT(0 && "not implemented");
}
void __stdcall gos_PopRenderStates()
{
    //gosASSERT(0 && "not implemented");
}

void __stdcall gos_RenderIndexedArray( gos_VERTEX* pVertexArray, DWORD NumberVertices, WORD* lpwIndices, DWORD NumberIndices )
{
 //   gosASSERT(0 && "not implemented");
}

void __stdcall gos_RenderIndexedArray( gos_VERTEX_2UV* pVertexArray, DWORD NumberVertices, WORD* lpwIndices, DWORD NumberIndices )
{
 //   gosASSERT(0 && "not implemented");
}

void __stdcall gos_SetRenderState( gos_RenderState RenderState, int Value )
{
    gosASSERT(g_gos_renderer);
    g_gos_renderer->setRenderState(RenderState, Value);
}

void __stdcall gos_SetScreenMode( DWORD Width, DWORD Height, DWORD bitDepth/*=16*/, DWORD Device/*=0*/, bool disableZBuffer/*=0*/, bool AntiAlias/*=0*/, bool RenderToVram/*=0*/, bool GotoFullScreen/*=0*/, int DirtyRectangle/*=0*/, bool GotoWindowMode/*=0*/, bool EnableStencil/*=0*/, DWORD Renderer/*=0*/)
{
    gosASSERT(0 && "not implemented");
}

void __stdcall gos_SetupViewport( bool FillZ, float ZBuffer, bool FillBG, DWORD BGColor, float top, float left, float bottom, float right, bool ClearStencil/*=0*/, DWORD StencilValue/*=0*/)
{
    gosASSERT(g_gos_renderer);
    g_gos_renderer->setupViewport(FillZ, ZBuffer, FillBG, BGColor, top, left, bottom, right, ClearStencil, StencilValue);
}

void __stdcall gos_TextDraw( const char *Message, ... )
{
 //   gosASSERT(0 && "not implemented");
}

void __stdcall gos_TextSetAttributes( HGOSFONT3D FontHandle, DWORD Foreground, float Size, bool WordWrap, bool Proportional, bool Bold, bool Italic, DWORD WrapType/*=0*/, bool DisableEmbeddedCodes/*=0*/)
{
    gosASSERT(g_gos_renderer);

    gosTextAttribs& ta = g_gos_renderer->getTextAttributes();
    ta.FontHandle = FontHandle;
    ta.Foreground = Foreground;
    ta.Size = Size;
    ta.WordWrap = WordWrap;
    ta.Proportional = Proportional;
    ta.Bold = Bold;
    ta.Italic = Italic;
    ta.WrapType = WrapType;
    ta.DisableEmbeddedCodes = DisableEmbeddedCodes;
}

void __stdcall gos_TextSetPosition( int XPosition, int YPosition )
{
    gosASSERT(g_gos_renderer);
    g_gos_renderer->setTextPos(XPosition, YPosition);
}

void __stdcall gos_TextSetRegion( int Left, int Top, int Right, int Bottom )
{
    gosASSERT(g_gos_renderer);
    g_gos_renderer->setTextRegion(Left, Top, Right, Bottom);
}

void __stdcall gos_TextStringLength( DWORD* Width, DWORD* Height, const char *fmt, ... )
{
    gosASSERT(Width && Height && fmt);

    const int   MAX_TEXT_LEN = 4096;
	char        text[MAX_TEXT_LEN] = {0};
	va_list	    ap;

    va_start(ap, fmt);
	vsnprintf(text, MAX_TEXT_LEN - 1, fmt, ap);
    va_end(ap);

	size_t len = strlen(text);
    text[len] = '\0';

    int num_newlines = 0;
    int max_width = 0;
    int cur_width = 0;
    const char* txtptr = text;
    while(*txtptr) {
        if(*txtptr++ == '\n') {
            num_newlines++;
            max_width = max_width > cur_width ? max_width : cur_width;
            cur_width = 0;
        } else {
            cur_width++;
        }
    }
    max_width = max_width > cur_width ? max_width : cur_width;

    int w = max_width;
    int h = num_newlines + 1;

    const gosTextAttribs& ta = g_gos_renderer->getTextAttributes();
    _FontInfo* fi = ta.FontHandle;
    gosASSERT(fi);
    w *= fi->Width;
    h *= fi->Height;

    *Width = w;
    *Height = h;

}

