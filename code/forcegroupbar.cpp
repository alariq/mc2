#define FORCEGROUPBAR_CPP
/*************************************************************************************************\
ForceGroupBar.cpp			: Implementation of the ForceGroupBar component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"forcegroupbar.h"
#include"mechicon.h"
#include"objmgr.h"
#include"team.h"
#include"missiongui.h"
#include"controlgui.h"
#include "../resource.h"
#include"multplyr.h"
#include"mc2movie.h"
#include"comndr.h"
#include"prefs.h"
#include"gamesound.h"

float ForceGroupBar::iconWidth = 48;
float ForceGroupBar::iconHeight = 42;
int	  ForceGroupBar::iconsPerRow = 8;

StaticInfo*  ForceGroupBar::s_coverIcon = NULL;

extern bool useLeftRightMouseProfile;
extern char CDInstallPath[];
void EnterWindowMode();
void EnterFullScreenMode();
void __stdcall ExitGameOS();

#define BOTTOM_OFFSET	5 * Environment.screenHeight/640.f

#define FORCEGROUP_LEFT		ForceGroupIcon::selectionRect[0].left
#define FORCEGROUP_WIDTH	(ForceGroupIcon::selectionRect[7].right - ForceGroupIcon::selectionRect[0].left)
#define FORCEGROUP_HEIGHT	(ForceGroupIcon::selectionRect[0].bottom - ForceGroupIcon::selectionRect[0].top)

extern float frameRate;
 
ForceGroupBar::ForceGroupBar()
{
	for ( int i = 0; i < MAX_ICONS; ++i )
	{
		icons[i] = 0;
	}

	iconCount = 0;

	forceNumFlashes = 0;
	forceFlashTime = 0.0f;
}

ForceGroupBar::~ForceGroupBar()
{
	removeAll();
	
	if ( ForceGroupIcon::gosFontHandle )
		delete ForceGroupIcon::gosFontHandle;

	ForceGroupIcon::gosFontHandle  = NULL;
}


bool ForceGroupBar::flashJumpers (long numFlashes)
{
	forceNumFlashes = numFlashes;
	forceFlashTime = 0.0f;

	return true;
}

bool ForceGroupBar::addMech( Mover* pMover )
{
	if ( iconCount >= MAX_ICONS )
	{
		gosASSERT( false );
		return 0;
	}
	
	MechIcon* pIcon = new MechIcon;
	bool bRetVal = pIcon->init( pMover );

	icons[iconCount++] = pIcon;
	
	return bRetVal;
}

bool ForceGroupBar::addVehicle( Mover* pMover )
{
	if ( iconCount >= MAX_ICONS )
	{
		gosASSERT( false );
		return 0;
	}
	
	VehicleIcon* pIcon = new VehicleIcon;
	bool bRetVal = pIcon->init( pMover );

	icons[iconCount++] = pIcon;

	return bRetVal;
}

void ForceGroupBar::removeMover (Mover* mover) {

	for (long i = 0; i < iconCount; i++)
		if (icons[i]->unit == mover) {
			delete icons[i];
			iconCount --;
			memmove( &icons[i], &icons[i] + 1, (iconCount - i) * sizeof (ForceGroupIcon*) );
			icons[iconCount] = 0;
			break;
		}
}

void ForceGroupBar::update( )
{
	bool bSelect = userInput->isLeftClick();
	bool bCommand = useLeftRightMouseProfile ? userInput->isRightClick() : userInput->isLeftClick();
	bool shiftDn = userInput->getKeyDown( KEY_LSHIFT ) ? true : false;
	bool bCamera = useLeftRightMouseProfile ? (userInput->isLeftDoubleClick()) : (userInput->isRightClick() && !userInput->isRightDrag());
	bool bForceGroup = useLeftRightMouseProfile ? (userInput->isLeftDoubleClick()) : userInput->isLeftDoubleClick();
	
	if ( bCamera )
		bSelect = 0;

	Stuff::Vector2DOf<long> screen;
	screen.x = userInput->getMouseX();
	screen.y = userInput->getMouseY();

	 if ( screen.x > FORCEGROUP_LEFT && screen.x < FORCEGROUP_LEFT + FORCEGROUP_WIDTH
		  && screen.y > FORCEGROUP_TOP )
	 {
		 if ( ControlGui::instance->isSelectingInfoObject() )
			userInput->setMouseCursor( mState_INFO );
		 else if ( ControlGui::instance->getRepair() )
			 userInput->setMouseCursor( mState_XREPAIR );
		 else if ( MissionInterfaceManager::instance()->hotKeyIsPressed( EJECT_COMMAND_INDEX ) )
			 userInput->setMouseCursor( mState_EJECT );
		 else
			userInput->setMouseCursor( mState_NORMAL );

		helpTextID = IDS_FORCEGROUP_BAR_DESC;
		helpTextHeaderID = IDS_FORCEGROUP_BAR;
	 }



	// unselect all if appropriate
	if ( bSelect && !shiftDn && inRegion(screen.x, screen.y) 
		&& !ControlGui::instance->isSelectingInfoObject() && (!ControlGui::instance->getRepair()
		&& !MissionInterfaceManager::instance()->hotKeyIsPressed( EJECT_COMMAND_INDEX )
		&& !ControlGui::instance->getGuard()
		|| useLeftRightMouseProfile) )
	{
		Team* pTeam = Team::home;
		for ( int i = 0; i < pTeam->rosterSize; ++i )
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			if (pMover->getCommander()->getId() == Commander::home->getId())
			{
				pMover->setSelected( false );
			}
		}
	}

	
	// remove dead mechs
	for ( int t = 0; t < iconCount; ++t )
	{
		if ( (icons[t]->unit->isDestroyed() || icons[t]->unit->isDisabled()) && !icons[t]->unit->recoverBuddyWID )
		{
			if ( !icons[t]->isAnimatingDeath() )
				icons[t]->beginDeathAnimation();
			if ( icons[t]->deathAnimationOver() || icons[t]->unit->causeOfDeath == POWER_USED_UP )
			{
				delete icons[t];
				iconCount --;
				memmove( &icons[t], &icons[t] + 1, (iconCount - t) * sizeof (ForceGroupIcon*) );
				icons[iconCount] = 0;
			}
		}
	}

	qsort( icons, iconCount, sizeof( ForceGroupIcon* ), ForceGroupIcon::sort );

	for ( int i = 0; i < iconCount; i++ )
	{
		icons[i]->setLocationIndex( i );
	}
	

	for (int i = 0; i < iconCount; ++i )
	{
		if ( icons[i]->inRegion( screen.x, screen.y ) )
		{
			icons[i]->unit->setTargeted(true); 
			if ( ControlGui::instance->getRepair() )
			{
				if ( !MissionInterfaceManager::instance()->canRepair(icons[i]->unit ) )
				{
					userInput->setMouseCursor( mState_XREPAIR );
				
					// need to go back and unselect everything
					if ( bSelect  )
					{
						if ( !shiftDn )
						{
							Team* pTeam = Team::home;
							for ( int j = 0; j < pTeam->rosterSize; ++j )
							{
								Mover* pMover = (Mover*)pTeam->getMover( j );
								if (pMover->getCommander()->getId() == Commander::home->getId())
								{
									pMover->setSelected( false );
								}
							}
						}						
					}
				}
				else
				{
					userInput->setMouseCursor( mState_REPAIR );
				}
			}
			else if ( ControlGui::instance->getGuard() )
			{
				userInput->setMouseCursor( mState_GUARD );
			}
			else
			{
				ControlGui::instance->setRolloverHelpText( IDS_UNIT_SELECT_HELP );
			}

			if ( bSelect && !ControlGui::instance->infoButtonPressed() )
			{
				if ( !(ControlGui::instance->getRepair() && MissionInterfaceManager::instance()->canRepair(icons[i]->unit ) && !useLeftRightMouseProfile) )
					icons[i]->click( shiftDn ); 

				ControlGui::instance->setInfoWndMover( icons[i]->unit );	
			}

			if ( bCommand )
			{
				 if ( MissionInterfaceManager::instance()->hotKeyIsPressed( EJECT_COMMAND_INDEX ) )
				 {
					 MissionInterfaceManager::instance()->doEject( icons[i]->unit );
				 }
				 else if ( ControlGui::instance->getGuard() )
				 {
					 MissionInterfaceManager::instance()->doGuard( icons[i]->unit );
				 }
				 else if ( ControlGui::instance->getRepair() )
				 {
					 if ( MissionInterfaceManager::instance()->canRepair(icons[i]->unit ) )
						MissionInterfaceManager::instance()->doRepair( icons[i]->unit );
				 }
				 

				 else
					 ControlGui::instance->setInfoWndMover( icons[i]->unit );	
			}

			if ( bCamera )
			{
				icons[i]->rightClick();
			}

			if ( bForceGroup )
			{
				for( int j = 0; j < 10; ++j )
				{
					if ( icons[i]->unit->isInUnitGroup( j ) )
					{
						
						MissionInterfaceManager::selectForceGroup( j, true );

					}
				}				
			}
		
		}
		else
			icons[i]->unit->setTargeted( 0 );
		
		icons[i]->update();
		
	}

}

bool ForceGroupBar::inRegion( int x, int y )
{
	for ( int i = 0; i < iconCount; ++i )
	{
		if ( icons[i]->inRegion( x, y ) )
			return true;
	}

	return false;

}

void ForceGroupBar::render()
{	
	s_coverIcon->setColor( 0 );
		
	int maxUnits = 16;

	if ( MPlayer )
	{
		long playerCount;
		MPlayer->getPlayers( playerCount );
		if (playerCount)
			maxUnits = (MAX_MULTIPLAYER_MECHS_IN_LOGISTICS/playerCount) + 4;
		else
			maxUnits = 0;


		if ( maxUnits > 16 )
			maxUnits = 16;
	}


	for ( int i = 0; i < MAX_ICONS; i++ )
	{
		if (forceNumFlashes && icons[i] && icons[i]->unit->canJump())
		{
			if ( forceFlashTime > .25f )
			{
				if ( icons[i])
					icons[i]->render();
			}
		}
		else
		{
			if ( icons[i] )
				icons[i]->render();
		}

		if ( i >= maxUnits )
		{
			if ( s_coverIcon )
			{
				s_coverIcon->setLocation( ForceGroupIcon::selectionRect[i].left, ForceGroupIcon::selectionRect[i].top );
				s_coverIcon->setColor( 0xffffffff );
				s_coverIcon->render();
				s_coverIcon->setColor( 0 );
			}
		}
	}

	if (forceNumFlashes)
	{
		forceFlashTime += frameLength;
		if ( forceFlashTime > .5f )
		{
			forceFlashTime = 0.0f;
			forceNumFlashes--;
		}
	}
}

void ForceGroupBar::removeAll()
{
	for ( int i = 0; i < iconCount; i++ )
	{
		if ( icons[i] )
			delete icons[i];

		icons[i] = NULL;
	}

	iconCount = 0;
}

void ForceGroupBar::init( FitIniFile& file, StaticInfo* pCoverIcon )
{

	if ( NO_ERR != file.seekBlock( "Fonts" ) )
		Assert( 0, 0, "couldn't find the font block" );

	if ( !ForceGroupIcon::gosFontHandle )
		ForceGroupIcon::gosFontHandle = new aFont;

	long fontID;
	file.readIdLong( "IconFont", fontID );
	ForceGroupIcon::gosFontHandle->init( fontID );

	
	swapResolutions();

	for ( int i = 0; i < 16; i++ )
		ForceGroupIcon::init( file, i );

	s_coverIcon = pCoverIcon;
}

void ForceGroupBar::swapResolutions()
{
	ForceGroupIcon::resetResolution(0);
	
	for ( int i = 0; i < iconCount; i++ )
		icons[i]->swapResolutions(0);

	
}

bool ForceGroupBar::setPilotVideo( const char* pVideo, MechWarrior* pPilot )
{
	if ( !pVideo  )
	{
		if ( ForceGroupIcon::bMovie )
		{
			delete ForceGroupIcon::bMovie;
			ForceGroupIcon::bMovie = NULL;
			
		}
		else if ( ForceGroupIcon::pilotVideoTexture )
			gos_DestroyTexture( ForceGroupIcon::pilotVideoTexture );
		
		ForceGroupIcon::pilotVideoTexture = 0;
		ForceGroupIcon::pilotVideoPilot = 0;
	}

	else if  (ForceGroupIcon::bMovie || ControlGui::instance->isMoviePlaying()
		|| ForceGroupIcon::pilotVideoTexture || !prefs.pilotVideos)
	{
		// one already playing...
		// OR we don't want them playing.
		return 0;
	}

	else
	{
		for ( int i = 0; i < iconCount; i++ )
		{
			if ( icons[i] && icons[i]->unit->getPilot() == pPilot )
			{
				ForceGroupIcon::pilotVideoPilot = pPilot;
				FullPathFileName aviPath;
				aviPath.init( moviePath, pVideo, ".bik" );

				if ( (frameRate > 15.0) && fileExists(aviPath) && prefs.pilotVideos) // This is about correct.  Slower then this and movie has hard time keeping up!
				{
					//Update the RECT every frame.  What if we shift Icons around cause someone died!!
					RECT vRect;
					vRect.left 		= icons[i]->bmpLocation[icons[i]->locationIndex][1].x;
					vRect.right 	= icons[i]->pilotLocation[icons[i]->locationIndex][2];
					vRect.top 		= icons[i]->bmpLocation[icons[i]->locationIndex][3].y;
					vRect.bottom 	= icons[i]->bmpLocation[icons[i]->locationIndex][1].y;

					ForceGroupIcon::bMovie = new MC2Movie;
					ForceGroupIcon::bMovie->init(aviPath,vRect,true);
				}
				else // make a still texture
				{
					char realPilotName[9];

					//Set everything to zero so the strncpy below doesn't go off into LALA land!!
					memset(realPilotName,0,9);
					strncpy(realPilotName,pPilot->getName(),8);

					FullPathFileName path;
					path.init( moviePath, realPilotName, ".tga" );

					if (fileExists(path))
						ForceGroupIcon::pilotVideoTexture = gos_NewTextureFromFile( gos_Texture_Solid, path, 0 );
					else
					{
						char realMovieName[256];
						char realMoviePath[1024];
						_splitpath(path,NULL,realMoviePath,realMovieName,NULL);

						//Not in main installed directory and not in fastfile.  Look on CD.
						char actualPath[2048];
						strcpy(actualPath,CDInstallPath);
						strcat(actualPath,realMoviePath);
						strcat(actualPath,realMovieName);
						strcat(actualPath,".tga");

						bool fileThere = fileExists(actualPath);
						if (fileThere)
							ForceGroupIcon::pilotVideoTexture = gos_NewTextureFromFile( gos_Texture_Solid, actualPath, 0 );

						bool openFailed = false;
						while (!fileThere)
						{
							openFailed = true;
							EnterWindowMode();

							char data[2048];
							char msg[1024];
							char msg1[512];
							char title[256];
							cLoadString(IDS_MC2_movieMISSING,msg1,511);
							cLoadString(IDS_MC2_CDMISSING,msg,1023);
							cLoadString(IDS_MC2_MISSING_TITLE,title,255);
							sprintf(data,msg1,(const char*)path,msg);
							DWORD result = MessageBox(NULL,data,title,MB_OKCANCEL | MB_ICONWARNING);
							if (result == IDCANCEL)
							{
								ExitGameOS();
								return 1; 
							}

							fileThere = fileExists(actualPath);
							if (fileThere)
								ForceGroupIcon::pilotVideoTexture = gos_NewTextureFromFile( gos_Texture_Solid, actualPath, 0 );
						}

						if (openFailed && (Environment.fullScreen == 0) && prefs.fullScreen)
							EnterFullScreenMode();
					}
				}

				break;
			}
		}
	}

	return 1;
}

bool ForceGroupBar::isPlayingVideo()
{
	if ( ForceGroupIcon::bMovie )
		return true;

	return false;
}

//*************************************************************************************************
// end of file ( ForceGroupBar.cpp )
