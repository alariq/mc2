#define MECHLOPEDIA_CPP
/*************************************************************************************************\
Mechlopedia.cpp			: Implementation of the Mechlopedia component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mechlopedia.h"
#include"inifile.h"
#include"mclib.h"
#include"logisticsdata.h"
#include"../resource.h"
#include"prefs.h"
#include"cmponent.h"

#define ENCYCLO_MECHS	130
#define ENCYCLO_BUILD	131
#define ENCYCLO_VEHIC	132
#define ENCYCLO_WEAPONS 133
#define ENCYCLO_PILOTS	134
#define ENCYCLO_HISTORY 135
#define ENCYCLO_MM	136

#define PERSONALITY_COUNT 11

MechlopediaListItem* MechlopediaListItem::s_templateItem = NULL;				




Mechlopedia::Mechlopedia(  )
{
	helpTextArrayID = 1;

	for ( int i = 0; i < 6; i++ )
	{
		subScreens[i] = 0;
	}

	currentScreen = 0;
}

//-------------------------------------------------------------------------------------------------

Mechlopedia::~Mechlopedia()
{
}



int Mechlopedia::init()
{
	FullPathFileName path;
	path.init( artPath, "mcl_en", ".fit" );

	FitIniFile file;
	if ( NO_ERR != file.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, errorStr );
		return 0;
	}

	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button" );

	buttons[buttonCount-1].setMessageOnRelease();

	listBox.init( rects[0].left(), rects[0].top(), rects[0].width(), rects[0].height() );
	listBox.setOrange(true);

	Mechlopedia::MechScreen* pMechScreen = new Mechlopedia::MechScreen;
	pMechScreen->setListBox( &listBox );
	pMechScreen->setVehicle( false );
	pMechScreen->init();
	subScreens[0] = pMechScreen;
	
	Mechlopedia::BuildingScreen* pBuildingScreen = new Mechlopedia::BuildingScreen;
	pBuildingScreen->setListBox( &listBox );
	pBuildingScreen->init();
	subScreens[1] = pBuildingScreen;
	
	Mechlopedia::WeaponScreen* pWeaponScreen = new Mechlopedia::WeaponScreen;
	pWeaponScreen->setListBox( &listBox );
	pWeaponScreen->init();
	subScreens[3] = pWeaponScreen;

	pMechScreen = new Mechlopedia::MechScreen;
	pMechScreen->setVehicle( true );
	pMechScreen->setListBox( &listBox );
	pMechScreen->init();
	subScreens[2] = pMechScreen;

	Mechlopedia::PersonalityScreen* pPersScreen = new Mechlopedia::PersonalityScreen;
	pPersScreen->setListBox( &listBox );
	pPersScreen->init();
	pPersScreen->setIsHistory( false );
	subScreens[4] = pPersScreen;

	Mechlopedia::PersonalityScreen* pHistroyScreen = new Mechlopedia::PersonalityScreen;
	pHistroyScreen->setListBox( &listBox );
	pHistroyScreen->init();
	pHistroyScreen->setIsHistory( true );
	subScreens[5] = pHistroyScreen;

	
	MechlopediaListItem::init();

	for ( int i = ENCYCLO_MECHS; i < ENCYCLO_MM; i++ )
	{
		getButton( i )->setPressFX( LOG_VIDEOBUTTONS );
		getButton( i )->setHighlightFX( LOG_DIGITALHIGHLIGHT );
		getButton( i )->setDisabledFX( -1 );
	}

	return true;
}

int			Mechlopedia::handleMessage( unsigned long, unsigned long who)
{
	// unpress all the others
	for ( int i = ENCYCLO_MECHS; i < ENCYCLO_MM; i++ )
		getButton( i )->press( 0 );

	if ( getButton( who ) )
		getButton( who )->press( true );

	if ( who < ENCYCLO_MM )
	{
		if ( subScreens[currentScreen] )
			subScreens[currentScreen]->end();

		currentScreen = who - ENCYCLO_MECHS;
		if ( subScreens[currentScreen] )
			subScreens[currentScreen]->begin();
			
	}

	else 
	{
		for ( int i= 0;i < 6; i++ )
		{
			subScreens[i]->end();
		}
		beginFadeOut( .5f );
		status = NEXT;
	}

	return 1;
}

void Mechlopedia::update()
{
	LogisticsScreen::update();

	if ( subScreens[currentScreen] )
		subScreens[currentScreen]->update();
}

void Mechlopedia::render()
{
	GUI_RECT rect = { 0, 0, Environment.screenWidth, Environment.screenHeight };
	drawRect( rect, 0xff000000 );

	if ( subScreens[currentScreen] )
		subScreens[currentScreen]->render();
	
	LogisticsScreen::render();	
}


//////////////////////////////////////////////////////////


int Mechlopedia::SubScreen::init( FitIniFile& file )
{
	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button" );

	for ( int i = 0; i < buttonCount; i++ )
		buttons[i].setMessageOnRelease();

	descriptionListBox.init( rects[0].left(), rects[0].top(), rects[0].width(), rects[0].height() );
	descriptionListBox.move( 285, 58 );
	descriptionListBox.setDisabledFX( -1 );
	descriptionListBox.setHighlightFX( -1 );
	descriptionListBox.setPressFX( -1 );

	return 0;

}

void Mechlopedia::SubScreen::end(  )
{
	camera.setMech( NULL );
}


void Mechlopedia::SubScreen::update()
{
	int mouseX = userInput->getMouseX();
	int mouseY = userInput->getMouseY();

	// check for new selection....
	groupListBox->update();
	if ( groupListBox->pointInside( mouseX, mouseY) && userInput->isLeftClick() )
	{
		int index = groupListBox->GetSelectedItem();
		if ( index != -1 )
		{
		//	for ( int i = 0; i < groupListBox->GetItemCount(); i++ )
		//	{
		//		groupListBox->GetItem( i )->setColor( 0xff43311C );
		//	}

			aTextListItem* pItem = (aTextListItem*)groupListBox->GetItem( index );
		//	pItem->setColor( 0xff866234 );
			select( pItem );
		
		}
	}

	descriptionListBox.update();
	camera.update();

}

void Mechlopedia::begin()
{
	beginFadeIn( 2.0f );
	status = RUNNING;

	if ( !currentScreen )
	{
		getButton( ENCYCLO_MECHS )->press( true );
		handleMessage( 0, ENCYCLO_MECHS );

		listBox.setScrollPos( 0 );
	}
}

//////////////////////////////////////////////////////////
void Mechlopedia::MechScreen::init()
{
	FullPathFileName path;
	path.init( artPath, "mcl_en_mechs", ".fit" );

	FitIniFile file;
	if ( NO_ERR != file.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, errorStr );
		return;
	}

	SubScreen::init( file );

	compListBox.init( rects[2].left(), rects[2].top(), rects[2].width(), rects[2].height() );
	compListBox.move( 285, 58 );
	statsListBox.init( rects[1].left(), rects[1].top(), rects[1].width(), rects[1].height() );
	statsListBox.move( 285, 58 );

	statsListBox.setDisabledFX( -1 );
	statsListBox.setHighlightFX( -1 );
	statsListBox.setPressFX( -1 );

	camera.init( statics[4].left() + 285, statics[4].top() + 58, statics[4].right() + 285, statics[4].bottom() + 58 );
	statics[4].setColor( 0 );

	textObjects[0].setText( "" );

}

void Mechlopedia::MechScreen::begin()
{
	// need to fill that list box
	if ( bIsVehicle )
	{
		textObjects[1].setText( IDS_VEHICLE_STATS );
		int count = 256;
		const LogisticsVehicle* pVehicles[256];
		LogisticsData::instance->getVehicles( pVehicles, count );
		
		const LogisticsVariant* pCopters[256];
		int copterCount = 256;
		LogisticsData::instance->getHelicopters( pCopters, copterCount );

		for ( int i = 1; i < count; ++i )
		{
			const LogisticsVehicle* cur = pVehicles[i];
			for ( int j = 0; j < i; ++j )
			{
				if ( cur->getNameID() == pVehicles[j]->getNameID() && j != i )
				{
					pVehicles[i] = pVehicles[j];
					pVehicles[j] = cur;
					break;
				}
			}
		}

		

		

		groupListBox->removeAllItems( true );
		for (int i = 0; i < count; i++ )
		{
			MechlopediaListItem* pEntry = new MechlopediaListItem();
			char name[256];
			cLoadString( pVehicles[i]->getNameID(), name, 255 );
			EString text = name;
			text.MakeUpper();
			pEntry->setText( text );
			pEntry->resize( groupListBox->width() - groupListBox->getScrollBarWidth() - 18, pEntry->height() );
			bool bFound = 0;

			for ( int j = 0; j < groupListBox->GetItemCount(); j++ )
			{
				aTextListItem* pItem = (aTextListItem*)groupListBox->GetItem( j );
				if ( stricmp( name, pItem->getText() ) < 0 )
				{
					groupListBox->InsertItem( pEntry, j );
					bFound = true;
					break;
				}
			}
			if ( !bFound )
				groupListBox->AddItem( pEntry );
		}
		for (int i = 0; i < copterCount; i++ )
		{
			MechlopediaListItem* pEntry = new MechlopediaListItem();
			EString text = pCopters[i]->getName();
			text.MakeUpper();
			pEntry->setText( text );
			pEntry->resize( groupListBox->width() - groupListBox->getScrollBarWidth() - 18,
				pEntry->height() );

			bool bFound = 0;
			for ( int j = 0; j < groupListBox->GetItemCount(); j++ )
			{
				aTextListItem* pItem = (aTextListItem*)groupListBox->GetItem( j );
				if ( stricmp( pCopters[i]->getName(), pItem->getText() ) < 0 )
				{
					groupListBox->InsertItem( pEntry, j );
					bFound = true;
					break;
				}
			}
			if ( !bFound )
				groupListBox->AddItem( pEntry );

		}
	}
	else
	{
		textObjects[1].setText( IDS_MECH_STATS );
		int count = 256;
		const LogisticsVariant* pChassis[256];
		LogisticsData::instance->getEncyclopediaMechs( pChassis, count );

		for ( int i = 1; i < count; ++i )
		{
			const LogisticsVariant* cur = pChassis[i];
			for ( int j = 0; j <= i; ++j )
			{
				if ( stricmp( cur->getName(), pChassis[j]->getName() ) < 0  )
				{
					for ( int l = i-1; l >= j; l-- )
					{
						pChassis[l+1] = pChassis[l];
					}
					pChassis[j] = cur;

					break;
				}
			}
		}

		groupListBox->removeAllItems( true );
		for (int i = 0; i < count; i++ )
		{
			MechlopediaListItem* pEntry = new MechlopediaListItem();
			pEntry->setText( pChassis[i]->getChassisName() );
			pEntry->resize( groupListBox->width() - groupListBox->getScrollBarWidth() - 18,
				pEntry->height() );
			groupListBox->AddItem( pEntry );
		}
	}

	aTextListItem* pEntry = (aTextListItem*)groupListBox->GetItem( 0 );
	if ( pEntry )
	{
		select( pEntry );
		groupListBox->SelectItem( 0 );
		pEntry->setColor( 0xff866234 );
	}

	groupListBox->setScrollPos( 0 );
} 

void Mechlopedia::MechScreen::update()
{
	SubScreen::update();

	compListBox.update();
	statsListBox.update();

}

void Mechlopedia::MechScreen::select( aTextListItem* pItem )
{
	const char* pText = pItem->getText();


	if ( !bIsVehicle )
	{
		EString name = pText;
		name += " Prime";

		LogisticsVariant* pChassis  = LogisticsData::instance->getVariant( name );
		setMech( pChassis, 1 );
	}
	else
	{
		LogisticsVehicle* pVehicle = LogisticsData::instance->getVehicle( pText );
		if ( pVehicle )
			setVehicle( pVehicle );
		else // copter
		{
			LogisticsVariant* pMech = LogisticsData::instance->getVariant( pText );
			if ( pMech )
				setMech( pMech, 0 );
		}
	}
}
void Mechlopedia::MechScreen::render()
{
	groupListBox->render();
	descriptionListBox.render();
	camera.render();
	statsListBox.render();
	compListBox.render();

	LogisticsScreen::render(285, 58);
}

void Mechlopedia::MechScreen::setVehicle( LogisticsVehicle* pVehicle )
{
	if ( !pVehicle )
		return;

	int descID = pVehicle->getEncyclopediaID();
	char text[256];
	cLoadString( pVehicle->getNameID(), text, 255 );
	EString tmpStr = text;
	tmpStr.MakeUpper();
	descriptionListBox.removeAllItems( true );

	aTextListItem* pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );
	pItem->forceToTop( true );
	pItem->resize( descriptionListBox.width() - descriptionListBox.getScrollBarWidth() - 16, pItem->height() );
 	pItem->setText( descID );
	pItem->sizeToText( );
	pItem->setColor( 0xff005392 );

	descriptionListBox.AddItem( pItem );

	compListBox.setVehicle( pVehicle );

	camera.setVehicle( pVehicle->getFileName(), prefs.baseColor, prefs.highlightColor, prefs.highlightColor );
	camera.setScale( pVehicle->getScale() );

	textObjects[0].setText( tmpStr );

	statsListBox.removeAllItems( true );

	char formatText[256];
	char tmp[256];


	
	long color = textObjects[0].getColor();
	//	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );
	// add house stats NO HOUSE FOR VEHICLES
//	long houseID = pVehicle->getHouseID();
//	cLoadString( IDS_HOUSE0 + houseID, tmp, 255 );
//	cLoadString( IDS_EN_HOUSE, text, 255 );
//	sprintf( formatText, text, tmp );

//	pItem->setText( formatText );
//	pItem->setColor( color );
//	statsListBox.AddItem( pItem );

	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );
	// add weight stats
	cLoadString( IDS_EN_WEIGHT, text, 255 );
	sprintf( formatText, text, pVehicle->getMaxWeight() );	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );

	// add weight class stats
	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

	cLoadString( IDS_EN_CLASS, text, 255 );
	sprintf( formatText, text, (const char*)pVehicle->getMechClass() );	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );

	// now armor
	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

	cLoadString( IDS_EN_ARMOR, text, 255 );
	cLoadString( pVehicle->getArmorClass(), tmp, 255 );
 	sprintf( formatText, text, tmp, pVehicle->getArmor() );	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );


	// now speed
	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

	cLoadString( IDS_EN_SPEED, text, 255 );
	sprintf( formatText, text, (long)pVehicle->getDisplaySpeed());	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );

}

void Mechlopedia::MechScreen::setMech( LogisticsVariant* pChassis, bool bShowJump )
{
	if ( !pChassis )
		return;

	int descID = pChassis->getEncyclopediaID();
	descriptionListBox.removeAllItems( true );

	aTextListItem* pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );
	pItem->resize( descriptionListBox.width()  - descriptionListBox.getScrollBarWidth()- 16, pItem->height() );
 	pItem->setText( descID );
	pItem->sizeToText( );
	pItem->setColor( 0xff005392 );

	pItem->forceToTop( true );
	descriptionListBox.AddItem( pItem );

	compListBox.setMech( pChassis );

	camera.setMech( pChassis->getFileName(), prefs.baseColor, prefs.highlightColor, prefs.highlightColor );

	char name[256];
	cLoadString( pChassis->getChassisName(), name, 255 );
	EString upper = name;
	upper.MakeUpper();
	textObjects[0].setText( upper  );

	statsListBox.removeAllItems( true );

	char text[256];
	char formatText[256];
	char tmp[256];

	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

	long color = textObjects[0].getColor();

	// add house stats
	if ( !bIsVehicle )
	{
		long houseID = pChassis->getHouseID();
		cLoadString( IDS_HOUSE0 + houseID, tmp, 255 );
		cLoadString( IDS_EN_HOUSE, text, 255 );
		sprintf( formatText, text, tmp );

		pItem->setText( formatText );
		pItem->setColor( color );
		statsListBox.AddItem( pItem );
	}

	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );
	// add weight stats
	cLoadString( IDS_EN_WEIGHT, text, 255 );
	sprintf( formatText, text, pChassis->getMaxWeight() );	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );

	// add weight class stats
	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

	cLoadString( IDS_EN_CLASS, text, 255 );
	sprintf( formatText, text, (const char*)pChassis->getMechClass() );	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );

	// now armor
	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

	cLoadString( IDS_EN_ARMOR, text, 255 );
	cLoadString( pChassis->getArmorClass(), tmp, 255 );
 	sprintf( formatText, text, tmp, pChassis->getArmor() );	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );


	// now speed
	pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

	cLoadString( IDS_EN_SPEED, text, 255 );
	sprintf( formatText, text, (long)pChassis->getDisplaySpeed());	
	
	pItem->setText( formatText );
	pItem->setColor( color );
	statsListBox.AddItem( pItem );

	// now jump range
	if ( bShowJump )
	{
		pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );

		cLoadString( IDS_EN_JUMP, text, 255 );
		sprintf( formatText, text, (long)pChassis->getJumpRange() * 25 );	
		
		pItem->setText( formatText );
		pItem->setColor( color );
		statsListBox.AddItem( pItem );
	}

}


//////////////////////////////////////////////////////////
void Mechlopedia::WeaponScreen::init()
{
	FullPathFileName path;
	path.init( artPath, "mcl_en_wep", ".fit" );

	FitIniFile file;
	if ( NO_ERR != file.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, errorStr );
	}

	SubScreen::init( file );

	statsListBox.init( rects[2].left(), rects[2].top(), rects[2].right(), rects[2].bottom() );
	statsListBox.move( 285, 58 );

	camera.init( statics[4].left() + 285, statics[4].top() + 58, statics[4].right() + 285, statics[4].bottom() + 58 );
	statics[4].setColor( 0 );

	textObjects[0].setText( "" );

}
void Mechlopedia::WeaponScreen::update()
{
	SubScreen::update();
	
	statsListBox.update();
}

void Mechlopedia::WeaponScreen::render()
{
	descriptionListBox.render();
	statsListBox.render();
	groupListBox->render();

	LogisticsScreen::render( 285, 58 );
	camera.render();
	
}

void Mechlopedia::WeaponScreen::select( aTextListItem* pEntry )
{
	LogisticsComponent* pComponent = LogisticsData::instance->getComponent( pEntry->getID() );
	setWeapon( pComponent );

	
}

int __cdecl sortWeapon( const void* pW1, const void* pW2 )
{
	LogisticsComponent* p1 = *(LogisticsComponent**)pW1;
	LogisticsComponent* p2 = *(LogisticsComponent**)pW2;

	return stricmp( p1->getName(), p2->getName() );

}

void Mechlopedia::WeaponScreen::begin()
{
	groupListBox->removeAllItems( true );

	LogisticsComponent* comps[256];
	int count = 256;
	LogisticsData::instance->getAllComponents( comps, count );

	qsort( comps, count, sizeof( LogisticsComponent* ), sortWeapon );

	for ( int i = 0; i < count; i++ )
	{
		MechlopediaListItem* pItem = new MechlopediaListItem();
		EString text = comps[i]->getName();
		text.MakeUpper();
		pItem->setText( text  );
		pItem->setID( comps[i]->getID() );
		pItem->resize( groupListBox->width() - groupListBox->getScrollBarWidth() - 18, pItem->height() );

		groupListBox->AddItem( pItem );
	}

	aTextListItem* pEntry = (aTextListItem*)groupListBox->GetItem( 0 );
	if ( pEntry )
	{
		select( pEntry );
		groupListBox->SelectItem( 0 );
		pEntry->setColor( 0xff866234 );
	}

	groupListBox->setScrollPos( 0 );
}
void Mechlopedia::WeaponScreen::setWeapon ( LogisticsComponent* pComponent )
{
	if ( !pComponent )
		return;
	
	statsListBox.removeAllItems( true );
	descriptionListBox.removeAllItems( true );

	//set header
	EString name = pComponent->getName();
	name.MakeUpper();
	textObjects[0].setText( name );
	
	// set description
	aTextListItem* pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );
	pEntry->setColor( 0xff005392 );

	pEntry->setText( pComponent->getHelpID() );
	pEntry->resize( descriptionListBox.width() - descriptionListBox.getScrollBarWidth() - 16, pEntry->height() );
	pEntry->sizeToText();
	pEntry->forceToTop( true );
	descriptionListBox.AddItem( pEntry );

	char buffer[256];
	char final[256];
	char tmp[256];

	/*
	RATE OF FIRE
	HEAT
	AMMO (if the Ammo Tracking option is set to on in the Options Screen)
	COST*/

	// set stats
/*	pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );
	
	cLoadString( IDS_EN_WEAPON_WEIGHT, buffer, 255 );
	sprintf( final, buffer, pComponent->getWeight() );
	pEntry->setText( final );
	pEntry->setColor( textObjects[0].getColor() );
	statsListBox.AddItem( pEntry );*/

	// RANGE
	if ( pComponent->isWeapon() )
	{
		pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );
		
		cLoadString( IDS_EN_WEAPON_RANGE, buffer, 255 );
		cLoadString( IDS_HOTKEY1 + pComponent->getRangeType(), tmp, 255 );
		sprintf( final, buffer, tmp );
		pEntry->setText( final );
		pEntry->setColor( textObjects[0].getColor() );
		statsListBox.AddItem( pEntry );

		//	DAMAGE
		pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );
		
		cLoadString( IDS_EN_WEAPON_DAMAGE, buffer, 255 );
		sprintf( final, buffer, (long)pComponent->getDamage() );
		pEntry->setText( final );
		pEntry->setColor( textObjects[0].getColor() );
		statsListBox.AddItem( pEntry );

	}
	else if ( pComponent->getType() == COMPONENT_FORM_BULK )
	{
		pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );
		
		cLoadString( IDS_EN_WEAPON_ARMOR, buffer, 255 );
		sprintf( final, buffer, 32 );
		pEntry->setText( final );
		pEntry->setColor( textObjects[0].getColor() );
//		pEntry->setHelpID( IDS_EN_WEAPON_ARMOR_HELP );
		statsListBox.AddItem( pEntry );


	}


	// RATE OF FIRE
	if ( pComponent->getRecycleTime() )
	{
		pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );
		
		cLoadString( IDS_EN_WEAPON_RATEOFFIRE, buffer, 255 );
		sprintf( final, buffer, (10.f/pComponent->getRecycleTime()) );
		pEntry->setText( final );
		pEntry->setColor( textObjects[0].getColor() );
		statsListBox.AddItem( pEntry );
	}


	// heat
	pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );
	
	cLoadString( IDS_EN_WEAPON_HEAT, buffer, 255 );
	sprintf( final, buffer, (long)pComponent->getHeat() );
	pEntry->setText( final );
	pEntry->setColor( textObjects[0].getColor() );
	statsListBox.AddItem( pEntry );

	// AMMO
	if ( !prefs.useUnlimitedAmmo && pComponent->getAmmo() )
	{
		pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );

		cLoadString( IDS_EN_WEAPON_AMMO, buffer, 255 );
		sprintf( final, buffer, (long)pComponent->getAmmo() );
		pEntry->setText( final );
		pEntry->setColor( textObjects[0].getColor() );
		statsListBox.AddItem( pEntry );
	}

	// COST
	pEntry = new aTextListItem( IDS_EN_WEAPON_FONT );

	cLoadString( IDS_EN_WEAPON_COST, buffer, 255 );
	sprintf( final, buffer, (long)pComponent->getCost() );
	pEntry->setText( final );
	pEntry->setColor( textObjects[0].getColor() );
	statsListBox.AddItem( pEntry );



	FullPathFileName path;
	path.init( artPath, pComponent->getIconFileName(), ".tga" );

	int sizeX = pComponent->getComponentWidth();
	int sizeY = pComponent->getComponentHeight();
	float oldMidX = (rects[1].right() + rects[1].left())/2.f;
	float oldMidY = (rects[1].bottom() + rects[1].top())/2.f;
	statics[9].setTexture( path);
	statics[9].resize( sizeX * LogisticsComponent::XICON_FACTOR, sizeY * LogisticsComponent::YICON_FACTOR);
	statics[9].setUVs( 0.f, 0.f, sizeX * 48.f, sizeY * 32.f );
	statics[9].moveTo( oldMidX - .5 * statics[9].width(), oldMidY - .5 * statics[9].height() );



	camera.setComponent( pComponent->getPictureFileName() );
	camera.setScale( 1.5 );

}

//*************************************************************************************************

void Mechlopedia::PersonalityScreen::init()
{
	FullPathFileName path;
	path.init( artPath, "mcl_en_person", ".fit" );

	FitIniFile file;
	if ( NO_ERR != file.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, errorStr );
		return;
	}

	SubScreen::init( file );
}
void Mechlopedia::PersonalityScreen::update()
{
	groupListBox->update();
	SubScreen::update();
}
void Mechlopedia::PersonalityScreen::render()
{
	descriptionListBox.render();
	groupListBox->render();

	LogisticsScreen::render(285, 58);
}
void Mechlopedia::PersonalityScreen::begin()
{
	groupListBox->removeAllItems(true);

	int FirstID = bIsHistory ? IDS_HISTORY_0 : IDS_PERSONALITY_0;

	int count = bIsHistory ? 5 : PERSONALITY_COUNT;

	for ( int i = 0; i < count; i++ )
	{
		MechlopediaListItem* pItem = new MechlopediaListItem();
		char text[256];
		cLoadString( FirstID + i, text, 255 );
		EString upper = text;
		upper.MakeUpper();
		pItem->setText( upper );
		pItem->setID( i );
		pItem->resize( groupListBox->width() - groupListBox->getScrollBarWidth() - 18, pItem->height() );


		bool bAdded = 0;
		if ( !bIsHistory ) // turns out we need to sort 'em
		{
			for ( int j = 0; j < groupListBox->GetItemCount(); j++ )
			{
				MechlopediaListItem* pTmpItem = (MechlopediaListItem*)groupListBox->GetItem( j );
				if ( upper.Compare( pTmpItem->getText(), 1 ) < 0 )
				{
					groupListBox->InsertItem( pItem, j );
					bAdded = 1;
					break;
				}
			}

			if ( !bAdded )
				groupListBox->AddItem( pItem );
		}
		else
			groupListBox->AddItem( pItem );

	}

	aTextListItem* pItem = (aTextListItem*)groupListBox->GetItem( 0 );
	if ( pItem )
	{
		select( pItem );
		groupListBox->SelectItem( 0 );
	}

	groupListBox->setScrollPos( 0 );
}

void Mechlopedia::PersonalityScreen::select( aTextListItem* pEntry )
{
	int ID = pEntry->getID();

	descriptionListBox.removeAllItems( true );

	
	int PictureID = bIsHistory ? IDS_HISTORY_PICTURE0 : IDS_PERSONALITY_PICTURE0;
	int DescriptionID = bIsHistory ? IDS_HISTORY_DESCRIPTION_0 : IDS_PERONSALITY_DESCRIPTION0;


	aTextListItem* pItem = new aTextListItem( IDS_EN_WEAPON_FONT );
	pItem->setColor( 0xff005392);
	pItem->resize( descriptionListBox.width() - descriptionListBox.getScrollBarWidth() - 16, pEntry->height() );
	pItem->setText( DescriptionID + ID );
	pItem->sizeToText();
	pItem->forceToTop( true );
	descriptionListBox.AddItem( pItem );

	textObjects[0].setText( pEntry->getText() );

	char fileName[256];
	cLoadString( PictureID + ID, fileName, 255 );
	FullPathFileName path;
	path.init( artPath, fileName, ".tga" );

	statics[4].setTexture( path );
	statics[4].setUVs( 0, 0, statics[4].width(), statics[4].height() );
}

////////////////////////////////////////////////////////
void MechlopediaListItem::render()
{
	bmpAnim.setState( (aAnimGroup::STATE)state );
	bmpAnim.update();
	long color = bmpAnim.getCurrentColor( (aAnimGroup::STATE)state );
	bmp.setColor( color );
	bmp.render();

	aAnimTextListItem::render();

}

void MechlopediaListItem::init( )
{
	FitIniFile file;
	FullPathFileName path;
	path.init( artPath, "mcl_en_sub", ".fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn' open file %s", (const char*)path );
		Assert( 0, 0, errorStr );
		return;
	}

	if ( !s_templateItem )
	{
		s_templateItem = new MechlopediaListItem();
		s_templateItem->bmp.init( &file, "Static0" );
		s_templateItem->bmpAnim.init( &file, "Animation1" );
		((aAnimTextListItem*)s_templateItem)->init( file );

	}
	
	
}


MechlopediaListItem::MechlopediaListItem() 
: aAnimTextListItem(IDS_EN_LISTBOX_FONT)
{
	if ( s_templateItem&& this != s_templateItem )
	{
		operator=( *s_templateItem );

		bmp = s_templateItem->bmp;
		bmpAnim = s_templateItem->bmpAnim;
		
		resize( bmp.width(), bmp.height()+2 );
	}

	addChild( &bmp );
}

///////////////////////////////////////////////////////////////////////////////////

void Mechlopedia::BuildingScreen::init()
{
	FullPathFileName path;
	path.init( artPath, "mcl_en_bldg", ".fit" );

	FitIniFile file;
	if ( NO_ERR != file.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, errorStr );
		return;
	}

	SubScreen::init( file );

	compListBox.init( rects[2].left(), rects[2].top(), rects[2].width(), rects[2].height() );
	compListBox.move( 285, 58 );

	camera.init( statics[4].left() + 285, statics[4].top() + 58, statics[4].right() + 285, statics[4].bottom() + 58 );
	statics[4].setColor( 0 );

	textObjects[0].setText( "" );

	descriptionListBox.init( rects[1].left() + 285, rects[1].top() + 58, rects[1].width(), rects[1].height() );


}
void Mechlopedia::BuildingScreen::update()
{
	SubScreen::update();
	compListBox.update();

}
void Mechlopedia::BuildingScreen::render()
{
	compListBox.render();
	descriptionListBox.render();
	groupListBox->render();
	LogisticsScreen::render(285, 58);
	camera.render();
}
void Mechlopedia::BuildingScreen::begin()
{
	groupListBox->removeAllItems( true );

	LogisticsData::Building* pBldgs[256];
	int count = 255;
	LogisticsData::instance->getBuildings( pBldgs, count );
	for (  int i = 0; i < count; i++ )
	{
		char tmp[256];
		cLoadString( pBldgs[i]->nameID, tmp, 255 );
		EString str = tmp;
		str.MakeUpper();
		EString liao = "Liao ";
		str.Remove( liao);
		liao = "LIAO ";
		str.Remove( liao );

		bool bFound = 0;
		for ( int j = 0; j < groupListBox->GetItemCount(); j++ )
		{
			if ( stricmp(  str, ((aTextListItem*)groupListBox->GetItem( j ))->getText() ) < 0 )
			{
				MechlopediaListItem* pItem = new MechlopediaListItem();
				pItem->setText( str );
				pItem->setID( pBldgs[i]->nameID );
				pItem->resize( groupListBox->width() - groupListBox->getScrollBarWidth() - 18, pItem->height() );

				groupListBox->InsertItem( pItem, j );
				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			MechlopediaListItem* pItem = new MechlopediaListItem();
			pItem->setText( str );
			pItem->setID( pBldgs[i]->nameID );
			pItem->resize( groupListBox->width() - groupListBox->getScrollBarWidth() - 18, pItem->height() );

			groupListBox->AddItem( pItem );
		}
		
	}

	aTextListItem* pItem = (aTextListItem*)groupListBox->GetItem( 0 );
	select( pItem );
}
void Mechlopedia::BuildingScreen::select( aTextListItem* pEntry )
{
	if ( !pEntry )
		return;

	LogisticsData::Building* pBldg = LogisticsData::instance->getBuilding( pEntry->getID() );

	if ( pBldg )
	{
		char name[256];
		cLoadString( pBldg->nameID, name, 255 );
		EString tmpStr = name;
		tmpStr.MakeUpper();
			
		EString	liao = "LIAO ";
		tmpStr.Remove( liao );
		
		textObjects[0].setText( tmpStr );

		cLoadString( IDS_EN_BUILDING_WEIGHT, name, 255 );
		char formatted[256];

		sprintf( formatted, name, pBldg->weight );
		textObjects[1].setText( formatted );

		descriptionListBox.removeAllItems( true );

		aTextListItem* pItem = new aTextListItem( IDS_EN_LISTBOX_FONT );
		pItem->resize( descriptionListBox.width() - descriptionListBox.getScrollBarWidth() - 16, pItem->height() );
 		pItem->setText( pBldg->encycloID );
		pItem->sizeToText( );
		pItem->setColor( 0xff005392 );
		pItem->forceToTop( true );
		descriptionListBox.AddItem( pItem );
		camera.setBuilding( pBldg->fileName );
		camera.setScale( pBldg->scale );

		LogisticsComponent* pComps[4];

		compListBox.removeAllItems( true );

		int count= 0;
		for ( int i = 0; i < 4; i++ )
		{
			int ID = pBldg->componentIDs[i];
			if ( ID )
			{
				LogisticsComponent* pComp = LogisticsData::instance->getComponent( ID );
				if ( pComp )
				{
					pComps[count++] = pComp;
				}
			}
		}

		if ( count )
			compListBox.setComponents( count, pComps );
		}


}

//*************************************************************************************************
// end of file ( Mechlopedia.cpp )
