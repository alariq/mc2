//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRTEXTURE_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_GOSIMAGE_HPP)
#include<mlr/gosimage.hpp>
#endif

namespace MidLevelRenderer {

	class MLRTexturePool;

	class MLRTexture
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
		friend class MLRTexturePool;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	protected:
		MLRTexture(Stuff::MemoryStream *stream);

	public:
		MLRTexture(
			MLRTexturePool *pool,
			const char* name,
			int instance,
			int handle,
			int hint=0
		);

		MLRTexture(
			MLRTexturePool *pool,
			GOSImage* image,
			int handle,
			int hint=0
		);

		MLRTexture(const MLRTexture&);
		~MLRTexture();

		static MLRTexture*
			Make(Stuff::MemoryStream *stream);

		void
			Save(Stuff::MemoryStream *stream);

		GOSImage*
			GetImage(int *h=NULL)
				{ Check_Object(this); if(h) { *h = hint; } return image; }

		const char*
			GetTextureName()
				{Check_Object(this); return textureName;}

		int
			GetTextureHandle()
				{Check_Object(this); return textureHandle;}

		int
			GetImageNumber();

		int
			GetInstanceNumber();

		int
			GetTextureInstance()
				{Check_Object(this); return instance;}

		bool
			GetAnimateTexture()
				{Check_Object(this); return !textureMatrixIsIdentity;}

		void
			SetAnimateTexture(bool yesNo)
				{
					Check_Object(this);
					if(yesNo==true)
					{
						textureMatrixIsIdentity = false;
					}
					else
					{
						textureMatrixIsIdentity = true;
						textureMatrix = Stuff::AffineMatrix4D::Identity;
					}
				}

		Stuff::AffineMatrix4D&
			GetTextureMatrix()
				{ Check_Object(this); return textureMatrix; }

		void
			SetHint(int h)
				{ Check_Object(this); hint = h; }

		int
			GetHint()
				{ Check_Object(this); return hint; }

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const;

	protected:
		Stuff::MString textureName;
		int textureNameHashValue;

		int instance;

		int textureHandle;

		int hint;

		bool textureMatrixIsIdentity;

		Stuff::AffineMatrix4D textureMatrix;

		GOSImage *image;

		MLRTexturePool *thePool;
	};

}
