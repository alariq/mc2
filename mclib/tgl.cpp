//-------------------------------------------------------------------------------
//
// Tiny Geometry Layer
//
// For MechCommander 2 -- Copyright (c) 1999 Microsoft
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TGL_H
#include"tgl.h"
#endif

#ifndef CLIP_H
#include"clip.h"
#endif

#ifndef TIMING_H
#include"timing.h"
#endif

#ifndef TERRAIN_H
#include"terrain.h"
#endif

#ifndef CAMERA_H
#include"camera.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#include "platform_str.h"
#include <stddef.h> // linux offsetof()

//-------------------------------------------------------------------------------
// Include Files
#include<toolos.hpp>

#define USE_ASSEMBLY
//-------------------------------------------------------------------------------
// Statics
Stuff::LinearMatrix4D 	*TG_Shape::s_cameraOrigin = NULL;
Stuff::Matrix4D			*TG_Shape::s_cameraToClip = NULL;
Stuff::LinearMatrix4D	TG_Shape::s_worldToCamera;
Stuff::Matrix4D			TG_Shape::s_worldToClip;

float					TG_Shape::viewMulX = 0.0;
float					TG_Shape::viewAddX = 0.0;
float					TG_Shape::viewMulY = 0.0;
float					TG_Shape::viewAddY = 0.0;

DWORD					TG_Shape::fogColor = 0xffffffff;
float					TG_Shape::fogStart = 0.0f;
float					TG_Shape::fogFull = 0.0f;

TG_LightPtr				*TG_Shape::s_listOfLights = NULL;
DWORD					TG_Shape::s_numLights = 0;

Stuff::LinearMatrix4D 	TG_Shape::s_lightToShape[MAX_LIGHTS_IN_WORLD];
Stuff::Vector3D			TG_Shape::s_lightDir[MAX_LIGHTS_IN_WORLD];
Stuff::Vector3D			TG_Shape::s_rootLightDir[MAX_LIGHTS_IN_WORLD];
Stuff::Vector3D			TG_Shape::s_spotDir[MAX_LIGHTS_IN_WORLD];

UserHeapPtr 			TG_Shape::tglHeap = NULL;

DWORD					TG_Shape::lighteningLevel = 0;

TG_VertexPool 			*colorPool = NULL;
TG_GOSVertexPool 		*vertexPool = NULL;
TG_DWORDPool			*facePool = NULL;
TG_ShadowPool			*shadowPool = NULL;
TG_TrianglePool			*trianglePool = NULL;

//-------------------------------------------------------------------------------
extern bool useVertexLighting;
extern bool useFaceLighting;
extern bool hasGuardBand;
extern bool useFog;
extern DWORD BaseVertexColor;
bool drawOldWay = false;
extern bool useShadows;
bool useLocalShadows = false;

bool renderTGLShapes = true;

bool silentMode = false;		//Used for automated builds to keep errors from popping up.

//-------------------------------------------------------------------------------
// Parse Functions
void GetNumberData (char *rawData, char *result)
{
	long startIndex = 0;
	long endIndex = 0;
	while (	(rawData[startIndex] != '+') &&
			(rawData[startIndex] != '-') &&
			(rawData[startIndex] != '.') &&
			((rawData[startIndex] < '0') ||
			(rawData[startIndex] > '9')))
	{
		startIndex++;
	}

	endIndex = startIndex;
	while (	(rawData[endIndex] == '+') ||
			(rawData[endIndex] == '-') ||
			(rawData[endIndex] == '.') ||
			((rawData[endIndex] >= '0') &&
			(rawData[endIndex] <= '9')))
	{
		endIndex++;
	}

	strncpy(result,&rawData[startIndex],endIndex - startIndex);
	result[endIndex-startIndex] = 0;
}	

//-------------------------------------------------------------------------------
void GetWordData (char *rawData, char *result)
{
	long startIndex = 0;
	long endIndex = 0;
	while (	(rawData[startIndex] < 'A') ||
			((rawData[startIndex] > 'Z') &&
			(rawData[startIndex] < 'a')) ||
			(rawData[startIndex] > 'z'))
	{
		startIndex++;
	}

	endIndex = startIndex;
	while (	((rawData[endIndex] >= 'A') && (rawData[endIndex] <= 'Z')) ||
			(rawData[endIndex] == '_') ||
			((rawData[endIndex] >= 'a') && (rawData[endIndex] <= 'z')) )
	{
		endIndex++;
	}

	strncpy(result,&rawData[startIndex],endIndex - startIndex);
	result[endIndex-startIndex] = 0;
}	

//-------------------------------------------------------------------------------
void GetNameData (char *rawData, char *result)
{
	long startIndex = 0;
	long endIndex = 0;
	while (	(rawData[startIndex] != '"') )
	{
		startIndex++;
	}

	startIndex++;
	endIndex = startIndex;
	while (	(rawData[endIndex] != '"') )
	{
		endIndex++;
	}

	strncpy(result,&rawData[startIndex],endIndex - startIndex);
	result[endIndex-startIndex] = 0;
}	

//-------------------------------------------------------------------------------
// Class TG_TypeNode

//-------------------------------------------------------------------------------
 void *TG_TypeNode::operator new (size_t mySize)
 {
	void *result = TG_Shape::tglHeap->Malloc(mySize);
	return result;
 }
 
//-------------------------------------------------------------------------------
 void TG_TypeNode::operator delete (void *us)
 {
	TG_Shape::tglHeap->Free(us);
 }
 
//-------------------------------------------------------------------------------
// Frees memory and resets locals to defaults.
void TG_TypeNode::destroy (void)
{
	init();
}

//-------------------------------------------------------------------------------
// This function creates an instance of a TG_Shape from a TG_TypeNode.  Saves RAM.
TG_ShapePtr TG_TypeNode::CreateFrom (void)
{
	TG_ShapePtr newShape = NULL;
	newShape = (TG_ShapePtr)TG_Shape::tglHeap->Malloc(sizeof(TG_Shape));
	gosASSERT(newShape != NULL);
	
	//listOfVertices
	newShape->numVertices = 0;
	newShape->listOfColors = NULL;
	newShape->listOfVertices = NULL;
	newShape->listOfShadowVertices = NULL;

	//listOfTriangles
	newShape->numTriangles = 0;
	newShape->listOfTriangles = NULL;

	//listOfVisibleFaces
	newShape->numVisibleFaces = 0;
	newShape->listOfVisibleFaces = NULL;

	//listOfVisibleShadows
	newShape->numVisibleShadows = 0;
	newShape->listOfVisibleShadows = NULL;

	//Other Data
	newShape->myType = this;
	if (newShape->myType == NULL)
		STOP(("ShapeType NULL.  BAD Shape in MAX FILE"));
		
	newShape->aRGBHighlight = 0x00000000;
	newShape->lightsOut = false;
	newShape->recalcShadows = true;
	newShape->shapeScalar = 0.0f;

	for (long i=0;i<MAX_SHADOWS;i++)
		newShape->shadowsVisible[i] = false;

	newShape->isSpotlight = (S_strnicmp(newShape->getNodeName(),"SpotLight_",10) == 0);
	newShape->isWindow = (S_strnicmp(newShape->getNodeName(),"LitWin_",6) == 0);

	if (newShape->isSpotlight)
		newShape->noShadow = true;
	else
		newShape->noShadow = false;

	return newShape;
}

//-------------------------------------------------------------------------------
//Function return 0 is OK.  -1 if file is not ASE Format or missing data.
//This function simply parses the ASE buffers handed to it.  This allows
//users to load the ase file themselves and manage their own memory for it.
//It allocates memory for internal Lists.  These are straight tglHeap->Mallocs at present.
//
long TG_TypeNode::MakeFromHelper (BYTE *aseBuffer, const char *fileName)
{
	//------------------------------------------
	// Store off the Node Names.
	char *nodeName = strstr((char *)aseBuffer,ASE_NODE_NAME);
	gosASSERT(nodeName != NULL);

	char nodeString[1024];
	nodeName += strlen(ASE_NODE_NAME)+1;
	GetNameData(nodeName,nodeString);
	
#ifdef _DEBUG
	if (strlen(nodeString) >= TG_NODE_ID)
	{
		if (!silentMode)
			PAUSE(("WARNING: Node ID %s in Shape %s is greater then %d characters!!",nodeString,fileName, TG_NODE_ID-1));
	}
#endif

	strncpy(nodeId,nodeString, TG_NODE_ID-1);
	nodeId[TG_NODE_ID - 1] = '\0'; //sebi
		 
	char* parentName = strstr((char *)nodeName,ASE_NODE_PARENT);

	//-------------------------------------------------------------------
	// Must also check to make sure we HAVE a parent.
	// We will get the next GeomObject's parent if we don't check length!
	if ((parentName != NULL) && ((parentName - nodeName) < MAX_SCAN_LENGTH))
	{
		parentName += strlen(ASE_NODE_PARENT)+1;
		GetNameData(parentName,nodeString);
		
		strncpy(parentId,nodeString, TG_NODE_ID-1);
		parentId[TG_NODE_ID - 1] = '\0'; //sebi
	}
	else
	{
		strcpy(parentId,"None");
	}

	//----------------------------------------------------
	// Store off NODE ABS position for heirarchy
	nodeName = strstr((char *)aseBuffer,ASE_NODE_POS);
	gosASSERT(nodeName != NULL);

	nodeName += strlen(ASE_NODE_POS)+1;

	char numData[512];

	GetNumberData(nodeName,numData);
	nodeCenter.x = -(float)atof(numData);
	nodeName += strlen(numData)+1;

	GetNumberData(nodeName,numData);
	nodeCenter.z = (float)atof(numData);
	nodeName += strlen(numData)+1;

	GetNumberData(nodeName,numData);
	nodeCenter.y = (float)atof(numData);

	return(0);
}	

//-------------------------------------------------------------------------------
void TG_TypeNode::LoadBinaryCopy (File &binFile)
{
	//Other Data
	nodeCenter.x = binFile.readFloat();
	nodeCenter.y = binFile.readFloat();
	nodeCenter.z = binFile.readFloat();
	
	relativeNodeCenter.x = binFile.readFloat();
	relativeNodeCenter.y = binFile.readFloat();
	relativeNodeCenter.z = binFile.readFloat();
	
	binFile.read((MemoryPtr)nodeId,TG_NODE_ID);
	binFile.read((MemoryPtr)parentId,TG_NODE_ID);
}

//-------------------------------------------------------------------------------
void TG_TypeNode::SaveBinaryCopy (File &binFile)
{
	//What kind of thing is this
	binFile.writeInt(TYPE_NODE);
	
	//Other Data
	binFile.writeFloat(nodeCenter.x);
	binFile.writeFloat(nodeCenter.y);
	binFile.writeFloat(nodeCenter.z);
	
	binFile.writeFloat(relativeNodeCenter.x);
	binFile.writeFloat(relativeNodeCenter.y);
	binFile.writeFloat(relativeNodeCenter.z);
	
	binFile.write((MemoryPtr)nodeId,TG_NODE_ID);
	binFile.write((MemoryPtr)parentId,TG_NODE_ID);
}

//-------------------------------------------------------------------------------
// Class TG_Shape && TG_TypeShape

//-------------------------------------------------------------------------------
// TG_TypeShape
// This function creates an instance of a TG_Shape from a TG_TypeShape.  Saves RAM.
TG_ShapePtr TG_TypeShape::CreateFrom (void)
{
	TG_ShapePtr newShape = NULL;
	//newShape = (TG_ShapePtr)TG_Shape::tglHeap->Malloc(sizeof(TG_Shape));
	void* memarea = TG_Shape::tglHeap->Malloc(sizeof(TG_Shape));
	newShape = ::new(memarea) TG_Shape();
	gosASSERT(newShape != NULL);
	
	//listOfVertices
	newShape->numVertices = numTypeVertices;
	if (numTypeVertices)
	{
		//Mark with invalid pointer so we now to get RAM from POOL! 	
		newShape->listOfVertices = NULL;
		newShape->listOfColors = NULL;
	}
	else
	{
		//Otherwise mark with NULL so we know not to reference!!
		newShape->listOfVertices = NULL;
		newShape->listOfColors = NULL;
	}

	//listOfShadowVertices
	if (numTypeVertices)
	{
		//These are UNIQUE to each instance and do not change much per frame.
		// Thus, store them between frames to save processing time!
		newShape->listOfShadowVertices = (TG_ShadowVertexPtr)TG_Shape::tglHeap->Malloc(sizeof(TG_ShadowVertex) * numTypeVertices * MAX_SHADOWS);
		gosASSERT(newShape->listOfShadowVertices != NULL);

		memset(newShape->listOfShadowVertices,0xff,sizeof(TG_ShadowVertex) * numTypeVertices * MAX_SHADOWS);
		
		//Mark with invalid pointer so we now to get RAM from POOL! 	
		newShape->listOfShadowTVertices = NULL;
	}
	else
	{
		//Otherwise mark with NULL so we know not to reference!!
		newShape->listOfShadowVertices = NULL;
		newShape->listOfShadowTVertices = NULL;
	}

	//listOfTriangles
	newShape->numTriangles = numTypeTriangles;
	if (numTypeTriangles)
	{
		//Mark with invalid pointer so we now to get RAM from POOL! 	
		newShape->listOfTriangles = NULL;
	}
	else
	{
		//Otherwise mark with NULL so we know not to reference!!
		newShape->listOfTriangles = NULL;
	}

	//listOfVisibleFaces
	newShape->numVisibleFaces = 0;
	if (numTypeTriangles)
	{
		//Mark with invalid pointer so we now to get RAM from POOL! 	
		newShape->listOfVisibleFaces = NULL;
	}
	else
	{
		//Otherwise mark with NULL so we know not to reference!!
		newShape->listOfVisibleFaces = NULL;
	}

	//listOfVisibleShadows
	newShape->numVisibleShadows = 0;
	if (numTypeTriangles)
	{
		//Mark with invalid pointer so we now to get RAM from POOL! 	
		newShape->listOfVisibleShadows = NULL;
	}
	else
	{
		//Otherwise mark with NULL so we know not to reference!!
		newShape->listOfVisibleShadows = NULL;
	}

	//Other Data
	newShape->myType = this;
	if (newShape->myType == NULL)
		STOP(("ShapeType NULL.  BAD Shape in MAX FILE"));
 	
	newShape->aRGBHighlight = 0x00000000;
	newShape->lightsOut = false;
	newShape->recalcShadows = true;
	newShape->shapeScalar = 0.0f;

	for (long i=0;i<MAX_SHADOWS;i++)
		newShape->shadowsVisible[i] = false;
		
	newShape->isSpotlight = (S_strnicmp(newShape->getNodeName(),"SpotLight_",10) == 0);
	newShape->isWindow = (S_strnicmp(newShape->getNodeName(),"LitWin_",6) == 0);

	if (newShape->isSpotlight)
		newShape->noShadow = true;
	else
		newShape->noShadow = false;

	return newShape;
}

//-------------------------------------------------------------------------------
void TG_TypeShape::LoadBinaryCopy (File &binFile)
{
	gosVERTEX_FORMAT_RECORD vdecl[] =
	{ 
		{0, 3, false, sizeof(TG_HWTypeVertex), 0, gosVERTEX_ATTRIB_TYPE::FLOAT },
		{1, 3, false, sizeof(TG_HWTypeVertex), offsetof(TG_HWTypeVertex, normal) , gosVERTEX_ATTRIB_TYPE::FLOAT },
		{2, 4, true, sizeof(TG_HWTypeVertex), offsetof(TG_HWTypeVertex, aRGBLight) , gosVERTEX_ATTRIB_TYPE::UNSIGNED_BYTE},
		{3, 2, false, sizeof(TG_HWTypeVertex), offsetof(TG_HWTypeVertex, u) , gosVERTEX_ATTRIB_TYPE::FLOAT },
	};

	vdecl_ = gos_CreateVertexDeclaration(vdecl, sizeof(vdecl) / sizeof(gosVERTEX_FORMAT_RECORD));

	//listOfTypeVertices
	numTypeVertices = binFile.readInt();
	if (numTypeVertices)
	{
		listOfTypeVertices = (TG_TypeVertexPtr)TG_Shape::tglHeap->Malloc(sizeof(TG_TypeVertex) * numTypeVertices);
		gosASSERT(listOfTypeVertices != NULL);

		binFile.read((MemoryPtr)listOfTypeVertices,sizeof(TG_TypeVertex) * numTypeVertices);

	}
	else
	{
		listOfTypeVertices = NULL;
	}

	//listOfTypeTriangles
	numTypeTriangles = binFile.readInt();
	if (numTypeTriangles)
	{
		listOfTypeTriangles = (TG_TypeTrianglePtr)TG_Shape::tglHeap->Malloc(sizeof(TG_TypeTriangle) * numTypeTriangles);
		gosASSERT(listOfTypeTriangles != NULL);

		binFile.read((MemoryPtr)listOfTypeTriangles,sizeof(TG_TypeTriangle) * numTypeTriangles);
	}
	else
	{
		listOfTypeTriangles = NULL;
	}

	//listOfTextures
	numTextures = binFile.readInt();
	if (numTextures)
	{
		listOfTextures = (TG_TinyTexturePtr)TG_Shape::tglHeap->Malloc(sizeof(TG_TinyTexture) * numTextures);
		gosASSERT(listOfTextures != NULL);

		binFile.read((MemoryPtr)listOfTextures,sizeof(TG_TinyTexture) * numTextures);
	}
	else
	{
		listOfTextures = NULL;
	}

	if (numTypeVertices)
	{
		// no real index buffer for now...
		uint32_t num_vertices = numTypeTriangles * 3;
		uint32_t num_indices = numTypeTriangles * 3;

		uint16_t* ib = new uint16_t[num_indices];
		TG_HWTypeVertex* vb = new TG_HWTypeVertex[num_vertices];

		for (uint32_t i = 0; i < numTypeTriangles; i++)
		{
			TG_TypeTriangle triType = listOfTypeTriangles[i];

			for (uint32_t j = 0; j < 3; ++j)
			{
				vb[3 * i + j].aRGBLight = listOfTypeVertices[triType.Vertices[j]].aRGBLight;
				vb[3 * i + j].normal = listOfTypeVertices[triType.Vertices[j]].normal;
				vb[3 * i + j].position = listOfTypeVertices[triType.Vertices[j]].position;

				ib[3 * i + j] = 3 * i + j;
			}

			vb[3*i + 0].u = triType.uvdata.u0;
			vb[3*i + 0].v = triType.uvdata.v0;
			vb[3*i + 1].u = triType.uvdata.u1;
			vb[3*i + 1].v = triType.uvdata.v1;
			vb[3*i + 2].u = triType.uvdata.u2;
			vb[3*i + 2].v = triType.uvdata.v2;
		}

		ib_ = gos_CreateBuffer(gosBUFFER_TYPE::INDEX, gosBUFFER_USAGE::STATIC_DRAW, sizeof(uint16_t), num_indices, ib);
		vb_ = gos_CreateBuffer(gosBUFFER_TYPE::VERTEX, gosBUFFER_USAGE::STATIC_DRAW, sizeof(TG_HWTypeVertex), num_vertices, vb);

		delete[] ib;
		delete[] vb;
	}


	//Other Data
	nodeCenter.x = binFile.readFloat();
	nodeCenter.y = binFile.readFloat();
	nodeCenter.z = binFile.readFloat();
	
	relativeNodeCenter.x = binFile.readFloat();
	relativeNodeCenter.y = binFile.readFloat();
	relativeNodeCenter.z = binFile.readFloat();
	
	alphaTestOn = false;
	filterOn = true;

	binFile.read((MemoryPtr)nodeId,TG_NODE_ID);
	binFile.read((MemoryPtr)parentId,TG_NODE_ID);
}

//-------------------------------------------------------------------------------
void TG_TypeShape::SaveBinaryCopy (File &binFile)
{
	//What kind of thing is this
	binFile.writeInt(SHAPE_NODE);
	
	//listOfTypeVertices
	binFile.writeInt(numTypeVertices);
	if (numTypeVertices)
	{
		binFile.write((MemoryPtr)listOfTypeVertices,sizeof(TG_TypeVertex) * numTypeVertices);
	}

	//listOfTypeTriangles
	binFile.writeInt(numTypeTriangles);
	if (numTypeTriangles)
	{
		binFile.write((MemoryPtr)listOfTypeTriangles,sizeof(TG_TypeTriangle) * numTypeTriangles);
	}

	//listOfTextures
	binFile.writeInt(numTextures);
	if (numTextures)
	{
		binFile.write((MemoryPtr)listOfTextures,sizeof(TG_TinyTexture) * numTextures);
	}

	//Other Data
	binFile.writeFloat(nodeCenter.x);
	binFile.writeFloat(nodeCenter.y);
	binFile.writeFloat(nodeCenter.z);
	
	binFile.writeFloat(relativeNodeCenter.x);
	binFile.writeFloat(relativeNodeCenter.y);
	binFile.writeFloat(relativeNodeCenter.z);
	
	binFile.write((MemoryPtr)nodeId,TG_NODE_ID);
	binFile.write((MemoryPtr)parentId,TG_NODE_ID);
}

//-------------------------------------------------------------------------------
//Frees memory and resets locals to defaults.
void TG_TypeShape::destroy (void)
{
	if (listOfTypeVertices)
		TG_Shape::tglHeap->Free(listOfTypeVertices);
	listOfTypeVertices = NULL;

	if (listOfTypeTriangles)
		TG_Shape::tglHeap->Free(listOfTypeTriangles);
	listOfTypeTriangles = NULL;

	if (listOfTextures)
		TG_Shape::tglHeap->Free(listOfTextures);
	listOfTextures = NULL;

	numTypeVertices = numTypeTriangles = numTextures = 0;

	if (vb_) {
		gos_DestroyBuffer(vb_);
		vb_ = 0;
	}

	if (ib_) {
		gos_DestroyBuffer(ib_);
		ib_ = 0;
	}

	if (vdecl_) {
		gos_DestroyVertexDeclaration(vdecl_);
		vdecl_ = 0;
	}
}	

//-------------------------------------------------------------------------------
//Frees memory and resets locals to defaults.
void TG_Shape::destroy (void)
{
	//THIS IS CORRECT!!!!
	// They come from a common pool now, Deleteing them would be BAD!!
	listOfVertices = NULL;
	listOfColors = NULL;
	listOfTriangles = NULL;
	listOfVisibleFaces = NULL;
	listOfVisibleShadows = NULL;
	listOfShadowTVertices = NULL;

	if (listOfShadowVertices)
		tglHeap->Free(listOfShadowVertices);
	listOfShadowVertices = NULL;

	numVertices = numTriangles = numVisibleFaces = 0;
}	

//-------------------------------------------------------------------------------
//Function return 0 is OK.  -1 if file is not ASE Format or missing data.
//This function simply parses the ASE buffers handed to it.  This allows
//users to load the ase file themselves and manage their own memory for it.
//It allocates memory for internal Lists.  These are straight tglHeap->Mallocs at present.
//
// NOTE: Only takes the first HELPOBJECT from the ASE file.  Multi-object
// Files will require user intervention to parse!!
long TG_TypeShape::MakeFromHelper (BYTE *aseBuffer, const char *fileName)
{
	//------------------------------------------
	// Store off the Node Names.
	char *nodeName = strstr((char *)aseBuffer,ASE_NODE_NAME);
	gosASSERT(nodeName != NULL);

	char nodeString[1024];
	nodeName += strlen(ASE_NODE_NAME)+1;
	GetNameData(nodeName,nodeString);
	
#ifdef _DEBUG
	if (strlen(nodeString) >= TG_NODE_ID)
	{
		if (!silentMode)
			PAUSE(("WARNING: Node ID %s in Shape %s is greater then %d characters!!",nodeString,fileName, TG_NODE_ID-1));
	}
#endif

	strncpy(nodeId,nodeString, TG_NODE_ID-1);
	nodeId[TG_NODE_ID - 1] = '\0';
		 
	char* parentName = strstr((char *)nodeName,ASE_NODE_PARENT);

	//-------------------------------------------------------------------
	// Must also check to make sure we HAVE a parent.
	// We will get the next GeomObject's parent if we don't check length!
	if ((parentName != NULL) && ((parentName - nodeName) < MAX_SCAN_LENGTH))
	{
		parentName += strlen(ASE_NODE_PARENT)+1;
		GetNameData(parentName,nodeString);
		
		strncpy(parentId,nodeString, TG_NODE_ID-1);
		parentId[TG_NODE_ID - 1] = '\0';
	}
	else
	{
		strcpy(parentId,"None");
	}

	//----------------------------------------------------
	// Store off NODE ABS position for heirarchy
	nodeName = strstr((char *)aseBuffer,ASE_NODE_POS);
	gosASSERT(nodeName != NULL);

	nodeName += strlen(ASE_NODE_POS)+1;

	char numData[512];

	GetNumberData(nodeName,numData);
	nodeCenter.x = -(float)atof(numData);
	nodeName += strlen(numData)+1;

	GetNumberData(nodeName,numData);
	nodeCenter.z = (float)atof(numData);
	nodeName += strlen(numData)+1;

	GetNumberData(nodeName,numData);
	nodeCenter.y = (float)atof(numData);

	//----------------------------------------
	// Find the Number of Vertices
	numTypeVertices = 0;

	//---------------------------------------
	// Find the Number of faces
	numTypeTriangles = 0;

	return(0);
}	

//-------------------------------------------------------------------------------
//Function return 0 is OK.  -1 if file is not ASE Format or missing data.
//This function simply parses the ASE buffers handed to it.  This allows
//users to load the ase file themselves and manage their own memory for it.
//It allocates memory for internal Lists.  These are straight tglHeap->Mallocs at present.
//
// NOTE: Only takes the first GEOMOBJECT from the ASE file.  Multi-object
// Files will require user intervention to parse!!
long TG_TypeShape::ParseASEFile (BYTE *aseBuffer, const char *fileName)
{
	//------------------------------------------
	// Store off the Node Names.
	char *nodeName = strstr((char *)aseBuffer,ASE_NODE_NAME);
	gosASSERT(nodeName != NULL);

	char nodeString[1024];
	nodeName += strlen(ASE_NODE_NAME)+1;
	GetNameData(nodeName,nodeString);

#ifdef _DEBUG
	if (strlen(nodeString) >= TG_NODE_ID)
	{
		if (!silentMode)
			PAUSE(("WARNING: Node ID %s in Shape %s is greater then %d characters!!",nodeString,fileName, TG_NODE_ID-1));
	}
#endif

	//sebi: ORIG BUG FIX srtrncpy will not add 0 at the end of source string bigger then destination one
	//strncpy(nodeId,nodeString, 24);
	strncpy(nodeId,nodeString, TG_NODE_ID-1);
	nodeId[TG_NODE_ID - 1] = '\0';
	
	char* parentName = strstr((char *)nodeName,ASE_NODE_PARENT);

	//-------------------------------------------------------------------
	// Must also check to make sure we HAVE a parent.
	// We will get the next GeomObject's parent if we don't check length!
	if ((parentName != NULL) && ((parentName - nodeName) < MAX_SCAN_LENGTH))
	{
		parentName += strlen(ASE_NODE_PARENT)+1;
		GetNameData(parentName,nodeString);
		
		strncpy(parentId,nodeString, TG_NODE_ID-1);
		parentId[TG_NODE_ID - 1] = '\0';
	}
	else
	{
		strcpy(parentId,"None");
	}

	//----------------------------------------------------
	// Store off NODE ABS position for heirarchy
	nodeName = strstr((char *)aseBuffer,ASE_NODE_POS);
	gosASSERT(nodeName != NULL);

	nodeName += strlen(ASE_NODE_POS)+1;

	char numData[512];

	GetNumberData(nodeName,numData);
	nodeCenter.x = -(float)atof(numData);
	nodeName += strlen(numData)+1;

	GetNumberData(nodeName,numData);
	nodeCenter.z = (float)atof(numData);
	nodeName += strlen(numData)+1;

	GetNumberData(nodeName,numData);
	nodeCenter.y = (float)atof(numData);

	//----------------------------------------
	// Find the Number of Vertices
	char *vertexCount = strstr((char *)aseBuffer,ASE_NUM_VERTEX);
	gosASSERT(vertexCount != NULL);
	
	//sebi: ORIG BUG FIX
	//vertexCount += strlen(ASE_NUM_FACE)+1;
	vertexCount += strlen(ASE_NUM_VERTEX)+1;
	numTypeVertices = atol(vertexCount);
	if (numTypeVertices == 0)			//NO Data for this node.  WARN and then do NOT allocate anything!
	{
#ifdef _DEBUG
		if (!silentMode)
			PAUSE(("WARNING: No Vertices for Shape Node %s in Shape %s!",nodeId,fileName));
#endif
		init();

		return (0);
	}

	//---------------------------------------
	// Find the Number of faces
	char *faceCount = strstr((char *)aseBuffer,ASE_NUM_FACE);
	gosASSERT(faceCount != NULL);
		
	faceCount += strlen(ASE_NUM_FACE)+1;
	numTypeTriangles = atol(faceCount);
	gosASSERT(numTypeTriangles != 0);

	//-------------------
	// Vertex Data Next.
	listOfTypeVertices = (TG_TypeVertexPtr)TG_Shape::tglHeap->Malloc(sizeof(TG_TypeVertex) * numTypeVertices);
	gosASSERT(listOfTypeVertices != NULL);

	memset(listOfTypeVertices,0xff,sizeof(TG_TypeVertex) * numTypeVertices);

	//---------------------------
	// Create vertex normal list
	int *vertexNormalCount = (int*)TG_Shape::tglHeap->Malloc(sizeof(int) * numTypeVertices);
	gosASSERT(vertexNormalCount != NULL);

	memset(vertexNormalCount,0,sizeof(int) * numTypeVertices);

	for (int i=0;i<numTypeVertices;i++)
	{
		char numberData[256];

		//------------------------------------------------
		// First the Vertex Position
		char vertexID[256];
		sprintf(vertexID,"%s% 5d",ASE_MESH_VERTEX_ID,i);

		char *vertexData = strstr((char *)aseBuffer,vertexID);
		gosASSERT(vertexData != NULL);
			
		vertexData += strlen(vertexID)+1;

		GetNumberData(vertexData,numberData);
		vertexData += strlen(numberData)+1;
		listOfTypeVertices[i].position.x = -(float)atof(numberData);

		GetNumberData(vertexData,numberData);
		vertexData += strlen(numberData)+1;
		listOfTypeVertices[i].position.z = (float)atof(numberData);

		GetNumberData(vertexData,numberData);
		listOfTypeVertices[i].position.y = (float)atof(numberData);

		//------------------------------------------------
		// Next, Vertex Normal
		// These are "funny" because MAX writes out a
		// separate normal for each instance of the vertex
		// wrt a face.  So we must catch EVERY instance
		// and average together and then normalize.
		// As such, we must now extract during the Face part below
		// Just init the normal here!
		listOfTypeVertices[i].normal.x = 
		listOfTypeVertices[i].normal.y = 
		listOfTypeVertices[i].normal.z = 0.0f;

		//-----------------------------------------------------
		// Next, Read in Vertex Colors

		listOfTypeVertices[i].aRGBLight 			= 0xff000000;

		//-----------------------------------------------------
	}

	//---------------------------------------
	// Face Data Next.
	listOfTypeTriangles = (TG_TypeTrianglePtr)TG_Shape::tglHeap->Malloc(sizeof(TG_TypeTriangle) * numTypeTriangles);
	gosASSERT(listOfTypeTriangles != NULL);

	memset(listOfTypeTriangles,0xff,sizeof(TG_TypeTriangle) * numTypeTriangles);

	for (int i=0;i<numTypeTriangles;i++)
	{
		char numberData[256];

		//-----------------------------------------------
		// First the Vertices for the face
		char faceId[256];
		sprintf(faceId,"%s% 5d:",ASE_MESH_FACE_ID,i);

		char *faceData = strstr((char *)aseBuffer,faceId);
		gosASSERT(faceData != NULL);

		faceData = strstr(faceData,"A:");
		gosASSERT(faceData != NULL);
			
		GetNumberData(faceData,numberData);
		listOfTypeTriangles[i].Vertices[0] = atol(numberData);

		faceData = strstr(faceData,"B:");
		gosASSERT(faceData != NULL);

		GetNumberData(faceData,numberData);
		listOfTypeTriangles[i].Vertices[1] = atol(numberData);

		faceData = strstr(faceData,"C:");
		gosASSERT(faceData != NULL);
	
		GetNumberData(faceData,numberData);
		listOfTypeTriangles[i].Vertices[2] = atol(numberData);

		if ((numTextures/2) == 1)		//We don't need the MATLID, its always 0.  Now divBytwo to load shadow texture as well.
			listOfTypeTriangles[i].localTextureHandle = 0;
		else
		{
			//Must parse MATLID to find material ID.
			faceData = strstr(faceData,ASE_FACE_MATERIAL_ID);
			gosASSERT(faceData != NULL);

			faceData += strlen(ASE_FACE_MATERIAL_ID)+1;

			GetNumberData(faceData,numberData);

			int matLid = atol(numberData);
			if (matLid >= (numTextures/2))
				matLid = (numTextures/2) - 1;

			listOfTypeTriangles[i].localTextureHandle = matLid;

		}

		listOfTypeTriangles[i].renderStateFlags = 0;		//No Flags yet.

		//---------------------------------------------------------------------
		// UVData for the face.
		// This should be nummies!
		//
		//Load up the TFaces
		sprintf(faceId,"%s",ASE_NUM_TVFACES);

		faceData = strstr((char *)aseBuffer,faceId);
		if (faceData)
		{
			faceData += strlen(faceId);
			GetNumberData(faceData,numberData);
	
			Stuff::Vector3D tVFaces;
	
			sprintf(faceId,"%s %d",ASE_MESH_TFACE_ID,i);
	
			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);
	
			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			tVFaces.x = (float)atol(numberData);
			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			tVFaces.y = (float)atol(numberData);
			faceData += strlen(numberData)+1;
	
			GetNumberData(faceData,numberData);
			tVFaces.z = (float)atol(numberData);
	
			//Load up the TVERT0
			sprintf(faceId,"%s",ASE_NUM_TVERTEX);
	
			faceData = strstr((char *)aseBuffer,faceId);
			gosASSERT(faceData != NULL);
	
			sprintf(faceId,"%s %d",ASE_MESH_TVERT_ID,(int)tVFaces.x);
	
			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);
	
			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			listOfTypeTriangles[i].uvdata.u0 = (float)atof(numberData);
	
			if ((listOfTypeTriangles[i].uvdata.u0 > 100.0f) || (listOfTypeTriangles[i].uvdata.u0 < -100.0f))
			{
#ifdef _DEBUG
				if (!silentMode)
					PAUSE(("WARNING: U0 is %f which is out of range in Shape %s!",listOfTypeTriangles[i].uvdata.u0,fileName));
#endif
				listOfTypeTriangles[i].uvdata.u0 = 0.0f;
			}
	
			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			listOfTypeTriangles[i].uvdata.v0 = 1.0f - (float)atof(numberData);
	
			if ((listOfTypeTriangles[i].uvdata.v0 > 100.0f) || (listOfTypeTriangles[i].uvdata.v0 < -100.0f))
			{
#ifdef _DEBUG
				if (!silentMode)
					PAUSE(("WARNING: V0 is %f which is out of range in Shape %s!",listOfTypeTriangles[i].uvdata.v0,fileName));
#endif
				listOfTypeTriangles[i].uvdata.v0 = 0.0f;
			}
	
			//Load up the TVERT1
			sprintf(faceId,"%s",ASE_NUM_TVERTEX);
	
			faceData = strstr((char *)aseBuffer,faceId);
			gosASSERT(faceData != NULL);
	
			sprintf(faceId,"%s %d",ASE_MESH_TVERT_ID,(int)tVFaces.y);
	
			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);
	
			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			listOfTypeTriangles[i].uvdata.u1 = (float)atof(numberData);
			if ((listOfTypeTriangles[i].uvdata.u1 > 100.0f) || (listOfTypeTriangles[i].uvdata.u1 < -100.0f))
			{
#ifdef _DEBUG
				if (!silentMode)
					PAUSE(("WARNING: U1 is %f which is out of range in Shape %s!",listOfTypeTriangles[i].uvdata.u1,fileName));
#endif
				listOfTypeTriangles[i].uvdata.u1 = 0.0f;
			}
	
			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			listOfTypeTriangles[i].uvdata.v1 = 1.0f - (float)atof(numberData);
			if ((listOfTypeTriangles[i].uvdata.v1 > 100.0f) || (listOfTypeTriangles[i].uvdata.v1 < -100.0f))
			{
#ifdef _DEBUG
				if (!silentMode)
					PAUSE(("WARNING: V1 is %f which is out of range in Shape %s!",listOfTypeTriangles[i].uvdata.v1,fileName));
#endif
				listOfTypeTriangles[i].uvdata.v1 = 0.0f;
			}
	
			//Load up the TVERT2
			sprintf(faceId,"%s",ASE_NUM_TVERTEX);
	
			faceData = strstr((char *)aseBuffer,faceId);
			gosASSERT(faceData != NULL);
	
			sprintf(faceId,"%s %d",ASE_MESH_TVERT_ID,(int)tVFaces.z);
	
			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);
	
			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			listOfTypeTriangles[i].uvdata.u2 = (float)atof(numberData);
			if ((listOfTypeTriangles[i].uvdata.u2 > 100.0f) || (listOfTypeTriangles[i].uvdata.u2 < -100.0f))
			{
#ifdef _DEBUG
				if (!silentMode)
					PAUSE(("WARNING: U2 is %f which is out of range in Shape %s!",listOfTypeTriangles[i].uvdata.u2,fileName));
#endif
				listOfTypeTriangles[i].uvdata.u2 = 0.0f;
			}
	
			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			listOfTypeTriangles[i].uvdata.v2 = 1.0f - (float)atof(numberData);
			if ((listOfTypeTriangles[i].uvdata.v2 > 100.0f) || (listOfTypeTriangles[i].uvdata.v2 < -100.0f))
			{
#ifdef _DEBUG
				if (!silentMode)
					PAUSE(("WARNING: V2 is %f which is out of range in Shape %s!",listOfTypeTriangles[i].uvdata.v2,fileName));
#endif
				listOfTypeTriangles[i].uvdata.v2 = 0.0f;
			}
		}
		else
		{
			listOfTypeTriangles[i].uvdata.u0 = 
			listOfTypeTriangles[i].uvdata.v0 = 
   			listOfTypeTriangles[i].uvdata.u1 = 
   			listOfTypeTriangles[i].uvdata.v1 = 
   			listOfTypeTriangles[i].uvdata.u2 = 
   			listOfTypeTriangles[i].uvdata.v2 = 0.0f;
		}

		//------------------------------------------------------------

		//---------------------------------------------------------------------
		// Color Data for the face.
		//Load up the CFaces
		sprintf(faceId,"%s",ASE_NUM_CVFACES);

		faceData = strstr((char *)aseBuffer,faceId);
		if (faceData)
		{
			gosASSERT(faceData != NULL);

			faceData += strlen(faceId);
			GetNumberData(faceData,numberData);

			Stuff::Vector3D cVFaces;

			sprintf(faceId,"%s %d",ASE_MESH_CFACE_ID,i);

			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);

			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			cVFaces.x = (float)atol(numberData);
			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			cVFaces.y = (float)atol(numberData);
			faceData += strlen(numberData)+1;

			GetNumberData(faceData,numberData);
			cVFaces.z = (float)atol(numberData);

			//Load up the VERTCOL0
			sprintf(faceId,"%s",ASE_NUM_CVERTEX);

			faceData = strstr((char *)aseBuffer,faceId);
			gosASSERT(faceData != NULL);

			sprintf(faceId,"%s %d",ASE_MESH_VERTCOL_ID,(int)cVFaces.x);

			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);

			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			float red = (float)atof(numberData);

			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			float green = (float)atof(numberData);

			faceData += strlen(numberData)+1;

			GetNumberData(faceData,numberData);
			float blue = (float)atof(numberData);

			DWORD redC = red * 0xff;
			DWORD greenC = green * 0xff;
			DWORD blueC = blue * 0xff;

			listOfTypeVertices[listOfTypeTriangles[i].Vertices[0]].aRGBLight = (0xff << 24) + (redC << 16) + (greenC << 8) + blueC;

			//Load up the VERTCOL1
			sprintf(faceId,"%s",ASE_NUM_CVERTEX);

			faceData = strstr((char *)aseBuffer,faceId);
			gosASSERT(faceData != NULL);

			sprintf(faceId,"%s %d",ASE_MESH_VERTCOL_ID,(int)cVFaces.y);

			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);

			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			red = (float)atof(numberData);

			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			green = (float)atof(numberData);

			faceData += strlen(numberData)+1;

			GetNumberData(faceData,numberData);
			blue = (float)atof(numberData);

			redC = red * 0xff;
			greenC = green * 0xff;
			blueC = blue * 0xff;

			listOfTypeVertices[listOfTypeTriangles[i].Vertices[1]].aRGBLight = (0xff << 24) + (redC << 16) + (greenC << 8) + blueC;

			//Load up the VERTCOL2
			sprintf(faceId,"%s",ASE_NUM_CVERTEX);

			faceData = strstr((char *)aseBuffer,faceId);
			gosASSERT(faceData != NULL);

			sprintf(faceId,"%s %d",ASE_MESH_VERTCOL_ID,(int)cVFaces.z);

			faceData = strstr(faceData,faceId);
			gosASSERT(faceData != NULL);

			faceData += strlen(faceId);
				
			GetNumberData(faceData,numberData);
			red = (float)atof(numberData);

			faceData += strlen(numberData)+1;
				
			GetNumberData(faceData,numberData);
			green = (float)atof(numberData);

			faceData += strlen(numberData)+1;

			GetNumberData(faceData,numberData);
			blue = (float)atof(numberData);

			redC = red * 0xff;
			greenC = green * 0xff;
			blueC = blue * 0xff;

			listOfTypeVertices[listOfTypeTriangles[i].Vertices[2]].aRGBLight = (0xff << 24) + (redC << 16) + (greenC << 8) + blueC;
		}
		else
		{
			listOfTypeVertices[listOfTypeTriangles[i].Vertices[0]].aRGBLight = 
			listOfTypeVertices[listOfTypeTriangles[i].Vertices[1]].aRGBLight = 
			listOfTypeVertices[listOfTypeTriangles[i].Vertices[2]].aRGBLight = 0xff000000;
		}

		//------------------------------------------------------------

		//------------------------------------------------------------
		// Dig out the face Normal.
		sprintf(faceId,"%s %d",ASE_FACE_NORMAL_ID,i);
		faceData = strstr((char *)aseBuffer,faceId);
		gosASSERT(faceData != NULL);		

		faceData += strlen(faceId);
			
		GetNumberData(faceData,numberData);
		listOfTypeTriangles[i].faceNormal.x = -(float)atof(numberData);
		faceData += strlen(numberData)+1;
			
		GetNumberData(faceData,numberData);
		listOfTypeTriangles[i].faceNormal.z = (float)atof(numberData);
		faceData += strlen(numberData)+1;

		GetNumberData(faceData,numberData);
		listOfTypeTriangles[i].faceNormal.y = (float)atof(numberData);

		//------------------------------------------------------------
		char vertexID[256];
		sprintf(vertexID,"%s",ASE_VERTEX_NORMAL_ID);
		char *vertexData = strstr((char *)faceData,vertexID);
		GetNumberData(vertexData,numberData);
		int vertexNum = atol(numberData);
		vertexNormalCount[vertexNum]++;

		vertexData += strlen(vertexID)+strlen(numberData)+1;

			GetNumberData(vertexData,numberData);
			vertexData += strlen(numberData)+1;
			listOfTypeVertices[vertexNum].normal.x += -(float)atof(numberData);

			GetNumberData(vertexData,numberData);
			vertexData += strlen(numberData)+1;
			listOfTypeVertices[vertexNum].normal.z += (float)atof(numberData);

			GetNumberData(vertexData,numberData);
			listOfTypeVertices[vertexNum].normal.y += (float)atof(numberData);

		vertexData = strstr((char *)vertexData,vertexID);
		GetNumberData(vertexData,numberData);
		vertexNum = atol(numberData);
		vertexNormalCount[vertexNum]++;

		vertexData += strlen(vertexID)+strlen(numberData)+1;

			GetNumberData(vertexData,numberData);
			vertexData += strlen(numberData)+1;
			listOfTypeVertices[vertexNum].normal.x += -(float)atof(numberData);

			GetNumberData(vertexData,numberData);
			vertexData += strlen(numberData)+1;
			listOfTypeVertices[vertexNum].normal.z += (float)atof(numberData);

			GetNumberData(vertexData,numberData);
			listOfTypeVertices[vertexNum].normal.y += (float)atof(numberData);

		vertexData = strstr((char *)vertexData,vertexID);
		GetNumberData(vertexData,numberData);
		vertexNum = atol(numberData);
		vertexNormalCount[vertexNum]++;

		vertexData += strlen(vertexID)+strlen(numberData)+1;

			GetNumberData(vertexData,numberData);
			vertexData += strlen(numberData)+1;
			listOfTypeVertices[vertexNum].normal.x += -(float)atof(numberData);

			GetNumberData(vertexData,numberData);
			vertexData += strlen(numberData)+1;
			listOfTypeVertices[vertexNum].normal.z += (float)atof(numberData);

			GetNumberData(vertexData,numberData);
			listOfTypeVertices[vertexNum].normal.y += (float)atof(numberData);

		//------------------------------------------------------------
	}

	//-----------------------------------------------
	// Must average together the vertexNormals
	for (int i=0;i<numTypeVertices;i++)
	{
		if (vertexNormalCount[i] == 0)
		{
			listOfTypeVertices[i].normal.x = 0.0f;
			listOfTypeVertices[i].normal.y = 0.0f;
			listOfTypeVertices[i].normal.z = 0.0f;
		}
		else
		{
			listOfTypeVertices[i].normal.x /= float(vertexNormalCount[i]);
			listOfTypeVertices[i].normal.y /= float(vertexNormalCount[i]);
			listOfTypeVertices[i].normal.z /= float(vertexNormalCount[i]);

			if (listOfTypeVertices[i].normal.GetLength() > Stuff::SMALL)
				listOfTypeVertices[i].normal.Normalize(listOfTypeVertices[i].normal);
		}
	}

	TG_Shape::tglHeap->Free(vertexNormalCount);
	return(0);
}	

//-------------------------------------------------------------------------------
//Function returns 0 if OK.  -1 if file not found or file not ASE Format.		
//This function loads the ASE file into the TG_Triangle and TG_Vertex lists.
//It allocates memory.  These are straight tglHeap->Mallocs at present.
//
// NOTE: Only takes the first GEOMOBJECT from the ASE file.  Multi-object
// Files will require user intervention to parse!!
int TG_TypeShape::LoadTGShapeFromASE (const char *fileName)
{
	//-----------------------------------------------------
	// Fully loads and parses an ASE File.  These files are
	// output by 3D Studio MAX's ASCII Scene Exporter.

	//------------------------------------------
	// Check if exists by getting size
	long aseFileSize = gos_FileSize(fileName);
	gosASSERT(aseFileSize > 0);

	//---------------------------------------
	// Create Buffer to read entire file into
	BYTE *aseContents = (BYTE *)TG_Shape::tglHeap->Malloc(aseFileSize);
	gosASSERT(aseContents != NULL);

	//---------------------------------------
	// Open File and read it and close it
	HGOSFILE aseFile;
	gos_OpenFile(&aseFile,fileName,READONLY);

#ifdef _DEBUG
	long dataRead = 
#endif
		gos_ReadFile(aseFile,aseContents,aseFileSize);
	gosASSERT(dataRead == aseFileSize);

	gos_CloseFile(aseFile);

	//----------------------------------------
	// Check for valid ASE Header data.
	gosASSERT(S_strnicmp(ASE_HEADER,(char *)aseContents,strlen(ASE_HEADER)) == 0);

	//---------------------------------------
	// Find the number of Textures
	char textureId[256];
	char *textureData;

	char numberData[256];
	sprintf(textureId,ASE_MATERIAL_COUNT);
	textureData = strstr((char *)aseContents,textureId);
	if (!textureData)
		return(-1);

	textureData += strlen(ASE_MATERIAL_COUNT)+1;
	GetNumberData(textureData,numberData);
	long numMaterials = atol(numberData);

	numTextures = 0;
	unsigned char *aseBuffer = aseContents;
	for (long nmt=0;nmt<numMaterials;nmt++)
	{
		sprintf(textureId,ASE_MATERIAL_CLASS);
		textureData = strstr((char *)aseBuffer,textureId);
		if (!textureData)
			return(-1);

		textureData += strlen(ASE_MATERIAL_CLASS)+1;
		GetWordData(textureData,numberData);

		if (strstr(numberData,"Standard"))
		{
			numTextures++;
		}
		else if (strstr(numberData,"Multi"))
		{
			//----------------------------------------------------
			//NOT SUPPORTED YET!  Multiple Textures per building!
			// Support NOW!  Must have trees!
			textureData = strstr(textureData,ASE_SUBMATERIAL_COUNT);
			gosASSERT(textureData != NULL);

			textureData += strlen(ASE_SUBMATERIAL_COUNT);
			GetNumberData(textureData,numberData);

			numTextures += atol(numberData);
		}

		aseBuffer = (unsigned char *)textureData;
	}

	listOfTextures = (TG_TinyTexturePtr)TG_Shape::tglHeap->Malloc(sizeof(TG_TinyTexture) * numTextures);
	gosASSERT(listOfTextures != NULL);

	memset(listOfTextures,0x0,sizeof(TG_TinyTexture) * numTextures);

	for (long i=0;i<numTextures;i++)
	{
		//-------------------------------------------------------------------------------
		// Get and store texture Name.  Will need multiple for Multi-Sub if implemented
		// NEVER GET ACTUAL TEXTURES HERE.  ALWAYS HAPPENS IN MULTI!!!!!!!!!!
		// Just set 'em!
		listOfTextures[i].mcTextureNodeIndex = 0xffffffff;
		listOfTextures[i].gosTextureHandle = 0xffffffff;
		listOfTextures[i].textureAlpha = false;
	}

	//---------------------------------------------------
	// Calling from top will load just first GEOMOBJECT!
	long parseResult = ParseASEFile(aseContents,fileName);

	TG_Shape::tglHeap->Free(aseContents);
	aseContents = NULL;

	return(parseResult);
}	

//-------------------------------------------------------------------------------
//Need this so that Multi-Shapes can let each shape know texture info.
void TG_TypeShape::CreateListOfTextures (TG_TexturePtr list, DWORD numTxms)
{
	numTextures = numTxms;
	if (numTextures)
	{
		listOfTextures = (TG_TinyTexturePtr)TG_Shape::tglHeap->Malloc(sizeof(TG_TinyTexture) * numTextures);
		gosASSERT(listOfTextures != NULL);
   
	   for (long i=0;i<numTextures;i++)
	   {
		   //-------------------------------------------------------------------------------
		   // Get and store texture Name.  Will need multiple for Multi-Sub if implemented
		   // NEVER GET ACTUAL TEXTURES HERE.  ALWAYS HAPPENS IN MULTI!!!!!!!!!!
		   // Just set 'em!
		   listOfTextures[i].mcTextureNodeIndex = list[i].mcTextureNodeIndex;
		   listOfTextures[i].gosTextureHandle = list[i].gosTextureHandle; 
		   listOfTextures[i].textureAlpha = list[i].textureAlpha;
	   }
 	}
	else
		listOfTextures = NULL;
}	

//-------------------------------------------------------------------------------
//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
//This function takes the gosTextureHandle passed in and assigns it to the
//textureNum entry of the listOfTextures;
long TG_TypeShape::SetTextureHandle (DWORD textureNum, DWORD gosTextureHandle)
{
	if (textureNum >= numTextures)
		return(-1);

	listOfTextures[textureNum].mcTextureNodeIndex = gosTextureHandle;
	listOfTextures[textureNum].gosTextureHandle = mcTextureManager->get_gosTextureHandle(gosTextureHandle);

	return(0);
}	

//-------------------------------------------------------------------------------
//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
//This function takes the gosTextureHandle passed in and assigns it to the
//textureNum entry of the listOfTextures;
long TG_TypeShape::SetTextureAlpha (DWORD textureNum, bool alphaFlag)
{
	if (textureNum >= numTextures)
		return(-1);

	listOfTextures[textureNum].textureAlpha = alphaFlag;

	return(0);
}

//-------------------------------------------------------------------------------
// TG_Shape
void *TG_Shape::operator new (size_t mySize)
{
	void *result = TG_Shape::tglHeap->Malloc(mySize);
	return result;
}
 
//-------------------------------------------------------------------------------
 void TG_Shape::operator delete (void *us)
{
	TG_Shape::tglHeap->Free(us);
}

//-------------------------------------------------------------------------------
//This function sets up the camera Matrices for this TG_Shape to transform
//itself with.  These matrices are static and only need to be set once per
//render pass if the camera does not change for that pass.
void TG_Shape::SetCameraMatrices (Stuff::LinearMatrix4D *camOrigin, Stuff::Matrix4D *camToClip)
{
	s_cameraOrigin = camOrigin;
	s_cameraToClip = camToClip;

	s_worldToCamera.Invert(*s_cameraOrigin);

	s_worldToClip.Multiply(s_worldToCamera, *s_cameraToClip);
}	

//-------------------------------------------------------------------------------
void TG_Shape::SetFog (DWORD fRGB, float fStart, float fFull)
{
	fogColor = fRGB;
	fogStart = fStart;
	fogFull = fFull;

	if (fogStart == fogFull)
	{
		fogStart = -100000.0f;
		fogFull = -99999.0f;
	}
}	

//-------------------------------------------------------------------------------
void TG_Shape::SetViewport (float mulX, float mulY, float addX, float addY)
{
	viewMulX = mulX;
	viewMulY = mulY;
	viewAddX = addX;
	viewAddY = addY;
}	

//-------------------------------------------------------------------------------
//This function sets the list of lights used by the TransformShape function
//to light the shape.
//Function returns 0 if lightList entries are all OK.  -1 otherwise.
//
long TG_Shape::SetLightList (TG_LightPtr *lightList, DWORD nLights)
{
	if (lightList)
	{
		s_listOfLights = lightList;
		s_numLights = nLights;
	}
	else
	{
		s_listOfLights = NULL;
		s_numLights = 0;
	}

	return 0;
}	

//-------------------------------------------------------------------------------
//This function sets the fog values for the shape.  Straight fog right now.
void TG_Shape::SetFogRGB (DWORD fRGB)
{
	fogRGB = fRGB;
}	

//-------------------------------------------------------------------------------
void TG_TypeShape::movePosRelativeCenterNode (void)
{
	for (long i=0;i<numTypeVertices;i++)
	{
		listOfTypeVertices[i].position -= nodeCenter;
	}
}


extern float yawRotation;
//-------------------------------------------------------------------------------
//This function does exactly what TranformShape does EXCEPT that the shapeToClip,
//Lighting and backface matrices have been calculated in the step above this one.
//This saves enormous processor cycles when matrices are the same and transforms
//Can just be run from the same matrices without recalcing them!
//Function returns -1 if all vertex screen positions are off screen.
//Function returns 0 if any one vertex screen position is off screen.
//Function returns 1 is all vertex screen positions are on screen.
// NOTE:  THIS IS NOT A RIGOROUS CLIP!!!!!!!!!
#define MAX_FOG_ELEVATION			100.0f
#define MAX_FOG_ELEVATION_INV		.01f
#define TGL_WINDOW_THRESHOLD		0.75f

long TG_Shape::MultiTransformShape (Stuff::Matrix4D *shapeToClip, Stuff::Point3D *backFacePoint, TG_ShapeRecPtr parentNode, bool isHudElement, BYTE alphaValue, bool isClamped)
{
	if (!numVertices)		//WE are the root Shape which may have no shape or a helper shape which defintely has no shape!
		return(1);
		
	//-------------------------------------------------
	// Transform entire list of vertices.
	// shapeOrigin is ShapeToWorld.
	// invShapeOrigin is WorldToShape.
	bool oneOff = false;
	bool oneOn = false;

	bool isNight = eye->getIsNight();
	float nightFactor = eye->getNightFactor();

	if (lightsOut)
	{
		isNight = false;
		nightFactor = 0.0f;
	}
	
	if (isSpotlight && !isNight)
	{
		listOfVertices = NULL;		//Mark this as untransformed!!
		return(1);
	}
		
	TG_TypeShapePtr theShape = (TG_TypeShapePtr)myType;
	
	//At this point, we know we are going to process this shape,
	// Get memory for its components from the pools!
	listOfVertices = vertexPool->getVerticesFromPool(numVertices);
	listOfColors = colorPool->getColorsFromPool(numVertices);
	
	listOfShadowTVertices = shadowPool->getShadowsFromPool(numVertices);

	listOfTriangles = trianglePool->getTrianglesFromPool(numTriangles);

	listOfVisibleFaces = facePool->getFacesFromPool(numTriangles);
	listOfVisibleShadows = facePool->getFacesFromPool(numTriangles);

	if (!listOfVertices ||
		!listOfColors ||
		!listOfShadowTVertices ||
		!listOfTriangles ||
		!listOfVisibleFaces ||
		!listOfVisibleShadows)
		return(1);

	lastTurnTransformed = turn;

	
	for (long j=0;j<numVertices;j++)
	{
		Stuff::Point3D pos = theShape->listOfTypeVertices[j].position;
		if (shapeScalar > 0.0f)
			pos *= shapeScalar;
			
		Stuff::Vector4D xformCoords;
		Stuff::Vector4D screen;

		xformCoords.Multiply(pos,*shapeToClip);

		if (eye->usePerspective)
		{
			//---------------------------------------
			// Perspective Transform
			float rhw = 1.0f;
			if (xformCoords.w != 0.0f)
				rhw = 1.0f / xformCoords.w;
			
			screen.x = (xformCoords.x * rhw) * viewMulX + viewAddX;
			screen.y = (xformCoords.y * rhw) * viewMulY + viewAddY;
			screen.z = (xformCoords.z * rhw);
			screen.w = fabs(rhw);
		}
		else
		{
			//---------------------------------------
			// Parallel Transform	
			screen.x = (1.0f - xformCoords.x) * viewMulX + viewAddX;
			screen.y = (1.0f - xformCoords.y) * viewMulY + viewAddY;
			screen.z = xformCoords.z;
			screen.w = 0.000001f;
		}

		if ((screen.x < 0) || (screen.y < 0) || (screen.x >= viewMulX) || (screen.y >= viewMulY))
			oneOff = TRUE;

		if ((screen.x >= 0) && (screen.y >= 0) && (screen.x < viewMulX) && (screen.y <= viewMulY))
			oneOn = TRUE;

		listOfVertices[j].x = screen.x;
		listOfVertices[j].y = screen.y;
		listOfVertices[j].z = screen.z;
		listOfVertices[j].rhw = screen.w;
		listOfVertices[j].frgb = fogRGB;
		memset(&listOfColors[j],0,sizeof(listOfColors[j]));

		//----------------------------------------------------
		// Lighting goes here.
		DWORD redFinal=0, greenFinal=0, blueFinal=0;
		DWORD redAmb = 0, greenAmb = 0, blueAmb = 0;
		DWORD redSpec=0, greenSpec=0, blueSpec=0;
		
		if (lighteningLevel > 0)
		{
			redSpec = blueSpec = greenSpec = lighteningLevel;
		}

		DWORD startVLight = theShape->listOfTypeVertices[j].aRGBLight;
   		if (startVLight == 0xffff00ff)			//Hot Pink -- Lit Windows -- ONLY at NIGHT
   		{
   			if (isNight)
   			{
   				redFinal = (theShape->hotPinkRGB>>16) & 0x000000ff;
   				greenFinal = (theShape->hotPinkRGB>>8) & 0x000000ff;
   				blueFinal = (theShape->hotPinkRGB) & 0x000000ff;
   			}
   			else if (nightFactor > Stuff::SMALL)
   			{
   				redFinal = (theShape->hotPinkRGB>>16) & 0x000000ff;
   				greenFinal = (theShape->hotPinkRGB>>8) & 0x000000ff;
   				blueFinal = (theShape->hotPinkRGB) & 0x000000ff;
   				
   				redFinal *= nightFactor;
   				blueFinal *= nightFactor;
   				greenFinal *= nightFactor;
   			}
			else		//Its not night, paint windows dark grey
			{
				redFinal = 0x2f;
				greenFinal = 0x2f;
				blueFinal = 0x2f;
			}
   		}
   		else if (startVLight == 0xffffff00)		//Hot Yellow -- Outside Building Lights -- ONLY at NIGHT
   		{
   			if (nightFactor >= 0.75f)
   			{
   				if (isNight)
   				{
   					redFinal = (theShape->hotYellowRGB>>16) & 0x000000ff;
   					greenFinal = (theShape->hotYellowRGB>>8) & 0x000000ff;
   					blueFinal = (theShape->hotYellowRGB) & 0x000000ff;
   				}
   				else
   				{
   					redFinal = (theShape->hotYellowRGB>>16) & 0x000000ff;
   					greenFinal = (theShape->hotYellowRGB>>8) & 0x000000ff;
   					blueFinal = (theShape->hotYellowRGB) & 0x000000ff;
   					
   					redFinal *= nightFactor;
   					blueFinal *= nightFactor;
   					greenFinal *= nightFactor;
   				}
   			}
   		}
   		else if (startVLight == 0xff00ff00)		//Hot Green -- Building Base Lights -- ONLY at NIGHT
   		{
   			if (isNight)
   			{
   				redFinal = (theShape->hotGreenRGB>>16) & 0x000000ff;
   				greenFinal = (theShape->hotGreenRGB>>8) & 0x000000ff;
   				blueFinal = (theShape->hotGreenRGB) & 0x000000ff;
   			}
   			else if (nightFactor > Stuff::SMALL)
   			{
   				redFinal = (theShape->hotGreenRGB>>16) & 0x000000ff;
   				greenFinal = (theShape->hotGreenRGB>>8) & 0x000000ff;
   				blueFinal = (theShape->hotGreenRGB) & 0x000000ff;
   				
   				redFinal *= nightFactor;
   				blueFinal *= nightFactor;
   				greenFinal *= nightFactor;
   			}
   		}
   		else if (startVLight == 0xffff0000)		//Hot Red -- Blink this light.
   		{
   		
   		}
   		else if (startVLight == 0xff0000ff)		//Hot Blue -- Blink this light.
   		{
   		
   		}
   		else if (startVLight & 0x00ffffff)		//Some other kind of light, just add it in.
   		{
			if (!lightsOut)
			{
				redFinal = (startVLight>>16) & 0x000000ff;
				greenFinal = (startVLight>>8) & 0x000000ff;
				blueFinal = (startVLight) & 0x000000ff;
			}
   		}
		else if (isHudElement)
		{
			redFinal = blueFinal = greenFinal = 0xff;	//Just max out its light.
		}

		if (BaseVertexColor)
		{
			redFinal += ((BaseVertexColor>>16) & 0x000000ff);
			if (redFinal > 0xff)
				redFinal = 0xff;
				
			greenFinal += ((BaseVertexColor>>8) & 0x000000ff);
			if (greenFinal > 0xff)
				greenFinal = 0xff;
				
			blueFinal += (BaseVertexColor & 0x000000ff);
			if (blueFinal > 0xff)
				blueFinal = 0xff;
		}
			
		if (useVertexLighting && (Environment.Renderer != 3))
		{
			if (!isSpotlight && !isWindow)
			{
				for (long i=0;i<s_numLights;i++)
				{
					if ((s_listOfLights[i] != NULL) && (s_listOfLights[i]->active))
					{
						DWORD startLight = s_listOfLights[i]->GetaRGB();
						switch (s_listOfLights[i]->lightType)
						{
							case TG_LIGHT_AMBIENT:
							{
								redAmb = ((startLight>>16) & 0x000000ff);
								greenAmb = ((startLight>>8) & 0x000000ff);
								blueAmb = ((startLight) & 0x000000ff);
							}
							break;
	
							case TG_LIGHT_INFINITE:
							{
								float cosine = s_lightDir[i].x * theShape->listOfTypeVertices[j].normal.x;
								cosine += s_lightDir[i].y * theShape->listOfTypeVertices[j].normal.y;
								cosine += s_lightDir[i].z * theShape->listOfTypeVertices[j].normal.z;

								if (cosine < 0.0f)
								{
									float cos = fabs(cosine);
									float red = float((startLight>>16) & 0x000000ff) * cos;
									float green = float((startLight>>8) & 0x000000ff) * cos;
									float blue = float((startLight) & 0x000000ff) * cos;

									redFinal += float2long(red);
									greenFinal += float2long(green);
									blueFinal += float2long(blue);

									/*
									//-----------------------
									// Run Specular next.
									// Mirror the Light vector.
									// Then DOT with Camera vector.
									// Mul Result by itself 4 times.
									// Mul that result by shinyness.
									// Multiply THAT through color and apply!
									float mirrorScalar = cosine * 2.0f;
									Stuff::Vector3D MirrorVector(listOfVertices[i].normal);
									MirrorVector *= mirrorScalar;
									MirrorVector.Subtract(MirrorVector,s_lightDir[i]);
									MirrorVector.Normalize(MirrorVector);

									Stuff::Vector3D SpecPoint;
									if (turn > 3)
										SpecPoint.Normalize(*backFacePoint);
									float specular = SpecPoint.x * MirrorVector.x;
									specular += SpecPoint.y * MirrorVector.y;
									specular += SpecPoint.z * MirrorVector.z;

									if (specular < 0.0f)
									{
										//specular = fabs(specular);
										specular *= specular;
										specular *= specular;
										specular *= specular;
										specular *= specular;

										specular *= 1.0f;	
										// For now, a shinyness of 1.0f.  Perfect reflection!
										// Multiplies will make specular positive!
										float red = float((startLight>>16) & 0x000000ff) * specular;
										float green = float((startLight>>8) & 0x000000ff) * specular;
										float blue = float((startLight) & 0x000000ff) * specular;

										redSpec += (DWORD)red;
										greenSpec += (DWORD)green;
										blueSpec += (DWORD)blue;
									}
									*/
								}
							}
							break;
	
							case TG_LIGHT_INFINITEWITHFALLOFF:
							{
								Stuff::Point3D vertexToLight;
								vertexToLight = s_lightToShape[i];
								vertexToLight -= theShape->listOfTypeVertices[j].position;
	
								float length = vertexToLight.GetApproximateLength();
	
								float falloff = 1.0f;
	
								float red,green,blue;
	
								if (s_listOfLights[i]->GetFalloff(length, falloff))
								{
									float cosine = -(s_lightDir[i] * (theShape->listOfTypeVertices[j].normal));
	
									red = float((startLight>>16) & 0x000000ff) * falloff;
									green = float((startLight>>8) & 0x000000ff) * falloff;
									blue = float((startLight) & 0x000000ff) * falloff;
	
									red *= cosine;
									green *= cosine;
									blue *= cosine;
	
									redFinal += (DWORD)red;
									greenFinal += (DWORD)green;
									blueFinal += (DWORD)blue;
								}
							}
							break;
	
							case TG_LIGHT_POINT:
							{
								Stuff::Point3D vertexToLight;
								vertexToLight = s_lightDir[i];
								float length = vertexToLight.GetApproximateLength();
	
								if (length > Stuff::SMALL)
								{	
									vertexToLight.Normalize(vertexToLight);
		
									float falloff = 1.0f;
		
									if (s_listOfLights[i]->GetFalloff(length, falloff))
									{
										float cosine = vertexToLight * (theShape->listOfTypeVertices[j].normal);
		
										float red,green,blue;
		
										if (cosine < 0.0f)
										{
											cosine = fabs(cosine);
											red = float((startLight>>16) & 0x000000ff) * falloff;
											green = float((startLight>>8) & 0x000000ff) * falloff;
											blue = float((startLight) & 0x000000ff) * falloff;
		
											red *= cosine;
											green *= cosine;
											blue *= cosine;
		
											redSpec += (DWORD)red;
											greenSpec += (DWORD)green;
											blueSpec += (DWORD)blue;
										}
									}
								}
								else
								{
									//Object is in center of light.  BLOW IT OUT!
									float cosine = 1.0f;
	
									float red,green,blue;
	
									if (cosine < 0.0f)
									{
										cosine = fabs(cosine);
										red = float((startLight>>16) & 0x000000ff);
										green = float((startLight>>8) & 0x000000ff);
										blue = float((startLight) & 0x000000ff);
	
										red *= cosine;
										green *= cosine;
										blue *= cosine;
	
										redSpec += (DWORD)red;
										greenSpec += (DWORD)green;
										blueSpec += (DWORD)blue;
									}
								}
							}
							break;
								  
							case TG_LIGHT_TERRAIN:
							{
								if (useShadows)
								{
									Stuff::Point3D vertexToLight;
									Stuff::Vector3D pos = theShape->listOfTypeVertices[j].position;
									RotateLight(pos,yawRotation);
									vertexToLight.Add(s_lightDir[i],pos);
									float length = vertexToLight.GetApproximateLength();
		
									if (length > Stuff::SMALL)
									{	
										float falloff = 1.0f;
										if (s_listOfLights[i]->GetFalloff(length, falloff))
										{
											float red,green,blue;
			
											red = float((startLight>>16) & 0x000000ff) * falloff;
											green = float((startLight>>8) & 0x000000ff) * falloff;
											blue = float((startLight) & 0x000000ff) * falloff;
			
											listOfColors[j].redSpec = (DWORD)red;
											listOfColors[j].greenSpec = (DWORD)green;
											listOfColors[j].blueSpec = (DWORD)blue;
										}
									}
									else
									{
										//Object is in center of light.  NOTHING HAPPENS WITH THIS KIND!!!
										//Light is already burned in!
									}
								}
							}
							break;
							
 							case TG_LIGHT_SPOT:
							{
								Stuff::Point3D vertexToLight;
								vertexToLight = s_lightDir[i];
								
								//-------------------------------------------------
								// Defines the actual spot of light on the ground
								float length = vertexToLight.GetApproximateLength();
	
								//-------------------------------------------------
								// Defines the REAL direction of the spot light.
								vertexToLight = s_spotDir[i];
								if (vertexToLight.GetApproximateLength() > Stuff::SMALL)
									vertexToLight.Normalize(vertexToLight);
								else
									length = 99999999999999.0f;
	
								float falloff = 1.0f;
	
								if (s_listOfLights[i]->GetFalloff(length, falloff))
								{
									float cosine = vertexToLight * (theShape->listOfTypeVertices[j].normal);
	
									float red,green,blue;
	
									if (cosine < 0.0f)
									{
										cosine = fabs(cosine);
										red = float((startLight>>16) & 0x000000ff) * falloff;
										green = float((startLight>>8) & 0x000000ff) * falloff;
										blue = float((startLight) & 0x000000ff) * falloff;
	
										red *= cosine;
										green *= cosine;
										blue *= cosine;
	
										redSpec += (DWORD)red;
										greenSpec += (DWORD)green;
										blueSpec += (DWORD)blue;
									}
								}
							}
							break;
						}
					}
				}
				
				redFinal += redAmb;
				blueFinal += blueAmb;
				greenFinal += greenAmb;
			}
			
			if (redFinal > 255)
				redFinal = 255;

			if (greenFinal > 255)
				greenFinal = 255;

			if (blueFinal > 255)
				blueFinal = 255;
					
			listOfVertices[j].argb = (0xff << 24) + (redFinal << 16) + (greenFinal << 8) + (blueFinal);

			// NEW
			// Add in the pre-calced or calced this frame terrain lights.
			// Terrain lights will only be made ACTIVE when day/night changes.
			// One frame of active is enough to store new values!
			redSpec += listOfColors[j].redSpec;
			greenSpec += listOfColors[j].greenSpec;
			blueSpec += listOfColors[j].blueSpec;
			
			if (redSpec > 255)
				redSpec = 255;

			if (greenSpec > 255)
				greenSpec = 255;

			if (blueSpec > 255)
				blueSpec = 255;
		}
		else
		{
			listOfVertices[j].argb = (0xff << 24) + (redAmb << 16) + (greenAmb << 8) + (blueAmb);
		}

		DWORD fogValue = (listOfVertices[j].frgb >> 24) & 0x000000ff;
		if (Environment.Renderer != 3)
		{
			if (useFog)
			{
				float relElevation = theShape->listOfTypeVertices[j].position.y;
				relElevation += theShape->relativeNodeCenter.y;

				TG_ShapeRecPtr curNode = parentNode;
				while (curNode && curNode->node)
				{
					relElevation += curNode->node->myType->GetRelativeNodeCenter().y;
					curNode = curNode->parentNode;
				}

				if (relElevation < MAX_FOG_ELEVATION)
				{
					float fogFactor = MAX_FOG_ELEVATION - relElevation;
					if (fogFactor >= 0.0)
					{
						fogFactor /= (MAX_FOG_ELEVATION);
						if (fogFactor <= 1.0)
						{
							fogFactor *= fogFactor;
							fogFactor = 1.0 - fogFactor;
						}
						else
						{
							fogFactor = 0.0;
						}

						fogValue += float2long((0x000000ff - fogValue) * fogFactor);
					}
				}
				else
				{
					fogValue = 0x000000ff;
				}

				listOfVertices[j].frgb = (fogValue << 24) + (redSpec << 16) + (greenSpec << 8) + (blueSpec);
			}
			else
			{
				listOfVertices[j].frgb = (0xff << 24) + (redSpec << 16) + (greenSpec << 8) + (blueSpec);
			}
		}
		else
		{
			listOfVertices[j].frgb = (0xff << 24) + (redSpec << 16) + (greenSpec << 8) + (blueSpec);
		}

		//-------------------
		// Distance FOG now.
		if (useFog && Camera::HazeFactor != 0.0f)
		{
			float fogFactor = 1.0 - Camera::HazeFactor;
			DWORD distFog = float2long(fogFactor * 255.0f);
			
			if (distFog < fogValue)
				fogValue = distFog;
				
			listOfVertices[j].frgb = (fogValue << 24) + (redSpec << 16) + (greenSpec << 8) + (blueSpec);
		}
		
		if (aRGBHighlight)
		{
			DWORD argb = listOfVertices[j].argb;

			DWORD fFinal = (argb>>24) & 0x000000ff;
			DWORD rFinal = (argb>>16) & 0x000000ff;
			DWORD gFinal = (argb>>8) & 0x000000ff;
			DWORD bFinal = (argb) & 0x000000ff;

			rFinal += (aRGBHighlight>>16) & 0x000000ff;
			if (rFinal > 255)
				rFinal = 255;

			gFinal += (aRGBHighlight>>8) & 0x000000ff;
			if (gFinal > 255)
				gFinal = 255;

			bFinal += (aRGBHighlight) & 0x000000ff;;
			if (bFinal > 255)
				bFinal = 255;

			argb = (fFinal << 24) + (rFinal << 16) + (gFinal << 8) + (bFinal);
			listOfVertices[j].argb = argb;
		}
	}

	numVisibleFaces = 0;			//Reset Visible Faces

	TG_TypeTrianglePtr tri = &(theShape->listOfTypeTriangles[0]);
	for (int j=0;j<numTriangles;j++,tri++)
	{
		//---------------------------------------
		// Mark backfacing those that are.
		float cosine = 0.0;
		cosine = (*backFacePoint * (tri->faceNormal));
		if ((cosine > 0.0f) || isSpotlight)
		{
			listOfVisibleFaces[numVisibleFaces] = j;
			numVisibleFaces++;

			DWORD redFinal = 0, greenFinal = 0, blueFinal = 0;
			DWORD redSpec=0, greenSpec=0, blueSpec=0;
			DWORD redAmb=0, greenAmb=0, blueAmb=0;
	
			if (useFaceLighting)
			{
				//--------------------------------------------
				// Flat Shade any face which are flat shaded.
				for (long i=0;i<s_numLights;i++)
				{
					if (s_listOfLights[i] != NULL)
					{
						DWORD startVLight = s_listOfLights[i]->GetaRGB();
						switch (s_listOfLights[i]->lightType)
						{
							case TG_LIGHT_INFINITE:
							{
								if (s_lightDir[i].GetLength() > Stuff::SMALL)
									s_lightDir[i].Normalize(s_lightDir[i]);
		
								float cosine = -(s_lightDir[i] * (theShape->listOfTypeTriangles[j].faceNormal));
		
								if (cosine > 0.0f)
								{
									float red = float((startVLight>>16) & 0x000000ff) * cosine;
									float green = float((startVLight>>8) & 0x000000ff) * cosine;
									float blue = float((startVLight) & 0x000000ff) * cosine;
		
									redFinal += float2long(red);
									greenFinal += float2long(green);
									blueFinal += float2long(blue);
								}
							}
							break;
		
							case TG_LIGHT_INFINITEWITHFALLOFF:
							{
								if (s_lightDir[i].GetLength() > Stuff::SMALL)
									s_lightDir[i].Normalize(s_lightDir[i]);
		
								float cosine = -(s_lightDir[i] * (theShape->listOfTypeTriangles[j].faceNormal));
		
								Stuff::Point3D vertexToLight;
								vertexToLight = s_lightToShape[i];
								vertexToLight -= theShape->listOfTypeVertices[theShape->listOfTypeTriangles[j].Vertices[0]].position;
		
								float length = vertexToLight.GetApproximateLength();
		
								float falloff = 1.0f;
		
								float red,green,blue;
		
								if ((cosine > 0.0f) && s_listOfLights[i]->GetFalloff(length, falloff))
								{
									red = float((startVLight>>16) & 0x000000ff) * falloff;
									green = float((startVLight>>8) & 0x000000ff) * falloff;
									blue = float((startVLight) & 0x000000ff) * falloff;
		
									red *= cosine;
									green *= cosine;
									blue *= cosine;
		
									redFinal += (DWORD)red;
									greenFinal += (DWORD)green;
									blueFinal += (DWORD)blue;
								}
							}
							break;
		
							case TG_LIGHT_POINT:
							{
								Stuff::Point3D vertexToLight;
								vertexToLight = s_lightToShape[i];
								vertexToLight -= theShape->listOfTypeVertices[theShape->listOfTypeTriangles[j].Vertices[0]].position;
		
								float length = vertexToLight.GetApproximateLength();
		
								vertexToLight.Normalize(vertexToLight);
								float falloff = 1.0f;
		
								if (s_listOfLights[i]->GetFalloff(length, falloff))
								{
									float cosine = -(vertexToLight * (theShape->listOfTypeTriangles[j].faceNormal));
		
									float red,green,blue;
		
									if (cosine > 0.0f)
									{
										red = float((startVLight>>16) & 0x000000ff) * falloff;
										green = float((startVLight>>8) & 0x000000ff) * falloff;
										blue = float((startVLight) & 0x000000ff) * falloff;
		
										red *= cosine;
										green *= cosine;
										blue *= cosine;
		
										redSpec += (DWORD)red;
										greenSpec += (DWORD)green;
										blueSpec += (DWORD)blue;
									}
								}
							}
							break;
		
							case TG_LIGHT_AMBIENT:
							{
								redAmb = ((startVLight>>16) & 0x000000ff);
								greenAmb = ((startVLight>>8) & 0x000000ff);
								blueAmb = ((startVLight) & 0x000000ff);
							}
							break;
						}
					}
				}
		
				redFinal += redAmb;
				blueFinal += blueAmb;
				greenFinal += greenAmb;
				
				if (redFinal > 255)
					redFinal = 255;
	
				if (greenFinal > 255)
					greenFinal = 255;
	
				if (blueFinal > 255)
					blueFinal = 255;
					
				if (redSpec > 255)
					redSpec = 255;
	
				if (greenSpec > 255)
					greenSpec = 255;
	
				if (blueSpec > 255)
					blueSpec = 255;
			}
	
			for (long i=0;i<3;i++ )
			{
				DWORD argb = listOfVertices[theShape->listOfTypeTriangles[j].Vertices[i]].argb;
				DWORD rFinal = (argb>>16) & 0x000000ff;
				DWORD gFinal = (argb>>8) & 0x000000ff; 
				DWORD bFinal = (argb) & 0x000000ff;    
	
				rFinal += redFinal;
				if (rFinal > 255)
					rFinal = 255;
	
				gFinal += greenFinal;
				if (gFinal > 255)
					gFinal = 255;
	
				bFinal += blueFinal;
				if (bFinal > 255)
					bFinal = 255;
	
				listOfTriangles[j].aRGBLight[i] = (alphaValue << 24) + (rFinal << 16) + (gFinal << 8) + (bFinal);
	
				DWORD frgb = listOfVertices[theShape->listOfTypeTriangles[j].Vertices[i]].frgb;
	
				DWORD fFinal = (frgb>>24) & 0x000000ff;
				rFinal = (frgb>>16) & 0x000000ff;
				gFinal = (frgb>>8) & 0x000000ff;
				bFinal = (frgb) & 0x000000ff;
	
				rFinal += (redSpec>>16) & 0x000000ff;
				if (rFinal > 255)
					rFinal = 255;
	
				gFinal += (greenSpec>>8) & 0x000000ff;
				if (gFinal > 255)
					gFinal = 255;
	
				bFinal += (blueSpec) & 0x000000ff;
				if (bFinal > 255)
					bFinal = 255;
	
				listOfTriangles[j].fRGBLight[i] = (fFinal << 24) + (rFinal << 16) + (gFinal << 8) + (bFinal);
			}
			
			DWORD addFlags = 0;
			if (isHudElement)
			{
				addFlags |= MC2_ISCOMPASS;
			}
			
			if (isClamped)
			{
				addFlags |= MC2_ISTERRAIN;
			}

            if(theShape->alphaTestOn)
			{
				addFlags |= MC2_ALPHATEST;
			}
			
			if (isSpotlight)
			{
				mcTextureManager->addTriangle(0xffffffff,MC2_ISSPOTLGT);
			}
			else if (isWindow)
			{
				mcTextureManager->addTriangle(0xffffffff,MC2_DRAWALPHA);
			}
			else
			{
				//--------------------------------------------------
				// Any sorting we need to do occurs in here as well!
				// So, find out what texture this face is using and mark that texture as needing 3 more vertices.
				if (theShape->listOfTextures[theShape->listOfTypeTriangles[j].localTextureHandle].textureAlpha || (alphaValue != 0xff))
				{
					mcTextureManager->addTriangle(theShape->listOfTextures[theShape->listOfTypeTriangles[j].localTextureHandle].mcTextureNodeIndex,MC2_DRAWALPHA | addFlags);
				}
				else
				{
					mcTextureManager->addTriangle(theShape->listOfTextures[theShape->listOfTypeTriangles[j].localTextureHandle].mcTextureNodeIndex,MC2_DRAWSOLID | addFlags);
				}

			}
		}
	}

	// FIXME: this (listOfTypeTriangles[0]) is not correct if model has more than 1 texture! 
	if (!isSpotlight && !isWindow && !theShape->listOfTextures[theShape->listOfTypeTriangles[0].localTextureHandle].textureAlpha && (alphaValue == 0xff))
	{
		DWORD addFlags = 0;
		if (isHudElement)
		{
			addFlags |= MC2_ISCOMPASS;
		}

		if (isClamped)
		{
			addFlags |= MC2_ISTERRAIN;
		}

		if(theShape->alphaTestOn)
		{
			addFlags |= MC2_ALPHATEST;
		}

		if (theShape->ib_ && theShape->vb_) {

            size_t numLights = MAX_HW_LIGHTS_IN_WORLD;
            GatherLightsParameters(&lightData_);
            gosASSERT(numLights > 0); // ensure at least one light was filled

			cur_viewport[0] = viewMulX;
			cur_viewport[1] = viewMulY;
			cur_viewport[2] = viewAddX;
			cur_viewport[3] = viewAddY;
			cur_shape2clip = *shapeToClip;

			// FIXME: this (listOfTypeTriangles[0]) is not correct if model has more than 1 texture! 
			mcTextureManager->addRenderShape(theShape->listOfTextures[theShape->listOfTypeTriangles[0].localTextureHandle].mcTextureNodeIndex, MC2_DRAWSOLID | addFlags);
		}
	}

	gosASSERT(oneOff || oneOn);

	if (oneOff && !oneOn)
	{
		return(-1);
	}
	else if (oneOff && oneOn)
	{
		return(0);
	}		
	else if (!oneOff && oneOn)
	{
		return(1);
	}

	return(-1);
}

#define TERRAIN_DEPTH_FUDGE		(0.000f)
//-------------------------------------------------------------------------------
//This function takes the current listOfVisibleFaces and draws them using
//gos_DrawTriangle.
void TG_Shape::Render (float forceZ, bool isHudElement, BYTE alphaValue, bool isClamped, const Stuff::Matrix4D* shapeToClip, const Stuff::Matrix4D* shapeToWorld)
{
	if (!renderTGLShapes)
		return;
		
	if (!numVertices)		//WE are the root Shape which may have no shape or a helper shape which defintely has no shape!
		return;
		
	if (!listOfVertices ||
		!listOfColors ||
		!listOfShadowTVertices ||
		!listOfTriangles ||
		!listOfVisibleFaces ||
		!listOfVisibleShadows ||
		(/*(lastTurnTransformed != (turn-1)) &&*/ (lastTurnTransformed != turn)))
		return;

	if (fogColor != 0xffffffff)
	{
		//gos_SetRenderState( gos_State_Fog, (int)&(fogColor));
		gos_SetRenderState( gos_State_Fog, fogColor); // sebi
	}
	else
	{
		gos_SetRenderState( gos_State_Fog, 0);
	}

	TG_TypeShapePtr theShape = (TG_TypeShapePtr)myType;

	gos_SetRenderState( gos_State_AlphaTest, theShape->alphaTestOn);
	gos_SetRenderState( gos_State_Filter, theShape->filterOn ? gos_FilterBiLinear : gos_FilterNone);

	DWORD lastTextureUsed = 0xffffffff;
	
	bool isNight = eye->getIsNight();
	float nightFactor = eye->getNightFactor();
	
	if (lightsOut)
	{
		isNight = false;
		nightFactor = 0.0f;
	}

	if (isSpotlight && !isNight)
		return;
 	
 	for (long j=0;j<numVisibleFaces;j++)
	{
		if (listOfVisibleFaces[j] < numTriangles)
		{
			TG_TypeTriangle triType = theShape->listOfTypeTriangles[listOfVisibleFaces[j]];
			TG_Triangle tri = listOfTriangles[listOfVisibleFaces[j]];

			gos_VERTEX gVertex[3];

			gVertex[0] = listOfVertices[triType.Vertices[0]];
			gVertex[1] = listOfVertices[triType.Vertices[1]];
			gVertex[2] = listOfVertices[triType.Vertices[2]];

			gVertex[0].u		= triType.uvdata.u0;
			gVertex[0].v		= triType.uvdata.v0;

			gVertex[0].argb		= tri.aRGBLight[0];

			gVertex[1].u		= triType.uvdata.u1;
			gVertex[1].v		= triType.uvdata.v1;

			gVertex[1].argb		= tri.aRGBLight[1];

			gVertex[2].u		= triType.uvdata.u2;
			gVertex[2].v		= triType.uvdata.v2;

			gVertex[2].argb		= tri.aRGBLight[2];

			DWORD addFlags = 0;
			if (isHudElement)		//We are a HUD Element like the compass.  Mark us as such.
			{
				addFlags |= MC2_ISCOMPASS;
			}
			
			if (isClamped)
			{
				addFlags |= MC2_ISTERRAIN;
			}

            if(theShape->alphaTestOn)
			{
				addFlags |= MC2_ALPHATEST;
			}

			if (drawOldWay)
			{
			
				if (theShape->listOfTextures && 
					(theShape->listOfTextures[triType.localTextureHandle].gosTextureHandle != 0xffffffff))
				{
					if (theShape->listOfTextures[triType.localTextureHandle].gosTextureHandle != lastTextureUsed)
					{
						gos_SetRenderState( gos_State_Texture, theShape->listOfTextures[triType.localTextureHandle].gosTextureHandle );
						if (theShape->listOfTextures[triType.localTextureHandle].textureAlpha)
						{
							gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
							gos_SetRenderState( gos_State_AlphaTest, true);
						}
						
						lastTextureUsed = theShape->listOfTextures[triType.localTextureHandle].gosTextureHandle;
					}
				}
				else
					gos_SetRenderState( gos_State_Texture, 0 );
	
				if ((gVertex[0].z >= 0.0f) &&
					(gVertex[0].z < 1.0f) &&
					(gVertex[1].z >= 0.0f) &&  
					(gVertex[1].z < 1.0f) && 
					(gVertex[2].z >= 0.0f) &&  
					(gVertex[2].z < 1.0f))
				{
					//-----------------------------------------------------------------------------
					// Reject Any triangle which has vertices off screeen in software for now.
					// Do real cliping in geometry layer for software and hardware that needs it!
					gos_DrawTriangles(gVertex, 3);
				}
			}
			else
			{
				if ((gVertex[0].z >= 0.0f) &&
					(gVertex[0].z < 1.0f) &&
					(gVertex[1].z >= 0.0f) &&  
					(gVertex[1].z < 1.0f) && 
					(gVertex[2].z >= 0.0f) &&  
					(gVertex[2].z < 1.0f))
				{
					if ((forceZ >= 0.0f) && (forceZ < 1.0f))
					{
						gVertex[0].z	= forceZ;
						gVertex[1].z	= forceZ;
						gVertex[2].z	= forceZ;
					}
 				
					//Can make this a flag to optimize!
					if (isSpotlight)
					{
						mcTextureManager->addVertices(0xffffffff,gVertex,MC2_ISSPOTLGT);
					}
					else if (isWindow)
					{
						mcTextureManager->addVertices(0xffffffff,gVertex,MC2_DRAWALPHA);
					}
					else
					{
						//--------------------------------------------------
						// Any sorting we need to do occurs in here as well!
						// So, find out what texture this face is using and mark that texture as needing 3 more vertices.
						if (theShape->listOfTextures[triType.localTextureHandle].textureAlpha || (alphaValue != 0xff))
						{
							mcTextureManager->addVertices(theShape->listOfTextures[triType.localTextureHandle].mcTextureNodeIndex,gVertex,MC2_DRAWALPHA | addFlags);
						}
						else
						{
							mcTextureManager->addVertices(theShape->listOfTextures[triType.localTextureHandle].mcTextureNodeIndex,gVertex,MC2_DRAWSOLID | addFlags);

							
						}
					}
				}
			}
		}
	}


	// FIXME: this (listOfTypeTriangles[0]) is not correct if model has more than 1 texture! 
	if (!isSpotlight && !isWindow && !theShape->listOfTextures[theShape->listOfTypeTriangles[0].localTextureHandle].textureAlpha && (alphaValue == 0xff))
	{
		DWORD addFlags = 0;
		if (isHudElement)
		{
			addFlags |= MC2_ISCOMPASS;
		}

		if (isClamped)
		{
			addFlags |= MC2_ISTERRAIN;
		}

        if(theShape->alphaTestOn)
		{
			addFlags |= MC2_ALPHATEST;
		}

		if (theShape->ib_ && theShape->vb_) {

			// FIXME: this is not correct if model has more than 1 texture! 
			// TODO: split on per texture batches basis
			TG_TypeTriangle triType = theShape->listOfTypeTriangles[0];

			Stuff::Matrix4D mvp;
			if ((forceZ >= 0.0f) && (forceZ < 1.0f))
			{
				// matrix that wil make (x,y,w * forceZ, w) from (x,y,z,w), so that later after division by w, it will become just forceZ
				Stuff::Matrix4D forceZMatrix = Stuff::Matrix4D::Identity;
				forceZMatrix(2, 2) = 0.0f;
				forceZMatrix(3, 2) = forceZ;
				mvp.Multiply(cur_shape2clip, forceZMatrix);
			}
			else {
				mvp = cur_shape2clip;
			}

			TG_RenderShape rs;
			rs.ib_ = theShape->ib_;
			rs.vb_ = theShape->vb_;
			rs.vdecl_ = theShape->vdecl_;
			rs.mvp_ = mvp;
			rs.mw_ = *shapeToWorld;
			memcpy(rs.viewport_, cur_viewport, 4 * sizeof(float));
            rs.light_data_buffer_index_ = mcTextureManager->addLightDataStructure(&lightData_);

			mcTextureManager->addRenderShape(
				theShape->listOfTextures[triType.localTextureHandle].mcTextureNodeIndex,
				&rs,
				MC2_DRAWSOLID | addFlags);
		}
	}

	gos_SetRenderState( gos_State_Fog, 0);
}	

extern bool InEditor;
//------------------------------------------------------------------------------------------------------------
// This function checks each visible triangle against the mouse position and returns if we are over ANY poly!
bool TG_Shape::PerPolySelect (float mouseX, float mouseY)
{
	if (!InEditor)
	{
		if (!listOfVertices ||
			!listOfColors ||
			!listOfShadowTVertices ||
			!listOfTriangles ||
			!listOfVisibleFaces ||
			!listOfVisibleShadows ||
			((lastTurnTransformed != (turn-1)) /*&& (lastTurnTransformed != turn)*/))
			return false;
	}
	else
	{
		if (!listOfVertices ||
			!listOfColors ||
			!listOfShadowTVertices ||
			!listOfTriangles ||
			!listOfVisibleFaces ||
			!listOfVisibleShadows ||
			((lastTurnTransformed != (turn-1)) && (lastTurnTransformed != turn)))
			return false;
	}

	TG_TypeShapePtr theShape = (TG_TypeShapePtr)myType;
 	for (long j=0;j<numVisibleFaces;j++)
	{
		if (listOfVisibleFaces[j] < numTriangles)
		{
			TG_TypeTriangle triType = theShape->listOfTypeTriangles[listOfVisibleFaces[j]];
			Stuff::Point3D v0,v1,v2;
			v0.x = listOfVertices[triType.Vertices[0]].x;
			v0.y = listOfVertices[triType.Vertices[0]].y;
			
			v1.x = listOfVertices[triType.Vertices[1]].x;
			v1.y = listOfVertices[triType.Vertices[1]].y;
			
			v2.x = listOfVertices[triType.Vertices[2]].x;
			v2.y = listOfVertices[triType.Vertices[2]].y;
			v0.z = v1.z = v2.z = 0.0f;
			
			//Using the above vertex Data, determine if the mouse is over this poly!
			//
			//Theorem:
			// Given the sides of the triangle defined by the lines v0v1, v1v2 and v2v0
			// in the form Ax + By + C = 0
			//
			// the point mousex, mousey lies inside the triangle if and only if
			//
			//  	A0 * mouseX + B0 * mouseY + C0 = D0
			//		A1 * mouseX * B1 * mouseY + c1 = D1
			//		A2 * mouseX + B2 * mouseY + c2 = D2
			//
			// Dx is the same sign for each line as the correct sign for clockwise or counterclockwise vertices!
			//
			Stuff::Vector3D line1;
			Stuff::Vector3D line2;
			line1.Subtract(v0,v1);
			line2.Subtract(v1,v2);
			
			float order = line2.x * line1.y - line1.x * line2.y;
			order = sign2(order);
			
			float A0 = -(v0.y - v1.y);
			float B0 = (v0.x - v1.x);
			float C0 = -B0*(v0.y) - A0*(v0.x);
			float D0 = A0 * mouseX + B0 * mouseY + C0;
			
			float A1 = -(v1.y - v2.y);
			float B1 = (v1.x - v2.x);
			float C1 = -B1*(v1.y) - A1*(v1.x);
			float D1 = A1 * mouseX + B1 * mouseY + C1;
 			
 			float A2 = -(v2.y - v0.y);
			float B2 = (v2.x - v0.x);
			float C2 = -B2*(v2.y) - A2*(v2.x);
			float D2 = A2 * mouseX + B2 * mouseY + C2;
			
			if ((sign2(D0) == order) && (sign2(D0) == sign2(D1)) && (sign2(D0) == sign2(D2)))
				return true;
    	}
	}
	
	return false;
}

#define SHADOW_HARDWARE_BLACK 			0xff000000
#define SHADOW_SOFTWARE_BLACK			0xff000000
#define FLOAT_SHADOW_HARDWARE_BLACK		128.0f
//-------------------------------------------------------------------------------
//This function creates the list of shadows and transforms them in preparation to drawing.
//
void TG_Shape::MultiTransformShadows (Stuff::Point3D *pos, Stuff::LinearMatrix4D *s2w, float rotation)
{
	if (!numVertices)		//WE are the root Shape which may have no shape or a helper shape which defintely has no shape!
		return;
		
	//Can make this a flag to optimize!
	if (noShadow)
		return;
		
	if (!listOfVertices ||
		!listOfColors ||
		!listOfShadowTVertices ||
		!listOfTriangles ||
		!listOfVisibleFaces ||
		!listOfVisibleShadows)
		return;

	Stuff::Matrix4D identityMatrix;
	identityMatrix.BuildIdentity();
	
	TG_TypeShapePtr theShape = (TG_TypeShapePtr)myType;
	//-----------------------------------------------------------------
	// Run Through listOfVertices and create the listOfShadowVertices
	// for each light source.  Only need to calc infinite rarely if at all.
	// May need some way to delineate this.  Above here, probably!
	//
	// As we calc shadow vertices, transform and store results.
	// 
	// listOfTriangles will simply use these vertices to draw shadows.
	//
	// Again, note that after first iteration of infinite light we only need to transform infinite shadows!
	Stuff::Matrix4D shapeToLocal;
	Stuff::Matrix4D shapeToClip;
	
	long shadowNum = 0;

	memset(shadowsVisible,0,sizeof(bool) * MAX_SHADOWS);

		//-------------------------------------------------------
		// Now, for each light IN RANGE
		// Use formula form Blinn-Trip Down the Graphics Pipeline-Chapter 6
		for (long i=0;i<s_numLights;i++)
		{
			if ((s_listOfLights[i] != NULL) && (s_listOfLights[i]->active) && (shadowNum < MAX_SHADOWS))
			{
				switch (s_listOfLights[i]->lightType)
				{
					case TG_LIGHT_AMBIENT:
					{
						//Ambient light casts no shadows!
					}
					break;

					case TG_LIGHT_INFINITE:
					{
						//----------------------------------------------
						// The Sun casts no shadows at night!!!
						if (!eye->getIsNight())
						{
							Stuff::Vector3D s_lightDir = s_rootLightDir[i];
							RotateLight(s_lightDir,rotation);
							
							for (long j=0;j<numVertices;j++)
							{
								long index = j + (shadowNum * numVertices);

								//if (!recalcShadows)
								{
									if (listOfShadowVertices[index].bDataIsNotValid)
									{
										recalcShadows = true;
										listOfShadowVertices[index].bDataIsNotValid = false;
									}
								}

								if (recalcShadows)	//Otherwise, just project last known DATA!!
													//This should almost never be true for any non-moving object.
													//Movers will recalc every frame.  Non-Movers will recalc 
													//when the sun moves enough!
								{
									Stuff::Point3D post = theShape->listOfTypeVertices[j].position;
									//--------------------------------------------------
									Stuff::Vector4D up;
									Stuff::Point3D s_position;
									shapeToLocal.Multiply(*s2w,identityMatrix);
									up.Multiply(post,shapeToLocal);

									//Everything is in WORLD coords now.
									//But this assumes terrain is FLAT and at zero elevation.
									// Thus, we must translate up to tangent plane at terrain position
									up.y -= pos->y;
									//--------------------------------------------------------
									// Check simple case first.  Vertex is on or below ground.
									if (up.y <= 0.0f)
									{
										s_position.x = up.x;
										s_position.z = up.z;
									}
									else
									{
										float zFactor = up.y / s_lightDir.y;
										s_position.x = up.x - (zFactor * s_lightDir.x);
										s_position.z = up.z - (zFactor * s_lightDir.z);
									}
									
									s_position.y = 0.0f;
			
									//-------------------------------------------------------------------
									// Do transform math for the vertex.
									//RotateLight(s_position,rotation);
									//post.Add(s_position,*pos);
									
									Stuff::Vector3D rPos;
									rPos.x = -s_position.x;
									rPos.y = s_position.z;
									rPos.z = land->getTerrainElevation(rPos);
									
									listOfShadowVertices[index].position = rPos;
									//--------------------------------------------------------------------------------
								}
								
								//The transformed position and fog data DO NOT NEED TO EXIST all of time.
								// ONLY the calced position data is invariant.  The other two could come
								// from a common pool and save us SCADS of RAM!!
								// Do this!!!!  I DID!
								// -fs
								eye->projectZ(listOfShadowVertices[index].position,listOfShadowTVertices[index].transformedPosition);
								shadowsVisible[shadowNum] = true;
								
								//eye->projectZ(listOfShadowVertices[index].position,listOfShadowTVertices[index].transformedPosition);
		
								DWORD fogRGB = 0xff000000;
								if (useFog)
								{
									if (pos->y < fogStart)
									{
										float fogFactor = fogStart - pos->y;
										if (fogFactor < 0.0)
											fogRGB = (0xff<<24);
										else
										{
											fogFactor /= (fogStart - fogFull);
											if (fogFactor <= 1.0)
											{
												fogFactor *= fogFactor;
												fogFactor = 1.0 - fogFactor;
												fogFactor *= 256.0;
											}
											else
											{
												fogFactor = 256.0;
											}
		
											unsigned char fogResult = float2long(fogFactor);
											fogRGB = (fogResult << 24);
										}
									}
									else
									{
										fogRGB = (0xff<<24);
									}
								}
								
								//-------------------
								// Distance FOG now.
								if (useFog && Camera::HazeFactor != 0.0f)
								{
									float fogFactor = 1.0 - Camera::HazeFactor;
									DWORD distFog = float2long(fogFactor * 255.0f);
									distFog <<= 24;
									
									if (distFog < fogRGB)
										fogRGB = distFog;
								}

								listOfShadowTVertices[index].fRGBFog = fogRGB;
				   			}

							shadowNum++;
						}
					}
					break;

					case TG_LIGHT_INFINITEWITHFALLOFF:
					{
						//Not planning to use this type of light anymore.  No shadows for now.
					}
					break;

					case TG_LIGHT_POINT:
					case TG_LIGHT_SPOT:
					{
						if (useLocalShadows)
						{
							//------------------------------------------------------------------------------------------
							// Point light does cast a shadow.  LightDir Will change every frame relative to the point.
							// I.e. we DO need to recalc every frame.
							
							//-----------------------------------------------------
							// Check if light source is IN_RANGE!
							Stuff::Point3D vertexToLight;
							vertexToLight = s_lightDir[i];
							float length = vertexToLight.GetApproximateLength();
							
							vertexToLight = s_rootLightDir[i];
							float spotLength = s_listOfLights[i]->maxSpotLength - vertexToLight.GetApproximateLength();
							if (spotLength < 50.0f)
								spotLength = 50.0f;
									
							float falloff = 1.0f;
	
							//Lights do not cast a shadow unless they are intense enough!!
							if (s_listOfLights[i]->GetFalloff(length, falloff) && (falloff > 0.5f))
							{
								for (long j=0;j<numVertices;j++)
								{
									long index = j + (shadowNum * numVertices);
									
									Stuff::Point3D post = theShape->listOfTypeVertices[j].position;
									//--------------------------------------------------
									Stuff::Vector4D up;
									Stuff::Point3D s_position;
									shapeToLocal.Multiply(*s2w,identityMatrix);
									up.Multiply(post,shapeToLocal);
	
									//Everything is in WORLD coords now.
									//But this assumes terrain is FLAT and at zero elevation.
									// Thus, we must translate up to tangent plane at terrain position
									up.y -= pos->y;
	
									shadowsVisible[shadowNum] = true;
									//--------------------------------------------------------
									// Check simple case first.  Vertex is on or below ground.
									if (up.y <= 0.0f)
									{
										s_position.x = up.x;
										s_position.z = up.z;
									}
									else
									{
										float zFactor = up.y / spotLength;
										s_position.x = up.x - (zFactor * -s_rootLightDir[i].x);
										s_position.z = up.z - (zFactor * -s_rootLightDir[i].z);
									}
									
									s_position.y = 0.0f;
		
									//-------------------------------------------------------------------
									// Do transform math for the vertex.
									//RotateLight(s_position,rotation);
									//post.Add(s_position,*pos);
	
									Stuff::Vector3D rPos;
									rPos.x = -s_position.x;
									rPos.y = s_position.z;
									rPos.z = land->getTerrainElevation(rPos);
	
									listOfShadowVertices[index].position = rPos;
									//--------------------------------------------------------------------------------
	
									//The transformed position and fog data DO NOT NEED TO EXIST all of time.
									// ONLY the calced position data is invariant.  The other two could come
									// from a common pool and save us SCADS of RAM!!
									// Do this!!!!  I DID!
									// -fs
									eye->projectZ(listOfShadowVertices[index].position,listOfShadowTVertices[index].transformedPosition);
									shadowsVisible[shadowNum] = true;
	
									DWORD fogRGB = 0xff000000;
									if (useFog)
									{
										if (pos->y < fogStart)
										{
											float fogFactor = fogStart - pos->y;
											if (fogFactor < 0.0)
												fogRGB = (0xff<<24);
											else
											{
												fogFactor /= (fogStart - fogFull);
												if (fogFactor <= 1.0)
												{
													fogFactor *= fogFactor;
													fogFactor = 1.0 - fogFactor;
													fogFactor *= 256.0;
												}
												else
												{
													fogFactor = 256.0;
												}
			
												unsigned char fogResult = float2long(fogFactor);
												fogRGB = (fogResult << 24);
											}
										}
										else
										{
											fogRGB = (0xff<<24);
										}
									}
									
									//-------------------
									// Distance FOG now.
									if (useFog && Camera::HazeFactor != 0.0f)
									{
										float fogFactor = 1.0 - Camera::HazeFactor;
										DWORD distFog = float2long(fogFactor * 255.0f);
										distFog <<= 24;
										
										if (distFog < fogRGB)
											fogRGB = distFog;
									}
	
									listOfShadowTVertices[index].fRGBFog = fogRGB;
								}
	
								shadowNum++;
							}
							else
							{
								shadowsVisible[shadowNum] = false;
							}
						}
					}
					break;
				}
			}
		}

	numVisibleShadows = 0;			//Reset Visible Shadows

	long totalShadows = 0;
	long j=0;
	for (j=0;j<MAX_SHADOWS;j++)
		if (shadowsVisible[j])
			totalShadows++;
		
	if (totalShadows)
	{
		for (j=0;j<numTriangles;j++)
		{
			listOfVisibleShadows[numVisibleShadows] = j;
			numVisibleShadows++;
			
			TG_TypeTriangle triType = theShape->listOfTypeTriangles[j];
			
			//--------------------------------------------------
			// Any sorting we need to do occurs in here as well!
			// So, find out what texture this face is using and mark that texture as needing 3 more vertices.
			if (theShape->listOfTextures[triType.localTextureHandle + (theShape->numTextures>>1)].gosTextureHandle != 0xffffffff)
			{
				for (long k=0;k<totalShadows;k++)
					mcTextureManager->addTriangle(theShape->listOfTextures[triType.localTextureHandle + (theShape->numTextures>>1)].mcTextureNodeIndex,MC2_DRAWALPHA | MC2_ISSHADOWS);
			}
			else
			{
				for (long k=0;k<totalShadows;k++)
					mcTextureManager->addTriangle(theShape->listOfTextures[triType.localTextureHandle].mcTextureNodeIndex,MC2_DRAWALPHA | MC2_ISSHADOWS);
			}
		}
	}
}	

//------------------------------------------------------------------------------------------------------------------------------------
// This functions draws the shadows.  May want zCompare and zWrite off at this point, hence separate render to facilitate draw order.
// So shadows fall across all 3d objects on terrain.
long TG_Shape::RenderShadows (long startFace)
{
	if (!renderTGLShapes)
		return(startFace);
		
 	if (!numVertices)		//WE are the root Shape which may have no shape or a helper shape which defintely has no shape!
		return(startFace);
		
	if (!numVisibleShadows)
		return startFace;
		
	//Can make this a flag to optimize!
	if (S_strnicmp(getNodeName(),"SpotLight_",10) == 0)
		return startFace;
		
	if (!listOfVertices ||
		!listOfColors ||
		!listOfShadowTVertices ||
		!listOfTriangles ||
		!listOfVisibleFaces ||
		!listOfVisibleShadows ||
		(/*(lastTurnTransformed != (turn-1)) &&*/ (lastTurnTransformed != turn)))
		return startFace;

 	if (fogColor != 0xffffffff)
	{
		//gos_SetRenderState( gos_State_Fog, (int)&(fogColor));
		gos_SetRenderState( gos_State_Fog, fogColor); //sebi
	}
	else
	{
		gos_SetRenderState( gos_State_Fog, 0);
	}

	TG_TypeShapePtr theShape = (TG_TypeShapePtr)myType;
	for (long i=0;i<MAX_SHADOWS;i++)
	{
		if (shadowsVisible[i])
		{
			for (long j=0;j<numVisibleShadows && listOfShadowTVertices;j++)
			{
				DWORD realStart = (j + startFace) % 64;
				if (listOfVisibleShadows[j] < numTriangles)
				{

					TG_TypeTriangle triType = theShape->listOfTypeTriangles[listOfVisibleShadows[j]];
	
					TG_ShadowVertexTemp vertex0 = listOfShadowTVertices[triType.Vertices[0] + (i * numVertices)];
					TG_ShadowVertexTemp vertex1 = listOfShadowTVertices[triType.Vertices[1] + (i * numVertices)];
					TG_ShadowVertexTemp vertex2 = listOfShadowTVertices[triType.Vertices[2] + (i * numVertices)];
	
					gos_VERTEX gVertex[3];
	
					gVertex[0].x		= vertex0.transformedPosition.x;
					gVertex[0].y		= vertex0.transformedPosition.y;
					gVertex[0].z		= vertex0.transformedPosition.z + (1.0f/60000.0f)*(realStart);
					gVertex[0].rhw		= vertex0.transformedPosition.w;
					gVertex[0].u		= triType.uvdata.u0;
					gVertex[0].v		= triType.uvdata.v0;
					gVertex[0].argb		= 0x3f000000;
					gVertex[0].frgb		= vertex0.fRGBFog;
	
					gVertex[1].x		= vertex1.transformedPosition.x;
					gVertex[1].y		= vertex1.transformedPosition.y;
					gVertex[1].z		= vertex1.transformedPosition.z + (1.0f/60000.0f)*(realStart);
					gVertex[1].rhw		= vertex1.transformedPosition.w;
					gVertex[1].u		= triType.uvdata.u1;
					gVertex[1].v		= triType.uvdata.v1;
					gVertex[1].argb		= 0x3f000000;
					gVertex[1].frgb		= vertex1.fRGBFog;
	
					gVertex[2].x		= vertex2.transformedPosition.x;
					gVertex[2].y		= vertex2.transformedPosition.y;
					gVertex[2].z		= vertex2.transformedPosition.z + (1.0f/60000.0f)*(realStart);
					gVertex[2].rhw		= vertex2.transformedPosition.w;
					gVertex[2].u		= triType.uvdata.u2;
					gVertex[2].v		= triType.uvdata.v2;
					gVertex[2].argb		= 0x3f000000;
					gVertex[2].frgb		= vertex2.fRGBFog;
	
					if (drawOldWay)
					{
						if (theShape->listOfTextures[triType.localTextureHandle + (theShape->numTextures>>1)].gosTextureHandle != 0xffffffff)
							gos_SetRenderState( gos_State_Texture, theShape->listOfTextures[triType.localTextureHandle + (theShape->numTextures>>1)].gosTextureHandle );
						else if (theShape->listOfTextures[triType.localTextureHandle].gosTextureHandle != 0xffffffff)
							gos_SetRenderState( gos_State_Texture, theShape->listOfTextures[triType.localTextureHandle].gosTextureHandle );
						else
							gos_SetRenderState( gos_State_Texture, 0 );
							
						if ((gVertex[0].z >= 0.0f) &&
							(gVertex[0].z < 1.0f) &&
							(gVertex[1].z >= 0.0f) &&  
							(gVertex[1].z < 1.0f) && 
							(gVertex[2].z >= 0.0f) &&  
							(gVertex[2].z < 1.0f))
						{
							gos_DrawTriangles(gVertex, 3);
						}
					}
					else
					{
						if ((gVertex[0].z >= 0.0f) &&
							(gVertex[0].z < 1.0f) &&
							(gVertex[1].z >= 0.0f) &&  
							(gVertex[1].z < 1.0f) && 
							(gVertex[2].z >= 0.0f) &&  
							(gVertex[2].z < 1.0f))
						{
							//--------------------------------------------------
							// Any sorting we need to do occurs in here as well!
							// So, find out what texture this face is using and mark that texture as needing 3 more vertices.
							if (theShape->listOfTextures[triType.localTextureHandle + (theShape->numTextures>>1)].gosTextureHandle != 0xffffffff)
							{
								mcTextureManager->addVertices(theShape->listOfTextures[triType.localTextureHandle + (theShape->numTextures>>1)].mcTextureNodeIndex,gVertex,MC2_ISSHADOWS | MC2_DRAWALPHA);
							}
							else
							{
								mcTextureManager->addVertices(theShape->listOfTextures[triType.localTextureHandle].mcTextureNodeIndex,gVertex,MC2_ISSHADOWS | MC2_DRAWALPHA);
							}
						}
					}
				}
			}
		}
	}

	gos_SetRenderState( gos_State_Fog, 0);

	return(startFace + numVisibleShadows);
}

//-------------------------------------------------------------------------------
