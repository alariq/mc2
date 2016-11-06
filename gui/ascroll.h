//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef ASCROLL_H
#define ASCROLL_H


#include"asystem.h"
#include"abutton.h"

class mcScrollButton : public aAnimButton
{
public:
	virtual void render();

	unsigned long lightEdge;
	unsigned long darkEdge;
	unsigned long regularColor;
};

class aScrollBar : public aObject
{
public:

	aScrollBar();

	virtual long	init(long xPos, long yPos, long w, long h);
	virtual void	destroy();
	virtual void	update();
	virtual void	render();

	virtual int		handleMessage( unsigned long message, unsigned long fromWho );

	void			SetScrollMax(float newMax);
	void			SetScrollPos(float newPos);
	float			GetScrollMax(void){return scrollMax;};
	float			GetScrollPos(void){return scrollPos;};
    // sebi: not used and type in name!
	//long			SetSrollInc( long newInc ){ scrollInc = newInc; } // amount you move for one arrow click
	//long			SetScrollPage(long newInc){ pageInc = newInc;} // amount you move if you click on the bar itself
	void			ScrollUp(void);
	void			ScrollPageUp(void);
	void			ScrollDown(void);
	void			ScrollPageDown(void);
	void			SetScroll( long newScrollPos );	
	void			Enable( bool enable );

protected:

	float			scrollMax;
	float			scrollPos;
	aAnimButton			topButton;
	aAnimButton			bottomButton;
	mcScrollButton		scrollTab;

	long			lastY;
	long			scrollInc;
	long			pageInc;
	unsigned long   color;
	void			ResizeAreas(void);
};

class mcScrollBar : public aScrollBar
{
public:
	long init ( long xPos, long yPos, long w, long h  );
	virtual void	resize(long w, long h);

	void setOrange();
	void setGreen();

private:

	aAnimation		orangeInfo[4];
	aAnimation		greenInfo[4];
	
};



#endif
