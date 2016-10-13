#include "utils/Image.h"

#include <stdio.h>
#include <cstring>
#include <stdlib.h>

//#include "math/vec.h"
//#include "system/types.h"
//#include "system/defines.h"


#ifdef _WIN32
#pragma warning(disable : 4996)
#else
#define stricmp strcasecmp
#endif


void Image::clear(){
	delete pixels;
	pixels = NULL;
	width = height = 0;
	format = FORMAT_NONE;
}

// Pack header tightly
#pragma pack (push, 1)

struct TGAHeader {
	unsigned char  descLen;
	unsigned char  cmapType;
	unsigned char  imageType;
	unsigned short cmapStart;
	unsigned short cmapEntries;
	unsigned char  cmapBits;
	unsigned short xOffset;
	unsigned short yOffset;
	unsigned short width;
	unsigned short height;
	unsigned char  bpp;
	unsigned char  attrib;
};

struct BMPHeader {
	unsigned short bfType;
	unsigned int   bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;
	unsigned int   biSize;
	unsigned int   biWidth;
	unsigned int   biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int   biCompression;
	unsigned int   biSizeImage;
	unsigned int   biXPelsPerMeter;
	unsigned int   biYPelsPerMeter;
	unsigned int   biClrUsed;
	unsigned int   biClrImportant;
};


template <typename DATA_TYPE>
inline void swapChannels(DATA_TYPE *pixels, int nPixels, const int channels, const int ch0, const int ch1){
	do {
		DATA_TYPE tmp = pixels[ch1];
		pixels[ch1] = pixels[ch0];
		pixels[ch0] = tmp;
		pixels += channels;
	} while (--nPixels);
}
 

Image::Image(void)
{
	pixels = 0;
}

Image::~Image(void)
{
	delete pixels;
}

unsigned char *Image::create(const FORMAT fmt, const int w, const int h){
	format = fmt;
	width  = w;
	height = h;

	int size = w*h;
	delete pixels;
	pixels = new unsigned char[size];

	return pixels;
}

bool Image::loadFromFile(const char *fileName){
	const char *ext = strrchr(fileName, '.');
	if (ext == NULL) return false;

	ext++;
	if (stricmp(ext, "tga") == 0)
	{
		return loadTGA(fileName);
	}
	else if (stricmp(ext, "bmp") == 0)
	{
		return loadBMP(fileName);
	}
	return false;
} 

bool Image::loadTGA(const char *fileName){
	clear();

	FILE *file;

	if ((file = fopen(fileName, "rb")) == NULL) return false;
	return loadTGA(file);
}

bool Image::loadTGA(FILE* file)
{
	TGAHeader header;

	fread(&header, sizeof(header), 1, file);
	// Skip past the description if there's any
	if (header.descLen) fseek(file, header.descLen, SEEK_CUR);

	width  = header.width;
	height = header.height;

	int pixelSize = header.bpp / 8;
	int size = width * height * pixelSize;

	unsigned char *readPixels = new unsigned char[size];
	fread(readPixels, size, 1, file);
	fclose(file);

	unsigned char *dest, *src = readPixels + width * (height - 1) * pixelSize;

	int x, y;
	switch (header.bpp) {
		case 8:
			format = FORMAT_I8;
			dest = pixels = new unsigned char[width * height];
			for (y = 0; y < height; y++){
				memcpy(dest, src, width);
				dest += width;
				src -= width;
			}
			break;
		case 16:
			format = FORMAT_RGBA8;
			dest = pixels = new unsigned char[width * height * 4];
			for (y = 0; y < height; y++){
				for (x = 0; x < width; x++){
					unsigned short tempPixel = *((unsigned short *) src);

					dest[0] = ((tempPixel >> 10) & 0x1F) << 3;
					dest[1] = ((tempPixel >>  5) & 0x1F) << 3;
					dest[2] = ((tempPixel      ) & 0x1F) << 3;
					dest[3] = ((tempPixel >> 15) ? 0xFF : 0);
					dest += 4;
					src += 2;
				}
				src -= 4 * width;
			}
			break;
		case 24:
			format = FORMAT_RGB8;
			dest = pixels = new unsigned char[width * height * 3];
			for (y = 0; y < height; y++){
				for (x = 0; x < width; x++){
					*dest++ = src[2];
					*dest++ = src[1];
					*dest++ = src[0];
					src += 3;
				}
				src -= 6 * width;
			}
			break;
		case 32:
			format = FORMAT_RGBA8;
			dest = pixels = new unsigned char[width * height * 4];
			for (y = 0; y < height; y++){
				for (x = 0; x < width; x++){
					*dest++ = src[2];
					*dest++ = src[1];
					*dest++ = src[0];
					*dest++ = src[3];
					src += 4;
				}
				src -= 8 * width;
			}
			break;
	}

	if (header.attrib & 0x20) flip();

	delete readPixels;
	return true;
}

bool Image::loadBMP(const char *fileName){
	clear();

	FILE *file = fopen(fileName, "rb");
	if (file == NULL) return false;

	return loadBMP(file);
}

bool Image::loadBMP(FILE* file)
{
	// Read the header
	BMPHeader header;
	fread(&header, sizeof(header), 1, file);
	if (header.bfType != 0x4D42){
		fclose(file);
		return false;
	}

	width    = header.biWidth;
	height   = header.biHeight;

	switch (header.biBitCount){
		case 8:
			// No support for RLE compressed bitmaps
			if (header.biCompression){
				fclose(file);
				return false;
			}
			format = FORMAT_RGB8;
			pixels = new unsigned char[width * height * 3];

			unsigned char palette[1024];
			fread(palette, sizeof(palette), 1, file);

			// 8-bit paletted bitmap
			unsigned char *tmp;
			tmp = new unsigned char[width];
			for (int i = height - 1; i >= 0; i--){
				unsigned char *dest = pixels + i * width * 3;
				fread(tmp, width, 1, file);
				int len = width;
				do {
					unsigned char *src = palette + ((*tmp++) << 2);
					*dest++ = src[2];
					*dest++ = src[1];
					*dest++ = src[0];
				} while (--len);
				tmp -= width;
			}
			delete tmp;
			break;
		case 24:
		case 32:
			int nChannels;
			if (header.biBitCount == 24){
				nChannels = 3;
				format = FORMAT_RGB8;
			} else {
				nChannels = 4;
				format = FORMAT_RGBA8;
			}
			pixels = new unsigned char[width * height * nChannels];
			for (int i = height - 1; i >= 0; i--){
				unsigned char *dest = pixels + i * width * nChannels;
				fread(dest, width * nChannels, 1, file);
				swapChannels(dest, width, nChannels, 0, 2);
			}
			break;
		default:
			fclose(file);
			return false;
	}

	fclose(file);

	return true;
}


bool Image::flip(){

	int lineWidth = getBytesPerPixel(format) * width;
	unsigned char *newPixels = new unsigned char[lineWidth * height];

	for (int i = 0; i < height; i++){
		memcpy(newPixels + i * lineWidth, pixels + (height - 1 - i) * lineWidth, lineWidth);
	}

	delete pixels;
	pixels = newPixels;

	return true;
}
