//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_GOSIMAGEPOOL_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

namespace MidLevelRenderer {

	class GOSImagePool
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	public:
		GOSImagePool();
		~GOSImagePool();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Image handling
	//
	public:
		GOSImage*
			GetImage(const char* imageName);
		GOSImage*
			GetImage(const char* imageName, gos_TextureFormat format, int size, gos_TextureHints hints);
		virtual bool
			LoadImage(GOSImage *image, int=0)=0;
		void
			RemoveImage(GOSImage *image);
		
		void UnLoadImages (void);

		void
			GetTexturePath(Stuff::MString* pName) const
				{ Check_Object(this); *pName = texturePath; }

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const
				{}

	protected:
		Stuff::HashOf<GOSImage*, Stuff::MString>
			imageHash;
		Stuff::MString
			texturePath;
	};

	class TGAFilePool:
		public GOSImagePool
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	public:
		TGAFilePool(const char* path);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Image handling
	//
	public:
		bool
			LoadImage(GOSImage *image, int=0);

	};
}
