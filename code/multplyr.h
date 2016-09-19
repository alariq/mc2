//***************************************************************************
//
//	Multplyr.h -- Defs for Multiplayer routines
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MULTPLYR_H
#define MULTPLYR_H

//***************************************************************************

//--------------
// Include Files

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DMULTPLYR_H
#include"dmultplyr.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

#ifndef DTURRET_H
#include"dturret.h"
#endif

#ifndef DTACORDR_H
#include"dtacordr.h"
#endif

#ifndef DGAMEOBJ_H
#include"dgameobj.h"
#endif

#ifndef DBLDNG_H
#include"dbldng.h"
#endif

//#ifndef DUNIT_H
//#include<dunit.h>
//#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#ifndef DGROUP_H
#include"dgroup.h"
#endif

//#ifndef FICOMMONNETWORK_H
//#include<ficommonnetwork.h>
//#endif

//#ifndef SESSIONMANAGER_H
//#include<sessionmanager.h>
//#endif

#include<windows.h>
//#include<stdio.h>

//***************************************************************************
typedef void *NETPLAYER;

enum NETMESSAGETYPE
{
	NMT_MESSAGE,		// a network message
	NMT_ADDPLAYER,		// a new player
	NMT_DELPLAYER,		// a player has left
	NMT_TERMSESSION,	// the session has been terminated
	NMT_HOSTMIGRATE,	// the host has migrated
};

//========================================================================
// NETMESSAGE
//========================================================================
class NETMESSAGE
{
public:
	NETMESSAGETYPE	m_Type;			// 0==network message
	NETPLAYER		m_pPlayer;		// player message is to, or who it's from
	DWORD			m_dwFlags;		// priority, guaranteed, etc.
	DWORD			m_dwInfo;		// additional message info
	DWORD			m_dwTimeStamp;
	void *			m_pData;
	int				m_size;
};

#define	SERVER_HANDLES_ALL_WEAPONHITS	TRUE

#define	MAX_MC_PLAYERS					8
#define	MAX_TEAMS						8
#define	MAX_LOCAL_MOVERS				MAX_MOVERGROUP_COUNT	// 16
#define	MAX_MULTIPLAYER_MOVERS			96
#define	MAX_MULTIPLAYER_MECHS_IN_LOGISTICS		36

#define	MAX_MULTIPLAYER_TURRETS			128
#define	MAX_WEAPONHIT_CHUNKS			1024
#define	MAX_WORLD_CHUNKS				1024
#define MAX_STORED_CHATS				16
#define MAX_CHAT_LENGTH					128

#define	MCGROUP_SERVER					0xFFFFFFFF
#define	MCGROUP_ALLPLAYERS				0xFFFFFFFE
#define	MCGROUP_INNERSPHERE				0xFFFFFFFD
#define	MCGROUP_CLAN					0xFFFFFFFC

#define	MPLAYER_NO_ERR					0
#define	MPLAYER_ERR_HOST_NOT_FOUND		-1
#define	MPLAYER_ERR_NO_CONNECTION		-2
#define	MPLAYER_ERR_SESSION_IN_PROGRESS	-3
#define	MPLAYER_ERR_SESSION_LOCKED		-4
#define	MPLAYER_ERR_SESSION_NOT_FOUND	-5
#define	MPLAYER_ERR_NOT_SCANNING		-6
#define	MPLAYER_ERR_BAD_VERSION			-7
#define	MPLAYER_ERR_SESSION_FULL		-8

typedef enum {
	BASECOLOR_PREFERENCE,
	BASECOLOR_SELF,
	BASECOLOR_TEAM
} BaseColorType;

typedef enum {
	MULTIPLAYER_MODE_NONE,
	MULTIPLAYER_MODE_BROWSER,
	MULTIPLAYER_MODE_PARAMETERS,
	MULTIPLAYER_MODE_LOGISTICS,
	MULTIPLAYER_MODE_LOADING,
	MULTIPLAYER_MODE_MISSION,
	MULTIPLAYER_MODE_RESULTS
} MultiplayerMode;

typedef enum {
	MISSION_TYPE_ELIMINATION,
	MISSION_TYPE_KING_OF_THE_HILL,
	MISSION_TYPE_CAPTURE_BASE,
	MISSION_TYPE_TERRITORIES,
	MISSION_TYPE_LAST_MAN_STANDING,
	MISSION_TYPE_LAST_MAN_ON_THE_HILL,
	MISSION_TYPE_OTHER,
	NUM_MISSION_TYPES
} MissionType;

enum {
	GO_SESSION_SCREEN = 1
};

enum {
	MCMSG_GENERIC,
	MCMSG_CHAT,
	MCMSG_NEW_SERVER,
	MCMSG_PLAYER_CID,
	MCMSG_PLAYER_UPDATE,
	MCMSG_MISSION_SETTINGS_UPDATE,
	MCMSG_PLAYER_INFO,
	MCMSG_PLAYER_CHECK_IN,
	MCMSG_PLAYER_SETUP,
	MCMSG_PLAYER_INSIGNIA,
	MCMSG_MISSION_SETUP,
	MCMSG_PLAYER_CHECK_IN_RECEIPT,
	MCMSG_MOVERS_SETUP,					//not used?
	MCMSG_START_PLANNING,				//not used?
	MCMSG_START_MISSION,
	MCMSG_END_MISSION,
	MCMSG_LEAVE_SESSION,
	MCMSG_PLAYER_ORDER,
	MCMSG_HOLD_POSITION,
	MCMSG_PLAYER_MOVERGROUP,
	MCMSG_PLAYER_ARTILLERY,
	MCMSG_MOVER_UPDATE,
	MCMSG_TURRET_UPDATE,
	MCMSG_MOVER_WEAPONFIRE_UPDATE,
	MCMSG_TURRET_WEAPONFIRE_UPDATE,
	MCMSG_MOVER_CRITICAL_UPDATE,
	MCMSG_WEAPONHIT_UPDATE,
	MCMSG_WORLD_UPDATE,
	MCMSG_DEPLOY_FORCE,
	MCMSG_REMOVE_FORCE,
	MCMSG_PREPARE_MISSION,
	MCMSG_READY_FOR_BATTLE,
	MCMSG_FILE_INQUIRY,
	MCMSG_FILE_REPORT,
	MCMSG_LOAD_MISSION,
	MCMSG_START,
	MCMSG_JOIN_TEAM,
	MCMSG_SWITCH_SCREEN,
	MCMSG_RP_UPDATE,
	MCMSG_TECHBASE_CHANGE,
	MCMSG_SESSIONCHECKIN,
	MCMSG_REINFORCEMENT
};

#define	MAXLEN_SESSION_NAME			25
#define	MAXLEN_MAP_NAME				25
#define	MAXLEN_MAP_DESC				32
#define	MAX_SESSIONS				512
#define	MAXLEN_PLAYER_NAME			25
#define	MAXLEN_UNIT_NAME			25
#define	MAXLEN_INSIGNIA_FILE		35
#define MAX_COLORS					32

typedef struct _MC2Session {
	char			name[MAXLEN_SESSION_NAME];
	char			maxPlayers;
	char			numPlayers;
	char			map[MAXLEN_MAP_NAME];
	long			ping;
	bool			locked;
	bool			inProgress;
	bool			cancelled;
	char			versionStamp[15];
	GUID			handle;
} MC2Session;

#pragma pack(1)
typedef struct _MC2SessionData {
	char			map[MAXLEN_MAP_NAME];
	bool			locked;
	bool			inProgress;
	char			maxPlayers;
	char			numPlayers;
	char			versionStamp[15];
	bool			cancelled;
} MC2SessionData;
#pragma pack()

typedef struct _TeamInfo {
	DWORD id;
	DWORD team;
} TeamInfo;

typedef struct _MC2PlayerSlot {
	NETPLAYER		player;
	char			commanderID;
	char			name[MAXLEN_PLAYER_NAME];
} MC2PlayerSlot;

typedef struct _MC2Player {
	bool			checkedIn;
	NETPLAYER		player;
	char			commanderID;
	char			dropZone;
	char			name[MAXLEN_PLAYER_NAME];
	char			ipAddress[16];
	char			unitName[MAXLEN_UNIT_NAME];
	char			insigniaFile[MAXLEN_INSIGNIA_FILE];
	char			baseColor[3];
	char			stripeColor;
	char			teamSelected;
	char			team;
	unsigned long	teamSeniority;
	char			faction;
	long			cBills;
	long			resourcePointsAtStart;
	long			resourcePointsGained;
	long			resourcePoints;
	long			rank;
	bool			ready;
	long			score;
	long			kills;
	long			losses;
	bool			winner;
	bool			leftSession;
	bool			booted;
} MC2Player;

typedef struct _MissionSettings {
	char			map[MAXLEN_MAP_NAME];	// file name
	char			name[MAXLEN_MAP_DESC];	// displayNmae
	GUID			mapGuid;				// Insures maps are the same version!!
	char			url[256]; /// GLENN, you probably want this somewhere else....
	long			defaultCBills;
	long			resourcePoints;
	float			timeLimit;
	bool			unlimitedAmmo;
	bool			variants;
	bool			allTech;
	bool			airStrike;
	bool			guardTower;
	bool			repairVehicle;
	bool			recoveryTeam;
	bool			sensorProbe;
	bool			scoutCopter;
	bool			mineLayer;
	bool			resourceBuilding;
	bool			resourceForMechs;
	char			missionType;
	bool			quickStart;
	char			maxTeams;
	char			maxPlayers;
	char			numPlayers;
	long			dropWeight;
	bool			locked;
	bool			inProgress;
} MissionSettings;

typedef struct _CompressedMech {
	bool			lastMech;
	long			objNumber;
	char			commanderID;
	unsigned long	baseColor;
	unsigned long	highlightColor1;
	unsigned long	highlightColor2;
	char			pilotFile[50];
	char			mechFile[50];
	char			variantName[64];
	char			variantNum;
	float			pos[2];
	long			cBills;
	bool			designerMech;
	long			numComponents;
	unsigned char	components[50];
} CompressedMech;

#pragma pack(1)

#pragma warning (disable : 4200)

//------------------------------------------------------------------------------------------

class WorldChunk {

	public:

		char				type;
		short				tileRC[2];
		long				objectId;
		long				objectBlockOrTrainNumber;
		long				objectVertexOrCarNumber;
		char				objectItemNumber;
		long				param1;
		long				param2;

		unsigned long		data;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void) {
			type = -1;
			tileRC[0] = -1;
			tileRC[1] = -1;
			objectId = 0;
			objectBlockOrTrainNumber = 0;
			objectVertexOrCarNumber = 0;
			objectItemNumber = 0;
			param1 = 0;
			param2 = 0;
			data = 0;
		}

		void destroy (void) {
		}

		WorldChunk (void) {
			init();
		}

		~WorldChunk (void) {
			destroy();
		}

		void buildMissionScriptMessage (long messageCode,
										long messageParam);

		void buildArtillery (long artilleryType,
							 long teamId,
							 Stuff::Vector3D location,
							 long seconds);

		void buildMine (long tileR,
						long tileC,
						long teamId,
						long mineState,
						long explosionType);

		void buildTerrainFire (GameObjectPtr object,
							   long seconds);

		void buildPilotKillStat (long moverIndex,
								 long vehicleClass);

		void buildScore (long commanderID, long score);

		void buildKillLoss (long killerCID, long loserCID);

		void buildEndMission (void);

		void buildCaptureBuilding (BuildingPtr building, long newCommanderID);

		void pack (void);

		void unpack (void);

		bool equalTo (WorldChunkPtr chunk);
};


// The following messages are for multiplayer games and are sent when
// the user deploys a unit or drags it back to MechBay.
//------------------------------------------------------------------------------------------

/*
class MCMSG_DeployForce: public FIGuaranteedMessageHeader
{
	private:
		
	// Keep the constructor private because we don't want 
	// anyone to call it when there is an undefined size 
	// for the class.
	MCMSG_DeployForce();
	
	unsigned char dataFlags;

	enum 
	{
		PartTypeBit	= 0,
		TeamBit,
		VariantBits1,
		VariantBits2,
		LanceBits1,
		LanceBits2,
		SlotBits1,
		SlotBits2
	};

	enum 
	{
		PartTypeFlag = 1 << PartTypeBit,
		TeamFlag = 1 << TeamBit,
		VariantMask = 1 << VariantBits1 | 1 << VariantBits2,
		LanceMask = 1 << LanceBits1 | 1 << LanceBits2,
		SlotMask = 1 << SlotBits1 | 1 << SlotBits2
	};

	public:
		// data
		unsigned char partNameIndex;
		unsigned char pilotNameIndex[2];
		unsigned char inventoryCount; // Number of components
		unsigned short  componentIDs[];

		// member functions
		void Init()
		{
			FIGuaranteedMessageHeader::Init();
			SetType(MCMSG_DEPLOY_FORCE);
			dataFlags = 0;
			partNameIndex = 0;
			pilotNameIndex[0] = 255;
			pilotNameIndex[1] = 255;
			inventoryCount = 0;
		}

		// Teams:  InnerSphere is 0, Clan is 1
		void SetInnerSphere()
		{
			dataFlags &= ~TeamFlag;
		}

		void SetClan()
		{
			dataFlags |= TeamFlag;
		}

		void SetVehicle()
		{
			dataFlags &= ~PartTypeFlag;
		}

		void SetMech()
		{
			dataFlags |= PartTypeFlag;
		}

		
		void SetVariant(int variant)
		{
			if (variant > 3)
				return;
			dataFlags &= ~VariantMask; // erase old variant
			dataFlags |= (variant << VariantBits1); // set new
		}

		void SetLance(int lance)
		{
			if (lance > (LanceMask >> LanceBits1))
				return;

			dataFlags &= ~LanceMask;
			dataFlags |= (lance << LanceBits1);
		}

		void SetSlot(int slot)
		{
			if (slot > (SlotMask >> SlotBits1))
				return;

			dataFlags &= ~SlotMask;
			dataFlags |= (slot << SlotBits1);
		}


		int GetVariant()
		{
			return (dataFlags & VariantMask) >> VariantBits1;
		}

		int GetLance()
		{
			return (dataFlags & LanceMask) >> LanceBits1;
		}

		int GetSlot()
		{
			return (dataFlags & SlotMask) >> SlotBits1;
		}

		BOOL IsClan()
		{
			return (dataFlags & TeamFlag);
		}
		
		BOOL IsMech()
		{
			return (dataFlags & PartTypeFlag);
		}


};
*/

//---------------------------------------------------------------------------

/*
class MCMSG_JoinTeam:public FIGuaranteedMessageHeader
{
	public:
		void Init()
		{
			FIGuaranteedMessageHeader::Init();
			SetType(MCMSG_JOIN_TEAM);
			player = -1;
			team = -99;
			slot = -99;
		}

		DPID player;
		char team;
		char slot;
};
*/

//---------------------------------------------------------------------------

/*
class MCMSG_RemoveForce:public FIGuaranteedMessageHeader
{
	public:	
		void Init()
		{
			FIGuaranteedMessageHeader::Init();
			SetType(MCMSG_REMOVE_FORCE);
			slotNumber = 0;
			lanceNumber = 0;
		}

		unsigned char slotNumber;
		unsigned char lanceNumber;
};
*/

//---------------------------------------------------------------------------

class MCMSG_Generic {

	public:

		unsigned char		type;

	public:

		MCMSG_Generic (void) {
			init();
		}

		void init (void) {
			type = MCMSG_GENERIC;
		}
};

//---------------------------------------------------------------------------

class MCMSG_PlayerCID {

	public:

		unsigned char		type;
		char				commanderID;
		unsigned char		subType;
		
	public:

		MCMSG_PlayerCID (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_CID;
			commanderID = -1;
			subType = 0;
		}

};

//---------------------------------------------------------------------------

class MCMSG_PlayerUpdate {

	public:

		unsigned char		type;
		char				stage;
		float				senderTime;
		char				sessionIPAddress[16];
		char				versionStamp[15];
		MC2Player			info;

	public:

		MCMSG_PlayerUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_UPDATE;
			senderTime = -1.0;
			stage = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_MissionSettingsUpdate {

	public:

		unsigned char		type;
		MissionSettings		missionSettings;

	public:

		MCMSG_MissionSettingsUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_MISSION_SETTINGS_UPDATE;
		}
};

//---------------------------------------------------------------------------

class MCMSG_MissionSetup {

	public:

		unsigned char		type;
		long				randomSeed;
		char				subType;
		char				commandersToLoad[MAX_MC_PLAYERS][3];
		CompressedMech		mechData;

	public:

		MCMSG_MissionSetup (void) {
			init();
		}

		void init (void) {
			type = MCMSG_MISSION_SETUP;
			subType = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_PlayerInfo {

	public:

		unsigned char		type;
		char				commanderID;

	public:

		MCMSG_PlayerInfo (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_INFO;
			commanderID = -1;
		}
};

//---------------------------------------------------------------------------

class MCMSG_Chat {

	//GUARANTEED

	public:
	
		unsigned char		type;
		bool				allPlayers;
		bool				isDeadChat;
		bool				hideName;
		char				string[];
	
	public:

		MCMSG_Chat (void) {
			init();
		}

		void init (void) {
			type = MCMSG_CHAT;
			allPlayers = false;
			isDeadChat = false;
			hideName = false;
			string[0] = '\0';
		}
};

//---------------------------------------------------------------------------

class MCMSG_PlayerCheckIn {

	//GUARANTEED

	public:
	
		unsigned char		type;
		char				commanderID;

	public:

		MCMSG_PlayerCheckIn (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_CHECK_IN;
			commanderID = -1;
		}
};

//---------------------------------------------------------------------------

class MCMSG_PlayerSetup {

	//GUARANTEED

	public:

		unsigned char		type;
		unsigned char		moverData[MAX_MULTIPLAYER_MOVERS];

	public:

		MCMSG_PlayerSetup (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_SETUP;
			for (long i = 0; i < MAX_MULTIPLAYER_MOVERS; i++)
				moverData[i] = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_PlayerInsignia {

	public:

		unsigned char		type;
		char				commanderID;
		char				fileName[64];
		long				dataSize;
		unsigned char		data[];

	public:

		MCMSG_PlayerInsignia (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_INSIGNIA;
		}
};

//---------------------------------------------------------------------------

class MCMSG_StartMission {

	//GUARANTEED

	public:

		unsigned char	type;
		long			huh;
	

	public:

		MCMSG_StartMission (void) {
			init();
		}

		void init (void) {
			type = MCMSG_START_MISSION;
			huh = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_EndMission {

	//GUARANTEED

	public:

		unsigned char	type;
		long			teamScore[MAX_MC_PLAYERS];
		long			playerScore[MAX_MC_PLAYERS];
		long			result;

	public:

		MCMSG_EndMission (void) {
			init();
		}

		void init (void) {
			type = MCMSG_END_MISSION;
			result = -1;
		}
};

//---------------------------------------------------------------------------

class MCMSG_LeaveSession {

	public:

		unsigned char	type;
		unsigned char	subType;
		char			commanderID;

	public:

		MCMSG_LeaveSession (void) {
			init();
		}

		void init (void) {
			type = MCMSG_LEAVE_SESSION;
			subType = 0;
			commanderID = -1;
		}
};

//---------------------------------------------------------------------------

class MCMSG_PlayerOrder {

	//GUARANTEED

	public:

		unsigned char	type;
		char			commanderID;
		unsigned char	flags;
		float			location[2];
		unsigned long	tacOrderChunk[2];

	public:

		MCMSG_PlayerOrder (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_ORDER;
			commanderID = -1;
			flags = 0;
			location[0] = 0.0;
			location[1] = 0.0;
			tacOrderChunk[0] = 0;
			tacOrderChunk[1] = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_HoldPosition {

	public:

		unsigned char	type;
		char			commanderID;
		unsigned short	flags;

	public:

		MCMSG_HoldPosition (void) {
			init();
		}

		void init (void) {
			type = MCMSG_HOLD_POSITION;
			commanderID = -1;
			flags = 0;
		}
};

//---------------------------------------------------------------------------

#define MOVERGROUP_ACTION_DEFAULT			0
#define	MOVERGROUP_ACTION_HOLDPOSITION_OFF	1
#define	MOVERGROUP_ACTION_HOLDPOSITION_ON	2

class MCMSG_PlayerMoverGroup {

	//GUARANTEED

	public:

		unsigned char		type;
		unsigned char		action;
		char				commanderID;
		char				groupId;
		unsigned short		moverGroupInfo;

	public:

		MCMSG_PlayerMoverGroup (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_MOVERGROUP;
			commanderID = -1;
			groupId = -1;
			moverGroupInfo = 0;
		}

};

//---------------------------------------------------------------------------

class MCMSG_PlayerArtillery {

	//GUARANTEED

	public:

		unsigned char		type;
		float				location[2];
		unsigned long		chunk;

		MCMSG_PlayerArtillery (void) {
			init();
		}

		void init (void) {
			type = MCMSG_PLAYER_ARTILLERY;
			location[0] = 0.0;
			location[1] = 0.0;
			chunk = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_MoverUpdate {

	public:

		unsigned char		type;
		long				teamScore[MAX_MC_PLAYERS];
		long				playerScore[MAX_MC_PLAYERS];
		//long				resourcePoints[MAX_MC_PLAYERS];
		unsigned short		updateId;
		unsigned char		numRLEs;
		unsigned char		moveData[];

	public:

		MCMSG_MoverUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_MOVER_UPDATE;
			updateId = 0xFFFF;
			numRLEs = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_TurretUpdate {

	public:

		unsigned char		type;
		unsigned short		updateId;
		char				targetList[];

	public:

		MCMSG_TurretUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_TURRET_UPDATE;
			updateId = 0xFFFF;
			for (long i = 0; i < MAX_MULTIPLAYER_TURRETS; i++)
				targetList[i] = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_MoverWeaponFireUpdate {

	//GUARANTEED

	public:

		unsigned char		type;
		unsigned char		numRLEs;
		unsigned char		weaponFireData[];

	public:

		MCMSG_MoverWeaponFireUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_MOVER_WEAPONFIRE_UPDATE;
			numRLEs = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_TurretWeaponFireUpdate {

	//GUARANTEED

	public:

		unsigned char		type;
		char				numTurrets;
		unsigned short		info[];

	public:

		MCMSG_TurretWeaponFireUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_TURRET_WEAPONFIRE_UPDATE;
			numTurrets = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_MoverCriticalUpdate {

	//GUARANTEED

	public:

		unsigned char		type;
		unsigned char		numCritHitChunks[MAX_MULTIPLAYER_MOVERS];
		unsigned char		numRadioChunks[MAX_MULTIPLAYER_MOVERS];
		unsigned char		chunk[];

	public:

		MCMSG_MoverCriticalUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_MOVER_CRITICAL_UPDATE;
			for (long i = 0; i < MAX_MULTIPLAYER_MOVERS; i++) {
				numCritHitChunks[i] = 0;
				numRadioChunks[i] = 0;
			}
		}
};

//---------------------------------------------------------------------------

class MCMSG_WeaponHitUpdate {

	//GUARANTEED

	public:

		unsigned char		type;
		unsigned char		numWeaponHits;
		unsigned long		weaponHitChunk[];

	public:

		MCMSG_WeaponHitUpdate (void) {
			init();
		}

		void init() {
			type = MCMSG_WEAPONHIT_UPDATE;
			numWeaponHits = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_WorldUpdate {

	//GUARANTEED

	public:

		unsigned char		type;
		unsigned char		numWorldChanges;
		unsigned char		numArtilleryStrikes;
		unsigned long		worldChunk[];

	public:

		MCMSG_WorldUpdate (void) {
			init();
		}

		void init (void) {
			type = MCMSG_WORLD_UPDATE;
			numWorldChanges = 0;
			numArtilleryStrikes = 0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_Reinforcement {

	public:

		unsigned char	type;
		unsigned char	stage;
		unsigned char	rosterIndex;
		long			vehicleID;
		char			pilotName[24];
		char			commanderID;
		float			location[2];

	public:

		MCMSG_Reinforcement (void) {
			init();
		}

		void init (void) {
			type = MCMSG_REINFORCEMENT;
			stage = 0;
			rosterIndex = 255;
			vehicleID = 0;
			pilotName[0] = '\0';
			commanderID = 0;
			location[0] = 0.0;
			location[1] = 0.0;
		}
};

//---------------------------------------------------------------------------

class MCMSG_NewServer {

	public:

		unsigned char	type;
		char			ipAddress[16];

	public:

		MCMSG_NewServer (void) {
			init();
		}

		void init (void) {
			type = MCMSG_NEW_SERVER;
			ipAddress[0] = '\0';
		}

};

//---------------------------------------------------------------------------

class MCMSG_FileReport {

	//GUARANTEED

	public:

		unsigned char		type;
		unsigned long		checkSum;
		GUID				fileGuid;
		char				fileName[];

		MCMSG_FileReport (void) {
			init();
		}

		void init() {
			type = MCMSG_FILE_REPORT;
			checkSum = 0;
			fileName[0] = '\0';
			memset(&fileGuid,0,sizeof(GUID));
		}
};

//---------------------------------------------------------------------------

/*
class MCMSG_PassValue {

	public:

		long			value[2];

	public:

		MCMSG_PassValue (void) {
			init();
		}

		void init (void) {
			value[0] = -1;
			value[1] = -1;
		}
};
*/

#pragma warning (default : 4200)

#pragma pack()

//***************************************************************************

typedef enum {
	VERSION_STATUS_UNKNOWN,
	VERSION_STATUS_GOOD,
	VERSION_STATUS_BAD,
	NUM_VERSION_STATUS_TYPES
} VersionStatus;

class MultiPlayer {

	public:

		bool				onLAN;
		bool				hostLeft;
		VersionStatus		versionStatus;
		long				numSessions;
		bool				availableCIDs[MAX_MC_PLAYERS];
		MC2Session			sessionList[MAX_SESSIONS];
		unsigned long		teamSeniority[MAX_TEAMS];
		static long			colors[MAX_COLORS];
		char				colorsCID[MAX_COLORS];
		char				sessionIPAddress[16];
		bool				sessionScanning;
		bool				sessionScanningError;
		bool				sessionScanningPersistent;
		bool				fitStart;
		long				numFitPlayers;
		MissionSettings		missionSettings;
		char				commandersToLoad[MAX_MC_PLAYERS][3];
		CompressedMech		mechData[MAX_MC_PLAYERS][12];
		bool				locked;
		bool				inProgress;
		bool				cancelled;
		long				randomSeed;
		bool				startLogistics;
		bool				startLoading;
		bool				readyToLoad[MAX_MC_PLAYERS];
		bool				mechDataReceived[MAX_MC_PLAYERS];
		bool				missionDataLoaded[MAX_MC_PLAYERS];
		bool				missionFullySetup[MAX_MC_PLAYERS];
		bool				leaveSessionConfirmed[MAX_MC_PLAYERS];

		long				numAirStrikesUsed[MAX_MC_PLAYERS];
		long				numMinelayersUsed[MAX_MC_PLAYERS];
		long				numScoutCoptersUsed[MAX_MC_PLAYERS];
		long				numArtilleryPiecesUsed[MAX_MC_PLAYERS];
		long				numSensorProbesUsed[MAX_MC_PLAYERS];
		long				numRepairVehiclesUsed[MAX_MC_PLAYERS];
		long				numSalvageCraftUsed[MAX_MC_PLAYERS];

		long				numTeams;
		long				teamScore[MAX_TEAMS];
		long				winningTeam;
		BuildingPtr			mainHqBuildings[MAX_TEAMS];
		long				numHqBuildings;
		long*				hqCIDs;
		BuildingPtr*		hqBuildings;
		long				numResourceBuildings;
		BuildingPtr*		resourceBuildings;
		unsigned long		numRandomResourceBuildings;

		long				mode;
		long				sessionEntry;
		unsigned char*		msgBuffer;
		long				numLocalMovers;
		long				numMovers;
		long				numTurrets;
		unsigned char		moverRosterRLE[MAX_MULTIPLAYER_MOVERS];
		long				numMoverRosterRLE;
		MoverPtr			localMovers[MAX_LOCAL_MOVERS];
		MoverPtr			moverRoster[MAX_MULTIPLAYER_MOVERS];
		MoverPtr			playerMoverRoster[MAX_MC_PLAYERS][MAX_LOCAL_MOVERS];
		TurretPtr			turretRoster[MAX_MULTIPLAYER_TURRETS];

		bool				iAmHost;
		//long				serverCID;
		NETPLAYER			myPlayer;
		NETPLAYER			serverPlayer;

		bool				inSession;
		bool				hostDroppedOut;
		long				commanderID;			// same as commanderId
		char				sessionName[80];
		char				playerName[80];

		//MC2Player			playerList[MAX_MC_PLAYERS];		// list of players--order doesn't matter
		MC2PlayerSlot			playerList[MAX_MC_PLAYERS];
		bool				insigniaList[MAX_MC_PLAYERS];		// list of insignia data
		long				insigniaSizeList[MAX_MC_PLAYERS];
		//long				maxPlayers;						// max number of players allowed in session
		MC2Player			playerInfo[MAX_MC_PLAYERS];		// list of players--indexed by commanderID

		bool				playerReady[MAX_MC_PLAYERS];
		bool				inSessionScreen[MAX_MC_PLAYERS];
		bool				startPlanning;
		bool				setupMission;
		bool				startMission;
		bool				endMission;
		bool				waitingToStartMission;
		bool				preparingMission;
		float				startTime;
		bool				allUnitsDestroyed[MAX_MC_PLAYERS];
		long				missionStatus;

		//TeamInfo			teamInfo[MAX_MC_PLAYERS];

		// ChatMessageCallback is called when a chat message arrives.
		//LPMESSAGECALLBACK ChatMessageCallback;


		// Server Data
		float				pingUpdateTime;
		float				pingUpdateFrequency;
		float				moverUpdateTime;
		float				moverUpdateFrequency;
		unsigned short		moverUpdateId;
		float				turretUpdateTime;
		float				turretUpdateFrequency;
		unsigned short		turretUpdateId;
		float				worldUpdateTime;
		float				worldUpdateFrequency;
		float				lastServerUpdateTime;
		long				numWeaponHitChunks;
		unsigned long		weaponHitChunks[MAX_WEAPONHIT_CHUNKS];
		long				numWorldChunks;
		unsigned long		worldChunks[MAX_WORLD_CHUNKS];
		unsigned long		serverOrder[MAX_MC_PLAYERS];
		long				reinforcements[MAX_MC_PLAYERS][2];	// index 0 = current reinforcement, index 1 = current recoverery
		char				reinforcementPilot[MAX_MC_PLAYERS][32];
		
		bool				isMPlayerGame;
		long				badSessionCounter;
		float				warpFactor;

		long				totalLoad;
		long				moverWeaponFireLoad;
		long				turretWeaponFireLoad;
		long				moverCriticalLoad;
		long				weaponHitLoad;
		long				worldStateLoad;
		long				maxTotalLoad;
		long				maxMoverWeaponFireLoad;
		long				maxTurretWeaponFireLoad;
		long				maxMoverCriticalLoad;
		long				maxWeaponHitLoad;
		long				maxWorldLoad;
		long				maxReceiveLoad;
		long				maxReceiveSize;
		long				worldChunkTally[NUM_WORLDCHUNK_TYPES];


		char				currentChatMessages[MAX_STORED_CHATS][MAX_CHAT_LENGTH];
		long				currentChatMessagePlayerIDs[MAX_STORED_CHATS];
		long				chatCount;

		static bool			launchedFromLobby;
		static bool			registerZone;
		static long			presetDropZones[MAX_MC_PLAYERS];

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void);

		long setup (void);

		// initUpdateFrequencies initializes the update frequencies from the 
		// prefs.cfg file if the entries exist.  Otherwise, it uses defaults.
		void initUpdateFrequencies (void);

		// initStartupParameters is called by both init() and 
		// leaveSession()
		void initStartupParameters (bool fresh);

		void resetForNewGame (void);


		MultiPlayer (void) {
			init();
		}
		
		void destroy (void);

		~MultiPlayer (void) {
			destroy();
		}

		bool isServer (void) {
				return(false);
		}

		void setServer (NETPLAYER player, char playerIPAddress[16]);

		bool isHost (void) {
			return(isServer());
		}

		void setMode (long newMode) {
			mode = newMode;
		}

		VersionStatus getVersionStatus (void) {
			return(VERSION_STATUS_GOOD);
		}

		long update (void);

		long beginSessionScan (char* ipAddress, bool persistent = true);

		long endSessionScan (void);

		bool getSessionScanFailed (void) {
			return(false);
		}

		MC2Session* getSessions (long& sessionCount);

		// for read purposes
		const MC2Player* getPlayers (long& playerCount);

		// for write purposes....
		MC2Player* getPlayerInfo( long commanderID )
		{
			return NULL;
		}

		bool hostSession(char* sessionName, char* playerName, long mxPlayers);

		long joinSession (MC2Session* session, char* playerName);

		bool getOnLAN (void) {
			return(onLAN);
		}

		bool isServerMissing (void);

		void updateSessionData (MC2SessionData* sessionData);

		void setLocked (bool set);

		void setCancelled (bool set);

		void setInProgress (bool set);

		bool getLocked (void) {
			return(locked);
		}

		bool getLaunchedFromLobby (void) {
			return(launchedFromLobby);
		}

		bool getInProgress (void) {
			return(inProgress);
		}

		void addTeamScore (long teamID, long score);

		long closeSession (void);

		void logMessage (NETMESSAGE* message, bool sent);
		
		long bootPlayer (NETPLAYER bootedPlayer);

		void sendMessage (NETPLAYER player,
						  void* data,
						  int dataSize,
						  bool guaranteed,
						  bool toSelf = true);

		bool hostGame (char* sessionName, char* playerName, long nPlayers);

		long joinGame (char* ipAddress, char* sessionName, char* playerName);

		bool waitTillStartLoading (void);

		bool waitTillMechDataReceived (void);

		bool waitTillMissionLoaded (void);

		bool waitTillMissionSetup (void);

		bool waitForSessionEntry (void);

		bool playersReadyToLoad (void);

		void handlePlayerLeftSession (NETPLAYER leftPlayer);

		void handleTerminateSession (void);

		bool launchBrowser (const char* link);

		void initParametersScreen (void);

		void setDefaultPlayerInfo (void);

		void initSpecialBuildings (char commandersToLoad[8][3]);

		char* getPlayerName (void) {
			return(NULL);
		}

		void addToLocalMovers (MoverPtr mover);

		void removeFromLocalMovers (MoverPtr mover);

		void addToMoverRoster (MoverPtr mover);

		bool canAddToMoverRoster (void)
		{
			return false;
		}

		void removeFromMoverRoster (MoverPtr mover);

		void addToTurretRoster (TurretPtr turret);

		void addToPlayerMoverRoster (long playerCommanderID, MoverPtr mover);

		void removeFromPlayerMoverRoster (MoverPtr mover);

		long getNumWorldChunks (void) {
			return(numWorldChunks);
		}

		void clearWorldChunks (void) {
			numWorldChunks = 0;
		}

		long setClosestColor (long colorIndex, long commanderID);

		long setNextFreeColor (long commanderID);

		void setColor (long colorIndex, long commanderID);

		void setPlayerBaseColor (long commanderID, long colorIndex);

		void setPlayerTeam (long commanderID, long teamID);

		void calcPlayerRanks (void);

		long addWorldChunk (WorldChunkPtr chunk);

		long addMissionScriptMessageChunk (long code, long param1);

//		long addTerrainAlignmentChunk (GameObjectPtr object, long alignment);

		long addArtilleryChunk (long artilleryType, long teamId, Stuff::Vector3D location, long seconds);

		long addMineChunk (long tileR, long tileC, long teamId, long mineState, long explosionType);

		long addLightOnFireChunk (GameObjectPtr object, long seconds);

		long addPilotKillStat (MoverPtr mover, long vehicleClass);

		long addScoreChunk (long commanderID, long score);

		long addKillLossChunk (long killerCID, long loserCID);

		long addCaptureBuildingChunk (BuildingPtr building, long prevCommanderID, long newCommanderID);

		long addEndMissionChunk (void);

		long grabWorldChunks (unsigned long* packedChunkBuffer);

		long updateWorldChunks (void);

		long getNumWeaponHitChunks (void) {
			return(numWeaponHitChunks);
		}

		void clearWeaponHitChunks (void) {
			numWeaponHitChunks = 0;
		}

		long addWeaponHitChunk (WeaponHitChunkPtr chunk);

		long addWeaponHitChunk (GameObjectPtr target, WeaponShotInfoPtr shotInfo, bool isRefit = false);

		void grabWeaponHitChunks (unsigned long* packedChunkBuffer, long numChunks);

		long updateWeaponHitChunks (void);

		long getHomeTeamPlayers (DWORD* playerIdList);

		long getEnemyTeamPlayers (DWORD* playerIdList);

		bool processGameMessage (NETMESSAGE* msg);

		void processMessages (void);

		long findPlayer (NETPLAYER player) {
			return(-1);
		}

		bool calcMissionStatus (void);

		void handleChat (NETPLAYER sender, MCMSG_Chat* msg);

		void handlePlayerCID (NETPLAYER sender, MCMSG_PlayerCID* msg);

		void handlePlayerUpdate (NETPLAYER sender, MCMSG_PlayerUpdate* msg);

		void handleMissionSettingsUpdate (NETPLAYER sender, MCMSG_MissionSettingsUpdate* msg);

		void handlePlayerInfo (NETPLAYER sender, MCMSG_PlayerInfo* msg);

		void handlePlayerCheckIn (NETPLAYER sender, MCMSG_PlayerCheckIn* msg);

		void handlePlayerSetup (NETPLAYER sender, MCMSG_PlayerSetup* msg);

		void handlePlayerInsignia (NETPLAYER sender, MCMSG_PlayerInsignia* msg);

		void handleMissionSetup (NETPLAYER sender, MCMSG_MissionSetup* msg);

		void handleStartPlanning (NETPLAYER sender);

		void handleReadyForBattle (NETPLAYER sender, MCMSG_PlayerCheckIn* msg);

		void handleStartMission (NETPLAYER sender);
		
		void handleEndMission (NETPLAYER sender, MCMSG_EndMission* msg);
		
		void handleReinforcement (NETPLAYER sender, MCMSG_Reinforcement* msg);

		void handleNewServer (NETPLAYER sender, MCMSG_NewServer* msg);

		void handleLeaveSession (NETPLAYER sender, MCMSG_LeaveSession* msg);

		void handleHoldPosition (NETPLAYER sender, MCMSG_HoldPosition* msg);

		void handlePlayerOrder (NETPLAYER sender, MCMSG_PlayerOrder* msg);

		void handlePlayerMoverGroup (NETPLAYER sender, MCMSG_PlayerMoverGroup* msg);
		
		void handlePlayerArtillery (NETPLAYER sender, MCMSG_PlayerArtillery* msg);

		void handleMoverUpdate (NETPLAYER sender, MCMSG_MoverUpdate* msg);

		void handleTurretUpdate (NETPLAYER sender, MCMSG_TurretUpdate* msg);

		void handleMoverWeaponFireUpdate (NETPLAYER sender, MCMSG_MoverWeaponFireUpdate* msg);

		void handleTurretWeaponFireUpdate (NETPLAYER sender, MCMSG_TurretWeaponFireUpdate* msg);

		void handleMoverCriticalUpdate (NETPLAYER sender, MCMSG_MoverCriticalUpdate* msg);

		void handleWeaponHitUpdate (NETPLAYER sender, MCMSG_WeaponHitUpdate* msg);

		void handleWorldUpdate (NETPLAYER sender, MCMSG_WorldUpdate* msg);

		void sendChat (NETPLAYER receiver, char team, char* chatString);
		void sendPlayerActionChat(NETPLAYER receiver, const char* playerName, unsigned long resID );

		void sendPlayerCID (NETPLAYER receiver, unsigned char subType, char CID);

		void sendPlayerUpdate (NETPLAYER receiver, long stage, long commanderID);

		void sendMissionSettingsUpdate (NETPLAYER receiver);

		void sendPlayerInfo (NETPLAYER receiver);

		void sendPlayerCheckIn (void);

		void sendPlayerSetup (void);

		void sendPlayerInsignia (char* insigniaFileName, unsigned char* insigniaData, long insigniaDataSize);

		void sendMissionSetup (NETPLAYER receiver, long stage, CompressedMech* mechData);

		void sendStartPlanning (void);

		void sendStartMission (void);

		void sendEndMission (long result);

		void sendReinforcement (long vehicleID, long rosterIndex, char pilotName[16], long commanderID, Stuff::Vector3D pos, unsigned char stage);

		void sendNewServer (void);

		void sendLeaveSession (char subType, char commanderID);

		void sendPlayerOrder (TacticalOrderPtr tacOrder,
							  bool needsSelection,
							  long numMovers,
							  MoverPtr* moverList,
							  long numGroups = 0,
							  MoverGroupPtr* groupList = NULL,
  							  bool queuedOrder = false);

		void sendHoldPosition (void);

		void sendPlayerMoverGroup (long groupId,
								   long numMovers,
								   MoverPtr* moverList,
								   long point);

		void sendPlayerArtillery (long strikeType, Stuff::Vector3D location, long seconds);
			
		long sendPing(void);

		void sendMoverUpdate (void);

		void sendTurretUpdate (void);

		void sendMoverWeaponFireUpdate (void);

		void sendTurretWeaponFireUpdate (void);

		void sendMoverCriticalUpdate (void);

		void sendWeaponHitUpdate (void);

		void sendWorldUpdate (void);

		void sendFile (char* path, char* fileName);

		void sendReadyForBattle (void);

		void sendPrepareMission (void);

		void sendFileInquiry (char* filename);

		long updateServer (void);

		void buildMoverRosterRLE (void);

		long updateClients (bool forceIt = false);

		bool isMyTeammate (DWORD playerId);

		bool allPlayersCheckedIn (void);

		bool allPlayersReady (void);

		void switchServers (void);

		void calcDropZones (char dropZonesCID[8], char hqs[MAX_TEAMS]);

		long saveTranscript (const char* fileName, bool debugging = false);

		void playerLeftGame (DWORD playerId);

		// call this to exit the current session.
		void leaveSession (void);

		void getChatMessages( char** buffer, long* playerIDs, long& count );

		void redistributeRP();
};

bool StartupNetworking (void);
void ResetNetworking (void);
void ShutdownNetworking (void);

extern MultiPlayer* MPlayer;

//***************************************************************************

#endif

