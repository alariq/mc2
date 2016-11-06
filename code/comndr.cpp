//******************************************************************************************
//
//	comndr.cpp - This file contains the Commander Class header
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef GROUP_H
#include"group.h"
#endif

#ifndef COMNDR_H
#include"comndr.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

#ifndef MULTPLYR_H
#include"multplyr.h"
#endif

#ifndef UNITDESG_H
#include"unitdesg.h"
#endif

#include"team.h"

#include<stdio.h>

//---------------------------------------------------------------------------

long				Commander::numCommanders = 0;
CommanderPtr		Commander::commanders[MAX_COMMANDERS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
CommanderPtr		Commander::home = NULL;

//***************************************************************************
// COMMANDER class
//***************************************************************************

void* Commander::operator new (size_t ourSize) {

	void* result = systemHeap->Malloc(ourSize);
	return(result);
}

//---------------------------------------------------------------------------

void Commander::operator delete (void* us) {

	systemHeap->Free(us);
}	

//---------------------------------------------------------------------------

void Commander::init (void) {

	id = -1;
	team = NULL;
	for (long i = 0; i < MAX_MOVERGROUPS; i++) {
		groups[i] = new MoverGroup;
		groups[i]->setId(i);
	}

	numCommanders++;
}

//---------------------------------------------------------------------------

void Commander::destroy (void) {

	for (long i = 0; i < MAX_MOVERGROUPS; i++) {
		delete groups[i];
		groups[i] = NULL;
	}

	numCommanders--;
	Assert(numCommanders >= 0, numCommanders, " Commander.destroy: numCommanders is neg ");
}

//---------------------------------------------------------------------------

long Commander::setGroup (long id, long numMates, MoverPtr* moverList, long point) {

	// First, does the unit currently have objects set up there already?
	// If so, we need to determine
	Assert((id >= 0) && (id < MAX_MOVERGROUPS), 0, " Commander::bad id in setGroup ");

	MoverGroupPtr curGroup = groups[id];

	Assert(curGroup != NULL, 0, " Commander::setGroup has null group ");

	//-----------------------------------------------
	// The current group in this slot must disband...
	curGroup->disband();

	//-----------------------------------------------
	// Now, slip the new group into this unit slot...
	for (long i = 0; i < numMates; i++) {
		MoverPtr mover = moverList[i];

		//-----------------------------------------
		// Pull the mover from its current group...
		MoverGroupPtr oldGroup = mover->getGroup();
		if (oldGroup)
			oldGroup->remove(mover);

		//----------------------------
		// Add 'em to the new group...
		curGroup->add(mover);
	}

	//-----------------------------------
	// Set the point for the new group...
	//
	// The MoverList passed in is INVALID passed
	// the numMates count.  If you select a point
	// which is dead, it invariably points to a 
	// MUCH higher location in the moverList then is valid.
	// We now guard against this...
	if ((point < numMates) && (point >= 0))
		curGroup->setPoint(moverList[point]);

	return(NO_ERR);
}

//---------------------------------------------------------------------------

void Commander::setLocalMoverId (long localMoverId) 
{
	if (getTeam())
	{
		for (long j = 0; j < getTeam()->getRosterSize(); j++)
			getTeam()->getMover(j)->setLocalMoverId(localMoverId);
	}
}

//---------------------------------------------------------------------------

void Commander::eject (void) {

	for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
		MoverPtr mover = ObjectManager->getMover(i);
		if (mover && (mover->getCommanderId() == id)) {
			if (mover->getObjectClass() == BATTLEMECH) {
				if (mover->getPilot())
					mover->getPilot()->orderEject(false, true, ORDER_ORIGIN_COMMANDER);
				}
			else {
				WeaponShotInfo shot;
				shot.init(0, -3, 254.0, 0, 0);
				mover->handleWeaponHit(&shot, (MPlayer != NULL));
			}
		}
	}
}

//---------------------------------------------------------------------------

void Commander::addToGUI (bool visible) {

	//----------------------------------
	// The GUI only supports 4 groups...
	for (long i = 0; i < 4; i++)
		groups[i]->addToGUI(visible);
}

//---------------------------------------------------------------------------
long Commander::Save (PacketFilePtr file, long packetNum)
{
	StaticCommanderData staticData;
	staticData.numCommanders = numCommanders;
	staticData.homeCommanderId = home->getId();

	file->writePacket(packetNum,(MemoryPtr)&staticData,sizeof(StaticCommanderData),STORAGE_TYPE_RAW);
	packetNum++;

	for (long i=0;i<numCommanders;i++)
	{
		CommanderData data;
		data.id = commanders[i]->getId();

		//Allied commanders have NO team!!??!
		// Just save the home team with them then.
		if (commanders[i]->getTeam())
			data.teamId = commanders[i]->getTeam()->getId();
		else
			data.teamId = Team::home->getId();

		for (long j=0;j<MAX_MOVERGROUPS;j++)
			commanders[i]->groups[j]->copyTo(data.groups[j]);

		file->writePacket(packetNum,(MemoryPtr)&data,sizeof(CommanderData),STORAGE_TYPE_RAW);
		packetNum++;
	}

	return packetNum;
}

//---------------------------------------------------------------------------
long Commander::Load (PacketFilePtr file, long packetNum)
{
	StaticCommanderData staticData;
	file->readPacket(packetNum,(MemoryPtr)&staticData);
	packetNum++;

	numCommanders = staticData.numCommanders;

	for (long i=0;i<numCommanders;i++)
	{
		CommanderData data;
		file->readPacket(packetNum,(MemoryPtr)&data);
		packetNum++;

		commanders[i] = new Commander;

		//Same drill as all the others.  HACK!
		// We increment numCommanders in the INIT.
		// Decrement here to maintain sanity.
		numCommanders--;

		commanders[i]->setId(data.id);
		for (long k=0;k<MAX_TEAMS;k++)
		{
			if (Team::teams[k] && (Team::teams[k]->getId() == data.teamId))
				commanders[i]->team = Team::teams[k];
		}

		for (long j=0;j<MAX_MOVERGROUPS;j++)
		{
//			commanders[i]->groups[j] = new MoverGroup;
			commanders[i]->groups[j]->init(data.groups[j]);
		}

		if (staticData.homeCommanderId == commanders[i]->getId())
			home = commanders[i];
	}

	return packetNum;
}

//***************************************************************************

