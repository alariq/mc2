/*************************************************************************************************\
gameTacMap.cpp			: Implementation of the gameTacMap component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"gametacmap.h"
#include"team.h"
#include"comndr.h"
#include"objmgr.h"
#include"cellip.h"
#include"gamecam.h"
#include"objective.h"
#include"mission.h"
#include"platform_windows.h"
#include "../resource.h"
extern unsigned char godMode;
extern bool useLeftRightMouseProfile;

#define SQUARE_BLIP 0
#define DOT_BLIP 1
#define TRIANGLE_BLIP 2

const float GameTacMap::s_blinkLength = .5f;
float		GameTacMap::s_lastBlinkTime = 0.f;

extern bool ShowMovers;

GameTacMap::GameTacMap()
{
	top = 0;
	left = 0;
	right = 0;
	bottom = 0;

	buildingPoints = NULL;
	buildingCount = 0;

	navMarkerCount = 0;
	curNavMarker = -1;

	objectiveAnimationId = -1;
	objectiveFlashTime = 0.0f;
	objectiveNumFlashes = 0;
}

void GameTacMap::init( unsigned char* bitmapData, int dataSize )
{

	EllipseElement::init();
	TGAFileHeader* pHeader = (TGAFileHeader*)bitmapData;

	bmpWidth = pHeader->width;
	bmpHeight = pHeader->height;

	textureHandle = gos_NewTextureFromMemory(gos_Texture_Solid,".tga",bitmapData,dataSize,0);
	char path[256];
	strcpy(  path, artPath );
	strcat( path, "viewingrect.tga" );
	viewRectHandle = mcTextureManager->loadTexture(path, gos_Texture_Alpha, 0 );

	strcpy( path, artPath );
	strcat( path, "blip.tga" );
	blipHandle = mcTextureManager->loadTexture( path, gos_Texture_Alpha, 0 );
}

void GameTacMap::update()
{
	Stuff::Vector2DOf<long> screen;
	screen.x = userInput->getMouseX();
	screen.y = userInput->getMouseY();

	float width = right - left;
	float height = bottom - top;


	if ( !inRegion(screen.x, screen.y) )
		return;

	ControlGui::instance->setRolloverHelpText( IDS_TACMAP_HELP );
	
	if ( userInput->isLeftClick() )	
	{
		screen.x -= left;
		screen.y -= top;
		
		Stuff::Vector3D world;

		tacMapToWorld( screen, width, height, world );
		if (MissionInterfaceManager::instance()->getControlGui()->isAddingAirstrike() &&  
			MissionInterfaceManager::instance()->getControlGui()->isButtonPressed( ControlGui::SENSOR_PROBE ))
			MissionInterfaceManager::instance()->doMove(world);
		else
		{
			eye->setPosition( world, false );
			((GameCamera*)(eye))->setTarget( 0 );
		}
	}
	else if ( userInput->isRightClick() && useLeftRightMouseProfile )
	{
		screen.x -= left;
		screen.y -= top;
		
		Stuff::Vector3D world;

		tacMapToWorld( screen, width, height, world );

		MissionInterfaceManager::instance()->doMove(world);
	}

}

bool GameTacMap::animate (long objectiveId, long nFlashes)
{
	if (objectiveAnimationId == -1)
	{
		objectiveAnimationId = objectiveId - 1;
		objectiveFlashTime = 0.0f;
		objectiveNumFlashes = nFlashes;

		return true;
	}

	return false;
}

void GameTacMap::render()
{
	if (turn < 2)		//Terrain not setup yet.  Left,Right,Top,Bottom are poopy!
		return;

	gos_VERTEX corners[5];

	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero );

	gos_SetRenderState( gos_State_Specular,FALSE );
	gos_SetRenderState( gos_State_AlphaTest, FALSE );
			
	gos_SetRenderState( gos_State_Filter, gos_FilterNone );
	gos_SetRenderState( gos_State_ZWrite, 0 );
	gos_SetRenderState( gos_State_ZCompare, 0 );
	gos_SetRenderState( gos_State_Texture, textureHandle );


	for ( int i = 0; i < 4; ++i )
	{
		corners[i].rhw = 1.0f;
		corners[i].argb = 0xffffffff;
		corners[i].frgb = 0;
		corners[i].z = 0.0f;
	}


	corners[0].x = corners[2].x = left;
	corners[1].x = corners[3].x = right;
	corners[3].y = corners[2].y = bottom;
	corners[0].y = corners[1].y = top;

	corners[0].u = 2.f/(float)bmpWidth;
	corners[3].u = 128.f/(float)bmpWidth;
	corners[2].u = 2.f/(float)bmpWidth;
	corners[1].u = 128.f/(float)bmpWidth;
	corners[0].v = 2.f/(float)bmpWidth;
	corners[3].v = 128.f/(float)bmpHeight;
	corners[2].v = 128.f/(float)bmpHeight;
	corners[1].v = 2.f/(float)bmpWidth;
	
	gos_DrawTriangles( corners, 3 );
	gos_DrawTriangles( &corners[1], 3 );


	Stuff::Vector2DOf<long> screen;
	Stuff::Vector4D 		nScreen;
	Stuff::Vector3D			world;

	//-----------------------------------------------------------
	// Render the objective markers
	long count = 0;
	for ( EList< CObjective*, CObjective* >::EIterator iter =  Team::home->objectives.Begin();
		!iter.IsDone(); iter++ )
	{
		//We are there.  Start flashing.
		if ((objectiveAnimationId == count) && objectiveNumFlashes)
		{
			objectiveFlashTime += frameLength;
			if ( objectiveFlashTime > .5f )
			{
				objectiveFlashTime = 0.0f;
				objectiveNumFlashes--;
			}
			else if ( objectiveFlashTime > 0.25f)
			{
				(*iter)->RenderMarkers(this, 0);
			}

			if (objectiveNumFlashes == 0)
			{
				//Flashing is done.  We now return you to your regularly scheduled program.
				objectiveAnimationId = 0;
			}
		}
		else
		{
			(*iter)->RenderMarkers(this, 0);
		}

		count++;
	}

	
	// this is the little viewing rect

	// Routine that InverseProjects is slightly less accurate but an order of magnitude faster.
	// Can make more accurate later at very little expense to performance.
	// Easy to fix with camera later.  -fs

	screen.x = 1;
	screen.y = 1;
	
	nScreen.x = nScreen.y = 1.0f;
	nScreen.z = nScreen.w = 0.0f;
	
	eye->inverseProjectZ( nScreen, world );
	worldToTacMap( world, corners[0] );

	screen.y = Environment.screenHeight - 1;
	nScreen.y = (Environment.screenHeight * 0.6667f) - 1;
	nScreen.z = nScreen.w = 0.0f;
	eye->inverseProjectZ( nScreen, world );
	worldToTacMap( world, corners[1] );

	screen.x = Environment.screenWidth - 1;
	nScreen.x = Environment.screenWidth - 1;
	nScreen.z = nScreen.w = 0.0f;
	eye->inverseProjectZ( nScreen, world );
	worldToTacMap( world, corners[2] );

	screen.y = 1;
	nScreen.y = 1;
	nScreen.z = nScreen.w = 0.0f;
	eye->inverseProjectZ( nScreen, world );
	worldToTacMap( world, corners[3] );

	corners[0].argb = 0xffffffff;
	corners[1].argb = 0xffffffff;
	corners[2].argb = 0xffffffff;
	corners[3].argb = 0xffffffff;
	corners[0].u = corners[1].u = 0.078125f;
	corners[3].u = corners[2].u = .99875f;
	corners[0].v = corners[3].v = 0.078125f;
	corners[1].v = corners[2].v = .99875f;
	corners[4] = corners[0];

	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
	gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
	gos_SetRenderState( gos_State_MonoEnable, 0);
	gos_SetRenderState( gos_State_Perspective, 0);
	gos_SetRenderState( gos_State_Clipping, 2);
	gos_SetRenderState( gos_State_AlphaTest, 0);
	gos_SetRenderState( gos_State_Specular, 0);
	gos_SetRenderState( gos_State_Dither, 1);
	gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulate);
	gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear);
	gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
	gos_SetRenderState( gos_State_ZCompare, 0);
	gos_SetRenderState(	gos_State_ZWrite, 0);
	DWORD gosTextureHandle = mcTextureManager->get_gosTextureHandle(viewRectHandle);

	gos_SetRenderState( gos_State_Texture, gosTextureHandle );

	gos_DrawQuads( &corners[0], 4 );

	unsigned long colors[MAX_MOVERS];
	unsigned long ringColors[MAX_MOVERS];
	Stuff::Vector3D positions[MAX_MOVERS];
	unsigned long ranges[MAX_MOVERS];
	bool		  selected[MAX_MOVERS];

	count = 0;

	//------------------------------------------------------------
	// draw non-movers, must do separate check for vehicles, I'm not sure they
	// have sensors
	for (int i = 0; i < MAX_TEAMS; ++i )
	{
		TeamSensorSystem* pSys = SensorManager->getTeamSensor( i );
		
		if ( pSys )
		{
			for ( int j = 0; j < pSys->numSensors; ++j )
			{
				SensorSystem* pSensor = pSys->sensors[j];

				if ( !pSensor )
					continue;

				if ( pSensor->owner->isDestroyed() || pSensor->owner->isDisabled() || pSensor->owner->getStatus() == OBJECT_STATUS_SHUTDOWN )
					continue;

				if ( pSensor->getRange() < 1.1 || pSensor->broken)
					continue;

				if (!pSensor->owner->getTeam())
					continue;

				ObjectClass objClass = pSensor->owner->getObjectClass();

				unsigned long colorBlip = pSensor->owner->getSelected() ? 0xff4bff4b : 0xff00cc00;
				unsigned long colorRing = 0xff00cc00;

			
				if ( pSensor->owner->getTeam()->isNeutral( Team::home ) )
				{
					colorBlip = pSensor->owner->getSelected() ? 0xff4c4cff : 0xff0000ff;
					colorRing = 0xff0000ff;
				}
				else if ( pSensor->owner->getTeam()->isEnemy( Team::home ) ) // enemy
				{
					
					{
						colorBlip = pSensor->owner->getSelected() ? 0xffff3f3f : 0xffff0000;
						colorRing = 0xffff0000;
					}
				}

				if ( objClass != BATTLEMECH && objClass != GROUNDVEHICLE)
				{
					if ( objClass == ARTILLERY )
					{
						// blink
						s_lastBlinkTime += frameLength;
						if ( s_lastBlinkTime > s_blinkLength )
						{
							colorBlip = 0;
							colorRing = 0;
							s_lastBlinkTime = 0.f;
						}

					}

					colors[count] = colorBlip;
					ringColors[count] = colorRing;
					ranges[count] = pSensor->getRange();
					selected[count] = 0;
					positions[count] = pSensor->owner->getPosition();
					count++;

				}
			}
		}
	}

	unsigned long colorBlip, colorRing;

	//-----------------------------------------------------	
	// draw the movers
	for (int i=0;i<(ObjectManager->numMovers);i++)
	{
		MoverPtr mover = ObjectManager->getMover(i);
		if (mover && mover->getExists() && !(mover->isDestroyed() || mover->isDisabled()))
		{
			SensorSystem* pSensor = mover->getSensorSystem();
			float range = pSensor ? pSensor->getRange() : 0;
			long contactStatus = mover->getContactStatus(Team::home->getId(), true);
			if (mover->getTeamId() == Team::home->id)
			{
				if (mover->getCommanderId() == Commander::home->getId())
				{
					if (mover->isOnGUI())
					{
						colorBlip = mover->getSelected() ? 0xff4bff4b : 0xff00cc00;
						mover->getStatus() == OBJECT_STATUS_SHUTDOWN ? colorRing = 0 : colorRing =  0xff00cc00;
					}
					else
						continue;
				}
				else
				{
					if (mover->isOnGUI() && land->IsGameSelectTerrainPosition(mover->getPosition()) && mover->pathLocks)
					{
						colorBlip = mover->getSelected() ? 0xff4b4bff : 0xff0000cc;
						mover->getStatus() == OBJECT_STATUS_SHUTDOWN ? colorRing = 0 : colorRing =  0xff0000cc;
					}
					else
						continue;
				}
			}
			else if (ShowMovers || (MPlayer && MPlayer->allUnitsDestroyed[MPlayer->commanderID]) || ((mover->getTeamId() != Team::home->id)
				&&  ( contactStatus != CONTACT_NONE )
				&&  (mover->getStatus() != OBJECT_STATUS_SHUTDOWN) 
				&&  (!mover->hasNullSignature())
				&&	(mover->getEcmRange() <= 0.0f) ) )	//Do not draw ECM mechs!!)
			{
				//Not on our side.  Draw by contact status
				colorBlip = mover->getSelected() ? 0xffff3f3f : 0xffff0000;
				colorRing = 0xffff0000;
				
			}
			else
				continue;

			colors[count] = colorBlip;
			ringColors[count] = colorRing;
			ranges[count] = range;
			selected[count] = mover->getSelected();
			positions[count] = mover->getPosition();
			count++;

		}
	}

	for (int i = 0; i < count; i++ )
	{
		drawSensor( positions[i], ranges[i], ringColors[i] );
	}
	bool bSel = 0; // draw unselected first
	for ( int j = 0; j < 2; j++ )
	{
		for ( int i = 0; i < count; i++ )
		{
			if ( selected[i] == bSel )
			{
				drawBlip( positions[i], colors[i], DOT_BLIP );
			}
		}
		bSel = 1;
	}



}

void GameTacMap::worldToTacMap( Stuff::Vector3D& world, gos_VERTEX& tac )
{
	TacMap::worldToTacMap( world, left, top, right - left, bottom - top, tac );
}	
void GameTacMap::initBuildings( unsigned char* data, int size )
{
	if ( data )
	{
		int32_t* pData = (int32_t*)data;
		buildingCount = *pData++;

		if (buildingCount)
		{
			buildingPoints = new gos_VERTEX[buildingCount];
			gos_VERTEX* pTmp = buildingPoints;

			for ( int i = 0; i < buildingCount; ++i, pTmp++)
			{
				pTmp->x = *pData++ + left;
				pTmp->y = *pData++ + top;
				pTmp->z = 0;
				pTmp->argb = BUILDING_COLOR;
				pTmp->frgb = 0;
				pTmp->rhw = .5;
				pTmp->u = 0.f;
				pTmp->v = 0.f;
			}
		}
	}
}

void GameTacMap::drawSensor( const Stuff::Vector3D& pos, float radius, long color )
{
	if ( color == 0 )
		return;
	gos_VERTEX sqare[4];

	if ( radius > 1 )
		radius += land->metersPerCell * 6.f; // a little fudge

	radius *= ((float)(right - left))/(land->metersPerCell * land->realVerticesMapSide * 3.f);

	for ( int i = 0; i < 4; ++i )
	{
		sqare[i].z = 0;
		sqare[i].rhw = .5;
		sqare[i].argb = color;
		sqare[i].frgb = 0;
	}

	worldToTacMap( (Stuff::Vector3D&)pos, sqare[0] );

	sqare[1].x = sqare[0].x;
	sqare[1].y = sqare[0].y + 1;

	sqare[2].x = sqare[0].x + 1;
	sqare[2].y = sqare[0].y;

	sqare[3].x = sqare[0].x + 1;
	sqare[3].y = sqare[0].y + 1;

	// need to draw that round thing
	Stuff::Vector2DOf< long > center;
	Stuff::Vector2DOf< long > radii;
	center.x = float2long(sqare[1].x);
	center.y = float2long(sqare[1].y);
	radii.x = 2.0f * float2long(radius+.5);
	radii.y = 2.0f * float2long(radius+.5);


	EllipseElement circle( center, radii, color, 0 ); 
	GUI_RECT rect = { left, top, right, bottom };
	circle.setClip( rect );

	circle.draw();


}

void GameTacMap::drawBlip( const Stuff::Vector3D& pos, long color, int type )
{
	if ( color == 0 )
		return;
	gos_VERTEX triangle[4];

	gos_SetRenderState( gos_State_AlphaTest, 1);
	gos_SetRenderState( gos_State_Specular, 0);
	gos_SetRenderState( gos_State_Dither, 1);
	gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear);
	gos_SetRenderState( gos_State_ZCompare, 0);
	gos_SetRenderState(	gos_State_ZWrite, 0);
	unsigned long gosID = mcTextureManager->get_gosTextureHandle( blipHandle );
	gos_SetRenderState( gos_State_Texture, gosID );

	for ( int i = 0; i < 4; ++i )
	{
		triangle[i].z = 0;
		triangle[i].rhw = .5;
		triangle[i].argb = color;
		triangle[i].frgb = 0;
		triangle[i].u = 0.f;
		triangle[i].v = 0.f;
	}

	

	worldToTacMap( (Stuff::Vector3D&)pos, triangle[0] );
	triangle[0].x -= 2.f;
	triangle[1].x = triangle[0].x;
	triangle[2].x = triangle[3].x = triangle[0].x + 4.1f;
	triangle[0].y -= 2.f;
	triangle[3].y = triangle[0].y;
	triangle[1].y = triangle[2].y = triangle[0].y + 4.1f;
	triangle[2].u = triangle[3].u = .250001f;
	triangle[1].v = triangle[2].v = .250001f;

	gos_DrawQuads( triangle, 4 );
}

void GameTacMap::setPos( const GUI_RECT& newPos )
{
	left = newPos.left;
	right = newPos.right;
	top = newPos.top;
	bottom = newPos.bottom;
}



//*************************************************************************************************
// end of file ( TacMap.cpp )

//*************************************************************************************************
// end of file ( gameTacMap.cpp )
