#ifndef MPSTATS_H
#define MPSTATS_H
/*************************************************************************************************\
MPStats.h			: Interface for the MPStats component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

//*************************************************************************************************
struct _MC2Player;

/**************************************************************************************************
CLASS DESCRIPTION
MPStats:
**************************************************************************************************/
class MPStatsEntry : public LogisticsScreen
{
	public:
		MPStatsEntry();
		virtual ~MPStatsEntry();

		void init( );
		void setData( const _MC2Player*, bool bShowScore );
		virtual void render( int x, int y );

		long	getPlayerHeadingX();
		long	getRankingX();
		long	getKillsX();
		long	getLossesX();
		long	overlayColor;

		

};

class MPStatsResultsEntry : public LogisticsScreen
{
	public:
		MPStatsResultsEntry();
		virtual ~MPStatsResultsEntry();
		virtual void render( int x, int y );

		void init( );
		void setData( const _MC2Player*, unsigned long laurelColor, bool bShowScore );
		long	overlayColor;

};




class MPStats: public LogisticsScreen
{
	public:

		MPStats();
		virtual ~MPStats();

		virtual void render(int xOffset, int yOffset);
		virtual void update();

		virtual void begin();
		virtual void end();

		virtual int handleMessage( unsigned long what, unsigned long who );

		int init();

		void		setHostLeftDlg( const char* hostName );

	private:

		MPStats( const MPStats& src );

		MPStatsResultsEntry		entries[MAX_MC_PLAYERS];

		bool				bShowMainMenu;
		bool				bSavingStats;
		bool				bHostLeftDlg;

		// HELPER FUNCTIONS

};


//*************************************************************************************************
#endif  // end of file ( MPStats.h )
