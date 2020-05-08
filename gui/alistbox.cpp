//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef ASYSTEM_H
#include"asystem.h"
#endif

#ifndef AFONT_H
#include"afont.h"
#endif

#ifndef ALISTBOX_H
#include"alistbox.h"
#endif

#include"soundsys.h"

extern SoundSystem *sndSystem;

#include"mclib.h"

///////////////////////////////////////////////////////////////////////////////
// aListBox methods
///////////////////////////////////////////////////////////////////////////////

aListBox::aListBox()
{
	itemCount = 0;
	itemSelected = -1;
	memset( items, 0, sizeof( aListItem*) * MAX_LIST_ITEMS );
	scrollBar = 0;
	skipAmount = 1;
	singleCheck = 1;
	topSkip = 0;

	clickSFX = LOG_SELECT;
	highlightSFX = LOG_DIGITALHIGHLIGHT;
	disabledSFX = LOG_WRONGBUTTON;

}

long aListBox::init(long xPos, long yPos,long w, long h )
{
	long err;
	
	err = aObject::init(xPos,yPos,w,h);
	if (err)
		return err;

	//If you call the FitIni INIT and then this init afterwards, we leak one.
	if (scrollBar)
	{
		scrollBar->destroy();
		delete scrollBar;
		scrollBar = NULL;
	}

	scrollBar = new mcScrollBar;
	scrollBar->init( w - 17, 2, 15, h-4 );
	scrollBar->SetScrollMax( 0 );
	addChild( scrollBar );

	return (NO_ERR);
}

void aListBox::init( FitIniFile* file, const char* blockName )
{
	long x, y, width, height;
	file->readIdLong( "XLocation", x );
	file->readIdLong( "YLocation", y );
	file->readIdLong( "Width", width );
	file->readIdLong( "Height", height );

	file->readIdLong( "HelpCaption", helpHeader );
	file->readIdLong( "HelpDesc", helpID );


	init( x, y, width, height );

}

void aListBox::destroy()
{
	if (scrollBar)
	{
		scrollBar->destroy();
		delete scrollBar;
		scrollBar = NULL;
	}

	removeAllItems( true );
	
	aObject::destroy();
}

void aListBox::removeAllItems( bool bDelete )
{
    // sebi: WTF??? always true
	//if (items)
	{
		for ( int i = 0; i < itemCount; i++ )
		{
			if ( items[i] && bDelete )
				delete items[i];

			items[i] = 0;
		}
	}

	itemCount = 0;
	itemSelected = -1;

	if ( scrollBar )
		scrollBar->SetScroll( 0 );

}

void aListBox::update()
{
	if ( showWindow )
	{ /* if the listbox is invisible, then we probably don't want to update it either */
		if ( scrollBar )
			scrollBar->update();


		int prevCheck = GetCheckedItem();
		int newCheck = -1;


		int iSel = -1;
		int iHighlight = -1;
		long mouseX = userInput->getMouseX();
		long mouseY = userInput->getMouseY();

		bool bInside = pointInside( mouseX, mouseY );
		if ( bInside && helpID )
		{
			::helpTextID = helpID;
		}

		for ( int i = 0; i < itemCount; i++ )
		{
			if ( bInside &&
				items[i]->pointInside(mouseX, mouseY) && items[i]->isShowing()
				&& items[i]->getState() != aListItem::DISABLED )
			{
				if ( userInput->isLeftClick() )
				{
					iSel = i;
					if ( items[i]->getState() != aListItem::SELECTED )
						sndSystem->playDigitalSample( clickSFX );
				}
				else 
				{
					iHighlight = i;
				}
			}

			if ( items[i]->globalBottom() > globalTop()
				&& items[i]->globalTop() < globalBottom() )
				{
					items[i]->showGUIWindow( 1 );
				}
				else
				{
					items[i]->showGUIWindow( 0 );
				}
		}

		for (int i = 0; i < itemCount; i++ )
		{
			if ( iSel == i )
			{
				items[i]->setState( aListItem::SELECTED );	
				itemSelected = i;
			}
			else if ( iHighlight == i && items[i]->getState() != aListItem::SELECTED )
			{
				if ( items[i]->getState() != aListItem::HIGHLITE )
						sndSystem->playDigitalSample( highlightSFX );
				items[i]->setState( aListItem::HIGHLITE );				
			}
			else if ( (iSel != -1 || items[i]->getState() != aListItem::SELECTED)
				&& items[i]->getState() != aListItem::DISABLED )
				items[i]->setState( aListItem::ENABLED );

			items[i]->update();

		}

		for (int i = 0; i < itemCount; i++ )
		{
			if ( items[i]->isChecked() && i != prevCheck )
			{
				newCheck = i;
			}
		}

		if ( singleCheck && newCheck != -1 && prevCheck != -1)
		{
			items[prevCheck]->setCheck( 0 );
		}
	}

	


}

	
int aListBox::handleMessage( unsigned long message, unsigned long who )
{
	switch ( message )
	{
	case aMSG_SCROLLUP:
		scroll( -1 );
		break;

	case aMSG_SCROLLDOWN:
		scroll( 1 );
		break;

	case aMSG_PAGEUP:
		scroll( 10 );
		break;

	case aMSG_PAGEDOWN:
		scroll( -10 );
		break;

	case aMSG_SCROLLTO:
		{
			scroll( -scrollBar->GetScrollPos() );
			scroll( who );
		}
		break;
	}

	return 0;
}

void aListBox::resize(long w, long h)
{
	aObject::resize(w, h);
	scrollBar->resize(scrollBar->width(), h - 4);
}

void aListBox::setScrollPos( int pos )
{
	scroll( pos - scrollBar->GetScrollPos() );
	scrollBar->SetScroll( pos );
}



void aListBox::render()
{
	if ( showWindow )
	{
		bool bTop  =0;
		bool bBottom = 0;

		gos_SetRenderState( gos_State_Texture, 0 );
		gos_SetRenderState(gos_State_Filter, gos_FilterNone);
		gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );
		gos_SetRenderState( gos_State_ZCompare, 0 );
		gos_SetRenderState( gos_State_ZWrite, 0 );

		gos_DrawQuads( location, 4 );

		long topHeight = 0;
		long bottomHeight = 0;


		bool bItemOutOfRange = 0;
		for ( int i = 0; i < itemCount; i++ )
		{
			if ( items[i]->globalBottom() > globalTop()
				&& items[i]->globalTop() < globalBottom() )
			{
				items[i]->render();

				if ( items[i]->globalTop() < globalTop() )
				{
					topHeight = items[i]->height();
					bTop = 1;
					bItemOutOfRange = true;
				}
				if ( items[i]->globalBottom() >= globalBottom() )
				{
					bBottom = 1;
					if ( !bottomHeight )
						bottomHeight = items[i]->height();

					bItemOutOfRange = true;
				}
			}
			else 
			{
				bItemOutOfRange = true;
			}
		}

		// draw black box above this
		if ( bTop || bBottom )
		{
			GUI_RECT rect = { globalX(), globalY() - topHeight, (globalX() + (long)width()), globalY() };
			if ( bTop )
				drawRect( rect, 0xff000000 ); 
			rect.top = globalY() + height()+1;
			rect.bottom =globalY() + height() + bottomHeight+2;
			if ( bBottom )
				drawRect( rect, 0xff000000 );
		}
		if ( scrollBar )
		{
			if ( bItemOutOfRange )
			{
				scrollBar->Enable( 1 );
			}

			else 
				scrollBar->Enable( 0 );

			scrollBar->render();
		}

			
	}
}

void aListBox::move( float offsetX, float offsetY )
{
	aObject::move( offsetX, offsetY );
	for ( int i = 0; i < itemCount; i++ )
	{
		items[i]->move( offsetX, offsetY );
	}
}

long aListBox::AddItem(aListItem* add)
{
	if (itemCount >= MAX_LIST_ITEMS)
		return TOO_MANY_ITEMS;

	
	float lastX = globalX() + add->globalX();
	float lastY = globalY() + topSkip;
	
	if ( itemCount )
	{
		lastY = items[itemCount-1]->globalY() + items[itemCount-1]->height() + skipAmount;
	}

	items[itemCount++] = add;
	add->moveTo( lastX, lastY );

	if ( add->globalRight() > globalRight() ) // can't have it hanging over the edge
	{
		float shrink = globalRight() - add->globalRight();
		add->resize( add->width() +  shrink, add->height() );
	}



	if ( scrollBar )
	{
		int itemsTotalHeight = 0;

        // sebi: WTF??? always true
        // putting assert here, because otherwise items[itemCount-1] does bad stuff
        gosASSERT(itemCount>0);
		if ( itemCount>0 )
			itemsTotalHeight = items[itemCount-1]->bottom() - items[0]->top() + skipAmount;

			if ( itemsTotalHeight > scrollBar->height() )
				scrollBar->SetScrollMax( itemsTotalHeight - scrollBar->height() );
			else
			{
				scrollBar->SetScroll( 0 );
				scrollBar->SetScrollMax( 0 );
			}
			 
	}

	return itemCount - 1;
}

long	aListBox::InsertItem(aListItem* itemString, long where)
{
	if ( itemCount >= MAX_LIST_ITEMS )
		return -1;
	gosASSERT( itemString );
	if ( where >= itemCount )
		where = itemCount;
	for ( int i = itemCount - 1; i > where - 1; i-- )
	{
		items[i+1] = items[i];
		items[i+1]->move( 0, itemString->height() + skipAmount );
	}

	float lastX = x() + itemString->x();
	float lastY = y();

	items[where] = itemString;

	if ( itemCount > 0 )
	{
		if ( where - 1 > -1 )
			lastY = items[where-1]->bottom() + skipAmount;
		else
			lastY = items[where+1]->top() - skipAmount - itemString->height();
	}

	itemString->moveTo( lastX, lastY );
	itemString->showGUIWindow( true );

	if ( itemString->globalRight() > globalRight() ) // can't have it hanging over the edge
	{
		float shrink = globalRight() - itemString->globalRight();
		itemString->resize( itemString->width() +  shrink, itemString->height() );
	}

	itemCount++;


	if ( scrollBar )
	{
		int itemsTotalHeight = 0;
        // sebi: WTF??? always true
        // putting assert here, because otherwise items[itemCount-1] does bad stuff
        gosASSERT(itemCount>0);
		if ( itemCount>0 )
			itemsTotalHeight = items[itemCount-1]->bottom() - items[0]->top();

			if ( itemsTotalHeight > scrollBar->height() )
				scrollBar->SetScrollMax( itemsTotalHeight - scrollBar->height() );
			else
				scrollBar->SetScrollMax( 0 );
			
	}


	return where;
}

long	aListBox::RemoveItem( aListItem* item, bool bDelete )
{
	if ( !item )
		return -1;

	float height = item->height();

	bool bFound = false;
	for ( int i = 0; i < itemCount; i++ )
	{
		if ( items[i] == item )
		{
			bFound = true;
			if ( bDelete )
				delete items[i];
			if ( i < itemCount - 1 )
			{
				items[i] = items[i+1];
				items[i]->move( 0, -height - skipAmount );
			}
			else
			{
				items[i] = NULL;
				if ( itemSelected == i )
					itemSelected = -1;
			}
		}
		else if ( bFound )
		{
			if ( i < itemCount - 1 )
			{
				items[i] = items[i+1];
				items[i]->move( 0, -height - skipAmount );
			}
			else
				items[i] = NULL;
		}
	}

	if ( bFound )
	{
		itemCount --;
		if ( itemSelected != -1 )
			SelectItem( itemSelected );
	}

	else
		return -1;

	return 0;
}

long aListBox::SelectItem(long itemNumber)
{
	if (itemNumber >= itemCount)
		return ITEM_OUT_OF_RANGE;
	
	for ( int i = 0; i < itemCount; i++ )
	{
		items[i]->deselect();
	}
	
	
	itemSelected = itemNumber;
	if ( itemSelected > -1 )
	{
		aListItem* pItem = GetItem( itemNumber );
		if ( pItem )
		{
			pItem->setState( aListItem::SELECTED );

			if ( !pItem->isShowing() || pItem->globalY() + pItem->height() > globalY() + height() 
				|| pItem->globalY() + pItem->height() < globalY() )
			{
				int newPos = (pItem->globalY() + pItem->height() - globalY()) - scrollBar->height() + scrollBar->GetScrollPos();

				scrollBar->SetScroll( newPos  );//				float itemY = pItem->globalY() + pItem->height();
			}
		}
	}

	
	return NO_ERR;
}

void	aListBox::enableAllItems()
{
	for ( int i = 0; i < itemCount; i++ )
	{
		if ( items[i]->getState() == aListItem::DISABLED )
			items[i]->setState( aListItem::ENABLED );
	}
}


aListItem* aListBox::GetItem(long itemNumber)
{
	if (itemNumber >= itemCount)
		return NULL;

	return items[itemNumber];
	
}	

void aListBox::scroll( int amount )
{
    // sebi: !items has no sense as will always return false
	//if ( !items || !itemCount )
	if ( !itemCount )
		return;

	if ( amount < 0 && items[0]->globalY() > globalY() ) // no need to scroll
		return;

	else if ( amount > 0 && (items[itemCount-1]->globalY() + items[itemCount-1]->height()) < (globalY() + height() ) )
		return;

	for ( int i = 0; i < itemCount; i++ )
	{
		items[i]->move( 0, -amount );
	}
}

long aListBox::GetCheckedItem() const
{
	int ret = -1;
    // sebi: always coca cola...
	//if ( items )
	{
		for ( int i = 0; i < itemCount; i++ )
		{
			if ( items[i] && items[i]->isChecked() )
				return i;
		}
	}

	return ret;
}

long	aListBox::getScrollBarWidth()
{
	if ( scrollBar )
		return scrollBar->width();

	return 0;
}

void	aListBox::setOrange(bool bOrange)
{
	scrollBar->setOrange();
}

bool				aListBox::pointInScrollBar( long mouseX, long mouseY )
{
	if ( scrollBar )
	{
		return scrollBar->pointInside( mouseX, mouseY );
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// aDropList methods
///////////////////////////////////////////////////////////////////////////////

aDropList::aDropList(void) 
: templateItem(  27333 )
{
	listBoxMaxHeight = 0.0;
	selectionIndex = -1;

	rects = NULL;
}	

aDropList& aDropList::operator=( const aDropList& src)
{
	aObject::operator =( src );

	
	//Toss the CURRENT listBox
	listBox.destroy();


	//TOss the current rects.  We are about to NEW over them!!
	if ( rects )
	{
		delete [] rects;
		rects = NULL;
	}

	listBox.removeAllItems( 0 );
	listBoxMaxHeight = src.listBoxMaxHeight;
	selectionIndex = -1;

	rectCount = src.rectCount;

	if ( rectCount )
	{
		rects = new aRect[rectCount];
		for ( int i = 0; i < rectCount; i++  )
		{
			addChild( &rects[i] );
			rects[i] = src.rects[i];
		}
	}
	addChild( &listBox );

	listBox.init( rects[2].globalX(), rects[2].globalY(), rects[2].width(), listBoxMaxHeight );
	listBox.setOrange(true);


	addChild( &expandButton );
	expandButton = src.expandButton;

	textLeft = src.textLeft;
	textTop = src.textTop;

	IsExpanded( 0 );

	return *this;
}


long aDropList::init( FitIniFile* file, const char* blockName )
{
	file->seekBlock( blockName );

	long x, y, width, height;
	x = y = width = height = 0;
	file->readIdLong( "XLocation", x );
	file->readIdLong( "YLocation", y );
	file->readIdLong( "Width", width );
	file->readIdLong( "Height", height );

	expandButton.init(*file, "ExpandButton");

	file->seekBlock( "Rects" );
	file->readIdLong( "Rectcount", rectCount );
	gosASSERT( rectCount > 1 );

	char tmpBlockName[64];
	if ( rectCount )
	{
		rects = new aRect[rectCount];
		for ( int i = 0; i < rectCount; i++ )
		{
			sprintf( tmpBlockName, "Rect%ld", i );
			rects[i].init( file, tmpBlockName );
		}
	}

	listBoxMaxHeight = rects[2].height();
	//rects[2].resize(rects[2].width(), rects[1].height());

	listBox.init( rects[2].globalX(), rects[2].globalY(), rects[2].width(), listBoxMaxHeight );

	rects[2].showGUIWindow(false);
	listBox.showGUIWindow(false);

	if (width < rects[1].width()) {
		width = rects[1].width();
	}
	if (height < rects[1].height()) {
		height = rects[1].height();
	}
	aObject::init(0, 0, width, height);

	textLeft = 0;
	textTop = 0;

	if ( NO_ERR == file->seekBlock( "Text0" ) )
	{
		file->readIdLong( "XLocation", textLeft );
		file->readIdLong( "YLocation", textTop );
	}

	addChild( &rects[1] );
	addChild( &listBox );
	addChild( &rects[2] );
	addChild( &expandButton );
	addChild( &rects[0] );

	expandButton.setPressFX( LOG_VIDEOBUTTONS );
	expandButton.setHighlightFX( LOG_DIGITALHIGHLIGHT );
	expandButton.setDisabledFX( LOG_WRONGBUTTON );

	expandButton.setMessageOnRelease();

	move(x, y);

	selectionIndex = -1;
	templateItem.init( *file );


	return 0;
}

void aDropList::destroy()
{
	removeAllChildren();
	expandButton.destroy();
	listBox.destroy(); // This deletes all the list items as well
	if ( rects )
	{
		delete [] rects;
		rects = NULL;
	}
	aObject::destroy();
}

void aDropList::specialDestroy()
{
	//Do NOT destroy the listBox.  Heidi's = operator just copies the pointer and base one is gone now.
	removeAllChildren();
	expandButton.destroy();
	if ( rects )
	{
		delete [] rects;
		rects = NULL;
	}
	aObject::destroy();
}

void aDropList::render()
{
	if ( showWindow )
	{
		for ( int i = 0; i < pNumberOfChildren; i++ )
		{
			pChildren[i]->render();
		}
		if (0 <= selectionIndex)
		{
			aListItem *pListItem = ListBox().GetItem(selectionIndex);
			if ( pListItem )
			{
				float l, t, w, h;
				l = pListItem->globalLeft();
				t = pListItem->globalTop();
				w = pListItem->width();
				h = pListItem->height();
				float availableWidth = (expandButton.globalLeft() - 1) - (globalLeft() + 1);
				if ((0 < availableWidth) && (w > availableWidth))
				{
					pListItem->resize(availableWidth, h);
				}
				bool bShowing = pListItem->isShowing();
				pListItem->moveTo( globalX() + textLeft, globalY() + textTop );
				pListItem->showGUIWindow( true );
				pListItem->render();
				pListItem->moveTo(l, t);
				pListItem->showGUIWindow( bShowing );
				if ((0 < availableWidth) && (w > availableWidth))
				{
					pListItem->resize(w, h);
				}
			}
		}
	}
}

void aDropList::IsExpanded(bool isExpanded)
{
	if (isExpanded)
	{
		listBox.showGUIWindow(true);
		listBox.SelectItem( -1 );
		rects[2].showGUIWindow(true);
	}
	else
	{
		listBox.showGUIWindow(false);
		rects[2].showGUIWindow(false);
	}
}

void aDropList::update()
{
	if ( showWindow )
	{
		aObject::update();
		if (userInput->isLeftClick())
		{
			int cx = userInput->getMouseX();
			int cy = userInput->getMouseY();

			if (expandButton.pointInside(cx, cy)) {
				handleMessage(aMSG_BUTTONCLICKED, &expandButton);
			}

			// lose focus if appropriate
			if ( ListBox().isShowing() && 
				( !pointInside( userInput->getMouseX(), userInput->getMouseY() ) 
				|| ( ListBox().pointInside(userInput->getMouseX(), userInput->getMouseY() )
					&& !ListBox().pointInScrollBar(userInput->getMouseX(), userInput->getMouseY() ) )  ) )
			{
				listBox.showGUIWindow(false);
				rects[2].showGUIWindow(false);
			}
		}
		if (0 <= ListBox().GetSelectedItem())
		{
			selectionIndex = ListBox().GetSelectedItem();
			if (listBox.isShowing())
			{
				IsExpanded(false);
			}
			ListBox().SelectItem(-1);
		}
		else if (listBox.isShowing())
		{
			if (userInput->getKeyDown( KEY_RETURN ) )
			{
				listBox.showGUIWindow(false);
				rects[2].showGUIWindow(false);

			}
		}

	}

	ListBox().setHelpID( helpID );
}


int aDropList::handleMessage( unsigned long message, void* who )
{
	{
		if (&expandButton == who) 
		{
			if (aMSG_BUTTONCLICKED == message)
			{
				if (listBox.isShowing())
				{
					IsExpanded(false);
				}
				else
				{
					IsExpanded(true);
				}
				return 1;
			}
		}
	}

	return 0;
}

bool aDropList::pointInside(long xPos, long yPos) const
{
	bool retval = aObject::pointInside(xPos, yPos);
	if (listBox.isShowing())
	{
		retval = retval || listBox.pointInside(xPos, yPos);
	}
	return retval;
}

long aDropList::AddItem(aListItem* itemString)
{
	long retval = ListBox().AddItem(itemString);
	float newHeight = (itemString->height() + ListBox().getSpaceBetweenItems()) * ListBox().GetItemCount()+4;
	if (newHeight > listBoxMaxHeight)
	{
		newHeight = listBoxMaxHeight;
	}
	ListBox().resize(ListBox().width(), newHeight);
	rects[2].resize(rects[2].width(), newHeight);
	return retval;
}

long				aDropList::AddItem( const char* text, unsigned long color )
{
	aAnimTextListItem* pItem = new aAnimTextListItem( 27333 );
	*pItem = templateItem;
	pItem->setText( text );
	pItem->setColor( color );
	return AddItem( pItem );
}

long aDropList::AddItem( unsigned long textID, unsigned long color )
{
		aAnimTextListItem* pItem = new aAnimTextListItem( 27333 );
		*pItem = templateItem;
		pItem->setText( textID );
		pItem->setColor( color );
		return AddItem( pItem );
}

long	aDropList::SelectItem(long item)
 { 
	listBox.showGUIWindow(false);
	rects[2].showGUIWindow(false);
	selectionIndex = item;
	return listBox.SelectItem(item);

 }



///////////////////////////////////////////////////////////////////////////////
// aComboBox methods
///////////////////////////////////////////////////////////////////////////////

aComboBox::aComboBox(void) : templateItem( 27333 )
{
	listBoxMaxHeight = 0.0;
	selectionIndex = -1;
	rects = NULL;
}

aComboBox& aComboBox::operator=(const aComboBox& src )
{
	aObject::operator =( src );

	

	//Toss the CURRENT listBox
	listBox.destroy();

	addChild( &listBox );


	//TOss the current rects.  We are about to NEW over them!!
	if ( rects )
	{
		delete [] rects;
		rects = NULL;
	}

	listBox = src.listBox;
	listBox.removeAllItems( 0 );
	listBoxMaxHeight = src.listBoxMaxHeight;
	selectionIndex = -1;

	rectCount = src.rectCount;
	if ( rectCount )
	{
		rects = new aRect[rectCount];
		for ( int i = 0; i < rectCount; i++  )
		{
			addChild( &rects[i] );
			rects[i] = src.rects[i];
		}
	}




	addChild( &expandButton );
	expandButton = src.expandButton;





	entry = src.entry;
	addChild( &entry );

	return *this;
}

long aComboBox::init( FitIniFile* file, const char* blockName )
{
	file->seekBlock( blockName );

	long x, y, width, height;
	x = y = width = height = 0;
	file->readIdLong( "XLocation", x );
	file->readIdLong( "YLocation", y );
	file->readIdLong( "Width", width );
	file->readIdLong( "Height", height );

	file->seekBlock( "Rects" );
	file->readIdLong( "Rectcount", rectCount );
	gosASSERT( rectCount > 1 );

	{
		char blockName[64];
		if ( rectCount )
		{
			rects = new aRect[rectCount];
			for ( int i = 0; i < rectCount; i++ )
			{
				sprintf( blockName, "Rect%ld", i );
				rects[i].init( file, blockName );
			}
		}
	}

	// editbox rect = first rect
	// drop list outline = 2nd rect
	entry.init(file, "EditBox");

	expandButton.init(*file, "ExpandButton");

	expandButton.setPressFX( LOG_VIDEOBUTTONS );
	expandButton.setHighlightFX( LOG_DIGITALHIGHLIGHT );
	expandButton.setDisabledFX( LOG_WRONGBUTTON );


	listBoxMaxHeight = rects[2].height();
	//rects[2].resize(rects[2].width(), entry.height());
	listBox.init( rects[2].globalX(), rects[2].globalY(), rects[2].width(), listBoxMaxHeight );

	rects[2].showGUIWindow(false);
	listBox.showGUIWindow(false);

	if (width < entry.width()) {
		width = entry.width();
	}
	if (height < entry.height()) {
		height = entry.height();
	}
	aObject::init(0, 0, width, height);

	addChild( &listBox );
	addChild( &entry );
	addChild( &expandButton );

	for ( int i = 0; i < rectCount; i++ )
		addChild( &rects[i] );

	templateItem.init( *file );
	expandButton.setMessageOnRelease();

	move(x, y);

	return 0;
}

long	aComboBox::SelectItem(long item)
 { 
	listBox.showGUIWindow(false);
	rects[2].showGUIWindow(false);
	selectionIndex = item;
	return listBox.SelectItem(item);

 }



void aComboBox::destroy()
{
	removeAllChildren();
	entry.destroy();
	expandButton.destroy();
	listBox.destroy(); // This deletes all the list items as well
	aObject::destroy();

	delete [] rects;
	rects = NULL;
}

void aComboBox::render()
{
	if ( showWindow )
	{
		for ( int i = 0; i < pNumberOfChildren; i++ )
		{
			pChildren[i]->render();
		}
	}
}

void aComboBox::update()
{
	if ( showWindow )
	{
		aObject::update();
		
		if (userInput->isLeftClick())
		{
			int cx = userInput->getMouseX();
			int cy = userInput->getMouseY();

			if (expandButton.pointInside(cx, cy)) {
				handleMessage(aMSG_BUTTONCLICKED, &expandButton);
			}

			// lose focus if appropriate
			else if ( ListBox().isShowing() && 
				( !pointInside( userInput->getMouseX(), userInput->getMouseY() )
				|| (  ListBox().pointInside(userInput->getMouseX(), userInput->getMouseY() ) )
				&& !ListBox().pointInScrollBar(userInput->getMouseX(), userInput->getMouseY()) ) )
			{
				listBox.showGUIWindow(false);
				rects[2].showGUIWindow(false);
			}
		}
		if (0 <= ListBox().GetSelectedItem())
		{
			selectionIndex = ListBox().GetSelectedItem();

			aListItem *pListItem = ListBox().GetItem(selectionIndex);
			aTextListItem *pTextListItem = dynamic_cast<aTextListItem *>(pListItem);
			if (0 != pTextListItem)
			{
				entry.setEntry(pTextListItem->getText());
			}
			if ( pListItem )
			{
				
				if (listBox.isShowing())
				{
					listBox.showGUIWindow(false);
					rects[2].showGUIWindow(false);
				}

				if ( getParent() )
					getParent()->handleMessage( aMSG_SELCHANGED, ID );
			}
			ListBox().SelectItem(-1);
		}
		else if (listBox.isShowing())
		{
			if (userInput->getKeyDown( KEY_RETURN ) )
			{
				listBox.showGUIWindow(false);
				rects[2].showGUIWindow(false);
			}
		}
	}

	ListBox().setHelpID( helpID );
}


int aComboBox::handleMessage( unsigned long message, const void* who )
{
	{
		if (&expandButton == who) 
		{
			if (aMSG_BUTTONCLICKED == message)
			{
				if (listBox.isShowing())
				{
					listBox.showGUIWindow(false);
					rects[2].showGUIWindow(false);
				}
				else
				{
					listBox.showGUIWindow(true);
					rects[2].showGUIWindow(true);
				}
				return 1;
			}
		}
	}

	return 0;
}

bool aComboBox::pointInside(long xPos, long yPos) const
{
	bool retval = aObject::pointInside(xPos, yPos);
	if (listBox.isShowing())
	{
		retval = retval || listBox.pointInside(xPos, yPos);
	}
	return retval;
}

long aComboBox::AddItem(aListItem* itemString)
{
	float newHeight = (itemString->height() + (float)ListBox().getSpaceBetweenItems()) * ((float)ListBox().GetItemCount()+1)+4;
	if (newHeight > listBoxMaxHeight)
	{
		newHeight = listBoxMaxHeight;
	}
	ListBox().resize(ListBox().width(), newHeight);
	rects[2].resize(rects[2].width(), newHeight);
	long retval = ListBox().AddItem(itemString);
	return retval;
}

long aComboBox::AddItem( unsigned long textID, unsigned long color )
{
		aAnimTextListItem* pItem = new aAnimTextListItem( 27333 );
		*pItem = templateItem;
		pItem->setText( textID );
		pItem->setColor( color );
		return AddItem( pItem );
}

long				aComboBox::AddItem( const char* text, unsigned long color )
{
	aAnimTextListItem* pItem = new aAnimTextListItem( 27333 );
	*pItem = templateItem;
	pItem->setText( text );
	pItem->setColor( color );
	return AddItem( pItem );
}



///////////////////////////////////////////////////////////////////////////////
// aTextListItem methods
///////////////////////////////////////////////////////////////////////////////
aTextListItem::aTextListItem( const aFont& newFont )
{
	font = newFont;
	DWORD height;
	DWORD width;
	font.getSize( width, height, "> ");
	aListItem::init( width, 0, Environment.screenWidth, ((float)height*1.25) );
	state = ENABLED;
	alignment = 0;
	bForceToTop = 0;
}
aTextListItem::aTextListItem(long newFontResID)
{
	font.init( newFontResID );
	DWORD height;
	DWORD width;
	font.getSize( width, height, "> ");
	aListItem::init( width, 0, Environment.screenWidth, ((float)height*1.25) );
	state = ENABLED;
	alignment = 0;
	bForceToTop = 0;
}

void aTextListItem::init(long newFontResID)
{
	font.init( newFontResID );
	DWORD height;
	DWORD width;
	font.getSize( width, height, "> ");
	aListItem::init( width, 0, Environment.screenWidth, ((float)height*1.25) );
	state = ENABLED;
	alignment = 0;
}

aTextListItem::~aTextListItem()
{
}

void		aTextListItem::setText( const char* newText )
{
	text = newText;
}
const char* aTextListItem::getText() const
{
	return text;
}

void aTextListItem::render()
{
	if ( !isShowing() )
		return;

	float y = location[2].y - location[0].y;
	float tmpHeight = font.height();
	if ( y > tmpHeight && font.height( text, width() ) <= tmpHeight
		&& !bForceToTop )
	{
		y = (location[2].y + location[0].y)/2.f - tmpHeight/2.f;
	}
	else
		y = location[0].y + tmpHeight/4.f;

	font.render( text, location[0].x, y,
		location[2].x - location[0].x, location[2].y - location[0].y, 
		location[0].argb, 0, alignment ); 
}

void aTextListItem::setText( long resID )
{
	char tmp[4096];
	tmp[0] = 0;
	cLoadString( resID, tmp, 4095 );
	text = tmp;
}

void aTextListItem::sizeToText()
{
	// assume the width is right and we need to make the height different
	unsigned long height = font.height( text, width() );
	resize( width(), height );

	
}

void aTextListItem::init( FitIniFile& file, const char* blockName )
{
	file.seekBlock( blockName );

	long x, y, width, height;

	file.readIdLong( "XLocation", x );
	file.readIdLong( "YLocation", y );
	file.readIdLong( "Width", width );
	file.readIdLong( "Height", height );

	aObject::init( x, y, width, height );

	long color;
	file.readIdLong( "Color", color );
	setColor( color );
}


//////////////////////////////////////////////////////////////////

aAnimTextListItem::aAnimTextListItem( const aAnimTextListItem& src )
{
	CopyData( src );
}
aAnimTextListItem& aAnimTextListItem::operator=( const aAnimTextListItem& src )
{
	if ( &src != this )
	{
		CopyData( src );
	}

	return *this;
}

void aAnimTextListItem::init( FitIniFile& file, const char* blockName )
{
	file.seekBlock( blockName );

	long x, y, width, height;

	file.readIdLong( "XLocation", x );
	file.readIdLong( "YLocation", y );
	file.readIdLong( "Width", width );
	file.readIdLong( "Height", height );

	aObject::init( x, y, width, height );

	long color;
	file.readIdLong( "Color", color );
	setColor( color );

	// I could read the font here

	char animationSt[256];
	if ( NO_ERR == file.readIdString( "Animation", animationSt, 255 ) )
		animInfo.init( &file, animationSt );
}


void aAnimTextListItem::CopyData( const aAnimTextListItem& src )
{
	animInfo = src.animInfo;
	font = src.font;
	text = src.text;
	state = src.state;

	aObject::operator=( src );
}

void aAnimTextListItem::render()
{
	animInfo.setState( (aAnimGroup::STATE)state );
	long color = animInfo.getCurrentColor( (aAnimGroup::STATE)state );
	setColor( color );

	aTextListItem::render();
	
}

void aAnimTextListItem::update()
{
	animInfo.update();

	aObject::update();
}

aLocalizedListItem::aLocalizedListItem() : aAnimTextListItem( 27663 )
{ 
}
long aLocalizedListItem::init( FitIniFile* file, const char* blockName )
{
	file->seekBlock( blockName );

	long x = 0;
	long y = 0;
	file->readIdLong( "XLocation", x );
	file->readIdLong( "YLocation", y );

	long fontResID = 0;
	file->readIdLong( "Font", fontResID );
	long textID = 0;
	file->readIdLong( "TextID", textID );
	aTextListItem::init(fontResID);
	setText(textID);
	long color = 0xff808080;
	file->readIdLong( "Color", color );

	char tmpStr[64];
	strcpy(tmpStr, "");
	file->readIdString( "Animation", tmpStr, 63 );
	if (0 != strcmp("", tmpStr))
	{
		animInfo.init(file, tmpStr);
	}

	moveTo(x, y);
	return 0;
}

void aLocalizedListItem::render()
{
	long color = 0xffffffff;
	if ( animInfo.getState() != getState() )
		animInfo.setState( (aAnimGroup::STATE)(long)getState() );
	color = animInfo.getCurrentColor( animInfo.getState() );
	aTextListItem::setColor((unsigned long)color);
	aTextListItem::render();
}




