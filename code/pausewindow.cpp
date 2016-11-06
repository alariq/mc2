#define PAUSEWINDOW_CPP
/*************************************************************************************************\
PauseWindow.cpp			: Implementation of the PauseWindow component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"pausewindow.h"
#include"controlgui.h"
#include "../resource.h"
#include"userinput.h"
#include"missiongui.h"
#include"logisticsdialog.h"
#include"gamesound.h"
#include"mission.h"
#include"logisticsdata.h"
#include"multplyr.h"

extern float frameLength;

extern bool bInvokeOptionsScreenFlag;
extern long helpTextHeaderID;
extern long helpTextID;
extern unsigned long scenarioResult;
		
extern bool loadInMissionSave;
extern bool saveInMissionSave;

bool aborted = false;

MoveInfo PauseWindow::moveInfo[8] = 
{
	0.0f,	820.f,
	0.04f,	820.f,
	0.28f,	661.f,
	.36f,	678.f,
	.44f,	661.f,
	.52f,	666.f,
	.60f,	661.f,
	3.00f,  661.f
};

PauseWindow::PauseWindow()
{
	buttons = 0;
	buttonData = 0;
	buttonCount = 0;
	statics = 0;
	staticCount = 0;

	finalReference = 0;
	currentTime = 0;
	wasDragging = 0;
	objectivesAlreadyOn = 0;
	bPromptToQuit = bPromptToAbort = 0;

}

//-------------------------------------------------------------------------------------------------

PauseWindow::~PauseWindow()
{
	if ( buttons )
		delete [] buttons;
	if  ( buttonData )
		delete [] buttonData;
	if ( statics )
		delete [] statics;
}

void PauseWindow::update()
{
	if ( bPromptToQuit || bPromptToAbort )
	{
		LogisticsOKDialog::instance()->update();
		if ( LogisticsOKDialog::instance()->getStatus() == LogisticsScreen::YES )
		{
			if ( bPromptToQuit )
				gos_TerminateApplication();
			else {
				//if (MPlayer) {
				//	MPlayer->leaveSession();
				//	}
				//else
				scenarioResult = mis_PLAYER_LOST_BIG;
				aborted = true;
			}

			bPromptToQuit = bPromptToAbort = 0;
		}
		else if ( LogisticsOKDialog::instance()->getStatus() == LogisticsScreen::NO )
		{
			if ( LogisticsOKDialog::instance()->isDone() )
				bPromptToQuit = bPromptToAbort = 0;
		}		
		
	}
	
	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();

	gosEnum_KeyIndex key;
	bool bShift, bCtrl, bAlt;
	MissionInterfaceManager::instance()->getHotKey( OBJECTVIES_COMMAND_KEY, key, bShift, bCtrl, bAlt );

	
	// hack, mission gui message isn't getting here...
	if ( gos_GetKeyStatus( key ) != KEY_HELD 
		&& gos_GetKeyStatus( key ) == KEY_PRESSED )
	{
		buttons[OBJECTIVES].toggle(); // big big hack.
	}


	if ( currentTime != 0 )
	{
		currentTime += frameLength;
		float p0 = 0.f;
		float p1 = 0.f;
		float t0 = 0.f;
		float t1 = 0.f;
		// figure out position based on time
		for ( int j = 0; j < 7; j++ )
		{
			if ( moveInfo[j].time <= currentTime && moveInfo[j+1].time > currentTime )
			{
				t0 = moveInfo[j].time;
				t1 = moveInfo[j + 1].time;
				p0 = -(800.f - moveInfo[j].position)  + ((float)Environment.screenWidth);
				p1 = -(800.f - moveInfo[j + 1].position) + ((float)Environment.screenWidth);
				break;
			}
		}
		if ( p1 )
		{
			float dT = currentTime - t0;
			float currentPosition = p0 + dT * ( (p1 - p0)/(t1 -t0) );
			float delta = currentPosition - currentPos;
			currentPos += delta;

			for ( int i = 0; i < buttonCount; i++ )
			{
				buttons[i].move( delta, 0 );
			}

			for (int i = 0; i < staticCount; i++ )
			{
				statics[i].move( delta, 0 );				
			}


			for (int i = 0; i < 2; i++ )
			{
				float dif = backgrounds[i].right - backgrounds[i].left;
				backgrounds[i].left = .5 + currentPos;
				backgrounds[i].right = .5 + currentPos + dif;
			}
		}
	}



	for ( int i = 0; i < buttonCount; i++ )
	{
		if ( buttons[i].location[0].x <= mouseX && mouseX <= buttons[i].location[2].x
				&& mouseY >= buttons[i].location[0].y && mouseY <= buttons[i].location[1].y  )
		{
			if ( buttons[i].isEnabled() )
			{
				helpTextHeaderID = buttonData[i].helpTextHeader;
				helpTextID = buttonData[i].helpTextID;

				long lastX = mouseX - userInput->getMouseXDelta();
				long lastY = mouseY - userInput->getMouseYDelta();

				if ( buttons[i].location[0].x >= lastX || lastX >= buttons[i].location[2].x
				|| lastY <= buttons[i].location[0].y || lastY >= buttons[i].location[1].y )
				{
					soundSystem->playDigitalSample( LOG_HIGHLIGHTBUTTONS );
				}

				if ( buttons[i].state != ControlButton::PRESSED )
					buttons[i].makeAmbiguous( true );
			}
			else
			{
				helpTextHeaderID = 0;
				helpTextID = 0;
				continue;
			}

			if ( userInput->leftMouseReleased() && !wasDragging )
			{
				{
					handleClick( buttons[i].ID );
				}
			}
		}
		else if ( buttons[i].isEnabled() && buttons[i].state != ControlButton::PRESSED  )
			buttons[i].makeAmbiguous( 0 );
	}

	if ( currentTime == 0 )
	{
		currentTime = .0001f;
		currentPos = -(800 - PauseWindow::moveInfo[0].position) + ((float)Environment.screenWidth);
		float delta = backgrounds[0].left - currentPos;

		for ( int i = 0; i < buttonCount; i++ )
		{
			for ( int j = 0; j < 4; j++ )
			{
				buttons[i].location[j].x -= delta;
			}
		}

		for (int i = 0; i < staticCount; i++ )
		{
			for ( int j = 0; j < 4; j++ )
			{
				statics[i].location[j].x -= delta;
			}
		}

	
		for (int i = 0; i < 2; i++ )
		{
			float dif = backgrounds[i].right - backgrounds[i].left;
			backgrounds[i].left = .5 + currentPos;
			backgrounds[i].right = .5 + currentPos + dif;
		}

		
	}

	wasDragging = userInput->wasLeftDrag();

	const char * campaignName = LogisticsData::instance->getCampaignName().Data();
	char campName[1024];
	_splitpath(campaignName,NULL,NULL,campName,NULL);
	if ( MPlayer || 
		LogisticsData::instance->isSingleMission() ||
		(stricmp("tutorial",campName) == 0))
	{
		buttons[ SAVE ].disable( true );
		buttons[ LOAD ].disable( true );
	}
	if ( MPlayer )
	{
		buttons[OPTIONS].disable( true );
	}
	
}

void PauseWindow::render()
{
	if ( !currentTime )
		return;
	drawRect( backgrounds[0], 0xff000000 );
	drawRect( backgrounds[1], 0xff000000 );

	for ( int i = 0; i < buttonCount; i++ )
	{
		buttons[i].render();
	}

	for (int i = 0; i < staticCount; i++ )
	{
		statics[i].render();
	}	


	char buffer[256];
	cLoadString( IDS_GAMEPAUSED, buffer, 256 );
	headerFont.render( buffer, backgrounds[1].left, backgrounds[1].top, 
		backgrounds[1].right - backgrounds[1].left,
		backgrounds[1].bottom - backgrounds[1].top,
		0xff5c96c2, 0, 3 );
		

	if ( bPromptToQuit || bPromptToAbort )
	{
		LogisticsOKDialog::instance()->render();
	}


}

void PauseWindow::init( FitIniFile& file )
{
	file.seekBlock( "PauseWindow" );
	file.readIdLong( "ButtonCount", buttonCount );
	file.readIdLong( "staticCount", staticCount );

	if ( buttons )
		delete [] buttons;

	if ( buttonData )
		delete [] buttonData;

	if ( statics )
		delete [] statics;

	buttonData = 0;
	buttons = 0;
	statics = 0;
		

	if ( buttonCount )
	{
		buttons = new ControlButton[buttonCount];
		buttonData = new ButtonData[buttonCount];
		font.init( IDS_PAUSEBUTTON800 );
		headerFont.init( IDS_PAUSEDFONT_800 );
		
		ControlButton::initButtons( file, buttonCount, buttons, buttonData, "PauseButton", &font );

		for ( int i = 0; i < buttonCount; i++ )
		{
			buttons[i].move( 0, -ControlGui::hiResOffsetY );
		}
	}

	if ( staticCount )
	{
		statics = new StaticInfo[staticCount];
		char buffer[256];
		for ( int i = 0; i < staticCount; i++ )
		{
			sprintf( buffer, "PauseStatic%ld", i );
			statics[i].init( file, buffer, ControlGui::hiResOffsetX, 0);
		}
	}

	if ( NO_ERR == file.seekBlock( "PauseBackRect" ) )
	{
		file.readIdLong( "left", backgrounds[0].left );
		file.readIdLong( "right", backgrounds[0].right );
		file.readIdLong( "top", backgrounds[0].top );
		file.readIdLong( "bottom", backgrounds[0].bottom );

		backgrounds[0].left += ControlGui::hiResOffsetX;
		backgrounds[0].right += ControlGui::hiResOffsetX;
//		backgrounds[0].top += ControlGui::hiResOffsetY;
//		backgrounds[0].bottom += ControlGui::hiResOffsetY;
	}

	if ( NO_ERR == file.seekBlock( "PauseText" ) )
	{
		file.readIdLong( "left", backgrounds[1].left );
		file.readIdLong( "right", backgrounds[1].right );
		file.readIdLong( "top", backgrounds[1].top );
		file.readIdLong( "bottom", backgrounds[1].bottom );

		backgrounds[1].left += ControlGui::hiResOffsetX;
		backgrounds[1].right += ControlGui::hiResOffsetX;
//		backgrounds[1].top += ControlGui::hiResOffsetY;
//		backgrounds[1].bottom += ControlGui::hiResOffsetY;
	}


}

void PauseWindow::handleClick( int id )
{
	int sound = LOG_SELECT;
	switch ( id )
	{
	case OBJECTIVES:
		if ( buttons[OBJECTIVES].state == ControlButton::PRESSED )
		{
			ControlGui::instance->startObjectives( 0 );		
			buttons[OBJECTIVES].press( 0 );
			objectivesAlreadyOn = 0;
		}
		else
		{
			ControlGui::instance->startObjectives( 1 );		
			buttons[OBJECTIVES].press( 1 );
		}
		break;
	case SAVE:
		if (!MPlayer && !LogisticsData::instance->isSingleMission())
			saveInMissionSave = true;
		else
			sound = INVALID_GUI;
		break;
	case LOAD:
		if (!MPlayer && !LogisticsData::instance->isSingleMission())
			loadInMissionSave = true;
		else
			sound = INVALID_GUI;
		break;
	case RESTART:
		bPromptToAbort = true;
		LogisticsOKDialog::instance()->setText( IDS_DIALOG_ABORT_PROMPT,
				IDS_DIALOG_NO, IDS_DIALOG_YES );				
		LogisticsOKDialog::instance()->begin();
		sound = LOG_MAINMENUBUTTON;
		break;
	case EXIT:
		// i suppose I should put a do you really mean it dialog here
		bPromptToQuit = true;
		LogisticsOKDialog::instance()->setText( IDS_DIALOG_QUIT_PROMPT,
					IDS_DIALOG_NO, IDS_DIALOG_YES );				
		LogisticsOKDialog::instance()->begin();
		sound = LOG_MAINMENUBUTTON;
		break;
	case OPTIONS:
		if (!MPlayer)
		{
			sound = LOG_CLICKONBUTTON;
			bInvokeOptionsScreenFlag = true;
		}
		else
		{
			sound = INVALID_GUI;
		}
		//break;
		//fallthrough
	case RETURN:
		MissionInterfaceManager::instance()->togglePause();
		if ( buttons[OBJECTIVES].state & ControlButton::PRESSED && !objectivesAlreadyOn )
		{
			ControlGui::instance->startObjectives( 0 );		
			buttons[OBJECTIVES].press( 0 );
		}
	
		break;
	}

	soundSystem->playDigitalSample( sound );
}

bool PauseWindow::inRect( int mouseX, int mouseY )
{
	return ( mouseX >= backgrounds[0].left && mouseX <= backgrounds[0].right &&
		 mouseY >= backgrounds[0].top && mouseY <= backgrounds[0].bottom );

}

void PauseWindow::end()
{ 
	currentTime = 5.f;
	if ( buttons[OBJECTIVES].state & ControlButton::PRESSED && !objectivesAlreadyOn )
	{
		ControlGui::instance->startObjectives( 0 );		
		buttons[OBJECTIVES].press( 0 );
	}
}

void PauseWindow::begin(bool objectivesOn)
{
	currentTime = 0.f;
	objectivesAlreadyOn = objectivesOn;
	if ( !objectivesAlreadyOn )
		buttons[OBJECTIVES].press( 0 );
	else
		buttons[OBJECTIVES].press( 1 );
}
//*************************************************************************************************
// end of file ( PauseWindow.cpp )
