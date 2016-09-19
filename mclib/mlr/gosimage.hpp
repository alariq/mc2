//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_GOSIMAGE_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(GAMEOS_HPP)
#include<gameos.hpp>
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

namespace MidLevelRenderer {

	class GOSImage:
		public Stuff::Plug
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	public:
		GOSImage(const char* imageName);
		GOSImage(DWORD imageHandle);
		GOSImage(const char*, gos_TextureHints);

		~GOSImage();

		int
			GetWidth();

		int
			GetHeight();

		const char*
			GetName()
				{ Check_Object(this); return imageName; }

		int
			Ref()
				{ Check_Object(this); instance++; return instance; }

		int
			DeRef()
				{ Check_Object(this); instance--; return instance; }

		int
			GetRef()
				{ Check_Object(this); return instance; }

		bool
			IsLoaded()
				{ Check_Object(this); return ( (flags & Loaded) != 0); }

		DWORD GetHandle()
		{ 
			Check_Object(this);
			DWORD imageHandle = mcTextureManager->get_gosTextureHandle(mcTextureNodeIndex);

			if (imageHandle == 0xffffffff)
				imageHandle = 0;

			return imageHandle; 
		}

		void SetHandle (DWORD handle)
		{
			//EVERY call to this must change from gos_load to our load
			Check_Object(this);  
			mcTextureNodeIndex = handle;
		}

		enum {
			Loaded = 1,
			Locked = 2
		};

		void
			LockImage();
		void
			UnlockImage();

		unsigned char*
			GetImagePtr();
		int
			GetPitch()
				{ return ptr.Pitch; }

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const
				{}

		Stuff::MString imageName;
		int flags;
	protected:
		gos_TextureHints ipHints;
		int instance;
		DWORD mcTextureNodeIndex;
		TEXTUREPTR ptr;
	};

}
