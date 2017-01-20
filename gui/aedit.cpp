#define AEDIT_CPP
/*************************************************************************************************\
aEdit.cpp			: Implementation of the aEdit component of the GUI library.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"aedit.h"
#include"userinput.h"
#include"ctype.h"
#include"inifile.h"
#include"mclib.h"
#include"platform_windows.h"
#include"platform_mbstring.h" // isleadbyte
//#include<winnls.h>
#include"soundsys.h"
#include "../resource.h"

#define ENTRY_MAX_CHARS 1000
#define ENTRY_MARGIN 4.f
#define HalfColorValue(color) (((color >> 1) & 0x007f0000) | ((color >> 1) & 0x00007f00) | ((color >> 1) & 0x0000007f))

static long acp = 0;
static bool g_bUseLangDll;
static gosIME_Appearance g_ia;

extern SoundSystem *sndSystem;


aEdit::aEdit(  )
{
	if (acp == 0) {
        //sebi !NB
		acp = 899;//GetACP();
        SPEW(("INPUT", "GetACP is not imlemented"));

		gos_SetIMELevel( 2 );
		gos_GetIMEAppearance( &g_ia );

		char szSetting[8];
		szSetting[0] = '1';
		cLoadString( IDS_IME_USE_LANGDLL_SETTING, szSetting, sizeof( szSetting ) );
		g_bUseLangDll = szSetting[0] == '1';

		if (!g_bUseLangDll) {
			// setup fixed colors
			g_ia.symbolColor = 0xff5c96c2;
			g_ia.symbolColorText = 0x7f000000;
			g_ia.symbolTranslucence = 0x7f;
			g_ia.symbolPlacement = 0;
			g_ia.candColorBase =  0xff43311c;
			g_ia.compColorTargetConv = 0xff5c96c2;
			g_ia.compColorTargetNotConv = 0xff002f55;
			g_ia.compColorInputErr = 0xffff00ff;
			g_ia.compTranslucence = 0x7f;

			if (acp != 949) {	// other than Korean version, reduce color value by half so that composition string can be seen
				g_ia.compColorTargetConv =		HalfColorValue(g_ia.compColorTargetConv);
				g_ia.compColorTargetNotConv =	HalfColorValue(g_ia.compColorTargetNotConv);
				g_ia.compColorInputErr =		HalfColorValue(g_ia.compColorInputErr);
			}
		}
	}
	nLimit = ENTRY_MAX_CHARS;
	nInsertion1 = nInsertion2 = 0;
	nLeftOffset = 0;
	bMouseDown=0;
	bCursorVisible=FALSE;
	bFocus = 0;
	dwStyleFlags = 0;
	cursorTime = 0.f;
	setColor( 0xff000000 );
	bAllowIME = true;
	bIMEInitialized = false;
	bWierdChars = true;

	//MarkForBackSurfaceUpdate();
}

aEdit& aEdit::operator =( const aEdit& src )
{

	aObject::operator=( src );
	nLimit = src.nLimit;
	nInsertion1 = nInsertion2 = 0;
	nLeftOffset = src.nLeftOffset;
	bMouseDown = 0;
	bCursorVisible = FALSE;
	dwStyleFlags = src.dwStyleFlags;

	text = src.text;
	textColor = src.textColor;
	highlightColor = src.highlightColor;

	
	cursorColor = src.cursorColor;
	selectedColor = src.selectedColor; // selected text
	outlineColor = src.outlineColor;

	font = src.font;

	bAllowIME = src.bAllowIME;

	return *this;
	
}

//-------------------------------------------------------------------------------------------------

aEdit::~aEdit()
{
	setFocus( 0 );
}

void aEdit::update()
{
	cursorTime += frameLength;
	if ( cursorTime > .5 )
	{
		bCursorVisible ^= 1;
		if ( !bFocus )
			bFocus = bCursorVisible = 0;
		cursorTime = 0.f;
	}
	handleMouse();
	handleKeyboard();
}

void aEdit::handleMouse()
{
	int mouseX = userInput->getMouseX();
	int mouseY = userInput->getMouseY();

	bool bLeftClick = userInput->isLeftClick();
	bool bRightClick = userInput->isRightClick();
	
	
	int nOffset = mouseX - (globalX() + ENTRY_MARGIN);
	nOffset += nLeftOffset;
	
	// find char position of nOffset and set insertion point there
	int nChar = findChar(nOffset);


	if ( bMouseDown && userInput->isLeftDrag() && bFocus )
	{
		nInsertion2 = nChar;
	}

	if ( pointInside( mouseX, mouseY ) )
	{
		if ( bLeftClick && !( dwStyleFlags & ES_EDITREADONLY ) )
		{
			setFocus( true );
			bMouseDown = true;
			bCursorVisible=TRUE;

			nInsertion1 = nChar;
			if (gos_GetKeyStatus(KEY_LSHIFT) == KEY_FREE)
				nInsertion2 = nInsertion1;
		}
		else if ( bRightClick )
		{
			

		}


	}
	else if ( bLeftClick )
		setFocus( false );
	


}

void aEdit::handleKeyboard()
{
	if ( !bFocus )
		return;

    //sebi: input works in a bit different way now
	//while( true ) // keep getting keys until buffer is empty
	{
		DWORD key = gos_GetKey();

		if ( !key )
			return;

		if (!handleFormattingKeys(key))
		{	
			// hack to eat control keys, must be a real way to do this
			if ( ((key & 0x00ff) + ((key & 0x00ff)<<8) + 0x4000) == key 
				&& key != 0x00006e2e )
				return;
	
			key &= 0x00ff;
			// this does happen when IME is enabled.
			if (key == 0)
				return;
			DWORD key2 = 0;

			if ( isleadbyte( key ) )
			{
				key2 = gos_GetKey( );	
				key2 &= 0x00ff;
			}


			if (dwStyleFlags & ES_EDITNUM)
			{
				if (!isdigit(key) && key != ' ' && key != '.')
					return;
			}
			else if (dwStyleFlags & ES_EDITALPHA)
			{
				if (!isalpha(key)	&& key != ' ')
					return;
			}
			else if (dwStyleFlags & ES_EDITALPHANUM)
			{
				if (!isalpha(key) && !isdigit(key) && key != ' ')
					return;
			}
			if((dwStyleFlags & ES_EDITNOBLANK) && (key==' '))
				return;

			// if there is a selection range, bolt the text within it	
			if (gos_GetKeyStatus(KEY_LMENU) != KEY_FREE )
			{
				return;
			}

			if ( key == '%'
				|| key == '\\'
				|| key == '/'
				|| key == '\"') // eat these so andy doesn't try and sprintf on us
				return; 
			if ( !bWierdChars )
			{
				if ( (key == '\\') ||
					(key == ':') ||
					(key == '*') ||
					(key == '\'') ||
					(key == '.') ||
					(key == ',') ||
					(key == '/') ||
					(key == '<') ||
					(key == '>') ||
					(key == '?') ||
					(key == '"') ||
					(key == '|')  )
					return;
			}

			// is there space to insert a character?
			if (text.Length() - labs( nInsertion2 - nInsertion1 ) + (key2 ? 1 : 0) >= nLimit)
			{
				sndSystem->playDigitalSample( LOG_WRONGBUTTON );
				gos_FinalizeStringIME();
				gos_KeyboardFlush();
				return;
			}

			// insert the character
			if ( key )
			{
				clearSelection();
				char str[3];
				str[0] = key;
				int amountToAdd = 1;
				if ( key2 )
				{
					str[1] = key2;
					str[2] = '\0';
					amountToAdd = 2;
				}
				else
					str[1] = '\0';
				
				if (nInsertion1 < text.Length() )
				{
					text.Insert( nInsertion1, str );
				}
				else
					text += str;

				nInsertion1 += amountToAdd;
				nInsertion2 = nInsertion1;
			}
		}


		
		
		makeCursorVisible();
	}

}

void	aEdit::renderWithDropShadow()
{
	// do not do this for .ttf where we already are having performance issues
	if ( font.getSize() == 1 ) 
	{
		long realTextColor = textColor;
		textColor = 0xff000000;
		move( -1, 1 );
		render();
		move( 1, -1 );
		textColor = realTextColor;
		render();
	}
	else
		render();
}

void aEdit::render()
{
	if ( !isShowing() )
		return;

	aObject::render();

	EString textToDraw = text;
	textToDraw += ' '; // have to do this for loc, some fonts are getting clipped in millenium

	// draw selection range
	if (nInsertion1 != nInsertion2) 
	{
		int nMin = nInsertion1<nInsertion2?nInsertion1:nInsertion2;
		int nMax = nInsertion1<nInsertion2?nInsertion2:nInsertion1;
		if ( nMax > text.Length() )
			nMax = text.Length();

		if ( nMax > 1 )
		{
			if (isleadbyte( nMax -2 ) )
				nMax -= 2;
		}

		int nXSelStart = (int)(globalX() + charXPos(nMin) - nLeftOffset + ENTRY_MARGIN);
		int nXSelEnd = (int)(globalX() + charXPos(nMax) - nLeftOffset + ENTRY_MARGIN);
		if (nXSelStart < globalX())
			nXSelStart = globalX();
		if (nXSelEnd > globalX() + width())
			nXSelEnd = globalX() + width();
	
		GUI_RECT rect = { (int)nXSelStart, 
						 (int)globalY(), 
						 (int)nXSelEnd,
						 (int)(globalY() + height()) };

		int startChar = 0;
		if ( nLeftOffset )
		{
			// need to find character where this position is
			startChar = findChar( nLeftOffset );
		}


		if ( text.Length() )
		{
			font.render( &textToDraw[startChar], 
				(int)(globalX() + ENTRY_MARGIN), 
				(int)globalY(), (int)width(), (int)height(), 
				textColor, 0, 0 );
		}
		drawRect( rect, highlightColor);

		if ( text.Length() )
		{
			char tmp = textToDraw[nMax];
			textToDraw[nMax] = '\0';
			font.render( &textToDraw[nMin], (int)nXSelStart, (int)globalY(),
				nXSelEnd, (int)height(), selectedColor, 0, 0 );
			textToDraw[nMax] = tmp;
		}
	}
	else
	{
		int startChar = 0;
		if ( nLeftOffset )
		{
			// need to find character where this position is
			startChar = findChar( nLeftOffset );
		}


		if ( textToDraw.Length() )
		{
			font.render( &textToDraw[startChar], 
				(int)(globalX() + ENTRY_MARGIN), 
				(int)globalY(), (int)width(), (int)height(), 
				textColor, 0, 0 );
		}
		else
		{
			font.render( "", 
				(int)(globalX() + ENTRY_MARGIN), 
				(int)globalY(), (int)width(), (int)height(), 
				textColor, 0, 0 );
		}
	}

	if ( bFocus && bAllowIME )
	{
		gos_TextSetRegion( globalX() + ENTRY_MARGIN, globalY(), globalX() + width(), globalY() + height() - 1 );
		long pos = charXPos( nInsertion1 );
		if ( acp == 949 && nInsertion1 > nInsertion2 && gos_GetMachineInformation(gos_Info_GetIMEStatus))
			pos = charXPos( nInsertion2 );
		gos_PositionIME( globalX() + pos - nLeftOffset + ENTRY_MARGIN, globalY() );
	}

	drawCursor();
}


void aEdit::getEntry(EString& str)
{
	str = text;
}
void aEdit::setEntry(const EString& str, BYTE byHighlight)
{
	text = str;

	if (byHighlight)
	{
		nInsertion2 = text.Length();
		nInsertion1 = 0;
	}
	else
	{
		nInsertion2 = nInsertion1 = text.Length();
		flushCursorRight();
	}

}
void aEdit::setFocus(bool bHasFocus)
{
	bool bFocusSave = bFocus;
	bFocus = bHasFocus;
	gos_KeyboardFlush();


	if ( bAllowIME && bHasFocus )
	{
		gos_EnableIME( true );
		bCursorVisible = bFocus;

		if (!g_bUseLangDll || g_ia.symbolPlacement == 0)
			g_ia.symbolHeight = height() - 2;

		if (!g_bUseLangDll) {
			// setup dynamic colors
			g_ia.candColorBorder = textColor;
			g_ia.candColorText = textColor;
			g_ia.compColorInput = textColor;
			g_ia.compColorConverted = textColor;
			g_ia.compColorText = textColor;

			if (acp != 949) {	// other than Korean version, reduce color value by half
				g_ia.compColorInput =			HalfColorValue(g_ia.compColorInput);
				g_ia.compColorConverted =		HalfColorValue(g_ia.compColorConverted);
				g_ia.compColorInputErr =		HalfColorValue(g_ia.compColorInputErr);
			}
		}
		gos_SetIMEAppearance( &g_ia );

		
		if ( !bIMEInitialized )
		{
			if ( acp == 932 || acp == 874 || acp == 936 || acp == 949 || acp == 950 )
			{
				// only want to do this if using IME
				resize( width() - height() - 2, height() ); 
			}

			bIMEInitialized = true;
		}
	}
	else {
		// bug #4862 - diable IME only when the edit box had the focus.
		if (bFocusSave)	
			gos_EnableIME( false );
	}
}


bool aEdit::clearSelection()
{
	if (nInsertion1 != nInsertion2)
	{ // delete selection range
		int nMin = nInsertion1<nInsertion2?nInsertion1:nInsertion2;
		int nMax = nInsertion1<nInsertion2?nInsertion2:nInsertion1;
		int len = 1;

		text.Remove( nMin, nMax-len );
		nInsertion1 = nMin;
		nInsertion2 = nInsertion1;
		flushCursorRight();
		return true;
	}

	return false;
}
void aEdit:: backSpace(int nPosition)
{
	int nCharCount = text.Length();
	if (nPosition>nCharCount)
		return;
	if (nPosition<=0)
	{
		nPosition = 0;
		return;
	}
	nCharCount = 1;

	if ( nPosition > 1 )
	{
		unsigned char* pPrev = _mbsdec( (const unsigned char*)(const char*)text, (const unsigned char*)(const char*)text + nPosition );		
		nCharCount = (const unsigned char*)(const char*)text + nPosition - pPrev;
	}

	text.Remove( nPosition-nCharCount, nPosition-1 );
	nInsertion2 = nInsertion1 = nPosition-nCharCount;

	
}
void aEdit:: drawCursor()
{


	if ( nInsertion1 != nInsertion2 || !bFocus || !gos_GetMachineInformation( gos_Info_GetIMECaretStatus ) )
		return;
	if (bCursorVisible && !( dwStyleFlags & ES_EDITREADONLY ))
	{
		gos_VERTEX v[2];
		memset( v, 0, sizeof( gos_VERTEX ) * 2 );
		v[0].rhw = v[1].rhw = .5;
		v[0].argb = v[1].argb = cursorColor;

		int nCursorX = charXPos(nInsertion1);
		nCursorX -= nLeftOffset;

		v[0].y  = globalY();
		v[1].y = globalY() + height();
		v[0].x = globalX() + nCursorX + ENTRY_MARGIN;
		v[1].x = v[0].x;
		
		gos_DrawLines( v, 2 );


	}

}
void aEdit:: hideCursor()
{
	bCursorVisible=FALSE;
}
bool aEdit::handleFormattingKeys(int keycode)
{
	int key = (keycode & 0xFF00)>>8;
	bool bExtendedKey = true;

	switch (key)
	{
		case KEY_LCONTROL:
			return true;
			break;
		case KEY_RETURN:
		{
			if ( gos_GetKeyStatus(KEY_RETURN) == KEY_PRESSED ) // don't want this called more than 1 time if held
				setFocus( false );
			return true;
		}
			break;

		case KEY_TAB:
			return true;
			break;

		case KEY_GRAVE:	// if there is a selection range, bolt the text within it	
			if (gos_GetKeyStatus(KEY_LMENU) != KEY_FREE && bAllowIME )
			{
				gos_ToggleIME( true );
				return true;
			}
			break;
		case KEY_HOME:
			if (gos_GetKeyStatus(KEY_LSHIFT) != KEY_FREE)
				nInsertion2 = 0;
			else
				nInsertion2 = nInsertion1=0;
			nLeftOffset = 0;
			bCursorVisible=TRUE;
			return true;
		case KEY_END:
			if (gos_GetKeyStatus(KEY_LSHIFT) != KEY_FREE)
				nInsertion2 = text.Length();
			else
				nInsertion2 = nInsertion1=text.Length();
			makeCursorVisible();
			bCursorVisible=TRUE;
			return true;
		case KEY_LEFT:
			if (gos_GetKeyStatus(KEY_LSHIFT) != KEY_FREE)
			{
				if (nInsertion2)
				{
					int decrementCount = 1;
					if ( nInsertion2 > 1  )
					{
						unsigned char* pPrev = _mbsdec( (const unsigned char*)(const char*)text, (const unsigned char*)(const char*)text + nInsertion2 );
						decrementCount = (const unsigned char*)(const char*)text + nInsertion2 - pPrev;
					}
					nInsertion2-= decrementCount;
				}
			}
			else
			{
				if (nInsertion1 != nInsertion2)
					nInsertion1 = nInsertion1<nInsertion2?nInsertion1:nInsertion2;
				else if (nInsertion1)
				{
					int decrementCount = 1;
					if ( nInsertion1 > 1 )
					{
						unsigned char* pPrev = _mbsdec( (const unsigned char*)(const char*)text, (const unsigned char*)(const char*)text + nInsertion2 );
						decrementCount = (const unsigned char*)(const char*)text + nInsertion2 - pPrev;	
					}
					nInsertion1 -= decrementCount;
				}
				nInsertion2 = nInsertion1;
			}

			makeCursorVisible();
			bCursorVisible=TRUE;
			return true;
		case KEY_RIGHT:
			if (gos_GetKeyStatus(KEY_LSHIFT) != KEY_FREE)
			{
				nInsertion2 += charLength(nInsertion2);
			}
			else
			{
				if (nInsertion1 != nInsertion2)
					nInsertion1 = nInsertion1>nInsertion2?nInsertion1:nInsertion2;
				else if (nInsertion1<text.Length())
					nInsertion1 += charLength( nInsertion1 );
				nInsertion2 = nInsertion1;
			}
			makeCursorVisible();
			bCursorVisible=TRUE;
			return true;
		case KEY_DELETE:
			if (bExtendedKey) // must check this, the period key will get mixed up with this 
			{
				if (!clearSelection())
					backSpace(nInsertion1+charLength(nInsertion1));
				makeCursorVisible();
				bCursorVisible=TRUE;
				return true;
			}
			break;
		case KEY_BACK:
			if (!clearSelection())
				backSpace(nInsertion1);
			flushCursorRight();
			makeCursorVisible();
			bCursorVisible=TRUE;
			return true;
			break;

		case KEY_UP:
		case KEY_DOWN:
			return true;
			break;


		case KEY_8:
			if ( !bWierdChars && userInput->getKeyDown( KEY_LSHIFT ) )
				return true;
			break;
//		case \*/?:<>|

	}
	return false;

}

int		aEdit::charXPos(int nCharIndex)
{
	if (nCharIndex > text.Length() || !text.Length())
		return 0;
	int nXPos = 0;
	if ( nCharIndex != text.Length() )
	{
		int oldChar = text[nCharIndex];
		text[nCharIndex] = 0;
		nXPos = font.width( text );
		text[nCharIndex] = oldChar;
	}
	else
	{
		nXPos = font.width( text );
	}
	return nXPos;
}
void	aEdit::makeCursorVisible()
{
	int nXPos = charXPos(nInsertion2);
	if (nXPos < nLeftOffset)
		nLeftOffset = nXPos;
	if (nXPos > nLeftOffset + width() - (ENTRY_MARGIN*2.f))
		nLeftOffset = nXPos - (int)(width() - (ENTRY_MARGIN*2.f));
}
void	aEdit::flushCursorRight()
{
	int nXPos = charXPos(nInsertion2);
	if (nXPos < width() - ENTRY_MARGIN*2)
		nLeftOffset = 0;
	else
		nLeftOffset = nXPos - (int)(width()- ENTRY_MARGIN*2.f);
}
int		aEdit::findChar(int nXPos)
{
	int nLastPos,nNextPos,n;
	nNextPos = nLastPos = 0;
	n=0;

	unsigned char* pBegin = (unsigned char*)&text[0];
	unsigned char* pCur = pBegin;
	unsigned char* pNext = pBegin;
	if ( text.Length() )
	{
		do
		{	
			pNext = _mbsinc( pCur );
			unsigned char tmp = *pNext;
			*pNext = '\0';
			nLastPos = nNextPos;
			nNextPos = font.width(text);
			*pNext = tmp;
			if ( nXPos > nNextPos )			
				pCur = pNext;
		}	while ( *pCur && nXPos > nNextPos);
	}

	if ( nXPos - nLastPos < nNextPos - nXPos )
		n = pCur - pBegin;
	else
		n = pNext - pBegin;

	if (n>text.Length())
		n=text.Length();

	return n;
}

void aEdit::init( FitIniFile* file, const char* header )
{
	int result = file->seekBlock( header );
	
	if ( result != NO_ERR )
	{
		char errorStr[256];
		sprintf( errorStr, "couldn't find the text block%s", header );
		Assert( result == NO_ERR, 0, errorStr );
		return;
	}

	long left, top, width, height; 

	file->readIdLong( "XLocation", left );
	file->readIdLong( "YLocation", top );
	file->readIdLong( "Width", width );
	file->readIdLong( "Height", height );

	aObject::init( left, top, width, height );
	
	file->readIdLong( "Color", textColor );

	long lfont;
	file->readIdLong( "Font", lfont );
	font.init( lfont );

	file->readIdLong( "CursorColor", cursorColor );
	file->readIdLong( "HighlightColor", highlightColor );
	file->readIdLong( "SelectedColor", selectedColor );
	file->readIdLong( "HelpCaption", helpHeader );
	file->readIdLong( "HelpDesc", helpID );


}

int aEdit::charLength( int index )
{
	if ( index < 0 )
		return 0;
	if ( index > text.Length() )
		return 0;

	char tmp = text[index];
	if ( isleadbyte( tmp ) )
		return 2;

	return 1;
}

void aEdit::setFont( long resID )
{
	font.init( resID );
}


//*************************************************************************************************
// end of file ( aEdit.cpp )
