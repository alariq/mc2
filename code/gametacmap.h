#ifndef GAMETACMAP_H
#define GAMETACMAP_H
/*************************************************************************************************\
gameTacMap.h			: Interface for the gameTacMap component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef TACMAP_H
#include"tacmap.h"
#endif

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
gameTacMap:
**************************************************************************************************/
class GameTacMap: public TacMap
{
	public:

	GameTacMap();
	~GameTacMap()
	{
		if (buildingPoints)
		{
			delete [] buildingPoints;
			buildingPoints = NULL;
		}
	}

	void init( unsigned char* bitmapData, int dataSize );
	void update(); // do not call these two functions from editor
	void render();
	
	bool animate (long objectiveId, long nFlashes);

	// if the world coords do not lie on the map, they will be changed.
	void worldToTacMap( Stuff::Vector3D& world, gos_VERTEX& tac );	


	bool inRegion( int x, int y ){ return x > left && x < right && y > top && y < bottom; }
	
	void initBuildings( unsigned char* data, int size );
	void setPos( const GUI_RECT& newPos );
	
	protected:

	int		top;	// position
	int		left;
	int		right;
	int		bottom;
	int		bmpWidth;
	int		bmpHeight;
	int		buildingCount;
	gos_VERTEX* buildingPoints;

	unsigned long textureHandle;
	unsigned long viewRectHandle;
	unsigned long blipHandle;

	Stuff::Vector3D navMarkers[6];
	unsigned long navMarkerCount;
	unsigned long curNavMarker;

	void drawSensor( const Stuff::Vector3D& pos, float radius, long color);
	void drawBlip( const Stuff::Vector3D& pos, long color, int shape  );

	const static float s_blinkLength;
	static float		s_lastBlinkTime;

	//Tutorial
	long objectiveAnimationId;
	long objectiveNumFlashes;
	float objectiveFlashTime;

};


//*************************************************************************************************
#endif  // end of file ( gameTacMap.h )
