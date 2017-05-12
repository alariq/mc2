#define MECHLISTBOX_CPP
/*************************************************************************************************\
MechListBox.cpp			: Implementation of the MechListBox component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mechlistbox.h"
#include"logisticsmech.h"
#include"paths.h"
#include"inifile.h"
#include"err.h"
#include"userinput.h"
#include"mechbayscreen.h"
#include"logisticsdata.h"
#include"mechpurchasescreen.h"
#include"gamesound.h"

MechListBoxItem* MechListBoxItem::s_templateItem = NULL;

bool MechListBox::s_DrawCBills = true;
bool MechListBoxItem::bAddCalledThisFrame = 0;



MechListBox::MechListBox( bool bDel, bool bInclude  )
{
	bDeleteIfNoInventory = bDel;
	bIncludeForceGroup = bInclude;
	bOrange = 0;
	skipAmount = 5;
}

//-------------------------------------------------------------------------------------------------

MechListBox::~MechListBox()
{
	removeAllItems( true );

	delete MechListBoxItem::s_templateItem;
	MechListBoxItem::s_templateItem = NULL;
}

void	MechListBox::setScrollBarOrange()
{
	scrollBar->setOrange();
}
void	MechListBox::setScrollBarGreen()
{
	scrollBar->setGreen();
}

void	MechListBox::drawCBills( bool bDraw )
{
	s_DrawCBills = bDraw;
}

void MechListBox::update()
{
	aListBox::update();
	MechListBoxItem::bAddCalledThisFrame = false;

	if ( bDeleteIfNoInventory )
	{
		for ( int i = 0; i < itemCount; i++ )
		{
			if ( ((MechListBoxItem*)items[i])->mechCount == 0 )
			{
				RemoveItem( items[i], true );
				i--;
				disableItemsThatCanNotGoInFG();


				// find better thing to select if necessary
				if ( itemSelected >= itemCount || itemSelected == -1 
					|| items[itemSelected]->getState() == aListItem::DISABLED
				//	|| !LogisticsData::instance->canAddMechToForceGroup( ((MechListBoxItem*)items[itemSelected])->getMech()  )
				)
				{
					if ( itemCount )
					{
						for ( int j = 0; j < itemCount; j++ )
							if ( items[j]->getState() != aListItem::DISABLED )
							{
								SelectItem( j );
								break;
							}
						
					}
					else
						itemSelected = -1;					
				}
				
			}
		}
	}
}

LogisticsMech* MechListBox::getCurrentMech()
{
	if ( itemSelected != -1 )
	{
		return ((MechListBoxItem*)items[itemSelected])->pMech;
	}

	return 0;
}



int MechListBox::init()
{
	if ( MechListBoxItem::s_templateItem )
		return 0;


	char path[256];
	strcpy( path, artPath );
	strcat( path, "mcl_gn_availablemechentry.fit" );
	FitIniFile file;
	if ( NO_ERR != file.open( path ) )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't open file %s", path );
		Assert( 0, 0, errorStr );
		return -1;
	}

	MechListBoxItem::init( file );
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
bool	MechListBoxItem::pointInside(long xPos, long yPos) const
{

	int minX = location[0].x + outline.globalX();
	int minY = location[0].y + outline.globalY();
	int maxX = location[0].x + outline.globalX() + outline.width();
	int maxY = location[0].y + outline.globalY() + outline.height();

	if ( minX < xPos && xPos < maxX
		&& minY < yPos && yPos < maxY )
		return true;

	return 0;
}
MechListBoxItem::MechListBoxItem( LogisticsMech* pRefMech, long count )
{
	
	state = ENABLED; // sebi: init so will not be garbage

	bIncludeForceGroup = 0;
	bOrange = 0;
	if ( s_templateItem )
	{
		*this = *s_templateItem;
	}

	animTime = 0.f;

	pMech = pRefMech;
	if ( !pMech )
		return;

	aObject::init( 0, outline.top(), outline.width(), outline.height() );
	setColor( 0, 0 );

	chassisName.setText( pMech->getChassisName() );
	
	char text[32];
	sprintf( text, "%ld", pMech->getCost() );
	costText.setText( text );

	mechCount = LogisticsData::instance->getVariantsInInventory( pRefMech->getVariant(), bIncludeForceGroup );
	sprintf( text, "%ld", mechCount );
	countText.setText( text );


	MechListBox::initIcon( pRefMech, mechIcon );

	variantName.setText( pMech->getName() );
	
	sprintf( text, "%.0lf", pMech->getMaxWeight() );
	weightText.setText( text );

	addChild( &weightIcon );
	addChild( &mechIcon );
	addChild( &costIcon );

	addChild( &chassisName );
	addChild( &weightText );
	addChild( &countText );
	addChild( &variantName );
	addChild( &costText );

	//	addChild( &line );
	//	addChild( &outline );

	bDim = 0;

}

MechListBoxItem::~MechListBoxItem()
{
	removeAllChildren( false );
}

void MechListBoxItem::init( FitIniFile& file )
{
	if ( !s_templateItem )
	{
		s_templateItem = new MechListBoxItem( NULL, 0 );
		file.seekBlock( "MainBox" );

		long width, height;

		file.readIdLong( "Width", width );
		file.readIdLong( "Height", height );

		((aObject*)s_templateItem)->init( 0, 0, width, height );

		memset( s_templateItem->animationIDs, 0, sizeof(long) * 9  );

		// rects
		s_templateItem->line.init( &file, "Rect1" );
		s_templateItem->outline.init( &file, "Rect0" );

		long curAnim = 0;
		// statics
		s_templateItem->weightIcon.init( &file, "Static0" );
		assignAnimation( file, curAnim );
	
		s_templateItem->mechIcon.init( &file, "Static1" );
		assignAnimation( file, curAnim );
		s_templateItem->costIcon.init( &file, "Static2" );
		assignAnimation( file, curAnim );

		// texts
		s_templateItem->chassisName.init( &file, "Text0" );
		assignAnimation( file, curAnim );
		s_templateItem->weightText.init( &file, "Text1" );
		assignAnimation( file, curAnim );
		s_templateItem->countText.init( &file, "Text2" );
		assignAnimation( file, curAnim );
		s_templateItem->variantName.init( &file, "Text3" );
		assignAnimation( file, curAnim );
		s_templateItem->costText.init( &file, "Text4" );
		assignAnimation( file, curAnim );

		char blockName[64];
		for ( int i = 0; i < 4; i++ )
		{
			sprintf( blockName, "OrangeAnimation%ld", i );
			s_templateItem->animations[1][i].init( &file, blockName );
			sprintf( blockName, "Animation%ld", i );
			s_templateItem->animations[0][i].init( &file, blockName );
		}
		
	}

}

void MechListBoxItem::assignAnimation( FitIniFile& file, long& curAnim )
{
	char tmpStr[64];

	s_templateItem->animationIDs[curAnim] = -1;
	if ( NO_ERR == file.readIdString( "Animation", tmpStr, 63 ) )
	{
		for ( int j = 0; j < strlen( tmpStr ); j++ )
		{
			if ( isdigit( tmpStr[j] ) )
			{
				tmpStr[j+1] = 0;
				s_templateItem->animationIDs[curAnim] = atoi( &tmpStr[j] );
			}
		}
	}
	curAnim++;
}
MechListBoxItem& MechListBoxItem::operator=( const MechListBoxItem& src )
{
	if ( &src != this )
	{
		chassisName = src.chassisName;
		costIcon = src.costIcon;
		costText = src.costText;
		line = src.line;
		mechIcon = src.mechIcon;
		outline = src.outline;
		variantName = src.variantName;
		weightIcon = src.weightIcon;
		weightText = src.weightText;
		countText = src.countText;
		for ( int i = 0; i < ANIMATION_COUNT; i++ )
		{
			animations[0][i] = src.animations[0][i];
			animations[1][i] = src.animations[1][i];
		}

		for (int i = 0; i < 9; i++ )
		{
			animationIDs[i] = src.animationIDs[i];
		}
	}

	return *this;
}

void MechListBoxItem::update()
{
	char text[32];
	int oldMechCount = mechCount;
	if ( !pMech )
	{
		mechCount = 0;
		return;
	}
	mechCount = LogisticsData::instance->getVariantsInInventory( pMech->getVariant(), bIncludeForceGroup );
	if ( oldMechCount != mechCount )
	{
		animTime = .0001f;
	}
	sprintf( text, "%ld", mechCount );
	countText.setText( text );
	if ( animTime )
	{
		if ( animTime < .25f 
			|| ( animTime > .5f && animTime <= .75f ) )
		{
			countText.setColor( 0 );
		}
		else 
			countText.setColor( 0xffffffff );

		animTime += frameLength;
		
		if ( animTime > 1.0f )
			animTime = 0.f;
		
	}

	bool isInside = pointInside( userInput->getMouseX(), userInput->getMouseY() );


	for ( int i = 0; i < ANIMATION_COUNT; i++ )
		animations[bOrange][i].update();

	if ( state == aListItem::SELECTED ) 
	{
		for ( int i = 0; i < ANIMATION_COUNT; i++ )
			animations[bOrange][i].setState( aAnimGroup::PRESSED );

	//	if ( userInput->isLeftClick() && isInside )
	//		setMech();
		
		if ( userInput->isLeftDrag() &&
			pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() ) )
			startDrag();

	}
	else if ( state == aListItem::HIGHLITE )
	{
		for ( int i = 0; i < ANIMATION_COUNT; i++ )
			animations[bOrange][i].setState( aAnimGroup::HIGHLIGHT );

	}
	else if ( state == aListItem::DISABLED &&  isShowing() )
	{
		if ( userInput->isLeftClick() && isInside )
		{
			soundSystem->playDigitalSample( LOG_WRONGBUTTON );	
			setMech(); // need to call explicitly
		}

		for ( int i = 0; i < ANIMATION_COUNT; i++ )
			animations[bOrange][i].setState( aAnimGroup::DISABLED );
	}
	else
	{
		for ( int i = 0; i < ANIMATION_COUNT; i++ )
			animations[bOrange][i].setState( aAnimGroup::NORMAL );
	}

	if ( userInput->isLeftDoubleClick() && isInside && state != aListItem::DISABLED && isShowing() )
		doAdd();

	aObject::update();
}



void MechListBoxItem::render()
{
	if ( !MechListBox::s_DrawCBills )
	{
		costText.showGUIWindow( 0 );
		costIcon.showGUIWindow( 0 );
	}
	else
	{
		costText.showGUIWindow( 1 );
		costIcon.showGUIWindow( 1 );

	}


	for ( int i = 0; i < this->pNumberOfChildren; i++ )
	{
		long index = animationIDs[i];
		if ( index != -1 )
		{
			if ( pChildren[i]->isShowing() )
			{
				if ( !animTime || pChildren[i] != &countText )
				{
					long color = animations[bOrange][index].getCurrentColor( animations[bOrange][index].getState());
					pChildren[i]->setColor( color );
				}

				
			}
		}
		pChildren[i]->render();

	}

	if ( bDim )
	{
		mechIcon.setColor( 0xa0000000 );
		mechIcon.render();
	}
		
	outline.setColor(animations[bOrange][2].getCurrentColor(animations[bOrange][2].getState()));
	outline.render( location[0].x, location[0].y );

	line.setColor(animations[bOrange][2].getCurrentColor(animations[bOrange][2].getState()));
	line.render(location[0].x, location[0].y);

}

void MechListBoxItem::setMech()
{
	MechBayScreen::instance()->setMech( pMech );
	MechPurchaseScreen::instance()->setMech( pMech, true );
	
}

void MechListBoxItem::startDrag()
{
	if ( state != DISABLED )
	{
		MechBayScreen::instance()->beginDrag( pMech );
		MechPurchaseScreen::instance()->beginDrag( pMech );
	}
}

void MechListBoxItem::doAdd()
{
	if ( !bAddCalledThisFrame ) // only select one, sometimes we auto scroll, don't want to be selecting each time
	{
		MechBayScreen::instance()->handleMessage( ID, MB_MSG_ADD );
		MechPurchaseScreen::instance()->handleMessage( ID, MB_MSG_ADD );
		bAddCalledThisFrame = true;
	}
}

void MechListBox::initIcon( LogisticsMech* pMech, aObject& mechIcon )
{
	mechIcon = (MechListBoxItem::s_templateItem->mechIcon);

	long index = pMech->getIconIndex();
	long xIndex = index % 10;
	long yIndex = index / 10;

	float fX = xIndex;
	float fY = yIndex;

	float width = mechIcon.width();
	float height = mechIcon.height();

	float u = (fX * width);
	float v = (fY * height);

	fX += 1.f;
	fY += 1.f;
	
	float u2 = (fX * width);
	float v2 = (fY * height);

	mechIcon.setFileWidth(256.f);
	mechIcon.setUVs( u, v, u2, v2 );

}

long MechListBox::AddItem(aListItem* itemString)
{
	itemString->setID( ID );
	MechListBoxItem* pItem = dynamic_cast<MechListBoxItem*>(itemString);
	EString addedName;
	char tmp[256];
	cLoadString( pItem->getMech()->getChassisName(), tmp, 255 );
	addedName = tmp;
	
	if ( pItem )
	{
		pItem->bOrange = bOrange;
		pItem->bIncludeForceGroup = bIncludeForceGroup;

		if ( !bDeleteIfNoInventory )
		{
			pItem->countText.setColor( 0 );
			pItem->countText.showGUIWindow( 0 );
		}
	
		EString chassisName;
		for ( int i = 0; i < itemCount; i++ )
		{

			long ID = ((MechListBoxItem*)items[i])->pMech->getChassisName();
			char tmpChassisName[256];
			cLoadString( ID, tmpChassisName, 255 );
			chassisName = tmpChassisName;
			if ( ((MechListBoxItem*)items[i])->pMech->getMaxWeight() < pItem->pMech->getMaxWeight() )
			{
				return InsertItem( itemString, i );
				break;
			}
			else if ( ((MechListBoxItem*)items[i])->pMech->getMaxWeight() == pItem->pMech->getMaxWeight()
				&& chassisName.Compare( addedName ) > 0 )
			{
				return InsertItem( itemString, i );
			}
			else if ( ((MechListBoxItem*)items[i])->pMech->getMaxWeight() == pItem->pMech->getMaxWeight()
				&& chassisName.Compare( addedName ) == 0 
				&& ((MechListBoxItem*)itemString)->pMech->getName().Find("Prime") != -1 )
			{
				return InsertItem( itemString, i );
			}
			else if ( ((MechListBoxItem*)items[i])->pMech->getMaxWeight() == pItem->pMech->getMaxWeight()
				&& chassisName.Compare( addedName ) == 0 
				&& ( ((MechListBoxItem*)items[i])->pMech->getName().Find("Prime" ) == -1 ) 
				&& ((MechListBoxItem*)items[i])->pMech->getName().Compare( pItem->pMech->getName() ) > 0 )
			{
				return InsertItem( itemString, i );
			}
		}

	}

	
	return aListBox::AddItem( itemString );
}

void	MechListBox::dimItem( LogisticsMech* pMech, bool bDim )
{
		for ( int i = 0; i < itemCount; i++ )
		{
			if ( ((MechListBoxItem*)items[i])->pMech == pMech )
			{
				
				((MechListBoxItem*)items[i])->bDim = bDim;	
			}
		}
		
}

void MechListBox::undimAll()
{
	for ( int i = 0; i < itemCount; i++ )
	{
			
			((MechListBoxItem*)items[i])->bDim = 0;	
		
	}
}

void MechListBox::disableItemsThatCostMoreThanRP()
{
	bool bDisabledSel = 0;
	for ( int i = 0; i < itemCount; i++ )
	{
		if ( ((MechListBoxItem*)items[i])->pMech->getCost() > LogisticsData::instance->getCBills() )
		{
			items[i]->setState( aListItem::DISABLED );
			if ( itemSelected == i )
				bDisabledSel = true;
		}
		else
		{
			if ( items[i]->getState() == aListItem::DISABLED )
				items[i]->setState( aListItem::ENABLED );
		}
	}

	if ( bDisabledSel )
	{
		for (int i = 0; i < itemCount; i++ )
		{
			if ( items[i]->getState() != aListItem::DISABLED )
			{
				SelectItem( i );
				bDisabledSel = 0;
				break;
			}
		}

		if ( bDisabledSel )
			SelectItem( -1 );
	}
}

void MechListBox::disableItemsThatCanNotGoInFG()
{
	bool bDisabledSel = 0;
	for ( int i = 0; i < itemCount; i++ )
	{
		if ( !LogisticsData::instance->canAddMechToForceGroup( ((MechListBoxItem*)items[i])->pMech ) )
		{
			if ( itemSelected == i )
				bDisabledSel = true;
			items[i]->setState( aListItem::DISABLED );
		}
		else
		{
			if ( items[i]->getState() == aListItem::DISABLED )
				items[i]->setState( aListItem::ENABLED );
		}
	}

	if ( bDisabledSel )
	{
		for (int i = 0; i < itemCount; i++ )
		{
			if ( items[i]->getState() != aListItem::DISABLED )
			{
				SelectItem( i );
				bDisabledSel = 0;
				break;
			}
		}

		if ( bDisabledSel )
			SelectItem( -1 );
	}
}

void MechListBox::setOrange( bool bNewOrange )
{
	bOrange = bNewOrange ? 1 : 0;

	for ( int i= 0; i < itemCount; i++ )
	{
		((MechListBoxItem*)items[i])->bOrange = bOrange;
	}

	if ( bNewOrange )
		scrollBar->setOrange( );
	else
		scrollBar->setGreen();

}




//*************************************************************************************************
// end of file ( MechListBox.cpp )
