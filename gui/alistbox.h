//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef ALISTBOX_H
#define ALISTBOX_H

#define MAX_LIST_ITEMS			128
#define	TOO_MANY_ITEMS			0xEEEE0001
#define ITEM_OUT_OF_RANGE		0xEEEE0002

#ifndef ASCROLL_H
#include"ascroll.h"
#endif

#ifndef AEDIT_H
#include"aedit.h"
#endif

#ifndef AFONT_H
#include"afont.h"
#endif

#ifndef ESTRING_H
#include"estring.h"
#endif

class aListItem : public aObject
{
public:

	enum State
	{
		ENABLED = 0,
		SELECTED,
		HIGHLITE,
		PRESSED_HIGHLITE,
		DISABLED,

	};

	virtual void update(){ aObject::update(); }
	virtual void render(){ aObject::render(); }

	void setState( int newState ){ state = (State)newState; }
	State getState( ) { return state; }

	void select(){ state = SELECTED; }
	void disable(){ state = DISABLED; }
	void highlite(){ state = HIGHLITE; }
	void deselect(){ state = state == DISABLED ? DISABLED : ENABLED; }

	virtual bool isChecked() { return false; }
	virtual void setCheck( bool ){}

	

protected:
	
	State		state;
};

class aTextListItem  : public aListItem
 {
public:

	aTextListItem(HGOSFONT3D newFont); // need the font, so we can determine the size
	aTextListItem( const aFont& newFont );
	aTextListItem( long fontResID );

	virtual ~aTextListItem();

	void		setText( const char* text );
	void		setText( long resID );
	const char* getText() const;
	void		sizeToText();

	void init( FitIniFile& file, const char* blockName = "Text0" );

	virtual void render();
	void		setAlignment( long newA ){ alignment = newA; }

	void		forceToTop( bool bForce ) { bForceToTop = bForce; }

protected:
	aTextListItem() {}
	void init( long fontResID );


	aFont		font;
	EString		text;
	long		alignment;

	bool		bForceToTop;


 };

class aAnimTextListItem : public aTextListItem
{

public:

	aAnimTextListItem(HGOSFONT3D newFont) : aTextListItem( newFont ) {}
	aAnimTextListItem( const aFont& newFont ) : aTextListItem( newFont ){}
	aAnimTextListItem( long fontResID ) : aTextListItem( fontResID ){}

	aAnimTextListItem( const aAnimTextListItem& src ); 
	aAnimTextListItem& operator=( const aAnimTextListItem& src ); 

	void init( FitIniFile& file, const char* blockName = "Text0" );
	virtual void render();
	virtual void update();

protected:

	aAnimGroup	animInfo;
	void CopyData( const aAnimTextListItem& src );

};

class aLocalizedListItem : public aAnimTextListItem
{
public:
	aLocalizedListItem();
	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void		render();

	void			setHiddenText( const char* pText ){ hiddenText = pText; }
	const char*		getHiddenText( ) const { return hiddenText;}

protected:

	EString				hiddenText;

};




class aListBox : public aObject
{
public:

	aListBox();

	virtual long		init(long xPos, long yPos, long w, long h);
	void				init( FitIniFile* file, const char* blockName );

	virtual void		destroy();
	virtual void		render();
	virtual void		update();
	virtual int			handleMessage( unsigned long message, unsigned long who );
	virtual void	resize(long w, long h);

	virtual long		AddItem(aListItem* itemString);
	virtual long		InsertItem(aListItem* itemString, long where);
	virtual long		RemoveItem( aListItem* itemString, bool bDelete );
	long 				ChangeItemString(short itemNumber, char* newString);
	long				GetSelectedItem(void) {return itemSelected;};
	long				GetCheckedItem() const;
	long				SelectItem(long itemNumber);
	bool				IsScrollActive(void) {return scrollActive;};
	long				ActivateScrollbar(void);


	aListItem*			GetItem(long itemNumber);
	long				GetItemCount(){ return itemCount; }

	void				removeAllItems( bool bDelete );

	void				setSpaceBetweenItems( long newSpace ){ skipAmount = newSpace; }
	long				getSpaceBetweenItems(){ return skipAmount; }

	void				setSingleCheck(bool checkOnlyOne){ singleCheck = checkOnlyOne; }

	virtual void		move( float offsetX, float offsetY );
	void				setScrollPos( int pos );

	long				getScrollBarWidth();

	void				setOrange( bool bOrange );
	void				enableAllItems();

	void setPressFX( int newFX ){ clickSFX = newFX; }
	void setHighlightFX( int newFX ){ highlightSFX = newFX; }
	void setDisabledFX( int newFX ){ disabledSFX = newFX; }

	void setTopSkip( long newSkip ){ topSkip = newSkip; }

	bool				pointInScrollBar( long mouseX, long mouseY );
	float				getScrollPos() {return  scrollBar ?  scrollBar->GetScrollPos() : 0; }

protected:
	long		itemCount;
	long		itemSelected;
	long		skipAmount;

	aListItem*	items[MAX_LIST_ITEMS];
	mcScrollBar*	scrollBar;
	long		itemHeight;
	bool		scrollActive;
	bool		singleCheck;

	int			clickSFX;
	int			highlightSFX;
	int			disabledSFX;
	int			topSkip;


	void		scroll( int amount );
};


/* Note that items in an aDropList items do not have to be aTextListItems, whereas in an
aComboBox they do (well, selectable items do).*/
class aDropList : public aObject
{
public:

	aDropList();

	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void	destroy();
	void specialDestroy();
	virtual void		render();
	virtual void		update();
	virtual int			handleMessage( unsigned long message, void* who );
	virtual bool		pointInside(long xPos, long yPos) const;
	//virtual void	resize(long w, long h);

	virtual long		AddItem(aListItem* itemString);
	long				SelectItem(long item);

	aListBox &ListBox() { return listBox; }
	bool IsExpanded() { return listBox.isShowing(); }
	void IsExpanded(bool isExpanded);
	void disable( bool bDisable ){ 
		if ( bDisable )
			IsExpanded( 0 );
		expandButton.showGUIWindow( !bDisable ); 
		rects[0].showGUIWindow( !bDisable );  rects[1].showGUIWindow( !bDisable );
	}

	long				AddItem( unsigned long textID, unsigned long color );
	long				AddItem( const char* text, unsigned long color );
	long				GetSelectedItem() const { return selectionIndex; }

	aDropList& operator=( const aDropList& );


protected:
	aRect*			rects;
	long			rectCount;
	aAnimButton		expandButton;
	aListBox		listBox;
	float			listBoxMaxHeight;
	int				selectionIndex;
	aAnimTextListItem	templateItem;
	aDropList( const aDropList& );

	long			textLeft;
	long			textTop;

};


class aComboBox : public aObject
{
public:

	aComboBox();
	~aComboBox() {};

	virtual long	init( FitIniFile* file, const char* blockName );
	virtual void	destroy();
	virtual void		render();
	virtual void		update();
	virtual int			handleMessage( unsigned long message, const void* who );
	virtual bool		pointInside(long xPos, long yPos) const;
	//virtual void	resize(long w, long h);

	virtual long		AddItem(aListItem* itemString);
	long				SelectItem(long item);

	void				setReadOnly( bool bReadOnly ){ entry.setReadOnly( bReadOnly ); }
	long				AddItem( unsigned long textID, unsigned long color );
	long				AddItem( const char* text, unsigned long color );
	long				GetSelectedItem() const { return selectionIndex; }


	aComboBox& operator=( const aComboBox& );

	aEdit &EditBox() { return entry; }
	aListBox &ListBox() { return listBox; }

	void				setFocus( bool bFocus ) { EditBox().setFocus( bFocus ); }

protected:
	aEdit		entry;
	aRect*		rects;
	long		rectCount;
	aAnimButton		expandButton;
	aListBox		listBox;
	float		listBoxMaxHeight;
	int				selectionIndex;


	aComboBox( const aComboBox& );
	aAnimTextListItem	templateItem;
};



#endif
