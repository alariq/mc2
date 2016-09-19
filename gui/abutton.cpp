//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"abutton.h"
#include"afont.h"
#include"mclib.h"
#include<windows.h>
#include"soundsys.h"

extern SoundSystem *sndSystem;

aButton::aButton()
{
	toggleButton = 0;
	singlePress = 0;
	messageOnRelease = 0;
	state = ENABLED;
	memset( &data, 0, sizeof( data ) );

	clickSFX = LOG_CLICKONBUTTON;
	highlightSFX = LOG_HIGHLIGHTBUTTONS;
	disabledSFX = LOG_WRONGBUTTON;

	data.textAlign = 2;
	holdTime = .5f;
	
}

long aButton::init(long xPos, long yPos, long w, long h)
{
	long err;
	err = aObject::init(xPos,yPos,w,h);
	if (err)
		return err;

	return (NO_ERR);
}

void aButton::destroy()
{
	aObject::destroy();
}

aButton& aButton::operator=( const aButton& src)
{
	copyData( src );
	aObject::operator=( src );
	return *this;
}
aButton::aButton( const aButton& src ) : aObject( src )
{
	copyData( src );
}
void aButton::copyData( const aButton& src )
{
	if ( &src != this )
	{
		data = src.data;
		state = src.state;
	}
}


void aButton::update()
{
	if ( !isShowing() )
		return;

	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();


	if ( pointInside( mouseX, mouseY ) )
	{
		long mouseDragX = userInput->getMouseDragX();
		long mouseDragY = userInput->getMouseDragY();

		if ( messageOnRelease && userInput->leftMouseReleased()
			&& pointInside( mouseDragX, mouseDragY ) )
		{
			press( false );
			if ( getParent() )
				getParent()->handleMessage( aMSG_LEFTMOUSEDOWN, data.ID );

			if ( state != DISABLED && state != HIDDEN )
				sndSystem->playDigitalSample( clickSFX );
			else
				sndSystem->playDigitalSample( disabledSFX );

		}
		if (userInput->isLeftClick())
		{
			press( true );
			if ( getParent() && !messageOnRelease && pointInside(userInput->getMouseDragX(), userInput->getMouseDragY() ) )
				getParent()->handleMessage( aMSG_LEFTMOUSEDOWN, data.ID );

			if ( state != DISABLED )
				sndSystem->playDigitalSample( clickSFX );
			else
				sndSystem->playDigitalSample( disabledSFX );
		}
		else if ( userInput->getMouseLeftHeld() > holdTime && !messageOnRelease &&
			pointInside(userInput->getMouseDragX(), userInput->getMouseDragY()) )
			handleMessage( aMSG_LEFTMOUSEHELD, data.ID );
		else
		{
			if ( state != HIGHLIGHT && state != DISABLED && state != HIDDEN )
				sndSystem->playDigitalSample( highlightSFX );
			state = HIGHLIGHT;
			makeUVs( location, state, data );	
		}
	}
	else if ( state == PRESSED && messageOnRelease )
		state = ENABLED;

	aObject::update();
}

bool		aButton::pointInside(long xPos, long yPos) const
{
	if ( aObject::pointInside( xPos, yPos ) )
		return true;

	if ( data.textRect.left  <= xPos && 
		 data.textRect.right >= xPos && 
		 data.textRect.top <= yPos &&
		 data.textRect.bottom >= yPos )
	{
		return true;
	}

	return 0;
}

/////////////////////////////////////////////////
void aButton::render()
{
	if ( state != HIDDEN )
	{
		if ( textureHandle )
		{
			unsigned long gosID = mcTextureManager->get_gosTextureHandle( textureHandle );
			gos_SetRenderState( gos_State_Texture, gosID ); 
		}
		else
			gos_SetRenderState( gos_State_Texture, 0 ); 

			gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
			gos_SetRenderState( gos_State_Filter, gos_FilterNone);
			gos_SetRenderState( gos_State_AlphaTest, true);
			gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
			gos_SetRenderState( gos_State_TextureMapBlend,gos_BlendModulateAlpha );

			gos_DrawQuads( location, 4 );


		if ( data.textID && data.textFont )
		{
			char buffer[256];
			cLoadString( data.textID, buffer, 256 );
			DWORD width, height;
			gos_TextSetAttributes(data.textFont, data.textColors[state], data.textSize, true, true, false, false, data.textAlign);			
			gos_TextSetRegion( data.textRect.left, data.textRect.top, data.textRect.right, data.textRect.bottom );
			gos_TextStringLength( &width, &height, buffer );
			gos_TextSetPosition( data.textRect.left, (data.textRect.top + data.textRect.bottom)/2 - height/2 + 1 );
			gos_TextDraw( buffer );

			if ( data.outlineText )
			{
				drawEmptyRect( data.textRect, data.textColors[state], data.textColors[state] );
			}
		}
		if ( data.outline )
		{
			GUI_RECT tmp;
			tmp.left = location[0].x;
			tmp.top = location[0].y;
			tmp.right = location[2].x;
			tmp.bottom = location[2].y;

			drawEmptyRect( tmp, location[0].argb, location[0].argb );
		}

		for ( int i = 0; i < numberOfChildren(); i++ )
		{
			pChildren[i]->render();
		}
	}
}
void aButton::press(bool bPress)
{
	if ( !isEnabled() )
		return;

	if ( !bPress && state == HIGHLIGHT )
		return;

	state = bPress ? PRESSED : ENABLED;
	makeUVs( location, state, data );	
	
}

void aButton::makeAmbiguous( bool bAmbiguous )
{
	state = bAmbiguous ? AMBIGUOUS : ENABLED;
	makeUVs( location, state, data );	

}

void aButton::disable( bool bDisable )
{
	if ( !bDisable )
	{
		if ( state == DISABLED )
			state = ENABLED;
	}
	else
		state = DISABLED;

	makeUVs( location, state, data );
}

void aButton::hide( bool bHide )
{
	state = bHide ? HIDDEN : ENABLED;
	if ( state !=  HIDDEN )
		aButton::makeUVs( location, state, data );
}

bool aButton::isEnabled()
{
	return state == ENABLED || state == PRESSED || state == AMBIGUOUS || state == HIGHLIGHT;
}

int aButton::getID()
{
	return data.ID;
}

void aButton::setID( int newID )
{
	data.ID = newID;
}

void aButton::makeUVs( gos_VERTEX* vertices, int State, aButton::aButtonData& data )
{
		float left = data.stateCoords[State][0];
		float top = data.stateCoords[State][1];

		if ( left == -1 || top == -1 )
		{
			SPEW(( 0, "makeUVs given an Invalid state\n" ));
		}
		
		float width = data.textureWidth;
		float height = data.textureHeight;

		float right = left + width;
		float bottom = top + height;

		if ( data.fileWidth && data.fileHeight ) // will crash if 0
		{

			if ( data.textureRotated )
			{
				vertices[0].u = right/(float)data.fileWidth + (.1f / (float)data.fileWidth);;
				vertices[1].u = left/(float)data.fileWidth + (.1f / (float)data.fileWidth);;
				vertices[2].u = left/(float)data.fileWidth + (.1f / (float)data.fileWidth);
				vertices[3].u = right/(float)data.fileWidth + (.1f / (float)data.fileWidth);

				vertices[0].v = top/(float)data.fileHeight + (.1f / (float)data.fileWidth);;
				vertices[1].v = top/(float)data.fileHeight + (.1f / (float)data.fileWidth);;
				vertices[2].v = bottom/(float)data.fileHeight + (.1f / (float)data.fileHeight);;
				vertices[3].v = bottom/(float)data.fileHeight + (.1f / (float)data.fileHeight);;
			}
			else

			{
				{
					vertices[0].u = vertices[1].u =  left/(float)data.fileWidth + (.1f / (float)data.fileWidth);;
					vertices[2].u = vertices[3].u = right/(float)data.fileWidth + (.1f / (float)data.fileWidth);

					vertices[0].v = vertices[3].v = top/(float)data.fileHeight + (.1f / (float)data.fileWidth);;
					vertices[1].v = vertices[2].v = bottom/(float)data.fileHeight + (.1f / (float)data.fileHeight);
				}
			}
		}
}

void aButton::init( FitIniFile& buttonFile, const char* str, HGOSFONT3D font )
{
	textureHandle = 0;

	long result = buttonFile.seekBlock( str );
	if ( result != NO_ERR )
	{
		char errorStr[256];
		sprintf(  errorStr, "couldn't find button %s", str );
		Assert( 0, 0, errorStr );
		return;
	}


	buttonFile.readIdLong( "ID", data.ID );
	buttonFile.readIdString("FileName", data.fileName, 32 );

	buttonFile.readIdLong( "HelpCaption", helpHeader );
	buttonFile.readIdLong( "HelpDesc", helpID );
	buttonFile.readIdLong( "TextID", data.textID );
	buttonFile.readIdLong( "TextNormal", data.textColors[0] );
	buttonFile.readIdLong( "TextPressed", data.textColors[1] );
	buttonFile.readIdLong( "TextDisabled", data.textColors[2] );
	buttonFile.readIdBoolean( "Toggle", toggleButton );
	buttonFile.readIdBoolean( "outline", data.outline );
	long fontID;
	buttonFile.readIdLong( "Font", fontID );
	if ( fontID )
		data.textFont = aFont::loadFont( fontID, data.textSize );
	else
		data.textFont = 0;


	long x, y, width, height;

	buttonFile.readIdLong( "XLocation", x );
	buttonFile.readIdLong( "YLocation", y );
		
	buttonFile.readIdLong( "Width", width );
	buttonFile.readIdLong( "Height", height );

	buttonFile.readIdLong( "HelpCaption", helpHeader );
	buttonFile.readIdLong( "HelpDesc", helpID );

	buttonFile.readIdBoolean( "texturesRotated", data.textureRotated );

	if ( NO_ERR != buttonFile.readIdLong( "Alignment", data.textAlign ) )
		data.textAlign = 2;
	
	location[0].x = location[1].x = x;
	location[0].y = location[3].y = y;
	location[2].x = location[3].x = x + width;
	location[1].y = location[2].y = y + height;

	for ( int j = 0; j < 4; j++ )
	{
		location[j].argb = 0xffffffff;
		location[j].frgb = 0;
		location[j].rhw = .5;
		location[j].u = 0.f;
		location[j].v = 0.f;
		location[j].z = 0.f;
	}
		
	
	if ( 0 == textureHandle && data.fileName && strlen( data.fileName ) )
	{
		char file[256];
		strcpy( file, artPath );
		strcat( file, data.fileName );
		_strlwr( file );
		if ( !strstr( data.fileName, ".tga" ) )
			strcat( file, ".tga" );
		
		int ID = mcTextureManager->loadTexture( file, gos_Texture_Alpha, 0, 0, 0x2 );
		int gosID = mcTextureManager->get_gosTextureHandle( ID );
		TEXTUREPTR textureData;
		gos_LockTexture( gosID, 0, 0, 	&textureData );
		gos_UnLockTexture( gosID );

		textureHandle = ID;
		data.fileWidth = textureData.Width;
		data.fileHeight = data.fileWidth;
	}

	if ( NO_ERR != buttonFile.readIdLong( "UNormal", data.stateCoords[0][0] ) )
		data.stateCoords[0][0] = -1.f;

	if ( NO_ERR != buttonFile.readIdLong( "VNormal", data.stateCoords[0][1] ) )
		data.stateCoords[0][1] = -1.f;


	if ( NO_ERR != buttonFile.readIdLong( "UPressed", data.stateCoords[1][0] ) )
		data.stateCoords[1][0] = -1.f;

	if ( NO_ERR != buttonFile.readIdLong( "VPressed", data.stateCoords[1][1] ) )
		data.stateCoords[1][1] = -1.f;

	if ( NO_ERR != buttonFile.readIdLong( "UDisabled", data.stateCoords[2][0] ) )
		data.stateCoords[2][0] = -1.f;

	if ( NO_ERR != buttonFile.readIdLong( "VDisabled", data.stateCoords[2][1] ) )
		data.stateCoords[2][1] = -1.f;

	if ( NO_ERR != buttonFile.readIdLong( "UAmbiguous", data.stateCoords[3][0] ) )
		data.stateCoords[3][0] = -1.f;

	if ( NO_ERR != buttonFile.readIdLong( "VAmbiguous", data.stateCoords[3][1] ) )
		data.stateCoords[3][1] = -1.f;

	if ( NO_ERR != buttonFile.readIdLong( "UHighlight", data.stateCoords[4][0] ) )
	{
		data.stateCoords[4][0] = data.stateCoords[0][0];
	}

	if ( NO_ERR != buttonFile.readIdLong( "VHighlight", data.stateCoords[4][1] ) )
	{
		data.stateCoords[4][1] = data.stateCoords[0][1];
	}

	buttonFile.readIdLong( "UWidth", data.textureWidth );
	buttonFile.readIdLong( "VHeight", data.textureHeight );

	if ( data.textID )
		buttonFile.readIdBoolean( "TextOutline", data.outlineText );


	if ( NO_ERR == buttonFile.readIdLong( "XTextLocation", data.textRect.left ) )
	{
		buttonFile.readIdLong( "YTextLocation", data.textRect.top );
		buttonFile.readIdLong( "TextWidth", width );
		buttonFile.readIdLong( "TextHeight", height );
		data.textRect.right = data.textRect.left + width;
		data.textRect.bottom = data.textRect.top + height;
		buttonFile.readIdBoolean( "TextOutline", data.outlineText );
	}
	else
	{
		data.textRect.left = x;
		data.textRect.right = x + width;
		data.textRect.top = y;
		data.textRect.bottom = y + height;
	}

	char bmpName[256];
	strcpy( bmpName, str );
	strcat( bmpName, "Bmp" );
	char finalName[256];
	int counter = 0;
	while(true)
	{
		sprintf( finalName, "%s%ld", bmpName, counter );
		if ( NO_ERR != buttonFile.seekBlock( finalName) )
			break;

		aObject* pObject = new aObject;
		pObject->init( &buttonFile, finalName );
		// Dorje is doing this in global coords
		pObject->move( -globalX(), -globalY() );
		addChild( pObject );

		counter++;
	}


	buttonFile.seekBlock( str );
	disable( 0 );
	press( 0 );


}

aAnimButton::aAnimButton()
{
	animateText = 1;
	animateBmp = 1;
	bAnimateChildren = 1;

}

aAnimButton& aAnimButton::operator=( const aAnimButton& src)
{
	copyData( src );
	aButton::operator=( src );
	return *this;
}
aAnimButton::aAnimButton( const aAnimButton& src ) : aButton( src )
{
	copyData( src );
}

void aAnimButton::copyData( const aAnimButton& src )
{
	if ( &src != this )
	{
		animateBmp = src.animateBmp;
		animateText = src.animateText;
		highlightData = src.highlightData;
		disabledData = src.disabledData;
		pressedData = src.pressedData;
		normalData = src.normalData;
		toggleButton = src.toggleButton;
		bAnimateChildren = src.bAnimateChildren;

	}
}
void aAnimButton::destroy()
{
	normalData.destroy();
	highlightData.destroy();
	pressedData.destroy();
	disabledData.destroy();

	aButton::destroy();
}

void aAnimButton::init( FitIniFile& file, const char* headerName, HGOSFONT3D font )
{
	if ( NO_ERR != file.seekBlock( headerName ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't find block %s in file %s", headerName, file.getFilename() );
		Assert( 0, 0, errorStr );
		animateBmp = 0;
		animateText = 0;
		return;
	}
	aButton::init( file, headerName, font );
	normalData.init( &file, "Normal" );
	pressedData.init( &file, "Pressed" );
	highlightData.init( &file, "Highlight" );
	disabledData.init( &file, "Disabled" );
	normalData.begin();

	if ( NO_ERR != file.readIdBoolean( "AnimateBmp", animateBmp ) )
		animateBmp = 1;
	if ( NO_ERR != file.readIdBoolean( "AnimateText", animateText ) )
		animateText = 1;

	bool bTmp = 0;
	if ( NO_ERR == file.readIdBoolean( "AnimateChildren", bTmp  ) )
	{
		bAnimateChildren = bTmp;
	}
	
}

void aAnimButton::update()
{
	if ( !isShowing() )
		return;

	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();

	bool bInside = pointInside( mouseX, mouseY );

	if ( bInside && state == DISABLED )
	{
		::helpTextID = this->helpID;
		::helpTextHeaderID = this->helpHeader;
	}

	if ( bInside && state != DISABLED && state != HIDDEN )
	{
		::helpTextID = this->helpID;
		::helpTextHeaderID = this->helpHeader;
		
		if (userInput->isLeftClick())
		{
			if ( toggleButton )
			{
				if ( state == PRESSED )
				{
					press( 0 );
					pressedData.end();
				}
				else
				{
					pressedData.begin();
					highlightData.end();
					press( 1 );
				}	
				
				sndSystem->playDigitalSample( clickSFX );
			
			}
			else
			{
				press( true );
				pressedData.begin();
				highlightData.end();

				sndSystem->playDigitalSample( clickSFX );
			}

			if ( getParent() && !messageOnRelease && pointInside(userInput->getMouseDragX(), userInput->getMouseDragY() ) )
				getParent()->handleMessage( aMSG_LEFTMOUSEDOWN, data.ID );			
	
		}
		else if (userInput->getMouseLeftHeld() > holdTime && getParent() && !messageOnRelease
			&& pointInside(userInput->getMouseDragX(), userInput->getMouseDragY()) )
			getParent()->handleMessage( aMSG_LEFTMOUSEHELD, data.ID );
		else if ( userInput->leftMouseReleased() && getParent() && messageOnRelease )
		{
			long mouseDragX = userInput->getMouseDragX();
			long mouseDragY = userInput->getMouseDragY();

			if ( pointInside( mouseDragX, mouseDragY ) )
			{

				getParent()->handleMessage( aMSG_LEFTMOUSEDOWN, data.ID );
				if ( !toggleButton )
					state = ENABLED;
			}

	//		sndSystem->playDigitalSample( clickSFX );
		}
		else if ( state != PRESSED )
		{
			if ( state != HIGHLIGHT && state != DISABLED && state != HIDDEN && isShowing() )
				sndSystem->playDigitalSample( highlightSFX );

			if ( !highlightData.isAnimating() )
				highlightData.begin();

			state = HIGHLIGHT;
			makeUVs( location, state, data );	
		}
		
	}
	else if ( state == PRESSED   )
	{
		// if clicked inside and release outside
		if ( userInput->leftMouseReleased() && ( messageOnRelease || singlePress )
			&& pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() ) )
		{
			state = ENABLED;
		}
		else
		{
			if ( !pressedData.isAnimating() )
				pressedData.begin();
			if ( singlePress && pressedData.isDone() )
				press( 0 );
		}
	}
	else if ( state == DISABLED )
	{
		if ( pointInside( mouseX, mouseY )  && userInput->isLeftClick() )
		{
			sndSystem->playDigitalSample( disabledSFX );
		}
		if ( !disabledData.isAnimating() )
			disabledData.begin();
		disabledData.update();
	}
	
	else
	{
		highlightData.end();
		if ( state == HIGHLIGHT )
			state = ENABLED;
		press( 0 );
	}

	if ( pressedData.isAnimating() && state != PRESSED )
	{
		pressedData.end();
		press( 0 );
	}

	if ( state != DISABLED )
		disabledData.end();

	highlightData.update();
	pressedData.update();
	normalData.update();


}
void aAnimButton::render()
{
	if ( !isShowing() )
		return;

	if ( disabledData.isAnimating() )
	{
		update( disabledData );
	}
	else if ( pressedData.isAnimating() )
	{
		update( pressedData );
	}	
	else if ( highlightData.isAnimating() )
	{
		update( highlightData );
	}
	else 
	{
		update( normalData );
	}


}

void aAnimButton::update( const aAnimation& animData )
{
	if ( !isShowing() )
		return;
	long color = animData.getColor();
	if ( animateBmp )
		setColor( color, 0 );
	if ( animateText )
		data.textColors[state] = color;

	long xPos = animData.getXDelta();
	long yPos = animData.getYDelta();

	move( xPos, yPos );

	if ( bAnimateChildren )
	{
		for ( int i = 0; i < numberOfChildren(); i++ )
			pChildren[i]->setColor( color );
	}
	
	float fXcaleX = animData.getScaleX();
	float fScaleY = animData.getScaleX();

	if ( fXcaleX != 1.0 && fScaleY != 1.0 )
	{

		float oldWidth = width();
		float oldHeight = height();
		float oldLeft = globalX();
		float oldTop = globalY();

		float scaleX = .5 * fXcaleX * width();
		float scaleY = .5 * fScaleY * height();

		float midX = globalX() + .5 * width();
		float midY = globalY() + .5 * height();

		float newLeft = midX - scaleX;
		float newTop = midY - scaleY;



		moveToNoRecurse( newLeft, newTop );
		resize( width() * scaleX, height() * scaleY );
		aButton::render();
		resize( oldWidth, oldHeight );
		moveToNoRecurse( oldLeft, oldTop );
	}
	else
		aButton::render();



}

void aButton::move( float offsetX, float offsetY )
{
	aObject::move( offsetX, offsetY );
	
	data.textRect.left += offsetX;
	data.textRect.right += offsetX;
	data.textRect.top += offsetY;
	data.textRect.bottom += offsetY;
}

void aAnimButton::setAnimationInfo( aAnimation* normal, aAnimation* highlight,
								  aAnimation* pressed, aAnimation* disabled )
{
	if ( normal )
	{
		normalData.destroy();
		normalData = *normal;
	}
	else
		normalData.destroy();

	if ( highlight )
	{
		highlightData.destroy();
		highlightData = *highlight;
	}
	else
		highlightData.destroy();

	if ( pressed )
	{
		pressedData.destroy();
		pressedData = *pressed;
	}
	else
		pressedData.destroy();

	if ( disabled )
	{
		disabledData.destroy();
		disabledData = *disabled;
	}
	else
		disabledData.destroy();
}






