#define TACMAP_CPP
/*************************************************************************************************\
TacMap.cpp			: Implementation of the TacMap component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"tacmap.h"
#include"tgainfo.h"
#include"gameos.hpp"

extern unsigned char godMode;

TacMap::TacMap()
{
}


void TacMap::worldToTacMap( Stuff::Vector3D& world, int xOffset, int yOffset, int xSize, int ySize, gos_VERTEX& tac )
{
	long tacX;
	long tacY;

	land->worldToCell( world, tacY, tacX );	
	if ( tacX < 0 )
		tacX = 0;
	if ( tacY < 0 )
		tacY = 0;
	if ( tacX > land->realVerticesMapSide * MAPCELL_DIM )
		tacX = land->realVerticesMapSide * MAPCELL_DIM;
	if ( tacY > land->realVerticesMapSide * MAPCELL_DIM )
		tacY = land->realVerticesMapSide * MAPCELL_DIM;
	
	tac.x = (float)tacX * (float)xSize /((float)land->realVerticesMapSide * MAPCELL_DIM);
	tac.y = (float)tacY * (float)ySize/((float)land->realVerticesMapSide * MAPCELL_DIM);

	tac.x += xOffset;
	tac.y += yOffset;

}

void TacMap::tacMapToWorld( const Stuff::Vector2DOf<long>& screen, int xSize, int ySize,  Stuff::Vector3D& world )
{
	// turn screen into cells
	long cellX = screen.x /(float)xSize * ((float)land->realVerticesMapSide * MAPCELL_DIM);
	long cellY = screen.y /(float)ySize * ((float)land->realVerticesMapSide * MAPCELL_DIM);

	if ((cellX < 0) || (cellX >= land->realVerticesMapSide * MAPCELL_DIM) ||
		(cellY < 0) || (cellY >= land->realVerticesMapSide * MAPCELL_DIM))
	{
		world = eye->getPosition();
	}
	else
	{
		world.x = land->cellColToWorldCoord[cellX];
		world.y = land->cellRowToWorldCoord[cellY];

		world.z = land->getTerrainElevation( world );
	}
}

