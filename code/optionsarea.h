#ifndef OPTIONSAREA_H
#define OPTIONSAREA_H
/*************************************************************************************************\
OptionsArea.h			: Interface for the OptionsArea component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************
#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef SIMPLECAMERA_H
#include"simplecamera.h"
#endif

#include"alistbox.h"
#include"attributemeter.h"

class aButton;

class CPrefs;


class OptionsXScreen : public LogisticsScreen
{
public:
	
	OptionsXScreen();
	virtual ~OptionsXScreen();
	
	void init(FitIniFile* file);
	bool isDone();
	virtual void render();
	virtual void update();
	virtual int			handleMessage( unsigned long, unsigned long );

	void updateOptions(); // put into inventory

	bool				bDone;


private:

	int indexOfButtonWithID(int id);

	LogisticsScreen*		tabAreas[4];
	int						curTab;
	bool			bShowWarning;


};

class ScrollX : public aObject
{
public:

	ScrollX();
	long	init(aButton* pLeft, aButton* pRight, aButton* pTab );
	virtual void	update();

	virtual int		handleMessage( unsigned long message, unsigned long fromWho );

	void			SetScrollMax(float newMax);
	void			SetScrollPos(float newPos);
	float			GetScrollMax(void){return scrollMax;};
	float			GetScrollPos(void){return scrollPos;};
    // sebi unused
	//long			SetSrollInc( long newInc ){ scrollInc = (float)newInc; } // amount you move for one arrow click
	//long			SetScrollPage(long newInc){ pageInc = (float)newInc;} // amount you move if you click on the bar itself
	void			ScrollUp(void);
	void			ScrollPageUp(void);
	void			ScrollDown(void);
	void			ScrollPageDown(void);
	void			SetScroll( long newScrollPos );	
	void			Enable( bool enable );

private:

	void ResizeAreas();

	 
	aButton*		buttons[3];
	float			scrollMax;
	float			scrollPos;
	float			scrollInc;
	float			pageInc;

	long			lastX;

};

class OptionsGraphics : public LogisticsScreen
{

public:
	void init(long xOffset, long yOffset);
	virtual void render();
	virtual void update();
	virtual void begin();
	virtual void end();
	void reset( const CPrefs& newPrefs);

	virtual int		handleMessage( unsigned long message, unsigned long fromWho );

private:
	aDropList		resolutionList;
	aDropList		cardList;
	bool			bExpanded;

};

class OptionsAudio : public LogisticsScreen
{
	public:
		void init(long xOffset, long yOffset);
		virtual void render();
		virtual void update();
		virtual void begin();
		virtual void end();
		void reset(const CPrefs& newPrefs);

		virtual int		handleMessage( unsigned long message, unsigned long fromWho );

	private:

		ScrollX		scrollBars[5];

};

class OptionsHotKeys : public LogisticsScreen
{

public:
		void init(long xOffset, long yOffset);
		virtual void render();
		virtual void update();
		virtual void begin();
		virtual void end();
		void reset(bool bUseOld);

		virtual int		handleMessage( unsigned long message, unsigned long fromWho );

private:

	static void makeKeyString( long hotKey, char* buffer );
	static int makeInputKeyString( long& hotKey, char* buffer );


	aListBox		hotKeyList;
	bool			bShowDlg;
	long			curHotKey;
};

class OptionsGamePlay : public LogisticsScreen
{
public:
	void init(long xOffset, long yOffset);
	virtual void render();
	virtual void update();
	virtual void begin();
	virtual void end();
	void reset(const CPrefs& newPrefs);

	virtual int		handleMessage( unsigned long message, unsigned long fromWho );

	void resetCamera();

	private:

	SimpleCamera	camera;
};

class HotKeyListItem : public aListItem
{
public:

	static void init();
	virtual void render();
	virtual void update();
	
	void setDescription( const char* pText );
	void setKey( const char* pText );

	HotKeyListItem( );
	~HotKeyListItem();

	void	setHotKey( long lNew ){ hotKey = lNew; }
	void	setCommand( long lCommand ) { command = lCommand; }

	long	getCommand() const { return command; }
	long	getHotKey() const { return hotKey; }

private:

	aText		description;
	aText		text;
	aAnimGroup	animations[3];
	aRect		rects[2];
	long		hotKey;
	long		command;

	static		HotKeyListItem* s_item;
	
};





//*************************************************************************************************
#endif  // end of file ( OptionsArea.h )
