#ifndef MPPREFS_H
#define MPPREFS_H
/*************************************************************************************************\
MPPrefs.h			: Interface for the MPPrefs component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef LOGISTICSSCREEN_H
#include"logisticsscreen.h"
#endif

#ifndef ALISTBOX_H
#include"alistbox.h"
#endif

#ifndef SIMPLECAMERA_H
#include"simplecamera.h"
#endif

class FitIniFile;
struct _MC2Player;

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
MPPrefs:
**************************************************************************************************/
class MPPrefs: public LogisticsScreen
{
	public:

		MPPrefs();
		virtual ~MPPrefs();
		
		static MPPrefs* instance(){ return s_instance; }
		int init( FitIniFile& file );
		virtual void update();
		virtual void render( int OffsetX, int OffsetY );
		virtual int	handleMessage( unsigned long message, unsigned long who );
		virtual void begin();
		virtual void end();
		void			initColors();

		void	saveSettings();
		void	cancelSettings();

		void setMechColors( unsigned long base, unsigned long highlight ); // called by MPlayer when it resets a color

	private:
		MPPrefs( const MPPrefs& src );
		MPPrefs& operator=( const MPPrefs& PPrefs );

		aComboBox		comboBox[3];
		SimpleCamera	camera;
		aObject			insigniaBmp; // the one inside the combo box...

		// HELPERS
		void	updateStripeColors(const _MC2Player* players, long playerCount, bool bDrawRect );
		void	updateBaseColors( const _MC2Player* players, long playerCount, bool bDrawRect);
		char	getColorIndex( unsigned long color );
		void	setColor( unsigned long color );
		void	setHighlightColor( unsigned long color );

		static MPPrefs* s_instance;


};

class aBmpListItem : public aListItem
{
public:

	int setBmp( const char* pFileName );
	const char* getBmp(){ return fileName; }
	
private:

	aObject		bmp;

	EString		fileName;
};


//*************************************************************************************************
#endif  // end of file ( MPPrefs.h )
