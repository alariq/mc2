/*************************************************************************************************\
MissionBegin.cpp			: Implementation of the MissionBegin component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"missionbegin.h"
#include"mclib.h"
#include"objmgr.h"
#include"mech.h"
#include"logisticsvariant.h"
#include"mechicon.h"
#include"logisticsdata.h"
#include"missionselectionscreen.h"
#include"mechbayscreen.h"
#include"pilotreadyscreen.h"
#include"mechpurchasescreen.h"
#include"mechlabscreen.h"
#include"missionbriefingscreen.h"
#include"mpconnectiontype.h"
#include"mpparameterscreen.h"
#include"mpgamebrowser.h"
#include"mploadmap.h"
#include"mainmenu.h"
#include"mission.h"
#include"gamesound.h"
#include"loadscreen.h"
#include"mpprefs.h"
#include"chatwindow.h"
#include"logisticsmechicon.h"

#include"prefs.h"
extern CPrefs prefs;

class MechLabScreen;

extern long renderer;

void initABL (void);
void closeABL (void);

//Tutorial
// Please save these two flags with the saveGames!!
bool	MissionBegin::FirstTimePurchase = true;
bool	MissionBegin::FirstTimeMechLab = true;

MissionBegin::MissionBegin()
{
	memset( screens, 0, sizeof( LogisticsScreen* ) * 5/*dim screen X*/ * 3/*dim screen Y*/ );
	memset( singlePlayerScreens, 0, sizeof( LogisticsScreen* ) * 5/*dim screen X*/ * 3/*dim screen Y*/ );
	memset( multiplayerScreens, 0, sizeof( LogisticsScreen* ) * 5/*dim screen X*/ * 3/*dim screen Y*/ );

	curScreenX = -1;
	curScreenY = 1;

	mainMenu = NULL;

	bSplash = 0;
	bMultiplayer = 0;
	animJustBegun = 0;
	placeHolderScreen = NULL;

}

MissionBegin::~MissionBegin()
{
	for ( int i = 0; i < 5; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			if ( singlePlayerScreens[i][j] )
			{
				delete singlePlayerScreens[i][j];
				singlePlayerScreens[i][j] = NULL;
			}
			if ( multiplayerScreens[i][j] )
			{
				delete multiplayerScreens[i][j];
				multiplayerScreens[i][j] = NULL;
			}
		}
	}

	delete LogisticsMechIcon::s_pTemplateIcon;
	LogisticsMechIcon::s_pTemplateIcon = NULL;

	delete placeHolderScreen;
	placeHolderScreen = NULL;

	if (mainMenu)
	{
		delete mainMenu;
		mainMenu = NULL;
	}
}

bool MissionBegin::startAnimation (long bId, bool isButton, float scrollTime, long nFlashes)
{
	if (animationRunning)
		return false;
	else
	{
		animationRunning = true;
		timeLeftToScroll = scrollTime;
		targetButtonId = bId;
		targetIsButton = isButton;
		buttonNumFlashes = nFlashes;
		buttonFlashTime = 0.0f;
	}

	return true;
}

void MissionBegin::begin()
{
	bReadyToLoad = 0;
	initABL();

	//-----------------------------------------------
	// Tutorial Data
	animationRunning = false;
	timeLeftToScroll = 0.0f;
	targetButtonId = 0;
	buttonNumFlashes = 0;
	buttonFlashTime = 0.0f;

	//---------------------------------------------
	//Load up the Logistics Brain for Tutorials.
	// OK if brain file is NOT there!!
	FullPathFileName brainFileName;
	const char * brainfile = LogisticsData::instance->getCurrentABLScript();
	if ( brainfile )
		 brainFileName.init(missionPath, brainfile, ".abl");
	
	if (brainfile && fileExists(brainFileName))
	{
		long numErrors, numLinesProcessed;
		logisticsScriptHandle = ABLi_preProcess(brainFileName, &numErrors, &numLinesProcessed);
		gosASSERT(logisticsScriptHandle >= 0);
		
		logisticsBrain = new ABLModule;
		gosASSERT(logisticsBrain != NULL);
			
	#ifdef _DEBUG
		long brainErr = 
	#endif
			logisticsBrain->init(logisticsScriptHandle);
		gosASSERT(brainErr == NO_ERR);
		
		logisticsBrain->setName("Logistics");
	}
	else
	{
		logisticsScriptHandle = 0;
		logisticsBrain = NULL;
	}

	//---------------------------------------------
	DWORD localRenderer = prefs.renderer;
	if (prefs.renderer != 0 && prefs.renderer != 3)
		localRenderer = 0;

   	bool localFullScreen = prefs.fullScreen;
   	bool localWindow = !prefs.fullScreen;
   	if (Environment.fullScreen && prefs.fullScreen)
   		localFullScreen = false;

	if (prefs.renderer == 3)
		gos_SetScreenMode(800,600,16,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
	else if (prefs.bitDepth)
		gos_SetScreenMode(800,600,32,prefs.renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
	else
		gos_SetScreenMode(800,600,16,prefs.renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);

	if ( mainMenu ) // already initialized
	{
		curScreenX = 0;
		curScreenY = 1;

		if ( LogisticsData::instance->skipLogistics() )
		{
					
			if ( LogisticsData::instance->showChooseMission() )
			{
				curScreenX = 3;
				curScreenY = 1;
				screens[3][1] = singlePlayerScreens[0][1];
			}
		}

		if ( screens[curScreenX][curScreenY] )
		{
			screens[curScreenX][curScreenY]->beginFadeIn( 1.0 );
			screens[curScreenX][curScreenY]->begin();
		}

		Mission::initTGLForLogistics();
		bDone = 0;
		return;
	}
	
	MissionSelectionScreen*		pMissionSelectionScreen;
	MechBayScreen*				pMechBayScreen;
	PilotReadyScreen*			pPilotSelectionScreen;
	MechLabScreen*				pMechLabScreen;
	MechPurchaseScreen*			pPurchaseMechScreen;
	MissionBriefingScreen*		pBriefingScreen;
	LoadScreenWrapper*			pLoadScreen;

	pMissionSelectionScreen = NULL;
	pMechBayScreen = NULL;
	pPilotSelectionScreen = NULL;
	pMechLabScreen = NULL;
	pPurchaseMechScreen = NULL;
	pBriefingScreen = NULL;
	pLoadScreen = NULL;

	bDone = 0;

	// initialize the main menu
	mainMenu = new MainMenu;

	char path[256];
	strcpy( path, artPath );
	strcat( path, "mcl_mm.fit" );

	FitIniFile file;
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;
	}

	mainMenu->init( file );
	mainMenu->setDrawBackground( true );
	mainMenu->begin();
	file.close();

	
	// initialize mission selection
	pMissionSelectionScreen = new MissionSelectionScreen();
	strcpy( path, artPath );
	strcat( path, "mcl_cm_layout.fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;
	}

	
	pMissionSelectionScreen->init( &file );	
	file.close();

	// initialize mission briefing
	pBriefingScreen = new MissionBriefingScreen();
	strcpy( path, artPath );
	strcat( path, "mcl_mn.fit" );
	
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;
	}
	pBriefingScreen->init( &file );	
	file.close();

	// initialize mech bay
	strcpy( path, artPath );
	strcat( path, "mcl_mb_layout.fit" );
	
	pMechBayScreen = new MechBayScreen();
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;
	}

	// initialize animations, these are held in the mech bay file
	pMechBayScreen->init( &file );	
	file.seekBlock( "DownAnim" );
	downAnim.init(&file, "");
	file.seekBlock("UpAnim"); 
	upAnim.init( &file, "" );
	file.seekBlock( "NextAnim" );
	leftAnim.init( &file, "" );
	file.seekBlock( "BackAnim" );
	rightAnim.init( &file, "" );
	file.close();


	// initialize pilot ready
	strcpy( path, artPath );
	strcat( path, "mcl_pr_layout.fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;	
	}



	// initialize pilot ready
	pPilotSelectionScreen = new PilotReadyScreen;
	pPilotSelectionScreen->init( &file );

	file.close();


	// initalize purchase pilot
	pPurchaseMechScreen = new MechPurchaseScreen;

	strcpy( path, artPath );
	strcat( path, "mcl_m$.fit" );

	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pPurchaseMechScreen->init( file );
	file.close();

	// initialize mech lab
	pMechLabScreen = new MechLabScreen;

	strcpy( path, artPath );
	strcat( path, "mcl_mc.fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pMechLabScreen->init( file );
	file.close();

	// initialize mech lab
	pLoadScreen = new LoadScreenWrapper;

	strcpy( path, artPath );
	strcat( path, "mcl_loadingscreen.fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pLoadScreen->init( file );
	file.close();

	singlePlayerScreens[0][1] = pMissionSelectionScreen;
	singlePlayerScreens[1][1] = pBriefingScreen;
	singlePlayerScreens[2][1] = pMechBayScreen;
	singlePlayerScreens[3][1] = pPilotSelectionScreen;
	singlePlayerScreens[2][0] = pPurchaseMechScreen;
	singlePlayerScreens[2][2] = pMechLabScreen;
	singlePlayerScreens[4][1] = pLoadScreen;

	for ( int i = 0; i < 4; i++ )
	{
		for ( int j = 0; j < 4; j++ )
		{
			if ( singlePlayerScreens[i][j] )
			{
				/*if (  singlePlayerScreens[i][j]->getButton(MB_MSG_NEXT) )
					singlePlayerScreens[i][j]->getButton(MB_MSG_NEXT)->setPressFX( -1 );

				if ( singlePlayerScreens[i][j]->getButton(MB_MSG_PREV) )
					singlePlayerScreens[i][j]->getButton(MB_MSG_PREV)->setPressFX( -1 );*/
				if ( singlePlayerScreens[i][j]->getButton(MB_MSG_MAINMENU) )
					singlePlayerScreens[i][j]->getButton(MB_MSG_MAINMENU)->setPressFX( LOG_MAINMENUBUTTON );
			}
			
		}
	}




	for (i = 0; i < 5/*dim screen X*/; i+=1)
	{
		int j;
		for (j = 0; j < 3/*dim screen Y*/; j += 1)
		{
			screens[i][j] = singlePlayerScreens[i][j];
		}
	}

	pMissionSelectionScreen->begin();

}

void MissionBegin::init()
{

	begin();
}

void MissionBegin::end()
{
	logisticsBrain = NULL;
	closeABL();
}

bool inPurchase = false;
bool inMechLab = false;
//Returns screen ID as a function of curScreenX and curScreenY 
long MissionBegin::getCurrentScreenId()
{
	//singlePlayerScreens[0][1] = pMissionSelectionScreen;		ID 1
	//singlePlayerScreens[1][1] = pBriefingScreen;				ID 11
	//singlePlayerScreens[2][1] = pMechBayScreen;				ID 21
	//singlePlayerScreens[3][1] = pPilotSelectionScreen;		ID 31
	//singlePlayerScreens[2][0] = pPurchaseMechScreen;			ID 20
	//singlePlayerScreens[2][2] = pMechLabScreen;				ID 22
	//singlePlayerScreens[4][1] = pLoadScreen;					ID 41
	long screenId = 10 * curScreenX + curScreenY;

	if ((screenId == 20) && FirstTimePurchase && !MPlayer )
		inPurchase = true;
	else if ((screenId == 22) && FirstTimeMechLab && !MPlayer )
		inMechLab = true;

	if ((screenId != 20) && inPurchase)
		FirstTimePurchase = false;
	else if ((screenId != 22) && inMechLab)
		FirstTimeMechLab = false;

	if ((screenId == 20) && !FirstTimePurchase)
		screenId = 0;
	else if ((screenId == 22) && !FirstTimeMechLab)
		screenId = 0;

	return (screenId);
}

const char* MissionBegin::update()
{

	if ( bSplash )
	{
		mainMenu->update();
		if ( LogisticsScreen::RUNNING != mainMenu->getStatus() )
		{
			bSplash = 0;
			if ( LogisticsScreen::RESTART == mainMenu->getStatus() )
			{
				LogisticsScreen*		pCurScreen = screens[curScreenX][curScreenY];
				if ( pCurScreen )
					pCurScreen->end();
				int i;
				for (i = 0; i < 5/*dim screen X*/; i+=1)
				{
					int j;
					for (j = 0; j < 3/*dim screen Y*/; j += 1)
					{
						screens[i][j] = singlePlayerScreens[i][j];
					}
				}
				if ( LogisticsData::instance->skipLogistics() )
				{
					if ( LogisticsData::instance->showChooseMission() )
					{
						curScreenX = 3;
						curScreenY = 1;
						screens[3][1] = singlePlayerScreens[0][1];
						screens[curScreenX][curScreenY]->begin();
					}
					else
					{
						curScreenX = 4;
						curScreenY = 1;
						screens[curScreenX][curScreenY]->begin();
					}
				}
				else
				{
					curScreenX = 0;
					curScreenY = 1;
					screens[curScreenX][curScreenY]->beginFadeIn( 1.0 );
					screens[curScreenX][curScreenY]->begin();
				}
				return LogisticsData::instance->getCurrentBigVideo();

			}
			else if ( LogisticsScreen::MULTIPLAYERRESTART == mainMenu->getStatus() )
			{
				LogisticsScreen*		pCurScreen = screens[curScreenX][curScreenY];
				if ( pCurScreen )
					pCurScreen->end();

				beginMPlayer();
				int i;
				for (i = 0; i < 5/*dim screen X*/; i+=1)
				{
					int j;
					for (j = 0; j < 3/*dim screen Y*/; j += 1)
					{
						screens[i][j] = multiplayerScreens[i][j];
					}
				}
				curScreenX = 0;
				curScreenY = 1;
				screens[curScreenX][curScreenY]->beginFadeIn( 1.0 );
			}
			else if ( LogisticsScreen::SKIPONENEXT == mainMenu->getStatus() )
			{
				LogisticsScreen*		pCurScreen = screens[curScreenX][curScreenY];
				if ( pCurScreen )
					pCurScreen->end();

				int i;
				for (i = 0; i < 5/*dim screen X*/; i+=1)
				{
					int j;
					for (j = 0; j < 3/*dim screen Y*/; j += 1)
					{
						screens[i][j] = singlePlayerScreens[i][j];
					}
				}
				curScreenX = 1;
				curScreenY = 1;
				screens[curScreenX][curScreenY]->beginFadeIn( 1.0 );
				screens[curScreenX][curScreenY]->begin();
			
			}
			if ( screens[curScreenX][curScreenY] && curScreenX != -1  )
			{
				screens[curScreenX][curScreenY]->begin();
			}
			else // no screen? stay on main menu
			{
				bSplash = true;
				mainMenu->begin();
			}
				
		}
		return NULL;
	}

	leftAnim.update();
	rightAnim.update();
	downAnim.update();
	upAnim.update();

	
	LogisticsScreen*		pCurScreen = screens[curScreenX][curScreenY];
	if ( pCurScreen )
	{
		if (logisticsBrain && !MPlayer)
			logisticsBrain->execute();

		//--------------------
		//For Tutorial
		if (animationRunning)
		{
			//Move mouse to correct position.
			if (targetIsButton)
			{
				aButton *targetButton = pCurScreen->getButton(targetButtonId);
				if (!targetButton)
				{
					animationRunning = false;
				}
				else
				{
					userInput->setMouseCursor(mState_TUTORIALS);

					//Get button position.
					float buttonPosX = (targetButton->left() + targetButton->right()) * 0.5f;

					float buttonPosY = (targetButton->top() + targetButton->bottom()) * 0.5f;

					//-------------------
					// Mouse Checks Next
					float realMouseX = userInput->realMouseX();
					float realMouseY = userInput->realMouseY();

					if (timeLeftToScroll > 0.0f)
					{
						float xDistLeft = buttonPosX - realMouseX;
						float yDistLeft = buttonPosY - realMouseY;

						float xDistThisFrame = xDistLeft / timeLeftToScroll * frameLength;
						float yDistThisFrame = yDistLeft / timeLeftToScroll * frameLength;

						userInput->setMousePos(realMouseX + xDistThisFrame, realMouseY+yDistThisFrame);

						timeLeftToScroll -= frameLength;
					}
					else
					{
						userInput->setMousePos(buttonPosX,buttonPosY);

						//We are there.  Start flashing.
						if (buttonNumFlashes)
						{
							buttonFlashTime += frameLength;
							if ( buttonFlashTime > .5f )
							{
								pCurScreen->getButton( targetButtonId )->setColor( 0xffffffff );
								buttonFlashTime = 0.0f;
								buttonNumFlashes--;
							}
							else if ( buttonFlashTime > .25f )
							{
								pCurScreen->getButton( targetButtonId )->setColor( 0xff7f7f7f );
							}
						}
						else
						{
							//Flashing is done.  We now return you to your regularly scheduled program.
							animationRunning = false;
							pCurScreen->getButton( targetButtonId )->setColor( 0xffffffff );
						}
					}
				}
			}
			else
			{
				aRect *targetButton = pCurScreen->getRect(targetButtonId);
				if (!targetButton)
				{
					animationRunning = false;
				}
				else
				{
					userInput->setMouseCursor(mState_TUTORIALS);
		
					//Get button position.
					float buttonPosX = (targetButton->left() + targetButton->right()) * 0.5f;
		
					float buttonPosY = (targetButton->top() + targetButton->bottom()) * 0.5f;
		
					//-------------------
					// Mouse Checks Next
					float realMouseX = userInput->realMouseX();
					float realMouseY = userInput->realMouseY();
		
					if (timeLeftToScroll > 0.0f)
					{
						float xDistLeft = buttonPosX - realMouseX;
						float yDistLeft = buttonPosY - realMouseY;
		
						float xDistThisFrame = xDistLeft / timeLeftToScroll * frameLength;
						float yDistThisFrame = yDistLeft / timeLeftToScroll * frameLength;
		
						userInput->setMousePos(realMouseX + xDistThisFrame, realMouseY+yDistThisFrame);
		
						timeLeftToScroll -= frameLength;
					}
					else
					{
						userInput->setMousePos(buttonPosX,buttonPosY);
		
						//We are there.  Start flashing.
						if (buttonNumFlashes)
						{
							buttonFlashTime += frameLength;
							if ( buttonFlashTime > .5f )
							{
								pCurScreen->getRect( targetButtonId )->setColor( 0xff000000 );
								buttonFlashTime = 0.0f;
								buttonNumFlashes--;
							}
							else if ( buttonFlashTime > .25f )
							{
								pCurScreen->getRect( targetButtonId )->setColor( 0xffffffff );
							}
						}
						else
						{
							//Flashing is done.  We now return you to your regularly scheduled program.
							animationRunning = false;
							pCurScreen->getRect( targetButtonId )->setColor( 0xff000000 );
						}
					}
				}
			}
		}

		pCurScreen->update();

		if ( pCurScreen->getStatus() == LogisticsScreen::GOTOSPLASH || ( MPlayer && MPlayer->hostLeft ) )
		{
			pCurScreen->end();
			beginSplash();
			if ( MPlayer &&  MPlayer->hostLeft )
			{
				MPlayer->closeSession();
				delete MPlayer;
				MPlayer = NULL;
			}
				
			return NULL;
		}

		if ( pCurScreen->getStatus() == LogisticsScreen::READYTOLOAD && 
			(curScreenX == 4 ||( MPlayer && curScreenX == 3 )) )
			bReadyToLoad = true;


		if ( pCurScreen->getStatus() != LogisticsScreen::RUNNING )
		{
			soundSystem->stopBettySample(); // don't want to carry droning on to next screen
			soundSystem->stopSupportSample();
			if ( pCurScreen->getStatus() == LogisticsScreen::NEXT )
			{
				pCurScreen->end();
				if ( curScreenX < 4 )
				{
					if ( MPlayer )// different rules for multiplayer
					{
						if ( dynamic_cast<MPParameterScreen*>( screens[curScreenX][curScreenY] ) )
						{
							if ( curScreenX == 2 )
							{
								
								if ( !MPlayer->missionSettings.quickStart)
								{
									setUpMultiplayerLogisticsScreens();	
								}						
							}
							else if ( curScreenX == 3 )
							{
								bDone = true;
							}
						}
					}
					if ( screens[curScreenX+1][curScreenY] )
					{
						if ( screens[curScreenX+1][curScreenY] == placeHolderScreen )
							curScreenX++;
						screens[curScreenX+1][curScreenY]->begin();
					}


					leftAnim.begin();
					curScreenX++;
					soundSystem->playDigitalSample( LOG_NEXTBACKBUTTONS );
					animJustBegun = true;
				}
				else
				{
					bDone = true;
				}

				
			}
			else if ( pCurScreen->getStatus() == LogisticsScreen::PREVIOUS )
			{
				pCurScreen->end();
				if (screens[curScreenX-1][curScreenY] ) 
				{
					if ( screens[curScreenX-1][curScreenY] == placeHolderScreen )
						curScreenX--;
				
					screens[curScreenX-1][curScreenY]->begin();
				}
				rightAnim.begin();
				curScreenX--;
				soundSystem->playDigitalSample( LOG_NEXTBACKBUTTONS );
				animJustBegun = true;
			}
			else if ( pCurScreen->getStatus() == LogisticsScreen::DOWN )
			{
				pCurScreen->end();
				if ( screens[curScreenX][curScreenY+1] )
					screens[curScreenX][curScreenY+1]->begin();
				upAnim.begin();
				curScreenY++;
				soundSystem->playDigitalSample( LOG_NEXTBACKBUTTONS );
				animJustBegun = true;
			}
			else if ( pCurScreen->getStatus() == LogisticsScreen::UP )
			{
				pCurScreen->end();
				if (screens[curScreenX][curScreenY-1]) 
					screens[curScreenX][curScreenY-1]->begin();
				downAnim.begin();
				curScreenY--;
				soundSystem->playDigitalSample( LOG_NEXTBACKBUTTONS );
				animJustBegun = true;
			}
			else if ( pCurScreen->getStatus() == LogisticsScreen::MAINMENU )
			{
				bSplash = true;
				mainMenu->setDrawBackground( false );
				mainMenu->begin();
				animJustBegun = true;
			}
			else if ( pCurScreen->getStatus() == LogisticsScreen::SKIPONENEXT )
			{
				pCurScreen->end();
				if ( curScreenX < 3 - 1 )
				{
					if ( screens[curScreenX+1+1][curScreenY] )
						screens[curScreenX+1+1][curScreenY]->begin();
					leftAnim.begin();
					curScreenX++;
					curScreenX++;
					soundSystem->playDigitalSample( LOG_NEXTBACKBUTTONS );
					animJustBegun = true;
				}
				else
				{
					bDone = true;
				}
			}
			else if ( pCurScreen->getStatus() == LogisticsScreen::SKIPONEPREVIOUS )
			{
				pCurScreen->end();
				if (screens[curScreenX-1-1][curScreenY] ) 
					screens[curScreenX-1-1][curScreenY]->begin();
				rightAnim.begin();
				curScreenX--;
				curScreenX--;
				soundSystem->playDigitalSample( LOG_NEXTBACKBUTTONS );
				animJustBegun = true;
			}
		} 
	}
	else
		bDone = true;

	return NULL;

}

void MissionBegin::render()
{

	long xOffset = 0;
	long yOffset = 0;
	
	LogisticsScreen* pOtherScreen = 0;
	LogisticsScreen* pCurScreen = 0;
	if ( curScreenX > -1 && curScreenY > -1 )
	{
		pCurScreen = screens[curScreenX][curScreenY];

	}

	long xOtherOffset = 0;
	long yOtherOffset = 0;

	if ( bSplash )
	{
		if ( pCurScreen )
		{
			if ( !MainMenu::bDrawMechlopedia)
				pCurScreen->render();
			else
				pCurScreen->beginFadeIn(1.0);
		}
		mainMenu->render();		
		return;
	}


	if ( pCurScreen /*&& pCurScreen->getStatus() == LogisticsScreen::RUNNING*/ )
	{
		if ( leftAnim.isAnimating() && !leftAnim.isDone() )
		{
			if ( animJustBegun )
				leftAnim.begin(); // restart to compensate for LONG begins

			xOffset = leftAnim.getXDelta() + 800;			
			yOffset = leftAnim.getYDelta();
			xOtherOffset = xOffset - 800;
			yOtherOffset = yOffset;
			pOtherScreen = screens[curScreenX-1][curScreenY];
			if ( pOtherScreen == placeHolderScreen )
				pOtherScreen = screens[curScreenX-2][curScreenY];
		}
		else if ( downAnim.isAnimating() && !downAnim.isDone() )
		{
			if ( animJustBegun )
				downAnim.begin(); // restart to compensate for LONG begins

			xOffset = downAnim.getXDelta();
			yOffset = downAnim.getYDelta();
			xOtherOffset = xOffset;
			yOtherOffset = yOffset + 600;
			pOtherScreen = screens[curScreenX][curScreenY + 1];
		}
		else if ( upAnim.isAnimating() && !upAnim.isDone() )
		{
			if ( animJustBegun )
				upAnim.begin(); // restart to compensate for LONG begins

			xOffset = upAnim.getXDelta();
			yOffset = upAnim.getYDelta();
			xOtherOffset = xOffset;
			yOtherOffset = yOffset - 600;
			pOtherScreen = screens[curScreenX][curScreenY - 1];
		}
		else if ( rightAnim.isAnimating() && !rightAnim.isDone() )
		{
			if ( animJustBegun )
				rightAnim.begin(); // restart to compensate for LONG begins

			xOffset = rightAnim.getXDelta() - 800;
			yOffset = rightAnim.getYDelta();
			xOtherOffset = xOffset + 800;
			yOtherOffset = yOffset;
			pOtherScreen = screens[curScreenX+1][curScreenY];
			if ( pOtherScreen == placeHolderScreen )
				pOtherScreen = screens[curScreenX+2][curScreenY];

		}

		if ( curScreenX == 4 ) // don't scroll last screen
		{
			if ( pOtherScreen )
				pOtherScreen->render( 0, 0 );
			pCurScreen->render( 0, 0 );
		}

		else
		{
			if ( pOtherScreen )
				pOtherScreen->render( xOtherOffset, yOtherOffset );
			pCurScreen->render( xOffset, yOffset );

		}

	

	}

	animJustBegun = false;
}

void MissionBegin::beginSplash( const char* playerName)
{
	// check for old screen and end that
	if ( curScreenX > -1 && curScreenY > -1 )
	{
		LogisticsScreen*		pCurScreen = screens[curScreenX][curScreenY];
		if ( pCurScreen )
			pCurScreen->end();
	}


	bSplash = true;
	curScreenX = 0;
	curScreenY = 1;
	bReadyToLoad = 0;
	bDone = 0;
	if ( mainMenu )
	{
		if ( MPlayer && MPlayer->launchedFromLobby )
			mainMenu->skipIntro();

		mainMenu->setDrawBackground( true );
		mainMenu->begin();
		soundSystem->playDigitalSample( LOG_MAINMENUBUTTON );
		if (playerName)
			mainMenu->setHostLeftDlg(playerName);
	}
}

void MissionBegin::beginMPlayer()
{
	// already set up
	if ( multiplayerScreens[0][1] )
		return;


		//multiplayer setup screens
	MPConnectionType*		pMPConnectionType = NULL;
	placeHolderScreen = NULL;
	MPGameBrowser*		pMPGameBrowser = NULL;
	MPParameterScreen*		pMPParameterScreen = NULL;

	char path[512];
	FitIniFile file;
	// initalize MPConnectionType
	pMPConnectionType = new MPConnectionType;

	strcpy( path, artPath );
	strcat( path, "mcl_mp_connectiontype.fit" );

	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pMPConnectionType->init( &file );
	file.close();

	// initalize MPPlaceHolderScreen
	placeHolderScreen = new MPPlaceHolderScreen;

	// initalize MPParameterScreen
	pMPParameterScreen = new MPParameterScreen;

	strcpy( path, artPath );
	strcat( path, "mcl_mp_param.fit" );

	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pMPParameterScreen->init( &file );
	file.close();

	// initalize MPGameBrowser
	pMPGameBrowser = new MPGameBrowser;

	strcpy( path, artPath );
	strcat( path, "mcl_mp_lanbrowser.fit" );

	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pMPGameBrowser->init( &file );
	file.close();


	// initalize MP prefs
	MPPrefs* pMPPrefs = new MPPrefs;

	strcpy( path, artPath );
	strcat( path, "mcl_mp_playerprefs.fit" );

	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pMPPrefs->init( file );
	file.close();

	LoadScreenWrapper* pMLoadScreen = new LoadScreenWrapper;

	strcpy( path, artPath );
	strcat( path, "mcl_loadingscreen.fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
		return;		
	}

	pMLoadScreen->init( file );
	file.close();


	
	pMPConnectionType->ppConnectionScreen = (void **)(&(screens[1][1]));
	pMPConnectionType->pLocalBrowserScreen = pMPGameBrowser;
	pMPConnectionType->pDirectTcpipScreen = pMPGameBrowser;
	pMPConnectionType->pMPPlaceHolderScreen = placeHolderScreen;

	
	multiplayerScreens[0][1] = pMPConnectionType;
	multiplayerScreens[1][1] = pMPGameBrowser;
	multiplayerScreens[2][1] = pMPParameterScreen;
	multiplayerScreens[3][1] = pMLoadScreen;
	multiplayerScreens[2][0] = pMPPrefs;

	pMPPrefs->initColors();
	

	{
		for ( int i = 0; i < 4; i++ )
		{
			for ( int j = 0; j < 4; j++ )
			{
				if ( multiplayerScreens[i][j] )
				{
					if (  multiplayerScreens[i][j]->getButton(MB_MSG_NEXT) )
						multiplayerScreens[i][j]->getButton(MB_MSG_NEXT)->setPressFX( -1 );

					if ( multiplayerScreens[i][j]->getButton(MB_MSG_PREV) )
						multiplayerScreens[i][j]->getButton(MB_MSG_PREV)->setPressFX( -1 );
					if ( multiplayerScreens[i][j]->getButton(MB_MSG_MAINMENU) )
						multiplayerScreens[i][j]->getButton(MB_MSG_MAINMENU)->setPressFX( LOG_MAINMENUBUTTON );
				}
				
			}
		}
	}
}

void MissionBegin::setUpMultiplayerLogisticsScreens()
{


	for (int i = 0; i < 5/*dim screen X*/; i+=1)
	{
		int j;
		for (j = 0; j < 3/*dim screen Y*/; j += 1)
		{
			screens[i][j] = singlePlayerScreens[i][j];
		}
	}
	curScreenX = 0;
	curScreenY = 1;

}

void MissionBegin::setToMissionBriefing()
{
	bReadyToLoad = 0;

	if ( screens[curScreenX][curScreenY] )
		screens[curScreenX][curScreenY]->end();
	{
		curScreenX = 1;
		curScreenY = 1;
		if ( screens[curScreenX][curScreenY] )
			screens[curScreenX][curScreenY]->begin();
	}

}

void MissionBegin::restartMPlayer( const char* playerName )
{
	bReadyToLoad = 0;
	bDone = 0;

	if ( screens[curScreenX][curScreenY] )
		screens[curScreenX][curScreenY]->end();

	for (int i = 0; i < 5/*dim screen X*/; i+=1)
	{
		int j;
		for (j = 0; j < 3/*dim screen Y*/; j += 1)
		{
			screens[i][j] = this->multiplayerScreens[i][j];
		}
	}

		curScreenX = 2;
		curScreenY = 1;
		if ( screens[curScreenX][curScreenY] )
		{
			screens[curScreenX][curScreenY]->begin();

			if ( playerName )
			{
				((MPParameterScreen*)screens[curScreenX][curScreenY])->setHostLeftDlg(playerName);
			}
		}



}

void MissionBegin::beginZone()
{
	beginMPlayer();
	restartMPlayer(NULL);
	bReadyToLoad = 0;
	bDone = 0;

	LogisticsData::instance->startMultiPlayer();

	mainMenu->skipIntro();

	ChatWindow::init();


}
void MissionBegin::beginAtConnectionScreen()
{

	beginMPlayer();
	int i;
	for (i = 0; i < 5/*dim screen X*/; i+=1)
	{
		int j;
		for (j = 0; j < 3/*dim screen Y*/; j += 1)
		{
			screens[i][j] = multiplayerScreens[i][j];
		}
	}
	curScreenX = 0;
	curScreenY = 1;
	screens[curScreenX][curScreenY]->beginFadeIn( 1.0 );
	bReadyToLoad = 0;
	bDone = 0;

	LogisticsData::instance->startMultiPlayer();

	ChatWindow::init();


}