//---------------------------------------------------------------------------
// GameSound.cpp - This file is the sound system code for the GAME
//
// MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files
#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef WARRIOR_H
#include"warrior.h"
#endif

#ifndef VIEWER
#ifndef CONTROLGUI_H
#include"controlgui.h"
#endif 
#endif

//---------------------------------------------------------------------------
GameSoundSystem *soundSystem = NULL;

extern bool wave_ParseWaveMemory(MemoryPtr lpChunkOfMemory, MC2_WAVEFORMATEX** lplpWaveHeader, MemoryPtr* lplpWaveSamples,DWORD *lpcbWaveSize);

bool friendlyDestroyed = false;
bool enemyDestroyed = false;
extern bool GeneralAlarm;

#define ALARM_TIME		5.0f

//---------------------------------------------------------------------------
void GameSoundSystem::purgeSoundSystem (void)
{
	wholeMsgDone = true;
	currentMessage = NULL;
	messagesInQueue = 0;
	currentFragment = 0;
	playingNoise = false;
	radioHandle = NULL;
	//------------------------------------------------------------
	// dump the Radio Message Queue.
	messagesInQueue = 0;
	wholeMsgDone = true;
	for (long i=0;i<MAX_QUEUED_MESSAGES;i++)
		queue[i] = NULL;

	generalAlarmTimer = 0.0f;
			
	//---------------------------------------------------------
	// Toss everybody's radio.  This MUST happen AFTER
	// all of the currently queued radio messages are dumped.
	// Radios all come from the Radio Heap now.  This dumps 'em.
	for (long i=0;i<MAX_RADIOS;i++)
	{
		Radio::radioList[i] = NULL;
		delete Radio::messagesFile[i];
		Radio::messagesFile[i] = NULL; 
	}

	delete Radio::radioHeap;
	Radio::radioHeap = NULL;
		
	delete Radio::noiseFile;
	Radio::noiseFile = NULL;

	Radio::currentRadio = 0;
	Radio::radioListGo = FALSE;
	Radio::messageInfoLoaded = FALSE;

	isRaining = oldRaining = false;

	largestSensorContact = -1;
	//-----------------------------------------------
	// Check all samples to see if one should end.
	for (long i=0;i<MAX_DIGITAL_SAMPLES;i++)
	{
		if (i != BETTY_CHANNEL)
		{
			gosAudio_SetChannelPlayMode(i, gosAudio_Stop);
			fadeDown[i] = FALSE;
			channelSampleId[i] = -1;
			channelInUse[i] = FALSE;
		}
	}
}	

//---------------------------------------------------------------------------
void GameSoundSystem::removeQueuedMessage (long msgNumber)
{
long i;

	if (msgNumber < 0 || msgNumber >= MAX_QUEUED_MESSAGES)
		return;

	if (queue[msgNumber])
	{
		if (queue[msgNumber]->pilot)
			queue[msgNumber]->pilot->clearMessagePlaying();

		UserHeapPtr msgHeap = queue[msgNumber]->msgHeap;
		for (i=0;i<MAX_FRAGMENTS;i++)
		{
			msgHeap->Free(queue[msgNumber]->data[i]);
			queue[msgNumber]->data[i] = NULL;

            // sebi: TEMP AND UGLY! have to make it because noise sample fails to load and then game logic goes belly up
            // and somehow just plays 2 times main radio message
			if(queue[msgNumber]->noise[i]!=(MemoryPtr)-1)
				msgHeap->Free(queue[msgNumber]->noise[i]);
			queue[msgNumber]->noise[i] = NULL;
		}
		
		msgHeap->Free(queue[msgNumber]);
		if (messagesInQueue > 0)
			messagesInQueue--;

#ifdef _DEBUG
		for (long test=0;test<(long)messagesInQueue;test++)
		{
			if (queue[test])
				continue;
			else
				Fatal(-1," Bad Message in Queue -- RmoveQMsg");
		}
#endif

		for (i=msgNumber;i<MAX_QUEUED_MESSAGES-1;i++)
			queue[i] = queue[i+1];
		queue[MAX_QUEUED_MESSAGES-1] = NULL;

#ifdef _DEBUG
		for (long test=0;test<(long)messagesInQueue;test++)
		{
			if (queue[test])
				continue;
			else
				Fatal(-1," Bad Message in Queue -- RmoveQMsg");
		}
#endif

	}
}

//---------------------------------------------------------------------------
bool GameSoundSystem::checkMessage (MechWarriorPtr pilot, byte priority, unsigned long messageType)
{
	for (long i=0;i<MAX_QUEUED_MESSAGES;i++)
	{
		if (queue[i])
			if ((queue[i]->pilot == pilot && priority > queue[i]->priority) ||			// I'm already saying something more important, or
					(queue[i]->priority > 1 && queue[i]->msgType == messageType))		// someone else is saying this
				return FALSE;
	}
	return TRUE;
}

//---------------------------------------------------------------------------
void GameSoundSystem::moveFromQueueToPlaying(void)
{
	removeCurrentMessage();

	while (queue[0] && 
			queue[0]->pilot && 
			!(queue[0]->pilot->active()) &&
			(queue[0]->msgType != RADIO_DEATH) &&
			(queue[0]->msgType != RADIO_EJECTING))
	{
		removeQueuedMessage(0);
	}

	currentMessage = queue[0];

	for (long i=0;i<MAX_QUEUED_MESSAGES-1;i++)
		queue[i] = queue[i+1];
		
	queue[MAX_QUEUED_MESSAGES-1] = NULL;
	if (messagesInQueue > 0)
		messagesInQueue--;
}

//---------------------------------------------------------------------------
void GameSoundSystem::removeCurrentMessage(void)
{
	if (currentMessage)
	{
		if (currentMessage->pilot)
			currentMessage->pilot->clearMessagePlaying();

		UserHeapPtr msgHeap = currentMessage->msgHeap;
		for (long j=0;j<MAX_FRAGMENTS;j++)
		{
			msgHeap->Free(currentMessage->data[j]);
			currentMessage->data[j] = NULL;
            // sebi: TEMP AND UGLY! have to make it because noise sample fails to load and then game logic goes belly up
            // and somehow just plays 2 times main radio message
			if(currentMessage->noise[j]!=(MemoryPtr)-1)
				msgHeap->Free(currentMessage->noise[j]);
			currentMessage->noise[j] = NULL;
		}
		
		msgHeap->Free(currentMessage);
		currentMessage = NULL;
	}

	gosAudio_SetChannelPlayMode(RADIO_CHANNEL, gosAudio_Stop);
	wholeMsgDone = TRUE;
}

//---------------------------------------------------------------------------
long GameSoundSystem::queueRadioMessage (RadioData *msgData)
{
long i;

	//-------------------------------------------------
	// First, search the Queue and see if this message
	// was already sent this turn.
	if (msgData->msgId >= MSG_TOTAL_MSGS)	// is this a real message? (why are we asking this???)
	{
		for (i=MAX_QUEUED_MESSAGES-1;i>=0;i--)
		{
			if (queue[i])
			{
				if ((msgData->turnQueued == queue[i]->turnQueued) &&
					(msgData->msgId == queue[i]->msgId))
				{
					removeQueuedMessage(i);
				}
			}
		}
	}

	if (msgData->priority == 0)	// top cancels playing message! top also means pilot's gone, so
	{							// remove other messages from that pilot.
		removeCurrentMessage();
		for (i=MAX_QUEUED_MESSAGES-1;i>=0;i--)
		{
			if (queue[i] && queue[i]->pilot == msgData->pilot)
			{
				removeQueuedMessage(i);
			}
		}
	}

	for (i=0; i<MAX_QUEUED_MESSAGES;i++)
	{
		if (!queue[i])
			break;

		if (queue[i]->priority > msgData->priority)	// if this messages priority higher (a lower number: 1 is top priority)
		{											// push things down to make room.
			for (long j=MAX_QUEUED_MESSAGES-1;j>i;j--)
			{
				queue[j] = queue[j-1];
			}
			break;
		}
	}
	
	if (i == MAX_QUEUED_MESSAGES)
		return(-1);

	if (queue[MAX_QUEUED_MESSAGES-1])
		removeQueuedMessage(MAX_QUEUED_MESSAGES-1);

	queue[i] = msgData;

#ifdef _DEBUG
		for (long test=0;test<(long)messagesInQueue;test++)
		{
			if (queue[test])
				continue;
			else
				Fatal(-1," Bad Message in Queue -- RmoveQMsg");
		}
#endif
	messagesInQueue++;
#ifdef _DEBUG
		for (long test=0;test<(long)messagesInQueue;test++)
		{
			if (queue[test])
				continue;
			else
				Fatal(-1," Bad Message in Queue -- RmoveQMsg");
		}
#endif

	return NO_ERR;
}

//---------------------------------------------------------------------------
void GameSoundSystem::update (void)
{
	//---------------------------------------------
	// Dynamic Music Code goes here!!!
	// New System(tm)
	//
	
	SoundSystem::update();

	if (GeneralAlarm && (generalAlarmTimer < ALARM_TIME))
	{
		playDigitalSample(BUZZER1);
		generalAlarmTimer += frameLength;
	}

	if (useSound && currentMessage && (gosAudio_GetChannelPlayMode(RADIO_CHANNEL) != gosAudio_PlayOnce))
	{
		if (radioHandle)
		{
			gosAudio_DestroyResource(&radioHandle);		//Toss the current radio data which just completed!
			radioHandle = NULL;
		}

		if (!wholeMsgDone)
		{
			//---------------------------------------------------------------
			// We are finished with a fragment, move on.
			if (playingNoise)
			{
				playingNoise = FALSE;
					
				if (currentMessage->data[currentFragment])
				{
					//--------------------------------------------------------------------
					// Hand GOS sound the data it needs to create the resource Handle
					gosAudio_Format soundFormat;
					soundFormat.wFormatTag = 1;				//PCM
		
					MC2_WAVEFORMATEX *waveFormat = NULL;
					MemoryPtr dataOffset = NULL;
					DWORD length = 0;
					DWORD bitsPerSec = 0;
					wave_ParseWaveMemory(currentMessage->data[currentFragment],&waveFormat,&dataOffset,&length);
					
					bitsPerSec = waveFormat->nBlockAlign / waveFormat->nChannels * 8;
			
					soundFormat.nChannels = waveFormat->nChannels;
					soundFormat.nSamplesPerSec = waveFormat->nSamplesPerSec;
					soundFormat.nAvgBytesPerSec = waveFormat->nAvgBytesPerSec;
					soundFormat.nBlockAlign = waveFormat->nBlockAlign;
					soundFormat.wBitsPerSample = bitsPerSec;
					soundFormat.cbSize = 0;

					gosAudio_CreateResource(&radioHandle,gosAudio_UserMemory, NULL, &soundFormat,dataOffset,length);
					
					if (isChannelPlaying(BETTY_CHANNEL))
						gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume * 0.5f);
					else
						gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume);

					gosAudio_AssignResourceToChannel(RADIO_CHANNEL, radioHandle);
					gosAudio_SetChannelPlayMode(RADIO_CHANNEL,gosAudio_PlayOnce);
					
					if (currentMessage->pilot)
						currentMessage->pilot->setMessagePlaying();
				}
				else
					wholeMsgDone = TRUE;
					
				currentFragment++;
			}
			else
			{
				if (currentMessage->noise[currentFragment])
				{
                    // sebi: TEMP AND UGLY! have to make it because noise sample fails to load and then game logic goes belly up
                    // and somehow just plays 2 times main radio message
					if(currentMessage->noise[currentFragment] != (MemoryPtr)-1) {

					//--------------------------------------------------------------------
					// Hand GOS sound the data it needs to create the resource Handle
					gosAudio_Format soundFormat;
					soundFormat.wFormatTag = 1;				//PCM
		
					MC2_WAVEFORMATEX *waveFormat = NULL;
					MemoryPtr dataOffset = NULL;
					DWORD length = 0;
					DWORD bitsPerSec = 0;
					wave_ParseWaveMemory(currentMessage->noise[currentFragment],&waveFormat,&dataOffset,&length);
					
					bitsPerSec = waveFormat->nBlockAlign / waveFormat->nChannels * 8;
			
					soundFormat.nChannels = waveFormat->nChannels;
					soundFormat.nSamplesPerSec = waveFormat->nSamplesPerSec;
					soundFormat.nAvgBytesPerSec = waveFormat->nAvgBytesPerSec;
					soundFormat.nBlockAlign = waveFormat->nBlockAlign;
					soundFormat.wBitsPerSample = bitsPerSec;
					soundFormat.cbSize = 0;

					gosAudio_CreateResource(&radioHandle, gosAudio_UserMemory, NULL, &soundFormat,dataOffset,length);
					
					if (isChannelPlaying(BETTY_CHANNEL))
						gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume * 0.5f);
					else
						gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume);

					gosAudio_AssignResourceToChannel(RADIO_CHANNEL, radioHandle);
					gosAudio_SetChannelPlayMode(RADIO_CHANNEL,gosAudio_PlayOnce);
					
					}
					if (currentMessage->pilot)
						currentMessage->pilot->setMessagePlaying();
					playingNoise = TRUE;
				}
				else
				{
					playingNoise = FALSE;
					if (currentMessage->data[currentFragment])
					{
						//--------------------------------------------------------------------
						// Hand GOS sound the data it needs to create the resource Handle
						gosAudio_Format soundFormat;
						soundFormat.wFormatTag = 1;				//PCM
			
						MC2_WAVEFORMATEX *waveFormat = NULL;
						MemoryPtr dataOffset = NULL;
						DWORD length = 0;
						DWORD bitsPerSec = 0;
						wave_ParseWaveMemory(currentMessage->data[currentFragment],&waveFormat,&dataOffset,&length);
						
						bitsPerSec = waveFormat->nBlockAlign / waveFormat->nChannels * 8;
				
						soundFormat.nChannels = waveFormat->nChannels;
						soundFormat.nSamplesPerSec = waveFormat->nSamplesPerSec;
						soundFormat.nAvgBytesPerSec = waveFormat->nAvgBytesPerSec;
						soundFormat.nBlockAlign = waveFormat->nBlockAlign;
						soundFormat.wBitsPerSample = bitsPerSec;
						soundFormat.cbSize = 0;

						gosAudio_CreateResource(&radioHandle,gosAudio_UserMemory, NULL, &soundFormat,dataOffset,length);
								
						if (isChannelPlaying(BETTY_CHANNEL))
							gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume * 0.5f);
						else
							gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume);

						gosAudio_AssignResourceToChannel(RADIO_CHANNEL, radioHandle);
						gosAudio_SetChannelPlayMode(RADIO_CHANNEL,gosAudio_PlayOnce);
						
						if (currentMessage->pilot)
							currentMessage->pilot->setMessagePlaying();
					}
					else
						wholeMsgDone = TRUE;
						
					currentFragment++;
				}
			}
		}
		else	// wholeMessageDone
		{
			if (currentMessage)
			{
				if (currentMessage->pilot)
				{
					currentMessage->pilot->clearMessagePlaying();
#ifndef VIEWER
					ControlGui::instance->endPilotVideo();
#endif
				}

				removeCurrentMessage();
			}
		}
	}
		
	if (useSound && messagesInQueue && wholeMsgDone)
	{
		if (radioHandle)
		{
			gosAudio_DestroyResource(&radioHandle);		//Toss the current radio data which just completed!
			radioHandle = NULL;
		}
		
		//-----------------------------------------------------
		// We are done with the current sample, start the next
		currentFragment = 0;
		moveFromQueueToPlaying();

		if (!currentMessage && !messagesInQueue)		//It is now possible for ALL remaining messages to go away because the pilot/mech died!
			return;

#ifndef VIEWER
		if ( currentMessage->movieCode )
		{
			ControlGui::instance->playPilotVideo( 
				currentMessage->pilot, currentMessage->movieCode );
		}
#endif

		if (currentMessage->noise[currentFragment])
		{

            // sebi: TEMP AND UGLY! have to make it because noise sample fails to load and then game logic goes belly up
            // and somehow just plays 2 times main radio message
            if(currentMessage->noise[currentFragment] != (MemoryPtr)-1) {
			//--------------------------------------------------------------------
			// Hand GOS sound the data it needs to create the resource Handle
			gosAudio_Format soundFormat;
			soundFormat.wFormatTag = 1;				//PCM

			MC2_WAVEFORMATEX *waveFormat = NULL;
			MemoryPtr dataOffset = NULL;
			DWORD length = 0;
			DWORD bitsPerSec = 0;
			wave_ParseWaveMemory(currentMessage->noise[currentFragment],&waveFormat,&dataOffset,&length);
			
			bitsPerSec = waveFormat->nBlockAlign / waveFormat->nChannels * 8;
	
			soundFormat.nChannels = waveFormat->nChannels;
			soundFormat.nSamplesPerSec = waveFormat->nSamplesPerSec;
			soundFormat.nAvgBytesPerSec = waveFormat->nAvgBytesPerSec;
			soundFormat.nBlockAlign = waveFormat->nBlockAlign;
			soundFormat.wBitsPerSample = bitsPerSec;
			soundFormat.cbSize = 0;

			gosAudio_CreateResource(&radioHandle,gosAudio_UserMemory, NULL, &soundFormat,dataOffset,length);
				
			if (isChannelPlaying(BETTY_CHANNEL))
				gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume * 0.5f);
			else
				gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume);

			gosAudio_AssignResourceToChannel(RADIO_CHANNEL, radioHandle);
			gosAudio_SetChannelPlayMode(RADIO_CHANNEL,gosAudio_PlayOnce);
			
            }

			if (currentMessage->pilot)
				currentMessage->pilot->setMessagePlaying();
			playingNoise = TRUE;
		}
		else
		{
			playingNoise = FALSE;
			if (currentMessage->data[currentFragment])
			{
				//--------------------------------------------------------------------
				// Hand GOS sound the data it needs to create the resource Handle
				gosAudio_Format soundFormat;
				soundFormat.wFormatTag = 1;				//PCM
	
				MC2_WAVEFORMATEX *waveFormat = NULL;
				MemoryPtr dataOffset = NULL;
				DWORD length = 0;
				DWORD bitsPerSec = 0;
				wave_ParseWaveMemory(currentMessage->data[currentFragment],&waveFormat,&dataOffset,&length);
				
				bitsPerSec = waveFormat->nBlockAlign / waveFormat->nChannels * 8;
		
				soundFormat.nChannels = waveFormat->nChannels;
				soundFormat.nSamplesPerSec = waveFormat->nSamplesPerSec;
				soundFormat.nAvgBytesPerSec = waveFormat->nAvgBytesPerSec;
				soundFormat.nBlockAlign = waveFormat->nBlockAlign;
				soundFormat.wBitsPerSample = bitsPerSec;
				soundFormat.cbSize = 0;

				gosAudio_CreateResource(&radioHandle,gosAudio_UserMemory, NULL, &soundFormat,dataOffset,length);
					
				if (isChannelPlaying(BETTY_CHANNEL))
					gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume * 0.5f);
				else
					gosAudio_SetChannelSlider(RADIO_CHANNEL, gosAudio_Volume, SoundSystem::digitalMasterVolume * radioVolume);

				gosAudio_AssignResourceToChannel(RADIO_CHANNEL, radioHandle);
				gosAudio_SetChannelPlayMode(RADIO_CHANNEL,gosAudio_PlayOnce);
				
				if (currentMessage->pilot)
					currentMessage->pilot->setMessagePlaying();
			}
		}
			
		wholeMsgDone = FALSE;
	}
}


