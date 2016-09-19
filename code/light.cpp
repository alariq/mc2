//***************************************************************************
//
//	Light.cpp -- File contains the Light Object code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef GAMEOBJ_H
#include"gameobj.h"
#endif

#ifndef LIGHT_H
#include"light.h"
#endif

#include<stdio.h>

#define LIGHT_DEPTH_FIXUP		-500

//***************************************************************************
//	LIGHT TYPE class
//***************************************************************************

GameObjectPtr LightType::createInstance (void) {

	LightPtr newLight = new Light;
	if (!newLight)
		return(NULL);

	newLight->init(true, this);

	return(newLight);
}

//---------------------------------------------------------------------------

void LightType::destroy (void) 
{
	ObjectType::destroy();
}
		
//---------------------------------------------------------------------------

long LightType::init (FilePtr objFile, unsigned long fileSize) {

	long result = 0;
	
	FitIniFile explFile;
	result = explFile.open(objFile,fileSize);
	if (result != NO_ERR)
		return(result);

	result = explFile.seekBlock("LightData");
	if (result != NO_ERR)
		return result;

	result = explFile.readIdBoolean("OneShotFlag",oneShotFlag);
	if (result != NO_ERR)
		return result;

	result = explFile.readIdFloat("AltitudeOffset",altitudeOffset);
	if (result != NO_ERR)
		return result;

	//------------------------------------------------------------------
	// Initialize the base object Type from the current file.
	result = ObjectType::init(&explFile);
	return(result);
}
	
//---------------------------------------------------------------------------

bool LightType::handleCollision (GameObjectPtr collidee, GameObjectPtr collider) {

	//-----------------------------
	// Nothing collides with light!
	return(false);
}

//---------------------------------------------------------------------------

bool LightType::handleDestruction (GameObjectPtr collidee, GameObjectPtr collider) {

	//----------------------------
	// No destroy'n light, either!
	return(false);
}

//***************************************************************************
// LIGHT class
//***************************************************************************

void Light::init (bool create) {

	setFlag(OBJECT_FLAG_JUSTCREATED, true);
}

//---------------------------------------------------------------------------

long Light::update (void) {

	if (!getFlag(OBJECT_FLAG_DONE)) {
		//----------------------------------
		//-- Always force position altitude 'cause set every frame.
		position.z += ((LightTypePtr)getObjectType())->altitudeOffset;
		setPosition(position);
	
		if (getFlag(OBJECT_FLAG_JUSTCREATED)) {
			setFlag(OBJECT_FLAG_JUSTCREATED, false);
			setFlag(OBJECT_FLAG_TANGIBLE, false);
		}
	
#ifdef USE_LIGHT_APPEARANCE
		//-----------------------------------------
		// Light is NEVER Done.  Unless its done!
		bool inView = onScreen();
		lightAppearance->setInView(inView);
		long result = lightAppearance->update();
		if (!result && ((LightTypePtr)getObjectType())->oneShotFlag)
			setFlag(OBJECT_FLAG_DONE, true);
#endif
	}

	return (true);
}

//---------------------------------------------------------------------------

void Light::render (void) {

	if (gamePaused)
		onScreen();

	if (!getFlag(OBJECT_FLAG_JUSTCREATED) && (windowsVisible == turn) && !getFlag(OBJECT_FLAG_DONE)) {
#ifdef USE_LIGHT_APPEARANCE
		lightAppearance->render(LIGHT_DEPTH_FIXUP);
#endif
	}
}

//---------------------------------------------------------------------------

void Light::destroy (void) 
{
}

//---------------------------------------------------------------------------

void Light::init (bool create, ObjectTypePtr _type) {

	//-------------------------------------------
	// Initialize the Light Appearance here.
	GameObject::init(create, _type);

	setFlag(OBJECT_FLAG_JUSTCREATED, true);
	setFlag(OBJECT_FLAG_TANGIBLE, false);

	//-------------------------------------------------------------
	// The appearance is initialized here using data from the type
#ifdef USE_LIGHT_APPEARANCE
	unsigned long appearanceType = _type->getAppearanceTypeNum();
	AppearanceTypePtr lightAppearanceType = appearanceTypeList->getAppearance(appearanceType);
	if (!lightAppearanceType)
		return(NO_APPEARANCE_TYPE_FOR_EXPL);
		
	//--------------------------------------------------------------
	// The only kind of appearanceType the explosions currently know how
	// to work with is a spriteTree.  Anything else is wrong.	
	if (lightAppearanceType->getAppearanceClass() == VFX_APPEAR)
	{
		lightAppearance = new VFXAppearance;
		if (!lightAppearance)
			return(NO_APPEARANCE_FOR_EXPL);

		result = lightAppearance->init((VFXAppearanceType *)lightAppearanceType, (GameObjectPtr)this);
		if (result != NO_ERR)
			return(result);
	}
	else
	{
		return(APPEARANCE_NOT_VFX_XPL);
	}
#endif

	objectClass = KLIEG_LIGHT;
	setFlag(OBJECT_FLAG_DONE, false);
}	

//***************************************************************************

