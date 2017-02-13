#define LOADSCREEN_CPP
/*************************************************************************************************\
LoadScreen.cpp			: Implementation of the LoadScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifdef LINUX_BUILD
#include"windows.h"
struct DDSURFACEDESC2 {
};
#else
#include<windows.h>
#include<ddraw.h>
#endif
#include"loadscreen.h"
#include"aanimobject.h"
#include"tgainfo.h"
#include"mclib.h"
#include"prefs.h"
#include"multplyr.h"
#include"mission.h"
#include"gamesound.h"
float loadProgress = 0.0f;

long LoadScreen::xProgressLoc = 0;
long LoadScreen::yProgressLoc = 0;
long LoadScreen::xWaitLoc = 0;
long LoadScreen::yWaitLoc = 0;
bool LoadScreen::turnOffAsyncMouse = false;

TGAFileHeader* LoadScreen::progressTextureMemory = 0;
TGAFileHeader* LoadScreen::progressBackground = 0;
TGAFileHeader* LoadScreen::mergedTexture = 0;
TGAFileHeader* LoadScreen::waitingForPlayersMemory = 0;

LoadScreen* LoadScreenWrapper::enterScreen = NULL;
LoadScreen* LoadScreenWrapper::exitScreen = NULL;


extern volatile bool mc2IsInMouseTimer;
extern volatile bool mc2IsInDisplayBackBuffer;

extern void (*AsynFunc)(RECT& WinRect,DDSURFACEDESC2& mouseSurfaceDesc );
extern CPrefs prefs;

void ProgressTimer(	RECT& WinRect,DDSURFACEDESC2& mouseSurfaceDesc );
//
// Returns the number of bits in a given mask.  Used to determine if we are in 555 mode vs 565 mode.
WORD GetNumberOfBits( DWORD dwMask );

void MouseTimerInit();
void MouseTimerKill();

LoadScreenWrapper::LoadScreenWrapper()
{
	//How about, for shits and giggles we toss the static members we created for single player when this inits for multiplayer
	// or any other player!!!!
	if ( enterScreen )
		delete enterScreen;
	if ( exitScreen )
		delete exitScreen;

	enterScreen = exitScreen = NULL;
	enterScreen = new LoadScreen;
	exitScreen = new LoadScreen;
	bFirstTime = 0;
}

LoadScreenWrapper::~LoadScreenWrapper()
{
	if ( enterScreen )
		delete enterScreen;
	if ( exitScreen )
		delete exitScreen;

	enterScreen = exitScreen = NULL;
}

void LoadScreenWrapper::init( FitIniFile& file )
{
	enterScreen->init( file );

//	changeRes();
	bFirstTime = 0;
}

void LoadScreenWrapper::changeRes()
{
	const char* Appendix = NULL;

    /*
	switch( prefs.resolution )
	{
	case 0:
		Appendix = "_640";
		break;

	case 1:
		break;

	case 2:
		Appendix = "_1024";

		break;

	case 3:
		Appendix = "_1280";

		break;

	case 4:
		Appendix = "_1600";

		break;

	default:
		Assert( 0, 0, "Unexpected resolution found in prefs" );
		break;
	}
    */

	switch( prefs.resolutionX )
	{
	case 640:
		Appendix = "_640";
		break;
	case 1024:
		Appendix = "_1024";
		break;
	case 1280:
		Appendix = "_1280";
		break;
	case 1600:
		Appendix = "_1600";
		break;
	case 1920:
		Appendix = "_1920";
		break;
	default:
		Assert( 0, 0, "Unexpected resolution found in prefs" );
		break;
	}

	char fileName[256];
	sprintf( fileName, "mcl_loadingscreen" );

	if ( Appendix )
		strcat( fileName, Appendix );

	FullPathFileName path;

	path.init( artPath, fileName, ".fit" );

	FitIniFile outFile;

	if ( NO_ERR != outFile.open( path ) )
	{
		char error[256];
		sprintf( error, "couldn't open file %s", (const char*)path );
		Assert( 0, 0, error );
		return;
	}

	//The 0x2 means that we do NOT want to flush this texture when we toss
	// the texture cache before a mission.  BUT we DO want the texture to cache out
	// to make more room for stuff.
	exitScreen->init( outFile, 0x2 );

	exitScreen->setupOutAnims();

	LoadScreen::changeRes( outFile );
}

void LoadScreen::changeRes( FitIniFile& outFile )
{

	if ( progressBackground )
		delete [] progressBackground;

	progressBackground = NULL;

	if ( waitingForPlayersMemory )
		delete [] waitingForPlayersMemory;

	waitingForPlayersMemory = NULL;

	if ( !progressBackground )
	{
		char progressPath[256];
		char progressBackgroundPath[256];



		long result = outFile.seekBlock( "LoadingBar" );
		gosASSERT( result == NO_ERR );

		outFile.readIdLong( "XLocation", xProgressLoc );
		outFile.readIdLong( "YLocation", yProgressLoc );
		outFile.readIdString( "FileName", progressPath, 255 );
		outFile.readIdString( "BackgroundFileName", progressBackgroundPath, 255);


		File tgaFile;
		FullPathFileName path;
		path.init( artPath, progressBackgroundPath, ".tga" );
		if ( NO_ERR != tgaFile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", (const char*)path );
			Assert( 0, 0, error );
			return;
		}

		long size = tgaFile.fileSize();
	
		progressBackground = (TGAFileHeader*)new char[size];
		tgaFile.read( (unsigned char*)progressBackground, tgaFile.fileSize() );

		tgaFile.close();

		path.init( artPath, progressPath, ".tga" );
		if ( NO_ERR != tgaFile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", (const char*)path );
			Assert( 0, 0, error );
			return;
		}

		size = tgaFile.fileSize();

		progressTextureMemory = (TGAFileHeader*)new char[size];
		tgaFile.read( (unsigned char*)progressTextureMemory, tgaFile.fileSize() );

		mergedTexture = (TGAFileHeader*)new char[size];
		memcpy( mergedTexture, progressTextureMemory, size );

		tgaFile.close();

		result = outFile.seekBlock( "WaitImage" );
		gosASSERT( result == NO_ERR );
		outFile.readIdString( "FileName", progressPath, 255 );
		outFile.readIdLong( "XLocation", xWaitLoc );
		outFile.readIdLong( "YLocation", yWaitLoc );

		path.init( artPath, progressPath, ".tga" );
		if ( NO_ERR != tgaFile.open( path ) )
		{
			char error[256];
			sprintf( error, "couldn't open file %s", (const char*)path );
			Assert( 0, 0, error );
			return;
		}

		size = tgaFile.fileSize();

		waitingForPlayersMemory = (TGAFileHeader*)new char[size];
		tgaFile.read( (unsigned char*)waitingForPlayersMemory, tgaFile.fileSize() );

		flipTopToBottom( (BYTE*)(waitingForPlayersMemory + 1), waitingForPlayersMemory->pixel_depth, 
			waitingForPlayersMemory->width, waitingForPlayersMemory->height );
	}
}

void LoadScreenWrapper::begin()
{
	waitForResChange = 0;
	bFirstTime = true;

	enterScreen->begin();
}

void LoadScreenWrapper::update()
{
	if ( loadProgress > 99 )
		soundSystem->playDigitalSample( LOAD_DOORS_OPENING );
	else if ( bFirstTime )
		soundSystem->playDigitalSample( LOAD_DOORS_CLOSING );

	 bFirstTime = 0;

	if ( waitForResChange ) // waiting one more render to force 
	{
		status = READYTOLOAD;
		waitForResChange = 0;
	}
	else
	{

	if ( Environment.screenWidth == 800 )
		{
			enterScreen->update();
			status = enterScreen->getStatus();
			if ( status == READYTOLOAD )
			{
				waitForResChange = 1;
				changeRes();
				status = RUNNING;
			}
		}
		else
		{
			exitScreen->update();
			status = exitScreen->getStatus();
		}
	}
}

void LoadScreenWrapper::render( int xOffset, int yOffset )
{
	if ( Environment.screenWidth == 800 )
	{
		enterScreen->render( xOffset, yOffset );
	}
	else
		exitScreen->render( xOffset, yOffset );
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
LoadScreen::LoadScreen(  )
{
	progressBackground = 0;
	progressTextureMemory = 0;
	animIndices = 0;
}

//-------------------------------------------------------------------------------------------------

LoadScreen::~LoadScreen()
{
	if ( progressBackground )
		delete [] progressBackground;

	if ( progressTextureMemory )
		delete [] progressTextureMemory;

	if ( mergedTexture )
		delete [] mergedTexture;

	if ( animIndices )
		delete [] animIndices;

	if ( waitingForPlayersMemory)
		delete [] waitingForPlayersMemory;

	animIndices = NULL;

	waitingForPlayersMemory = progressBackground = progressTextureMemory = mergedTexture = NULL;
}

void LoadScreen::begin()
{
	for ( int i = 0; i < animObjectsCount; i++ )
	{
		//The assign below will overwrite the assign in defaultConstructor, leaking the memory
		animObjects[i].animInfo.destroy();

		animObjects[i].animInfo = inAnims[animIndices[i]];
		animObjects[i].animInfo.begin();
		animObjects[i].update();
	}

	loadProgress = 0;

	//-----------------------------------------------
	//Turn the mouse cursor OFF until load is done.
	// This will be keep ghost images from occuring.
	userInput->mouseOff();	
}
void LoadScreen::init(FitIniFile& file, DWORD neverFlush)
{
	LogisticsScreen::init( file, "Static", "Text", "Rect", "Button", "Edit", "AnimObject", neverFlush );

	file.seekBlock( "AnimationTopOut" );
	outAnims[0].init( &file, "" );

	file.seekBlock( "AnimationBottomOut" );
	outAnims[1].init( &file, "" );

	file.seekBlock( "AnimationLeftOut" );
	outAnims[2].init( &file, "" );

	file.seekBlock( "AnimationRightOut" );
	outAnims[3].init( &file, "" );

	file.seekBlock( "AnimationTopIn" );
	inAnims[0].init( &file, "" );

	file.seekBlock( "AnimationBottomIn" );
	inAnims[1].init( &file, "" );

	file.seekBlock( "AnimationLeftIn" );
	inAnims[2].init( &file, "" );

	file.seekBlock( "AnimationRightIn" );
	inAnims[3].init( &file, "" );

	file.seekBlock( "AnimationTop2In" );
	inAnims[4].init( &file, "" );

	file.seekBlock( "AnimationTop2Out" );
	outAnims[4].init( &file, "" );


	text.init( &file, "AnimObject18" );
	

	if ( animObjectsCount )
	{
		char blockName[256];
		animIndices = new int[animObjectsCount];
		for ( int i= 0; i < animObjectsCount; i++ )
		{
			sprintf( blockName, "AnimObject%ld", i );
			file.seekBlock( blockName );
			file.readIdString( "AnimationOut", blockName, 255 );
			if ( strstr( blockName, "2" ) )
				animIndices[i] = 4;
			else if ( strstr( blockName, "Top" ) )
				animIndices[i] = 0;
			else if ( strstr( blockName, "Bottom" ) )
				animIndices[i] = 1;
			else if ( strstr( blockName, "Left" ) )
				animIndices[i] = 2;
			else
				animIndices[i] = 3;
		}
	}

}

void LoadScreen::update()
{
	status = RUNNING;
	LogisticsScreen::update();

	bool bDone = true;
	for ( int i = 0; i < animObjectsCount; i++ )
	{
		bDone &= animObjects[i].isDone();
	}

	if ( bDone )
	{
		if ( loadProgress < 99.f )
		{

			for ( int i = 0; i < animObjectsCount; i++ )
			{
				//The assign below will overwrite the assign in begin, leaking the memory
				animObjects[i].animInfo.destroy();

				animObjects[i].animInfo = outAnims[animIndices[i]];
				animObjects[i].begin();
				animObjects[i].update();
			}

			status = READYTOLOAD;
			prefs.applyPrefs( true );

			turnOffAsyncMouse = mc2UseAsyncMouse;
			if ( !mc2UseAsyncMouse )
				MouseTimerInit();
			mc2UseAsyncMouse = true;			
			AsynFunc = ProgressTimer;
		}
		else
		{
			loadProgress = 0;

			status = NEXT;

			//YIKES!!  We could be checking the if before the null and executing after!!  Block the thread!
			//Wait for thread to finish.
			while (mc2IsInMouseTimer)
				;

			//ONLY set the mouse BLT data at the end of each update.  NO MORE FLICKERING THEN!!!
			// BLOCK THREAD WHILE THIS IS HAPPENING
			mc2IsInDisplayBackBuffer = true;

			AsynFunc = NULL;
			mc2UseAsyncMouse = turnOffAsyncMouse;
			if ( !mc2UseAsyncMouse)
				MouseTimerKill();

			mc2IsInDisplayBackBuffer = false;

			//Force mouse Cursors to smaller or larger depending on new video mode.
			userInput->initMouseCursors("cursors");
			userInput->mouseOn();
		}
	}
	else
	{
		userInput->mouseOff();
	}
}

void LoadScreen::render( int x, int y )
{
	//ignoring animation information...
	LogisticsScreen::render();

	int curX = animObjects[0].animInfo.getXDelta();
	int curY = animObjects[0].animInfo.getYDelta();
	text.move( x + curX, y + curY );
	text.render( );
	text.move( -x - curX, -y - curY);
}

void ProgressTimer(	RECT& WinRect,DDSURFACEDESC2& mouseSurfaceDesc )
{

	if ( !LoadScreen::progressBackground )
		return;

	long destX = 0;
	long destY = 0;

	BYTE* pMem = (BYTE*)(LoadScreen::mergedTexture + 1);
	long destRight = 0;
	long destBottom = 0;

	long srcWidth = 0;
	long srcDepth = 0;


	if ( loadProgress > 0 && loadProgress < 100 )
	{
		destX = 0;
		destY = 0;

		long destWidth = LoadScreen::progressBackground->width;
		long destHeight = LoadScreen::progressBackground->height;

		float widthIncPerProgress = (float)destWidth * 0.01f;

		long* pLSrc =  (long*)(LoadScreen::progressBackground+1);
		long* pLDest = (long*)(LoadScreen::mergedTexture+1);

		// merge background and current progress together...
		for ( int i = 0; i < 2; i++ )
		{
			for ( int y = 0; y < destHeight; y++ )
			{

				for ( long x = 0; x < LoadScreen::progressBackground->width; x++ )
				{
					if ( x < destWidth )
						*pLDest++ = *pLSrc++;
					else
					{
						pLDest++;
						pLSrc++;
					}
				}
			}
			pLSrc = (long*)( LoadScreen::progressTextureMemory + 1 );
			pLDest = (long*)(LoadScreen::mergedTexture+1);

			destWidth = loadProgress * widthIncPerProgress;
		}

		destX = WinRect.left + (LoadScreen::xProgressLoc);
		destY = WinRect.top + (LoadScreen::yProgressLoc);

		pMem = (BYTE*)(LoadScreen::mergedTexture + 1);
		destRight = destX + LoadScreen::progressBackground->width;
		destBottom = (destY + LoadScreen::progressBackground->height);

		srcWidth = LoadScreen::progressBackground->width;
		srcDepth = LoadScreen::progressBackground->pixel_depth/8;
	}
	else if ( loadProgress == 1000 )
	{
		destX = WinRect.left + LoadScreen::xWaitLoc;
		destY = WinRect.top + LoadScreen::yWaitLoc;

		pMem = (BYTE*)(LoadScreen::waitingForPlayersMemory + 1);
		destRight = destX + LoadScreen::waitingForPlayersMemory->width;
		destBottom = (destY + LoadScreen::waitingForPlayersMemory->height);
		destRight = destRight > WinRect.right ? WinRect.right : destRight;
		destBottom = destBottom > WinRect.bottom ? WinRect.top : destBottom;


		srcWidth = LoadScreen::waitingForPlayersMemory->width;
		srcDepth = LoadScreen::waitingForPlayersMemory->pixel_depth/8;

	}
	else
    {
		return;
    }

#ifndef LINUX_BUILD
    // now put it on the screen...

    long destWidth = destRight - destX;
    long destHeight = destBottom - destY;


    for ( int y = 0; y < destHeight; y++ )
    {
        BYTE* pSrc = pMem + y * srcWidth * srcDepth;
        BYTE* pDest = (MemoryPtr)mouseSurfaceDesc.lpSurface + destX * mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8 + 										
            ((destY + y) * mouseSurfaceDesc.lPitch);

        for ( long x = 0; x < destWidth; x++ )
        {

            DWORD mColor = *(long*)pSrc;
            BYTE baseAlpha 		= 0;
            BYTE baseColorRed	= (mColor & 0x00ff0000)>>16;
            BYTE baseColorGreen	= (mColor & 0x0000ff00)>>8;
            BYTE baseColorBlue 	= (mColor & 0x000000ff);
            pSrc += 4;

            if ( mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32 )
            {
                (*(long*)pDest) = mColor;
                pDest += 4;
            }
            else if ( mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 24 )
            {

                if ( !baseAlpha )
                {
                    pDest++;
                    pDest++;
                    pDest++; 
                }

                *pDest++ = baseColorRed;
                *pDest++ = baseColorGreen;
                *pDest++ = baseColorBlue;

            }
            else if ( mouseSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16 )
            {
                bool in555Mode = false;
                if (GetNumberOfBits(mouseSurfaceDesc.ddpfPixelFormat.dwGBitMask) == 5)
                    in555Mode = true;

                if ( !baseAlpha )
                {
                    long clr;
                    if (in555Mode)
                    {
                        clr = (baseColorRed >> 3) << 10;
                        clr += (baseColorGreen >> 3) << 5;
                        clr += (baseColorBlue >> 3);
                    }
                    else
                    {
                        clr = (baseColorRed >> 3) << 11;
                        clr += (baseColorGreen >> 2) << 5;
                        clr += (baseColorBlue >> 3);
                    }

                    *pDest++ = clr & 0xff;
                    *pDest++ = clr >> 8;
                }
                else
                {
                    pDest++;
                    pDest++;
                }
            }


        }
    }
#endif
}


void LoadScreen::setupOutAnims()
{
	for ( int i = 0; i < animObjectsCount; i++ )
	{
		//The assign below will overwrite the assign in begin, leaking the memory
		animObjects[i].animInfo.destroy();

		animObjects[i].animInfo = outAnims[animIndices[i]];
		animObjects[i].begin();
		animObjects[i].update();
	}
}

//*************************************************************************************************
// end of file ( LoadScreen.cpp ) 
