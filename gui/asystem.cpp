//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mclib.h"
#include"asystem.h"
#include"packet.h"
#include"afont.h"
#include"paths.h"
#include"userinput.h"


long helpTextID = 0;
long helpTextHeaderID = 0;



void aObject::update()
{
	long x = userInput->getMouseX();
	long y = userInput->getMouseY();

	if ( pointInside( x, y ) && helpID && isShowing())
	{
		helpTextID = helpID;
	}
	// call update for the children
	for ( int i = 0; i < pNumberOfChildren; i++ )
		pChildren[i]->update();
}


// base class aObject methods

aObject::aObject()
{
	pNumberOfChildren = 0;
	pParent = NULL;
	textureHandle = 0;
	memset( location, 0, sizeof ( gos_VERTEX ) * 4 );
	for ( int i = 0; i < 4;i++ )
		location[i].rhw = .5;
	showWindow = 1;
	helpID = 0;
	fileWidth = 0.0f;
}

aObject::~aObject()
{
	destroy();	//	destroy will often have been called already, so better be sure
				//	it's safe to call twice
}

long aObject::init(long xPos, long yPos,long w, long h)
{

	location[0].x = xPos;
	location[0].y = yPos;
	location[1].x = xPos;
	location[1].y = yPos + h;
	location[2].x = xPos + w;
	location[2].y = yPos + h;
	location[3].x = xPos + w;
	location[3].y = yPos;

	for ( int i = 0; i < 4; i++ )
	{
		location[i].u = 0.f;
		location[i].v = 0.f;
		location[i].z = 0.f;
		location[i].rhw = 0.5f;
		location[i].argb = 0xff000000;
		location[i].frgb = 0;
	}

	showWindow = TRUE;

	pNumberOfChildren = 0;
	pParent = NULL;
	return (NO_ERR);
}

void aObject::init(FitIniFile* file, const char* blockName, DWORD neverFlush)
{
	memset( location, 0, sizeof( location ) );
	char fileName[256];
	textureHandle = 0;
	fileWidth = 256.; 
	
	if ( NO_ERR != file->seekBlock( blockName ) )
	{
		char errBuffer[256];
		sprintf( errBuffer, "couldn't find static block %s", blockName );
		Assert( 0, 0, errBuffer );
		return;
	}

	long x, y, width, height;
	file->readIdLong( "XLocation", x );
	file->readIdLong( "YLocation", y );

	file->readIdLong( "Width", width );
	file->readIdLong( "Height", height );

	file->readIdLong( "HelpCaption", helpHeader );
	file->readIdLong( "HelpDesc", helpID );
	

	if ( NO_ERR == file->readIdString( "fileName", fileName, 32 ) )
	{

		bool bAlpha = 0;
		file->readIdBoolean( "Alpha", bAlpha );


		if ( !textureHandle )
		{
			char buffer[256];
			strcpy( buffer, artPath );
			strcat( buffer, fileName );
			_strlwr( buffer );
			if ( !strstr( buffer, ".tga" ) )
				strcat( buffer, ".tga" );
			int ID = mcTextureManager->loadTexture( buffer, bAlpha ? gos_Texture_Alpha : gos_Texture_Keyed, 0, 0, 0x2);
			textureHandle = ID;
			unsigned long gosID = mcTextureManager->get_gosTextureHandle( ID );
			TEXTUREPTR textureData;
			gos_LockTexture( gosID, 0, 0, 	&textureData );
			fileWidth = textureData.Width;
			gos_UnLockTexture( gosID );
		}
	}

	long u, v, uWidth, vHeight;
	bool bRotated = 0;

	file->readIdLong( "UNormal", u );
	file->readIdLong( "VNormal", v );
	file->readIdLong( "UWidth", uWidth );
	file->readIdLong( "VHeight", vHeight );
	file->readIdBoolean( "texturesRotated", bRotated );

	for ( int k = 0; k < 4; k++ )
	{
		location[k].argb = 0xffffffff;
		location[k].frgb = 0;
		location[k].x = x;
		location[k].y = y;
		location[k].z = 0.f;
		location[k].rhw = .5;
		if ( fileWidth )
			location[k].u = (float)u/(float)fileWidth + (.1f / (float)fileWidth);
		if ( fileWidth )
			location[k].v = (float)v/(float)fileWidth + (.1f / (float)fileWidth);
	}

	location[3].x = location[2].x = x + width;
	location[2].y = location[1].y = y + height;

	if ( fileWidth )
		location[2].u = location[3].u = ((float)(u + uWidth))/((float)fileWidth) + (.1f / (float)fileWidth);
	if ( fileWidth )
	location[1].v = location[2].v = ((float)(v + vHeight))/((float)fileWidth) + (.1f / (float)fileWidth);

	if ( bRotated )
	{

		location[0].u = (u + uWidth)/(float)fileWidth + (.1f / (float)fileWidth);;
		location[1].u = u/(float)fileWidth + (.1f / (float)fileWidth);;
		location[2].u = u/(float)fileWidth + (.1f / (float)fileWidth);
		location[3].u = (u + uWidth)/(float)fileWidth + (.1f / (float)fileWidth);

		location[0].v = v/(float)fileWidth + (.1f / (float)fileWidth);;
		location[1].v = v/(float)fileWidth + (.1f / (float)fileWidth);;
		location[2].v = (v + vHeight)/(float)fileWidth + (.1f / (float)fileWidth);;
		location[3].v = (v + vHeight)/(float)fileWidth + (.1f / (float)fileWidth);;
	}

}

void aObject::destroy()
{

	removeAllChildren();

	if ( textureHandle )
	{
		int gosID = mcTextureManager->get_gosTextureHandle( textureHandle );
		if ( gosID && gosID != -1 )
			mcTextureManager->removeTexture( gosID );
	}


	if (pParent)
	{
		pParent->removeChild(this);
	}
	pParent = NULL;
	
}



bool aObject::pointInside(long mouseX, long mouseY) const
{
	if ( (location[0].x)  <= mouseX && 
		 location[3].x >= mouseX && 
		 location[0].y <= mouseY &&
		 location[1].y >= mouseY )
		 return true;

	return false;
}

bool aObject::rectIntersect(long left, long top, long right, long bottom) const
{
	if ((right > location[0].x) && (left < location[2].x) && (bottom > location[0].y) && (top < location[2].y))
		return (TRUE);
	else
		return (FALSE);
}

bool aObject::rectIntersect(const GUI_RECT& testRect) const
{
	if ((testRect.right > location[0].x) && (testRect.left < location[0].y) &&
		(testRect.bottom > location[2].y) && (testRect.top < location[2].y))
		return (TRUE);
	else
		return (FALSE);
}


aObject* aObject::findObject(long xPos, long yPos)
{
	aObject* target;

	if (showWindow)
	{
		for (long i = pNumberOfChildren; i > 0; i--)
		{
			target = pChildren[i-1]->findObject(xPos,yPos);
			if (target)
				return target;
		}
	}

	if (showWindow && pointInside(xPos,yPos))
		return (this);

	return NULL;
}


void aObject::setParent(aObject* p)
{
	pParent = p;
}



long aObject::numberOfChildren() const
{
	return pNumberOfChildren;
}

void aObject::addChild(aObject* c)
{
	Assert (pNumberOfChildren < MAX_CHILDREN, pNumberOfChildren+1, "Too many children!");
	Assert(c->getParent() == NULL || c->getParent() == this, 0, " Adding child that's someone else's ");
	if (!c)
		return;
	removeChild(c);	//	make sure this isn't already my child (Duplicate children screws up bringToFront())
	c->setParent(this);
	pChildren[pNumberOfChildren] = c;
	pNumberOfChildren++;

	c->move( x(), y() );
}

void aObject::removeChild(aObject* c)
{
	if (!c)			//If this is NULL, shouldn't we still remove it from the list?
		return;
			
	if ((c->getParent() == this) || (c->getParent() == NULL))	//Normal situation
	{
		for (long cc = 0; cc < pNumberOfChildren; cc++)
		{
			if (pChildren[cc] == c)
			{
				// found the child, remove it and shift the rest of the children up
				for (long sc = cc; sc < pNumberOfChildren - 1; sc++)
					pChildren[sc] = pChildren[sc+1];
				pChildren[pNumberOfChildren] = NULL;
				pNumberOfChildren--;
				c->setParent(NULL);
				return;
			}
		}
	}
	else
	{
		gosASSERT( 0 );
	}
}



aObject* aObject::child(long w)
{
	if (w > pNumberOfChildren - 1)
		return NULL;

	return pChildren[w];
}

float aObject::width() const
{
	return location[2].x - location[0].x;
}

float aObject::height() const
{
	return location[2].y - location[0].y;
}


float aObject::x() const
{
	if ( pParent )
		return location[0].x - pParent->globalX();
	else
		return location[0].x;
}

float aObject::y() const
{
	if ( pParent )
		return location[0].y - pParent->y();
	else
		return location[0].y;
}

long aObject::globalX() const
{
	return location[0].x;
}

long aObject::globalY() const
{
	return location[0].y;
}

long aObject::globalRight() const
{
	return globalX() + (long)width();
}

long aObject::globalBottom() const
{
	return globalY() + (long)height();
}

void aObject::moveTo(long xPos, long yPos )
{
	
	float offsetX = xPos - location[0].x;
	float offsetY = yPos - location[0].y;
	
	move( offsetX, offsetY );

}

void aObject::moveToNoRecurse(long xPos, long yPos )
{
	
	float offsetX = xPos - location[0].x;
	float offsetY = yPos - location[0].y;
	
	moveNoRecurse( offsetX, offsetY );

}


void aObject::move( float offsetX, float offsetY )
{
	for ( int i = 0; i < 4; i++ )
	{
		location[i].x += offsetX;
		location[i].y += offsetY;
	}

	for (int i = 0; i < pNumberOfChildren; i++)
	{
		pChildren[i]->move( offsetX, offsetY );
	}
}

void aObject::moveNoRecurse( float offsetX, float offsetY )
{
	for ( int i = 0; i < 4; i++ )
	{
		location[i].x += offsetX;
		location[i].y += offsetY;
	}
}
void aObject::resize(long w, long h)
{
	location[2].x = location[0].x + w;
	location[3].x = location[0].x + w;
	location[1].y = location[0].y + h;
	location[2].y = location[0].y + h;
}

void aObject::render()
{
	if ( showWindow )
	{
		unsigned long gosID = mcTextureManager->get_gosTextureHandle( textureHandle );	
		gos_SetRenderState( gos_State_Texture, gosID );
		gos_SetRenderState(gos_State_Filter, gos_FilterNone);
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );
		gos_SetRenderState( gos_State_ZCompare, 0 );
		gos_SetRenderState( gos_State_ZWrite, 0 );


		gos_DrawQuads( location, 4 );

		for ( int i = 0; i < pNumberOfChildren; i++ )
		{
			pChildren[i]->render();
		}
	}
}

void	aObject::render(long x, long y)
{
	move( x, y );
	render();
	move( -x, -y );
}
	
void	aObject::setTexture( const char* fileName )
{
	
	if ( textureHandle )
	{
		int gosID = mcTextureManager->get_gosTextureHandle( textureHandle );
		if ( gosID > 0 )	
			mcTextureManager->removeTexture( gosID );
	}
	textureHandle = mcTextureManager->loadTexture( fileName, gos_Texture_Keyed, 0, 0, 0x2);
	int gosID = mcTextureManager->get_gosTextureHandle( textureHandle );
	if ( gosID )
	{
		TEXTUREPTR textureData;
		gos_LockTexture( gosID, 0, 0, 	&textureData );
		fileWidth = textureData.Width;
		gos_UnLockTexture( gosID );
	}
	else
		fileWidth = 256; // guess

}

void	aObject::setTexture(unsigned long newHandle )
{
	//Gotta check handle.  If its the same as the new one,
	// We don't gotta delete the old one.  The texture manager already did!!
	if ( textureHandle && textureHandle != newHandle)
	{
		int gosID = mcTextureManager->get_gosTextureHandle( textureHandle );
		if ( gosID > 0 )
			mcTextureManager->removeTexture( gosID );
	}
	
	textureHandle = newHandle;
	
	if ( newHandle )
	{
		int gosID = mcTextureManager->get_gosTextureHandle( newHandle );
		TEXTUREPTR textureData;
		gos_LockTexture( gosID, 0, 0, 	&textureData );
		fileWidth = textureData.Width;
		gos_UnLockTexture( gosID );
	}

	

}


void aObject::setColor( unsigned long newColor, bool bRecurse )
{
	for ( int i = 0; i < 4; i++ )
	{
		location[i].argb = newColor;
	}

	//set the kids?
	if ( bRecurse )
	{
		for ( int i = 0; i < this->pNumberOfChildren; i++ )
		{
			pChildren[i]->setColor( newColor, 1 );
		}
	}
}

void	aObject::setUVs( float u1, float v1, float u2, float v2 )
{
	location[0].u = location[1].u = u1/fileWidth + (.1f / (float)fileWidth);
	location[2].u = location[3].u = u2/fileWidth + (.1f / (float)fileWidth);
	location[0].v = location[3].v = v1/fileWidth + (.1f / (float)fileWidth);
	location[1].v = location[2].v = v2/fileWidth + (.1f / (float)fileWidth);

	
}

void aObject::removeAllChildren( bool bDelete)
{
	for ( int i = 0; i < pNumberOfChildren; i++ )
	{
		pChildren[i]->setParent( 0 );
		if ( bDelete )
			delete pChildren[i];
	}
	memset( pChildren, 0, sizeof( aObject*)* MAX_CHILDREN );

	pNumberOfChildren = 0;
}

void aObject::copyData( const aObject& src )
{
	if ( &src != this )
	{
		if ( src.textureHandle )
			textureHandle = mcTextureManager->copyTexture( src.textureHandle );

		for ( int i = 0; i < 4;i++ )
			location[i] = src.location[i];
		
		fileWidth = src.fileWidth;
		showWindow = src.showWindow;

	
		pNumberOfChildren = 0; // not copying the kids.
		ID = src.ID;
	}


}

aObject::aObject( const aObject& src )
{
	copyData( src );
}
aObject& aObject::operator=( const aObject& src )
{
	if ( &src != this )
		copyData( src );

	return *this;
}
	

/////////////////////////////////////////////////////////////


aRect::aRect()
{
	bOutline = 0;
}

void aRect::render()
{
	long color = getColor();
	if ( isShowing() )
		//bOutline ? drawEmptyRect( getGUI_RECT(), color, color ) : drawRect( getGUI_RECT(), color );
		bOutline ? drawEmptyRect( getGlobalGUI_RECT(), color, color ) : drawRect( getGlobalGUI_RECT(), color );
}

void aRect::render( long x, long y )
{
	//GUI_RECT tmpRect = getGUI_RECT();
	GUI_RECT tmpRect = getGlobalGUI_RECT();
	tmpRect.left += x;
	tmpRect.right += x;
	tmpRect.top += y;
	tmpRect.bottom += y;

	long color = getColor();
	bOutline ? drawEmptyRect( tmpRect, color, color ) : drawRect( tmpRect, color );

}

void aRect::init( FitIniFile* file, const char* blockName )
{
	if ( NO_ERR != file->seekBlock( blockName ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't find block %s in file %s", blockName, file->getFilename() );
		Assert( 0, 0, errorStr );
	}


	long left;
	if ( NO_ERR == file->readIdLong( "left", 	left ) )
	{
		long right, top, bottom;
		file->readIdLong( "top", 	top );
		file->readIdLong( "right", 	right );
		file->readIdLong( "bottom", 	bottom );
		aObject::init(left, top, right - left, bottom - top);
	}
	else
	{
		//aObject::init(file, blockName);
		/*we're not using */
		long x, y, width, height;
		file->readIdLong( "XLocation", 	x );
		file->readIdLong( "YLocation", 	y );
		file->readIdLong( "Width", 	width );
		file->readIdLong( "Height", height );
		aObject::init(x, y, width, height);
	}

	long color = 0xff000000;
	file->readIdLong( "color", color );
	setColor(color);
	file->readIdBoolean( "outline", bOutline );

	file->readIdLong( "HelpCaption", helpHeader );
	file->readIdLong( "HelpDesc", helpID );
}

GUI_RECT aRect::getGUI_RECT()
{
	GUI_RECT rect;
	rect.left = left();
	rect.right = right();
	rect.top = top();
	rect.bottom = bottom();

	if (pParent)
	{
		/* if there is a parent then we have to translate from relative to absolute coordinates */
		rect.left += pParent->x();
		rect.right += pParent->x();
		rect.top += pParent->top();
		rect.bottom += pParent->top();
	}
	return rect;
}

GUI_RECT aRect::getGlobalGUI_RECT()
{
	GUI_RECT rect;
	rect.left = globalLeft();
	rect.right = globalRight();
	rect.top = globalTop();
	rect.bottom = globalBottom();
	return rect;
}

	
//////////////////////////////////////////////////////////

aText::aText()
{
	alignment = 0;
}

aText::~aText()
{
}

void aText::init( FitIniFile* file, const char* header )
{
	int result = file->seekBlock( header );
	
	if ( result != NO_ERR )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't find the text block%s", header );
		Assert( result == NO_ERR, 0, errorStr );
		return;
	}

	long lfont;
	file->readIdLong( "Font", lfont );
	font.init( lfont );

	long left, top, width, height; 

	file->readIdLong( "XLocation", left );
	file->readIdLong( "YLocation", top );
	file->readIdLong( "Width", width );
	file->readIdLong( "Height", height );
	
	aObject::init( left, top, width, height );
	
	long color;
	file->readIdLong( "Color", color );
	for ( int i = 0; i < 4; i++ )
		location[i].argb = color;

	

	file->readIdLong( "Alignment", alignment );

	long textID;
	if ( NO_ERR == file->readIdLong( "TextID", textID ) )
	{
		//WAY too small.  Good crash.  Only crashes in profile.
		// cLoadString now checks buffer length and keeps game from crashing!!
		// -fs
		char tmp[1024];
		cLoadString( textID, tmp, 1023 );
		text = tmp;
	}

	file->readIdLong( "HelpCaption", helpHeader );
	file->readIdLong( "HelpDesc", helpID );
}

void aText::setText(const EString& str)
{
	text = str;
}

void aText::render()
{
	if ( showWindow && text.Length()>0)
		font.render( text, location[0].x, location[0].y, location[2].x - location[0].x, 
		location[2].y - location[0].y, location[0].argb, 0, alignment );
}

void aText::render( long x, long y )
{
	move( x, y );
	render();
	move( -x, -y );
}

void aText::setText( long resID )
{
	char tmp[1280];
	cLoadString( resID, tmp, 1279 );
	if (tmp[0] != 0)
		text = tmp;
	else
	{
		char tmpy[1280];
		memset(tmpy,0,1280);
		sprintf( tmpy,"NULL for ID: %d",resID );
		text = tmpy;
	}
}

aText::aText( const aText& src )
{
	CopyData( src );
}

aText& aText::operator=( const aText& src )
{
	CopyData( src );
	return *this;
}

void aText::CopyData( const aText& src )
{
	if ( &src != this )
	{
		text = src.text;
		alignment = src.alignment;
		font = src.font;
		aObject::operator=( src );
	}
}

bool aText::pointInside(long xPos, long yPos) const
{
	if ( !width() || !height() )
	{
		long left = location[0].x;
		long top = location[0].y;
		long width = font.width( text );
		long height = font.height( );

		if ( alignment == 1 ) // right aligned
		{
			left -=  width;
		}

		long mouseX = userInput->getMouseX();;
		long mouseY = userInput->getMouseY();

		if (  left  <= mouseX && 
		 left + width >= mouseX && 
		 top <= mouseY &&
		 top + height >= mouseY )
		 return true;
	
	}

	return aObject::pointInside( xPos, yPos );


}



