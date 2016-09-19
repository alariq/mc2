//-------------------------------------------------
//
// Generic Image Resize code.
//
// Used to refractalize the terrain.
//
// Based on Graphics Gems III General Filtered Image Rescaling
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef RESIZEIMAGE_H
#define RESIZEIMAGE_H

#ifndef DSTD_H
#include"dstd.h"
#endif

#include<malloc.h>
#include<memory.h>
#include<math.h>

#define BLACK_PIXEL 0.0f
#define WHITE_PIXEL	255.0f
#define RESIZE_PI	3.1415926

float CLAMP (float x, float min, float max)
{
	if (x < min)
		return min;
		
	if (x > max)
		return max;

	return x;
}

typedef struct {
	long 	xSize;
	long 	ySize;
	float 	*data;
} Image;

float getPixel (Image *image, long x, long y)
{
	if ((x < 0) || (x >= image->xSize) || (y < 0) || (y > image->ySize))
		return 0.0f;
		
	float *p = image->data + (y*image->xSize);
	return p[x];
}

void getRow (float *row, Image *image, long y)
{
	if ((y < 0) || (y >= image->ySize))
		return;
		
	memcpy(row,image->data + (y * image->xSize), image->ySize * sizeof(float));
}

void getCol (float *col, Image *image, long x)
{
	if ((x < 0) || (x >= image->xSize))
		return;

	for (long i=0;i<image->ySize;i++)
	{
		*col = image->data[x + (i * image->xSize)];
		col++;
	}
}

float putPixel (Image *image, long x, long y, float data)
{
	if ((x < 0) || (x >= image->xSize) || (y < 0) || (y >= image->ySize))
		return 0.0f;
		
	float *p = image->data + (y*image->xSize);
	return (p[x] = data);
}

//Filters

#define filterSupport 	(1.0)
#define boxSupport 		(0.5)
#define triangleSupport (1.0)
#define bellSupport		(1.5)
#define bSplineSupport	(2.0)
#define Lanczos3Support (3.0)
#define MitchellSupport	(2.0)

double filter (double t)
{
	if (t < 0.0)
		t = -t;
		
	if (t < 1.0)
		return((2.0 * t - 3.0) * t * t + 1.0);
	
	return 0.0;
}

double boxFilter (double t)
{
	if ((t > -0.5) && (t <= 0.5))
		return 1.0;
		
	return 0.0;
}

double triangleFilter (double t)
{
	if (t < 0.0)
		t = -t;
		
	if (t < 1.0)
		return (1.0 - t);
		
	return 0.0;
}

double bellFilter (double t)
{
	if (t < 0.0)
		t = -t;
		
	if (t < 0.5)
		return (0.75 - (t*t));
		
	if (t < 1.5)
	{
		t = (t - 1.5);
		return (0.5 * (t*t));
	}
	
	return 0.0;
}

double bSplineFilter (double t)
{
	double tt;
	
	if (t < 0.0)
		t = -t;
		
	if (t < 1.0)
	{
		tt = t * t;
		return ((0.5 * tt * t) - tt + (2.0/3.0));
	}
	else if (t < 2.0)
	{
		t = 2.0 - t;
		return ((1.0 / 6.0) * (t * t * t));
	}
	
	return 0.0;
}

double sinc (double t)
{
	t *= RESIZE_PI;
	if (t != 0.0)
		return (sin(t) / t);
		
	return 1.0;
}

double lanczos3Filter (double t)
{
	if (t < 0.0)
		t = -t;
		
	if (t < 3.0)
		return (sinc(t) * sinc(t/3.0));
		
	return 0.0;
}

double mitchellFilter (double t)
{
	double B = (1.0 / 3.0);
	double C = (1.0 / 3.0);
	
	double tt;
	
	tt = t * t;
	if (t < 0.0)
		t = -t;
		
	if (t < 1.0)
	{
		t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt))
			+ ((-18.0 + 12.0 * B + 6.0 * C) * tt)
			+ (6.0 - 2.0 * B));
			
		return (t / 6.0);
	}
	else if (t < 2.0)
	{
		t = (((-1.0 * B - 6.0 * C) * (t * tt))
			+ ((6.0 * B + 30.0 * C) * tt)
			+ ((-12.0 * B - 48.0 * C) * t)
			+ (8.0 * B + 24.0 * C));
			
		return (t / 6.0);
	}
	
	return 0.0;
}

typedef struct {
	long pixel;
	double weight;
} CONTRIB;

typedef struct {
	long n;
	CONTRIB *p;
} CLIST;

CLIST *contrib;

Image *newImage(long xSize, long ySize)
{
	Image *image = (Image *)malloc(sizeof(Image));
	if (image)
	{
		image->data = (float *)malloc(sizeof(float) * xSize * ySize);
		memset(image->data,0,sizeof(float) * xSize * ySize);
		
		image->xSize = xSize;
		image->ySize = ySize;
	}
	
	return image;
}

void freeImage (Image *image)
{
	free(image->data);
	free(image);
}

void zoom (Image *dst, Image *src, double (*filter)(double t), double fwidth)
{
	Image *tmp;
	double xscale, yscale;
	long i,j,k;
	long n;
	double center, left, right;
	double width, fscale, weight;
	float *raster;
	
	//Create Intermediate image to hold horizontal zoom
	tmp = newImage(dst->xSize,src->ySize);
	xscale = (double)dst->xSize / (double)src->xSize;
	yscale = (double)dst->ySize / (double)src->ySize;
	
	//pre-calculate filter contributions for a row
	contrib = (CLIST *)malloc(dst->xSize * sizeof(CLIST));
	memset(contrib,0,dst->xSize * sizeof(CLIST));
	
	if (xscale < 1.0)
	{
		width = fwidth / xscale;
		fscale = 1.0 / xscale;
		for (i=0;i<dst->xSize;++i)
		{
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)malloc((width * 2 + 1) * sizeof(CONTRIB));
			
			center = (double)i / xscale;
			left = ceil(center - width);
			right = floor(center + width);
			
			for (j=left;j<=right;++j)
			{
				weight = center - (double)j;
				weight = (*filter)(weight / fscale) / fscale;
				if (j < 0)
				{
					n = -j;
				}
				else if (j >= src->xSize)
				{
					n = (src->xSize - j) + src->xSize - 1;
				}
				else
				{
					n = j;
				}
				
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}
	else
	{
		for (i=0;i<dst->xSize;++i)
		{
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)malloc((fwidth * 2 + 1) * sizeof(CONTRIB));
			
			center = (double)i / xscale;
			left = ceil(center - fwidth);
			right = floor(center + fwidth);
			
			for (j=left;j<=right;++j)
			{
				weight = center - (double)j;
				weight = (*filter)(weight);
				if (j < 0)
				{
					n = -j;
				}
				else if (j >= src->xSize)
				{
					n = (src->xSize - j) + src->xSize - 1;
				}
				else
				{
					n = j;
				}
				
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}
	
	//Apply filter to zoom horizontally from src to tmp
	raster = (float *)malloc(src->xSize * sizeof(float));
	memset(raster,0,src->xSize * sizeof(float));
	
	for (k=0;k<tmp->ySize; ++k)
	{
		getRow(raster,src, k);
		for (i=0;i<tmp->xSize;++i)
		{
			weight = 0.0;
			for (j=0;j<contrib[i].n;++j)
			{
				weight += raster[contrib[i].p[j].pixel] * contrib[i].p[j].weight;
			}
			
			putPixel(tmp,i,k,(float)CLAMP(weight,BLACK_PIXEL,WHITE_PIXEL));
		}
	}
	
	free(raster);
	raster = NULL;
	
	for (i=0;i<tmp->xSize; ++i)
	{
		free(contrib[i].p);
		contrib[i].p = NULL;
	}
	
	free(contrib);
	contrib = NULL;
	
	//pre-calculate filter contributions for horizontal filter weights
	contrib = (CLIST *)malloc(dst->ySize * sizeof(CLIST));
	memset(contrib,0,dst->ySize * sizeof(CLIST));
	
	if (yscale < 1.0)
	{
		width = fwidth / yscale;
		fscale = 1.0 / yscale;
		for (i=0;i<dst->ySize;++i)
		{
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)malloc((width * 2 + 1) * sizeof(CONTRIB));
			
			center = (double)i / yscale;
			left = ceil(center - width);
			right = floor(center + width);
			
			for (j=left;j<=right;++j)
			{
				weight = center - (double)j;
				weight = (*filter)(weight / fscale) / fscale;
				if (j < 0)
				{
					n = -j;
				}
				else if (j >= tmp->ySize)
				{
					n = (tmp->ySize - j) + tmp->ySize - 1;
				}
				else
				{
					n = j;
				}
				
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}
	else
	{
		for (i=0;i<dst->ySize;++i)
		{
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)malloc((fwidth * 2 + 1) * sizeof(CONTRIB));
			
			center = (double)i / yscale;
			left = ceil(center - fwidth);
			right = floor(center + fwidth);
			
			for (j=left;j<=right;++j)
			{
				weight = center - (double)j;
				weight = (*filter)(weight);
				if (j < 0)
				{
					n = -j;
				}
				else if (j >= tmp->ySize)
				{
					n = (tmp->ySize - j) + tmp->ySize - 1;
				}
				else
				{
					n = j;
				}
				
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}
	
	//Apply filter to zoom vertically from tmp to dst
	raster = (float *)malloc(sizeof(float) * tmp->ySize);
	memset(raster,0,sizeof(float) * tmp->ySize); 
	
	for (k=0;k<dst->xSize;++k)
	{
		getCol(raster,tmp,k);
		for (i=0;i<dst->ySize;++i)
		{
			weight = 0.0;
			for (j=0;j<contrib[i].n;++j)
			{
				weight += raster[contrib[i].p[j].pixel] * contrib[i].p[j].weight;
			}
			
			putPixel(dst,k,i,(float)CLAMP(weight,BLACK_PIXEL,WHITE_PIXEL));
		}
	}
	
	free(raster);
	
	for (i=0;i<dst->ySize; ++i)
	{
		free(contrib[i].p);
		contrib[i].p = NULL;
	}
	
	free(contrib);
	contrib = NULL;
	
	freeImage(tmp);
}

void rescaleMap (float *dst, float *src, long dstSize, long srcSize);

//----------------------------------------------------------------------------
#endif

