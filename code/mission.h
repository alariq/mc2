//******************************************************************************************
//	mission.h - This file contains the mission class header
//		Missions are what scenarios were in MechCommander 1.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MISSION_H
#define MISSION_H
//----------------------------------------------------------------------------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifndef MISSIONGUI_H
#include"missiongui.h"
#endif

#ifndef OBJECTIVE_H
#include"objective.h"
#endif

#ifndef LOGISTICSMECH_H
#include"logisticsmech.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

//----------------------------------------------------------------------------------
// Macro Definitions
#define mis_PLAYING						0
#define mis_PLAYER_LOST_BIG				1
#define mis_PLAYER_LOST_SMALL			2
#define mis_PLAYER_DRAW					3
#define mis_PLAYER_WIN_SMALL 			4
#define mis_PLAYER_WIN_BIG	 			5

extern unsigned long scenarioResult;

//----------------------------------------------------------------------------------
// These are placeholders for the scenario specific objects.  This allows us to
// create objects which the scenario can manipulate using other scenario data.
// For example, a part may be assigned to a certain area.  This would inform
// the part of where its supposed to be for AI AND allow us to remove the part
// if all player controlled objects leave that area.  What the folks downstairs
// would call an interest manager except that its simple and it works!
struct Part
{
	GameObjectWatchID	objectWID;			//Pointer to my physical incarnation
	unsigned long		objNumber;			//What kind of object am I?
	DWORD				baseColor;			//Base color of mech -- Overrides the RED in RGB
	DWORD				highlightColor1;	//First Highlight Color -- Overrides the GREEN in RGB
	DWORD				highlightColor2;	//Second Highlight Color -- Overrides the BLUE in RGB
	long				active;				//Am I currently awake?
	long				exists;				//Am I currently in Existance?
	bool				destroyed;			//Have I been destroyed for this scenario?
	Stuff::Vector3D		position;			//Where do I start?  (relative to area)
	float				velocity;			//How fast am I going?
	float				rotation;			//Which direction am I facing?
	unsigned long		gestureId;			//What gesture do I start in?
	char				alignment;			//Who do I fight for?
	char				teamId;				//Which team am I on?
	long				commanderID;		//Used when setting up multiplayer
	char				squadId;				//Which team am I on?
	char				myIcon;				//If set, start with Icon on Screen for this part.
	unsigned long		controlType;		//How am I controlled?
	unsigned long		controlDataType;	//What data do you need to control me?
	char				profileName[9];		//Name of Object Profile file.
	unsigned long		pilot;				//Name of Pilot File.
	bool				captureable;		//Is this a capturable "enemy" mech?
	DWORD				variantNum;			//Variant number of the Part.

	void Save (FitIniFilePtr file, long partNum);
	void Load (FitIniFilePtr file, long partNum);
};

typedef Part *PartPtr;

//----------------------------------------------------------------------------------
// This is the struct definition for the Scenario Objectives.
// These can be displayed in a window.  For now they are drawn with just text.
// There are a whole series of ABL commands to address these objectives.
typedef unsigned long	ObjectiveType;
#define					InvisibleGoal	-1
#define					PrimaryGoal		0
#define					SecondaryGoal	1
#define					OtherGoal		2
#define					BonusGoal		3
#define 				InvalidGoal		9999

typedef unsigned long	ObjectiveStatus;
#define					Incomplete		0
#define					Success			1
#define					Failed			2
#define					InvalidStatus	9999

#define					SCENARIO_TIMER_1		7
#define					SCENARIO_TIMER_2		(SCENARIO_TIMER_1 + 1)
#define					SCENARIO_TIMER_3		(SCENARIO_TIMER_1 + 2)
#define					SCENARIO_TIMER_4		(SCENARIO_TIMER_1 + 3)
#define					SCENARIO_TIMER_5		(SCENARIO_TIMER_1 + 4)
#define					SCENARIO_TIMER_6		(SCENARIO_TIMER_1 + 5)
#define					SCENARIO_TIMER_7		(SCENARIO_TIMER_1 + 6)
#define					SCENARIO_TIMER_8		(SCENARIO_TIMER_1 + 7)

#define					OBJECTIVE_1_TIMER		24
#define					OBJECTIVE_2_TIMER		(OBJECTIVE_1_TIMER + 1)
#define					OBJECTIVE_3_TIMER		(OBJECTIVE_1_TIMER + 2)
#define					OBJECTIVE_4_TIMER		(OBJECTIVE_1_TIMER + 3)
#define					OBJECTIVE_5_TIMER		(OBJECTIVE_1_TIMER + 4)
#define					OBJECTIVE_6_TIMER		(OBJECTIVE_1_TIMER + 5)
#define					OBJECTIVE_7_TIMER		(OBJECTIVE_1_TIMER + 6)
#define					OBJECTIVE_8_TIMER		(OBJECTIVE_1_TIMER + 7)

#define					MAX_OBJECTIVES			50

typedef enum {
	MISSION_LOAD_SP_QUICKSTART,
	MISSION_LOAD_SP_LOGISTICS,
	MISSION_LOAD_MP_QUICKSTART,
	MISSION_LOAD_MP_LOGISTICS
} MissionLoadType;

struct Objective
{
	char				name[80];
	ObjectiveType		type;
	float				timeLeft;
	ObjectiveStatus		status;
	Stuff::Vector3D		position;
	long				points;
	float				radius;
};

typedef Objective *ObjectivePtr;

//----------------------------------------------------------------------------------
// To operate, simply call init with the filename of the mission.
// that's all she wrote.  update and render return the mission completion status.
class Mission
{

	//Data Members
	//-------------
	protected:
	
		FitIniFilePtr					missionFile;
										
		long							operationId;		// aka operation id
		long							missionId;			// aka mission id
										
		char							missionFileName[80];
		char							missionScriptName[80];
		ABLModulePtr					missionBrain;
		ABLParamPtr						missionParams;
		SymTableNodePtr					missionBrainCallback;
										
		unsigned long					numParts;
		PartPtr							parts;
										
		bool							active;

		static double					missionTerminationTime;

										
	public:								

		static bool						terminationCounterStarted;
		static unsigned long			terminationResult;

		unsigned long					numObjectives;
		ObjectivePtr					objectives;
		float						m_timeLimit;
		//CObjectives						missionObjectives;
		//int						numPrimaryObjectives;
		unsigned long					duration;
		bool							warning1;
		bool							warning2;
										
		float							actualTime;
		float							runningTime;
										
		long 							numSmallStrikes;
		long 							numLargeStrikes;
		long 							numSensorStrikes;
		long 							numCameraStrikes;
										
		unsigned char					missionTuneNum;
										
		long							missionScriptHandle;
		ABLParam						*missionBrainParams;
		
		MissionInterfaceManagerPtr		missionInterface;

		Stuff::Vector3D					dropZone;
		
		long							theSkyNumber;

		static bool						statisticsInitialized;

	//Member Functions
	//-----------------
	protected:
	
	public:
	
		void init (void)
		{
			missionFile = NULL;

			operationId = missionId = -1;

			missionBrain = NULL;
			missionParams = NULL;
			missionBrainCallback = NULL;

			numParts = 0;
			parts = NULL;

			numObjectives = 0;
			objectives = NULL;

			duration = 0;

			active = FALSE;
			
			numSmallStrikes = 0;			
			numLargeStrikes = 0;
			numSensorStrikes = 0;
			numCameraStrikes = 0;
			
			missionTuneNum = 0;
			
			missionScriptHandle = -1;
			
			missionInterface = NULL;

			missionFileName[0] = 0;

			missionBrainParams = NULL;
		}
		
		Mission (void)
		{
			init();
		}

		bool calcComplexDropZones (char* missionName, char dropZoneList[MAX_MC_PLAYERS]);

		void init (const char *missionName, long loadType, long dropZoneID, Stuff::Vector3D* dropZoneList, char commandersToLoad[8][3], long numMoversPerCommander);

		static void initBareMinimum();
		
		static void initTGLForMission();
		static void initTGLForLogistics();
		
		void start (void);

		Stuff::Vector3D getDropZone( ) const { return dropZone; }
		
		long update (void);
		long render (void);
		
		void destroy (bool initLogistics = true);
		
		~Mission (void)
		{
			destroy(false);
		}

		long getStatus (void);

		long getNumParts (void) 
		{
			return(numParts);
		}

		PartPtr getPart (long partNumber) 
		{
			return(&parts[partNumber]);
		}

		long getPartTeamId (long partNumber) 
		{
			return(parts[partNumber].teamId);
		}

		GameObjectPtr getPartObject (long partNumber)
		{
			if ((partNumber <= 0) || ((unsigned long)partNumber > numParts))
				return NULL;
			if (!ObjectManager)
				return(NULL);
			return ((GameObjectPtr) ObjectManager->getByWatchID(parts[partNumber].objectWID));
		}

		long addMover (MoverInitData* moveSpec);
		long addMover (MoverInitData* moveSpec, LogisticsMech* mechData);
		long addMover (MoverInitData* moveSpec, CompressedMech* mechData);
		
		long removeMover (MoverPtr mover);

		void tradeMover (MoverPtr mover, long newTeamID, long newCommanderID, char* pilotFileName, const char* brainFileName);

		void createPartObject (long objectId, MoverPtr mover);
		
		void createMissionObject (long partId);		//Moves object from holding area to real world.
		
		ABLModulePtr getBrain (void) 
		{
			return(missionBrain);
		}

		void handleMultiplayMessage (long code, long param1);

		//-----------------------------------------------------
		// Objective Routines
		void startObjectiveTimers (void);
		
		long setObjectiveTimer (long objectiveNum, float timeLeft);
		float checkObjectiveTimer (long objectiveNum);
		
		long setObjectiveStatus (long objectiveNum, ObjectiveStatus status);
		ObjectiveStatus checkObjectiveStatus (long objectiveNum);
		
		long setObjectiveType (long objectiveNum, ObjectiveType type);
		ObjectiveType checkObjectiveType (long objectiveNum);
		
		void setObjectivePos (long objectiveNum, float realX, float realY, float realZ);
		
		void setupBonus (void);
		
		//Checks if any objective has succeeded or failed since we last checked.
		bool checkObjectiveSuccess (void);
		bool checkObjectiveFailed (void);
		//-----------------------------------------------------

		long GetOperationID(void) 
		{
			return operationId;
		}

		long GetMissionID(void) 
		{
			return missionId;
		}

		char * getMissionName (void)
		{
			return missionScriptName;
		}

		long getMissionTuneId (void)
		{
			return missionTuneNum;
		}

		const char* getMissionFileName(){ return missionFileName; }
		
		static void initializeStatistics();

		void load (const char *filename);
		void save (const char *filename);

		bool isActive (void)
		{
			return active;
		}

};

//----------------------------------------------------------------------------------

#define MAX_GLOBAL_MISSION_VALUES		50

extern float worldUnitsPerMeter;
extern float metersPerWorldUnit;
extern float maxVisualRange;
extern float fireVisualRange;
extern bool CantTouchThis;

extern float MineDamage;
extern float MineSplashDamage;
extern float MineSplashRange;
extern long MineExplosion;

extern float globalMissionValues [];

extern unsigned char godMode;

extern Mission *mission;
extern unsigned long scenarioResult;

extern UserHeapPtr missionHeap;
//----------------------------------------------------------------------------------
#endif
