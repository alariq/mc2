#ifndef MAINMENU_H
#define MAINMENU_H
/*************************************************************************************************\
MainMenu.h			: Interface for the MainMenu component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef AANIM_H
#include"aanim.h"
#endif

#ifndef MPLOADMAP_H
#include"mploadmap.h"
#endif

#ifndef MC2movie_H
#include"mc2movie.h"
#endif


class OptionsScreenWrapper;
class Mechlopedia;
//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MainMenu:
**************************************************************************************************/
class SplashIntro : public LogisticsScreen
{
public:

	SplashIntro(){}
	virtual ~SplashIntro(){}

	void init();

};


class MainMenu: public LogisticsScreen
{
	public:

		MainMenu();
		virtual ~MainMenu();


		int init( FitIniFile& file );

		virtual void begin();
		virtual void end();
		virtual void update();
		virtual void render();

		void setHostLeftDlg( const char* playerName );

		void setDrawBackground( bool bDrawBackground );
		void skipIntro();

		virtual int	handleMessage( unsigned long, unsigned long );
		

		static	bool	bDrawMechlopedia;

	private:

		MainMenu& operator=( const MainMenu& ainMenu );
		MainMenu( const MainMenu& src );



		bool	bDrawBackground;

		LogisticsScreen	background;

		bool	promptToQuit;
		bool	bOptions;
		bool	bSave;
		bool	bLoad;
		bool	bLoadSingle;
		bool	bLoadCampaign;
		bool	promptToDisconnect;
		bool	bLegal;

		long	tuneId;			//What music should I play here!
		bool	musicStarted;	//Should I restart the tune?

		long	endResult;

		aAnimation	beginAnim;
		aAnimation	endAnim;

		OptionsScreenWrapper*	optionsScreenWrapper;
		Mechlopedia*			mechlopedia;
		SplashIntro				intro;
		bool					introOver;
		MPLoadMap				singleLoadDlg;
		bool					bHostLeftDlg;

		MC2MoviePtr			introMovie;
			
		

};


//*************************************************************************************************
#endif  // end of file ( MainMenu.h )
