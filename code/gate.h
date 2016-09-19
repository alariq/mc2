//---------------------------------------------------------------------------
//
// gate.h -- File contains the Gate Object Class
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GATE_H
#define GATE_H
//---------------------------------------------------------------------------
// Include Files

#ifndef DGATE_H
#include"dgate.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

#ifndef TERROBJ_H
#include"terrobj.h"
#endif

#ifndef DMOVER_H
#include"dmover.h"
#endif
					
//---------------------------------------------------------------------------
// Macro Definitions

//---------------------------------------------------------------------------
class GateType : public ObjectType
{
	//Data Members
	//-------------
	protected:
		unsigned long	dmgLevel;

	public:
		unsigned long	blownEffectId;
		unsigned long	normalEffectId;
		unsigned long	damageEffectId;
	
		float			baseTonnage;
		
		long			basePixelOffsetX;
		long			basePixelOffsetY;
		
		float			explDmg;
		float			explRad;

		float			openRadius;
		
		float			littleExtent;
				
		long			gateTypeName;

		bool			blocksLineOfFire;

		long			buildingDescriptionID;

	//Member Functions
	//-----------------
	public:

		void init (void)
		{
			ObjectType::init();
			
			dmgLevel = 0;

			blownEffectId = 0xFFFFFFFF;
			normalEffectId = 0xFFFFFFFF;
			damageEffectId = 0xFFFFFFFF;
		
			explDmg = explRad = 0.0;
			baseTonnage = 0.0;
			
			gateTypeName = 0;
			
			objectClass = GATE;
		}
		
		GateType (void)
		{
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);
		long init (FitIniFilePtr objFile);
		
		~GateType (void)
		{
			destroy();
		}
		
		long getDamageLvl (void)
		{
			return dmgLevel;
		}
			
		virtual void destroy (void);
		
		virtual GameObjectPtr createInstance (void);
		
		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);
		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

//---------------------------------------------------------------------------
typedef struct _GateData : public TerrainObjectData
{
	char					teamId;

	bool					lockedOpen;
	bool					lockedClose;
	bool					reasonToOpen;
	bool					opened;
	bool					opening;
	bool					closed;
	bool					closing;
	bool					justDestroyed;

	bool					lastMarkedOpen;

	MoverPtr				closestObject;

	DWORD					parentId;
	GameObjectWatchID		parent;
	long					buildingDescriptionID;

	long					updatedTurn;
} GateData;

class Gate : public TerrainObject
{
	//Data Members
	//-------------
		public:
			char					teamId;
			
			bool					lockedOpen;
			bool					lockedClose;
			bool					reasonToOpen;
			bool					opened;
			bool					opening;
			bool					closed;
			bool					closing;
			bool					justDestroyed;

			bool					lastMarkedOpen;

			MoverPtr				closestObject;
			
			DWORD					parentId;
			GameObjectWatchID		parent;
			long					buildingDescriptionID;

			long					updatedTurn;

	//Member Functions
	//-----------------
		public:

		void init (bool create)
		{
		}

	   	Gate (void) : TerrainObject()
		{
			init (true);
			reasonToOpen = TRUE;
			
			lockedClose = FALSE;
			lockedOpen = FALSE;
			closed = TRUE;
			closing = opening = opened = FALSE;
			justDestroyed = FALSE;
			lastMarkedOpen = false;

			closestObject = NULL;
			
			parentId = 0xffffffff;
			parent = 0;

			updatedTurn = -1;
		}

		~Gate (void)
		{
			destroy();
		}

		virtual void destroy (void);
		
		virtual long update (void);
		virtual void render (void);
		
		virtual void init (bool create, ObjectTypePtr _type);

		virtual long handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk = FALSE);

		virtual long setTeamId (long _teamId, bool setup);
		
		virtual long getTeamId (void) {
			return(teamId);
		}

		TeamPtr getTeam (void); 

		void lightOnFire (float timeToBurn);
			
		virtual float getDestructLevel (void)
		{
			return ((GateTypePtr)getObjectType())->getDamageLvl() - damage;
		}

		virtual long kill (void)
		{
			//Do nothing for now.  Later, Buildings may do something.
			return NO_ERR;
		}

		bool isVisible (CameraPtr camera);

		void blowAnyOffendingObject (void);

		virtual bool isBuilding(void)
		{
			return (TRUE);
		}

		virtual void getBlockAndVertexNumber (long &blockNum, long &vertexNum)
		{
			blockNum = blockNumber;
			vertexNum = vertexNumber;
		}
		
		void openGate (void);
		
		void setLockedOpen()
		{
			lockedOpen = TRUE;
			lockedClose = FALSE;
		}
		
		void setLockedClose()
		{
			lockedOpen = FALSE;
			lockedClose = TRUE;
		}

		void releaseLocks()
		{
			lockedOpen = FALSE;
			lockedClose = FALSE;
		}

		float getLittleExtent (void);

		void destroyGate(void);
		
		virtual bool isLinked (void);

		virtual GameObjectPtr getParent (void);

		virtual void setParentId (DWORD pId);

		virtual long getDescription(){ return ((GateType*)getObjectType())->buildingDescriptionID; }
		
		virtual void setDamage (float newDamage);

		virtual void Save (PacketFilePtr file, long packetNum);

		void CopyTo (GateData *data);

		void Load (GateData *data);
};

//---------------------------------------------------------------------------
#endif




