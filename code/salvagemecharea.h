#ifndef SALVAGEMECHAREA_H
#define SALVAGEMECHAREA_H
/*************************************************************************************************\
SalvageMechArea.h			: Interface for the SalvageMechArea component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************
#include"asystem.h"
#include"simplecomponentlistbox.h"
#include"attributemeter.h"
#include"logisticsscreen.h"
#include"simplecamera.h"

class aButton;
class MechIcon;
class BattleMech;
class LogisticsVariant;

/**************************************************************************************************
CLASS DESCRIPTION
SalvageMechArea:
**************************************************************************************************/
class SalvageMechArea : public LogisticsScreen
{
	public:

	static SalvageMechArea* instance;
	SalvageMechArea();
	virtual ~SalvageMechArea();

	void init( FitIniFile* file );
	void setMech( LogisticsVariant* pMech, long red, long green, long blue );

	virtual void render(long xOffset, long yOffset);
	virtual void update();

	
	protected:

	LogisticsVariant*		unit;

	AttributeMeter		attributeMeters[3];
	ComponentListBox	loadoutListBox;
	SimpleCamera		mechCamera;

};


class SalvageMechScreen : public LogisticsScreen
{
public:
	
	SalvageMechScreen();
	virtual ~SalvageMechScreen();
	
	void init(FitIniFile* file);
	bool isDone();
	bool donePressed() { return bDone; }
	virtual void render();
	virtual void update();
	virtual int			handleMessage( unsigned long, unsigned long );

	void updateSalvage(); // put into inventory

	bool				bDone;


private:

	aListBox			salvageListBox;

	SalvageMechArea		selMechArea;
	aAnimation			exitAnim;
	aAnimation			entryAnim;

	float				countDownTime;
	float				curCount;
	float				previousAmount;
	float				oldCBillsAmount;
};

class SalvageListItem : public aListItem
{
public:
	virtual void		render();
	virtual void		update();

	virtual ~SalvageListItem();

	static void			init( FitIniFile* file );
	virtual int			handleMessage( unsigned long message, unsigned long who );

	bool				isChecked();
	LogisticsVariant*	getMech(){ return pVariant; }

	SalvageListItem( BattleMech* pUnit );

private:
	static aAnimButton*	templateCheckButton;
	static aText*		mechNameText;
	static aText*		variantNameText;
	static aText*		weightText;
	static aText*		costText;
	static aObject*		cBillsIcon;
	static aObject*		weightIcon;
	static GUI_RECT		iconRect;
	static GUI_RECT		rect;
	static aAnimation*	s_pressedAnim;
	static aAnimation*	s_highlightAnim;
	static aAnimation*	s_normalAnim;

	aAnimation			pressedAnim;
	aAnimation			highlightAnim;
	aAnimation			normalAnim;

	int					salvageAmount;
	float				costToSalvage;
	MechIcon*			icon;
	LogisticsVariant*	pVariant;
	aAnimButton*		checkButton;

	DWORD               psRed;
	DWORD               psGreen;
	DWORD               psBlue;

	friend class SalvageMechScreen;


	
};




//*************************************************************************************************
#endif  // end of file ( SalvageMechArea.h )
