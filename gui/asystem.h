//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef ASYSTEM_H
#define ASYSTEM_H

class aSystem;
class aCallback;
class aObject;
class aTitleWindow;
class InterfaceObject;

#include<gameos.hpp>

#include"estring.h"
#include"afont.h"

#define MAX_CHILDREN 64

#define aMSG_LEFTMOUSEDOWN              1
#define aMSG_MIDDLEMOUSEDOWN			2
#define aMSG_RIGHTMOUSEDOWN             3
#define aMSG_LEFTMOUSEUP                4
#define aMSG_MIDDLEMOUSEUP              5
#define aMSG_RIGHTMOUSEUP               6
#define aMSG_MOUSEMOVE                  7
#define aMSG_LEFTMOUSEDBLCLICK			16
#define aMSG_RIGHTMOUSEDBLCLICK			17
#define aMSG_LEFTMOUSEHELD				18
#define aMSG_RIGHTMOUSEHELD				19
#define aMSG_SCROLLUP		101
#define aMSG_SCROLLDOWN		102
#define aMSG_PAGEUP			103
#define aMSG_PAGEDOWN		104
#define aMSG_TRACKTAB		106
#define aMSG_SCROLLTO		107	//	sent by parent
#define aMSG_SCROLLPARENT	108	//	sent to parent
#define aMSG_SELECTME		109
#define aMSG_DONE			110
#define aMSG_BUTTONCLICKED			111
#define aMSG_SELCHANGED		112

typedef enum
{
	UNDEFINEDWINDOW = -1,
	GENERIC = 1,
	STATIC,
	SCROLLBAR,
	BUTTON,
	LISTBOX,
	COMBOBOX,
	NUMBER_OF_WINDOWTYPES
} WINDOW_ID;



// Error codes, local to this file...
#define	DUPLICATE_INSTANCE			-1
#define	FAILED_TO_CREATE_WINDOW		-2
#define	DISPLAY_MISMATCH			-3
#define INIT_FAILED					-4
#define	FAILED_TO_CREATE_SYSTEM		-5
#define	LOCK_FAILURE				-6
#define	UNLOCK_FAILURE				-7
#define	BLT_FAILURE					-8
#define FAILED_TO_ALLOCATE_PORT		-9
#define	USER_INIT_FAILED			-10

#include"utilities.h"

extern long helpTextID;
extern long helpTextHeaderID;



class aBaseObject
{
public:
	
	virtual void render(){}
	virtual void update(){}
};
// base class aObject definition
class aObject : public aBaseObject
{

public:
	aObject();
	virtual ~aObject();
	aObject( const aObject& src );
	aObject& operator=( const aObject& src );
	
	virtual long init(long xPos, long yPos, long w, long h);
	virtual void destroy();

	float		width() const;
	float		height() const;
	float		x() const;
	float		y() const ;
	
	virtual void	moveTo(long xPos, long yPos);
	virtual void	moveToNoRecurse(long xPos, long yPos);
	
	void		resize(long w, long h);
	void		addChild(aObject* c);
	void		removeChild(aObject* c);
	void		setParent(aObject* p);

	void		setTexture( const char* fileName );
	void		setTexture(unsigned long newHandle );
	void		setUVs( float u1, float v1, float u2, float v2 );
	void		setColor(uint32_t color, bool bRecurse = 0); // color the vertices

	void		init( FitIniFile* file, const char* block, DWORD neverFlush = 0 ); // for statics
	
	aObject*			getParent()
	{
		return pParent;			// No Need for this to be virtual!!!!!  Can now check if object has been deleted without crashing!
	}
	
	long				numberOfChildren() const;
	long				globalX() const;
	long				globalY() const;
	long				globalLeft() const { return globalX(); }
	long				globalTop() const { return globalY(); }
	long				globalRight() const;
	long				globalBottom() const;
	
	virtual aObject*	findObject(long xPos, long yPos);
	virtual int			handleMessage( unsigned long, unsigned long ){ return 0; }
	virtual bool		pointInside(long xPos, long yPos) const;
	bool				rectIntersect(long top, long left, long bottom, long right) const;
	bool				rectIntersect(const GUI_RECT& testRect) const;
	
	aObject*	children();
	aObject*	child(long w);

	
	virtual void		render();
	virtual void		render(long x, long y);
	virtual void		update();

	long getColor(){ return location[0].argb; }
		

	void		showGUIWindow(bool show) {showWindow = show;}
	bool		isShowing(void) const {return showWindow;}

	void				FillBox(short left, short top, short bottom, short right, char color);
	void				SetBit(long xpos, long ypos, char value);
	void				removeAllChildren( bool bDelete = 0);
	virtual void		move( float offsetX, float offsetY );
	virtual void		moveNoRecurse( float offsetX, float offsetY );

	void				setFileWidth( float newWidth ){ fileWidth = newWidth; }
	int				getID() const { return ID; }
	void			setID(int newID) { ID = newID; }

	void			setHelpID( int newID ) { helpID = newID; }
	int				getHelpID() const { return helpID; }
	

	float		left()
	{
		return x();
	}
	float		top()
	{
		return y();
	}
	float		right()
	{
		return x() + width();
	}
	
	float		bottom()
	{
		return y() + height();
	}


	
protected:

	gos_VERTEX		location[4];

	unsigned long	textureHandle;
	float		fileWidth;
	bool		showWindow;

	
	aObject*	pChildren[MAX_CHILDREN];
	long		pNumberOfChildren;
	aObject*	pParent;
	
	int			ID;

	void copyData( const aObject& src );

	long			helpHeader;
	long			helpID;


};


//class aRect : public aBaseObject
/* It may seem wasteful to derive from aObject instead of aBaseObject, but an aRect 
needs to able to be a child of an aObject. Perhaps bounding box and parent/child support
should be part of aBaseObject. */
class aRect : public aObject
{
public:

	aRect();
	virtual ~aRect(){}

	virtual void render();
	virtual void		render(long x, long y);

	virtual void init( FitIniFile* file, const char* blockName );

	GUI_RECT getGUI_RECT();
	GUI_RECT getGlobalGUI_RECT();

	bool	bOutline;

};

class aText : public aObject
{
public:

	aText();
	aText( const aText& src );
	aText& operator=( const aText& src );

	virtual ~aText();

	virtual void render();
	virtual void		render(long x, long y);
	

	void	init( FitIniFile* file, const char* header );

	void	setText( const EString& text );
	void		setText( long resID );

	EString		text;
	long			alignment; // left, right, ala GOS
	aFont		font;

	virtual bool		pointInside(long xPos, long yPos) const;

private:

	void CopyData( const aText& src );



};



#endif // ASYSTEM_H
