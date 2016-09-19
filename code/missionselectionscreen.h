#ifndef MISSIONSELECTION_H
#define MISSIONSELECTION_H

/*************************************************************************************************\
MissionSelectionScreen.h : Header file for mission selection
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef ALISTBOX_H
#include"alistbox.h"
#endif

#ifndef MC2movie_H
#include"mc2movie.h"
#endif

class FitIniFile;

#define MAX_MISSIONS_IN_GROUP 6

class MissionSelectionScreen : public LogisticsScreen
{

public:

	MissionSelectionScreen();
	virtual ~MissionSelectionScreen();
 	virtual void render( int xOffset, int yOffset );
	virtual void begin();
	virtual void end();
	virtual void update();
	void	init( FitIniFile* file );
	virtual int			handleMessage( unsigned long, unsigned long );


private:

	LogisticsScreen	operationScreen;

	MC2MoviePtr		bMovie;
	bool				playedLogisticsTune;

	//HGOSVIDEO			video;
	//unsigned long		videoTexture;

	const char*			missionNames[MAX_MISSIONS_IN_GROUP];
	long				missionCount;

	void				setMission( int whichOne );

	aListBox			missionDescriptionListBox;

	void				updateListBox();

	long				pressedButton;

	bool				bStop;




};

#endif