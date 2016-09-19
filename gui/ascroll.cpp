//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mclib.h"

#ifndef ASCROLL_H
#include"ascroll.h"
#endif



aScrollBar::aScrollBar()
{
	scrollMax = 0;
	scrollPos = 0;
	lastY = 0;
}

long aScrollBar::init(long xPos, long yPos, long w, long h)
{
	long err;
	
	err = aObject::init(xPos,yPos,w,h);
	if (err)
		return err;

	topButton.setID( aMSG_SCROLLUP );
	bottomButton.setID( aMSG_SCROLLDOWN );
	scrollTab.setSinglePress();
	topButton.setSinglePress();
	bottomButton.setSinglePress();
	addChild( &topButton );
	addChild( &bottomButton );
	addChild( &scrollTab );

	topButton.setHoldTime( 0.001f );
	scrollTab.setHoldTime( 0.001f );
	bottomButton.setHoldTime( 0.001f );

	topButton.setPressFX( -1 );
	bottomButton.setPressFX( -1 );
	scrollTab.setPressFX( -1 );

	topButton.setHighlightFX( -1 );
	bottomButton.setHighlightFX( -1 );
	scrollTab.setHighlightFX( -1 );

	topButton.setDisabledFX( -1 );
	bottomButton.setDisabledFX( -1 );
	scrollTab.setDisabledFX( -1 );

	scrollTab.setTexture( (unsigned long)0 );

	moveTo(xPos,yPos);

	SetScrollMax(0);
	SetScrollPos(0);

	scrollInc = 1;
	pageInc = 10;

	scrollTab.lightEdge = 0xff005392;
	scrollTab.darkEdge = 0xff002D51;
	scrollTab.regularColor =0xff004275;

	return (NO_ERR);
}

void aScrollBar::destroy()
{
	aObject::destroy();
}
	

void aScrollBar::render()
{

	topButton.moveTo( globalX() + 2, topButton.globalY() );
	bottomButton.moveTo( globalX() + 2, bottomButton.globalY() );
	scrollTab.moveTo( globalX() + 2, scrollTab.globalY() );

	aObject::render();
 	GUI_RECT area = { location[0].x, location[0].y, location[2].x, location[2].y };
	drawEmptyRect( area, color, color );
}

void aScrollBar::SetScrollMax(float newMax)
{
	scrollMax = newMax;
	scrollTab.showGUIWindow(newMax != 0);
	ResizeAreas();
}	

void aScrollBar::SetScrollPos(float newPos)
{
	if (newPos < 0)
		newPos = 0;
	if (newPos > scrollMax)
		newPos = scrollMax;
	scrollPos = newPos;
	ResizeAreas();	
	
}

void aScrollBar::SetScroll( long newScrollPos )
{
	if ( newScrollPos < 0 )
		newScrollPos = 0;

	if ( newScrollPos > scrollMax )
		newScrollPos = scrollMax;

	if ( getParent() )
		getParent()->handleMessage( aMSG_SCROLLTO, newScrollPos );
	
	SetScrollPos( newScrollPos );

}
void aScrollBar::update()
{
	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();

		if ( userInput->isLeftDrag() && lastY ) // dragging the little tab
		{
			int tmpLastY = mouseY; 
			tmpLastY -= userInput->getMouseDragY();
			tmpLastY += lastY;

			float finalPos = (float)tmpLastY;
			// figure out what this translates to
			float physicalRange = height() - topButton.height() - bottomButton.height() - scrollTab.height() -  2.f;
			float RealRange = scrollMax;
			if ( !physicalRange )
				physicalRange = RealRange;

			//Check for what if both of the above are zero.  Probably nothing to scroll to, eh?
			if ((fabs(physicalRange) > Stuff::SMALL) && (fabs(RealRange) > Stuff::SMALL))
			{
				float newScrollPos = .5 + (finalPos)*RealRange/physicalRange;
				if ( newScrollPos < 0 )
					newScrollPos = 0;
				if ( newScrollPos > scrollMax )
					newScrollPos = scrollMax;
				SetScroll( newScrollPos );
				scrollTab.press( true );
			}
		}
		else if ( pointInside( mouseX, mouseY ) )
		{
	
		
				
			if ( userInput->isLeftClick() || gos_GetKeyStatus(KEY_LMOUSE) == KEY_HELD
				|| userInput->leftMouseReleased() )
			{
				lastY = 0;
				if ( scrollTab.pointInside( mouseX, mouseY ) && !userInput->leftMouseReleased()  )
					lastY = scrollTab.top() - topButton.bottom();
				else if ( getParent() )
				{
					if ( !topButton.pointInside( mouseX, mouseY )
						&& !bottomButton.pointInside( mouseX, mouseY )
						&& !topButton.pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() )
						&& !bottomButton.pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() )
						&& mouseY > topButton.globalBottom() 
						&& mouseY < bottomButton.globalY()
						&& pointInside( userInput->getMouseDragX(), userInput->getMouseDragY() )
						&& !scrollTab.pointInside( mouseX, mouseY )
						&& ( userInput->leftMouseReleased() || userInput->getMouseLeftHeld() > .5 ) )
					{
					//	float physicalRange = height() - topButton.height() - bottomButton.height() - scrollTab.height();
					//	float RealRange = scrollMax;
					//	float delta = (float)mouseY - (topButton.globalY() + topButton.height());
				
						float newScrollPos = scrollPos;
						// if above the thumb, page up, otherwise page down
						if ( mouseY < scrollTab.globalY() )
						{
								newScrollPos = scrollPos - pageInc;
						}
						else if ( mouseY > scrollTab.globalBottom() )
						{
								newScrollPos = scrollPos + pageInc;
						}

						if( newScrollPos < 0 )
								newScrollPos = 0;

						if( newScrollPos > scrollMax )
							newScrollPos = scrollMax;

						
					
						getParent()->handleMessage( aMSG_SCROLLTO, newScrollPos );
						SetScrollPos( newScrollPos );

					}

					scrollTab.press( 0 );
					
				}
			}
		}

		if ( userInput->leftMouseReleased() )
			lastY = 0;
		
	
	aObject::update();
}

int aScrollBar::handleMessage( unsigned long message, unsigned long who )
{
	switch (who )
	{
		case aMSG_SCROLLUP:
			SetScrollPos( scrollPos - scrollInc );
			break;

		case aMSG_SCROLLDOWN:
			SetScrollPos( scrollPos + scrollInc );
			break;
	}

	return getParent()->handleMessage( who, who );
}

void aScrollBar::ResizeAreas(void)
{
	float range, position;

	if (scrollMax == 0)
		return;

	float physicalRange = height() - topButton.height() - bottomButton.height() - 6.f;
	float RealRange = scrollMax;

	pageInc = physicalRange;

	float	scrollTabSize =  physicalRange * physicalRange/(physicalRange + RealRange);

	if ( scrollTabSize < scrollTab.width() )
		scrollTabSize = scrollTab.width();

	scrollTab.resize( scrollTab.width(), scrollTabSize);


	range = height() - topButton.height() - bottomButton.height() - scrollTab.height() - 6.f;	// one scrollwidth for buttons, one for tab. 2 for lines at either end.
	position = range * scrollPos / scrollMax;	//	center of scroll tab;

	scrollTab.moveTo( globalX() + 2, topButton.globalY() + topButton.height() + position + 1);

}

void aScrollBar::Enable( bool enable )
{
	scrollTab.disable( !enable );
	topButton.disable( !enable );
	bottomButton.disable( !enable );
}
 
long mcScrollBar::init ( long xPos, long yPos, long w, long h  )	
{

	FitIniFile file; 
	char path[256];
	strcpy( path, artPath );
	strcat( path, "scrollbar.fit" );
	if ( NO_ERR != file.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", path );
		Assert( 0, 0, error );
	}

	setColor( 0, 0 ); // black background

	color = 0xff002F55; // outline color

	topButton.init( file, "ScrollButton0" );
	
	
	bottomButton.init( file, "ScrollButton1" );
	bottomButton.moveTo( bottomButton.x(), h - bottomButton.height() - 2 );
	
	
	scrollTab.init( file, "ScrollButton2" );
	scrollTab.moveTo( scrollTab.x(), topButton.bottom() + 1 );

	file.seekBlock( "Orange" );
	orangeInfo[0].init( &file, "Normal" );
	orangeInfo[1].init( &file, "Highlight" );
	orangeInfo[2].init( &file, "Pressed" );
	orangeInfo[3].init( &file, "Disabled" );
	
	file.seekBlock( "Green" );
	greenInfo[0].init( &file, "Normal" );
	greenInfo[1].init( &file, "Highlight" );
	greenInfo[2].init( &file, "Pressed" );
	greenInfo[3].init( &file, "Disabled" );
	
	aScrollBar::init( xPos, yPos, w, h );

	scrollTab.setTexture( (unsigned long)0 );

	return 0;

}

void mcScrollBar::resize(long w, long h)
{
	aScrollBar::resize(w, h);
	bottomButton.moveTo( bottomButton.globalX(), globalY() + h - bottomButton.height() - 2 );
	scrollTab.moveTo( scrollTab.globalX(), topButton.globalY() + topButton.height() + 1 );
	ResizeAreas();
}

void mcScrollBar::setGreen()
{
	topButton.setAnimationInfo( &greenInfo[0], &greenInfo[1], &greenInfo[2], &greenInfo[3] );
	bottomButton.setAnimationInfo( &greenInfo[0], &greenInfo[1], &greenInfo[2], &greenInfo[3] );
	scrollTab.setAnimationInfo( &greenInfo[0], &greenInfo[1], &greenInfo[2], &greenInfo[3] );

	scrollTab.lightEdge = 0xff6E7C00;
	scrollTab.darkEdge = 0xff303600;
	scrollTab.regularColor = 0xff586300;

	color = ( 0xff6E7C00 );

}

void mcScrollBar::setOrange()
{
	topButton.setAnimationInfo( &orangeInfo[0], &orangeInfo[1], &orangeInfo[2], &orangeInfo[3] );
	bottomButton.setAnimationInfo( &orangeInfo[0], &orangeInfo[1], &orangeInfo[2], &orangeInfo[3] );
	scrollTab.setAnimationInfo( &orangeInfo[0], &orangeInfo[1], &orangeInfo[2], &orangeInfo[3] );

	scrollTab.lightEdge = 0xffC66600;
	scrollTab.darkEdge = 0xff5C2F00;
	scrollTab.regularColor = 0xff9E5200;

	color = ( 0xff43311C );

}

void mcScrollButton::render()
{
	if ( isShowing() && state != DISABLED )
	{
		setColor( regularColor );
		aButton::render();

		GUI_RECT rect =  { globalX(), globalY(), globalRight()-1, globalBottom()-1 };

		drawEmptyRect( rect, lightEdge, darkEdge );
	}
}

