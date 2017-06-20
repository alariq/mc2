#define SIMPLECOMPONENTLISTBOX_CPP
/*************************************************************************************************\
SimpleComponentListBox.cpp			: Implementation of the SimpleComponentListBox component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"simplecomponentlistbox.h"
#include"../resource.h"
#include"logisticsvariant.h"
#include"logisticscomponent.h"
#include"logisticsdata.h"
#include"cmponent.h"

//-------------------------------------------------------------------------------------------------
ComponentListBox::ComponentListBox()
{
	clickSFX = -1;
	highlightSFX = -1;
	disabledSFX = -1;
	helpID = IDS_HELP_WEAPONS;
}

ComponentListBox::~ComponentListBox()
{
	aListBox::destroy();
}

void ComponentListBox::setMech( LogisticsVariant* pMech )
{
	removeAllItems( true );

	if ( pMech )
	{
		LogisticsComponent* components[256];
		long componentCount = 256;

		pMech->getComponents( componentCount, components );

		setComponents( componentCount, components );
		
		int sensor = pMech->getSensorID();
		int ECM = pMech->getECM();

		if ( sensor > 0 )
		{
			aTextListItem* textItem = new aTextListItem( IDS_SALVAGE_AREA_COMPONENTS );
			textItem->setText( 32000 + sensor );
			textItem->setColor( 0xffc29b00 );
			textItem->setHelpID( IDS_HELP_COMP0 + sensor );
			AddItem( textItem );			
		}

		if ( ECM > 0 )
		{
			aTextListItem* textItem = new aTextListItem( IDS_SALVAGE_AREA_COMPONENTS );
			textItem->setText( 32000 + ECM );
			textItem->setColor( 0xffc29b00 );
			textItem->setHelpID( IDS_HELP_COMP0 + ECM );
			AddItem( textItem );
		}

	}
}

void ComponentListBox::setVehicle( LogisticsVehicle* pVeh )
{
	removeAllItems( true );

	if ( pVeh )
	{
		LogisticsComponent* components[256];
		long componentCount = 256;

		pVeh->getComponents( componentCount, components );

		setComponents( componentCount, components );	

	}
}


void ComponentListBox::setComponents( long componentCount, LogisticsComponent** components )
{
		LogisticsComponent* finalList[64];


		uint32_t finalListCount[64];
//		long stringIDs[4] = { IDS_SHORT, IDS_MEDIUM, IDS_LONG, IDS_COMPONENT};
		uint32_t colors[4] = { 0xff6E7C00, 0xff005392, 0xffA21600, 0xffc29b00};
//		long headerColors[4] = { 0xFFC8E100, 0xff0091FF, 0xFFFF0000, 0xffFF8A00 };

		for ( int i = 0; i < 4; i++ ) // do short, medium long
		{
			MemSet(finalList, 0);
			MemSet(finalListCount, 0);
			for ( int j = 0; j < componentCount; j++ )
			{
				if ( components[j]->getRangeType() == (LogisticsComponent::WEAPON_RANGE)i ) // short, med, long
				{
					bool bFound = 0;
                    int k = 0;
					for ( ; k < 64; k++ )
					{
						if ( finalList[k] == components[j])
						{
							finalListCount[k]++;
							bFound = true;
							break;
						}
						else if ( !finalList[k] )
							break;
					}

					if ( !bFound )
					{
						finalList[k] = components[j];
						finalListCount[k]++;
					}
				}
			}

			if ( finalList[0] )
			{
				EString str;
				
				// add the header
				// no more headers, keeping code just in case
//				aTextListItem* textItem = new aTextListItem( IDS_SALVAGE_AREA_COMPONENTS );
//				textItem->setText( stringIDs[i] );
//				textItem->setColor( headerColors[i] );
//				AddItem( textItem );

				// add each componet
				for( int j = 0; j < 64; j++ )
				{
					if ( !finalList[j] )
						break;

					aTextListItem* textItem = new aTextListItem( IDS_SALVAGE_AREA_COMPONENTS );
					if ( finalList[j]->getType() != COMPONENT_FORM_JUMPJET )
					{
						str.Format( "%d/%d %s", finalListCount[j],  finalListCount[j], finalList[j]->getName() );
						textItem->setText( str );
					}
					else
						textItem->setText( finalList[j]->getName()  );
					textItem->setColor( colors[i] );
					textItem->setHelpID( IDS_HELP_COMP0 + finalList[j]->getID() );
					AddItem( textItem );
				
				}

			}
		}

}


//*************************************************************************************************
// end of file ( SimpleComponentListBox.cpp )
