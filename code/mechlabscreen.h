#ifndef MECHLABSCREEN_H
#define MECHLABSCREEN_H
/*************************************************************************************************\
MechLabScreen.h			: Interface for the MechLabScreen component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef ATTRIBUTEMETER_H
#include"attributemeter.h"
#endif

#ifndef COMPONENTLISTBOX_H
#include"componentlistbox.h"
#endif

#ifndef SIMPLECAMERA_H
#include"simplecamera.h"
#endif

#define MECH_LAB_ATTRIBUTE_METER_COUNT	9
class FitIniFile;
class LogisticsVariant;
class LogisticsComponent;
class LogisticsVariantDialog;
class LogisticsAcceptVariantDialog;
//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MechLabScreen:
**************************************************************************************************/
class MechLabScreen: public LogisticsScreen
{
	public:

		MechLabScreen();
		virtual ~MechLabScreen();

		int init( FitIniFile& );

		virtual void begin();
		virtual void end();
		virtual void update();
		virtual void render(int xOffset, int yOffset);

		virtual int	handleMessage( unsigned long, unsigned long );

		void	setComponent( LogisticsComponent* pComponent, bool bMessageFromLB = 0 );
		int		addComponent( LogisticsComponent* pComponent, long& x, long& y );
		void	beginDrag( LogisticsComponent* pComponent );
		void	endDrag( );

		int		canAddComponent( LogisticsComponent* pComponent );

		static MechLabScreen* instance() { return s_instance;}

		bool canRemoveComponent( LogisticsComponent* pComp );


	private:

		MechLabScreen( const MechLabScreen& src );
		MechLabScreen& operator=( const MechLabScreen& echLabScreen );
		static MechLabScreen*	s_instance;
		
		static LogisticsVariantDialog* saveDlg;
		static LogisticsAcceptVariantDialog* acceptDlg;

		LogisticsVariantDialog*			pCurDialog;

		LogisticsVariant*	pVariant;
		LogisticsComponent*	pSelectedComponent;
		long				selI;
		long				selJ;

		ComponentIconListBox	componentListBox;
		aComboBox				variantList;

		AttributeMeter attributeMeters[MECH_LAB_ATTRIBUTE_METER_COUNT];

		aObject				componentIcons[128];
		long				componentCount;

		aObject				payloadIcon;

		aObject				dragIcon;

		LogisticsComponent*	pDragComponent;
		LogisticsComponent*	pCurComponent;

		void showJumpJetItems( bool bShow );

		void updateDiagram();

		float originalCost;

		aObject				selRects[2][5];
		aObject				selJumpJetRect;
		aObject*			selRect;
		
		bool				bSaveDlg;
		bool				bDragLeft ;

		SimpleCamera		camera;

		EString				varName;

		static				GUI_RECT	sensorRects[4];
		static				long		sensorHelpIDs[4];

		float				countDownTime;
		float				curCount;
		float				previousAmount;
		float				oldCBillsAmount;

		float				oldHeat;
		float				heatTime;
		bool				flashHeatRange;

		float				oldArmor;
		float				newArmor;
		float				armorTime;

		bool				bErrorDlg;


		// HELPER FUNCTIONS
		void getMouseDiagramCoords( long& x, long& y );
		void getMouseDiagramCoords( long screenX, long screenY, long& x, long& y );

		void	diagramToScreen( long i, long j, long& x, long& y  );
		int		selectFirstDiagramComponent();
		int		selectFirstLBComponent(); 
		void	updateDiagramInput();
		void	swapVariant();
		void	updateHeatMeter();
		void	updateArmorMeter();
		void	removeComponent( long i, long j );



};


//*************************************************************************************************
#endif  // end of file ( MechLabScreen.h )
