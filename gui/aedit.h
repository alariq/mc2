#ifndef AEDIT_H
#define AEDIT_H
/*************************************************************************************************\
aEdit.h			: Interface for the aEdit component of the GUI library.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef ASYSTEM_H
#include"asystem.h"
#endif

#ifndef ESTRING_H
#include"estring.h"
#endif

//*************************************************************************************************
//Entry field styles
#define ES_EDITALPHA			0x00020000  //Entry field alpha only
#define ES_EDITNUM				0x00040000  //Entry field numeric only
#define ES_EDITALPHANUM   0x00080000  //Entry field alpha or numeric
#define ES_EDITUPPER			0x00100000  //Entry field convert to uppercase
#define ES_EDITREADONLY   0x00200000  //Read only
#define ES_EDITNOBLANK		0x00400000	//Ignore ' '

/**************************************************************************************************
CLASS DESCRIPTION
aEdit:
**************************************************************************************************/
class aEdit: public aObject
{
	public:

		aEdit( long fontID );
		virtual ~aEdit();
		aEdit(); // really need a font for this to work
		aEdit& operator=( const aEdit& );

		virtual void update();
		virtual void render();

		void	renderWithDropShadow();

	
		void getEntry(EString& str);
		void setEntry(const EString& str, BYTE byHighlight = 0);
		void limitEntry(int nNewLimit) { nLimit = nNewLimit; }
		void setFocus(bool bHasFocus);
		void setFont( long fontID );
		void setTextColor( long color ){ textColor = color; }
		void setSelectedColor( long color ){ selectedColor = color; }
		bool hasFocus()const { return bFocus; }


		void init( FitIniFile* file, const char* header );
		long getHighlightColor( ) const { return highlightColor; }
		long getColor() const { return textColor; }
		long getCursorColor() const { return cursorColor; }
		long getSelectedColor() const { return selectedColor; }

		void allowWierdChars( bool bAllow ){ bWierdChars = bAllow; }

		aFont* getFontObject() { return &font; }

		void setReadOnly( bool bReadOnly )
		{ 
			if ( bReadOnly )
				dwStyleFlags |= ES_EDITREADONLY; 
			else 
				dwStyleFlags ^= ES_EDITREADONLY;
		};

		void setNoBlank( bool bNoBlank )
		{
			if ( bNoBlank )
				dwStyleFlags |= ES_EDITNOBLANK; 
			else 
				dwStyleFlags ^= ES_EDITNOBLANK;
		}

		void setNumeric( bool bNoAlpha )
		{
			if ( bNoAlpha )
				dwStyleFlags |= ES_EDITNUM; 
			else 
				dwStyleFlags ^= ES_EDITNUM;
		}

		int getFont() { return font.getFontID(); }

		void initBufferSize( unsigned long newSize );
		void setBufferSize( int newSize ) { text.SetBufferSize( newSize ); }

		void	allowIME( bool bAllow ){ bAllowIME = bAllow; }


	private:

		aEdit( const aEdit& );

		// HELPER FUNCTIONS
		bool	clearSelection();
		void	backSpace(int nPosition);
		void	drawCursor();
		void	hideCursor();
		bool	handleFormattingKeys(int key);
		int		charXPos(int nOffset);
		void	makeCursorVisible();
		void	flushCursorRight();
		int		findChar(int nXPos);

		void	handleMouse();
		void	handleKeyboard();

		int charLength( int index );
		

		long cursorColor;
		long highlightColor; // backdrop
		long textColor;
		long selectedColor; // selected text
		long outlineColor;
		int		nLimit;
		int		nInsertion1,nInsertion2; // beginning and ending highlight positions (when equal there is no highlight)
		bool	bCursorVisible;
		int		nLeftOffset;
		float	cursorTime;
		bool	bMouseDown;
		EString	text;
		aFont	font;
		bool	bFocus;
		unsigned long dwStyleFlags;
		bool	bAllowIME;
		bool	bWierdChars;

		bool bIMEInitialized;

};


//*************************************************************************************************
#endif  // end of file ( aEdit.h )
