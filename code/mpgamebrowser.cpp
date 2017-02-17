#define MPGAMEBROWSER_CPP
/*************************************************************************************************\
MPGameBrowser.cpp			: Implementation of the MPGameBrowser component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mpgamebrowser.h"
#include"prefs.h"
#include"inifile.h"
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

#define SORT_ORDER_NAME	200
#define SORT_ORDER_PLAYERS	201
#define SORT_ORDER_MAP	202
#define SORT_ORDER_PING	203

extern CPrefs prefs;

MPGameBrowser::MPGameBrowser()
{

	status = NEXT;

	helpTextArrayID = 5;
	bShowErrorDlg = 0;

	sortOrder = SORT_ORDER_NAME;
	bSortUpward = 1;
	bHosting = 0;
	bShowErrorDlg = 0;

}

MPGameBrowser::~MPGameBrowser()
{
//	gameList.destroy();
}

int MPGameBrowser::indexOfButtonWithID(int id)
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

void MPGameBrowser::init(FitIniFile* file)
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
				buttons[i].setPressFX( LOG_VIDEOBUTTONS );
				buttons[i].setHighlightFX( LOG_DIGITALHIGHLIGHT );
				buttons[i].setDisabledFX( LOG_WRONGBUTTON );
			}
		}
	}


	{
		char path[256];
		strcpy( path, artPath );
		strcat( path, "mcl_mp_lanbrowsercombobox0.fit" );
		
		FitIniFile PNfile;
		if ( NO_ERR != PNfile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", path );
			Assert( 0, 0, error );
			return;
		}
		
		PNfile.seekBlock("GameList"); /*for some reason aListBox::init(...) doesn't do the seekBlock itself*/
		gameList.init(&PNfile, "GameList");

		gameList.SelectItem(0);
		gameList.init( rects[0].left(), rects[0].top(), rects[0].width(), rects[0].height() );
		gameList.setOrange( true );

		templateItem.init(&PNfile, "GameListItem");

		for ( int i = 0; i < 256; i++ )
		{
			items[i] = templateItem;
		}

	}


	hostDlg.init();
}

void MPGameBrowser::begin()
{
	status = RUNNING;
	bHosting = 0;
	bShowErrorDlg = 0;
	

	if ( MPlayer )
	{
		MPlayer->closeSession();
//		MPlayer->leaveSession(); // make sure I'm not hosting or already in a game....
		MPlayer->beginSessionScan (NULL);
		MPlayer->setMode(MULTIPLAYER_MODE_BROWSER);
	}

}


void MPGameBrowser::end()
{
	if ( MPlayer )
		MPlayer->endSessionScan();

	bHosting = 0;
}

void MPGameBrowser::render(int xOffset, int yOffset )
{
	if ((0 == xOffset) && (0 == yOffset))
	{
		gameList.render();
	}

	LogisticsScreen::render(xOffset, yOffset);

	if ( bHosting )
	{
		hostDlg.render();
	}
	if ( bShowErrorDlg )
	{
		LogisticsOneButtonDialog::instance()->render();
		return;
	}

	
}

void MPGameBrowser::render()
{
	render(0, 0);
}

int	MPGameBrowser::handleMessage( unsigned long message, unsigned long who)
{
	if ( RUNNING == status )
	{
		switch ( who )
		{

		case SORT_ORDER_NAME:
		case SORT_ORDER_PLAYERS:
		case SORT_ORDER_MAP:
		case SORT_ORDER_PING:
			if ( sortOrder == who )
				bSortUpward ^= 1;
			else 
				bSortUpward = 1;
			sortOrder = who;
			break;

		case 57/*MB_MSG_MAINMENU*/:
			{
				getButton( 57/*MB_MSG_MAINMENU*/ )->press( 0 );
				status = MAINMENU;
			}
			break;
		case 51/*MB_MSG_PREV*/:
			{
				getButton( 51/*MB_MSG_PREV*/ )->press( 0 );
				status = PREVIOUS;
			}
			break;
		case 50/*MB_MSG_NEXT*/:
			{
				getButton( 50/*MB_MSG_NEXT*/ )->press( 0 );
				int index = gameList.GetSelectedItem();
				if ( index != -1 )
				{
					aGameListItem* pItem = (aGameListItem*)gameList.GetItem( index );
					if ( pItem )
					{
						if ( MPlayer )
						{
							long retVal = MPlayer->joinSession ( (MC2Session*)pItem->getSession(), &prefs.playerName[0][0] );
							if ( retVal == MPLAYER_NO_ERR ) {
								MPlayer->setMode(MULTIPLAYER_MODE_PARAMETERS);
								status = NEXT;
							}
							else
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

								LogisticsOneButtonDialog::instance()->begin();
								LogisticsOneButtonDialog::instance()->setText( errorID, IDS_DIALOG_OK, IDS_DIALOG_OK  );
								LogisticsOneButtonDialog::instance()->setFont( fontID );
								bShowErrorDlg = true;
							}
						}
					}
				}
			}
			break;
		case FIRST_BUTTON_ID+1:
			{
				//join game
				getButton( FIRST_BUTTON_ID+1 )->press( 0 );
				status = NEXT;
				return 1;
			}
			break;
		case FIRST_BUTTON_ID+2:
			{
				//host game
				getButton( FIRST_BUTTON_ID+2 )->press( 0 );
				bHosting = true;
				hostDlg.begin();
				return 1;
			}
			break;
		}
	}

	return 0;

}

bool MPGameBrowser::isDone()
{
	return status != RUNNING;
}

void MPGameBrowser::update()
{
	if ( bHosting )
	{
		hostDlg.update();
		if ( hostDlg.isDone() )
		{
			bHosting = 0;
			if ( hostDlg.getStatus() == YES )
			{
				status = NEXT;
			}
		}
		return;
	}
	else if ( bShowErrorDlg )
	{
		LogisticsOneButtonDialog::instance()->update();
		if ( LogisticsOneButtonDialog::instance()->isDone() )
		{
			bShowErrorDlg = 0;
		}

		return;
	}

	LogisticsScreen::update();
	gameList.update();
	float oldScrollPos = gameList.getScrollPos();

	if ( userInput->isLeftDoubleClick() )
	{
		int index = gameList.GetSelectedItem();
		if ( index != -1 )
		{
			handleMessage( MB_MSG_NEXT, MB_MSG_NEXT );
			return;
		}
	}

	int oldSel = gameList.GetSelectedItem();
	int oldHighlight = -1;
	for ( int i = 0; i < gameList.GetItemCount(); i++ )
	{
		if ( gameList.GetItem( i )->getState() == aListItem::HIGHLITE )
			oldHighlight = i;
	}

	helpTextID = 0;
	helpTextHeaderID = 0;

	long sessionCount = 0;
	if ( MPlayer )
	{
		MC2Session* pSessions =  MPlayer->getSessions (sessionCount);

		gameList.removeAllItems( 0 );

		// could easily do sort here.

		
		for ( int i = 0; i < sessionCount; i++ )
		{
			if (pSessions[i].cancelled)
				continue;
			items[i].setSessionInfo( &pSessions[i] );
			items[i].moveTo( templateItem.globalX(), templateItem.globalY() );
			bool bAdded = 0;
			for ( int j = 0; j < gameList.GetItemCount(); j++ )
			{

				aGameListItem* pItem = (aGameListItem*)gameList.GetItem( j );
				int res = S_stricmp( pItem->getText( sortOrder ), items[i].getText( sortOrder ) );
				if ( (bSortUpward && res > 0) || (!bSortUpward && res < 0) )
				{
					gameList.InsertItem( &items[i], j );
					bAdded = true;
					break;
				}
			}
			if ( !bAdded )
			{
				gameList.AddItem( &items[i] );
			}

		}
		gameList.SelectItem( oldSel );

		if ( oldHighlight != -1 && gameList.GetItem( oldHighlight ) )
			gameList.GetItem( oldHighlight )->setState( aListItem::HIGHLITE );
	}

	long sel = gameList.GetSelectedItem();
	if ( sel == -1 )
	{
		getButton( MB_MSG_NEXT )->disable( true );
	}
	else
		getButton( MB_MSG_NEXT )->disable( false );

	gameList.setScrollPos(oldScrollPos);
}



long aStyle3TextListItem::init( FitIniFile* file, const char* blockName )
{
	file->seekBlock( blockName );

	long x = 0;
	long y = 0;
	file->readIdLong( "XLocation", x );
	file->readIdLong( "YLocation", y );

	long fontResID = 0;
	file->readIdLong( "Font", fontResID );
	long textID = 0;
	file->readIdLong( "TextID", textID );
	aTextListItem::init(fontResID);
	setText(textID);
	long color = 0xff808080;
	file->readIdLong( "Color", color );
	normalColor = color;
	setColor(color);

	char tmpStr[64];
	strcpy(tmpStr, "");
	file->readIdString( "Animation", tmpStr, 63 );
	if (0 == strcmp("", tmpStr))
	{
		hasAnimation = false;
	}
	else
	{
		hasAnimation = true;
		animGroup.init(file, tmpStr);
	}

	moveTo(x, y);
	return 0;
}

void aStyle3TextListItem::render()
{
	unsigned long color;
	animGroup.update();
	if (aListItem::SELECTED == getState())
	{
		animGroup.setState( aAnimGroup::PRESSED );
		color = animGroup.getCurrentColor( aAnimGroup::PRESSED );
	}
	else if (aListItem::HIGHLITE == getState())
	{
		animGroup.setState( aAnimGroup::HIGHLIGHT );
		color = animGroup.getCurrentColor( aAnimGroup::HIGHLIGHT );
	}
	else if (aListItem::DISABLED == getState())
	{
		animGroup.setState( aAnimGroup::DISABLED );
		color = animGroup.getCurrentColor( aAnimGroup::DISABLED );
	}
	else
	{
		animGroup.setState( aAnimGroup::NORMAL );
		color = animGroup.getCurrentColor( aAnimGroup::NORMAL );
	}
	aTextListItem::setColor((unsigned long)color);

	aTextListItem::render();
}

aGameListItem& aGameListItem::operator=( const aGameListItem& src )
{

	removeAllChildren( );

	aObject::operator=( src );

	session = src.session;
	allTechGraphic = src.allTechGraphic;
	gameName = src.gameName;
	numPlayers = src.numPlayers;
	mapName = src.mapName;
	latency = src.latency;
	allTechRect = src.allTechRect;
	gameNameRect = src.gameNameRect;
	numPlayersRect = src.numPlayersRect;
	mapNameRect = src.mapNameRect;
	latencyRect = src.latencyRect;
	pingIcon = src.pingIcon;

	addChild(&allTechGraphic);
	addChild(&gameName);
	addChild(&numPlayers);
	addChild(&mapName);
	addChild(&latency);
	addChild(&allTechRect);
	addChild(&gameNameRect);
	addChild(&numPlayersRect);
	addChild(&mapNameRect);
	addChild(&latencyRect);
	addChild(&pingIcon );

	return *this;
}
aGameListItem::aGameListItem() : latency( IDS_MP_LANBROW_PING_FONT )
{


}
long aGameListItem::init( FitIniFile* file, const char* blockName )
{
	file->seekBlock( blockName );

	long width = 0;
	long height = 0;
	file->readIdLong( "Width", width );
	file->readIdLong( "Height", height );

	EString graphicBlockName;
	graphicBlockName += "Static0";
	allTechGraphic.init(file, graphicBlockName.Data());
	if (allTechGraphic.height() + 5 > height)
	{
		height = allTechGraphic.height() + 5;
	}
	if (allTechGraphic.globalRight() - globalX() > width)
	{
		width = allTechGraphic.globalRight() - globalX();
	}

	EString textBlockName;
	textBlockName = "Text0";
	gameName.init(file, textBlockName.Data());
	if (gameName.height() > height)
	{
		height = gameName.height();
	}
	if (gameName.globalRight() - globalX() > width)
	{
		//width = gameName.globalRight() - globalX();
	}

	textBlockName = "Text1";
	numPlayers.init(file, textBlockName.Data());
	if (numPlayers.height() > height)
	{
		height = numPlayers.height();
	}
	if (numPlayers.globalRight() - globalX() > width)
	{
		//width = numPlayers.globalRight() - globalX();
	}

	textBlockName = "Text2";
	mapName.init(file, textBlockName.Data());
	if (mapName.height() > height)
	{
		height = mapName.height();
	}
	if (mapName.globalRight() - globalX() > width)
	{
		//width = mapName.globalRight() - globalX();
	}

	textBlockName = "Text3";
	latency.init(*file, textBlockName.Data());
	if (latency.height() > height)
	{
		height = latency.height();
	}
	if (latency.globalRight() - globalX() > width)
	{
		//width = latency.globalRight() - globalX();
	}

	EString rectBlockName;
	rectBlockName = "Rect0";
	allTechRect.init(file, rectBlockName.Data());
	if (allTechRect.height() > height)
	{
		height = allTechRect.height();
	}
	if (allTechRect.globalRight() - globalX() > width)
	{
		width = allTechRect.globalRight() - globalX();
	}

	rectBlockName = "Rect1";
	gameNameRect.init(file, rectBlockName.Data());
	if (gameNameRect.height() > height)
	{
		height = gameNameRect.height();
	}
	if (gameNameRect.globalRight() - globalX() > width)
	{
		width = gameNameRect.globalRight() - globalX();
	}

	rectBlockName = "Rect2";
	numPlayersRect.init(file, rectBlockName.Data());
	if (numPlayersRect.height() > height)
	{
		height = numPlayersRect.height();
	}
	if (numPlayersRect.globalRight() - globalX() > width)
	{
		width = numPlayersRect.globalRight() - globalX();
	}

	rectBlockName = "Rect3";
	mapNameRect.init(file, rectBlockName.Data());
	if (mapNameRect.height() > height)
	{
		height = mapNameRect.height();
	}
	if (mapNameRect.globalRight() - globalX() > width)
	{
		width = mapNameRect.globalRight() - globalX();
	}

	rectBlockName = "Rect4";
	latencyRect.init(file, rectBlockName.Data());
	if (latencyRect.height() > height)
	{
		height = latencyRect.height();
	}
	if (latencyRect.globalRight() - globalX() > width)
	{
		width = latencyRect.globalRight() - globalX();
	}

	pingIcon.init( file, "Static1" );

	aObject::init(0, 0, width, height);
	
	addChild(&allTechGraphic);
	addChild(&gameName);
	addChild(&numPlayers);
	addChild(&mapName);
	addChild(&latency);
	addChild(&allTechRect);
	addChild(&gameNameRect);
	addChild(&numPlayersRect);
	addChild(&mapNameRect);
	addChild(&latencyRect);
	addChild( &pingIcon );

	return 0;
}

void aGameListItem::setSessionInfo( MC2Session* pSession )
{
	memcpy(&session, pSession, sizeof(MC2Session));
	gameName.setText( pSession->name );
	mapName.setText( pSession->map );
	
	char tmp[256];
	sprintf( tmp, "%ld/%ld", pSession->numPlayers, pSession->maxPlayers );
	numPlayers.setText( tmp );
	sprintf( tmp, "%ld", pSession->ping );
	latency.setText( tmp );

	if ( state == DISABLED )
		setState( ENABLED );

	if ( pSession->locked )
	{
		allTechGraphic.showGUIWindow( true );
		setState( DISABLED );
	}
	else
	{
		allTechGraphic.showGUIWindow( false );
	}

	if ( pSession->numPlayers == pSession->maxPlayers || pSession->inProgress )
		setState( DISABLED );

	long color = 0xff00ff00;
	if ( pSession->ping > 500 )
		color = 0xffff0000;
	else if ( pSession->ping > 300 )
		color = 0xffffff00;


	latency.setColor( color );
	pingIcon.setColor( color );
	//<300ms	 - 	green 
	//301-500ms	 - 	yellow 
	//>501ms	 - 	red


}

const char* aGameListItem::getText( int which )
{
	if ( which == SORT_ORDER_NAME )
		return gameName.getText();
	else if ( which == SORT_ORDER_PLAYERS  )
		return numPlayers.getText();
	else if ( which == SORT_ORDER_PING )
		return numPlayers.getText();
	else if ( which == SORT_ORDER_MAP )
		return mapName.getText();

	return NULL;

}

const char* aGameListItem::getSessionName(  )
{
	return gameName.getText();
}



void aGameListItem::update()
{
	gameName.setState(getState());
	numPlayers.setState(getState());
	mapName.setState(getState());
	latency.setState(getState());
	aListItem::update();
}

//////////////////////////////////////////////



//*************************************************************************************************
// end of file ( MPGameBrowser.cpp )
