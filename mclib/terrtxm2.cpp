//---------------------------------------------------------------------------
//
// TerrTxm2.h -- File contains class definitions for the Terrain Textures
//
//	MechCommander 2 -- Microsoft
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files
#include"platform_windows.h" /*only for declaration of DeleteFile() */

#ifndef TERRTXM2_H
#include"terrtxm2.h"
#endif

#ifndef PATHS_H
#include"paths.h"
#endif

#ifndef TGAINFO_H
#include"tgainfo.h"
#endif

#ifndef CIDENT_H
#include"cident.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

#ifndef VERTEX_H
#include"vertex.h"
#endif

#ifndef TERRAIN_H
#include"terrain.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#include"resizeimage.h"

#include"platform_io.h"
#include<sys/stat.h>

#define COLOR_MAP_HEAP_SIZE				20480000
#define COLOR_TXM_HEAP_SIZE				4096
#define COLOR_MAP_TEXTURE_SIZE			256
#define COLOR_MAP_RES					float(COLOR_MAP_TEXTURE_SIZE)

bool forceShadowBurnIn = false;

void* DecodeJPG( const char* FileName, BYTE* Data, DWORD DataSize, DWORD* TextureWidth, DWORD* TextureHeight, bool TextureLoad, void *pDestSurf );

DWORD			TerrainColorMap::terrainTypeIDs[ TOTAL_COLORMAP_TYPES ] = 
{
	20000,
	20001,
	20002,
	20003,
	20004
};

#define MBOK		1
//---------------------------------------------------------------------------
// Class TerrainColorMap
void TerrainColorMap::init (void)
{
	ColorMap = NULL;
	colorMapHeap = NULL;
	colorMapRAMHeap = NULL;
	
	numTextures = 0;
	textures = NULL;
	txmRAM = NULL;
	
	numTexturesAcross = fractionPerTexture = 0.0f;
	
	colorMapStarted = false;
	
	detailTextureRAM = NULL;
	detailTextureNodeIndex = 0xffffffff;
	detailTextureTilingFactor = 30.0f;

	waterTextureRAM = NULL;
	waterTextureNodeIndex = 0xffffffff;
	waterTextureTilingFactor = 48.0f;

	numWaterDetailFrames = 0;
	int i;
	for (i = 0; i < MAX_WATER_DETAIL_TEXTURES; i++)
	{
		waterDetailNodeIndex[i] = 0xffffffff;
	}
	waterDetailFrameRate = 8;
	waterDetailTilingFactor = 45.0f;
	
	hGauss = 5.0f;
	roughDistance = -1.0f;
}

void TerrainColorMap::destroy (void)
{
	if (textures)
	{
		colorMapHeap->Free(textures);
		textures = NULL;
	}

	if (txmRAM)
	{
		for (unsigned long i=0;i<numTextures;i++)
		{
			colorMapRAMHeap->Free(txmRAM[i].ourRAM);
			txmRAM[i].ourRAM = NULL;
		}
		
		colorMapRAMHeap->Free(txmRAM);
		txmRAM = NULL;
	}

	if (ColorMap)
	{
		colorMapRAMHeap->Free(ColorMap);
		ColorMap = NULL;
	}
	
 	if (colorMapHeap)
	{
		delete colorMapHeap;
		colorMapHeap = NULL;
	}
	
 	if (colorMapRAMHeap)
	{
		delete colorMapRAMHeap;
		colorMapRAMHeap = NULL;
	}
	
 	numTextures = 0;
}

//---------------------------------------------------------------------------
void TerrainColorMap::getColorMapData (MemoryPtr ourRAM, long index, long width)
{
	long numWide = width / COLOR_MAP_TEXTURE_SIZE;

	long startCol = ((index % numWide) * COLOR_MAP_TEXTURE_SIZE);

	if (startCol > 0)
		startCol-= (index % numWide);
		
	long startRow = ((index / numWide) * width * COLOR_MAP_TEXTURE_SIZE);
	if (startRow > 0)
		startRow -= (index / numWide) * width;
	
	MemoryPtr ourColor = ColorMap + (startCol + startRow) * sizeof(DWORD);
	
	for (long i=0;i<COLOR_MAP_TEXTURE_SIZE;i++)
	{
		memcpy(ourRAM,ourColor,COLOR_MAP_TEXTURE_SIZE * sizeof(DWORD));
		ourRAM += COLOR_MAP_TEXTURE_SIZE * sizeof(DWORD);
		ourColor += width * sizeof(DWORD);
	}
}

//---------------------------------------------------------------------------
inline void fractalPass (float *heightMap, long edgeSize, long THRESHOLD, long NOISE)
{
	//Add a bunch o Gaussian noise to the new hi-res height map to break up the lines.
	// This works as follows:
	// If there is a Height Difference between the points, Add noise based on difference.
	// If there is NO height difference, DO NOT ADD NOISE.  Let smooth be smooth!
	float *noiseMap = (float *) malloc(sizeof(float) * edgeSize * edgeSize);
	for (long y=THRESHOLD;y<(edgeSize-THRESHOLD);y++)
	{
		for (long x=THRESHOLD;x<(edgeSize-THRESHOLD);x++)
		{
			if (RandomNumber(THRESHOLD) == 0)
			{
				float noise = float(NOISE * 0.5f) - RandomNumber(NOISE);
				noiseMap[x + (y * edgeSize)] = noise;
			}
			else
			{
				noiseMap[x + (y * edgeSize)] = 0.0f;
			}
		}
	}
	
	for (long i=0;i<edgeSize * edgeSize;i++)
	{
		heightMap[i] += noiseMap[i];
	}
	
	free(noiseMap);
}

//---------------------------------------------------------------------------
void rescaleMap (float *dst, float *src, long dstSize, long srcSize)
{
	Image source, dest;
	source.xSize = source.ySize = srcSize;
	source.data = src;
	
	dest.xSize = dest.ySize = dstSize;
	dest.data = dst;
	
//	zoom(&dest, &source, mitchellFilter, MitchellSupport);
	zoom(&dest, &source, triangleFilter, triangleSupport);
}

//---------------------------------------------------------------------------
void TerrainColorMap::refractalizeBaseMesh (const char *fileName, long Threshold, long Noise)
{
	//Find max and min vertex elevations for scaling below.
	float maxVertex = land->getTerrainElevation(0,0);
	float minVertex = maxVertex;
	for (long y=0;y<Terrain::realVerticesMapSide;y++)
	{
		for (long x=0;x<Terrain::realVerticesMapSide;x++)
		{
			float currentElevation = land->getTerrainElevation(y,x);
			if (currentElevation < minVertex)
				minVertex = currentElevation;
				
			if (currentElevation > maxVertex)
				maxVertex = currentElevation;
		}
	}
	
	float scalar = 0.0f;
	//Check if Map has no elevation!!
	if (maxVertex != minVertex)		
	{
		scalar = 255.0f / (maxVertex - minVertex); 
	}
		
	//Create an image which is x by x for the existing terrain.
	// Scale it into the range of 0 to 255.
	// these can be floats!!!!!!!
	float *srcData = (float *)malloc(sizeof(float) * Terrain::realVerticesMapSide * Terrain::realVerticesMapSide);
	memset(srcData,0,sizeof(float) * Terrain::realVerticesMapSide * Terrain::realVerticesMapSide); 
	
	for (int y=0;y<Terrain::realVerticesMapSide;y++)
	{
		for (long x=0;x<Terrain::realVerticesMapSide;x++)
		{
			float currentElevation = land->getTerrainElevation(y,x);
			float scaledValue = (currentElevation - minVertex) * scalar;
			srcData[x + (y * Terrain::realVerticesMapSide)] = scaledValue;
		}
	}
	
	//Spit out original Targa as well.
	MemoryPtr sourceData = (MemoryPtr)malloc(Terrain::realVerticesMapSide * Terrain::realVerticesMapSide);
	memset(sourceData,0,Terrain::realVerticesMapSide * Terrain::realVerticesMapSide);
	
	float *tmpEdge = srcData;
	MemoryPtr tmpOutput = sourceData;
	for (long i=0;i<Terrain::realVerticesMapSide * Terrain::realVerticesMapSide;i++)
	{
		*tmpOutput = (BYTE)CLAMP(*tmpEdge,BLACK_PIXEL,WHITE_PIXEL);
		tmpOutput++;
		tmpEdge++;
	}

	FullPathFileName OEMheightName;
	OEMheightName.init(terrainPath,fileName,".tga");
	
	SetFileAttributes(OEMheightName,FILE_ATTRIBUTE_NORMAL);
 	
	TGAFileHeader tgaOutput;
	tgaOutput.cm_entry_size = 0;
	tgaOutput.cm_first_entry = 0;
	tgaOutput.cm_length = 0;
	tgaOutput.color_map = 0;
	tgaOutput.height = Terrain::realVerticesMapSide;
	tgaOutput.image_descriptor = 32;	//Right side up!!
	tgaOutput.image_id_len = 0;
	tgaOutput.image_type = UNC_GRAY;
	tgaOutput.pixel_depth = 8;
	tgaOutput.width = Terrain::realVerticesMapSide;
	tgaOutput.x_origin = 0;
	tgaOutput.y_origin = 0;

	File OEMtgaFile;
	long result = OEMtgaFile.create(OEMheightName);
	if (result != NO_ERR)
		STOP(("Couldnt create height map %s on refractalize.  Error: %d",(const char*)OEMheightName,result));
	
	OEMtgaFile.write((MemoryPtr)&tgaOutput,sizeof(TGAFileHeader));
	OEMtgaFile.write(sourceData,Terrain::realVerticesMapSide * Terrain::realVerticesMapSide);
	
	OEMtgaFile.close();
 	
	long edgeSize = (numTexturesAcross * COLOR_MAP_RES);
	float *dstData = (float *)malloc(sizeof(float) * edgeSize * edgeSize);
	memset(dstData,0,sizeof(float) * edgeSize * edgeSize);

	rescaleMap(dstData,srcData,edgeSize,Terrain::realVerticesMapSide);
	
	//Now, apply refractalization to dstData.
	if (Noise)
		fractalPass(dstData,edgeSize,Threshold,Noise);
	
	//Save dstData as the height map.
	char newName[1024];
	sprintf(newName,"%s.height",fileName);
		
	FullPathFileName heightName;
	heightName.init(terrainPath,newName,".tga");
	
	SetFileAttributes(heightName,FILE_ATTRIBUTE_NORMAL);
	
	MemoryPtr outputData = (MemoryPtr)malloc(edgeSize * edgeSize);
	memset(outputData,0,sizeof(BYTE) * edgeSize * edgeSize);
	
	tmpEdge = dstData;
	tmpOutput = outputData;
	for (int i=0;i<edgeSize*edgeSize;i++)
	{
		*tmpOutput = (BYTE)CLAMP(*tmpEdge,BLACK_PIXEL,WHITE_PIXEL);
		tmpOutput++;
		tmpEdge++;
	}

	tgaOutput.cm_entry_size = 0;
	tgaOutput.cm_first_entry = 0;
	tgaOutput.cm_length = 0;
	tgaOutput.color_map = 0;
	tgaOutput.height = edgeSize;
	tgaOutput.image_descriptor = 32;	//Right side up!!
	tgaOutput.image_id_len = 0;
	tgaOutput.image_type = UNC_TRUE;
	tgaOutput.pixel_depth = 24;
	tgaOutput.width = edgeSize;
	tgaOutput.x_origin = 0;
	tgaOutput.y_origin = 0;

	//flipTopToBottom(outputData,8,edgeSize,edgeSize);
	
	File tgaFile;
	result = tgaFile.create(heightName);
	if (result != NO_ERR)
		STOP(("Couldnt create height map %s on refractalize.  Error: %d",(const char*)heightName,result));
	
	tgaFile.write((MemoryPtr)&tgaOutput,sizeof(TGAFileHeader));

	MemoryPtr outputByte = outputData;
	MemoryPtr output24Bit = (MemoryPtr)malloc(edgeSize * edgeSize * 3);
	MemoryPtr output24Temp = output24Bit;
	for (int i=0;i<edgeSize*edgeSize;i++)
	{
		*output24Temp = *outputByte;
		output24Temp++;
		*output24Temp = *outputByte;
		output24Temp++;
		*output24Temp = *outputByte;
		output24Temp++;

		outputByte++;
	}
	
	tgaFile.write(output24Bit,edgeSize * edgeSize * 3);
	tgaFile.close();
	
	free(output24Bit);
		
	//ReBurn the shadows.
	recalcLight(fileName);

	//Destroy RAM allocated.  No Leaking!!
	free(dstData);
	free(srcData);
	free(sourceData);
	free(outputData);
}

float ContrastEnhance = 1.5f;
float ShadowEnhance = 2.0f;
//---------------------------------------------------------------------------
void TerrainColorMap::burnInShadows (bool doBumpPass, const char *fileName)
{
	//-----------------------------------------------------------------
	// for each pixel in the colormap, figure out where light is and
	// darken/lighten color based on light angle and shadows
	
	//first create a height map which is pixel for pixel identical to
	// the color map.
	long pixelWidth = numTexturesAcross * COLOR_MAP_TEXTURE_SIZE;
	float *heightMap = (float *)malloc(sizeof(float) * pixelWidth * pixelWidth);
	gosASSERT(heightMap != NULL);
	memset(heightMap,0,sizeof(float) * pixelWidth * pixelWidth);
	
	float *shadowMap = (float *)malloc(sizeof(float) * pixelWidth * pixelWidth);
	gosASSERT(shadowMap != NULL);

	memset(shadowMap,0,sizeof(float) * pixelWidth * pixelWidth);
	
	float worldUnitsPerPixel = (Terrain::worldUnitsMapSide / pixelWidth);
	
	float startY = Terrain::mapTopLeft3d.y;
	float *hMap = heightMap;
	float *sMap = shadowMap;

	if (Terrain::userMin == Terrain::userMax)
	{
		//Spread 'em out so the lighting actually works!!
		Terrain::userMin = 0;
		Terrain::userMax = 500;
	}

	//First, check if there is a height Map on disk.  If so, use it, not a fractally generated one.
	bool heightMapExists = false;
	float highPoint = 0.0f;
	if (fileName)
	{
		char newName[1024];
		sprintf(newName,"%s.height",fileName);
		
		FullPathFileName heightName;
		heightName.init(terrainPath,newName,".tga");
		
		if (fileExists(heightName))
		{
			heightMapExists = true;
			
			File heightMapFile;
			long result = heightMapFile.open(heightName);
			if (result != NO_ERR)
				STOP(("Unable to find Hi-Res Height Data"));
				
			MemoryPtr tgaFileImage = (MemoryPtr)malloc(heightMapFile.fileSize());
			gosASSERT(tgaFileImage != NULL);
			
			heightMapFile.read(tgaFileImage,heightMapFile.fileSize());
			
			TGAFileHeader heightMapInfo;
			memcpy(&heightMapInfo,tgaFileImage,sizeof(TGAFileHeader));
			
			if (heightMapInfo.width != pixelWidth)
			{
				PAUSE(("Hi-Res Height Map wrong size %d. Should be %d. No shadows will be burnt.  Press Continue",heightMapInfo.width,pixelWidth));
				return;
			}

			if (heightMapInfo.image_type == UNC_TRUE)
			{
				MemoryPtr loadBuffer = tgaFileImage + sizeof(TGAFileHeader);
				if (heightMapInfo.width != heightMapInfo.height)
					STOP(("Height Map is not a perfect Square"));
				
				//-----------------------------------------------------------------
				// Check if 24 or 32 bit.  If 24, do the necessary stuff to it.
				// FIrst find MAX and MIN values for height field.
				BYTE mapMin = 255;
				BYTE mapMax = 0;
				if (heightMapInfo.pixel_depth == 24)
				{
					//24-Bit color means we must skip to every third color data.
					MemoryPtr lMap = loadBuffer;
					for (long i = 0;i<(heightMapInfo.width * heightMapInfo.width);i++)
					{
						BYTE val = *lMap;
						lMap += 3;
						if (val > mapMax)
							mapMax = val;
							
						if (val < mapMin)
							mapMin = val;
					}
				}
				else
				{
					//32-bit color means skip three instead of two bytes.
					MemoryPtr lMap = loadBuffer;
					for (long i = 0;i<(heightMapInfo.width * heightMapInfo.width);i++)
					{
						BYTE val = *lMap;
						lMap += 4;
						if (val > mapMax)
							mapMax = val;
							
						if (val < mapMin)
							mapMin = val;
					}
				}
				
				//Now create Height Map in Floating Point Space.				
				if (heightMapInfo.pixel_depth == 24)
				{
					//24-Bit color means we must skip to every third color data.
					float * hMap = heightMap;
					MemoryPtr lMap = loadBuffer;
					for (long i = 0;i<(heightMapInfo.width * heightMapInfo.width);i++)
					{
						float val = (float)(*lMap);
						lMap += 3;
						float h = Terrain::userMin;
						if (0.0 != mapMax)
						{
							h += (val - mapMin)/mapMax * ( Terrain::userMax - Terrain::userMin );
							*hMap = h * ShadowEnhance;
   						
							if (highPoint < *hMap)
								highPoint = *hMap;
								
							hMap++;
 						}
					}
				}
				else
				{
					//32-bit color means skip three instead of two bytes.
					float * hMap = heightMap;
					MemoryPtr lMap = loadBuffer;
					for (long i = 0;i<(heightMapInfo.width * heightMapInfo.width);i++)
					{
						float val = (float)(*lMap);
						lMap += 4;
						float h = Terrain::userMin;
						if (0.0 != mapMax)
						{
							h += (val - mapMin)/mapMax * ( Terrain::userMax - Terrain::userMin );
							*hMap = h * ShadowEnhance;
							
							if (highPoint < *hMap)
								highPoint = *hMap;
								
							hMap++;
						}
					}
				}
				
				free(tgaFileImage);
				tgaFileImage = NULL;
				
				//------------------------------------------------------------------------
				// Must check image_descriptor to see if we need to un upside down image.
				//bool left = (colorMapInfo.iimage_descriptor & 16) != 0;
				bool top = (heightMapInfo.image_descriptor & 32) != 0;
		
				if (!top)
				{
					flipTopToBottom((MemoryPtr)heightMap,32,heightMapInfo.width,heightMapInfo.height);
				}
			}
		}
	}
	else
	{
		long skipLong = 1;
		for (long y=0;y<pixelWidth;y++)
		{
			float startX = Terrain::mapTopLeft3d.x;
			for (long x=0;x<pixelWidth;x++)
			{
				Stuff::Vector3D pos;
				pos.x = startX;
				pos.y = startY;
				pos.z = 0.0f;
				
				if (!(x%skipLong) && !(y%skipLong))
					*hMap = land->getTerrainElevation(pos);
				else
					*hMap = 0.0f;
				
				*sMap = 0.0f;
				
				if (highPoint < *hMap)
					highPoint = *hMap;
					
				hMap++;
				sMap++;
				
				startX += worldUnitsPerPixel;
			}
			startY -= worldUnitsPerPixel;
		}
	}
	
	//----------------------------------------------------------
	// We now have a MUCH more accurate height map then we did.
	// Do normal calculations first.
	Stuff::Vector3D lightDir;
	lightDir.x = lightDir.y = 0.0f;
	lightDir.z = 1.0f;
	
	if (eye)
		lightDir = eye->lightDirection;
		
	Stuff::Vector3D shadowDir(lightDir);
	shadowDir *= worldUnitsPerPixel;		//Distance which is the per pixel distance for checking shadows
	
	//---------------------
	//Shadow Map Pass Here
	highPoint *= ShadowEnhance;
	float *thisHeight = heightMap;
	float *thisShadow = shadowMap;
	for (long y=0;y<pixelWidth;y++)
	{
		for (long x=0;x<pixelWidth;x++)
		{
			//---------------------------------------------
			// No problem
			// Generate at will!
			float v0,v1,v2,v3,v4,v5,v6,v7,v8;

			v0 = *thisHeight;
			if (x && y)
				v1 = *(thisHeight - pixelWidth - 1);
			else
				v1 = v0;

			if (y)
				v2 = *(thisHeight - pixelWidth    );
			else
				v2 = v0;

			if (y && (x < (pixelWidth - 1)))
				v3 = *(thisHeight - pixelWidth + 1);
			else
				v3 = v0;

			if (x < (pixelWidth - 1))
				v4 = *(thisHeight              + 1);
			else
				v4 = v0;

			if ((x < (pixelWidth - 1)) && (y < (pixelWidth - 1)))
				v5 = *(thisHeight + pixelWidth + 1);
			else
				v5 = v0;

			if ((y < (pixelWidth - 1)))
				v6 = *(thisHeight + pixelWidth	  );
			else
				v6 = v0;

			if (x && (y < (pixelWidth - 1)))
				v7 = *(thisHeight + pixelWidth - 1);
			else
				v7 = v0;

			if (x)
				v8 = *(thisHeight              - 1);
			else
				v8 = v0;
			
			if (Terrain::recalcShadows || forceShadowBurnIn)
			{
				//-----------------------------------------------------
				// Try and project shadow lines.
				Stuff::Vector3D vertexPos;
				vertexPos.x = ((float(x) * worldUnitsPerPixel) + Terrain::mapTopLeft3d.x);
				vertexPos.y = (Terrain::mapTopLeft3d.y - (float(y) * worldUnitsPerPixel));
				vertexPos.z = *thisHeight * ShadowEnhance;
					
				if ((shadowDir.x != 0.0f) || (shadowDir.y != 0.0f))
				{
					vertexPos.Add(vertexPos,shadowDir);
					if (!heightMapExists)
					{
						while (eye && (vertexPos.z < highPoint) && Terrain::IsValidTerrainPosition(vertexPos))
						{
							float elev = land->getTerrainElevation(vertexPos) * ShadowEnhance;
							if (elev >= vertexPos.z)
							{
								*thisShadow = -1.0f;		//Mark as shadowed
								break;						//Stop Looking!!
							}
							
							vertexPos.Add(vertexPos,shadowDir);
						}
					}
					else
					{
						float tx = x;
						float ty = y;
						while (eye && (vertexPos.z < highPoint) && Terrain::IsValidTerrainPosition(vertexPos))
						{
							//Figure out index into array from vertexPos and use REAL height data for shadows.
							long tileX = tx;
							long tileY = ty;
							
							float elev = heightMap[tileX + (tileY * pixelWidth)];
							if (elev > vertexPos.z)
							{
								*thisShadow = -1.0f;		//Mark as shadowed
								break;						//Stop Looking!!
							}

							tx += lightDir.x;
							ty -= lightDir.y;
							vertexPos.Add(vertexPos,shadowDir);
						}							
					}
				}
			}
				
			if (*thisShadow != -1.0f)
			{
				Stuff::Vector3D		normals[8];
				Stuff::Vector3D		triVect[2];
				
				//-------------------------------------
				// Tri 021
				triVect[0].x = 0.0f;
				triVect[0].y = worldUnitsPerPixel;
				triVect[0].z = (v2 - v0) * ContrastEnhance;
				
				triVect[1].x = -worldUnitsPerPixel;
				triVect[1].y = worldUnitsPerPixel;
				triVect[1].z = (v1 - v0) * ContrastEnhance;
				
				normals[0].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[0].z > 0.0);
				
				normals[0].Normalize(normals[0]);
				
				//-------------------------------------
				// Tri 032
				triVect[0].x = worldUnitsPerPixel;
				triVect[0].y = worldUnitsPerPixel;
				triVect[0].z = (v3 - v0) * ContrastEnhance;
				
				triVect[1].x = 0.0f;
				triVect[1].y = worldUnitsPerPixel;
				triVect[1].z = (v2 - v0) * ContrastEnhance;
				
				normals[1].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[1].z > 0.0);
				
				normals[1].Normalize(normals[1]);
				
				//-------------------------------------
				// Tri 043
				triVect[0].x = worldUnitsPerPixel;
				triVect[0].y = 0.0f;
				triVect[0].z = (v4 - v0) * ContrastEnhance;
	
				triVect[1].x = worldUnitsPerPixel;
				triVect[1].y = worldUnitsPerPixel;
				triVect[1].z = (v3 - v0) * ContrastEnhance;
				
			
				normals[2].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[2].z > 0.0);
				
				normals[2].Normalize(normals[2]);
				
				//-------------------------------------
				// Tri 054
				triVect[0].x = worldUnitsPerPixel;
				triVect[0].y = -worldUnitsPerPixel;
				triVect[0].z = (v5 - v0) * ContrastEnhance;
				
				triVect[1].x = worldUnitsPerPixel;
				triVect[1].y = 0.0f;
				triVect[1].z = (v4 - v0) * ContrastEnhance;
				
				normals[3].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[3].z > 0.0);
				
				normals[3].Normalize(normals[3]);
					
				//-------------------------------------
				// Tri 065
				triVect[0].x = 0;
				triVect[0].y = -worldUnitsPerPixel;
				triVect[0].z = (v6 - v0) * ContrastEnhance;
				
				triVect[1].x = worldUnitsPerPixel;
				triVect[1].y = -worldUnitsPerPixel;
				triVect[1].z = (v5 - v0) * ContrastEnhance;
				
				normals[4].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[4].z > 0.0);
				
				normals[4].Normalize(normals[4]);
				
				//-------------------------------------
				// Tri 076
				triVect[0].x = -worldUnitsPerPixel;
				triVect[0].y = -worldUnitsPerPixel;
				triVect[0].z = (v7 - v0) * ContrastEnhance;
				
				triVect[1].x = 0.0;
				triVect[1].y = -worldUnitsPerPixel;
				triVect[1].z = (v6 - v0) * ContrastEnhance;
				
				normals[5].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[5].z > 0.0);
				
				normals[5].Normalize(normals[5]);
		
				//-------------------------------------
				// Tri 087
				triVect[0].x = -worldUnitsPerPixel;
				triVect[0].y = 0.0;
				triVect[0].z = (v8 - v0) * ContrastEnhance;
				
				triVect[1].x = -worldUnitsPerPixel;
				triVect[1].y = -worldUnitsPerPixel;
				triVect[1].z = (v7 - v0) * ContrastEnhance;
				
				normals[6].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[6].z > 0.0);
				
				normals[6].Normalize(normals[6]);
		
				//-------------------------------------
				// Tri 018
				triVect[0].x = -worldUnitsPerPixel;
				triVect[0].y = worldUnitsPerPixel;
				triVect[0].z = (v1 - v0) * ContrastEnhance;
				
				triVect[1].x = -worldUnitsPerPixel;
				triVect[1].y = 0.0;
				triVect[1].z = (v8 - v0) * ContrastEnhance;
				
				normals[7].Cross(triVect[0],triVect[1]);
				gosASSERT(normals[7].z > 0.0);
				
				normals[7].Normalize(normals[7]);
				
				Stuff::Vector3D vertexNormal;
				vertexNormal.x = normals[0].x + normals[1].x + normals[2].x + normals[3].x + normals[4].x + normals[5].x + normals[6].x + normals[7].x;
				vertexNormal.y = normals[0].y + normals[1].y + normals[2].y + normals[3].y + normals[4].y + normals[5].y + normals[6].y + normals[7].y;
				vertexNormal.z = normals[0].z + normals[1].z + normals[2].z + normals[3].z + normals[4].z + normals[5].z + normals[6].z + normals[7].z;
				vertexNormal.x /= 8.0;
				vertexNormal.y /= 8.0;
				vertexNormal.z /= 8.0;
	
				gosASSERT(vertexNormal.z > 0.0);
				
				*thisShadow = vertexNormal * lightDir;
			}
			
			thisHeight++;
			thisShadow++;
		}
	}
	
	//Smooth out the shadow map to make shadows better.
	for (long i=pixelWidth;i<((pixelWidth * pixelWidth)-pixelWidth);i++)
	{
		if (shadowMap[i] == -1.0f)
		{
			float v1,v2,v3,v4,v5,v6,v7,v8;
			//All of the pixels around this one should be no more then 0.3f
			v1 = shadowMap[i - pixelWidth - 1];
			v2 = shadowMap[i - pixelWidth    ];
			v3 = shadowMap[i - pixelWidth + 1];
			v4 = shadowMap[i              + 1];
			v5 = shadowMap[i + pixelWidth + 1];
			v6 = shadowMap[i + pixelWidth    ];
			v7 = shadowMap[i + pixelWidth - 1];
			v8 = shadowMap[i              - 1];
			
			if (v1 > 0.2f)
				shadowMap[i - pixelWidth - 1] = 0.2f;
				
			if (v2 > 0.2f)
				shadowMap[i - pixelWidth    ] = 0.2f;
				
 			if (v3 > 0.2f)
				shadowMap[i - pixelWidth + 1] = 0.2f;
				
 			if (v4 > 0.2f)
				shadowMap[i              + 1] = 0.2f;
				
 			if (v5 > 0.2f)
				shadowMap[i + pixelWidth + 1] = 0.2f;
				
 			if (v6 > 0.2f)
				shadowMap[i + pixelWidth    ] = 0.2f;
				
 			if (v7 > 0.2f)
				shadowMap[i + pixelWidth - 1] = 0.2f;
				
 			if (v8 > 0.2f)
				shadowMap[i              - 1] = 0.2f;
 		}
	}	
	
	// Shadow map is now done being calculated.
	// Apply the shadows to the magical terrain.
	DWORD *cMap = (DWORD *)ColorMap;
	for (int i=0;i<(pixelWidth * pixelWidth);i++)
	{
		if (shadowMap[i] != 0.0f)
		{
			float lightFactor = 0.0f;
			if (shadowMap[i] != -1.0f)	//Otherwise, its a shadow and light is 0!!
			{
				lightFactor = shadowMap[i];
			}
			else
			{
				lightFactor = 0.0f;
			}
			
			DWORD lightr, lightg, lightb;
			lightr = eye->getLightRed(lightFactor);
			lightg = eye->getLightGreen(lightFactor);
			lightb = eye->getLightBlue(lightFactor);
			
			DWORD currentColor = *cMap;
			
			float currentBlue = currentColor & 0xff;
			float currentGreen = (currentColor >> 8) & 0xff;
			float currentRed = (currentColor >> 16) & 0xff;
			
			currentRed *= ((float)lightr / 255.0f);
			currentBlue *= ((float)lightb / 255.0f);
			currentGreen *= ((float)lightg / 255.0f);
			
			lightr = currentRed;
			lightg = currentGreen;
			lightb = currentBlue;
			
			*cMap = (0xff << 24) + (lightr << 16) + (lightg << 8) + (lightb);
		}
		cMap++;
	}
	
	//All Done.  Get rid of the height and Shadow maps
	free(heightMap);
	free(shadowMap);
}

//---------------------------------------------------------------------------
void saveTGAFile(MemoryPtr ColorMap, const char * fileName, long pixelWidth)
{
	TGAFileHeader colorMapInfo;
	colorMapInfo.image_id_len = 0;
	colorMapInfo.color_map = 0;
	colorMapInfo.image_type = UNC_TRUE;

	colorMapInfo.cm_first_entry= 0;
	colorMapInfo.cm_length = 0;
	colorMapInfo.cm_entry_size = 0;

	colorMapInfo.x_origin = 0;
	colorMapInfo.y_origin = 0;
	colorMapInfo.width = pixelWidth;
	colorMapInfo.height = pixelWidth;
	colorMapInfo.pixel_depth = 32.0;
	colorMapInfo.image_descriptor = 32;	//DO NOT FLIP!
	
	char newName[1024];
	sprintf(newName,"%s.burnin",fileName);
	
	FullPathFileName outputName;
	outputName.init(texturePath,newName,".tga");
	
	_chmod(outputName,_S_IREAD | _S_IWRITE);
	
	File outputFile;
	outputFile.create(outputName);
	
	outputFile.write((MemoryPtr)(&colorMapInfo),sizeof(TGAFileHeader));
	outputFile.write(ColorMap,pixelWidth*pixelWidth*sizeof(DWORD));
	outputFile.close();
}
	
//---------------------------------------------------------------------------
inline bool textureIsOKFormat (const char *fileName)
{
	File tgaFile;
	long result = tgaFile.open(fileName);
	if (result == NO_ERR)
	{
		struct TGAFileHeader tgaHeader;
		tgaFile.read((MemoryPtr)&tgaHeader,sizeof(TGAFileHeader));
		if (((tgaHeader.image_type == UNC_TRUE) || (tgaHeader.image_type == RLE_TRUE)) &&
			(tgaHeader.width == tgaHeader.height) &&
			((tgaHeader.width == 32) ||
			(tgaHeader.width == 64) ||
			(tgaHeader.width == 128) ||
			(tgaHeader.width == 256)))
			return true;

		tgaFile.close();
	}

	return false;
}

//---------------------------------------------------------------------------
void TerrainColorMap::resetDetailTexture (const char *fileName)
{
	if (!textureIsOKFormat(fileName))
	{
		char msg[2048];
		sprintf(msg,"Texture %s is not 24bit or 32bit, 32x32, 64x64, 128x128 or 256x256 TGA File.  Not loaded!",fileName);
		MessageBox( NULL, msg, NULL, MBOK );
		return;
	}

	mcTextureManager->removeTextureNode (detailTextureNodeIndex);
	detailTextureNodeIndex = 0xffffffff;

#if 0
	//Load up the detail texture.
	// Detail texture is named same as colormap with .detail in name.
	// if no file exists, no texture drawn.
	char dName[1024];
	sprintf(dName,"%s.detail",fileName);
		
	FullPathFileName detailFile;
	detailFile.init(texturePath,dName,".tga");

	if (fileExists(detailFile))		//Otherwise, its already 0xffffffff!!
		detailTextureNodeIndex = mcTextureManager->loadTexture(detailFile,gos_Texture_Alpha,gosHint_DontShrink);
#else
	detailTextureNodeIndex = mcTextureManager->loadTexture(fileName,gos_Texture_Alpha,gosHint_DontShrink);
#endif
}

//---------------------------------------------------------------------------
void TerrainColorMap::resetWaterTexture (const char *fileName)
{
	if (!textureIsOKFormat(fileName))
	{
		char msg[2048];
		sprintf(msg,"Texture %s is not 24bit or 32bit, 32x32, 64x64, 128x128 or 256x256 TGA File.  Not loaded!",fileName);
		MessageBox( NULL, msg, NULL, MBOK );
		return;
	}

	mcTextureManager->removeTextureNode (waterTextureNodeIndex);
	waterTextureNodeIndex = 0xffffffff;
	
#if 0
	//load up the water texture.
	// Water texture is named the same as the colormap with .water in name.
	// If no file exists, no texture drawn!!
	char dName[1024];
	sprintf(dName,"%s.water",fileName);
	
	FullPathFileName waterFile;
	waterFile.init(texturePath,dName,".tga");
	
	if (fileExists(waterFile))    //Otherwise, its already 0xffffffff!! 
		waterTextureNodeIndex = mcTextureManager->loadTexture(waterFile,gos_Texture_Solid,0);
#else
	waterTextureNodeIndex = mcTextureManager->loadTexture(fileName,gos_Texture_Solid,0);
#endif
}

//---------------------------------------------------------------------------
void TerrainColorMap::resetWaterDetailTextures (const char *fileName)
{
	//Then, load up the water detail texture(s).
	// Water detail texture is named the same as the colormap with .water0000 in name
	// Where 0000 is the frame number of the animation for the water detail.
	// If the first frame does not exist, no texture is drawn!!
	for (long i=0;i<MAX_WATER_DETAIL_TEXTURES;i++)
	{
		char waterFile[1024];
		if (strlen("0000.tga") > strlen(fileName))
		{
			strcpy(waterFile, "0000.tga");
		}
		else
		{
			strcpy(waterFile, fileName);
			char dName[1024];
			sprintf(dName,"%04d.tga",i);
			char *subStringToBeReplaced = &(waterFile[strlen(waterFile)-strlen("0000.tga")]);
			strcpy(subStringToBeReplaced, dName);
		}
		
		bool textureIsOK = true;
		if (!textureIsOKFormat(waterFile))
		{
			//PAUSE(("Texture %s is Not 24bit or 32bit TGA File.  Not loaded.  Press Continue",fileName));
			textureIsOK = false;
		}

		if (textureIsOK && fileExists(waterFile))
		{
			if (!i)		//If we found the first one OK, erase the current ones!!
			{
				for (long j=0;j<MAX_WATER_DETAIL_TEXTURES;j++)
				{
					mcTextureManager->removeTextureNode (waterDetailNodeIndex[j]);
					waterDetailNodeIndex[j] = 0xffffffff;
				}

				numWaterDetailFrames = 0;
			}

			waterDetailNodeIndex[i] = mcTextureManager->loadTexture(waterFile,gos_Texture_Alpha,gosHint_DontShrink);
			numWaterDetailFrames++;
		}
		else
		{
			if (!i)
				break;		//No water detail!
				
			waterDetailNodeIndex[i] = 0xffffffff;
		}
	}
}

//---------------------------------------------------------------------------
void TerrainColorMap::recalcLight(const char *fileName)
{
	colorMapRAMHeap = new UserHeap;
	colorMapRAMHeap->init(COLOR_MAP_HEAP_SIZE,"ColorMap");
		
	FullPathFileName colorMapName;
	colorMapName.init(texturePath,fileName,".tga");
		
	char newName[1024];
	sprintf(newName,"%s.burnin",fileName);
	
	FullPathFileName burnInName;
	burnInName.init(texturePath,newName,".tga");

	File colorMapFile;
	long result = colorMapFile.open(colorMapName);
	if (result != NO_ERR)
	{
		PAUSE(("Unable to open Terrain Color Map %s.  Cannot Re-light map.  Press Continue", (const char*)colorMapName));
		return;
	}
			
	MemoryPtr tgaFileImage = (MemoryPtr)malloc(colorMapFile.fileSize());
	gosASSERT(tgaFileImage != NULL);
		
	colorMapFile.read(tgaFileImage,colorMapFile.fileSize());
		
	TGAFileHeader colorMapInfo;
	memcpy(&colorMapInfo,tgaFileImage,sizeof(TGAFileHeader));
		
	if (colorMapInfo.image_type == UNC_TRUE)
	{
		MemoryPtr loadBuffer = tgaFileImage + sizeof(TGAFileHeader);
		if (colorMapInfo.width != colorMapInfo.height)
			STOP(("Color Map is not a perfect Square"));
			
		//-----------------------------------------------------------------
		// Check if 24 or 32 bit.  If 24, do the necessary stuff to it.
		ColorMap = (MemoryPtr)colorMapRAMHeap->Malloc(colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
		gosASSERT(ColorMap != NULL);
			
		if (colorMapInfo.pixel_depth == 24)
		{
			//24-Bit color means we must add in an opaque alpha to each 24 bits of color data.
			MemoryPtr cMap = ColorMap;
			MemoryPtr lMap = loadBuffer;
			for (long i = 0;i<(colorMapInfo.width * colorMapInfo.width);i++)
			{
				*cMap = *lMap;		//Red
				cMap++;
				lMap++;
					
				*cMap = *lMap;		//Green
				cMap++;
				lMap++;
				
				*cMap = *lMap;		//Blue
				cMap++;
				lMap++;
				
				*cMap = 0xff;		 //Alpha
				cMap++;
			}
		}
		else
		{
			//32-bit color means all we have to do is copy the buffer.
			memcpy(ColorMap,loadBuffer,colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
		}
		
		free(tgaFileImage);
		tgaFileImage = NULL;
			
		numTextures = colorMapInfo.width / COLOR_MAP_TEXTURE_SIZE;
		numTexturesAcross = numTextures;
		fractionPerTexture = 1.0f / numTextures;
			
		float checkNum = float(colorMapInfo.width) / float(COLOR_MAP_TEXTURE_SIZE);
		if (checkNum != float(numTextures))
			STOP(("Color Map is %d pixels wide which is not even divisible by %d",colorMapInfo.width,COLOR_MAP_TEXTURE_SIZE));
				
		numTextures *= numTextures;
			
		txmRAM = (ColorMapRAM *)colorMapRAMHeap->Malloc(sizeof(ColorMapRAM) * numTextures);
		gosASSERT(txmRAM != NULL);
			
		//------------------------------------------------------------------------
		// Must check image_descriptor to see if we need to un upside down image.
		//bool left = (colorMapInfo.iimage_descriptor & 16) != 0;
		bool top = (colorMapInfo.image_descriptor & 32) != 0;
	
		if (!top)
		{
			flipTopToBottom(ColorMap,32,colorMapInfo.width,colorMapInfo.height);
		}
	
		//Apply shadow map.  calc every time for now.  Save as in editor, eventually.
		burnInShadows(true,fileName);
		saveTGAFile(ColorMap,fileName,colorMapInfo.width);
						
		//Now, divide up the color map into separate COLOR_MAP_TEXTURE_SIZE textures.
		// and hand the data to the textureManager.
			
		for (unsigned long i=0;i<numTextures;i++)
		{
			mcTextureManager->removeTextureNode(textures[i].mcTextureNodeIndex);
			
			txmRAM[i].ourRAM = (MemoryPtr)colorMapRAMHeap->Malloc(sizeof(DWORD) * COLOR_MAP_TEXTURE_SIZE * COLOR_MAP_TEXTURE_SIZE);
			gosASSERT(txmRAM[i].ourRAM != NULL);
				
			getColorMapData(txmRAM[i].ourRAM,i, colorMapInfo.width);
			
			textures[i].mcTextureNodeIndex = mcTextureManager->textureFromMemory((DWORD *)txmRAM[i].ourRAM,gos_Texture_Solid,gosHint_DontShrink,COLOR_MAP_TEXTURE_SIZE);
		}
	}

	//At this point, the color Map DATA should be freeable!!
	// All of the textures have been passed to the mcTextureManager!
	for (unsigned long i=0;i<numTextures;i++)
	{
		colorMapRAMHeap->Free(txmRAM[i].ourRAM);
		txmRAM[i].ourRAM = NULL;
	}
	
	colorMapRAMHeap->Free(txmRAM);
	txmRAM = NULL;
	
	colorMapRAMHeap->Free(ColorMap);
	ColorMap = NULL;
	
	if (colorMapRAMHeap)
	{
		delete colorMapRAMHeap;
		colorMapRAMHeap = NULL;
	}
}

//---------------------------------------------------------------------------
void TerrainColorMap::resetBaseTexture (const char *fileName)
{
	//First, free up the existing colormap.
	for (unsigned long i=0;i<numTextures;i++)
	{
		mcTextureManager->removeTextureNode(textures[i].mcTextureNodeIndex);
		textures[i].mcTextureNodeIndex = 0xffffffff;
	}
			
	colorMapRAMHeap = new UserHeap;
	colorMapRAMHeap->init(COLOR_MAP_HEAP_SIZE,"ColorMap");
		
	FullPathFileName colorMapName;
	colorMapName.init(texturePath,fileName,".tga");
		
	char newName[1024];
	sprintf(newName,"%s.burnin",fileName);
	
	FullPathFileName burnInName;
	burnInName.init(texturePath,newName,".tga");

	bool burnedIn = false;
	File colorMapFile;
	long result = colorMapFile.open(burnInName);
	if (result != NO_ERR)
	{
		result = colorMapFile.open(colorMapName);
		if (result != NO_ERR)
			STOP(("Unable to open Terrain Color Map %s",(const char*)colorMapName));
	}
	else
	{
		burnedIn = true;
	}
			
	MemoryPtr tgaFileImage = (MemoryPtr)malloc(colorMapFile.fileSize());
	gosASSERT(tgaFileImage != NULL);
		
	colorMapFile.read(tgaFileImage,colorMapFile.fileSize());
		
	TGAFileHeader colorMapInfo;
	memcpy(&colorMapInfo,tgaFileImage,sizeof(TGAFileHeader));
		
	if (colorMapInfo.image_type == UNC_TRUE)
	{
		MemoryPtr loadBuffer = tgaFileImage + sizeof(TGAFileHeader);
		if (colorMapInfo.width != colorMapInfo.height)
			STOP(("Color Map is not a perfect Square"));
			
		//-----------------------------------------------------------------
		// Check if 24 or 32 bit.  If 24, do the necessary stuff to it.
		ColorMap = (MemoryPtr)colorMapRAMHeap->Malloc(colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
		gosASSERT(ColorMap != NULL);
			
		if (colorMapInfo.pixel_depth == 24)
		{
			//24-Bit color means we must add in an opaque alpha to each 24 bits of color data.
			MemoryPtr cMap = ColorMap;
			MemoryPtr lMap = loadBuffer;
			for (long i = 0;i<(colorMapInfo.width * colorMapInfo.width);i++)
			{
				*cMap = *lMap;		//Red
				cMap++;
				lMap++;
				
				*cMap = *lMap;		//Green
				cMap++;
				lMap++;
				
				*cMap = *lMap;		//Blue
				cMap++;
				lMap++;
				
				*cMap = 0xff;		 //Alpha
				cMap++;
			}
		}
		else
		{
			//32-bit color means all we have to do is copy the buffer.
			memcpy(ColorMap,loadBuffer,colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
		}
		
		free(tgaFileImage);
		tgaFileImage = NULL;
		
		numTextures = colorMapInfo.width / COLOR_MAP_TEXTURE_SIZE;
		numTexturesAcross = numTextures;
		fractionPerTexture = 1.0f / numTextures;
		
		float checkNum = float(colorMapInfo.width) / float(COLOR_MAP_TEXTURE_SIZE);
		if (checkNum != float(numTextures))
			STOP(("Color Map is %d pixels wide which is not even divisible by %d",colorMapInfo.width,COLOR_MAP_TEXTURE_SIZE));
			
		numTextures *= numTextures;
		textures = (ColorMapTextures *)colorMapHeap->Malloc(sizeof(ColorMapTextures) * numTextures);
		gosASSERT(textures != NULL);
		
		txmRAM = (ColorMapRAM *)colorMapRAMHeap->Malloc(sizeof(ColorMapRAM) * numTextures);
		gosASSERT(txmRAM != NULL);
		
		//------------------------------------------------------------------------
		// Must check image_descriptor to see if we need to un upside down image.
		//bool left = (colorMapInfo.iimage_descriptor & 16) != 0;
		bool top = (colorMapInfo.image_descriptor & 32) != 0;

		if (!top)
		{
			flipTopToBottom(ColorMap,32,colorMapInfo.width,colorMapInfo.height);
		}

		//Apply shadow map.  calc every time for now.  Save as in editor, eventually.
		if (!burnedIn)
		{
			burnInShadows(true,fileName);
			saveTGAFile(ColorMap,fileName,colorMapInfo.width);
		}
					
		//Now, divide up the color map into separate COLOR_MAP_TEXTURE_SIZE textures.
		// and hand the data to the textureManager.
		
		for (unsigned long i=0;i<numTextures;i++)
		{
			txmRAM[i].ourRAM = (MemoryPtr)colorMapRAMHeap->Malloc(sizeof(DWORD) * COLOR_MAP_TEXTURE_SIZE * COLOR_MAP_TEXTURE_SIZE);
			gosASSERT(txmRAM[i].ourRAM != NULL);
			
			getColorMapData(txmRAM[i].ourRAM,i, colorMapInfo.width);
			
			textures[i].mcTextureNodeIndex = mcTextureManager->textureFromMemory((DWORD *)txmRAM[i].ourRAM,gos_Texture_Solid,gosHint_DontShrink,COLOR_MAP_TEXTURE_SIZE);
		}
	}
	
	//At this point, the color Map DATA should be freeable!!
	// All of the textures have been passed to the mcTextureManager!
	for (int i=0;i<numTextures;i++)
	{
		colorMapRAMHeap->Free(txmRAM[i].ourRAM);
		txmRAM[i].ourRAM = NULL;
	}
	
	colorMapRAMHeap->Free(txmRAM);
	txmRAM = NULL;
	
	colorMapRAMHeap->Free(ColorMap);
	ColorMap = NULL;
	
	if (colorMapRAMHeap)
	{
		delete colorMapRAMHeap;
		colorMapRAMHeap = NULL;
	}
}

//---------------------------------------------------------------------------
//Used by editor for TacMap
void TerrainColorMap::getScaledColorMap (MemoryPtr bfr, long dWidth)
{
	bool wasColorMapAround = true;

	if (!ColorMap)
	{
		wasColorMapAround = false;
		char *fileName = Terrain::colorMapName;
		if (!fileName)
			fileName = Terrain::terrainName;

		colorMapRAMHeap = new UserHeap;
		colorMapRAMHeap->init(COLOR_MAP_HEAP_SIZE,"ColorMap");

 		FullPathFileName colorMapName;
		colorMapName.init(texturePath,fileName,".tga");
		
		char newName[1024];
		sprintf(newName,"%s.burnin",fileName);
	
		FullPathFileName burnInName;
		burnInName.init(texturePath,newName,".tga");

		//This is what I meant.  DO NOT use burnin unless we have to!
 		File colorMapFile;
		long result = colorMapFile.open(burnInName);
		if (result != NO_ERR)
		{
			result = colorMapFile.open(colorMapName);
			if (result != NO_ERR)
			{
#if defined(DEBUG) || defined(PROFILE)
				PAUSE(("Unable to open Terrain Color Map %s.  Press Continue",colorMapName));
#endif
				return;
			}
		}
			
		MemoryPtr tgaFileImage = (MemoryPtr)malloc(colorMapFile.fileSize());
		gosASSERT(tgaFileImage != NULL);
		
		colorMapFile.read(tgaFileImage,colorMapFile.fileSize());
		
		TGAFileHeader colorMapInfo;
		memcpy(&colorMapInfo,tgaFileImage,sizeof(TGAFileHeader));
		
		if (colorMapInfo.image_type == UNC_TRUE)
		{
			MemoryPtr loadBuffer = tgaFileImage + sizeof(TGAFileHeader);
			if (colorMapInfo.width != colorMapInfo.height)
				STOP(("Color Map is not a perfect Square"));
			
			//-----------------------------------------------------------------
			// Check if 24 or 32 bit.  If 24, do the necessary stuff to it.
			ColorMap = (MemoryPtr)colorMapRAMHeap->Malloc(colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
			gosASSERT(ColorMap != NULL);
			
			if (colorMapInfo.pixel_depth == 24)
			{
				//24-Bit color means we must add in an opaque alpha to each 24 bits of color data.
				MemoryPtr cMap = ColorMap;
				MemoryPtr lMap = loadBuffer;
				for (long i = 0;i<(colorMapInfo.width * colorMapInfo.width);i++)
				{
					*cMap = *lMap;		//Red
					cMap++;
					lMap++;
					
					*cMap = *lMap;		//Green
					cMap++;
					lMap++;
					
					*cMap = *lMap;		//Blue
					cMap++;
					lMap++;
					
					*cMap = 0xff;		 //Alpha
					cMap++;
				}
			}
			else
			{
				//32-bit color means all we have to do is copy the buffer.
				memcpy(ColorMap,loadBuffer,colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
			}
			
			free(tgaFileImage);
			tgaFileImage = NULL;
			
			DWORD numTextures = colorMapInfo.width / COLOR_MAP_TEXTURE_SIZE;
			numTexturesAcross = numTextures;
			
			//------------------------------------------------------------------------
			// Must check image_descriptor to see if we need to un upside down image.
			//bool left = (colorMapInfo.iimage_descriptor & 16) != 0;
			bool top = (colorMapInfo.image_descriptor & 32) != 0;
	
			if (!top)
			{
				flipTopToBottom(ColorMap,32,colorMapInfo.width,colorMapInfo.height);
			}
		}
	}

	//Grab every xth pixel from every yth scan line!
	float cMapWidth = numTexturesAcross * COLOR_MAP_TEXTURE_SIZE;
	float xSkip = cMapWidth / float(dWidth);
	float ySkip = xSkip;

	long xOffset = 0, yOffset = 0;

	DWORD *currentColor = (DWORD *)ColorMap;
	DWORD *currentBfr = (DWORD *)bfr;
	for (long i=0;i<(dWidth * dWidth);i++)
	{
		*currentBfr = *currentColor;
		xOffset++;
		if (xOffset >= dWidth)
		{
			xOffset = 0;
			yOffset++;
		}

		currentBfr++;
		currentColor = ((DWORD *)ColorMap) + long(xOffset * xSkip) + (long(yOffset*ySkip) * (long)cMapWidth);
	}

	if (!wasColorMapAround)
	{
		delete colorMapRAMHeap;
		colorMapRAMHeap = NULL;

		ColorMap = NULL;
	}
}

static long sReadIdFloat(FitIniFile* missionFile, const char *varName, float &value) {
	long result = 0;
	float tmpFloat;
	result = missionFile->readIdFloat((char *)varName, tmpFloat);
	if (NO_ERR != result) {
		//assert(false);
	} else {
		value = tmpFloat;
	}
	return result;
}

//---------------------------------------------------------------------------
long TerrainColorMap::init (char *fileName)
{
	bool usedJPG = false;
	if (!colorMapStarted)
	{
		//Load up the detail texture first.
		// Detail texture is named same as colormap with .detail in name.
		// if no file exists, no texture drawn.
		char dName[1024];
		sprintf(dName,"%s.detail",fileName);
		
		FullPathFileName detailFile;
		detailFile.init(texturePath,dName,".tga");

		if (!fileExists(detailFile))
		{
			/* If a detail texture doesn't exist for this mission, use the default detail texture.*/
			detailFile.destroy();
			sprintf(dName,"defaults" PATH_SEPARATOR "default_detail");
			detailFile.init(texturePath,dName,".tga");
		}
		if (fileExists(detailFile))		//Otherwise, its already 0xffffffff!!
			detailTextureNodeIndex = mcTextureManager->loadTexture(detailFile,gos_Texture_Alpha,gosHint_DontShrink);
		else
			gosASSERT(false);
		
		//Ok, now load up the water texture.
		// Water texture is named the same as the colormap with .water in name.
		// If no file exists, no texture drawn!!
		sprintf(dName,"%s.water",fileName);
		
		FullPathFileName waterFile;
		waterFile.init(texturePath,dName,".tga");
		
		if (!fileExists(waterFile))
		{
			/* If a water texture doesn't exist for this mission, use the default water texture.*/
			waterFile.destroy();
			sprintf(dName,"defaults" PATH_SEPARATOR "default_water");
			waterFile.init(texturePath,dName,".tga");
		}
		if (fileExists(waterFile))
			waterTextureNodeIndex = mcTextureManager->loadTexture(waterFile,gos_Texture_Solid,0);
		else
			gosASSERT(false);

		//Then, load up the water detail texture(s).
		// Water detail texture is named the same as the colormap with .water0000 in name
		// Where 0000 is the frame number of the animation for the water detail.
		// If the first frame does not exist, no texture is drawn!!
		char waterDetailBaseName[1024];
		sprintf(waterDetailBaseName,"%s.water",fileName);
		{
			sprintf(dName,"%s%04d",waterDetailBaseName,0);
			
			FullPathFileName waterFile;
			waterFile.init(texturePath,dName,".tga");
			
			if (!fileExists(waterFile))
			{
				sprintf(waterDetailBaseName,"defaults" PATH_SEPARATOR "default_water");
			}
		}
		for (long i=0;i<MAX_WATER_DETAIL_TEXTURES;i++)
		{
			waterDetailNodeIndex[i] = 0xffffffff;
		}
		sprintf(dName,"%s%04d",waterDetailBaseName,0);
		waterFile.init(texturePath,dName,".tga");
		resetWaterDetailTextures((const char *)waterFile);

		{
			detailTextureTilingFactor = 30.0f;
			waterTextureTilingFactor = 48.0f;
			waterDetailTilingFactor = 45.0f;

			FullPathFileName missionFitFilePath;
			missionFitFilePath.init(missionPath,fileName,".fit");
			FitIniFile missionFitIni;
			if (fileExists(missionFitFilePath))
			{
				missionFitIni.open((char*)(const char*)missionFitFilePath);
				long result = missionFitIni.seekBlock( "Terrain" );
				gosASSERT( result == NO_ERR );
				result = sReadIdFloat(&missionFitIni, "DetailTextureTilingFactor", detailTextureTilingFactor);
				result = sReadIdFloat(&missionFitIni, "WaterTextureTilingFactor", waterTextureTilingFactor);
				result = sReadIdFloat(&missionFitIni, "WaterDetailTilingFactor", waterDetailTilingFactor);
				missionFitIni.close();
			}
		}

		colorMapHeap = new UserHeap;
		colorMapHeap->init(COLOR_TXM_HEAP_SIZE,"ColorTXM");
		
		colorMapRAMHeap = new UserHeap;
		colorMapRAMHeap->init(COLOR_MAP_HEAP_SIZE,"ColorMap");
		
 		FullPathFileName colorMapName;
		colorMapName.init(texturePath,fileName,".tga");
		
		char newName[1024];
		sprintf(newName,"%s.burnin",fileName);
	
		FullPathFileName burnInName;
		burnInName.init(texturePath,newName,".tga");

		FullPathFileName burnInJpg;
		burnInJpg.init(texturePath,newName,".jpg");

		//NEW!!
		// Look for the JPG first.
		// If its there, read it in, pass it to the Decoder and bypass the rest of this!!
		DWORD jpgColorMapWidth = 0;
		DWORD jpgColorMapHeight = 0;

		File colorMapFile;
		long result = colorMapFile.open(burnInJpg);
		// sebi :-)
		if (result == NO_ERR && false)
		{
			long fileSize = colorMapFile.fileSize();
			MemoryPtr jpgData = (MemoryPtr)malloc(fileSize);
			colorMapFile.read(jpgData,fileSize);

			ColorMap = (MemoryPtr)DecodeJPG(burnInJpg, jpgData, fileSize, &jpgColorMapWidth, &jpgColorMapHeight, false, NULL);

			DWORD numTextures = jpgColorMapWidth / COLOR_MAP_TEXTURE_SIZE;
			numTexturesAcross = numTextures;
			fractionPerTexture = 1.0f / numTextures;
			
			float checkNum = float(jpgColorMapWidth) / float(COLOR_MAP_TEXTURE_SIZE);
			if (checkNum != float(numTextures))
				STOP(("Color Map is %d pixels wide which is not even divisible by %d",jpgColorMapWidth,COLOR_MAP_TEXTURE_SIZE));
				
			numTextures *= numTextures;
			textures = (ColorMapTextures *)colorMapHeap->Malloc(sizeof(ColorMapTextures) * numTextures);
			gosASSERT(textures != NULL);
			
			txmRAM = (ColorMapRAM *)colorMapRAMHeap->Malloc(sizeof(ColorMapRAM) * numTextures);
			gosASSERT(txmRAM != NULL);
			
			//Now, divide up the color map into separate COLOR_MAP_TEXTURE_SIZE textures.
			// and hand the data to the textureManager.
			for (unsigned long i=0;i<numTextures;i++)
			{
				txmRAM[i].ourRAM = (MemoryPtr)colorMapRAMHeap->Malloc(sizeof(DWORD) * COLOR_MAP_TEXTURE_SIZE * COLOR_MAP_TEXTURE_SIZE);
				gosASSERT(txmRAM[i].ourRAM != NULL);
				
				getColorMapData(txmRAM[i].ourRAM,i, jpgColorMapWidth);
				
				textures[i].mcTextureNodeIndex = mcTextureManager->textureFromMemory((DWORD *)txmRAM[i].ourRAM,gos_Texture_Solid,gosHint_DontShrink,COLOR_MAP_TEXTURE_SIZE);
			}
		
			free(jpgData);
			jpgData = NULL;
			colorMapStarted = true;
			usedJPG = true;
		}
		else
		{
			bool burnedIn = false;
			File colorMapFile;
			result = colorMapFile.open(burnInName);
			if (result != NO_ERR)
			{
				result = colorMapFile.open(colorMapName);
				if (result != NO_ERR)
					STOP(("Unable to open Terrain Color Map %s", (const char*)colorMapName));
			}
			else
			{
				burnedIn = true;
			}

			MemoryPtr tgaFileImage = (MemoryPtr)malloc(colorMapFile.fileSize());
			gosASSERT(tgaFileImage != NULL);

			colorMapFile.read(tgaFileImage,colorMapFile.fileSize());

			TGAFileHeader colorMapInfo;
			memcpy(&colorMapInfo,tgaFileImage,sizeof(TGAFileHeader));

			if (colorMapInfo.image_type == UNC_TRUE)
			{
				MemoryPtr loadBuffer = tgaFileImage + sizeof(TGAFileHeader);
				if (colorMapInfo.width != colorMapInfo.height)
					STOP(("Color Map is not a perfect Square"));

				//-----------------------------------------------------------------
				// Check if 24 or 32 bit.  If 24, do the necessary stuff to it.
				ColorMap = (MemoryPtr)colorMapRAMHeap->Malloc(colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
				gosASSERT(ColorMap != NULL);

				if (colorMapInfo.pixel_depth == 24)
				{
					//24-Bit color means we must add in an opaque alpha to each 24 bits of color data.
					MemoryPtr cMap = ColorMap;
					MemoryPtr lMap = loadBuffer;
					for (long i = 0;i<(colorMapInfo.width * colorMapInfo.width);i++)
					{
						*cMap = *lMap;		//Red
						cMap++;
						lMap++;

						*cMap = *lMap;		//Green
						cMap++;
						lMap++;

						*cMap = *lMap;		//Blue
						cMap++;
						lMap++;

						*cMap = 0xff;		 //Alpha
						cMap++;
					}
				}
				else
				{
					//32-bit color means all we have to do is copy the buffer.
					memcpy(ColorMap,loadBuffer,colorMapInfo.width * colorMapInfo.width * sizeof(DWORD));
				}

				free(tgaFileImage);
				tgaFileImage = NULL;

				DWORD numTextures = colorMapInfo.width / COLOR_MAP_TEXTURE_SIZE;
				numTexturesAcross = numTextures;
				fractionPerTexture = 1.0f / numTextures;

				float checkNum = float(colorMapInfo.width) / float(COLOR_MAP_TEXTURE_SIZE);
				if (checkNum != float(numTextures))
					STOP(("Color Map is %d pixels wide which is not even divisible by %d",colorMapInfo.width,COLOR_MAP_TEXTURE_SIZE));

				numTextures *= numTextures;
				textures = (ColorMapTextures *)colorMapHeap->Malloc(sizeof(ColorMapTextures) * numTextures);
				gosASSERT(textures != NULL);

				txmRAM = (ColorMapRAM *)colorMapRAMHeap->Malloc(sizeof(ColorMapRAM) * numTextures);
				gosASSERT(txmRAM != NULL);

				//------------------------------------------------------------------------
				// Must check image_descriptor to see if we need to un upside down image.
				//bool left = (colorMapInfo.iimage_descriptor & 16) != 0;
				bool top = (colorMapInfo.image_descriptor & 32) != 0;

				if (!top)
				{
					flipTopToBottom(ColorMap,32,colorMapInfo.width,colorMapInfo.height);
				}

				//Apply shadow map.  calc every time for now.  Save as in editor, eventually.
				if (!burnedIn)
				{
					burnInShadows(true,fileName);
					saveTGAFile(ColorMap,fileName,colorMapInfo.width);
				}

				//Now, divide up the color map into separate COLOR_MAP_TEXTURE_SIZE textures.
				// and hand the data to the textureManager.

				for (unsigned long i=0;i<numTextures;i++)
				{
					txmRAM[i].ourRAM = (MemoryPtr)colorMapRAMHeap->Malloc(sizeof(DWORD) * COLOR_MAP_TEXTURE_SIZE * COLOR_MAP_TEXTURE_SIZE);
					gosASSERT(txmRAM[i].ourRAM != NULL);

					getColorMapData(txmRAM[i].ourRAM,i, colorMapInfo.width);

					textures[i].mcTextureNodeIndex = mcTextureManager->textureFromMemory((DWORD *)txmRAM[i].ourRAM,gos_Texture_Solid,gosHint_DontShrink,COLOR_MAP_TEXTURE_SIZE);
				}
			}

			colorMapStarted = true;
		}
	}

	//At this point, the color Map DATA should be freeable!!
	// All of the textures have been passed to the mcTextureManager!
	for (unsigned long i=0;i<numTextures;i++)
	{
		colorMapRAMHeap->Free(txmRAM[i].ourRAM);
		txmRAM[i].ourRAM = NULL;
	}
	
	colorMapRAMHeap->Free(txmRAM);
	txmRAM = NULL;
	
	if (usedJPG)
	{
		gos_Free(ColorMap);
		ColorMap = NULL;
	}
	else
	{
		colorMapRAMHeap->Free(ColorMap);
		ColorMap = NULL;
	}
	
	if (colorMapRAMHeap)
	{
		delete colorMapRAMHeap;
		colorMapRAMHeap = NULL;
	}

	return 0;
}

float textureOffset = 0.4f;
//---------------------------------------------------------------------------
DWORD TerrainColorMap::getTextureHandle (VertexPtr vMin, VertexPtr vMax, TerrainUVData *uvData)
{
	float posX = 0.0f, posY = 0.0f, maxX = 0.0f, maxY = 0.0f;
	
	Stuff::Vector3D pos1(vMin->vx,vMin->vy,0.0f),pos2(vMax->vx,vMax->vy,0.0f);
	if (Terrain::IsValidTerrainPosition(pos1) && Terrain::IsValidTerrainPosition(pos2))
	{
		//-----------------------------------------------------------------------------------------------------
		//Since we have quads with tris in both directions, we need to determine what max and min really are!!
		if (vMin->vx > vMax->vx)
		{
			//Tris are in a weird arrangement.  Swap max and min X.
			posX = (vMax->vx - Terrain::mapTopLeft3d.x) * Terrain::oneOverWorldUnitsMapSide;
			maxX = (vMin->vx - Terrain::mapTopLeft3d.x) * Terrain::oneOverWorldUnitsMapSide;
		}
		else
		{
			//Tris in normal arrangement.  Just calc 'em out.
			posX = (vMin->vx - Terrain::mapTopLeft3d.x) * Terrain::oneOverWorldUnitsMapSide;
			maxX = (vMax->vx - Terrain::mapTopLeft3d.x) * Terrain::oneOverWorldUnitsMapSide;
		}
		
		posY = (Terrain::mapTopLeft3d.y - vMin->vy) * Terrain::oneOverWorldUnitsMapSide;
		maxY = (Terrain::mapTopLeft3d.y - vMax->vy) * Terrain::oneOverWorldUnitsMapSide;
		
		//------------------------------------------------------
		// Figure out which texture by the vertex v's position.
	   
		//We now have the position in x and y as fraction from 0.0 to 1.0
		// The texture is easy.  Just mutiply by number of textures across.
		#define EDGE_ADJUST_FACTOR		0.0005f

        long txmNumX = (posX + EDGE_ADJUST_FACTOR) * numTexturesAcross;
		long txmNumY = (posY + EDGE_ADJUST_FACTOR) * numTexturesAcross;
        //long txmNumX = (posX) * numTexturesAcross;
		//long txmNumY = (posY) * numTexturesAcross;

		long resultTexture = txmNumX + (txmNumY * numTexturesAcross);
		
		// Now we need the UV Coords in this texture.  This is the fraction from 0.0 to 1.0
		// across the texture we are as vMin and vMax.
		//
		// IMPORTANT OPTIMIZATION NOTE:  We only need to do this ONCE per terrain face at load time.
		// It never changes!!!!!!!!
		// We could calc as we get to the terrain faces and just know they're done, too!
		// Could also store in the new PostcompVertex Structure!!
		
		//Also need to adjust UVs by texturePixelAdjust to handle overlaping correctly.
		float minX = (float)txmNumX * fractionPerTexture;
		uvData->maxU = (maxX - minX) * numTexturesAcross + 0.5f / COLOR_MAP_RES;
		uvData->minU = (posX - minX) * numTexturesAcross + 0.5f / COLOR_MAP_RES;
		
		float minY = (float)txmNumY * fractionPerTexture;
		uvData->maxV = (maxY - minY) * numTexturesAcross + 0.5f / COLOR_MAP_RES;
		uvData->minV = (posY - minY) * numTexturesAcross + 0.5f / COLOR_MAP_RES;
		
		
		float textureAdjustFactor = (textureOffset / COLOR_MAP_RES);
		if (uvData->minU <= textureAdjustFactor)
			uvData->minU = textureAdjustFactor;
		//else
			//uvData->minU += xAdjust;
			
		if (uvData->minV <= textureAdjustFactor)
			uvData->minV = textureAdjustFactor;
		//else
			//uvData->minV += yAdjust;
			
		if (uvData->maxU >= 1.0f)
			uvData->maxU = 1.0f - textureAdjustFactor;
		//else
			//uvData->maxU += xAdjust;
		
		if (uvData->maxV >= 1.0f)
			uvData->maxV = 1.0f - textureAdjustFactor;
		//else
			//uvData->maxV += yAdjust;
		

#if 0
		if ((uvData->minU < 0.0f) || (uvData->minU >= 1.0f))
			STOP(("UvData our of range in minU %f",uvData->minU));
			
		if ((uvData->minV < 0.0f) || (uvData->minV >= 1.0f))
			STOP(("UvData our of range in minV %f",uvData->minV));
	 
		if ((uvData->maxU < 0.0f) || (uvData->maxU >= 1.0f))
			STOP(("UvData our of range in maxU %f",uvData->maxU));
	 
		if ((uvData->maxV < 0.0f) || (uvData->maxV >= 1.0f))
			STOP(("UvData our of range in maxV %f",uvData->maxV));
#endif
	
		mcTextureManager->get_gosTextureHandle(textures[resultTexture].mcTextureNodeIndex);		
		return textures[resultTexture].mcTextureNodeIndex;
	}
	
	uvData->minU = uvData->minV = uvData->maxU = uvData->maxV = 0.0f;
	return 0;
}

long TerrainColorMap::saveDetailTexture(const char *fileName)
{
	char dName[1024];
	sprintf(dName,"%s.detail",fileName);
	FullPathFileName detailFile;
	detailFile.init(texturePath,dName,".tga");
	return mcTextureManager->saveTexture(detailTextureNodeIndex, detailFile);
}

long TerrainColorMap::saveWaterTexture(const char *fileName)
{
	char dName[1024];
	sprintf(dName,"%s.water",fileName);
	FullPathFileName waterFile;
	waterFile.init(texturePath,dName,".tga");
	return mcTextureManager->saveTexture(waterTextureNodeIndex, waterFile);
}

long TerrainColorMap::saveWaterDetail(const char *fileName)
{
	long result = NO_ERR;
	int i;
	for (i = 0; i < getWaterDetailNumFrames(); i++)
	{
		char dName[1024];
		sprintf(dName,"%s.water%04d",fileName, i);
		FullPathFileName waterDetailFile;
		waterDetailFile.init(texturePath,dName,".tga");
		long lResult = mcTextureManager->saveTexture(waterDetailNodeIndex[i], waterDetailFile);
		if (NO_ERR != lResult)
		{
			result = lResult;
		}
	}
	for (i = getWaterDetailNumFrames(); i < MAX_WATER_DETAIL_TEXTURES; i++)
	{
		char dName[1024];
		sprintf(dName,"%s.water%04d",fileName, i);
		FullPathFileName waterDetailFile;
		waterDetailFile.init(texturePath,dName,".tga");
		File textureFile;
		long textureFileOpenResult = textureFile.open(waterDetailFile);
		textureFile.close();
		if (NO_ERR != textureFileOpenResult)
		{
			break;
		}
		textureFileOpenResult = textureFile.create(waterDetailFile);
		textureFile.deleteFile(); /*what is this supposed to do?*/
		textureFile.close();
		DeleteFile(waterDetailFile);
	}
	return result;
}

long TerrainColorMap::saveTilingFactors(FitIniFile *fitFile)
{
	long result = NO_ERR;
	fitFile->writeIdFloat( "DetailTextureTilingFactor", getDetailTilingFactor() );
	fitFile->writeIdFloat( "WaterTextureTilingFactor", getWaterTextureTilingFactor() );
	fitFile->writeIdFloat( "WaterDetailTilingFactor", getWaterDetailTilingFactor() );
	return result;
}
//---------------------------------------------------------------------------

