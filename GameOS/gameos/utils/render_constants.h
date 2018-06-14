#ifndef RENDER_CONSTANTS_H
#define RENDER_CONSTANTS_H

#ifdef PLATFORM_WINDOWS
	#include<stdint.h>
#endif
#include <inttypes.h>

#include "utils/graphics.h"

#define DEPTH_NONE 0        /**< Disable depthstencil. */
#define BLEND_NONE (-1)     /**< Disable blending. */

enum TexType {
    TT_NONE,
    TT_1D,
    TT_1D_ARRAY,
    TT_2D,
    TT_2D_ARRAY,
    TT_2D_MULTISAMPLE,
    TT_2D_MULTISAMPLE_ARRAY,
    TT_3D,
    TT_CUBE_MAP,
    TT_CUBE_MAP_ARRAY,
    TT_COUNT
};

enum TexFormat {
    TF_NONE,
    TF_R8,
    TF_RG8,
    TF_RGB8,
    TF_RGBA8,
    TF_R32F,
    TF_RG32F,
    TF_RGB32F,
    TF_RGBA32F,
    TF_COUNT
};

enum TexAddressMode {
    TAM_NONE,
    TAM_CLAMP_TO_EDGE,
    TAM_REPEAT,
    TAM_COUNT
};

/*extern const int tex_filter_mode_nearest;
extern const int tex_filer_mode_linear;
extern const int tex_filter_mode_nearest_mipmap_nearest;
extern const int tex_filter_mode_nearest_mipmap_linear;
extern const int tex_filter_mode_linear_mipmap_nearest;
extern const int tex_filter_mode_linear_mipmap_linear;
*/

enum TexFilterMode {
    TFM_NONE = 0,
    TFM_NEAREST,                 //= tex_filter_mode_nearest,
    TFM_LINEAR,                  //= tex_filer_mode_linear,
    TFM_NEAREST_MIPMAP_NEAREST,  //= tex_filter_mode_nearest_mipmap_nearest,
    TFM_NEAREST_MIPMAP_LINEAR,   //= tex_filter_mode_nearest_mipmap_linear,
    TFM_LNEAR_MIPMAP_NEAREST,    //= tex_filter_mode_linear_mipmap_nearest,
    TFM_LNEAR_MIPMAP_LINEAR,     //= tex_filter_mode_linear_mipmap_linear
    TFM_COUNT
};

// Blending constants
extern const int ZERO;
extern const int ONE;
extern const int SRC_COLOR;
extern const int ONE_MINUS_SRC_COLOR;
extern const int DST_COLOR;
extern const int ONE_MINUS_DST_COLOR;
extern const int SRC_ALPHA;
extern const int ONE_MINUS_SRC_ALPHA;
extern const int DST_ALPHA;
extern const int ONE_MINUS_DST_ALPHA;
extern const int SRC_ALPHA_SATURATE;
extern const int CONSTANT_COLOR;
extern const int ONE_MINUS_CONSTANT_COLOR;

enum BlendMode {
	BM_ADD,
	BM_SUBTRACT,
	BM_REVERSE_SUBTRACT,
	BM_MIN,
	BM_MAX
};
GLuint getNativeBlendMode(BlendMode bm);

// Depth testing constants
extern const int NEVER;
extern const int LESS;
extern const int EQUAL;
extern const int LEQUAL;
extern const int GREATER;
extern const int NOTEQUAL;
extern const int GEQUAL;
extern const int ALWAYS;

// Primitive types
extern const int TRIANGLES;
extern const int TRIANGLE_STRIP;
extern const int TRIANGLE_FAN;
extern const int QUADS;			

// Culling constants
extern const int CULL_NONE;
extern const int BACK;
extern const int FRONT;

// Resource locking flags
extern const int WRITE_ONLY;
extern const int READ_ONLY;
extern const int READ_WRITE;

// Buffer clear flags
extern const int COLOR_BUFFER;
extern const int DEPTH_BUFFER;
extern const int STENCIL_BUFFER;

typedef int TextureID;      /**< Texture handle. */
typedef int ShaderID;       /**< Shader handle. */
typedef int VertexFormatID; /**< Vertex format handle. */
typedef int VertexBufferID; /**< Vertex buffer handle. */
typedef int IndexBufferID;  /**< Index buffer handle. */

#endif // RENDER_CONSTANTS_H
