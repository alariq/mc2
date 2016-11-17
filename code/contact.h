//***************************************************************************
//
//	contact.h - This file contains the Contact Class header definitions
//
//	MechCommander 2 -- FASA Interactive Technologies
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CONTACT_H
#define	CONTACT_H

//---------------------------------------------------------------------------
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DCONTACT_H
#include"dcontact.h"
#endif

#ifndef DGAMEOBJ_H
#include"dgameobj.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif

#ifndef DOBJTYPE_H
#include"dobjtype.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

#ifndef DTEAM_H
#include"dteam.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

//***************************************************************************

class ContactInfo {

	public:

		unsigned char				contactStatus[MAX_TEAMS];
		//unsigned char				allContactStatus[MAX_TEAMS];
		unsigned char				contactCount[MAX_TEAMS];	//How many mechs/vehicles have me on sensors?
		unsigned char				sensors[MAX_SENSORS];		//index into sensor's contact list
		unsigned short				teams[MAX_TEAMS];			//index into team's contact list
		unsigned char				teamSpotter[MAX_TEAMS];

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);

		void destroy (void) {
		}

		void init (void) {
			for (long i = 0; i < MAX_TEAMS; i++) {
				contactStatus[i] = CONTACT_NONE;
				//allContactStatus[i] = CONTACT_NONE;
				contactCount[i] = 0;
				teams[i] = 0;
				teamSpotter[i] = 0;
			}
			for (int i = 0; i < MAX_SENSORS; i++)
				sensors[i] = 255;
		}

		ContactInfo (void) {
			init ();
		}

		~ContactInfo (void) {
			destroy();
		}

		long getContactStatus (long teamId, bool includingAllies) {
			if (teamId == -1)
				return(CONTACT_NONE);
			return(contactStatus[teamId]);
		}

		void incContactCount (long teamId) {
			contactCount[teamId]++;
		}

		void decContactCount (long teamId) {
			contactCount[teamId]--;
		}

		long getContactCount (long teamId) {
			return(contactCount[teamId]);
		}

		void setSensor (long sensor, long contactIndex) {
			sensors[sensor] = contactIndex;
		}

		long getSensor (long sensor) {
			return(sensors[sensor]);
		}

		void setTeam (long teamId, long contactIndex) {
			teams[teamId] = contactIndex;
		}

		long getTeam (long teamId) {
			return(teams[teamId]);
		}
};

//---------------------------------------------------------------------------

typedef struct _ECMInfo {
	GameObjectPtr				owner;
	float						range;
} ECMInfo;

//---------------------------------------------------------------------------

class SensorSystem {

	public:

		long					id;
		TeamSensorSystemPtr		master;
		long					masterIndex;
		GameObjectPtr			owner;
		float					range;
		long					skill;
		bool					broken;
		bool					notShutdown;
		bool					hasLOSCapability;

		float					ecmEffect;

		float					nextScanUpdate;
		float					lastScanUpdate;

		unsigned short			contacts[MAX_CONTACTS_PER_SENSOR];
		long					numContacts;
		long					numExclusives;
		long					totalContacts;

		SensorSystemPtr			prev;
		SensorSystemPtr			next;

		static long				numSensors;
		static float			scanFrequency;
		static SortListPtr		sortList;

	public:

		void* operator new (size_t ourSize);
		
		void operator delete (void* us);
		
		void init (void);

		void destroy (void);

		void setMaster (TeamSensorSystemPtr newMaster);

		void setOwner (GameObjectPtr newOwner);

		void disable (void);

		bool enabled (void);

		void setShutdown (bool setting);

		void setMasterIndex (long index) {
			masterIndex = index;
		}

		long getMasterIndex (void) {
			return(masterIndex);
		}

		void setRange (float newRange);

		float getRange (void);

		float getEffectiveRange (void);

		void setSkill (long newSkill);

		long getSkill (void) {
			return(skill);
		}

		long getTotalContacts (void) {
			return(totalContacts);
		}

		void setNextScanUpdate (float when) {
			nextScanUpdate = when;
		}

		void setScanFrequency (float seconds) {
			scanFrequency = seconds;
		}

		SensorSystem (void) {
			init();
		}

		~SensorSystem (void) {
			destroy();
		}

		long getSensorQuality (void);

		long calcContactStatus (MoverPtr mover);

		bool isContact (MoverPtr mover);

		void addContact (MoverPtr mover, bool visual);

		void modifyContact (MoverPtr mover, bool visual);

		void removeContact (long contactIndex);

		void removeContact (MoverPtr mover);

		void clearContacts (void);

		long scanBattlefield (void);

		long scanMover (Mover* mover);

		void updateContacts (void);

		void updateScan (bool forceUpdate = false);

		long getTeamContacts (int* contactList, int contactCriteria, int ortType);
		
		void setLOSCapability (bool flag)
		{
			hasLOSCapability = flag;
		}
};

//---------------------------------------------------------------------------


#define	MAX_SENSORS_PER_TEAM MAX_MOVERS

class TeamSensorSystem {

	public:

		long				teamId;
		long				nextContactId;
		long				numContactUpdatesPerPass;
		long				curContactUpdate;
		long				contacts[MAX_MOVERS];
		//long				allContacts[MAX_MOVERS];
		long				numContacts;
		//long				numAllContacts;
		long				numEnemyContacts;
		SensorSystemPtr		sensors[MAX_SENSORS_PER_TEAM];
		long				numSensors;
		SystemTrackerPtr	ecms;
		long				numEcms;
		SystemTrackerPtr	jammers;
		long				numJammers;

		static bool			homeTeamInContact;

	public:

		void* operator new (size_t ourSize);
		
		void operator delete (void* us);
		
		void init (void);

		void destroy (void);

		TeamSensorSystem (void) {
			init();
		}
		
		~TeamSensorSystem (void) {
			destroy();
		}

		void update (void);

		void setTeam (TeamPtr newTeam);

		void incNumEnemyContacts (void);

		void decNumEnemyContacts (void);

		void addContact (SensorSystemPtr sensor, MoverPtr contact, long contactIndex, long contactStatus);

		SensorSystemPtr findBestSpotter (MoverPtr contact, long* status);

		void modifyContact (SensorSystemPtr sensor, MoverPtr contact, long contactStatus);

		void removeContact (SensorSystemPtr sensor, MoverPtr contact);

		void addSensor (SensorSystemPtr sensor);

		void removeSensor (SensorSystemPtr sensor);

		long getVisualContacts (MoverPtr* moverList);

		long getSensorContacts (MoverPtr* moverList);

		bool hasSensorContact (long teamID);

		long getContacts (GameObjectPtr looker, int* contactList, int contactCriteria, int sortType);

		long getContactStatus (MoverPtr mover, bool includingAllies);

		bool meetsCriteria (GameObjectPtr looker, MoverPtr mover, long contactCriteria);

		void scanBattlefield (void);

		void scanMover (Mover* mover);

		void addEcm (GameObjectPtr owner, float range);

		void updateEcmEffects (void);

		//void updateContactList (void);
};

//---------------------------------------------------------------------------

class SensorSystemManager {

	//-------------
	// Data Members
	
	protected:
		
		long					freeSensors;			//How many sensors are currently free
		SensorSystemPtr*		sensorPool;				//Pool of ALL sensors
		SensorSystemPtr			freeList;				//List of available sensors
		TeamSensorSystemPtr		teamSensors[MAX_TEAMS];
		ECMInfo					ecms[MAX_ECMS];
		long					numEcms;
		long 					teamToUpdate;
		static float			updateFrequency;
		
	public:
		static bool				enemyInLOS;				//Flag is set every frame that I can see someone on sensors or visually.

	//-----------------
	// Member Functions
	
	public:

		void* operator new (size_t ourSize);
		void operator delete (void* us);
		
		SensorSystemManager (void) {
			init();
		}
		
		~SensorSystemManager (void) {
			destroy();
		}
		
		void destroy (void);
		
		void init (void) {
			freeSensors = 0;
			sensorPool = NULL;
			freeList = NULL;
			for (long i = 0; i < MAX_TEAMS; i++)
				teamSensors[i] = NULL;
			numEcms = 0;
			teamToUpdate = 0;
		}
		
		long init (bool debug);

		TeamSensorSystemPtr getTeamSensor (long teamId) {
			return(teamSensors[teamId]);
		}
		
		SensorSystemPtr newSensor (void);

		void freeSensor (SensorSystemPtr sensor);

		SensorSystemPtr getSensor (long id) 
		{
			if ((id < 0) || (id >= MAX_SENSORS))
				STOP(("Tried to access Sensor outside of range.  Tried to access: %d",id));
				
			return(sensorPool[id]);
		}

		long checkIntegrity (void);

		void addTeamSensor (long teamId, SensorSystemPtr sensor);

		void removeTeamSensor (long teamId, SensorSystemPtr sensor);

		void addEcm (GameObjectPtr owner, float range);

		float getEcmEffect (GameObjectPtr victim);

		void updateEcmEffects (void);

		void updateSensors (void);

		//void updateTeamContactLists (void);

		void update (void);
};

//---------------------------------------------------------------------------

extern SensorSystemManagerPtr		SensorManager;

//***************************************************************************

#endif
