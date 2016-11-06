 #define MECHICON_CPP
/*************************************************************************************************\
MechIcon.cpp			: Implementation of the MechIcon component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"mechicon.h"
#include"mover.h"
#include"utilities.h"
#include"mech.h"
#include"gvehicl.h"
#include"gamecam.h"
#include"controlgui.h"
#include"gamesound.h"
#include<windows.h>

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

TGAFileHeader *MechIcon::s_MechTextures = NULL;
TGAFileHeader *VehicleIcon::s_VehicleTextures = NULL;
TGAFileHeader* ForceGroupIcon::s_textureMemory = 0;

StaticInfo*		ForceGroupIcon::jumpJetIcon = NULL;

MC2MoviePtr	ForceGroupIcon::bMovie = NULL;
DWORD ForceGroupIcon::pilotVideoTexture = 0;
MechWarrior*	ForceGroupIcon::pilotVideoPilot = NULL;


#define HEALTH_BAROFFSET 12.0f * Environment.screenHeight/600.f
#define HEALTH_BARLENGTH 31.0f * Environment.screenWidth/600.f
#define HEALTH_BARHEIGHT	2.0f * Environment.screenHeight/600.f
#define TEXT_HEIGHT				8.0f * Environment.screenHeight/480.f

float			ForceGroupIcon::unitIconX = 32;
float			ForceGroupIcon::unitIconY = 38;
float			PilotIcon::pilotIconX = 25;
float			PilotIcon::pilotIconY = 36;
const long		PilotIcon::DEAD_PILOT_INDEX = 27;

unsigned long	PilotIcon::s_pilotTextureHandle = 0;
unsigned long	PilotIcon::s_pilotTextureWidth = 0;
long			ForceGroupIcon::pilotTextTop[17] = {0};


DWORD ForceGroupIcon::damageColors[4][3] = 
		{ 194 << 16 | 229 << 8 | 255 | 0xff << 24,	92 << 16 | 150 << 8 | 194 | 0xff << 24, 0 << 16 | 83 << 8 | 146 | 0xff << 24,
		  248 << 16 | 241 << 8 | 193 | 0xff << 24,	248 << 16 | 206 << 8 | 31 | 0xff << 24, 139 << 16 | 114 << 8 | 0 | 0xff << 24,
		  248 << 16 | 193 << 8 | 193 | 0xff << 24,	248 << 16 | 31 << 8 | 31 | 0xff << 24, 139 << 16 | 0 << 8 | 0 | 0xff << 24,
		  94 << 16 | 101 << 8 | 101 | 0xff << 24,	56 << 16 | 64 << 8 | 64 | 0xff << 24, 26 << 16 | 33 << 8 | 33 | 0xff << 24
		};

DWORD ForceGroupIcon::ForceGroupColors[11] = {
		0xff005392,
		0xffC66600,
		0xff6E7C00,
		0xffB70000,
		0xff620053,
		0xffD8AD00,
		0xff8B00BE,
		0xffFF0000,
		0xffFF8A00,
		0xff0091FF,
		0xff505050

 };

ForceGroupIcon::AnimationInfo ForceGroupIcon::animationInfos[NUM_DEATH_INFOS] = 
{
	0.f, 0,
	2.5f, 0, 
	5.0f, 0x7fff0000, 
	5.35f, 0x7fff0000,
	6.0f, 0xff000000,
	6.5f, 0xff000000

};

unsigned long	ForceGroupIcon::s_textureHandle[5] = { -1, -1, -1, -1, -1 };
bool			ForceGroupIcon::s_slotUsed[240] = { 0 };

gos_VERTEX ForceGroupIcon::	bmpLocation[17][5]  = {0}; // in screen coords

GUI_RECT ForceGroupIcon:: textArea[17]  = {0};
GUI_RECT ForceGroupIcon:: pilotRect[17]  = {0};
GUI_RECT ForceGroupIcon:: selectionRect[17]  = {0};
GUI_RECT ForceGroupIcon:: healthBar[17]  = {0};

float ForceGroupIcon::	pilotLocation[17][4]  = {0};

aFont* ForceGroupIcon::gosFontHandle = 0;

int ForceGroupIcon::curScreenWidth = 800;

#define ICON_REALSIZE 256

ForceGroupIcon::ForceGroupIcon(  )
{
	unit = NULL;
	bDrawBack = 0;
	backDamageIndex = -1;
	damageIconIndex = 0;

	locationIndex = 0;

	if ( s_textureHandle[0] == -1 )
	{
		// this is the textures that all icons are using
		long size = ( 256 * 256 * sizeof( long ) + sizeof( TGAFileHeader ) );
		s_textureMemory = (TGAFileHeader*)new char[size];
		memset( s_textureMemory, 0, size );
		s_textureMemory->height = 256;
		s_textureMemory->width = 256;
		s_textureMemory->pixel_depth = 32;
		s_textureMemory->image_type = UNC_PAL;

		if ( s_textureHandle[0] == -1 )
			s_textureHandle[0] = gos_NewTextureFromMemory( gos_Texture_Alpha, ".tga", (unsigned char*)s_textureMemory, size, 0 );

		memset( s_slotUsed, 0, 240 * sizeof( bool ) );
	}

	bool bFound = 0;

	for ( int index = 0; index < 240; index++ )
	{
		if ( !s_slotUsed[ index ] )
		{
			damageIconIndex = index;
			s_slotUsed[index] = 1;
			bFound = 1;
			break;
		}
	}

	
	Assert( bFound, bFound, "out of space for icon textures" );

	deathAnimationTime = 0.0;
	msgPlayTime = 0.0f;
	
}

void ForceGroupIcon::resetResolution(bool bForce)
{
	if ( curScreenWidth !=  Environment.screenWidth || bForce )
	{
		delete [] MechIcon::s_MechTextures;
		MechIcon::s_MechTextures = 0;

		delete [] VehicleIcon::s_VehicleTextures;
		VehicleIcon::s_VehicleTextures = 0;

		setIconVariables();

		PilotIcon::swapResolutions();
	}

	curScreenWidth = Environment.screenWidth;
}

void ForceGroupIcon::swapResolutions( bool bForce )
{
	resetResolution(bForce);
	init( unit );
}

ForceGroupIcon::~ForceGroupIcon()
{

	if (pilotVideoPilot && pilotVideoPilot == unit->getPilot() )
	{
		if ( bMovie )
		{
			delete bMovie;
			bMovie = NULL;
			pilotVideoTexture = 0;
		}
		pilotVideoPilot = NULL;
	}
	s_slotUsed[damageIconIndex] = 0;

	if ( backDamageIndex != -1 )
		s_slotUsed[ backDamageIndex ] = 0;
}

void ForceGroupIcon::init( FitIniFile& file, int which )
{
	char buffer[256];
	sprintf( buffer, "MechIcon%ld", which );
	if ( NO_ERR != file.seekBlock( buffer ) )
		Assert( 0, 0, "couldn't find the icon" );

	if ( !gosFontHandle )
		gosFontHandle = new aFont();

	file.readIdLong( "TextAReaLeft", 	textArea[which].left );
	file.readIdLong( "TextAreaRight", 	textArea[which].right );
	file.readIdLong( "TextAReaTop", 	textArea[which].top );
	file.readIdLong( "TextAreaBottom", 	textArea[which].bottom );
	
	textArea[which].left	+= ControlGui::hiResOffsetX;
	textArea[which].right	+= ControlGui::hiResOffsetX; 
	textArea[which].top     += ControlGui::hiResOffsetY; 
	textArea[which].bottom  += ControlGui::hiResOffsetY; 

	file.readIdLong( "PilotTextTop", 	pilotTextTop[which] );
	
	pilotTextTop[which] += ControlGui::hiResOffsetY;  

	long left, right, top, bottom;
	file.readIdLong( "MechIconAreaLeft", 	left );
	file.readIdLong( "MechIconArearight", 	right );
	file.readIdLong( "MechIconAreaTop", 	top );
	file.readIdLong( "MechIconAreaBottom", 	bottom );
	
	left	+= ControlGui::hiResOffsetX;
	right   += ControlGui::hiResOffsetX;
	top     += ControlGui::hiResOffsetY;
	bottom  += ControlGui::hiResOffsetY;

	for ( int i = 0; i < 4; i++ )
	{
		bmpLocation[which][i].x = left;
		bmpLocation[which][i].y = top;
		bmpLocation[which][i].argb = 0xffffffff;
		bmpLocation[which][i].rhw = 0.5;
	}

	bmpLocation[which][2].x = bmpLocation[which][3].x = right;
	bmpLocation[which][1].y = bmpLocation[which][2].y = bottom;

	bmpLocation[which][4] = bmpLocation[which][0];

	file.readIdLong( "PilotAreaLeft", 	left );
	file.readIdLong( "PilotAreaRight",	right );
	file.readIdLong( "PilotAreaTop", 	top );
	file.readIdLong( "PilotAreaBottom", bottom );

	left	+= ControlGui::hiResOffsetX;
	right   += ControlGui::hiResOffsetX;
	top     += ControlGui::hiResOffsetY;
	bottom  += ControlGui::hiResOffsetY;


	pilotLocation[which][0] = left;
	pilotLocation[which][1] = top;
	pilotLocation[which][2] = right;
	pilotLocation[which][3] = bottom;
	
	file.readIdLong( "PilotRectLeft", 	pilotRect[which].left );
	file.readIdLong( "PilotRectRight", 	pilotRect[which].right );
	file.readIdLong( "PilotRectTop", 	pilotRect[which].top );
	file.readIdLong( "PilotRectBottom", pilotRect[which].bottom );
	
	pilotRect[which].left		+= ControlGui::hiResOffsetX;
	pilotRect[which].right      += ControlGui::hiResOffsetX;
	pilotRect[which].top        += ControlGui::hiResOffsetY;
	pilotRect[which].bottom     += ControlGui::hiResOffsetY;

	file.readIdLong( "SelectionRectLeft", 	selectionRect[which].left );
	file.readIdLong( "SelectionRectRight", 	selectionRect[which].right );
	file.readIdLong( "SelectionRectTop", 	selectionRect[which].top );
	file.readIdLong( "SelectionRectBottom", selectionRect[which].bottom );
	
	selectionRect[which].left   += ControlGui::hiResOffsetX;
	selectionRect[which].right  += ControlGui::hiResOffsetX;
	selectionRect[which].top    += ControlGui::hiResOffsetY;
	selectionRect[which].bottom += ControlGui::hiResOffsetY;

	file.readIdLong( "HealthBarLeft", 	healthBar[which].left );
	file.readIdLong( "HealthBarRight", 	healthBar[which].right );
	file.readIdLong( "HealthBarTop", 	healthBar[which].top );
	file.readIdLong( "HealthBarBottom", healthBar[which].bottom );
	
	healthBar[which].left       += ControlGui::hiResOffsetX;
	healthBar[which].right      += ControlGui::hiResOffsetX;
	healthBar[which].top        += ControlGui::hiResOffsetY;
	healthBar[which].bottom     += ControlGui::hiResOffsetY;

	if ( !jumpJetIcon )
		jumpJetIcon = new StaticInfo;

	jumpJetIcon->init( file, "JumpJetIcon", 0, 0 );

	
}


void MechIcon::setDrawBack( bool bSet)
{
	bDrawBack = bSet;

	bool bFound = 0;

	for ( int index = 0; index < 240; index++ )
	{
		if ( !s_slotUsed[ index ] )
		{
			backDamageIndex = index;
			s_slotUsed[index] = 1;
			bFound = 1;
			break;
		}
	}

	
	int iconsPerLine = ((int)s_textureMemory->width/(int)unitIconX);
	int iconsPerPage = ((int)s_textureMemory->width/(int)unitIconY);
	long textureIndex = 0;
	long row = (backDamageIndex/iconsPerLine);
	if ( row >= iconsPerPage )
	{
		textureIndex = row/iconsPerPage;
		row = row % iconsPerPage;
	}

	long column = (backDamageIndex % iconsPerLine);
	long offsetY = row * unitIconY;
	long offsetX = column  * unitIconX;

	Assert( bFound, bFound, "out of space for icon textures" );

	if ( s_textureHandle[textureIndex] == -1 )
	{
		long size = ( 256 * 256 * sizeof( long ) + sizeof( TGAFileHeader ) );
		s_textureHandle[textureIndex] = gos_NewTextureFromMemory( gos_Texture_Alpha, ".tga", (unsigned char*)s_textureMemory, size, 0 );
	}

	
	// now need to make a copy of the data
	TEXTUREPTR textureData;
	gos_LockTexture( s_textureHandle[textureIndex], 0, 0, &textureData );


	DWORD* pDestData, *pDestRow = textureData.pTexture + offsetY * textureData.Width + offsetX;
	char* pTmp = (char*)s_MechTextures + sizeof ( TGAFileHeader );
	DWORD* pSrcRow = (DWORD*)pTmp;

	long whichMech = unit->getIconPictureIndex();

	offsetY = unitIconY;
	offsetX = whichMech  * unitIconX;

	long tmpOffset = ((s_MechTextures->width) * (offsetY ) + offsetX);

	pSrcRow += tmpOffset;

	DWORD* pSrcData = pSrcRow;

	for( int j = 0; j < unitIconY; ++j )
	{
		pDestData = pDestRow;
		pSrcData = pSrcRow;
		for ( int i = 0; i < unitIconX; ++i ) // do four icons per row
		{	
			bool bDraw = 0;
			// compare colors, only draw back ones which are 4, 5, 6, and 7
			for ( int i = 0; i < 3; i++ )
			{
				long compColor =  damageColors[0][i];
				long srcColor = *pSrcData;

				long compMin = compColor >> 16;
				compMin += 4;
				long compMax = compMin + 3;

				if ( srcColor >> 16 >= compMin && srcColor >> 16 <= compMax )
				{
					bDraw = 1;
				}
			}
			if (  *pSrcData == 0xff505050 )
				bDraw = 1;

			if ( bDraw )
				*pDestData++ = *pSrcData;
			else
				*pDestData++ = 0;

			pSrcData++;
		}
		pSrcRow += s_MechTextures->width;
		pDestRow += textureData.Width;
	}


	gos_UnLockTexture( s_textureHandle[textureIndex] );

}
bool ForceGroupIcon::inRegion( int x, int y )
{
	return ( x >= selectionRect[locationIndex].left && x <= selectionRect[locationIndex].right &&
			y >= selectionRect[locationIndex].top && y <= selectionRect[locationIndex].bottom );
}

void ForceGroupIcon::rightClick()
{
	((GameCamera*)eye)->setTarget( unit );
}

void ForceGroupIcon::click( bool shiftDn )
{
	if ( shiftDn && !unit->isSelected() )
	{
		unit->setSelected( true );
	}
	else if ( !shiftDn )
	{
		unit->setSelected( true );
	}
	else
	{
		unit->setSelected( false );
	}

	soundSystem->playDigitalSample( LOG_SELECT );
}

void ForceGroupIcon::init( )
{
}

bool MechIcon::initTextures()
{
	if ( !s_MechTextures )
	{
		File file;
		char path[256];
		strcpy( path, artPath );
		
		if ( Environment.screenWidth == 800 )
			strcat( path, "mcui_med4.tga" );
		else if ( Environment.screenWidth == 640 )
			strcat( path, "mcui_low4.tga" );
		else 
			strcat( path, "mcui_high7.tga" );

		_strlwr( path );

		if ( NO_ERR != file.open( path ) ) 
		{
			Assert( 0, 0, "Couldn't open the mech parts" );
			return false;
		}

		setIconVariables();

		int size = file.getLength();

		s_MechTextures = (TGAFileHeader*)new char[size];
		file.read( (BYTE*)s_MechTextures, size );
		BYTE* pTmp = (BYTE*)(s_MechTextures + 1);

		flipTopToBottom (pTmp, s_MechTextures->pixel_depth, s_MechTextures->width, s_MechTextures->height );

	}

	return true;


}

bool MechIcon::init( Mover* pMover )
{
	ForceGroupIcon::init();

	if ( !pMover )
		return 0;

	long whichMech = pMover->getIconPictureIndex();
	bool bRetVal = init( whichMech );

	unit = pMover;
	pilotIcon.setPilot( pMover->getPilot() );

	return bRetVal;


}

bool MechIcon::init( long whichIndex )
{
	initTextures();

	int pixelDepth = s_MechTextures->pixel_depth/8; 
	if ( pixelDepth != 4 )
		PAUSE(("Texture for MechIcon not 32-bit"));


	int iconsPerLine = ((int)s_textureMemory->width/(int)unitIconX);
	int iconsPerPage = ((int)s_textureMemory->width/(int)unitIconY);
	long textureIndex = 0;
	long row = (damageIconIndex/iconsPerLine);
	if ( row >= iconsPerPage )
	{
		textureIndex = row/iconsPerPage;
		row = row % iconsPerPage;
	}
	long column = (damageIconIndex % iconsPerLine);
	long offsetY = (row) * unitIconY;
	long offsetX = (column)  * unitIconX;

	if ( s_textureHandle[textureIndex] == -1 )
	{
		long size = ( 256 * 256 * sizeof( long ) + sizeof( TGAFileHeader ) );
		s_textureHandle[textureIndex] = gos_NewTextureFromMemory( gos_Texture_Alpha, ".tga", (unsigned char*)s_textureMemory, size, 0 );
	}

	// now need to make a copy of the data
	TEXTUREPTR textureData;
	gos_LockTexture( s_textureHandle[textureIndex], 0, 0, &textureData );

	DWORD* pDestData, *pDestRow = textureData.pTexture + offsetY * textureData.Width + offsetX;
	char* pTmp = (char*)s_MechTextures + sizeof ( TGAFileHeader );
	DWORD* pSrcRow = (DWORD*)pTmp;


	offsetY = 0;
	offsetX = whichIndex * unitIconX;

	long tmpOffset = ((s_MechTextures->width) * (offsetY ) + offsetX);

	pSrcRow += tmpOffset;

	DWORD* pSrcData = pSrcRow;

	for( int j = 0; j < unitIconY; ++j )
	{
		pDestData = pDestRow;
		pSrcData = pSrcRow;
		for ( int i = 0; i < unitIconX; ++i ) // do four icons per row
		{	
			*pDestData++ = *pSrcData++;
		}
		pSrcRow += s_MechTextures->width;
		pDestRow += textureData.Width;
	}


	gos_UnLockTexture( s_textureHandle[textureIndex] );


	for ( int i = 0; i < 5; ++i )
	{
		bmpLocation[locationIndex][i].argb = 0xffffffff;
		bmpLocation[locationIndex][i].frgb = 0;
		bmpLocation[locationIndex][i].rhw = .5;
		bmpLocation[locationIndex][i].u = 0.;
		bmpLocation[locationIndex][i].v = 0.;
		bmpLocation[locationIndex][i].z = 0.;
	}

	memset( damage, 0, 8 );
	memset( backDamage, 0, 8 );

	return true;

	
}

void MechIcon::update()
{
	// need to check object's status here
	char newDamage[8];
	char rearDamage[8];

	memset( newDamage, 0, 8 );
	memset( rearDamage, 0, 8 );

	// do check against armor location
	ArmorLocation armor = unit->armor[MECH_ARMOR_LOCATION_HEAD];
	float tmp = armor.curArmor/(float)armor.maxArmor;
	if (unit->body[MECH_BODY_LOCATION_HEAD].damageState == IS_DAMAGE_DESTROYED)
		tmp = -1.0f;

	float backTmp = 0;

	if ( tmp < .0 )
		newDamage[7] = 3;
	else if ( tmp < .3 )
		newDamage[7] = 2;
	else if ( tmp < .6 )
		newDamage[7] = 1;

	tmp = unit->armor[MECH_ARMOR_LOCATION_CTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_CTORSO].maxArmor;
	if ( !bDrawBack )
	{
		if ( (float)unit->armor[MECH_ARMOR_LOCATION_RCTORSO].maxArmor )
			tmp += unit->armor[MECH_ARMOR_LOCATION_RCTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RCTORSO].maxArmor;
		tmp/= 2.f;
	}
	else
	{
		backTmp = unit->armor[MECH_ARMOR_LOCATION_RCTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RCTORSO].maxArmor;
	}
	
	if (unit->body[MECH_BODY_LOCATION_CTORSO].damageState == IS_DAMAGE_DESTROYED)
	{
		tmp = -1.0f;
		backTmp = -1.0f;
	}

	if ( tmp < .0 )
		newDamage[6] = 3;
	else if ( tmp < .3 )
		newDamage[6] = 2;
	else if ( tmp < .6 )
		newDamage[6] = 1;

	if ( backTmp < .0 )
		rearDamage[6] = 3;
	else if ( backTmp < .3 )
		rearDamage[6] = 2;
	else if ( backTmp < .6 )
		rearDamage[6] = 1;


	tmp = unit->armor[MECH_ARMOR_LOCATION_LTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_LTORSO].maxArmor;
	if ( !bDrawBack )
	{
		if ( (float)unit->armor[MECH_ARMOR_LOCATION_RLTORSO].maxArmor )
			tmp += unit->armor[MECH_ARMOR_LOCATION_RLTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RLTORSO].maxArmor;
		tmp/= 2.f;
	}
	else
	{
		backTmp = unit->armor[MECH_ARMOR_LOCATION_RLTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RLTORSO].maxArmor;

	}

	if (bMovie)
	{
		bool result = bMovie->update();
		if (result)
		{
			delete bMovie;
			bMovie = NULL;
		}
	}

	if (unit->body[MECH_BODY_LOCATION_LTORSO].damageState == IS_DAMAGE_DESTROYED)
	{
		tmp = -1.0f;
		backTmp = -1.0f;
	}

	if ( tmp < .0 )
		newDamage[5] = 3;
	else if ( tmp < .3 )
		newDamage[5] = 2;
	else if ( tmp < .6 )
		newDamage[5] = 1;

	if ( backTmp < .0 )
		rearDamage[5] = 3;
	else if ( backTmp < .3 )
		rearDamage[5] = 2;
	else if ( backTmp < .5 )
		rearDamage[5] = 1;


	tmp = unit->armor[MECH_ARMOR_LOCATION_RTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RTORSO].maxArmor;
	if ( !bDrawBack )
	{
		if ( (float)unit->armor[MECH_ARMOR_LOCATION_RRTORSO].maxArmor )
			tmp += unit->armor[MECH_ARMOR_LOCATION_RRTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RRTORSO].maxArmor;
		tmp/= 2.f;
	}
	else
	{
		backTmp = unit->armor[MECH_ARMOR_LOCATION_RRTORSO].curArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RRTORSO].maxArmor;
	}
	
	if (unit->body[MECH_BODY_LOCATION_RTORSO].damageState == IS_DAMAGE_DESTROYED)
	{
		tmp = -1.0f;
		backTmp = -1.0f;
	}

	if ( tmp < .0 )
		newDamage[4] = 3;
	else if ( tmp < .3 )
		newDamage[4] = 2;
	else if ( tmp < .6 )
		newDamage[4] = 1;

	if ( backTmp < .0 )
		rearDamage[4] = 3;
	else if ( backTmp < .3 )
		rearDamage[4] = 2;
	else if ( backTmp < .4 )
		rearDamage[4] = 1;


	armor = unit->armor[MECH_ARMOR_LOCATION_LLEG];
	tmp = armor.curArmor/(float)armor.maxArmor;
	if (unit->body[MECH_BODY_LOCATION_LLEG].damageState == IS_DAMAGE_DESTROYED)
	{
		tmp = -1.0f;
	}

	if ( tmp < .0 )
		newDamage[0] = 3;
	else if ( tmp < .3 )
		newDamage[0] = 2;
	else if ( tmp < .6 )
		newDamage[0] = 1;

	armor = unit->armor[MECH_ARMOR_LOCATION_RLEG];
	tmp = armor.curArmor/(float)armor.maxArmor;
	if (unit->body[MECH_BODY_LOCATION_RLEG].damageState == IS_DAMAGE_DESTROYED)
	{
		tmp = -1.0f;
	}

	if ( tmp < .0 )
		newDamage[1] = 3;
	else if ( tmp < .3 )
		newDamage[1] = 2;
	else if ( tmp < .6 )
		newDamage[1] = 1;

	armor = unit->armor[MECH_ARMOR_LOCATION_LARM];
	tmp = armor.curArmor/(float)armor.maxArmor;
	if (unit->body[MECH_BODY_LOCATION_LARM].damageState == IS_DAMAGE_DESTROYED)
	{
		tmp = -1.0f;
	}

	if ( tmp < .0 )
		newDamage[2] = 3;
	else if ( tmp < .3 )
		newDamage[2] = 2;
	else if ( tmp < .6 )
		newDamage[2] = 1;

	armor = unit->armor[MECH_ARMOR_LOCATION_RARM];
	tmp = armor.curArmor/(float)armor.maxArmor;
	if (unit->body[MECH_BODY_LOCATION_RARM].damageState == IS_DAMAGE_DESTROYED)
	{
		tmp = -1.0f;
		backTmp = -1.0f;
	}

	if ( tmp < .0 )
		newDamage[3] = 3;
	else if ( tmp < .3 )
		newDamage[3] = 2;
	else if ( tmp < .6 )
		newDamage[3] = 1;

	doDraw( newDamage, damage, -1, damageIconIndex );
	if ( bDrawBack )
	{
		float lratio = 1.f;
		float rratio = 1.f;
		if ( (float)unit->armor[MECH_ARMOR_LOCATION_RCTORSO].maxArmor )
		{
			lratio = (float)unit->armor[MECH_ARMOR_LOCATION_RLTORSO].maxArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RCTORSO].maxArmor;
			rratio = (float)unit->armor[MECH_ARMOR_LOCATION_RRTORSO].maxArmor/(float)unit->armor[MECH_ARMOR_LOCATION_RCTORSO].maxArmor;
		}
		rearDamage[7] = .5 + (float)((rratio * rearDamage[4] + lratio * rearDamage[5] + rearDamage[6]))/(lratio + rratio + 1.f);
		backDamage[7] = .5 + (float)((rratio * backDamage[4] + lratio * backDamage[5] + backDamage[6]))/(lratio + rratio + 1.f);
		doDraw( rearDamage, backDamage, -1, backDamageIndex );
	}

}

void MechIcon::doDraw( char* newDamage, char* oldDamage, unsigned long handle, unsigned long index )
{
	int i = 0; 
	int maxToCheck = 8;
	if ( index == backDamageIndex )
		i = 7;

	for ( ; i < maxToCheck; ++i )
	{
		// if damage has changeed, update that part of the icon, we really don't want to do this
		// every frame
		if ( newDamage[i] != oldDamage[i] )
		{			
			// find the icon
			long texIndex = 0;
			int iconsPerLine = ((int)s_textureMemory->width/(int)unitIconX);
			int iconsPerPage = ((int)s_textureMemory->width/(int)unitIconY);
			int yIndex = index / iconsPerLine;
			if ( yIndex >= iconsPerPage )
			{
				texIndex = yIndex/iconsPerPage;
				yIndex = yIndex % iconsPerPage;
			}
			int xIndex = index % iconsPerLine;

			if ( s_textureHandle[texIndex] == -1 )
			{
				long size = ( 256 * 256 * sizeof( long ) + sizeof( TGAFileHeader ) );
				s_textureHandle[texIndex] = gos_NewTextureFromMemory( gos_Texture_Alpha, ".tga", (unsigned char*)s_textureMemory, size, 0 );
			}

			TEXTUREPTR textureData;
			gos_LockTexture( s_textureHandle[texIndex], 0, 0, &textureData );

			DWORD* pData = textureData.pTexture;

			// go to write spot in texture
			int jLine = yIndex * unitIconY;

			int iLine = xIndex * unitIconX;

			DWORD* pLine = (DWORD*)(pData + (jLine * textureData.Width) + iLine);
			DWORD* pChange = pLine;

			DWORD oldColors[3];
			DWORD newColors[3];

			// find the colors we're looking for
			for ( int p = 0; p < 3; p++ )
			{
				int tmp = damageColors[damage[i]][p];
				int redVal = (tmp >> 16) & 0x000000ff;
				redVal += i;
				oldColors[p] = ( tmp & 0xff00ffff ) | (redVal << 16);

				tmp = damageColors[newDamage[i]][p];
				redVal = (tmp >> 16) & 0x000000ff;
				redVal += i;
				newColors[p] = ( tmp & 0xff00ffff ) | (redVal << 16);

			}
			
			// look through the entire bitmap, changing old colors to new colors
			for ( int k = 0; k < unitIconY; ++k )
			{
				pChange = pLine;
				for ( int j = 0; j < unitIconX; ++j )
				{

					if ( (*pChange)&0xff000000 ) // not transparent
					{
						for (int p = 0; p < 3; p++ )
						{
							if ( oldColors[p] == *pChange )		
							{
								*pChange = newColors[p];
							}
						}
					}
					pChange++;

				}
				pLine += textureData.Width;				
			}

			gos_UnLockTexture( s_textureHandle[texIndex] );
		}
		oldDamage[i] = newDamage[i];
	}


}

bool IsDbcsString(const char* pszString)
{
	if (pszString) {
		while (*pszString) {
			if (IsDBCSLeadByte((BYTE)*pszString))
				return true;
			pszString++;
		}
	}
	return false;
}


void ForceGroupIcon::render()
{
	// here I draw the text
	if ( !unit )
		return;

	int forceGroup = -1;
	for ( int i = 0; i < 10; ++i )
	{
		if ( unit->isInUnitGroup( i ) )
			forceGroup = i;
	}

	
	// draw little colored background
	if ( forceGroup == 0 )
		forceGroup = 10;
	int color = forceGroup == -1 ? ForceGroupColors[10] : ForceGroupColors[forceGroup-1];
	float health = unit->getAppearance()->barStatus;

	
	// draw pilotName
	
	GUI_RECT tmpRect = { textArea[locationIndex].left, textArea[locationIndex].top, textArea[locationIndex].right, textArea[locationIndex].bottom };
	drawRect( tmpRect, color);

	char buffer[256];
	strcpy( buffer, unit->getPilot()->getName() );
	for (int i = 0; i < strlen( buffer ); i++ )
		CharUpper( buffer );

	gosFontHandle->render( buffer, textArea[locationIndex].left + 1, pilotTextTop[locationIndex], 0, 0, 0xffffffff, 0, 0 );

	// draw the health bar
	drawBar( health );

	// draw the little unit icon
	renderUnitIcon( bmpLocation[locationIndex][0].x, bmpLocation[locationIndex][0].y, bmpLocation[locationIndex][2].x, bmpLocation[locationIndex][2].y );
	
	pilotIcon.render( pilotLocation[locationIndex][0], pilotLocation[locationIndex][1], pilotLocation[locationIndex][2], pilotLocation[locationIndex][3] );

	drawEmptyRect( pilotRect[locationIndex], 0xff002f55, 0xff002f55 );



	// if has jumpjets, draws jump jet icon
	if ( unit->canJump() )
	{
		jumpJetIcon->move( selectionRect[locationIndex].left, selectionRect[locationIndex].bottom );
		jumpJetIcon->render( );
		jumpJetIcon->move( -selectionRect[locationIndex].left, -selectionRect[locationIndex].bottom );
	}
	



	if ( pilotVideoPilot == unit->getPilot()  )
	{
		gos_VERTEX v[4];
		for (int i = 0; i < 4; i++ )
		{
			v[i] = bmpLocation[locationIndex][i];
			v[i].u = 0.f;
			v[i].v = 0.f;
		}

		v[2].x = v[3].x = pilotLocation[locationIndex][2];
		v[2].u = v[3].u = .59375;
		v[1].v = v[2].v = .828125f;

		RECT vRect;
		vRect.left 		= bmpLocation[locationIndex][1].x;
		vRect.right 	= pilotLocation[locationIndex][2];
		vRect.top 		= bmpLocation[locationIndex][3].y;
		vRect.bottom 	= bmpLocation[locationIndex][1].y;

		if ( bMovie )
		{
			if ( unit && (unit->getStatus () == OBJECT_STATUS_SHUTDOWN) )
			{
				bMovie->stop();
				delete bMovie;
				bMovie = NULL;
				pilotVideoPilot = NULL;
			}
			else
			{
				bMovie->setRect(vRect);
				bMovie->render();
			}
		}
		else if ( pilotVideoTexture )
		{
			gos_SetRenderState( gos_State_Texture,  pilotVideoTexture );
			v[1].v = v[2].v = .4140625;

			gos_DrawQuads( v, 4 );
		}
	}


	if ( unit && (unit->getStatus () == OBJECT_STATUS_SHUTDOWN) )
	{
		drawRect( selectionRect[locationIndex], 0xaa000000 );
	}


	// if selected, draw white rectangle
	if ( unit->getSelected() )
	{
		drawEmptyRect( selectionRect[locationIndex], 0xffffffff, 0xffffffff );
	}

	if ( unit->getPilot() )
	{
		if ( unit->getPilot()->getMessagePlaying() )
		{
			long color = 0xffffff00;
			msgPlayTime += frameLength;
			if ( msgPlayTime > .25 )
				msgPlayTime = 0;
			if ( msgPlayTime > .125 )
				color = 0x00000000;
			drawEmptyRect( selectionRect[locationIndex], color, color );
		}
		else
			msgPlayTime = 0;
	}

	// draw dying effect
	if ( deathAnimationTime )
	{
		drawDeathEffect();
	}


}

void ForceGroupIcon::renderUnitIcon( float left, float top, float right, float bottom )
{
	int texIndex = 0;
	int iconsPerLine = ((int)s_textureMemory->width/(int)unitIconX);
	int iconsPerPage= ((int)s_textureMemory->width/(int)unitIconY);
	int yIndex = damageIconIndex / iconsPerLine;
	if ( yIndex >= iconsPerPage )
	{
		texIndex = yIndex / iconsPerPage;
		yIndex = yIndex % iconsPerPage;
	}
	int xIndex = damageIconIndex % iconsPerLine;
	
	float u = xIndex * unitIconX/s_textureMemory->width + (.1f / 256.f);
	float v = yIndex * unitIconY/s_textureMemory->height+ (.1f / 256.f);

	float uDelta = unitIconX/s_textureMemory->width + (.1f / 256.f );
	float vDelta = unitIconY/s_textureMemory->height + (.1f / 256.f);

	gos_SetRenderState( gos_State_Texture, s_textureHandle[texIndex] );
	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
	gos_SetRenderState( gos_State_Filter, gos_FilterNone);
	gos_SetRenderState( gos_State_AlphaTest, true);


	bmpLocation[locationIndex][0].u = bmpLocation[locationIndex][1].u = u;
	bmpLocation[locationIndex][2].u = bmpLocation[locationIndex][3].u = u + uDelta;
	bmpLocation[locationIndex][0].v = bmpLocation[locationIndex][3].v = v;
	bmpLocation[locationIndex][1].v = bmpLocation[locationIndex][2].v = v + vDelta;
	
	bmpLocation[locationIndex][0].x = 	bmpLocation[locationIndex][1].x = left;
	bmpLocation[locationIndex][2].x = bmpLocation[locationIndex][3].x = right;
	bmpLocation[locationIndex][0].y = bmpLocation[locationIndex][3].y = top;
	bmpLocation[locationIndex][1].y = bmpLocation[locationIndex][2].y  = bottom;
	bmpLocation[locationIndex][4] = bmpLocation[locationIndex][0];

	gos_DrawTriangles( bmpLocation[locationIndex], 3 );
	gos_DrawTriangles( &bmpLocation[locationIndex][2], 3 );
			

}

void ForceGroupIcon::renderUnitIconBack( float left, float top, float right, float bottom )
{
	int iconsPerLine = ((int)s_textureMemory->width/(int)unitIconX);
	int yIndex = backDamageIndex / iconsPerLine;
	int xIndex = backDamageIndex % iconsPerLine;
	
	float u = xIndex * unitIconX/s_textureMemory->width+ (.1f / (float)256.f);
	float v = yIndex * unitIconY/s_textureMemory->height+ (.1f / (float)256.f);

	float uDelta = unitIconX/s_textureMemory->width + (.1f / (float)256.f );
	float vDelta = unitIconY/s_textureMemory->height + (.1f / (float)256.f);

	gos_SetRenderState( gos_State_Texture, s_textureHandle[0] );
	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
	gos_SetRenderState( gos_State_Filter, gos_FilterNone);
	gos_SetRenderState( gos_State_AlphaTest, true);

	gos_VERTEX vertices[5];

	for ( int i = 0; i < 5; i++ )
	{
		vertices[i].argb = 0xffffffff;
		vertices[i].frgb = 0;
		vertices[i].z = 0;
		vertices[i].rhw = .5;
	}

	vertices[0].u = vertices[1].u = u;
	vertices[2].u = vertices[3].u = u + uDelta;
	vertices[0].v = vertices[3].v = v;
	vertices[1].v = vertices[2].v = v + vDelta;
	
	vertices[0].x = 	vertices[1].x = left;
	vertices[2].x = vertices[3].x = right;
	vertices[0].y = vertices[3].y = top;
	vertices[1].y = vertices[2].y  = bottom;
	vertices[4] = vertices[0];

	gos_DrawTriangles( vertices, 3 );
	gos_DrawTriangles( &vertices[2], 3 );
	
}
void ForceGroupIcon::renderPilotIcon( float left, float top, float right, float bottom )
{
	pilotIcon.render( left, top, right, bottom );
}


bool VehicleIcon::init( Mover* pMover )
{
	if ( !s_VehicleTextures )
	{
		File file;
		char path[256];
		strcpy( path, artPath );
		if ( Environment.screenWidth == 800 )
			strcat( path, "mcui_med5.tga" );
		else if ( Environment.screenWidth == 640 )
			strcat( path, "mcui_low5.tga" );
		else 
			strcat( path, "mcui_high8.tga" );

		setIconVariables();

		
		if ( NO_ERR != file.open( path ) )
		{
			Assert( 0, 0, "Couldn't open the vehicle parts" );
			return false;
		}

		int size = file.getLength();

		s_VehicleTextures = (TGAFileHeader*)new char[size];
		file.read( (BYTE*)s_VehicleTextures, size );
		BYTE* pTmp = (BYTE*)(s_VehicleTextures + 1); 

		flipTopToBottom (pTmp, s_VehicleTextures->pixel_depth, s_VehicleTextures->width, s_VehicleTextures->height );

	}

	pilotIcon.setTextureIndex( 26 );

	if ( !pMover )
		return 0;

	gosASSERT( unitIconX * 4 < ICON_REALSIZE && unitIconY * 4 < ICON_REALSIZE );
	int pixelDepth = s_VehicleTextures->pixel_depth/8; 
	if ( pixelDepth != 4 )
		PAUSE(("Texture for VehicleIcon not 32-bit"));

	// now need to make a copy of the data
	// now need to make a copy of the data
	
	long texIndex = 0;
	int iconsPerLine = ((int)s_textureMemory->width/(int)unitIconX);
	int iconsPerPage = ((int)s_textureMemory->width/(int)unitIconY);
	long row = damageIconIndex/iconsPerLine;
	long column = (damageIconIndex % iconsPerLine);
	if ( row >= iconsPerPage )
	{
		texIndex = row/iconsPerPage;
		row = row % iconsPerPage;
	}

	
	long offsetX =  column * unitIconX;
	long offsetY = row * unitIconY;
	
	if ( s_textureHandle[texIndex] == -1 )
	{
		long size = ( 256 * 256 * sizeof( long ) + sizeof( TGAFileHeader ) );
		s_textureHandle[texIndex] = gos_NewTextureFromMemory( gos_Texture_Alpha, ".tga", (unsigned char*)s_textureMemory, size, 0 );
	}


	TEXTUREPTR textureData;
	gos_LockTexture( s_textureHandle[texIndex], 0, 0, &textureData );

	
	DWORD* pDestData, *pDestRow = textureData.pTexture + offsetY * textureData.Width + offsetX;
	char* pTmp = (char*)s_VehicleTextures + sizeof ( TGAFileHeader );
	DWORD* pSrcRow = (DWORD*)pTmp;

	long whichMech = pMover->getIconPictureIndex();

	offsetY = 0;
	offsetX = whichMech * unitIconX;

	long tmpOffset = ((s_VehicleTextures->width) * (offsetY ) + offsetX);

	pSrcRow += tmpOffset;

	DWORD* pSrcData = pSrcRow;

	for( int j = 0; j < unitIconY; ++j )
	{
		pDestData = pDestRow;
		pSrcData = pSrcRow;
		for ( int i = 0; i < unitIconX; ++i ) // do four icons per row
		{	
			*pDestData++ = *pSrcData++;
		}
		pSrcRow += s_VehicleTextures->width;
		pDestRow += textureData.Width;
	}


	gos_UnLockTexture( s_textureHandle[texIndex] );
	
	unit = pMover;


	for ( int i = 0; i < 5; ++i )
	{
		bmpLocation[locationIndex][i].argb = 0xffffffff;
		bmpLocation[locationIndex][i].frgb = 0;
		bmpLocation[locationIndex][i].rhw = .5;
		bmpLocation[locationIndex][i].u = 0.;
		bmpLocation[locationIndex][i].v = 0.;
		bmpLocation[locationIndex][i].z = 0.;
	}

	memset( damage, 0, 5 );

	return true;

}

void VehicleIcon::update()
{
	char newDamage[5];

	memset( newDamage, 0, 5 );

	// do check against armor location
	ArmorLocation armor = unit->armor[GROUNDVEHICLE_LOCATION_FRONT];
	float tmp = armor.curArmor/(float)armor.maxArmor;
	
	if ( tmp < .1 )
		newDamage[0] = 3;
	else if ( tmp < .3 )
		newDamage[0] = 2;
	else if ( tmp < .6 )
		newDamage[0] = 1;

	tmp = unit->armor[GROUNDVEHICLE_LOCATION_REAR].curArmor/(float)unit->armor[GROUNDVEHICLE_LOCATION_REAR].maxArmor;
	if ( tmp < .1 )
		newDamage[1] = 3;
	else if ( tmp < .3 )
		newDamage[1] = 2;
	else if ( tmp < .6 )
		newDamage[1] = 1;

	tmp = unit->armor[GROUNDVEHICLE_LOCATION_LEFT].curArmor/(float)unit->armor[GROUNDVEHICLE_LOCATION_LEFT].maxArmor;
		
	if ( tmp < .1 )
		newDamage[2] = 3;
	else if ( tmp < .3 )
		newDamage[2] = 2;
	else if ( tmp < .6 )
		newDamage[2] = 1;


	tmp = unit->armor[GROUNDVEHICLE_LOCATION_RIGHT].curArmor/(float)unit->armor[GROUNDVEHICLE_LOCATION_RIGHT].maxArmor;

	if ( tmp < .1 )
		newDamage[3] = 3;
	else if ( tmp < .3 )
		newDamage[3] = 2;
	else if ( tmp < .6 )
		newDamage[3] = 1;

	armor = unit->armor[GROUNDVEHICLE_LOCATION_TURRET];
	tmp = armor.curArmor/(float)armor.maxArmor;
	if ( tmp < .1 )
		newDamage[4] = 3;
	else if ( tmp < .3 )
		newDamage[4] = 2;
	else if ( tmp < .6 )
		newDamage[4] = 1;


	for ( int i = 0; i < 5; ++i )
	{
		if ( newDamage[i] != damage[i] )
		{	
			long texIndex = 0;
			int iconsPerLine = ((int)s_textureMemory->width/(int)unitIconX);
			int iconsPerPage = ((int)s_textureMemory->width/(int)unitIconY);
			int yIndex = damageIconIndex / iconsPerLine;
			int xIndex = damageIconIndex % iconsPerLine;
			if ( yIndex >= iconsPerPage )
			{
				texIndex = yIndex/iconsPerPage;
				yIndex = yIndex % iconsPerPage;
			}

			
			TEXTUREPTR textureData;
			gos_LockTexture( s_textureHandle[texIndex], 0, 0, &textureData );

			DWORD* pData = textureData.pTexture;

			// go to write spot in texture
			int jLine = yIndex * unitIconY;

			int iLine = xIndex * unitIconX;

			DWORD* pLine = (DWORD*)(pData + (jLine * textureData.Width) + iLine);
			DWORD* pChange = pLine;

			DWORD oldColors[3];
			DWORD newColors[3];

			for ( int p = 0; p < 3; p++ )
			{
				int tmp = damageColors[damage[i]][p];
				int redVal = (tmp >> 16) & 0x000000ff;
				redVal += i;
				oldColors[p] = ( tmp & 0xff00ffff ) | (redVal << 16);

				tmp = damageColors[newDamage[i]][p];
				redVal = (tmp >> 16) & 0x000000ff;
				redVal += i;
				newColors[p] = ( tmp & 0xff00ffff ) | (redVal << 16);

			}
			
	
			for ( int k = 0; k < unitIconY; ++k )
			{
				pChange = pLine;
				for ( int j = 0; j < unitIconX; ++j )
				{
					if ( (*pChange)&0xff000000 ) // not transparent
					{
						for ( int p = 0; p < 3; p++ )
						{
							if ( oldColors[p] == *pChange )		
							{
								*pChange = newColors[p];
							}
						}
					}
					pChange++;

				}
				pLine += textureData.Width;				
			}

			gos_UnLockTexture( s_textureHandle[texIndex] );
		}
		damage[i] = newDamage[i];
	}
}

void ForceGroupIcon::drawBar( float barStatus )
{
	
	DWORD					color = SB_GREEN;
	
	float barLength =  (healthBar[locationIndex].right - healthBar[locationIndex].left) * barStatus;
		
	/*if (barStatus >= 0.5)
		color = SB_GREEN;
	else if (barStatus > 0.2)
		color = SB_YELLOW;
	else 
		color = SB_RED;*/
		
	
	GUI_RECT rect = { healthBar[locationIndex].left, healthBar[locationIndex].top,healthBar[locationIndex].left + barLength, healthBar[locationIndex].bottom };
	drawRect( rect, color );
	drawEmptyRect( healthBar[locationIndex],  color, color & 0x7f7f7f );
}

int ForceGroupIcon::sort( const void* p1, const void* p2 )
{
	ForceGroupIcon* m1 = *(ForceGroupIcon**)p1;
	ForceGroupIcon* m2 = *(ForceGroupIcon**)p2;

	bool bv1 = dynamic_cast<VehicleIcon*>(m1) ? 1 : 0;
	bool bv2 = dynamic_cast<VehicleIcon*>(m2) ? 1 : 0;


	for ( int i = 1; i < 10; ++i )
	{
		bool bM1 =  m1->unit->isInUnitGroup( i == 10 ? 0 : i );
		bool bM2 =  m2->unit->isInUnitGroup( i == 10 ? 0 : i );

		if ( bM1 && !bM2 )
			return -1;
		if ( bM2 && !bM1 )
			return 1;
		if ( bM2 && bM1 ) // if same force group, use weight
		{
			break;
		}
	}

	if ( bv2 && !bv1 )
		return -1;
	else if ( bv1 && !bv2 )
		return 1;
	

	int w1 = m1->unit->getTonnage();
	int w2 = m2->unit->getTonnage();

	if ( w1 < w2  )
		return -1;
	else if ( w2 < w1)
		return 1;

	// OK these are exactly the same, compare pilot names
	int retVal = ( strcmp( m1->getPilotName(), m2->getPilotName() ) );

	if ( retVal == 0 )
		return m1 < m2;

	return retVal;
}

const char* ForceGroupIcon::getPilotName()
{
	return unit->getPilot()->getName();
}

void ForceGroupIcon::setLocationIndex( int i )
{ 
	locationIndex = i; 
}

void ForceGroupIcon::drawDeathEffect()
{
	unit->setSelected( 0 );
	bool bFinished = true;
	deathAnimationTime += frameLength;
    int i = 0;
	for (; i < NUM_DEATH_INFOS - 1; i++ )
	{
		if ( animationInfos[i].time < deathAnimationTime &&
			 animationInfos[i+1].time > deathAnimationTime )
		{
			float totalTime = animationInfos[i+1].time - animationInfos[i].time;
			float currentTime = deathAnimationTime - animationInfos[i].time;

			float percent = currentTime/totalTime;
			
			// since I know we're going from white to black, I'm only going to much with the alpha separately
			long alphaMin = (animationInfos[i].color >> 24) & 0xff;
			long alphaMax = (animationInfos[i+1].color >> 24)& 0xff;

			long newAlpha = (float)alphaMin + ((float)(alphaMax - alphaMin))*percent;
			
			long redMin = (animationInfos[i].color & 0x00ff0000)>>16;
			long redMax = (animationInfos[i+1].color & 0x00ff0000)>>16;
			long newRed = (float)redMin + ((float)(redMax - redMin))*percent;
			
			long greenMin = (animationInfos[i].color & 0x0000ff00)>>8;
			long greenMax = (animationInfos[i+1].color & 0x0000ff00)>>8;
			long newGreen = (float)greenMin + ((float)(greenMax - greenMin))*percent;
			
			long blueMin = animationInfos[i].color & 0x000000ff;
			long blueMax = animationInfos[i+1].color & 0x000000ff;
			long newBlue = (float)blueMin + ((float)(blueMax - blueMin))*percent;

			long newColor = newBlue + (newGreen << 8) + (newRed << 16) + (newAlpha <<24);
							
			drawRect( selectionRect[locationIndex], newColor );
			bFinished = false;

			if ( i == 0 )
			{
				long color = 0xff000000;
				if ( deathAnimationTime < .4 
					|| (deathAnimationTime > .8 && deathAnimationTime < 1.2)
					|| deathAnimationTime > 1.6 && deathAnimationTime < 2.0)
				{
					color = 0xffff0000;
				}
				drawEmptyRect( selectionRect[locationIndex], color, color );
			}
		}
	}

	if ( bFinished )
	{
		drawRect( selectionRect[locationIndex], animationInfos[i].color );
		unit->addToUnitGroup( -1 );
		//------------------------------------------------------------
		// This tells all machines to drop this mover from the team...
		if (MPlayer)
			MPlayer->sendReinforcement(0, unit->netRosterIndex, "noname", 0, unit->getPosition(), 7);
	}
}

void ForceGroupIcon::setIconVariables()
{
	switch( Environment.screenWidth )
	{
		case 800:
			PilotIcon::pilotIconX = 25;
			PilotIcon::pilotIconY = 36;
			unitIconX = 32;
			unitIconY = 38;
			break;

		case 640:
			PilotIcon::pilotIconX = 20;
			PilotIcon::pilotIconY = 28;
			unitIconX = 25;
			unitIconY = 30;
			break;

		default:
			PilotIcon::pilotIconX = 33;
			PilotIcon::pilotIconY = 46;
			unitIconX = 40;
			unitIconY = 48;
			break;

	}

	curScreenWidth = Environment.screenWidth;
}

PilotIcon::PilotIcon()
{
	pilotTextureIndex = 0;

	if ( !s_pilotTextureHandle )
	{
		// find proper file based on resolution
		char path[256];
		strcpy( path, "data\\art\\");
		if ( Environment.screenWidth == 640 )
		{
			strcat( path, "mcui_low2.tga" );
		}
		else if ( Environment.screenWidth == 800 )
		{
			strcat( path, "mcui_med2.tga" );
		}
		else
		{
			strcat( path, "mcui_high2.tga" );
		}

		s_pilotTextureHandle = mcTextureManager->loadTexture( path, gos_Texture_Alpha, 0, 0, 0x2);

		// find its width and store it for future reference.
		int gosID = mcTextureManager->get_gosTextureHandle( s_pilotTextureHandle );
		TEXTUREPTR textureData;
		gos_LockTexture( gosID, 0, 0, 	&textureData );
		s_pilotTextureWidth = textureData.Width;
		gos_UnLockTexture( gosID );
	}

}

void PilotIcon::render( float left, float top, float right, float bottom ) 
{
	gos_VERTEX pilotLocation[5];
	for ( int i = 0; i < 5; i++ )
	{
		pilotLocation[i].argb = 0xffffffff;
		pilotLocation[i].rhw = .5;
		pilotLocation[i].frgb = 0;
		pilotLocation[i].z = 0;
	}
	pilotLocation[0].x = pilotLocation[1].x = left;
	pilotLocation[3].x = pilotLocation[2].x = right;
	pilotLocation[0].y = pilotLocation[3].y = top;
	pilotLocation[1].y = pilotLocation[2].y = bottom;
	pilotLocation[4] = pilotLocation[0];

	int iconsPerRow = s_pilotTextureWidth / pilotIconX;
	int xIndex = pilotTextureIndex % iconsPerRow;
	int yIndex = pilotTextureIndex / iconsPerRow;

	float xLocation = xIndex * pilotIconX;
	float yLocation = yIndex * pilotIconY;
	pilotLocation[0].u = pilotLocation[1].u = xLocation/s_pilotTextureWidth + (.1f/s_pilotTextureWidth);
	pilotLocation[3].u = pilotLocation[2].u = (xLocation + pilotIconX)/s_pilotTextureWidth+ (.1f/s_pilotTextureWidth);
	pilotLocation[0].v = pilotLocation[3].v = yLocation/s_pilotTextureWidth+ (.1f/s_pilotTextureWidth);
	pilotLocation[1].v = pilotLocation[2].v = (yLocation + pilotIconY)/s_pilotTextureWidth+ (.1f/s_pilotTextureWidth);
	pilotLocation[4] = pilotLocation[0];


	int gosID = mcTextureManager->get_gosTextureHandle( s_pilotTextureHandle );
	gos_SetRenderState( gos_State_Texture, gosID );
	// here I need to draw the pilot icon
	gos_DrawTriangles( pilotLocation, 3 );
	gos_DrawTriangles( &pilotLocation[2], 3 );

}

void PilotIcon::setPilot( MechWarrior* pWarrior )
{
	if ( pWarrior )
		pilotTextureIndex = pWarrior->getPhoto();
}

void PilotIcon::swapResolutions( bool bForce)
{
	if ( s_pilotTextureHandle )
	{
		int gosID = mcTextureManager->get_gosTextureHandle( s_pilotTextureHandle );
		mcTextureManager->removeTexture( gosID );
	}
	s_pilotTextureHandle = 0;

	if ( !s_pilotTextureHandle )
	{
		// find proper file based on resolution
		char path[256];
		strcpy( path, "data\\art\\");
		if ( Environment.screenWidth == 640 )
		{
			strcat( path, "mcui_low2.tga" );
		}
		else if ( Environment.screenWidth == 800 )
		{
			strcat( path, "mcui_med2.tga" );
		}
		else
		{
			strcat( path, "mcui_high2.tga" );
		}

		s_pilotTextureHandle = mcTextureManager->loadTexture( path, gos_Texture_Alpha, 0, 0, 0x2);

		// find its width and store it for future reference.
		int gosID = mcTextureManager->get_gosTextureHandle( s_pilotTextureHandle );
		TEXTUREPTR textureData;
		gos_LockTexture( gosID, 0, 0, 	&textureData );
		s_pilotTextureWidth = textureData.Width;
		gos_UnLockTexture( gosID );
	}

}

//*************************************************************************************************
// end of file ( MechIcon.cpp )
