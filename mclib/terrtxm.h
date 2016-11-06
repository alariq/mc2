//---------------------------------------------------------------------------
//
// TerrTxm.h -- File contains class definitions for the Terrain Textures
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TERRTXM_H
#define TERRTXM_H
//---------------------------------------------------------------------------
// Include Files
#ifndef DTERRTXM_H
#include"dterrtxm.h"
#endif

#ifndef MAPDATA_H
#include"mapdata.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions
#ifndef NO_ERR
#define NO_ERR		0
#endif

#define BLANK_TILE					(-1)

#define MC_MAX_TERRAIN_TXMS			3000

#define MC_MASK_NUM					14

#define MC_MAX_MIP_LEVELS			1

#define USE_SOFTWARE_MIPMAP

#define BASE_CEMENT_TYPE			10
#define START_CEMENT_TYPE			13
#define END_CEMENT_TYPE				20

extern int TERRAIN_TXM_SIZE;
//---------------------------------------------------------------------------
// Class Definitions
extern long tileCacheReqs;
extern long tileCacheHits;
extern long tileCacheMiss;

#define	MC2_TERRAIN_CEMENT_FLAG		0x00000001
#define MC2_TERRAIN_ALPHA_FLAG		0x00000002

//----------------------------------------------------
struct TerrainTXM
{
	DWORD			flags;						//Tells me if this texture is a cement texture, has alpha, etc.
	DWORD			mcTextureNodeIndex;			//Pointer to MCTextureNode which is used to cache handles if necessary
};

struct MC_TerrainType
{
	long			terrainId;
	char			*terrainName;
	char			*maskName;
	long			terrainPriority;
	MemoryPtr		*textureData;
	MemoryPtr		*maskData;
	long			baseTXMIndex;				//Index of Highest MIP Level of Texture.
	unsigned long	terrainMapRGB;				//Rgb Color used for TacMap.
	long			nameId;
};

struct MC_DetailType
{
	long			detailId;
	char			*detailName;
	MemoryPtr		*detailData;
	long			numDetails;
	float			frameRate;
	float			tilingFactor;		//How often does texture repeat
	long			baseTXMIndex;
};

struct MC_OverlayType
{
	long			overlayId;
	char			*overlayName;
	long			numTextures;
	long			oldOverlayId;
	bool			isMLRAppearance;
	MemoryPtr		*overlayData;				
	long			baseTXMIndex;
	unsigned long	terrainMapRGB;
};

struct TransitionType
{
	DWORD 			baseTXMIndex;				//When we build transition, where did we stick it
	DWORD 			transitionIndex;			//Code/Hash/Whatever to uniquely ID this transition.
	DWORD			overlayIndex;				//Overlays blended onto texture now.
};

//---------------------------------------------------------------------------
class TerrainTextures
{
	//Data Members
	//-------------
	protected:

		static long			numTxms;
		TerrainTXM			*textures;
		static long			nextAvailable;
		long				firstTransition;

		long				numTypes;
		MC_TerrainTypePtr	types;

		long				numOverlays;
		MC_OverlayTypePtr	overlays;
		
		long				numDetails;
		MC_DetailTypePtr	details;
		
		long				numTransitions;
		TransitionTypePtr	transitions;
		long				nextTransition;

		UserHeapPtr			tileHeap;
		UserHeapPtr			tileRAMHeap;

		long				globalMipLevel;
		long				firstOverlay;

		bool				quickLoad;				//Checks for list o magic textures and then checks that each magic texture exists.
		char				*localBaseName;
		File				*listTransitionFile;	

		static bool			terrainTexturesInstrumented;

	public:

	//Member Functions
	//-----------------
	protected:

		long initTexture (long typeNum);
		void initMask (long typeNum);
		long initDetail (long typeNum, long detailNum);
		long initOverlay (long overlayNum, long txmNum, char *txmName);

		long loadTextureMemory (const char *textureName, long mipSize);
		long loadOverlayMemory (const char *overlayName);
		long textureFromMemory (MemoryPtr ourRam, long mipSize);
		long loadDetailMemory  (const char *detailName, long mipSize);
		long textureFromMemoryAlpha (MemoryPtr ourRam, long mipSize);

		long createTransition (DWORD typeInfo, DWORD overlayInfo = 0);
		void combineTxm (MemoryPtr dest, DWORD binNumber, long type, long mipLevel);
		void combineOverlayTxm (MemoryPtr dest, long type, long mipLevel);

	public:
	
		void init (void)
		{
			tileHeap = NULL;
			tileRAMHeap = NULL;

			//----------------------------------------------
			// Stores composited and raw textures in VidMem
			numTxms = MC_MAX_TERRAIN_TXMS;
			nextAvailable = 0;
			textures = NULL;
			firstTransition = -1;

			//--------------------------------------------------
			// Stores Base Terrain Textures and Masks in SysMem
			numTypes = 0;
			types = NULL;

			//----------------------------------------------------------
			// Stores Overlay Terrain Textures in VidMem and MLR Shapes
			numOverlays = 0;
			overlays = NULL;

			numDetails = 0;
			details = NULL;
			
			//--------------------------------------------------------
			// Stores Transition Texture in VidMem.
			numTransitions = 0;
			transitions = NULL;
			nextTransition = 0;
			
			//------------------------------------------------------
			// Used to convert maps to new formats when textures change.
			firstOverlay = 0;

			//---------------------------------------------------------------------------------
			//Checks for list o magic textures and then checks that each magic texture exists. 
			quickLoad = false;
			localBaseName = NULL;
			listTransitionFile = NULL;
		}

		TerrainTextures (void)
		{
			init();
		}

		void destroy (void);

		~TerrainTextures (void)
		{
			destroy();
		}

		long init (const char *fileName, const char* baseName);
		
		long setTexture (DWORD typeInfo, DWORD overlayInfo);
		
		long setDetail (DWORD typeInfo, DWORD frameNum);
		
		float getDetailTilingFactor (long typeInfo)
		{
			if (typeInfo < numDetails)
				return details[typeInfo].tilingFactor;
				
			return 64.0f;
		}

		long getNumTypes() const { return numTypes; }

		long getTextureNameID( long id ) const { return types[id].nameId; }
		
		float getDetailFrameRate (long typeInfo)
		{
			if ((typeInfo < numDetails) && details[typeInfo].frameRate > Stuff::SMALL)
				return (1.0f / details[typeInfo].frameRate);
				
			return 0.066666667f;		//15 FPS
		}

		long setOverlay (DWORD overlayInfo);
		long getOverlayHandle( Overlays id, int Offset );
		void getOverlayInfoFromHandle( long handle, Overlays& id, DWORD& Offset );

		void purgeTransitions (void);
		
		DWORD getTextureTypeRGB (long typeInfo)
		{
			gosASSERT(typeInfo < numTypes);
			return types[typeInfo].terrainMapRGB;
		}

		DWORD getOverlayTypeRGB (long overlayNum)
		{
			gosASSERT(overlayNum < numOverlays);
			return overlays[overlayNum].terrainMapRGB;
		}

		long getTexturePriority (long typeInfo)
		{
			gosASSERT(typeInfo < numTypes);
			return types[typeInfo].terrainPriority;
		}

		DWORD getTexture (DWORD texture)
		{
			if ((long)texture >= nextAvailable) 
				return 0;

			if ( textures[texture].mcTextureNodeIndex == 0xffffffff )
				return 0;

			return (mcTextureManager->get_gosTextureHandle(textures[texture].mcTextureNodeIndex));
		}

		DWORD getTextureHandle (DWORD texture)
		{
			if ((long)texture >= nextAvailable) 
				return 0xffffffff;

			mcTextureManager->get_gosTextureHandle(textures[texture].mcTextureNodeIndex);
			return (textures[texture].mcTextureNodeIndex);
		}

		DWORD getDetail (DWORD dTexture)
		{
			if ((long)dTexture >= nextAvailable) 
				return 0;

			if ( textures[dTexture].mcTextureNodeIndex == 0xffffffff )
				return 0;

			return (mcTextureManager->get_gosTextureHandle(textures[dTexture].mcTextureNodeIndex));
		}

		DWORD getDetailHandle (DWORD dTexture)
		{
			if ((long)dTexture >= nextAvailable) 
				return 0;

			mcTextureManager->get_gosTextureHandle(textures[dTexture].mcTextureNodeIndex);
			return (textures[dTexture].mcTextureNodeIndex);
		}

		void setMipLevel (long mipLevel)
		{
			if ((mipLevel >= 0) && (mipLevel < MC_MAX_MIP_LEVELS))
				globalMipLevel = mipLevel;
		}
		
		long getFirstOverlay (void)
		{
			return firstOverlay;
		}
		
		bool isCement (DWORD typeInfo)
		{
			return (textures[typeInfo].flags & MC2_TERRAIN_CEMENT_FLAG) == MC2_TERRAIN_CEMENT_FLAG;
		}
		
		bool isAlpha (DWORD typeInfo)
		{
			return (textures[typeInfo].flags & MC2_TERRAIN_ALPHA_FLAG) == MC2_TERRAIN_ALPHA_FLAG;
		}

		static void initializeStatistics();
		
		void update (void);
};

//---------------------------------------------------------------------------
#endif
