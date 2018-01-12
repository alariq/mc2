#include "utils/graphics.h"
#include "utils/render_constants.h"

const int ZERO                 = GL_ZERO;
const int ONE                  = GL_ONE;
const int SRC_COLOR            = GL_SRC_COLOR;
const int ONE_MINUS_SRC_COLOR  = GL_ONE_MINUS_SRC_COLOR;
const int DST_COLOR            = GL_DST_COLOR;
const int ONE_MINUS_DST_COLOR  = GL_ONE_MINUS_DST_COLOR;
const int SRC_ALPHA            = GL_SRC_ALPHA;
const int ONE_MINUS_SRC_ALPHA  = GL_ONE_MINUS_SRC_ALPHA;
const int DST_ALPHA            = GL_DST_ALPHA;
const int ONE_MINUS_DST_ALPHA  = GL_ONE_MINUS_DST_ALPHA;
const int SRC_ALPHA_SATURATE   = GL_SRC_ALPHA_SATURATE;
const int CONSTANT_COLOR       = GL_CONSTANT_COLOR;
const int ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR;
/*
const int BM_ADD              = GL_FUNC_ADD_EXT;
const int BM_SUBTRACT         = GL_FUNC_SUBTRACT_EXT;
const int BM_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT_EXT;
const int BM_MIN              = GL_MIN_EXT;
const int BM_MAX              = GL_MAX_EXT;
*/

GLuint getNativeBlendMode(BlendMode bm) {
	switch(bm)
	{
		case BM_ADD:
		default:
			return GL_FUNC_ADD_EXT;
		case BM_SUBTRACT:
			return GL_FUNC_SUBTRACT_EXT;
		case BM_REVERSE_SUBTRACT:
			return GL_FUNC_REVERSE_SUBTRACT_EXT;
		case BM_MIN:
			return GL_MIN_EXT;
		case BM_MAX:
			return GL_MAX_EXT;
	}
}

// Depth testing constants
const int NEVER    = GL_NEVER;
const int LESS     = GL_LESS;
const int EQUAL    = GL_EQUAL;
const int LEQUAL   = GL_LEQUAL;
const int GREATER  = GL_GREATER;
const int NOTEQUAL = GL_NOTEQUAL;
const int GEQUAL   = GL_GEQUAL;
const int ALWAYS   = GL_ALWAYS;

// Primitive types
const int TRIANGLES      = GL_TRIANGLES;
const int TRIANGLE_STRIP = GL_TRIANGLE_STRIP;
const int TRIANGLE_FAN   = GL_TRIANGLE_FAN;
const int QUADS			 = GL_QUADS;

// Culling constants
const int CULL_NONE = 0;
const int BACK  = GL_BACK;
const int FRONT = GL_FRONT;

// Lock flags
const int WRITE_ONLY = GL_WRITE_ONLY;
const int READ_ONLY  = GL_READ_ONLY;
const int READ_WRITE = GL_READ_WRITE;

// Buffer clear flags
const int COLOR_BUFFER   = GL_COLOR_BUFFER_BIT;
const int DEPTH_BUFFER   = GL_DEPTH_BUFFER_BIT;
const int STENCIL_BUFFER = GL_STENCIL_BUFFER_BIT;

