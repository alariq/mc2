//---------------------------------------------------------------------------
//
//	terrobj.cpp -- File contains the misc terrain object code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef TERROBJ_H
#include"terrobj.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef SOUNDS_H
#include"sounds.h"
#endif

#ifndef MOVE_H
#include"move.h"
#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef COLLSN_H
#include"collsn.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef CARNAGE_H
#include"carnage.h"
#endif

#ifndef DOBJNUM_H
#include"dobjnum.h"
#endif

#include"objectappearance.h"

extern unsigned long NextIdNumber;
extern float worldUnitsPerMeter;
extern bool drawExtents;
extern bool somethingOnFire;
extern bool	useOldProject;
extern bool MLRVertexLimitReached;

inline float agsqrt( float _a, float _b )
{
	return sqrt(_a*_a + _b*_b);
}

#define TREE_FALL_RATE		15.0f
#define TREE_FALL_ACCEL		5.0f;

char lastName[256];

extern MidLevelRenderer::MLRClipper * theClipper;
extern bool useNonWeaponEffects;
//---------------------------------------------------------------------------
// class TerrainObjectType
//---------------------------------------------------------------------------

GameObjectPtr TerrainObjectType::createInstance (void) {

	TerrainObjectPtr result = new TerrainObject;
	if (!result)
		return NULL;

	result->init(true, this);
	//result->setIdNumber(NextIdNumber++);
	
	return(result);
}

//---------------------------------------------------------------------------

void TerrainObjectType::init (void) {
			
	objectTypeClass = TERRAINOBJECT_TYPE;
	objectClass = TERRAINOBJECT;
			
	subType = TERROBJ_NONE;
	damageLevel = 0.0;
	collisionOffsetX = 0;
	collisionOffsetY = 0;
	setImpassable = false;
	xImpasse = 0;
	yImpasse = 0;
	extentRadius = -1.0;
	explDmg = 0.0;
	explRad = 0.0;
	fireTypeHandle = -1;
}

//---------------------------------------------------------------------------

void TerrainObjectType::destroy (void) 
{
	ObjectType::destroy();
}
		
//---------------------------------------------------------------------------

void TerrainObjectType::initMiscTerrObj (long objTypeNum) {

	//---------------------------------------------------------------------
	// This function is here to maintain compatibility with MC1. The values
	// used for the various miscTerrainTypes are hardcoded, based on
	// MC1 ship values. If we need to modify 'em, then try adding new
	// object types to the packet file!
	explosionObject = 0xFFFFFFFF;
	destroyedObject = 0xFFFFFFFF;
	extentRadius = -1.0;
	keepMe = true;
	iconNumber = -1;
	teamId = -1;

	if (objTypeNum == ObjectTypeManager::bridgeTypeHandle) {
		subType = TERROBJ_BRIDGE;
		damageLevel = 100.0;
		}
	else if (objTypeNum == ObjectTypeManager::forestTypeHandle) {
		subType = TERROBJ_FOREST;
		damageLevel = 100.0;
		fireTypeHandle = 1;
		}
	else if (objTypeNum == ObjectTypeManager::wallHeavyTypeHandle) {
		subType = TERROBJ_WALL_HEAVY;
		damageLevel = 100.0;
		}
	else if (objTypeNum == ObjectTypeManager::wallMediumTypeHandle) {
		subType = TERROBJ_WALL_MEDIUM;
		damageLevel = 100.0;
		}
	else if (objTypeNum == ObjectTypeManager::wallLightTypeHandle) {
		subType = TERROBJ_WALL_LIGHT;
		damageLevel = 100.0;
		}
	else
		Fatal(objTypeNum, " TerrainObjectType.init: bad MiscTerrainObj num ");
}

//---------------------------------------------------------------------------

long TerrainObjectType::init (FilePtr objFile, unsigned long fileSize) {

	long result = 0;
	
	FitIniFile bldgFile;
	result = bldgFile.open(objFile, fileSize);
	if (result != NO_ERR)
		return(result);
	
	//----------------------------------------------
	// Read in the data needed for the TerrainObject
	subType = TERROBJ_NONE;
	result = bldgFile.seekBlock("TerrainObjectData");
	if (result != NO_ERR) {
		result = bldgFile.seekBlock("TreeData");
		if (result != NO_ERR)
			return(result);
		subType = TERROBJ_TREE;
		objectClass = TREE;
	}

	unsigned long dmgLevel;
	result = bldgFile.readIdULong("DmgLevel",dmgLevel);
	if (result != NO_ERR)
		return(result);
	damageLevel = (float)dmgLevel;
		
	result = bldgFile.readIdLong("CollisionOffsetX",collisionOffsetX);
	if (result != NO_ERR)
		collisionOffsetX = 0;
	
	result = bldgFile.readIdLong("CollisionOffsetY",collisionOffsetY);
	if (result != NO_ERR)	
		collisionOffsetY = 0;

	long setImpass;
	result = bldgFile.readIdLong("SetImpassable",setImpass);
	setImpassable = false;
	if (result == NO_ERR)
		setImpassable = setImpass ? true : false;
		
	result = bldgFile.readIdLong("XImpasse",xImpasse);
	if (result != NO_ERR)
		xImpasse = 0;
	
	result = bldgFile.readIdLong("YImpasse",yImpasse);
	if (result != NO_ERR)	
		yImpasse = 0;

	float realExtent = 0.0;
	result = bldgFile.readIdFloat("ExtentRadius", realExtent);
	if (result != NO_ERR)
		realExtent = -1.0;

	result = bldgFile.readIdFloat("ExplosionRadius",explRad);
	//-----------------------------------------------------------------
	// if this fails, explosion radius is not set and no splash damage.
	if (result != NO_ERR)
		explRad = 0.0;
		
	result = bldgFile.readIdFloat("ExplosionDamage",explDmg);
	// if this fails, explosion damage is not set and no splash damage.
	if (result != NO_ERR)
		explDmg = 0.0;

	//-------------------------------------------------------
	// Initialize the base object Type from the current file.
	result = ObjectType::init(&bldgFile);
	extentRadius = realExtent;

	return(result);
}
	
//---------------------------------------------------------------------------

bool TerrainObjectType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider) {

	if (MPlayer && !MPlayer->isServer())
		return(true);

	switch (subType) {
		case TERROBJ_NONE:
			//-------------------------------------------------------
			// The Building ceases to exist when its effect is done.
			// always return FALSE or the collision will make it
			// go away!  We may want to play a sound effect here.
			switch (collider->getObjectClass()) {
				case EXPLOSION:
				case BATTLEMECH:
				case GROUNDVEHICLE:
				if (!collider->isMover() || (collider->isMover() && ((MoverPtr)collider)->pathLocks))
				{
					WeaponShotInfo shot;
					shot.init(0, -1, collidee->getDamageLevel(), 0, 0);
					collidee->handleWeaponHit(&shot, (MPlayer != NULL));
				}
				break;
			}
			break;
		case TERROBJ_TREE:
			//-------------------------------------------------------
			// Trees are magical.  If a mech hits one, it goes down
			// and is replaced by its last frame.  In other words,
			// play the animation and stop on last frame.
			//
			// When a tree falls, we change its frame_of_ref to match
			// the direction it should fall in from the nominal world frame.
			//
			switch (collider->getObjectClass()) {
				case EXPLOSION:
				case BATTLEMECH:
				case GROUNDVEHICLE:
				if (!collider->isMover() || (collider->isMover() && ((MoverPtr)collider)->pathLocks))
				{
					TerrainObjectPtr tree = (TerrainObjectPtr)collidee;
					if (!tree->getFlag(OBJECT_FLAG_FALLEN) && !tree->getFlag(OBJECT_FLAG_FALLING)) {
						tree->setFlag(OBJECT_FLAG_FALLING, true);
						
						float fallAngle = collidee->relFacingTo(collider->getPosition());
						collidee->rotate(fallAngle,0.0f);
						
						//------------------------------------------------------
						// Tree has fallen.  You may no longer collide with it.
						tree->setTangible(false);
					}
					}
					break;
			}
			break;
		case TERROBJ_BRIDGE:
		case TERROBJ_FOREST:
		case TERROBJ_WALL_HEAVY:
		case TERROBJ_WALL_MEDIUM:
			break;
		case TERROBJ_WALL_LIGHT:
			switch (collider->getObjectClass()) {
				case BATTLEMECH:
				case GROUNDVEHICLE: {
					WeaponShotInfo shotInfo;
					shotInfo.init(collider->getWatchID(), -1, 250.0, 0, 0);
					if (MPlayer) {
						if (MPlayer->isServer()) {
							collidee->handleWeaponHit(&shotInfo, true);
						}
						}
					else
						collidee->handleWeaponHit(&shotInfo);
					}
					break;
			}
			break;
	}
	
	return(true);
}

//---------------------------------------------------------------------------

bool TerrainObjectType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider) {

	TerrainObjectPtr me = (TerrainObjectPtr)collidee;
	if (me->getObjectType()->getSubType() == TERROBJ_FOREST) {
		me->openSubAreas();
	}
	return(false);
}

//***************************************************************************
// class TerrainObject
//***************************************************************************
void TerrainObject::rotate (float yaw, float pitch)
{
	rotation = yaw;
	pitchAngle = pitch;
}

void TerrainObject::updateDebugWindow (GameDebugWindow* debugWindow) {

	debugWindow->clear();
	char s[128];
	//-------------------------------------------------------
	// For now, show the floating help text if we have one...
	if (((ObjectAppearance*)appearance)->objectNameId != -1) {
		char myName[255];
		cLoadString(((ObjectAppearance*)appearance)->objectNameId, myName, 254);
		debugWindow->print(myName);
		}
	else
		debugWindow->print("<no name>");
	sprintf(s, "team: %d, handle: %d, partID: %d %s", getTeamId(), getHandle(), getPartId(), getFlag(OBJECT_FLAG_CAPTURABLE) ? "[C]" : " ");
	debugWindow->print(s);
	sprintf(s, "objType: %d", getObjectType()->whatAmI());
	debugWindow->print(s);
	sprintf(s, "damage: %.2f/%.2f", getDamage(), getDamageLevel());
	debugWindow->print(s);
	sprintf(s, "pos: [%d, %d](area = %d)", cellPositionRow, cellPositionCol, GlobalMoveMap[0]->calcArea(cellPositionRow, cellPositionCol));
	debugWindow->print(s);
	sprintf(s, "footprint:[%d,%d]:[%d,%d]", cellFootprint[0], cellFootprint[1], cellFootprint[2], cellFootprint[3]);
	debugWindow->print(s);
	if (numSubAreas0 > 0) {
		sprintf(s, "subAreas:");
		for (long i = 0; i < numSubAreas0; i++) {
			char tempStr[15];
			sprintf(tempStr, " %d", subAreas0[i]);
			strcat(s, tempStr);
		}
		strcat(s, " *");
		for (i = 0; i < numSubAreas1; i++) {
			char tempStr[15];
			sprintf(tempStr, " %d", subAreas1[i]);
			strcat(s, tempStr);
		}
		debugWindow->print(s);
	}
}

//---------------------------------------------------------------------------

bool TerrainObject::isVisible (void) {

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

char* TerrainObject::getName (void) {

	/*
	static char* terrainObjectNames[NUM_TERROBJ_SUBTYPES] = {
		"Nothing",
		"Tree",
		"Bridge",
		"Forest",
		"Heavy Wall",
		"Medium Wall",
		"Light Wall"
	};

	TerrainObjectTypePtr type = (TerrainObjectTypePtr)getObjectType();
	return(terrainObjectNames[type->subType]);
	*/

	if (((ObjectAppearance*)appearance)->objectNameId != -1) {
		cLoadString(((ObjectAppearance*)appearance)->objectNameId, lastName, 254);
		return(lastName);
	}
	return(NULL);
}

//---------------------------------------------------------------------------

float TerrainObject::getStatusRating (void) {

	float curDamage = getDamage();
	float maxHealth = getDamageLevel();
	float rating = (maxHealth - curDamage) / maxHealth;
	if (rating < 0.0)
		rating = 0.0;
	return(rating);
}

//---------------------------------------------------------------------------

#define BRIDGE_OFFSET		60

long TerrainObject::update (void) {

	if (getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		setFlag(OBJECT_FLAG_JUSTCREATED, false);
		setFlag(OBJECT_FLAG_TILECHANGED, false);

		TerrainObjectTypePtr type = (TerrainObjectTypePtr)ObjectManager->getObjectType(typeHandle);

		switch (type->subType) {
			case TERROBJ_NONE:
			case TERROBJ_TREE: 
			{
				setTangible(true);
			}
			break;
			
			case TERROBJ_BRIDGE:
				if (!GameMap->getPassable(cellPositionRow, cellPositionCol)) {
					damage = type->getDamageLevel();
					setStatus(OBJECT_STATUS_DESTROYED);
				}
				break;
			case TERROBJ_FOREST:
			case TERROBJ_WALL_HEAVY:
			case TERROBJ_WALL_MEDIUM:
			case TERROBJ_WALL_LIGHT:
				if (GameMap->getPassable(cellPositionRow, cellPositionCol)) {
					damage = type->getDamageLevel();
					setStatus(OBJECT_STATUS_DESTROYED);
				}
				break;
		}

	}

	//-------------------------------------------
	// Handle power out.
	if (powerSupply && (ObjectManager->getByWatchID(powerSupply)->getStatus() == OBJECT_STATUS_DESTROYED))
		appearance->setLightsOut(true);
		
 	if (appearance)
	{
		if (getFlag(OBJECT_FLAG_FALLING))
		{
			if (fallRate == 0.0f)
			{
				if (useSound && soundSystem)
					soundSystem->playDigitalSample(TREEFALL, getPosition(), true);
					
				fallRate = TREE_FALL_RATE;
			}
			else
				fallRate += TREE_FALL_ACCEL;
				
			pitchAngle -= (frameLength * fallRate);
			if (pitchAngle < -85.0f)
			{
				setFlag(OBJECT_FLAG_FALLEN,true);
				setFlag(OBJECT_FLAG_FALLING,false);
			}
		}
		
		appearance->setObjectParameters(position,rotation,FALSE,getTeamId(),Team::getRelation(getTeamId(), Team::home->getId()));
		appearance->setMoverParameters(pitchAngle);
		bool inView = appearance->recalcBounds();

		if (inView)
		{
			windowsVisible = turn;
			appearance->update();

			if (bldgDustPoofEffect && bldgDustPoofEffect->IsExecuted())
			{
				Stuff::Point3D			actualPosition;
				Stuff::LinearMatrix4D 	shapeOrigin;
				Stuff::LinearMatrix4D 	localToWorld;
					
				actualPosition.x = -position.x;
				actualPosition.y = position.z;
				actualPosition.z = position.y;
			
 				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(actualPosition);
		
				Stuff::OBB boundingBox;
				gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
		
				bool result = bldgDustPoofEffect->Execute(&info);
				if (!result)
				{
					bldgDustPoofEffect->Kill();
					delete bldgDustPoofEffect;
					bldgDustPoofEffect = NULL;
				}
			}
		}
	}

	return(1);
}

//---------------------------------------------------------------------------

void TerrainObject::render (void) {

	if (!getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
	}

	if (appearance->canBeSeen())
	{
		if (getSelected())
		{
			TerrainObjectTypePtr type = (TerrainObjectTypePtr)getObjectType();
			float barStatus = 1.0;
			float totalDmgLvl = type->getDamageLevel();
			if (totalDmgLvl > 0.0)
				barStatus -= getDamage() / totalDmgLvl;
			
			if (barStatus < 0.0)
				barStatus = 0.0;

			DWORD color = 0xff7f7f7f;
			
			appearance->setBarColor(color);
			appearance->setBarStatus(barStatus);
		}

		//For debug purposes only.  Will crash pause!!	Sorry Heidi!
//		if (windowsVisible != turn)
//			STOP(("Rendering without an update!"));

		appearance->setVisibility(true,true);
		appearance->render();
		
		//------------------------------------------------
		// Render GOSFX
		gosFX::Effect::DrawInfo drawInfo;
		drawInfo.m_clipper = theClipper;
		
		MidLevelRenderer::MLRState mlrState;
		mlrState.SetDitherOn();
		mlrState.SetTextureCorrectionOn();
		mlrState.SetZBufferCompareOn();
		mlrState.SetZBufferWriteOn();

		drawInfo.m_state = mlrState;
		drawInfo.m_clippingFlags = 0x0;
		
		if (bldgDustPoofEffect && bldgDustPoofEffect->IsExecuted())
		{
			Stuff::Point3D			actualPosition;
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;

			actualPosition.x = -position.x;
			actualPosition.y = position.z;
			actualPosition.z = position.y;
							
 			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(actualPosition);
				
			drawInfo.m_parentToWorld = &shapeOrigin;
	 
			if (!MLRVertexLimitReached)
				bldgDustPoofEffect->Draw(&drawInfo);
		}
	}
	
	setSelected(false);		//ALWAYS reset the selected flags.  GUI needs this to work!
	setTargeted( false );	//ALWAYS do it here, too!  Otherwise things may draw FUNNY!
}

//---------------------------------------------------------------------------
void TerrainObject::renderShadows (void)
{
	if (getFlag(OBJECT_FLAG_FALLING) || getFlag(OBJECT_FLAG_FALLEN))
		return;			//No shadows on fallen trees.
		
	if (appearance->canBeSeen())
	{
		appearance->renderShadows();
	}
	
	setSelected(false);		//ALWAYS reset the selected flags.  GUI needs this to work!
	setTargeted( false );	//ALWAYS do it here, too!  Otherwise things may draw FUNNY!

}	

//---------------------------------------------------------------------------
void TerrainObject::destroy (void) 
{
	if (cellsCovered) 
	{
		numCellsCovered = 0;
		systemHeap->Free(cellsCovered);
		cellsCovered = NULL;
	}

	if (subAreas0)
	{
		ObjectTypeManager::objectCache->Free(subAreas0);
		subAreas0 = NULL;
	}

	if (subAreas1)
	{
		ObjectTypeManager::objectCache->Free(subAreas1);
		subAreas1 = NULL;
	}

	//-----------------------------------------------------
	// This will free any memory the Building is using.
	if (appearance) 
	{
		delete appearance;
		appearance = NULL;
	}
}

//---------------------------------------------------------------------------

void TerrainObject::setDamage (long newDamage) {

	damage = (float)newDamage;

	TerrainObjectTypePtr type = (TerrainObjectTypePtr)getObjectType();

	switch (type->subType) 
	{
		case TERROBJ_TREE:
		case TERROBJ_NONE:
			
		break;
	}

	//---------------------------------------------
	// Code needs to go in here to fix appearance
	if (damage >= getDamageLevel())
	{
		setStatus(OBJECT_STATUS_DESTROYED);
		if (appearance)
			appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
	}
}
		
//---------------------------------------------------------------------------

void TerrainObject::init (bool create, ObjectTypePtr objType) {

	GameObject::init(true, objType);

	setFlag(OBJECT_FLAG_JUSTCREATED, true);

	if (((TerrainObjectTypePtr)objType)->subType == TERROBJ_TREE)
	{
		//-------------------------------------------------------------
		// The appearance is initialized here using data from the type
		// Need an MLR appearance class
		char *appearanceName = objType->getAppearanceTypeName();

		//--------------------------------------------------------------
		// New code!!!
		// We need to append the sprite type to the appearance num now.
		// The MechEdit tool does not assume a sprite type, nor should it.
		// MechCmdr2 features much simpler objects which only use 1 type of sprite!
		long appearanceType = (TREED_TYPE << 24);

		AppearanceTypePtr terrainObjectAppearanceType = appearanceTypeList->getAppearance(appearanceType,appearanceName);
		if (!terrainObjectAppearanceType)
		{
			//------------------------------------------------------
			// LOAD a dummy appearance until real ones are available
			// for this building!
			terrainObjectAppearanceType = appearanceTypeList->getAppearance(appearanceType,"TREE");
			gosASSERT(terrainObjectAppearanceType != NULL);
		}
		  
	   	appearance = new TreeAppearance;
		gosASSERT(appearance != NULL);

		appearance->init((TreeAppearanceType*)terrainObjectAppearanceType, (GameObjectPtr)this);
	}
	else
	{
		//-------------------------------------------------------------
		// The appearance is initialized here using data from the type
		// Need an MLR appearance class
		char *appearanceName = objType->getAppearanceTypeName();

		//--------------------------------------------------------------
		// New code!!!
		// We need to append the sprite type to the appearance num now.
		// The MechEdit tool does not assume a sprite type, nor should it.
		// MechCmdr2 features much simpler objects which only use 1 type of sprite!
		long appearanceType = (BLDG_TYPE << 24);

		AppearanceTypePtr terrainObjectAppearanceType = appearanceTypeList->getAppearance(appearanceType,appearanceName);
		if (!terrainObjectAppearanceType)
		{
			//------------------------------------------------------
			// LOAD a dummy appearance until real ones are available
			// for this building!
			terrainObjectAppearanceType = appearanceTypeList->getAppearance(appearanceType,"TESTOBJ");
		}
	  
	   	appearance = new BldgAppearance;
		gosASSERT(appearance != NULL);

		//--------------------------------------------------------------
		// The only appearance type for buildings is MLR_APPEARANCE.
		gosASSERT(terrainObjectAppearanceType->getAppearanceClass() == BLDG_TYPE);
		
		appearance->init((BldgAppearanceType*)terrainObjectAppearanceType, (GameObjectPtr)this);
	}

	if (objType->getExtentRadius() > 0.0)
		setTangible(true);

	objectClass = TERRAINOBJECT;
	switch (((TerrainObjectTypePtr)objType)->subType) {
		case TERROBJ_NONE:
			if (((TerrainObjectTypePtr)objType)->getDamageLevel() == 0.0) {
				//--------------------------------------------------------
				// We are already destroyed.  Used for extraction Markers
				setTangible(false);
				setStatus(OBJECT_STATUS_DESTROYED);
			}
			break;
		case TERROBJ_TREE:
			objectClass = TREE;
			setFlag(OBJECT_FLAG_DAMAGED, false);
			break;
		case TERROBJ_BRIDGE:
		case TERROBJ_FOREST:
		case TERROBJ_WALL_HEAVY:
		case TERROBJ_WALL_MEDIUM:
		case TERROBJ_WALL_LIGHT:
			setTangible(false);
			objectClass = BRIDGE;
			break;
	}
}	

//---------------------------------------------------------------------------

void TerrainObject::killFire (void) {

}

//---------------------------------------------------------------------------

void TerrainObject::lightOnFire (float timeToBurn) 
{
}

#define DUST_POOF_ID		32
//---------------------------------------------------------------------------
long TerrainObject::handleWeaponHit (WeaponShotInfoPtr shotInfo, bool addMultiplayChunk) {

	if (!shotInfo)
		return(NO_ERR);

	if (addMultiplayChunk)
		MPlayer->addWeaponHitChunk(this, shotInfo);

	if (!getFlag(OBJECT_FLAG_DAMAGED)) 
	{
		float curDamage = getDamage();
		TerrainObjectTypePtr type = (TerrainObjectTypePtr)getObjectType();
		switch (type->subType) 
		{
			case TERROBJ_NONE:
				curDamage += shotInfo->damage;
				if (curDamage > type->getDamageLevel())
					curDamage = type->getDamageLevel();
				setDamage(curDamage);
				
				if (curDamage >= type->getDamageLevel())
				{
					setFlag(OBJECT_FLAG_DAMAGED, true);
					curDamage = type->getDamageLevel();
					setStatus(OBJECT_STATUS_DESTROYED);
					setTangible(false);

					appearance->markLOS(true);
					appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
					appearance->setLightsOut(true);
					appearance->recalcBounds();
					appearance->update();
					appearance->markLOS();
					
					if (!shotInfo->attackerWID && (shotInfo->masterId == -1))	//Somebody stepped on me.
					{
						//--------------------------------------------
						//Play a Dust Poof.
						if (useNonWeaponEffects)
						{
							unsigned flags = gosFX::Effect::ExecuteFlag;
					
							Check_Object(gosFX::EffectLibrary::Instance);
							gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(DUST_POOF_ID));
							
							if (gosEffectSpec)
							{
								bldgDustPoofEffect = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
								gosASSERT(bldgDustPoofEffect != NULL);
							}
								
							MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
						}
						
						if (bldgDustPoofEffect)
						{
							Stuff::Point3D			actualPosition;
							Stuff::LinearMatrix4D 	shapeOrigin;
							Stuff::LinearMatrix4D	localToWorld;
							
							actualPosition.x = -position.x;
							actualPosition.y = position.z;
							actualPosition.z = position.y;
							
							shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
							shapeOrigin.BuildTranslation(actualPosition);
							
							gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
							bldgDustPoofEffect->Start(&info);
						}
					}
				}

			break;

			case TERROBJ_TREE:
				setDamage(curDamage + 1.0);
				
				if (curDamage > type->getDamageLevel())
				{
					setFlag(OBJECT_FLAG_DAMAGED, true);
					curDamage = type->getDamageLevel();
					setStatus(OBJECT_STATUS_DESTROYED);

					appearance->markLOS(true);
					appearance->setObjStatus(OBJECT_STATUS_DESTROYED);
					appearance->setLightsOut(true);
					appearance->recalcBounds();
					appearance->update();
					appearance->markLOS();
				}
				break;
			case TERROBJ_FOREST: 
			{
				curDamage += shotInfo->damage;
				if (curDamage >= type->getDamageLevel()) 
				{
					type->createExplosion(position, 0, 0);
					setStatus(OBJECT_STATUS_DESTROYED);
					openSubAreas();
				}
				setDamage(curDamage);
			}
			break;

			case TERROBJ_WALL_HEAVY:
			case TERROBJ_WALL_MEDIUM:
			case TERROBJ_WALL_LIGHT:
				curDamage += shotInfo->damage;
				if (curDamage >= type->getDamageLevel()) 
				{
					type->createExplosion(position, 0, 0);
					setStatus(OBJECT_STATUS_DESTROYED);
					if (type->subType == TERROBJ_WALL_LIGHT)
						soundSystem->playDigitalSample(BREAKINGFENCE, getPosition(), true);
				}
				setDamage(curDamage);
			break;
		}
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------

void TerrainObject::setTerrainPosition (const Stuff::Vector3D& position, const Stuff::Vector2DOf<long>& numbers)
{
	setPosition( position );
				
	vertexNumber = numbers.x;
	blockNumber = numbers.y;
}

void TerrainObject::setRotation( float rot )
{
	rotation = rot;
	((ObjectAppearance*)appearance)->rotation = rot;
}

//---------------------------------------------------------------------------

void TerrainObject::calcCellFootprint (Stuff::Vector3D& pos) {

	short cellList[MAX_CELL_COORDS];
	cellList[0] = MAX_CELL_COORDS;
	long numCoords = appearance->calcCellsCovered(pos, cellList);
	long minRow = 10000;
	long minCol = 10000;
	long maxRow = 0;
	long maxCol = 0;
	long curCoord = 0;
	while (curCoord < numCoords) {
		if (cellList[curCoord] < minRow)
			minRow = cellList[curCoord];
		if (cellList[curCoord] > maxRow)
			maxRow = cellList[curCoord];
		curCoord++;
		if (cellList[curCoord] < minCol)
			minCol = cellList[curCoord];
		if (cellList[curCoord] > maxCol)
			maxCol = cellList[curCoord];
		curCoord++;
	}
	
	if (numCoords)
	{
		cellFootprint[0] = minRow;
		cellFootprint[1] = minCol;
		cellFootprint[2] = maxRow;
		cellFootprint[3] = maxCol;
		land->cellToWorld(cellFootprint[0], cellFootprint[1], vectorFootprint[0]);
		land->cellToWorld(cellFootprint[0], cellFootprint[3], vectorFootprint[1]);
		land->cellToWorld(cellFootprint[2], cellFootprint[3], vectorFootprint[2]);
		land->cellToWorld(cellFootprint[2], cellFootprint[1], vectorFootprint[3]);
	}
	else
	{
		cellFootprint[0] = 0;
		cellFootprint[1] = 0;
		cellFootprint[2] = 0;
		cellFootprint[3] = 0;

		vectorFootprint[0].Zero();
		vectorFootprint[1].Zero();
		vectorFootprint[2].Zero();
		vectorFootprint[3].Zero();
	}
}

//---------------------------------------------------------------------------

long TerrainObject::getLineOfSightNodes (long eyeCellRow, long eyeCellCol, long* cells) {

	cells[0] = cellFootprint[0];
	cells[1] = cellFootprint[1];

	cells[2] = cellFootprint[0];
	cells[3] = cellFootprint[3];

	cells[4] = cellFootprint[2];
	cells[5] = cellFootprint[3];

	cells[6] = cellFootprint[2];
	cells[7] = cellFootprint[1];

	return(4);
}

//---------------------------------------------------------------------------

void TerrainObject::calcSubAreas (long numCells, short cells[MAX_GAME_OBJECT_CELLS][2]) {

	numCellsCovered = numCells;
	if (numCellsCovered) {
		cellsCovered = (short*)systemHeap->Malloc(4 * numCellsCovered);
		if (cellsCovered) {
			short* curCoord = cellsCovered;
			for (long j = 0; j < numCellsCovered; j++) {
				*curCoord++ = cells[j][0];
				*curCoord++ = cells[j][1];
			}
		}
	
		numSubAreas0 = 0;
		short* curCoord = cellsCovered;
		for (long i = 0; i < numCellsCovered; i++) 
		{
			long r = *curCoord++;
			long c = *curCoord++;
			long area = GlobalMoveMap[0]->calcArea(r, c);
			bool addIt = true;
			for (long j = 0; j < numSubAreas0; j++)
				if (subAreas0[j] == area) 
				{
					addIt = false;
					break;
				}

			if (addIt) 
			{
				if (!subAreas0)
				{
					subAreas0 = (short *)ObjectTypeManager::objectCache->Malloc(sizeof(short) * MAX_SPECIAL_SUB_AREAS);
					memset(subAreas0,0,sizeof(short) * MAX_SPECIAL_SUB_AREAS);
				}

				subAreas0[numSubAreas0++] = area;
			}
		}

		numSubAreas1 = 0;
		curCoord = cellsCovered;
		for (i = 0; i < numCellsCovered; i++) 
		{
			long r = *curCoord++;
			long c = *curCoord++;
			long area = GlobalMoveMap[1]->calcArea(r, c);
			bool addIt = true;
			for (long j = 0; j < numSubAreas1; j++)
				if (subAreas1[j] == area) 
				{
					addIt = false;
					break;
				}

			if (addIt) 
			{
				if (!subAreas1)
				{
					subAreas1 = (short *)ObjectTypeManager::objectCache->Malloc(sizeof(short) * MAX_SPECIAL_SUB_AREAS);
					memset(subAreas1,0,sizeof(short) * MAX_SPECIAL_SUB_AREAS);
				}

				subAreas1[numSubAreas1++] = area;
			}
		}

		for (i = 0; i < numSubAreas0; i++)
			GlobalMoveMap[0]->setAreaOwnerWID(subAreas0[i], getWatchID());

		for (i = 0; i < numSubAreas1; i++)
			GlobalMoveMap[1]->setAreaOwnerWID(subAreas1[i], getWatchID());
	}
}

//---------------------------------------------------------------------------

void TerrainObject::markMoveMap (bool passable) {
	
	short* curCoord = cellsCovered;
	for (long i = 0; i < numCellsCovered; i++) {
		long r = *curCoord++;
		long c = *curCoord++;
		GameMap->setPassable(r, c, passable);
		if (passable)
			GameMap->setLocalHeight(r, c, 0.0f);
	}
}

//---------------------------------------------------------------------------

void TerrainObject::openSubAreas (void) {

	markMoveMap(true);
	for (long i = 0; i < numSubAreas0; i++)
		GlobalMoveMap[0]->openArea(subAreas0[i]);
	for (i = 0; i < numSubAreas1; i++)
		GlobalMoveMap[1]->openArea(subAreas1[i]);
}

//---------------------------------------------------------------------------

void TerrainObject::closeSubAreas (void) {

	markMoveMap(false);
	for (long i = 0; i < numSubAreas0; i++)
		GlobalMoveMap[0]->closeArea(subAreas0[i]);
	for (i = 0; i < numSubAreas1; i++)
		GlobalMoveMap[1]->closeArea(subAreas1[i]);
}

//---------------------------------------------------------------------------

void TerrainObject::setSubAreasTeamId (long id) {

	for (long i = 0; i < numSubAreas0; i++)
		GlobalMoveMap[0]->setAreaTeamID(subAreas0[i], id);
	for (i = 0; i < numSubAreas1; i++)
		GlobalMoveMap[1]->setAreaTeamID(subAreas1[i], id);
}

//---------------------------------------------------------------------------

bool TerrainObject::calcAdjacentAreaCell (long moveLevel, long areaID, long& adjRow, long& adjCol) {

	if (areaID == -1) {
		short* curCoord = cellsCovered;
		for (long i = 0; i < numCellsCovered; i++) {
			long cellRow = *curCoord++;
			long cellCol = *curCoord++;
			long adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow - 1, cellCol);
			if (adjArea > -1) {
				adjRow = cellRow - 1;
				adjCol = cellCol;
				return(true);
			}
			adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow, cellCol + 1);
			if (adjArea > -1) {
				adjRow = cellRow;
				adjCol = cellCol + 1;
				return(true);
			}
			adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow + 1, cellCol);
			if (adjArea > -1) {
				adjRow = cellRow + 1;
				adjCol = cellCol;
				return(true);
			}
			adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow, cellCol - 1);
			if (adjArea > -1) {
				adjRow = cellRow;
				adjCol = cellCol - 1;
				return(true);
			}
		}
		}
	else {
		short* curCoord = cellsCovered;
		for (long i = 0; i < numCellsCovered; i++) {
			long cellRow = *curCoord++;
			long cellCol = *curCoord++;
			long adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow - 1, cellCol);
			if (adjArea == areaID) {
				adjRow = cellRow - 1;
				adjCol = cellCol;
				return(true);
			}
			adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow, cellCol + 1);
			if (adjArea == areaID) {
				adjRow = cellRow;
				adjCol = cellCol + 1;
				return(true);
			}
			adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow + 1, cellCol);
			if (adjArea == areaID) {
				adjRow = cellRow + 1;
				adjCol = cellCol;
				return(true);
			}
			adjArea = GlobalMoveMap[moveLevel]->calcArea(cellRow, cellCol - 1);
			if (adjArea == areaID) {
				adjRow = cellRow;
				adjCol = cellCol - 1;
				return(true);
			}
		}
	}
	return(false);
}

//***************************************************************************
void TerrainObject::Save (PacketFilePtr file, long packetNum)
{
	TerrainObjectData data;
	CopyTo(&data);

	//PacketNum incremented in ObjectManager!!
	file->writePacket(packetNum,(MemoryPtr)&data,sizeof(TerrainObjectData),STORAGE_TYPE_ZLIB);
}

//***************************************************************************
void TerrainObject::CopyTo (TerrainObjectData *data)
{
	data->damage = damage;
	data->vertexNumber = vertexNumber;
	data->blockNumber = blockNumber;
	data->pitchAngle = pitchAngle;
	data->fallRate = fallRate;
	data->powerSupply = powerSupply;
	memcpy(data->cellFootprint,cellFootprint,sizeof(short) * 4);
	memcpy(data->vectorFootprint,vectorFootprint,sizeof(Stuff::Vector3D) * 4);
	data->numSubAreas0 = numSubAreas0;
	data->numSubAreas1 = numSubAreas1;

	if (subAreas0)
		memcpy(data->subAreas0,subAreas0,sizeof(short) * MAX_SPECIAL_SUB_AREAS);
	else
		memset(data->subAreas0,0,sizeof(short) * MAX_SPECIAL_SUB_AREAS);

	if (subAreas1)
		memcpy(data->subAreas1,subAreas1,sizeof(short) * MAX_SPECIAL_SUB_AREAS);
	else
		memset(data->subAreas1,0,sizeof(short) * MAX_SPECIAL_SUB_AREAS);

	data->listID = listID;

	data->numCellsCovered = numCellsCovered;
	if (numCellsCovered >= 162)
		STOP(("Object %d covers too many cells in Save/Load!!",getObjectType()->getObjTypeNum()));

	memcpy(data->cellsCovered,cellsCovered, sizeof(short) * numCellsCovered * 2);

	GameObject::CopyTo(dynamic_cast<GameObjectData *>(data));
}

//---------------------------------------------------------------------------
void TerrainObject::Load (TerrainObjectData *data)
{
	GameObject::Load(dynamic_cast<GameObjectData *>(data)); 

	damage = data->damage;
	vertexNumber = data->vertexNumber;
	blockNumber = data->blockNumber;
	pitchAngle = data->pitchAngle;
	fallRate = data->fallRate;
	powerSupply = data->powerSupply;
	memcpy(cellFootprint,data->cellFootprint,sizeof(short) * 4);
	memcpy(vectorFootprint,data->vectorFootprint,sizeof(Stuff::Vector3D) * 4);
	numSubAreas0 = data->numSubAreas0;
	numSubAreas1 = data->numSubAreas1;

	if (numSubAreas0)
	{
		subAreas0 = (short *)ObjectTypeManager::objectCache->Malloc(sizeof(short) * MAX_SPECIAL_SUB_AREAS);
		memcpy(subAreas0,data->subAreas0,sizeof(short) * MAX_SPECIAL_SUB_AREAS);
	}

	if (numSubAreas1)
	{
		subAreas1 = (short *)ObjectTypeManager::objectCache->Malloc(sizeof(short) * MAX_SPECIAL_SUB_AREAS);
		memcpy(subAreas1,data->subAreas1,sizeof(short) * MAX_SPECIAL_SUB_AREAS);
	}

	listID = data->listID;

	numCellsCovered = data->numCellsCovered;
	if (numCellsCovered)
	{
		cellsCovered = (short*)systemHeap->Malloc(sizeof(short) * numCellsCovered * 2);
		memcpy(cellsCovered, data->cellsCovered,sizeof(short) * numCellsCovered * 2);
	}
}

//***************************************************************************

