/*
Generic image class for 1D, 2D, 3D and Cubmaps.

1D textures have height = 1 and depth = 1.
2D textures have depth = 1.
3D textures have depth > 1.
Cubemaps are identified with depth = 0.
*/
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdio.h>

// Texture format enums
enum FORMAT {
	FORMAT_NONE    = 0,
	FORMAT_A8      = 1,
	FORMAT_I8      = 2,
	FORMAT_IA8     = 3,
	FORMAT_RGB8    = 4,
	FORMAT_RGBA8   = 5,
	FORMAT_I16     = 6,
	FORMAT_IA16    = 7,
	FORMAT_RGB16   = 8,
	FORMAT_RGBA16  = 9,
	FORMAT_R16f    = 10,
	FORMAT_RG16f   = 11,
	FORMAT_RGB16f  = 12,
	FORMAT_RGBA16f = 13,
	FORMAT_R32f    = 14,
	FORMAT_RG32f   = 15,
	FORMAT_RGB32f  = 16,
	FORMAT_RGBA32f = 17,
	FORMAT_I16f    = 18,
	FORMAT_IA16f   = 19,
	FORMAT_I32f    = 20,
	FORMAT_IA32f   = 21,
	FORMAT_UV8     = 22,
	FORMAT_UVWQ8   = 23,
	FORMAT_UV16    = 24,
	FORMAT_UVWQ16  = 25,
	FORMAT_DEPTH16 = 26,
	FORMAT_DEPTH24 = 27,
	FORMAT_RGB332  = 28,
	FORMAT_RGB565  = 29,
	FORMAT_RGB5A1  = 30,
	FORMAT_RGB10A2 = 31,
	FORMAT_UV5L6   = 32,
	FORMAT_UVW10A2 = 33,
	FORMAT_DXT1    = 34,
	FORMAT_DXT3    = 35,
	FORMAT_DXT5    = 36,
	FORMAT_ATI1N   = 37,
	FORMAT_ATI2N   = 38,
};

#define FORMAT_R8    FORMAT_I8
#define FORMAT_RG8   FORMAT_IA8
#define FORMAT_R16   FORMAT_I16
#define FORMAT_RG16  FORMAT_IA16

// Accepts plain, packed and depth formats
inline int getBytesPerPixel(const FORMAT format){
	static const int bytes[] = {
		0,
		1, 1, 2, 3, 4,    // Unsigned formats
		2, 4, 6, 8,
		2, 4, 6, 8,       // Float formats
		4, 8, 12, 16,
		2, 4, 4, 8,
		2, 4, 4, 8,       // Signed formats
		2, 4,             // Depth formats
		1, 2, 2, 4, 2, 4, // Packed formats
	};
	return bytes[format];
}

class Image
{
public:
	Image(void);
	~Image(void);
	
	unsigned char *create(const FORMAT fmt, const int w, const int h);
	void clear();

	bool loadFromFile(const char *fileName);
	void loadFromMemory(void *mem, const FORMAT fmt, const int w, const int h, const int d, const int mipMapCount, bool ownsMemory);

	
	bool loadTGA(const char *fileName);
	bool loadBMP(const char *fileName);

	bool loadTGA(FILE* file);
	bool loadBMP(FILE* file);


	bool flip();

	int getWidth()  const { return width;  }
	int getHeight() const { return height; }
	
	FORMAT getFormat() const { return format; }

	unsigned char *getPixels() const { return pixels; }

private:

	unsigned char *pixels;
	long width, height;
	FORMAT format;
};



#endif // _IMAGE_H_