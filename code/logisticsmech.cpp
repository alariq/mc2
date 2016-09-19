#define LOGISTICSMECH_CPP
/*************************************************************************************************\
LogisticsMech.cpp			: Implementation of the LogisticsMech component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"logisticsmech.h"
#include"logisticsdata.h"
#include"inifile.h"

LogisticsMech::LogisticsMech( LogisticsVariant* pVar, int Count ) : pVariant( pVar )
{
	ID = (Count << 24 | pVar->getID() );
	pilot = 0;
	forceGroup = 0;
}

LogisticsMech::~LogisticsMech()
{
	if ( pilot )
		pilot->setUsed( 0 );

	pilot = NULL;
}

void LogisticsMech::setPilot( LogisticsPilot* pPilot )
{
	if ( pilot )
		pilot->setUsed( 0 );

	pilot = pPilot;
	if ( pilot )
		pilot->setUsed( 1 );
}

void LogisticsMech::setVariant( LogisticsVariant* pVar )
{
	pVariant = pVar;
	ID = ( LogisticsData::instance->createInstanceID( pVar ) << 24 | pVar->getID() );

}


long LogisticsMech::save( FitIniFile& file, long counter )
{
	char tmp[256];
	sprintf( tmp, "Inventory%ld", counter );

	file.writeBlock( tmp );

	file.writeIdString( "Chassis", pVariant->getFileName() );
	file.writeIdString( "Variant", pVariant->getName() );
	if ( pilot )
		file.writeIdString( "Pilot", pilot->getFileName() );
	file.writeIdBoolean( "Designer", pVariant->isDesignerMech() );

	return 0;
}

void	LogisticsMech::setColors( unsigned long base, unsigned long high1, unsigned long high2 )
{
	baseColor = base;
	highlightColor1 = high1;
	highlightColor2 = high2;
}

void	LogisticsMech::getColors( unsigned long& base, unsigned long& high1, unsigned long& high2 ) const
{
	base = baseColor;
	high1 = highlightColor1;
	high2 = highlightColor2;
}

//*************************************************************************************************
// end of file ( LogisticsMech.cpp )
