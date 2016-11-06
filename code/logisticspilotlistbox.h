#ifndef LOGISTICSPILOTLISTBOX_H
#define LOGISTICSPILOTLISTBOX_H
/*************************************************************************************************\
LogisticsPilotListBox.h			: Interface for the LogisticsPilotListBox component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef ALISTBOX_H
#include"alistbox.h"
#endif

class LogisticsPilot;

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
LogisticsPilotListBox:
**************************************************************************************************/
#define PILOT_LIST_BOX_CHILD_COUNT 7


class LogisticsPilotListBoxItem : public aListItem
{
public:
	static int init( FitIniFile* file);
	LogisticsPilotListBoxItem( LogisticsPilot* pPilot );

	virtual ~LogisticsPilotListBoxItem();

	virtual void  render();
	virtual void update();

	LogisticsPilot* getPilot(){ return pPilot; }

private:

	aText nameText;
	aText rankText;
	aObject icon;
	aObject rankIcon;
	aRect	outline;
	aRect	line;
	aRect	pilotOutline;

	aAnimGroup	animations[3];
	long	pChildAnimations[PILOT_LIST_BOX_CHILD_COUNT];

	LogisticsPilot* pPilot;
	static LogisticsPilotListBoxItem* s_templateItem;

	static void setAnimation( FitIniFile& file, int whichOne );


	friend class LogisticsPilotListBox;
};


class LogisticsPilotListBox: public aListBox
{
	public:

	LogisticsPilotListBox();
	virtual ~LogisticsPilotListBox();

	static void  makeUVs( LogisticsPilot* pPilot, aObject& pObject );

	virtual long AddItem( aListItem* pItem );
	virtual void update();

	void removePilot(LogisticsPilot* pPilot);
	private:
	
		LogisticsPilotListBox( const LogisticsPilotListBox& src );
		
		// HELPER FUNCTIONS

};


//*************************************************************************************************
#endif  // end of file ( LogisticsPilotListBox.h )
