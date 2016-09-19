//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// DPPlayer.h: interface for the FIDPPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(NETPLAYER_HPP)
#define NETPLAYER_HPP

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef FLINKEDLIST_HPP
#include"flinkedlist.hpp"
#endif

#ifndef NETWORKMESSAGES_HPP
#include"networkmessages.hpp"
#endif


class FIDPPlayer:public ListItem
{
protected:	
	char				shortName[128];
	char				longName[256];
	unsigned long		playerID;
	unsigned long		playerFlags;

	// The awaitingVerificationList holds a list of messages
	// that were sent to this player and are awaiting 
	// verification.
	FLinkedList<FIDPMessage>		verifyList;

	// The incomingHoldBuffer holds a list of guaranteed messages
	// that came in out of order.  They will be processed as soon
	// as the item before them is processed.
	FIDPMessage *incomingHoldBuffer[256];
	
	// the playerCriticalSection protects data from being accessed by
	// the receive thread and the main thread at once.a
	CRITICAL_SECTION playerCriticalSection;



public:
	// sendCount is used for guaranteed messages.  It
	// is always between 0 and 255 and it represents
	// the next message sent to this player
	unsigned char				sendCount;

	// The incomingHoldIndex is the next expected sendCount for
	// incoming messages from this player.
	unsigned char incomingHoldIndex;

	unsigned char nextIncomingSendCount;

	int nItemsInHoldBuffer;

	// player_number is used for guaranteed messages.  It represents
	// which 4 bits to use for this player's messages.
	int					playerNumber;

	// isActive is set to true when the player is created, and is set
	// to FALSE when the player is kicked out.
	BOOL				isActive;


	// messageResendTime is the time to wait before re-sending a message.  If
	// a message is re-sent, this value is bumped up a bit.  If the average 
	// latency is well below the resend time, it is dropped.
	unsigned long		messageResendTime;

	// physicalMemory is the amount of RAM on this player's machine.
	unsigned long		physicalMemory;



	
	// Public functions
	FIDPPlayer();
	FIDPPlayer(DPID& id,
		LPCDPNAME name,
		DWORD flags);
	virtual ~FIDPPlayer();


	// SetMessageSendCount is called each time a new message is
	// prepared to send to this player.  
	inline void SetMessageSendCount(FIGuaranteedMessageHeader *msg)
	{
		sendCount++;
		if (sendCount == 255)
			sendCount = 0;
		msg -> sendCounts[playerNumber] = sendCount;		
	}

	void JoinGroup(DPID group);

	void LeaveGroup(DPID group);
	
	inline void  SetShortName(char *name)
	{
		if (name != NULL)
		{
			strncpy(shortName,name,127);		
		}
		else
		{
			strcpy(shortName,"");
		}
	}
	
	inline void SetLongName(char *name)
	{
		if (name != NULL)
		{
			strncpy(longName,name,255);		
		}
		else
		{
			strcpy(longName,"");
		}
	}

	inline const char *GetShortName()
	{
		return shortName;
	}

	inline const char *GetLongName()
	{
		return longName;
	}

	inline DPID PlayerID()
	{
		return playerID;
	}

	inline FLinkedList<FIDPMessage> *GetVerifyList()
	{
		return &verifyList;
	}


	void AddToVerifyList(FIDPMessage *msg);
	

	// HandleIncomingMessage places the message into the incomingHoldBuffer.
	// if its new and returns TRUE.  If the message is old, it returns FALSE.
	BOOL HandleIncomingMessage(FIDPMessage *message_info, int send_count);

	void SetNextIncomingSendCount();

	FIDPMessage *RemoveFromVerifyList(int send_count);

	BOOL IsVerifyListFull();

	int VerifyCountDifference();

	FIDPMessage *NextMessageToProcess();


	BOOL IsInGroup(DPID group_id);



};

#endif // !defined(DPPLAYER_H)
