//***************************************************************************
//
//	artillery.h -- File contains the artillery strike Object Class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef ARTLRY_H
#define ARTLRY_H

//---------------------------------------------------------------------------

#ifndef DARTLRY_H
#include"dartlry.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#include<gosfx/gosfxheaders.hpp>
//***************************************************************************

#define NO_RAM_FOR_ARTY					0xDEFD0000
#define NO_RAM_FOR_ARTY_SHAPES			0xDEFD0001

#define NO_RAM_FOR_ARTILLERY			0xDEFD0002
#define NO_APPEARANCE_TYPE_FOR_ARTY		0xDEFD0003
#define NO_APPEARANCE_FOR_ARTY			0xDEFD0004
#define APPEARANCE_NOT_VFX_APPEAR		0xDCDC0009

#define CLAN_SEEN_FLAG				2
#define IS_SEEN_FLAG				1

#define	MAX_ARTILLERY_EXPLOSIONS		32

//***************************************************************************

class ArtilleryChunk {

	public:

		char				commanderId;
		char				strikeType;
		int                 cellRC[2];
		char				secondsToImpact;

		unsigned int        data;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void) {
			commanderId = -1;
			strikeType = -1;
			cellRC[0] = -1;
			cellRC[1] = -1;
			secondsToImpact = -1;
			data = 0;
		}

		void destroy (void) {
		}

		ArtilleryChunk (void) {
			init();
		}

		~ArtilleryChunk (void) {
			destroy();
		}

		void build (long commanderId,
					long strikeType,
					Stuff::Vector3D location,
					long seconds);

		void pack (void);

		void unpack (void);

		bool equalTo (ArtilleryChunkPtr chunk);
};

//***************************************************************************

class ArtilleryType : public ObjectType {

	public:

		MemoryPtr			frameList;				//Pointer to JMiles shape file binary 
		unsigned long		frameCount;				//Number of frames in shape file
		unsigned long		startFrame;				//Frame in List to start with.
		float				frameRate;				//Speed at which frames playback
		
		float				nominalTimeToImpact;
		float				nominalTimeToLaunch;
		
		float				nominalDamage;
		float				nominalMajorRange;
		float				nominalMajorHits;
		float				nominalMinorRange;
		float				nominalMinorHits;
		
		float				nominalSensorTime;	
		float				nominalSensorRange;
		
		float				fontScale;
		float				fontXOffset;
		float				fontYOffset;
		unsigned long		fontColor;

		long				numExplosions;
		float*				explosionOffsetX;
		float*				explosionOffsetY;
		float*				explosionDelay;

		long				numExplosionsPerExplosion;
		long				explosionRandomX;
		long				explosionRandomY;

		long				minArtilleryHeadRange;
				
	public:

		void init (void);
		
		ArtilleryType (void) {
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);
		
		long init (FitIniFilePtr objFile);
		
		~ArtilleryType (void) {
			destroy();
		}
		
		virtual void destroy (void);
		
		virtual GameObjectPtr createInstance (void);
		
		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);

		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

//***************************************************************************

typedef union _ArtilleryInfo {
	struct {
		float				timeToImpact;		//Time until strike goes off.
		float				timeToLaunch;		//Time until strike is launched (and unmoveable).
		float				sensorRange;		//If I am a sensor round, how big a range.
		float				timeToBlind;		//How long do I return sensor data.
		short				sensorSystemIndex;
		float				contactUpdate;
		char				timeString[5];
		unsigned long		explosionOffFlags;
	} strike;
} ArtilleryInfo;

typedef struct _ArtilleryData : public GameObjectData
{
	char				artilleryType;
	char				teamId;
	char				commanderId;
	ArtilleryInfo		info;
	long				effectId;

	bool				bombRunStarted;
	bool				inView;
	Stuff::Vector3D		iFacePosition;

} ArtilleryData;

class Artillery : public GameObject 
{
	//-------------
	// Data Members

	public:

		char				artilleryType;
		char				teamId;
		char				commanderId;
		ArtilleryInfo		info;
		long				effectId;
		
		gosFX::Effect		*hitEffect;
		gosFX::Effect		*rightContrail;
		gosFX::Effect		*leftContrail;
				
		Appearance			*bomber;
		bool				bombRunStarted;
		
		bool				inView;

		Stuff::Vector3D		iFacePosition;
		
		
	//Member Functions
	//-----------------
		public:

		virtual void init (bool create);

		virtual void init (bool create, long _artilleryType);

	   	Artillery (void) : GameObject() 
		{
			init(true);
		}

		~Artillery (void) 
		{
			destroy();
		}

		bool recalcBounds (CameraPtr myEye);
		
		void setArtilleryData (float impactTime) 
		{
			info.strike.timeToImpact = impactTime;
			info.strike.timeToLaunch = impactTime - 10;
		}

		virtual void setSensorRange (float range);

		void setSensorData (TeamPtr team, float sensorTime = -1.0, float range = -1.0);

		void setJustCreated (void);

		float getTimeToImpact (void) 
		{
			return(info.strike.timeToImpact);
		}
		
		float getTimeToLaunch (void) 
		{
			return(info.strike.timeToLaunch);
		}

		bool launched (void) 
		{
			return(info.strike.timeToLaunch <= 0);
		}
		
		bool impacted(void) 
		{
			return(info.strike.timeToImpact <= 0);
		}
		
		void drawSelectBox (unsigned char color);

		bool isStrike (void) 
		{
			static bool lookup[NUM_ARTILLERY_TYPES] = {true, true, false};
			return(lookup[artilleryType]);
		}

		bool isSensor (void) 
		{
			static bool lookup[NUM_ARTILLERY_TYPES] = {false, false, true};
			return(lookup[artilleryType]);
		}

		virtual void destroy (void);
		
		virtual long update (void);

		virtual void render (void);

		virtual void init (bool create, ObjectTypePtr _type);

		virtual long kill (void) 
		{
			return(NO_ERR);
		}
		
		virtual void setCommanderId (long _commanderId);
		virtual long getCommanderId (void)
		{
			return commanderId;
		}

		virtual long setTeamId (long _teamId, bool setup);
		
		virtual long getTeamId (void) 
		{
			return(teamId);
		}

		virtual bool isFriendly (TeamPtr team);

		virtual bool isEnemy (TeamPtr team);

		virtual bool isNeutral (TeamPtr team);

		virtual TeamPtr getTeam (void);

		virtual long handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk = false);

		virtual void handleStaticCollision (void);

		virtual void Save (PacketFilePtr file, long packetNum);

		void Load (ArtilleryData *data);

		void CopyTo (ArtilleryData *data);
};

//---------------------------------------------------------------------------
extern void CallArtillery (long commanderId,
						   long strikeType,
						   Stuff::Vector3D strikeLoc,
						   long secondsToImpact,
						   bool randomOffset);

extern void IfaceCallStrike (long strikeID,
							 Stuff::Vector3D* strikeLoc,
							 GameObjectPtr strikeTarget,
							 bool playerStrike = true,
							 bool clanStrike = false,
							 float timeToImpact = -1.00);

//---------------------------------------------------------------------------
#endif



