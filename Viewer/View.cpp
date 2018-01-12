//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// Viewer.cpp : Defines the class behaviors for the application.
//


#ifndef VERSION_H
#include "version.h"
#endif

#include <gameos.hpp>
#include <toolos.hpp>

#include "../resource.h"

#include "mclib.h"
#include "gamesound.h"
#include <windows.h>
#include "mission.h"
#include "mechlopedia.h"
#include "logisticsdata.h"
#include "prefs.h"


CPrefs prefs;


SoundSystem*	 sndSystem = NULL;

MultiPlayer* MPlayer = NULL;

//CPrefs prefs;

extern float frameRate;

// globals used for memory
UserHeapPtr systemHeap = NULL;
UserHeapPtr guiHeap = NULL;

long GameVisibleVertices = 30;
bool useLeftRightMouseProfile = true;

float MaxMinUV = 8.0f;

unsigned long systemHeapSize = 65535000;
unsigned long guiHeapSize = 1023999;
unsigned long tglHeapSize = 32767000;
bool		  GeneralAlarm = 0;

extern long DigitalMasterVolume;
extern long MusicVolume;
extern long sfxVolume;
extern long RadioVolume;

extern char FileMissingString[];
extern char CDMissingString[];
extern char MissingTitleString[];

extern char CDInstallPath[];

long FilterState = gos_FilterNone;
long gammaLevel = 0;
long renderer = 0;
long GameDifficulty = 0;
long resolutionX = 0;
long resolutionY = 0;
bool useUnlimitedAmmo = true;

Camera* eye = NULL;
unsigned long BaseVertexColor  =0;

#ifdef LINUX_BUILD
#else
enum { CPU_UNKNOWN, CPU_PENTIUM, CPU_MMX, CPU_KATMAI } Processor = CPU_PENTIUM;		//Needs to be set when GameOS supports ProcessorID -- MECHCMDR2
#endif

bool	reloadBounds = false;
int     ObjectTextureSize = 128;
char	missionName[1024];
float	gosFontScale = 1.0;

float   doubleClickThreshold;
float	dragThreshold;


HSTRRES gosResourceHandle = 0;
HGOSFONT3D gosFontHandle = 0;


bool quitGame = FALSE;


// these globals are necessary for fast files for some reason
FastFile 	**fastFiles = NULL;
long 		numFastFiles = 0;
long		maxFastFiles = 0;

char*	ExceptionGameMsg = NULL;

bool	justResaveAllMaps = 0;
bool	useLOSAngle = 0;

Stuff::MemoryStream *effectStream = NULL;
extern MidLevelRenderer::MLRClipper * theClipper;



Mechlopedia*	pMechlopedia;
LogisticsData*  pLogData;


const char *SpecialtySkillsTable[NUM_SPECIALTY_SKILLS] = {
	"LightMechSpecialist",
	"LaserSpecialist",
	"LightACSpecialist",
	"MediumACSpecialist",
	"SRMSpecialist",
	"SmallArmsSpecialist",
	"SensorProfileSpecialist",
	"ToughnessSpecialist",			//Thoughness Specialty

	"MediumMechSpecialist",
	"PulseLaserSpecialist",
	"ERLaserSpecialist",
	"LRMSpecialist",
	"Scout",						//Scouting Specialty
	"LongJump",						//Jump Jet Specialty

	"HevayMechSpecialist",			//Heavy mech Specialty
	"PPCSpecialist",
	"HeavyACSpecialist",
	"ShortRangeSpecialist",
	"MediumRangeSpecialist",
	"LongRangeSpecialist",

	"AssaultMechSpecialist",
	"GaussSpecialist",
	"SharpShooter",					//Sharpshooter specialty
};

// called by gos
//---------------------------------------------------------------------------
char* __stdcall GetGameInformation() 
{
	return(ExceptionGameMsg);
}

// called by GOS when you need to draw
//---------------------------------------------------------------------------
void __stdcall UpdateRenderers()
{
	
	DWORD bColor = 0x0;
	
 	gos_SetupViewport(1,1.0,1,bColor, 0.0, 0.0, 1.0, 1.0 );		//ALWAYS FULL SCREEN for now

	gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear );
	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );

	gos_SetRenderState( gos_State_AlphaTest, TRUE );

	gos_SetRenderState( gos_State_Clipping, TRUE);

	gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );

	gos_SetRenderState( gos_State_Dither, 1);
	
	float viewMulX, viewMulY, viewAddX, viewAddY;

	gos_GetViewport(&viewMulX, &viewMulY, &viewAddX, &viewAddY);
			
	//------------------------------------------------------------
	gos_SetRenderState( gos_State_Filter, gos_FilterNone );

	pMechlopedia->render();
	
	gos_SetRenderState( gos_State_Filter, gos_FilterNone );
	userInput->setViewport(viewMulX,viewMulY,viewAddX,viewAddY);
	userInput->render();

}

//------------------------------------------------------------
void __stdcall DoGameLogic()
{
	//---------------------------------------------------------------
	// Somewhere in all of the updates, we have asked to be excused!
	if (quitGame)
	{
		//EnterWindowMode();				//Game crashes if _TerminateApp called from fullScreen
		gos_TerminateApplication();
	}

	if (frameRate < Stuff::SMALL)
		frameRate = 4.0f;

	frameLength = 1.0 / frameRate;
	if (frameLength > 0.25f)
		frameLength = 0.25f;

	userInput->update();

	soundSystem->update();

	pMechlopedia->update();

	if ( LogisticsScreen::RUNNING != pMechlopedia->getStatus() )
		quitGame = true;
}



//---------------------------------------------------------------------------
void __stdcall InitializeGameEngine()
{
#ifdef PLATFORM_WINDOWS
#ifdef _WIN64
	gosResourceHandle = gos_OpenResourceDLL("mc2res_64.dll", NULL, 0);
#else
	gosResourceHandle = gos_OpenResourceDLL("mc2res_32.dll", NULL, 0);
#endif
#else
	gosResourceHandle = gos_OpenResourceDLL("./libmc2res.so", NULL, 0);
#endif
	
	char temp[256];
	cLoadString( IDS_FLOAT_HELP_FONT, temp, 255 );
	char* pStr = strstr( temp, "," );
	if ( pStr )
	{
		gosFontScale = atoi( pStr + 2 );
		*pStr = 0;
	}
	char path [256];
	strcpy( path, "assets" PATH_SEPARATOR "graphics" PATH_SEPARATOR );
	strcat( path, temp );	

	gosFontHandle = gos_LoadFont(path);

   	//-------------------------------------------------------------
   	// Find the CDPath in the registry and save it off so I can
   	// look in CD Install Path for files.
	//Changed for the shared source release, just set to current directory
	//DWORD maxPathLength = 1023;
	//gos_LoadDataFromRegistry("CDPath", CDInstallPath, &maxPathLength);
	//if (!maxPathLength)
	//	strcpy(CDInstallPath,"..\\");
#ifdef LINUX_BUILD    
	strcpy(CDInstallPath,"../FinalBuild/");
#else
	strcpy(CDInstallPath,".\\");
#endif

	cLoadString(IDS_MC2_FILEMISSING,FileMissingString,511);
	cLoadString(IDS_MC2_CDMISSING,CDMissingString,1023);
	cLoadString(IDS_MC2_MISSING_TITLE,MissingTitleString,255);

	//--------------------------------------------------------------
	// Start the SystemHeap and globalHeapList
	globalHeapList = new HeapList;
	gosASSERT(globalHeapList != NULL);

	globalHeapList->init();
	globalHeapList->update();		//Run Instrumentation into GOS Debugger Screen

	systemHeap = new UserHeap;
	gosASSERT(systemHeap != NULL);

	systemHeap->init(systemHeapSize,"SYSTEM");
	
	float doubleClickThreshold = 0.2f;
	long dragThreshold = .016667;

	//--------------------------------------------------------------
	// Read in System.CFG
	FitIniFile systemFile;

#ifdef _DEBUG
	long systemOpenResult = 
#endif
		systemFile.open("system.cfg");
		   
#ifdef _DEBUG
	if( systemOpenResult != NO_ERR)
	{
		char Buffer[256];
		gos_GetCurrentPath( Buffer, 256 );
		STOP(( "Cannot find \"system.cfg\" file in %s",Buffer ));
	}
#endif

	{
#ifdef _DEBUG
		long systemBlockResult = 
#endif
		systemFile.seekBlock("systemHeap");
		gosASSERT(systemBlockResult == NO_ERR);
		{
			long result = systemFile.readIdULong("systemHeapSize",systemHeapSize);
			gosASSERT(result == NO_ERR);
		}

#ifdef _DEBUG
		long systemPathResult = 
#endif
		systemFile.seekBlock("systemPaths");
		gosASSERT(systemPathResult == NO_ERR);
		{
			long result = systemFile.readIdString("terrainPath",terrainPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("artPath",artPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("fontPath",fontPath,79);
			gosASSERT(result == NO_ERR);

			//result = systemFile.readIdString("savePath",savePath,79);
			//gosASSERT(result == NO_ERR);

            // sebi: get user dependent savegame directory
            char userDataDir[1024] = {0};
            if(!gos_GetUserDataDirectory(userDataDir, sizeof(userDataDir))) {
                SPEW(("PATHS", "Failed to get user data directory"));
                gos_TerminateApplication();
            }

            snprintf(savePath, sizeof(savePath), "%s" PATH_SEPARATOR "%s" PATH_SEPARATOR, userDataDir, "savegame" );

            SPEW(("SAVELOAD", savePath));

			result = systemFile.readIdString("spritePath",spritePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("shapesPath",shapesPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("soundPath",soundPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("objectPath",objectPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("cameraPath",cameraPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("tilePath",tilePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("missionPath",missionPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("warriorPath",warriorPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("profilePath",profilePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("interfacepath",interfacePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("moviepath",moviePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("CDsoundPath",CDsoundPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("CDmoviepath",CDmoviePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("tglPath",tglPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("texturePath",texturePath,79);
			gosASSERT(result == NO_ERR);
		}

#ifdef _DEBUG
		long fastFileResult = 
#endif
		systemFile.seekBlock("FastFiles");
		gosASSERT(fastFileResult == NO_ERR);
		{
			long result = systemFile.readIdLong("NumFastFiles",maxFastFiles);
			if (result != NO_ERR)
				maxFastFiles = 0;

			if (maxFastFiles)
			{
				fastFiles = (FastFile **)malloc(maxFastFiles*sizeof(FastFile *));
				memset(fastFiles,0,maxFastFiles*sizeof(FastFile *));

				long fileNum = 0;
				char fastFileId[10];
				char fileName[100];
				sprintf(fastFileId,"File%d",fileNum);
	
				while (systemFile.readIdString(fastFileId,fileName,99) == NO_ERR)
				{
					bool result = FastFileInit(fileName);
					if (!result)
						STOP(("Unable to startup fastfiles.  Probably an old one in the directory!!"));

					fileNum++;
					sprintf(fastFileId,"File%d",fileNum);
				}
			}
		}
	}

	systemFile.close();

		//--------------------------------------------------------------
	// Read in Prefs.cfg
	bool fullScreen = false;
	FitIniFilePtr prefs = new FitIniFile;

#ifdef _DEBUG
	long prefsOpenResult = 
#endif
		prefs->open("prefs.cfg");

	gosASSERT (prefsOpenResult == NO_ERR);
	{
#ifdef _DEBUG
		long prefsBlockResult = 
#endif
		prefs->seekBlock("MechCommander2");
		gosASSERT(prefsBlockResult == NO_ERR);
		{
			long filterSetting;
			long result = prefs->readIdLong("FilterState",filterSetting);
			if (result == NO_ERR)
			{
				switch (filterSetting)
				{
				default:
					case 0:
						FilterState = gos_FilterNone;
					break;

					case 1:
						FilterState = gos_FilterBiLinear;
					break;

					case 2:
						FilterState = gos_FilterTriLinear;
					break;
				}
			}

            long tmp;
			result = prefs->readIdLong("TerrainTextureRes", tmp);
			if (result != NO_ERR)
				TERRAIN_TXM_SIZE = 64;
            else {
                TERRAIN_TXM_SIZE = tmp;
            }

			result = prefs->readIdLong("ObjectTextureRes", tmp);
			if (result != NO_ERR)
				ObjectTextureSize = 128;
            else {
                ObjectTextureSize = tmp;
            }

			result = prefs->readIdLong("Brightness",gammaLevel);
			if (result != NO_ERR)
				gammaLevel = 0;

			// store volume settings in global variable since soundsystem 
			// does not exist yet.  These will be set in SoundSystem::init()
			result = prefs->readIdLong("DigitalMasterVolume",DigitalMasterVolume);
			if (result != NO_ERR)
				DigitalMasterVolume = 255;

			result = prefs->readIdLong("MusicVolume",MusicVolume);
			if (result != NO_ERR)
				MusicVolume = 64;

			result = prefs->readIdLong("RadioVolume",RadioVolume);
			if (result != NO_ERR)
				RadioVolume = 64;

			result = prefs->readIdLong("SFXVolume",sfxVolume);
			if (result != NO_ERR)
				sfxVolume = 64;

			result = prefs->readIdFloat("DoubleClickThreshold",doubleClickThreshold);
			if (result != NO_ERR)
				doubleClickThreshold = 0.2f;

			result = prefs->readIdLong("DragThreshold",dragThreshold);
			if (result != NO_ERR)
				dragThreshold = .016667;
				
			result = prefs->readIdULong("BaseVertexColor",BaseVertexColor);
			if (result != NO_ERR)
				BaseVertexColor = 0x00000000;
				
			result = prefs->readIdBoolean("FullScreen",fullScreen);
			if (result != NO_ERR)
				fullScreen = true;

			result = prefs->readIdLong("Rasterizer",renderer);
			if (result != NO_ERR)
				renderer = 0;

			if ((renderer < 0) || (renderer > 3))
				renderer = 0;
		}
	}
	
	prefs->close();
	
	delete prefs;
	prefs = NULL;

 	//-------------------------------
	// Used to output debug stuff!
	// Mondo COOL!
	// simply do this in the code and stuff goes to the file called mc2.output
	//		DEBUG_STREAM << thing_you_want_to_output
	//
	// IMPORTANT NOTE:
	Stuff::InitializeClasses();
	MidLevelRenderer::InitializeClasses(8192*4,1024,0,0,true);
	gosFX::InitializeClasses();
	
	gos_PushCurrentHeap(MidLevelRenderer::Heap);

	// sebi NB! changed, but maybe original data/Effects is ok, if we use ".fst"s
	//MidLevelRenderer::TGAFilePool *pool = new MidLevelRenderer::TGAFilePool("data/Effects/");
	MidLevelRenderer::TGAFilePool *pool = new MidLevelRenderer::TGAFilePool("data/tgl/128/");
	//
	
	MidLevelRenderer::MLRTexturePool::Instance = new MidLevelRenderer::MLRTexturePool(pool);

	MidLevelRenderer::MLRSortByOrder *cameraSorter = new MidLevelRenderer::MLRSortByOrder(MidLevelRenderer::MLRTexturePool::Instance);
	theClipper = new MidLevelRenderer::MLRClipper(0, cameraSorter);
	
	gos_PopCurrentHeap();

	//------------------------------------------------------
	// Start the GOS FX.
	gos_PushCurrentHeap(gosFX::Heap);
	
	gosFX::EffectLibrary::Instance = new gosFX::EffectLibrary();
	Check_Object(gosFX::EffectLibrary::Instance);

	FullPathFileName effectsName;
	effectsName.init(effectsPath,"mc2.fx","");

	File effectFile;
	long result = effectFile.open(effectsName);
	if (result != NO_ERR)
		STOP(("Could not find MC2.fx"));
		
	long effectsSize = effectFile.fileSize();
	MemoryPtr effectsData = (MemoryPtr)systemHeap->Malloc(effectsSize);
	effectFile.read(effectsData,effectsSize);
	effectFile.close();
	
	effectStream = new Stuff::MemoryStream(effectsData,effectsSize);
	gosFX::EffectLibrary::Instance->Load(effectStream);
	
	gosFX::LightManager::Instance = new gosFX::LightManager();

	gos_PopCurrentHeap();

	systemHeap->Free(effectsData);
	

	//------------------------------------------------
	// Fire up the MC Texture Manager.
	mcTextureManager = new MC_TextureManager;
	mcTextureManager->start();

	//Startup the vertex array pool
	mcTextureManager->startVertices(500000);
	mcTextureManager->startShapes(50000);

	//--------------------------------------------------
	// Setup Mouse Parameters from Prefs.CFG
	userInput = new UserInput;
	userInput->init();
	userInput->setMouseDoubleClickThreshold(doubleClickThreshold);
	userInput->setMouseDragThreshold(dragThreshold);
	userInput->initMouseCursors( "cursors" );
	userInput->setMouseCursor( mState_NORMAL );
	userInput->mouseOn();



	// now the sound system
	soundSystem = new GameSoundSystem;
	soundSystem->init();
	((SoundSystem *)soundSystem)->init("sound");
	sndSystem = soundSystem; // for things in the lib that use sound
	soundSystem->playDigitalMusic( LOGISTICS_LOOP );

	
	pLogData = new LogisticsData;
	pLogData->init();
	

	pMechlopedia = new Mechlopedia;
	pMechlopedia->init();
	pMechlopedia->begin();




}

void __stdcall TerminateGameEngine()
{

	if ( pMechlopedia )
		delete pMechlopedia;

	if ( userInput )
		delete userInput;

	if ( soundSystem )
		delete soundSystem;

	if ( pLogData )
		delete pLogData;

	//------------------------------------------------
	// shutdown the MC Texture Manager.
	if (mcTextureManager)
	{
		mcTextureManager->destroy();

		delete mcTextureManager;
		mcTextureManager = NULL;
	}

	//--------------------------------------------------------------
	// End the SystemHeap and globalHeapList
	if (systemHeap)
	{
		systemHeap->destroy();

		delete systemHeap;
		systemHeap = NULL;
	}


	if (globalHeapList)
	{
		globalHeapList->destroy();

		delete globalHeapList;
		globalHeapList = NULL;
	}

	//----------------------------------------------------
	// Shutdown the MLR and associated stuff libraries
	//----------------------------------------------------
	gos_PushCurrentHeap(gosFX::Heap);

	delete effectStream;
	delete gosFX::LightManager::Instance;

	gos_PopCurrentHeap();

	//
	//-------------------
	// Turn off libraries
	//-------------------
	//
	gosFX::TerminateClasses();
	MidLevelRenderer::TerminateClasses();
	Stuff::TerminateClasses();

	//Redundant.  Something else is shutting this down.
	//GOS sure does think its bad to delete something multiple times though.
	//Even though it simply never is!
	//gos_DeleteFont(gosFontHandle);

	gos_CloseResourceDLL(gosResourceHandle);

	//
	//--------------------------
	// Turn off the fast Files
	//--------------------------
	//
	FastFileFini();
	
	
}


//---------------------------------------------------------------------
void __stdcall GetGameOSEnvironment( const char* CommandLine )
{
	Environment.applicationName			= "MechCommander 2 Encyclopedia";

	Environment.debugLog				= "";			//"DebugLog.txt";
	Environment.memoryTraceLevel		= 5;
	Environment.spew					= ""; //"GameOS_Texture GameOS_DirectDraw GameOS_Direct3D ";
	Environment.TimeStampSpew			= 0;

	Environment.GetGameInformation		= GetGameInformation;
	Environment.UpdateRenderers			= UpdateRenderers;
	Environment.InitializeGameEngine	= InitializeGameEngine;
	Environment.DoGameLogic				= DoGameLogic;
	Environment.TerminateGameEngine		= TerminateGameEngine;

    Environment.checkCDForFiles         = true;
	
	if (useSound)
	{
		Environment.soundDisable			= FALSE;
		Environment.soundHiFi				= TRUE;
		Environment.soundChannels			= 24;
	}
	else
	{
		Environment.soundDisable			= TRUE;
		Environment.soundHiFi				= FALSE;
		Environment.soundChannels			= 0;
	}

	Environment.version					= versionStamp;

	Environment.AntiAlias				= 0;
//
// Texture infomation
//
	Environment.Texture_S_256			= 6;
	Environment.Texture_S_128			= 1;
	Environment.Texture_S_64			= 0;
	Environment.Texture_S_32			= 1;
	Environment.Texture_S_16			= 5;

	Environment.Texture_K_256			= 2;
	Environment.Texture_K_128			= 5;
	Environment.Texture_K_64			= 5;
	Environment.Texture_K_32			= 5;
	Environment.Texture_K_16			= 5;

	Environment.Texture_A_256			= 0;
	Environment.Texture_A_128			= 1;
	Environment.Texture_A_64			= 5;
	Environment.Texture_A_32			= 1;
	Environment.Texture_A_16			= 0;

	Environment.bitDepth				= 16;

	Environment.RaidDataSource			= "MechCommander 2:Raid4"; 
	Environment.RaidFilePath			= "\\\\aas1\\MC2\\Test\\GOSRaid";
	Environment.RaidCustomFields		= "Area=GOSRaid"; 	

	
	Environment.screenWidth = 800;
	Environment.screenHeight = 600;
}


