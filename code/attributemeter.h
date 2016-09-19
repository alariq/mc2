#ifndef ATTRIBUTEMETER_H
#define ATTRIBUTEMETER_H
//===========================================================================//
//AttributeMeter.h	: Implementation of the AttributeMeter component.        //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"utilities.h"
//*************************************************************************************************
class FifIniFile;

#define MAX_ARMOR_RANGE 432.f
#define MAX_SPEED_RANGE 35.f
#define MAX_JUMP_RANGE 125.f
/**************************************************************************************************
CLASS DESCRIPTION
AttributeMeter:
**************************************************************************************************/
class AttributeMeter
{
	public:

	AttributeMeter();

	void update();
	void render();
	void render( long xOffset, long yOffset );

	void init( FitIniFile* file, const char* headerName );

	void setValue( float val ){ percent = val < 0 ? 0 : val; }
	void setAddedValue( float val ){ addedPercent = val < 0 ? 0 : val; }
	void setColorMin( long newColor ) { colorMin = newColor; }
	void setColorMax( long newColor ){ colorMax = newColor; }
	void setAddedColorMin( long newColor ){ addedColorMin = newColor; }
	void setAddedColorMax( long newColor ){ addedColorMax = newColor; }
	void showGUIWindow( bool show ){ bShow = show; }
	bool pointInside(long mouseX, long mouseY) const;


	private:

	GUI_RECT	outsideRect;
	long		unitHeight;
	long		unitWidth;
	long		unitCount;
	long		skipWidth;
	long		rectColor;
	long		colorMin;
	long		colorMax;
	long		addedColorMin;
	long		addedColorMax;
	float		percent;
	float		addedPercent;
	long		numBars;

	bool		bShow;

	long helpID;
};


//*************************************************************************************************
#endif  // end of file ( AttributeMeter.h )
