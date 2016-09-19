#ifndef MECHLISTBOX_H
#define MECHLISTBOX_H
/*************************************************************************************************\
MechListBox.h			: Interface for the MechListBox component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef ALISTBOX_H
#include"alistbox.h"
#endif

#ifndef AANIM_H
#include"aanim.h"
#endif

class FitIniFile;
class LogisticsMech;


//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MechListBox:
**************************************************************************************************/

#define ANIMATION_COUNT 4
class MechListBoxItem : public aListItem
{
public:
	
	virtual ~MechListBoxItem();
	MechListBoxItem( LogisticsMech* pMech, long count );

	static void init( FitIniFile& file );

	virtual void update();
	virtual void render();

	LogisticsMech* getMech( ){ return pMech; }
	void	resetMech( ) {pMech = NULL; }
	virtual bool		pointInside(long xPos, long yPos) const;

	bool		bOrange;
	static bool	bAddCalledThisFrame;



	
private:

	static MechListBoxItem* s_templateItem;
	
	aAnimGroup		animations[2][ANIMATION_COUNT];
	
	aObject		mechIcon;
	aText		chassisName;
	aText		variantName;
	aText		countText;
	aText		weightText;
	aObject		weightIcon;
	aText		costText;
	aObject		costIcon;
	aRect		outline;
	aRect		line;

	long		animationIDs[9];

	bool		drawCBills;
	bool		bIncludeForceGroup;

	LogisticsMech* pMech;

	long mechCount;

	MechListBoxItem& operator=( const MechListBoxItem& src );

	friend class MechListBox;

	void setMech();
	void doAdd();
	void startDrag();
	static void assignAnimation( FitIniFile& file, long& curAnimation );


	bool bDim;

	float animTime;


};


class MechListBox: public aListBox
{
public:

	MechListBox(bool deleteIfNoInventory, bool bIncludeForceGroup);

	
	void	setScrollBarOrange();
	void	setScrollBarGreen();

	virtual ~MechListBox();
	
	static int		init();
	void	drawCBills( bool bDraw );

	virtual void update();

	LogisticsMech* getCurrentMech();

	static void initIcon( LogisticsMech* pMec, aObject& icon );

	virtual long		AddItem(aListItem* itemString);

	void	dimItem( LogisticsMech* pMech, bool bDim );
	void	undimAll();
	void	disableItemsThatCostMoreThanRP();
	void	disableItemsThatCanNotGoInFG();

	void	setOrange( bool bOrange );


	

private:

	static bool	s_DrawCBills;
	
	MechListBox( const MechListBox& src );
	MechListBox& operator=( const MechListBox& src );

	friend class MechListBoxItem;

	bool		bDeleteIfNoInventory;
	bool		bIncludeForceGroup;

	bool		bOrange;


	

};


//*************************************************************************************************
#endif  // end of file ( MechListBox.h )
