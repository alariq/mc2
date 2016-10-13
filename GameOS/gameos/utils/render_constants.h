#ifndef RENDER_CONSTANTS_H
#define RENDER_CONSTANTS_H

#ifdef PLATFORM_WINDOWS
	#include<stdint.h>
#endif
#include <inttypes.h>

#include "utils/graphics.h"

#define DEPTH_NONE 0        /**< Disable depthstencil. */
#define BLEND_NONE (-1)     /**< Disable blending. */

enum TexFormat {
    TF_NONE,
    TF_R8,
    TF_RG8,
    TF_RGB8,
    TF_RGBA8,
    TF_R32,
    TF_RG32,
    TF_RGB32,
    TF_RGBA32,
    TF_COUNT
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
