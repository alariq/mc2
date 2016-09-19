//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"aanim.h"
#include"mclib.h"
#include"estring.h"

extern float frameRate;

aAnimation::aAnimation()
{
	currentTime = -1.f;
	infoCount = 0;
	infos = NULL;
	bLoops = 0;
	refX = 0;
	refY = 0;
	direction = 1.0;
}

aAnimation::~aAnimation()
{
	destroy();
}

aAnimation& aAnimation::operator=( const aAnimation& src )
{
	copyData( src );
	return *this;
}

aAnimation::aAnimation( const aAnimation& src )
{
	copyData( src );
}

void aAnimation::copyData( const aAnimation& src )
{
	if ( &src != this )
	{
		currentTime = src.currentTime;
		infoCount = src.infoCount;
		infos = NULL;
		bLoops = src.bLoops;
		refX = src.refX;
		refY = src.refY;

		//Maybe someone already inited this?
		if (infos)
		{
			delete [] infos;
			infos = NULL;
		}

		if ( src.infoCount )
		{
			infos = new MoveInfo[infoCount];
			for ( int i = 0; i < infoCount; i++ )
			{
				infos[i] = src.infos[i];
			}
		}
	}
}


long	aAnimation::init(FitIniFile* file, const char* headerName)
{
	EString strToCheck = headerName;
	strToCheck += "AnimationTimeStamps";

	if ( NO_ERR != file->readIdLong( strToCheck, infoCount ) )
	{
		EString strToCheck = headerName;
		strToCheck += "NumTimeStamps";
		file->readIdLong( strToCheck, infoCount );
	}

	strToCheck = headerName;
	strToCheck += "AnimationLoops";
	file->readIdBoolean( strToCheck, bLoops ); 

	//Maybe someone already inited this?
	if (infos)
	{
		delete [] infos;
		infos = NULL;
	}

	if ( infoCount )
	{
		infos = new MoveInfo[infoCount];
		for ( int i = 0; i < infoCount; i++ )
		{
			strToCheck.Format( "%s%s%ld", headerName, "Time", i );
			file->readIdFloat( strToCheck, infos[i].time );

			strToCheck.Format( "%s%s%ld", headerName, "Color", i );
			if ( NO_ERR != file->readIdLong( strToCheck, infos[i].color ) )
				infos[i].color = 0xffffffff;

			strToCheck.Format( "%s%s%ld%s", headerName, "Pos", i, "X" );
			long tmp;
			file->readIdLong( strToCheck, tmp );
			infos[i].positionX = tmp;

			strToCheck.Format( "%s%s%ld%s", headerName, "Pos", i, "Y" );
			file->readIdLong( strToCheck, tmp );
			infos[i].positionY = tmp;

			strToCheck.Format( "%s%s%ld", headerName, "Scale", i );
			if ( NO_ERR != file->readIdFloat( strToCheck, infos[i].scaleX ) )
			{
				// look for scaleX and scale Y
				strToCheck.Format( "%s%s%ld", headerName, "ScaleX", i );
				if ( NO_ERR != file->readIdFloat( strToCheck, infos[i].scaleX ) )
				{
					infos[i].scaleX = infos[i].scaleY = 1.0f;
				}
				else
				{
					strToCheck.Format( "%s%s%ld", headerName, "ScaleY", i );
					if ( NO_ERR != file->readIdFloat( strToCheck, infos[i].scaleY ) )
					{
						infos[i].scaleY = 1.0f;
					}
				}


			}
			else
				infos[i].scaleY = infos[i].scaleX;

		}
	}
	
	return 0;
}
	
void	aAnimation::destroy()
{
	if ( infos )
		delete [] infos;

	infos = NULL;
	infoCount = 0;
	currentTime = -1.f;
}

void	aAnimation::begin()
{
	currentTime = 0;
	direction = 1.0;
}

void	aAnimation::reverseBegin()
{
	if ( infos && infoCount )
		currentTime = infos[infoCount-1].time;
	else
	currentTime = 0.0;
	direction = -1.0;
}
	
void	aAnimation::end()
{
	currentTime = -1.f;
}

void	aAnimation::update()
{
	if ( currentTime != -1.f )
		currentTime += direction * frameLength;

	if ( direction < 0 && bLoops && infoCount && infos )
	{
		if ( currentTime < 0 )
			currentTime += infos[infoCount-1].time;
	}

	else if ( infos && currentTime > infos[infoCount-1].time )
	{
		if ( bLoops )
			currentTime -= infos[infoCount-1].time;
	}
}

float	aAnimation::getXDelta() const
{
	float t0, t1, p0, p1;

	t1 = p1 = t0 = p0 = 0.f;
	float delta = 0.f;
	
	// figure out where we are in animation
	for ( int j = 0; j < infoCount - 1; j++ )
	{
		if ( infos[j].time <= currentTime && infos[j+1].time > currentTime )
		{
			t0 = infos[j].time;
			t1 = infos[j + 1].time;
			p0 = (infos[j].positionX);
			p1 = (infos[j + 1].positionX);
			break;
		}
	}
	
	// if not done yet
	if ( t1 )
	{

		float dT = currentTime - t0;
		float currentPosition = p0 + dT * ( (p1 - p0)/(t1 -t0) );
		delta = currentPosition - refX;
	}
	else if ( infos )
	{
		delta = infos[infoCount - 1].positionX - refX;
	}

	return delta;

}

float	aAnimation::getYDelta() const
{
	float t0, t1, p0, p1;

	t1 = p1 = t0 = p0 = 0.f;
	float delta = 0.f;
	
	// figure out where we are in animation
	for ( int j = 0; j < infoCount - 1; j++ )
	{
		if ( infos[j].time <= currentTime && infos[j+1].time > currentTime )
		{
			t0 = infos[j].time;
			t1 = infos[j + 1].time;
			p0 = (infos[j].positionY);
			p1 = (infos[j + 1].positionY);
			break;
		}
	}
	
	// if not done yet
	if ( t1 )
	{

		float dT = currentTime - t0;
		float currentPosition = p0 + dT * ( (p1 - p0)/(t1 -t0) );
		delta = currentPosition - refY;
	}
	else if ( infos )
	{
		delta = infos[infoCount - 1].positionY - refY ;
	}

	return delta;

}

float	aAnimation::getScaleX() const
{
	float t0, t1, p0, p1;

	t1 = p1 = t0 = p0 = 0.f;

	float curScale = 1.0;
	
	// figure out where we are in animation
	for ( int j = 0; j < infoCount - 1; j++ )
	{
		if ( infos[j].time <= currentTime && infos[j+1].time > currentTime )
		{
			t0 = infos[j].time;
			t1 = infos[j + 1].time;
			p0 = (infos[j].scaleX);
			p1 = (infos[j + 1].scaleX);
			break;
		}
	}
	
	// if not done yet
	if ( t1 )
	{
		float dT = currentTime - t0;
		curScale = p0 + dT * ( (p1 - p0)/(t1 -t0) );
	}
	else if ( infos )
	{
		curScale = infos[infoCount - 1].scaleX ;
	}

	return curScale;

}

float	aAnimation::getScaleY() const
{
	float t0, t1, p0, p1;

	t1 = p1 = t0 = p0 = 0.f;

	float curScale = 1.0;
	
	// figure out where we are in animation
	for ( int j = 0; j < infoCount - 1; j++ )
	{
		if ( infos[j].time <= currentTime && infos[j+1].time > currentTime )
		{
			t0 = infos[j].time;
			t1 = infos[j + 1].time;
			p0 = (infos[j].scaleY);
			p1 = (infos[j + 1].scaleY);
			break;
		}
	}
	
	// if not done yet
	if ( t1 )
	{
		float dT = currentTime - t0;
		curScale = p0 + dT * ( (p1 - p0)/(t1 -t0) );
	}
	else if ( infos )
	{
		curScale = infos[infoCount - 1].scaleY ;
	}

	return curScale;

}


unsigned long aAnimation::getColor(  ) const
{
	return getColor( currentTime );
}

unsigned long aAnimation::getColor( float time ) const
{
	float t0, t1;

	if ( infoCount && (time > infos[infoCount-1].time) && bLoops )
	{

		float numCycles = infos[infoCount-1].time ?  time/infos[infoCount-1].time : 0;
		int numCyc = (long)numCycles;
		time -= ((float)numCyc * infos[infoCount-1].time);
	}

	t1 = t0 = 0.f;
	unsigned long color = 0xffffffff;
	unsigned long color1, color2;
	color1 = color2 = 0;
	
	// figure out where we are in animation
	for ( int j = 0; j < infoCount - 1; j++ )
	{
		if ( infos[j].time <= time && infos[j+1].time > time )
		{
			t0 = infos[j].time;
			t1 = infos[j + 1].time;
			color1 = (infos[j].color);
			color2 = (infos[j + 1].color);
			break;
		}
	}
	
	// if not done yet
	if ( t1 )
	{

		float totalTime = t1 - t0;
		float tmpTime= time - t0;

		float percent = tmpTime/totalTime;

		return interpolateColor( color1, color2, percent );
		
	
	}
	else if ( infos )
	{
		return infos[infoCount-1].color;
	}

	return color;

}

void aAnimation::setReferencePoints( float X, float Y )
{
	refX = X;
	refY = Y;
}

bool aAnimation::isDone() const
{
	if ( bLoops )
		return 0;
	
	if ( direction < 0 )
		return currentTime < 0 ? 1 : 0;

	else if ( infos && currentTime > infos[infoCount-1].time )
	{
		return 1;
	}

	return 0;
}

long	aAnimation::initWithBlockName( FitIniFile* file, const char* blockName )
{
	if ( NO_ERR != file->seekBlock( blockName ) )
	{
		char errorStr[255];
		sprintf( errorStr, "couldn't find block %s in file %s", blockName, file->getFilename() );
		Assert( 0, 0, errorStr );
		return -1;
	}

	return init( file, "" );
}

float	aAnimation::getMaxTime()
{
	if ( infos && infoCount)
	{
		return infos[infoCount-1].time;
	}

	return -1;
}

long aAnimGroup::init( FitIniFile* file, const char* blockName )
{
	if ( NO_ERR != file->seekBlock( blockName ) )
	{
		char errorStr[255];
		sprintf( errorStr, "couldn't find block %s in file %s", blockName, file->getFilename() );
		Assert( 0, 0, errorStr );
		return -1;
	}
		animations[NORMAL].init( file, "Normal" );
		animations[HIGHLIGHT].init( file, "Highlight" );
		animations[PRESSED].init( file, "Pressed" );
		animations[PRESSEDHIGHLIGHT].init( file, "HighlightPressed" );
		animations[DISABLED].init( file, "Disabled" );

		return 0;
}

void aAnimGroup::update()
{
	animations[NORMAL].update();
	animations[getState()].update();
}

long aAnimGroup::getCurrentColor( aAnimGroup::STATE state) const
{
	return animations[state].getColor();
}

void aAnimGroup::setState( aAnimGroup::STATE newState )
{
	if ( newState != curState )
	{
		curState = newState;
		if ( newState != NORMAL )
			animations[curState].begin();
	}

}

aAnimGroup::STATE aAnimGroup::getState() const
{
	return curState;
}
