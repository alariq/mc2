//***************************************************************************
//
//	Carnage.h -- Base class for any and all damage-related effects that
//				 dynamically appear and disappear with short lifetimes.
//				 Things such as fires, explosions and debris.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CARNAGE_H
#define CARNAGE_H

//---------------------------------------------------------------------------

#ifndef DCARNAGE_H
#include"dcarnage.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#include<gosfx/gosfxheaders.hpp>
//***************************************************************************
typedef union _CarnageInfo 
{
	struct {
		float			timeToBurn;
		float			radius;
		long			reallyVisible;
	} fire;
	
	struct {
		float			timer;
		float			radius;
		float			chunkSize;
	} explosion;
} CarnageInfo;

//---------------------------------------------------------------------------
typedef struct _CarnageData : public GameObjectData
{
	CarnageEnumType		carnageType;
	GameObjectWatchID	ownerWID;
	CarnageInfo			info;

	long				effectId;
} CarnageData;

class Carnage : public GameObject 
{
	//-------------
	// Data Members

		public:

			CarnageEnumType		carnageType;
			GameObjectWatchID	ownerWID;
			CarnageInfo			info;

			static float		maxFireBurnTime;
			
			//NEW  GOS FX
			long				effectId;
			gosFX::Effect		*gosEffect;
			
			//Lighting Data
			TG_LightPtr			pointLight;
			DWORD				lightId;

			float				intensity;
			float				inRadius;
			float 				outRadius;
			float				duration;

	//-----------------
	// member Functions

		public:

			virtual void init (bool create);

			Carnage (void) : GameObject() 
			{
				init(true);
			}

			~Carnage (void) 
			{
				destroy();
			}

			virtual void init (bool create, ObjectTypePtr _type);

			virtual void init (CarnageEnumType _carnageType);
			
			virtual void init (long fxId)
			{
				effectId = fxId;
			}

			virtual void destroy (void);

			virtual long kill (void) 
			{
				return(NO_ERR);
			}

			virtual bool onScreen (void);

			virtual long update (void);

			virtual void render (void);

			virtual void setOwner (GameObjectPtr myOwner) 
			{
				if (myOwner) {
					ownerWID = myOwner->getWatchID();
					if (carnageType == CARNAGE_FIRE)
						myOwner->setFireHandle(getHandle());
					}
				else
					ownerWID = 0;
			}

			GameObjectPtr getOwner (void);

			bool isVisible (long whichFire = 0);

			virtual float getExtentRadius (void) 
			{
				if (carnageType == CARNAGE_FIRE)
					return(info.fire.radius);
				else if (carnageType == CARNAGE_EXPLOSION)
					return(info.explosion.radius);
				return(0.0);
			}

			virtual void setExtentRadius (float radius) 
			{
				if (carnageType == CARNAGE_FIRE)
					info.fire.radius = radius;
				else if (carnageType == CARNAGE_EXPLOSION)
					info.explosion.radius = radius;
			}

 			virtual void handleStaticCollision (void);

			void addTimeLeft (float seconds);

			void finishNow (void);

			virtual void Save (PacketFilePtr file, long packetNum);

			void CopyTo (CarnageData *data);

			void Load (CarnageData *data);
};

//---------------------------------------------------------------------------
class FireType : public ObjectType 
{
	//-------------
	// Data Members
	
	public:
	
		float			damageLevel;
		unsigned long	soundEffectId;
		unsigned long	lightObjectId;
		
		unsigned long 	startLoopFrame;
		unsigned long	endLoopFrame;
		unsigned long	numLoops;
		
		float			maxExtent;		//How Good am I at setting off other fires
		float			timeToMax;		//How long before I grow to MaxExtent size
		
		long			totalFires;
		
		float*			fireOffsetX;
		float*			fireOffsetY;
		float*			fireDelay;
		
		long*			fireRandomOffsetX;
		long*			fireRandomOffsetY;
		long*			fireRandomDelay;
		
	//-----------------
	// Member Functions

	public:

		void init (void) 
		{
			ObjectType::init();
			
			objectTypeClass = FIRE_TYPE;
			objectClass = FIRE;

			damageLevel = 0.0;
			soundEffectId = -1;
			
			timeToMax = 0.0;
			maxExtent = 0.0;
			
			totalFires = 1;
			
			fireOffsetX = NULL;
			fireOffsetY = NULL;
			fireDelay = NULL;
			
			fireRandomOffsetX = NULL;
			fireRandomOffsetY = NULL;
			fireRandomDelay = NULL;
		}
		
		FireType (void) 
		{
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);

		long init (FitIniFilePtr objFile);
		
		~FireType (void) 
		{
			destroy();
		}
		
		virtual void destroy (void);
		
		virtual GameObjectPtr createInstance (void);
		
		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);

		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

//---------------------------------------------------------------------------

class ExplosionType : public ObjectType 
{
	public:

		float				damageLevel;
		unsigned long		soundEffectId;
		unsigned long		lightObjectId;
		long				explRadius;
		float				chunkSize;

		float 				lightMinMaxRadius;
		float				lightMaxMaxRadius;
		float 				lightOutMinRadius;
		float 				lightOutMaxRadius;
		DWORD 				lightRGB;
		float 				maxIntensity;
		float 				minIntensity;
		float				duration;
		float 				delayUntilCollidable;

	public:

		void init (void) 
		{
			ObjectType::init();
			objectTypeClass = EXPLOSION_TYPE;
			objectClass = EXPLOSION;
			damageLevel = 0.0;
			soundEffectId = -1;
			explRadius = 0;
			chunkSize = 0.0;
			delayUntilCollidable = 0.5f;

			lightMinMaxRadius = 0.0f;
			lightMaxMaxRadius = 0.0f;
			lightOutMinRadius = 0.0f;
			lightOutMaxRadius = 0.0f;
			lightRGB = 0x00000000;         
			maxIntensity = 0.0f;     
			minIntensity = 0.0f;     
			duration = 0.0f;         
		}

		ExplosionType (void) 
		{
			init();
		}

		virtual long init (FilePtr objFile, unsigned long fileSize);

		long init (FitIniFilePtr objFile);

		~ExplosionType (void) 
		{
			destroy();
		}

		virtual void destroy (void);

		virtual GameObjectPtr createInstance (void);

		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);

		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

#endif

//***************************************************************************




