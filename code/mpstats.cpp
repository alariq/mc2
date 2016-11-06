#define MPSTATS_CPP
/*************************************************************************************************\
MPStats.cpp			: Implementation of the MPStats component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mpstats.h"
#include"../resource.h"
#include"missionbriefingscreen.h"
#include"chatwindow.h"
#include"mechbayscreen.h"
#include"logisticsdialog.h"

#define MP_STATS_SAVE	400


MPStats::MPStats(  )
{
	helpTextArrayID = 8;
	status = -1;
	bHostLeftDlg = 0;
}

//-------------------------------------------------------------------------------------------------

MPStats::~MPStats()
{
}



int MPStats::init()
{
	FitIniFile file;
	FullPathFileName path;
	path.init( artPath, "mcl_mp_stats", ".fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char buffer2[512];
		sprintf( buffer2, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, buffer2 );
		return false;	

	}

	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button", "Edit" );	
	
	entries[0].init( );
	entries[0].moveTo( rects[1].globalX(), rects[1].globalY() );
	entries[0].resize( rects[1].width(), rects[1].height() );

	for ( int i = 0; i < MAX_MC_PLAYERS-1; i++ )
	{
		entries[i+1] = entries[0];
		entries[i+1].move( 0, (i+1) * (entries[0].height()+1) );
	}

	for (int i = 0; i < buttonCount; i++ )
		buttons[i].setMessageOnRelease();

	bSavingStats = 0;

	bHostLeftDlg = 0;

	return true;
}

int __cdecl sortStats( const void* pPlayer1, const void* pPlayer2 )
{
	MC2Player* player1 = *(MC2Player**)pPlayer1;
	MC2Player* player2 = *(MC2Player**)pPlayer2;

	
	if ( player1->rank > player2->rank )
		return 1;
	else if ( (player1)->rank < (player2)->rank )
		return -1;

	//else if ( (player1)->kills > (player2)->kills )
	//	return 1;

	//else if ( (player1)->losses > (player2)->losses )
	//	return 1;


	return 0;
}

void MPStats::end()
{
	MPlayer->setInProgress(false);
	bSavingStats = 0;
}

void MPStats::begin()
{

	MPlayer->setMode( MULTIPLAYER_MODE_RESULTS );
	bSavingStats = 0;
	status = RUNNING;
	beginFadeIn( .5 );


	for ( int i = 0; i < MAX_MC_PLAYERS; i++ )
		entries[i].showGUIWindow( 0 );


	statics[15].setColor( 0 );

	// need to set up map name

	bool bRes = 0;
	char text2[256];
	char text[256];

	cLoadString( IDS_MP_LM_MAP_LIST_MAP_NAME, text, 255 );
	sprintf( text2, text, MPlayer->missionSettings.name );
	textObjects[1].setText( text2 );

	unsigned long type = MPlayer->missionSettings.missionType ;
	cLoadString( IDS_MP_LM_MAP_LIST_TYPE, text, 255 );
	char mType[128];
	cLoadString( IDS_MP_LM_TYPE0 + type, mType, 127 );
	
	sprintf( text2, text, mType );
	textObjects[6].setText( text2 );

	unsigned long numPlayers = MPlayer->missionSettings.maxPlayers;

	cLoadString( IDS_MP_LM_MAP_LIST_MAX_PLAYERS, text, 255 );
	sprintf( text2, text, numPlayers );
	textObjects[7].setText( text2 );



}

int MPStats::handleMessage( unsigned long what, unsigned long who )
{
	if ( who == MP_STATS_SAVE )
	{
		LogisticsVariantDialog::instance()->beginTranscript();

		bSavingStats = true;

	}

	else if ( who == MB_MSG_NEXT )
	{
		status = NEXT;
		end();
		beginFadeOut(.5f);
		statics[15].setTexture( (unsigned long)0 );
		statics[15].setColor( 0 );
	}

	return 1;
}


void MPStats::render( int xOffset, int yOffset )
{

	LogisticsScreen::render( xOffset, yOffset );

	for ( int i = 0; i < MAX_MC_PLAYERS; i++ )
	{
		entries[i].render( 0, 0 );
	}

	ChatWindow::instance()->render(xOffset, yOffset);

	if ( bSavingStats )
	{
		LogisticsVariantDialog::instance()->render();
	}

	if ( bHostLeftDlg )
	{
		LogisticsOneButtonDialog::instance()->render();
	}


}

void MPStats::update()
{
	if ( status == RUNNING && !statics[15].getColor() )
	{
		long textureHandle = MissionBriefingScreen::getMissionTGA( MPlayer->missionSettings.map );

		statics[15].setTexture( textureHandle );
		statics[15].setUVs( 0, 127, 127, 0 );
		statics[15].setColor( 0xffffffff );

		long playerCount = 0;
		const MC2Player* players = MPlayer->getPlayers(playerCount);
		const MC2Player* sorted[MAX_MC_PLAYERS];
		int winnerCount = 0;
		for ( int j = 0; j < playerCount; j++ )
		{
			sorted[j] = &players[j];
			if ( sorted[j]->rank == 1 )
				winnerCount++;
		}

		bool scoreShown[MAX_MC_PLAYERS]; // keep track of whose shown the score
		memset( scoreShown, 0, sizeof( bool ) * MAX_MC_PLAYERS );


		unsigned long winnerColor = 0xffFFCC00; // gold
		if ( winnerCount > 1 )
			winnerColor = 0xffA6A6A6;

		qsort( sorted, playerCount, sizeof( MC2Player*), sortStats );

	

		for ( int i = 0; i < MAX_MC_PLAYERS; i++ )
		{
			if ( i < playerCount )
			{
				entries[i].setData( sorted[i], winnerColor, !scoreShown[sorted[i]->team] );
				scoreShown[sorted[i]->team] = true;
				entries[i].showGUIWindow( true );
			}
			else
				entries[i].showGUIWindow( 0 );
		}
	}

	if ( bSavingStats )
	{
		LogisticsVariantDialog::instance()->update();
		if ( LogisticsVariantDialog::instance()->getStatus() == YES )
		{
			EString str = LogisticsVariantDialog::instance()->getFileName();

			FullPathFileName oldPath;
			FullPathFileName newPath;
			newPath.init( "data\\multiplayer\\transcripts\\", str, ".txt");
			oldPath.init( "data\\multiplayer\\transcripts\\", "transcript", ".txt" );

			LogisticsVariantDialog::instance()->end();
			
			CopyFile(oldPath, newPath,0);

			bSavingStats = 0;
		}
		else if ( LogisticsVariantDialog::instance()->getStatus() == NO )
		{
			if ( LogisticsVariantDialog::instance()->isDone() )
				bSavingStats = 0;
		}

	}
	else if ( bHostLeftDlg )
	{
		LogisticsOneButtonDialog::instance()->update();
		if ( LogisticsOneButtonDialog::instance()->isDone() )
		{
			LogisticsOneButtonDialog::instance()->end();
			bHostLeftDlg = 0;
		}
	}
	else if ( !ChatWindow::instance()->pointInside(userInput->getMouseX(), userInput->getMouseY()) )
			LogisticsScreen::update();

	if ( ChatWindow::instance() )
		ChatWindow::instance()->update();

}

void MPStats::setHostLeftDlg( const char* hostName )
{
	char leaveStr[256];
	char formatStr[256];

	cLoadString( IDS_PLAYER_LEFT, leaveStr, 255 );
	sprintf( formatStr, leaveStr, hostName );

	LogisticsOneButtonDialog::instance()->setText( IDS_PLAYER_LEFT,
					IDS_DIALOG_OK, IDS_DIALOG_OK );
	LogisticsOneButtonDialog::instance()->setText( formatStr );

	LogisticsOneButtonDialog::instance()->begin();
	bHostLeftDlg = true;
}

//*************************************************************************************************
MPStatsEntry::MPStatsEntry()
{
	overlayColor = 0;
}

void MPStatsEntry::render( int x, int y )
{

	LogisticsScreen::render( x, y );
	
}

MPStatsEntry::~MPStatsEntry()
{
}

void MPStatsEntry::init()
{
	FitIniFile file;
	FullPathFileName path;
	path.init( artPath, "mcl_mp_scoreboard", ".fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char buffer2[512];
		sprintf( buffer2, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, buffer2 );
		return;	

	}

	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button", "Edit" );	

	aObject::init( rects[2].left(), rects[2].top(), rects[5].right() - rects[0].left(), rects[2].height() );

	overlayColor = 0; 



}

void MPStatsEntry::setData(const MC2Player* data, bool bShowScore )
{
	long color = data ? MPlayer->colors[data->baseColor[BASECOLOR_TEAM]] : 0x00000000;
	long color2 = data ? MPlayer->colors[data->stripeColor] : 0x00000000;

	long scoreColor = 0xffffffff;

	if ( data && MPlayer->allUnitsDestroyed[data->commanderID] )
		scoreColor = 0xff7f7f7f;

	rects[0].setColor( color2 );
	rects[1].setColor( color );

	long textColor = 0xff000000;

	if ( ((color & 0xff) + ( (color & 0xff00)>>8 ) + ( (color & 0xff0000)>>16 ))/3 < 85 )
		textColor = 0xffffffff;

	if ( !data )
	{
		textObjects[0].setText( IDS_MP_STATS_PLAYER_NAME );
		textObjects[1].setText( "" );
		textObjects[2].setText( IDS_MP_STATS_SCORE );
		textObjects[3].setText( IDS_MP_STATS_KILLS );
		textObjects[4].setText( IDS_MP_STATS_LOSSES );
	}
	else
	{
		textObjects[1].setColor(scoreColor);
		textObjects[2].setColor(scoreColor);
		textObjects[3].setColor(scoreColor);
		textObjects[4].setColor(scoreColor);

		textObjects[0].setColor( textColor );

		textObjects[0].setText( data->name );
		char team[32];
		sprintf( team, "%ld", data->teamSelected + 1 );
		textObjects[1].setText( team );
		// score needs to do here
		sprintf( team, "%ld", data->score );
		if ( MPlayer->missionSettings.missionType == MISSION_TYPE_KING_OF_THE_HILL )
		{
			sprintf( team, "%ld:%.2ld", data->score/60, data->score % 60 );
		}
		if ( bShowScore )
			textObjects[2].setText( team );
		else
			textObjects[2].setText( "" );

		sprintf( team, "%ld", data->kills );
		textObjects[3].setText( team );

		sprintf( team, "%ld", data->losses );
		textObjects[4].setText( team );

	}




}

long MPStatsEntry::	getPlayerHeadingX()
{
	return location[0].x;
}
long MPStatsEntry::	getRankingX()
{
	return textObjects[0].globalX();
}
long MPStatsEntry::	getKillsX()
{
	return textObjects[1].globalX();
}
long MPStatsEntry::	getLossesX()
{
	return textObjects[2].globalX();
}


//*************************************************************************************************
MPStatsResultsEntry::MPStatsResultsEntry()
{
	overlayColor = 0;
}

MPStatsResultsEntry::~MPStatsResultsEntry()
{
}

void MPStatsResultsEntry::render( int x, int y )
{
	LogisticsScreen::render( x, y );
	//if ( overlayColor )
	//{
	//	GUI_RECT rect = { textObjects[5].left(), rects[0].top(), rects[5].right(), rects[0].y() + rects[0].height() };
	//	drawRect( rect, overlayColor );
	//}
}

void MPStatsResultsEntry::init()
{
	FitIniFile file;
	FullPathFileName path;
	path.init( artPath, "mcl_mp_stats_entry", ".fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char buffer2[512];
		sprintf( buffer2, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, buffer2 );
		return;	

	}

	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button", "Edit" );	

	aObject::init( 0, 0, rects[0].width(), rects[0].height() );


}

void MPStatsResultsEntry::setData(const MC2Player* data, unsigned long laurelColor, bool bShowScore )
{
	rects[4].setColor( MPlayer->colors[data->baseColor[BASECOLOR_TEAM]] );
	rects[2].setColor( MPlayer->colors[data->stripeColor] );

	if ( data &&  MPlayer->allUnitsDestroyed[data->commanderID] )
		overlayColor = 0x7f000000;
	else
		overlayColor = 0;


	long color = MPlayer->colors[data->baseColor[BASECOLOR_TEAM]];
	long textColor = 0xff000000;

	if ( ((color & 0xff) + ( (color & 0xff00)>>8 ) + ( (color & 0xff0000)>>16 ))/3 < 85 )
		textColor = 0xffffffff;

	textObjects[3].setText( data->name );
	textObjects[4].setText( data->unitName );

	textObjects[3].setColor( textColor );
	textObjects[4].setColor( textColor );

	char text[64];
	sprintf( text, "%ld", MPlayer->teamScore[data->team] );
	if ( MPlayer->missionSettings.missionType == MISSION_TYPE_KING_OF_THE_HILL )
	{
		sprintf( text, "%ld:%.2ld", MPlayer->teamScore[data->team]/60, MPlayer->teamScore[data->team] % 60 );
	}
	if ( bShowScore )
		textObjects[0].setText( text );
	else
		textObjects[0].setText( "" );

	sprintf( text, "%ld", data->kills );
	textObjects[1].setText( text );

	sprintf( text, "%ld", data->losses );
	textObjects[2].setText( text );


	sprintf( text, "%ld", data->teamSelected + 1);
	textObjects[5].setText( text );

	char path[256];
	strcpy( path, "data\\multiplayer\\insignia\\" );
	strcat( path, data->insigniaFile );

	if ( data->winner )
	{
		statics[1].setColor( laurelColor );
		rects[7].setColor( laurelColor );
	}
	else
	{
		statics[1].setColor( 0 );
		rects[7].setColor( 0 );
	}

	if ( fileExists( path ) )
	{
		statics[0].setTexture( path );
		statics[0].setUVs( 0, 0, 32, 32 );
	}
	else
	{
		TGAFileHeader*  pData = (TGAFileHeader*)MPlayer->insigniaList[data->commanderID];
		if ( pData )
		{
			int size = pData->pixel_depth/8;
			int ID = mcTextureManager->textureFromMemory( (DWORD*)(pData+1), gos_Texture_Solid, 0, pData->width, size  );
			statics[0].setTexture( ID );
			statics[0].setUVs( 0, 32, 32, 0 );
		}
	}


}


//*************************************************************************************************
// end of file ( MPStats.cpp )
