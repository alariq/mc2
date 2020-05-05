#ifndef MPSETUP_H
#define MPSETUP_H
/*************************************************************************************************\
MPSetupArea.h			: Interface for the MPSetupArea component.
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

#ifndef AANIM_H
#include"aanim.h"
#endif

class aButton;


class aColorPicker : public aObject
{
public:

	aColorPicker();

	virtual long		init(long xPos, long yPos, long w, long h);
	void				init( FitIniFile* file, const char* blockName );

	virtual void		destroy();
	virtual void		render();
	virtual void		update();
	virtual int			handleMessage( unsigned long message, const void* who );
	virtual void		move( float offsetX, float offsetY );
	void				setColor0(int color);
	int				getColor0() const { return color0; }
	void				setColor1(int color);
	int				getColor1() const { return color1; }

protected:
	aRect		mainRect;

	aText		tab0text;
	aRect		tab0ColorOutlineRect;
	aRect		tab0ColorRect;
	aButton		tab0Button;

	aText		tab1text;
	aRect		tab1ColorOutlineRect;
	aRect		tab1ColorRect;
	aButton		tab1Button;

	aObject		colorPlaneStatic;
	aRect		intensityGradientRect;
	mcScrollBar		intesitySliderScrollBar;
	aObject		colorPlaneCursorStatic;
	int		color0;
	int		color1;
	int		activeTab;
};

class aStyle1TextListItem : public aTextListItem
{
public:
	aStyle1TextListItem() { hasAnimation = false; normalColor = 0xff808080; }
	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void		render();

protected:
	bool hasAnimation;
	aAnimGroup animGroup;
	int normalColor;
};

class aInsigniaListItem : public aListItem
{
public:
	aInsigniaListItem() { ; }
	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void update();

protected:
	aObject graphic;
	aStyle1TextListItem text;
};

class MPSetupXScreen : public LogisticsScreen
{
public:
	
	MPSetupXScreen();
	virtual ~MPSetupXScreen();
	
	void init(FitIniFile* file);
	bool isDone();
	virtual void		begin();
	virtual void		end();
 	virtual void render( int xOffset, int yOffset );
	virtual void render();
	virtual void update();
	virtual int			handleMessage( unsigned long, unsigned long );

	void updateMPSetup(); // put into inventory

	bool				bDone;

	void **ppConnectionScreen;
	void *pLocalBrowserScreen;
	void *pDirectTcpipScreen;
	void *pModem2ModemScreen;

private:

	int indexOfButtonWithID(int id);

	aComboBox				playerNameComboBox;
	aComboBox				unitNameComboBox;
	aDropList				insigniaDropList;

	aColorPicker		colorPicker;
	int		baseColor;
	int		stripeColor;

	SimpleCamera		mechCamera;
	bool bPaintSchemeInitialized;
};



//*************************************************************************************************
#endif  // end of file ( MPSetupArea.h )
