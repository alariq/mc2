#ifndef LOGISTICSSCREEN_H
#define LOGISTICSSCREEN_H
/*************************************************************************************************\
LogisticsScreen.h			: Interface for the LogisticsScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"asystem.h"
//*************************************************************************************************
class FitIniFile;
class aObject;
class aRect;
class aText;
class aAnimButton;
class aButton;
class aEdit;
class aAnimObject;
/**************************************************************************************************
CLASS DESCRIPTION
LogisticsScreen:
**************************************************************************************************/
class LogisticsScreen : public aObject
{
	public:

	enum Status
	{
		RUNNING = 0,
		NEXT = 1,
		PREVIOUS = 2,
		DONE = 3,
		PAUSED = 4,
		UP,
		DOWN,
		YES,
		NO,
		MAINMENU,
		RESTART,
		MULTIPLAYERRESTART,
		SKIPONENEXT,
		SKIPONEPREVIOUS,
		FADEDOWN,
		FADEUP,
		READYTOLOAD,
		GOTOSPLASH
	};

	LogisticsScreen();
	virtual ~LogisticsScreen();
	LogisticsScreen( const LogisticsScreen& src );
	LogisticsScreen& operator=( const LogisticsScreen& src );

	void init(FitIniFile& file, const char* staticName, const char* textName, const char* rectName,
					  const char* buttonName, const char* editName = "Edit",
					  const char* animObjectName = "AnimObject", DWORD neverFlush = 0 );

	virtual void update();
	virtual void render();

	virtual void begin();
	virtual void end(){}

	virtual void render( int xOffset, int yOffset );

	long getStatus();

	aButton* getButton( long who );
	aRect* getRect( long who );

	virtual void  moveTo( long xPos, long yPos );
	virtual void  move( long xPos, long yPos );


	bool	inside( long x, long y);

	void	beginFadeIn( float fNewTime ){ fadeInTime = fNewTime; fadeOutTime = fadeTime = 0.f; }
	void	beginFadeOut( float fNewTime ) { fadeInTime = 0.f; fadeOutTime = fNewTime; fadeTime = 0.f; }


	void	clear(); // remove everything


	
	aObject*			statics;
	aRect*				rects;
	long				rectCount;
	long				staticCount;

	aText*				textObjects;
	long				textCount;

	aAnimButton*			buttons;
	long				buttonCount;

	aEdit*				edits;
	long				editCount;

	aAnimObject*		animObjects;
	long				animObjectsCount;

	float				fadeInTime;
	float				fadeOutTime;
	float				fadeTime;


	protected:

	long				status;
	long				fadeOutMaxColor;

	long				helpTextArrayID;
	private:

	void copyData( const LogisticsScreen& );
	void destroy();





};


//*************************************************************************************************
#endif  // end of file ( LogisticsScreen.h )
