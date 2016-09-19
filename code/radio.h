//******************************************************************************************
//
//	Radio.h -- File contains the Radio Definition
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef RADIO_H
#define RADIO_H

//------------------------------------------------------------------------------------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DRADIO_H
#include"dradio.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

#ifndef DGAMEOBJ_H
#include"dgameobj.h"
#endif

//------------------------------------------------------------------------------------------
#define MAX_FRAGMENTS	16
#define MAX_RADIOS		256

struct RadioData
{
	unsigned long	msgId;
	unsigned long	msgType;
	unsigned long	noiseId;
	long			numFragments;
	MemoryPtr		data[MAX_FRAGMENTS];
	long			dataSize[MAX_FRAGMENTS];
	MemoryPtr		noise[MAX_FRAGMENTS];
	long			noiseSize[MAX_FRAGMENTS];
	UserHeapPtr		msgHeap;
	unsigned long	turnQueued;
	byte			priority;
	byte			movieCode;
	float			expirationDate;
	MechWarriorPtr	pilot;
};

struct RadioMessageInfo
{
	byte			priority;
	float			shelfLife;
	char			movieCode;
	byte			styleCount;
	byte			styleChance[3];
	DWORD			messageMapping;
	bool			pilotIdentifiesSelf;
	bool			kludgeStyle;
};

//------------------------------------------------------------------------------------------
class Radio
{
	//Data Members
	//-------------
	protected:
	
		MechWarriorPtr	owner;
		bool			enabled;
		bool			ammoOutPlayed;
		long			radioID;

	public:		
		static PacketFilePtr 	noiseFile;
		static RadioPtr			radioList[MAX_RADIOS];		//Warriors no longer delete their radios.  We do when the
															//SoundSystem Shutdown occurs.

		static PacketFilePtr	messagesFile[MAX_RADIOS];	//Do these when the shutdown occurs too to avoid leaks
														
		static bool				messageInfoLoaded;
		static bool				radioListGo;
		static long				currentRadio;
		static UserHeapPtr		radioHeap;				//Only one Heap Per Game!!!
	
	//Member Functions
	//-----------------
	public:
	
		void * operator new (size_t mySize);
		void operator delete (void * us);

		void init (void)
		{
			enabled = TRUE;
			ammoOutPlayed = false;
			radioID = -1;
		}

		void destroy (void)
		{
		}

		Radio (void)
		{
			init();
		}
	
		~Radio (void)
		{
			destroy();
		}
	
		long init (char *fileName, unsigned long heapSize, char *movie);

		void setOwner (MechWarriorPtr _owner)
		{
			owner = _owner;
		}
				
		long playMessage (RadioMessageType msgId);

		void cancelMessage (RadioMessageType msgId);
		
		void turnOn (void) {
			enabled = TRUE;
		}

		void turnOff (void) {
			enabled = FALSE;
		}
		
		long loadMessageInfo(void);

		void resetAmmoMessage (void)
		{
			ammoOutPlayed = false;
		}
};

//------------------------------------------------------------------------------------------
#endif