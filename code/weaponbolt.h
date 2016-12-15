//---------------------------------------------------------------------------
//
// weaponBolt.h -- File contains the NEW and IMPROVED Weapon class for MC2
//					ALL weapons go through this class now.
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef WEAPONBOLT_H
#define WEAPONBOLT_H
//---------------------------------------------------------------------------
// Include Files

#ifndef DWEAPONBOLT_H
#include"dweaponbolt.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#include<stuff/stuff.hpp>
#include<gosfx/gosfxheaders.hpp>
//---------------------------------------------------------------------------
// Macro Definitions
#define ALL_COLORS			4

//---------------------------------------------------------------------------
class WeaponBoltType : public ObjectType
{
	//Data Members
	//-------------
	public:
	
		long			fireSoundFX;
		long			hitEffectObjNum;
		long			missEffectObjNum;
		long			lightEffectObjNum;
		long			fireEffect;
		long			trailEffect;
		
		DWORD           frontRGB;
		DWORD           backRGB;
		DWORD           middleRGB;
		DWORD           midEdgeRGB;
		
		float			projLength;
		float			bulgeLength;
		float			bulgeWidth;
		float			velocity;
		
		unsigned char	boltAlpha;
		unsigned char	edgeAlpha;

		bool			lightSource;
		float			maxRadius;
		float			outRadius;
		DWORD			lightRGB;
		float			intensity;
		
		bool			isBeam;
		float			beamDuration;
		float			beamWiggle;
		char			*textureName;
		float			uvAnimRate;
		float			uvRepeat;
		float			unitLength;
		bool			mipTexture;
		bool			arcEffect;
		float			arcHeight;
		float			afterHitTime;
		float			areaEffectDmg;
		float			areaEffectRad;
		
	//Member Functions
	//-----------------
	public:

		void init (void)
		{
			ObjectType::init();
		
			objectTypeClass = WEAPONBOLT_TYPE;
			objectClass = WEAPONBOLT;

			fireSoundFX = hitEffectObjNum = missEffectObjNum = lightEffectObjNum = fireEffect = trailEffect = 0;
				
			frontRGB = middleRGB = backRGB = midEdgeRGB = 0;
			
			projLength = bulgeLength = bulgeWidth = velocity = 0.0;	

			boltAlpha = edgeAlpha = 0xff;

			lightSource = false;
			maxRadius = outRadius = 0.0f;
			lightRGB = 0x00000000;
			intensity = 1.0;
			
			isBeam = false;
			beamDuration = 0.0f;
			beamWiggle = 0.0f;
			mipTexture = false;
			
			textureName = NULL;
			
			arcEffect = false;
			arcHeight = -1.0f;
			
			afterHitTime = 0.0f;
			
			areaEffectDmg = areaEffectRad = 0.0f;
		}
		
		WeaponBoltType (void)
		{
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);
		long init (FitIniFilePtr objFile);
		
		~WeaponBoltType (void)
		{
			destroy();
		}
		
		virtual void destroy (void);
		
		virtual GameObjectPtr createInstance (void);

		//--------------------------------------------------
		// This functions creates lights for the below bolts
		TG_LightPtr getLight (void);
		
		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);
		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

//---------------------------------------------------------------------------
typedef struct _WeaponBoltData : public GameObjectData
{
	GameObjectWatchID	ownerWID;
	int32_t				hotSpotNumber;

	GameObjectWatchID	targetWID;
	int32_t				targetHotSpot;
	Stuff::Vector3D		targetPosition;
	float				distanceToTarget;
	float				halfDistanceToTarget;

	WeaponShotInfo		weaponShot;

	Stuff::Vector3D		laserPosition;
	Stuff::Vector3D		laserVertices[4];
	Stuff::Vector3D		laserSide[4];

	int32_t				effectId;
	bool 				hitTarget;

	float				timeLeft;
	Stuff::Vector3D		hsPos;

	float				hitLeft;
	DWORD				mcTextureHandle;
	DWORD				gosTextureHandle;
	float				startUV;
	float				goalHeight;
} WeaponBoltData;

class WeaponBolt : public GameObject
{
	//Data Members
	//-------------
		protected:

			GameObjectWatchID	ownerWID;
			int32_t				hotSpotNumber;
		
			GameObjectWatchID	targetWID;
			int32_t				targetHotSpot;
			Stuff::Vector3D		*targetPosition;
			float				distanceToTarget;
			float				halfDistanceToTarget;
			
			WeaponShotInfo		weaponShot;
			
			Stuff::Vector3D		laserPosition;
			Stuff::Vector3D		laserVertices[4];
			Stuff::Vector3D		laserSide[4];

			TG_LightPtr			pointLight;
			DWORD				lightId;
			
			int32_t				effectId;
			bool 				hitTarget;
			
			//NEW  GOS FX
			gosFX::Effect		*gosEffect;
			gosFX::Effect		*muzzleEffect;
			gosFX::Effect		*hitEffect;
			gosFX::Effect		*missEffect;
			gosFX::Effect		*waterMissEffect;
			
			float				timeLeft;
			Stuff::Vector3D		hsPos;
			
			float				hitLeft;
			DWORD				mcTextureHandle;
			DWORD				gosTextureHandle;
			float				startUV;
			float				goalHeight;
			
	//Member Functions
	//-----------------
		public:

		virtual void init (bool create)
		{
		}

	   	WeaponBolt (void) : GameObject()
		{
			init(true);
			hotSpotNumber = 0;
			targetHotSpot = 0;
			targetPosition = NULL;
			
			distanceToTarget = 0.0;
			
			ownerWID = targetWID = 0;

			pointLight = NULL;
			lightId = 0xffffffff;
			
			gosEffect = NULL;
			muzzleEffect = NULL;
			hitEffect = NULL;
			missEffect = NULL;
			waterMissEffect = NULL;
			
			effectId = -1;
			
			hitTarget = false;
			
			timeLeft = 0.0;
			hsPos.Zero();
			
			hitLeft = 0.0f;
			
			startUV = 0.0f;
			mcTextureHandle = 0;
			gosTextureHandle = 0xffffffff;
			
			weaponShot.damage = 0.0f;
			
			goalHeight = 0.0f;
		}

		~WeaponBolt (void)
		{
			destroy();
		}

		virtual void destroy (void);
		
		virtual long update (void);
		virtual void render (void);
		
		virtual void init (bool create, ObjectTypePtr _type);
		
		virtual void init (long fxId)
		{
			effectId = fxId;
		}

		virtual void setExists (bool set)
		{
			GameObject::setExists(set);

			//If we are setting this effect to not existing, clean up its RAM!!
			if (!set)
			{
				destroy();
			}
		}

		bool isVisible (void);
		
		void setOwner (GameObjectPtr who);
		void setTarget (GameObjectPtr who);
		void setTargetPosition (Stuff::Vector3D pos);

		void connect (GameObjectPtr source, GameObjectPtr dest, WeaponShotInfo* shotInfo = NULL, long sourceHS = 0, long targetHS = 0)
		{
			ownerWID = source->getWatchID();
			targetWID = dest->getWatchID();
			
			hotSpotNumber = sourceHS;
			targetHotSpot = targetHS;
			
			if (dest->isMech())
			{
				targetHotSpot = 0;
				if (RollDice(50))
					targetHotSpot = 1;
			}
			else	//Vehicles need to know to use the hitnode for this hotspot and NOT the weaponNode!
			{
				targetHotSpot = -1;
			}

			if (shotInfo)
				weaponShot = *shotInfo;

			//If this is an AreaEffect weapon, NO target WID, hit the target's CURRENT LOCATION!!!
			if (((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum &&
				(((WeaponBoltTypePtr)getObjectType())->areaEffectDmg > 0.0f) &&
				(((WeaponBoltTypePtr)getObjectType())->areaEffectRad > 0.0f))
			{
				Stuff::Vector3D targetLoc;
				targetLoc = ObjectManager->getByWatchID(targetWID)->getPosition();
				setTargetPosition(targetLoc);
				targetWID = 0;
			}

			//Reset Node recycle time to either BASE or beam duration.
			GameObjectPtr myOwner = ObjectManager->getByWatchID(ownerWID);
			if (((WeaponBoltTypePtr)getObjectType())->isBeam)
				myOwner->appearance->setWeaponNodeRecycle(sourceHS,((WeaponBoltTypePtr)getObjectType())->beamDuration);
			else
				myOwner->appearance->setWeaponNodeRecycle(sourceHS,BASE_NODE_RECYCLE_TIME);
		}
		
		void connect (GameObjectPtr source, Stuff::Vector3D targetLoc, WeaponShotInfo* shotInfo = NULL, long sourceHS = 0)
		{
			ownerWID = source->getWatchID();
			targetWID = 0;
			hotSpotNumber = sourceHS;
			
			setTargetPosition(targetLoc);
			
			if (shotInfo)
				weaponShot = *shotInfo;
				
			//Reset Node recycle time to either BASE or beam duration.
			GameObjectPtr myOwner = ObjectManager->getByWatchID(ownerWID);
			if (((WeaponBoltTypePtr)getObjectType())->isBeam)
				myOwner->appearance->setWeaponNodeRecycle(sourceHS,((WeaponBoltTypePtr)getObjectType())->beamDuration);
			else
				myOwner->appearance->setWeaponNodeRecycle(sourceHS,BASE_NODE_RECYCLE_TIME);
		}

		virtual void Save (PacketFilePtr file, long packetNum);

		void Load (WeaponBoltData *data);

		void CopyTo (WeaponBoltData *data);

		void finishNow (void);
};

//---------------------------------------------------------------------------
#endif





























