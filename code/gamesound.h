//---------------------------------------------------------------------------
// GameSound.h - This file is the sound system header for the GAME
//
// MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef GAMESOUND_H
#define GAMESOUND_H
//---------------------------------------------------------------------------
// Include files
#ifndef MCLIB_H
#include"mclib.h"
#endif

#ifndef DWARRIOR_H
#include"dwarrior.h"
#endif

#ifndef RADIO_H
#include"radio.h"
#endif

//---------------------------------------------------------------------------
// Macro Defintiions

//---------------------------------------------------------------------------
class GameSoundSystem : public SoundSystem
{
	//Data Members
	//-------------
	protected:
		RadioData			*currentMessage;						//Radio message playing.
		unsigned long		messagesInQueue;						//Radio messages waiting to play.
		RadioData			*queue[MAX_QUEUED_MESSAGES];			//Radio message queue.
		unsigned long		currentFragment;						//Which piece are we playing.
		unsigned long		playingNoise;							//are we playing noise right now?
		bool 				wholeMsgDone;							//Are all fragments played?
		HGOSAUDIO			radioHandle;

		float				generalAlarmTimer;						//How long do we play the alarm buzzer?
			
	//Member Functions
	//----------------
	public:
	
		GameSoundSystem (void)
		{
			init();
		}
		
		~GameSoundSystem (void)
		{
			destroy();
		}
		
		void init (void)
		{
			SoundSystem::init();

			wholeMsgDone = true;
			currentMessage = NULL;
			messagesInQueue = 0;
			currentFragment = 0;
			playingNoise = false;
			radioHandle = NULL;

			//------------------------------------------------------------
			// Startup the Radio Message Queue.
			messagesInQueue = 0;
			wholeMsgDone = true;
			for (long i=0;i<MAX_QUEUED_MESSAGES;i++)
				queue[i] = NULL;

			generalAlarmTimer = 0.0f;
		}
		
		virtual void update (void);

		void purgeSoundSystem (void);					//This will shutdown all active sound.
		void removeQueuedMessage (long msgNumber);
		bool checkMessage (MechWarriorPtr pilot, byte priority, unsigned long messageType);
		long queueRadioMessage (RadioData *msgData);
		void moveFromQueueToPlaying (void);
		void removeCurrentMessage (void);
};

//---------------------------------------------------------------------------
extern GameSoundSystem *soundSystem;

extern bool useSound;
extern bool useMusic;

//---------------------------------------------------------------------------
#endif
