#ifndef KEYBOARDREF_H
#define KEYBOARDREF_H
/*************************************************************************************************\
KeyboardRef.h			: Interface for the KeyboardRef component.
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef ALISTBOX_H
#include"alistbox.h"
#endif

#ifndef MISSIONGUI_H
#include"missiongui.h"
#endif

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
KeyboardRef:
**************************************************************************************************/
class KeyboardRef: public LogisticsScreen
{
	public:

		KeyboardRef();
		virtual ~KeyboardRef();

		KeyboardRef& operator=( const KeyboardRef& eyboardRef );
		int init();

		virtual void update();
		virtual void render();

		void reseed(MissionInterfaceManager::Command* );

		virtual int			handleMessage( unsigned long, unsigned long );


	private:

		KeyboardRef( const KeyboardRef& src );
		// HELPER FUNCTIONS

		aListBox		listBox;
		aTextListItem	listItemTemplate;
		aTextListItem	listItemTemplate2;

};


//*************************************************************************************************
#endif  // end of file ( KeyboardRef.h )
