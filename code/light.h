//***************************************************************************
//
//	Light.h -- File contains the Explosion Object Definition
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef LIGHT_H
#define LIGHT_H

//---------------------------------------------------------------------------

#ifndef DCARNAGE_H
#include"dcarnage.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef OBJTYPE_H
#include"objtype.h"
#endif

//---------------------------------------------------------------------------
/*
#define NO_APPEARANCE_TYPE_FOR_EXPL		0xDCDC0003
#define NO_APPEARANCE_FOR_EXPL			0xDCDC0004
#define APPEARANCE_NOT_VFX_XPL			0xDCDC0005
*/
//---------------------------------------------------------------------------

class LightType : public ObjectType {

	public:

		bool		oneShotFlag;
		float		altitudeOffset;
	
	public:

		void init (void) {
			ObjectType::init();
		}
		
		LightType (void) {
			init();
		}
		
		virtual long init (FilePtr objFile, unsigned long fileSize);

		long init (FitIniFilePtr objFile);
		
		~LightType (void) {
			destroy();
		}
		
		virtual void destroy (void);
		
		virtual GameObjectPtr createInstance (void);
		
		virtual bool handleCollision (GameObjectPtr collidee, GameObjectPtr collider);

		virtual bool handleDestruction (GameObjectPtr collidee, GameObjectPtr collider);
};

//---------------------------------------------------------------------------

class Light : public GameObject {

	public:

		virtual void init (bool create);

	   	Light (void) : GameObject() {
			init(true);
		}

		~Light (void) {
			destroy();
		}

		virtual void destroy (void);
				
		virtual long update (void);

		virtual void render (void);
		
		virtual void init (bool create, ObjectTypePtr _type);

		virtual long kill (void) {
			return(NO_ERR);
		}
};

//***************************************************************************

#endif




