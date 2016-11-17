#define MPPREFS_CPP
/*************************************************************************************************\
mpprefs.cpp			: Implementation of the mpprefs component.ef
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mpprefs.h"
#include"mclib.h"
#include"mechbayscreen.h"
#include"prefs.h"
#include"multplyr.h"
#include"gamesound.h"
#include"chatwindow.h"

#define MP_PREFS_BASE	200
#define MP_PREFS_STRIPE	201
#define FIRST_COLOR_RECT	3
#define LAST_COLOR_RECT	34
#define BASE_RECT	35
#define STRIPE_RECT	36

MPPrefs* MPPrefs::s_instance = NULL;

static const char* path2insignia = "data" PATH_SEPARATOR "multiplayer" PATH_SEPARATOR "insignia" PATH_SEPARATOR;

MPPrefs::MPPrefs(  )
{
	helpTextArrayID = 0;
	s_instance = this;
}

//-------------------------------------------------------------------------------------------------

MPPrefs::~MPPrefs()
{
	comboBox[0].destroy();
	comboBox[1].destroy();
	comboBox[2].destroy();
}

int MPPrefs::init( FitIniFile& file )
{
	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button" );

	for ( int i = 0; i < buttonCount; i++ )
	{
		if ( buttons[i].getID() != MP_PREFS_BASE &&
			buttons[i].getID() != MP_PREFS_STRIPE )
			buttons[i].setMessageOnRelease( );

		int id = buttons[i].getID();
		if ( id != MB_MSG_NEXT && id != MB_MSG_PREV && id != MB_MSG_MAINMENU )
		{
			buttons[i].setPressFX( LOG_VIDEOBUTTONS );
			buttons[i].setHighlightFX( LOG_DIGITALHIGHLIGHT );
			buttons[i].setDisabledFX( LOG_WRONGBUTTON );
		}
	}

	file.seekBlock( "ComboBoxes" );
	long count = 0;
	file.readIdLong( "ComboBoxCount", count );
	char blockName[256];

	const char* headers[3] = {"PlayerNameComboBox", "UnitNameComboBox", "UnitInsigniaComboBox" };
	for (int i = 0; i< count; i++ )
	{
		sprintf( blockName, "ComboBox%ld",  i );
		file.seekBlock( blockName );
		file.readIdString( "FileName", blockName, 255);

		long tmpX;
		long tmpY;
		file.readIdLong( "XLocation", tmpX );
		file.readIdLong( "YLocation", tmpY );

		FitIniFile tmpFile;
		FullPathFileName path;
		path.init( artPath, blockName, ".fit" );
		
		if ( NO_ERR != tmpFile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", (const char*)path );
			Assert( 0, 0, error );
			return -1;
		}

		comboBox[i].moveTo( tmpX, tmpY );

		comboBox[i].init( &tmpFile, headers[i] );
		comboBox[i].ListBox().setOrange(true );

		comboBox[0].EditBox().limitEntry( 24 );
		comboBox[1].EditBox().limitEntry( 24 );

		if ( i == 2 )
		{
			insigniaBmp.init( &tmpFile, "Static0" );
			//insigniaBmp.move( comboBox[i].globalX(), comboBox[i].globalY() );
			comboBox[i].addChild( &insigniaBmp );
		}

		comboBox[i].EditBox().allowIME( 0 );
	}


	comboBox[2].EditBox().setReadOnly(true);
	

	camera.init( rects[2].left(), rects[2].top(), rects[2].right(), rects[2].bottom() );

	status = NEXT;


	return 0;
}

void MPPrefs::begin()
{
	status = RUNNING;

	comboBox[0].ListBox().removeAllItems( true );
	comboBox[0].SelectItem( -1 );

	for ( int i = 0; i < 10; i++ )
	{
		if ( strlen( prefs.playerName[i] ) )
		{
			comboBox[0].AddItem( prefs.playerName[i], 0xffffffff );
		}
		else
			break;
	}

	comboBox[0].SelectItem( 0 );

	comboBox[1].ListBox().removeAllItems( true );
	comboBox[1].SelectItem( -1 );

	for (int i = 0; i < 10; i++ )
	{
		if ( strlen( prefs.unitName[i] ) )
		{
			
			comboBox[1].AddItem( prefs.unitName[i], 0xffffffff );
		}
		else
			break;
	}

	comboBox[1].SelectItem( 0 );

	comboBox[2].ListBox().removeAllItems( true );
	comboBox[2].SelectItem( -1 );

    // need to add items to the save game list
	char findString[512];
    char path[256];
	sprintf(findString,"%s*.tga",path2insignia);

	WIN32_FIND_DATA	findResult;
	HANDLE searchHandle = FindFirstFile(findString,&findResult); 
	do
	{
		// 24 or 32 bit files
		if ((findResult.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
		{
			aBmpListItem* pItem = new aBmpListItem;
			sprintf(path, "%s%s", path2insignia, findResult.cFileName  );
			if ( !pItem->setBmp( findResult.cFileName ) )
			{
				delete pItem;
			}
			else
			{
				long index = comboBox[2].AddItem( pItem );
				if ( strstr( findResult.cFileName, prefs.insigniaFile ) )
				{
					comboBox[2].SelectItem( index );
					insigniaBmp.setTexture( path );
					insigniaBmp.setUVs( 0, 0, 32, 32 );
					statics[12].setTexture( path );
					statics[12].setUVs( 0, 0, 32, 32 );
				}
			}

		}
	} while (FindNextFile(searchHandle,&findResult) != 0);

	FindClose(searchHandle);

	if ( comboBox[2].GetSelectedItem( ) == -1 )
	{
		comboBox[2].SelectItem( 0 );
	}
	

	MC2Player* player = MPlayer->getPlayerInfo( MPlayer->commanderID );
	long mySeniority = player->teamSeniority;
	bool bMostSenior = true;

	long playerCount;
	const MC2Player* players = MPlayer->getPlayers(playerCount);
	for(int i = 0; i < playerCount; i++ )
	{
		if ( players[i].teamSeniority > mySeniority && players[i].team == player->team )
		{
			bMostSenior = false;
		}
	}

	if ( !bMostSenior )
	{
		getButton( MP_PREFS_BASE )->press( 0 );
		getButton( MP_PREFS_BASE )->disable( true );
		getButton( MP_PREFS_STRIPE )->press( true );
	}
	else
	{
		getButton( MP_PREFS_BASE )->disable( false );
		getButton( MP_PREFS_BASE )->press( true );
		getButton( MP_PREFS_STRIPE )->press( 0 );
	}

	


	camera.setMech( "Bushwacker", MPlayer->colors[player->baseColor[BASECOLOR_SELF]], 
						MPlayer->colors[player->stripeColor], MPlayer->colors[player->stripeColor] );

	camera.zoomIn( 1.5 );
									


}
void MPPrefs::end()
{
	camera.setMech(NULL);
	status = NEXT;
}


void MPPrefs::update()
{

	MC2Player* player = MPlayer->getPlayerInfo( MPlayer->commanderID );
	long mySeniority = player->teamSeniority;
	bool bMostSenior = true;
	
	long playerCount;
	const MC2Player* players = MPlayer->getPlayers(playerCount);
	for( int i = 0; i < playerCount; i++ )
	{
		if ( players[i].teamSeniority > mySeniority && players[i].team == player->team )
		{
			bMostSenior = false;
		}
	}

	if ( !bMostSenior )
	{
		getButton( MP_PREFS_BASE )->disable( true );
	}
	else
		getButton( MP_PREFS_BASE )->disable( false );
	

	if ( !ChatWindow::instance()->pointInside(userInput->getMouseX(), userInput->getMouseY()) )
		LogisticsScreen::update();

	camera.update();

	int oldSel = comboBox[2].GetSelectedItem();
	bool bExpanded = 0;
	for (int i = 0; i < 3; i++ )
	{
		if ( comboBox[i].ListBox().isShowing() )
		{
			bExpanded = true;
		}
		comboBox[i].update();

		
	}
	int newSel = comboBox[2].GetSelectedItem();

	if ( newSel != oldSel && newSel != -1 )
	{
		aBmpListItem* pItem = (aBmpListItem*)(comboBox[2].ListBox().GetItem( newSel ));
		const char* pName = pItem->getBmp();

		MC2Player* pInfo = MPlayer->getPlayerInfo( MPlayer->commanderID );

		strcpy( pInfo->insigniaFile, pName );

		MPlayer->sendPlayerUpdate( 0, 5, -1 );

	
		FullPathFileName path;
		path.init( path2insignia, pName, ".tga" );
		insigniaBmp.setTexture( path );
		insigniaBmp.setUVs( 0, 0, 32, 32 );
		statics[12].setTexture( path );
		statics[12].setUVs( 0, 0, 32, 32 );

		File file;
		file.open( path );
		long size = file.getLength();

		unsigned char* pData = new unsigned char[size];

		file.read( pData, size );
		MPlayer->sendPlayerInsignia( (char*)pName, pData, size );
		MPlayer->insigniaList[MPlayer->commanderID] = 1;

		delete pData;
	}

	if ( userInput->isLeftClick() && !bExpanded )
	{
		for ( int j = FIRST_COLOR_RECT; j < LAST_COLOR_RECT+1; j++ )
		{
			if ( rects[j].pointInside( userInput->getMouseX(), userInput->getMouseY() ) )
			{
				// OK, figure out if this is remotely viable....
				// x out colors that are already taken
				if ( getButton(MP_PREFS_BASE )->isPressed() )
				{

					setColor( rects[j].getColor() );
				}
				else
					setHighlightColor( rects[j].getColor() );


				break;
	
			}
		}			
	}

	if ( MPlayer && ChatWindow::instance() )
		ChatWindow::instance()->update();

	

 

}

void MPPrefs::setColor( unsigned long color )
{
	long playerCount;
	const MC2Player* players = MPlayer->getPlayers(playerCount);
	for( int i = 0; i < playerCount; i++ )
	{
		if ( MPlayer->colors[players[i].baseColor[BASECOLOR_SELF]] == color && i != MPlayer->commanderID )
		{
			soundSystem->playDigitalSample(LOG_WRONGBUTTON);
			return;
			
		}
	}
					
	//GD:MPlayer->setPlayerBaseColor( MPlayer->commanderID, getColorIndex( color ) );
	MC2Player* player = MPlayer->getPlayerInfo( MPlayer->commanderID );
	player->baseColor[BASECOLOR_PREFERENCE] = getColorIndex(color);
	MPlayer->sendPlayerUpdate( 0, 6, -1 );

	//GD:camera.setMech( "Bushwacker", color, MPlayer->colors[player->stripeColor], MPlayer->colors[player->stripeColor] );
	//GD:camera.zoomIn( 1.5 );

}

void MPPrefs::setHighlightColor( unsigned long color )
{
	MC2Player* player = MPlayer->getPlayerInfo( MPlayer->commanderID );
	player->stripeColor = getColorIndex( color );
	MPlayer->sendPlayerUpdate( 0, 6, -1 );

	camera.setMech( "Bushwacker", MPlayer->colors[player->baseColor[BASECOLOR_PREFERENCE]], color, color );
	camera.zoomIn( 1.5 );


}

char MPPrefs::getColorIndex( unsigned long color )
{
	for ( int i = 0; i < MAX_COLORS; i++ )
	{
		if ( MPlayer->colors[i] == color )
			return i;
	}

	return 0;
}

void MPPrefs::updateBaseColors( const MC2Player* players, long playerCount, bool bDrawRect )
{

	if ( getButton(MP_PREFS_BASE )->isPressed() )
	{
		for( int i = 0; i < playerCount; i++ )
		{
			if ( players[i].commanderID == MPlayer->commanderID )
			{
				for ( int j = FIRST_COLOR_RECT; j < LAST_COLOR_RECT+1; j++ )
				{
					if ( MPlayer->colors[players[i].baseColor[BASECOLOR_PREFERENCE]] == rects[j].getColor() )
					{
						GUI_RECT rect = { rects[j].globalX() - 1, rects[j].globalY() - 1,
							rects[j].right(), rects[j].bottom() };

						if ( bDrawRect )
							drawEmptyRect( rect, 0xffffffff, 0xffffffff );
						break;
					}
				}					
			}
			else
			{
				for ( int j = FIRST_COLOR_RECT; j < LAST_COLOR_RECT+1; j++ )
				{
					if ( MPlayer->colors[players[i].baseColor[BASECOLOR_SELF]] == rects[j].getColor() && bDrawRect )
					{
						statics[21].moveTo( rects[j].globalX(), rects[j].globalY() );
						statics[21].render();
						break;
					}
				}
			}
		}	
	}

	MC2Player* pInfo = MPlayer->getPlayerInfo( MPlayer->commanderID );

	rects[BASE_RECT].setColor(  MPlayer->colors[pInfo->baseColor[BASECOLOR_PREFERENCE]] );

	
	
}

void MPPrefs::updateStripeColors(const MC2Player* players, long playerCount, bool bDrawRect)
{
	if ( getButton(MP_PREFS_STRIPE )->isPressed() )
	{
		for( int i = 0; i < playerCount; i++ )
		{
			if ( players[i].commanderID == MPlayer->commanderID )
			{
				for ( int j = FIRST_COLOR_RECT; j < LAST_COLOR_RECT+1; j++ )
				{
					if ( MPlayer->colors[players[i].stripeColor] == rects[j].getColor() )
					{
						GUI_RECT rect = { rects[j].globalX() - 1, rects[j].globalY() - 1,
							rects[j].right(), rects[j].bottom() };

						if ( bDrawRect )
							drawEmptyRect( rect, 0xffffffff, 0xffffffff );
						break;
					}
				}					
			}
		}	
	}

	MC2Player* pInfo = MPlayer->getPlayerInfo( MPlayer->commanderID );
	rects[STRIPE_RECT].setColor(  MPlayer->colors[pInfo->stripeColor] );
}

void MPPrefs ::render(int OffsetX, int OffsetY )
{
	statics[21].showGUIWindow( 0 );
	LogisticsScreen::render( OffsetX, OffsetY );
	statics[21].showGUIWindow( 1 );

	aObject* pObject = NULL;
	if ( OffsetX == 0 && OffsetY == 0 )
	{

		camera.render();

		for ( int i = 0; i < 3; i++ )
		{
			if ( !comboBox[i].ListBox().isShowing() )			
				comboBox[i].render();
			else 
				pObject = &comboBox[i];
		}

		// x out colors that are already taken

		bool bRect = (pObject == &comboBox[2] || pObject == &comboBox[1]) ? 0 : 1;

		long playerCount;
		const MC2Player* players = MPlayer->getPlayers(playerCount);
		updateBaseColors( players, playerCount, bRect );
		updateStripeColors( players, playerCount, bRect );


		textObjects[helpTextArrayID].render();



	}

	if ( MPlayer && ChatWindow::instance() )
		ChatWindow::instance()->render(OffsetX, OffsetY);

	if ( pObject )
	{
		pObject->render();

		if ( pObject == &comboBox[2] && !ChatWindow::instance()->isExpanded() )
		{
			statics[16].render( );
			statics[17].render( );
		}
	}



}

int			MPPrefs::handleMessage( unsigned long message, unsigned long who )
{
	switch ( who )
	{
	case MB_MSG_NEXT:
		status = DOWN;
		saveSettings( );
		break;

	case MB_MSG_PREV:
		status = DOWN;
		cancelSettings();
		break;

	case MB_MSG_MAINMENU:
		status = MAINMENU;
		break;

	case MP_PREFS_BASE:
	case MP_PREFS_STRIPE:
		{
			getButton( MP_PREFS_BASE )->press( 0 );
			getButton( MP_PREFS_STRIPE )->press( 0 );
			getButton( who )->press( 1 );

		}
		break;


	}

	return 0;
}

void MPPrefs::saveSettings()
{
	// check and see if name has changed
	EString txt;
	comboBox[0].EditBox().getEntry( txt );

	if ( txt != prefs.playerName[0] )
	{
		prefs.setNewName( txt );
	}
	
	MC2Player* pInfo = MPlayer->getPlayerInfo( MPlayer->commanderID );

	strcpy( pInfo->name, txt );

	// check and see if name has changed
	comboBox[1].EditBox().getEntry( txt );

	if ( txt != prefs.unitName[0] )
	{
		prefs.setNewUnit( txt );
	}

	if ( txt )
		strcpy( pInfo->unitName, txt );


	// colors should already be updated by now
	// update insignia
	int index = comboBox[2].GetSelectedItem();
	if ( index != -1 )
	{
		aBmpListItem* pItem = (aBmpListItem*)(comboBox[2].ListBox().GetItem( index ));
		if ( pItem )
		{
			const char* pName = pItem->getBmp();
			strcpy( prefs.insigniaFile, pName );
			strcpy( pInfo->insigniaFile, pName );
		}
	}

//	if ( MPlayer->isHost() )
	{
		prefs.baseColor =  MPlayer->colors[pInfo->baseColor[BASECOLOR_PREFERENCE]];
		prefs.highlightColor = MPlayer->colors[pInfo->stripeColor];
	}

	MPlayer->sendPlayerUpdate( 0, 5, -1 );
	prefs.save();




}

void MPPrefs::cancelSettings()
{
	MC2Player* pInfo = MPlayer->getPlayerInfo( MPlayer->commanderID );
	strcpy( pInfo->name, prefs.playerName[0] );

	strcpy( pInfo->unitName, prefs.unitName[0] );
	strcpy( pInfo->insigniaFile, prefs.insigniaFile );

	MPlayer->sendPlayerUpdate( 0, 5, -1 );
	prefs.save();


}

void MPPrefs::initColors()
{
	for ( int j = FIRST_COLOR_RECT; j < LAST_COLOR_RECT+1; j++ )
	{
		if ( MPlayer )
			MPlayer->colors[j - FIRST_COLOR_RECT] = rects[j].getColor();
	}

}

int aBmpListItem::setBmp( const char* pFileName )
{
	if ( strlen( pFileName ) >= MAXLEN_INSIGNIA_FILE )
		return 0;

	FullPathFileName path;
	path.init(path2insignia, pFileName, ".tga" );

	TGAFileHeader header;
	File file;
	if ( NO_ERR == file.open( path ) )
	{
		file.read( (BYTE*)&header, sizeof( header ) );
		if ( header.width != 32 || header.height != 32 || header.pixel_depth < 24 )
			return 0;
	}
	else
		return 0;

	file.close();
		
	bmp.moveTo( 2, 2 );
	bmp.resize( 32, 32 );
	bmp.setTexture( path );
	bmp.setUVs( 0, 0, 32, 32 );
	bmp.setColor( 0xffffffff );
	
	resize( 64, 36 );

	addChild( &bmp );

	fileName = pFileName;
	return true;
}

void MPPrefs::setMechColors( unsigned long base, unsigned long highlight )
{
	if ( status == RUNNING )
	{
		camera.setMech( "Bushwacker", base, highlight, highlight );
		camera.zoomIn( 1.5 );
	}

}

//*************************************************************************************************
// end of file ( mpprefs.cpp )
