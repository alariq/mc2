#ifndef MECHBAYSCREEN_H
#define MECHBAYSCREEN_H

/*************************************************************************************************\
MechBayScreen.h : Header file for mech selection
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"logisticsscreen.h"
#include"attributemeter.h"
#include"mechlistbox.h"
#include"simplecomponentlistbox.h"


class LogisticsMech;
class LogisticsMechIcon;
class SimpleCamera;

#define ICON_COUNT		12
#define ICON_COUNT_X	4
#define ICON_COUNT_Y	3


#define MB_MSG_NEXT 50
#define MB_MSG_PREV 51
#define MB_MSG_ADD 52
#define MB_MSG_REMOVE 53
#define MB_MSG_BUY 54
#define MB_MSG_CHANGE_LOADOUT 55
#define MB_MSG_BUY_SELL 56
#define MB_MSG_MAINMENU	57


class MechBayScreen : public LogisticsScreen
{

public:

	static MechBayScreen* instance(){ return s_instance; }
	MechBayScreen();
	virtual ~MechBayScreen();

	void init(FitIniFile* file);
	virtual void render(int xOffset, int yOffset);
	virtual void update();
	virtual void begin();
	virtual void end();
	
	virtual int			handleMessage( unsigned long, unsigned long );
	void setMech( LogisticsMech* pMech, bool bCommandFromLB = true );	
	void beginDrag( LogisticsMech* pMech );

	
private:

	LogisticsMech*		pCurMech;
	LogisticsMech*		pDragMech;
	LogisticsMechIcon*	pIcons;
	aObject				dragIcon;
	bool				dragLeft;
	long				forceGroupCount;

	AttributeMeter		attributeMeters[3];
	MechListBox			mechListBox;
	aListBox			componentListBox;
	aObject				dropWeightMeter;
	aAnimation			addWeightAnim;
	long				addWeightAmount;
	aAnimation			removeWeightAnim;
	long				removeWeightAmount;

	long				weightCenterX;
	long				weightCenterY;
	long				weightStartColor;
	long				weightEndColor;

	static	MechBayScreen*	s_instance;

	SimpleCamera*		mechCamera;
	ComponentListBox	loadoutListBox;

	void removeSelectedMech();
	void addSelectedMech();
	
	void drawWeightMeter(long xOffset, long yOffset);
	void reinitMechs();


	MechBayScreen( const MechBayScreen& );
	MechBayScreen& operator=( const MechBayScreen& );

	void				unselectDeploymentTeam();
	bool				selectFirstFGItem();
	bool				selectFirstViableLBMech();
	LogisticsMech*		getFGSelMech();




};

#endif