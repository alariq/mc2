#define LOGISTICSSCREEN_CPP
/*************************************************************************************************\
LogisticsScreen.cpp			: Implementation of the LogisticsScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================// 
\*************************************************************************************************/

#include"logisticsscreen.h"
#include"inifile.h"
#include"asystem.h"
#include"abutton.h"
#include"aedit.h"
#include"err.h"
#include"aanimobject.h"

extern long helpTextID;
extern long helpTextHeaderID;
extern float frameLength;


LogisticsScreen::LogisticsScreen()
{
	statics = 0;
	rects = 0;
	buttons = 0;
	edits = 0;
	textObjects = 0;
	animObjects = 0;	
	staticCount = rectCount = buttonCount = textCount = editCount = animObjectsCount = 0;

	helpTextArrayID = -1;
	
	
	fadeInTime = fadeOutTime= fadeTime = 0;
	fadeOutMaxColor = 0xff000000;

	
}

//-------------------------------------------------------------------------------------------------

LogisticsScreen::~LogisticsScreen()
{
	destroy();
}

void LogisticsScreen::destroy()
{
	clear();	

}

void	LogisticsScreen::clear()
{
	if ( statics )
		delete [] statics;

	if ( rects )
		delete [] rects;

	if ( buttons )
		delete  [] buttons;

	if ( edits )
		delete [] edits;

	if ( textObjects )
		delete[] textObjects;

	if ( animObjects )
		delete [] animObjects;

	statics = 0;
	rects = 0;
	buttons = 0;
	edits = 0;
	textObjects = 0;
	animObjects = 0;

	staticCount = 0;
	rectCount = 0;
	buttonCount = 0;
	editCount= 0;
	textCount = 0;
	animObjectsCount= 0;
}



//-------------------------------------------------------------------------------------------------

void LogisticsScreen::init( FitIniFile& file, const char* staticName, const char* textName, const char* rectName,
					  const char* buttonName, const char* editName, const char* animObjectName, DWORD neverFlush )
{
	clear();
	
	char blockName[256];

	// init statics
	if ( staticName )
	{
		sprintf( blockName, "%s%c", staticName, 's' );
		if ( NO_ERR == file.seekBlock( blockName ) )
		{
			file.readIdLong( "staticCount", staticCount );

			if ( staticCount )
			{
				statics = new aObject[staticCount];

				char blockName[128];
				for ( int i = 0; i < staticCount; i++ )
				{
					sprintf( blockName, "%s%ld", staticName, i );
					statics[i].init( &file, blockName );			
				}
				
			}
		}
	}

	if ( rectName )
	{
		// init rects
		sprintf( blockName, "%s%c", rectName, 's' );
		if ( NO_ERR == file.seekBlock( blockName ) )
		{
			file.readIdLong( "rectCount", rectCount );
			if ( rectCount )
			{
				rects = new aRect[rectCount];

				char blockName[128];
				for ( int i = 0; i < rectCount; i++ )
				{
					sprintf( blockName, "%s%ld", rectName, i );
					rects[i].init( &file, blockName );
				}
			}
		}
	}

	
	// init buttons
	if ( buttonName )
	{
		sprintf( blockName, "%s%c", buttonName, 's' );
		if ( NO_ERR == file.seekBlock( blockName ) )
		{
			file.readIdLong( "buttonCount", buttonCount );

			if ( buttonCount )
			{
				char blockName[128];
				buttons = new aAnimButton[buttonCount];
				for ( int i = 0; i < buttonCount; i++ )
				{
					sprintf( blockName,"%s%ld", buttonName, i );
					buttons[i].init( file, blockName );
					addChild( &buttons[i] );
				}
			}
		
		}
	}

	// init texts
	if ( textName )
	{
		sprintf( blockName, "%s%c", textName, 's' );
		if ( NO_ERR == file.seekBlock( blockName ) )
		{
			if ( NO_ERR != file.readIdLong( "TextEntryCount", textCount ) )
				file.readIdLong( "TextCount", textCount );

			if ( textCount )
			{
				textObjects = new aText[textCount];
				char blockName[64];
				for ( int i = 0; i < textCount; i++ )
				{
					sprintf( blockName, "%s%ld", textName, i );
					textObjects[i].init( &file, blockName );
				}
				
			}
		}
	}

	if ( editName )
	{
		sprintf( blockName, "%s%c", editName, 's' );
		if ( NO_ERR == file.seekBlock( blockName ) )
		{
			if ( NO_ERR != file.readIdLong( "EditCount", editCount ) )
				file.readIdLong( "EditCount", editCount );

			if ( editCount )
			{
				edits = new aEdit[editCount];
				char blockName[64];
				for ( int i = 0; i < editCount; i++ )
				{
					sprintf( blockName, "%s%ld", editName, i );
					edits[i].init( &file, blockName );
				}
				
			}
		}
	}

	if ( animObjectName )
	{
		sprintf( blockName, "%s%c", animObjectName, 's' );
		if ( NO_ERR == file.seekBlock( blockName ) )
		{
			file.readIdLong( "Count", animObjectsCount );

			if ( animObjectsCount )
			{
				animObjects = new aAnimObject[animObjectsCount];
				char blockName[64];
				for ( int i = 0; i < animObjectsCount; i++ )
				{
					sprintf( blockName, "%s%ld", animObjectName, i );
					animObjects[i].init( &file, blockName, neverFlush );
				}
				
			}
		}
	}
}

aButton* LogisticsScreen::getButton( long who )
{
	for ( int i = 0; i < buttonCount; i++ )
	{
		if ( buttons[i].getID() == who )
		{
			return &buttons[i];
		}
	}

	return NULL;
}

aRect* LogisticsScreen::getRect( long who )
{
	if ((who >= 0) && (who < rectCount))
	{
		return &rects[who];
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------
void LogisticsScreen::update()
{
	for ( int i = 0; i < staticCount; i++ )
	{
		statics[i].update();
	}

	for (int i = 0; i < buttonCount; i++ )
	{
		buttons[i].update();
	}

	for (int i = 0; i < textCount; i++ )
		textObjects[i].update();

	for (int i = 0; i < rectCount; i++ )
		rects[i].update();

	// help text
	if ( helpTextArrayID != -1 )
	{
		if ( ::helpTextID )
		{

			EString helpText;
			char tmp[1024];
		//	if ( helpTextHeaderID )
		//	{
		//		cLoadString( helpTextHeaderID, tmp, 255 );
		//		helpText = tmp;
		//		helpText.MakeUpper();
		//		helpText += '\n';
		//	}

			cLoadString( helpTextID, tmp, 1024 );
			helpText = tmp;
			textObjects[helpTextArrayID].setText( helpText );
		}
		else
			textObjects[helpTextArrayID].setText( "" );
	}

	for (int i = 0; i < editCount; i++ )
	{ 
		edits[i].update();
	}

	for (int i = 0; i < animObjectsCount; i++ )
		animObjects[i].update();

//	if ( gos_GetKeyStatus( KEY_RETURN ) == KEY_RELEASED )
//	{
//		if ( getButton( 50 /*MB_MSG_NEXT*/ ) )
//		{
//			if ( getButton(50 )->isEnabled() )
//				handleMessage( aMSG_LEFTMOUSEDOWN, 50 );
//		}
//	}
//	if ( gos_GetKeyStatus( KEY_ESCAPE ) == KEY_RELEASED )
//	{
//		if ( getButton( 57 /*MB_MSG_MAINMENU*/ ) )
//		{
//			if ( getButton(57 )->isEnabled() )
//				handleMessage( aMSG_LEFTMOUSEDOWN, 57 );
//		}

//	}

	helpTextID = 0;


}


//-------------------------------------------------------------------------------------------------
void LogisticsScreen::render()
{
	if ( !isShowing() )
		return;
	for (int i = 0; i < rectCount; i++ )
	{
		if ( !rects[i].bOutline && 
			( (rects[i].getColor() & 0xff000000) == 0xff000000 ) )
			rects[i].render();
	}


	for (int i = 0; i < staticCount; i++ )
		statics[i].render();

	for (int i = 0; i < rectCount; i++ )
	{
		if ( rects[i].bOutline )
			rects[i].render();
	}

	// transparencies after statics
	for (int i = 0; i < rectCount; i++ )
	{
		if ( rects[i].getColor() & 0xff000000 != 0xff000000 )
			rects[i].render();
	}


	for (int  i = 0; i < buttonCount; i++ )
		buttons[i].render();

	for (int  i = 0; i < textCount; i++ )
	{
		textObjects[i].render();
	}

	for (int  i = 0; i < editCount; i++ )
		edits[i].render( );

	for (int  i = 0; i < animObjectsCount; i++ )
		animObjects[i].render();



	if ( fadeOutTime )
	{
		fadeTime += frameLength;
		long color = interpolateColor( 0,fadeOutMaxColor, fadeTime/fadeOutTime );
		GUI_RECT rect = { 0,0, Environment.screenWidth, Environment.screenHeight };
		drawRect( rect, color );
	}
	else if ( fadeInTime && fadeInTime > fadeTime )
	{
		fadeTime += frameLength;
		long color = interpolateColor( fadeOutMaxColor, 0, fadeTime/fadeInTime );
		GUI_RECT rect = { 0,0, Environment.screenWidth, Environment.screenHeight };
		drawRect( rect, color );
	}
	

}

long LogisticsScreen::getStatus()
{
	if ( status != RUNNING && fadeOutTime )
	{
		if ( fadeTime > fadeOutTime )
		{
			return status;
		}
		else
			return RUNNING; // fake it until done fading
	}

	return status;
}

void LogisticsScreen::render( int xOffset, int yOffset )
{
	if ( !isShowing() )
		return;
	
	for (int i = 0; i < rectCount; i++ )
	{
		if ( !rects[i].bOutline&& 
			( (rects[i].getColor() & 0xff000000) == 0xff000000 ) )
		{
			rects[i].move( xOffset, yOffset );
			rects[i].render();
			rects[i].move( -xOffset, -yOffset );
		}
	}


	for (int  i = 0; i < staticCount; i++ )
	{
		statics[i].move( xOffset, yOffset );
		statics[i].render();
		statics[i].move( -xOffset, -yOffset );
	}

	for (int  i = 0; i < rectCount; i++ )
	{
		if ( rects[i].bOutline )
		{
			rects[i].move( xOffset, yOffset );
			rects[i].render();
			rects[i].move( -xOffset, -yOffset );
		}
	}

	// transparencies after statics
	for (int  i = 0; i < rectCount; i++ )
	{
		if ( (rects[i].getColor() & 0xff000000) != 0xff000000 )
		{
			rects[i].move( xOffset, yOffset );
			rects[i].render();
			rects[i].move( -xOffset, -yOffset );
		}
	}

	for (int  i = 0; i < buttonCount; i++ )
	{
		buttons[i].move( xOffset, yOffset );
		buttons[i].render();
		buttons[i].move( -xOffset, -yOffset );
	}

	for (int  i = 0; i < textCount; i++ )
	{
		textObjects[i].move( xOffset, yOffset );
		textObjects[i].render();
		textObjects[i].move( -xOffset, -yOffset );
	}

	for (int  i = 0; i < editCount; i++ )
	{
		edits[i].move( xOffset, yOffset );
		edits[i].render();
		edits[i].move( -xOffset, -yOffset );
	}

	for (int  i = 0; i < animObjectsCount; i++ )
	{
		animObjects[i].move( xOffset, yOffset );
		animObjects[i].render();
		animObjects[i].move( -xOffset, -yOffset );
	}

	if ( fadeOutTime )
	{
		fadeTime += frameLength;
		long color = interpolateColor( 0,0xff000000, fadeTime/fadeOutTime );
		GUI_RECT rect = { 0,0, Environment.screenWidth, Environment.screenHeight };
		drawRect( rect, color );
	}
	else if ( fadeInTime && fadeInTime > fadeTime )
	{
		fadeTime += frameLength;
		long color = interpolateColor( 0xff000000, 0, fadeTime/fadeInTime );
		GUI_RECT rect = { 0,0, Environment.screenWidth, Environment.screenHeight };
		drawRect( rect, color );
	}




}

LogisticsScreen::LogisticsScreen( const LogisticsScreen& src )
{
	copyData( src );
}
LogisticsScreen& LogisticsScreen::operator=( const LogisticsScreen& src )
{
	copyData( src );
	return *this;
}

void LogisticsScreen::copyData( const LogisticsScreen& src )
{
	if ( &src != this )
	{
		destroy();

		rectCount = src.rectCount;
		if ( rectCount )
		{
			rects = new aRect[rectCount];
			for (int i = 0; i < src.rectCount; i++ )
			{
				rects[i] = (src.rects[i]);
			}
		}

		staticCount = src.staticCount;
		if ( staticCount )
		{
			statics = new aObject[staticCount];
			for ( int i = 0; i < staticCount; i++ )
			{  
				statics[i] = src.statics[i];
			}
		}

		buttonCount = src.buttonCount;
		if ( buttonCount )
		{
			buttons = new aAnimButton[buttonCount];
			for ( int i = 0; i < buttonCount; i++ )
			{
				buttons[i] = src.buttons[i];
			}
		}

		textCount = src.textCount;
		if ( textCount )
		{
			textObjects = new aText[textCount];
			for ( int i = 0; i < textCount; i++ )
			{
				textObjects[i] = src.textObjects[i];
			}
		}

		animObjectsCount = src.animObjectsCount;
		if ( animObjectsCount )
		{
			animObjects = new aAnimObject[animObjectsCount];
			for ( int i = 0; i < animObjectsCount; i++ )
				animObjects[i] = src.animObjects[i];
		}

		editCount = src.editCount;
		if ( editCount )
		{
			edits = new aEdit[editCount];
			for ( int i = 0; i < editCount; i++ )
				edits[i] = src.edits[i];
		}

	}
}

void  LogisticsScreen::moveTo( long xPos, long yPos )
{
	long xOffset = xPos - globalX();
	long yOffset = yPos - globalY();

	aObject::init( xPos, yPos, 800, 600 );


	move( xOffset, yOffset );
}

void  LogisticsScreen::move( long xOffset, long yOffset )
{
	for (int i = 0; i < rectCount; i++ )
	{
		rects[i].move( xOffset, yOffset );
	}


	for (int  i = 0; i < staticCount; i++ )
	{
		statics[i].move( xOffset, yOffset );
	}

	for (int  i = 0; i < buttonCount; i++ )
	{
		buttons[i].move( xOffset, yOffset );
	}

	for (int  i = 0; i < textCount; i++ )
	{
		textObjects[i].move( xOffset, yOffset );
	}

	for (int  i = 0; i < editCount; i++ )
		edits[i].move( xOffset, yOffset );

	for (int  i = 0; i < animObjectsCount; i++ )
		animObjects[i].move( xOffset, yOffset );

}


bool	LogisticsScreen::inside( long x, long y)
{
	for ( int i = 0; i < staticCount; i++ )
	{
		if ( statics[i].pointInside( x, y ) )
			return true;
	}

	for (int  i = 0; i < buttonCount; i++ )
	{
		if ( buttons[i].pointInside( x, y ) )
			return true;
	}

	for (int  i = 0; i < textCount; i++ )
	{
		if ( 	textObjects[i].pointInside( x, y ) )
			return true;
	}

	for (int  i = 0; i < rectCount; i++ )
	{
		if ( rects[i].pointInside( x, y ) )
			return true;
	}

	for (int  i = 0; i < animObjectsCount; i++ )
	{
		if ( animObjects[i].pointInside( x, y ) )
			return true;
	}

		return false;

	return false;
}

void LogisticsScreen::begin()
{

	for ( int i = 0; i < animObjectsCount; i++ )
		animObjects[i].begin();

	status = RUNNING; 

	gos_KeyboardFlush();
}




//*************************************************************************************************
// end of file ( LogisticsScreen.cpp )
