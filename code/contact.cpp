//***************************************************************************
//
//	contact.cpp - This file contains the Contact and Sensor Classes code
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifndef CONTACT_H
#include"contact.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

//#ifndef GVEHICL_H
#include"gvehicl.h"
//#endif

#ifndef TEAM_H
#include"team.h"
#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#ifndef SOUNDSYS_H
#include"soundsys.h"
#endif

#include<gameos.hpp>

//***************************************************************************

SensorSystemManagerPtr		SensorManager = NULL;

long						SensorSystem::numSensors = 0;
SortListPtr					SensorSystem::sortList = NULL;
float						SensorSystem::scanFrequency = 0.5;

bool						TeamSensorSystem::homeTeamInContact = false;
bool						SensorSystemManager::enemyInLOS = true;

extern float scenarioTime;
extern UserHeapPtr missionHeap;

#define	VISUAL_CONTACT_FLAG	0x8000

//***************************************************************************
// CONTACT INFO class
//***************************************************************************

void* ContactInfo::operator new (size_t ourSize) {

	void* result;
	result = missionHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void ContactInfo::operator delete (void* us) {

	missionHeap->Free(us);
}	

//***************************************************************************
// SENSOR SYSTEM routines
//***************************************************************************

void* SensorSystem::operator new (size_t mySize) {

	void *result = missionHeap->Malloc(mySize);
	return(result);
}

//---------------------------------------------------------------------------

void SensorSystem::operator delete (void* us) {

	missionHeap->Free(us);
}

//---------------------------------------------------------------------------

void SensorSystem::init (void) {

	//id = 0			// this should be set by the sensor system manager only!
	master = NULL;
	masterIndex = -1;
	owner = NULL;
	range = -1.0;
	skill = -1;
	broken = false;
	notShutdown = true;

	ecmEffect = 1.0;

	//ALWAYS true UNLESS this is a sensor probe or a sensor tower!!
	hasLOSCapability = true;
	
	//----------------------------------------------------
	// Don't update any sensors on the very first frame...
	nextScanUpdate = 0.5;
	
	lastScanUpdate = 0.0;
	
	numContacts = 0;
	id = numSensors++;

	numExclusives = 0;
	totalContacts = 0;

	if (!sortList) {
		sortList = new SortList;
		if (!sortList)
			Fatal(0, " Unable to create Contact::sortList ");
		sortList->init(MAX_CONTACTS_PER_SENSOR);
	}
}

//---------------------------------------------------------------------------

void SensorSystem::destroy (void) {

	numSensors--;
	if (numSensors == 0) {
		delete sortList;
		sortList = NULL;
	}
}

//---------------------------------------------------------------------------

void SensorSystem::setMaster (TeamSensorSystemPtr newMaster) {

	master = newMaster;
	broken = false;
}

//---------------------------------------------------------------------------

void SensorSystem::setOwner (GameObjectPtr newOwner) {

	owner = newOwner;
}

//---------------------------------------------------------------------------

void SensorSystem::disable (void) {

	clearContacts();
}

//---------------------------------------------------------------------------

void SensorSystem::setShutdown (bool setting) {

	if (setting) {
		if (notShutdown) {
			long i = 0;
			while (i < numContacts) {
				MoverPtr mover = (MoverPtr)ObjectManager->get(contacts[i] & 0x7FFF);
				if (contacts[i] & VISUAL_CONTACT_FLAG) {
					master->removeContact(this, mover);
					i++;
					}
				else
					removeContact(i);
			}
		}
//		clearContacts();
		notShutdown = false;
		}
	else {
		if (!notShutdown)
			for (long i = 0; i < numContacts; i++) {
				MoverPtr mover = (MoverPtr)ObjectManager->get(contacts[i] & 0x7FFF);
				if (contacts[i] & VISUAL_CONTACT_FLAG)
					master->addContact(this, mover, i, CONTACT_VISUAL);
				else
					master->addContact(this, mover, i, getSensorQuality());
			}
		notShutdown = true;
	}
}

//---------------------------------------------------------------------------

bool SensorSystem::enabled (void) {

	if (master && (masterIndex > -1)) {
		if (!owner->getExistsAndAwake())
			return(false);
		if (owner->isDisabled())
			return(false);
		if (owner->isMover()) {
			MoverPtr mover = (MoverPtr)owner;
			return((mover->sensor != 255) && !mover->inventory[mover->sensor].disabled);
		}
		return(true);
	}
	return(false);
}

//---------------------------------------------------------------------------
void SensorSystem::setRange (float newRange) {

	range = newRange;
}

//---------------------------------------------------------------------------
float SensorSystem::getRange (void) 
{
	float result = range;

	/*		NO More recon specialist
	if (owner && owner->isMover())
	{
		MoverPtr mover = (MoverPtr)owner;
		if (mover->pilot && mover->pilot->isReconSpecialist())
			result += range * 0.2f;
	}
	*/
	
	return result;
}

//---------------------------------------------------------------------------

void SensorSystem::setSkill (long newSkill) {

	skill = newSkill;
}

//---------------------------------------------------------------------------

float SensorSystem::getEffectiveRange (void) {

	return(range * ecmEffect);
}

//---------------------------------------------------------------------------

long SensorSystem::calcContactStatus (MoverPtr mover) {

	if (!owner->getTeam())
		return(CONTACT_NONE);

	if (mover->getFlag(OBJECT_FLAG_REMOVED)) {
		return(CONTACT_NONE);
	}

	//----------------------------------------------------------
	//If object we are looking for is at the edge, NO CONTACT!!
	if (!Terrain::IsGameSelectTerrainPosition(mover->getPosition()))
		return CONTACT_NONE;
		
	//-------------------------------------------------------------
	// Should be properly set when active probes are implemented...
	long newContactStatus = CONTACT_NONE;
	if (!notShutdown || (range == 0.0) && !broken) 
	{
		if (owner->lineOfSight(mover) && !mover->isDisabled())
			newContactStatus = CONTACT_VISUAL;
		return(newContactStatus);
	}

	if ((masterIndex == -1) || (range < 0.0)) {
		return(CONTACT_NONE);
	}

	if (owner->isMover()) {
		MoverPtr mover = (MoverPtr)owner;
		if ((mover->sensor == 255) || mover->inventory[mover->sensor].disabled || broken) {
			return(CONTACT_NONE);
		}
	}

	if (mover->getFlag(OBJECT_FLAG_SENSOR) && !mover->isDisabled()) 
	{
		bool moverNotContact = mover->hasNullSignature() || (mover->getEcmRange() != 0.0f);
		bool moverInvisible = (mover->getStatus() == OBJECT_STATUS_SHUTDOWN);
		if (!moverInvisible && !moverNotContact)
		{
			float distanceToMover = owner->distanceFrom(mover->getPosition());
			float sensorRange = getEffectiveRange();
			if (distanceToMover <= sensorRange)
			{
				//-------------------------------------------
				//No need to check shutdown and probe AGAIN!
				newContactStatus = getSensorQuality();

				//---------------------------------------
				// If ecm affecting me, my skill drops...
				// CUT, per Mitch 2/10/00
				if ((ecmEffect < 1.0) && (newContactStatus > CONTACT_SENSOR_QUALITY_1))
					newContactStatus--;

				//---------------------------------------------------
				// We now we are within sensor range, check visual.
				float startRadius = 0.0f;
				if (!owner->isMover())
					startRadius = owner->getAppearRadius();

				if (hasLOSCapability && owner->lineOfSight(mover,startRadius))
					newContactStatus = CONTACT_VISUAL;
			}
			else	//Still need to check if visual!!! ECM and Lookout Towers!!
			{
				float startRadius = 0.0f;
				if (!owner->isMover())
					startRadius = owner->getAppearRadius();

				if (hasLOSCapability && owner->lineOfSight(mover,startRadius))
					newContactStatus = CONTACT_VISUAL;
			}
		}
		else
		{
			//Target is shutdown, can ONLY be visual cause this platform has no probe.
			float startRadius = 0.0f;
			if (!owner->isMover())
				startRadius = owner->getAppearRadius();

			if (hasLOSCapability && owner->lineOfSight(mover,startRadius))
		    	newContactStatus = CONTACT_VISUAL;
		}
	}

	//Let us know that we can see something, sensor or otherwise!!
	if (mover->getTeam() && (owner->getTeam() == Team::home) &&	mover->getTeam()->isEnemy(Team::home) &&
		(newContactStatus != CONTACT_NONE))
	{
		SensorSystemManager::enemyInLOS = true;
	}
 
	return(newContactStatus);
}

//---------------------------------------------------------------------------

long SensorSystem::getSensorQuality (void) {

	if (owner && owner->getPilot())
	{
		if (owner->getPilot()->isSensorProfileSpecialist())
		{
			if (owner->getPilot()->getRank() < 4)
			{
				return(owner->getPilot()->getRank() + CONTACT_SENSOR_QUALITY_1);
			}
			else if (owner->getPilot()->getRank() == 4)
			{
				return(CONTACT_SENSOR_QUALITY_4);
			}
		}
	}
	
	return(CONTACT_SENSOR_QUALITY_1);
}

//---------------------------------------------------------------------------

bool SensorSystem::isContact (MoverPtr mover) {

	if (mover->getFlag(OBJECT_FLAG_REMOVED))
		return(false);
	if (notShutdown)
		return(mover->getContactInfo()->getSensor(id) < 255);
	for (long i = 0; i < numContacts; i++)
		if ((contacts[i] & 0x7FFF) == mover->getHandle())
			return(true);
	return(false);
}

//---------------------------------------------------------------------------

void SensorSystem::addContact (MoverPtr mover, bool visual) {

	Assert(!isContact(mover), 0, " SensorSystem.addContact: already contact ");
	if (numContacts < MAX_CONTACTS_PER_SENSOR) {
		contacts[numContacts] = mover->getHandle();
		if (visual)
			contacts[numContacts] |= VISUAL_CONTACT_FLAG;
		if (notShutdown)
			master->addContact(this, mover, numContacts, visual ? CONTACT_VISUAL :  getSensorQuality());
		numContacts++;
	}
}

//---------------------------------------------------------------------------

void SensorSystem::modifyContact (MoverPtr mover, bool visual) {

	long contactNum = mover->getContactInfo()->getSensor(id);
	if (contactNum < MAX_CONTACTS_PER_SENSOR)
	{
		if (visual)
			contacts[contactNum] =  mover->getHandle() | 0x8000;
		else
			contacts[contactNum] =  mover->getHandle();
	}

	if (notShutdown)
		master->modifyContact(this, mover, visual ? CONTACT_VISUAL :  getSensorQuality());
}

//---------------------------------------------------------------------------

void SensorSystem::removeContact (long contactIndex) {

	//-----------------------------------------------
	// This assumes the contactIndex is legitimate...

	MoverPtr contact = (MoverPtr)ObjectManager->get(contacts[contactIndex] & 0x7FFF);
	Assert(contact != NULL, contacts[contactIndex] & 0x7FFF, " SensorSystem.removeContact: bad contact ");
	
	numContacts--;
	if ((numContacts > 0) && (contactIndex != numContacts)) {
		//-----------------------------------------------
		// Fill vacated slot with contact in last slot...
		contacts[contactIndex] = contacts[numContacts];
		MoverPtr contact = (MoverPtr)ObjectManager->get(contacts[numContacts] & 0x7FFF);
		contact->getContactInfo()->sensors[id] = contactIndex;
	}

	if (notShutdown)
		master->removeContact(this, contact);
}

//---------------------------------------------------------------------------

void SensorSystem::removeContact (MoverPtr contact) {

	long contactIndex = contact->getContactInfo()->getSensor(id);
	if (contactIndex < 255)
		removeContact(contactIndex);
}

//---------------------------------------------------------------------------

void SensorSystem::clearContacts (void) {

	while (numContacts)
		removeContact((long)0);
}

//---------------------------------------------------------------------------

void SensorSystem::updateContacts (void) {

	if ((masterIndex == -1) || (range < 0.0))
		return;

	if (!enabled()) {
		clearContacts();
		return;
	}

	//---------------------------------------------------------------------
	// If we've already scanned this frame, don't bother updating contacts.
	// Otherwise, update contacts...
	if (scenarioTime == lastScanUpdate)
		return;

	long i = 0;
	while (i < numContacts) 
	{
		MoverPtr contact = (MoverPtr)ObjectManager->get(contacts[i] & 0x7FFF);
		long contactStatus = calcContactStatus(contact);
		if (contactStatus == CONTACT_NONE)
			removeContact(i);
		else 
		{
			contacts[i] =  contact->getHandle();
			if (contactStatus == CONTACT_VISUAL)
				contacts[i] |= 0x8000;
			modifyContact(contact, contactStatus == CONTACT_VISUAL);
			
/*			if (teamContactStatus < contactStatus) {
				//--------------------------------------------------
				// Better sensor info, so update the team sensors...
				contactInfo->contactStatus[owner->getTeamId()] = contactStatus;
				contactInfo->teamSpotter[owner->getTeamId()] = (unsigned char)owner->getHandle();
			}
*/			i++;
		}
	}
}

//---------------------------------------------------------------------------
#define CONTACT_TYPE1	0
#define CONTACT_TYPE2	1
#define CONTACT_TYPE3	2
#define CONTACT_TYPE4	3

void SensorSystem::updateScan (bool forceUpdate) {

	if (!forceUpdate)
		if ((masterIndex == -1) || (range < 0.0) || (turn < 2))
			return;

	if (!enabled()) {
		clearContacts();
		return;
	}

	if (!owner->getTeam())
		return;

	if (1/*(nextScanUpdate < scenarioTime) || forceUpdate*/) 
	{
	
		long currentScan = -1;
		if ((currentScan = scanBattlefield()) > -1)		//No returns size of largest contact.
		{
			if (owner->isMover() && 
				(owner->getTeam() == Team::home) && 
				(currentScan > SoundSystem::largestSensorContact))
			{
				SoundSystem::largestSensorContact = currentScan;
			}
		}

		lastScanUpdate = scenarioTime;
		if (!forceUpdate)
			nextScanUpdate += scanFrequency;
	}
}

//---------------------------------------------------------------------------
__inline void getLargest (long &currentLargest, MoverPtr mover, long contactStatus)
{
	long thisMoverSize = -1;
	switch (contactStatus)
	{
		case	CONTACT_SENSOR_QUALITY_1:
		case	CONTACT_SENSOR_QUALITY_2:
			thisMoverSize = 0;
			break;

		case	CONTACT_SENSOR_QUALITY_3:
		case	CONTACT_SENSOR_QUALITY_4:
		case	CONTACT_VISUAL:
			float tonnage = mover->getTonnage();
			if (tonnage < 35.0f)
				thisMoverSize = 1;
			else if (tonnage < 55.0f)
				thisMoverSize = 2;
			else if (tonnage < 75.0f)
				thisMoverSize = 3;
			else
				thisMoverSize = 4;
			break;
	}

	if (thisMoverSize > currentLargest)
		currentLargest = thisMoverSize;
}

//---------------------------------------------------------------------------
long SensorSystem::scanBattlefield (void) 
{
	//NOW returns size of largest contact!
	long currentLargest = -1;
	
	if (!owner)
		Fatal(0, " Sensor has no owner ");

	if (!master)
		Fatal(0, " Sensor has no master ");

	if ((masterIndex == -1) || (range < 0.0))
		return(0);

	long numNewContacts = 0;

	long numMovers = ObjectManager->getNumMovers();
	for (long i = 0; i < numMovers; i++) 
	{
		MoverPtr mover = (MoverPtr)ObjectManager->getMover(i);
		if (mover->getExists() && (mover->getTeamId() != owner->getTeamId())) 
		{
			long contactStatus = calcContactStatus(mover);
			if (isContact(mover)) 
			{
				if (contactStatus == CONTACT_NONE)
					removeContact(mover);
				else
				{
					modifyContact(mover, contactStatus == CONTACT_VISUAL ? true : false);
					getLargest(currentLargest,mover,contactStatus);
				}
			}
			else 
			{
				if (contactStatus != CONTACT_NONE) 
				{
					addContact(mover, contactStatus == CONTACT_VISUAL ? true : false);
					getLargest(currentLargest,mover,contactStatus);
					numNewContacts++;
				}
			}
		}
	}

	totalContacts += numNewContacts;
	
	return(currentLargest);
}

//---------------------------------------------------------------------------

long SensorSystem::scanMover (Mover* mover) {

	//---------------------------------------------------------------------
	// For now, I DO NOT return the largest contact. This should be
	// okay, since it'll just get caught during the next normal update next
	// frame.

	if (!enabled())
		return(0);

	if (mover->getExists() && (mover->getTeamId() != owner->getTeamId())) {
		long contactStatus = calcContactStatus(mover);
		if (isContact(mover)) 
		{
			if (contactStatus == CONTACT_NONE)
				removeContact(mover);
			else
			{
				modifyContact(mover, contactStatus == CONTACT_VISUAL ? true : false);
				//getLargest(currentLargest,mover,contactStatus);
			}
		}
		else 
		{
			if (contactStatus != CONTACT_NONE) 
			{
				addContact(mover, contactStatus == CONTACT_VISUAL ? true : false);
				//getLargest(currentLargest,mover,contactStatus);
				totalContacts++;
			}
		}
	}
	return(0);
}

//---------------------------------------------------------------------------

long SensorSystem::getTeamContacts (long* contactList, long contactCriteria, long sortType) {

	Assert(master != NULL, 0, " SensorSystem.getTeamContacts: null master ");
	return(master->getContacts(owner, contactList, contactCriteria, sortType));
}

//***************************************************************************
// TEAM SENSOR SYSTEM class
//***************************************************************************

void* TeamSensorSystem::operator new (size_t mySize) {

	void *result = missionHeap->Malloc(mySize);
	return(result);
}

//---------------------------------------------------------------------------

void TeamSensorSystem::operator delete (void* us) {

	missionHeap->Free(us);
}

//---------------------------------------------------------------------------

void TeamSensorSystem::init (void) {

	teamId = -1;
	nextContactId = 0;
	numContactUpdatesPerPass = NUM_CONTACT_UPDATES_PER_PASS;
	curContactUpdate = 0;
	numContacts = 0;
	numEnemyContacts = 0;
	numSensors = 0;
	ecms = NULL;
	jammers = NULL;
}

//---------------------------------------------------------------------------
void TeamSensorSystem::destroy (void)
{
	init();
}

//---------------------------------------------------------------------------

void TeamSensorSystem::setTeam (TeamPtr newTeam) {

	teamId = newTeam->getId();

	if (Team::teams[teamId]->rosterSize < NUM_CONTACT_UPDATES_PER_PASS)
		numContactUpdatesPerPass = Team::teams[teamId]->rosterSize;
	else
		numContactUpdatesPerPass = NUM_CONTACT_UPDATES_PER_PASS;
}

//---------------------------------------------------------------------------

void TeamSensorSystem::addSensor (SensorSystemPtr sensor) {

	if (numSensors == MAX_SENSORS_PER_TEAM)
		Fatal(0, " TeamSensorSystem.addSensor: too many sensors ");

	sensor->setMasterIndex(numSensors);
	sensors[numSensors++] = sensor;
}

//---------------------------------------------------------------------------

void TeamSensorSystem::removeSensor (SensorSystemPtr sensor) {

	sensor->clearContacts();
	long index = sensor->getMasterIndex();
	sensor->setMasterIndex(-1);

	sensors[index] = NULL;
	if (index < (numSensors - 1)) {
		sensors[index] = sensors[numSensors - 1];
		sensors[index]->setMasterIndex(index);
		sensors[numSensors - 1] = NULL;
	}
	numSensors--;
}

//---------------------------------------------------------------------------

void TeamSensorSystem::update (void) {

	if (numSensors > 0) {
#if 0
		//DEBUGGING
		for (long k = 0; k < numContacts; k++) {
			MoverPtr mover = (MoverPtr)ObjectManager->get(contacts[k]);
			Assert(mover->getContactInfo()->teams[teamId] == k, 0, " Bad teams/contact link ");
		}
#endif
		
		if (Team::teams[teamId] == Team::home)
			SoundSystem::largestSensorContact = -1;

		//---------------------------------
		// First, update actual scanning...
		for (long i = 0; i < numSensors; i++)
			sensors[i]->updateScan();

		if (Team::teams[teamId]->rosterSize < NUM_CONTACT_UPDATES_PER_PASS)
			numContactUpdatesPerPass = Team::teams[teamId]->rosterSize;
		else
			numContactUpdatesPerPass = NUM_CONTACT_UPDATES_PER_PASS;

		//--------------------------------
		// Now, update current contacts...
		for (i = 0; i < numContactUpdatesPerPass; i++) {
			if (curContactUpdate >= numSensors)
				curContactUpdate = 0;
			sensors[curContactUpdate]->updateContacts();
			curContactUpdate++;
		}
	}
}

//---------------------------------------------------------------------------

long TeamSensorSystem::getVisualContacts (MoverPtr* moverList) {

	long numVisualContacts = 0;
	for (long i = 0; i < numContacts; i++) {
		MoverPtr contact = (MoverPtr)ObjectManager->get(contacts[i]);
		if (!contact->getFlag(OBJECT_FLAG_REMOVED))
			if (contact->getContactStatus(teamId, true) == CONTACT_VISUAL)
				moverList[numVisualContacts++] = contact;
	}
	return(numVisualContacts);
}

//---------------------------------------------------------------------------

long TeamSensorSystem::getSensorContacts (MoverPtr* moverList) {

	static bool isSensor[NUM_CONTACT_STATUSES] = {false, true, true, true, true, false};
	long numSensorContacts = 0;
	for (long i = 0; i < numContacts; i++) {
		MoverPtr contact = (MoverPtr)ObjectManager->get(contacts[i]);
		if (!contact->getFlag(OBJECT_FLAG_REMOVED))
			if (isSensor[contact->getContactStatus(teamId, true)])
				moverList[numSensorContacts++] = contact;
	}
	return(numSensorContacts);
}

//---------------------------------------------------------------------------

bool TeamSensorSystem::meetsCriteria (GameObjectPtr looker, MoverPtr mover, long contactCriteria) {

	bool isSensor[NUM_CONTACT_STATUSES] = {false, true, true, true, true, false};
	long status = mover->getContactStatus(teamId, true);

	if (mover->getFlag(OBJECT_FLAG_REMOVED))
		return(false);

	if (status == CONTACT_NONE)
		return(false);

	if (contactCriteria & CONTACT_CRITERIA_VISUAL_OR_SENSOR)
		if ((status != CONTACT_VISUAL) && !isSensor[status])
			return(false);

	if (contactCriteria & CONTACT_CRITERIA_VISUAL)
		if (status != CONTACT_VISUAL)
			return(false);

	if (contactCriteria & CONTACT_CRITERIA_SENSOR)
		if (!isSensor[status])
			return(false);

	if (contactCriteria & CONTACT_CRITERIA_NOT_CHALLENGED) {
		if (mover->getChallenger() != NULL)
			return(false);
	}

	if (contactCriteria & CONTACT_CRITERIA_NOT_DISABLED) {
		if (mover->isDisabled())
			return(false);
	}

	if (contactCriteria & CONTACT_CRITERIA_ENEMY) {
		if ((teamId == -1) || !mover->getTeam())
			return(false);
		if (!Team::teams[teamId]->isEnemy(mover->getTeam()))
			return(false);
	}

	if (contactCriteria & CONTACT_CRITERIA_ARMED) {
		if (mover->numFunctionalWeapons == 0)
			return(false);
	}

	return(true);
}

//---------------------------------------------------------------------------

bool TeamSensorSystem::hasSensorContact (long teamID) {

	for (long i = 0; i < numContacts; i++) {
		MoverPtr mover = (MoverPtr)ObjectManager->get(contacts[i]);
		if (!mover->getFlag(OBJECT_FLAG_REMOVED)) {
			static bool isSensor[NUM_CONTACT_STATUSES] = {false, true, true, true, true, false};
			if (isSensor[mover->getContactStatus(teamID, true)])
				return(true);
		}
	}
	return(false);
}

//---------------------------------------------------------------------------

long TeamSensorSystem::getContacts (GameObjectPtr looker, long* contactList, long contactCriteria, long sortType) {

	if ((sortType != CONTACT_SORT_NONE) && !looker)
		return(0);

	static float sortValues[MAX_CONTACTS_PER_SENSOR];

	float CV = 0;
	long numValidContacts = 0;
	long handleList[MAX_CONTACTS_PER_SENSOR];
	for (long i = 0; i < numContacts; i++) {
		MoverPtr mover = (MoverPtr)ObjectManager->get(contacts[i]);
		if (!meetsCriteria(looker, mover, contactCriteria))
			continue;
		handleList[numValidContacts] = mover->getHandle();
		switch (sortType) {
			case CONTACT_SORT_NONE:
				sortValues[numValidContacts] = 0.0;
				break;
			case CONTACT_SORT_CV:
				CV = (float)mover->getCurCV();
				sortValues[numValidContacts] = CV;
				break;
			case CONTACT_SORT_DISTANCE:
				sortValues[numValidContacts] = looker->distanceFrom(mover->getPosition());
				break;
		}
		numValidContacts++;
	}

	if ((numValidContacts > 0) && (sortType != CONTACT_SORT_NONE)) {
		//---------------------------------------------------------
		// BIG ASSUMPTION HERE: That a mech will not have more than
		// MAX_CONTACTS_PER_SENSOR contacts.
		if (!SensorSystem::sortList) {
			SensorSystem::sortList = new SortList;
			if (!SensorSystem::sortList)
				Fatal(0, " Unable to create Contact sortList ");
			SensorSystem::sortList->init(MAX_CONTACTS_PER_SENSOR);
		}
		bool descendSort = true;
		if (sortType == CONTACT_SORT_DISTANCE)
			descendSort = false;
		SensorSystem::sortList->clear(descendSort);
		for (long contact = 0; contact < numValidContacts; contact++) {
			SensorSystem::sortList->setId(contact, handleList[contact]);
			SensorSystem::sortList->setValue(contact, sortValues[contact]);
		}
		SensorSystem::sortList->sort(descendSort);
		for (contact = 0; contact < numValidContacts; contact++)
			contactList[contact] = SensorSystem::sortList->getId(contact);
		}
	else if (contactList)
		for (long contact = 0; contact < numValidContacts; contact++)
			contactList[contact] = handleList[contact];


	return(numValidContacts);
}

//---------------------------------------------------------------------------

long TeamSensorSystem::getContactStatus (MoverPtr mover, bool includingAllies) {

	if (mover->getFlag(OBJECT_FLAG_REMOVED))
		return(CONTACT_NONE);
	return(mover->getContactInfo()->getContactStatus(Team::teams[teamId]->getId(), true));
}

//---------------------------------------------------------------------------

void TeamSensorSystem::scanBattlefield (void) {

	if (numSensors)
		for (long i = 0; i < numSensors; i++)
			sensors[i]->updateScan(true);
}

//---------------------------------------------------------------------------

void TeamSensorSystem::scanMover (Mover* mover) {

	if (numSensors)
		for (long i = 0; i < numSensors; i++)
			sensors[i]->scanMover(mover);
}

//---------------------------------------------------------------------------

void TeamSensorSystem::incNumEnemyContacts (void) {

	numEnemyContacts++;
	if ((Team::teams[teamId] == Team::home) && numEnemyContacts)
		homeTeamInContact = true;
}

//---------------------------------------------------------------------------

void TeamSensorSystem::decNumEnemyContacts (void) {

	numEnemyContacts--;
	if (!numEnemyContacts && (Team::teams[teamId] == Team::home))
		homeTeamInContact = false;
		
	if (numEnemyContacts < 0)
		Fatal(0, " Negative Team Contact Count ");
}

//---------------------------------------------------------------------------

void TeamSensorSystem::addContact (SensorSystemPtr sensor, MoverPtr contact, long contactIndex, long contactStatus) {

	Assert(numContacts < MAX_CONTACTS_PER_TEAM, numContacts, " TeamSensorSystem.addContact: max team contacts ");
	ContactInfoPtr contactInfo = contact->getContactInfo();
	contactInfo->sensors[sensor->id] = contactIndex;
	contactInfo->contactCount[teamId]++;
	if (contactInfo->contactStatus[teamId] < contactStatus) {
		contactInfo->contactStatus[teamId] = contactStatus;
		contactInfo->teamSpotter[teamId] = sensor->owner->getHandle();

	}
	if (contactInfo->contactCount[teamId] == 1) {
		contacts[numContacts] = contact->getHandle();
		contactInfo->teams[teamId] = numContacts;
		numContacts++;
		sensor->numExclusives++;
	}
}

//---------------------------------------------------------------------------

SensorSystemPtr TeamSensorSystem::findBestSpotter (MoverPtr contact, long* status) {

	ContactInfoPtr contactInfo = contact->getContactInfo();
	if (!contactInfo) {
		char s[256];
		sprintf(s, "TeamSensorSystem.findBestSpotter: NULL contactInfo for objClass %d partID %d team %d", contact->getObjectClass(), contact->getPartId(), contact->getTeamId());
		STOP((s));
	}
	SensorSystemPtr bestSensor = NULL;
	long bestStatus = CONTACT_NONE;
	for (long i = 0; i < MAX_SENSORS; i++)
		if (contactInfo->sensors[i] != 255) {
			SensorSystemPtr sensor = SensorManager->getSensor(i);
			if (sensor && sensor->owner && (teamId == sensor->owner->getTeamId())) {
				long status = sensor->calcContactStatus(contact);
				if (status >= bestStatus) {
					bestSensor = sensor;
					bestStatus = status;
				}
			}
		}
	if (status)
		*status = bestStatus;
	return(bestSensor);
}

//---------------------------------------------------------------------------

void TeamSensorSystem::modifyContact (SensorSystemPtr sensor, MoverPtr contact, long contactStatus) {

	ContactInfoPtr contactInfo = contact->getContactInfo();
	if (contactInfo->teamSpotter[teamId] == sensor->owner->getHandle()) {
		long curStatus = contactInfo->contactStatus[teamId];
		if (contactStatus > curStatus)
			contactInfo->contactStatus[teamId] = contactStatus;
		else if (contactStatus < curStatus) {
			long bestStatus;
			SensorSystemPtr bestSensor = findBestSpotter(contact, &bestStatus);
			contactInfo->contactStatus[teamId] = bestStatus;
			contactInfo->teamSpotter[teamId] = bestSensor->owner->getHandle();
		}
		}
	else {
		long curStatus = contactInfo->contactStatus[teamId];
		if (contactStatus > curStatus) {
			contactInfo->contactStatus[teamId] = contactStatus;
			contactInfo->teamSpotter[teamId] = sensor->owner->getHandle();
		}
	}
}

//---------------------------------------------------------------------------

void TeamSensorSystem::removeContact (SensorSystemPtr sensor, MoverPtr contact) {

	ContactInfoPtr contactInfo = contact->getContactInfo();
	contactInfo->sensors[sensor->id] = 255;
	contactInfo->contactCount[teamId]--;
	Assert(contactInfo->contactCount[teamId] != 255, 0, "FUDGE");

	if (contactInfo->contactCount[teamId] == 0) {
		long contactIndex = contactInfo->teams[teamId];
		Assert(contactIndex < 0xFFFF, contactIndex, " 0xFFFF ");
		MoverPtr removedContact = (MoverPtr)ObjectManager->get(contacts[contactIndex]);
		Assert(removedContact == contact, contactIndex, " TeamSensorSystem.removeContact: bad contact ");
		
		contactInfo->teams[teamId] = 0xFFFF;
		contactInfo->contactStatus[teamId] = CONTACT_NONE;
		contactInfo->teamSpotter[teamId] = 0;         
	
		numContacts--;
		if ((numContacts > 0) && (contactIndex != numContacts)) {
			//-----------------------------------------------
			// Fill vacated slot with contact in last slot...
			contacts[contactIndex] = contacts[numContacts];
			MoverPtr mover = (MoverPtr)ObjectManager->get(contacts[numContacts]);
			mover->getContactInfo()->teams[teamId] = contactIndex;
		}
		}
	else if (contactInfo->teamSpotter[teamId] == sensor->owner->getHandle()) {
		if (sensor->owner->getObjectClass() == BATTLEMECH)
			Assert(sensor != NULL, 0, " dumb ");
		long bestStatus;
		SensorSystemPtr bestSensor = findBestSpotter(contact, &bestStatus);
//		Assert(bestSensor != NULL, 0, " hit ");
		if (bestSensor) {
			contact->contactInfo->contactStatus[teamId] = bestStatus;
			contact->contactInfo->teamSpotter[teamId] = bestSensor->owner->getHandle();
		}
	}
}

//---------------------------------------------------------------------------
#if 0
void TeamSensorSystem::updateContactList (void) {

	numAllContacts = 0;
	for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
		MoverPtr mover = ObjectManager->getMover(i);
		if (mover) {
			long bestStatus = CONTACT_NONE;
			for (long i = 0; i < Team::numTeams; i++) {
				if (Team::teams[teamId]->isFriendly(Team::teams[i]))
					if (mover->contactInfo->contactStatus[i] > bestStatus)
						bestStatus = mover->contactInfo->contactStatus[i];
			}
			mover->contactInfo->allContactStatus[teamId] = bestStatus;
			if (bestStatus != CONTACT_NONE) {
				allContacts[numAllContacts++] = mover->getHandle();
			}
		}
	}
}
#endif
//---------------------------------------------------------------------------

void TeamSensorSystem::updateEcmEffects (void) {

	for (long i = 0; i < numSensors; i++)
		sensors[i]->ecmEffect = SensorManager->getEcmEffect(sensors[i]->owner); 
}

//***************************************************************************
// SENSOR SYSTEM MANAGER class
//***************************************************************************

void* SensorSystemManager::operator new (size_t mySize) {

	void *result = missionHeap->Malloc(mySize);
	return(result);
}

//---------------------------------------------------------------------------

void SensorSystemManager::operator delete (void* us) {

	missionHeap->Free(us);
}

//---------------------------------------------------------------------------

long SensorSystemManager::init (bool debug) {

	if (MAX_SENSORS < 2)
		Fatal(0, " Way too few sensors in Sensor System Manager! ");

	sensorPool = (SensorSystemPtr*)missionHeap->Malloc(MAX_SENSORS * sizeof(SensorSystemPtr));
	gosASSERT(sensorPool!=NULL);

	for (long i = 0; i < MAX_SENSORS; i++)
		sensorPool[i] = new SensorSystem;

	//-----------------------------------------------------
	// This assumes we have at least 2 sensors in the pool
	// when initializing the pool...
	sensorPool[0]->id = 0;
	sensorPool[0]->prev = NULL;
	sensorPool[0]->next = sensorPool[1];

	for (i = 1; i < (MAX_SENSORS - 1); i++) {
		sensorPool[i]->id = i;
		sensorPool[i]->prev = sensorPool[i - 1];
		sensorPool[i]->next = sensorPool[i + 1];
	}

	sensorPool[MAX_SENSORS - 1]->id = MAX_SENSORS - 1;
	sensorPool[MAX_SENSORS - 1]->prev = sensorPool[MAX_SENSORS - 2];
	sensorPool[MAX_SENSORS - 1]->next = NULL;

	//------------------------------
	// All start on the free list...
	freeList = sensorPool[0];
	freeSensors = MAX_SENSORS;

	for (i = 0; i < MAX_TEAMS; i++)
		teamSensors[i] = NULL;

	Assert (!debug || (Team::numTeams > 0), 0, " SensorSystemManager.init: 0 teams ");

	for (i = 0; i < Team::numTeams; i++) {
		teamSensors[i] = new TeamSensorSystem;
		teamSensors[i]->teamId = i;
	}

	SensorSystem::numSensors = 0;

	return(NO_ERR);
}

long SensorSystemManager::checkIntegrity (void)
{
	//See if every pointer in sensorPool is still OK.
	// SOMETHING is trashing memory here.  Damned if I know what.
	long result = 0;

	for (long i=0;i<MAX_SENSORS;i++)
	{
		SensorSystem *sensor = getSensor(i);
		if (sensor->owner)
			result = 1;
		else
			result = 0;
	}

	return result;
}

//---------------------------------------------------------------------------

SensorSystemPtr SensorSystemManager::newSensor (void) {

	if (!freeSensors)
		Fatal(0, " No More Free Sensors ");

	freeSensors--;

	//---------------------------------------
	// Grab the first free sensor in line...
	SensorSystemPtr sensor = freeList;

	//------------------------------------------
	// Cut the new sensor from the free list...
	freeList = freeList->next;
	if (freeList)
		freeList->prev = NULL;

	//----------------------------------------------------
	// New system has no next. Already has no previous...
	sensor->next = NULL;

	return(sensor);
}

//---------------------------------------------------------------------------

void SensorSystemManager::freeSensor (SensorSystemPtr sensor) {

	freeSensors++;

	sensor->prev = NULL;
	sensor->next = freeList;
	freeList->prev = sensor;
	freeList = sensor;
}

//---------------------------------------------------------------------------

void SensorSystemManager::destroy (void) {

	if (sensorPool)
	{
		for (long i = 0; i < MAX_SENSORS; i++) 
		{
			delete sensorPool[i];
			sensorPool[i] = NULL;
		}
		
		missionHeap->Free(sensorPool);
		sensorPool = NULL;
	}

	freeSensors = 0;
	freeList = NULL;

	for (long i = 0; i < Team::numTeams; i++) 
	{
		delete teamSensors[i];
		teamSensors[i] = NULL;
	}

	if (SensorSystem::sortList)
	{
		delete SensorSystem::sortList;
		SensorSystem::sortList = NULL;
	}
}

//---------------------------------------------------------------------------

void SensorSystemManager::addTeamSensor (long teamId, SensorSystemPtr sensor) 
{
	if ((teamId > -1) && (teamId < Team::numTeams))
	{
		teamSensors[teamId]->addSensor(sensor);
		sensor->setMaster(teamSensors[teamId]);
	}
}

//---------------------------------------------------------------------------

void SensorSystemManager::removeTeamSensor (long teamId, SensorSystemPtr sensor) {

	if (teamId == -1)
		return;
	teamSensors[teamId]->removeSensor(sensor);
	sensor->setMaster(NULL);
}

//---------------------------------------------------------------------------

void SensorSystemManager::update (void) 
{
	updateEcmEffects();
	updateSensors();
//	updateTeamContactLists();
}

//---------------------------------------------------------------------------

void SensorSystemManager::updateEcmEffects (void) {

	for (long i = 0; i < Team::numTeams; i++)
		teamSensors[i]->updateEcmEffects();
}

//---------------------------------------------------------------------------

void SensorSystemManager::updateSensors (void) 
{
	/*
	for (long i = 0; i < Team::numTeams; i++)
		teamSensors[i]->update();
	*/
	
	//Let's try one team per frame update!!
	if (Team::teams[teamToUpdate] == Team::home)
		enemyInLOS = false;		//Resets every scan for HOME team!!!!
		
 	teamSensors[teamToUpdate]->update();
	teamToUpdate++;
	if (teamToUpdate == Team::numTeams)
		teamToUpdate = 0;
}

//---------------------------------------------------------------------------
#if 0
void SensorSystemManager::updateTeamContactLists (void) {

	for (long i = 0; i < Team::numTeams; i++) {
		teamSensors[i]->updateContactList();
	}
}
#endif
//---------------------------------------------------------------------------

void SensorSystemManager::addEcm (GameObjectPtr owner, float range) {

	if (numEcms == MAX_ECMS)
		Fatal(0, " SensorSystemManager.addEcm: too many ecms ");

	ecms[numEcms].owner = owner;
	ecms[numEcms].range = range;
	numEcms++;
}

//---------------------------------------------------------------------------

float SensorSystemManager::getEcmEffect (GameObjectPtr victim) {

	/*		ECM stealths the mech carrying it.  Period.
	for (long i = 0; i < numEcms; i++)
		if (!victim->isFriendly(ecms[i].owner))
			if (ecms[i].owner->distanceFrom(victim->getPosition()) <= ecms[i].range)
				return(0.5);
	*/
	return(1.0);
}

//***************************************************************************




