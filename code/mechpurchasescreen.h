#ifndef MECHPURCHASESCREEN_H
#define MECHPURCHASESCREEN_H
/*************************************************************************************************\
MechPurchaseScreen.h			: Interface for the MechPurchaseScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#include"mechlistbox.h"
#include"logisticsmech.h"
#include"elist.h"
#include"logisticsmechdisplay.h"

class LogisticsMech;

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MechPurchaseScreen:
**************************************************************************************************/
class MechPurchaseScreen: public LogisticsScreen
{
	public:

		static MechPurchaseScreen* instance() { return s_instance; }

		MechPurchaseScreen();
		virtual				~MechPurchaseScreen();

		int					init( FitIniFile& file );

		virtual void		begin();
		virtual void		end();
		virtual void		update();
		virtual void		render(int xOffset, int yOffset);

		void				setMech( LogisticsMech* pMech, bool bFromLB = 0 );
		void				beginDrag( LogisticsMech* pMech );
		virtual int			handleMessage( unsigned long, unsigned long );

	private:
	
		MechPurchaseScreen( const MechPurchaseScreen& src );
		MechPurchaseScreen& operator=( const MechPurchaseScreen& echPurchaseScreen );

		MechListBox			inventoryListBox;
		MechListBox			variantListBox;
		LogisticsMechDisplay	mechDisplay;

		LogisticsMech*		pDragMech;
		aObject				dragIcon;

		bool				acceptPressed;


		static MechPurchaseScreen* s_instance;


		bool				dragStartLeft;

		
		typedef EList<LogisticsMech, LogisticsMech > MECH_LIST;	
		MECH_LIST		prevInventory;

		float				countDownTime;
		float				curCount;
		float				previousAmount;
		float				oldCBillsAmount;

		float				flashTime;


	// HELPER FUNCTIONS

		void addSelectedMech();
		void removeSelectedMech();
		void addMech( LogisticsMech* pMech );
		void removeMech( LogisticsMech* pMech );
		void endDrag();
		bool selectFirstBuyableMech();



};


//*************************************************************************************************
#endif  // end of file ( MechPurchaseScreen.h )
