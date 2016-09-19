//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef ABUTTON_H
#define ABUTTON_H

#include"asystem.h"
#include"afont.h"
#include"aanim.h"


class FitIniFile;


class aButton : public aObject
{
public:
	aButton();
	long	init(long xPos, long yPos, long w, long h);
	void	destroy();
	virtual void	update();
	virtual void	render();

	void	init( FitIniFile& file, const char* headerName, HGOSFONT3D font = 0 );
	
	aButton& operator=( const aButton& src);
	aButton( const aButton& src);

	virtual void move( float offsetX, float offsetY );

	void	setHoldTime( float newTime ) { holdTime = newTime; }

	virtual bool		pointInside(long xPos, long yPos) const;
	

	
	void press( bool );
	void toggle();
	
	void disable( bool );
	bool isEnabled();
	bool isPressed() { return state == PRESSED; }
	void makeAmbiguous( bool bAmbiguous );
	void setSinglePress(){ singlePress = true; }
	void setMessageOnRelease() { messageOnRelease = true; }
	
	void hide( bool );

	int getID();
	void setID( int ID );
	void setText( int newID ) { data.textID = newID; }

	void setPressFX( int newFX ){ clickSFX = newFX; }
	void setHighlightFX( int newFX ){ highlightSFX = newFX; }
	void setDisabledFX( int newFX ){ disabledSFX = newFX; }

	enum States
	{
		ENABLED = 0,
		PRESSED,
		DISABLED,
		AMBIGUOUS,
		HIGHLIGHT,
		HIDDEN
	};

	

	
protected:

	struct	aButtonData
	{
		long			ID;
		long			textID;
		long			textColors[5];
		HGOSFONT3D		textFont;
		long			textSize; // for ttf

		char			fileName[32];
		long			stateCoords[5][2];	
		long			textureWidth;
		long			textureHeight;
		int				fileWidth;
		int				fileHeight;
		bool			textureRotated;
		GUI_RECT		textRect; // center text in here
		bool			outlineText; // draw empty square around text rect
		bool			outline;		// draw empty square around button
		long			textAlign;
	};

	aButtonData		data;
	int				state;
	bool			toggleButton;
	bool			singlePress;
	bool			messageOnRelease;
	float			holdTime;		

	long			clickSFX;
	long			highlightSFX;
	long			disabledSFX;
	

	static void makeUVs( gos_VERTEX* vertices, int State, aButtonData& data );

	private:
		void	copyData( const aButton& src );

	
};

class aAnimButton : public aButton
{
public:

	aAnimButton();
	void	init( FitIniFile& file, const char* headerName, HGOSFONT3D font = 0 );
	virtual void update();
	virtual void render();

	void destroy();

	aAnimButton& operator=( const aAnimButton& src);
	aAnimButton( const aAnimButton& src );

	void setAnimationInfo( aAnimation* normal, aAnimation* highlight,
								  aAnimation* pressed, aAnimation* disabled );


	void animateChildren( bool bAnimate ){ bAnimateChildren = bAnimate; }

private:

	aAnimation	normalData;
	aAnimation	highlightData;
	aAnimation	pressedData;
	aAnimation	disabledData;

	void update( const aAnimation& data );

	bool		animateText;
	bool		animateBmp;

	bool		bAnimateChildren;

	void copyData( const aAnimButton& src );



};

#endif