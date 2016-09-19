#define AANIMOBJECT_CPP
/*************************************************************************************************\
aAnimObject.cpp			: Implementation of the aAnimObject component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"aanimobject.h"
#include"mclib.h"

aAnimObject::aAnimObject(  )
{
}

//-------------------------------------------------------------------------------------------------

aAnimObject::~aAnimObject()
{
}

aAnimObject& aAnimObject::operator =( const aAnimObject& src )
{
	if ( &src != this )
	{
		aObject::operator =( src );
		animInfo = src.animInfo;
	}

	return *this;
}


int aAnimObject::init( FitIniFile* file, const char* blockName, DWORD neverFlush )
{
	aObject::init( file, blockName, neverFlush );

	long color = 0xffffffff;

	if ( NO_ERR == file->readIdLong( "Color", color ) )
	{
		setColor( color );
	}
	else
		setColor( 0xffffffff );

	char animName[256];
	file->readIdString( "Animation", animName, 255 );

	if ( NO_ERR == file->seekBlock( animName ) )
		animInfo.init( file, "" );

	animInfo.begin();

	return 0;
}

void aAnimObject::update()
{
	animInfo.update();
}
void aAnimObject::render( )
{
	if ( !isShowing() )
		return;
	long color = animInfo.getColor();


	float xNewOffset = animInfo.getXDelta()+.5f;
	float yNewOffset =	animInfo.getYDelta()+.5f;

	move( xNewOffset, yNewOffset );


	setColor( color );


	float fScaleX = animInfo.getScaleX();
	float fScaleY = animInfo.getScaleY();

	if ( fScaleX != 1.0 || fScaleY != 1.0 )
	{

		float oldWidth = width()+.5f;
		float oldHeight = height()+.5f;;
		float oldLeft = globalX();
		float oldTop = globalY();

		float scaleX = .5 * fScaleX * width();
		float scaleY = .5 * fScaleY * height();

		float midX = globalX() + .5 * width();
		float midY = globalY() + .5 * height();

		float newLeft = midX - scaleX;
		float newTop = midY - scaleY;



		moveToNoRecurse( newLeft, newTop );
		resize( fScaleX * width(), fScaleY * height() );
		aObject::render();
		resize( oldWidth, oldHeight );
		moveToNoRecurse( oldLeft, oldTop );
	}
	else
		aObject::render();

	move( -xNewOffset, -yNewOffset );
}

void aAnimObject::end()
{
	animInfo.end();
}





//*************************************************************************************************
// end of file ( aAnimObject.cpp )
