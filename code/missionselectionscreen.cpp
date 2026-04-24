#define MISSIONSELECTIONSCREEN_CPP
/*************************************************************************************************\
MissionSelectionScreen.cpp			: Implementation of the MissionSelectionScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"missionselectionscreen.h"
#include"mechbayscreen.h"
#include"logisticsdata.h"
#include "../resource.h"
#include"missionbriefingscreen.h"
#include"sounds.h"
#include"mc2movie.h"
#include"gamesound.h"
#include <SDL2/SDL.h>
#include <iostream>  // Add this for std::cerr

#define VIDEO_RECT 7
#define MAP_RECT 3
#define MSG_FIRST_MISSION 100
#define LB_RECT 4
#define RP_TEXT 4

MissionSelectionScreen::MissionSelectionScreen(  )
{
	status = RUNNING;
	LogisticsScreen::helpTextArrayID = 3;
	bMovie = 0;

	missionCount = 0;

	memset( missionNames, 0, sizeof( const char* ) * MAX_MISSIONS_IN_GROUP );
	bStop = 0;

	missionDescriptionListBox.setPressFX( -1 );
	missionDescriptionListBox.setHighlightFX( -1 );
	missionDescriptionListBox.setDisabledFX( -1 );
}

MissionSelectionScreen::~MissionSelectionScreen()
{
	if (bMovie)
	{
		delete bMovie;
		bMovie = NULL;
	}

	missionDescriptionListBox.destroy();
}

//-------------------------------------------------------------------------------------------------


void MissionSelectionScreen::init( FitIniFile* file )
{
	LogisticsScreen::init( *file, "CMStatic", "CMTextEntry", "CMRect", "CMButton" );
	for ( int i= 0; i < buttonCount; i++ )
		buttons[i].setMessageOnRelease();

	missionCount = 0;

	missionDescriptionListBox.init( rects[LB_RECT].left(), rects[LB_RECT].top(),
		rects[LB_RECT].width(), rects[LB_RECT].height() );

	missionDescriptionListBox.setOrange(true);

	getButton( MN_MSG_PLAY )->setPressFX( LOG_VIDEOBUTTONS );
	getButton( MN_MSG_STOP )->setPressFX( LOG_VIDEOBUTTONS );
	getButton( MN_MSG_PAUSE )->setPressFX( LOG_VIDEOBUTTONS );
	getButton( MN_MSG_PLAY )->setHighlightFX( LOG_DIGITALHIGHLIGHT );
	getButton( MN_MSG_STOP )->setHighlightFX( LOG_DIGITALHIGHLIGHT );
	getButton( MN_MSG_PAUSE )->setHighlightFX( LOG_DIGITALHIGHLIGHT );


}

void MissionSelectionScreen::render(int xOffset, int yOffset )
{
	if ( xOffset == 0 && yOffset == 0 )
		missionDescriptionListBox.render();

	LogisticsScreen::render( xOffset, yOffset );
	if ( !xOffset && !yOffset && bMovie && bMovie->isPlaying() && bMovie->getTextureHandle() )
	{
		// Restored from the original commented-out block (pre-open-source).
		// Per FMV_DESIGN.md §5: argb MUST be 0xffffffff (the gos_tex_vertex
		// fragment shader multiplies vertex color into the sampled texel).
		gos_VERTEX v[4];

		for ( int i = 0; i < 4; i++ )
		{
			v[i].argb = 0xffffffff;
			v[i].frgb = 0;
			v[i].rhw = .5;
			v[i].u = 0.f;
			v[i].v = 0.f;
			v[i].x = rects[VIDEO_RECT].left() + 1 + xOffset;
			v[i].y = rects[VIDEO_RECT].top()  + 1 + yOffset;
			v[i].z = 0.f;
		}

		v[2].x = v[3].x = v[0].x + rects[VIDEO_RECT].width()  - 2;
		v[2].y = v[1].y = v[0].y + rects[VIDEO_RECT].height() - 2;

		v[2].u = v[3].u = 1.0f;
		v[2].v = v[1].v = 1.0f;

		// State discipline note: gos render state is a global state machine,
		// not a stack — gos_State_AlphaMode set here leaks to whatever
		// gos_DrawQuads runs next. Each subsequent UI element must set the
		// alpha mode it needs; we do not restore here.
		gos_SetRenderState( gos_State_Texture,   bMovie->getTextureHandle() );
		gos_SetRenderState( gos_State_Filter,    gos_FilterNone );
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero );
		gos_DrawQuads( v, 4 );
		gos_SetRenderState( gos_State_Texture,   0 );
	}


	operationScreen.render( xOffset, yOffset );


}

void MissionSelectionScreen::update()
{
	if (!playedLogisticsTune)
	{
		soundSystem->playDigitalMusic(LogisticsData::instance->getCurrentMissionTune());
		playedLogisticsTune = true;
	}

	if ( bStop )
	{
		if ( bMovie )
			bMovie->stop();
		bStop = false;
	}

	if ( bMovie )
		bMovie->update();

	LogisticsScreen::update();
	operationScreen.update();

	long oldButton = -1;
	int highlightButton = -1;

	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();

	unsigned long highlightColor = 0;

	for ( int i = 0; i < operationScreen.buttonCount; i++ )
	{
		if ( operationScreen.buttons[i].isShowing() )
		{
			if ( operationScreen.buttons[i].pointInside( mouseX, mouseY ) )
			{
				highlightButton = i;
				highlightColor = operationScreen.buttons[i].getColor( );
			}

			if ( operationScreen.buttons[i].isPressed() && i != pressedButton )
			{
				handleMessage( 0, operationScreen.buttons[i].getID() );
				oldButton = pressedButton;
				pressedButton = i;
				break;
			}
		}
	}

	if ( oldButton != -1 )
		operationScreen.buttons[oldButton].press( 0 );

	missionDescriptionListBox.update();

	if ( pressedButton != -1 )
	{
		operationScreen.textObjects[0].setText( LogisticsData::instance->getMissionFriendlyName(
			missionNames[pressedButton] ) );
//		if ( highlightColor )
//			operationScreen.textObjects[0].setColor( highlightColor );
	}
	else
	{
		operationScreen.textObjects[0].setText( "" );
	}

	if ( LogisticsData::instance->skipLogistics() ) // the launch button
	{
		buttons[5].showGUIWindow( 1 );
	}
	else
		buttons[5].showGUIWindow( 0 );

	

}

void MissionSelectionScreen::begin()
{
	status = RUNNING;
	playedLogisticsTune = false;

	if ( fadeInTime )
		operationScreen.beginFadeIn( fadeInTime );

	// initialize the operation
	const char* str = LogisticsData::instance->getCurrentOperationFileName();

	FullPathFileName fileName;
	fileName.init( artPath, str, ".fit" );

	FitIniFile file;
	if ( NO_ERR != file.open( fileName ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)fileName );
		Assert( 0, 0, errorStr );
	}
	
	operationScreen.moveTo( 0, 0 );
	operationScreen.init( file, "Static", "Text", "Rect", "Button" );
	
	operationScreen.moveTo( rects[MAP_RECT].left(), 
							rects[MAP_RECT].top() );


	
	str = LogisticsData::instance->getCurrentVideoFileName();
	std::cout << "[MissionSelectionScreen] Current video filename: " << (str ? str : "NULL") << "\n";
	if ( str && strlen( str ) )
	{
		FullPathFileName movieName;
		//videoName.init( moviePath, str, ".bik" );
		movieName.init( moviePath, str, ".mp4" );
		std::cout << "[MissionSelectionScreen] Full movie path: " << (const char*)movieName << "\n";

		if (fileExists(movieName) || fileExistsOnCD(movieName))
		{
			std::cout << "[MissionSelectionScreen] Video file found, creating movie\n";
			RECT movieRect;
			movieRect.left = rects[VIDEO_RECT].left()+1;
			movieRect.top = rects[VIDEO_RECT].top()+1;
			movieRect.right = movieRect.left + rects[VIDEO_RECT].width()-2;
			movieRect.bottom = movieRect.top + rects[VIDEO_RECT].height()-2;
			std::cout << "[MissionSelectionScreen] Movie rect: " << movieRect.left << "," << movieRect.top 
			          << " " << (movieRect.right - movieRect.left) << "x" << (movieRect.bottom - movieRect.top) << "\n";
		
			//If there is one already here, cause we loaded a savegame or something,
			// Toss it to prevent leaking from the system Heap!
			if (bMovie)
			{
				bMovie->stop();
				delete bMovie;
				bMovie = NULL;
			}

			bMovie = new MC2Movie();
			RECT convertedRect = { movieRect.left, movieRect.top, movieRect.right, movieRect.bottom };
			bMovie->init( (const char*)movieName, convertedRect, true );
			if (Environment.Renderer == 3)
			{
				//DO NOT show the movies by default in software.
				// It will still work, if they hit play.
				bMovie->stop();
			}

			if ( LogisticsData::instance->getVideoShown() )
				bMovie->stop();

			LogisticsData::instance->setVideoShown( );
		}
	}

	missionCount = MAX_MISSIONS_IN_GROUP;
	int result = LogisticsData::instance->getCurrentMissions( missionNames, missionCount );

	EString selMissionName = LogisticsData::instance->getCurrentMission();
	gosASSERT( result == NO_ERR );

	bool bPressed = 0;
    int i = 0;
	for (; i < missionCount; i++ )
	{
		if ( i > operationScreen.buttonCount )
		{
			Assert( 0, 0, "not enough buttons on the operation screen" );
		}
		else 
		{
			operationScreen.buttons[i].setID( MSG_FIRST_MISSION + i );
			operationScreen.buttons[i].setPressFX( LOG_SELECT );
			if ( !LogisticsData::instance->getMissionAvailable( missionNames[i] ) )
				operationScreen.buttons[i].disable( true );


			else
			{
				operationScreen.buttons[i].disable( false );
				if ( selMissionName.Compare( missionNames[i], 0 ) == 0 )
				{
					operationScreen.buttons[i].press( true );
					handleMessage( 0, MSG_FIRST_MISSION + i );
					bPressed = 1;
					pressedButton = i;

				}
				if ( !bPressed && !selMissionName.Length()  )
				{
					bPressed = 1;
					pressedButton = i;
					operationScreen.buttons[i].press( true );
					handleMessage( 0, MSG_FIRST_MISSION + i );
				}
			}
		}

	}

	for ( ; i < operationScreen.buttonCount; i++ )
	{
		operationScreen.buttons[i].showGUIWindow( 0 );
	}

	bStop = 0;
}

void MissionSelectionScreen::end()
{
	if ( bMovie )
	{
		bMovie->stop();

		delete bMovie;
		bMovie = NULL;
	}

	beginFadeOut( 0 );
}

int MissionSelectionScreen::handleMessage( unsigned long msg, unsigned long who )
{
	if ( who >= MSG_FIRST_MISSION && who < MSG_FIRST_MISSION + MAX_MISSIONS_IN_GROUP )
	{
		setMission( who - MSG_FIRST_MISSION );
	}
	
	switch( who )
	{
	case MB_MSG_NEXT:
		status = NEXT;
		break;

	case MB_MSG_PREV:
		status = PREVIOUS;
		break;

	case MN_MSG_PLAY:
		if ( bMovie )
			bMovie->restart();
		getButton( MN_MSG_STOP )->press( 0 );
		getButton( MN_MSG_PAUSE )->press( 0 );
		break;
	case MN_MSG_STOP:
		if ( bMovie )
			bMovie->stop();
		bStop = true;
		break;

		case MN_MSG_PAUSE:
		// MC2Movie has no pause() — see FMV_DESIGN.md §2.2. The button
		// remains for visual continuity but no longer affects playback.
		if ( bMovie )
		{
			getButton(who)->toggle();
		}
		break;


		break;

	case MB_MSG_MAINMENU:
		status = MAINMENU;
		break;
	}

	return 0;

}

void MissionSelectionScreen::setMission( int whichOne )
{
	LogisticsData::instance->setCurrentMission( missionNames[whichOne] );

	char text[64];
	sprintf( text, "%ld ", LogisticsData::instance->getCBills() );
	textObjects[RP_TEXT].setText( text );


	updateListBox(); 

}

void MissionSelectionScreen::updateListBox()
{
	missionDescriptionListBox.removeAllItems( true );

	
	aTextListItem* pEntry = new aTextListItem( IDS_MN_LB_FONT );
	pEntry->resize( missionDescriptionListBox.width() - missionDescriptionListBox.getScrollBarWidth() - 2,
		pEntry->height() );
	pEntry->setText( IDS_MN_DIVIDER );
	pEntry->setColor( 0xffC66600 );
	missionDescriptionListBox.AddItem( pEntry );


	pEntry = new aTextListItem( IDS_MN_LB_FONT );
	pEntry->resize( missionDescriptionListBox.width() - missionDescriptionListBox.getScrollBarWidth() - 2,
		pEntry->height() );
	pEntry->setText( LogisticsData::instance->getCurrentMissionFriendlyName() );
	pEntry->setColor( 0xffC66600 );
	missionDescriptionListBox.AddItem( pEntry );

	pEntry = new aTextListItem( IDS_MN_LB_FONT );
	pEntry->resize( missionDescriptionListBox.width() - missionDescriptionListBox.getScrollBarWidth() - 2,
		pEntry->height() );
	pEntry->setText( IDS_MN_DIVIDER );
	pEntry->setColor( 0xffC66600   );
	missionDescriptionListBox.AddItem( pEntry );

	pEntry = new aTextListItem( IDS_MN_LB_FONT );
	pEntry->resize( missionDescriptionListBox.width() - missionDescriptionListBox.getScrollBarWidth() - 2,
		pEntry->height() );
	pEntry->setText( LogisticsData::instance->getCurrentMissionDescription() );
	pEntry->setColor( 0xffC66600 );
	pEntry->sizeToText();
	missionDescriptionListBox.AddItem( pEntry );







}


//*************************************************************************************************
// end of file ( MissionSelectionScreen.cpp )
