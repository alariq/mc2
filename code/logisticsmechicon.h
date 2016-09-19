#ifndef LOGISTICSMECHICON_H
#define LOGISTICSMECHICON_H
//===========================================================================//
//LogisticsMechIcon.h		: Interface for the LogisticsMechIcon component. //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
#include"asystem.h"
#include"aanim.h"

class LogisticsMech;
class LogisticsPilot;

#define ICON_ANIM_COUNT 4
/**************************************************************************************************
CLASS DESCRIPTION
LogisticsMechIcon:
**************************************************************************************************/
class LogisticsMechIcon
{

public:

	LogisticsMechIcon();
	~LogisticsMechIcon();
	
	static int init( FitIniFile& file );

	void update();
	void render( long offsetX, long offsetY );

	void setMech( LogisticsMech* pMech );
	LogisticsMech* getMech( ){ return pMech; }
	LogisticsPilot* getPilot();
	void setPilot( LogisticsPilot* pPilot);

	float width(){ return outline.width(); }
	float height(){ return outline.height(); }
	void move( long x, long y );

	bool justSelected(){ return bJustSelected; }
	void select( bool bSelect );
	bool isSelected();

	long globalX() const { return outline.globalX(); }
	long globalY() const { return outline.globalY(); }
	float width() const { return outline.width(); }
	float height() const { return outline.height(); }

	bool pointInside(long x, long y) const { return outline.pointInside( x, y ); }

	void dimPilot( bool bDim );
	void setHelpID( int newID ) { helpID = newID; }

	void disable( bool bdo) { bDisabled = bdo; }

	static LogisticsMechIcon*	s_pTemplateIcon;

private:
	
	LogisticsMechIcon& operator=( const LogisticsMechIcon& src );
	LogisticsMechIcon( const LogisticsMechIcon& src );

	aText		pilotName;
	aText		chassisName;
	aObject		iconConnector;
	aObject		icon;
	aObject		pilotIcon;
	aObject		outline;

	long		pilotID;
	long		chassisNameID;
	long		iconConnectorID;
	long		iconID;
	long		pilotIconID;
	long		outlineID;

	aAnimGroup	animations[ICON_ANIM_COUNT];

	LogisticsMech*	pMech;
	long			state;
	bool			bJustSelected;
	long			helpID;
	bool			bDisabled;

	void renderObject( aObject& obj, long color, long xOffset, long yOffset );
	static void assignAnimation( FitIniFile& file, long& number );




	
	
};


//*************************************************************************************************
#endif  // end of file ( LogisticsMechIcon.h )
