/*************************************************************************************************\
MechBayScreen.cpp			: Implementation of the MechBayScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mechbayscreen.h"
#include"logisticsmech.h"
#include"logisticsdata.h"
#include"logisticsmechicon.h"
#include "../resource.h"
#include"simplecamera.h"
#include"windows.h"
#include"gamesound.h"
#include"multplyr.h"
#include"chatwindow.h"
#include"prefs.h"

MechBayScreen* MechBayScreen::s_instance = NULL;



MechBayScreen::MechBayScreen() : mechListBox( 1, 0 )
{
	pCurMech = NULL;
	pIcons = NULL;
	status = LogisticsScreen::RUNNING;
	s_instance = this;
	forceGroupCount = 0;
	mechCamera = new SimpleCamera;

	addWeightAmount = 0;
	removeWeightAmount = 0;
	pDragMech = 0;
	dragLeft = 0;

	helpTextArrayID = 13;

}

MechBayScreen::~MechBayScreen()
{
	if ( mechCamera )
		delete mechCamera;

	if ( pIcons )
		delete[] pIcons;

	mechListBox.destroy();
}
void MechBayScreen::init(FitIniFile* file)
{
	// init button, texts, statics, rects
	LogisticsScreen::init( *file, "MechBayStatic", "MechBayTextEntry", "MechBayRect", "MechBayButton" );
	
	// initialize the list box
	mechListBox.init();
	mechListBox.setHelpID( IDS_HELP_MECHSTORAGE );

	loadoutListBox.init( rects[3].left(), rects[3].top(), rects[3].width(), rects[3].height() );

	// fill the mech list box
	((aObject*)&mechListBox)->init( rects[0].left(), rects[0].top(), rects[0].width(), rects[0].height() );

	// initialize little icons
	FitIniFile iconFile;
	char path[256];
	strcpy( path, artPath );
	strcat( path, "mcl_gn_deploymentteams.fit" );
	strlwr( path );
	if ( NO_ERR != iconFile.open( path ) )
	{
		gosASSERT( "couldn't open the MCL_GN_DevploymentTeams file" );
		return;
	}
	LogisticsMechIcon::init(iconFile);
	pIcons = new LogisticsMechIcon[ICON_COUNT];

	int count = 0;

	
	long x = 0;
	long y = 0;

	for ( int j = 0; j < ICON_COUNT_Y; j++ )
	{
        int i = 0;
		for (; i < ICON_COUNT_X; i++ )
		{
			pIcons[count].setMech( 0 );
			pIcons[count].move( x, y );
			x+= pIcons[count].width();
			x+=4;

			count++;
		}

		y+= pIcons[i].height();
		y += 5;
		x = 0;
	}
	
	mechListBox.drawCBills( 0 );
	mechListBox.setScrollBarOrange();


	// initialize the attribute meeters
	attributeMeters[0].init( file, "AttributeMeter0" );
	attributeMeters[1].init( file, "AttributeMeter1" );
	attributeMeters[2].init( file, "AttributeMeter2" );


	dropWeightMeter.init( file, "DropWeightMeter" );
	file->readIdLong( "RayCenterX", weightCenterX );
	file->readIdLong( "RayCenterY", weightCenterY );
	file->readIdLong( "RayStartColor ", weightStartColor );
	file->readIdLong( "RayEndColor ", weightEndColor );

	file->seekBlock( "weightAddAnimation" );
	addWeightAnim.init( file, "" );

	file->seekBlock( "weightRemoveAnimation" );
	removeWeightAnim.init( file, "" );

	file->seekBlock( "FRectAnim" );

	mechCamera->init( rects[1].left(), rects[1].top(), rects[2].right(), rects[1].bottom() );

	
	
	for ( int i= 0; i < buttonCount; i++ )
		buttons[i].setMessageOnRelease();


	textObjects[7].setText( "" );
	textObjects[8].setText( "" );
	textObjects[9].setText( "" );
	textObjects[10].setText( "" );
	textObjects[11].setText( "" );
	textObjects[12].setText( "" );
	textObjects[14].setText( "" );
	for (int i = 0; i < 3; i++ )
	{
		attributeMeters[i].setValue(0);
	}
	setMech( NULL, 0 );

}

void MechBayScreen::begin()
{
	status = RUNNING;

	pDragMech = NULL;

	mechListBox.removeAllItems( true );
	reinitMechs();	

	int mechCount[256];
	memset( mechCount, 0, sizeof ( int ) * 256 );

	bool bCurMechIsValid = 0;

	// initialize both the inventory and icon lists
	EList< LogisticsMech*, LogisticsMech* > mechList;
	LogisticsData::instance->getInventory( mechList );
	
	for ( EList< LogisticsMech*, LogisticsMech* >::EIterator iter = mechList.Begin();
		!iter.IsDone(); iter++ )
		{
			if ( *iter == pCurMech )
				bCurMechIsValid = true;

			if ( !(*iter)->getForceGroup() )
			{
				bool bFound = 0;

				for ( int i = 0; i < mechListBox.GetItemCount(); i++ )
				{
					if ( ((MechListBoxItem*)mechListBox.GetItem(i))->getMech()->getVariant()
						== (*iter)->getVariant() )
						bFound = true;
				}

				if ( !bFound )
				{
					MechListBoxItem* item = new MechListBoxItem( (*iter), 1 );
					mechListBox.AddItem( item );
				}

			}
		}

	
	
	// reset the old mech to NULL to make sure everything gets set
	
	mechListBox.drawCBills( 0 );
	mechListBox.setOrange( true );

	if ( !pCurMech || !bCurMechIsValid )
	{
		if ( !selectFirstFGItem() )
		{
			if ( !selectFirstViableLBMech() )
				setMech( NULL );
		}
	}
	else
	{
		LogisticsMech* pMech = pCurMech;
		setMech( NULL ); // need to make sure the guy is still running
		setMech( pMech );

		bool bFound = 0;
		for ( int i = 0; i < mechListBox.GetItemCount(); i++ )
		{
			MechListBoxItem* pItem = (MechListBoxItem*)mechListBox.GetItem( i );
			if ( pItem && pItem->getMech() == pCurMech )
			{
				mechListBox.SelectItem( i );
				bFound = true;
			}
		}

		if ( !bFound )
		{
			for ( int i = 0; i < ICON_COUNT; i++ )
			{
				if ( pIcons[i].getMech() == pMech )
				{
					pIcons[i].select( true );
					break;
				}
			}
		}
	}
	
}
void MechBayScreen::render(int xOffset, int yOffset)
{
	mechListBox.move( xOffset, yOffset );
	mechListBox.render();
	mechListBox.move( -xOffset, -yOffset );

	loadoutListBox.move( xOffset, yOffset );
	loadoutListBox.render();
	loadoutListBox.move( -xOffset, -yOffset );

	if ( !xOffset && !yOffset && pCurMech)
		mechCamera->render();



	for ( int i = 0; i < 3; i++ )
	{
		attributeMeters[i].render(xOffset, yOffset);
	}



	drawWeightMeter(xOffset, yOffset);

	LogisticsScreen::render(xOffset, yOffset);

	for (int i = 0; i < ICON_COUNT; i++ )
	{
		pIcons[i].render( xOffset, yOffset );
	}

	if ( MPlayer && ChatWindow::instance() )
		ChatWindow::instance()->render(xOffset, yOffset);




	if ( pDragMech )
		dragIcon.render();


}

void MechBayScreen::drawWeightMeter(long xOffset, long yOffset)
{
		// there are 20 rays, each 9 degrees
	// they should be 50 pixels long
	float currentDropWeight = LogisticsData::instance->getCurrentDropWeight();
	float maxDropWeight = LogisticsData::instance->getMaxDropWeight();
	long numAddBars = 0;
	long numRemoveBars = 0;

	long addColor = 0;
	long removeColor = 0;		

	long numberOfBars = 20;
	if ( maxDropWeight != 0 )
	{
		float percent = currentDropWeight/maxDropWeight;
		percent *= 20.f;
		numberOfBars = percent + .5;
	}

	if ( addWeightAnim.isAnimating() && !addWeightAnim.isDone() )
	{
		addColor = addWeightAnim.getColor();
		float percent = (currentDropWeight - addWeightAmount) / maxDropWeight;
		percent *= 20.f;
		numAddBars = percent + .5;
		numAddBars = numberOfBars - numAddBars;
	}

	if ( removeWeightAnim.isAnimating() && !removeWeightAnim.isDone() )
	{
		addColor = removeWeightAnim.getColor();
		float percent = removeWeightAmount / maxDropWeight;
		percent *= 20.f;
		numRemoveBars = percent + .5;
	}

	gos_VERTEX v[3];
	for ( int i = 0; i <3; i++ )
	{
		v[i].u = 0.f;
		v[i].v = 0.f;
		v[i].rhw = .5;
		v[i].z = 0.f;
		v[i].argb = 0;
		v[i].frgb = 0;
		v[i].x = weightCenterX + xOffset;
		v[i].y = weightCenterY + yOffset;
	}
	 
	float curArc = 180.f;
	float curX = 50.f * cos( DEGREES_TO_RADS * curArc );
	float curY = 50.f * sin( DEGREES_TO_RADS * curArc );
	for (int i= 0; i < numberOfBars + numRemoveBars; i++ )
	{

		v[1].x = curX + weightCenterX + xOffset;
		v[1].y = curY + weightCenterY + yOffset;

		curArc += 9.f;
		curX = 50.f * cos( DEGREES_TO_RADS * curArc );
		curY = 50.f * sin( DEGREES_TO_RADS * curArc );

		v[2].x = curX + weightCenterX + xOffset;
		v[2].y = curY + weightCenterY + yOffset;

		if ( i >= numberOfBars - numAddBars )
			v[0].argb = v[1].argb = v[2].argb = addColor;
		else if ( i >= numberOfBars )
			v[0].argb = v[1].argb = v[2].argb = removeColor;
		else
			v[0].argb = v[1].argb = v[2].argb = interpolateColor( weightStartColor, weightEndColor, i * .05 );

		gos_DrawTriangles( v, 3 );
		

	}


	dropWeightMeter.render(xOffset, yOffset);
}

void MechBayScreen::update()
{

	mechListBox.disableItemsThatCanNotGoInFG();

	if ( !pIcons[0].getMech() )
		getButton( MB_MSG_REMOVE )->disable( true );

	// update current text
	char str[64];

	// RP
	sprintf( str, "%ld ", LogisticsData::instance->getCBills() );
	textObjects[1].setText( str );

	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();
	if ( userInput->isLeftClick() && dropWeightMeter.pointInside( mouseX, mouseY ) )
	{
		soundSystem->playDigitalSample( LOG_WRONGBUTTON );
	}
	
	// current drop weight
	char tmpStr[128];
	cLoadString( IDS_MB_WEIGHT, tmpStr, 63 );
	long currentDropWeight = LogisticsData::instance->getCurrentDropWeight();
	long maxDropWeight = LogisticsData::instance->getMaxDropWeight();

	sprintf( str, tmpStr, currentDropWeight, maxDropWeight );
	textObjects[6].setText( str );


	// update drag and drop
	// must be done before Icon's are updated, or we can get out of sync
	if ( pDragMech )
	{
		dragIcon.moveTo( userInput->getMouseX() - dragIcon.width() / 2, 
			userInput->getMouseY() - dragIcon.height() / 2 );

		bool bLeft = 0;
		bool bRight = 0;
		
		GUI_RECT testRect;
		testRect.left = mechListBox.globalX();
		testRect.right = mechListBox.globalX() + mechListBox.width();
		testRect.top = mechListBox.globalY();
		testRect.bottom = mechListBox.globalY() + mechListBox.height();


		if ( dragIcon.rectIntersect( testRect ) )
		{
			// set the focus
		}	bRight = true;

		testRect.left = pIcons[0].globalX();
		testRect.right = pIcons[ICON_COUNT - 1].globalX() + pIcons[0].width();
		testRect.top = pIcons[0].globalY();
		testRect.bottom = pIcons[ICON_COUNT-1].globalY() + pIcons[0].height();

		if ( dragIcon.rectIntersect( testRect ) )
		{
			bLeft = true;
		}
		else 
			bLeft = 0;

		if ( userInput->leftMouseReleased() || userInput->isLeftDoubleClick() )
		{
			if ( dragLeft )
			{
				if ( bLeft )
				{
					LogisticsData::instance->removeMechFromForceGroup( pDragMech, false );
					LogisticsData::instance->addMechToForceGroup( pDragMech, forceGroupCount+1 );
					forceGroupCount++;

					reinitMechs();
				}
				else
				{
					if ( 0 == LogisticsData::instance->getVariantsInInventory( pDragMech->getVariant(), 0 ) )
					{ 
						MechListBoxItem* item = new MechListBoxItem( pDragMech, 0 );
						int index = mechListBox.AddItem( item );
						mechListBox.SelectItem( index );
					}
					LogisticsData::instance->removeMechFromForceGroup( pDragMech, true );
					forceGroupCount--;
					reinitMechs();

					removeWeightAmount = pDragMech->getMaxWeight();
					removeWeightAnim.begin();
					addWeightAnim.end();
				}
			}
			else
			{
				if ( bLeft )
				{
					reinitMechs();
					addWeightAmount = pDragMech->getMaxWeight();
					addWeightAnim.begin();
					removeWeightAnim.end();
		
				}
				else
				{
					if ( 0 == LogisticsData::instance->getVariantsInInventory( pDragMech->getVariant(), 0 ) )
					{
						MechListBoxItem* item = new MechListBoxItem( pDragMech, 0 );
						int index = mechListBox.AddItem( item );
						mechListBox.SelectItem( index );
					}
					LogisticsData::instance->removeMechFromForceGroup( pDragMech, true );
					reinitMechs();

				}
			}
		
			pDragMech = 0;
		}

	}


	// update icon selection
	int newSel = -1;
	int oldSel = -1;
	int removeMech = -1;
	for ( int i = 0; i < ICON_COUNT; i++ )
	{
		pIcons[i].update();
		if ( pIcons[i].justSelected() )
			newSel = i;
		else if ( pIcons[i].isSelected() )
			oldSel = i;

		bool bInside = pIcons[i].pointInside( userInput->getMouseX(), userInput->getMouseY() );

		if ( bInside && userInput->isLeftDoubleClick() && pIcons[i].getMech() )
		{
			 removeMech = i;
		}
		else if ( userInput->isLeftDrag() 
			&& pIcons[i].pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() ) )
		{
			beginDrag( pIcons[i].getMech() );
		}
	}

	if ( newSel != -1 )
	{
		if ( oldSel != -1 )
			pIcons[oldSel].select( 0 );
		mechListBox.SelectItem( -1 );
		
	}
	if ( removeMech != -1 )
	{
		 removeSelectedMech();
	}


	if ( !MPlayer || !ChatWindow::instance()->pointInside(userInput->getMouseX(), userInput->getMouseY()) )
		LogisticsScreen::update();
	
	if ( newSel == -1 && oldSel == -1 )
	{
		// disable the remove button
		getButton( MB_MSG_REMOVE )->disable(1);
	}
	else
		getButton( MB_MSG_REMOVE )->disable(0);

	
	// update list box, 
	oldSel = mechListBox.GetSelectedItem();
	mechListBox.update();
	newSel = mechListBox.GetSelectedItem( );

	// did anything change?
	if ( oldSel != newSel && newSel != -1)
	{
		unselectDeploymentTeam();
	}

	// component list box
	loadoutListBox.update();	

	addWeightAnim.update();
	removeWeightAnim.update();

	LogisticsMech* pTmp = mechListBox.getCurrentMech();
	if ( !pTmp )
	{
		pTmp = getFGSelMech();
	}

	if ( pTmp )
		setMech( pTmp, 0);

	// update buttons!
	if ( newSel == -1 || !LogisticsData::instance->canAddMechToForceGroup( mechListBox.getCurrentMech() ) )
	{
		// disable the add button
		getButton( MB_MSG_ADD )->disable(1);
	}
	else
		getButton( MB_MSG_ADD )->disable(0);

		

	if ( MPlayer && ( !MPlayer->missionSettings.variants || !pCurMech ) )
	{
		getButton( MB_MSG_CHANGE_LOADOUT )->disable( true );
	}
	else if ( LogisticsData::instance->skipPurchasing() )
	{
		getButton( MB_MSG_CHANGE_LOADOUT )->disable( true );
		getButton( MB_MSG_BUY )->disable( true );
	}
	else 
	{
		getButton( MB_MSG_CHANGE_LOADOUT )->disable( false );
		getButton( MB_MSG_BUY )->disable( false );

		if ( !pCurMech )
			getButton( MB_MSG_CHANGE_LOADOUT )->disable( true );
		else
		{
			getButton( MB_MSG_CHANGE_LOADOUT )->disable( false );
			mechCamera->update();
		}
	}





	if ( MPlayer && ChatWindow::instance() )
		ChatWindow::instance()->update();



}

int	MechBayScreen::handleMessage( unsigned long message, unsigned long who )
{
	if ( status != RUNNING )
		return 0;

	switch( who )
	{
	case MB_MSG_NEXT:
		status = NEXT;
		break;

	case MB_MSG_PREV:
		status = PREVIOUS;
		break;

	case MB_MSG_ADD:
		
		addSelectedMech();
		soundSystem->playDigitalSample( LOG_SELECT );		
		break;

	case MB_MSG_REMOVE:
		removeSelectedMech();
		break;

	case MB_MSG_BUY:
		status = UP;
		break;

	case MB_MSG_CHANGE_LOADOUT:
		status = DOWN;
		if ( pCurMech )
		{
			LogisticsData::instance->setMechToModify( pCurMech );
		}
		break;

	case MB_MSG_BUY_SELL:
		break;

	case MB_MSG_MAINMENU:
		status = MAINMENU;
		break;
	}


	return 0;
}

void MechBayScreen::addSelectedMech()
{
	if ( mechListBox.getCurrentMech() )
	{
		if ( !LogisticsData::instance->canAddMechToForceGroup( mechListBox.getCurrentMech() ) )
			return;

		if ( NO_ERR != LogisticsData::instance->addMechToForceGroup( mechListBox.getCurrentMech(),
			forceGroupCount + 1 ) )
		{
			// its already in the force group, add probably called 2x'2
			return;
		}

		if ( forceGroupCount > 11 )
			return;

		unselectDeploymentTeam();
		forceGroupCount++;
		reinitMechs();

		addWeightAmount = pCurMech->getMaxWeight();
		addWeightAnim.begin();
		removeWeightAnim.end();

		bool bFound = 0;
		for ( int i = 0; i < mechListBox.GetItemCount(); i++ )
		{
			if ( LogisticsData::instance->canAddMechToForceGroup( 
				((MechListBoxItem*)mechListBox.GetItem( i ))->getMech() ) )
			{
				bFound = true;
			}
		}

		if ( !bFound ) // select first thing in FG
		{
			selectFirstFGItem();
			mechListBox.SelectItem( -1 );
		}
	}

}

void MechBayScreen::removeSelectedMech()
{
    int i = 0;
	for (i = 0; i < ICON_COUNT; i++ )
	{
		if ( pIcons[i].isSelected() )
		{
			LogisticsMech* pMech = pIcons[i].getMech();
			if ( !pMech )
				return;
			if ( 0 == LogisticsData::instance->getVariantsInInventory( pMech->getVariant(), 0 ) )
			{
				MechListBoxItem* item = new MechListBoxItem( pMech, 0 );
				mechListBox.AddItem( item );
			}

			LogisticsData::instance->removeMechFromForceGroup( pMech, true );	
			
			forceGroupCount--;
			removeWeightAmount = pMech->getMaxWeight();
			removeWeightAnim.begin();
			addWeightAnim.end();
			if ( i >= forceGroupCount )
			{
				if ( !selectFirstFGItem() )
				{
					selectFirstViableLBMech();
				}
			}
			else
				pIcons[i].select( true );
			break;
		}
	}
	
	for ( ; i < ICON_COUNT - 1; i++ )
	{
		LogisticsData::instance->removeMechFromForceGroup(pIcons[i+1].getMech(), false);		
		LogisticsData::instance->addMechToForceGroup(pIcons[i+1].getMech(), i+1);
	}

	reinitMechs();

	if ( !forceGroupCount )
		selectFirstViableLBMech();


}


void MechBayScreen::end()
{
	mechCamera->setMech( NULL );
}

void MechBayScreen::setMech( LogisticsMech* pMech,  bool bCommandFromLB )
{

	if ( pMech == pCurMech || status != RUNNING)
		return;

	pCurMech = pMech;
	if ( pMech )
	{
		loadoutListBox.setMech( pMech->getVariant() );

		EString fileName = pMech->getFileName();
		int index = fileName.Find( '.' );
		fileName = fileName.Left( index );
		index = fileName.ReverseFind( '\\' );
		fileName = fileName.Right( fileName.Length() - index - 1 );
		mechCamera->setMech( fileName, prefs.baseColor, prefs.highlightColor, prefs.highlightColor );
		mechCamera->setScale( pMech->getVariant()->getChassis()->getScale() );
	}
	else
		loadoutListBox.setMech( 0 );

	if ( bCommandFromLB ) // clicked on a disabled mech
	{
		unselectDeploymentTeam();
		mechListBox.SelectItem( -1 );
	}

	if ( pCurMech )
	{
		char tmpStr[256];

		// update current text
		char str[64];

		// weight
		cLoadString( IDS_MB_MECH_WEIGHT, tmpStr, 63 );
		sprintf( str, tmpStr, pCurMech->getMaxWeight(), (const char*)pCurMech->getMechClass() );
		textObjects[7].setText( str );

		// firing range
		long tmpColor;
		textObjects[14].setText( pCurMech->getVariant()->getOptimalRangeString( tmpColor ) );
		textObjects[14].setColor( tmpColor );

		// armor
		int armor = pCurMech->getArmor();
		sprintf( str, "%ld", armor );
		textObjects[8].setText( str );
		attributeMeters[0].setValue( armor/MAX_ARMOR_RANGE );

		// speed
		int speed = pCurMech->getDisplaySpeed();
		sprintf( str, "%ld", speed );
		textObjects[9].setText( str );
		attributeMeters[1].setValue( pCurMech->getSpeed()/MAX_SPEED_RANGE );


		// jump
		int jumpRange = pCurMech->getJumpRange();
		sprintf( str, "%ld", jumpRange * 25 );
		textObjects[10].setText( str );
		attributeMeters[2].setValue( jumpRange * 25 / MAX_JUMP_RANGE );

		textObjects[11].setText( pCurMech->getChassisName() );
		textObjects[12].setText( pCurMech->getName() );
	}
	else
	{
		textObjects[7].setText( "" );
		textObjects[8].setText( "" );
		textObjects[9].setText( "" );
		textObjects[10].setText( "" );
		textObjects[11].setText( "" );
		textObjects[12].setText( "" );
		textObjects[14].setText( "" );
		for ( int i = 0; i < 3; i++ )
		{
			attributeMeters[i].setValue(0);
		}

	}
}

void MechBayScreen::beginDrag( LogisticsMech* pMech )
{	
	if ( pDragMech || status != RUNNING )
		return;

	if ( !pMech )
		return;


	// need to set the uv's of the mech icon
	MechListBox::initIcon( pMech, dragIcon );

	dragIcon.moveTo( userInput->getMouseX() - dragIcon.width() / 2, 
			userInput->getMouseY() - dragIcon.height() / 2 );


	pDragMech = pMech;

	// figure out where the mech came from 
	if ( mechListBox.pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() ) )
	{
		if ( pMech->getForceGroup() )
		{
			pMech = LogisticsData::instance->getMechWithoutForceGroup( pMech );
		}
		pDragMech = pMech;


		if ( !LogisticsData::instance->canAddMechToForceGroup( pMech ) )
		{
			pDragMech = 0;
			return;
		}

		LogisticsData::instance->addMechToForceGroup( pMech, forceGroupCount+1 );
		dragLeft = 0;
		
	}
	else
	{
		for ( int i = 0; i < ICON_COUNT; i++ )
		{
			if ( pIcons[i].pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() ) )
			{
				forceGroupCount--;

				// select the next one
				if ( i == forceGroupCount && forceGroupCount )
				{
					pIcons[i].select( 0 );
					pIcons[0].select( true );
				}
				else if ( !forceGroupCount) 
					pIcons[i].select( 0 );
				
				for ( ; i < ICON_COUNT - 1; i++ )
				{		
					LogisticsData::instance->removeMechFromForceGroup( pIcons[i+1].getMech(), false );
					LogisticsData::instance->addMechToForceGroup( pIcons[i+1].getMech(), i+1 );
					pIcons[i].setMech( pIcons[i+1].getMech() );
					pIcons[i].setPilot( pIcons[i+1].getPilot() );
				}

				if ( i < ICON_COUNT )
				{
					pIcons[i].setMech( NULL );
					pIcons[i].setPilot( NULL );
				}


				break;
			}

		}
		dragLeft = 1;
	}
	
}

void MechBayScreen::reinitMechs()
{
	
	int count = 0;

	
	int maxUnits = 12;

	if ( MPlayer )
	{
		long playerCount;
		MPlayer->getPlayers( playerCount );
		if ( playerCount )
			maxUnits = MAX_MULTIPLAYER_MECHS_IN_LOGISTICS/playerCount;

		if ( maxUnits > 12 )
			maxUnits = 12;
	}

	for ( int j = 0; j < ICON_COUNT_Y; j++ )
	{
		for ( int i = 0; i < ICON_COUNT_X; i++ )
		{
			pIcons[count].setMech( 0 );
			if ( count >= maxUnits )
				pIcons[count].disable( true );
			else
				pIcons[count].disable( 0 );

			count++;
		}
	}


	forceGroupCount = 0;

	// initialize both the inventory and icon lists
	EList< LogisticsMech*, LogisticsMech* > mechList;
	LogisticsData::instance->getInventory( mechList );
	
	
	for ( EList< LogisticsMech*, LogisticsMech* >::EIterator iter = mechList.Begin();
		!iter.IsDone(); iter++ )
		{
			if ( (*iter)->getForceGroup() )
			{
				long FG = (*iter)->getForceGroup();

				if ( FG <= maxUnits )
				{
				
					pIcons[FG-1].setMech( (*iter) );
					pIcons[FG-1].setPilot( (*iter)->getPilot() );
					forceGroupCount++;
				}
				else 
					(*iter)->setForceGroup( 0 );
				
			}
		}
}

void	MechBayScreen::unselectDeploymentTeam()
{
	int count = 0;
	for ( int j = 0; j < ICON_COUNT_Y; j++ )
	{
		for ( int i = 0; i < ICON_COUNT_X; i++ )
		{
			pIcons[count].select( 0 );
			count++;
		}
	}
}

bool	MechBayScreen::selectFirstFGItem()
{
	bool bRetVal = false;
	for ( int i = 0; i < ICON_COUNT_X * ICON_COUNT_Y; i++ )
	{
		if ( pIcons[i].getMech() && !bRetVal )
		{
			pIcons[i].select( true );
			bRetVal = true;
		}
		else
			pIcons[i].select( false );
	}

	return bRetVal;
}
bool	MechBayScreen::selectFirstViableLBMech()
{
	for ( int i = 0; i < mechListBox.GetItemCount(); i++ )
	{
		if ( mechListBox.GetItem( i )->getState() != aListItem::DISABLED )
		{
			mechListBox.SelectItem( i );
			return true;
		}
	}

	return 0;
}

LogisticsMech* MechBayScreen::getFGSelMech()
{
	for ( int i = 0; i < ICON_COUNT_X * ICON_COUNT_Y; i++ )
	{
		if ( pIcons[i].getMech() && pIcons[i].isSelected()  )
			return pIcons[i].getMech();
	}

	return NULL;
}

