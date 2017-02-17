#define LOGISTICSDATA_CPP
/*************************************************************************************************\
LogisticsData.cpp			: Implementation of the LogisticsData component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"logisticsdata.h"
#include"file.h"
#include"logisticserrors.h"
#include"cmponent.h"
#include"paths.h"
#include"warrior.h"
#include"../resource.h"
#include"malloc.h"
#include"team.h"
#include"mech.h"
#include"logisticsmissioninfo.h"
#include"packet.h"
#include"gamesound.h"
#include"prefs.h"
#include"comndr.h"
#include"missionresults.h"
#include<zlib/zlib.h>

#ifndef VIEWER
#include"multplyr.h"
#include"chatwindow.h"
#else
	bool MissionResults::FirstTimeResults = false;
#endif

extern CPrefs prefs;

//----------------------------------------------------------------------
// WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//PLEASE CHANGE THIS IF THE SAVEGAME FILE FORMAT CHANGES!!!!!!!!!!!!!
long SaveGameVersionNumber = 10004;
//----------------------------------------------------------------------

LogisticsData* LogisticsData::instance = NULL;

//*************************************************************************************************
LogisticsData::LogisticsData()
{
	gosASSERT( !instance );
	instance = this;
	resourcePoints = 0;

	currentlyModifiedMech = 0;
	missionInfo = 0;
	rpJustAdded = false;

	bNewMechs = bNewWeapons = bNewPilots = 0;
}

LogisticsData::~LogisticsData()
{
	for( VARIANT_LIST::EIterator iter = variants.Begin(); !iter.IsDone(); iter++ )
	{
		delete( *iter );
	}

	for ( VEHICLE_LIST::EIterator vIter = vehicles.Begin(); !vIter.IsDone(); vIter++ )
	{
		delete (*vIter );
	}

	for ( MECH_LIST::EIterator mIter = inventory.Begin(); !mIter.IsDone(); mIter++ )
	{
		delete (*mIter);
	}

	variants.Clear();
	vehicles.Clear();

	delete missionInfo;
	missionInfo = NULL;

#ifndef VIEWER
	ChatWindow::static_destroy();
#endif
}

//*************************************************************************************************
void LogisticsData::init()
{
	if ( components.Count() ) // already been initialized
	{
		return;
	}
	
	initComponents();
	initPilots();
	initVariants();

	missionInfo = new LogisticsMissionInfo;

	FitIniFile file;
	if ( NO_ERR != file.open( "data" PATH_SEPARATOR "campaign" PATH_SEPARATOR "campaign.fit" ) )
	{
		Assert( 0, 0, "coudln't find the campaign file\n" );
	}
	missionInfo->init( file );

	// temporary, just so we can test
	int count = 32;
	const char* missionNames[32];
	missionInfo->getAvailableMissions( missionNames, count );

	setCurrentMission( missionNames[0] );
}

//*************************************************************************************************
void LogisticsData::initComponents()
{
	char componentPath[256];
	strcpy( componentPath, objectPath );
	strcat( componentPath, "compbas.csv" );

	
	File componentFile;
#ifdef _DEBUG
	int result = 
#endif
		componentFile.open( componentPath );
	gosASSERT( result == NO_ERR );

	BYTE* data = new BYTE[componentFile.getLength()];

	componentFile.read( data, componentFile.getLength() );


	File dataFile;
	dataFile.open( (char*)data, componentFile.getLength() );

	componentFile.close();
	
	BYTE line[1024];
	char* pLine = (char*)line;

	// skip the first line
	dataFile.readLine(line, 1024);

	int		Ammo[512];
	memset( Ammo, 0, sizeof ( int ) * 512 );


	LogisticsComponent tmpComp;
	int counter = 0;
	while(true)
	{
		int result = dataFile.readLine( line, 1023 );

		if ( result < 2 || result == 0xBADF0008 || result > 1024  )
			break;

		components.Append( tmpComp );

		LogisticsComponent& tmp = components.GetTail();

		if ( -1 == tmp.init( pLine ) ) // failure
		{
			Ammo[counter] = (long)tmp.getRecycleTime();
			components.DeleteTail();
		}

		counter++;
	}
	
	// fix up ammo
	for ( COMPONENT_LIST::EIterator iter = components.Begin(); !iter.IsDone(); iter++ )
	{
		if ((*iter).getAmmo() )
		{
			(*iter).setAmmo( Ammo[(*iter).getAmmo()] );
		}
	}

	delete [] data;
	data = NULL;
}

//*************************************************************************************************
void LogisticsData::initPilots()
{

	pilots.Clear();

	char pilotPath[256];
	strcpy( pilotPath, objectPath );
	strcat( pilotPath, "pilots.csv" );

	
	File pilotFile;
	pilotFile.open( pilotPath );

	BYTE pilotFileName[256];

	int id = 1;

	while( true )
	{
		int bytesRead = pilotFile.readLine( pilotFileName, 256 );
		
		if ( bytesRead < 2 )
			break;

		LogisticsPilot tmpPilot;
		pilots.Append( tmpPilot );
		LogisticsPilot& pilot = pilots.GetTail();
		pilot.id = id;

		if ( -1 == pilot.init( (char*)pilotFileName ) )
			pilots.DeleteTail();

		id++;

	}
}

void LogisticsData::initVariants()
{
	char variantPath[256];
	strcpy( variantPath, artPath );
	strcat( variantPath, "buildings.csv" );


	CSVFile variantFile;
	variantFile.open( variantPath );

	FullPathFileName pakPath;
	pakPath.init( objectPath, "Object2", ".pak" );

	PacketFile pakFile;
	
	if ( NO_ERR !=pakFile.open( pakPath ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)pakPath );
		Assert( 0, 0, errorStr );
	}


	char variantFileName[256];
	char variantFullPath[1024];

	int chassisID = 0;

	char tmpStr[256];

	int i = 1;
	while( true )
	{
		long fitID;

		int retVal = variantFile.readString( i, 4, tmpStr, 256 );
		
		if ( retVal != 0 )
			break;

		if ( S_stricmp( tmpStr, "VEHICLE" ) == 0 )
		{
			float scale;
			variantFile.readFloat( i, 11, scale );
			if ( scale )
			{
				variantFile.readLong( i, 5, fitID );
				addVehicle( fitID, pakFile, scale);
			}
			i++;
			continue;
		}
		if ( S_stricmp( tmpStr, "MECH" ) != 0 )
		{
		
			float scale;
			if ( NO_ERR == variantFile.readFloat( i, 11, scale ) && scale )
			{
				variantFile.readLong( i, 5, fitID );
				addBuilding( fitID, pakFile, scale );
			}
			
			i++;
			continue;

		}
		
		float scale;
		if ( NO_ERR != variantFile.readFloat( i, 11, scale ))
			scale = 1.0;

		variantFile.readString( i, 1, variantFileName, 256 );

		variantFile.readLong( i, 5, fitID );

		strcpy( variantFullPath, objectPath );
		strcat( variantFullPath, variantFileName );
		strcat(  variantFullPath, ".csv" );
		_strlwr( variantFullPath );

		CSVFile mechFile;
		if ( NO_ERR != mechFile.open( variantFullPath ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", variantFullPath );
			Assert( 0, 0, error );
			return;
		}

		LogisticsChassis* chassis = new LogisticsChassis();
		chassis->init( &mechFile, chassisID++ );
		chassis->setFitID(fitID);
		chassis->setScale( scale );

		int row = 23;
		char buffer[256];
		int varCount = 0;
		while( NO_ERR == mechFile.readString( row, 2, buffer, 256 ) )
		{
			LogisticsVariant* pVariant = new LogisticsVariant;
			
			if ( 0 == pVariant->init( &mechFile, chassis, varCount++ ) )
				variants.Append( pVariant );
			else
				delete pVariant;

			row += 97;
		}

		i++;

	}
}

void LogisticsData::addVehicle( long fitID, PacketFile& objectFile, float scale )
{
	if ( NO_ERR != objectFile.seekPacket(fitID) )
		return;

	int fileSize = objectFile.getPacketSize();

	if ( fileSize )
	{
		LogisticsVehicle* pVehicle = new LogisticsVehicle;

		FitIniFile file;
		 file.open(&objectFile, fileSize);

		pVehicle->init( file );
		pVehicle->setScale( scale );
		vehicles.Append( pVehicle );
	}
}


//*************************************************************************************************
void LogisticsData::RegisterFunctions()
{
	

}

//*************************************************************************************************
void LogisticsData::UnRegisterFunctions()
{

}


int LogisticsData::getAvailableComponents( LogisticsComponent** pComps, int& maxCount )
{
	int retVal = 0;
	
	int i = 0;
	for ( COMPONENT_LIST::EIterator iter = components.Begin(); 
		!iter.IsDone(); iter++ )
		{
			if ( (*iter).isAvailable()  )
			{
				if ( i < maxCount )
					pComps[i]	= &(*iter);	
				else 
					retVal = NEED_BIGGER_ARRAY;
				++i;

				
			}
		}

	maxCount = i;

	return retVal; 

}
int	LogisticsData::getAllComponents( LogisticsComponent** pComps, int& maxCount )
{
	int retVal = 0;
	
	int i = 0;
	for ( COMPONENT_LIST::EIterator iter = components.Begin(); 
		!iter.IsDone(); iter++ )
		{
			if ( i < maxCount )
					pComps[i]	= &(*iter);	
				else 
					retVal = NEED_BIGGER_ARRAY;
				++i;
		}

	maxCount = components.Count();

	return retVal; 
}




int LogisticsData::getPurchasableMechs( LogisticsVariant** array, int& count )
{
	long retVal = 0;
	long arraySize = count;

	count = 0;
	for( VARIANT_LIST::EIterator iter = instance->variants.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->isAvailable() )
		{
			if ( count >= arraySize )
			{
				retVal =  NEED_BIGGER_ARRAY; // keep counting
			}
			else
			{
				array[count] = (*iter);
			}

			(count)++;	
		}
	}

	return retVal;

}




int LogisticsData::purchaseMech( LogisticsVariant* pVariant )
{
	if ( !pVariant )
		return -1;

	int RP = pVariant->getCost();

	if ( missionInfo->getCBills() - RP >= 0 )
	{
		int count = instance->createInstanceID( pVariant );
		LogisticsMech* pMech = new LogisticsMech( pVariant, count );
		instance->inventory.Append( pMech );
		missionInfo->decrementCBills( pVariant->getCost() );
		return 0;
	}

	return NOT_ENOUGH_RESOURCE_POINTS;
}

int LogisticsData::canPurchaseMech( LogisticsVariant* pVar )
{
	if ( !pVar )
		return INVALID_ID;

	int RP = pVar->getCost();

	if ( missionInfo->getCBills() - RP >= 0 )
	{
		return 0;
	}

	return NOT_ENOUGH_RESOURCE_POINTS;

}



int LogisticsData::sellMech( LogisticsMech* pVar )
{
	if ( !pVar )
		return -1;

	for ( MECH_LIST::EIterator iter = instance->inventory.End(); !iter.IsDone(); iter-- )
	{
		if ( (*iter)->getForceGroup() )
			continue;
		if ( (*iter)->getVariant() == pVar->getVariant() )
		{
			int cost = ((*iter))->getCost();
			(*iter)->setPilot( NULL );
			delete *iter;
			instance->inventory.Delete( iter );

			missionInfo->incrementCBills( cost );
			return 0;
		}
	}

	return -1;
}

int LogisticsData::removeVariant( const char* varName )
{
	if ( !varName )
		return -1;

	LogisticsVariant* pVar = 0;

	if ( currentlyModifiedMech->getName() == varName || oldVariant->getName() == varName )
		return -1;

    VARIANT_LIST::EIterator vIter = variants.Begin();
	for ( ; !vIter.IsDone(); vIter++ )
	{
		if ( (*vIter)->getName().Compare( varName, 0 ) == 0 )
		{
			pVar = (*vIter );
			break;
		}
	}

	if ( !pVar )
	{
		return INVALID_VARIANT;
	}

    
	for (MECH_LIST::EIterator iter = instance->inventory.End(); !iter.IsDone(); iter-- )
	{
		if ( (*iter)->getVariant() == pVar )
		{
			return VARIANT_IN_USE;
		}
	}

	delete pVar;
	variants.Delete( vIter );

	return 0;
}



int LogisticsData::createInstanceID( LogisticsVariant* pVar )
{
	int count = -1;
	for ( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( pVar->getVariantID() == (*iter)->getVariantID() ) 
		{
			int tmp = (*iter)->getInstanceID();
			if ( tmp > count )
				count = tmp;
		}
	}
	return count + 1;
}

LogisticsVariant* LogisticsData::getVariant( int ID )
{
	for ( VARIANT_LIST::EIterator iter = variants.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getID() == (ID & 0x00ffffff) )
		{
			return *iter;
		}
	}

	if ( instance->currentlyModifiedMech && ID == instance->currentlyModifiedMech->getID() )
		return instance->currentlyModifiedMech->getVariant();

	return NULL;
}

LogisticsMech*	LogisticsData::getMech( int ID )
{
	for ( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getID() == ID )
			return (*iter );
	}

	return NULL;
}




int LogisticsData::addMechToForceGroup( LogisticsMech* pMech, int slot )
{
	if ( !pMech )
		return -1;

	if ( slot > 12 )
		return -1;

	if ( pMech && !pMech->getForceGroup() )
	{
		pMech->setForceGroup( slot );
		return 0;
	}
	else // find another of the same variant
	{

		LogisticsMech* pNewMech = getMechWithoutForceGroup( pMech );
		if ( pNewMech )
		{
			pNewMech->setForceGroup( slot );
			return 0;
		}
	}
	return -1;
}

int		LogisticsData::removeMechFromForceGroup( int slot )
{
	for ( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getForceGroup() == slot )
		{
			(*iter)->setForceGroup( 0 );
			(*iter)->setPilot( 0 );
			return 0;
		}
	}

	return -1;

}

LogisticsMech*		LogisticsData::getMechWithoutForceGroup( LogisticsMech* pMech )
{
	if ( !pMech->getForceGroup() )
		return pMech;

	for ( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
		{
			if ( (*iter)->getVariant() == pMech->getVariant() && !(*iter)->getForceGroup() )
			{
				return (*iter);
				
			}
		}
	
	return NULL;
}
int LogisticsData::removeMechFromForceGroup( LogisticsMech* pMech, bool bRemovePilot )
{
	if ( !pMech )
		return -1;

	if ( pMech && pMech->getForceGroup() )
	{
		pMech->setForceGroup( 0 );
		// no mechs in inventory have pilots
		if ( bRemovePilot )
			pMech->setPilot( 0 );
		return 0;
	}

	// find similar one
	for ( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getVariant() == pMech->getVariant() && (*iter)->getForceGroup() )
		{
			(*iter)->setForceGroup( 0 );

			if ( bRemovePilot )
				pMech->setPilot( 0 );

			return 0;
		}
	}
	return -1;
}

LogisticsPilot* LogisticsData::getFirstAvailablePilot()
{
	for ( PILOT_LIST::EIterator iter = pilots.Begin(); !iter.IsDone(); iter++  )
	{
		 bool bIsUsed = false;
		 for ( MECH_LIST::EIterator mIter = inventory.Begin(); !mIter.IsDone(); mIter++ )
		 {
			LogisticsPilot* pPilot = (*mIter)->getPilot();
			if ( pPilot && pPilot->getID() == (*iter).getID() )
			{
				bIsUsed = true;
				break;
			}
		 }
		 if ( !bIsUsed )
		 {
			 return &(*iter);
		 }
	}

	return NULL;

}



// GetAvailableMissions( char** missionNames, long& count )
int LogisticsData::getAvailableMissions( const char** missionNames, long& count )
{
	int numberOfEm = 0;

	// first figure out how many there are
	missionInfo->getAvailableMissions( 0, numberOfEm );

	// next make sure the array is big enough
	if ( count < numberOfEm )
		return NEED_BIGGER_ARRAY;

	missionInfo->getAvailableMissions( missionNames, numberOfEm );
	count= numberOfEm;
	
	return 0;



}

int LogisticsData::getCurrentMissions( const char** missionNames, long& count )
{
	int numberOfEm = 0;

	// first figure out how many there are
	missionInfo->getCurrentMissions( 0, numberOfEm );

	// next make sure the array is big enough
	if ( count < numberOfEm )
		return NEED_BIGGER_ARRAY;

	numberOfEm = count;

	missionInfo->getCurrentMissions( missionNames, numberOfEm );
	count= numberOfEm;
	
	return 0;



}

bool LogisticsData::getMissionAvailable( const char* missionName )
{
	return missionInfo->getMissionAvailable( missionName );
}


// SetCurrentMission( char* missionName )
int LogisticsData::setCurrentMission( const char* missionName )
{
	long result = missionInfo->setNextMission( missionName );

	if ( result == NO_ERR )
	{
		// if we made it this far
		updateAvailability();

		resourcePoints = missionInfo->getCurrentRP();

		removeDeadWeight();
	}


	

	return result;
}

void LogisticsData::removeDeadWeight()
{
	int maxDropWeight = getMaxDropWeight();
	int curDropWeight = getCurrentDropWeight();

	int i = 12;
	while ( curDropWeight > maxDropWeight )
	{
		LogisticsData::removeMechFromForceGroup( i );
		i--;

		curDropWeight = getCurrentDropWeight();

		if ( i == 0 )
			break;
	}
}

int		LogisticsData::setCurrentMission( const EString& missionName )
{
	return setCurrentMission( (const char*)missionName );
}


void	LogisticsData::getForceGroup( EList<LogisticsMech*, LogisticsMech*>& newList )
{
	int count = 0;

	for ( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( count > 11 )
			break;

		if ( (*iter)->getForceGroup() )
		{
			newList.Append( (*iter) );
			count++;
		}
	}
}

void	LogisticsData::getInventory( EList<LogisticsMech*, LogisticsMech*>& newList )
{
	for ( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{

		newList.Append( (*iter) );
	}
}


void	LogisticsData::addMechToInventory( LogisticsVariant* pVar, LogisticsPilot* pPilot, int ForceGroup,
										  bool bSubtractPts)
{
	if ( pVar )
	{
		int count = instance->createInstanceID( pVar );
		LogisticsMech* pMech = new LogisticsMech( pVar, count );
		inventory.Append( pMech );
		pMech->setForceGroup( ForceGroup );
		if ( pPilot && !pPilot->isDead() )
			pMech->setPilot( pPilot );
		if ( ForceGroup > -1 && ForceGroup < 13 )
			pMech->setForceGroup( ForceGroup );
		if ( bSubtractPts )
			missionInfo->decrementCBills( pMech->getCost() );
	}
}
void	LogisticsData::addMechToInventory( LogisticsVariant* pVar, int addToForceGroup, 
										  LogisticsPilot* pPilot,
										  unsigned long baseColor,
										  unsigned long highlight1,
										  unsigned long highlight2 )
{
	if ( pVar )
	{
		int count = instance->createInstanceID( pVar );
		LogisticsMech* pMech = new LogisticsMech( pVar, count );
		inventory.Append( pMech );
		if ( addToForceGroup > -1 && addToForceGroup < 13 )
			pMech->setForceGroup( addToForceGroup );
		if ( pPilot && !pPilot->isDead() )
			pMech->setPilot( pPilot );
		pMech->setColors( baseColor, highlight1, highlight2 );
		return;
	}
	else
	{
		gosASSERT(!"couldn't add the mech to the inventory" );
	}
}

LogisticsVariant* LogisticsData::getVariant( const char* pCSVFileName, int VariantNum )
{
	EString lowerCase = pCSVFileName;
	lowerCase.MakeLower();
	for( VARIANT_LIST::EIterator iter = variants.Begin(); !iter.IsDone(); iter++ )
	{
		if ( -1 !=( (*iter)->getFileName().Find( lowerCase, -1 ) )
			&& (((*iter)->getVariantID()>>16)&0xff) == VariantNum )
		{
			return *iter;
		}
	}

	return NULL;
}

void LogisticsData::removeMechsInForceGroup()
{
	if ( !inventory.Count() )
		return;
	for ( MECH_LIST::EIterator iter = inventory.End(); !iter.IsDone();  )
	{
		if ( (*iter)->getForceGroup() )
		{
			MECH_LIST::EIterator tmpIter = iter;
			iter--;
			delete *tmpIter;
			inventory.Delete( tmpIter );
		}
		else
			iter--;
	}
}

const char*	LogisticsData::getBestPilot( long mechWeight )
{
	if ( !pilots.Count() )
		initPilots();

	LogisticsPilot** pPilots = (LogisticsPilot**)_alloca( pilots.Count() * sizeof( LogisticsPilot*) );
	memset( pPilots, 0, pilots.Count() * sizeof( LogisticsPilot*) );

	int counter = 0;
#ifndef VIEWER
	for ( PILOT_LIST::EIterator iter = pilots.Begin();
		!iter.IsDone(); iter++ )
		{
			const char *nameCheck = (*iter).getName();
			if ( (*iter).isAvailable() && (MPlayer || !MechWarrior::warriorInUse((char *)nameCheck)) )
				pPilots[counter++] = &(*iter);
		}
#endif

	long count = counter;

	for ( int i = 1; i < count; ++i )
	{
		LogisticsPilot* cur = pPilots[i];
		for ( int j = 0; j < i; ++j )
		{
			if ( comparePilots( cur, pPilots[j], mechWeight ) > 0 && j != i )
			{
				pPilots[i] = pPilots[j];
				pPilots[j] = cur;
				break;
			}
		}
	}

	for (int i = 0; i < count; i++ )
	{
		if ( pPilots[i]->isAvailable() && !pPilots[i]->isUsed() )
		{
			pPilots[i]->setUsed( 1 );
			return pPilots[i]->getFileName();
		}
	}

	gosASSERT( !"We're out of pilots, shouldn't be here" );
	
	pPilots[0]->setUsed( true );
	return pPilots[0]->getFileName();


}

bool		LogisticsData::gotPilotsLeft()
{
	if ( !pilots.Count() )
		initPilots();

	LogisticsPilot** pPilots = (LogisticsPilot**)_alloca( pilots.Count() * sizeof( LogisticsPilot*) );
	memset( pPilots, 0, pilots.Count() * sizeof( LogisticsPilot*) );

	int counter = 0;

	#ifndef VIEWER

	for ( PILOT_LIST::EIterator iter = pilots.Begin();
		!iter.IsDone(); iter++ )
		{
			const char *nameCheck = (*iter).getName();
			if ( (*iter).isAvailable() && ( MPlayer || !MechWarrior::warriorInUse((char *)nameCheck)) )
				pPilots[counter++] = &(*iter);
		}

	#endif
	long count = counter;

	for ( int i = 0; i < count; i++ )
	{
		if ( pPilots[i]->isAvailable() && !pPilots[i]->isUsed() )
		{
			return 1;
		}
	}

	return 0;

}

int LogisticsData::comparePilots( LogisticsPilot* p1, LogisticsPilot* p2, long weight )
{
	if ( p1->isUsed() )
		return -1;
	else if ( p2->isUsed() )
		return 1;
	
		for ( MECH_LIST::EIterator mIter = instance->inventory.Begin(); !mIter.IsDone(); mIter++ )
		 {
			LogisticsPilot* pPilot = (*mIter)->getPilot();
			if ( pPilot )
			{
				if ( pPilot->getID() == p1->getID()  )
					return -1;
				else if ( pPilot->getID() == p2->getID() )
					return 1;
			}
		 }



	if ( p1->getRank() > p2->getRank() )
		return 1;
	
	else if ( p2->getRank() < p1->getRank() )
		return -1;

	// need to check specialty skill text for weight, not really done yet

	else if ( p2->getGunnery() > p1->getGunnery() )
		return -1;

	else if ( p1->getGunnery() > p2->getGunnery() )
		return 1;

	else if ( p1->getPiloting() > p2->getPiloting() )
		return 1;

	else if ( p2->getPiloting() > p1->getPiloting() )
		return -1;

	return 0;


}

long	LogisticsData::save( FitIniFile& file )
{
		
	int variantCount = 0;
	// save the player created variants
	for ( VARIANT_LIST::EIterator vIter = variants.Begin();
		!vIter.IsDone();  vIter++ )
		{
			if ( !(*vIter)->isDesignerMech() )
			{
				(*vIter)->save( file, variantCount );
				variantCount++;
			}
		}

	file.writeBlock( "Version" );
	file.writeIdLong( "VersionNumber", SaveGameVersionNumber);

	file.writeBlock( "General" );	
	
	file.writeIdLong( "VariantCount", variantCount );
	file.writeIdLong( "PilotCount", pilots.Count() );
	file.writeIdLong( "InventoryCount", inventory.Count() );
	file.writeIdBoolean( "FirstTimeResults", MissionResults::FirstTimeResults);

	// save the campaign info
	missionInfo->save( file );

	int pilotCount = 0;
	// save the pilots
	for ( PILOT_LIST::EIterator pIter = pilots.Begin();
		!pIter.IsDone(); pIter++ )
		{
			(*pIter).save( file, pilotCount++ );
		}

	int mechCount = 0;
	// save the inventory
	for ( MECH_LIST::EIterator mIter = inventory.Begin();
		!mIter.IsDone(); mIter++ )
		{
			(*mIter)->save( file, mechCount++ );
		}

	return 0;
}

void LogisticsData::clearVariants()
{
	for ( VARIANT_LIST::EIterator iter = variants.End(); !iter.IsDone();  )
	{
		if ( !(*iter)->isDesignerMech() )
		{
			VARIANT_LIST::EIterator tmpIter = iter;
			iter --;
			delete *tmpIter;
			variants.Delete( tmpIter );
	
		}
		else
			iter--;
	}
		
}

long	LogisticsData::load( FitIniFile& file )
{
	clearInventory();
	resourcePoints = 0;
	pilots.Clear();
	initPilots();
	clearVariants();

	if ( !missionInfo )
		missionInfo = new LogisticsMissionInfo;

	missionInfo->load( file );

	long result = file.seekBlock( "Version" );
	if (result != NO_ERR)
	{
		PAUSE(("SaveGame has no version number.  Not Loading"));
		return -1;
	}

	long testVersionNum = 0;
	result = file.readIdLong( "VersionNumber", testVersionNum);
	if (result != NO_ERR)
	{
		PAUSE(("SaveGame has no version number.  Not Loading"));
		return -1;
	}

	if (testVersionNum != SaveGameVersionNumber)
	{
		PAUSE(("SaveGame is not Version %d.  It was Version %d which is not valid!",SaveGameVersionNumber,testVersionNum));
		return -1;
	}

	file.seekBlock( "General" );	
	
	long variantCount, pilotCount, inventoryCount;
	variantCount = pilotCount = inventoryCount = 0;

	file.readIdLong( "VariantCount", variantCount );
	file.readIdLong( "PilotCount", pilotCount );
	file.readIdLong( "InventoryCount", inventoryCount );
	file.readIdBoolean( "FirstTimeResults", MissionResults::FirstTimeResults);

	char tmp[64];

	// load variants
	for ( int i = 0; i < variantCount; i++ )
	{
		sprintf( tmp, "Variant%ld", i );
		file.seekBlock( tmp );
		result = loadVariant( file );
		if ( result != NO_ERR )
		{
			gosASSERT( 0 );
			return -1;
		}
	}

	// load pilots
	for (int i = 0; i < pilotCount; i++ )
	{
		sprintf( tmp, "Pilot%ld", i );
		if ( NO_ERR != file.seekBlock( tmp ) )
		{
			gosASSERT( 0 );
		}

		file.readIdString( "FileName", tmp, 255 );

		// pilot should already exist
		for ( PILOT_LIST::EIterator pIter = pilots.Begin(); !pIter.IsDone(); pIter++ )
		{
			if ( (*pIter).getFileName().Compare( tmp, 0 ) == 0 )
			{
				(*pIter).load( file );
			}

			(*pIter).setAvailable( true );
		}
	}

	// load inventory
	int count = 0;
	for (int i = 0; i < inventoryCount; i++ )
	{
		sprintf( tmp, "Inventory%ld", i ) ;
		if ( NO_ERR != file.seekBlock( tmp ) )
		{
			gosASSERT( 0 );
		}

		loadMech( file, count );
	}

	updateAvailability();


	//Start finding the Leaks
	//systemHeap->dumpRecordLog();

	return 0;
}

long LogisticsData::loadVariant( FitIniFile& file )
{
	char tmp[256];
	
	file.readIdString( "Chassis", tmp, 255 );

	const LogisticsChassis* pChassis  = NULL;
	// go out and find that chassis
	for ( VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone(); vIter++ )
	{
		if ( (*vIter)->getFileName().Compare(  tmp, 0 ) == 0 )
		{
			pChassis = (*vIter)->getChassis();
		}
	}

	if ( !pChassis ) // we can always try and make it ourseleves, but it should have been loaded by now
	{
		gosASSERT( 0 );
		return INVALID_FILE_NAME;
	}

	// create the variant, add to the list
	file.readIdString( "VariantName", tmp, 255 );
	LogisticsVariant* pVariant = new LogisticsVariant( pChassis, 0 );	

	variants.Append( pVariant );
	
	pVariant->setName( tmp );

	long componentCount = 0;
	long x = 0;
	long y = 0;
	long location = 0;
	long id = 0;

	char tmp2[256];
	
	// read number of components
	file.readIdLong( "ComponentCount", componentCount );

	// add those components
	for ( int i = 0; i < componentCount; i++ )
	{
		sprintf( tmp, "Component%ld", i );
		file.readIdLong(tmp, id );
		
		strcpy( tmp2, tmp );
		strcat( tmp2, "x" );
		file.readIdLong( tmp2, x );

		strcpy( tmp2, tmp );
		strcat( tmp2, "y" );
		file.readIdLong( tmp2, y );

		strcpy( tmp2, tmp );
		strcat( tmp2, "Location" );
		file.readIdLong( tmp2, location );

		pVariant->addComponent( id, x, y );
	}

	return 0;
}

long LogisticsData::loadMech( FitIniFile& file, int& count )
{
	char tmp[256];
	file.readIdString( "Variant", tmp, 255 );
	for ( VARIANT_LIST::EIterator mIter = variants.Begin(); !mIter.IsDone(); mIter++ )
	{
		if ( (*mIter)->getName().Compare( tmp, 0 ) == 0 )
		{
			
			LogisticsMech* pMech = new LogisticsMech( (*mIter), count );
			file.readIdString( "Pilot", tmp, 255 );
			inventory.Append( pMech );

			for ( PILOT_LIST::EIterator pIter = pilots.Begin(); !pIter.IsDone(); pIter++ )
			{
				if ( (*pIter).getFileName().Compare( tmp, 0  ) == 0 )
				{
					pMech->setPilot( &(*pIter ) );
					count++;
					if ( count > -1 && count < 13 )
						pMech->setForceGroup( count );
					break;
				}
			}

			// it could have had no pilot
			return 0;
		}
	}

	return -1; // failed in finding the variant
}



void	LogisticsData::setMissionCompleted( )
{
#ifndef VIEWER
	const char* pMissionName = missionInfo->getCurrentMission();
	missionInfo->setMissionComplete();

	rpJustAdded = 0;

	// first set all pilots as not just dead
	for ( PILOT_LIST::EIterator iter = pilots.Begin(); !iter.IsDone();
		iter++ )
		{
			(*iter).setUsed( 0 );
		}

	for ( EList< CObjective*, CObjective* >::EIterator oIter =  Team::home->objectives.Begin();
			!oIter.IsDone(); oIter++ )
	{
		if ( (*oIter)->Status(Team::home->objectives) == OS_SUCCESSFUL )
		{
			addCBills( (*oIter)->ResourcePoints() );
		}
	}

	// need to go find out which pilots died.
	Team* pTeam = Team::home;

	int ForceGroupCount = 1;

	if ( pTeam )
	{
		for ( int i = pTeam->getRosterSize() - 1; i > -1; i-- )
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			
			//Must check if we ever linked up with the mech!!
			if ( pMover->isOnGUI() && 
				 (pMover->getObjectType()->getObjectTypeClass() == BATTLEMECH_TYPE) && 
				 (pMover->getCommanderId() == Commander::home->getId()) &&
				 (pMover->getMoveType() != MOVETYPE_AIR))
			{
				LogisticsMech* pMech = getMech( pMover->getName(), pMover->getPilot()->getName() );

				DWORD base, highlight1, highlight2;
				((Mech3DAppearance*)pMover->getAppearance())->getPaintScheme( base, highlight1, highlight2 );
				LogisticsPilot* pPilot = getPilot( pMover->getPilot()->getName() );
				if ( pMech )
				{
					if ( pMover->isDestroyed() || pMover->isDisabled() )
					{
						removeMechFromInventory( pMech->getName(), pMover->getPilot()->getName() );
					}
					else
					{
						removeMechFromInventory( pMech->getName(), pMover->getPilot()->getName() );
						LogisticsVariant* pVar = getVariant( ((BattleMech*)pMover)->variantName );
						addMechToInventory( pVar, ForceGroupCount++, pPilot, base, highlight1, highlight2 );
						if ( pPilot )
							pPilot->setUsed( true );

					}
				}
				else // mech was recovered during the mission
				{
					if ( !pMover->isDestroyed() && !pMover->isDisabled() )
					{
						// find the variant with this mech's info
					LogisticsVariant* pVariant = getVariant( ((BattleMech*)pMover)->variantName );
						if ( !pVariant )
						{
							Assert( 0, 0, "couldn't find the variant of a salvaged mech" );
						}
						else
						{
							addMechToInventory( pVariant, ForceGroupCount++, pPilot, base, highlight1, highlight2 );
							if ( pPilot )
								pPilot->setUsed( true );

						}
					}
				}

  				if ( pPilot )
				{
					pPilot->update( pMover->getPilot() );
//					if ( pMover->isDestroyed() || pMover->isDisabled() )
//						pPilot->setUsed( false );

				}
			}
		}
	}

#endif
}

LogisticsMech*  LogisticsData::getMech( const char* MechName, const char* pilotName )
{
	for( MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getName().Compare( MechName, 0 ) == 0 )
		{
			if ( !pilotName )
			{
				if ( !(*iter)->getPilot() )
					return (*iter);
			}

			else
			{
				if ( (*iter)->getPilot() && (*iter)->getPilot()->getName().Compare( pilotName, 0 ) == 0 )
					return (*iter);
			}
		}
	}

	return NULL;
}

void LogisticsData::removeMechFromInventory( const char* mechName, const char* pilotName )
{
	LogisticsMech* pMech = getMech( mechName, pilotName );

	gosASSERT( pMech );

	if ( pMech )
	{
		inventory.Delete( inventory.Find(pMech) );
		delete pMech;
	}
}

LogisticsPilot*	LogisticsData::getPilot( const char* pilotName )
{
	// look for available ones first
	for( PILOT_LIST::EIterator iter = pilots.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter).isAvailable() )
		{
			if ( (*iter).getName().Compare( pilotName, 0 ) == 0 )
			{
				return &(*iter);
			}
		}
	}
	
	for( PILOT_LIST::EIterator iter = pilots.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter).getName().Compare( pilotName, 0 ) == 0 )
		{
			return &(*iter);
		}
	}

	return NULL;
}

LogisticsVariant* LogisticsData::getVariant( const char* mechName )
{
	for ( VARIANT_LIST::EIterator iter = variants.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getName().Compare( mechName, 0 ) == 0 )
			return (*iter);
	}

	return NULL;
}

long LogisticsData::updateAvailability()
{
	bNewWeapons = 0;
	EString purchaseFileName = missionInfo->getCurrentPurchaseFile();
	purchaseFileName.MakeLower();

	if ( purchaseFileName.Length() < 1 )
	{
		purchaseFileName = missionPath;
		purchaseFileName += "purchase.fit";
	}

	int oldMechAvailableCount= 0;
	int newMechAvailableCount = 0;
	int oldPilotAvailableCount = 0;
	int newPilotAvailableCount= 0;
	for ( PILOT_LIST::EIterator pIter = pilots.Begin(); !pIter.IsDone(); pIter++ )
	{
		if ( (*pIter).bAvailable )
			oldPilotAvailableCount++;

		(*pIter).setAvailable( 0 );
	}


	// make sure its around and you can open it 
	FitIniFile file;
	if ( NO_ERR != file.open( (char*)(const char*)purchaseFileName ) )
	{
		EString error;
		error.Format( "Couldn't open %s", (char*)(const char*)purchaseFileName );
		PAUSE(((char*)(const char*)error ));
		return NO_PURCHASE_FILE;
	}
	// read in available components
	bool available[255];
	memset( available, 0, sizeof( bool ) * 255 );

	long result = file.seekBlock( "Components" );
	if ( result != NO_ERR )
	{
		Assert( 0, 0, "No components in the purchase file" );
	}

	char tmp[32];
	long component;

	bool bAll = 0;
	file.readIdBoolean( "AllComponents", bAll );
	for ( int i = 0; i < 255; i++ )
	{
		if ( bAll )
			available[i] = 1;
		else
		{
			sprintf( tmp, "Component%ld", i );
			if ( NO_ERR != file.readIdLong( tmp, component ) )
				break;

			available[component] = 1;
		}


	}

	// go through comonent list, and set 'em
	for ( COMPONENT_LIST::EIterator cIter = components.Begin(); !cIter.IsDone(); cIter++ )
	{
		if ( available[(*cIter).getID()] || bAll )
		{
			if ( !(*cIter).isAvailable() )
				bNewWeapons = true;
			(*cIter).setAvailable( 1 );
		}
	}

	const char* pFileNames[512];
	long count = 512;
 	missionInfo->getAdditionalPurachaseFiles( pFileNames, count );


	// reset all variants to unavailable
	for ( VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone(); vIter++ )
	{
		if ( (*vIter)->isAvailable()  && !((*vIter)->getID() >> 16 ))
			oldMechAvailableCount++;
		(*vIter)->setAvailable(0);
	}

	for (int i = 0; i < count; i++ )
	{
		appendAvailability( pFileNames[i], available );
	}

	// go through comonent list, and set 'em
	for (COMPONENT_LIST::EIterator cIter = components.Begin(); !cIter.IsDone(); cIter++ )
	{
		if ( !available[(*cIter).getID()]  )
		{
			(*cIter).setAvailable( 0 );
		}
	}



	// go through each variant, and see if it's available
	char chassisFileName[255];
	long componentArray[255];
	long componentCount;

	file.seekBlock( "Mechs" );
	for (int i = 0; i < 255; i++ )
	{
		sprintf( tmp, "Mech%ld", i );
		if ( NO_ERR != file.readIdString( tmp, chassisFileName, 254 ) )
			break;

		// go through each variant, if it has the same chassis, check and see if all of its components are valid
		for (VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone(); vIter++ )
		{
			EString mechName = (*vIter)->getFileName();
			char realName[1024];
			_splitpath( mechName, NULL, NULL, realName, NULL );
			if ( S_stricmp( realName, chassisFileName ) == 0 )
			{
				componentCount = 255;
				bool bRight = true;
				(*vIter)->getComponents( componentCount, componentArray );
				for ( int i = 0; i < componentCount; i++ )
				{
					if ( !available[componentArray[i]] ) // unavailable componets
					{
						//char errorStr[256];
						//sprintf( errorStr, "mech %s discarded because it contains a %ld", 
						//	chassisFileName, componentArray[i] );
						//PAUSE(( errorStr ));
						bRight= false;
						break;
					}
				}

				if ( bRight )
				{
					(*vIter)->setAvailable( true );

					if (  !((*vIter)->getID() >> 16 ) )
						newMechAvailableCount++;
				}
			}
		}
	}

	if ( newMechAvailableCount != oldMechAvailableCount )
		bNewMechs = true;
	else
		bNewMechs = false;

	// add new pilots
	char pilotName[255];
	file.seekBlock( "Pilots" );
	for (int i = 0; i < 255; i++ )
	{
		sprintf( tmp, "Pilot%ld", i );
		if ( NO_ERR != file.readIdString( tmp, pilotName, 254 ) )
			break;

		for ( PILOT_LIST::EIterator pIter = pilots.Begin(); !pIter.IsDone(); pIter++ )
		{
			if ( (*pIter).getFileName().Compare( pilotName, 0 ) == 0 )
			{
				(*pIter).setAvailable( true );
			}

		}
	}

	for (PILOT_LIST::EIterator pIter = pilots.Begin(); !pIter.IsDone(); pIter++ )
	{
		
		if ( (*pIter).bAvailable )
			newPilotAvailableCount++;
	}


	if ( oldPilotAvailableCount != newPilotAvailableCount && newPilotAvailableCount > oldPilotAvailableCount )
		bNewPilots = true;
	else
		bNewPilots = 0;



	return 0;

}

void LogisticsData::appendAvailability(const char* pFileName, bool* availableArray )
{
	FitIniFile file;
	if ( NO_ERR != file.open( pFileName ) )
	{
		return;
	}
	long result = file.seekBlock( "Components" );
	if ( result == NO_ERR )
	{

		char tmp[32];
		long component;

		bool bAll = 0;
		file.readIdBoolean( "AllComponents", bAll );
		for ( int i = 0; i < 255; i++ )
		{
			{
				sprintf( tmp, "Component%ld", i );
				if ( NO_ERR != file.readIdLong( tmp, component ) )
					break;

				availableArray[component] = true;

				LogisticsComponent* pComp = getComponent( component );
				if ( !pComp->isAvailable() )
					bNewWeapons = true;
				pComp->setAvailable( true );
			}
		}
	}

	// add new pilots
	char pilotName[255];
	char tmp[256];
	file.seekBlock( "Pilots" );
	for ( int i = 0; i < 255; i++ )
	{
		sprintf( tmp, "Pilot%ld", i );
		if ( NO_ERR != file.readIdString( tmp, pilotName, 254 ) )
			break;

		for ( PILOT_LIST::EIterator pIter = pilots.Begin(); !pIter.IsDone(); pIter++ )
		{
			if ( (*pIter).getFileName().Compare( pilotName, 0 ) == 0 )
			{
				(*pIter).setAvailable( true );
				bNewPilots = true;
				
			}

		}
	}

	file.seekBlock( "Mechs" );
	int newAvailableCount = 0;
	char chassisFileName[256];
	for (int i = 0; i < 255; i++ )
	{
		sprintf( tmp, "Mech%ld", i );
		if ( NO_ERR != file.readIdString( tmp, chassisFileName, 255 ) )
			break;

		// go through each variant, if it has the same chassis, check and see if all of its components are valid
		for ( VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone(); vIter++ )
		{
			EString mechName = (*vIter)->getFileName();
			char realName[255];
			_splitpath( mechName, NULL, NULL, realName, NULL );
			if ( S_stricmp( realName, chassisFileName ) == 0 )
			{
				long componentCount = 255;
				long componentArray[256];
				bool bRight = true;
				(*vIter)->getComponents( componentCount, componentArray );
				for ( int i = 0; i < componentCount; i++ )
				{
					LogisticsComponent* pComp = getComponent(componentArray[i]);
					if ( !pComp->isAvailable() ) // unavailable componets
					{
						//char errorStr[256];
						//sprintf( errorStr, "mech %s discarded because it contains a %ld", 
						//	chassisFileName, componentArray[i] );
						//PAUSE(( errorStr ));
						bRight= false;
						break;
					}
				}

				if ( bRight )
				{
					(*vIter)->setAvailable( true );
					newAvailableCount++;
				}

				break;
			}
		}
	}
}

const EString& LogisticsData::getCurrentMission() const
{
	return missionInfo->getCurrentMission(); 
}

const EString& LogisticsData::getLastMission() const
{
	return missionInfo->getLastMission(); 
}

const char * LogisticsData::getCurrentABLScript() const
{
	return missionInfo->getCurrentABLScriptName();
}

long LogisticsData::getCurrentMissionTune()
{
	return missionInfo->getCurrentLogisticsTuneId();
}

long LogisticsData::getCurrentMissionId()
{
	return missionInfo->getCurrentMissionId();
}

void LogisticsData::clearInventory()
{
	for (MECH_LIST::EIterator iter = inventory.Begin(); !iter.IsDone(); iter++ )
	{
		(*iter)->setPilot( NULL );
		delete *iter;
	}

	inventory.Clear();
}

int	LogisticsData::getPilotCount()
{
	return pilots.Count();
}
int	LogisticsData::getPilots( LogisticsPilot** pArray, long& count )
{
	if ( count < pilots.Count() )
	 {
		return NEED_BIGGER_ARRAY;
	 }	 

	 count= 0;

	 for ( PILOT_LIST::EIterator iter = instance->pilots.Begin(); !iter.IsDone(); iter++  )
	 {
		 pArray[count++] = &(*iter);
	 }

	 return 0;
}

int LogisticsData::getMaxDropWeight() const
{
	return  missionInfo->getCurrentDropWeight();
}

int LogisticsData::getCurrentDropWeight() const
{
	long retVal = 0;
	for ( MECH_LIST::EIterator iter = instance->inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getForceGroup() )
		{
			retVal += (*iter)->getMaxWeight();
		}

	}

	return retVal;

}

bool	LogisticsData::canAddMechToForceGroup( LogisticsMech* pMech )
{
	if ( !pMech )
		return 0;

	int maxUnits = 12;

#ifndef VIEWER
	if ( MPlayer )
	{
		long playerCount;
		MPlayer->getPlayers( playerCount );
		maxUnits = MAX_MULTIPLAYER_MECHS_IN_LOGISTICS/playerCount;

		if ( maxUnits > 12 )
			maxUnits = 12;
	}
#endif

	int fgCount = 0;
	
	for ( MECH_LIST::EIterator iter = instance->inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getForceGroup() )
			{
				fgCount ++;
			}


	}

	if ( fgCount >= maxUnits )
		return 0;


	return (pMech->getMaxWeight() + getCurrentDropWeight() <= getMaxDropWeight() ) ? 1 : 0;
}


int LogisticsData::getVariantsInInventory( LogisticsVariant* pVar, bool bIncludeForceGroup )
{
	long retVal = 0;
	for ( MECH_LIST::EIterator iter = instance->inventory.Begin(); !iter.IsDone(); iter++ )
	{
		if ( (*iter)->getVariant() == pVar )
		{
			if ( !(*iter)->getForceGroup() || bIncludeForceGroup )
			{
				retVal ++;
			}
		}

	}

	return retVal;

}

int		LogisticsData::getChassisVariants( const LogisticsChassis* pChassis, 
										  LogisticsVariant** pVar, 
										  int& maxCount )
{
	int retVal = 0;
	
	int i = 0;
	for ( VARIANT_LIST::EIterator iter = variants.Begin(); 
		!iter.IsDone(); iter++ )
		{
			if ( (*iter)->getChassis() == pChassis  )
			{
				if ( i < maxCount )
					pVar[i]	= (*iter);	
				else 
					retVal = NEED_BIGGER_ARRAY;
				++i;

				
			} 
		}

	maxCount = i;

	return retVal; 
}


int LogisticsData::setMechToModify( LogisticsMech* pMech )
{
	if ( !pMech )
		return -1;

	currentlyModifiedMech = pMech;
	oldVariant = pMech->getVariant();

	LogisticsVariant* pVar = new LogisticsVariant( *oldVariant );
	pMech->setVariant( pVar );

	return 0;
}

void encryptFile (const char *inputFile, const char* outputFile)
{
	//Now we encrypt this by zlib Compressing the file passed in.
	// Then LZ Compressing the resulting zlib data.
	// Since our LZ compression is pretty much non-standard, that should be enough.
	MemoryPtr rawData = NULL;
	MemoryPtr zlibData = NULL;
	MemoryPtr LZData = NULL;

	File dataFile;
	dataFile.open(inputFile);
	DWORD fileSize = dataFile.fileSize();
	rawData = (MemoryPtr)malloc(fileSize);
	zlibData = (MemoryPtr)malloc(fileSize*2);
	LZData = (MemoryPtr)malloc(fileSize*2);

	dataFile.read(rawData,fileSize);

	unsigned long int zlibSize = fileSize * 2;
	compress2(zlibData,&zlibSize,rawData,fileSize,0);
	size_t lzSize = LZCompress (LZData, zlibData, zlibSize);

	dataFile.close();

	File binFile;
	binFile.create(outputFile);
	binFile.writeLong(lzSize);
	binFile.writeLong(zlibSize);
	binFile.writeLong(fileSize);
	binFile.write(LZData,lzSize);
	binFile.close();

	free(rawData);
	free(zlibData);
	free(LZData);
}

void decryptFile (char *inputFile, char *outputFile)
{
	//Now we decrypt this by lz deCompressing the zlib file created.
	// Then zlib deCompressing the resulting zlib data into the raw File again.
	// Since our LZ compression is pretty much non-standard, that should be enough.
	MemoryPtr rawData = NULL;
	MemoryPtr zlibData = NULL;
	MemoryPtr LZData = NULL;

	File dataFile;
	long result = dataFile.open(inputFile);
	if (result == NO_ERR) 
	{
		long lzSize = dataFile.readLong();
		long zlibSize = dataFile.readLong();
		long fileSize = dataFile.readLong();
	
		rawData = (MemoryPtr)malloc(fileSize);
		zlibData = (MemoryPtr)malloc(zlibSize);
		LZData = (MemoryPtr)malloc(lzSize);
	
		dataFile.read(LZData,lzSize);
	
		unsigned long testSize = fileSize;
		size_t test2Size = LZDecomp(zlibData, LZData, lzSize);
		if (test2Size != zlibSize) 
			STOP(("Didn't Decompress to same size as started with!!"));
	
		uncompress((MemoryPtr)rawData,&testSize,zlibData,zlibSize);
		if (testSize != fileSize) 
			STOP(("Didn't Decompress to correct format"));
	
		dataFile.close();
	
		File binFile;
		binFile.create(outputFile);
		binFile.write(rawData,fileSize);
		binFile.close();
	
		free(rawData);
		free(zlibData);
		free(LZData);
	}
}

int LogisticsData::acceptMechModifications( const char* name )
{
	if ( !currentlyModifiedMech )
		return -1;

	bool bFound = 0;
	if ( oldVariant )
	{
		missionInfo->incrementCBills( oldVariant->getCost() );
	}
	if ( oldVariant && oldVariant->isDesignerMech() )
	{
		bFound = 1;
	}
	else
	{
		for ( MECH_LIST::EIterator iter = inventory.Begin();
			!iter.IsDone(); iter++ )
		{
			if ( (*iter)->getVariant() == oldVariant && (*iter) != currentlyModifiedMech  )
			{
				bFound = 1;
			}
		}
	}

	if ( !bFound )
	{
		VARIANT_LIST::EIterator vIter = variants.Find( oldVariant );
		if ( vIter != VARIANT_LIST::INVALID_ITERATOR
			&& oldVariant->getName().Compare( name ) == 0 )
		{
			variants.Delete( vIter );
			delete oldVariant;
		}
	}

	// now need to get rid of variants with this name....

	//Code added by Frank on  May 3, 2001 @ 9:54pm.
	// If you run this with the compare set to vIter.IsDone, it crashes in Profile
	// based on Sean Bug number 4359.  We traverse past end of list and crash.
	// Doing it by count does not crash and has the added advantage of being easy to debug!
	// I suspect ESI going south again.  Probably a compiler option...

	// 05/04 HKG, actually, if you increment vIter after deleteing it, it still won't work

	// Good Point.  As you can see, it was pretty late when I "fixed" this!
	long numVariants = variants.Count();
	long i=0;
	for ( VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone();  )
	{
		if ( (*vIter)->getName().Compare( name, 0 ) == 0 )
		{
			VARIANT_LIST::EIterator tmpIter = vIter;
			vIter++;
			delete (*tmpIter);
			variants.Delete( tmpIter );
		}
		else
			vIter++;
	}

	currentlyModifiedMech->getVariant()->setName( name );
	variants.Append( currentlyModifiedMech->getVariant() );
	missionInfo->decrementCBills( currentlyModifiedMech->getVariant()->getCost() );

	currentlyModifiedMech = 0;
	oldVariant = 0;


	// temporary, looking for dangling pointers
	for ( MECH_LIST::EIterator iter = inventory.Begin();
		!iter.IsDone(); iter++ )
	{
		if ( (*iter)->getVariant()->getCost() )
		{
			bFound = 1;
		}
	}
 
#ifndef VIEWER

	if ( MPlayer )
	{
		// save the player created variants
		for ( VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone();  vIter++ )
		{
			if ( !(*vIter)->isDesignerMech() )
			{
				FullPathFileName mechFile;
				mechFile.init("data" PATH_SEPARATOR "multiplayer" PATH_SEPARATOR ,(*vIter)->getName(),".var");

				FitIniFile file;
				file.create(mechFile);
				(*vIter)->save( file, 0 );
				file.close();

                const char* filename = (const char*)mechFile;
				encryptFile(filename, filename);
			}
		}
	}

#endif

	return 0;
}
int LogisticsData::acceptMechModificationsUseOldVariant( const char* name )
{
	if ( !currentlyModifiedMech )
		return -1;

	if ( oldVariant )
	{
		missionInfo->incrementCBills( oldVariant->getCost() );
	}

	LogisticsVariant* pVar = getVariant( name );

	if ( !pVar )
		Assert( 0, 0, "couldn't find the old variant\n" );

	LogisticsVariant* pOldVar = currentlyModifiedMech->getVariant();
	delete pOldVar;

	currentlyModifiedMech->setVariant( pVar );
	missionInfo->decrementCBills( pVar->getCost() );
	
	currentlyModifiedMech = 0;
	oldVariant = 0;	

	return 0;
}

bool LogisticsData::canReplaceVariant( const char* name )
{
	int nameCount = 0;
	for ( MECH_LIST::EIterator iter = inventory.Begin();
		!iter.IsDone(); iter++ )
		{
			if ( (*iter)->getName().Compare( name, 0 ) == 0 )
			{
				nameCount++;
				if ( (*iter)->getVariant() != oldVariant && (*iter) != currentlyModifiedMech )
				{
					return 0;
				}
			}
		}

		if ( nameCount > 1 )
			return 0;

		for ( VARIANT_LIST::EIterator vIter = variants.Begin();
		!vIter.IsDone(); vIter++ )
		{
			if ( (*vIter)->isDesignerMech() && (*vIter)->getName().Compare( name, 0 ) == 0 )
				return 0;
		}

		return true;
}

bool	LogisticsData::canDeleteVariant( const char* name )
{
	LogisticsVariant* pVariant = getVariant( name );
	if ( !pVariant )
		return 0;
		
	if ( !canReplaceVariant(name) )
		return 0;

	if ( currentlyModifiedMech->getName() == name || oldVariant->getName() == name )
		return 0;

	return 1;

}

int LogisticsData::cancelMechModfications()
{
	if ( !currentlyModifiedMech )
		return -1;

	LogisticsVariant* pCancel = currentlyModifiedMech->getVariant();

	delete pCancel;

	currentlyModifiedMech->setVariant( oldVariant );

	oldVariant = 0;
	currentlyModifiedMech = 0;

	return 0;
}

const char*			LogisticsData::getCurrentOperationFileName()
{
	return missionInfo->getCurrentOperationFile();
}
const char*			LogisticsData::getCurrentVideoFileName()
{
	return missionInfo->getCurrentVideo();
}

const char*			LogisticsData::getCurrentMissionDescription()
{
	return missionInfo->getCurrentMissionDescription();
}


const char*				LogisticsData::getCurrentMissionFriendlyName( )
{
	return missionInfo->getCurrentMissionFriendlyName();
}

const char*				LogisticsData::getMissionFriendlyName( const char* missionName )
{
	return missionInfo->getMissionFriendlyName( missionName );
}

/*long				LogisticsData::getMaxTeams() const
{
/	return missionInfo->getMaxTeams( );
}*/


void				LogisticsData::startNewCampaign( const char* fileName )
{
#ifndef VIEWER
	if ( MPlayer )
	{
		delete MPlayer;
		MPlayer = NULL;
	}
#endif

	inventory.Clear();
	resourcePoints = 0;
	pilots.Clear();
	initPilots();

	FitIniFile file;

	FullPathFileName path;
	path.init( campaignPath, fileName, ".fit" );
	if ( NO_ERR != file.open( path ) )
	{
		STOP(("COuld not find file %s to load campaign",path));
	}
	missionInfo->init( file );

	// temporary, just so we can test
	int count = 32;
	const char* missionNames[32];
	missionInfo->getAvailableMissions( missionNames, count );

	setCurrentMission( missionNames[0] );

	soundSystem->setMusicVolume( prefs.MusicVolume );
	soundSystem->playDigitalMusic(missionInfo->getCurrentLogisticsTuneId());
}

void LogisticsData::startMultiPlayer()
{
	inventory.Clear();
	resourcePoints = 0;
	pilots.Clear();
	initPilots();

	// kill all old designer mechs
	clearVariants();

	// need to initialize multiplayer variants here...
	char findString[512];
	sprintf(findString,"data" PATH_SEPARATOR "multiplayer" PATH_SEPARATOR "*.var");


#ifdef LINUX_BUILD
    // sebi !NB
    ENTER_DEBUGGER
#else
	WIN32_FIND_DATA	findResult;
	HANDLE searchHandle = FindFirstFile(findString,&findResult); 
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((findResult.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				FullPathFileName path;
				path.init("data" PATH_SEPARATOR "multiplayer" PATH_SEPARATOR, findResult.cFileName,"");
				decryptFile(path,"tmp.fit");

				FitIniFile file;
				file.open("tmp.fit");
				file.seekBlock( "Variant0" );

				loadVariant(file);
				file.close();

				DeleteFile("tmp.fit");
			}
		} while (FindNextFile(searchHandle,&findResult) != 0);

		FindClose(searchHandle);
	}
#endif

	missionInfo->setMultiplayer();

#ifndef VIEWER
	if ( !MPlayer )
	{
		MPlayer = new MultiPlayer;
		MPlayer->setup();

		if ( !strlen( &prefs.playerName[0][0] ) )
		{
			cLoadString( IDS_UNNAMED_PLAYER, &prefs.playerName[0][0], 255 );
		}

		ChatWindow::init();
	}
#endif

}
void				LogisticsData::setPurchaseFile( const char* fileName )
{
	missionInfo->setPurchaseFile( fileName );
	if ( MPlayer )
		clearInventory();
	updateAvailability();

}


int					LogisticsData::getCBills() 
{ 
	return missionInfo->getCBills(); 
}
void				LogisticsData::addCBills( int amount )
{ 
	missionInfo->incrementCBills(amount); 
}
void				LogisticsData::decrementCBills( int amount ) 
{ 
	missionInfo->decrementCBills(amount); 
}

int					LogisticsData::getPlayerVariantNames( const char** array, int& count )
{
	int maxCount = count;
	count = 0;

	int retVal = 0;
	for ( VARIANT_LIST::EIterator iter = variants.Begin(); 
		!iter.IsDone(); iter++ )
		{
			if ( !(*iter)->isDesignerMech() )
			{
				if ( count < maxCount )
				{
					array[count] = (*iter)->getName();
				}
				else
				{
					retVal = NEED_BIGGER_ARRAY;
				}
				count++;
			}
		}

		return retVal;
}

int		LogisticsData::getEncyclopediaMechs( const LogisticsVariant** pChassis, int& count )
{
	int retVal = 0;
	int maxCount = count;
	count = 0;
	for ( VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone(); vIter++ )
	{
		if ( (*vIter)->getName().Find( "Prime" ) != -1  && (*vIter)->isDesignerMech() )
		{
			if ( count < maxCount )
				pChassis[count] = (*vIter);
			else 
				retVal = NEED_BIGGER_ARRAY;

			count++;

		}

	}

	return retVal;
}

int	LogisticsData::getHelicopters( const LogisticsVariant** pChassis, int& count )
{
	int retVal = 0;
	int maxCount = count;
	count = 0;
	for ( VARIANT_LIST::EIterator vIter = variants.Begin(); !vIter.IsDone(); vIter++ )
	{
		if ( (((*vIter)->getVariantID() >> 16 & 0xff) == 0) && 
			(*vIter)->getName().Find( "Prime" ) == -1  && (*vIter)->isDesignerMech() )
		{
			if ( count < maxCount )
				pChassis[count] = (*vIter);
			else 
				retVal = NEED_BIGGER_ARRAY;

			count++;

		}

	}

	return retVal;
}


int		LogisticsData::getVehicles( const LogisticsVehicle** pChassis, int& count )
{
	int retVal = 0;
	int maxCount = count;
	count = 0;
	for ( VEHICLE_LIST::EIterator vIter = vehicles.Begin(); !vIter.IsDone(); vIter++ )
	{
			if ( count < maxCount )
				pChassis[count] = (*vIter);
			else 
				retVal = NEED_BIGGER_ARRAY;

			count++;

		

	}

	return retVal;
}


LogisticsVehicle*	LogisticsData::getVehicle( const char* pName )
{
	char tmpStr[256];
	for ( VEHICLE_LIST::EIterator vIter = vehicles.Begin(); !vIter.IsDone(); vIter++ )
	{
		cLoadString( (*vIter)->getNameID(), tmpStr, 255 );
		if ( S_stricmp( tmpStr, pName ) == 0 )
		{
			return *vIter;
		}
	}

	return NULL;
}

int LogisticsData::addBuilding( long fitID, PacketFile& objectFile, float scale )
{
	if ( NO_ERR != objectFile.seekPacket(fitID) )
		return -1;

	int fileSize = objectFile.getPacketSize();

	if ( fileSize )
	{
		Building bldg;

		FitIniFile file;
		file.open(&objectFile, fileSize);
		if ( NO_ERR != file.seekBlock( "ObjectType" ) )
			gosASSERT( 0 );

		file.readIdString( "AppearanceName", bldg.fileName, 63 );
		file.readIdLong( "EncyclopediaID", bldg.encycloID );


		bool bIsTurret = 0;

		if ( NO_ERR != file.seekBlock( "BuildingData" ) )
		{
			if ( NO_ERR != file.seekBlock( "GateData" ) )
			{
				if ( NO_ERR != file.seekBlock( "TurretData" ) )
				{
					if ( NO_ERR != file.seekBlock( "General" ) ) // hack for artillery piece
					{
						char errorStr[256];
						sprintf( errorStr, "coudn't find appropriate block in file %s", bldg.fileName );
						Assert( 0, 0, errorStr  );
					}
				}
				else
					bIsTurret = true;
			}
		}
		unsigned long tmp;
		file.readIdLong( "BuildingName", bldg.nameID );
		file.readIdULong( "DmgLevel", tmp );
		bldg.weight = tmp;
		if ( bIsTurret )
		{
			char weaponNameStr[64];
			strcpy( weaponNameStr, "WeaponType" );
			for ( int i = 0; i < 4; i++ )
			{
				
				file.readIdLong( weaponNameStr, bldg.componentIDs[i] );
				sprintf( weaponNameStr, "WeaponType%ld", i+1 );
			}
			
		}
		else
		{
			for ( int i = 0; i < 4; i++ )
			{
				bldg.componentIDs[i] = 0;
			}
		}
		
		bldg.scale = scale;
		buildings.Append( bldg );

		
	}

	return 0;
}

//*************************************************************************************************
LogisticsComponent* LogisticsData::getComponent( int componentID )
{
	for ( COMPONENT_LIST::EIterator iter = components.Begin();
		!iter.IsDone(); iter++ )
	{
			if ( ((*iter).getID() & 0x000000ff) == (componentID & 0x000000ff) )
				return &(*iter);
	}

	return NULL;
}

//*************************************************************************************************
LogisticsData::Building*			LogisticsData::getBuilding( int nameID )
{
	for ( BUILDING_LIST::EIterator iter = buildings.Begin();
		!iter.IsDone(); iter++ )
		{
			if ( (*iter).nameID == nameID )
				return &(*iter);
		}

	return NULL;
}


//*************************************************************************************************
int					LogisticsData::getBuildings( Building** bdgs, int& count )
{
	int maxCount = count;
	count = 0;
	int retVal = 0;

	for ( BUILDING_LIST::EIterator iter = buildings.Begin();
		!iter.IsDone(); iter++ )
	{
		if ( count < maxCount )
		{
			bdgs[count] = &(*iter);
		}
		else
			retVal = NEED_BIGGER_ARRAY;

		count++;
	}


	return retVal;
	
}

const EString&	LogisticsData::getCampaignName() const
{ 
	return missionInfo->getCampaignName();
}



bool				LogisticsData::campaignOver() 
{ 
	return missionInfo->campaignOver();
}
const char*			LogisticsData::getCurrentBigVideo() const 
{ 
	return missionInfo->getCurrentBigVideo(); 
}
const char*			LogisticsData::getFinalVideo() const
{ 
	return missionInfo->getFinalVideo();
}

void				LogisticsData::addNewBonusPurchaseFile( const char* pFileName )
{
	missionInfo->addBonusPurchaseFile( pFileName );
}

bool				LogisticsData::skipLogistics()
{
	return missionInfo->skipLogistics();
}
bool				LogisticsData::skipPilotReview()
{
	return missionInfo->skipPilotReview();
}
bool				LogisticsData::skipSalvageScreen()
{
	return missionInfo->skipSalvageScreen();
}
bool				LogisticsData::skipPurchasing()
{
	return missionInfo->skipPurchasing();
}

bool				LogisticsData::showChooseMission()
{
	return missionInfo->showChooseMission();
}

void	LogisticsData::setSingleMission( const char* pName )
{
	missionInfo->setSingleMission( pName );
	clearVariants();
	initPilots(); // reset pilotsb
	clearInventory();
	updateAvailability();
}

bool	LogisticsData::isSingleMission()
{
	if ( missionInfo )
	{
		return missionInfo->isSingleMission();
	}
	
	return 0;
}

bool LogisticsData::canHaveSalavageCraft()
{
	if ( !missionInfo )
		return true;
		
	return missionInfo->canHaveSalavageCraft();
}
bool LogisticsData::canHaveRepairTruck()
{
	if ( !missionInfo )
		return true;
	return missionInfo->canHaveRepairTruck();
}
bool LogisticsData::canHaveScoutCopter()
{
	if ( !missionInfo )
		return true;
	return missionInfo->canHaveScoutCopter();
}
bool LogisticsData::canHaveArtilleryPiece()
{
	if ( !missionInfo )
		return true;
	return missionInfo->canHaveArtilleryPiece();
}
bool LogisticsData::canHaveAirStrike()
{
	if ( !missionInfo )
		return true;
	return missionInfo->canHaveAirStrike();
}
bool LogisticsData::canHaveSensorStrike()
{
	if ( !missionInfo )
		return true;
	return missionInfo->canHaveSensorStrike();
}
bool LogisticsData::canHaveMineLayer()
{
	if ( !missionInfo )
		return true;
	return missionInfo->canHaveMineLayer();
}

bool				LogisticsData::getVideoShown()
{
	if ( !missionInfo )
		return true;
	return missionInfo->getVideoShown();
}
void				LogisticsData::setVideoShown()
{
	if ( missionInfo )
		missionInfo->setVideoShown();

}
void	LogisticsData::setPilotUnused( const char* pName )
{
	for ( PILOT_LIST::EIterator iter = pilots.Begin(); !iter.IsDone(); iter++  )
	{
		if ( (*iter).getFileName().Compare( pName, 0 ) == 0 )
		{
			(*iter).setUsed( 0 );
			break;
		}
	}


}

void LogisticsData::setCurrentMissionNum (long cMission)
{
	missionInfo->setCurrentMissionNumber(cMission);
}

long LogisticsData::getCurrentMissionNum (void)
{
	return missionInfo->getCurrentMissionNumber();
}

//*************************************************************************************************
// end of file ( LogisticsData.cpp )
