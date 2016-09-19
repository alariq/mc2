#ifndef MISSIONRESULTS_H
#define MISSIONRESULTS_H
/*************************************************************************************************\
MissionResults.h			: Interface for the MissionResults component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MissionResults:
**************************************************************************************************/

#include"asystem.h"
#include"alistbox.h"
#include"mpstats.h"


class SalvageMechScreen;
class MechIcon;
class BattleMech;
class PilotReviewScreen;

class MissionResults
{
	public:

		MissionResults();
		~MissionResults();

		void init();

		void update();
		void render();

		void begin();
		void end();
		bool isDone(){ return bDone; }

		void setHostLeftDlg( const char* pName );

		//Tutorial - Used to play Voice Over, first time in!!
		// THIS must be added to savegame!!
		//
		static bool FirstTimeResults;	

	private:

		SalvageMechScreen*		pSalvageScreen;
		PilotReviewScreen*		pPilotScreen;
		MPStats					mpStats;
		bool					bDone;
		bool					bPilotStarted;

		friend class Logistics;

};





//*************************************************************************************************
#endif  // end of file ( MissionResults.h )
