//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

MLRTexturePool *MLRTexturePool::Instance;

MLRTexturePool::ClassData*
	MLRTexturePool::DefaultData = NULL;

//#############################################################################
//############################    MLRTexture    ###############################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRTexturePool::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRTexturePoolClassID,
			"MidLevelRenderer::MLRTexturePool",
			RegisteredClass::DefaultData
		);
	Register_Object(DefaultData);

	MLRTexturePool::Instance = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRTexturePool::TerminateClass()
{
	if (MLRTexturePool::Instance)
	{
		Unregister_Object(MLRTexturePool::Instance);
		delete MLRTexturePool::Instance;
	}

	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexturePool::MLRTexturePool(MemoryStream *stream):
	RegisteredClass(DefaultData)
{
	Verify(gos_GetCurrentHeap() == Heap);

	unLoadedImages = false;

	STOP(("Not implemented"));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void MLRTexturePool::Stop (void)
{
	Verify(gos_GetCurrentHeap() == Heap);

	int i;

	for(i=0;i<MLRState::TextureMask;i++)
	{
		if(textureArray[i] != NULL)
		{
			Unregister_Object(textureArray[i]);

			delete textureArray[i];

			textureArray[i] = NULL;
		}
	}

	Unregister_Pointer(freeHandle);
	delete [] freeHandle;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void MLRTexturePool::Restart (void)
{
	Verify(gos_GetCurrentHeap() == Heap);

	freeHandle = new int [handleMax];
	Register_Pointer(freeHandle);
	lastHandle = 0;
	firstFreeHandle = 0;
	lastFreeHandle = 0;

	storedTextures = 0;

	for(int i=0;i<MLRState::TextureMask+1;i++)
	{
		textureArray[i] = NULL;
	}

	unLoadedImages = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexturePool::MLRTexturePool(GOSImagePool *image_pool, int insDep):
	RegisteredClass(DefaultData)
{
	Check_Object(image_pool);
	Verify(gos_GetCurrentHeap() == Heap);

	instanceDepth = insDep;
	instanceMax = 1<<insDep;

	handleDepth = MLRState::TextureNumberBits - insDep;
	handleMax = 1<<handleDepth;

	freeHandle = new int [handleMax];
	Register_Pointer(freeHandle);
	lastHandle = 0;
	firstFreeHandle = 0;
	lastFreeHandle = 0;

	storedTextures = 0;

	imagePool = image_pool;

	for(int i=0;i<MLRState::TextureMask+1;i++)
	{
		textureArray[i] = NULL;
	}

	unLoadedImages = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexturePool::~MLRTexturePool()
{
	int i;

	for(i=0;i<MLRState::TextureMask;i++)
	{
		if(textureArray[i] != NULL)
		{
			Unregister_Object(textureArray[i]);

			delete textureArray[i];

			textureArray[i] = NULL;
		}
	}
	Unregister_Object(imagePool);
	delete imagePool;

	Unregister_Pointer(freeHandle);
	delete [] freeHandle;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexturePool*
	MLRTexturePool::Make(MemoryStream *stream)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLRTexturePool *pool = new MLRTexturePool(stream);
	gos_PopCurrentHeap();

	return pool;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRTexturePool::Save(MemoryStream *stream)
{
	STOP(("Not implemented"));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture*
	MLRTexturePool::Add(const char *tn, int instance)
{
	MString textureName(tn);

	int i, j, textureNameHashValue = textureName.GetHashValue();

	for(i=0;i<lastHandle;i++)
	{
		int first = i<<instanceDepth;
		bool yo = false;

		for(j=first;j<first+instanceMax;j++)
		{
			if(	textureArray[j] && 
				textureArray[j]->textureNameHashValue == textureNameHashValue )
			{
				yo = 1;
			}
		}

		if(yo == false)
		{
			continue;
		}

		for(j=first;j<first+instanceMax;j++)
		{
			if(	textureArray[j] && 
				textureArray[j]->instance == instance)
			{
				return textureArray[j];
			}
		}

		for(j=first;j<first+instanceMax;j++)
		{
			if(!textureArray[j])
			{
				gos_PushCurrentHeap(Heap);
				textureArray[j] = 
					new MLRTexture(
						this, 
						textureName,
						instance,
						j+1
					);
				Register_Object(textureArray[j]);
				gos_PopCurrentHeap();

				storedTextures++;

				unLoadedImages = true;

				return textureArray[j];
			}
		}
		STOP(("Asked for too much image instances !"));
	}

	int newHandle;

	gos_PushCurrentHeap(Heap);
	if(firstFreeHandle < lastFreeHandle)
	{
		newHandle = (freeHandle[firstFreeHandle&(handleMax-1)])<<instanceDepth;

		textureArray[newHandle] = 
			new MLRTexture(
				this, 
				textureName,
				instance,
				newHandle+1
			);

		storedTextures++;

		firstFreeHandle++;
	}
	else
	{
		Verify( ((lastHandle<<instanceDepth)+1) < MLRState::TextureMask );

		newHandle = lastHandle<<instanceDepth;

		textureArray[newHandle] = 
			new MLRTexture(
				this, 
				textureName,
				instance,
				newHandle+1
			);
	
		storedTextures++;

		lastHandle++;
	}

	Register_Object(textureArray[newHandle]);
	gos_PopCurrentHeap();

	unLoadedImages = true;

	return textureArray[newHandle];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture*
	MLRTexturePool::Add(GOSImage *image)
{
	MString textureName;

	textureName = image->GetName();

	int i, j, textureNameHashValue = textureName.GetHashValue();

	for(i=0;i<lastHandle;i++)
	{
		int first = i<<instanceDepth;

		for(j=first;j<first+instanceMax;j++)
		{
			if(	textureArray[j] && 
				textureArray[j]->textureNameHashValue == textureNameHashValue )
			{
				Verify(image == textureArray[j]->GetImage());
				return textureArray[j];
//				STOP(("Image allready in texture pool !"));
			}
		}
	}

	int newHandle;

	gos_PushCurrentHeap(Heap);
	if(firstFreeHandle < lastFreeHandle)
	{
		newHandle = (freeHandle[firstFreeHandle&(handleMax-1)])<<instanceDepth;

		textureArray[newHandle] = 
			new MLRTexture(
				this, 
				image,
				newHandle+1
			);

		storedTextures++;

		firstFreeHandle++;
	}
	else
	{
		Verify( ((lastHandle<<instanceDepth)+1) < MLRState::TextureMask );

		newHandle = lastHandle<<instanceDepth;

		textureArray[newHandle] = 
			new MLRTexture(
				this, 
				image,
				newHandle+1
			);
	
		storedTextures++;

		lastHandle++;
	}

	Register_Object(textureArray[newHandle]);
	gos_PopCurrentHeap();

	return textureArray[newHandle];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRTexturePool::Remove(MLRTexture *tex)
{
	textureArray[tex->textureHandle-1] = NULL;
	storedTextures--;

	int i, first = (tex->textureHandle-1) & ~(instanceMax-1);

	for(i=first;i<first+instanceMax;i++)
	{
		if(textureArray[i] != NULL)
		{
			break;
		}
	}

	if(i >= first+instanceMax)
	{
		imagePool->RemoveImage(tex->image);

		tex->image = NULL;

		freeHandle[lastFreeHandle&(handleMax-1)] = (tex->textureHandle-1) >> instanceDepth;

		lastFreeHandle++;
	}

	tex->textureHandle = 0;
	unLoadedImages = true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture*
	MLRTexturePool::operator()(const char *tn, int instance)
{
	Check_Object(this);

	MString textureName = tn;

	int i, j, textureNameHashValue = textureName.GetHashValue();

	for(i=0;i<lastHandle;i++)
	{
		int first = i<<instanceDepth;

		for(j=first;j<first+instanceMax;j++)
		{
			if(	textureArray[j] )
			{
				if(	textureArray[j]->textureNameHashValue == textureNameHashValue )
				{
					if (textureArray[j]->instance == instance )
					{
						return textureArray[j];
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
unsigned
	MLRTexturePool::LoadImages()
{
	Check_Object(imagePool);

	if(unLoadedImages == false)
	{
		return lastHandle;
	}
//
// Statistic timing function
//
	for (unsigned i=0;i<MLRState::TextureMask+1;i++)
	{
		if(textureArray[i])
		{
			int hint;
			GOSImage *image = textureArray[i]->GetImage(&hint);
			if (image && !image->IsLoaded())
			{
				Check_Object(image);
				if (!imagePool->LoadImage(image, hint))
				{
					STOP(("Cannot load texture: %s!", textureArray[i]->textureName));
				}
			}
		}
	}
//
// End timing function
//

	unLoadedImages = false;

	return lastHandle;
}
