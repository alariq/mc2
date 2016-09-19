//-----------------------------------------------------------------------------------------
//
// Floating Help class.  Used for tool tips, building/mech/vehicle IDs.
// Basically draws itself AFTER everything else since gos_Font"3D" has no Z Depth!
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef FLOATHELP_H
#define FLOATHELP_H
//--------------------------
// Include Files
#ifndef DBASEGUI_H
#include"dbasegui.h"
#endif

#include<gameos.hpp>
#include<stuff/stuff.hpp>

extern HGOSFONT3D gosFontHandle;
extern float	gosFontScale;

#define MAX_FLOAT_HELPS	100				//Used for sensor information, too!
//-----------------------------------------------------------------------------------------
// Classes
class FloatHelp
{
	protected:
		char 				text[2048];				//Last person to set this displays the font.
		Stuff::Vector4D 	screenPos;				//x,y are left and top.  z,w are width and height.
		DWORD 				foregroundColor;		//Color in aRGB Format.
		DWORD 				backgroundColor;		//Color in aRGB Format.
		float 				scale;					//Scale.  1.0f is normal.
		bool 				proportional;			//if false, spacing is equal for each letter.
		bool 				bold;					//if true, draws bold.
		bool 				italic;					//if true, draws italic.
		bool 				wordWrap;				//if true, wraps word.

		static	DWORD		currentFloatHelp;		//How many of them are we using at present
		static	FloatHelp*	floatHelps;				//POinters to all of them.

	public:

		FloatHelp (long maxHelps)
		{
			init(maxHelps);
			text[0] = 0;
			screenPos.x = screenPos.y = screenPos.z = screenPos.w = 0.0f;
			foregroundColor = SD_WHITE;
			backgroundColor = SD_BLACK;
			scale = 1.0f;
			proportional = true;
			bold = italic = wordWrap = false;
		}

		~FloatHelp (void)
		{
			destroy();
		}

		void init (long maxHelps);
		
		void destroy (void);
		
		static void resetAll (void);

		static void renderAll (void);

		static void setFloatHelp(char * txt, 
								Stuff::Vector4D screenPos, 
								DWORD fClr, 
								DWORD bClr, 
								float scl,
								bool proportional,
								bool bold,
								bool italic, 
								bool wordWrap);

		static void getTextStringLength (char *text,
										DWORD fColor,
										float scl,
										bool proportional,
										bool bold,        
										bool italic,      
										bool wordWrap,
										DWORD &width, DWORD &height);
										
	protected:
		
		void setHelpText (char *txt)
		{
			if (strlen(txt) < 2048)
			{
				strcpy(text,txt);
			}
			else
			{
				text[0] = 0;
			}
		}

		void setScreenPos (Stuff::Vector4D pos)
		{
			screenPos = pos;
		}
		
		void setForegroundColor (DWORD clr)
		{
			foregroundColor = clr;
		}

		void setBackgroundColor (DWORD clr)
		{
			backgroundColor = clr;
		}
		
		void setScale (float scl)
		{
			scale = scl;
		}

		void setProportional (bool flag)
		{
			proportional = flag;
		}

		void setBold (bool flag)
		{
			bold = flag;
		}

		void setItalic (bool flag)
		{
			italic = flag;
		}
		
		void setWordWrap (bool flag)
		{
			wordWrap = flag;
		}

		void reset (void)
		{
			//Call every frame.  Otherwise floating help keeps floating!
			text[0] = 0;
		}

		void render (void)
		{
			if (text[0])
			{
				// must use global scale, incase of True Type fonts.
				gos_TextSetAttributes (gosFontHandle, foregroundColor, gosFontScale, wordWrap, proportional, bold, italic);

				//gos_TextDrawBackground ((int)screenPos.x, (int)screenPos.y, (int)(screenPos.x+screenPos.z), (int)(screenPos.y+screenPos.w), SD_BLACK);

				gos_TextSetPosition((int)screenPos.x,(int)screenPos.y);

				gos_TextDraw(text);
			}
		}
};

typedef FloatHelp* FloatHelpPtr;

//-----------------------------------------------------------------------------------------
#endif
