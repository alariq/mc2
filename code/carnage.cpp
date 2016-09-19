//***************************************************************************
//
//	carnage.cpp -- File contains the Carnage class code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef CARNAGE_H
#include"carnage.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
#endif

#ifndef TURRET_H
#include"turret.h"
#endif

#ifndef GATE_H
#include"gate.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#include<gameos.hpp>

//---------------------------------------------------------------------------

#define BLIP_FRAME_RATE			0.067
#define	EXPLOSION_TIME			0.5
#define	EXPLOSION_DEPTH_FIXUP	-150

extern unsigned long	NextIdNumber;
extern TeamPtr homeTeam;
extern bool useSound;
extern bool somethingOnFire;

float Carnage::maxFireBurnTime = 5.0;

extern MidLevelRenderer::MLRClipper * theClipper;
extern bool MLRVertexLimitReached;
//***************************************************************************
// FIRE TYPE class
//***************************************************************************

GameObjectPtr FireType::createInstance (void) {

	CarnagePtr newFire = new Carnage;
	if (!newFire)
		return(NULL);

	newFire->init(true, this);

	return(newFire);
}

//---------------------------------------------------------------------------

void FireType::destroy (void) 
{
	if (fireOffsetX) 
	{
		systemHeap->Free(fireOffsetX);
		fireOffsetX = NULL;
	}

	if (fireOffsetY) 
	{
		systemHeap->Free(fireOffsetY);
		fireOffsetY = NULL;
	}
	
	if (fireDelay) 
	{
		systemHeap->Free(fireDelay);
		fireDelay = NULL;
	}

	if (fireRandomOffsetX) 
	{
		systemHeap->Free(fireRandomOffsetX);
		fireRandomOffsetX = NULL;
	}

	if (fireRandomOffsetY) 
	{
		systemHeap->Free(fireRandomOffsetY);
		fireRandomOffsetY = NULL;
	}

	if (fireRandomDelay) 
	{
		systemHeap->Free(fireRandomDelay);
		fireRandomDelay = NULL;
	}

	ObjectType::destroy();
}
		
//---------------------------------------------------------------------------

long FireType::init (FilePtr objFile, unsigned long fileSize) {

	long result = 0;
	
	FitIniFile explFile;
	result = explFile.open(objFile,fileSize);
	if (result != NO_ERR)
		return(result);
	
	result = explFile.seekBlock("FireData");
	if (result != NO_ERR)
		return(result);

	unsigned long dmgLevel;
	result = explFile.readIdULong("DmgLevel",dmgLevel);
	if (result != NO_ERR)
		return(result);
	damageLevel = dmgLevel;
		
	result = explFile.readIdULong("SoundEffectId",soundEffectId);
	if (result != NO_ERR)
		return(result);
	
	result = explFile.readIdULong("LightObjectId",lightObjectId);
	if (result != NO_ERR)
		lightObjectId = -1;

	result = explFile.readIdULong("startLoopFrame",startLoopFrame);
	if (result != NO_ERR)
		return(result);

	result = explFile.readIdULong("numLoops",numLoops);
	if (result != NO_ERR)
		return(result);
		
	result = explFile.readIdULong("endLoopFrame",endLoopFrame);
	if (result != NO_ERR)
		return(result);

	result = explFile.readIdFloat("maxExtentRadius",maxExtent);
	if (result != NO_ERR)		
		maxExtent = 0.0;
	
	result = explFile.readIdFloat("TimeToMaxExtent",timeToMax);
	if (result != NO_ERR)		
		timeToMax = 0.0;

	result = explFile.readIdLong("TotalFireShapes",totalFires);
	if (result != NO_ERR)
		totalFires = 1;

	//------------------------------------------------------------------
	// Initialize the base object Type from the current file.
	result = ObjectType::init(&explFile);

	objectClass = FIRE;

	return(result);
}
	
//---------------------------------------------------------------------------

bool FireType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider) {

	if (MPlayer && !MPlayer->isServer())
		return(false);

	if (!collider->isDestroyed()) 
	{
		switch (collider->getObjectClass()) 
		{
			case BATTLEMECH:
				//-------------------------------------------------------------
				// We should induce heat in a battlemech if it is in the fire.
				break;
			case BUILDING:
				//----------------------------------------------------------------------
				// We should light a building on fire if we have expanded to engulf it.
				// Not anymore.  Fires are undefined at present.
				break;
			case TERRAINOBJECT:
				if (((TerrainObjectPtr)collider)->getSubType() == TERROBJ_TREE) 
				{
					if (RollDice(10)) 
					{
						((TerrainObjectPtr)collider)->lightOnFire(15.0);
						if (MPlayer)
							MPlayer->addLightOnFireChunk(collider, 15);
					}
				}
				break;
		}
	}
	
	return(false);
}

//---------------------------------------------------------------------------

bool FireType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider) {
	return(FALSE);
}

//***************************************************************************
// EXPLOSION TYPE class
//***************************************************************************

GameObjectPtr ExplosionType::createInstance (void) {

	CarnagePtr newExplosion = new Carnage;
	if (!newExplosion)
		return(NULL);

	newExplosion->init(true, this);

	return(newExplosion);
}

//---------------------------------------------------------------------------

void ExplosionType::destroy (void) 
{
	ObjectType::destroy();
}
		
//---------------------------------------------------------------------------

long ExplosionType::init (FilePtr objFile, unsigned long fileSize) {

	long result = 0;
	
	FitIniFile explFile;
	result = explFile.open(objFile,fileSize);
	if (result != NO_ERR)
		return(result);
	
	//------------------------------------------------------------------
	// Read in the data needed to fire the laser
	result = explFile.seekBlock("ExplosionData");
	if (result != NO_ERR)
		return(result);

	unsigned long dmgLevel;
	result = explFile.readIdULong("DmgLevel",dmgLevel);
	if (result != NO_ERR)
		return(result);
	damageLevel = dmgLevel;
		
	result = explFile.readIdULong("SoundEffectId",soundEffectId);
	if (result != NO_ERR)
		return(result);
		
	result = explFile.readIdLong("ExplosionRadius",explRadius);
	if (result != NO_ERR)
		explRadius = 0;
		
	result = explFile.readIdULong("LightObjectId",lightObjectId);
	if (result != NO_ERR)
		lightObjectId = -1;

	result = explFile.readIdFloat("DamageChunkSize",chunkSize);
	if (result != NO_ERR)
		chunkSize = 5.0;

	result = explFile.readIdFloat("DelayUntilCollidable",delayUntilCollidable);
	if (result != NO_ERR)
		delayUntilCollidable = EXPLOSION_TIME;

	result = explFile.seekBlock("LightData");
	if (result == NO_ERR)
	{
		result = explFile.readIdFloat("LightMinMaxRadius",lightMinMaxRadius);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));

		result = explFile.readIdFloat("LightMaxMaxRadius",lightMaxMaxRadius);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));

		result = explFile.readIdFloat("LightOutMinRadius",lightOutMinRadius);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));

		result = explFile.readIdFloat("LightOutMaxRadius",lightOutMaxRadius);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));

		result = explFile.readIdULong("LightRGB",lightRGB);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));

		result = explFile.readIdFloat("MaxIntensity",maxIntensity);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));

		result = explFile.readIdFloat("MinIntensity",minIntensity);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));

		result = explFile.readIdFloat("Duration",duration);
		if (result != NO_ERR)
			STOP(("Light data for explosion BAD"));
	}

	//------------------------------------------------------------------
	// Initialize the base object Type from the current file.
	result = ObjectType::init(&explFile);

	objectClass = EXPLOSION;

	return(result);
}
	
//---------------------------------------------------------------------------

bool ExplosionType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider) {

	if (MPlayer && !MPlayer->isServer())
		return(false);

	float damageLeft = collidee->getExplDmg();

	//-------------------------------------------------------
	// Explosions CAN damage objects around them if they have
	// a damage level associated with them.
	if (damageLeft != 0.0) {
		float damageAmount = 0.0;
		WeaponShotInfo shot;
		if (damageLeft > ((CarnagePtr)collidee)->info.explosion.chunkSize)
			damageAmount = ((CarnagePtr)collidee)->info.explosion.chunkSize;
		else
			damageAmount = damageLeft;

		if (collider->isMover()) {
			shot.init(NULL, -1, damageAmount, 0, 0);		 
			while (damageLeft > 0.0) {
				shot.hitLocation = collider->calcHitLocation(collidee,-1,ATTACKSOURCE_ARTILLERY,0);
				collider->handleWeaponHit(&shot, (MPlayer != NULL));
				damageLeft -= ((CarnagePtr)collidee)->info.explosion.chunkSize;
			}
			}
		else {
			switch (collider->getObjectClass()) 
			{
				case GATE: 
				{
					//---------------------------------------------------
					// An explosion may NOT damage a gate or turret unless
					// is is within the "little" extent radius.
					Stuff::Vector3D distance = collider->getPosition();
					distance -= collidee->getPosition();
					float range = distance.GetLength();

			 		float littleExtent = ((TurretPtr)collider)->getLittleExtent();
					if (littleExtent < range) 
					{
						float realRange = range - littleExtent;
						if (realRange > collidee->getExtentRadius())
							return(false);
					}
					
					shot.init(NULL, -1, damageAmount, 0, 0);		 
					while (damageLeft > 0.0) 
					{
						shot.hitLocation = 0;
						collider->handleWeaponHit(&shot, (MPlayer != NULL));
						damageLeft -= chunkSize;
					}
				}
				break;

				case TURRET: 
				{
					//---------------------------------------------------
					// An explosion may NOT damage a gate or turret unless
					// is is within the "little" extent radius.
					Stuff::Vector3D distance = collider->getPosition();
					distance -= collidee->getPosition();
					float range = distance.GetLength();

					float littleExtent = ((TurretPtr)collider)->getLittleExtent();
					if (littleExtent < range) 
					{
						float realRange = range - littleExtent;
						if (realRange > collidee->getExtentRadius())
							return(FALSE);
					}
					
					float damageLeft = collidee->getExplDmg();
					WeaponShotInfo shot;
					shot.init(NULL, -1, damageAmount, 0, 0);		 
					while (damageLeft > 0.0) 
					{
						shot.hitLocation = 0;
						collider->handleWeaponHit(&shot, (MPlayer != NULL));
						damageLeft -= chunkSize;

					}
							
					}
					break;

				default: 
				{
					WeaponShotInfo shot;
					shot.init(NULL, -1, collidee->getExplDmg(), 0, 0);
					collider->handleWeaponHit(&shot, (MPlayer != NULL));
				}
			}
		}
	}
	
	return(false);
}

//---------------------------------------------------------------------------

bool ExplosionType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider) {

	//-------------------------------------------------------
	// The laser ceases to exist when its effect is done.
	// always return FALSE or the collision will make it
	// go away!
	
	return(false);
}

//***************************************************************************
// CARNAGE class
//***************************************************************************
void Carnage::init (bool create) 
{
	carnageType = CARNAGE_NONE;
	ownerWID = 0;
	info.fire.timeToBurn = 0.0;
	info.fire.radius = 0.0;
	info.fire.reallyVisible = 0;
	info.explosion.timer = EXPLOSION_TIME;
	info.explosion.chunkSize = 0.0;
	gosEffect = NULL;

	intensity = 0.0f;
	inRadius = 0.0f;
	outRadius = 0.0f;
	duration = 0.0f;

	pointLight = NULL;
	lightId = 0xffffffff;
}

//---------------------------------------------------------------------------
void Carnage::init (CarnageEnumType _carnageType) 
{
	//--------------------------------------------------------------------
	// This function not only sets the carnage type, but also re-inits the
	// object so it may be used again in the game without re-allocating
	// it...
	carnageType = _carnageType;
	if (carnageType == CARNAGE_FIRE) 
	{
	
	}
	else if (carnageType == CARNAGE_EXPLOSION) 
	{
		intensity = 0.0f;
		inRadius = 0.0f;
		outRadius = 0.0f;
		duration = 0.0f;

		pointLight = NULL;
		lightId = 0xffffffff;
	}
}

//---------------------------------------------------------------------------
GameObjectPtr Carnage::getOwner (void) 
{
	return(ObjectManager->getByWatchID(ownerWID));
}

//---------------------------------------------------------------------------
void Carnage::handleStaticCollision (void) 
{
	if (getTangible()) 
	{
		//-----------------------------------------------------
		// What is our block and vertex number?
		long blockNumber = 0;
		long vertexNumber = 0;
		getBlockAndVertexNumber(blockNumber,vertexNumber);
			
		//-------------------------------------------------------------------------
		// We must now move out into other tiles for the artillery strike to work.
		// Remember, Its pretty big!
		// Just grab the nine vertices around this one.  Problems arise when on Block border.  Handle it.
		long topLeftBlockNumber = blockNumber - Terrain::blocksMapSide - 1;
		long currentBlockNumber = topLeftBlockNumber;
		long totalBlocks = Terrain::blocksMapSide * Terrain::blocksMapSide;

		for (long i = 0; i < 3; i++) 
		{
			for (long j = 0; j < 3; j++) 
			{
				if ((currentBlockNumber >= 0) && (currentBlockNumber < totalBlocks))
				{
					long numObjectsInBlock = ObjectManager->getObjBlockNumObjects(currentBlockNumber);
					for (long objIndex = 0; objIndex < numObjectsInBlock; objIndex++) 
					{
						GameObjectPtr obj = ObjectManager->getObjBlockObject(currentBlockNumber, objIndex);
						if (obj->getExists() && (obj->getObjectClass() != GATE) && (obj->getObjectClass() != TURRET)) 
							ObjectManager->detectStaticCollision(this, obj);
					}
				}
				currentBlockNumber++;
			}
			currentBlockNumber = topLeftBlockNumber + (Terrain::blocksMapSide * (i + 1));
		}

		//-------------------------------------------------------
		// MUST figure out radius to set off mines in CELLS now.
		// UNLESS WE ARE A MINE EXPLOSION!!!
		// -fs
		if ((effectId != MINE_EXPLOSION_ID) && (info.explosion.radius > 0.0f))
		{
			long CellRow, CellCol;
			land->worldToCell(getPosition(), CellRow, CellCol);
	
			long startCellRow = CellRow - 4;
			long startCellCol = CellCol - 4;
				
			for (i = startCellRow; i < startCellRow + 9; i++) 
			{
				for (long j = startCellCol; j < startCellCol + 9; j++) 
				{
					if (GameMap->inBounds(i,j)) 
					{
						long mineResult = 0;
						mineResult = GameMap->getMine(i,j);
						
						if (mineResult == 1)
						{
							Stuff::Vector3D minePosition;
							land->cellToWorld(i,j,minePosition);
							Stuff::Vector3D distance;
							distance.Subtract(getPosition(),minePosition);
							distance.z = 0.0f;
							float dist = distance.GetApproximateLength();
							if (dist < info.explosion.radius)
							{
								GameMap->setMine(i,j,2);
								if (MPlayer) 
								{
									MPlayer->addMineChunk(i,
														  j,
														  1,
														  2,
														  2);
								}
								ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, minePosition, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
							}
						}
					}
				}
			}
		}
	}
}	

//---------------------------------------------------------------------------
bool Carnage::onScreen (void) 
{
	//----------------------------------------------------------------------
	// Need a real check here against the OBB Radius of the effect
	
	return(true);
}

//---------------------------------------------------------------------------
void Carnage::finishNow (void) 
{
	//When the fire goes out, it may spread.
	if (carnageType == CARNAGE_FIRE) 
	{
		setFlag(OBJECT_FLAG_SPREAD, true);
	}
	else if (carnageType == CARNAGE_EXPLOSION) 
	{
		if (gosEffect)
		{
			gosEffect->Kill();		//Effect is over.
			delete gosEffect;
			gosEffect = NULL;
		}

		if (pointLight)
		{
			eye->removeWorldLight(lightId,pointLight);
			systemHeap->Free(pointLight);
			pointLight = NULL;
		}

		init(false);
	}
}	
		
//---------------------------------------------------------------------------
void Carnage::addTimeLeft (float timeLeft) 
{
	if (carnageType == CARNAGE_FIRE) 
	{
	}
	else if (carnageType == CARNAGE_EXPLOSION) 
	{
	}
}

//---------------------------------------------------------------------------
long Carnage::update (void) 
{
	if (carnageType == CARNAGE_FIRE) 
	{
	}
	else if (carnageType == CARNAGE_EXPLOSION) 
	{
		if (getFlag(OBJECT_FLAG_JUSTCREATED)) 
		{
			setFlag(OBJECT_FLAG_JUSTCREATED, false);
				
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
				
			Stuff::Point3D actualPosition;
			actualPosition.x = -position.x;
			actualPosition.y = position.z;
			actualPosition.z = position.y;
	
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(actualPosition);
			
			if (gosEffect && gosEffect->IsExecuted())
			{
				gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
				gosEffect->Start(&info);
			}
			
			setExists(true);
			
			//------------------------------------------
			// If just created, play our sound effect...
			if (((ExplosionTypePtr)getObjectType())->soundEffectId != -1)
				soundSystem->playDigitalSample(((ExplosionTypePtr)getObjectType())->soundEffectId,position,true);

			if (((ExplosionTypePtr)getObjectType())->duration > 0.0f)
			{
				// Explosion Has Point Light Source
				// Make and Init it here.
				pointLight = (TG_LightPtr)systemHeap->Malloc(sizeof(TG_Light));
				pointLight->init(TG_LIGHT_POINT);

				pointLight->SetaRGB(((ExplosionTypePtr)getObjectType())->lightRGB);
				intensity = ((ExplosionTypePtr)getObjectType())->maxIntensity;
				outRadius = ((ExplosionTypePtr)getObjectType())->lightOutMaxRadius;
				inRadius = ((ExplosionTypePtr)getObjectType())->lightMaxMaxRadius;
				pointLight->SetIntensity(intensity);
				pointLight->SetFalloffDistances(inRadius,outRadius);

				lightId = eye->addWorldLight(pointLight);
				duration = ((ExplosionTypePtr)getObjectType())->duration;

				if (lightId == -1)
				{
					//NO LIGHT ALLOWED!  TOO Many in World!
					// LightId is now -1 which will cause this to NEVER make a light!
					systemHeap->Free(pointLight);
					pointLight = NULL;
					duration = 0.0f;
				}
			}
		}
		
		//------------------------------------------------
		// Attach explosion to thing dying if we need to.
		if (ownerWID)
		{
			position = ObjectManager->getByWatchID(ownerWID)->getPosition();
		}
		
		//------------------------------------------------
		// Update pointLight if its not NULL
		if (pointLight)
		{
			if (duration > 0.0f)
			{
				float timeFactor = duration / ((ExplosionTypePtr)getObjectType())->duration;
				intensity = ((ExplosionTypePtr)getObjectType())->minIntensity + 
							(((ExplosionTypePtr)getObjectType())->maxIntensity - ((ExplosionTypePtr)getObjectType())->minIntensity) * timeFactor;
				outRadius = ((ExplosionTypePtr)getObjectType())->lightOutMinRadius + 
							(((ExplosionTypePtr)getObjectType())->lightOutMaxRadius - ((ExplosionTypePtr)getObjectType())->lightOutMinRadius) * timeFactor;
				inRadius = ((ExplosionTypePtr)getObjectType())->lightMinMaxRadius + 
							(((ExplosionTypePtr)getObjectType())->lightMaxMaxRadius - ((ExplosionTypePtr)getObjectType())->lightMinMaxRadius) * timeFactor;

				pointLight->SetIntensity(intensity);
				pointLight->SetFalloffDistances(inRadius,outRadius);

				duration -= frameLength;
				
				Stuff::Point3D ourPosition;
				ourPosition.x = -position.x;
				ourPosition.y = position.z;
				ourPosition.z = position.y;
			
				pointLight->direction = ourPosition;
			
				Stuff::LinearMatrix4D lightToWorldMatrix;
				lightToWorldMatrix.BuildTranslation(ourPosition);
				lightToWorldMatrix.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				pointLight->SetLightToWorld(&lightToWorldMatrix);
				pointLight->SetPosition(&position);
			}
			else
			{
				eye->removeWorldLight(lightId,pointLight);
				systemHeap->Free(pointLight);
				pointLight = NULL;
			}
		}

		setFlag(OBJECT_FLAG_TANGIBLE, false);

		if (info.explosion.timer > 0.0f)
		{
			info.explosion.timer -= frameLength;
			if (info.explosion.timer <= 0.0f)
			{
				setFlag(OBJECT_FLAG_TANGIBLE, true);
			}
		}

		//------------------------------------------------
		// Update GOSFX
		if (gosEffect && gosEffect->IsExecuted())
		{
			Stuff::Point3D actualPosition;
			actualPosition.x = -position.x;
			actualPosition.y = position.z;
			actualPosition.z = position.y;
			
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D 	localToWorld;
			   
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(actualPosition);
			localToWorld.Multiply(shapeOrigin,gosFX::Effect_Into_Motion);
		
 			Stuff::OBB boundingBox;
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
			
			bool result = gosEffect->Execute(&info);
			if (!result)
			{
				gosEffect->Kill();		//Effect is over.
				delete gosEffect;
				gosEffect = NULL;

				if (pointLight)
				{
					eye->removeWorldLight(lightId,pointLight);
					systemHeap->Free(pointLight);
					pointLight = NULL;
				}
		
				return false;			//Tell the object system to stop updating, too.
			}
		}
		else if (info.explosion.timer <= 0.0f)	//If we had no GosEffect, make sure we've collided and then move on.
		{
			if (pointLight)
			{
				eye->removeWorldLight(lightId,pointLight);
				systemHeap->Free(pointLight);
				pointLight = NULL;
			}

			return false;
		}
	}
	
	return (true);
}

//---------------------------------------------------------------------------

void Carnage::render (void) 
{
	if (carnageType == CARNAGE_FIRE) 
	{
	}
	else if (carnageType == CARNAGE_EXPLOSION) 
	{
		if (gosEffect && gosEffect->IsExecuted() && !getFlag(OBJECT_FLAG_JUSTCREATED))
		{
			gosFX::Effect::DrawInfo drawInfo;
			drawInfo.m_clipper = theClipper;
			
			MidLevelRenderer::MLRState mlrState;
			mlrState.SetDitherOn();
			mlrState.SetTextureCorrectionOn();
			mlrState.SetZBufferCompareOn();
			mlrState.SetZBufferWriteOn();
	
			drawInfo.m_state = mlrState;
			drawInfo.m_clippingFlags = 0x0;
	
			Stuff::Point3D actualPosition;
			actualPosition.x = -position.x;
			actualPosition.y = position.z;
			actualPosition.z = position.y;
			
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(actualPosition);
			
			drawInfo.m_parentToWorld = &shapeOrigin;
	 
			if (!MLRVertexLimitReached)
				gosEffect->Draw(&drawInfo);
		}
	}
}

//---------------------------------------------------------------------------

void Carnage::destroy (void) 
{
	if (carnageType == CARNAGE_FIRE) 
	{
	}
	else if (carnageType == CARNAGE_EXPLOSION) 
	{
		if (gosEffect)
		{
			gosEffect->Kill();		//Effect is over.
			delete gosEffect;
			gosEffect = NULL;
		}

		if (pointLight)
		{
			if (eye)
				eye->removeWorldLight(lightId,pointLight);
			systemHeap->Free(pointLight);
			pointLight = NULL;
		}
	}
}

//---------------------------------------------------------------------------

void Carnage::init (bool create, ObjectTypePtr _type) 
{
	GameObject::init(true, _type);

	gosEffect = NULL;
	pointLight = NULL;
	setFlag(OBJECT_FLAG_JUSTCREATED, true);
	
	if (carnageType == CARNAGE_FIRE)
	{
	}
	else if (carnageType == CARNAGE_EXPLOSION) 
	{
		info.explosion.chunkSize = ((ExplosionType *)_type)->chunkSize;
		info.explosion.timer = ((ExplosionType *)_type)->delayUntilCollidable; 
			
		if (stricmp(weaponEffects->GetEffectName(effectId),"NONE") != 0)
		{
			//--------------------------------------------------------------
			// We need to create the GOSFX we need for this explosion here.
			// The name of the GOSFX is stored as the appearanceName in the Type.
			// ONLY Execute.  Explosions just run once!
			unsigned flags = gosFX::Effect::ExecuteFlag;
	
			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(effectId));
			
			gosEffect = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
			gosASSERT(gosEffect != NULL);
			
			MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
		}

		setFlag(OBJECT_FLAG_TANGIBLE, false);
	}
}	

//---------------------------------------------------------------------------
void Carnage::Save (PacketFilePtr file, long packetNum)
{
	CarnageData data;
	CopyTo(&data);

	//PacketNum incremented in ObjectManager!!
	file->writePacket(packetNum,(MemoryPtr)&data,sizeof(CarnageData),STORAGE_TYPE_ZLIB);
}

//---------------------------------------------------------------------------
void Carnage::CopyTo (CarnageData *data)
{
	data->carnageType = carnageType;
	data->ownerWID = ownerWID;
	data->info = info;

	data->effectId = effectId;

	GameObject::CopyTo(dynamic_cast<GameObjectData *>(data));
}

//---------------------------------------------------------------------------
void Carnage::Load (CarnageData *data)
{
	GameObject::Load(dynamic_cast<GameObjectData *>(data)); 

	carnageType = data->carnageType;
	ownerWID = data->ownerWID;
	info = data->info;

	effectId = data->effectId;
}

//---------------------------------------------------------------------------

