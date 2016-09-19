//-----------------------------------------------------------------------------------------
//
// Floating Help class.  Used for tool tips, building/mech/vehicle IDs.
// Basically draws itself AFTER everything else since gos_Font"3D" has no Z Depth!
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//--------------------------
// Include Files
#ifndef FLOATHELP_H
#include"floathelp.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

//-----------------------------------------------------------------------------------------
DWORD		FloatHelp::currentFloatHelp = 0;		//How many of them are we using at present
FloatHelp*	FloatHelp::floatHelps = NULL;			//POinters to all of them.

//-----------------------------------------------------------------------------------------
// class FloatHelp
void FloatHelp::init (long maxHelps)
{
	floatHelps = (FloatHelp *)systemHeap->Malloc(sizeof(FloatHelp) * MAX_FLOAT_HELPS);
	gosASSERT(floatHelps != NULL);
	
	FloatHelp *tmp = floatHelps;
	for (long i=0;i<MAX_FLOAT_HELPS;i++,tmp++)
	{
		tmp->text[0] = 0;
		tmp->screenPos.x = tmp->screenPos.y = tmp->screenPos.z = tmp->screenPos.w = 0.0f;
		tmp->foregroundColor = SD_WHITE;
		tmp->backgroundColor = SD_BLACK;
		tmp->scale = 1.0f;
		tmp->proportional = true;
		tmp->bold = tmp->italic = tmp->wordWrap = false;
	}
	
	currentFloatHelp = 0;
}
		
//-----------------------------------------------------------------------------------------
void FloatHelp::destroy (void)
{
	systemHeap->Free(floatHelps);
	floatHelps = NULL;
	currentFloatHelp = 0;
}

//-----------------------------------------------------------------------------------------
void FloatHelp::resetAll (void)
{
	currentFloatHelp = 0;
	
	for (long i=0;i<MAX_FLOAT_HELPS;i++)
		floatHelps[i].reset();
}

//-----------------------------------------------------------------------------------------
void FloatHelp::renderAll (void)
{
	for (long i=0;i<MAX_FLOAT_HELPS;i++)
		floatHelps[i].render();
}

//-----------------------------------------------------------------------------------------
void FloatHelp::setFloatHelp(char * txt, 
								Stuff::Vector4D screenPos, 
								DWORD fClr, 
								DWORD bClr, 
								float scl,
								bool proportional,
								bool bold,
								bool italic, 
								bool wordWrap)
{
	if (currentFloatHelp < MAX_FLOAT_HELPS)
	{
		floatHelps[currentFloatHelp].setHelpText(txt);
		floatHelps[currentFloatHelp].setScreenPos(screenPos);
		floatHelps[currentFloatHelp].setForegroundColor(fClr);
		floatHelps[currentFloatHelp].setBackgroundColor(bClr);
		floatHelps[currentFloatHelp].setScale(scl);
		floatHelps[currentFloatHelp].setProportional(proportional);
		floatHelps[currentFloatHelp].setBold(bold);
		floatHelps[currentFloatHelp].setItalic(italic);
		floatHelps[currentFloatHelp].setWordWrap(wordWrap);
		
		currentFloatHelp++;
	}
}

//-----------------------------------------------------------------------------------------
void FloatHelp::getTextStringLength (char * txt,                  
										DWORD fColor,
										float scl,                
										bool proportional,        
										bool bold,                
										bool italic,              
										bool wordWrap,
										DWORD &width, DWORD &height)
{
	// must use globalFloat Scale because of true type fonts
	gos_TextSetAttributes (gosFontHandle, fColor, gosFontScale, wordWrap, proportional, bold, italic);

    // sebi
	//unsigned long gHeight = 0, gWidth = 0;
	unsigned int gHeight = 0, gWidth = 0;
	if (txt[0])
	{
		gos_TextStringLength(&gWidth,&gHeight,txt);
	}

	width = gWidth;
	height = gHeight;
}

//-----------------------------------------------------------------------------------------

