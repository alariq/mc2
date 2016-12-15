//---------------------------------------------------------------------------
//
//	bldng.cpp -- File contains the Building Object code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCLIB_h
#include"mclib.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef CARNAGE_H
#include"carnage.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#ifndef COLLSN_H
#include"collsn.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifdef USE_ELEMENTALS
#ifndef ELEMNTL_H
#include"elemntl.h"
#endif
#endif

#ifdef USE_TACMAP
#ifndef TERRMAP_H
#include"terrmap.h"
#endif
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
#endif

#ifndef GAMELOG_H
#include"gamelog.h"
#endif

#ifndef LOGISTICSDATA_H
#include"logisticsdata.h"
#endif

#include "../resource.h"

#define BLIP_FRAME_RATE		0.067
//extern unsigned long NextIdNumber;
extern float worldUnitsPerMeter;
bool drawExtents = false;
extern bool somethingOnFire;
extern bool	useOldProject;
extern bool useSound;
extern char lastName[];

extern GameLog* CombatLog;

extern float scenarioTime;

//#ifndef SOUNDS_H
#include"sounds.h"
//#endif

extern long DefaultPilotId;
extern char marineProfileName[];
//extern ObjectMapPtr GameObjectMap;

extern long NumMarines;

extern bool GeneralAlarm;

inline float agsqrt( float _a, float _b )
{
	return sqrt(_a*_a + _b*_b);
}

DWORD WallType[18] = 
{
	26,125,152,154,156,158,160,161,162,202,204,206,231,232,233,286,287,288
};

#define BRIDGE_TYPE		607

//---------------------------------------------------------------------------
// class BuildingType
//---------------------------------------------------------------------------

void BuildingType::init (void) {

	objectTypeClass = BUILDING_TYPE; //any reason to record TREEBUILDING_TYPE?
	objectClass = BUILDING;
	damageLevel = 0.0;
	sensorRange = -1.0;
	teamId = -1;
	explDmg = explRad = 0.0;
	baseTonnage = 0.0;
	buildingTypeName = 0;
	buildingDescriptionID = -1;
	startBR = 0;
	numMarines = 0;
	canRefit = false;
	mechBay = false;
}

//---------------------------------------------------------------------------

GameObjectPtr BuildingType::createInstance (void) {

	BuildingPtr result = new Building;
	if (!result)
		return(NULL);

	result->init(true, this);
	//result->setIdNumber(NextIdNumber++);
	
	return(result);
}

//---------------------------------------------------------------------------

void BuildingType::destroy (void) 
{
	ObjectType::destroy();
}
		
//---------------------------------------------------------------------------

long BuildingType::init (FilePtr objFile, unsigned long fileSize) {

	long result = 0;
	
	FitIniFile bldgFile;
	result = bldgFile.open(objFile, fileSize);
	if (result != NO_ERR)
		return(result);

	//-------------------------------------------------------------------
	// Since this object type handles MC1's Building and TreeBuilding
	// object types, we need to check for both. Basically, "TreeBuilding"
	// stands for animated building. Ultimately, would be nice to get
	// rid of this misleading legacy object type...
	result = bldgFile.seekBlock("TreeData");
	if (result != NO_ERR) 
	{
		result = bldgFile.seekBlock("BuildingData");
		if (result != NO_ERR)
			return(result);
	}

	unsigned long dmgLevel;
	result = bldgFile.readIdULong("DmgLevel",dmgLevel);
	if (result != NO_ERR)
		return(result);
	damageLevel = (float)dmgLevel;
		
	result = bldgFile.readIdBoolean("CanRefit", canRefit);
	if (result != NO_ERR)
		canRefit = false;

	if (canRefit) 
	{
		result = bldgFile.readIdBoolean("MechBay", mechBay);
		if (result != NO_ERR)
			mechBay = false;
	}

	result = bldgFile.readIdFloat("ExplosionRadius", explRad);
	if (result != NO_ERR)
		explRad = 0.0;
		
	result = bldgFile.readIdFloat("ExplosionDamage", explDmg);
	if (result != NO_ERR)
		explDmg = 0.0;

	result = bldgFile.readIdFloat("Tonnage", baseTonnage);
	if (result != NO_ERR)
		baseTonnage = 20;

	result = bldgFile.readIdLong("BattleRating", startBR);
	if (result != NO_ERR)
		startBR = 20;
	
	result = bldgFile.readIdLong("NumMarines", numMarines);
	if (result != NO_ERR)
		numMarines = 0;
	
	float realExtent = 0.0;
	result = bldgFile.readIdFloat("ExtentRadius", realExtent);
	if (result != NO_ERR)
		realExtent = -1.0;

	result = bldgFile.readIdULong("ActivityEffectID", activityEffectId);
	if (result != NO_ERR)
		activityEffectId = 0xffffffff;
		
 	//----------------------------
	// Init sensor-related data...
	result = bldgFile.readIdLong("TeamID", teamId);
	if (result != NO_ERR)
		teamId = -1;

	result = bldgFile.readIdFloat("SensorRange", sensorRange);
	if (result != NO_ERR)
		sensorRange = -1.0;

	result = bldgFile.readIdLong ("BuildingName", buildingTypeName);
	if (result != NO_ERR)
		buildingTypeName = IDS_BLDOBJ_NAME;

	result = bldgFile.readIdLong( "BuildingDescription", buildingDescriptionID );
	if ( result != NO_ERR )
		buildingDescriptionID = -1;


	result = bldgFile.readIdLong( "BuildingDescription", buildingDescriptionID );
	if ( result != NO_ERR )
		buildingDescriptionID = -1;

	result = bldgFile.readIdLong( "ResourcePoints", resourcePoints );
	if ( result != NO_ERR )
		resourcePoints = 0;

	result = bldgFile.readIdBoolean("ImpassableWhenDestroyed",marksImpassableWhenDestroyed);
	if (result != NO_ERR)
		marksImpassableWhenDestroyed = true;

	result = bldgFile.readIdBoolean("Capturable",capturable);
	if (result != NO_ERR)
		capturable = false;

	result = bldgFile.readIdBoolean("IsPowerSource",powerSource);
	if (result != NO_ERR)
		powerSource = false;

	result = bldgFile.readIdFloat("LookoutTowerRange",lookoutTowerRange);
	if (result != NO_ERR)
		lookoutTowerRange = 0.0f;

	result = bldgFile.readIdFloat("PerimeterAlarmRange",perimeterAlarmRange);
	if (result != NO_ERR)
		perimeterAlarmRange = 0.0f;

	result = bldgFile.readIdFloat("PerimeterAlarmTimer",perimeterAlarmTimer);
	if (result != NO_ERR)
		perimeterAlarmTimer = 0.0f;

	result = ObjectType::init(&bldgFile);
	extentRadius = realExtent;
	if (perimeterAlarmRange > 0.0f)
		extentRadius = perimeterAlarmRange;

	//--------------------------------------------------------------------------------
	// HACK!!!!! Must fix this for localization purposes--should be in the object type
	// data (can't look in the string!) --gd
	long objectNameID = buildingTypeName;
	if (objectNameID < IDS_MC2_STRING_START)
		objectNameID += IDS_MC2_STRING_START;

	if (objectNameID != -1)	
	{
		char s[255];
		cLoadString(objectNameID, s, 254);
	}

	for (long i=0;i<18;i++)
	{
		if (objTypeNum == WallType[i])
		{
			setSubType(BUILDING_SUBTYPE_WALL);
			break;
		}
	}

	if (objTypeNum == BRIDGE_TYPE)
		setSubType(BUILDING_SUBTYPE_LANDBRIDGE);

	return(result);
}
	
//---------------------------------------------------------------------------

bool BuildingType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider) {

	if (MPlayer && !MPlayer->isServer())
		return(true);

	//-------------------------------------------------------
	// The Building ceases to exist when its effect is done.
	// always return FALSE or the collision will make it
	// go away!  We may want to play a sound effect here.
	switch (collider->getObjectClass()) {
		case BATTLEMECH:
		case GROUNDVEHICLE:
		case ELEMENTAL:
		{
			if ((perimeterAlarmRange > 0.0f) &&
				(perimeterAlarmTimer > 0.0f) &&
				(collider->getTeamId() != collidee->getTeamId()))
			{
				((BuildingPtr)collidee)->moverInProximity = true;
				return true;		//Don't blow the perimeter alarm up!!
			}
		}
		case TERRN:
		case EXPLOSION:
		case FIRE: {
			WeaponShotInfo shot;
			shot.init(0, -1, 10, 0, 0);
			if (collider->getCollisionFreeTime() < scenarioTime)
				return(true);
			collidee->handleWeaponHit(&shot, (MPlayer != NULL));
			}
			break;
	}
	
	return(true);
}

//---------------------------------------------------------------------------

bool BuildingType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider) {

	return(false);
}

//---------------------------------------------------------------------------
// class Building
//---------------------------------------------------------------------------

bool Building::isVisible (void) {

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

	if (isVisible) {
		windowsVisible = turn;
		return(true);
	}
		
	return(false);
}

//---------------------------------------------------------------------------

void Building::openFootPrint (void) {
}

//---------------------------------------------------------------------------

void Building::closeFootPrint (void) {
}

//---------------------------------------------------------------------------
long Building::updateAnimations (void)
{
	//---------------------------------------------
	// Animate Sensor Towers first.
	/*
	if (sensorSystem != NULL)
	{
		long animState = appearance->getCurrentGestureId();
		if (sensorSystem->numContacts)
		{
			switch (animState)
			{
				case -1:		//NOT UPDATED YET. SWITCH TO ZERO
					if (!appearance->getInTransition())
					{
						appearance->setGesture(0);
					}
					break;
					
				case 2:			//Just Animating.  Do NOTHING!
					break;
					
				case 0:			//Not triggered yet.  Switch to 1
					appearance->setGesture(1);
					break;
					
				case 1:			//triggered, when fully open switch to 2
					if (!appearance->getInTransition())
					{
						appearance->setGesture(2);
					}
					break;
					
				case 3:			//Closing.  Wait until closed and then switch to 1.
					if (!appearance->getInTransition())
					{
						appearance->setGesture(1);
					}
					break;
			}
		}
		else	//This will be handled by sensor system going disabled if building destroyed OR parent destroyed!
		{
			switch (animState)
			{
				case -1:		//NOT UPDATED YET. SWITCH TO ZERO
					if (!appearance->getInTransition())
					{
						appearance->setGesture(0);
					}
					break;
					
				case 2:			//Just Animating.  Wait until one loop done, then trigger closing
					if (!appearance->getInTransition())
					{
						appearance->setGesture(3);
					}
					break;
					
				case 0:			//Not triggered yet.  DO NOTHING!
					break;
					
				case 1:			//triggered, when fully open switch to 3 to close it.
					if (!appearance->getInTransition())
					{
						appearance->setGesture(3);
					}
					break;
					
				case 3:		//Closing to closed.  When Closed, DO NOTHING
					break;		
			}
		}
	}
	else*/
	if (((BuildingTypePtr)getObjectType())->resourcePoints)
	{
		if (!MPlayer)
		{
			//We are a resource Points Building.
			//OPEN when we have been CAPTURED!!
			long animState = appearance->getCurrentGestureId();
			if (!teamId)	//We've been Captured
			{
				switch (animState)
				{
					case -1:		//NOT UPDATED YET. SWITCH TO ZERO
						if (!appearance->getInTransition())
						{
							appearance->setGesture(0);
						}
						break;

					case 2:			//Just Animating.  Do NOTHING!
						break;

					case 0:			//Not triggered yet.  Switch to 1
						appearance->setGesture(1);
						break;

					case 1:			//triggered, when fully open switch to 2
						if (!appearance->getInTransition())
						{
							appearance->setGesture(2);
						}
						break;

					case 3:			//Closing.  Wait until closed and then switch to 1.
						if (!appearance->getInTransition())
						{
							appearance->setGesture(1);
						}
						break;
				}
			}
			else
			{
				//Stay Closed
				switch (animState)
				{
					case -1:		//NOT UPDATED YET. SWITCH TO ZERO
						if (!appearance->getInTransition())
						{
							appearance->setGesture(0);
						}
						break;

					case 2:			//Just Animating.  Do NOTHING!
						break;

					case 0:			//Do Nothing
						break;

					case 1:			//triggered, when fully open switch to 3
						if (!appearance->getInTransition())
						{
							appearance->setGesture(3);
						}
						break;

					case 3:			//Closing.  Wait until closed and then switch to 1.
						if (!appearance->getInTransition())
						{
							appearance->setGesture(0);
						}
						break;
				}
			}
		}
	}
	else if (((BuildingTypePtr)getObjectType())->mechBay)
	{
		if (!MPlayer)
		{
			//We are a Mech Repair Bay Building.
			// Open to the correct place based on our RepairBuddyWID
			long animState = appearance->getCurrentGestureId();
			if (!refitBuddyWID)	//We're repairing no one.
			{
				switch (animState)
				{
					case -1:		//NOT UPDATED YET. SWITCH TO ZERO
						if (!appearance->getInTransition())
						{
							appearance->setGesture(0);
						}
						break;

					case 1:			//We were repairing someone.  Switch to doing nothing!
						if (!appearance->getInTransition())
						{
							appearance->setGesture(5);
						}
						break;

					case 2:			//We were repairing someone.  Switch to doing nothing! 
						if (!appearance->getInTransition())
						{
							appearance->setGesture(6);
						}
						break;

					case 3:			//We were repairing someone.  Switch to doing nothing! 
						if (!appearance->getInTransition())
						{
							appearance->setGesture(7);
						}
						break;

					case 4:			//We were repairing someone.  Switch to doing nothing! 
						if (!appearance->getInTransition())
						{
							appearance->setGesture(8);
						}
						break;

					case 5:			//We were repairing someone.  Switch to doing nothing!
					case 6:			//We were repairing someone.  Switch to doing nothing! 
					case 7:			//We were repairing someone.  Switch to doing nothing! 
					case 8:			//We were repairing someone.  Switch to doing nothing! 
						if (!appearance->getInTransition())
						{
							appearance->setGesture(0);
						}
						break;

					case 0:			//Do Nothing.  We aren't repairing anyone
						break;
				}
			}
			else
			{
				//We are repairing someone.
				switch (animState)
				{
					case -1:		//NOT UPDATED YET. SWITCH TO ZERO
						if (!appearance->getInTransition())
						{
							appearance->setGesture(0);
						}
						break;

					case 1:
					case 2:
					case 3:
					case 4:		//DO NOTHING!!  We are in position.
						{
							GameObjectPtr refitMech = ObjectManager->getByWatchID(refitBuddyWID);
							if (refitMech && (refitMech->getStatus() == OBJECT_STATUS_SHUTDOWN))
							{
								if (((BuildingTypePtr)getObjectType())->activityEffectId != 0xffffffff)
									appearance->startActivity(((BuildingTypePtr)getObjectType())->activityEffectId,true);
							}
							else
							{
								appearance->stopActivity();
							}
						}
						break;

					case 0:
						if (!appearance->getInTransition())
						{
							GameObjectPtr refitMech = ObjectManager->getByWatchID(refitBuddyWID);
							if (refitMech) {
								Stuff::Vector3D distance;
								distance.Subtract(refitMech->getPosition(),getPosition());
								float dist = distance.GetApproximateLength();
								if (dist <= 128.0f)
								{
									if (refitMech && (refitMech->getTonnage() <= 40.0f))
									{
										appearance->setGesture(1);
									}
									else if (refitMech && (refitMech->getTonnage() <= 60.0f))
									{	
										appearance->setGesture(2);
									}
									else if (refitMech && (refitMech->getTonnage() <= 80.0f))
									{
										appearance->setGesture(3);
									}
									else if (refitMech && (refitMech->getTonnage() <= 100.0f))
									{
										appearance->setGesture(4);
									}
								}
							}
						}
						break;

					case 5:			//We were repairing someone.  Switch to doing nothing!
					case 6:			//We were repairing someone.  Switch to doing nothing! 
					case 7:			//We were repairing someone.  Switch to doing nothing! 
					case 8:			//We were repairing someone.  Switch to doing nothing! 
						if (!appearance->getInTransition())
						{
							appearance->setGesture(0);
						}
						break;
				}
			}
		}
		else	//MechBays must ALWAYS be open in Mplayer or it will look dumb!
		{
			appearance->setGesture(4);
		}
	}
	else
	{
		//Just about everything else animates here.
		if (!isDisabled() && !isDestroyed())
		{
			if (!appearance->getInTransition())
			{
				appearance->setGesture(0);
			}
		}
		else	//This will be handled by building destroyed OR parent destroyed!
		{
		}
	}

	return 0;
}

//---------------------------------------------------------------------------
long Building::update (void) 
{
	if (getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		setFlag(OBJECT_FLAG_JUSTCREATED, false);
		BuildingTypePtr type = (BuildingTypePtr)ObjectManager->getObjectType(typeHandle);
				
		//-------------------------------------------------------
		// OK, now use the sprite Bounds to calculate the extent
		// Radius so that nobody has to enter it!
		if (type->getExtentRadius() < 0.0) 
		{
				Stuff::Vector4D diagonalSize;
			diagonalSize.Subtract(appearance->upperLeft,appearance->lowerRight);
			float actualSize = diagonalSize.GetLength();
			actualSize /= worldUnitsPerMeter;
			actualSize *= 1.25;
			
			type->setExtentRadius(actualSize);
		}

		if (type->getExtentRadius() != 0.0)
			setTangible(true);

		//-----------------------------------------------------
		// Check if ParentId is NOT 0xffffffff.
		// if not, find parent in ObjMgr and get its pointer.
		if ((parentId != 0xffffffff) && (parentId != 0))
		{
			parent = ObjectManager->findByCellPosition((parentId>>16),(parentId & 0x0000ffff))->getWatchID();
			if (ObjectManager->getByWatchID(parent)->canBeCaptured())
				ObjectManager->getByWatchID(parent)->setFlag(OBJECT_FLAG_CAPTURABLE, true);

			gosASSERT(parent != 0);
		}
	}

	if (refitBuddyWID && !ObjectManager->getByWatchID(refitBuddyWID))
		refitBuddyWID = 0;

	//We can call update multiple times now since a special building will be updated
	//every frame regardless AND it could also be near where the camera is looking!	
	if (turn != updatedTurn)
	{
		//----------------------------------------------
		// Perimeter Alarms
		// Was there a mover near here last check?
		// Will only be set if we ARE a perimeter alarm in HandleCollision.
		if (moverInProximity)
		{
			proximityTimer += frameLength;
		}
		else
		{
			proximityTimer = 0.0f;
		}
		
		moverInProximity = false;
		
		if (!GeneralAlarm && proximityTimer > 0.0f)
		{
			//Play something close by sound FX
			soundSystem->playDigitalSample(PING_SFX);
			if (proximityTimer > ((BuildingTypePtr)getObjectType())->perimeterAlarmTimer)
			{
				//Set GeneralAlarm to TRUE.
				// It can NEVER go false unless ABL sets it back!!
				GeneralAlarm = true;
			}
		}

		updatedTurn = turn;
		if (appearance)
		{
			updateAnimations();
	
			if ( parent && (!ObjectManager->getByWatchID(parent)->isDisabled()) && ObjectManager->getByWatchID(parent)->getTargeted() ) //must do before we set selection
			{
				setTargeted( true );
			}
	
			
			appearance->setObjectParameters(position,((ObjectAppearance*)appearance)->rotation, drawFlags,getTeamId(),Team::getRelation(getTeamId(), Team::home->getId()));
			bool inView = appearance->recalcBounds();
	
			//------------------------------------------------
			if (getObjectType()->getObjTypeNum() == GENERIC_DESTRUCTIBLE_RESOURCE_BUILDING_OBJNUM) 
			{
				//We are a random resource building.  Mark the terrain under us impassable.
				appearance->markMoveMap(false,NULL);
			}

			// MUST update appearance every frame or animation goes HINKY!
			// Appearance update now checks inView and does NOT run transform math unless necessary!
			// Whoops!
			appearance->update();

			if (inView)
			{
				windowsVisible = turn;
	
				float zPos = land->getTerrainElevation(position);
				position.z = zPos;
				setPosition(position);

				//Check if this object has a GOSFX associated with it for its "activity"
				if ((getStatus() != OBJECT_STATUS_DESTROYED) && 
					(getStatus() != OBJECT_STATUS_DISABLED) &&
					!((BuildingTypePtr)getObjectType())->mechBay)
				{
					if (((BuildingTypePtr)getObjectType())->activityEffectId != 0xffffffff)
						appearance->startActivity(((BuildingTypePtr)getObjectType())->activityEffectId,true);
				}
			}
		}

		//If we are destroyed and we are a bridge, change the overlay under us to the destroyed
		// one.  Must do every frame becuase this terrain data will NOT be saved in a quicksave
		// and I don't want to get the bug of the bridge being fixed after a quickLOAD!!
		if ((baseTileId != 177) && (status == OBJECT_STATUS_DESTROYED))
		{
   			//We don't use this anymore...
   			// NOW we use it to ONLY do this ONCE.
   			// MASSIVE frame rate hit if we do all the time.
   			baseTileId = 177;

			if (getObjectType()->whatAmI() == 607)
			{
				//
				//Find out what vertex we are in the terrain.
				long mx = (float2long(position.x) >> 7) + Terrain::halfVerticesMapSide;
				long my = Terrain::halfVerticesMapSide - ((float2long(position.y) >> 7) + 1);

				Overlays oType;
				DWORD offset;
				Terrain::mapData->getOverlay(my,mx,oType,offset);
				if (oType == OBRIDGE)
					Terrain::mapData->setOverlay(my,mx,DAMAGED_BRIDGE,offset);
			}

   			if (!((BuildingTypePtr)getObjectType())->marksImpassableWhenDestroyed) 
   			{
   				if (getObjectType()->getSubType() == BUILDING_SUBTYPE_LANDBRIDGE)
   					closeSubAreas();
   				else
   					openSubAreas();

   				GlobalMoveMap[0]->clearPathExistsTable();
   				GlobalMoveMap[1]->clearPathExistsTable();
   			}
		}
	}

	//-------------------------------------------
	// Handle power out.
	if (powerSupply && (ObjectManager->getByWatchID(powerSupply)->getStatus() == OBJECT_STATUS_DESTROYED))
		appearance->setLightsOut(true);
	
	//---------------------------------------
	// Handle Lookout tower
	if ((((BuildingTypePtr)getObjectType())->lookoutTowerRange > 0.0f) && getTeam() && 
		(!parent || 
		 (parent && 
		  !ObjectManager->getByWatchID(parent)->isDisabled() && 
		  !ObjectManager->getByWatchID(parent)->isDestroyed())))
	{
		float lookoutRange = ((BuildingTypePtr)getObjectType())->lookoutTowerRange;
		getTeam()->markSeen(position,lookoutRange);
	}                                         
	
	//-------------------------------------------
	// Handle Sensor Building
	if (parent && sensorSystem)
	{
		if (ObjectManager->getByWatchID(parent)->isDisabled() || 
			ObjectManager->getByWatchID(parent)->isDestroyed())
		{
			sensorSystem->disable();
			sensorSystem->broken = true;
		}
	}

	//---------------------------------------
	// Handle Building captured.
	if (parent && 
		!ObjectManager->getByWatchID(parent)->isDisabled() && 
		!ObjectManager->getByWatchID(parent)->isDestroyed() && 
		(ObjectManager->getByWatchID(parent)->getTeamId() != getTeamId()))
	{
		// if building recaptured play a sound
		if ( (ObjectManager->getByWatchID(parent)->getTeamId() != Team::home->getId()) && (turn > 5) && (getTeamId() != -1))
			soundSystem->playBettySample(BETTY_BUILDING_RECAPTURED);
		setTeamId(ObjectManager->getByWatchID(parent)->getTeam()->getId(),false);
	}

	//-----------------------------------------------
	// Handle parent disabled or destroyed or asleep
	if (parent && 
		(ObjectManager->getByWatchID(parent)->isDisabled() || 
		 ObjectManager->getByWatchID(parent)->isDestroyed() || 
		 !ObjectManager->getByWatchID(parent)->getAwake()))
	{
		//--------------------------------------------------
		// Put the child to sleep.  DO NOT DESTROY CHILD!
		// This will make mission objectives cascade badly!
		setAwake(false);
	}


	return(1);
}

//---------------------------------------------------------------------------

long Building::setTeamId (long _teamId, bool setup) 
{
	if (sensorSystem)
		SensorManager->removeTeamSensor(teamId, sensorSystem);

	if (MPlayer)
	{
		//--------------------------------------------------------------------------
		// If this building is set for a team that isn't in this session, kill it...
		if (_teamId >= MPlayer->numTeams && (getObjectType()->getObjTypeNum() != GENERIC_HQ_BUILDING_OBJNUM))
			_teamId = -1;
		captureTime = scenarioTime;
		scoreTime = scenarioTime + 1.0;
		//-----------------------
		// Now, reset the team...
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

	if ((teamId > -1) && sensorSystem)
	{
		SensorManager->addTeamSensor(teamId, sensorSystem);
		if ( (turn > 5) && !isLookoutTower())
			soundSystem->playBettySample( BETTY_SENSOR_CAPTURED );
	}

	static unsigned long highLight[8] = {0x00007f00, 0x0000007f, 0x007f0000};
	if (turn > 10)
		appearance->flashBuilding(5.0,0.5,highLight[Team::relations[teamId][Team::home->getId()]]);
		
	if ((turn > 10) && ((BuildingTypePtr)getObjectType())->resourcePoints)
	{
		if (MPlayer) {
			if (MPlayer->isServer())
				MPlayer->sendReinforcement(((BuildingTypePtr)getObjectType())->resourcePoints, 0, "noname", commanderId, getPosition(), 6);
				//MPlayer->playerInfo[commanderId].resourcePoints += ((BuildingTypePtr)getObjectType())->resourcePoints;
			if (teamId == Team::home->getId())
				soundSystem->playBettySample(BETTY_RESOURCES);
		}
		else {
			LogisticsData::instance->addResourcePoints(((BuildingTypePtr)getObjectType())->resourcePoints);
			soundSystem->playBettySample(BETTY_RESOURCES);
		}
	}

	// Tell the turret control to reset the turret power!!
	// The only other case of buildings with parents is sensor towers which are uncapturable!!
	// If they become capturable, capturing one will capture all!!
	// -fs
	if (parent && 
		ObjectManager->getByWatchID(parent)->getTeamId() != teamId)
		ObjectManager->getByWatchID(parent)->setTeamId(teamId,false);

	//--------------------------------------------------------------------------
	// If this is a multiplayer resource building, reset it to a neutral team...
	if (MPlayer) 
	{
		if (getObjectType()->getObjTypeNum() == GENERIC_INDESTRUCTIBLE_RESOURCE_BUILDING_OBJNUM) 
		{
			teamId = -1;
			commanderId = -1;
		}
	}

	return(NO_ERR);
}

//---------------------------------------------------------------------------

TeamPtr Building::getTeam (void) {

	if (teamId == -1)
		return(NULL);
	return(Team::teams[teamId]);
}

//---------------------------------------------------------------------------

bool Building::isFriendly (TeamPtr team) {

	if (teamId > -1)
		return(Team::relations[teamId][team->getId()] == RELATION_FRIENDLY);
	return(false);
}

//---------------------------------------------------------------------------

bool Building::isEnemy (TeamPtr team) {

	if (teamId > -1)
		return(Team::relations[teamId][team->getId()] == RELATION_ENEMY);
	return(false);
}

//---------------------------------------------------------------------------

bool Building::isNeutral (TeamPtr team) {

	if (teamId > -1)
		return(Team::relations[teamId][team->getId()] == RELATION_NEUTRAL);
	return(true);
}

//---------------------------------------------------------------------------

void Building::lightOnFire (float timeToBurn) 
{
	//Nothing lights on fire now. Part of the effect.
}

//---------------------------------------------------------------------------

bool Building::isCaptureable (long capturingTeamID) {

//	if (MPlayer)
//		return(getFlag(OBJECT_FLAG_CAPTURABLE) && !isDestroyed());
//	else
		return(getFlag(OBJECT_FLAG_CAPTURABLE) && getAwake() && (getTeamId() != capturingTeamID)  && !isDestroyed());
}

//---------------------------------------------------------------------------

void Building::setCommanderId (long _commanderId) {

	commanderId = _commanderId;
}

//---------------------------------------------------------------------------

void Building::render (void) {

	if (!getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		setFlag(OBJECT_FLAG_ONFIRE, false);
	}

	if (appearance->canBeSeen())
	{
		//--------------------------------------
		if (getDrawBars())
		{
			BuildingTypePtr type = (BuildingTypePtr)getObjectType();
			float barStatus = 1.0;
			
			if (getStatus() != OBJECT_STATUS_DESTROYED)
			{
				float totalDmgLvl = type->getDamageLevel();
				if (totalDmgLvl > 0.0)
					barStatus -= getDamage() / totalDmgLvl;
				
				if (barStatus < 0.0)
					barStatus = 0.0;
			}
			else
			{
				barStatus = 0.0f;
			}

			DWORD color = 0xff7f7f7f;
			if ((teamId > -1) && (teamId < 8)) 
			{
				if (getTeam()->isFriendly(Team::home))
					color = SB_GREEN;
				else if (getTeam()->isEnemy(Team::home))
					color = SB_RED;
			}
			
			appearance->setBarColor(color);
			appearance->setBarStatus(barStatus);
		}

		if (((BuildingTypePtr)getObjectType())->buildingTypeName < IDS_MC2_STRING_START)
		{
			appearance->setObjectNameId(((BuildingTypePtr)getObjectType())->buildingTypeName + IDS_MC2_STRING_START);
		}
		else
		{
			appearance->setObjectNameId(((BuildingTypePtr)getObjectType())->buildingTypeName);
		}


		windowsVisible = turn;
		appearance->setVisibility(true,true);
		appearance->render();
	}
	
	setSelected(false);		//ALWAYS reset the selected flags.  GUI needs this to work!
	setTargeted( false );	//ALWAYS do it here, too!  Otherwise things may draw FUNNY!
}

//---------------------------------------------------------------------------
char* Building::getName (void) 
{
	if (((BuildingTypePtr)getObjectType())->buildingTypeName != -1) 
	{
		cLoadString(((BuildingTypePtr)getObjectType())->buildingTypeName, lastName, 254);
		return(lastName);
	}

	return(NULL);
}

//---------------------------------------------------------------------------
void Building::destroy (void)
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

void Building::setDamage (float newDamage) 
{
	damage = newDamage;

	if (damage >= getDamageLevel())
	{
		setStatus(OBJECT_STATUS_DESTROYED);
		appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
	}
}
		
//---------------------------------------------------------------------------

void Building::setSensorRange (float range) {

	if (sensorSystem)
		sensorSystem->setRange(range);
}

//---------------------------------------------------------------------------

void Building::setSensorData (TeamPtr team, float range, bool setTeam) 
{
	if (range > -1.0) 
	{
		if (!sensorSystem) 
		{
			sensorSystem = SensorManager->newSensor();
			if (!sensorSystem)
				Fatal(0, " No RAM for Sensor System ");
		}
		sensorSystem->setOwner(this);
		sensorSystem->setRange(range);
		
		if (!isLookoutTower())
			sensorSystem->setLOSCapability(false);
	}
	
	if (setTeam)
	{
		SensorManager->addTeamSensor(team->getId(), sensorSystem);
	}
}

extern long languageOffset;
//---------------------------------------------------------------------------

void Building::init (bool create, ObjectTypePtr objType) {

	//-------------------------------------------
	// Initialize the Building Appearance here.
	GameObject::init(create, objType);
	
	setExists(true);
	setFlag(OBJECT_FLAG_JUSTCREATED, true);

	captureTime = -1000.0;

	//-------------------------------------------------------------
	// The appearance is initialized here using data from the type
	// Need an MLR appearance class
	char *appearName = objType->getAppearanceTypeName();

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
#if USE_FORESTS
	if (((BldgAppearance*)appearance)->isForestClump())
		getObjectType()->setSubType(BUILDING_SUBTYPE_FOREST);
#endif

	objectClass = BUILDING;
	setFlag(OBJECT_FLAG_DAMAGED, true);

	BuildingTypePtr type = (BuildingTypePtr)getObjectType();
	if (type->getExtentRadius() > 0.0)
		setTangible(true);

	tonnage = type->baseTonnage;
	explDamage = type->explDmg;
	explRadius = type->explRad;
	
	curCV = maxCV = type->startBR;

	setFlag(OBJECT_FLAG_CANREFIT, type->canRefit);
	setFlag(OBJECT_FLAG_MECHBAY, type->mechBay);

	setTeamId(type->teamId,true);

	if (type->sensorRange > -1.0 && getTeam())
		setSensorData(getTeam(), type->sensorRange, true);

	setFlag(OBJECT_FLAG_CAPTURABLE, false);
	if (type->capturable)
		setFlag(OBJECT_FLAG_CAPTURABLE, true);

	setRefitBuddy(0);

	if (type->getDamageLevel() == 0.0f) 
	{
		//-------------------------------------------------------
		// We are already destroyed. Used for extraction markers.
		setTangible(false);
		setStatus(OBJECT_STATUS_DESTROYED);
		setFlag(OBJECT_FLAG_DAMAGED, true);
	}
}	

//---------------------------------------------------------------------------

void Building::createBuildingMarines (void) {

#ifdef USE_ELEMENTALS
	long totalMarines = ((BuildingTypePtr)type)->numMarines;
	long numCreated = 0;
	if (totalMarines) {
		//-----------------------------------------------------
		// Scan the Pilot list for non-home team pilots WHICH are
		// occupying a destroyed vehicle.
		long numPilots = scenario->getNumWarriors();
		for (long j=0;j<numPilots;j++)
		{
			MechWarriorPtr pilot = scenario->getWarrior(j);
			if (pilot && (pilot->getAlignment() != homeTeam->getAlignment()))
			{
				GameObjectPtr myVehicle = pilot->getVehicle();
				
				long pilotVehicleStatus = OBJECT_STATUS_DESTROYED;
				if (myVehicle)
					pilotVehicleStatus = myVehicle->getStatus();
				
				if (!myVehicle || ((pilotVehicleStatus == OBJECT_STATUS_DESTROYED) || (pilotVehicleStatus == OBJECT_STATUS_DISABLED)))	
				{
					//--------------------------------------------------------------------
					// We must create the vehicle for the pilot and then lobotomize him!!
					ElementalPtr vehiclePilot = (ElementalPtr)createObject(DefaultPilotId);
					if (!vehiclePilot)
						Fatal(-1," Couldnt create Marine for Building ");

					vehiclePilot->setAwake(TRUE);

					//----------------------------------------------
					// Load the profile data into the game object...
					FullPathFileName objFullProfileName;
					objFullProfileName.init(profilePath,marineProfileName,".fit");
					FitIniFile profileFile;
					long result = profileFile.open(objFullProfileName);
					if (result != NO_ERR)
					{
						Fatal(result," Unable to open Vehicle Marine Profile ");
					}
								
					//-------------------------------------------
					// <deleted really obnoxious Glenn Slam>
					// Thanks!
					result = vehiclePilot->init(&profileFile);
					if (result != NO_ERR)
						Fatal(-1," Bad Vehicle Marine Profile File ");

					profileFile.close();

					vehiclePilot->setPilot(pilot);		//Gets Current Vehicle Pilot.
					pilot->setVehicle(vehiclePilot);
					pilot->lobotomy();

					vehiclePilot->setControl(2, 3);	//AI Control
					vehiclePilot->setTeam(clanTeam);
					
					//------------------------------------------------------------------
					// Set the object's position, initial gesture and rotation.
					// Apply a random factor to the position.  You must Love me Glenn!!!
					vector_3d randomPosition;
					randomPosition.init(type->getExtentRadius(),type->getExtentRadius(),0);
					randomPosition = randomPosition.get_random_vec();
					vehiclePilot->setPosition(position+randomPosition);
					
					//------------------------------------------
					// Start tracking it on the GameObjectMap...
					GameObjectMap->addObject(vehiclePilot);
					vehiclePilot->bounceToAdjCell();
						vehiclePilot->bounceToAdjCell();
								
					AppearancePtr objAppr = vehiclePilot->getAppearance();
					if (objAppr)
					{
						objAppr->setGesture(0);
						
						if (getAlignment() == CLANS)
							((ElementalActor *)objAppr)->setPaintScheme(28);
						else
							((ElementalActor *)objAppr)->setPaintScheme(18);
					}
											
					//-----------------------------------------------------------------
					// Set object Unique ID to be Part Number
					vehiclePilot->setIdNumber(2500000);
					// Following is a HACK!! at last minute--marines need to be handled
					// properly in next game (partId-wise)...
					vehiclePilot->setPartId(MAX_MOVER_PART_ID - NumMarines++);

					//----------------------------------------------------------------
					// Multiplayer stuff for vehicle Pilot.  What should I do Glenn?
				#if 0
					if (MPlayer) {
						MPlayer->addToMoverRoster((MoverPtr)parts[i].object);
						if (parts[i].commanderID == MPlayer->commanderID)
							MPlayer->addToLocalMovers((MoverPtr)parts[i].object);
					}
				#endif

					//--------------------------------------------------------------------
					// Add the object to the object list, if it exists
					// Otherwise, add it to the holder list which is a member of scenario
					vehiclePilot->setAlignment(getAlignment());
					if (getAlignment() == CLANS)
						objectList->addToList(clanMechList, vehiclePilot);
					else
						objectList->addToList(innerSphereMechList, vehiclePilot);
										
					vehiclePilot->setPotentialContact(POTCONTACT_LEVEL_NONE);
					vehiclePilot->setExists(TRUE);
						
					//---------------------------------------------------------------
					// We need to override the TacOrder of the pilot so that the
					// little marine goes running about like he is trying to escape.
					pilot->clearAttackOrders();
					pilot->clearMoveOrders();
					
					vector_3d location;
					location.zero();
					
					pilot->orderMoveToPoint(FALSE, TRUE, ORDER_ORIGIN_PLAYER, location, -1, TACORDER_PARAM_RUN);
					numCreated++;
					if (numCreated == totalMarines)
						return;
				}
			}
		}
	}
#endif
}	

//---------------------------------------------------------------------------

long Building::handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk) 
{
	if (!shotInfo)
		return(NO_ERR);

	if (addMultiplayChunk) {
		//----------------------------------
		// Generic HQs are indestructible...
		if (getObjectType()->getObjTypeNum() == GENERIC_HQ_BUILDING_OBJNUM)
			return(NO_ERR);
		if (getObjectType()->getObjTypeNum() == GENERIC_INDESTRUCTIBLE_RESOURCE_BUILDING_OBJNUM)
			return(NO_ERR);
		MPlayer->addWeaponHitChunk(this, shotInfo);
	}

	if (!isDestroyed()) 
	{
		float dmg = getDamage() + shotInfo->damage;
		setFlag(OBJECT_FLAG_DAMAGED, true);
		BuildingTypePtr type = (BuildingTypePtr)getObjectType();
		if (dmg >= type->getDamageLevel()) 
		{
			dmg = type->getDamageLevel();
			bool blowItUp = false;
			if (getFlag(OBJECT_FLAG_ANIMATED)) 
			{
				if (!getFlag(OBJECT_FLAG_FALLEN) && !getFlag(OBJECT_FLAG_FALLING)) 
				{
					setFlag(OBJECT_FLAG_FALLING, true);
					setTangible(false);
					setStatus(OBJECT_STATUS_DESTROYED);
					appearance->stopActivity();
					GameObjectPtr attacker = ObjectManager->getByWatchID(shotInfo->attackerWID);
					if (attacker && attacker->isMover())
						((MoverPtr)attacker)->getPilot()->triggerAlarm(PILOT_ALARM_KILLED_TARGET, getWatchID());

					if (sensorSystem)
						sensorSystem->disable();

					blowItUp = true;
				}
			}
			else 
			{
				if (sensorSystem)
					sensorSystem->disable();

				setStatus(OBJECT_STATUS_DESTROYED);
				appearance->stopActivity();
				setTangible(false);

				blowItUp = true;
			}

			if (blowItUp) 
			{
				//-----------------------------------------------------
				// Yes we do want to blow it up but using a generic Bldg Explosion
				// From now until better building destruction

				//-----------------------------------------------------
				// Now, blow the building up using its type->explosion
				// ONLY if the building has no special MAGIC gos FX version!
				Stuff::Vector3D hitNodePos = appearance->getHitNode();
				if (!appearance->playDestruction())
					ObjectManager->createExplosion(BUILDING_EXPLOSION_ID,NULL,hitNodePos,explDamage,explRadius);
				else	//Play the sound effect and do splash damage but don't draw any effect.  We are playing a magical GosFX one!!
					ObjectManager->createExplosion(EMPTY_EXPLOSION_ID,NULL,hitNodePos,explDamage,explRadius);

#if 0
				if (type->marksImpassableWhenDestroyed) 
					appearance->markMoveMap(true,NULL);
#endif

				appearance->markLOS(true);

				appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
				appearance->setLightsOut(true);
				appearance->recalcBounds();
				appearance->update();
				appearance->markLOS();
				
				if (!type->marksImpassableWhenDestroyed) 
				{
					if (getObjectType()->getSubType() == BUILDING_SUBTYPE_LANDBRIDGE)
						closeSubAreas();
					else
						openSubAreas();
					GlobalMoveMap[0]->clearPathExistsTable();
					GlobalMoveMap[1]->clearPathExistsTable();
				}
#if 0
				else	//We want the buildings remaining shape to correctly calc LOS and Impassability
				{
					appearance->markLOS();
					appearance->markMoveMap(false,NULL);	//Then, use the destroyed shape to mark impassable
				}
#endif							
				if (CombatLog) {
					char s[1024];
					sprintf(s, "[%.2f] building.destroyed: [%05d]%s", scenarioTime, this->getPartId(), this->getName());
					CombatLog->write(s);
					CombatLog->write(" ");
				}

				if (getObjectType()->getSubType() == BUILDING_SUBTYPE_LANDBRIDGE)
				{
					//Check each mover to see if any cell they are standing on went impassable.  If so,
					// BLOW the mech to completely destroyed.  Don't want to be able to salvage it either!!
					// UNFORTUNATELY, the move code does not store the landBridge stuff.
					// ALL I can do is blow a guy for standing on impassable terrain now.
					short* curCoord = cellsCovered;
					for (long i = 0; i < numCellsCovered; i++) 
					{
						long r = *curCoord++;
						long c = *curCoord++;

						//Traverse the moverLists and blow anyone standing on these cells which JUST went impassable
						long numMovers = ObjectManager->getNumMovers();
						for (int j = 0; j < numMovers; j+= 1) 
						{
							MoverPtr pMover = ObjectManager->getMover(j);
							if (pMover && pMover->getExists())
							{
								int cellRow, cellCol;
								land->worldToCell(pMover->getPosition(), cellRow, cellCol);
								if ((cellCol == c) && (cellRow == r))
								{
									//DIE DIE DIE
									for (long k=0;k<50;k++)
									{
										WeaponShotInfo shotInfo;
										shotInfo.init(0, 160, 50.0f, pMover->calcHitLocation(NULL,-1,ATTACKSOURCE_WEAPONFIRE,0), 0);
										pMover->handleWeaponHit(&shotInfo);
										if (MPlayer && MPlayer->isServer())
											MPlayer->addWeaponHitChunk((GameObjectPtr)this, &shotInfo);
									}
								}
							}
						}
					}
				}
			}
		}
		setDamage(dmg);
	}
	
	return(NO_ERR);
}


//---------------------------------------------------------------------------

float Building::getDamageLevel (void) {

	BuildingTypePtr type = (BuildingTypePtr)getObjectType();
	if (type)
		return(type->getDamageLevel());
	return(0.0);
}

//---------------------------------------------------------------------------
bool Building::isLinked (void)
{
	return (parent != 0);
}

//---------------------------------------------------------------------------
GameObjectPtr Building::getParent (void)
{
	return (ObjectManager->getByWatchID(parent));
}

//---------------------------------------------------------------------------
void Building::setParentId (DWORD pId)
{
	parentId = pId;
}

//***************************************************************************
void Building::Save (PacketFilePtr file, long packetNum)
{
	BuildingData data;
	CopyTo(&data);

	//PacketNum incremented in ObjectManager!!
	file->writePacket(packetNum,(MemoryPtr)&data,sizeof(BuildingData),STORAGE_TYPE_ZLIB);
}

//***************************************************************************
void Building::CopyTo (BuildingData *data)
{																	   

	data->teamId = teamId;;
	data->baseTileId = baseTileId;
	data->commanderId = commanderId;
	data->refitBuddyWID = refitBuddyWID;
	data->parentId = parentId;
	data->parent = parent;
	data->listID = listID;
	data->captureTime = captureTime;

	data->moverInProximity = moverInProximity;
	data->proximityTimer = proximityTimer;
	data->updatedTurn = updatedTurn;

	TerrainObject::CopyTo(data);
}

//---------------------------------------------------------------------------
void Building::Load (BuildingData *data)
{
	TerrainObject::Load(data);

	teamId = data->teamId;
	baseTileId = 0;
	commanderId = data->commanderId;
	refitBuddyWID = data->refitBuddyWID;
	parentId = data->parentId;
	parent = data->parent;
	listID = data->listID;
	captureTime = data->captureTime;

	moverInProximity = data->moverInProximity;
	proximityTimer = data->proximityTimer;
	updatedTurn = 0;
}

bool Building::burnRefitPoints(float pointsToBurn) 
{
	setDamage(pointsToBurn + getDamage());

	if (getDamage() >= getDamageLevel())
	{
		soundSystem->playBettySample( BETTY_REPAIR_GONE );
		return(false);
	}

	return true;
}

//***************************************************************************
