
//---------------------------------------------------------------------------
//
//	gameobj.h -- File contains the Basic Game Object definition
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GAMEOBJ_H
#define GAMEOBJ_H

//---------------------------------------------------------------------------
// Include Files

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DOBJTYPE_H
#include"dobjtype.h"
#endif

#ifndef DAPPEAR_H
#include"dappear.h"
#endif

#ifndef DCONTACT_H
#include"dcontact.h"
#endif

#ifndef DTEAM_H
#include"dteam.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

#ifndef MECHCLASS_H
#include"mechclass.h"
#endif

#ifndef DGAMEOBJ_H
#include"dgameobj.h"
#endif

#ifndef DCARNAGE_H
#include"dcarnage.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#ifndef STUFF_HPP
#include<stuff/stuff.hpp>
#endif

extern float metersPerWorldUnit;
extern ObjectTypeManagerPtr objectTypeManager;

//---------------------------------------------------------------------------

#define	CELLS_PER_TILE					3

#define	GAMEOBJECT_FLOATS				4
#define	GAMEOBJECT_LONG					2
#define	GAMEOBJECT_BYTES				1

#define	RELPOS_FLAG_ABS					1
#define	RELPOS_FLAG_PASSABLE_START		2
#define	RELPOS_FLAG_PASSABLE_GOAL		4

typedef enum {
	ATTACKSOURCE_WEAPONFIRE,
	ATTACKSOURCE_COLLISION,
	ATTACKSOURCE_DFA,
	ATTACKSOURCE_MINE,
	ATTACKSOURCE_ARTILLERY,
	NUM_ATTACKSOURCES
} AttackSourceType;

typedef struct _WeaponShotInfo {
	GameObjectWatchID	attackerWID;
	long				masterId;			// attack weapon master ID
	float				damage;				// damage caused by this shot
	long				hitLocation;		// hit location on target
	float				entryAngle;			// angle from which target was hit

	void init (GameObjectWatchID _attackerWID, long _masterId, float _damage, long _hitLocation, float _entryAngle);
	
	void setDamage (float _damage);

	void setEntryAngle (float _entryAngle);

	void operator = (_WeaponShotInfo copy) {
		init(copy.attackerWID, copy.masterId, copy.damage, copy.hitLocation, copy.entryAngle);
	}
	
} WeaponShotInfo;

//---------------------------------------------------------------------------

typedef struct _SalvageItem {
	unsigned char		itemID;			// id from MasterComponentList;
	unsigned char		numItems;		// how many are there?
	unsigned char		numSalvagers;	// how many are salvagers are going for this item?
} SalavageItem;

//------------------------------------------------------------------------------------------

class WeaponFireChunk {

	public:
		
		char				targetType;
		long				targetId;
		long				targetCell[2];
		char				specialType;
		long				specialId;
		unsigned char		weaponIndex;
		bool				hit;
		char				entryAngle;
		char				numMissiles;
		char				hitLocation;

		unsigned long		data;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void) {
			targetType = 0;
			targetId = 0;
			targetCell[0] = 0;
			targetCell[1] = 0;
			specialType = -1;
			specialId = -1;
			weaponIndex = 0;
			hit = false;
			entryAngle = 0;
			numMissiles = 0;
			hitLocation = -1;
			data = 0;
		}

		void destroy (void) {
		}

		WeaponFireChunk (void) {
			init();
		}

		~WeaponFireChunk (void) {
			destroy();
		}

		void buildMoverTarget (GameObjectPtr target,
							   long weaponIndex,
							   bool hit,
							   float entryAngle,
							   long numMissiles,
							   long hitLocation);

		void buildTerrainTarget (GameObjectPtr target,
								 long _weaponIndex,
								 bool _hit,
								 long _numMissiles);

		void buildCameraDroneTarget (GameObjectPtr target,
									 long _weaponIndex,
									 bool _hit,
									 float _entryAngle,
									 long _numMissiles);

		void buildLocationTarget (Stuff::Vector3D location,
								  long weaponIndex,
								  bool hit,
								  long numMissiles);

		void pack (GameObjectPtr attacker);

		void unpack (GameObjectPtr attacker);

		bool equalTo (WeaponFireChunkPtr chunk);
};

//------------------------------------------------------------------------------------------

class WeaponHitChunk {

	public:

		char				targetType;
		long				targetId;
		long				targetCell[2];
		char				specialType;
		long				specialId;
		char				cause;
		float				damage;
		char				hitLocation;
		char				entryAngle;
		bool				refit;

		unsigned long		data;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);
		
		void init (void) {
			targetType = 0;
			targetId = 0;
			targetCell[0] = 0;
			targetCell[1] = 0;
			specialType = -1;
			specialId = -1;
			cause = 0;
			damage = 0.0;
			hitLocation = -1;
			entryAngle = 0;
			refit = false;
			data = 0;
		}

		void destroy (void) {
		}

		WeaponHitChunk (void) {
			init();
		}

		~WeaponHitChunk (void) {
			destroy();
		}

		void buildMoverTarget (GameObjectPtr target,
							   long cause,
							   float damage,
							   long hitLocation,
							   float entryAngle,
							   bool isRefit);

		void buildTerrainTarget (GameObjectPtr target,
								 float damage);

		void buildCameraDroneTarget (GameObjectPtr target,
									 float _damage,
									 float _entryAngle);

		void build (GameObjectPtr target, WeaponShotInfoPtr shotInfo, bool isRefit = false);

		void pack (void);

		void unpack (void);

		bool equalTo (WeaponHitChunkPtr chunk);

		bool valid (long from);
};
 
//------------------------------------------------------------------------------------------
typedef struct _GameObjectData
{
	long						objectTypeNum;
	ObjectClass					objectClass;		
	GameObjectHandle			handle;				
	long						partId;				
	unsigned long				watchID;			

	GameObjectTypeHandle		typeHandle;
	Stuff::Vector3D				position;			
	unsigned short				cellPositionRow;	
	unsigned short				cellPositionCol;
	long						d_vertexNum;		
	unsigned long				flags;				
	unsigned short				debugFlags;			
	unsigned char				status;				

	float						tonnage;			
	float			   			rotation;			
	char						appearanceTypeID[256];
	GameObjectWatchID			collisionFreeFromWID;
	float						collisionFreeTime;
	Stuff::Vector4D				screenPos;			
	long						windowsVisible;		
	float						explRadius;			
	float						explDamage;			
	short						maxCV;
	short						curCV;
	short						threatRating;
	float						lastFrameTime;		
	unsigned char				blipFrame;
	unsigned char				numAttackers;

	long						drawFlags;			
} GameObjectData;

class GameObject {

	public:

		ObjectClass					objectClass;		//What kind of object is this.
		GameObjectHandle			handle;				//Used to reference into master obj table
		long						partId;				//What is my unique part number.
		unsigned long				watchID;			//Used to reference in the game engine

		GameObjectTypeHandle		typeHandle;			//Who made me?
		Stuff::Vector3D				position;			//Where am I?
		unsigned short				cellPositionRow;	//Cell RC position
		unsigned short				cellPositionCol;
		long						d_vertexNum;		//Physical Vertex in mapData array that I'm lower right from
		unsigned long				flags;				//See GAMEOBJECT_FLAGS_ defines
		unsigned short				debugFlags;			// use ONLY for debugging purposes...
		unsigned char				status;				//Am I normal, disabled, destroyed, etc..?
	
		float						tonnage;			//How hefty am I?
		float			   			rotation;			//everything's base facing
		AppearancePtr				appearance;
		GameObjectWatchID			collisionFreeFromWID;	//Index into GameObject Table
		float						collisionFreeTime;
		Stuff::Vector4D				screenPos;			//Actual Screen position
		long						windowsVisible;		//Which Windows can see me.
		float						explRadius;			//How big is my explosion.
		float						explDamage;			//How much damage does it do?
		short						maxCV;
		short						curCV;
		short						threatRating;
		float						lastFrameTime;		//Time elapsed since last frame was drawn.  (Replaces HEAT.  No net gain in size!)
		unsigned char				blipFrame;
		unsigned char				numAttackers;

		long						drawFlags;			// bars, text, brackets, and highlight colors

		static unsigned long		spanMask;			//Used to preserve tile's LOS
		static float				blockCaptureRange;
		static bool					initialize;

	public:

		void* operator new (size_t ourSize);

		void operator delete (void *us);

		virtual void set (GameObject copy);

		virtual void init (bool create);
	
		GameObject (void) {
			init(true);
		}

		virtual void destroy (void);

		virtual ~GameObject (void) {
			destroy();
		}
		
		ObjectClass getObjectClass (void) {
			return(objectClass);
		}

		virtual long update (void) {
			return(NO_ERR);
		}

		virtual void render (void) {
		}
		
		virtual void renderShadows (void) {
		}

		virtual void updateDebugWindow (GameDebugWindow* debugWindow) {
		}

		virtual AppearancePtr getAppearance (void) {
			return(appearance);
		}
		
		virtual bool underPlayerControl (void) {
			return (false);
		}
		
		virtual long getGroupId (void) {
			return(-1);
		}

		virtual void setPartId (long newPartId) {
			partId = newPartId;
		}
		
		long getPartId (void) {
			return(partId);
		}

		virtual void setHandle (GameObjectHandle newHandle) {
			handle = newHandle;
		}

		GameObjectHandle getHandle (void) {
			return(handle);
		}

		unsigned long getWatchID (bool assign = true);

		virtual char* getName (void) {
			return(NULL);
		}

		virtual float getStatusRating (void) {
			return(0.0);
		}

		virtual float getDestructLevel (void)
		{
			return 5000.0f;		//If we somehow miss a object class, KEEP SHOOTING!!!!
		}

		void getCellPosition (long& cellRow, long& cellCol) {
			cellRow = cellPositionRow;
			cellCol = cellPositionCol;
		}

		virtual void handleStaticCollision (void) {
		}
		
		virtual void getBlockAndVertexNumber (long &blockNum, long &vertexNum);

		virtual long getTypeHandle (void) {
			return(typeHandle);
		}

		virtual ObjectTypePtr getObjectType (void);
		
		virtual void init (bool create, ObjectTypePtr _type);

		virtual long init (FitIniFile* objProfile) {
			return(NO_ERR);
		}

		bool isMover (void) {
			return((objectClass == BATTLEMECH) || (objectClass == GROUNDVEHICLE) || (objectClass == ELEMENTAL) || (objectClass == MOVER));
		}

		bool isMech (void) {
			return((objectClass == BATTLEMECH));
		}

		virtual long calcHitLocation (GameObjectPtr attacker, long weaponIndex, long attackSource, long attackType) {
			return(-1);
		}

		virtual long handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk = false) {
			return(NO_ERR);
		}

		virtual void setFireHandle (GameObjectHandle handle) {
		}

		virtual void killFire (void) {
		}

		virtual float getAppearRadius (void)
		{
			return 0.0f;
		}
		
		virtual long getTeamId (void) {
			return(-1);
		}

		virtual long getVertexNum (void)
		{
			return d_vertexNum;
		}
		
		virtual long setTeamId (long _teamId, bool setup) {
			return(NO_ERR);
		}

		virtual TeamPtr getTeam (void) {
			return(NULL);
		}

		virtual long setTeam (TeamPtr _team) {
			return(NO_ERR);
		}

		virtual bool isFriendly (TeamPtr team) {
			return(false);
		}

		virtual bool isEnemy (TeamPtr team) {
			return(false);
		}

		virtual bool isNeutral (TeamPtr team) {
			return(true);
		}

		virtual Stuff::Vector3D getPosition (void) {
			return(position);
		}

		virtual Stuff::Vector3D getLOSPosition (void) {
			return(position);
		}

		virtual Stuff::Vector3D relativePosition (float angle, float distance, unsigned long flags);
		
		virtual Stuff::Vector3D getPositionFromHS (long weaponType) 
		{
			//-----------------------------------------
			// No hot spots with regular game objects.
			// just return position.
			return(position);
		}
		
		virtual void setPosition (const Stuff::Vector3D& newPosition, bool calcPositions = true);
		
		virtual void setTerrainPosition (const Stuff::Vector3D& position, 
			const Stuff::Vector2DOf<long>& numbers){}

		virtual Stuff::Vector3D getVelocity (void) {
			Stuff::Vector3D result;
			result.Zero();
			return(result);
		}
		
		virtual Stuff::Vector4D getScreenPos (long whichOne) {
			return(screenPos);
		}
		
		virtual void setVelocity (Stuff::Vector3D &newVelocity) {
		}

		virtual float getSpeed (void) {
			return(0.0);
		}

		virtual long getMoveLevel (void) {
			return(0);
		}

		virtual float getRotation (void) 
		{
			return(rotation);
		}

		virtual void setRotation (float rot) 
		{
			rotation = rot;
		}

		virtual void rotate (float angle)
		{
		}

		virtual void rotate (float yaw, float pitch)
		{
		}
		
		virtual Stuff::Vector3D getRotationVector (void) 
		{
			Stuff::Vector3D rotationVec;
			rotationVec.x = 0.0f;
			rotationVec.y = -1.0f;
			rotationVec.z = 0.0f;
			Rotate(rotationVec, -rotation);
			return(rotationVec);
		}
		
		virtual bool calcAdjacentAreaCell (long moveLevel, long areaID, long& adjRow, long& adjCol) {
			return(false);
		}

		unsigned char getStatus (void) {
			return(status);
		}

		//NEVER call this with forceStatus UNLESS you are recovering a mech!!!
		void setStatus (long newStatus, bool forceStatus = false) 
		{
			if (((status != OBJECT_STATUS_DESTROYED) && (status != OBJECT_STATUS_DISABLED)) || forceStatus)
				status = newStatus;

			if (newStatus == OBJECT_STATUS_DESTROYED)
				status = newStatus;
		}

		virtual bool isCrippled (void) {
			return(false);
		}

		virtual bool isDisabled (void) {
			return((status == OBJECT_STATUS_DISABLED) || (status == OBJECT_STATUS_DESTROYED));
		}

		virtual bool isDestroyed (void) {
			return(status == OBJECT_STATUS_DESTROYED);
		}

		virtual float getDamage (void) {
			return(0.0);
		}

		virtual void setDamage (float newDamage) {
		}

		virtual float getDamageLevel (void) {
			return(0.0);
		}

		virtual long getContacts (long* contactList, long contactCriteria, long sortType) {
			return(0);
		}

		bool getTangible (void) {
			return((flags & OBJECT_FLAG_TANGIBLE) != 0);
		}
		
		void setTangible (bool set) {
			if (set)
				flags |= OBJECT_FLAG_TANGIBLE;
			else
				flags &= (OBJECT_FLAG_TANGIBLE ^ 0xFFFFFFFF);
		}
		
		virtual void setCommanderId (long _commanderId) {
		}

		virtual MechWarriorPtr getPilot (void) {
			return(NULL);
		}

		virtual long getCommanderId (void) {
			return(-1);
		}


		virtual long write (FilePtr objFile)
		{
			return NO_ERR;
		}
		
		virtual float distanceFrom (Stuff::Vector3D goal);

		virtual long cellDistanceFrom (Stuff::Vector3D goal);

		virtual long cellDistanceFrom (GameObjectPtr obj);

		virtual void calcLineOfSightNodes (void) {
		}

		virtual long getLineOfSightNodes (long eyeCellRow, long eyeCellCol, long* cells);

		virtual bool lineOfSight (long cellRow, long cellCol, bool checkVisibleBits = true);

		virtual bool lineOfSight (Stuff::Vector3D point, bool checkVisibleBits = true);

		virtual bool lineOfSight (GameObjectPtr target, float startExtRad = 0.0f, bool checkVisibleBits = true);
	
		virtual float relFacingTo (Stuff::Vector3D goal, long bodyLocation = -1);

		virtual float relViewFacingTo (Stuff::Vector3D goal) 
		{
			return(GameObject::relFacingTo(goal));
		}

		virtual long openStatusWindow (long x, long y, long w, long h) 
		{
			return(NO_ERR);
		}

		virtual long closeStatusWindow (void) 
		{
			return(NO_ERR);
		}

		virtual long getMoveState (void)
		{
			return 0;
		}

		virtual void orderWithdraw (void) {
			//Does nothing until this is a mover.
		}
		
		virtual bool isWithdrawing (void) {
			return(false);
		}
		
		virtual float getExtentRadius (void);

		virtual void setExtentRadius (float newRadius);
		
		virtual bool isBuilding(void) {
			return(false);
		}

		virtual bool isTerrainObject (void) {
			return(false);
		}

		virtual bool inTransport(void) {
			return(false);
		}

		virtual bool isCaptureable (long capturingTeamID) {
			return(false);
		}

		virtual bool canBeCaptured (void)
		{
			return false;
		}

		virtual bool isPrison(void) {
			return(false);
		}

		virtual bool isPowerSource(void)
		{
			return false;
		}
		
		virtual bool isSpecialBuilding(void)
		{
			return false;
		}
		
 		virtual bool isLit (void)
		{
			return false;
		}

		virtual void setPowerSupply (GameObjectPtr power)
		{
		
		}
		
		//----------------------
		// DEBUG FLAGS functions

		virtual void setDebugFlag (unsigned short flag, bool set) {
			if (set)
				debugFlags |= flag;
			else
				debugFlags &= (flag ^ 0xFFFF);
		}

		virtual bool getDebugFlag (unsigned short flag) {
			return((debugFlags & flag) != 0);
		}

		//---------------
		// FLAG functions

		virtual void setFlag (unsigned long flag, bool set) {
			if (set)
				flags |= flag;
			else
				flags &= (flag ^ 0xFFFFFFFF);
		}

		virtual bool getFlag (unsigned long flag) {
			return((flags & flag) != 0);
		}

		virtual void initFlags (void) {
			flags = OBJECT_FLAG_USEME | OBJECT_FLAG_AWAKE;
		}

		virtual void setSelected (bool set) {
			if (set)
				flags |= OBJECT_FLAG_SELECTED;
			else
				flags &= (OBJECT_FLAG_SELECTED ^ 0xFFFFFFFF);

			setDrawBars( set );
			setDrawBrackets( set );
		}

		virtual bool getSelected(void) {
			return ((flags & OBJECT_FLAG_SELECTED) != 0);
		}
		
		virtual bool isSelected (void) {
			return (getSelected());
		}

		virtual bool isSelectable()
		{
			return true;
		}

		virtual void setTargeted (bool set) {
			if (set)
				flags |= OBJECT_FLAG_TARGETED;
			else
				flags &= (OBJECT_FLAG_TARGETED ^ 0xFFFFFFFF);

			setDrawBars( set );
			setDrawText( set );
			setDrawColored( set );

			if ( isSelected() )
				setSelected( true ); // reset flags

			setDrawColored( set );

		}

		virtual bool getTargeted(void) {
			return ((flags & OBJECT_FLAG_TARGETED) != 0);
		}

		virtual void setDrawNormal()
		{
			drawFlags = DRAW_NORMAL;
		}

		virtual void setDrawText( bool set )
		{
			if ( set )
				drawFlags |= DRAW_TEXT;
			else
				drawFlags &=( DRAW_TEXT ^ 0xffffffff );
		}

		virtual long getDrawText()
		{
			return drawFlags & DRAW_TEXT;
		}

		virtual void setDrawBars( bool set )
		{
			if ( set )
				drawFlags |= DRAW_BARS;
			else
				drawFlags &=( DRAW_BARS ^ 0xffffffff );
		}

		virtual long getDrawBars()
		{
			return drawFlags & DRAW_BARS;
		}

		virtual void setDrawBrackets( bool set )
		{
			if ( set )
				drawFlags |= DRAW_BRACKETS;
			else
				drawFlags &=( DRAW_BRACKETS ^ 0xffffffff );
		}

		virtual long getDrawBrackets()
		{
			return drawFlags & DRAW_BRACKETS;
		}

		virtual void setDrawColored( bool set )
		{
			if ( set )
				drawFlags |= DRAW_COLORED;
			else
				drawFlags &=( DRAW_COLORED ^ 0xffffffff );
		}

		virtual long getDrawColored()
		{
			return drawFlags & DRAW_COLORED;
		}
		
		virtual void setObscured (bool set) {
			if (set)
				flags |= OBJECT_FLAG_OBSCURED;
			else
				flags &= (OBJECT_FLAG_OBSCURED ^ 0xFFFFFFFF);
		}

		virtual bool getObscured (void) {
			return ((flags & OBJECT_FLAG_OBSCURED) != 0);
		}

		virtual bool isTargeted (void) {
			return(getTargeted());
		}

		virtual void setExists (bool set) {
			if (set)
				flags |= OBJECT_FLAG_EXISTS;
			else
				flags &= (OBJECT_FLAG_EXISTS ^ 0xFFFFFFFF);
		}

		virtual bool getExists(void) {
			return ((flags & OBJECT_FLAG_EXISTS) != 0);
		}

		virtual void setAwake (bool set) {
			if (set)
				flags |= OBJECT_FLAG_AWAKE;
			else
				flags &= (OBJECT_FLAG_AWAKE ^ 0xFFFFFFFF);
		}

		virtual bool getAwake (void) {
			return ((flags & OBJECT_FLAG_AWAKE) != 0);
		}

		virtual bool getExistsAndAwake (void) {
			return(getExists() && getAwake());
		}

		virtual void setUseMe (bool set) {
			if (set)
				flags |= OBJECT_FLAG_USEME;
			else
				flags &= (OBJECT_FLAG_USEME ^ 0xFFFFFFFF);
		}

		virtual bool getUseMe (void) {
			return ((flags & OBJECT_FLAG_USEME) != 0);
		}

		virtual void setCaptured (bool set) {
			if (set)
				flags |= OBJECT_FLAG_CAPTURED;
			else
				flags &= (OBJECT_FLAG_CAPTURED ^ 0xFFFFFFFF);
		}

		virtual bool getCaptured (void) {
			return ((flags & OBJECT_FLAG_CAPTURED) != 0);
		}

		virtual void clearCaptured(void) {
			setCaptured(false);
		}

		virtual bool isCaptured (void) {
			return(getCaptured());
		}
		
		virtual void setTonnage (float _tonnage) {
			tonnage = _tonnage;
		}

		virtual float getTonnage (void) {
			return(tonnage);
		}

		virtual void setCollisionFreeFromWID (GameObjectWatchID objWID) {
			collisionFreeFromWID = objWID;
		}

		virtual GameObjectWatchID getCollisionFreeFromWID (void) {
			return(collisionFreeFromWID);
		}

#ifdef USE_COLLISION
		virtual GameObjectHandle getCollisionFreeFromObject (void) {
			return(...);
		}
#endif

		virtual void setCollisionFreeTime (float time) {
			collisionFreeTime = time;
		}

		virtual float getCollisionFreeTime (void) {
			return(collisionFreeTime);
		}

		virtual void damageObject (float dmgAmount) {
			//damage += dmgAmount;
		}
		
		virtual void setExplDmg (float newDmg) {
			explDamage = newDmg;
		}
		
		virtual void setExplRad (float newRad) {
			explRadius = newRad;
		}
		
		virtual float getExplDmg (void) {
			return(explDamage);
		}

		virtual void setSensorRange (float range) {
		}

		virtual bool hasActiveProbe (void) {
			return(false);
		}

		virtual float getEcmRange (void) {
			return(0.0);
		}

		virtual bool hasNullSignature (void) {
			return(false);
		}

		virtual void setSalvage (SalvageItemPtr newSalvage) {
		}
				
		virtual SalvageItemPtr getSalvage (void) {
			return(NULL);
		}
		
		virtual long getWindowsVisible (void) {
			return(windowsVisible);
		}
		
		virtual long getCaptureBlocker (GameObjectPtr capturingMover, GameObjectPtr* blockerList = NULL);

		virtual long kill (void);

		virtual bool isMarine(void) {
			return(false);
		}

		virtual float getRefitPoints(void) {
			return(0.0);
		}

		virtual bool burnRefitPoints(float pointsToBurn) {
			return(false);
		}

		virtual float getRecoverPoints(void) {
			return(0.0);
		}

		virtual bool burnRecoverPoints(float pointsToBurn) {
			return(false);
		}

		virtual long getCurCV (void) {
			return(curCV);
		}

		virtual long getMaxCV (void) {
			return(maxCV);
		}
		
		virtual void setCurCV (long newCV) {
			curCV = newCV;
		}

		virtual long getThreatRating (void) {
			return(threatRating);
		}

		virtual void setThreatRating (short rating) {
			threatRating = rating;
		}

		virtual void incrementAttackers(void) {
			numAttackers++;
		}

		virtual void decrementAttackers(void) {
			Assert(numAttackers > 0, numAttackers, " GameObject.decrementAttackers: neg ");
			numAttackers--;
		}

		virtual long getNumAttackers(void) {
			return(numAttackers);
		}

		virtual bool onScreen (void);

		virtual MechClass getMechClass(void);

#if 0

		virtual void setSalvage (SalvageItemPtr newSalvage)
		{
			salvage = newSalvage;
		}
		
		virtual SalvageItemPtr getSalvage (void)
		{
			return salvage;
		}
	
#endif
		virtual bool isFriendly (GameObjectPtr obj);

		virtual bool isEnemy (GameObjectPtr obj);

		virtual bool isNeutral (GameObjectPtr obj);

		virtual bool isLinked (void)
		{
			return false;
		}

		virtual GameObjectPtr getParent (void)
		{
			return NULL;
		}

		virtual void setParentId (DWORD pId)
		{
			//Do Nothing.  Most of the time, this is OK!
		}

		virtual SensorSystem* getSensorSystem(){ return NULL; }

		static void setInitialize (bool setting) {
			initialize = setting;
		}

		virtual long getDescription(){ return -1; }
		
		virtual bool isOnGUI (void)
		{
			return false;
		}
		
		virtual void setOnGUI (bool onGui)
		{
		}
		
		virtual float getLOSFactor (void)
		{
			return 1.0f;
		}
		
		virtual bool isLookoutTower (void)
		{
			return false;
		}
		
		virtual void Save (PacketFilePtr file, long packetNum);

		void Load (GameObjectData *data);

		void CopyTo (GameObjectData *data);

		virtual void repairAll (void)
		{
		}
};

//---------------------------------------------------------------------------

#endif










