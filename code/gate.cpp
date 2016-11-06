//---------------------------------------------------------------------------
//
// gate.cpp -- File contains the Gate Object code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files
#ifndef GATE_H
#include"gate.h"
#endif

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef SOUNDS_H
#include"sounds.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#ifndef COLLSN_H
#include"collsn.h"
#endif

#ifndef TIMING_H
#include"timing.h"
#endif

#ifndef MECH_H
#include"mech.h"
#endif

#ifndef GVEHICL_H
#include"gvehicl.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#include "../resource.h"
#include<stdio.h>

extern unsigned long NextIdNumber;
extern float worldUnitsPerMeter;
extern bool useSound;
extern bool useOldProject;
extern bool drawExtents;

//---------------------------------------------------------------------------
// class GateType
//---------------------------------------------------------------------------
GameObjectPtr GateType::createInstance (void)
{
	GatePtr result = new Gate;
	if (!result)
		return NULL;

	result->init(true, this);

	return(result);
}

//---------------------------------------------------------------------------
void GateType::destroy (void)
{
	ObjectType::destroy();
}
		
//---------------------------------------------------------------------------
long GateType::init (FilePtr objFile, unsigned long fileSize)
{
	long result = 0;
	
	FitIniFile bldgFile;
	result = bldgFile.open(objFile,fileSize);
	if (result != NO_ERR)
		return(result);
	
	//------------------------------------------------------------------
	// Read in the data needed for the Gates
	result = bldgFile.seekBlock("GateData");
	if (result != NO_ERR)
		return(result);

	result = bldgFile.readIdULong("DmgLevel",dmgLevel);
	if (result != NO_ERR)
		return(result);

	bldgFile.readIdULong("BlownEffectId",blownEffectId);
	if (result != NO_ERR)
		blownEffectId = -1;
		
	bldgFile.readIdULong("NormalEffectId",normalEffectId);
	if (result != NO_ERR)
		normalEffectId = -1;
		
	bldgFile.readIdULong("DamageEffectId",damageEffectId);
	if (result != NO_ERR)
		damageEffectId = -1;

	result = bldgFile.readIdLong("BasePixelOffsetX",basePixelOffsetX);
	if (result != NO_ERR)
		basePixelOffsetX = 0;
	
	result = bldgFile.readIdLong("BasePixelOffsetY",basePixelOffsetY);
	if (result != NO_ERR)	
		basePixelOffsetY = 0;

	result = bldgFile.readIdFloat("ExplosionRadius",explRad);
	if (result != NO_ERR)
		explRad = 0.0;
		
	result = bldgFile.readIdFloat("ExplosionDamage",explDmg);
	if (result != NO_ERR)
		explDmg = 0.0;

	result = bldgFile.readIdFloat("OpenRadius",openRadius);
	if (result != NO_ERR)
		return(result);

	result = bldgFile.readIdFloat("LittleExtent",littleExtent);
	if (result != NO_ERR)
		littleExtent = 20.0;

	result = bldgFile.readIdLong ("BuildingName", gateTypeName);
	if (result != NO_ERR)
		gateTypeName = IDS_BLDOBJ_NAME;

	result = bldgFile.readIdLong( "BuildingDescription", buildingDescriptionID );
	if ( result != NO_ERR )
			buildingDescriptionID = -1;


	result = bldgFile.readIdBoolean("BlocksLineOfFire",blocksLineOfFire);
	if (result != NO_ERR)
		blocksLineOfFire = FALSE;

	//------------------------------------------------------------------
	// Initialize the base object Type from the current file.
	result = ObjectType::init(&bldgFile);
	return(result);
}
	
//---------------------------------------------------------------------------
bool GateType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider)
{
	//---------------------------------------------------------------
	// OK.  This handleCollision will open the gate if the
	// Mech or Vehicle is of the correct alignment and close enough.
	float closestRange = 119999999.99f;

	GatePtr gate = (GatePtr)collidee;
	//----------------------------------------------
	// On my side.  Close Enough to Open?
	switch (collider->getObjectClass())
	{
		case BATTLEMECH:
		case GROUNDVEHICLE:
		case ELEMENTAL:
		{
			Stuff::Vector3D colliderRangeV3;
			colliderRangeV3.Subtract(gate->getPosition(),collider->getPosition());
			float colliderRange = colliderRangeV3.x * colliderRangeV3.x + colliderRangeV3.y * colliderRangeV3.y;

			if ((collider->getTeamId() == gate->getTeamId()) || (gate->getTeamId() == -1))
			{
				if (collider->isDisabled() || collider->isDestroyed())
				{
					//--------------------------------------
					// Don't Open, Its Dead!!
				}
				else
				{
					if (colliderRange < (openRadius * openRadius))
						gate->reasonToOpen = true;
				}
			}

			//------------------------------------------------
			// Regardless of alignment, if I'm closest, mark me
			// So if gate closes and Im too close.  BOOM
			if ((colliderRange < closestRange) && (!collider->isDisabled() && !collider->isDestroyed()) && !((MoverPtr)collider)->isJumping())
			{
				gate->closestObject = (MoverPtr)collider;
				closestRange = colliderRange * colliderRange;
			}
		}
		break;
	}
	
	return(TRUE);
}

//---------------------------------------------------------------------------
bool GateType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider)
{
	return(FALSE);
}

//---------------------------------------------------------------------------
// class Gate
//---------------------------------------------------------------------------
bool Gate::isVisible (CameraPtr camera)
{
	//----------------------------------------------------------------------
	// This function is the meat and potatoes of the object cull system.
	// Its job is to determine if the object is on screen or not.
	// It does this by transforming the position for each active camera to
	// its screen coords and saving them.  It then checks each set of coords
	// to see if they are in the viewport of each camera.  Returned value
	// is number of windows that object can be seen in.
	bool isVisible = false; //land->getVertexScreenPos(blockNumber, vertexNumber, screenPos);
	if (appearance)
		isVisible = appearance->recalcBounds();

	if (isVisible) 
	{
		windowsVisible = turn;
		return(true);
	}
		
	return(false);
}

//---------------------------------------------------------------------------
long Gate::update (void)
{
	if (getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		setFlag(OBJECT_FLAG_JUSTCREATED, false);

		//--------------------------------------------------------
		// Figure out what the box should be for the tBuilding.
		appearance->update();
		appearance->recalcBounds();
		appearance->setHighlightColor(0x00000000);
		
		//-----------------------------------------------------
		// Check if ParentId is NOT 0xffffffff.
		// if not, find parent in ObjMgr and get its pointer.
		if ((parentId != 0xffffffff) && (parentId != 0))
		{
			parent = ObjectManager->findByCellPosition((parentId>>16),(parentId & 0x0000ffff))->getWatchID();
			ObjectManager->getByWatchID(parent)->setFlag(OBJECT_FLAG_CAPTURABLE, true);

			gosASSERT(parent != 0);
		}
	}
	
	for (long i = 0; i < numSubAreas0; i++) {
		GlobalMoveMap[0]->setAreaOwnerWID(subAreas0[i], getWatchID());
		GlobalMoveMap[1]->setAreaOwnerWID(subAreas1[i], getWatchID());
		if (status == OBJECT_STATUS_DESTROYED) {
			GlobalMoveMap[0]->setAreaTeamID(subAreas0[i], -1);
			GlobalMoveMap[1]->setAreaTeamID(subAreas1[i], -1);
			}
		else {
			GlobalMoveMap[0]->setAreaTeamID(subAreas0[i], teamId);
			GlobalMoveMap[1]->setAreaTeamID(subAreas1[i], teamId);
		}
	}

	//We can call update multiple times now since a gate will be updated
	//every frame regardless AND it could also be near where the camera is looking!	
	if (turn != updatedTurn)
	{
		updatedTurn = turn;
		
		//---------------------------------------
		// Handle Building captured.
		if (parent && 
			!ObjectManager->getByWatchID(parent)->isDisabled() && 
			!ObjectManager->getByWatchID(parent)->isDestroyed() && 
			(ObjectManager->getByWatchID(parent)->getTeamId() != getTeamId())) 
		{
			if ((ObjectManager->getByWatchID(parent)->getTeamId() != Team::home->getId()) && (turn > 5) && (getTeamId() != -1))
				soundSystem->playBettySample(BETTY_BUILDING_RECAPTURED);

			long parentTeamID = ObjectManager->getByWatchID(parent)->getTeamId();
			setTeamId(parentTeamID, false);
		}
	
		if ( parent && 
			 (!ObjectManager->getByWatchID(parent)->isDisabled()) && 
			 ObjectManager->getByWatchID(parent)->getSelected() )
		{
			setSelected( true );
		}
	
		//-----------------------------------------------
		// Handle parent disabled or destroyed or asleep
		if (parent && 
			(ObjectManager->getByWatchID(parent)->isDisabled() || 
			 ObjectManager->getByWatchID(parent)->isDestroyed() || 
			 !ObjectManager->getByWatchID(parent)->getAwake()))
		{
			//--------------------------------------------------
			// Lock gate CLOSED if its parent is dead
			setLockedClose();
		}
	
		openGate();		//Actually updates the gate states.
		
		closestObject = NULL;		//reset everytime we've successfully gotten through openGate.
									//was either not needed or now its dead.

		//MUST update appearance last.  Why?  If we have changed the gate state, the LOD is no longer valid.
		// Make damned sure the LOD updates!!!!!
		if (appearance)
		{
			bool inView = appearance->recalcBounds();

			// this has to be done before we set the object parameters.
			if ( parent && 
				 (!ObjectManager->getByWatchID(parent)->isDisabled()) && 
				 ObjectManager->getByWatchID(parent)->getTargeted() )
			{
				setTargeted( true );
			}
			
			appearance->setObjectParameters(position,((ObjectAppearance*)appearance)->rotation,drawFlags,getTeamId(),Team::getRelation(getTeamId(), Team::home->getId()));
	
			//------------------------------------------------
			// MUST update appearance every frame or animation goes HINKY!
			// Appearance update now checks inView and does NOT run transform math unless necessary!
			// Whoops!
			appearance->setInView(true);
			appearance->update();
			appearance->setInView(inView);
			
			if (inView)
			{
				windowsVisible = turn;
	
				float zPos = land->getTerrainElevation(position);
				position.z = zPos;
				setPosition(position);
			}
		}
	}
	
  	long result = true;
	return(result);
}

//---------------------------------------------------------------------------
void Gate::blowAnyOffendingObject (void)
{
	if (MPlayer && !MPlayer->isServer())
		return;

	if (closestObject)
	{
		Stuff::Vector3D objectRangeV3;
		objectRangeV3.Subtract(getPosition(),closestObject->getPosition());
		float objectRange = objectRangeV3.x * objectRangeV3.x + objectRangeV3.y * objectRangeV3.y;
		float littleRadius = (getLittleExtent() + closestObject->getObjectType()->getExtentRadius());
		littleRadius *= littleRadius;
		if (objectRange < littleRadius)
		{
			//-------------------------------------------------------------
			// Put about a million points of damage into the closestObject
			WeaponShotInfo shot;
			shot.init(0, -3, 250.00, 0, 0);

			for (long i=0;i<10;i++)
			{
				shot.hitLocation = closestObject->calcHitLocation(NULL,-1,ATTACKSOURCE_ARTILLERY,0);
				closestObject->handleWeaponHit(&shot, (MPlayer != NULL));
			}

			//--------------------
			// Gate must die too.
			shot.init(0, -3, ((GateTypePtr)(ObjectManager->getObjectType(typeHandle)))->getDamageLvl()+5, 0, 0);

			handleWeaponHit(&shot, (MPlayer != NULL));
		}
	}
}	

//---------------------------------------------------------------------------


void Gate::openGate (void)
{
	if (!isDestroyed())
	{
		if (lockedClose || !reasonToOpen)
		{
			long animState = appearance->getCurrentGestureId();
			if (animState == -1)	//Never Updated - Closed by default
			{
				appearance->setGesture(0);
				closed = true;
				opened = closing = opening = false;
			}
			else if (animState == 0)	//If closed, we're done.
			{
				appearance->setGesture(0);
				closed = true;
				opened = closing = opening = false;
			}
			else if (animState == 1)	//Gate is opening.
			{
				if (!appearance->getInTransition())	//If done opening, switch to closing.
				{
					appearance->setGesture(3);
					closing = true;
					closed = opened = opening = false;
					soundSystem->playDigitalSample(GATE_CLOSE,position,true);
				}
				else	//Wait for done opening.
				{
					opening = true;
					closed = closing = opened = false;
				}
			}
			else if (animState == 2)	//Gate is Open.  Start it closing.
			{
				appearance->setGesture(3);
				soundSystem->playDigitalSample(GATE_CLOSE,position,true);
				
				closing = TRUE;
				closed = opening = opened = false;
			}
			else if (animState == 3)	//Gate is closing.
			{
				if (!appearance->getInTransition())
				{
					appearance->setGesture(0);
					closed = true;
					opened = closing = opening = false;
					
					blowAnyOffendingObject();		//Check if something is "inside" gate
													//If so, blow it and the gate.
				}
				else
				{
					closing = true;
					closed = opening = opened = false;
				}
			}
		}
		else if (lockedOpen || reasonToOpen)
		{
			long animState = appearance->getCurrentGestureId();
			if (animState == -1)	//Never Updated - Closed by default
			{
				appearance->setGesture(1);
				soundSystem->playDigitalSample(GATE_OPEN,position,true);
				opening = true;
				closed = closing = opened = false;
			}
			if (animState == 0)
			{
				appearance->setGesture(1);
				soundSystem->playDigitalSample(GATE_OPEN,position,true);
				opening = true;
				closed = closing = opened = false;
			}
			else if (animState == 1)
			{
				if (!appearance->getInTransition())
				{
					appearance->setGesture(2);
					opened = true;
					opening = closing = closed = false;
				}
				else
				{
					opening = true;
					closed = closing = opened = false;
				}
			}
			else if (animState == 2)
			{
				appearance->setGesture(2);
				opened = true;
				opening = closing = closed = false;
			}
			else if (animState == 3)
			{
				if (!appearance->getInTransition())
				{
					appearance->setGesture(1);
					soundSystem->playDigitalSample(GATE_OPEN,position,true);
					
					opening = true;
					closed = closing = opened = false;
				}
				else
				{
					closing = true;
					closed = opening = opened = false;
				}

				//blowAnyOffendingObject();		//Check if something is "inside" gate
												//If so, blow it and the gate.
												//NO.  If locked Open, NEVER blow anyone!
			}
		}

		if (opened && (turn >3))
		{
			//-----------------------------------------------------------------------------------------
			// MARK True when open so that the side pieces of the gate are still IMPASSABLE!!
			// First mark ENTIRE block PASSABLE.
			// THEN Use Height to mark out areas which are IMPASSABLE!
			
			//MUST use appearance here!!!!  Glenn's other way does NOT assume animation!
			
			openSubAreas();
			lastMarkedOpen = true;
		}
		else if (!opened && (turn > 3))
		{
			//----------------------------------------------------
			// Any other state but Opened is Closed!!
			// Regardless of what kind of gate, gate should be able to mark passable/impassable
			// Using same method as editor uses to mark.  Gates are rotated now so overlay is set to?
			closeSubAreas();
			appearance->markLOS();						//ONLY need to re-mark here.  Only need to clear when we open!
			lastMarkedOpen = false;
		}
		
		//GlobalMoveMap[0]->clearPathExistsTable();
		//GlobalMoveMap[1]->clearPathExistsTable();

		reasonToOpen = false;		//Always reset the electric eye
	}
}	

//---------------------------------------------------------------------------
long Gate::setTeamId (long _teamId, bool setup) 
{
	if (MPlayer)
	{
		teamId = _teamId;
	}
	else
	{
		if (_teamId == 2)		//Allies
			teamId = 0;			//Same as PlayerTeam.
		else if (_teamId > 2)
			teamId = 1;			//Not ally.  ENEMY!!
		else
			teamId = _teamId;	//Otherwise we were set to either -1, 0 or 1.
	}

	static unsigned long highLight[8] = {0x00007f00, 0x007f0000,
										  0x0000007f, 0x0000007f,
										  0x0000007f, 0x0000007f,
										  0x0000007f, 0x0000007f};

	if (turn > 10)
		appearance->flashBuilding(5.0,0.5,highLight[teamId]);

	setSubAreasTeamId(_teamId);
 		
	return(NO_ERR);
}

//---------------------------------------------------------------------------
void Gate::lightOnFire (float timeToBurn)
{
	//Gates have never burned
}

//---------------------------------------------------------------------------
void Gate::render (void)
{
	if (appearance->canBeSeen())
	{
		//--------------------------------------
		if (getDrawBars())
		{
			GateTypePtr type = (GateTypePtr)getObjectType();
			float barStatus = 1.0;
			float totalDmgLvl = type->getDamageLvl();
			if (totalDmgLvl > 0.0)
				barStatus -= getDamage() / totalDmgLvl;
			
			if (barStatus < 0.0)
				barStatus = 0.0;

			DWORD color = 0xff7f7f7f;
			if ((teamId > -1) && (teamId < 8) && getTeam()) {
				if (getTeam()->isFriendly(Team::home))
					color = SB_GREEN;
				else if (getTeam()->isEnemy(Team::home))
					color = SB_RED;
				else
					color = SB_BLUE;
			}
				
			appearance->setBarColor(color);
			appearance->setBarStatus(barStatus);
		}

		if (((GateTypePtr)getObjectType())->gateTypeName < IDS_MC2_STRING_START)
		{
			appearance->setObjectNameId(((GateTypePtr)getObjectType())->gateTypeName + IDS_MC2_STRING_START);
		}
		else
		{
			appearance->setObjectNameId(((GateTypePtr)getObjectType())->gateTypeName);
		}


		windowsVisible = turn;
		appearance->setVisibility(true,true);
		appearance->render();
	}
	
	setSelected(false);		//ALWAYS reset the selected flags.  GUI needs this to work!
	setTargeted( false );	//ALWAYS do it here, too!  Otherwise things may draw FUNNY!
}

//---------------------------------------------------------------------------
void Gate::destroy (void)
{
	//-----------------------------------------------------
	// This will free any memory the Building is using.
	if (appearance)
	{
		delete appearance;
		appearance = NULL;
	}
}

//---------------------------------------------------------------------------
void Gate::init (bool create, ObjectTypePtr _type)
{
	//-------------------------------------------
	// Initialize the Building Appearance here.
	GameObject::init(create, _type);

	setFlag(OBJECT_FLAG_JUSTCREATED, true);

	//-------------------------------------------------------------
	// The appearance is initialized here using data from the type
	// Need an MLR appearance class
	char *appearName = _type->getAppearanceTypeName();

	//--------------------------------------------------------------
	// New code!!!
	// We need to append the sprite type to the appearance num now.
	// The MechEdit tool does not assume a sprite type, nor should it.
	// MechCmdr2 features much simpler objects which only use 1 type of sprite!
	long appearanceType = (BLDG_TYPE << 24);

	AppearanceTypePtr buildingAppearanceType = NULL;
	if (!appearName)
	{
		//------------------------------------------------------
		// LOAD a dummy appearance until real ones are available
		// for this building!
		appearanceType = (BLDG_TYPE << 24);
		buildingAppearanceType = appearanceTypeList->getAppearance(appearanceType,"TESTBLDG");
	}
	else
	{
		buildingAppearanceType = appearanceTypeList->getAppearance(appearanceType,appearName);
		if (!buildingAppearanceType)
		{
			char msg[1024];
			sprintf(msg,"No Building Appearance Named %s",appearName);
			Fatal(0,msg);
		}
	}
	  
   	appearance = new BldgAppearance;
	gosASSERT(appearance != NULL);

	//--------------------------------------------------------------
	// The only appearance type for buildings is MLR_APPEARANCE.
	gosASSERT(buildingAppearanceType->getAppearanceClass() == BLDG_TYPE);
	
	appearance->init((BldgAppearanceType*)buildingAppearanceType, (GameObjectPtr)this);

	objectClass = GATE;
	reasonToOpen = false;		//Always reset the electric eye
	lastMarkedOpen = false;
	
	if (((GateTypePtr)_type)->openRadius != 0.0)
		_type->setExtentRadius(((GateTypePtr)_type)->openRadius);

	if (_type->getExtentRadius() > 0.0)
		setFlag(OBJECT_FLAG_TANGIBLE, true);
	
	explDamage = ((GateTypePtr)_type)->explDmg;
	explRadius = ((GateTypePtr)_type)->explRad;
}	

//---------------------------------------------------------------------------
void Gate::setDamage (float newDamage) 
{
	damage = newDamage;

	GateTypePtr type = (GateTypePtr)getObjectType();
	if (damage >= type->getDamageLvl())
	{
		setStatus(OBJECT_STATUS_DESTROYED);
		appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
	}
}
 
//---------------------------------------------------------------------------
long Gate::handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk) 
{
	if (!shotInfo)
		return(NO_ERR);

	if (addMultiplayChunk)
		MPlayer->addWeaponHitChunk(this, shotInfo);

	damage = getDamage() + shotInfo->damage;
			
	GateTypePtr type = (GateTypePtr)getObjectType();
	if (damage >= type->getDamageLvl() && !isDestroyed())	
	{
		destroyGate();
	}
		
	return(NO_ERR);
}

//---------------------------------------------------------------------------
void Gate::destroyGate(void)
{
	justDestroyed = false;
		
	if (!getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		//-----------------------------------------------------------------------------------------
		// First, mark every occupied cell PASSABLE for original shape.
		// then, load the destroyed shape and use it to mark impassable!
		markMoveMap(true);
		GlobalMoveMap[0]->clearPathExistsTable();
		GlobalMoveMap[1]->clearPathExistsTable();
		//----------------------------------------------------------
		// Unmark these cells as gate cells, so it'll be passable...
		short* curCoord = cellsCovered;
		for (long i = 0; i < numCellsCovered; i++) {
			long r = *curCoord++;
			long c = *curCoord++;
			GameMap->setGate(r, c, false);
		}
	}

	if (appearance)
	{
		appearance->markLOS(true);			//Need to clear out before we change to the destroyed shape!!
		appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
		appearance->recalcBounds();
		appearance->update();
	}

	if (!getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		//-----------------------------------------------------------------------------------------
		// Regardless of what kind of gate, gate should be able to mark passable/impassable
		// Using same method as editor uses to mark.  Gates are rotated now so overlay is set to?
		openSubAreas();
		appearance->markLOS();			//Now mark LOS for the destroyed shape!
	}

	opened = true;
	closed = closing = opening = false;
	
	lockedOpen = true;
	justDestroyed = false;
	setFlag(OBJECT_FLAG_TANGIBLE, false);

	//------------------------------------------------------
	// Gate is dead.  You may no longer collide with it.
	setStatus(OBJECT_STATUS_DESTROYED);
	if (!getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		//-----------------------------------------------------
		// Now, blow the building up using its type->explosion
		// (this won't do anything if a building type doesn't have an explosion)
		ObjectManager->createExplosion(BUILDING_EXPLOSION_ID,this,position,explDamage,explRadius);
	}
}

//---------------------------------------------------------------------------
float Gate::getLittleExtent (void)
{
	return (((GateTypePtr)(ObjectManager->getObjectType(typeHandle)))->littleExtent);
}

//---------------------------------------------------------------------------
bool Gate::isLinked (void)
{
	return (parent != 0);
}

//---------------------------------------------------------------------------
GameObjectPtr Gate::getParent (void)
{
	return (ObjectManager->getByWatchID(parent));
}

//---------------------------------------------------------------------------
void Gate::setParentId (DWORD pId)
{
	parentId = pId;
}

//---------------------------------------------------------------------------
TeamPtr Gate::getTeam (void) {

	if (teamId == -1)
		return(NULL);
	return(Team::teams[teamId]);
}

//***************************************************************************
void Gate::Save (PacketFilePtr file, long packetNum)
{
	GateData data;
	CopyTo(&data);

	//PacketNum incremented in ObjectManager!!
	file->writePacket(packetNum,(MemoryPtr)&data,sizeof(GateData),STORAGE_TYPE_ZLIB);
}

//***************************************************************************
void Gate::CopyTo (GateData *data)
{																	   
	data->teamId = teamId;

	data->lockedOpen = lockedOpen;
	data->lockedClose = lockedClose;
	data->reasonToOpen = reasonToOpen;
	data->opened = opened;
	data->opening = opening;
	data->closed = closed;
	data->closing = closing;
	data->justDestroyed = justDestroyed;

	data->lastMarkedOpen = lastMarkedOpen;

	data->closestObject = closestObject;

	data->parentId = parentId;
	data->parent = parent;
	data->buildingDescriptionID = buildingDescriptionID;

	data->updatedTurn = updatedTurn;

	TerrainObject::CopyTo(dynamic_cast<TerrainObjectData *>(data));
}

//---------------------------------------------------------------------------
void Gate::Load (GateData *data)
{
	TerrainObject::Load(dynamic_cast<TerrainObjectData *>(data));

	teamId = data->teamId;

	lockedOpen = data->lockedOpen;
	lockedClose = data->lockedClose;
	reasonToOpen = data->reasonToOpen;
	opened = data->opened;
	opening = data->opening;
	closed = data->closed;
	closing = data->closing;
	justDestroyed = data->justDestroyed;

	lastMarkedOpen = data->lastMarkedOpen;

	closestObject = data->closestObject;

	parentId = data->parentId;
	parent = data->parent;
	buildingDescriptionID = data->buildingDescriptionID;

	updatedTurn = 0;
}

//---------------------------------------------------------------------------

