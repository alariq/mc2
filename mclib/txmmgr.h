//---------------------------------------------------------------------
//
//
// This class will manage the texture memory provided by GOS
// GOS gives me a maximum of 256 256x256 pixel texture pages.
// I want GOS to think I only use 256x256 textures.  This class
// will insure that GOS believes that completely and provided
// smaller texture surfaces out of the main surface if necessary
// as well as returning the necessary UVs to get to the other surface.
//
// NOT ANY MORE
// Now we no longer coalesce textures.  Causes too many problems with rendering
// Now it simply checks for redundant filenames and can instance a texture instead of reloading it.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
//----------------------------------------------------------------------
#ifndef TXMMGR_H
#define TXMMGR_H

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include<string.h>
#include<gameos.hpp>
//----------------------------------------------------------------------
enum MC_TextureKey
{
	MC_INVALIDTEXTUREKEY = 0,
	MC_Solid,
	MC_Keyed,
	MC_Alpha
};

//----------------------------------------------------------------------
//Can store up to 1024 8x8 textures in 1 256x256.
#define MC_MAXTEXTURES				4096
#define CACHED_OUT_HANDLE			0xFFFFFACE		//If this value is in gosTextureHandle, texture is cached out and must be cached in.
#define MAX_MC2_GOS_TEXTURES		750
#define TEXTURE_CACHE_SIZE			(40*1024*1024)	//Extra space here to facilitate editting
#define MAX_CACHE_SIZE				(40*1024*1024)	//Actual amount map must run in!
#define MC_MAXFACES					50000
#define MAX_LZ_BUFFER_SIZE			((256*256*4) + 1024)

#define MC2_ISTERRAIN				1
#define MC2_DRAWSOLID				2
#define MC2_DRAWALPHA				4
#define MC2_ISSHADOWS				8
#define MC2_ISEFFECTS				16
#define MC2_DRAWONEIN				32
#define MC2_ISCRATERS				64
#define MC2_ISCOMPASS				128
#define MC2_ISSPOTLGT				256
#define MC2_ISHUDLMNT				512

//----------------------------------------------------------------------
// No More MC_BlockInfos.  gosTextureHandle is protected to enforce caching.
class MC_TextureManager;

//---------------------------------------------------------------------------
typedef struct _MC_VertexArrayNode
{
	friend MC_TextureManager;

	public:
		DWORD				textureIndex;
		DWORD				flags;						//Marks texture render state and terrain or not, etc.
		long				numVertices;				//Number of vertices this texture will be used to draw this frame.
		gos_VERTEX			*currentVertex;				//CurrentVertex data being added.
		gos_VERTEX			*vertices;					//Pointer into the vertex Pool for this texture to draw.

	void init (void)
	{
		flags = 0;
		numVertices = 0;
		currentVertex = 0;
		vertices = NULL;
		textureIndex = 0;
	}

	void destroy (void);							//Frees all blocks, free GOS_TextureHandle, blank all data.

} MC_VertexArrayNode;

//----------------------------------------------------------------------
typedef struct _MC_TextureNode
{
	friend MC_TextureManager;

	protected:
		DWORD				gosTextureHandle;			//Handle returned by GOS
		
	public:
		char 				*nodeName;					//Used for Unique nodes so I can just return the handle!
		DWORD				uniqueInstance;				//Texture is modifiable.  DO NOT CACHE OUT!!!!!!
		DWORD				neverFLUSH;					//Textures used by Userinput, etc.  DO NOT CACHE OUT!!!!!! 
		DWORD				numUsers;					//Pushed up for each user using.
														//Users can "free" a texture which will decrement the number and actually free it if number is 0
		gos_TextureFormat 	key;						//Used to recreate texture if cached out.
		DWORD 				hints;						//Used to recreate texture if cached out.
		DWORD				width;						//Used to recreate texture if cached out.
		DWORD				lzCompSize;					//Size of Compressed version.
		long				lastUsed;					//Last Game turn texture was used.  Used to cache textures.
		DWORD 				*textureData;				//Raw texture data.  Texture is stored here in system RAM 
														//if we overrun the max number of GOS HAndles.
														//When the texture is needed, another least used GOS handle is
														//cached out and this one is copied in.
														//This means that all instances where the game refers
														//To gosTextureHandle must be replaced with a get_gosTextureHandle
														//Or texture may not be cached in and it will draw the texture
														//at this location instead.  Just like some video cards do!
														//This value can be NULL if texture is on card.  
														//Should only be alloced when we need to cache.
														//This will keep system memory usage to a minimum.
		MC_VertexArrayNode 	*vertexData;				//This holds the vertex draw data.  NULL if not used.
		MC_VertexArrayNode	*vertexData2;
		MC_VertexArrayNode	*vertexData3;				

	void init (void)
	{
		gosTextureHandle = 0xffffffff;
		numUsers = 0;
		nodeName = NULL;
		textureData = NULL;
		lastUsed = -1;							//NEVER been used.
		key = gos_Texture_Solid;
		hints = gosHint_DisableMipmap;
		width = 0;
		uniqueInstance = 0x0;
		neverFLUSH = false;
		vertexData = NULL;
		vertexData2 = NULL;
		vertexData3 = NULL;
		lzCompSize = 0xffffffff;
	}

	DWORD findFirstAvailableBlock (void);

	void destroy (void);							//Frees all blocks, free GOS_TextureHandle, blank all data.

	void removeBlock (DWORD blockNum);				//Just free one block.  DO NOT FREE GOS_TextureHandle.

	void markBlock (DWORD blockNum);
	
	DWORD get_gosTextureHandle (void);				//If texture is not in VidRAM, cache a texture out and cache this one in.

} MC_TextureNode;

//---------------------------------------------------------------------------
class gos_VERTEXManager : public HeapManager
{
	//Data Members
	//-------------
	protected:
	
		long						totalVertices;		//Total number of vertices in pool.
		long						currentVertex;		//Pointer to next available vertex in pool.

	//Member Functions
	//-----------------
	public:

		void init (void)
		{
			HeapManager::init();

			totalVertices = 0;
			currentVertex = 0;
		}

		gos_VERTEXManager (void) : HeapManager()
		{
			init();
		}

		void destroy (void)
		{
			HeapManager::destroy();
			reset();
			totalVertices = 0;
		}
		
		~gos_VERTEXManager (void)
		{
			destroy();
		}

		void init (long maxVertices)
		{
			totalVertices = maxVertices;
			DWORD heapSize = totalVertices * sizeof(gos_VERTEX);
			createHeap(heapSize);
			commitHeap();
			reset();
		}
		
		gos_VERTEX *getVertexBlock(long numVertices)
		{
			gos_VERTEX *start = (gos_VERTEX *)getHeapPtr();
			start = &(start[currentVertex]);
			currentVertex += numVertices;
			gosASSERT(currentVertex < totalVertices);
			return start;
		}
		
		void reset (void)
		{
			currentVertex = 0;
		}
};

//----------------------------------------------------------------------
class MC_TextureManager
{
	friend MC_TextureNode;
	
	//Data Members
	//------------
	protected:

		MC_TextureNode					*masterTextureNodes;		//Dynamically allocated from an MC Heap.
		long							currentUsedTextures;		//Number of textures on video card.
													
		MC_VertexArrayNode 				*masterVertexNodes;			//Dynamically allocated from an MC Heap.
		long							nextAvailableVertexNode;	//index to next available vertex Node
													
		UserHeapPtr						textureCacheHeap;			//Heap used to cache textures from vidCard to system RAM.
		UserHeapPtr						textureStringHeap;			//Heap used to store filenames of textures so no dupes.
		bool 							textureManagerInstrumented;	//Texture Manager Instrumented.
		long							totalCacheMisses;			//NUmber of times flush has been called.\
		
		static gos_VERTEXManager		*gvManager;					//Stores arrays of vertices for draw.
		static MemoryPtr				lzBuffer1;					//Used to compress/decompress textures from cache.
		static MemoryPtr				lzBuffer2;					//Used to compress/decompress textures from cache.
		/* iBufferRefCount is used to help determine if lzBuffer1&2 are valid. The assumption
		is that if there are no valid MC_TextureManagers then lzBuffer1&2 are not valid. */
		static int				iBufferRefCount;
		
		WORD							*indexArray;				//Master Vertex Index array.

																	//Upto four different kinds of untextured triangle!
		MC_VertexArrayNode 				*vertexData;				//This holds the vertex draw data for UNTEXTURED triangles!
		MC_VertexArrayNode				*vertexData2;				//This holds the vertex draw data for UNTEXTURED triangles!
		MC_VertexArrayNode 				*vertexData3;				//This holds the vertex draw data for UNTEXTURED triangles!
		MC_VertexArrayNode				*vertexData4;				//This holds the vertex draw data for UNTEXTURED triangles!
		
	//Member Functions
	//-----------------
	public:

		void init (void)
		{
			masterTextureNodes = NULL;
			
			textureCacheHeap = NULL;
			textureStringHeap = NULL;
			textureManagerInstrumented = false;
			totalCacheMisses = 0;
			currentUsedTextures = 0;
			indexArray = NULL;
			
			masterVertexNodes = NULL;
			nextAvailableVertexNode = 0;
			
			vertexData = vertexData2 = vertexData3 = vertexData4 = NULL;
		}

		MC_TextureManager (void)
		{
			init();

			MC_TextureManager::iBufferRefCount++;
		}

		void destroy (void);

		~MC_TextureManager (void);

		void start (void);					// This function creates the Blank Nodes from an MC Heap.

		//-----------------------------------------------------------------------------
		// Returns whether or not loadTexture() would return a handle to a texture instance
		// that was already loaded in memory. This function was motivated by the fact that
		// that an existing texture instance can been modified in memory after it's loaded, and
		// thus be different the from an instance that would be loaded from disk.
		DWORD textureInstanceExists (const char *textureFullPathName, gos_TextureFormat key, DWORD hints, DWORD uniqueInstance = 0x0, DWORD nFlush = 0x0);

		//-----------------------------------------------------------------------------
		// Returns the TextureNode Id based on what you asked for.
		//
		// uniqueInstance is an ID for the instance of the texture. If its value matches that of
		// an already existing instance of the texture, the handle of the existing instance will
		// be returned. Used for Mech Coloration possibly, damage states, etc.
		DWORD loadTexture (const char *textureFullPathName, gos_TextureFormat key, DWORD hints, DWORD uniqueInstance = 0x0, DWORD nFlush = 0x0);

		long saveTexture (DWORD textureIndex, const char *textureFullPathName);

		//-----------------------------------------------------------------------------
		// Returns the TextureNode Id based on what you asked for.
		DWORD textureFromMemory (DWORD *data, gos_TextureFormat key, DWORD hints, DWORD width, DWORD bitDepth = 4);

		// increments the ref count
		DWORD copyTexture( DWORD texNodeID );

		//------------------------------------------------------
		// Tosses ALL of the textureNodes and frees GOS Handles
		void flush (bool justTextures=false);

		//------------------------------------------------------
		// Frees up gos_VERTEX manager memory
		void freeVertices (void);
		
		//------------------------------------------------------
		// Creates gos_VERTEX Manager and allocates RAM.  Will not allocate if already done!
		void startVertices (long maxVertices = 30000);
		
		//------------------------------------------------------
		// Frees a specific texture. 
		void removeTexture (DWORD gosTextureHandle);
		
		//------------------------------------------------------
		// Frees a specific textureNode. 
		void removeTextureNode (DWORD textureNode);
		
 		//-----------------------------------------------------------------
		// Gets gosTextureHandle for Node ID.  Does all caching necessary.
		DWORD get_gosTextureHandle (DWORD nodeId)
		{
			if (nodeId != 0xffffffff)
				return masterTextureNodes[nodeId].get_gosTextureHandle();
			else
				return nodeId;
		}

		void addTriangle (DWORD nodeId, DWORD flags)
		{
			if ((nodeId < MC_MAXTEXTURES) && (nextAvailableVertexNode < MC_MAXTEXTURES))
			{
				if (!masterTextureNodes[nodeId].vertexData)
				{
					masterTextureNodes[nodeId].vertexData = &(masterVertexNodes[nextAvailableVertexNode]);
					gosASSERT(masterTextureNodes[nodeId].vertexData->numVertices == 0);
					gosASSERT(masterTextureNodes[nodeId].vertexData->vertices == NULL); 

					nextAvailableVertexNode++;
					masterTextureNodes[nodeId].vertexData->flags = flags;
					masterTextureNodes[nodeId].vertexData->textureIndex = nodeId;
				}
				else if (masterTextureNodes[nodeId].vertexData &&
					(masterTextureNodes[nodeId].vertexData->flags != flags) &&
					!masterTextureNodes[nodeId].vertexData2)
				{
					masterTextureNodes[nodeId].vertexData2 = &(masterVertexNodes[nextAvailableVertexNode]);
					gosASSERT(masterTextureNodes[nodeId].vertexData2->numVertices == 0);
					gosASSERT(masterTextureNodes[nodeId].vertexData2->vertices == NULL); 

					nextAvailableVertexNode++;
					masterTextureNodes[nodeId].vertexData2->flags = flags;
					masterTextureNodes[nodeId].vertexData2->textureIndex = nodeId;
				}
				else if (masterTextureNodes[nodeId].vertexData &&
						(masterTextureNodes[nodeId].vertexData->flags != flags) &&
						masterTextureNodes[nodeId].vertexData2 &&
						(masterTextureNodes[nodeId].vertexData2->flags != flags) &&
						!masterTextureNodes[nodeId].vertexData3)
				{
					masterTextureNodes[nodeId].vertexData3 = &(masterVertexNodes[nextAvailableVertexNode]);
					gosASSERT(masterTextureNodes[nodeId].vertexData3->numVertices == 0);
					gosASSERT(masterTextureNodes[nodeId].vertexData3->vertices == NULL); 

					nextAvailableVertexNode++;
					masterTextureNodes[nodeId].vertexData3->flags = flags;
					masterTextureNodes[nodeId].vertexData3->textureIndex = nodeId;
				}
					
				if (masterTextureNodes[nodeId].vertexData->flags == flags)
					masterTextureNodes[nodeId].vertexData->numVertices += 3;
				else if (masterTextureNodes[nodeId].vertexData2 &&
						masterTextureNodes[nodeId].vertexData2->flags == flags)
					masterTextureNodes[nodeId].vertexData2->numVertices += 3;
				else if (masterTextureNodes[nodeId].vertexData3 &&
						masterTextureNodes[nodeId].vertexData3->flags == flags)
					masterTextureNodes[nodeId].vertexData3->numVertices += 3;
#ifdef _DEBUG
				else
					STOP(("Could not AddTriangles.  No flags match vertex data"));
#endif
			}
			else if (nextAvailableVertexNode < MC_MAXTEXTURES)
			{
				//Add this one to the untextured vertexBuffers
				if (!vertexData)
				{
					vertexData = &(masterVertexNodes[nextAvailableVertexNode]);
					gosASSERT(vertexData->numVertices == 0);
					gosASSERT(vertexData->vertices == NULL); 

					nextAvailableVertexNode++;
					vertexData->flags = flags;
					vertexData->textureIndex = 0;
				}
				else if (vertexData && (vertexData->flags != flags) &&
						!vertexData2)
				{
					vertexData2 = &(masterVertexNodes[nextAvailableVertexNode]);
					gosASSERT(vertexData2->numVertices == 0);
					gosASSERT(vertexData2->vertices == NULL); 

					nextAvailableVertexNode++;
					vertexData2->flags = flags;
					vertexData2->textureIndex = 0;
				}
				else if (vertexData && (vertexData->flags != flags) &&
						vertexData2 && (vertexData2->flags != flags) && 
						!vertexData3)
				{
					vertexData3 = &(masterVertexNodes[nextAvailableVertexNode]);
					gosASSERT(vertexData3->numVertices == 0);
					gosASSERT(vertexData3->vertices == NULL); 

					nextAvailableVertexNode++;
					vertexData3->flags = flags;
					vertexData3->textureIndex = 0;
				}
				else if (vertexData && (vertexData->flags != flags) &&
						vertexData2 && (vertexData2->flags != flags) && 
						vertexData3 && (vertexData3->flags != flags) && 
						!vertexData4)
				{
					vertexData4 = &(masterVertexNodes[nextAvailableVertexNode]);
					gosASSERT(vertexData4->numVertices == 0);
					gosASSERT(vertexData4->vertices == NULL); 

					nextAvailableVertexNode++;
					vertexData4->flags = flags;
					vertexData4->textureIndex = 0;
				}
				
 				if (vertexData->flags == flags)
					vertexData->numVertices += 3;
				else if (vertexData2 && vertexData2->flags == flags)
					vertexData2->numVertices += 3;
				else if (vertexData3 && vertexData3->flags == flags)
					vertexData3->numVertices += 3;
				else if (vertexData4 && vertexData4->flags == flags)
					vertexData4->numVertices += 3;
#ifdef _DEBUG
				else
					PAUSE(("Could not AddTriangles.  Too many untextured triangle types"));
#endif
			}
		}

		void addVertices (DWORD nodeId, gos_VERTEX *data, DWORD flags)
		{
			//This function adds the actual vertex data to the texture Node.
			if (nodeId < MC_MAXTEXTURES)
			{
				if (masterTextureNodes[nodeId].vertexData && 
					masterTextureNodes[nodeId].vertexData->flags == flags)
				{
					gos_VERTEX * vertices = masterTextureNodes[nodeId].vertexData->currentVertex;
					if (!vertices && !masterTextureNodes[nodeId].vertexData->vertices)
					{
						masterTextureNodes[nodeId].vertexData->currentVertex =
						vertices = 
						masterTextureNodes[nodeId].vertexData->vertices =
						gvManager->getVertexBlock(masterTextureNodes[nodeId].vertexData->numVertices);
					}
	
					if (vertices < (masterTextureNodes[nodeId].vertexData->vertices + masterTextureNodes[nodeId].vertexData->numVertices))
					{
						#if 0
						if ((data[0].u > 64.0f) ||
							(data[0].u < -64.0f) || 
							(data[1].u > 64.0f) ||  
							(data[1].u < -64.0f) ||  
							(data[2].u > 64.0f) ||  
							(data[2].u < -64.0f))
						{
							PAUSE(("Vertex U Out of range"));
						}

						if ((data[0].v > 64.0f) ||
							(data[0].v < -64.0f) || 
							(data[1].v > 64.0f) ||  
							(data[1].v < -64.0f) ||  
							(data[2].v > 64.0f) ||  
							(data[2].v < -64.0f))
						{
							PAUSE(("Vertex V Out of range"));
						}
						#endif

						memcpy(vertices,data,sizeof(gos_VERTEX) * 3);
						vertices += 3;
					}
					
					masterTextureNodes[nodeId].vertexData->currentVertex = vertices;
				}
				else if (masterTextureNodes[nodeId].vertexData2 &&
						 masterTextureNodes[nodeId].vertexData2->flags == flags)
				{
					gos_VERTEX * vertices = masterTextureNodes[nodeId].vertexData2->currentVertex;
					
					#if defined( _DEBUG) || defined(_ARMOR)
					gos_VERTEX * oldVertices = vertices;
					gos_VERTEX * oldStart = (masterTextureNodes[nodeId].vertexData2->vertices + masterTextureNodes[nodeId].vertexData2->numVertices);
					#endif
					gosASSERT(oldVertices < oldStart);

					if (!vertices && !masterTextureNodes[nodeId].vertexData2->vertices)
					{
						masterTextureNodes[nodeId].vertexData2->currentVertex =
						vertices = 
						masterTextureNodes[nodeId].vertexData2->vertices =
						gvManager->getVertexBlock(masterTextureNodes[nodeId].vertexData2->numVertices);
					}
	
					if (vertices < (masterTextureNodes[nodeId].vertexData2->vertices + masterTextureNodes[nodeId].vertexData2->numVertices))
					{
						#if 0
						if ((data[0].u > 64.0f) ||
							(data[0].u < -64.0f) || 
							(data[1].u > 64.0f) ||  
							(data[1].u < -64.0f) ||  
							(data[2].u > 64.0f) ||  
							(data[2].u < -64.0f))
						{
							PAUSE(("Vertex U Out of range"));
						}

						if ((data[0].v > 64.0f) ||
							(data[0].v < -64.0f) || 
							(data[1].v > 64.0f) ||  
							(data[1].v < -64.0f) ||  
							(data[2].v > 64.0f) ||  
							(data[2].v < -64.0f))
						{
							PAUSE(("Vertex V Out of range"));
						}
						#endif
						
						memcpy(vertices,data,sizeof(gos_VERTEX) * 3);
						vertices += 3;
					}
					
					masterTextureNodes[nodeId].vertexData2->currentVertex = vertices;
				}
				else if (masterTextureNodes[nodeId].vertexData3 &&
						 masterTextureNodes[nodeId].vertexData3->flags == flags)
				{
					gos_VERTEX * vertices = masterTextureNodes[nodeId].vertexData3->currentVertex;
					
					#if defined(_DEBUG) || defined(_ARMOR)
					gos_VERTEX * oldVertices = vertices;
					gos_VERTEX * oldStart = (masterTextureNodes[nodeId].vertexData3->vertices + masterTextureNodes[nodeId].vertexData3->numVertices);
					#endif
					gosASSERT(oldVertices < oldStart);

					if (!vertices && !masterTextureNodes[nodeId].vertexData3->vertices)
					{
						masterTextureNodes[nodeId].vertexData3->currentVertex =
						vertices = 
						masterTextureNodes[nodeId].vertexData3->vertices =
						gvManager->getVertexBlock(masterTextureNodes[nodeId].vertexData3->numVertices);
					}
	
					if (vertices < (masterTextureNodes[nodeId].vertexData3->vertices + masterTextureNodes[nodeId].vertexData3->numVertices))
					{
						#if 0
						if ((data[0].u > 64.0f) ||
							(data[0].u < -64.0f) || 
							(data[1].u > 64.0f) ||  
							(data[1].u < -64.0f) ||  
							(data[2].u > 64.0f) ||  
							(data[2].u < -64.0f))
						{
							PAUSE(("Vertex U Out of range"));
						}

						if ((data[0].v > 64.0f) ||
							(data[0].v < -64.0f) || 
							(data[1].v > 64.0f) ||  
							(data[1].v < -64.0f) ||  
							(data[2].v > 64.0f) ||  
							(data[2].v < -64.0f))
						{
							PAUSE(("Vertex V Out of range"));
						}
						#endif
						
						memcpy(vertices,data,sizeof(gos_VERTEX) * 3);
						vertices += 3;
					}
					
					masterTextureNodes[nodeId].vertexData3->currentVertex = vertices;
				}
				else	//If we got here, something is really wrong
				{
#ifdef _DEBUG
					PAUSE(("Flags do not match either set of vertex Data"));
#endif
				}
			}
			else
			{
				if (vertexData && vertexData->flags == flags)
				{
					gos_VERTEX * vertices = vertexData->currentVertex;
					if (!vertices && !vertexData->vertices)
					{
						vertexData->currentVertex =
						vertices = 
						vertexData->vertices =
						gvManager->getVertexBlock(vertexData->numVertices);
					}
	
					if (vertices <= (vertexData->vertices + vertexData->numVertices))
					{
						memcpy(vertices,data,sizeof(gos_VERTEX) * 3);
						vertices += 3;
					}
					
					vertexData->currentVertex = vertices;
				}
				else if (vertexData2 && vertexData2->flags == flags)
				{
					gos_VERTEX * vertices = vertexData2->currentVertex;
					if (!vertices && !vertexData2->vertices)
					{
						vertexData2->currentVertex =
						vertices = 
						vertexData2->vertices =
						gvManager->getVertexBlock(vertexData2->numVertices);
					}
	
					if (vertices <= (vertexData2->vertices + vertexData2->numVertices))
					{
						memcpy(vertices,data,sizeof(gos_VERTEX) * 3);
						vertices += 3;
					}
					
					vertexData2->currentVertex = vertices;
				}
				else if (vertexData3 && vertexData3->flags == flags)
				{
					gos_VERTEX * vertices = vertexData3->currentVertex;
					if (!vertices && !vertexData3->vertices)
					{
						vertexData3->currentVertex =
						vertices = 
						vertexData3->vertices =
						gvManager->getVertexBlock(vertexData3->numVertices);
					}
	
					if (vertices <= (vertexData3->vertices + vertexData3->numVertices))
					{
						memcpy(vertices,data,sizeof(gos_VERTEX) * 3);
						vertices += 3;
					}
					
					vertexData3->currentVertex = vertices;
				}
				else if (vertexData4 && vertexData4->flags == flags)
				{
					gos_VERTEX * vertices = vertexData4->currentVertex;
					if (!vertices && !vertexData4->vertices)
					{
						vertexData4->currentVertex =
						vertices = 
						vertexData4->vertices =
						gvManager->getVertexBlock(vertexData4->numVertices);
					}

					if (vertices <= (vertexData4->vertices + vertexData4->numVertices))
					{
						memcpy(vertices,data,sizeof(gos_VERTEX) * 3);
						vertices += 3;
					}
					
					vertexData4->currentVertex = vertices;
				}
				else	//If we got here, something is really wrong
				{
#ifdef _DEBUG
					PAUSE(("Flags do not match any set of untextured vertex Data"));
#endif
				}
			}
		}

		void clearArrays (void)
		{
			for (long i=0;i<MC_MAXTEXTURES;i++)
			{
				masterTextureNodes[i].vertexData = NULL;
				masterTextureNodes[i].vertexData2 = NULL;
				masterTextureNodes[i].vertexData3 = NULL;
			}

			vertexData = vertexData2 = vertexData3 = vertexData4 = NULL;
			
			memset(masterVertexNodes,0,sizeof(MC_VertexArrayNode)*MC_MAXTEXTURES);
			
			nextAvailableVertexNode = 0;

			gvManager->reset();
		}
		
		//Sends down the triangle lists
		void renderLists (void);

		DWORD getWidth( DWORD nodeId )
		{
			if (nodeId != 0xffffffff)
				return masterTextureNodes[nodeId].width;
			else
				return nodeId;
		}
		
		//----------------------------------------------------------------------------
		// FlushCache copies a single flushable texture from VidRAM to system Memory
		// No real logic yet, just find the first unused texture and swap it.
		// DO NOT want to implement a priority system at all.
		// Returns FALSE if no texture can be cached out this frame. 
		bool flushCache (void);
		
		//-----------------------------------------------------------------------
		// This routine will run through the TXM Cache on a regular basis and free
		// up GOS Handles which haven't been used in some time.  Some Time TBD 
		// more accurately with time.
		DWORD update (void);
		
		bool checkCacheHeap (void)
		{
			if (textureCacheHeap->totalCoreLeft() <= (TEXTURE_CACHE_SIZE - MAX_CACHE_SIZE))
			{
				return false;
			}
			
			return true;
		}
		
};

//----------------------------------------------------------------------
extern MC_TextureManager *mcTextureManager;

//----------------------------------------------------------------------
#endif
