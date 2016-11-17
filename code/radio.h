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
/*
struct RadioData
{
	uint32_t        msgId;
	uint32_t        msgType;
	uint32_t        noiseId;
	int32_t         numFragments;
	//MemoryPtr		data[MAX_FRAGMENTS];
	DWORD           data_Legacy32BitPtr[MAX_FRAGMENTS];
	int32_t         dataSize[MAX_FRAGMENTS];
	//MemoryPtr		noise[MAX_FRAGMENTS];
	MemoryPtr		noise_Legacy32BitPtr[MAX_FRAGMENTS];
	int32_t         noiseSize[MAX_FRAGMENTS];
	//UserHeapPtr		msgHeap;
	DWORD           msgHeap_Legacy32BitPtr;
	uint32_t        turnQueued;
	byte			priority;
	byte			movieCode;
	float			expirationDate;
	//MechWarriorPtr	pilot;
	DWORD           pilot_Legacy32BitPtr;
};

struct RadioDataRuntime {
	MemoryPtr		data[MAX_FRAGMENTS];
	MemoryPtr		noise[MAX_FRAGMENTS];
	UserHeapPtr		msgHeap;
	MechWarriorPtr	pilot;
};
*/

struct RadioData
{
	uint32_t        msgId;
	uint32_t        msgType;
	uint32_t        noiseId;
	int32_t         numFragments;
	MemoryPtr		data[MAX_FRAGMENTS];
	int32_t         dataSize[MAX_FRAGMENTS];
	MemoryPtr		noise[MAX_FRAGMENTS];
	int32_t         noiseSize[MAX_FRAGMENTS];
	UserHeapPtr		msgHeap;
	uint32_t        turnQueued;
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
