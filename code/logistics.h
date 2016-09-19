//******************************************************************************************
// logistics.h - This file contains the logistics class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef LOGISTICS_H
#define LOGISTICS_H
//----------------------------------------------------------------------------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef MC2movie_H
#include"mc2movie.h"
#endif

#include"logisticsdata.h"

//----------------------------------------------------------------------------------
// Macro Definitions
#define log_DONE						0
#define log_STARTMISSIONFROMCMDLINE		1
#define log_SPLASH						2
#define log_RESULTS						3
#define log_ZONE						4

class MissionResults;
class MissionBegin;


//----------------------------------------------------------------------------------
class Logistics
{
	protected:
	
		
		bool							active;						//Am I currently in control?
		
		long							logisticsState;
		long							prevState;					//Used to cleanup previous state
		
		LogisticsData					logisticsData;

		
		MissionResults*					missionResults;
		MissionBegin*					missionBegin;
	
	public:
	
		Logistics (void)
		{
			init();
		}
		
		~Logistics (void)
		{
			destroy();
		}
		
		void init (void)
		{
			active = FALSE;
			missionResults = 0;
			missionBegin = 0;
			logisticsState = log_SPLASH;
			bMovie = NULL;
		}
		
		void destroy (void);
		
		void initSplashScreen (char *screenFile, char *artFile);
		void destroySplashScreen (void);
		
		void start (long logState);		//Actually Starts execution of logistics in state Specified
		void stop (void);				//Guess what this does!
				
		long update (void);
		
		void render (void);

		void setResultsHostLeftDlg( const char* pName );
		
		void setLogisticsState (long state)
		{
			prevState = logisticsState;
			logisticsState = state;
		}

		MissionBegin *getMissionBegin (void)
		{
			return missionBegin;
		}

		static int _stdcall beginMission( void*, int, void*[] );

		int DoBeginMission();
		void playFullScreenVideo( const char* fileName );

		MC2MoviePtr	bMovie;


	private:

		

		void initializeLogData();
		bool		bMissionLoaded;
		long		lastMissionResult;
};

extern Logistics *logistics;
//----------------------------------------------------------------------------------
#endif