#define MECHPURCHASESCREEN_CPP
/*************************************************************************************************\
MechPurchaseScreen.cpp			: Implementation of the MechPurchaseScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mechpurchasescreen.h"
#include"mclib.h"
#include"logisticsdata.h"
#include"mechbayscreen.h"
#include"gamesound.h"
#include "..\resource.h"
#include"simplecamera.h"
#include"attributemeter.h"
#include"chatwindow.h"
#include"multplyr.h"

MechPurchaseScreen* MechPurchaseScreen::s_instance = NULL;

#define INVENTORY_ID 88
#define VARIANT_ID	89


//-------------------------------------------------------------------------------------------------

MechPurchaseScreen::MechPurchaseScreen(  ) 
: inventoryListBox( 1, 0 ), variantListBox( 0, 0 )
{
	status = RUNNING;
	gosASSERT( !s_instance );
	s_instance = this;
	acceptPressed = 0;
	pDragMech = NULL;
	dragStartLeft = 0;
	helpTextArrayID = 4; 
	inventoryListBox.setID( INVENTORY_ID );
	variantListBox.setID( VARIANT_ID );

	countDownTime = .5;
	curCount = 0.0;
	previousAmount = 0;

	oldCBillsAmount  =0;

	flashTime = 0.f;
}

//-------------------------------------------------------------------------------------------------

MechPurchaseScreen::~MechPurchaseScreen()
{
	s_instance = NULL;

	variantListBox.destroy();
	inventoryListBox.destroy();
}


//-------------------------------------------------------------------------------------------------

int MechPurchaseScreen::init( FitIniFile& file )
{
	MechListBox::init();

	mechDisplay.init();

	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button" );

	((aObject*)&inventoryListBox)->init( rects[0].left(), rects[0].top(),
		rects[0].width(), rects[0].height() );

	inventoryListBox.setScrollBarOrange();

	((aObject*)&variantListBox)->init( rects[1].left(), rects[1].top(),
		rects[1].width(), rects[1].height() );

	variantListBox.setScrollBarOrange();


	for ( int i= 0; i < buttonCount; i++ )
		buttons[i].setMessageOnRelease();


	return NO_ERR;
}
//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::update()
{

	
	if ( !MPlayer || !ChatWindow::instance()->pointInside(userInput->getMouseX(), userInput->getMouseY()) )
		LogisticsScreen::update();
	

	// update CBills

	int amount = LogisticsData::instance->getCBills();
	long color = 0xff005392;
	if ( amount != oldCBillsAmount )
	{
		previousAmount = oldCBillsAmount - amount;
		curCount = .00001f;
		oldCBillsAmount = amount;
		if ( previousAmount < 0 )
			soundSystem->playDigitalSample( WINDOW_OPEN );
		else
			soundSystem->playDigitalSample( WINDOW_CLOSE );
	}
	if ( curCount && curCount + frameLength < countDownTime  )
	{
		curCount += frameLength;
		float curAmount = previousAmount - (curCount/countDownTime * previousAmount); 
		amount += curAmount;

		color = 0xffc8e100;
		if ( curAmount > 0 )
			color = 0xffa21600;
	}
	else if ( flashTime )
	{
		flashTime += frameLength;
		if ( flashTime < .125
			|| ( flashTime > .25 && flashTime < .3875 )
			|| ( flashTime > .5 && flashTime < .6625 ) )
		{
			color = 0xff000000;
		}
		else if ( flashTime > .75 )
		{
			flashTime = 0;
		}
		else
		{
			color = 0xffff0000;			
		}
	}
	

	char tmp[64];
	sprintf( tmp, "%ld ", amount);
	textObjects[1].setText( tmp );
	textObjects[1].setColor( color );

	int oldSell = inventoryListBox.GetSelectedItem();
	inventoryListBox.update();
	int newSell = inventoryListBox.GetSelectedItem();
	if ( oldSell != newSell && newSell != -1 )
	{
		variantListBox.SelectItem( -1 );
	}

	oldSell = variantListBox.GetSelectedItem();
	variantListBox.update();
	newSell = variantListBox.GetSelectedItem();

	if ( oldSell != newSell  && newSell != -1 )
	{
		inventoryListBox.SelectItem( -1 );
	}

	variantListBox.disableItemsThatCostMoreThanRP();

	LogisticsMech* pCurSell = inventoryListBox.getCurrentMech();
	LogisticsMech* pCurBuy = variantListBox.getCurrentMech();

	// disable the sell button
	buttons[2].disable( pCurSell ? 0 : 1);

	if ( pCurBuy )
	{
		if ( pCurBuy->getCost() > LogisticsData::instance->getCBills() )
		{
			pCurBuy = NULL;
		}

		setMech( pCurBuy );
		
	}
	else if ( pCurSell )
		setMech( pCurSell );

	buttons[1].disable( pCurBuy ? 0 : 1 );
	if ( !pCurBuy &&
		userInput->isLeftClick() && buttons[1].pointInside( userInput->getMouseX(), userInput->getMouseY() ) )
	{
		if ( !flashTime )
			flashTime = .0001f;
	}

	// drag and drop
	// update drag and drop
	if ( pDragMech )
	{
		dragIcon.moveTo( userInput->getMouseX() - dragIcon.width() / 2, 
			userInput->getMouseY() - dragIcon.height() / 2 );

		if ( userInput->leftMouseReleased(  ) )
		{
			endDrag();	
		}

	}

	mechDisplay.update();

	inventoryListBox.enableAllItems();

	if ( MPlayer && ChatWindow::instance() )
		ChatWindow::instance()->update();

}

//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::render( int xOffset, int yOffset )
{

	if ( !xOffset && !yOffset )
	{
		if ( !MPlayer && !LogisticsData::instance->isSingleMission() && LogisticsData::instance->newMechsAvailable() )
		{
			soundSystem->playBettySample( BETTY_NEW_MECHS );
			LogisticsData::instance->setNewMechsAcknowledged();
		}
	}

	inventoryListBox.move( xOffset, yOffset);
	inventoryListBox.render();
	inventoryListBox.move( -xOffset, -yOffset );

	variantListBox.move(xOffset, yOffset);
	variantListBox.render();
	variantListBox.move(-xOffset, -yOffset);	

	mechDisplay.render( xOffset, yOffset );

	LogisticsScreen::render( xOffset, yOffset );

	if ( pDragMech )
		dragIcon.render();

	
	if ( MPlayer && ChatWindow::instance() )
		ChatWindow::instance()->render(xOffset, yOffset);



}

//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::begin()
{

	variantListBox.removeAllItems(true);
	inventoryListBox.removeAllItems(true);
	// initialize both the inventory and icon lists
	EList< LogisticsMech*, LogisticsMech* > mechList;
	LogisticsData::instance->getInventory( mechList );
	prevInventory.Clear();

	LogisticsMech* pSelMech = 0;

	oldCBillsAmount = LogisticsData::instance->getCBills();


	for ( EList< LogisticsMech*, LogisticsMech* >::EIterator iter = mechList.Begin();
		!iter.IsDone(); iter++ )
		{
			if ( (*iter)->getForceGroup() )
				continue;
		
			prevInventory.Append( *(*iter) );
			
	
			bool bFound = 0;

			for ( int i = 0; i < inventoryListBox.GetItemCount(); i++ )
			{
				if ( ((MechListBoxItem*)inventoryListBox.GetItem(i))->getMech()->getVariant()
					== (*iter)->getVariant() )
					bFound = true;
			}

			if ( !bFound )
			{
				MechListBoxItem* item = new MechListBoxItem( (*iter), 1 );
				inventoryListBox.AddItem( item );	

			}

		}

	MechListBoxItem* item = (MechListBoxItem*)inventoryListBox.GetItem( 0 );
	if ( item )
	{
		inventoryListBox.SelectItem( 0 );
		pSelMech = item->getMech();
	}
	

	LogisticsVariant* pVariants[256];
	int count = 256;

	LogisticsData::instance->getPurchasableMechs( pVariants, count );

	for ( int i =0; i < count; i++ )
	{
		if ( !MPlayer || MPlayer->missionSettings.variants || pVariants[i]->isDesignerMech() )
		{
			LogisticsMech* pMech = new LogisticsMech( pVariants[i], -1 );
			MechListBoxItem* item = new MechListBoxItem( pMech, 1 );
			variantListBox.AddItem( item );
		}
	}

	if ( !pSelMech )
	{
		MechListBoxItem* item = (MechListBoxItem*)variantListBox.GetItem( 0 );
		if ( item )
		{
			variantListBox.SelectItem( 0 );
			pSelMech = item->getMech();
		}
	}

	status = RUNNING;
	acceptPressed = 0;

	inventoryListBox.drawCBills(1);
	variantListBox.drawCBills(1);
	variantListBox.setOrange( 0 );
	inventoryListBox.setOrange( 1 );
	
	
	setMech( pSelMech );

	pDragMech = NULL;

}
//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::end()
{
	if ( !acceptPressed && status != MAINMENU )
	{ 
		// sell all current stuff
		EList< LogisticsMech*, LogisticsMech* > list;
		LogisticsData::instance->getInventory( list );
		for ( EList< LogisticsMech*, LogisticsMech* >::EIterator iter = list.End(); !iter.IsDone(); iter-- )
		{
			LogisticsData::instance->sellMech( (*iter) );
		}


		unsigned long base, color1, color2;
		for ( MECH_LIST::EIterator pIter = prevInventory.Begin(); !pIter.IsDone(); pIter++ )
		{
			(*pIter).getColors(base, color1, color2);
			LogisticsData::instance->addMechToInventory( (*pIter).getVariant(), (*pIter).getPilot(),
				(*pIter).getForceGroup(), true );
		}

	}

	variantListBox.removeAllItems(true);
	inventoryListBox.removeAllItems(true);

	mechDisplay.setMech( NULL );
}

//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::setMech( LogisticsMech* pMech, bool FromLB )
{
	if ( status == RUNNING )
	{
		if ( FromLB ) // clicked on a disabled item
		{
			if ( !flashTime )
				flashTime = .000001f;
		}
		
		if ( FromLB ) // clicked on a disabled item
		{
			inventoryListBox.SelectItem( -1 );
			variantListBox.SelectItem( -1 );

		}
		mechDisplay.setMech( pMech );		

	}
}

//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::beginDrag( LogisticsMech* pMech )
{
	if ( status != RUNNING )
		return;
	
	if ( !pDragMech )
	{
		pDragMech = pMech;
	}

	MechListBox::initIcon( pMech, dragIcon );

	if ( inventoryListBox.pointInside( userInput->getMouseDragX(),
		userInput->getMouseDragY() ) )
	{
		dragStartLeft = true;
		inventoryListBox.dimItem( pMech, true );
	}
	else 
	{
		dragStartLeft = false;
		variantListBox.dimItem( pMech, true );
	}
}

void MechPurchaseScreen::endDrag( )
{
	if ( pDragMech )
	{
	
		if ( inventoryListBox.pointInside( userInput->getMouseX(),
			userInput->getMouseY() ) )
		{
			if ( !dragStartLeft )
			{
				addMech( pDragMech );
			}
		}
		else
		{
			if ( dragStartLeft )
				removeMech( pDragMech );
		}
	}

	inventoryListBox.undimAll();
	variantListBox.undimAll();

	pDragMech = NULL;
}


//-------------------------------------------------------------------------------------------------

int	MechPurchaseScreen::handleMessage( unsigned long what, unsigned long who )
{
	if ( status == RUNNING )
	{
		switch (who)
		{
			case MB_MSG_ADD:
				if ( INVENTORY_ID == what )
				{
					int index = inventoryListBox.GetSelectedItem();
					if ( index != -1 )
					{
						aListItem* pItem = inventoryListBox.GetItem( index );
						if ( pItem )
						{
							LogisticsMech* pMech = ((MechListBoxItem*)pItem)->getMech();
							int oldCount = LogisticsData::instance->getVariantsInInventory( pMech->getVariant(), true );
							if ( NO_ERR == LogisticsData::instance->sellMech( pMech ) && ( oldCount < 2 ) )
								((MechListBoxItem*)pItem)->resetMech( );
							soundSystem->playDigitalSample( LOG_SELECT );		
						}                                    
					}
				}

				else
					addSelectedMech();
					soundSystem->playDigitalSample( LOG_SELECT );		
				break;
			case MB_MSG_REMOVE:
				removeSelectedMech();
				break;
			case MB_MSG_NEXT:
				acceptPressed = true;
				status = DOWN;
				break;
			case MB_MSG_PREV:
				status = DOWN;
				break;
			case MB_MSG_MAINMENU:
				status = MAINMENU;
				break;
		}
	}

	return 0;

}

//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::addSelectedMech()
{
	LogisticsMech* pMech = variantListBox.getCurrentMech();
	if ( pMech )
	{
		addMech( pMech );
	}

	variantListBox.disableItemsThatCostMoreThanRP();
	int curSel = variantListBox.GetSelectedItem();
	if ( curSel == -1 )
	{
		variantListBox.SelectItem( -1 );
		inventoryListBox.SelectItem( 0 );
	}

}

void MechPurchaseScreen::addMech( LogisticsMech* pMech )
{
	LogisticsData::instance->purchaseMech( pMech->getVariant() );

		for ( int i = 0; i < inventoryListBox.GetItemCount(); i++ )
		{
			if ( ((MechListBoxItem*)inventoryListBox.GetItem(i))->getMech()->getVariant()
				== pMech->getVariant() )
			{
				return;
			}
		}

		MechListBoxItem* pItem = new MechListBoxItem( pMech, 1 );
		inventoryListBox.AddItem( pItem );
}

//-------------------------------------------------------------------------------------------------

void MechPurchaseScreen::removeSelectedMech()
{
	LogisticsMech* pMech = inventoryListBox.getCurrentMech();
	if ( pMech )
	{
		removeMech( pMech );
	}
}

void MechPurchaseScreen::removeMech( LogisticsMech* pMech )
{
	int index = inventoryListBox.GetSelectedItem();
	aListItem* pItem = inventoryListBox.GetItem( index );

	int oldCount = LogisticsData::instance->getVariantsInInventory( pMech->getVariant(), true );
	if ( NO_ERR == LogisticsData::instance->sellMech( pMech ) && ( oldCount < 2 ) )
	{
		inventoryListBox.RemoveItem( pItem, true );
	}

	if ( !inventoryListBox.GetItemCount() )
	{
		selectFirstBuyableMech();
	}
	else if ( -1 == inventoryListBox.GetSelectedItem() )
		inventoryListBox.SelectItem( 0 );

}

bool MechPurchaseScreen::selectFirstBuyableMech()
{
	for ( int i = 0; i < variantListBox.GetItemCount(); i++ )
	{
		if ( NO_ERR == LogisticsData::instance->canPurchaseMech( 
			((MechListBoxItem*)variantListBox.GetItem(i))->getMech()->getVariant() ) )
		{
			variantListBox.SelectItem( i );
			return true;
		}
	}

	return false;
}




//*************************************************************************************************
// end of file ( MechPurchaseScreen.cpp )
