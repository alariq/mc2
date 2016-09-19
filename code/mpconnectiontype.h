#ifndef MPCONNECTIONTYPE_H
#define MPCONNECTIONTYPE_H
/*************************************************************************************************\
MPConnectionType.h			: Interface for the MPConnectionType component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#include"asystem.h"
#include"abutton.h"
#include"alistbox.h"

#ifndef AANIM_H
#include"aanim.h"
#endif

#ifndef MPHOSTGAME_H
#include"mphostgame.h"
#endif

#include"assert.h"

static const int ZONE_PANEL_FIRST_BUTTON_ID = 1000100;

class aZonePanel : public aObject
{
public:
	void init(FitIniFile* pFile, LogisticsScreen* pParent);
	virtual int			handleMessage( unsigned long, unsigned long );
	virtual void		update();
	virtual void		render();

private:
	LogisticsScreen *pParentScreen;

	aAnimButton button;
	aText text;

	bool	bShowWarning;
};


static const int LAN_PANEL_FIRST_BUTTON_ID = 1000200;

class aLanPanel : public aObject
{
public:
	aLanPanel(LogisticsScreen &refParentScreenParam)
	{
		pParentScreen = &refParentScreenParam;
	}
	void init(FitIniFile* pFile);
	virtual int			handleMessage( unsigned long, unsigned long );
	virtual void update();
private:
	LogisticsScreen *pParentScreen;
	aAnimButton button0;
	aAnimButton button1;
	aText text;
};


static const int TCPIP_PANEL_FIRST_BUTTON_ID = 1000300;

class aTcpipPanel : public aObject
{
public:
	aTcpipPanel(LogisticsScreen &refParentScreenParam)
	{
		pParentScreen = &refParentScreenParam;
		connectingTime = 0.f;
	}
	void init(FitIniFile* pFile);
	virtual void destroy();
	virtual int			handleMessage( unsigned long, unsigned long );
	virtual void		update();
	virtual void		render();
	virtual void		begin();
private:

	long		getNum( char* pStr, long index1, long index2 );
	LogisticsScreen *pParentScreen;
	aAnimButton button0;
	aAnimButton button1;
	aText text0;
	aText text1;
	aRect helpRect;
	aComboBox comboBox;
	bool	bConnectingDlg;
	float	connectingTime;
	bool	bErrorDlg;
	bool			bExpanded;
	bool	bFoundConnection;

};

class MPConnectionType : public LogisticsScreen
{
public:
	
	MPConnectionType();
	virtual ~MPConnectionType();
	
	void init(FitIniFile* file);
	bool isDone();
	virtual void		begin();
	virtual void		end();
 	virtual void render( int xOffset, int yOffset );
	virtual void render();
	virtual void update();
	virtual int			handleMessage( unsigned long, unsigned long );

	bool				bDone;

	void **ppConnectionScreen;
	void *pLocalBrowserScreen;
	void *pDirectTcpipScreen;
	void *pMPPlaceHolderScreen;
	void *pMPHostGame;

private:
	int indexOfButtonWithID(int id);

	aZonePanel zonePanel;
	aLanPanel lanPanel;
	aTcpipPanel tcpipPanel;
	aObject *pPanel;

	MPHostGame	hostDlg;

	bool		bHosting;
};

class MPPlaceHolderScreen : public LogisticsScreen
{
public:
	
	//MPPlaceHolderScreen();
	//virtual ~MPPlaceHolderScreen();
	
	virtual void		begin() { status = RUNNING; }
	//virtual void		end();
 	virtual void render( int xOffset, int yOffset ) {
		static int lastXOffset = 0;
		static int lastYOffset = 0;
		if ((0 == xOffset) && (0 == yOffset)) {
			if (xOffset < lastXOffset) {
				status = NEXT;
			} else if (xOffset > lastXOffset) {
				status = PREVIOUS;
			} else if (yOffset > lastYOffset) {
				status = UP;
			} else if (yOffset < lastYOffset) {
				status = DOWN;
			} else {
				assert(false);
				status = NEXT;
			}
		}
		lastXOffset = xOffset;
		lastYOffset = yOffset;
	}
	virtual void render() { render(0, 0); }
	//virtual void update();

private:
};



//*************************************************************************************************
#endif  // end of file ( MPConnectionType.h )
