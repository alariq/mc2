#ifndef MPLOADMAP_H
#define MPLOADMAP_H
/*************************************************************************************************\
MPLoadMap.h			: Interface for the MPLoadMap component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

//*************************************************************************************************

#ifndef LOGISTICSDIALOG_H
#include"logisticsdialog.h"
#endif

#include"asystem.h"
#include"alistbox.h"
#include"attributemeter.h"
#include"simplecamera.h"

#ifndef AANIM_H
#include"aanim.h"
#endif

class aButton;



class MPLoadMap : public LogisticsDialog
{
public:
	
	MPLoadMap();
	virtual ~MPLoadMap();
	
	void init(FitIniFile* file);
	bool isDone();
	virtual void		begin();
	virtual void		end();
 	virtual void		render( int xOffset, int yOffset );
	virtual void		render();
	virtual void		update();
	virtual int			handleMessage( unsigned long, unsigned long );

	void				beginSingleMission();

	const char* getMapFileName(){ return selMapName; }

	static void			getMapNameFromFile( const char* pFileName, char* pBuffer, long bufferLength );




private:
	int indexOfButtonWithID(int id);
	void seedDialog( bool bSeedSingle );
	void seedFromCampaign();



	aListBox				mapList;
	aLocalizedListItem	templateItem;

	EString					selMapName;

	bool					bIsSingle;



	void	updateMapInfo();
	void	seedFromFile( const char* pFileName );
	void	addFile( const char* pFileName, bool bSeedSingle );



};



//*************************************************************************************************
#endif  // end of file ( MPLoadMap.h )
