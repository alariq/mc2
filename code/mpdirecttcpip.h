#ifndef MPDIRECTTCPIP_H
#define MPDIRECTTCPIP_H
/*************************************************************************************************\
MPDirectTcpip.h			: Interface for the MPDirectTcpip component.
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


class aStyle7TextListItem : public aTextListItem
{
public:
	aStyle7TextListItem() { hasAnimation = false; normalColor = 0xff808080; }
	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void		render();

protected:
	bool hasAnimation;
	aAnimGroup animGroup;
	int normalColor;
};

class MPDirectTcpip : public LogisticsScreen
{
public:
	
	MPDirectTcpip();
	virtual ~MPDirectTcpip();
	
	void init(FitIniFile* file);
	bool isDone();
	virtual void		begin();
	virtual void		end();
 	virtual void render( int xOffset, int yOffset );
	virtual void render();
	virtual void update();
	virtual int			handleMessage( unsigned long, unsigned long );

	bool				bDone;


private:
	int indexOfButtonWithID(int id);

	aComboBox				ipAddressComboBox;
};



//*************************************************************************************************
#endif  // end of file ( MPDirectTcpip.h )
