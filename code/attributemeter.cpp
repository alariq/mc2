#define ATTRIBUTEMETER_CPP
//===========================================================================//
//AttributeMeter.cpp	: Implementation of the AttributeMeter component.    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"attributemeter.h"
#include"inifile.h"
#include"mclib.h"

extern long helpTextID;
extern long helpTextHeaderID;


AttributeMeter::AttributeMeter()
{

	memset( &outsideRect, 0, sizeof( outsideRect ) );
	unitHeight = 0;
	unitWidth = 0;
	unitCount = 0;
	skipWidth = 0;
	rectColor = 0;
	colorMin = 0xff005392;
	colorMax = 0xffcdeaff;
	numBars = 0;
	percent = 0;
	addedPercent = 0;
	bShow = 1;
}

bool AttributeMeter::pointInside(long mouseX, long mouseY) const
{
	if ( (outsideRect.left)  <= mouseX && 
		 outsideRect.right >= mouseX && 
		 outsideRect.top <= mouseY &&
		 outsideRect.bottom >= mouseY )
		 return true;

	return false;
}


void AttributeMeter::update()
{
	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();

	if ( pointInside( mouseX, mouseY ) )
		::helpTextID = helpID;
}
void AttributeMeter::render()
{
	
	render( 0, 0 );
}


void AttributeMeter::render( long xOffset, long yOffset )
{
	if ( !bShow )
		return; 

	float barCount = percent * numBars;
	int  nBarCount = (int)barCount;

	float remainder = addedPercent * numBars + (barCount - float(nBarCount));
	int nAddedCount = (int)remainder;

	GUI_RECT tmpOutside = outsideRect;
	tmpOutside.left += xOffset;
	tmpOutside.right += xOffset;
	tmpOutside.top += yOffset;
	tmpOutside.bottom += yOffset;	
	//drawRect( tmpOutside, 0xff000000 );
	drawEmptyRect( tmpOutside, rectColor, rectColor );

	GUI_RECT tmpRect;
	tmpRect.left = outsideRect.left + 2 * skipWidth + xOffset;
	tmpRect.right = tmpRect.left + unitWidth;
	tmpRect.top = outsideRect.top + 2 * skipWidth + yOffset;
	tmpRect.bottom = outsideRect.bottom - 2 * skipWidth + yOffset;

	long color = colorMin;

	tmpRect.bottom += skipWidth;
	for ( int i = 0; i < nBarCount; i++ )
	{
		drawRect( tmpRect, color );
		color = interpolateColor( colorMin, colorMax, ((float)i)/(float)numBars );
		
		tmpRect.left += unitWidth + skipWidth;
		tmpRect.right = tmpRect.left + unitWidth;
	}

	for ( i = 0; i < nAddedCount; i++ )
	{
		drawRect( tmpRect, color );
		color = interpolateColor( addedColorMin, addedColorMax, ((float)i)/(float)numBars );
		
		tmpRect.left += unitWidth + skipWidth;
		tmpRect.right = tmpRect.left + unitWidth;
	}

	tmpRect.bottom -= skipWidth;
	tmpRect.right -= 1;
	for ( i = nBarCount + nAddedCount; i < numBars; i++ )
	{
		drawEmptyRect( tmpRect, rectColor, rectColor );
		
		tmpRect.left += unitWidth + skipWidth;
		tmpRect.right = tmpRect.left + unitWidth-1;
	}

}


void AttributeMeter::init( FitIniFile* file, const char* headerName )
{
	if ( NO_ERR != file->seekBlock( headerName ) )
	{
		char errorTxt[256];
		sprintf( errorTxt, "couldn't find block %s in file %s", headerName, file->getFilename() );
		Assert( 0, 0, errorTxt );
		return;
	}

	if ( NO_ERR == file->readIdLong( "left", outsideRect.left ) )
	{
		file->readIdLong( "right", outsideRect.right );
		file->readIdLong( "top", outsideRect.top );
		file->readIdLong( "bottom", outsideRect.bottom );	
	}
	else
	{
		file->readIdLong( "XLocation", outsideRect.left );
		file->readIdLong( "YLocation", outsideRect.top );
		long tmp;
		file->readIdLong( "Width", tmp );
		outsideRect.right = outsideRect.left + tmp;
		file->readIdLong( "Height", tmp );
		outsideRect.bottom = outsideRect.top + tmp;

	}
	file->readIdLong( "UnitWidth", unitWidth );
	file->readIdLong( "Skip", skipWidth );
	file->readIdLong( "NumberUnits", numBars );
	file->readIdLong( "Color", rectColor );

	file->readIdLong( "HelpDesc", helpID );


	long tmp;
	if ( NO_ERR == file->readIdLong( "ColorMin", tmp ) )
	{
		colorMin = tmp;
	}

	if ( NO_ERR == file->readIdLong( "ColorMax", tmp ) )
	{
		colorMax = tmp;
	}


}


	
//*************************************************************************************************
// end of file ( AttributeMeter.cpp )
