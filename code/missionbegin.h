#ifndef MISSIONNBEGIN_H
#define MISSIONNBEGIN_H
/*************************************************************************************************\
MissionBegin.h			: Interface for the MissionBegin component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MissionBegin:
**************************************************************************************************/

#include"asystem.h"
#include"alistbox.h"
#include"aanim.h"
#include"abl.h"

class LogisticsScreen;
class MainMenu;

class MissionBegin
{
	public:

		//Tutorial - SAVE with savegame, please!!
		static bool 				FirstTimePurchase;
		static bool					FirstTimeMechLab;

		MissionBegin();
		~MissionBegin();

		void init();

		const char* update();
		void render();

		void begin();
		void end();
		bool isDone(){ return bDone; }
		bool readyToLoad() { return bReadyToLoad; }

		void beginSplash( const char* playerName = 0 );

		void beginMPlayer();
		void beginSPlayer();
		void beginZone();
		void setToMissionBriefing();

		long getCurrentScreenId();	//Returns screen ID as a function of curScreenX and curScreenY
		bool startAnimation (long bId, bool isButton, float scrollTime, long nFlashes);

		bool isInCalloutAnimation()
		{
			return animationRunning;
		}

		void beginAtConnectionScreen();

		void restartMPlayer( const char* playerName);

	private:

		bool						bDone;
		bool						bReadyToLoad;

		ABLModulePtr				logisticsBrain;
		long						logisticsScriptHandle;

		aAnimation					leftAnim;
		aAnimation					rightAnim;
		aAnimation					upAnim;
		aAnimation					downAnim;
	
		bool						bMultiplayer;

		LogisticsScreen*			multiplayerScreens[5/*dim screen X*/][3/*dim screen Y*/];
		LogisticsScreen*			singlePlayerScreens[5/*dim screen X*/][3/*dim screen Y*/];
		LogisticsScreen*			screens[5/*dim screen X*/][3/*dim screen Y*/];
		LogisticsScreen*			placeHolderScreen;

		long						curScreenX;
		long						curScreenY;

		MainMenu*					mainMenu;
		bool						bSplash;

		bool						animJustBegun;

		void						setUpMultiplayerLogisticsScreens();

		//-----------------------------------------------
		// Tutorial Data
		bool						animationRunning;
		float						timeLeftToScroll;
		long						targetButtonId;
		bool						targetIsButton;
		long						buttonNumFlashes;
		float						buttonFlashTime;
};





//*************************************************************************************************
#endif  // end of file ( MissionBegin.h )
