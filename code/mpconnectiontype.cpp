#define MPCONNECTIONTYPE_CPP
/*************************************************************************************************\
MPConnectionType.cpp			: Implementation of the MPConnectionType component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mpconnectiontype.h"
#include"prefs.h"
#include"inifile.h"
// sebi
//#include"../userinput.h"
#include"userinput.h"

#include "../resource.h"
#include"multplyr.h"
#include"mechbayscreen.h"

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#define CHECK_BUTTON 200

static int connectionType = 0;

static const int FIRST_BUTTON_ID = 1000010;
static const int OK_BUTTON_ID = 1000001;
static const int CANCEL_BUTTON_ID = 1000002;

extern bool quitGame;

MPConnectionType::MPConnectionType() : lanPanel(*this), tcpipPanel(*this)
{
	bDone = 0;

	connectionType = 0;
	pPanel = 0;

	ppConnectionScreen = 0;
	pLocalBrowserScreen = 0;
	pDirectTcpipScreen = 0;
	pMPPlaceHolderScreen = 0;

	status = RUNNING;
	LogisticsScreen::helpTextArrayID = 2;
	bHosting = 0;
}

MPConnectionType::~MPConnectionType()
{
	tcpipPanel.destroy();
}

int MPConnectionType::indexOfButtonWithID(int id)
{
	int i;
	for (i = 0; i < buttonCount; i++)
	{
		if (buttons[i].getID() == id)
		{
			return i;
		}
	}
	return -1;
}

void MPConnectionType::init(FitIniFile* file)
{
	LogisticsScreen::init( *file, "Static", "Text", "Rect", "Button" );

	if ( buttonCount )
	{
		for ( int i = 0; i < buttonCount; i++ )
		{
			buttons[i].setMessageOnRelease();
			if (buttons[i].getID() == 0)
			{
				buttons[i].setID(FIRST_BUTTON_ID + i);
			
			}
			if ( buttons[i].getID()  != MB_MSG_MAINMENU )
			{

				buttons[i].setPressFX( LOG_VIDEOBUTTONS );
				buttons[i].setHighlightFX( LOG_DIGITALHIGHLIGHT );
				buttons[i].setDisabledFX( LOG_WRONGBUTTON );
			}
		}
	}

	{
		char path[256];
		strcpy( path, artPath );
		strcat( path, "mcl_mp_contype_zone.fit" );
		
		FitIniFile PNfile;
		if ( NO_ERR != PNfile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", path );
			Assert( 0, 0, error );
			return;
		}
		zonePanel.init(&PNfile, this);
	}

	{
		char path[256];
		strcpy( path, artPath );
		strcat( path, "mcl_mp_contype_lan.fit" );
		
		FitIniFile PNfile;
		if ( NO_ERR != PNfile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", path );
			Assert( 0, 0, error );
			return;
		}
		lanPanel.init(&PNfile);
	}

	{
		char path[256];
		strcpy( path, artPath );
		strcat( path, "mcl_mp_contype_tcpip.fit" );
		
		FitIniFile PNfile;
		if ( NO_ERR != PNfile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", path );
			Assert( 0, 0, error );
			return;
		}
		tcpipPanel.init(&PNfile);
	}

	connectionType = 0;

	buttons[indexOfButtonWithID(FIRST_BUTTON_ID+1)].press(!((1 == connectionType) || (2 == connectionType) || (3 == connectionType)));
	buttons[indexOfButtonWithID(FIRST_BUTTON_ID+2)].press(1 == connectionType);
	buttons[indexOfButtonWithID(FIRST_BUTTON_ID+3)].press(2 == connectionType);

	switch (connectionType)
	{
	case 1:
		pPanel = &lanPanel;
		break;
	case 2:
		pPanel = &tcpipPanel;
		break;
	default:
		pPanel = &zonePanel;
		break;
	}

	
	hostDlg.init();

}


void MPConnectionType::begin()
{
	status = RUNNING;
	beginFadeIn( 1.0 );
	bHosting = 0;

	if ( MPlayer )
	{
		MPlayer->closeSession();

		if ( !MPlayer->getOnLAN() )
		{
			getButton( FIRST_BUTTON_ID+2 )->disable( true );
		}
		else
		{
			getButton( FIRST_BUTTON_ID+2 )->disable( false );
		}
	}

	tcpipPanel.begin();
}


void MPConnectionType::end()
{
}

void MPConnectionType::render(int xOffset, int yOffset )
{
	LogisticsScreen::render(xOffset, yOffset);

	if ((0 == xOffset) && (0 == yOffset))
	{
		if (pPanel)
		{
			pPanel->render(xOffset, yOffset);
		}
	}
	else // if scrolling, make sure we don't fade
		beginFadeIn( 0 );

	if ( bHosting )
		hostDlg.render();
}

void MPConnectionType::render()
{
	render(0, 0);
}

int	MPConnectionType::handleMessage( unsigned long message, unsigned long who)
{
	if ( RUNNING == status )
	{
		switch ( who )
		{
		case 57/*MB_MSG_MAINMENU*/:
			{
				getButton( 57/*MB_MSG_MAINMENU*/ )->press( 0 );
				status = MAINMENU;
			}
			break;
		case 51/*MB_MSG_PREV*/:
			{
				getButton( 51/*MB_MSG_PREV*/ )->press( 0 );
				status = UP;
			}
			break;
		case 50/*MB_MSG_NEXT*/:
			{
				switch (connectionType)
				{
				case 0/*internet*/:
					{
						(*ppConnectionScreen) = pLocalBrowserScreen;
					}
					break;
				case 1/*local browser*/:
					{
						(*ppConnectionScreen) = pLocalBrowserScreen;
					}
					break;
				case 2/*direct tcp/ip*/:
					{
						(*ppConnectionScreen) = pDirectTcpipScreen;
					}
					break;
				case 3/*modem2modem*/:
					{
						(*ppConnectionScreen) = pLocalBrowserScreen;
					}
					break;
				}
				getButton( 50/*MB_MSG_NEXT*/ )->press( 0 );
				status = NEXT;
			}
			break;
		case FIRST_BUTTON_ID+1:
			{
				connectionType = 0;
				pPanel = &zonePanel;
			}
			break;
		case FIRST_BUTTON_ID+2:
			{
				connectionType = 1;
				pPanel = &lanPanel;
			}
			break;
		case FIRST_BUTTON_ID+3:
			{
				connectionType = 2;
				pPanel = &tcpipPanel;
			}
			break;
		case LAN_PANEL_FIRST_BUTTON_ID:
			{
				(*ppConnectionScreen) = pLocalBrowserScreen;
				status = NEXT;
			}
			break;
		case LAN_PANEL_FIRST_BUTTON_ID + 1:
			{
				bHosting = true;
				hostDlg.begin();
			}
			break;
		case TCPIP_PANEL_FIRST_BUTTON_ID:
			{
				(*ppConnectionScreen) = pMPPlaceHolderScreen;
				// now I need to pop a connecting dialog...
				status = NEXT;
			}
			break;
		case TCPIP_PANEL_FIRST_BUTTON_ID + 1:
			{
				bHosting = true;
				hostDlg.begin();
			}
			break;

		case ZONE_PANEL_FIRST_BUTTON_ID:
			{
				MPlayer->launchBrowser( NULL );
				(*ppConnectionScreen) = pMPPlaceHolderScreen;
				// now I need to pop a connecting dialog...
				status = NEXT;
				quitGame = true;
			}
			break;
		}

	

		switch ( who )
		{
		case FIRST_BUTTON_ID+1:
		case FIRST_BUTTON_ID+2:
		case FIRST_BUTTON_ID+3:
				buttons[indexOfButtonWithID(FIRST_BUTTON_ID+1)].press(!((1 == connectionType) || (2 == connectionType) || (3 == connectionType)));
				buttons[indexOfButtonWithID(FIRST_BUTTON_ID+2)].press(1 == connectionType);
				buttons[indexOfButtonWithID(FIRST_BUTTON_ID+3)].press(2 == connectionType);
				return 1;
			break;
		}
	}

	return 0;

}

bool MPConnectionType::isDone()
{
	return bDone;
}

void MPConnectionType::update()
{
	if ( bHosting )
	{
		hostDlg.update();
		if ( hostDlg.isDone() )
		{
			if ( hostDlg.getStatus() == YES )
			{
				bHosting = 0;
				hostDlg.end();
				(*ppConnectionScreen) = pMPPlaceHolderScreen;
				status = NEXT;

			}
			else if ( hostDlg.getStatus() == NO )
				bHosting = 0;
		}

		return;
	}
	LogisticsScreen::update();
	if (pPanel)
	{
		pPanel->update();
	}



	//helpTextID = 0;
	//helpTextHeaderID = 0;

	/*
	for ( int i = 0; i < buttonCount; i++ )
	{
		buttons[i].update();
		if ( buttons[i].pointInside( userInput->getMouseX(), userInput->getMouseY() )
			&& userInput->isLeftClick() )
		{
			handleMessage( buttons[i].getID(), buttons[i].getID() );
		}

	}
	*/
}


void aZonePanel::init(FitIniFile* pFile, LogisticsScreen* pParent)
{
	FitIniFile &file = (*pFile);

	button.init( file, "Button0" );
	button.setMessageOnRelease();

	button.setPressFX( LOG_VIDEOBUTTONS );
	button.setHighlightFX( LOG_DIGITALHIGHLIGHT );
	button.setDisabledFX( LOG_WRONGBUTTON );
	if (button.getID() == 0)
	{
		button.setID(ZONE_PANEL_FIRST_BUTTON_ID);
	}
	addChild(&button);

	text.init( &file, "Text0" );
	addChild(&text);
	pParentScreen = pParent;
	bShowWarning = 0;
}

void aZonePanel::update()
{
	if ( bShowWarning )
	{
		LogisticsOKDialog::instance()->update();
		if ( LogisticsOKDialog::instance()->isDone() )
		{
			if ( LogisticsOKDialog::instance()->getStatus() == LogisticsScreen::YES )
			{
				pParentScreen->handleMessage( ZONE_PANEL_FIRST_BUTTON_ID, ZONE_PANEL_FIRST_BUTTON_ID );
				quitGame = true;
			}

			LogisticsOKDialog::instance()->end();
			bShowWarning = 0;
		}
	}
	else
		aObject::update();


	
}

void aZonePanel::render()
{
	aObject::render();
	
	if ( bShowWarning )
	{
		LogisticsOKDialog::instance()->render();
	}


}

int aZonePanel::handleMessage( unsigned long, unsigned long )
{
	bShowWarning = true;
	
	LogisticsOKDialog::instance()->begin();
	LogisticsOKDialog::instance()->setText( IDS_MP_CON_ZONE_WARNING, IDS_PM_CANCEL, IDS_MP_CON_ZONE_WARNING_NEXT );
	return 1;
}

void aLanPanel::update()
{
	aObject::update();


}

void aLanPanel::init(FitIniFile* pFile)
{
	FitIniFile &file = (*pFile);

	button0.init( file, "Button0" );
	button0.setMessageOnRelease();
	if (button0.getID() == 0)
	{
		button0.setID(LAN_PANEL_FIRST_BUTTON_ID);

	}

	button0.setPressFX( LOG_VIDEOBUTTONS );
	button0.setHighlightFX( LOG_DIGITALHIGHLIGHT );
	button0.setDisabledFX( LOG_WRONGBUTTON );

	addChild(&button0);

	button1.init( file, "Button1" );
	button1.setMessageOnRelease();
	if (button1.getID() == 0)
	{
		button1.setID(LAN_PANEL_FIRST_BUTTON_ID + 1);
	}
	addChild(&button1);

	button1.setPressFX( LOG_VIDEOBUTTONS );
	button1.setHighlightFX( LOG_DIGITALHIGHLIGHT );
	button1.setDisabledFX( LOG_WRONGBUTTON );


	text.init( &file, "Text0" );
	addChild(&text);
}

int	aLanPanel::handleMessage( unsigned long message, unsigned long who)
{
	switch ( who )
	{
	case LAN_PANEL_FIRST_BUTTON_ID:
	case LAN_PANEL_FIRST_BUTTON_ID + 1:
		if (pParentScreen)
		{
			pParentScreen->handleMessage(message, who);
		}
		break;
	}
	return 0;
}

void aTcpipPanel::begin()
{

	comboBox.ListBox().removeAllItems(true);

	for ( int i = 0; i < 10; i++ )
	{
		if ( !strlen( prefs.ipAddresses[i] ) )
			break;

		comboBox.AddItem( prefs.ipAddresses[i], 0xffffffff );
	}

	bExpanded = 0;
	bFoundConnection = 0;
	bConnectingDlg = 0;
	bErrorDlg = 0;


}

void aTcpipPanel::destroy()
{
	comboBox.destroy();
}

void aTcpipPanel::init(FitIniFile* pFile)
{
	FitIniFile &file = (*pFile);
	bConnectingDlg = 0;

	helpRect.init( pFile, "Rect0" );
	

	button0.init( file, "Button0" );
	button0.setMessageOnRelease();
	if (button0.getID() == 0)
	{
		button0.setID(TCPIP_PANEL_FIRST_BUTTON_ID);
	}
	addChild(&button0);

	button0.setPressFX( LOG_VIDEOBUTTONS );
	button0.setHighlightFX( LOG_DIGITALHIGHLIGHT );
	button0.setDisabledFX( LOG_WRONGBUTTON );


	button1.init( file, "Button1" );
	button1.setMessageOnRelease();
	if (button1.getID() == 0)
	{
		button1.setID(TCPIP_PANEL_FIRST_BUTTON_ID + 1);
	}
	addChild(&button1);

	button1.setPressFX( LOG_VIDEOBUTTONS );
	button1.setHighlightFX( LOG_DIGITALHIGHLIGHT );
	button1.setDisabledFX( LOG_WRONGBUTTON );


	text0.init( &file, "Text0" );
	addChild(&text0);

	text1.init( &file, "Text1" );
	addChild(&text1);

	FitIniFile tmpFile;
	FullPathFileName path;
	path.init( artPath, "mcl_mp_tcpip_combobox0", ".fit" );
	if ( NO_ERR != tmpFile.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, errorStr );
	}
	
	comboBox.init(&tmpFile, "TCPIPAddressComboBox");
	comboBox.EditBox().setNumeric(true);
	comboBox.EditBox().limitEntry(15);

	comboBox.EditBox().setNoBlank( true );


	comboBox.EditBox().allowIME( false );
	
	addChild( &comboBox );

	for ( int i = 0; i < 10; i++ )
	{
		if ( !strlen( prefs.ipAddresses[i] ) )
			break;

		comboBox.AddItem( prefs.ipAddresses[i], 0xffffffff );
	}

	addChild( &helpRect );

	connectingTime = 0;
	bFoundConnection = 0;

	// need to initialize old addresses
	
}

void aTcpipPanel::update()
{
	long retVal = 0;

	if ( bConnectingDlg || bErrorDlg )
	{
		LogisticsOneButtonDialog::instance()->update();
		if ( LogisticsOneButtonDialog::instance()->isDone()  )
		{
			if ( !bErrorDlg )
			{
				if ( bFoundConnection )
				{
					long sessionCount;
					MC2Session* pSessions =  MPlayer->getSessions( sessionCount );
					if ( sessionCount )
					{
						bConnectingDlg = 0;
						retVal = MPlayer->joinSession( &pSessions[0], prefs.playerName[0] );
						if ( !retVal )
						{
							pParentScreen->handleMessage( 1, TCPIP_PANEL_FIRST_BUTTON_ID );
							MPlayer->endSessionScan();
							EString ipAddress;
							comboBox.EditBox().getEntry( ipAddress );
							prefs.setNewIP( ipAddress );
							prefs.save();
							connectingTime = 0.f;

						}
					}

					bFoundConnection = 0;
					bConnectingDlg = 0;

				}
				//they cancelled
				MPlayer->endSessionScan();
				bConnectingDlg = 0;
			}
			else
			{
				LogisticsOneButtonDialog::instance()->end();
				bErrorDlg = 0;
			}

		}
		else  if ( !bErrorDlg && !bFoundConnection )
		{
			connectingTime += frameLength;

			long sessionCount;
			MC2Session* pSessions =  MPlayer->getSessions( sessionCount );
			if ( sessionCount )
			{
				bFoundConnection = true;
				LogisticsOneButtonDialog::instance()->end();
			}
			else
			{
				bFoundConnection = false;
				if ( connectingTime > 20 )
				{
					retVal = MPLAYER_ERR_NO_CONNECTION;
					connectingTime = 0.f;
				}
			}

		}	

		if ( retVal )
		{
			int errorID =  IDS_MP_CONNECT_NO_SESSION;
			int fontID = IDS_MP_CONNECT_ERROR_NO_SESSION_FONT;
			// display a dialog about why this can't happen....
			switch ( retVal )
			{
			case MPLAYER_ERR_HOST_NOT_FOUND:
				errorID = IDS_MP_CONNECT_ERROR_NO_HOST;
				fontID = IDS_MP_CONNECT_ERROR_NO_HOST_FONT;									
				break;

			case MPLAYER_ERR_NO_CONNECTION:
				errorID = IDS_MP_CONNECT_ERROR_NO_CONNECTION;
				fontID = IDS_MP_CONNECT_ERROR_NO_CONNECTION_FONT;									
				break;

			case MPLAYER_ERR_SESSION_IN_PROGRESS:
				errorID = IDS_MP_CONNECT_ERROR_IN_PROGRESS;
				fontID = IDS_MP_CONNECT_ERROR_IN_PROGRESS_FONT;									
				break;

			case MPLAYER_ERR_SESSION_LOCKED:
				errorID = IDS_MP_CONNECT_ERROR_LOCKED;
				fontID = IDS_MP_CONNECT_ERROR_LOCKED_FONT;									
				break;

			case MPLAYER_ERR_BAD_VERSION:
				errorID = IDS_MP_CONNECTION_ERROR_WRONG_VERSION;
				fontID = IDS_MP_CONNECTION_ERROR_WRONG_VERSION_FONT;
				break;

			case MPLAYER_ERR_SESSION_FULL:
				errorID = IDS_MP_CONNECTION_ERROR_FULL;
				fontID = IDS_MP_CONNECTION_ERROR_FULL_FONT;
					break;

			}
			LogisticsOneButtonDialog::instance()->end();
			LogisticsOneButtonDialog::instance()->setText( errorID, IDS_DIALOG_OK, IDS_DIALOG_OK  );
			LogisticsOneButtonDialog::instance()->setFont( fontID );
			LogisticsOneButtonDialog::instance()->begin();
			bConnectingDlg = 0;
			bErrorDlg = 1;
		}
		return;
	}
	
	if ( comboBox.ListBox().isShowing() )
	{
		bExpanded = true;
		comboBox.update();
	}
	else
	{
		if ( !bExpanded )
			aObject::update();
		if ( userInput->leftMouseReleased() )
			bExpanded = 0;
	}


	// grey out button if inavlid...
	EString str;
	comboBox.EditBox().getEntry(str);
	bool bValid = 1;

	if ( str.Length() )
	{
		// now look for 3 periods
		int dotIndex[5];
		for ( int i = 0; i < 5; i++ )
			dotIndex[i] = -1;

		dotIndex[4] = str.Length();

		int tmp = -1;
		while ( tmp )
		{
			tmp = str.Find( '.', tmp+1 );
			if ( tmp == -1 )
			{
				break;
			}
			else
			{
                int i = 0;
				for (; i < 3; i++ )
				{
					if ( dotIndex[i+1] == -1 )
					{
						dotIndex[i+1] = tmp;
						break;
					}
				}
				if ( i == 3 ) // to many decimals
				{
					bValid = 0;
					break;
				}
			}
		}
		if ( dotIndex[3] == -1 || !bValid )
			bValid = 0;
		else if ( dotIndex[3] < str.Length() - 1 )
		{
			bValid = 1;
			char tmp[256];
			strcpy( tmp, str );
			for ( int i = 0; i < 4; i++ )
			{
				long num = getNum( tmp, dotIndex[i]+1, dotIndex[i+1] );
				if ( num < 0 || num > 255 )
				{
					bValid = 0;
				}
			}

			
		}
		else
			bValid = 0;
	}
	else
		bValid = 0;

	if ( bValid )
	{
		button0.disable( 0 );
	} 
	else
		button0.disable( 1 );

	if ( bValid && 	gos_GetKeyStatus( KEY_RETURN ) == KEY_RELEASED )
	{
		handleMessage( TCPIP_PANEL_FIRST_BUTTON_ID, TCPIP_PANEL_FIRST_BUTTON_ID);
	}

	
}

void		aTcpipPanel::render()
{
	aObject::render();

	if ( bConnectingDlg || bErrorDlg )
	{
		LogisticsOneButtonDialog::instance()->render();
	}
		
}

long aTcpipPanel::getNum( char* pStr, long index1, long index2 )
{
	char tmp = pStr[index2];
	pStr[index2] = 0;

	return atoi( &pStr[index1] );

}
int	aTcpipPanel::handleMessage( unsigned long message, unsigned long who)
{
	switch ( who )
	{
	case TCPIP_PANEL_FIRST_BUTTON_ID:
		{
		bConnectingDlg = true;
		bFoundConnection = 0;
		LogisticsOneButtonDialog::instance()->setText( IDS_MP_CON_MODEM_CONNECTING, IDS_PM_CANCEL, IDS_PM_CANCEL );
		char text[256];
		char Display[256];
		cLoadString( IDS_MP_CON_MODEM_CONNECTING, text, 255 );
		EString str;
		comboBox.EditBox().getEntry( str );
		sprintf( Display, text, (const char*)str );
		LogisticsOneButtonDialog::instance()->setText( Display );
		LogisticsOneButtonDialog::instance()->begin();

		if ( MPlayer->beginSessionScan ( (char*)(const char*)str) )
		{
			LogisticsOneButtonDialog::instance()->setText( IDS_MP_CONNECT_ERROR_NO_CONNECTION, IDS_DIALOG_OK, IDS_DIALOG_OK );
		}
		connectingTime = 0.f;

		break;
		}
	case TCPIP_PANEL_FIRST_BUTTON_ID + 1:
		if (pParentScreen)
		{
			pParentScreen->handleMessage(message, who);
		}
		break;
	}
	return 0;
}
//////////////////////////////////////////////



//*************************************************************************************************
// end of file ( MPConnectionType.cpp )
