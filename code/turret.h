//***************************************************************************
//
//	turret.h -- File contains the Turret Object Class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TURRET_H
#define TURRET_H

//---------------------------------------------------------------------------

#ifndef DTURRET_H
#include"dturret.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef TERROBJ_H
#include"terrobj.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions
#define NO_RAM_FOR_BUILDING				0xDCDC0006
#define NO_APPEARANCE_TYPE_FOR_BLD		0xDCDC0007
#define NO_APPEARANCE_FOR_BLD			0xDCDC0008
#define APPEARANCE_NOT_VFX_APPEAR		0xDCDC0009

#define	MAX_TURRET_WEAPONFIRE_CHUNKS	32

#define MAX_TURRET_WEAPONS				4
//---------------------------------------------------------------------------

class TurretType : public ObjectType {

	protected:

		float			damageLevel;
		//unsigned long	dmgLevelClosed;

	public:
		
		unsigned long	blownEffectId;
		unsigned long	normalEffectId;
		unsigned long	damageEffectId;
		
		float			baseTonnage;
		
		float			explDmg;
		float			explRad;
		
		float			littleExtent;
		float			LOSFactor;
		
		float			engageRadius;
		float			turretYawRate;
		long			weaponMasterId[MAX_TURRET_WEAPONS];
		long			pilotSkill;
		float			punch;
		
		long			turretTypeName;
		long			buildingDescriptionID;

	public:

		void init (void) {
			ObjectType::init();
			objectTypeClass = TURRET_TYPE;
			objectClass = TURRET;
			damageLevel = 0.0;
			blownEffectId = 0xFFFFFFFF;
			normalEffectId = 0xFFFFFFFF;
			damageEffectId = 0xFFFFFFFF;
			explDmg = explRad = 0.0;
			baseTonnage = 0.0;
			weaponMasterId[0] = weaponMasterId[1] = weaponMasterId[2] = weaponMasterId[3] = -1;
			pilotSkill = 0;
			punch = 0.0;
			turretYawRate = 0.0;
			turretTypeName = 0;
			LOSFactor = 1.0f;
		}
		
		TurretType (void) {
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);

		long init (FitIniFilePtr objFile);
		
		~TurretType (void) {
			destroy();
		}
		
		float getDamageLevel (void) {
			return(damageLevel);
		}
			
		virtual void destroy (void);
		
		virtual GameObjectPtr createInstance (void);
		
		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);
		
		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

//---------------------------------------------------------------------------
typedef struct _TurretData : public TerrainObjectData
{
	char					teamId;
	float					turretRotation;
	bool					didReveal;
	GameObjectWatchID		targetWID;
	float					readyTime[MAX_TURRET_WEAPONS];
	float					lastFireTime[MAX_TURRET_WEAPONS];
	float					minRange;										// current min attack range
	float					maxRange;										// current max attack range
	int32_t                 numFunctionalWeapons;							// takes into account damage, etc.

	float					idleWait;
	Stuff::Vector3D			idlePosition;
	Stuff::Vector3D			oldPosition;
	DWORD					parentId;
	GameObjectWatchID		parent;
	int32_t					currentWeaponNode;
} TurretData;

class Turret : public TerrainObject {
	
	public:
	
		char					teamId;
		float					turretRotation;
		bool					didReveal;
		GameObjectWatchID		targetWID;
		float					readyTime[MAX_TURRET_WEAPONS];
		float					lastFireTime[MAX_TURRET_WEAPONS];
		float					minRange;										// current min attack range
		float					maxRange;										// current max attack range
		int32_t					numFunctionalWeapons;							// takes into account damage, etc.
		
		long					netRosterIndex;
		long					numWeaponFireChunks[2];
		unsigned long			weaponFireChunks[2][MAX_TURRET_WEAPONFIRE_CHUNKS];

		TG_LightPtr				pointLight;
		DWORD					lightId;
		float					idleWait;
		Stuff::Vector3D			idlePosition;
		Stuff::Vector3D			oldPosition;
		DWORD					parentId;
		GameObjectWatchID		parent;
		int32_t					currentWeaponNode;

		static bool				turretsEnabled[MAX_TEAMS];

	public:

		void init (bool create);

	   	Turret (void) : TerrainObject() {
			init(true);
		}

		~Turret (void) {
			destroy();
		}

		virtual void updateDebugWindow (GameDebugWindow* debugWindow);

		virtual long setTeamId (long _teamId, bool setup);
		
		virtual long getTeamId (void) {
			return(teamId);
		}

		virtual TeamPtr getTeam (void);

		virtual bool isFriendly (TeamPtr team);

		virtual bool isEnemy (TeamPtr team);

		virtual bool isNeutral (TeamPtr team);

		virtual void destroy (void);
		
		virtual long update (void);
		
		virtual void render (void);
		
		virtual void init (bool create, ObjectTypePtr _type);

		virtual long handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk = false);

		long getNetRosterIndex (void) {
			return(netRosterIndex);
		}

		void setNetRosterIndex (long index) {
			netRosterIndex = index;
		}

		void lightOnFire (float timeToBurn);

		virtual long kill (void) 
		{
			//Do nothing for now.  Later, Buildings may do something.
			return NO_ERR;
		}

		virtual bool isBuilding(void) 
		{
			return (true);
		}
		
		virtual void getBlockAndVertexNumber (int &blockNum, int &vertexNum) {
			blockNum = blockNumber;
			vertexNum = vertexNumber;
		}
		
		bool isWeaponReady (long weaponId);

		bool isWeaponMissile (long weaponId);

		bool isWeaponStreak (long weaponId);

		float calcAttackChance (GameObjectPtr target, long* range, long weaponId);
		
		void recordWeaponFireTime (long weaponId);
		
		void startWeaponRecycle (long weaponId);

		long getNumWeaponFireChunks (long which) 
		{
			return(numWeaponFireChunks[which]);
		}

		long clearWeaponFireChunks (long which);

		long addWeaponFireChunk (long which, WeaponFireChunkPtr chunk);

		long addWeaponFireChunks (long which, unsigned long* packedChunkBuffer, long numChunks);

		long grabWeaponFireChunks (long which, unsigned long* packedChunkBuffer);

		virtual long updateWeaponFireChunks (long which);

		void fireWeapon (GameObjectPtr target, long weaponId);
		
		virtual Stuff::Vector3D getPositionFromHS (long weaponNum);

		virtual float relFacingTo (Stuff::Vector3D goal, long bodyLocation = -1);
		
		long handleWeaponFire (long weaponIndex,
							   GameObjectPtr target,
							   Stuff::Vector3D* targetPoint,
							   bool hit,
							   float entryAngle,
							   long numMissiles,
							   long hitLocation);

		virtual void printFireWeaponDebugInfo (GameObjectPtr target, Stuff::Vector3D* targetPoint, long chance, long roll, WeaponShotInfo* shotInfo);

		virtual void printHandleWeaponHitDebugInfo (WeaponShotInfo* shotInfo);
		
		float getLittleExtent (void) 
		{
			return (((TurretTypePtr)getObjectType())->littleExtent);
		}

		virtual bool isLinked (void);

		virtual GameObjectPtr getParent (void);

		virtual void setParentId (DWORD pId);

		virtual long getDescription(){ return ((TurretType*)getObjectType())->buildingDescriptionID; }

		long updateAnimations (void);
		
		virtual Stuff::Vector3D getLOSPosition (void);
		
		virtual float getDestructLevel (void)
		{
			return ((TurretTypePtr)getObjectType())->getDamageLevel() - damage;
		}

		virtual void setDamage (float newDamage);

		virtual long calcFireRanges (void);

		virtual void Save (PacketFilePtr file, long packetNum);

		void CopyTo (TurretData *data);

		void Load (TurretData *data);

		virtual void renderShadows (void);
};

//***************************************************************************

#endif



