//---------------------------------------------------------------------------
//
// weaponbolt.cpp -- File contains the WeaponBolt Object code
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

#ifndef WEAPONBOLT_H
#include"weaponbolt.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef WEAPONFX_H
#include"weaponfx.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#include<gameos.hpp>

//---------------------------------------------------------------------------
extern bool hasGuardBand;
extern MidLevelRenderer::MLRClipper * theClipper;
extern bool drawOldWay;
extern float BaseHeadShotElevation;
extern bool useNonWeaponEffects;
extern bool MLRVertexLimitReached;

#define BASE_EFFECT_SCALAR		1.0f
#define EFFECT_LEFT_TIME		1.0f
#define WEAPON_ARC_FACTOR		-20.0f
#define WEAPON_ARC_HEIGHT		150.0f
#define ARC_THRESHOLD			0.0f
//---------------------------------------------------------------------------
// class WeaponBoltType
//---------------------------------------------------------------------------
GameObjectPtr WeaponBoltType::createInstance (void)
{
	WeaponBoltPtr result = new WeaponBolt;
	if (!result)
		return NULL;

	result->init(true, this);
		
	return(result);
}

//---------------------------------------------------------------------------
void WeaponBoltType::destroy (void)
{
	if (textureName)
		ObjectTypeManager::objectCache->Free(textureName); 
		
	textureName = NULL;

	ObjectType::destroy();
}

//---------------------------------------------------------------------------
DWORD bgrTorgb (DWORD frontRGB);
	
//---------------------------------------------------------------------------
long WeaponBoltType::init (FilePtr objFile, unsigned long fileSize)
{
	long result = 0;
	
	FitIniFile bullFile;
	result = bullFile.open(objFile,fileSize);
	if (result != NO_ERR)
		Fatal(result,"WeaponBoltType::init -- Unable to open File");
	
	//------------------------------------------------------------------
	result = bullFile.seekBlock("BoltProjectileData");
	if (result != NO_ERR)
	{
		//Check if we are a NEW beam type.
		result = bullFile.seekBlock("BeamProjectileData");
		if (result != NO_ERR)
		{
			STOP(("WeaponBoltTYpe::init -- Not a valid weaponBolt"));
		}
		
		result = bullFile.readIdBoolean("IsBeam",isBeam);
		if (result != NO_ERR)
			isBeam = false;
			
		result = bullFile.readIdFloat("BeamDuration",beamDuration);
		if (result != NO_ERR)
			beamDuration = 0.0f;
			
		result = bullFile.readIdFloat("BeamWiggle",beamWiggle);
		if (result != NO_ERR)
			beamWiggle = 0.0f;
			
		result = bullFile.readIdBoolean("PointLight",lightSource);
		if (result != NO_ERR)
			lightSource = false;
	
		char txmName[1024];
		result = bullFile.readIdString("TextureName",txmName,1023);
		if (result != NO_ERR)
			strcpy(txmName,"NONE");
		
		textureName = (char *)ObjectTypeManager::objectCache->Malloc(strlen(txmName)+1); 
		strcpy(textureName,txmName);

		result = bullFile.readIdFloat("UVAnimRate",uvAnimRate);
		if (result != NO_ERR)
			uvAnimRate = 0.0f;
			
		result = bullFile.readIdFloat("UVRepeat",uvRepeat);
		if (result != NO_ERR)
			uvRepeat = 1.0f;
			
		result = bullFile.readIdFloat("UnitLength",unitLength);
		if (result != NO_ERR)
			unitLength = 1.0f;
			
		if (unitLength < Stuff::SMALL)
			unitLength = 1.0f;

		result = bullFile.readIdBoolean("MipTexture",mipTexture);
		if (result != NO_ERR)
			mipTexture = false;
			
 	 	result = bullFile.readIdLong("FiringSoundFX",fireSoundFX);
		if (result != NO_ERR)
			fireSoundFX = 0;
			
		result = bullFile.readIdFloat("BulgeWidth",bulgeWidth);
		if (result != NO_ERR)
			bulgeWidth = 1.0f;
			
		result = bullFile.readIdULong("MidEdgeRGB",midEdgeRGB);
		if (result != NO_ERR)
			midEdgeRGB = 0x00ffffff;
		
		result = bullFile.readIdULong("MiddleRGB",middleRGB);
		if (result != NO_ERR)
			middleRGB = 0x00ffffff;
				
		result = bullFile.readIdUChar("EdgeAlpha",edgeAlpha);
		if (result != NO_ERR)
			edgeAlpha = 0xff;
			
		result = bullFile.readIdUChar("BoltAlpha",boltAlpha);
		if (result != NO_ERR)
			boltAlpha = 0xff;
		
		projLength = 1.0f;
		velocity = 1.0f;
					
   		if (lightSource)
		{
			result = bullFile.readIdULong("LightRGB",lightRGB);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find lightRGB");
	
			result = bullFile.readIdFloat("MaxRadius",maxRadius);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find maxRadius");
	
			result = bullFile.readIdFloat("OutRadius",outRadius);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find outRadius");
	
			result = bullFile.readIdFloat("Intensity",intensity);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find intensity");
		}
	}
	else
	{
		result = bullFile.readIdFloat("ProjectileLength",projLength);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find projectileLength");
				
		result = bullFile.readIdFloat("BulgeLength",bulgeLength);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find bulgeLength");
			
		result = bullFile.readIdFloat("BulgeWidth",bulgeWidth);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find bulgeWidth");
		
		result = bullFile.readIdFloat("Velocity",velocity);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find velocity");
		
		result = bullFile.readIdLong("HitEffect",hitEffectObjNum);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find HitEffect");
		
		result = bullFile.readIdLong("MissEffect",missEffectObjNum);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find missEffect");
		
		result = bullFile.readIdLong("FiringSoundFX",fireSoundFX);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find FiringSoundFx");
		
		result = bullFile.readIdULong("FrontRGB",frontRGB);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find FrontRGB");
			
		result = bullFile.readIdULong("MidEdgeRGB",midEdgeRGB);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find MIDEDGERGB");
		
		result = bullFile.readIdULong("MiddleRGB",middleRGB);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find MiddleRGB");
		
		result = bullFile.readIdULong("BackRGB",backRGB);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find backRGB");
			
		result = bullFile.readIdUChar("EdgeAlpha",edgeAlpha);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find edgeAlpha");
			
		result = bullFile.readIdUChar("BoltAlpha",boltAlpha);
		if (result != NO_ERR)
			Fatal(result,"WeaponBoltType::init -- Unable to find boltAlpha");
		
		result = bullFile.readIdBoolean("IsBeam",isBeam);
		if (result != NO_ERR)
			isBeam = false;
			
		result = bullFile.readIdFloat("BeamDuration",beamDuration);
		if (result != NO_ERR)
			beamDuration = 0.0f;
			
		result = bullFile.readIdFloat("BeamWiggle",beamWiggle);
		if (result != NO_ERR)
			beamWiggle = 0.0f;
			
		result = bullFile.readIdBoolean("PointLight",lightSource);
		if (result != NO_ERR)
			lightSource = false;

		result = bullFile.readIdBoolean("ArcEffect",arcEffect);
		if (result != NO_ERR)
			arcEffect = false;
			
		result = bullFile.readIdFloat("ArcHeight",arcHeight);
		if (result != NO_ERR)
			arcHeight = -1.0f;

		result = bullFile.readIdFloat("AfterHitTime",afterHitTime);
		if (result != NO_ERR)
			afterHitTime = 0.0f;
			
		result = bullFile.readIdFloat("AreaEffectDamage",areaEffectDmg);
		if (result != NO_ERR)
			areaEffectDmg = 0.0f;
			
		result = bullFile.readIdFloat("AreaEffectRadius",areaEffectRad);
		if (result != NO_ERR)
			areaEffectRad = 0.0f;
			
   		if (lightSource)
		{
			result = bullFile.readIdULong("LightRGB",lightRGB);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find lightRGB");
	
			result = bullFile.readIdFloat("MaxRadius",maxRadius);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find maxRadius");
	
			result = bullFile.readIdFloat("OutRadius",outRadius);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find outRadius");
	
			result = bullFile.readIdFloat("Intensity",intensity);
			if (result != NO_ERR)
				Fatal(result,"WeaponBoltType::init -- Unable to find intensity");
		}
	
		//------------------------------------------------------------------
		// Editor writes RGBs as BGRs.  Change over here.
		frontRGB = bgrTorgb(frontRGB);
		backRGB = bgrTorgb(backRGB);
		middleRGB = bgrTorgb(middleRGB);
		midEdgeRGB = bgrTorgb(midEdgeRGB);
		lightRGB = bgrTorgb(lightRGB);
	}
	
	//------------------------------------------------------------------
	// Initialize the base object Type from the current file.
	result = ObjectType::init(&bullFile);
	
	return(result);
}
	
//---------------------------------------------------------------------------
bool WeaponBoltType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider)
{
	//-------------------------------------------------------
	// The bolt ceases to exist when its effect is done.
	// always return FALSE or the collision will make it
	// go away!
	
	return(FALSE);
}

//---------------------------------------------------------------------------
bool WeaponBoltType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider)
{
	//-------------------------------------------------------
	// The bolt ceases to exist when its effect is done.
	// always return FALSE or the collision will make it
	// go away!
	
	return(FALSE);
}

//---------------------------------------------------------------------------
TG_LightPtr WeaponBoltType::getLight (void)
{
	TG_LightPtr pointLight = NULL;

	if (lightSource)
	{
		pointLight = (TG_LightPtr)systemHeap->Malloc(sizeof(TG_Light));
		pointLight->init(TG_LIGHT_POINT);

		pointLight->SetaRGB(lightRGB);
		pointLight->SetIntensity(intensity);
		pointLight->SetFalloffDistances(maxRadius,outRadius);
	}

	return(pointLight);
}	

//---------------------------------------------------------------------------
// class WeaponBolt
//---------------------------------------------------------------------------
bool WeaponBolt::isVisible (void)
{
	//----------------------------------------------------------------------
	// This function checks to see if this weaponBolt is visible on screen
	bool isVisible = true;
	if (isVisible)
	{
		windowsVisible = turn;
	}
		
	return(true);
}

//---------------------------------------------------------------------------
long WeaponBolt::update (void)
{
	Stuff::Vector3D laserDirection;
	
	bool moveThisFrame = true;
	if (getFlag(OBJECT_FLAG_JUSTCREATED)) 
	{
		setFlag(OBJECT_FLAG_JUSTCREATED, false);
		setFlag(OBJECT_FLAG_TANGIBLE, false);
		
		hitTarget = false;
		moveThisFrame = false;
		//-------------------------------------------------------------
		// Update position relative to my owner
		GameObjectPtr myOwner = ObjectManager->getByWatchID(ownerWID);
		if (myOwner)
		{
			position = myOwner->getPositionFromHS(hotSpotNumber);
		}

		//-------------------------------------------------------------------
		// If we were just Created, play our sound Effect.
		if (((WeaponBoltTypePtr)getObjectType())->fireSoundFX != 0)
			soundSystem->playDigitalSample(((WeaponBoltTypePtr)getObjectType())->fireSoundFX, position, true);
			
		laserPosition = position;
		
		Stuff::Vector3D ownerPosition = laserPosition;
		Stuff::Point3D actualPosition;
		actualPosition.x = -laserPosition.x;
		actualPosition.y = laserPosition.z;
		actualPosition.z = laserPosition.y;

		GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
			
		Stuff::Vector3D targetPos;
		targetPos.Zero();
		if (target)
		{
			if (!target->isMech())
			{
				setTargetPosition(target->getPositionFromHS(targetHotSpot));
			}
			else
			{
				if (targetHotSpot)
					setTargetPosition(target->appearance->getHitNodeLeft());
				else
					setTargetPosition(target->appearance->getHitNodeRight()); 
			}
		}
			
		if (targetPosition)
		{
			targetPos = *targetPosition;
		}
		else
		{
			//Somehow, we are updating a fresh weaponbolt which has not been connected.
			// Do not allow it to go on or we will crash for sure.
			return false;
		}
					
		laserDirection.Subtract(targetPos,ownerPosition);

		if (laserDirection.GetLength() != 0.0)
			laserDirection.Normalize(laserDirection);

		laserDirection *= ((WeaponBoltTypePtr)getObjectType())->projLength;
		//laserPosition.Add(laserPosition,laserDirection);
		
		if (((WeaponBoltTypePtr)getObjectType())->isBeam)
		{
			timeLeft = ((WeaponBoltTypePtr)getObjectType())->beamDuration;
			actualPosition.x = -(*targetPosition).x;	//Play the beam hitting effect at the hit location
			actualPosition.y = (*targetPosition).z;
			actualPosition.z = (*targetPosition).y;
		}
		else
			timeLeft = 0.0f;
			
		if (gosEffect && gosEffect->IsExecuted())
		{
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(actualPosition);
			
			rotation = -world_angle_between(position, *targetPosition);

			Stuff::UnitQuaternion effectRot;
			effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
			localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
			
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,NULL);
			gosEffect->Start(&info);
		}
		
		if (muzzleEffect && muzzleEffect->IsExecuted())
		{
			Stuff::Point3D			mPosition;
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;
			
			mPosition.x = -position.x;
			mPosition.y = position.z;
			mPosition.z = position.y;
			
 			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(mPosition);
			
			Stuff::UnitQuaternion effectRot;
			if (myOwner)
				effectRot = Stuff::EulerAngles(0.0f, myOwner->getRotation() * DEGREES_TO_RADS,0.0f);
			else
				effectRot = Stuff::EulerAngles(0.0f, 0.0f ,0.0f);

			localToWorld.Multiply(gosFX::Effect_Into_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
			
 			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,NULL);
			muzzleEffect->Start(&info);
		}
		
		Stuff::Vector3D laserVelocity;
		laserVelocity.Subtract(targetPos,ownerPosition);
		float distance = laserVelocity.x * laserVelocity.x + laserVelocity.y * laserVelocity.y;
		distanceToTarget = distance;
		halfDistanceToTarget = distance / 2.0f;
		
		goalHeight = ((WeaponBoltTypePtr)getObjectType())->arcHeight;
		goalHeight *= 10.0f;
	}

	//-------------------------------------------------------------
	// Update position relative to my owner
	GameObjectPtr myOwner = ObjectManager->getByWatchID(ownerWID);
	if (myOwner)
		position = myOwner->getPositionFromHS(hotSpotNumber);
	
	bool inView = TRUE;

	//---------------------
	//Update beamDuration
	if (((WeaponBoltTypePtr)getObjectType())->isBeam)
	{
		timeLeft -= frameLength;
		if (timeLeft >= 0.0f)
		{
			gosTextureHandle = mcTextureManager->get_gosTextureHandle(mcTextureHandle);
   			mcTextureManager->addTriangle(mcTextureHandle,MC2_ISEFFECTS|MC2_DRAWONEIN);
   			mcTextureManager->addTriangle(mcTextureHandle,MC2_ISEFFECTS|MC2_DRAWONEIN); 
   			mcTextureManager->addTriangle(mcTextureHandle,MC2_ISEFFECTS|MC2_DRAWONEIN);
   			mcTextureManager->addTriangle(mcTextureHandle,MC2_ISEFFECTS|MC2_DRAWONEIN); 
		}

		startUV += ((WeaponBoltTypePtr)getObjectType())->uvAnimRate * frameLength;
	}
	else if (!gosEffect && !((WeaponBoltTypePtr)getObjectType())->isBeam)
	{
		mcTextureManager->addTriangle(0xffffffff,MC2_DRAWALPHA);
		mcTextureManager->addTriangle(0xffffffff,MC2_DRAWALPHA);
		mcTextureManager->addTriangle(0xffffffff,MC2_DRAWALPHA);
		mcTextureManager->addTriangle(0xffffffff,MC2_DRAWALPHA);
	}
		
	//---------------------------------------------------------------
	// Keep projectile flying toward target.  Even if Target Moves!!
	Stuff::Vector3D ownerPosition = laserPosition;
	GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
		
	Stuff::Vector3D targetPos;
	targetPos.Zero();
	if (target && (!hitTarget || ((WeaponBoltTypePtr)getObjectType())->isBeam))
	{
		if (!target->isMech())
		{
			setTargetPosition(target->getPositionFromHS(targetHotSpot));
		}
		else
		{
			if (targetHotSpot)
				setTargetPosition(target->appearance->getHitNodeLeft());
			else
				setTargetPosition(target->appearance->getHitNodeRight()); 
		}
	}
		
	if (targetPosition)
	{
		targetPos = *targetPosition;
	}
				
	Stuff::Vector3D laserVelocity;
	float velMag = ((WeaponBoltTypePtr)getObjectType())->velocity;
	velMag *= frameLength;
		
	laserVelocity.Subtract(targetPos,ownerPosition);
	float distance = laserVelocity.x * laserVelocity.x + laserVelocity.y * laserVelocity.y;
	
	//----------------------------------------------------------------------
	// For the first half of the distance, bring weapons up if NOT beam!!!
	bool isArcing = false;
	if (((WeaponBoltTypePtr)getObjectType())->arcEffect)
	{
		//GoalHeight is actually the velocity
		if ( goalHeight > 0.0f )
		{
			float accel = -((WeaponBoltTypePtr)getObjectType())->arcHeight * 10.0f;
			goalHeight += accel * frameLength;
			if (goalHeight >= 0.0f)
			{
				laserVelocity.z = goalHeight;
				isArcing = true;
			}
		}
	}

	//-------------------------------------------------------------------
	// Make the bullets go away when they get close enough to the target.
	// This is determined by checking the distanceToTarget.  A soon as it
	// starts to get bigger, we have passed the target and "hit" it.
	if (!hitTarget)
	{
		if (!((WeaponBoltTypePtr)getObjectType())->isBeam)
		{
			float lastDistanceMoved = distanceToTarget - distance;
			distanceToTarget = distance;
			
			if (lastDistanceMoved >= distance)
			{
				hitTarget = TRUE;
				hitLeft = ((WeaponBoltTypePtr)getObjectType())->afterHitTime;
			
				if (target)
				{
					Stuff::Vector3D hotSpot = target->getPositionFromHS(targetHotSpot);

					if (target->isMech())
					{
						if (targetHotSpot)
							hotSpot = target->appearance->getHitNodeLeft();
						else
							hotSpot = target->appearance->getHitNodeRight(); 

					}

					if (hitEffect && hitEffect->IsExecuted())
					{
						Stuff::Point3D			tPosition;
						Stuff::LinearMatrix4D 	shapeOrigin;
						Stuff::LinearMatrix4D	localToWorld;
						
						tPosition.x = -hotSpot.x;
						tPosition.y = hotSpot.z;
						tPosition.z = hotSpot.y;
						
						shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
						shapeOrigin.BuildTranslation(tPosition);
						
						gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
						hitEffect->Start(&info);
					}
					
					if (missEffect)
					{
						missEffect->Kill();
						delete missEffect;
						missEffect = NULL;
					}
					
					if (waterMissEffect)
					{
						waterMissEffect->Kill();
						delete waterMissEffect;
						waterMissEffect = NULL;
					}
					
 					//Put effect ON the spot!!
					laserPosition = hotSpot;
				}
				else
				{
					Stuff::Vector3D hotSpot = *targetPosition;
					int cellR, cellC;
					land->worldToCell(hotSpot,cellR, cellC);
					if (GameMap->getDeepWater(cellR, cellC) || GameMap->getShallowWater(cellR, cellC))
					{
						if (waterMissEffect && waterMissEffect->IsExecuted())
						{
							Stuff::Point3D			tPosition;
							Stuff::LinearMatrix4D 	shapeOrigin;
							Stuff::LinearMatrix4D	localToWorld;
							
							tPosition.x = -hotSpot.x;
							tPosition.y = Terrain::waterElevation;
							tPosition.z = hotSpot.y;
							
							shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
							shapeOrigin.BuildTranslation(tPosition);
							
							gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
							waterMissEffect->Start(&info);
						}
							
						if (missEffect)
						{
							missEffect->Kill();
							delete missEffect;
							missEffect = NULL;
						}
					}
					else
					{
						if (missEffect && missEffect->IsExecuted())
						{
							Stuff::Point3D			tPosition;
							Stuff::LinearMatrix4D 	shapeOrigin;
							Stuff::LinearMatrix4D	localToWorld;
							
							tPosition.x = -hotSpot.x;
							tPosition.y = hotSpot.z;
							tPosition.z = hotSpot.y;
							
							shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
							shapeOrigin.BuildTranslation(tPosition);
							
							gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
							missEffect->Start(&info);
						}
						
						if (waterMissEffect)
						{
							waterMissEffect->Kill();
							delete waterMissEffect;
							waterMissEffect = NULL;
						}
					}
					
					if (hitEffect)
					{
						hitEffect->Kill();
						delete hitEffect;
						hitEffect = NULL;
					}
					
					//Put effect ON the spot!!
					laserPosition = hotSpot;
				}
				
				//--------------------------------
				// Laser has hit target.
				if (target)
				{
					//Figure out angle between target and Shooter if target is BELOW shooter.
					// If no shooter, no bonus!!
					GameObjectPtr owner = ObjectManager->getByWatchID(ownerWID);
					if (owner)
					{
						if (target->getPosition().z < owner->getPosition().z)
						{
							Stuff::Vector3D targetPos = target->getPosition();
							Stuff::Vector3D ownerPos = owner->getPosition();
							
							float zDistance = ownerPos.z - targetPos.z;
							targetPos.z = ownerPos.z = 0.0f;
							
							Stuff::Vector3D distance;
							distance.Subtract(ownerPos,targetPos);
							
							float yDistance = distance.GetApproximateLength();
							
							float angle = mc2_atan2(zDistance,yDistance) * RADS_TO_DEGREES;
							
							if (RollDice(angle * BaseHeadShotElevation))
							{
								weaponShot.hitLocation = 0;		//Make it a head shot!
							}
							
						}
					}
					
					if (((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectDmg > 0.0f) &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectRad > 0.0f))
					{
						//This is an area effect weapon. Hit or MISS, its the same!
						if (hitEffect)
						{
							hitEffect->Kill();
							delete hitEffect;
							hitEffect = NULL;
						}
						
						if (missEffect)
						{
							missEffect->Kill();
							delete missEffect;
							missEffect = NULL;
						}
						
						if (waterMissEffect)
						{
							waterMissEffect->Kill();
							delete waterMissEffect;
							waterMissEffect = NULL;
						}
						
						//Create the explosion here.  HIT or MISS, its the same one!!
						// Damage is done by explosion NOT by hit!
						ObjectManager->createExplosion(((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum,
														NULL,*targetPosition,
														((WeaponBoltTypePtr)getObjectType())->areaEffectDmg,
														((WeaponBoltTypePtr)getObjectType())->areaEffectRad);
					}
					else
					{
						if (MPlayer)
						{
							if (MPlayer->isServer())
								target->handleWeaponHit(&weaponShot, true);
						}
						else
							target->handleWeaponHit(&weaponShot);
					}
				}
				else if (targetPosition)
				{
					long randomCrater = 0;
					if (weaponShot.damage > 9)
						randomCrater = 3;
					else if (weaponShot.damage > 6)
						randomCrater = 2;
					else if (weaponShot.damage > 3)
						randomCrater = 1;
						
					if (missEffect)		//We missed and hit land.  Otherwise, we hit water, do NOT add crater
						craterManager->addCrater(CRATER_1+randomCrater,*targetPosition,RandomNumber(180));
		
					//if (MasterComponentList[weaponShot.masterId].getWeaponAmmoType() != WEAPON_AMMO_NONE)	
					// energy weapons don't set off mines
					// Yes, they should DT, 5/30/98
					{
						int cellRow, cellCol;
		
						land->worldToCell(*targetPosition, cellRow, cellCol);
						if (GameMap->getMine(cellRow, cellCol) == 1)
						{
							ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, *targetPosition, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
							GameMap->setMine(cellRow, cellCol, 2);
						}
					}
					
					if (((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectDmg > 0.0f) &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectRad > 0.0f))
					{
						//This is an area effect weapon. Hit or MISS, its the same!
						if (hitEffect)
						{
							hitEffect->Kill();
							delete hitEffect;
							hitEffect = NULL;
						}
						
						if (missEffect)
						{
							missEffect->Kill();
							delete missEffect;
							missEffect = NULL;
						}
						
						if (waterMissEffect)
						{
							waterMissEffect->Kill();
							delete waterMissEffect;
							waterMissEffect = NULL;
						}
						
						//Create the explosion here.  HIT or MISS, its the same one!!
						// Damage is done by explosion NOT by hit!
						ObjectManager->createExplosion(((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum,
														NULL,*targetPosition,
														((WeaponBoltTypePtr)getObjectType())->areaEffectDmg,
														((WeaponBoltTypePtr)getObjectType())->areaEffectRad);
					}
				}
			}
		}
		else
		{
			if (timeLeft < 0.0)
			{
				hitTarget = TRUE;
				
				if (target)
				{
					Stuff::Vector3D hotSpot = target->getPositionFromHS(targetHotSpot);
					if (target->isMech())
					{
						if (targetHotSpot)
							hotSpot = target->appearance->getHitNodeLeft();
						else
							hotSpot = target->appearance->getHitNodeRight(); 
					}

					if (hitEffect && hitEffect->IsExecuted())
					{
						Stuff::Point3D			tPosition;
						Stuff::LinearMatrix4D 	shapeOrigin;
						Stuff::LinearMatrix4D	localToWorld;
						
						tPosition.x = -hotSpot.x;
						tPosition.y = hotSpot.z;
						tPosition.z = hotSpot.y;
						
						shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
						shapeOrigin.BuildTranslation(tPosition);
						
						gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
						hitEffect->Start(&info);
					}
					
					if (missEffect)
					{
						missEffect->Kill();
						delete missEffect;
						missEffect = NULL;
					}
					
					if (waterMissEffect)
					{
						waterMissEffect->Kill();
						delete waterMissEffect;
						waterMissEffect = NULL;
					}
				}
				else
				{
					Stuff::Vector3D hotSpot = *targetPosition;
					int cellR, cellC;
					land->worldToCell(hotSpot,cellR, cellC);
					if (GameMap->getDeepWater(cellR, cellC) || GameMap->getShallowWater(cellR, cellC))
					{
						if (waterMissEffect && waterMissEffect->IsExecuted())
						{
							Stuff::Point3D			tPosition;
							Stuff::LinearMatrix4D 	shapeOrigin;
							Stuff::LinearMatrix4D	localToWorld;
							
							tPosition.x = -hotSpot.x;
							tPosition.y = Terrain::waterElevation;
							tPosition.z = hotSpot.y;
							
							shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
							shapeOrigin.BuildTranslation(tPosition);
							
							gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
							waterMissEffect->Start(&info);
						}
						
						if (missEffect)
						{
							missEffect->Kill();
							delete missEffect;
							missEffect = NULL;
						}
					}
					else
					{
						if (missEffect && missEffect->IsExecuted())
						{
							Stuff::Point3D			tPosition;
							Stuff::LinearMatrix4D 	shapeOrigin;
							Stuff::LinearMatrix4D	localToWorld;
							
							tPosition.x = -hotSpot.x;
							tPosition.y = hotSpot.z;
							tPosition.z = hotSpot.y;
							
							shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
							shapeOrigin.BuildTranslation(tPosition);
							
							gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,NULL);
							missEffect->Start(&info);
						}
						
						if (waterMissEffect)
						{
							waterMissEffect->Kill();
							delete waterMissEffect;
							waterMissEffect = NULL;
						}
					}
					
					if (hitEffect)
					{
						hitEffect->Kill();
						delete hitEffect;
						hitEffect = NULL;
					}
				}
				
				//--------------------------------
				// Laser has hit target.
				if (target)
				{
					//Figure out angle between target and Shooter if target is BELOW shooter.
					// If no shooter, no bonus!!
					GameObjectPtr owner = ObjectManager->getByWatchID(ownerWID);
					if (owner)
					{
						if (target->getPosition().z < owner->getPosition().z)
						{
							Stuff::Vector3D targetPos = target->getPosition();
							Stuff::Vector3D ownerPos = owner->getPosition();

							float zDistance = ownerPos.z - targetPos.z;
							targetPos.z = ownerPos.z = 0.0f;

							Stuff::Vector3D distance;
							distance.Subtract(ownerPos,targetPos);

							float yDistance = distance.GetApproximateLength();

							float angle = mc2_atan2(zDistance,yDistance) * RADS_TO_DEGREES;

							if (RollDice(angle * BaseHeadShotElevation))
							{
								weaponShot.hitLocation = 0;		//Make it a head shot!
							}
						}
					}
					
					if (((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectDmg > 0.0f) &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectRad > 0.0f))
					{
						//This is an area effect weapon. Hit or MISS, its the same!
						if (hitEffect)
						{
							hitEffect->Kill();
							delete hitEffect;
							hitEffect = NULL;
						}
						
						if (missEffect)
						{
							missEffect->Kill();
							delete missEffect;
							missEffect = NULL;
						}
						
						if (waterMissEffect)
						{
							waterMissEffect->Kill();
							delete waterMissEffect;
							waterMissEffect = NULL;
						}
						
 						//Create the explosion here.  HIT or MISS, its the same one!!
						// Damage is done by explosion NOT by hit!
						ObjectManager->createExplosion(((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum,
														NULL,*targetPosition,
														((WeaponBoltTypePtr)getObjectType())->areaEffectDmg,
														((WeaponBoltTypePtr)getObjectType())->areaEffectRad);
					}
					else
					{
						if (MPlayer) 
						{
							if (MPlayer->isServer()) 
								target->handleWeaponHit(&weaponShot, true);
						}
						else
							target->handleWeaponHit(&weaponShot);
					}
				}
				else if (targetPosition) 
				{
					long randomCrater = 0;
					if (weaponShot.damage > 9)
						randomCrater = 3;
					else if (weaponShot.damage > 6)
						randomCrater = 2;
					else if (weaponShot.damage > 3)
						randomCrater = 1;
						
 					if (missEffect)
						craterManager->addCrater(CRATER_1+randomCrater,*targetPosition,RandomNumber(180));
		
					//if (MasterComponentList[weaponShot.masterId].getWeaponAmmoType() != WEAPON_AMMO_NONE)	
					// energy weapons don't set off mines
					// Yes, they should DT, 5/30/98
					{
						int cellRow, cellCol;
		
						land->worldToCell(*targetPosition, cellRow, cellCol);
						if (GameMap->getMine(cellRow, cellCol) == 1)
						{
							ObjectManager->createExplosion(MINE_EXPLOSION_ID, NULL, *targetPosition, MineSplashDamage, MineSplashRange * worldUnitsPerMeter);
							GameMap->setMine(cellRow, cellCol, 2);
						}
					}
					
					if (((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectDmg > 0.0f) &&
						(((WeaponBoltTypePtr)getObjectType())->areaEffectRad > 0.0f))
					{
						//This is an area effect weapon. Hit or MISS, its the same!
						if (hitEffect)
						{
							hitEffect->Kill();
							delete hitEffect;
							hitEffect = NULL;
						}
						
						if (missEffect)
						{
							missEffect->Kill();
							delete missEffect;
							missEffect = NULL;
						}
						
						if (waterMissEffect)
						{
							waterMissEffect->Kill();
							delete waterMissEffect;
							waterMissEffect = NULL;
						}
						
 						//Create the explosion here.  HIT or MISS, its the same one!!
						// Damage is done by explosion NOT by hit!
						ObjectManager->createExplosion(((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum,
														NULL,*targetPosition,
														((WeaponBoltTypePtr)getObjectType())->areaEffectDmg,
														((WeaponBoltTypePtr)getObjectType())->areaEffectRad);
					}
				}
			}
		}
	}
	
	//Actually move the effect toward the target
	// But only AFTER the first Start for the effect
	// Otherwise, weapon moves one time of flight distant
	if (!hitTarget)
	{
		float tmpZ = laserVelocity.z;
		if (isArcing)
		{
			laserVelocity.z = 0.0f;
			tmpZ *= frameLength;
		}
		
		if (laserVelocity.GetLength() != 0.0)
			laserVelocity.Normalize(laserVelocity);
	
		if (moveThisFrame)
		{
			laserVelocity *= velMag;
			if (isArcing)
				laserVelocity.z = tmpZ;
		}
		
		laserPosition += laserVelocity;
	
		laserDirection.Negate(laserVelocity);
		if (laserDirection.GetLength() != 0.0)
			laserDirection.Normalize(laserDirection);
	}

	//------------------------------------------------
	// Update GOSFX
   	if (gosEffect && gosEffect->IsExecuted())
   	{
		Stuff::Point3D actualPosition;
		if (((WeaponBoltTypePtr)getObjectType())->isBeam)
		{
			actualPosition.x = -(*targetPosition).x;	//Play the beam hitting effect at the hit location
			actualPosition.y = (*targetPosition).z;
			actualPosition.z = (*targetPosition).y;
		}
		else
		{
			actualPosition.x = -laserPosition.x;
			actualPosition.y = laserPosition.z;
			actualPosition.z = laserPosition.y;
		}
	
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D 	localToWorld;
		Stuff::LinearMatrix4D 	localResult;
			
		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(actualPosition);
		
		rotation = -world_angle_between(laserPosition, *targetPosition);
		
		Stuff::UnitQuaternion effectRot;
		effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
		localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
		localResult.Multiply(localToWorld,shapeOrigin);
 
		Stuff::OBB boundingBox;
   		gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,&boundingBox);

		bool result = gosEffect->Execute(&info);
		if (!result)
		{
			gosEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
			delete gosEffect;
			gosEffect = NULL;
		}
   	}
	
   	if (muzzleEffect && muzzleEffect->IsExecuted())
   	{
		Stuff::Point3D mPosition;
		mPosition.x = -position.x;
		mPosition.y = position.z;
		mPosition.z = position.y;
	
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D 	localToWorld;
		Stuff::LinearMatrix4D 	localResult;
			
		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(mPosition);

		Stuff::UnitQuaternion effectRot;
		if (myOwner)
			effectRot = Stuff::EulerAngles(0.0f, myOwner->getRotation() * DEGREES_TO_RADS,0.0f);
		else
			effectRot = Stuff::EulerAngles(0.0f, 0.0f ,0.0f); 

		localToWorld.Multiply(gosFX::Effect_Into_Motion,effectRot);
		localResult.Multiply(localToWorld,shapeOrigin);
			
 		Stuff::OBB boundingBox;
   		gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&localResult,&boundingBox);

		bool result = muzzleEffect->Execute(&info);
		if (!result)
		{
			muzzleEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
			delete muzzleEffect;
			muzzleEffect = NULL;
		}

   	}

	if (!hitTarget && !((WeaponBoltTypePtr)getObjectType())->isBeam)
	{
		//--------------------------------------------------------------------------------------
		// LaserPosition is front of projectile, laserVertices[0] is top vertex of bulge.
		// LaserVertices[1] is bottom vertex of bulge, laservertices[2] is back of projectile,
		// and laservertices[3] is the middle of the bulge.	
		Stuff::Vector3D lDir(laserDirection);
		lDir *= ((WeaponBoltTypePtr)getObjectType())->bulgeLength;
		laserVertices[0].Add(laserPosition,lDir);
		laserVertices[1] = laserVertices[0];
		laserVertices[3] = laserVertices[0];
		
		laserVertices[1].x += laserDirection.y * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserVertices[1].y -= laserDirection.x * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		
		laserVertices[0].x -= laserDirection.y * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserVertices[0].y += laserDirection.x * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
	
		laserDirection *= ((WeaponBoltTypePtr)getObjectType())->projLength;
		laserVertices[2].Add(laserPosition,laserDirection);
	
		if (((WeaponBoltTypePtr)getObjectType())->lightSource && !pointLight && (lightId != -1))
		{
			pointLight = ((WeaponBoltTypePtr)getObjectType())->getLight();
			lightId = eye->addWorldLight(pointLight);
			if (lightId == -1)
			{
				//NO LIGHT ALLOWED!  TOO Many in World!
				// LightId is now -1 which will cause this to NEVER make a light!
				systemHeap->Free(pointLight);
				pointLight = NULL;
			}
		}
	}
	else if (!hitTarget && ((WeaponBoltTypePtr)getObjectType())->isBeam)
	{
		//------------------------------------------------
		// For beam we need 4 vertices for the two faces.
		laserVertices[0] = position;
		laserVertices[1] = position;
		laserSide[0] = position;
		laserSide[1] = position;
		
		laserVertices[0].x -= laserDirection.y * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserVertices[0].y += laserDirection.x * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
 		
		laserVertices[1].x += laserDirection.y * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserVertices[1].y -= laserDirection.x * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;

		laserSide[0].z += ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserSide[1].z -= ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;

   		hsPos = *targetPosition;
   		if (target)
		{
			if (!target->isMech())
			{
				hsPos = target->getPositionFromHS(targetHotSpot);
			}
			else
			{
				if (targetHotSpot)
					hsPos = target->appearance->getHitNodeLeft();
				else
					hsPos = target->appearance->getHitNodeRight(); 
			}
		}

		//-------------------------------------------
		// Wiggle the hotspot.
		float beamWiggle = ((WeaponBoltTypePtr)getObjectType())->beamWiggle;
		Stuff::Vector3D offset(beamWiggle,beamWiggle,beamWiggle);
		offset.x = RandomNumber(offset.x * 2) - offset.x;
		offset.y = RandomNumber(offset.y * 2) - offset.y;
		offset.z = RandomNumber(offset.z * 2) - offset.z;
		hsPos += offset;

		laserVertices[2] = hsPos;
		laserVertices[3] = hsPos;
		
		laserSide[2] = hsPos;
		laserSide[3] = hsPos;
		
		laserVertices[2].x -= laserDirection.y * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserVertices[2].y += laserDirection.x * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
 		
		laserVertices[3].x += laserDirection.y * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserVertices[3].y -= laserDirection.x * ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		
		laserSide[2].z += ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
		laserSide[3].z -= ((WeaponBoltTypePtr)getObjectType())->bulgeWidth;
 	}

	if (pointLight)
	{
		Stuff::Point3D ourPosition;
		ourPosition.x = -laserPosition.x;
		ourPosition.y = laserPosition.z;
		ourPosition.z = laserPosition.y;

		pointLight->direction = ourPosition;

		Stuff::LinearMatrix4D lightToWorldMatrix;
		lightToWorldMatrix.BuildTranslation(ourPosition);
		lightToWorldMatrix.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		pointLight->SetLightToWorld(&lightToWorldMatrix);
		pointLight->SetPosition(&laserPosition);
	}
		
 	if (hitTarget)
	{
		hitLeft -= frameLength;
		
		if (gosEffect && (hitLeft < 0.0f))
		{
			gosEffect->Kill();
			delete gosEffect;
			gosEffect = NULL;
			
			//Need to kill the light source here too!
			if (pointLight)
			{
				eye->removeWorldLight(lightId,pointLight);
				systemHeap->Free(pointLight);
				pointLight = NULL;
			}
 		}
		
		if (hitEffect && hitEffect->IsExecuted())
		{
			Stuff::Vector3D hotSpot = *targetPosition;
			if (target)
			{
				if (!target->isMech())
				{
					hotSpot = target->getPositionFromHS(targetHotSpot);
				}
				else
				{
					if (targetHotSpot)
						hotSpot = target->appearance->getHitNodeLeft();
					else
						hotSpot = target->appearance->getHitNodeRight(); 
				}
			}
				
			Stuff::Point3D tPosition;
			tPosition.x = -hotSpot.x;
			tPosition.y = hotSpot.z;
			tPosition.z = hotSpot.y;
		
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D 	localToWorld;
				
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(tPosition);
	
			Stuff::OBB boundingBox;
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
	
			bool result = hitEffect->Execute(&info);
			if (!result)
			{
				if (hitEffect)
				{
					hitEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
					delete hitEffect;
					hitEffect = NULL;
				}
					
				inView = FALSE;
				
				if (pointLight)
				{
					eye->removeWorldLight(lightId,pointLight);
					systemHeap->Free(pointLight);
					pointLight = NULL;
				}
				
				//Kill later to make Steve Happy!!
				if (muzzleEffect)
				{
					muzzleEffect->Kill();
					delete muzzleEffect;
					muzzleEffect = NULL;
				}
			}
		}
		else
		{
			if (hitEffect)
			{
				hitEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
				delete hitEffect;
				hitEffect = NULL;
			}
				
			if (pointLight)
			{
				eye->removeWorldLight(lightId,pointLight);
				systemHeap->Free(pointLight);
				pointLight = NULL;
			}
		}
		
		if (missEffect && missEffect->IsExecuted())
		{
			Stuff::Vector3D hotSpot = *targetPosition;
			if (target)
			{
				if (!target->isMech())
				{
					hotSpot = target->getPositionFromHS(targetHotSpot);
				}
				else
				{
					if (targetHotSpot)
						hotSpot = target->appearance->getHitNodeLeft();
					else
						hotSpot = target->appearance->getHitNodeRight(); 
				}
			}
				
			Stuff::Point3D tPosition;
			tPosition.x = -hotSpot.x;
			tPosition.y = hotSpot.z;
			tPosition.z = hotSpot.y;
		
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D 	localToWorld;
				
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(tPosition);
	
			Stuff::OBB boundingBox;
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
	
			bool result = missEffect->Execute(&info);
			if (!result)
			{
				if (missEffect)
				{
					missEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
					delete missEffect;
					missEffect = NULL;
				}
					
				//Kill later to make Steve Happy!!
				if (muzzleEffect)
				{
					muzzleEffect->Kill();
					delete muzzleEffect;
					muzzleEffect = NULL;
				}
				
				inView = FALSE;
				
				if (pointLight)
				{
					eye->removeWorldLight(lightId,pointLight);
					systemHeap->Free(pointLight);
					pointLight = NULL;
				}
			}
		}
		else
		{
			if (missEffect)
			{
				missEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
				delete missEffect;
				missEffect = NULL;
			}
				
			if (pointLight)
			{
				eye->removeWorldLight(lightId,pointLight);
				systemHeap->Free(pointLight);
				pointLight = NULL;
			}
		}
		
		if (waterMissEffect && waterMissEffect->IsExecuted())
		{
			Stuff::Vector3D hotSpot = *targetPosition;
			if (target)
			{
				if (!target->isMech())
				{
					hotSpot = target->getPositionFromHS(targetHotSpot);
				}
				else
				{
					if (targetHotSpot)
						hotSpot = target->appearance->getHitNodeLeft();
					else
						hotSpot = target->appearance->getHitNodeRight(); 
				}
			}
				
			Stuff::Point3D tPosition;
			tPosition.x = -hotSpot.x;
			tPosition.y = Terrain::waterElevation; 
			tPosition.z = hotSpot.y;
		
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D 	localToWorld;
				
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(tPosition);
	
			Stuff::OBB boundingBox;
			gosFX::Effect::ExecuteInfo info((Stuff::Time)scenarioTime,&shapeOrigin,&boundingBox);
	
			bool result = waterMissEffect->Execute(&info);
			if (!result)
			{
				if (waterMissEffect)
				{
					waterMissEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
					delete waterMissEffect;
					waterMissEffect = NULL;
				}
					
				//Kill later to make Steve Happy!!
				if (muzzleEffect)
				{
					muzzleEffect->Kill();
					delete muzzleEffect;
					muzzleEffect = NULL;
				}
				
				inView = FALSE;
				
				if (pointLight)
				{
					eye->removeWorldLight(lightId,pointLight);
					systemHeap->Free(pointLight);
					pointLight = NULL;
				}
			}
		}
		else
		{
			if (waterMissEffect)
			{
				waterMissEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
				delete waterMissEffect;
				waterMissEffect = NULL;
			}
				
			if (pointLight)
			{
				eye->removeWorldLight(lightId,pointLight);
				systemHeap->Free(pointLight);
				pointLight = NULL;
			}
		}

		if (!waterMissEffect && !hitEffect && !missEffect && !gosEffect)
			inView = false;
	}
	
	return (inView);
}

//---------------------------------------------------------------------------
void WeaponBolt::render (void)
{
	if (!getFlag(OBJECT_FLAG_JUSTCREATED) && !isTargeted())	//can we be seen by the enemy?
	{
		gosFX::Effect::DrawInfo drawInfo;
		drawInfo.m_clipper = theClipper;
		
		MidLevelRenderer::MLRState mlrState;
		//mlrState.SetBackFaceOn();
		mlrState.SetDitherOn();
		mlrState.SetTextureCorrectionOn();
		mlrState.SetZBufferCompareOn();
		mlrState.SetZBufferWriteOn();

		//mlrState.SetFilterMode(MidLevelRenderer::MLRState::BiLinearFilterMode);
		//mlrState.SetAlphaMode(MidLevelRenderer::MLRState::AlphaInvAlphaMode);

		drawInfo.m_state = mlrState;
		drawInfo.m_clippingFlags = 0x0;
			
		if (!gosEffect && !hitTarget && !((WeaponBoltTypePtr)getObjectType())->isBeam)
		{
			//---------------------------
			// Render the polygons here.
			gos_VERTEX	lq1Vertices[3], lq2Vertices[3], lq3Vertices[3], lq4Vertices[3];
			Stuff::Vector4D screenPos;
			
			DWORD alphaMode = ((WeaponBoltTypePtr)getObjectType())->boltAlpha;
			alphaMode <<= 24;
			
			DWORD edgeMode = ((WeaponBoltTypePtr)getObjectType())->edgeAlpha;
			edgeMode <<= 24;
			
			eye->projectZ(laserPosition,screenPos);
			lq1Vertices[0].x		= screenPos.x;
			lq1Vertices[0].y		= screenPos.y;
			lq1Vertices[0].z		= 0.1f;
			lq1Vertices[0].rhw		= screenPos.w;
			lq1Vertices[0].u		= 0.0f;
			lq1Vertices[0].v		= 0.0f;
			lq1Vertices[0].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->frontRGB;
	
			lq2Vertices[0].x		= screenPos.x;
			lq2Vertices[0].y		= screenPos.y;
			lq2Vertices[0].z		= 0.1f;
			lq2Vertices[0].rhw		= screenPos.w;
			lq2Vertices[0].u		= 0.0f;
			lq2Vertices[0].v		= 0.0f;
			lq2Vertices[0].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->frontRGB;
	
			eye->projectZ(laserVertices[0],screenPos);
			lq1Vertices[1].x		= screenPos.x;
			lq1Vertices[1].y		= screenPos.y;
			lq1Vertices[1].z		= 0.1f;
			lq1Vertices[1].rhw		= screenPos.w;
			lq1Vertices[1].u		= 0.0f;
			lq1Vertices[1].v		= 0.0f;
			lq1Vertices[1].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
			
			lq3Vertices[1].x		= screenPos.x;
			lq3Vertices[1].y		= screenPos.y;
			lq3Vertices[1].z		= 0.1f;
			lq3Vertices[1].rhw		= screenPos.w;
			lq3Vertices[1].u		= 0.0f;
			lq3Vertices[1].v		= 0.0f;
			lq3Vertices[1].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
			
			eye->projectZ(laserVertices[3],screenPos);
			lq1Vertices[2].x		= screenPos.x;
			lq1Vertices[2].y		= screenPos.y;
			lq1Vertices[2].z		= 0.1f;
			lq1Vertices[2].rhw		= screenPos.w;
			lq1Vertices[2].u		= 0.0f;
			lq1Vertices[2].v		= 0.0f;
			lq1Vertices[2].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			lq2Vertices[2].x		= screenPos.x;
			lq2Vertices[2].y		= screenPos.y;
			lq2Vertices[2].z		= 0.1f;
			lq2Vertices[2].rhw		= screenPos.w;
			lq2Vertices[2].u		= 0.0f;
			lq2Vertices[2].v		= 0.0f;
			lq2Vertices[2].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			lq3Vertices[0].x		= screenPos.x;
			lq3Vertices[0].y		= screenPos.y;
			lq3Vertices[0].z		= 0.1f;
			lq3Vertices[0].rhw		= screenPos.w;
			lq3Vertices[0].u		= 0.0f;
			lq3Vertices[0].v		= 0.0f;
			lq3Vertices[0].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			lq4Vertices[0].x		= screenPos.x;
			lq4Vertices[0].y		= screenPos.y;
			lq4Vertices[0].z		= 0.1f;
			lq4Vertices[0].rhw		= screenPos.w;
			lq4Vertices[0].u		= 0.0f;
			lq4Vertices[0].v		= 0.0f;
			lq4Vertices[0].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			eye->projectZ(laserVertices[2],screenPos);
			lq3Vertices[2].x		= screenPos.x;
			lq3Vertices[2].y		= screenPos.y;
			lq3Vertices[2].z		= 0.1f;
			lq3Vertices[2].rhw		= screenPos.w;
			lq3Vertices[2].u		= 0.0f;
			lq3Vertices[2].v		= 0.0f;
			lq3Vertices[2].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->backRGB;
			
			lq4Vertices[2].x		= screenPos.x;
			lq4Vertices[2].y		= screenPos.y;
			lq4Vertices[2].z		= 0.1f;
			lq4Vertices[2].rhw		= screenPos.w;
			lq4Vertices[2].u		= 0.0f;
			lq4Vertices[2].v		= 0.0f;
			lq4Vertices[2].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->backRGB;
			
			eye->projectZ(laserVertices[1],screenPos);
			lq2Vertices[1].x		= screenPos.x;
			lq2Vertices[1].y		= screenPos.y;
			lq2Vertices[1].z		= 0.1f;
			lq2Vertices[1].rhw		= screenPos.w;
			lq2Vertices[1].u		= 0.0f;
			lq2Vertices[1].v		= 0.0f;
			lq2Vertices[1].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
		
			lq4Vertices[1].x		= screenPos.x;
			lq4Vertices[1].y		= screenPos.y;
			lq4Vertices[1].z		= 0.1f;
			lq4Vertices[1].rhw		= screenPos.w;
			lq4Vertices[1].u		= 0.0f;
			lq4Vertices[1].v		= 0.0f;
			lq4Vertices[1].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
		
			if (((lq1Vertices[0].x > eye->viewMulX) ||
				(lq1Vertices[0].y > eye->viewMulY) || 
				(lq1Vertices[0].x < eye->viewAddX) || 
				(lq1Vertices[0].y < eye->viewAddY) || 
				(lq1Vertices[1].x > eye->viewMulX) ||
				(lq1Vertices[1].y > eye->viewMulY) ||
				(lq1Vertices[1].x < eye->viewAddX) ||
				(lq1Vertices[1].y < eye->viewAddY) ||
				(lq1Vertices[2].x > eye->viewMulX) ||
				(lq1Vertices[2].y > eye->viewMulY) ||
				(lq1Vertices[2].x < eye->viewAddX) ||
				(lq1Vertices[2].y < eye->viewAddY)))
			{
			}
			else
			{
				if (drawOldWay)
				{
					//------------------------
					// Draw em!
					gos_SetRenderState( gos_State_Texture, 0 );
			
					//--------------------------------
					//Set States for Software Renderer
					if (Environment.Renderer == 3)
					{
						gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
						gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
						gos_SetRenderState( gos_State_MonoEnable, 1);
						gos_SetRenderState( gos_State_Perspective, 0);
						gos_SetRenderState( gos_State_Clipping, 0);
						gos_SetRenderState( gos_State_AlphaTest, 0);
						gos_SetRenderState( gos_State_Specular, 0);
						gos_SetRenderState( gos_State_Dither, 0);
						gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);
						gos_SetRenderState( gos_State_Filter, gos_FilterNone);
						gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
					}
					//--------------------------------
					//Set States for Hardware Renderer	
					else
					{
						gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneZero);
						gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
						gos_SetRenderState( gos_State_MonoEnable, 0);
						gos_SetRenderState( gos_State_Perspective, 1);
						gos_SetRenderState( gos_State_Clipping, 2);
						gos_SetRenderState( gos_State_AlphaTest, 0);
						gos_SetRenderState( gos_State_Specular, 0);
						gos_SetRenderState( gos_State_Dither, 1);
						gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);
						gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear);
						gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
					}

					gos_DrawTriangles(lq1Vertices,3);
				}
				else
				{
					mcTextureManager->addVertices(0xffffffff,lq1Vertices,MC2_DRAWALPHA);
				}
			}
	
			if (((lq2Vertices[0].x > eye->viewMulX) ||
				(lq2Vertices[0].y > eye->viewMulY) || 
				(lq2Vertices[0].x < eye->viewAddX) || 
				(lq2Vertices[0].y < eye->viewAddY) || 
				(lq2Vertices[1].x > eye->viewMulX) ||
				(lq2Vertices[1].y > eye->viewMulY) ||
				(lq2Vertices[1].x < eye->viewAddX) ||
				(lq2Vertices[1].y < eye->viewAddY) ||
				(lq2Vertices[2].x > eye->viewMulX) ||
				(lq2Vertices[2].y > eye->viewMulY) ||
				(lq2Vertices[2].x < eye->viewAddX) ||
				(lq2Vertices[2].y < eye->viewAddY)))
			{
			}
			else
			{
				if (drawOldWay)
				{
					gos_DrawTriangles(lq2Vertices,3);
				}
				else
				{
					mcTextureManager->addVertices(0xffffffff,lq2Vertices,MC2_DRAWALPHA);
				}
			}
				
			if (((lq3Vertices[0].x > eye->viewMulX) ||
				(lq3Vertices[0].y > eye->viewMulY) || 
				(lq3Vertices[0].x < eye->viewAddX) || 
				(lq3Vertices[0].y < eye->viewAddY) || 
				(lq3Vertices[1].x > eye->viewMulX) ||
				(lq3Vertices[1].y > eye->viewMulY) ||
				(lq3Vertices[1].x < eye->viewAddX) ||
				(lq3Vertices[1].y < eye->viewAddY) ||
				(lq3Vertices[2].x > eye->viewMulX) ||
				(lq3Vertices[2].y > eye->viewMulY) ||
				(lq3Vertices[2].x < eye->viewAddX) ||
				(lq3Vertices[2].y < eye->viewAddY)))
			{
			}
			else
			{
				if (drawOldWay)
				{
					gos_DrawTriangles(lq3Vertices,3);
				}
				else
				{
					mcTextureManager->addVertices(0xffffffff,lq3Vertices,MC2_DRAWALPHA);
				}
			}
	
			if (((lq4Vertices[0].x > eye->viewMulX) ||
				(lq4Vertices[0].y > eye->viewMulY) || 
				(lq4Vertices[0].x < eye->viewAddX) || 
				(lq4Vertices[0].y < eye->viewAddY) || 
				(lq4Vertices[1].x > eye->viewMulX) ||
				(lq4Vertices[1].y > eye->viewMulY) ||
				(lq4Vertices[1].x < eye->viewAddX) ||
				(lq4Vertices[1].y < eye->viewAddY) ||
				(lq4Vertices[2].x > eye->viewMulX) ||
				(lq4Vertices[2].y > eye->viewMulY) ||
				(lq4Vertices[2].x < eye->viewAddX) ||
				(lq4Vertices[2].y < eye->viewAddY)))
			{
			}
			else
			{
				if (drawOldWay)
				{
					gos_DrawTriangles(lq4Vertices,3);
				}
				else
				{
					mcTextureManager->addVertices(0xffffffff,lq4Vertices,MC2_DRAWALPHA);
				}
			}
		}
		else if (!hitTarget && ((WeaponBoltTypePtr)getObjectType())->isBeam)
		{
			//---------------------------
			// Render the polygons here.
			gos_VERTEX	lq1Vertices[3], lq2Vertices[3], lq3Vertices[3], lq4Vertices[3];
			Stuff::Vector4D screenPos;
			
			DWORD alphaMode = ((WeaponBoltTypePtr)getObjectType())->boltAlpha;
			alphaMode <<= 24;
			
			DWORD edgeMode = ((WeaponBoltTypePtr)getObjectType())->edgeAlpha;
			edgeMode <<= 24;
			
			Stuff::Point3D actualPosition;
			Stuff::Vector3D hotSpot = *targetPosition;
			GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
			if (target)
			{
				if (!target->isMech())
				{
					hotSpot = target->getPositionFromHS(targetHotSpot);
				}
				else
				{
					if (targetHotSpot)
						hotSpot = target->appearance->getHitNodeLeft();
					else
						hotSpot = target->appearance->getHitNodeRight(); 
				}
			}
				
			Stuff::Vector3D length;
			length.Subtract(hotSpot,position);
			float dist = length.GetApproximateLength();
			
 			float realUVRepeat = ((WeaponBoltTypePtr)getObjectType())->uvRepeat * dist / ((WeaponBoltTypePtr)getObjectType())->unitLength;
			
			eye->projectZ(laserVertices[0],screenPos);
			lq1Vertices[0].x		= screenPos.x;
			lq1Vertices[0].y		= screenPos.y;
			lq1Vertices[0].z		= screenPos.z;
			lq1Vertices[0].rhw		= screenPos.w;
			lq1Vertices[0].u		= startUV;
			lq1Vertices[0].v		= 0.0f;
			lq1Vertices[0].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
	
			lq2Vertices[0].x		= screenPos.x;
			lq2Vertices[0].y		= screenPos.y;
			lq2Vertices[0].z		= screenPos.z;
			lq2Vertices[0].rhw		= screenPos.w;
			lq2Vertices[0].u		= startUV;
			lq2Vertices[0].v		= 0.0f;
			lq2Vertices[0].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
			
 			eye->projectZ(laserVertices[1],screenPos);
			lq2Vertices[1].x		= screenPos.x;
			lq2Vertices[1].y		= screenPos.y;
			lq2Vertices[1].z		= screenPos.z;
			lq2Vertices[1].rhw		= screenPos.w;
			lq2Vertices[1].u		= startUV;
			lq2Vertices[1].v		= 0.999999f;
			lq2Vertices[1].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
	
			eye->projectZ(laserVertices[2],screenPos);
			lq1Vertices[1].x		= screenPos.x;
			lq1Vertices[1].y		= screenPos.y;
			lq1Vertices[1].z		= screenPos.z;
			lq1Vertices[1].rhw		= screenPos.w;
			lq1Vertices[1].u		= startUV + realUVRepeat;
			lq1Vertices[1].v		= 0.0f;
			lq1Vertices[1].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			eye->projectZ(laserVertices[3],screenPos);
			lq1Vertices[2].x		= screenPos.x;
			lq1Vertices[2].y		= screenPos.y;
			lq1Vertices[2].z		= screenPos.z;
			lq1Vertices[2].rhw		= screenPos.w;
			lq1Vertices[2].u		= startUV + realUVRepeat; 
			lq1Vertices[2].v		= 0.999999f;
			lq1Vertices[2].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			lq2Vertices[2].x		= screenPos.x;
			lq2Vertices[2].y		= screenPos.y;
			lq2Vertices[2].z		= screenPos.z;
			lq2Vertices[2].rhw		= screenPos.w;
			lq2Vertices[2].u		= startUV + realUVRepeat; 
			lq2Vertices[2].v		= 0.999999f;
			lq2Vertices[2].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			eye->projectZ(laserSide[0],screenPos);
			lq3Vertices[0].x		= screenPos.x;
			lq3Vertices[0].y		= screenPos.y;
			lq3Vertices[0].z		= screenPos.z;
			lq3Vertices[0].rhw		= screenPos.w;
			lq3Vertices[0].u		= startUV;
			lq3Vertices[0].v		= 0.0f;
			lq3Vertices[0].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
	
			lq4Vertices[0].x		= screenPos.x;
			lq4Vertices[0].y		= screenPos.y;
			lq4Vertices[0].z		= screenPos.z;
			lq4Vertices[0].rhw		= screenPos.w;
			lq4Vertices[0].u		= startUV;
			lq4Vertices[0].v		= 0.0f;
			lq4Vertices[0].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
			
 			eye->projectZ(laserSide[1],screenPos);
			lq4Vertices[1].x		= screenPos.x;
			lq4Vertices[1].y		= screenPos.y;
			lq4Vertices[1].z		= screenPos.z;
			lq4Vertices[1].rhw		= screenPos.w;
			lq4Vertices[1].u		= startUV;
			lq4Vertices[1].v		= 0.999999f;
			lq4Vertices[1].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->midEdgeRGB;
	
			eye->projectZ(laserSide[2],screenPos);
			lq3Vertices[1].x		= screenPos.x;
			lq3Vertices[1].y		= screenPos.y;
			lq3Vertices[1].z		= screenPos.z;
			lq3Vertices[1].rhw		= screenPos.w;
			lq3Vertices[1].u		= startUV + realUVRepeat;
			lq3Vertices[1].v		= 0.0f;
			lq3Vertices[1].argb		= edgeMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			eye->projectZ(laserSide[3],screenPos);
			lq3Vertices[2].x		= screenPos.x;
			lq3Vertices[2].y		= screenPos.y;
			lq3Vertices[2].z		= screenPos.z;
			lq3Vertices[2].rhw		= screenPos.w;
			lq3Vertices[2].u		= startUV + realUVRepeat; 
			lq3Vertices[2].v		= 0.999999f;
			lq3Vertices[2].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;
			
			lq4Vertices[2].x		= screenPos.x;
			lq4Vertices[2].y		= screenPos.y;
			lq4Vertices[2].z		= screenPos.z;
			lq4Vertices[2].rhw		= screenPos.w;
			lq4Vertices[2].u		= startUV + realUVRepeat; 
			lq4Vertices[2].v		= 0.999999f;
			lq4Vertices[2].argb		= alphaMode + ((WeaponBoltTypePtr)getObjectType())->middleRGB;

	
			if (drawOldWay)
			{
 				//------------------------
				// Draw em!
				gos_SetRenderState( gos_State_Texture, 0 );
		
				//--------------------------------
				//Set States for Software Renderer
				if (Environment.Renderer == 3)
				{
					gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneInvAlpha);
					gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
					gos_SetRenderState( gos_State_MonoEnable, 1);
					gos_SetRenderState( gos_State_Perspective, 0);
					gos_SetRenderState( gos_State_Clipping, 1);
					gos_SetRenderState( gos_State_AlphaTest, 0);
					gos_SetRenderState( gos_State_Specular, 0);
					gos_SetRenderState( gos_State_Dither, 0);
					gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);
					gos_SetRenderState( gos_State_Filter, gos_FilterNone);
					gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
					gos_SetRenderState( gos_State_ZCompare, 1);
					gos_SetRenderState(	gos_State_ZWrite, 0);
				}
				//--------------------------------
				//Set States for Hardware Renderer	
				else
				{
					gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_OneInvAlpha);
					gos_SetRenderState( gos_State_ShadeMode, gos_ShadeGouraud);
					gos_SetRenderState( gos_State_MonoEnable, 0);
					gos_SetRenderState( gos_State_Perspective, 1);
					gos_SetRenderState( gos_State_Clipping, 1);
					gos_SetRenderState( gos_State_AlphaTest, 0);
					gos_SetRenderState( gos_State_Specular, 0);
					gos_SetRenderState( gos_State_Dither, 1);
					gos_SetRenderState( gos_State_TextureMapBlend, gos_BlendModulateAlpha);
					gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear);
					gos_SetRenderState( gos_State_TextureAddress, gos_TextureWrap );
					gos_SetRenderState( gos_State_ZCompare, 1);
					gos_SetRenderState(	gos_State_ZWrite, 0);
				}
			
				if (gosTextureHandle != 0xffffffff)
					gos_SetRenderState( gos_State_Texture,gosTextureHandle);
				else
					gos_SetRenderState( gos_State_Texture,0);

				gos_DrawTriangles(lq1Vertices,3);
				gos_DrawTriangles(lq2Vertices,3);
				gos_DrawTriangles(lq3Vertices,3);
				gos_DrawTriangles(lq4Vertices,3);
			}
			else
			{
				if ((lq1Vertices[0].z >= 0.0f) &&
					(lq1Vertices[0].z < 1.0f) &&
					(lq1Vertices[1].z >= 0.0f) &&  
					(lq1Vertices[1].z < 1.0f) && 
					(lq1Vertices[2].z >= 0.0f) &&  
					(lq1Vertices[2].z < 1.0f))
					{
						mcTextureManager->addVertices(mcTextureHandle,lq1Vertices,MC2_ISEFFECTS|MC2_DRAWONEIN);
					}
					
				if ((lq2Vertices[0].z >= 0.0f) &&
					(lq2Vertices[0].z < 1.0f) &&
					(lq2Vertices[1].z >= 0.0f) &&  
					(lq2Vertices[1].z < 1.0f) && 
					(lq2Vertices[2].z >= 0.0f) &&  
					(lq2Vertices[2].z < 1.0f))
					{
						mcTextureManager->addVertices(mcTextureHandle,lq2Vertices,MC2_ISEFFECTS|MC2_DRAWONEIN);
					}
					
				if ((lq3Vertices[0].z >= 0.0f) &&
					(lq3Vertices[0].z < 1.0f) &&
					(lq3Vertices[1].z >= 0.0f) &&  
					(lq3Vertices[1].z < 1.0f) && 
					(lq3Vertices[2].z >= 0.0f) &&  
					(lq3Vertices[2].z < 1.0f))
					{
						mcTextureManager->addVertices(mcTextureHandle,lq3Vertices,MC2_ISEFFECTS|MC2_DRAWONEIN);
					}
					
				if ((lq4Vertices[0].z >= 0.0f) &&
					(lq4Vertices[0].z < 1.0f) &&
					(lq4Vertices[1].z >= 0.0f) &&  
					(lq4Vertices[1].z < 1.0f) && 
					(lq4Vertices[2].z >= 0.0f) &&  
					(lq4Vertices[2].z < 1.0f))
					{
						mcTextureManager->addVertices(mcTextureHandle,lq4Vertices,MC2_ISEFFECTS|MC2_DRAWONEIN);
					}
			}
				
			if (gosEffect)
			{
				Stuff::Point3D actualPosition;
				Stuff::Vector3D hotSpot = *targetPosition;
				GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
				if (target)
				{
					if (!target->isMech())
					{
						hotSpot = target->getPositionFromHS(targetHotSpot);
					}
					else
					{
						if (targetHotSpot)
							hotSpot = target->appearance->getHitNodeLeft();
						else
							hotSpot = target->appearance->getHitNodeRight(); 
					}
				}
	
				actualPosition.x = -hotSpot.x;	//Play the beam hitting effect at the hit location
				actualPosition.y = hotSpot.z;
				actualPosition.z = hotSpot.y;
				
				Stuff::LinearMatrix4D 	shapeOrigin;
				Stuff::LinearMatrix4D	localToWorld;
				Stuff::LinearMatrix4D	localResult;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(actualPosition);
				
				Stuff::UnitQuaternion effectRot;
				effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
				localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
				localResult.Multiply(localToWorld,shapeOrigin);
				
				drawInfo.m_parentToWorld = &localResult;
		 
				if (!MLRVertexLimitReached)
					gosEffect->Draw(&drawInfo);
			}
		}
		else if (gosEffect)
		{
			Stuff::Point3D actualPosition;
			actualPosition.x = -laserPosition.x;
			actualPosition.y = laserPosition.z;
			actualPosition.z = laserPosition.y;
			
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(actualPosition);
			
			Stuff::UnitQuaternion effectRot;
			effectRot = Stuff::EulerAngles(0.0f,rotation * DEGREES_TO_RADS,0.0f);
			localToWorld.Multiply(gosFX::Effect_Against_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
			
 			drawInfo.m_parentToWorld = &localResult;
	 
			if (!MLRVertexLimitReached)
				gosEffect->Draw(&drawInfo);
		}
		
		if (muzzleEffect)
		{
			Stuff::Point3D mPosition;
			mPosition.x = -position.x;
			mPosition.y = position.z;
			mPosition.z = position.y;
			
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(mPosition);
			
			GameObjectPtr myOwner = ObjectManager->getByWatchID(ownerWID);
			Stuff::UnitQuaternion effectRot;
		
			if (myOwner)
				effectRot = Stuff::EulerAngles(0.0f, myOwner->getRotation() * DEGREES_TO_RADS,0.0f);
			else
				effectRot = Stuff::EulerAngles(0.0f,0.0f,0.0f); 

			localToWorld.Multiply(gosFX::Effect_Into_Motion,effectRot);
			localResult.Multiply(localToWorld,shapeOrigin);
		
 			drawInfo.m_parentToWorld = &localResult;
	 
			if (!MLRVertexLimitReached)
				muzzleEffect->Draw(&drawInfo);
		}
		
		if (hitEffect && hitTarget && hitEffect->IsExecuted())
		{
			Stuff::Vector3D hotSpot = *targetPosition;
			GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
			if (target)
			{
				if (!target->isMech())
				{
					hotSpot = target->getPositionFromHS(targetHotSpot);
				}
				else
				{
					if (targetHotSpot)
						hotSpot = target->appearance->getHitNodeLeft();
					else
						hotSpot = target->appearance->getHitNodeRight(); 
				}
			}

			Stuff::Point3D tPosition;
			tPosition.x = -hotSpot.x;
			tPosition.y = hotSpot.z;
			tPosition.z = hotSpot.y;
			
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(tPosition);
			
			drawInfo.m_parentToWorld = &shapeOrigin;
	 
			if (!MLRVertexLimitReached)
				hitEffect->Draw(&drawInfo);
		}
		
		if (missEffect && hitTarget && missEffect->IsExecuted())
		{
			Stuff::Vector3D hotSpot = *targetPosition;
			
			Stuff::Point3D tPosition;
			tPosition.x = -hotSpot.x;
			tPosition.y = hotSpot.z;
			tPosition.z = hotSpot.y;
			
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(tPosition);
			
			drawInfo.m_parentToWorld = &shapeOrigin;
	 
			if (!MLRVertexLimitReached)
				missEffect->Draw(&drawInfo);
		}
		
		if (waterMissEffect && hitTarget && waterMissEffect->IsExecuted())
		{
			Stuff::Vector3D hotSpot = *targetPosition;
			
			Stuff::Point3D tPosition;
			tPosition.x = -hotSpot.x;
			tPosition.y = Terrain::waterElevation; 
			tPosition.z = hotSpot.y;
			
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			
			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(tPosition);
			
			drawInfo.m_parentToWorld = &shapeOrigin;
	 
			if (!MLRVertexLimitReached)
				waterMissEffect->Draw(&drawInfo);
		}
	}
}

//---------------------------------------------------------------------------
void WeaponBolt::destroy (void)
{
	if (targetPosition)
	{
		delete targetPosition;
		targetPosition = NULL;
	}

	//Must toss these here as mission may have ended with effect still playing!!
	if (pointLight)
	{
		if (eye)
			eye->removeWorldLight(lightId,pointLight);
		systemHeap->Free(pointLight);
		pointLight = NULL;
	}

	if (hitEffect)
	{
		hitEffect->Kill();
		delete hitEffect;
		hitEffect = NULL;
	}

	if (muzzleEffect)
	{
		muzzleEffect->Kill();
		delete muzzleEffect;
		muzzleEffect = NULL;
	}

	if (gosEffect)
	{
		gosEffect->Kill();	 
		delete gosEffect;
		gosEffect = NULL;
	}

	if (missEffect)
	{
		missEffect->Kill();	 
		delete missEffect;
		missEffect = NULL;
	}

	if (waterMissEffect)
	{
		waterMissEffect->Kill();
		delete waterMissEffect;
		waterMissEffect = NULL;
	}
}

//---------------------------------------------------------------------------
void WeaponBolt::init (bool create, ObjectTypePtr _type)
{
	GameObject::init(create, _type);

	setFlag(OBJECT_FLAG_JUSTCREATED, true);

	objectClass = WEAPONBOLT;
	
	lightId = -9;	//Tell it to make one!
	
	//Make SURE all of the old Effects are GONE!!

	destroy();
	//-----------------------------------------
	// Are we a magical new GOSFX(tm)?
	if (effectId >= 0)
	{
		if (strcmp(weaponEffects->GetEffectName(effectId),"NONE") != 0)
		{
			//--------------------------------------------
			// Yes, load it on up.
			unsigned flags = gosFX::Effect::ExecuteFlag|gosFX::Effect::LoopFlag;
	
			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(effectId));
			
			if (gosEffectSpec)
			{
				gosEffect = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
				gosASSERT(gosEffect != NULL);
			
				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			
				//gosEffect->SetScalar(BASE_EFFECT_SCALAR);
			}
		}
		
		if ((strcmp(weaponEffects->GetEffectMuzzleFlashName(effectId),"NONE") != 0) && useNonWeaponEffects)
		{
			//--------------------------------------------
			// Yes, load it on up.
			unsigned flags = gosFX::Effect::ExecuteFlag;
	
			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectMuzzleFlashName(effectId));
			
			if (gosEffectSpec)
			{
				muzzleEffect = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
				gosASSERT(muzzleEffect != NULL);
				
				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			
				//muzzleEffect->SetScalar(BASE_EFFECT_SCALAR);
			}
		}
		
		if (strcmp(weaponEffects->GetEffectHitName(effectId),"NONE") != 0)
		{
			//--------------------------------------------
			// Yes, load it on up.
			unsigned flags = gosFX::Effect::ExecuteFlag;
	
			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectHitName(effectId));
			
			if (gosEffectSpec)
			{
				hitEffect = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
				gosASSERT(hitEffect != NULL);
			
				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			
				//hitEffect->SetScalar(BASE_EFFECT_SCALAR);
			}
		}
		
		if ((strcmp(weaponEffects->GetEffectMissName(effectId),"NONE") != 0) && useNonWeaponEffects)
		{
			//--------------------------------------------
			// Yes, load it on up.
			unsigned flags = gosFX::Effect::ExecuteFlag;
	
			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectMissName(effectId));
			
			if (gosEffectSpec)
			{
				missEffect = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
				gosASSERT(missEffect != NULL);
			
				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			
				//hitEffect->SetScalar(BASE_EFFECT_SCALAR);
			}
		}
		
		//No water miss for machine guns
		if (useNonWeaponEffects && (effectId != 11) && (strcmp(weaponEffects->GetEffectName(WATER_MISS_FX),"NONE") != 0))
		{
			//--------------------------------------------
			// Yes, load it on up.
			unsigned flags = gosFX::Effect::ExecuteFlag;
	
			Check_Object(gosFX::EffectLibrary::Instance);
			gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(WATER_MISS_FX));
			
			if (gosEffectSpec)
			{
				waterMissEffect = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
				gosASSERT(waterMissEffect != NULL);
			
				MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
			
				//hitEffect->SetScalar(BASE_EFFECT_SCALAR);
			}
		}
		else
		{
			waterMissEffect = NULL;
		}
	}
	
	//----------------------------------------------
	// Get a texture Handle from the textureManager
	// Assume ALPHA!
	if (((WeaponBoltTypePtr)_type)->textureName && stricmp(((WeaponBoltTypePtr)_type)->textureName,"NONE") != 0)
	{
		char tPath[1024];
		sprintf(tPath,"%s128" PATH_SEPARATOR, tglPath);

		FullPathFileName textureName;
		textureName.init(tPath,((WeaponBoltTypePtr)_type)->textureName,".tga");
	
		if (((WeaponBoltTypePtr)_type)->mipTexture)
			mcTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,0);
		else
			mcTextureHandle = mcTextureManager->loadTexture(textureName,gos_Texture_Alpha,gosHint_DisableMipmap);
	}
	else
	{
		mcTextureHandle = 0;
		gosTextureHandle = 0xffffffff;
	}
}	

//---------------------------------------------------------------------------
void WeaponBolt::setOwner (GameObjectPtr who)
{
	if (who)
		ownerWID = who->getWatchID();
	else
		ownerWID = 0;
}

//---------------------------------------------------------------------------
void WeaponBolt::setTarget (GameObjectPtr who)
{
	if (who)
		targetWID = who->getWatchID();
	else
		targetWID = 0;
}

//---------------------------------------------------------------------------
void WeaponBolt::setTargetPosition (Stuff::Vector3D pos)
{
	if (!targetPosition)
		targetPosition = new Stuff::Vector3D;
		
	*targetPosition = pos;
}

//***************************************************************************
void WeaponBolt::Save (PacketFilePtr file, long packetNum)
{
	WeaponBoltData data;
	CopyTo(&data);

	//PacketNum incremented in ObjectManager!!
	file->writePacket(packetNum,(MemoryPtr)&data,sizeof(WeaponBoltData),STORAGE_TYPE_ZLIB);
}

//***************************************************************************
void WeaponBolt::CopyTo (WeaponBoltData *data)
{																	   
	data->ownerWID = ownerWID;
	data->hotSpotNumber = hotSpotNumber;

	data->targetWID = targetWID;
	data->targetHotSpot = targetHotSpot;

	Stuff::Vector3D dmy;
	dmy.Zero();

	if (targetPosition)
		data->targetPosition = *targetPosition;
	else
		data->targetPosition = dmy;

	data->distanceToTarget = distanceToTarget;
	data->halfDistanceToTarget = halfDistanceToTarget;

	data->weaponShot = weaponShot;

	data->laserPosition = laserPosition;
	memcpy(data->laserVertices,laserVertices,sizeof(Stuff::Vector3D) * 4);
	memcpy(data->laserSide,laserSide,sizeof(Stuff::Vector3D) * 4);

	data->effectId = effectId;
	data->hitTarget = hitTarget;

	data->timeLeft = timeLeft;
	data->hsPos = hsPos;

	data->hitLeft = hitLeft;
	data->mcTextureHandle = mcTextureHandle;
	data->gosTextureHandle = gosTextureHandle;
	data->startUV = startUV;
	data->goalHeight = goalHeight;

	GameObject::CopyTo(data);
}

//---------------------------------------------------------------------------
void WeaponBolt::Load (WeaponBoltData *data)
{
	GameObject::Load(data); 

	ownerWID = data->ownerWID;
	hotSpotNumber = data->hotSpotNumber;

	targetWID = data->targetWID;
	targetHotSpot = data->targetHotSpot;

	if ((data->targetPosition.x == data->targetPosition.y) &&
		(data->targetPosition.x == data->targetPosition.z))
		targetPosition = NULL;
	else
	{
		targetPosition = new Stuff::Vector3D;
		*targetPosition = data->targetPosition;
	}

	distanceToTarget = data->distanceToTarget;
	halfDistanceToTarget = data->halfDistanceToTarget;

	weaponShot = data->weaponShot;

	laserPosition = data->laserPosition;
	memcpy(laserVertices,data->laserVertices,sizeof(Stuff::Vector3D) * 4);
	memcpy(laserSide,data->laserSide,sizeof(Stuff::Vector3D) * 4);

	effectId = data->effectId;
	hitTarget = data->hitTarget;

	timeLeft = data->timeLeft;
	hsPos = data->hsPos;

	hitLeft = data->hitLeft;
	startUV = data->startUV;
	goalHeight = data->goalHeight;
}

//---------------------------------------------------------------------------
void WeaponBolt::finishNow (void)
{
	if (getExists())
	{
		//This weapon is still enroute or in some other way is not resolved.  Resolve it.
		// Then setExists to false!!

		//Toss its effects.
		if (hitEffect)
		{
			hitEffect->Kill();
			delete hitEffect;
			hitEffect = NULL;
		}

		if (muzzleEffect)
		{
			muzzleEffect->Kill();
			delete muzzleEffect;
			muzzleEffect = NULL;
		}

		if (missEffect)
		{
			missEffect->Kill();
			delete missEffect;
			missEffect = NULL;
		}

		if (waterMissEffect)
		{
			waterMissEffect->Kill();
			delete waterMissEffect;
			waterMissEffect = NULL;
		}

		if (gosEffect)
		{
			gosEffect->Kill();		//Effect is over.  Otherwise, wait until hit!
			delete gosEffect;
			gosEffect = NULL;
		}

		//Need to kill the light source here too!
		if (pointLight)
		{
			if (eye)
				eye->removeWorldLight(lightId,pointLight);
			systemHeap->Free(pointLight);
			pointLight = NULL;
		}

		GameObjectPtr target = ObjectManager->getByWatchID(targetWID);
		if (target)
		{
			if (MPlayer)
			{
				if (MPlayer->isServer())
					target->handleWeaponHit(&weaponShot, true);
			}
			else
				target->handleWeaponHit(&weaponShot);
		}

		if (((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum &&
			(((WeaponBoltTypePtr)getObjectType())->areaEffectDmg > 0.0f) &&
			(((WeaponBoltTypePtr)getObjectType())->areaEffectRad > 0.0f))
		{
			//Create the explosion here.  HIT or MISS, its the same one!!
			// Damage is done by explosion NOT by hit!
			ObjectManager->createExplosion(((WeaponBoltTypePtr)getObjectType())->hitEffectObjNum,
											NULL,*targetPosition,
											((WeaponBoltTypePtr)getObjectType())->areaEffectDmg,
											((WeaponBoltTypePtr)getObjectType())->areaEffectRad);
		}

		setExists(false);
	}
}

//---------------------------------------------------------------------------
