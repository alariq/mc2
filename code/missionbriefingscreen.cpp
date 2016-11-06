#define MISSIONBRIEFINGSCREEN_CPP
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"missionbriefingscreen.h"
#include"mechbayscreen.h"
#include"logisticsdata.h"
#include"inifile.h"
#include"packet.h"
#include "../resource.h"
#include"objective.h"
#include"multplyr.h"
#include"chatwindow.h"
#include"gamesound.h"


#define MAP_INDEX 32
#define BUTTON_TEXT 34
#define VIDEO_SCREEN 33
#define RP_INDEX 1

#define MN_MSG_PLAY 80
#define MN_MSG_STOP 82
#define MN_MSG_PAUSE 81


MissionBriefingScreen::MissionBriefingScreen(  )
{
	status = RUNNING;
	LogisticsScreen::helpTextArrayID = 4;

	for ( int i = 0; i < MAX_OBJECTIVES; i++ )
		objectiveButtons[i] = NULL;
}

MissionBriefingScreen::~MissionBriefingScreen()
{
	for ( int i = 0; i < MAX_OBJECTIVES; i++ )
	{
		if (objectiveButtons[i])
		{
			delete objectiveButtons[i]; 
			objectiveButtons[i] = NULL;
		}
	}

	missionListBox.destroy();  //NO Default destructor for aListBox.  Just call destroy.
}

//-------------------------------------------------------------------------------------------------


void MissionBriefingScreen::init( FitIniFile* file )
{
	LogisticsScreen::init( *file, "Static", "Text", "Rect", "Button" );
	for ( int i= 0; i < buttonCount; i++ )
		buttons[i].setMessageOnRelease();

	missionListBox.init( rects[1].left(), rects[1].top(),
						rects[1].width(), rects[1].height() );

	missionListBox.setPressFX( -1 );
	missionListBox.setHighlightFX( -1 );
	missionListBox.setDisabledFX( -1 );

	getButton( MN_MSG_PLAY )->showGUIWindow( 0 );
	getButton( MN_MSG_STOP )->showGUIWindow( 0 );
	getButton( MN_MSG_PAUSE )->showGUIWindow( 0 );
	camera.init( statics[VIDEO_SCREEN].left(), statics[VIDEO_SCREEN].top(), 
		statics[VIDEO_SCREEN].right(), statics[VIDEO_SCREEN].bottom() );


}

void MissionBriefingScreen::render(int xOffset, int yOffset )
{
	missionListBox.move(xOffset, yOffset );
	missionListBox.render();
	missionListBox.move( -xOffset, -yOffset);

	camera.render();

	LogisticsScreen::render( xOffset, yOffset );

	for ( int i = 0; i < MAX_OBJECTIVES; i++ )
	{
		if ( objectiveButtons[i] )
			objectiveButtons[i]->render(xOffset, yOffset);
	}

	if ( !MPlayer )
		dropZoneButton.render(xOffset, yOffset);

	if ( MPlayer && ChatWindow::instance() )
		ChatWindow::instance()->render(xOffset, yOffset);



	

}

void MissionBriefingScreen::update()
{

	if ( MPlayer || LogisticsData::instance->isSingleMission() )
	{
		getButton( MB_MSG_PREV )->disable( true );

	}
	else
		getButton( MB_MSG_PREV )->disable( false );

	int oldSel = missionListBox.GetSelectedItem();
	missionListBox.update();

	if ( userInput->isLeftClick() )
	{
		bClicked = true;
		for ( int i = 0; i < MAX_OBJECTIVES; i++ )
		{
			if ( objectiveButtons[i] && 
				objectiveButtons[i]->pointInside( userInput->getMouseX(), userInput->getMouseY () ) )
			{
				// find the item that has this objective
				for ( int j = 0; j < missionListBox.GetItemCount(); j++ )
				{
					if ( missionListBox.GetItem( j )->getID() == i )
					{
						missionListBox.SelectItem( j );
						break;
					}
				}
			}
	
		}
	}

	runTime += frameLength;
	
	long selItem = missionListBox.GetSelectedItem( );
	int ID = -1;

	if ( selItem != -1 )
		ID = missionListBox.GetItem( selItem )->getID();

	if ( selItem != -1 && oldSel != selItem )
	{
		bClicked = true;
		

		// set old selections back to white
		for ( int i= 0; i < missionListBox.GetItemCount(); i++ )
		{
			if ( missionListBox.GetItem( i )->getID() != -1 )
			{
				missionListBox.GetItem( i )->setColor( 0xffffffff );
				if ( objectiveButtons[missionListBox.GetItem( i )->getID()] )
					objectiveButtons[missionListBox.GetItem( i )->getID()]->setColor( 0xffffffff );
			}
		}

		if ( ID != -1  )
		{
			if ( objectiveModels[ID].Length() )
			{
				camera.setObject( objectiveModels[ID], modelTypes[ID], modelColors[ID][0],
					modelColors[ID][1], modelColors[ID][2] );
				camera.setScale( modelScales[ID] );
				soundSystem->playDigitalSample( LOG_VIDEOBUTTONS );
				statics[35].showGUIWindow( 0 );
			}
			else
			{
				camera.setObject( NULL, -1 );
				statics[35].showGUIWindow( true );
			}
		
			missionListBox.GetItem( selItem )->setColor( 0xffff0000 );

			if ( objectiveButtons[ID] )
				objectiveButtons[ID]->setColor( 0xffff0000 );
		}

		
	}
	

	if ( !bClicked && runTime > 3.0 ) // every second switch selection until user clicks
	{
		runTime = 0;

		// turn old sel back into white
		if ( selItem != -1 && ID != -1 )
		{
			missionListBox.GetItem( selItem )->setColor( 0xffffffff );
			if ( objectiveButtons[ID] )
				objectiveButtons[ID]->setColor( 0xffffffff );
		}
	
		selItem++;
		
		// wrap if necessary
		if ( selItem >= missionListBox.GetItemCount() )
			selItem = 0;

		// find next viable item
		while( true )
		{
			if ( selItem >= missionListBox.GetItemCount() )
			{
				selItem = -1;
				break;
			}
			else if ( missionListBox.GetItem(selItem)->getID() == -1 )
				selItem++;
		
			else
				break;
		}

		missionListBox.SelectItem( selItem );
		

		if ( selItem != -1 )
		{
			int ID = missionListBox.GetItem(selItem)->getID();
			if ( ID != -1 )
			{
				missionListBox.GetItem( selItem )->setColor( 0xffff0000 );
				if ( objectiveButtons[ID] )
					objectiveButtons[ID]->setColor( 0xffff0000 );
				camera.setObject( objectiveModels[ID], modelTypes[ID], modelColors[ID][0], 
					modelColors[ID][1], modelColors[ID][2]);
				camera.setScale( modelScales[ID] );
				if ( objectiveModels[ID].Length() )
					statics[35].showGUIWindow( 0 );
				else
					statics[35].showGUIWindow( 1 );
			}
			else
			{
				camera.setObject( NULL, -1 );

				statics[35].showGUIWindow( 1 );
			}
		}
	}

	camera.update();

	if ( !MPlayer || !ChatWindow::instance()->pointInside(userInput->getMouseX(), userInput->getMouseY()) )
		LogisticsScreen::update();

	 if ( MPlayer && ChatWindow::instance() )
	 { 
		 if ( ChatWindow::instance()->pointInside(userInput->getMouseX(), userInput->getMouseY()) )
			textObjects[helpTextArrayID].setText( "" );

		ChatWindow::instance()->update();
	 }



	
}

long	MissionBriefingScreen::getMissionTGA( const char* missionName )
{
	if ( !missionName )
		return 0;

	// do I need to open the file?  I guess so, if this proves too slow,
	// it could be done when a stage is completed
	FullPathFileName path;
	path.init( missionPath, missionName, ".pak" );

    // sebi: check that file was removed? NB!
#ifndef LINUX_BUILD
	if ( 1 == fileExists( path ) )
	{

		// big hack here for some reason we can open files while they're being transferred.
		HANDLE hFile = CreateFile( path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0 );

		int error = GetLastError();

		if ( hFile == INVALID_HANDLE_VALUE )
			return 0;

		CloseHandle( hFile );
	}
#endif

	// read the tga out of the pak file
	PacketFile file;
	if ( NO_ERR == file.open( path ) ) // in case file has just been created
	{
		if ( file.getNumPackets() > 3 )
		{
	
			file.seekPacket(3);
			long size = file.getPacketSize( );
			BYTE* mem = new BYTE[size];

			file.readPacket( 3, mem );
			
			TGAFileHeader* pHeader = (TGAFileHeader*)mem;
			long bmpWidth = pHeader->width;
			long bmpHeight = pHeader->height;
			flipTopToBottom( (BYTE*)(pHeader + 1), pHeader->pixel_depth, bmpWidth, bmpHeight );
			
			// set up the texture
			long tmpMapTextureHandle = mcTextureManager->textureFromMemory( (DWORD*)(pHeader+1), gos_Texture_Solid, 0, bmpWidth );

			delete mem;

			return tmpMapTextureHandle;
		}
	}

	return 0;


}

void MissionBriefingScreen::begin()
{
	missionListBox.removeAllItems( true );

	runTime = 0;
	bClicked = 0;

	statics[VIDEO_SCREEN].setColor( 0 );

	memset( objectiveButtons, 0, sizeof ( aObject* ) * MAX_OBJECTIVES );
	// need to set up all pertinent mission info
	EString missionName = LogisticsData::instance->getCurrentMission();


	long tmpMapTextureHandle = getMissionTGA( missionName );
	statics[MAP_INDEX].setTexture( tmpMapTextureHandle );
	statics[MAP_INDEX].setUVs( 0, 127, 127, 0 );
	statics[MAP_INDEX].setColor( 0xffffffff );


	// need to get all the objectives and stuff
	FullPathFileName fitPath;
	fitPath.init( missionPath, missionName, ".fit" );
	FitIniFile fitFile;
	fitFile.open( fitPath );

	// put initial divider in list box
	addItem(IDS_MN_DIVIDER, 0xff005392, -1);

	fitFile.seekBlock( "MissionSettings" );

	long result = fitFile.seekBlock( "MissionSettings" );
	Assert( result == NO_ERR, 0, "Coudln't find the mission settings block in the mission file" );

	bool bRes;
	result = fitFile.readIdBoolean( "MissionNameUseResourceString", bRes );
	Assert( result == NO_ERR, 0, "couldn't find the MissionNameUseResourceString" );
	if ( bRes )
	{
		unsigned long ulRes;
		result = fitFile.readIdULong( "MissionNameResourceStringID", ulRes );
		Assert( result == NO_ERR, 0, "couldn't find the MissionNameResourceStringID" );
		addItem(ulRes, 0xff005392, -1);
	}
	else
	{
		char missionName[256];
		fitFile.readIdString( "MissionName", missionName, 255 );
		addLBItem( missionName, 0xff005392, -1 );

	}

	addItem(IDS_MN_DIVIDER, 0xff005392, -1);
	addItem( IDS_MN_MISSION_OBJECTIVES, 0xff005392, -1 );
	addItem(IDS_MN_DIVIDER, 0xff005392, -1);

	// put in primary objectives
	fitFile.seekBlock( "Team0Objectives" );
	unsigned long objectiveCount;
	fitFile.readIdULong( "NumObjectives", objectiveCount );
	bool bHasSecondary = 0;
	int count = 0; 

	fitFile.seekBlock( "Terrain" );
	float terrainExtentX;
	float terrainExtentY;
	fitFile.readIdFloat( "TerrainMinX", terrainExtentX );
	if ( !terrainExtentX )
		terrainExtentX = 120 * 128;
	fitFile.readIdFloat( "TerrainMinY", terrainExtentY );
	if ( !terrainExtentY )
		terrainExtentY = 120 * 128;

	CObjectives Objectives(0/*alignment*/);
	/*Note that ObjectManager is probably NULL as these objectives are read, so it's not
	cool to call any of the Status() functions of this instance of objectives (access violation
	may ensue).*/
	Objectives.Read(&fitFile);

	gosASSERT( Objectives.Count() < MAX_OBJECTIVES );

	int buttonCount = 0;

	for ( int j = 1; j < 3; j++ )
	{
		CObjectives::EIterator it = Objectives.Begin();
		buttonCount = 0;
		for ( int i = 0; i < Objectives.Count(); i++, it++ )
		{
			CObjective *pObjective = (*it);
			if ( (!pObjective->IsHiddenTrigger()) && (pObjective->IsActive()) )
			{
				
				if ( pObjective->Priority() == j )
				{
					addObjectiveButton( pObjective->MarkerX(), pObjective->MarkerY(), buttonCount,pObjective->Priority(), fabs(terrainExtentX),
									fabs(terrainExtentY), pObjective->DisplayMarker());

					if ( j == 0 )
					{
						bHasSecondary = true;
						if ( i == 0 )
							addItem( IDS_MN_DIVIDER, 0xff005392, -1 );
					}

					addLBItem( (pObjective->LocalizedDescription()).Data(), 0xffffffff, count );

					objectiveModels[count] = (pObjective->ModelName()).Data();
					modelTypes[count] = pObjective->ModelType();

					modelColors[count][0] = pObjective->ModelBaseColor();
					modelColors[count][1] = pObjective->ModelHighlightColor();
					modelColors[count][2] = pObjective->ModelHighlightColor2();
					modelScales[count] = pObjective->ModelScale();

					count++;
					buttonCount++;
				}
			}
		}
	}
	
	addItem( IDS_MN_DIVIDER, 0xff005392, -1 );

	fitFile.seekBlock( "MissionSettings" );

	char blurb[4096];
	result = fitFile.readIdString("Blurb", blurb, 4095 );

	bool tmpBool = false;
	result = fitFile.readIdBoolean("BlurbUseResourceString", tmpBool);
	if (NO_ERR == result && tmpBool )
	{
		unsigned long tmpInt = 0;
		result = fitFile.readIdULong("BlurbResourceStringID", tmpInt);
		if (NO_ERR == result)
		{
			cLoadString( tmpInt, blurb, 2047 );
		}
	}


	addLBItem( blurb, 0xff005392, -1 );

	int RP = LogisticsData::instance->getCBills();
	char text[32];
	sprintf( text, "%ld ", RP );
	textObjects[RP_INDEX].setText( text );

	// need to find a drop zone, because our designers were never convinced to place
	// 'em explicitly, we need to do it for them
	int i = 1;
	while( true )
	{
		char blockName[32];
		sprintf( blockName, "Part%ld", i );
		i++;
		if ( NO_ERR != fitFile.seekBlock( blockName ) )
			break;
		
		bool bPlayer = 0;
		fitFile.readIdBoolean( "PlayerPart", bPlayer );

		if ( bPlayer )
		{
			float fX;
			float fY;

			fitFile.readIdFloat( "PositionX", fX );
			fitFile.readIdFloat( "PositionY", fY );

			setupDropZone( fX, fY, fabs(terrainExtentX), fabs(terrainExtentY) );
		
			break;
		}
	}

	statics[BUTTON_TEXT].showGUIWindow( 0 );
	status = RUNNING;

}

void MissionBriefingScreen::setupDropZone( float fX, float fY, float mapWidth, float mapHeight )
{
	dropZoneButton = statics[BUTTON_TEXT];

	float bmpX = statics[MAP_INDEX].width();
	float bmpY = statics[MAP_INDEX].height();

	// in terms of map, this is where the button goes
	float xLoc = fX/mapWidth * bmpX/2.f;
	float yLoc = -fY/mapHeight * bmpY/2.f;

	// offset by the map...
	xLoc += statics[MAP_INDEX].globalX() + bmpX/2.f;
	yLoc += statics[MAP_INDEX].globalY() + bmpY/2.f;

	dropZoneButton.resize( 18, 18 );
	dropZoneButton.setUVs( 100, 100, 118, 118);

	dropZoneButton.moveTo( xLoc, yLoc );
	dropZoneButton.showGUIWindow( true );


}

void MissionBriefingScreen::addObjectiveButton( float fX, float fY, int count, int priority,
											   float mapWidth, float mapHeight, bool display )
{
	float lineOffset = 0;
	if ( priority == 1 )
	{
		count += 12;
	}



	float bmpX = statics[MAP_INDEX].width();
	float bmpY = statics[MAP_INDEX].height();

	// in terms of map, this is where the button goes
	float xLoc = fX/mapWidth * bmpX/2.f;
	float yLoc = -fY/mapHeight * bmpY/2.f;

	// offset by the map...
	xLoc += statics[MAP_INDEX].globalX() + bmpX/2.f;
	yLoc += statics[MAP_INDEX].globalY() + bmpY/2.f;

	aObject* pButtonText = new aObject;
	*pButtonText = statics[BUTTON_TEXT];
	if (display)
		pButtonText->showGUIWindow( true );
	else
		pButtonText->showGUIWindow( false );

	// need to reset the uv's based on count....
	float textWidth = pButtonText->width();
	float textHeight = pButtonText->height();

	lineOffset *= textHeight;
 
	int itemsPerLine = 128/textWidth;

	int iIndex = count % itemsPerLine;
	int jIndex = count / itemsPerLine;

	pButtonText->setUVs( iIndex * textWidth, jIndex * textHeight + lineOffset,
						 (iIndex+1) * textWidth, (jIndex+1) * textHeight + lineOffset );

	pButtonText->moveTo( xLoc, yLoc );
	int i = 0;
	for (; i < MAX_OBJECTIVES; i++ )
	{
		if ( !objectiveButtons[i])
		{
			objectiveButtons[i] = pButtonText; 
			break;
		}
	}

	//If there are too many objectives, don't leave the RAM lying around!
	if (i >= MAX_OBJECTIVES)
	{
		delete pButtonText;
		pButtonText = NULL;
	}
}

/*int MissionBriefingScreen::addLBItem( FitIniFile& file, const char* itemName, unsigned long color, int ID)
{
	char buffer[1024];
	file.readIdString( itemName, buffer, 1023 );
	return addLBItem( buffer, color, ID);
}*/

int MissionBriefingScreen::addLBItem( const char* text, unsigned long color, int ID)
{
	aTextListItem* pEntry = new aTextListItem( IDS_MN_LB_FONT );
	pEntry->setID( ID );
	pEntry->resize( missionListBox.width() - missionListBox.getScrollBarWidth() - 10,
		pEntry->height() );
	pEntry->setText( text );
	pEntry->setColor( color );
	pEntry->sizeToText();
	pEntry->forceToTop(true);
	return missionListBox.AddItem( pEntry );

}

int  MissionBriefingScreen::addItem( int ID, unsigned long color, int LBid)
{
	
	aTextListItem* pEntry = new aTextListItem( IDS_MN_LB_FONT );
	pEntry->setID( LBid );
	pEntry->resize( missionListBox.width() - missionListBox.getScrollBarWidth() - 10,
		pEntry->height() );
	pEntry->setText( ID );
	pEntry->setColor( color );
	return missionListBox.AddItem( pEntry );

}


void MissionBriefingScreen::end()
{
//	statics[MAP_INDEX].setTexture( (long)0 );
// 	statics[MAP_INDEX].setColor( 0 );
	camera.setMech( NULL );

}

int MissionBriefingScreen::handleMessage( unsigned long msg, unsigned long who )
{
	switch( who )
	{
	case MB_MSG_NEXT:
		status = NEXT;
		break;

	case MB_MSG_PREV:
		status = PREVIOUS;
		break;

	case MN_MSG_PLAY:
		break;
	case MN_MSG_STOP:
		break;

	case MN_MSG_PAUSE:
		break;

	case MB_MSG_MAINMENU:
		status = MAINMENU;
		break;
	}

	return 0;

}




//*************************************************************************************************
// end of file ( MissionBriefingScreen.cpp )
