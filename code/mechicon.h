#ifndef MECHICON_H
#define MECHICON_H
/*************************************************************************************************\
MechIcon.h			: Interface for the MechIcon component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef AFONT_H
#include"afont.h"
#endif

#ifndef MC2movie_H
#include"mc2movie.h"
#endif

//*************************************************************************************************
class Mover;
class MechWarrior;

#define NUM_DEATH_INFOS 6
/**************************************************************************************************
CLASS DESCRIPTION
MechIcon:
**************************************************************************************************/
class PilotIcon
{

public:

	static const long DEAD_PILOT_INDEX;

	PilotIcon();

	static void swapResolutions(bool bForce = 0);

	void setPilot( MechWarrior* pWarrior );
	void setTextureIndex(  int newIndex ) { pilotTextureIndex = newIndex; }
	void render( float left, float top, float right, float bottom );


private:

	unsigned long pilotTextureIndex;
	
	static float			pilotIconX;
	static float			pilotIconY;

	static unsigned long	s_pilotTextureHandle;
	static unsigned long	s_pilotTextureWidth;

	friend class ForceGroupIcon;
	friend class MechIcon;
	friend class Mission;
};


class ForceGroupIcon
{
public:

		ForceGroupIcon();
		virtual ~ForceGroupIcon();

		virtual void update() = 0;
		virtual void render();
		void renderUnitIcon( float left, float top, float right, float bottom );
		virtual void renderUnitIconBack( float left, float top, float right, float bottom );
		void renderPilotIcon( float left, float top, float right, float bottom );
		virtual bool			init( Mover* pMover ){ return false; }
		void init();

		


		bool inRegion( int x, int y );

		void click( bool shiftDn );
		void rightClick();

		virtual void setDrawBack( bool bSet) { };

		static int __cdecl sort( const void* p1, const void* p2 );

		const char* getPilotName();

		static void init( FitIniFile& file, int which );
		void swapResolutions(bool bForce);
		static void resetResolution(bool bForce);

		void setLocationIndex( int i );
		void beginDeathAnimation( ){ deathAnimationTime = .0001f; } 
		bool deathAnimationOver(){ return deathAnimationTime > animationInfos[NUM_DEATH_INFOS-1].time; }
		bool isAnimatingDeath() { return deathAnimationTime ? 1 : 0; }

protected:

		static DWORD            damageColors[4][3];

		static DWORD            s_textureHandle[5];
		static bool				s_slotUsed[240];
		static TGAFileHeader*	s_textureMemory;

		static float			unitIconX;
		static float		 	unitIconY;
		static int				curScreenWidth;

		static StaticInfo*		jumpJetIcon;

		static aFont*			gosFontHandle;
		
		static gos_VERTEX	bmpLocation[17][5]; // in screen coords
		
		static GUI_RECT textArea[17];
		static GUI_RECT pilotRect[17];
		static GUI_RECT selectionRect[17];
		static GUI_RECT healthBar[17];
		static long		pilotTextTop[17];

		static float	pilotLocation[17][4];
		Mover*		unit;

		static DWORD ForceGroupColors[11];

		void drawBar( float barStatus );
		static void setIconVariables();


		friend class ForceGroupBar;
		friend void __stdcall TerminateGameEngine(void);

		bool	bDrawBack;

		int		damageIconIndex;
		int		backDamageIndex;
		int		locationIndex;

		float	deathAnimationTime;

		struct AnimationInfo
		{
			float	time;
			long	color;
		};

		static AnimationInfo animationInfos[NUM_DEATH_INFOS];

		void drawDeathEffect();

		PilotIcon	pilotIcon;

		float		msgPlayTime;

		static		MC2MoviePtr 	bMovie;
		static		DWORD			pilotVideoTexture;
		static		MechWarrior*	pilotVideoPilot;
		
};

class MechIcon : public ForceGroupIcon
{
	public:

		MechIcon(){}
		~MechIcon(){}

		virtual bool			init( Mover* pMover );
		virtual void			update();

		static TGAFileHeader* s_MechTextures;

		void doDraw( char* newDamage, char* oldDamage, unsigned long handle, unsigned long where);
		virtual void setDrawBack( bool bSet);

		

		

	private:

		char	damage[8];
		char	backDamage[8];
		bool init( long whichIndex );
		bool initTextures();

		
};

class VehicleIcon : public ForceGroupIcon
{
public:

	VehicleIcon(){}
	~VehicleIcon(){}

	virtual void renderUnitIconBack( float left, float top, float right, float bottom ){}
	virtual void	update();
	virtual bool			init( Mover* pMover );

	static TGAFileHeader* s_VehicleTextures;

private:

		char	damage[5];

};



//*************************************************************************************************
#endif  // end of file ( MechIcon.h )
