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


    //sebi:
    const int resolutions[] = {     640,    1024,       1280,       1600,       1920};
    const char* const str_resolutions[] = { "_640", "_1024",    "_1280",    "_1600",    "_1920"};
    bool b_search_for_best_resolution = true;
    int suitable_res = 0;

    for(int i=0;i<sizeof(resolutions)/sizeof(resolutions[0]);++i) {
        if(Environment.screenWidth == resolutions[i]) {
            suitable_res = i;
            b_search_for_best_resolution = false;
            break;
        }
    }

    if(b_search_for_best_resolution) {
        int min_pos_dist = 10000;
        SPEW(("GRAPHICS", "Not native resolution, selecting closest matching resolution for loading screens"));
        for(int i=0;i<sizeof(resolutions)/sizeof(resolutions[0]);++i) {
            int dst = resolutions[i] - Environment.screenWidth;
            if(dst >=0 && dst < min_pos_dist) {
                min_pos_dist = dst;
                suitable_res = resolutions[i];
            }
        }
    }
    const char* Appendix = str_resolutions[suitable_res];
    //

	FullPathFileName path;

	char fileName[256];
	S_snprintf(fileName, 256, "mcui_keyref%s", Appendix);
    path.init( artPath, fileName, ".fit" );

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
