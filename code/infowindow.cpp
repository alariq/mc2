#define INFOWINDOW_CPP
/*************************************************************************************************\
InfoWindow.cpp			: Implementation of the InfoWindow component.
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"infowindow.h"
#include"mclib.h"
#include "../resource.h"
#include"mover.h"
#include"mechicon.h"
#include"team.h"
#include"txmmgr.h"
#include"estring.h"

long InfoWindow::SCROLLLEFT = 0;
long InfoWindow::SCROLLRIGHT = 0;
long InfoWindow::SCROLLTOP = 0;
long InfoWindow::SCROLLBOTTOM = 0;
long InfoWindow::SECTIONSKIP = 0;
long InfoWindow::NAMELEFT = 0;
long InfoWindow::NAMERIGHT = 0;
long InfoWindow::NAMETOP = 0;
long InfoWindow::NAMEBOTTOM = 0;
long InfoWindow::HEALTHLEFT = 0;
long InfoWindow::HEALTHRIGHT = 0;
long InfoWindow::HEALTHTOP = 0;
long InfoWindow::HEALTHBOTTOM = 0;
long InfoWindow::DIVIDERCOLOR = 0;
long InfoWindow::DIVIDERLEFT = 0;
long InfoWindow::DIVIDERRIGHT = 0;
long	InfoWindow::PILOTLEFT = 0;
long InfoWindow::PILOTRIGHT = 0;
long InfoWindow::PILOTHEIGHT = 0;
long InfoWindow::MECHLEFT = 0;
long InfoWindow::MECHRIGHT = 0;
long InfoWindow::MECHHEIGHT = 0;
long InfoWindow::MECHBACKLEFT = 0;
long InfoWindow::MECHBACKRIGHT = 0;
long InfoWindow::MECHBACKHEIGHT = 0;
long InfoWindow::SCROLLBUTTONV = 0;
StaticInfo* InfoWindow::skillInfos = 0;
long InfoWindow::SCROLLBUTTONU = 0;
long InfoWindow::SCROLLBUTTONHEIGHT = 0;
long InfoWindow::SCROLLBUTTONWIDTH = 0;
long InfoWindow::SCROLLBUTTONX = 0;
long InfoWindow::SCROLLMAX = 0;
long InfoWindow::SCROLLMIN = 0;
long InfoWindow::NUMBERSKILLBARS = 0;
long InfoWindow::SKILLUNITWIDTH = 0;
long InfoWindow::SKILLSKIP = 0;
long InfoWindow::SKILLHEIGHT = 0;
long InfoWindow::SKILLRIGHT = 0;
long InfoWindow::SKILLLEFT = 0;
long InfoWindow::SCROLLCOLOR = -1;
long InfoWindow::INFOHEIGHT = 0;
long InfoWindow::INFOWIDTH = 0;
long InfoWindow::INFOTOP = 0;
long InfoWindow::INFOLEFT = 0;
long InfoWindow::COMPONENTLEFT = 0;

long InfoWindow::SCROLLBOXLEFT= 0;
long InfoWindow::SCROLLBOXRIGHT= 0;
long InfoWindow::SCROLLBOXTOP= 0;
long InfoWindow::SCROLLBOXBOTTOM= 0;
long InfoWindow::PILOTNAMELEFT = 0;

GUI_RECT InfoWindow::NameRect = { 0 };

ControlButton InfoWindow::buttons[2] = {0};

InfoWindow* InfoWindow::s_instance = NULL;


#define SCROLLAMOUNT		3.f * Environment.screenWidth/640.f
extern float WeaponRanges[NUM_WEAPON_RANGE_TYPES][2];

// initialize statics
ButtonData InfoWindow::buttonData[2];;

extern bool useUnlimitedAmmo;

InfoWindow::InfoWindow()
{
	pUnit = NULL;
	scrollLength = 0;
	scrollPos = 0;
	backgroundTexture = 0;
	icon = NULL;
	infoLength = 0;

	skillInfos = new StaticInfo[7];
	memset( skillInfos, 0, sizeof( StaticInfo ) *  7);
	memset( buttonData, 0, sizeof( buttonData ) );

	lastYClick = -1.f;
	s_instance = this;
}

void InfoWindow::init( FitIniFile& file )
{
	
	if ( NO_ERR != file.seekBlock( "Fonts" ) )
		Assert( 0, 0, "couldn't find the font block" );

	for ( int i = 0; i < 7; i++ )
	{
		if ( skillInfos[i].textureHandle )
		{
			long gosID = mcTextureManager->get_gosTextureHandle( skillInfos[i].textureHandle );
			mcTextureManager->removeTexture( gosID );
		}
	}

	memset( skillInfos, 0, sizeof( skillInfos ) );
	memset( buttonData, 0, sizeof( buttonData ) );


	for (int i = 0; i < 2; i++ )
	{
		if ( buttonData[i].textureHandle )
		{
			long gosID = mcTextureManager->get_gosTextureHandle( buttonData[i].textureHandle );
			mcTextureManager->removeTexture( gosID );
		}
	}


	long fontID;
	if ( NO_ERR != file.readIdLong( "InfoWndFont", fontID ) )
	{
		Assert( 0, 0,"couldn't find infoWndFont in button layout file" );
	}

	s_instance->nameFont.init( fontID );
	

	if ( NO_ERR != file.readIdLong( "ComponentFont", fontID ) )
	{
		Assert( 0, 0, "couldn't find componentFont in button layout file" );
	}

	s_instance->componentFont.init( fontID );



	if ( NO_ERR != file.seekBlock( "InfoWindow" ) )
	{
		Assert( 0, 0, "couldn't find Info block in button layout file" );
	}

	file.readIdLong( "XLocation", 	InfoWindow::INFOLEFT );
	file.readIdLong( "YLocation", 	InfoWindow::INFOTOP );
	file.readIdLong( "Width", 		InfoWindow::INFOWIDTH );
	file.readIdLong( "Height", 		InfoWindow::INFOHEIGHT );
	
	InfoWindow::INFOLEFT 	+= ControlGui::hiResOffsetX;
	InfoWindow::INFOTOP 	+= ControlGui::hiResOffsetY;
	
	file.readIdLong( "SkipBetweenSections", InfoWindow::SECTIONSKIP );

	file.readIdLong( "InfoHeaderBoxLeft", 	NameRect.left );
	file.readIdLong( "InfoHeaderBoxRight", 	NameRect.right );
	file.readIdLong( "InfoHeaderBoxTop", 	NameRect.top );
	file.readIdLong( "InfoHeaderBoxBottom", NameRect.bottom );

	NameRect.left 	+= ControlGui::hiResOffsetX; 
	NameRect.right 	+= ControlGui::hiResOffsetX; 
	NameRect.top 	+= ControlGui::hiResOffsetY;
	NameRect.bottom += ControlGui::hiResOffsetY; 
	
	file.readIdLong( "ScrollBarToP", 	InfoWindow::SCROLLTOP );
	file.readIdLong( "ScrollBarBottom", InfoWindow::SCROLLBOTTOM );
	file.readIdLong( "ScrollBarLeft", 	InfoWindow::SCROLLLEFT );
	file.readIdLong( "ScrollBarRight", 	InfoWindow::SCROLLRIGHT );
	file.readIdLong( "ScrollBarColor", 	InfoWindow::SCROLLCOLOR );
	
	InfoWindow::SCROLLTOP 		+= ControlGui::hiResOffsetY;
	InfoWindow::SCROLLBOTTOM	+= ControlGui::hiResOffsetY; 
	InfoWindow::SCROLLLEFT		+= ControlGui::hiResOffsetX; 
	InfoWindow::SCROLLRIGHT     += ControlGui::hiResOffsetX; 

	file.readIdLong( "ScrollContentBoxLeft", 		InfoWindow::SCROLLBOXLEFT );
	file.readIdLong( "ScrollContentBoxRight", 		InfoWindow::SCROLLBOXRIGHT );
	file.readIdLong( "ScrollContentBoxBarTop", 		InfoWindow::SCROLLBOXTOP );
	file.readIdLong( "ScrollContentBoxBarBottom", 	InfoWindow::SCROLLBOXBOTTOM );
	
	InfoWindow::SCROLLBOXLEFT  	+= ControlGui::hiResOffsetX;  
	InfoWindow::SCROLLBOXRIGHT  += ControlGui::hiResOffsetX;  
	InfoWindow::SCROLLBOXTOP    += ControlGui::hiResOffsetY;  
	InfoWindow::SCROLLBOXBOTTOM += ControlGui::hiResOffsetY;  

	file.readIdLong( "NameLocationLeft", 	InfoWindow::NAMELEFT );
	file.readIdLong( "NameLocationRight", 	InfoWindow::NAMERIGHT );
	file.readIdLong( "NameLocationTop", 	InfoWindow::NAMETOP );
	file.readIdLong( "NameLocationBottom", 	InfoWindow::NAMEBOTTOM );
	
	InfoWindow::NAMELEFT  	+= ControlGui::hiResOffsetX;
	InfoWindow::NAMERIGHT   += ControlGui::hiResOffsetX;
	InfoWindow::NAMETOP     += ControlGui::hiResOffsetY;
	InfoWindow::NAMEBOTTOM  += ControlGui::hiResOffsetY;

	file.readIdLong( "HealthBarLeft", 	InfoWindow::HEALTHLEFT );
	file.readIdLong( "HealthBarRight", 	InfoWindow::HEALTHRIGHT );
	file.readIdLong( "HealthBarTop", 	InfoWindow::HEALTHTOP );
	file.readIdLong( "HealthBarBottom", InfoWindow::HEALTHBOTTOM );
	
	InfoWindow::HEALTHLEFT		+= ControlGui::hiResOffsetX; 
	InfoWindow::HEALTHRIGHT     += ControlGui::hiResOffsetX; 
	InfoWindow::HEALTHTOP       += ControlGui::hiResOffsetY; 
	InfoWindow::HEALTHBOTTOM    += ControlGui::hiResOffsetY; 

	file.readIdLong( "DividerLeft", 	InfoWindow::DIVIDERLEFT );
	file.readIdLong( "DividerRight", 	InfoWindow::DIVIDERRIGHT );
	file.readIdLong( "DividerColor", InfoWindow::DIVIDERCOLOR );
	
	InfoWindow::DIVIDERLEFT 	+= ControlGui::hiResOffsetX;  
	InfoWindow::DIVIDERRIGHT    += ControlGui::hiResOffsetX;  

	file.readIdLong( "PilotLeft", 		InfoWindow::PILOTLEFT );
	file.readIdLong( "PilotRight", 		InfoWindow::PILOTRIGHT );
	
	InfoWindow::PILOTLEFT 	+= ControlGui::hiResOffsetX;  
	InfoWindow::PILOTRIGHT	+= ControlGui::hiResOffsetX;  
	
	file.readIdLong( "PilotHeight", 	InfoWindow::PILOTHEIGHT );
	file.readIdLong( "PilotNameLeft", 	PILOTNAMELEFT );
	
	PILOTNAMELEFT += ControlGui::hiResOffsetX;   

	file.readIdLong( "MechIconLeft", 		InfoWindow::MECHLEFT );
	file.readIdLong( "MechIconRight", 		InfoWindow::MECHRIGHT );
	file.readIdLong( "MechIconHeight", 		InfoWindow::MECHHEIGHT );
	file.readIdLong( "MechIconBackLeft", 	InfoWindow::MECHBACKLEFT );
	file.readIdLong( "MechIconBackRight", 	InfoWindow::MECHBACKRIGHT );
	file.readIdLong( "MechIconBackHeight", 	InfoWindow::MECHBACKHEIGHT );
	
	InfoWindow::MECHLEFT		+= ControlGui::hiResOffsetX;   
	InfoWindow::MECHRIGHT       += ControlGui::hiResOffsetX;   
	InfoWindow::MECHBACKLEFT    += ControlGui::hiResOffsetX;   
	InfoWindow::MECHBACKRIGHT   += ControlGui::hiResOffsetX;   
	
	file.seekBlock( "SkillMeter" );
	file.readIdLong( "Left", 		InfoWindow::SKILLLEFT );
	file.readIdLong( "Right", 		InfoWindow::SKILLRIGHT );
	
	InfoWindow::SKILLLEFT 	+= ControlGui::hiResOffsetX;  
	InfoWindow::SKILLRIGHT 	+= ControlGui::hiResOffsetX;  
	
	file.readIdLong( "Height", 		InfoWindow::SKILLHEIGHT );
	file.readIdLong( "Skip", 		InfoWindow::SKILLSKIP );
	file.readIdLong( "UnitWidth", 	InfoWindow::SKILLUNITWIDTH );
	file.readIdLong( "NumberUnits", InfoWindow::NUMBERSKILLBARS );
	
	file.seekBlock( "ScrollButton" );
	file.readIdLong( "Min", 		SCROLLMIN );
	file.readIdLong( "Max", 		SCROLLMAX );
	file.readIdLong( "XLocation", 	SCROLLBUTTONX );

	SCROLLMIN		+= ControlGui::hiResOffsetY; 
	SCROLLMAX	    += ControlGui::hiResOffsetY; 
	SCROLLBUTTONX 	+= ControlGui::hiResOffsetX;
	
	file.readIdLong( "Width", 		SCROLLBUTTONWIDTH );
	file.readIdLong( "Height", 		SCROLLBUTTONHEIGHT );
	
	file.readIdLong( "UNormal", SCROLLBUTTONU );
	file.readIdLong( "VNormal", SCROLLBUTTONV );



	ControlButton::initButtons( file, 2, buttons, buttonData, "InfoButton" );

	char SkillText[32];
	for (int i = 0; i < 7; i++ )
	{
		sprintf( SkillText, "Skill%ld", i );
		skillInfos[i].init( file, SkillText ,ControlGui::hiResOffsetX, ControlGui::hiResOffsetY);
	
	}

	InfoWindow::COMPONENTLEFT = InfoWindow::PILOTLEFT;
	

}

InfoWindow::~InfoWindow()
{
	delete [] skillInfos;
}

void InfoWindow::setUnit(Mover* pNewMover)
{

	if ( pNewMover && ( pNewMover->getTeamId() != Team::home->getId() &&
		( CONTACT_VISUAL != pNewMover->getContactStatus(Team::home->getId(), true)) 
		&& !pNewMover->isDisabled() ))
	{
		return;
	}

	if ( pUnit != pNewMover )
	{
		pUnit = pNewMover;
		bUnitChanged = true;

		if ( icon )
		{
			delete icon;
			icon = NULL;
		}

		icon = NULL;

		scrollPos = 0;
		infoLength = 0;

		if ( pNewMover )
		{

			if ( pNewMover->getObjectType()->getObjectTypeClass() == BATTLEMECH_TYPE )
			{
				MechIcon* pIcon = new MechIcon;
				pIcon->setLocationIndex( 16 );
				pIcon->init( pNewMover );
				pIcon->setDrawBack( true );
				icon = pIcon;
			}
			else
			{
				VehicleIcon* pIcon = new VehicleIcon;
				pIcon->setLocationIndex( 16 );
				pIcon->setDrawBack( 1 );
				pIcon->init( pNewMover );
				icon = pIcon;
			}
		}
	}
}

void InfoWindow::drawName( const char* name )
{
	GUI_RECT rect = { NAMELEFT, NAMETOP, NAMERIGHT, SCROLLTOP};
	drawRect( rect, 0xff000000 );
	drawEmptyRect( NameRect, 0xff002f55, 0xff002f55 );
	nameFont.render( name, NAMELEFT, NAMETOP, NAMERIGHT - NAMELEFT, NAMEBOTTOM - NAMETOP, 0xff5c96c2, 0, 3 );
}

void InfoWindow::render()
{

	
	drawScrollingStuff();

	GUI_RECT tmpRect = { SCROLLBOXLEFT, SCROLLBOXBOTTOM, SCROLLBOXRIGHT, INFOTOP + INFOHEIGHT };
	drawRect( tmpRect, 0xff000000 );

	for ( int i = 0; i < 2; i++ )
	{
		if ( buttons[i].isEnabled() )
			buttons[i].render();
	}
	
	long scrollBarLength = buttons[1].location[0].y - buttons[0].location[2].y - 4 - SCROLLBUTTONHEIGHT;

	gos_VERTEX v[4];
	for (int i = 0; i < 4; i++ )
	{
		v[i].argb = 0xff5c96c2;
		v[i].frgb = 0;
		v[i].rhw = .5;
		v[i].x = SCROLLBUTTONX;
		v[i].y = SCROLLMIN + scrollPos;
		v[i].z = 0.f;
		v[i].u = v[i].v = 0.f;
//		v[i].u = ((float)SCROLLBUTTONU)/256.f + .1/256.f;
//		v[i].v = ((float)SCROLLBUTTONV)/256.f + .1/256.f ;
	}
	if ( infoLength < scrollBarLength  || !pUnit )
	{
		buttons[0].disable( 1 );
		buttons[1].disable( 1 );
		SCROLLMAX = 0;
	}
	else
	{
		buttons[0].disable( 0 );
		buttons[1].disable( 0 );

		float physicalRange = buttons[1].location[0].y - buttons[0].location[2].y;
		float buttonHeight = SCROLLBUTTONHEIGHT;

		float RealRange = infoLength;

		buttonHeight =  physicalRange * physicalRange/(physicalRange + RealRange);

		if ( buttonHeight < SCROLLBUTTONHEIGHT )
			buttonHeight = SCROLLBUTTONHEIGHT;

		SCROLLMAX = buttons[1].location[0].y - buttonHeight - 2;
		



		v[2].x = v[3].x = SCROLLBUTTONX + SCROLLBUTTONWIDTH;
		v[1].y = v[2].y = SCROLLBOTTOM;	
		v[1].y = v[2].y = v[0].y + buttonHeight;

	//	v[2].u = v[3].u = .1/256.f + (float)(SCROLLBUTTONU + ((float)SCROLLBUTTONWIDTH))/256.f;
	//	v[1].v = v[2].v = .1/256.f + (float)(SCROLLBUTTONV + ((float)SCROLLBUTTONHEIGHT))/256.f;

		gos_SetRenderState( gos_State_Texture, 0 );
		gos_DrawQuads( v, 4 );
	}



	// draw the name of the unit
	if ( pUnit )
	{
		drawName( pUnit->getIfaceName() );
	
		gos_SetRenderState( gos_State_Texture, 0 );

		// draw the health bar
		DWORD					color;
		
		float barStatus = pUnit->getAppearance()->barStatus;
			
		if (barStatus > 1.0f)
			barStatus = 1.0f;

		if (barStatus >= 0.5)
			color = SB_GREEN;
		else if (barStatus > 0.2)
			color = SB_YELLOW;
		else 
			color = SB_RED;
			
		v[0].x = v[1].x = HEALTHLEFT;
		v[2].x = v[3].x = HEALTHRIGHT;
		v[0].y = v[3].y = HEALTHTOP;
		v[1].y = v[2].y = HEALTHBOTTOM;

		unsigned long dimColor = ( color & 0xff7f7f7f );
		for ( int i = 0; i < 4; i++ )
			v[i].argb = dimColor;

		gos_DrawQuads( v, 4 );

		v[2].x = v[3].x = HEALTHLEFT + (HEALTHRIGHT - HEALTHLEFT)* barStatus;

		for (int i = 0; i < 4; i++ )
			v[i].argb = color | 0xff000000;

		gos_DrawQuads( v, 4 );
		
	}
	else
	{
		char noUnit[256];
		cLoadString( IDS_NOUNIT, noUnit, 255 );
		drawName( noUnit );
	}

	GUI_RECT border = { SCROLLBOXLEFT, SCROLLBOXTOP, SCROLLBOXRIGHT, SCROLLBOXBOTTOM  };
	drawEmptyRect( border, SCROLLCOLOR, SCROLLCOLOR );
	GUI_RECT rect = { SCROLLLEFT, SCROLLTOP, SCROLLRIGHT, SCROLLBOTTOM };
	drawEmptyRect( rect, SCROLLCOLOR, SCROLLCOLOR );


}

void InfoWindow::update()
{
	if ( pUnit && ( pUnit->getTeamId() != Team::home->getId() &&
		( CONTACT_VISUAL != (pUnit->getContactStatus(Team::home->getId(), true)) 
		&& !pUnit->isDisabled() ) ) )
	{
		setUnit( 0 );
		return;
	}

	long mouseX = userInput->getMouseX();
	long mouseY = userInput->getMouseY();



	if ( icon )
		icon->update();

	for ( int i = 0; i < 2; i++ )
	{	
		if ( buttons[i].location[0].x <= mouseX && mouseX <= buttons[i].location[2].x
				&& mouseY >= buttons[i].location[0].y && mouseY <= buttons[i].location[1].y  )
		{
			if ( userInput->getMouseLeftButtonState() == MC2_MOUSE_DOWN && buttons[i].isEnabled() )
			{
				{
					handleClick( buttons[i].ID );
					return;
				}
			}
		}
	}

	if ( userInput->isLeftDrag() )
	{
		if ( lastYClick != -1.f )
		{
			int tmpLastY = mouseY; 
			tmpLastY -= userInput->getMouseDragY();
			setScrollPos( lastYClick + tmpLastY );
			return;
		}

	}	
	// see if its in the scroll bar area
	else if ( mouseX > SCROLLLEFT && mouseX < SCROLLRIGHT
				&& mouseY > SCROLLTOP && mouseY < SCROLLBOTTOM 
				&&  buttons[0].isEnabled() 
				&& userInput->getMouseDragX() > SCROLLLEFT && userInput->getMouseDragX() < SCROLLRIGHT
				&& userInput->getMouseDragY() > SCROLLTOP && userInput->getMouseDragY() < SCROLLBOTTOM 
				)
		{


		// if its in the thumbdrag thingie, save the y
			float physicalRange = buttons[1].location[0].y - buttons[0].location[2].y;
			float buttonHeight = SCROLLBUTTONHEIGHT;

			float RealRange = infoLength;

			buttonHeight =  physicalRange * physicalRange/(physicalRange + RealRange);

			if ( buttonHeight < SCROLLBUTTONHEIGHT )
				buttonHeight = SCROLLBUTTONHEIGHT;

			if ( mouseY > scrollPos + SCROLLMIN && mouseY < scrollPos + SCROLLMIN + buttonHeight )
			{
				lastYClick = scrollPos;
			}
			else if ( ( userInput->leftMouseReleased() || userInput->getMouseLeftHeld() > .5 ) )
			{
				lastYClick = -1;
				if ( mouseY > buttons[0].location[2].y 
					&& mouseY < buttons[1].location[0].y )
				{
			
					float newScrollPos = scrollPos;
					// if above the thumb, page up, otherwise page down
					if ( mouseY < SCROLLMIN + scrollPos )
					{
							newScrollPos = scrollPos - buttonHeight;
					}
					else if ( mouseY > SCROLLMIN + scrollPos + buttonHeight )
					{
							newScrollPos = scrollPos + buttonHeight;
					}

					if( newScrollPos < 0 )
							newScrollPos = 0;

					if( newScrollPos > infoLength )
						newScrollPos = infoLength;

					
					setScrollPos( newScrollPos );

				}

			}			
		}
		
		

		if ( userInput->leftMouseReleased() )
			lastYClick = -1;
		
}
	

void InfoWindow::drawScrollingStuff()
{
	if ( !pUnit )
		return;

	float offset = 0.f;
	if ( infoLength )
	{
		float increment = infoLength/(SCROLLMAX - SCROLLMIN );
		offset = -increment * scrollPos;
	}

	float curY = SCROLLBOXTOP + SECTIONSKIP  + offset;

	if ( icon )
	{
		if ( SCROLLTOP + offset > NAMETOP ) // draw icons if visible
		{
			icon->renderUnitIcon( MECHLEFT, curY, MECHRIGHT, curY + MECHHEIGHT);
			icon->renderUnitIconBack( MECHBACKLEFT, curY, MECHBACKRIGHT, curY + MECHHEIGHT );
		}

		curY += MECHHEIGHT + SECTIONSKIP;

		if( curY > NAMEBOTTOM ) // draw divider if visible
			drawDivider( curY );

	}

	long textColors[4] = { 0xff6E7C00, 0xff005392, 0xffA21600 };
	

	char disabledCount[60][2];
	long ammo[60];
	char ranges[60];
	long names[60];
	memset( disabledCount, 0, sizeof( char ) * 60 * 2);
	memset( names, 0, sizeof( char* ) * 60 );
	memset( ranges, 0, sizeof( char ) * 60 );
	memset( ammo, 0, sizeof( long ) * 60 );

	bool bDraw[4];
	memset( bDraw, 0, sizeof( bool ) * 4 );
	
	int curComponentCount = 60;

	int i = 0;

	for (long curWeapon = pUnit->numOther; curWeapon < (pUnit->numOther + pUnit->numWeapons); curWeapon++) 
	{
			long nName = pUnit->inventory[curWeapon].masterID;
			bool bFound = 0;
			for ( int j = 0; j < i; j++ )
			{
				if ( nName == names[j] )
				{
					disabledCount[j][1] ++;
					if (!pUnit->inventory[curWeapon].disabled && (pUnit->getWeaponShots(curWeapon) > 0)
						&& pUnit->inventory[curWeapon].health > 0 ) 
					{
						disabledCount[j][0]++;
					}
		
					bFound = true;
				}
			}
			if ( bFound )
				continue;
			
			names[i] = nName;
			
			//ONly need to add in the AMMO once!!
			// Glenn has taken the liberty of combining all shots of this type
			// into one master ammo list!!
			if ( MasterComponent::masterList[pUnit->inventory[curWeapon].masterID].getWeaponAmmoType() == WEAPON_AMMO_NONE 
				|| useUnlimitedAmmo )
				ammo[i] = -1;
			else
				ammo[i] += pUnit->getWeaponShots( curWeapon );

			int  range = MasterComponent::masterList[pUnit->inventory[curWeapon].masterID].getWeaponRange();
			ranges[i] = range;
			bDraw[range] = true;

			if (!pUnit->inventory[curWeapon].disabled && (pUnit->getWeaponShots(curWeapon) > 0)
				&& pUnit->inventory[curWeapon].health > 0 ) 
			{
				disabledCount[i][0]++;
			}
			disabledCount[i][1]++;
			i++;
	}

	curY += SECTIONSKIP;


	unsigned long height = componentFont.height();
	

	// removing headers for now
//	long stringIDs[4] = { IDS_SHORT, IDS_MEDIUM, IDS_LONG, IDS_COMPONENT};
//	long headerColors[4] = { 0xFFC8E100, 0xff0091FF, 0xFFFF0000, 0xffFF8A00 };
	EString capHeader;



	for ( int j = 0; j < 3; j++ )
	{
		if ( !bDraw[j] ) // make sure we have one
			continue;

//		char header[64];
		if ( curY > NAMETOP )
		{
//			cLoadString( stringIDs[j], header, 63 );
//			capHeader = header;
			//capHeader.MakeUpper();
//			componentFont.render( capHeader, COMPONENTLEFT, curY, SCROLLLEFT - COMPONENTLEFT, height, headerColors[j], 0, 0 );
		}
//		curY += height;
		
		for ( i = 0; i < curComponentCount; i++ )
		{
			if ( !names[i] )
				break;
			char tmpName[256];
			if ( ranges[i] == j )
			{
				if ( curY > NAMETOP )
				{
					cLoadString( IDS_COMP_ABBR0 + names[i], tmpName, 255 );
					capHeader.Format( "%ld/%ld  %s", disabledCount[i][0], disabledCount[i][1], tmpName );
					componentFont.render( capHeader, COMPONENTLEFT, curY, SCROLLLEFT - COMPONENTLEFT, height, textColors[j], 0, 0 );
				}
				curY += height;	

				if ( ammo[i] != -1 )
				{
					if ( curY > NAMETOP )
					{
						// make the ammo number
						char tmpNumber[64];
						char tmpNumber2[64];
						cLoadString( IDS_MISSION_SHOTSLEFT, tmpNumber, 63 );
						sprintf( tmpNumber2, tmpNumber, ammo[i] );
						componentFont.render( tmpNumber2, COMPONENTLEFT, curY, SCROLLLEFT - COMPONENTLEFT, height,  textColors[j], 0, 0 );	
				
					}
					curY += height;
				}
			}
	
		}

		curY += SECTIONSKIP;

		if ( curY > NAMEBOTTOM )
			drawDivider( curY );

		curY += SECTIONSKIP;

	}

	memset( names, 0, sizeof( char* ) * 60 );
	long count[4];
	count[0] = pUnit->ecm;
	count[1] = pUnit->probe;
	count[2] = pUnit->jumpJets;
	count[3] = pUnit->isMech() ?  pUnit->sensor : 255;

	if ((count[0] || count[1] || count[2] || count[3] ) )
	{
		if ( curY > NAMETOP )
		{
//			cLoadString( stringIDs[j], header, 63 );
//			componentFont.render( header, COMPONENTLEFT, curY, SCROLLLEFT - COMPONENTLEFT, height, headerColors[j], 0, 0 );
		}

//		curY += height;
	}
	
	

	for (int curWeapon = 0; curWeapon < 4; curWeapon++ )
	{
		if ( count[curWeapon] != 255)
		{
			long color = 0xffc29b00;
			//Neither the ecm, probe, sensors or JumpJets can ever REALLY be disabled. No matter what the setting is!!
//			if (pUnit->inventory[count[curWeapon]].disabled) 
//				color = 0xff7f7f7f;

			char tmpName[256];
			cLoadString( IDS_COMP_ABBR0 + pUnit->inventory[count[curWeapon]].masterID, tmpName, 255 );

			if ( curY > NAMETOP )
			{
				componentFont.render( tmpName, COMPONENTLEFT, curY, SCROLLLEFT - COMPONENTLEFT, height, color, 0, 0 );
			}
	
			curY += height;
			curY += InfoWindow::SECTIONSKIP;

		}
	}

	if ( curY > NAMEBOTTOM )
		drawDivider( curY );

	curY += SECTIONSKIP;

	// DON'T DO PILOT INFO if ENEMY OR mech is destroyed or disabled.
	if ( pUnit->getTeam() && !Team::home->isEnemy(pUnit->getTeam()) && pUnit->isMech() && !pUnit->isDisabled() && !pUnit->isDestroyed())
	{
		MechWarrior* pWarrior = pUnit->getPilot();

		if ( icon )
		{
			if ( curY > NAMETOP )
			{
				if ( pWarrior->active() )
					icon->renderPilotIcon( PILOTLEFT, curY, PILOTRIGHT, curY + PILOTHEIGHT );
				GUI_RECT tmpRect = { PILOTLEFT, curY, PILOTRIGHT + 1, curY + PILOTHEIGHT + 1 }; 
				drawEmptyRect( tmpRect, SCROLLCOLOR, SCROLLCOLOR  );
			
				float right = SCROLLLEFT;
				float top = curY + PILOTHEIGHT/2 - height/2;
				float bottom = top + height;
				// draw the name of the pilot
				char deadPilotName[256];
				cLoadString( IDS_NOPILOT, deadPilotName, 255 );
				capHeader = pWarrior->active() ? pWarrior->getName() : deadPilotName;
				componentFont.render( capHeader, PILOTNAMELEFT, top, right - PILOTNAMELEFT, bottom - top, 0xff005392, 0, 0 );
				
			}
			
			curY += PILOTHEIGHT;
			curY += SECTIONSKIP;
		}

		curY += SECTIONSKIP;

		int rank = pWarrior->getRank();
		int skills[2] = { MWS_GUNNERY, MWS_PILOTING };

		char buffer[256];
		//ACE not continguous with other ranks.  Added too late!
		if (rank != 4)
			cLoadString( IDS_GREEN + rank, buffer, 256 );
		else
			cLoadString( IDS_ACE, buffer, 256 );

		if ( curY > NAMETOP )
		{
			componentFont.render( buffer, SKILLLEFT, curY, SCROLLLEFT - SKILLLEFT, height, 0xff005392, 0, 0 );
		}

		int currentSkill = rank;
		for (int j = 0; j < 3; j ++ )
		{
			gos_VERTEX v[4];

			float height = skillInfos[currentSkill].location[1].y - skillInfos[currentSkill].location[0].y;
			for ( i = 0; i < 4; i++ )
			{
				v[i] = skillInfos[currentSkill].location[i];
				v[i].y = curY;	
				v[i].rhw = .5;
			}

			v[1].y = v[2].y = curY + height;		
			
			if ( curY > NAMETOP )
			{
				GUI_RECT tmpRect = { v[0].x - .5, v[0].y - .5, v[2].x + 1.5, v[2].y + 1.5 };
				drawEmptyRect( tmpRect, 0xff002f55, 0xff002f55  );


				unsigned long gosID = mcTextureManager->get_gosTextureHandle( skillInfos[currentSkill].textureHandle );
				gos_SetRenderState( gos_State_Texture, gosID );
				gos_DrawQuads( v, 4 );

				if ( j != 0 )
				{
					int skill = pWarrior->getSkill( skills[j-1] );
					drawSkillBar( skill, curY, height );
				}
			}

			curY += height;
			curY += SECTIONSKIP;
			currentSkill = j + 5;

		}

			
		
		for ( i = 0; i < NUM_SPECIALTY_SKILLS; i++ )
		{
			if ( pWarrior->specialtySkills[i] )
			{
				if ( curY > NAMETOP )
				{
					cLoadString( IDS_SPECIALTY + i, buffer, 256 );
					componentFont.render( buffer, NAMELEFT, curY, NAMERIGHT - NAMELEFT, height, 0xff005392, 0, 0 );
				}
				curY += height;
			}
		}
	}
	else
		curY += 10 * SECTIONSKIP;


	if ( infoLength == 0 )
		infoLength = curY - SCROLLTOP - ( SCROLLBOTTOM - SCROLLTOP );
}

void InfoWindow::handleClick( int ID )
{
	switch( ID )
	{
	case SCROLLUP:
		setScrollPos( scrollPos - SCROLLAMOUNT );
		break;

	case SCROLLDOWN:
		setScrollPos( scrollPos + SCROLLAMOUNT );
		break;

	}
}

void InfoWindow::drawDivider( float yVal )
{
	gos_VERTEX v[2];

//	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );

	gos_SetRenderState( gos_State_Specular,FALSE );

	gos_SetRenderState( gos_State_AlphaTest, 0 );
		
	gos_SetRenderState( gos_State_Texture, 0 );

	gos_SetRenderState( gos_State_Filter, gos_FilterNone );


	memset( v, 0, sizeof( gos_VERTEX ) * 2 );
	for ( int i = 0; i < 2; i++ )
		v[i].rhw = .5f;

	v[0].x = DIVIDERLEFT;
	v[0].y = v[1].y = yVal;
	v[1].x = DIVIDERRIGHT;
	v[0].argb = v[1].argb = DIVIDERCOLOR;

 	gos_DrawLines( v, 2 );

}

void InfoWindow::drawSkillBar( int skill, float yVal, float height )
{
	float left = InfoWindow::SKILLLEFT;
	float right = InfoWindow::SKILLRIGHT;

	int barCount = skill/InfoWindow::NUMBERSKILLBARS;

	int redIncrement = 0;
	int greenIncrement = 0;
	int blueIncrement = 0;
	if ( barCount )
	{
		redIncrement =  (205/barCount) << 16;
		greenIncrement = ((234-83)/barCount) << 8;
		blueIncrement = (255 - 146)/barCount;
	}

	unsigned long color = 0xff005392;

	GUI_RECT outSideRect = { left - SKILLSKIP + .5, yVal - .5, right + SKILLSKIP + .5, yVal + height + 1.5};
	drawRect( outSideRect, 0xff000000 );
	GUI_RECT rect = { left  + InfoWindow::SKILLSKIP, yVal + InfoWindow::SKILLSKIP + .5,
		left  + InfoWindow::SKILLSKIP + SKILLUNITWIDTH, yVal + height - InfoWindow::SKILLSKIP + .5 };

	drawEmptyRect( outSideRect, 0xff002f55, 0xff002f55 );

	for ( int i = 0; i < barCount; i++ )
	{
		drawRect( rect, color );
		color += redIncrement;
		color += greenIncrement;
		color += blueIncrement;
		
		rect.left += SKILLUNITWIDTH + 1;
		rect.right = rect.left + SKILLUNITWIDTH;
	}

	char buffer[32];
	sprintf( buffer, "%ld", skill );
	componentFont.render( buffer, SKILLRIGHT+2, yVal, SCROLLLEFT - SKILLRIGHT - 2, SKILLHEIGHT, 0xff005392, 0, 0 );
}

void InfoWindow::setScrollPos( int where )
{
	if ( where < 0 )
		scrollPos = 0;
	else if  ( where > SCROLLMAX - SCROLLMIN )
		scrollPos = SCROLLMAX - SCROLLMIN;
	else
		scrollPos = where;
}



//*************************************************************************************************
// end of file ( InfoWindow.cpp )

