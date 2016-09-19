#define CHATWINDOW_CPP
//===========================================================================//
//ChatWindow.cpp		: Implementation of the ChatWindow component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"chatwindow.h"
#include"mclib.h"
#include"multplyr.h"
#include "..\resource.h"

ChatWindow* ChatWindow::s_instance = NULL;

#define MP_CHAT_HELPTOGGLE	300
#define MP_CHAT_EXPAND		301
#define MP_CHAT_TEAMONLY	302

ChatWindow::ChatWindow(  )
{
	curItem = 0;
	helpTextArrayID = 0;
}

//-------------------------------------------------------------------------------------------------

ChatWindow::~ChatWindow()
{
//	listBox.destroy( );
}

//-------------------------------------------------------------------------------------------------
void ChatWindow::destroy()
{
	if (s_instance)
	{
		delete s_instance;
		s_instance = NULL;
	}
}

//-------------------------------------------------------------------------------------------------

void ChatWindow::init()
{
	s_instance = new ChatWindow;
	s_instance->initInstance();
}

//-------------------------------------------------------------------------------------------------

int ChatWindow::initInstance()
{
	FitIniFile file;
	FullPathFileName path;
	path.init( artPath, "mcl_mp_chatwidget", ".fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char buffer2[512];
		sprintf( buffer2, "couldn't open file %s", (char*)path );
		Assert( 0, 0, buffer2 );
		return false;	

	}

	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button", "Edit" );		

	listBox.init( rects[1].left(), rects[1].top(), rects[1].width(), rects[1].height() );
	listBox.setSpaceBetweenItems( 0 );

	chatWidget.init();

	for ( int i = 0; i < buttonCount; i++ )
	{
		buttons[i].setMessageOnRelease();
		buttons[i].setPressFX( LOG_VIDEOBUTTONS );
		buttons[i].setDisabledFX( LOG_WRONGBUTTON );
		buttons[i].setHighlightFX( LOG_DIGITALHIGHLIGHT );
	}	

	ChatMessageItem item;
	float itemHeight = item.height();

	maxItems = listBox.height()/itemHeight;

	edits[0].limitEntry( 128 );
	edits[0].setSelectedColor( 0xffffffff );

	return true;
}

bool ChatWindow::pointInside( long xPos, long yPos )
{
	if ( getButton( MP_CHAT_EXPAND )->isPressed() )
		return chatWidget.inside( xPos, yPos );

	return LogisticsScreen::inside( xPos, yPos );
}


bool ChatWindow::isExpanded()
{
	return getButton( MP_CHAT_EXPAND )->isPressed();
}
int ChatWindow::handleMessage( unsigned long, unsigned long who )
{
	if ( who == MP_CHAT_HELPTOGGLE )
	{
		if ( getButton( MP_CHAT_HELPTOGGLE )->isPressed() 
			&& getButton( MP_CHAT_EXPAND )->isPressed() )
		{
			getButton( MP_CHAT_EXPAND )->press( 0 );
		}
	}
	else if ( who == MP_CHAT_EXPAND )
	{
		if ( getButton( MP_CHAT_EXPAND )->isPressed() )
			getButton( MP_CHAT_HELPTOGGLE )->press( 0 );
	}
	else if ( who == MP_CHAT_TEAMONLY )
	{
		
		getButton( MP_CHAT_HELPTOGGLE )->press( 0 );

	}
	return 1;
}


void ChatWindow::render( int xOffset, int yOffset )
{
	if ( getButton( MP_CHAT_EXPAND )->isPressed() )
	{
		chatWidget.listBox.move( xOffset, yOffset );
		chatWidget.listBox.render();
		chatWidget.listBox.move( -xOffset, -yOffset );
		textObjects[0].showGUIWindow( 0 );

	}
	else if ( !getButton( MP_CHAT_HELPTOGGLE )->isPressed() )
	{
		listBox.move( xOffset, yOffset );
		listBox.render();
		listBox.move( -xOffset, -yOffset );
		textObjects[0].showGUIWindow( 0 );
	}
	else
		textObjects[0].showGUIWindow( 1 );

	LogisticsScreen::render( xOffset, yOffset );

	
	if ( getButton( MP_CHAT_EXPAND )->isPressed() )
	{
		GUI_RECT rect = { edits[0].globalX(), edits[0].globalY(), listBox.globalRight(), edits[0].globalBottom() };
		drawRect( rect, 0xff000000 );
		chatWidget.render( xOffset, yOffset );
		edits[0].move(xOffset, yOffset );
		edits[0].render();
		edits[0].move(-xOffset, -yOffset );
	}
	else if ( !getButton( MP_CHAT_HELPTOGGLE )->isPressed() )
	{
		GUI_RECT rect = { edits[0].globalX(), edits[0].globalY(), listBox.globalRight(), edits[0].globalBottom() };
		drawRect( rect, 0xff000000 );
		edits[0].move(xOffset, yOffset );
		edits[0].render();
		edits[0].move(-xOffset, -yOffset );
	}

	textObjects[0].render(); // render this last, otherwise it gets cut off

}

void ChatWindow::update()
{
	bool bEnterPressed = gos_GetKeyStatus(KEY_RETURN) == KEY_PRESSED;
	bool bFocused = edits[0].hasFocus();

	if ( getButton( MP_CHAT_EXPAND )->isPressed() )
	{
		chatWidget.update();
		chatWidget.listBox.update();
	}
	else
		listBox.update();


	
	edits[0].showGUIWindow( true );
	if ( getButton( MP_CHAT_HELPTOGGLE )->isPressed() )
	{
		edits[0].showGUIWindow( false );
		rects[2].setColor( rects[2].getColor( ) & 0x00ffffff );
	}
	else
		rects[2].setColor( rects[2].getColor( ) | 0xff000000 );
	

	char* chatTexts[MAX_STORED_CHATS];
	long playerIDs[MAX_STORED_CHATS];

	long count = MAX_STORED_CHATS;
	MPlayer->getChatMessages( chatTexts, playerIDs, count );
	
	refillListBox( listBox, chatTexts, playerIDs, listItems, curItem, count, maxItems );
	refillListBox( chatWidget.listBox, chatTexts, playerIDs, chatWidget.listItems, chatWidget.curItem, count, 128 );


	// now do the same for the chat widget....

	LogisticsScreen::update();

	if ( bEnterPressed )
	{
		if ( bFocused )
		{
			EString text;
			edits[0].getEntry( text );
			int team = getButton( MP_CHAT_TEAMONLY )->isPressed() ? 
				MPlayer->getPlayerInfo( MPlayer->commanderID )->team : -1;
			if ( text.Length() )
			{
				MPlayer->sendChat( 0, team, (char*)(const char*)text );
			}
			edits[0].setEntry( "" );
			edits[0].setFocus( false );
		}
		else
		{
			// repress chat button if appropriate
			if ( getButton( MP_CHAT_HELPTOGGLE )->isPressed() )
				getButton( MP_CHAT_HELPTOGGLE )->press( 0 );
			edits[0].setFocus( true );
		}
	}


}

void ChatWindow::refillListBox( aListBox& listBox, char** chatTexts, long* playerIDs, ChatMessageItem* pItems, long& curItem, long itemCount, long maxCount )
{
	long linesToAdd = 0;
	for ( int i = 0; i < itemCount && i < maxCount; i++ )
	{
		int item = (curItem+i)%maxCount;
		int playerID = playerIDs[i] & 0x00ffffff;
		MC2Player* player = &MPlayer->playerInfo[0];
		for ( int j = 0; j < MAX_MC_PLAYERS; j++ )
		{
			if ( MPlayer->playerInfo[j].commanderID == playerID )
			{
				player = &MPlayer->playerInfo[j];
				break;
			}
		}
		pItems[item].setPlayerColor( MPlayer->colors[ player->baseColor[0] ]); 
		if ( (playerIDs[i] & 0x01000000) )
			pItems[item].setPlayerName( "" );
		else
			pItems[item].setPlayerName( player->name );
		int lineCount = pItems[item].setText( chatTexts[i] );
		linesToAdd += lineCount;
		pItems[item].setTextColor( (playerIDs[i] & 0x10000000)  ? 0xff41c700 : 0xffffffff );
		pItems[item].moveTo( 0, 0 );

	}

	int curLinesInListBox = 0;
	for ( i = 0; i < listBox.GetItemCount(); i++ )
	{
		ChatMessageItem* pItem = (ChatMessageItem*)listBox.GetItem( i );
		curLinesInListBox += pItem->getLineCount();
	}
	if ( curLinesInListBox + linesToAdd > maxCount )
	{
		for ( int i = 0; i < maxCount; i++ )
		{
			ChatMessageItem* pItem = (ChatMessageItem*)listBox.GetItem( 0 );
			if ( pItem )
			{
				curLinesInListBox -= pItem->getLineCount();

				listBox.RemoveItem( listBox.GetItem( 0 ), 0 );
			}
			if ( curLinesInListBox + linesToAdd <= maxCount )
				break;
		}
	}

	for ( i = 0; i < itemCount && i < maxCount; i++ )
	{
		int item = (curItem)%maxCount;
		listBox.AddItem( &pItems[item] );
		if ( maxCount > 2 )
			listBox.SelectItem( listBox.GetItemCount() - 1 );
		curItem++;
	}
}

//-------------------------------------------------------------------------------------------------
ChatWidget::ChatWidget()
{
	curItem = 0;
}

ChatWidget::~ChatWidget()
{
//	listBox.destroy();
}

void ChatWidget::init( )
{

	FitIniFile file;
	FullPathFileName path;
	path.init( artPath, "mcl_mp_chatwidget_up", ".fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char buffer2[512];
		sprintf( buffer2, "couldn't open file %s", (char*)path );
		Assert( 0, 0, buffer2 );
		return;

	}


	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button", "Edit" );
	listBox.init( rects[2].left(), rects[2].top(), rects[2].width(), rects[2].height() );
	listBox.setSpaceBetweenItems( 0 );


}

//-------------------------------------------------------------------------------------------------

ChatMessageItem::ChatMessageItem()

{
	name.font.init( IDS_CHAT_FONT );
	playerText.font.init( IDS_CHAT_FONT );
	playerText.setColor( 0xffffffff );
	name.setColor( 0xffffffff );
	name.move( 1, 1 );
	playerText.move( 0, 1 );
	playerRect.move( 0, 1 );


	resize(140, name.font.height() );

	addChild( &playerRect );
	addChild( &name );
	addChild( &playerText );
	lineCount = 1;
}

void ChatMessageItem::setPlayerName( const char* pName )
{
	name.setText( pName );
	name.moveTo( globalX()+1, globalY()+1 );
	long tmpWidth = name.font.width( pName );
	playerText.moveTo( globalX() + tmpWidth+3, globalY() +1 );

	playerRect.resize( tmpWidth+2, playerText.font.height() - 2 );
	playerText.resize( 437 - playerRect.width() - 2, playerText.font.height()  );
	lineCount = 1;


}

int ChatMessageItem::setText( const char* pText )
{
	playerText.setText( pText );
	lineCount = 1;

	unsigned long height = playerText.font.height( pText, playerText.width() );
	int retVal = 1;
	if ( height > playerText.font.height() * 2 )
	{
		height = playerText.font.height() * 2;
		retVal = 2;
		lineCount = 2;
	}
	playerText.resize( playerText.width(), height );
	resize( width(), height);

	return retVal;
}

void ChatMessageItem::setPlayerColor( long newColor )
{
	playerRect.setColor( newColor );

	if ( ((newColor & 0xff) + ( (newColor & 0xff00)>>8 ) + ( (newColor & 0xff0000)>>16 ))/3 < 85 )
		name.setColor( 0xffffffff );
	else
		name.setColor( 0xff000000 );
}

void ChatMessageItem::setTextColor( long newColor )
{
	playerText.setColor( newColor );
}



//*************************************************************************************************
// end of file ( ChatWindow.cpp )
