//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//############################    GOSImage    ###############################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImage::GOSImage( const char* iName ) : Plug (DefaultData)
{
	imageName = iName;

	flags = 0;

	instance = 0;

	mcTextureNodeIndex = 0xffffffff;

	ptr.pTexture = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImage::GOSImage( DWORD iHandle ) : Plug (DefaultData)
{
	char str[20];

	sprintf(str, "image%03d", iHandle);

	imageName = str;

	flags = Loaded;

	instance = 0;

	mcTextureNodeIndex = iHandle;

	ptr.pTexture = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImage::GOSImage(const char *name, gos_TextureHints hints) : Plug (DefaultData)
{
	imageName = name;

	flags = Loaded;

	instance = 0;

	ipHints = hints;

	mcTextureNodeIndex = mcTextureManager->loadTexture(name,gos_Texture_Detect,ipHints);

	ptr.pTexture = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
GOSImage::~GOSImage()
{
	imageName = "";

	if((flags & Locked) != 0)
	{
//		gos_UnLockTexture(imageHandle);
	}

	if((flags & Loaded) != 0)
	{
		if (mcTextureManager)
		{
			mcTextureManager->removeTexture(mcTextureNodeIndex);
			mcTextureNodeIndex = 0xffffffff;
		}
	}
	flags = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	GOSImage::GetWidth()
{
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	GOSImage::GetHeight()
{
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	GOSImage::LockImage()
{
	if(!(flags & Locked))
	{
		flags |= Locked;
		DWORD imageHandle = mcTextureManager->get_gosTextureHandle(mcTextureNodeIndex);

		if (imageHandle != 0xffffffff)
			gos_LockTexture(imageHandle, 0, false, &ptr);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	GOSImage::UnlockImage()
{
	if(flags & Locked)
	{
		flags &= ~Locked;
		Start_Timer(Unlock_Texture_Time);
		DWORD imageHandle = mcTextureManager->get_gosTextureHandle(mcTextureNodeIndex);

		if (imageHandle != 0xffffffff)
			gos_UnLockTexture(imageHandle);

		Stop_Timer(Unlock_Texture_Time);

		ptr.pTexture = NULL;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
unsigned char*
	GOSImage::GetImagePtr()
{
	return (unsigned char *)ptr.pTexture;
}
