//***************************************************************************
//
//	team.h - This file contains the Team Class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TEAM_H
#define TEAM_H

//---------------------------------------------------------------------------

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DTEAM_H
#include"dteam.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#ifndef OBJECTIVE_H
#include"objective.h"
#endif

//***************************************************************************

typedef struct _SystemTracker {
	GameObjectPtr			owner;
	long					masterId;
	float					effect;
	SystemTrackerPtr		prev;
	SystemTrackerPtr		next;
} SystemTracker;

//---------------------------------------------------------------------------
typedef struct _TeamData
{
	long 				id;
	long 				rosterSize;
	GameObjectWatchID	roster[MAX_MOVERS_PER_TEAM];
} TeamData;

typedef struct _TeamStaticData
{
	long		numTeams;
	long		homeTeamId;
	char		relations[MAX_TEAMS][MAX_TEAMS];
	bool		noPain[MAX_TEAMS];
} TeamStaticData;

class Team {

	public:

		//-------------
		// general info
		long				id;
		long				rosterSize;
		GameObjectWatchID	roster[MAX_MOVERS_PER_TEAM];			// list of mover WIDs
		
		//-------------
		// mission info
		CObjectives				objectives;
		long					numPrimaryObjectives;

		//------------------
		// static class info
		static long			numTeams;
		static TeamPtr		home;
		static TeamPtr		teams[MAX_TEAMS];
		static SortListPtr	sortList;
		static char			relations[MAX_TEAMS][MAX_TEAMS];
		static bool			noPain[MAX_TEAMS];

	public:

		virtual void init (void);
		
		Team (void) {
			init();
		}
			
		virtual long init (long _id, FitIniFile *pMissionFile = 0);

		virtual long loadObjectives (FitIniFile *pMissionFile = 0);

		long getId (void) {
			return(id);
		}

		void buildRoster (void);

		void addToRoster (MoverPtr mover);

		long getRosterSize (void) {
			return(rosterSize);
		}

		MoverPtr getMover (long index);

		void removeFromRoster (MoverPtr mover);

		virtual long getRoster (GameObjectPtr* objList, bool existsOnly = false);


		void disableTargets (void);

		void destroyTargets (void);

		bool isTargeting (GameObjectWatchID targetWID, GameObjectWatchID exceptWID);

		bool isCapturing (GameObjectWatchID targetWID, GameObjectWatchID exceptWID);

		bool isContact (GameObjectPtr looker, MoverPtr mover, long contactCriteria);

		virtual long getContacts (GameObjectPtr looker, int* contactList, int contactCriteria, int sortType);

		bool hasSensorContact (long teamID);

		void addToGUI (void);

		Stuff::Vector3D calcEscapeVector (MoverPtr mover, float threatRange);

		void statusCount (int* statusTally);

		void eject (void);

		virtual long init (FitIniFile* unitFile) {
			return(NO_ERR);
		}
		
		virtual void destroy (void);

		~Team (void) {
			destroy();
		}

		static bool lineOfSight (float startLocal, long mCellRow, long mCellCol, float endLocal, long tCellRow, long tCellCol, long teamId, float targetRadius, float startRadius = 0.0f, bool checkVisibleBits = true);

		static bool lineOfSight (Stuff::Vector3D myPos, Stuff::Vector3D targetPosition, long teamId, float targetRadius, float startRadius = 0.0f, bool checkVisibleBits = true);

		//-------------------------------------------		
		// Can anyone on my team see this position?
		// Used for cursors, artillery, indirect fire.
		bool teamLineOfSight (Stuff::Vector3D tPos, float extRad);

		void markRadiusSeen (Stuff::Vector3D& location, float radius);

		void markRadiusSeenToTeams (Stuff::Vector3D& location, float radius = -1, bool shrinkForNight = false);

		void markSeen (Stuff::Vector3D& location, float specialUnitExpand);

		void setRelation (TeamPtr team, char relation) {
			relations[id][team->getId()] = relation;
			relations[team->getId()][id] = relation;
		}

		long getRelation (TeamPtr team) {
			if ( !team )
				return RELATION_NEUTRAL;
			return(relations[id][team->getId()]);
		}

		static long getRelation (long teamID1, long teamID2) {
			if (teamID1 == -1)
				return(0);
			if (teamID2 == -1)
				return(0);
			return(relations[teamID1][teamID2]);
		}

		static long Save (PacketFilePtr file, long packetNum);
		static long Load (PacketFilePtr file, long packetNum);

		bool isFriendly (TeamPtr team) {
			if ( !team )
				return 0;

			return(relations[id][team->getId()] == RELATION_FRIENDLY);
		}

		bool isEnemy (TeamPtr team) {
			if ( !team )
				return 0;
			return(relations[id][team->getId()] == RELATION_ENEMY);
		}

		bool isNeutral (TeamPtr team) {
			if ( !team )
				return true;
			return(relations[id][team->getId()] == RELATION_NEUTRAL);
		}
};


//***************************************************************************

#endif
