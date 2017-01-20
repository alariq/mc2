#define KEYBOARDREF_CPP
/*************************************************************************************************\
KeyboardRef.cpp			: Implementation of the KeyboardRef component.
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"keyboardref.h"
#include"mclib.h"
#include "../resource.h"
#include"missiongui.h"
#include"utilities.h"

#define CTRL	0x10000000
#define SHIFT	0x01000000
#define ALT		0x00100000
#define WAYPT	0x20000000


KeyboardRef::KeyboardRef(  )
:	listItemTemplate( IDS_KEYBOARD_REF_FONT ),
	listItemTemplate2( IDS_KEYBOARD_REF_FONT )
{
}

KeyboardRef::~KeyboardRef()
{
	listBox.destroy();
}

//-------------------------------------------------------------------------------------------------

int KeyboardRef::init()
{
	// clear out old stuff first
	clear();

	FullPathFileName path;

	switch( Environment.screenWidth )
	{
	case 640:
		path.init( artPath, "mcui_keyref_640", ".fit" );
			break;

	case 800:
		path.init( artPath, "mcui_keyref_800", ".fit" );
		break;

	case 1024:
		path.init( artPath, "mcui_keyref_1024", ".fit" );
		break;
	case 1280:
		path.init( artPath, "mcui_keyref_1280", ".fit" );
		break;
	case 1600:
		path.init( artPath, "mcui_keyref_1600", ".fit" );
		break;
	case 1920:
		path.init( artPath, "mcui_keyref_1920", ".fit" );
		break;
	default:
		gosASSERT( !"Invalid resolution" );
		return -1;

	}

	FitIniFile file;
	file.open( path );


	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button" );

	listBox.init( rects[2].left(), rects[2].top(), rects[2].width(), rects[2].height() );

	file.close();


	path.init( artPath, "mcui_keyref_entry", ".fit" );
	file.open( path );

	listItemTemplate.init( file, "Text0" );
	listItemTemplate2.init( file, "Text1" );

	buttons[0].setMessageOnRelease( );


	return true;
}

void KeyboardRef::update()
{
	listBox.update();
	LogisticsScreen::update();
}

void KeyboardRef::render()
{
	GUI_RECT rect = { 0, 0, Environment.screenWidth, Environment.screenHeight };
	drawRect( rect, 0xff000000 );

	listBox.render();
	LogisticsScreen::render();
}

void KeyboardRef::reseed( MissionInterfaceManager::Command* commands )
{
	listBox.removeAllItems( true );

	char shift[32];
	char control[32];
	char alt[32];
	char descText[128];
	char keysString[128];

	cLoadString( IDS_SHIFT, shift, 31 );
	cLoadString( IDS_CONTROL, control, 31 );
	cLoadString( IDS_ALT, alt, 31 );

	// first count the number of hotTexts
	long count = 0;
	for ( int i = 0; i < MAX_COMMAND; i++ )
	{
		if ( commands[i].hotKeyDescriptionText != -1 )
		{
			count++;
		}
	}

	long curCount = 0;

	for (int i = 0; i < MAX_COMMAND; i++ )
	{
		if ( commands[i].hotKeyDescriptionText != -1 )
		{
			cLoadString( commands[i].hotKeyDescriptionText, descText, 127 );
			long key = commands[i].key;
			const char* pKey = gos_DescribeKey( (key & 0x000fffff) << 8 );
			strcpy( keysString, pKey );

			if ( ((key & SHIFT)) )
			{
				strcat( keysString, " + " );
				strcat( keysString, shift );
			}
		
			if ( ((key & CTRL)) )
			{
				strcat( keysString, " + " );
				strcat( keysString, control );
			}
				
			if ( ((key & ALT)) )
			{
				strcat( keysString, " + " );
				strcat( keysString, alt );
			}

			aTextListItem* item = new aTextListItem( IDS_KEYBOARD_REF_FONT );
			*item = listItemTemplate;
			item->setText( keysString );
			item->setAlignment( 1 );

			listBox.AddItem( item );

			long yVal = item->y();

			item = new aTextListItem( IDS_KEYBOARD_REF_FONT );
			*item = listItemTemplate2;
			long xVal = listItemTemplate2.left();
			item->setText( descText );
			item->setAlignment( 0 );

			listBox.AddItem( item );

			item->moveTo( xVal + listBox.left(), yVal );



		}
	}
}

int	KeyboardRef::handleMessage( unsigned long who, unsigned long )
{
	return MissionInterfaceManager::instance()->toggleHotKeys();
}



//*************************************************************************************************
// end of file ( KeyboardRef.cpp )
