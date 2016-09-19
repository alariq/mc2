#ifndef MISSIONBRIEFINGSCREEN_H
#define MISSIONBRIEFINGSCREEN_H
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef ALISTBOX_H
#include"alistbox.h"
#endif

#ifndef ABUTTON_H
#include"abutton.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef SIMPLECAMERA_H
#include"simplecamera.h"
#endif

#define MN_MSG_PLAY 80
#define MN_MSG_STOP 82
#define MN_MSG_PAUSE 81


//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MissionBriefingScreen:
**************************************************************************************************/
class MissionBriefingScreen: public LogisticsScreen
{
	public:

	MissionBriefingScreen();
	virtual ~MissionBriefingScreen();

	virtual void render( int xOffset, int yOffset );
	virtual void begin();
	virtual void end();
	virtual void update();
	void	init( FitIniFile* file );
	virtual int			handleMessage( unsigned long, unsigned long );


	static long	getMissionTGA( const char* missionName );



	private:

	aObject*		objectiveButtons[MAX_OBJECTIVES];
	aObject			dropZoneButton;
	EString			objectiveModels[MAX_OBJECTIVES];
	long			modelTypes[MAX_OBJECTIVES];
	float			modelScales[MAX_OBJECTIVES];
	long			modelColors[MAX_OBJECTIVES][3];
	aListBox		missionListBox; 

	int			addLBItem( const char* itemName, unsigned long color, int ID);
	int			addItem( int ID, unsigned long color, int LBid );
	void		addObjectiveButton( float fMakerX, float fMarkerY, int count, int priority,
											   float mapWidth, float mapHeight, bool display );
	void		setupDropZone( float fX, float fY, float mapWidth, float mapHeight );


	float		runTime;
	bool		bClicked;


	SimpleCamera	camera;





};


//*************************************************************************************************
#endif  // end of file ( MissionBriefingScreen.h )
