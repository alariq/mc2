#ifndef MPGAMEBROWSER_H
#define MPGAMEBROWSER_H
/*************************************************************************************************\
MPGameBrowser.h			: Interface for the MPGameBrowser component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#include"asystem.h"
#include"alistbox.h"
#include"attributemeter.h"
#include"simplecamera.h"

#include"multplyr.h"

#ifndef AANIM_H
#include"aanim.h"
#endif

#ifndef MPHOSTGAME_H
#include"mphostgame.h"
#endif

class aButton;
//struct _MC2Session;


class aStyle3TextListItem : public aTextListItem
{
public:
	aStyle3TextListItem() { hasAnimation = false; normalColor = 0xff808080; }
	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void		render();

protected:
	bool hasAnimation;
	aAnimGroup animGroup;
	int normalColor;
};

class aGameListItem : public aListItem
{
public:
	aGameListItem();
	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void update();
	void setSessionInfo( _MC2Session* pSessions );

	const char*		getSessionName();

	const char*		getText( int which );

	const MC2Session* getSession (void) { return(&session); }


	aGameListItem& operator=( const aGameListItem& );

protected:
	MC2Session session;
	aObject allTechGraphic;
	aStyle3TextListItem gameName;
	aStyle3TextListItem numPlayers;
	aStyle3TextListItem mapName;
	aTextListItem latency;
	aRect allTechRect;
	aRect gameNameRect;
	aRect numPlayersRect;
	aRect mapNameRect;
	aRect latencyRect;
	aObject pingIcon;
};

class MPGameBrowser : public LogisticsScreen
{
public:
	
	MPGameBrowser();
	virtual ~MPGameBrowser();
	
	void init(FitIniFile* file);
	bool isDone();
	virtual void		begin();
	virtual void		end();
 	virtual void render( int xOffset, int yOffset );
	virtual void render();
	virtual void update();
	virtual int			handleMessage( unsigned long, unsigned long );


private:
	int indexOfButtonWithID(int id);

	aListBox				gameList;
	aGameListItem			items[256];
	aGameListItem			templateItem;

	MPHostGame				hostDlg;

	bool					bHosting;
	bool					bShowErrorDlg;

	int						sortOrder;

	int						bSortUpward;
	long					oldScrollPos;
};



//*************************************************************************************************
#endif  // end of file ( MPGameBrowser.h )
