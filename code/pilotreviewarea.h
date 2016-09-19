#ifndef PILOTREVIEWAREA_H
#define PILOTREVIEWAREA_H
/*************************************************************************************************\
PilotReviewARea.h			: Interface for the PilotReviewARea component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************
#include"logisticsscreen.h"
#include"alistbox.h"
#include"attributemeter.h"
#include"aanim.h"
#include"logisticspilot.h"

class aButton;
class PilotIcon;
class PilotPromotionArea;

/**************************************************************************************************
CLASS DESCRIPTION
PilotReviewARea:
**************************************************************************************************/


// holds the dead and active pilots
class PilotListBox : public aListBox
{
public:

	PilotListBox();
	virtual long AddItem( aListItem* item );
	virtual void update();

	bool isDone( ){ return bDone; }

private:
 
	float		timeSinceStart;
	int			curItem;
	long		oldScroll;
	long		newScroll;
	float		scrollTime;
	bool		bDone;
};

// this screen shows up after the salvage screen
class PilotReviewScreen : public LogisticsScreen
{
public:
	
	PilotReviewScreen();
	virtual ~PilotReviewScreen();
	
	void init(FitIniFile* file);
	bool isDone();
	virtual void render();
	virtual void update();
	virtual int			handleMessage( unsigned long, unsigned long );

	void				updatePilots(); // put into inventory, save??

	bool				bDone;

	static PilotPromotionArea*	s_curPromotion;
	static PilotReviewScreen*	instance;

private:

	PilotListBox			pilotListBox;
	aAnimation				entryAnim;
	aAnimation				exitAnim;
};


// base class for dead, and active pilot list box items
class PilotListItem : public aListItem
{
public:

	PilotListItem()
	{
		currentTime = -1.f;
		bDone = 0;
	}

	virtual void begin();	// for animation purposes...
	virtual bool isDone() = 0; // implement this yourself
	virtual void update();

protected:
	float	currentTime;
	bool	bDone;

};


// list box item for dead pilots
class DeadPilotListItem : public PilotListItem
{
public:
	virtual void		render();
	virtual void		update();
	virtual bool		isDone();

	virtual ~DeadPilotListItem();

	static void			init( FitIniFile* file );

	LogisticsPilot*		getPilot(){ return pPilot; }

	DeadPilotListItem( LogisticsPilot* pUnit );

private:
	friend class PilotReviewScreen;

	LogisticsPilot*		pPilot;
	
	static aText*		s_nameText;
	static aText*		s_rankText;
	static aText*		s_missionText;
	static aText*		s_killsText;
	static aRect*		s_area;
	static aRect*		s_liveIconRect;
	static aRect*		s_deadIconRect;
	static long			s_itemCount;
	
	aText				nameText;
	aText				rankText;
	aText				missionText;
	aText				killsText;

	PilotIcon*			liveIcon;
	PilotIcon*			deadIcon;


	
};

// pilots that weren't killed
class ActivePilotListItem : public PilotListItem
{
public:

	virtual void		render();
	virtual void		update();
	virtual bool		isDone();

	virtual ~ActivePilotListItem();

	static void			init( FitIniFile* file );
	ActivePilotListItem( LogisticsPilot* pUnit );
	static long			s_totalWidth;

	float				flashTime();


protected:

	AttributeMeter		attributeMeters[2];
	static AttributeMeter*		s_attributeMeters[2];

	static aObject*		s_icons[8];
	static aText*		s_nameText;
	static aText*		s_missionText;
	static aText*		s_killsText;
	static long			s_itemCount;
	static aRect*		s_outline[5];

	static aText*		s_rankText;
	static aText*		s_gunneryText;
	static aText*		s_pilotingText;
	static aRect*		s_area;
	static aRect*		s_iconRect;
	static aRect*		s_killIconRect;
	static PilotPromotionArea* s_pilotPromotionArea;
	static aObject*		s_medals[MAX_MEDAL];
	static aText*		s_promotionText;
	static aText*		s_medalText;
	static aText*		s_medalAwardedText;
	static aAnimation*		s_skillAnim;
	static aAnimation*		s_medalAwardedAnim;
	static aAnimation*		s_pilotPromotedAnim;
	
	aObject		icons[4];
	aText		nameText;
	aText		missionText;
	aText		killsText;
	long			itemCount;

	aText		rankText;
	aText		gunneryText;
	aText		pilotingText;
	aRect		area;

	aObject*	medalIcons[MAX_MEDAL];
	aText*	medalTexts[MAX_MEDAL];
	aText		promotionText;
	aText		medalAwardedText;

	
	PilotIcon*	pilotIcon;
	LogisticsPilot* pilot;

	bool		showingPromotion;
	bool		promotionShown;

	long		medalCount;

};

class PilotPromotionArea : public LogisticsScreen
{
public:

	bool isDone();
	void init( FitIniFile& file );
	virtual void render();
	virtual void update();
	void setPilot(LogisticsPilot* pPilot, PilotIcon* pIcon);
	virtual int			handleMessage( unsigned long, unsigned long );
	virtual ~PilotPromotionArea();

	aListBox*	getSkillListBox() { return &skillListBox; }

private:

	AttributeMeter	attributeMeters[2];

	LogisticsScreen	areaLeft;
	LogisticsScreen	areaRight;
	LogisticsPilot* pilot;
	aAnimation		leftInfo;
	aAnimation		rightInfo;
	aAnimation		leftExitInfo;
	aAnimation		rightExitInfo;
	bool			bDone;

	aListBox		skillListBox;
	PilotIcon*		pilotIcon;
	aAnimation		selSkillAnim;
	long			lastCheck;

};



// specialty skills each show up in here
class SpecialtyListItem : public aListItem
{
public:
	virtual void		render();
	virtual void		update();

	virtual ~SpecialtyListItem();

	static void			init( FitIniFile* file );
	virtual int			handleMessage( unsigned long message, unsigned long who );

	virtual bool		isChecked();
	virtual void		setCheck( bool );
	int					getID();
	SpecialtyListItem( int ID );

private:
	static aButton*	s_radioButton;
	static aObject*		s_skillIcons[4];
	static aAnimation*	s_highlightAnim;
	static aAnimation*	s_normalAnim;
	static aAnimation*	s_pressedAnim;
	static aText*		s_description;
	static aRect*		s_area;
	static aRect*		s_outline;
	static long			s_itemCount;

	static void			deleteStatics();

	aAnimation			pressedAnim;
	aAnimation			highlightAnim;
	aAnimation			normalAnim;
	aObject				icon;
	aText				description;
	aButton			radioButton;
	aRect				outline;

	int					ID;

	friend class PilotPromotionArea;

	
};




//*************************************************************************************************
#endif  // end of file ( PilotReview.h )
