//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRTEXTUREPOOL_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_GOSIMAGEPOOL_HPP)
#include<mlr/gosimagepool.hpp>
#endif

namespace MidLevelRenderer {

	class MLRTexturePool:
		public Stuff::RegisteredClass

	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialization
	//
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();
		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	protected:
		MLRTexturePool(Stuff::MemoryStream *stream);

	public:
	//	insDep == nr of lower bits used for image instancing
		MLRTexturePool(GOSImagePool *image_pool, int insDep=3);

		~MLRTexturePool();

		static MLRTexturePool*
			Make(Stuff::MemoryStream *stream);

		void
			Save(Stuff::MemoryStream *stream);

		MLRTexture*
			Add(const char *textureName, int instance=0);

		MLRTexture*
			Add(GOSImage*);

		MLRTexture*
			Add(const char* imageName, gos_TextureFormat format, int size, gos_TextureHints hints)
				{ return Add(imagePool->GetImage(imageName, format, size, hints) ); }

	// only removes the texture from the texture pool, it doesnt destroy the texture
		void
			Remove(MLRTexture*);

		unsigned
			LoadImages();

		MLRTexture*
			operator() (const char *name, int=0);

		MLRTexture*
			operator[] (int index)
				{ Check_Object(this); Verify(index-1 < MLRState::TextureMask); return textureArray[index-1]; }

		MLRTexture*
			operator[] (const MLRState *state)
				{ Check_Object(this); return textureArray[state->GetTextureHandle()-1]; }

		GOSImage*
			GetImage(const char* imageName)
				{ Check_Object(this); return imagePool->GetImage(imageName);	}

		const GOSImagePool*
			GetGOSImagePool()
				{ Check_Object(this); return imagePool;	}

		unsigned
			GetLastHandle()
				{Check_Object(this); return lastHandle;}

		void Stop (void);
		void Restart (void);

		unsigned 
			GetNumStoredTextures()
				{Check_Object(this); return storedTextures;}

		int
			GetInstanceDepth() const
				{ Check_Object(this); return instanceDepth; }

		static MLRTexturePool
			*Instance;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const
				{}

	protected:
		bool
			unLoadedImages;

		int	instanceDepth, // bits used for image instancing
			instanceMax; // max for image instancing

		int	handleDepth, // bits used for image instancing
			handleMax; // max for image instancing

		int lastHandle;
		int storedTextures;
		
		Stuff::StaticArrayOf<MLRTexture*, MLRState::TextureMask+1> textureArray;

		int *freeHandle;
		int firstFreeHandle;
		int lastFreeHandle;

		GOSImagePool
			*imagePool;
	};

}
