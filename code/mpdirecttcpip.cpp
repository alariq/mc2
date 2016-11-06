#define MPDIRECTTCPIP_CPP
/*************************************************************************************************\
MPDirectTcpip.cpp			: Implementation of the MPDirectTcpip component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mpdirecttcpip.h"
#include"prefs.h"
#include"inifile.h"
#include"../userinput.h"
#include "..\resource.h"

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#define CHECK_BUTTON 200

static int connectionType = 0;

static const int FIRST_BUTTON_ID = 1000010;
static const int OK_BUTTON_ID = 1000001;
static const int CANCEL_BUTTON_ID = 1000002;

MPDirectTcpip::MPDirectTcpip()
{
	bDone = 0;

	status = RUNNING;
}

MPDirectTcpip::~MPDirectTcpip()
{
}

int MPDirectTcpip::indexOfButtonWithID(int id)
{
	int i;
	for (i = 0; i < buttonCount; i++)
	{
		buttons[i].setMessageOnRelease();
		if (buttons[i].getID() == id)
		{
			return i;
		}
	}
	return -1;
}

void MPDirectTcpip::init(FitIniFile* file)
{
	LogisticsScreen::init( *file, "Static", "Text", "Rect", "Button" );

	if ( buttonCount )
	{
		for ( int i = 0; i < buttonCount; i++ )
		{
			if (buttons[i].getID() == 0)
			{
				buttons[i].setID(FIRST_BUTTON_ID + i);
			}
		}
	}


	{
		char path[256];
		strcpy( path, artPath );
		strcat( path, "mcl_mp_tcpip_combobox0.fit" );
		
		FitIniFile PNfile;
		if ( NO_ERR != PNfile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", path );
			Assert( 0, 0, error );
			return;
		}
		ipAddressComboBox.init(&PNfile, "TCIPNumberComboBox");

		aStyle7TextListItem *pTmp2;
		int i;
		for (i = 0; i < 15; i += 1)
		{
			pTmp2 = new aStyle7TextListItem;
			EString tmpStr;
			tmpStr.Format("Text0");
			pTmp2->init(&PNfile, tmpStr.Data());
			ipAddressComboBox.AddItem(pTmp2);
		}
		ipAddressComboBox.SelectItem(0);
	}
}

void MPDirectTcpip::begin()
{
	status = RUNNING;
}


void MPDirectTcpip::end()
{
}

void MPDirectTcpip::render(int xOffset, int yOffset )
{
	if ((0 == xOffset) && (0 == yOffset))
	{
		ipAddressComboBox.render();
	}

	LogisticsScreen::render(xOffset, yOffset);
}

void MPDirectTcpip::render()
{
	render(0, 0);
}

int	MPDirectTcpip::handleMessage( unsigned long message, unsigned long who)
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
				status = PREVIOUS;
			}
			break;
		case 50/*MB_MSG_NEXT*/:
			{
				getButton( 50/*MB_MSG_NEXT*/ )->press( 0 );
				//status = NEXT;
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
				status = SKIPONENEXT;
				return 1;
			}
			break;
		}
	}

	return 0;

}

bool MPDirectTcpip::isDone()
{
	return bDone;
}

void MPDirectTcpip::update()
{
	LogisticsScreen::update();
	ipAddressComboBox.update();

	helpTextID = 0;
	helpTextHeaderID = 0;

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



long aStyle7TextListItem::init( FitIniFile* file, const char* blockName )
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

void aStyle7TextListItem::render()
{
	float color;
	if (aListItem::SELECTED == getState())
	{
		color = 0.33 * ((unsigned long)normalColor) + 0.67 * ((unsigned long)0xffffffff);
	}
	else if (aListItem::HIGHLITE == getState())
	{
		color = 0.67 * ((unsigned long)normalColor) + 0.33 * ((unsigned long)0xffffffff);
	}
	else
	{
		color = normalColor;
	}
	aTextListItem::setColor((unsigned long)color);

	aTextListItem::render();
}

//////////////////////////////////////////////



//*************************************************************************************************
// end of file ( MPDirectTcpip.cpp )
