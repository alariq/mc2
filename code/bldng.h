//---------------------------------------------------------------------------
//
//	bldng.h -- File contains the Building Object Class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef BLDNG_H
#define BLDNG_H

#ifndef DBLDNG_H
#include"dbldng.h"
#endif

#ifndef TERROBJ_H
#include"terrobj.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

//---------------------------------------------------------------------------

#define NO_RAM_FOR_BUILDING				0xDCDC0006
#define NO_APPEARANCE_TYPE_FOR_BLD		0xDCDC0007
#define NO_APPEARANCE_FOR_BLD			0xDCDC0008
#define APPEARANCE_NOT_VFX_APPEAR		0xDCDC0009

#define	MAX_PRISONERS					4

typedef enum {
	BUILDING_SUBTYPE_NONE,
	BUILDING_SUBTYPE_WALL,
	BUILDING_SUBTYPE_LANDBRIDGE,
	NUM_BUILDING_SUBTYPES
} BuildingSubType;

//---------------------------------------------------------------------------

class BuildingType : public ObjectType {

	public:

		float			damageLevel;
		float			sensorRange;
		long			teamId;
		float			baseTonnage;
		float			explDmg;
		float			explRad;
		long			buildingTypeName;
		long			buildingDescriptionID;
		long			startBR;
		long			numMarines;
		long			resourcePoints;
		bool			marksImpassableWhenDestroyed;

		bool			canRefit;
		bool			mechBay;							// otherwise it's a vehicle bay.
		bool			capturable;
		bool			powerSource;
		float			perimeterAlarmRange;
		float			perimeterAlarmTimer;
		float			lookoutTowerRange;
		
		unsigned long	activityEffectId;
		
	public:

		void init (void);
		
		BuildingType (void) {
			ObjectType::init();
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);

		long init (FitIniFilePtr objFile);
		
		~BuildingType (void) {
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
typedef struct _BuildingData : public TerrainObjectData
{
	char					teamId;
	unsigned char			baseTileId;
	char					commanderId;
	GameObjectWatchID		refitBuddyWID;
	DWORD					parentId;
	GameObjectWatchID		parent;
	unsigned char			listID;
	float					captureTime;

	//PerimeterAlarms 		
	bool					moverInProximity;
	float					proximityTimer;
	long					updatedTurn;
} BuildingData;

class Building : public TerrainObject 
{
	public:
			
		char					teamId;
		unsigned char			baseTileId;
		SensorSystemPtr			sensorSystem;
		char					commanderId;									//If capturable, who last captured it...
		GameObjectWatchID		refitBuddyWID;
		DWORD					parentId;
		GameObjectWatchID		parent;
		unsigned char			listID;
		float					captureTime;
		float					scoreTime;

		//PerimeterAlarms 		
		bool					moverInProximity;
		float					proximityTimer;
		long					updatedTurn;

	public:

		virtual void init (bool create) {
			sensorSystem = NULL;
			setFlag(OBJECT_FLAG_JUSTCREATED, true);
			appearance = NULL;
			vertexNumber = 0;
			blockNumber = 0;
			baseTileId = 0;
			commanderId = -1;
			teamId = -1;
			refitBuddyWID = 0;
			parentId = 0xffffffff;
			parent = 0;
			powerSupply = 0;
			numSubAreas0 = 0;
			numSubAreas1 = 0;
			subAreas0 = NULL;
			subAreas1 = NULL;
			listID = 255;
			captureTime = 0.0;
			scoreTime = 0.0;
			moverInProximity = false;
			proximityTimer = 0.0f;
			updatedTurn = -1;
		}

	   	Building (void) : TerrainObject() {
			init(true);
		}

		~Building (void) {
			destroy();
		}
		
		virtual void destroy (void);
		
		virtual long update (void);

		virtual void render (void);
		
		virtual void init (bool create, ObjectTypePtr objType);

		virtual void setSensorRange (float range);

		void setSensorData (TeamPtr team, float range = -1.0, bool setTeam = true);

		virtual long setTeamId (long _teamId, bool setup);
		
		virtual long getTeamId (void) {
			return(teamId);
		}

		virtual long getDescription(){ return ((BuildingType*)getObjectType())->buildingDescriptionID; }

		virtual TeamPtr getTeam (void);

		virtual bool isFriendly (TeamPtr team);

		virtual bool isEnemy (TeamPtr team);

		virtual bool isNeutral (TeamPtr team);

		void lightOnFire (float timeToBurn);

		long updateAnimations (void);
		
		virtual long handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk = false);

		virtual void setDamage (float newDamage);		//Damage encodes which groundtile to use, too.
		
		virtual long kill (void) {
			return(NO_ERR);
		}

		virtual char* getName (void);

		virtual Stuff::Vector3D getPositionFromHS (long weaponType) 
		{
			//-----------------------------------------
			// Hotspot for buildings is position plus 
			// some Z based on OBB to make Effect visible.
			// If this doesn't work, replace with art defined site.
			Stuff::Vector3D hsPos = position;
			if (appearance)
			{
				hsPos = appearance->getHitNode();
				if (hsPos == position)
				{
					hsPos.z += appearance->getTopZ() * 0.5f;
				}
			}

			return(hsPos);
		}

		virtual Stuff::Vector3D getLOSPosition (void) 
		{
			//-----------------------------------------
			// Hotspot for buildings is position plus 
			// some Z based on OBB to make Effect visible.
			//
			// Use THIS position for LOS Calc!!!
			Stuff::Vector3D hsPos = position;

			if (appearance)
				hsPos.z += appearance->getTopZ() * 0.5f;

			BuildingTypePtr bldgType = ((BuildingTypePtr)getObjectType());
			if ((bldgType->lookoutTowerRange > 0.0f) ||
				(bldgType->sensorRange > 0.0f))
			{
				hsPos.z = position.z + 75.0f;
			}

			return(hsPos);
		}

		virtual float getDestructLevel (void)
		{
			return (getDamageLevel() - damage);
		}

		virtual void setRefitBuddy (GameObjectWatchID objWID) {
			refitBuddyWID = objWID;
		}

		virtual void openFootPrint (void);

		virtual void closeFootPrint (void);

		bool isVisible (void);

		virtual bool isCaptureable (long capturingTeamID);

		virtual void setCommanderId (long _commanderId);

		virtual long getCommanderId (void) {
			return(commanderId);
		}

		virtual float getDamageLevel (void);

		virtual void getBlockAndVertexNumber (int& blockNum, int& vertexNum) {
			blockNum = blockNumber;
			vertexNum = vertexNumber;
		}

		virtual bool isBuilding(void) {
			return(true);
		}

		virtual bool isTerrainObject (void) {
			return(true);
		}
		
		void createBuildingMarines (void);

		virtual bool isLinked (void);

		virtual GameObjectPtr getParent (void);

		virtual void setParentId (DWORD pId);

		virtual SensorSystem* getSensorSystem(){ return sensorSystem; }
		
		virtual float getAppearRadius (void)
		{
			return appearance->getRadius();
		}

		virtual bool canBeCaptured (void)
		{
			return ((BuildingTypePtr)getObjectType())->capturable;
		}

		virtual bool isSelectable()
		{
			return appearance->isSelectable();
		}

		virtual bool isPowerSource(void)
		{
			return ((BuildingTypePtr)getObjectType())->powerSource;
		}
		
		virtual bool isLit (void)
		{
			if (appearance)
				return appearance->getIsLit();
				
			return false;
		}
		
		virtual bool isSpecialBuilding(void)
		{
			BuildingTypePtr bldgType = ((BuildingTypePtr)getObjectType());
//			if ((bldgType->getObjTypeNum() == GENERIC_HQ_BUILDING_OBJNUM) ||
//				(bldgType->getObjTypeNum() == GENERIC_DESTRUCTIBLE_RESOURCE_BUILDING_OBJNUM) ||
//				(bldgType->getObjTypeNum() == GENERIC_INDESTRUCTIBLE_RESOURCE_BUILDING_OBJNUM))
//				return(true);

			if (((bldgType->perimeterAlarmRange > 0.0f) &&
				(bldgType->perimeterAlarmTimer > 0.0f)) ||
				(bldgType->lookoutTowerRange > 0.0f) ||
				(bldgType->sensorRange > 0.0f))
			{
				return true;
			}
				
			return false;
		}
 
		virtual bool isLookoutTower (void)
		{
			BuildingTypePtr bldgType = ((BuildingTypePtr)getObjectType());
			if (bldgType->lookoutTowerRange > 0.0f)
			{
				return true;
			}
				
			return false;
		}
		
		virtual float getRefitPoints(void) 
		{
			return getDamageLevel() - getDamage();
		}

		virtual bool burnRefitPoints(float pointsToBurn);

		virtual void Save (PacketFilePtr file, long packetNum);

		void Load (BuildingData *data);

		void CopyTo (BuildingData *data);

};

#endif

//***************************************************************************




