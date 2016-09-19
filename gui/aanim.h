//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef AANIMATION_H
#define AANIMATION_H

class aAnimation;
class FitIniFile;

class aAnimation
{

public:
	aAnimation();
	~aAnimation();
	aAnimation( const aAnimation& );

	

	aAnimation& operator=( const aAnimation& src );

	long	init(FitIniFile* file, const char* prependName);
	long	initWithBlockName( FitIniFile* file, const char* blockName );
	void	destroy();

	void	begin();
	void	reverseBegin();
	void	end();

	void	update();

	bool	isAnimating() const { return currentTime != -1.f; }
	bool	isDone() const;

	float	getXDelta() const;
	float	getYDelta() const;

	float	getScaleX() const;
	float	getScaleY() const;

	unsigned long getColor() const;
	unsigned long getColor( float time ) const;

	void setReferencePoints( float X, float Y );

	float	getDirection(){ return direction; }
	float	getCurrentTime() { return currentTime; }
	float	getMaxTime();

	


protected:
	
	float currentTime;

	struct MoveInfo
	{
		float time; // in seconds
		float positionX; // x pixels
		float positionY; // pixels
		float scaleX;
		float scaleY;
		long color;
	};

	MoveInfo*	infos;
	long			infoCount;

	float		refX;
	float		refY;

	float		direction;

	bool		bLoops;

	private:

	void copyData( const aAnimation&  );



};

// animations for the states we use everywhere
class aAnimGroup
{
public:

	enum STATE
	{
		NORMAL = 0,
		PRESSED = 1,
		HIGHLIGHT = 2,
		PRESSEDHIGHLIGHT = 3,
		DISABLED = 4,
		MAX_ANIMATION_STATE
	};

	aAnimGroup(){curState = NORMAL; }

	long init( FitIniFile* file, const char* blockName );

	void setState( STATE );
	STATE getState() const;

	long getCurrentColor( STATE ) const;
	long getXDelta( STATE ) const;
	long getYDelta( STATE ) const;
	float getCurrnetScale( STATE ) const;

	void update();




private:


	aAnimation		animations[MAX_ANIMATION_STATE];

	STATE			curState;
};


#endif

