//---------------------------------------------------------------------------
//
//	ObjType.h -- File contains the Basic Game Object Type definition
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef OBJTYPE_H
#define OBJTYPE_H

//---------------------------------------------------------------------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DOBJTYPE_H
#include"dobjtype.h"
#endif

#ifndef DGAMEOBJ_H
#include"dgameobj.h"
#endif

#ifndef STUFF_HPP
#include<stuff/stuff.hpp>
#endif

#define MAX_NAME		25

//---------------------------------------------------------------------------
// Classes

class ObjectType {

	protected:
	
		ObjectTypeNumber		objTypeNum;				//What exactly am I?
		long					numUsers;				//How many people love me?
		long					objectTypeClass;		//What type am I?
		ObjectClass				objectClass;			//What object class am i?
		ObjectTypeNumber		destroyedObject;		//What I turn into when I die.
		ObjectTypeNumber		explosionObject;		//How I blow up
		bool					potentialContact;		//Can I can be a contact?
		char					*appearName;			//Base Name of appearance Files.
		float					extentRadius;			//Smallest sphere which will hold me.
		bool					keepMe;					//Do not EVER cache this objType out.
		long					iconNumber;				//my index into the big strip o' icons
		long					teamId;					//DEfault for this type
		unsigned char			subType;				//if building, what type of building? etc.

	public:

		void* operator new (size_t ourSize);
		void operator delete (void *us);
			
		void init (void) {
			objectClass = INVALID;
			objectTypeClass = -1;			//This is an invalid_object
			destroyedObject = -1;
			explosionObject = -1;
			potentialContact = false;

			extentRadius = 0;				//Nothing can hit me if this is zero.
			
			keepMe = false;
			
			iconNumber = -1;				//defaults to no icon

			appearName = NULL;
			subType = 0;
		}
		
		ObjectType (void) {
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);

		long init (FitIniFilePtr objFile);
		
		virtual ~ObjectType (void) {
			destroy();
		}
		
		virtual void destroy (void);
		
		virtual GameObjectPtr createInstance (void);
		
		void addUser (void) {
			numUsers++;
		}
		
		void removeUser (void) {
			numUsers--;
		}
		
		void noMoreUsers (void) {
			numUsers = 0;
		}

		bool inUse (void) {
			return (numUsers > 0);
		}

		bool lovable (void) {
			return keepMe;
		}
		
		void makeLovable (void) {
			keepMe = true;
		}
		
		ObjectTypeNumber whatAmI (void) {
			return(objTypeNum);
		}

		char * getAppearanceTypeName (void) 
		{
			return(appearName);
		}
			
		bool getPotentialContact (void) {
			return(potentialContact);
		}

		long getObjectTypeClass (void) {
			return(objectTypeClass);
		}

		ObjectClass getObjectClass (void) {
			return(objectClass);
		}

		ObjectTypeNumber getDestroyedObject (void) {
			return(destroyedObject);
		}
		
		ObjectTypeNumber getExplosionObject (void) {
			return(explosionObject);
		}
		
		float getExtentRadius (void) {
			return(extentRadius);
		}

		void setExtentRadius (float newRadius) {
			extentRadius = newRadius;
		}
		
		ObjectTypeNumber getObjTypeNum (void) {
			return(objTypeNum);
		}

		void setObjTypeNum (ObjectTypeNumber objTNum) {
			objTypeNum = objTNum;
		}

		void setIconNumber(long newNumber) {
			iconNumber = newNumber;
		}

		long getIconNumber(void) {
			return iconNumber;
		}
						
		long getTeamId (void) {
			return teamId;
		}

		void setSubType (unsigned char type) {
			subType = type;
		}

		unsigned char getSubType (void) {
			return(subType);
		}

		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);
		
		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);

		virtual float getBurnTime (void) {
			return(0.0);
		}

		void createExplosion (Stuff::Vector3D &position, float dmg = 0.0, float rad = 0.0);
};

//---------------------------------------------------------------------------

class ObjectTypeManager {

	public:

		long					numObjectTypes;
		ObjectTypePtr*			table;

		static UserHeapPtr		objectTypeCache;
		static UserHeapPtr		objectCache;
		static PacketFilePtr	objectFile;

		//--------------------------------------------------------
		// Following is done to maintain compatibility with MC1...
		static long				bridgeTypeHandle;
		static long				forestTypeHandle;
		static long				wallHeavyTypeHandle;
		static long				wallMediumTypeHandle;
		static long				wallLightTypeHandle;
			
	public:

		void init (void) {
		}
			
		ObjectTypeManager (void) {
			init();
		}

		long init (char* objectFileName, long objectTypeCacheSize, long objectCacheSize, long maxObjectTypes = 1024);
			
		void destroy (void);
							
		~ObjectTypeManager (void) {
			destroy();
		}

		void remove (long objTypeNum);

		void remove (ObjectTypePtr ptr);
			
		ObjectTypePtr load (ObjectTypeNumber objTypeNum, bool noCacheOut = true, bool forceLoad = false);

		ObjectTypePtr get (ObjectTypeNumber objTypeNum, bool loadIt = true);

		GameObjectPtr create (ObjectTypeNumber objTypeNum);
};

//---------------------------------------------------------------------------
#endif
