//******************************************************************************************
//	weather.h - This file contains the weather class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef WEATHER_H
#define WEATHER_H
//----------------------------------------------------------------------------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

//----------------------------------------------------------------------------------
// Macro Definitions
#define			BASE_RAIN_UPDATE_TIME			30.0f
#define			BASE_RAIN_HEIGHT				500.0f
#define			BASE_RAIN_LENGTH				10.0f
#define			BASE_RAIN_VEL					300.0f
#define			BASE_RAIN_TILT					10.0f
#define			BASE_RAIN_LIGHT_RATE			0.01f
#define			BASE_RAIN_RANDOM_POS_FACTOR		500.0f
#define			BASE_RAIN_RANDOM_HGT_FACTOR		250.0f
#define			BASE_RAIN_RANDOM_VEL_FACTOR		10.0f
#define			BASE_RAIN_RANDOM_LEN_FACTOR		10.0f

#define			BASE_LIGHTENING_CHANCE			0.03f
#define			BASE_LIGHTENING_FALLOFF			400.0f
#define 		BASE_LIGHTENING_FLASH_AGAIN		2.0f
#define			BASE_LIGHTENING_FLASH_CHECK		1.0f
#define			BASE_THUNDER_RANDOM_START		5.0f
//----------------------------------------------------------------------------------
// Struct Definitions
typedef struct _RainDrops
{
	Stuff::Point3D		position;
	float				length;
	
	void init (void)
	{
		position.Zero();
		length = 0.0f;
	}
	
} RainDrops;

//----------------------------------------------------------------------------------
// Class Definitions
class Weather
{
	protected:
		bool			weatherActive;				//Should we even bother calling update or render?
	
		float			rainLevel;					// How bad is it raining
													// This determines probability of thunder/lightening
													// how many drops we are drawing and how dark it is.
													// Levels are:
													//		0.0f				Not raining.
													//		>0.0f - <1.0f		Threatening.  Getting darker.  Some ambient wind noise.  Distant thunder.
													//		>=1.0f - <1.2f		Drizzle.  ambient wind.  No thunder.
													//		>=1.2f - <2.0f		Raining.  Steady downpour.  Not much thunder.  Dark enough for lights.
													//		>=2.0f - <3.0f		Raining Hard.  Some lightening/thunder.  Full night conditions.
													//		>=3.0f				Storm.  Lots of thunder/lightening.  Full Night Conditions.
													
		float			lighteningLevel;			// Brightness of lightening this frame.  Effects Ambient/sun lighting.
		float			thunderTime;				// Time until crack of thunder is heard.
		DWORD			thunderSFX;					// Sound Effect of thunder to play
		float			lighteningCheck;			// Interval between checks of lightening.
		float			baseLighteningChance;		// Base chance lightening will flash at each check.
		
		DWORD			totalRainDrops;				// Total number of drops I can draw.
		DWORD			currentRainDrops;			// Current number of drops in use.
		
		RainDrops*		rainDrops;					// Pointer to data for the drops.
		
		float			rainTrend;					// Either plus 1.0f or minus 1.0f.  Based on raining more or less then it was.
		float			rainFactor;					// Increase/decrease in rainLevel per second based on rainTrend.
		float			rainLightLevel;				// Matches the rainLevel but moves slowly per frame slow light fades out.
		float			rainUpdateTime;				// Time until next RainTrend check in seconds.
		long			baseRainChance;				// Chance of it raining on any given rainUpdateTime.
		
		float			oldFog;						// Original FOGFull Height value.
		
	public:
	
		Weather (void)
		{
			init();
		}
		
		void init (void)
		{
			rainLevel = lighteningLevel = rainLightLevel = 0.0f;
			lighteningCheck = 1.0f;
			totalRainDrops = currentRainDrops = 0;
			
			rainTrend = 1.0f;
			rainFactor = 0.0f;
			rainUpdateTime = BASE_RAIN_UPDATE_TIME;
			
			baseRainChance = 0;
			
			rainDrops = NULL;
			
			thunderSFX = 0xffffffff;
			thunderTime = 0.0f;
			
			oldFog = 0.0f;
			
			weatherActive = false;
		}
		
		void destroy (void);
		
		~Weather (void)
		{
			destroy();
		}
		
		void init (DWORD maxDrops, float startingRain, float baseRainChance, float baseLighteningChance = BASE_LIGHTENING_CHANCE);
		
		void init (FitIniFilePtr missionFile);
		
		void update (void);
		
		void render (void);

		void save (FitIniFile *file);
		void load (FitIniFile *file);
};

//----------------------------------------------------------------------------------
extern Weather *weather;
//----------------------------------------------------------------------------------
#endif

