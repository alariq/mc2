//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImagePool::GOSImagePool() :
	imageHash(
		4099,
		NULL,
		true
	)
{
	Verify(gos_GetCurrentHeap() == Heap);
	texturePath = "";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImagePool::~GOSImagePool()
{
	HashIteratorOf<GOSImage*, MString> images(&imageHash);
	images.DeletePlugs();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void GOSImagePool::UnLoadImages (void)
{
	HashIteratorOf<GOSImage*, MString> images(&imageHash);
	GOSImage *image = images.ReadAndNext();
	while (image)
	{
		RemoveImage(image);
		image = images.ReadAndNext();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImage*
	GOSImagePool::GetImage(const char* image_name)
{
	Check_Object(this);

	MString imageName = image_name;
	Verify(imageName.GetLength() > 0);

	//
	//---------------------------
	// Get the image for the name
	//---------------------------
	//
	GOSImage *image;

	if ((image = imageHash.Find(imageName)) == NULL)
	{
		gos_PushCurrentHeap(Heap);
		image = new GOSImage(image_name);
		Register_Object(image);
		gos_PopCurrentHeap();
		imageHash.AddValue(image, image->imageName);
	}
	Check_Object(image);

	return image;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImage*
	GOSImagePool::GetImage(const char *image_name, gos_TextureFormat format, int size, gos_TextureHints hints)
{
	Check_Object(this);

	MString imageName = image_name;
	Verify(imageName.GetLength() > 0);

	//
	//---------------------------
	// Get the image for the name
	//---------------------------
	//
	GOSImage *image;

	if ((image = imageHash.Find(imageName)) == NULL)
	{
		gos_PushCurrentHeap(Heap);
		image = new GOSImage(image_name, hints);
		Register_Object(image);
		gos_PopCurrentHeap();
		imageHash.AddValue(image, image->imageName);
	}
#ifdef _ARMOR
	else
	{
	}
#endif

	Check_Object(image);

	return image;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	GOSImagePool::RemoveImage(GOSImage *image)
{
	Check_Object(this);
	Unregister_Object(image);
	delete image;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
TGAFilePool::TGAFilePool(const char* path)
{
	texturePath = path;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	TGAFilePool::LoadImage(GOSImage *image, int hint)
{
	if( (image->flags & GOSImage::Loaded) != 0)
		return true;

	MString file_name = texturePath;
	file_name += image->imageName;
	file_name += ".tga";

	// sebi: ORIG BUG FIX: I really suspect that also fFileName should be changed to image->imageName!
	// but who cares about correct code in this game... not original developers for sure
	char *fFileName = image->imageName;
	if (((fFileName[0] != 'F') && (fFileName[0] != 'f')) ||
		((fFileName[1] != 'X') && (fFileName[1] != 'x')))
		hint |= gosHint_DisableMipmap;

	DWORD nodeIndex = mcTextureManager->loadTexture(file_name,gos_Texture_Detect, hint);

	image->SetHandle(nodeIndex);
	
	image->flags |= GOSImage::Loaded;

	return ((image->flags & GOSImage::Loaded) != 0);
}
