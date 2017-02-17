//******************************************************************************************
//
//	Radio.cpp -- File contains the Radio class functions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//------------------------------------------------------------------------------------------
// Include Files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef RADIO_H
#include"radio.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef MOVER_H
#include"mover.h"
#endif

//------------------------------------------------------------------------------------------
// Macro Definitions
extern bool useSound;

PacketFilePtr 		Radio::noiseFile = NULL;
RadioPtr	  		Radio::radioList[MAX_RADIOS];	//Warriors no longer delete their radios.
bool				Radio::radioListGo = false;
bool				Radio::messageInfoLoaded = false;
long				Radio::currentRadio = 0;
UserHeapPtr			Radio::radioHeap = NULL;
PacketFilePtr		Radio::messagesFile[MAX_RADIOS];

RadioMessageInfo	messageInfo[RADIO_MESSAGE_COUNT];

//------------------------------------------------------------------------------------------
// Class Radio
//------------------------------------------------------------------------------------------
void *Radio::operator new (size_t mySize)
{
	if (!radioListGo)
	{
		radioListGo = true;
		for (long i=0;i<MAX_RADIOS;i++)
		{
			radioList[i] = NULL;
			messagesFile[i] = NULL;
		}
		
		currentRadio = 0;
		
		radioHeap = new UserHeap;
		radioHeap->init((4096 * 51)-1,"Radios");
		radioHeap->setMallocFatals(false);
	}

	void *result = radioHeap->Malloc(mySize);
	return result;
}
 
//-------------------------------------------------------------------------------
void Radio::operator delete (void *us)
{
	radioHeap->Free(us);
}

//-------------------------------------------------------------------------------
long Radio::init (char *fileName, unsigned long heapSize, char *movie)
{
	FullPathFileName pilotAudioPath;
	pilotAudioPath.init(CDsoundPath,fileName,".pak");

	FullPathFileName noisePath;
	noisePath.init(CDsoundPath,"noise",".pak");

	//--------------------------------------
	// Startup the packet file.
	radioID = currentRadio;
	messagesFile[radioID] = new PacketFile;
	gosASSERT(messagesFile[radioID] != NULL);
		
	long result = messagesFile[radioID]->open(pilotAudioPath);
	gosASSERT(result == NO_ERR);
	
	if (!noiseFile)
	{
		//--------------------------------------
		// Startup the Noise packet file.
		noiseFile = new PacketFile;
		gosASSERT(noiseFile != NULL);
			
		result = noiseFile->open(noisePath);
		gosASSERT(result == NO_ERR);
	}
		
	// load message parameters
	if (!messageInfoLoaded)
	{
		if (loadMessageInfo() == NO_ERR)
			messageInfoLoaded = TRUE;
		else
			Fatal(0, "Unable to load message info.");
	}
	
	radioList[currentRadio] = this;
	currentRadio++;
	
	return(NO_ERR);
}
		
#define NO_PLAY			-1
//------------------------------------------------------------------------------------------
long Radio::playMessage (RadioMessageType msgType)
{
long i, roll, callsign, fragmentNum, dropOut = 0;

	if (!useSound)
		return(NO_PLAY);
	
	if (!enabled)
		return(NO_PLAY);

	if (!owner)
		return(NO_PLAY);

	if (!soundSystem->checkMessage(owner, messageInfo[msgType].priority, msgType))
		return(NO_PLAY);

	if ((msgType == RADIO_AMMO_OUT) && ammoOutPlayed)
		return(NO_PLAY);
	
	if (messageInfo[msgType].styleCount > 1)
	{
		roll = RandomNumber(100);	// choose which style of message to play
		for (i=0; i<messageInfo[msgType].styleCount; i++)
		{
			dropOut += messageInfo[msgType].styleChance[i];
			if (roll < dropOut)
				break;
		}
		
		if (i != 0 && i == messageInfo[msgType].styleCount)
			return NO_PLAY;
			
		if (messageInfo[msgType].messageMapping + i == owner->getLastMessage())
			i++;
			
		if (i >= messageInfo[msgType].styleCount)
			i = 0;
	}
	else
		i = 0;

	RadioData *msgData = (RadioData *)radioHeap->Malloc(sizeof(RadioData));
	if (!msgData)
	{
		return(NO_PLAY);
	}
	
	memset(msgData,0,sizeof(RadioData));	
			
	msgData->noiseId = SHORT_STATIC;
	msgData->msgType = msgType;
	msgData->msgId = messageInfo[msgType].messageMapping + i;
	msgData->movieCode = messageInfo[msgType].movieCode;
	msgData->msgHeap = radioHeap;
	msgData->turnQueued = turn;
			
	msgData->priority = messageInfo[msgType].priority;
	msgData->pilot = owner;
	msgData->expirationDate = scenarioTime + messageInfo[msgType].shelfLife;
	
	//-----------------------------------------------------------------------
	// Load the pieces need for playback.
	callsign = 0;
	fragmentNum = 0;
	if (messageInfo[msgType].pilotIdentifiesSelf)
	{
		if (messageInfo[RADIO_CALLSIGN].styleCount > 1)
		{
			roll = RandomNumber(100);	// choose which style of message to play
			for (i=0; i<messageInfo[RADIO_CALLSIGN].styleCount; i++)
			{
				dropOut += messageInfo[RADIO_CALLSIGN].styleChance[i];
				if (roll < dropOut)
					break;
			}
			
			if (i != 0 && i == messageInfo[RADIO_CALLSIGN].styleCount)
				callsign = 0;
				
			if (i >= messageInfo[RADIO_CALLSIGN].styleCount)
				callsign = 0;
		}
		else
			callsign = 0;

		callsign += messageInfo[RADIO_CALLSIGN].messageMapping;
	}
	
	if (callsign)
	{
		if (messagesFile[radioID]->seekPacket(callsign) == NO_ERR)
		{

			unsigned long messageSize = messagesFile[radioID]->getPacketSize();
			msgData->data[fragmentNum] = (MemoryPtr)radioHeap->Malloc(messageSize);
			if (!msgData->data[fragmentNum]) 
			{
				radioHeap->Free(msgData);
				return(NO_PLAY);
			}
			messagesFile[radioID]->readPacket(callsign,msgData->data[fragmentNum]);
			msgData->dataSize[fragmentNum] = messageSize;
			fragmentNum++;
		}
	}

	if (messagesFile[radioID]->seekPacket(msgData->msgId) == NO_ERR)
	{
		unsigned long messageSize = messagesFile[radioID]->getPacketSize();
		msgData->data[fragmentNum] = (MemoryPtr)radioHeap->Malloc(messageSize);
		if (!msgData->data[fragmentNum]) 
		{
			while (fragmentNum >= 0)
			{
				radioHeap->Free(msgData->data[fragmentNum]);
				fragmentNum--;
			}
			radioHeap->Free(msgData);
			return(NO_PLAY);
		}
		
        // sebi: added failure handling
        if(messagesFile[radioID]->readPacket(msgData->msgId,msgData->data[fragmentNum])) {
    		msgData->dataSize[fragmentNum] = messageSize;
        } else {
            radioHeap->Free(msgData->data[fragmentNum]);
            msgData->data[fragmentNum] = NULL;
        }
        
		
		if (noiseFile->seekPacket(msgData->noiseId) == NO_ERR)
		{
			unsigned long messageSize = noiseFile->getPacketSize();
			msgData->noise[0] = (MemoryPtr)radioHeap->Malloc(messageSize);
			if (!msgData->noise[0])
			{
				radioHeap->Free(msgData);
				if (fragmentNum > 0)
					radioHeap->Free(msgData->data[0]);

				return(NO_PLAY);
			}
		
            // sebi added failure handling
			if(noiseFile->readPacket(msgData->noiseId,msgData->noise[0])) {
				msgData->noiseSize[fragmentNum] = messageSize;
			} else {
				radioHeap->Free(msgData->noise[0]);
				msgData->noise[0] = (MemoryPtr)-1;
			}
		}
	}
	
	//------------------------------------------------------
	// Big ol bug here.  We weren't checking to see if the
	// queue was full.  If it was, memory would leak from
	// the smacker window.  It wouldn't leak from the RadioHeap
	// because we clear the radio heap every mission!!
	if (soundSystem->queueRadioMessage(msgData) != NO_ERR)
	{
		if (msgData)
		{
			for (long j=0;j<MAX_FRAGMENTS;j++)
			{
				radioHeap->Free(msgData->data[j]);
				msgData->data[j] = NULL;
				if(msgData->noise[j] != (MemoryPtr)-1) {
					radioHeap->Free(msgData->noise[j]);
				}
				msgData->noise[j] = NULL;
			}
			
			radioHeap->Free(msgData);
			msgData = NULL;
		}
		return (NO_PLAY);
	}
	
	if (msgType == RADIO_AMMO_OUT)
		ammoOutPlayed = true;

	return(msgData->msgId);
}

//------------------------------------------------------------------------------------------
long Radio::loadMessageInfo(void)
{
FullPathFileName	messageInfoPath;
FilePtr				messageInfoFile;
long				result;
char				dataLine[512];
char*				field;

	messageInfoPath.init(soundPath,"radio",".csv");
	messageInfoFile = new File;
	if (!messageInfoFile)
		return -1; //quasit

	result = messageInfoFile->open(messageInfoPath);
	if (result != NO_ERR)
	{
		delete messageInfoFile;
		return result;
	}

	messageInfoFile->readLine((MemoryPtr)dataLine, 511);	// skip title line

	for (long i=0; i<RADIO_MESSAGE_COUNT; i++)
	{
		result = messageInfoFile->readLine((MemoryPtr)dataLine, 511);
		if (!result)
			Fatal(0, "Bad Message Info File");

		field = strtok(dataLine, ",");	// get past command name
		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].priority = atoi(field);
		else
			messageInfo[i].priority = 4;

		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].shelfLife = atoi(field);
		else
			messageInfo[i].shelfLife = 0;

		field = strtok(NULL, ",");
		if (field && *field != 'x')
			messageInfo[i].movieCode = *field;
		else
			messageInfo[i].movieCode = '\0';

		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].styleCount = atoi(field);
		else
			messageInfo[i].styleCount = 1;

		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].styleChance[0] = atoi(field);
		else
			messageInfo[i].styleChance[0] = 0;
		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].styleChance[1] = atoi(field);
		else
			messageInfo[i].styleChance[1] = 0;
		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].styleChance[2] = atoi(field);
		else
			messageInfo[i].styleChance[2] = 0;

		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].pilotIdentifiesSelf = (*field == 'y');
		else
			messageInfo[i].pilotIdentifiesSelf = FALSE;

		field = strtok(NULL, ",");
		if (field)
			messageInfo[i].messageMapping = atoi(field);
		else
			messageInfo[i].messageMapping = 0;
		
		field = strtok(NULL, ",");
		if (field)
		{
			field = strtok(NULL, ",");
			if (field)
			{
				messageInfo[i].kludgeStyle = (*field == 'x');
			}
		}
		if (!field)
			messageInfo[i].kludgeStyle = FALSE;
	}
	messageInfoFile->close();
	delete messageInfoFile;
	return NO_ERR;
}
