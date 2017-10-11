#include "utils/Image.h"

#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <cassert>

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

enum class TGADataType {
	kNoImageData = 0,
	kUncompressedColorMapped = 1,
	kUncompressedRGB = 2,
	kUncompressedBlackAndWhite = 3,
	kRLEColorMapped = 9,
	kRLERGB = 10,
	kCompressedBlackAndWhite = 11,
	kCompressedColorMappedHuffman = 32,
	kCompressedColorMappedHuffman4PassQuadTree = 33,
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


static FORMAT getFormatFromBpp(const unsigned int bpp) {
	switch (bpp) {
	case 8:
		return FORMAT_I8;
		break;
	case 16:
		return FORMAT_RGBA8;
		break;
	case 24:
		return FORMAT_RGB8;
		break;
	case 32:
		return FORMAT_RGBA8;
		break;
	}
}

Image::Image(void)
{
	pixels = 0;
}

Image::~Image(void)
{
	delete[] pixels;
}

unsigned char *Image::create(const FORMAT fmt, const int w, const int h){
	format = fmt;
	width  = w;
	height = h;

	int size = w*h;
	delete[] pixels;
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

	int pixelSize = header.bpp / 8;

	bool rv = false;
	unsigned char *readPixels = 0;

	if (header.imageType == (unsigned char)TGADataType::kUncompressedRGB) {

		int size = header.width * header.height * pixelSize;
		readPixels = new unsigned char[size];
		fread(readPixels, size, 1, file);

		rv = loadTGA(&header, readPixels);
	}
	else if(header.imageType == (unsigned char)TGADataType::kRLERGB) {

		size_t fpos = ftell(file);
		fseek(file, 0, SEEK_END);
		size_t fsize = ftell(file) - fpos;
		fseek(file, fpos, SEEK_SET);

		readPixels = new unsigned char[fsize];
		fread(readPixels, fsize, 1, file);

		rv = loadCompressedTGA(&header, readPixels, fsize);
	}

	fclose(file);
	delete[] readPixels;

    return rv;
}

bool Image::loadTGA(const unsigned char* mem, size_t len)
{
    assert(mem);
    assert(len > sizeof(TGAHeader));

	TGAHeader header;

	memcpy(&header, mem, sizeof(header));
    mem += sizeof(header);
    len -= sizeof(header);

	// Skip past the description if there's any
	if (header.descLen) {
        mem += header.descLen;
        len -= header.descLen;
    }

	unsigned int pixelSize = header.bpp / 8;
	int size = header.width * header.height * pixelSize;

	unsigned char *readPixels = 0;
	bool rv = false;


	if (header.imageType == (unsigned char)TGADataType::kUncompressedRGB) {
		assert(len >= header.width * header.height * pixelSize);
		readPixels = new unsigned char[size];
		memcpy(readPixels, mem, size);
		rv = loadTGA(&header, readPixels);
		delete[] readPixels;
	}
	else if(header.imageType == (unsigned char)TGADataType::kRLERGB) {
		rv = loadCompressedTGA(&header, mem, len);
	}

    return rv;
}

bool Image::loadTGA(const TGAHeader* header, unsigned char* readPixels)
{
	width  = header->width;
	height = header->height;

	format = getFormatFromBpp(header->bpp);

	int pixelSize = header->bpp / 8;

	unsigned char *dest, *src = readPixels + width * (height - 1) * pixelSize;

	int x, y;
	switch (header->bpp) {
		case 8:
			dest = pixels = new unsigned char[width * height];
			for (y = 0; y < height; y++){
				memcpy(dest, src, width);
				dest += width;
				src -= width;
			}
			break;
		case 16:
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

	if (header->attrib & 0x20) flip();

	return true;
}


static void mergeBytes(unsigned char *dest, unsigned char *src, int pixelSize)
{
	if (pixelSize == 4) {
		*dest++ = src[2];
		*dest++ = src[1];
		*dest++ = src[0];
		*dest++ = src[3];
	}
	else if (pixelSize == 3) {
		*dest++ = src[2];
		*dest++ = src[1];
		*dest++ = src[0];
	}
	else if (pixelSize == 2) {
		unsigned short tempPixel = *((unsigned short *)src);
		dest[0] = ((tempPixel >> 10) & 0x1F) << 3;
		dest[1] = ((tempPixel >> 5) & 0x1F) << 3;
		dest[2] = ((tempPixel) & 0x1F) << 3;
		dest[3] = ((tempPixel >> 15) ? 0xFF : 0);
	}
	else {
		assert(0 && "Unsupported pixelSize");
	}
}

bool Image::loadCompressedTGA(const TGAHeader* header, const unsigned char* mem, size_t len) {

	width  = header->width;
	height = header->height;

	format = getFormatFromBpp(header->bpp);

	const int pixelSize = header->bpp / 8;
	unsigned char p[5];
	int num_read_pixels = 0;
	int offset = 0;
	const size_t num_pixels = width * height;

	unsigned char* dest = pixels = new unsigned char[width * height * pixelSize];

	while (num_read_pixels < num_pixels) {

		if (len < pixelSize + 1)
			return false;

		memcpy(p, mem + offset, pixelSize + 1);
		if (len < pixelSize + 1)
		{
			int sdfas = 0;
		}
		len -= pixelSize + 1;
		offset += pixelSize + 1;

		mergeBytes(&(pixels[num_read_pixels]), &(p[1]), pixelSize);
		num_read_pixels++;

		unsigned int  j = p[0] & 0x7f;
		bool is_rle = p[0] & 0x80;
		if (is_rle) {
			for (unsigned int i = 0; i < j; i++) {
				mergeBytes(&(pixels[num_read_pixels]), &(p[1]), pixelSize);
				num_read_pixels++;
			}
		}
		else {
			for (unsigned int i = 0; i < j; i++) {

				if (len < pixelSize)
					return false;
				memcpy(p, mem + offset, pixelSize);
				len -= pixelSize;
				offset += pixelSize;

				mergeBytes(&(pixels[num_read_pixels]), p, pixelSize);
				num_read_pixels++;
			}
		}
	}

	if (header->attrib & 0x20) flip();

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

    // seek to start of image data
    fseek(file, header.bfOffBits, SEEK_SET);

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
			delete[] tmp;
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

	delete[] pixels;
	pixels = newPixels;

	return true;
}
