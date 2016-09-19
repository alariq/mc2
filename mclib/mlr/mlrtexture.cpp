//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//############################    MLRTexture    ###############################
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture::MLRTexture(MemoryStream *stream)
{
	Verify(gos_GetCurrentHeap() == Heap);
	Check_Pointer(this);
	Check_Object(stream);

	hint = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture::MLRTexture(
	MLRTexturePool *tp,
	const char* texName,
	int _instance,
	int handle,
	int _hint
)
{
	Verify(gos_GetCurrentHeap() == Heap);
	thePool = tp;
	textureHandle = handle;

	textureName = texName;

	textureNameHashValue = textureName.GetHashValue();

	instance = _instance;

	textureMatrix = AffineMatrix4D::Identity;
	textureMatrixIsIdentity = true;

	image = thePool->GetImage(textureName);

	hint = _hint;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture::MLRTexture(
	MLRTexturePool *tp,
	GOSImage *_image,
	int handle,
	int _hint
)
{
	Verify(gos_GetCurrentHeap() == Heap);
	thePool = tp;
	textureHandle = handle;

	image = _image;

	textureName = image->GetName();

	textureNameHashValue = textureName.GetHashValue();

	instance = 0;

	textureMatrix = AffineMatrix4D::Identity;
	textureMatrixIsIdentity = true;

	hint = _hint;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRTexture::TestInstance() const
{
	Verify((*thePool)[textureHandle]);
	Check_Object(image);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture::MLRTexture(const MLRTexture& tex)
{
	Check_Object(&tex);
	Verify(gos_GetCurrentHeap() == Heap);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture::~MLRTexture()
{
	if(NULL==thePool->GetImage(image->GetName()))
	{
		Unregister_Object(image);
		delete image;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRTexture*
	MLRTexture::Make(MemoryStream *stream)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	MLRTexture *texture = new MLRTexture(stream);
	gos_PopCurrentHeap();

	return texture;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRTexture::Save(MemoryStream *stream)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRTexture::GetImageNumber()
{
	Check_Object(this);
	return ((textureHandle-1) >> (thePool->GetInstanceDepth()));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MLRTexture::GetInstanceNumber()
{
	Check_Object(this);
	return ((textureHandle-1) & ~((1<<thePool->GetInstanceDepth())-1));
}

