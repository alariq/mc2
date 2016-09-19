#pragma once
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//////////////////////////////////////////////////////////////////////
//
// NetworkMessages.hpp: interface for all network messages in Linkup Library
//
#include"directx.hpp"
#include"networking.hpp"
#include"flinkedlist.hpp"
#include"net.hpp"


#pragma pack(1)

class FIMessageHeader
{
	protected: 
		unsigned short	flags;

#ifdef _ARMOR
	public:
	
	unsigned short packetNumber;
#endif

		enum 
		{
			TypeBits = 0,
			MulticastBit = TypeBits+11,
			GuaranteedBit
		};

		enum 
		{
			TypeMask = 0x3FF, // 1st 10 bits
			MulticastFlag = 1 << MulticastBit,
			GuaranteedFlag = 1 << GuaranteedBit
		};

	public:

		FIMessageHeader()
		{
			flags = 0;
		}

		void Init()
		{
			flags = 0;
		}

		// Accessors
		inline bool IsMulticast()
		{
			if (flags & MulticastFlag)
				return true;
			else
				return false;
		}

		inline bool IsGuaranteed()
		{
			if (flags & GuaranteedFlag)
				return true;
			else
				return false;

		}

		// This message construct limits the application to
		// 1024 unique types of messages.  
		inline int GetType()
		{
			return flags & TypeMask;
		}

		// Information setters
		inline void SetMulticast()
		{
			flags |= MulticastFlag;
		}

		inline void SetUnicast()
		{
			flags &= ~MulticastFlag;
		}

		inline void SetGuaranteed()
		{
			flags |= GuaranteedFlag;
		}

		inline void SetNonGuaranteed()
		{
			flags &= ~GuaranteedFlag;
		}

		inline void SetType(unsigned int new_type)
		{
			gosASSERT(new_type < 0x3FF);
			flags &= ~TypeMask; // clear the type
			flags |= (new_type);
		}

		
};

class MessageTagger
{
	public:
		unsigned char	sendCounts[MAXPLAYERS];

	
		inline void Clear()
		{
			int i;
			for (i=0; i<MAXPLAYERS; i++)
			{
				sendCounts[i] = 255;
			}
		}
};


class FIGuaranteedMessageHeader: public FIMessageHeader,public MessageTagger
{
	public:
		// Constructor just calls Clear
		FIGuaranteedMessageHeader():FIMessageHeader(),MessageTagger()
		{
			SetGuaranteed();
			Clear();
		}

			
		inline void Init()
		{
			Clear();
			FIMessageHeader::Init();
			SetGuaranteed();
		}
};


#pragma warning (disable : 4200)
class FIGenericGuaranteedMessage:public FIGuaranteedMessageHeader
{
private:
	// Keep the constructor private because we don't want 
	// anyone to call it when there is an undefined size 
	// for the class.
	FIGenericGuaranteedMessage():FIGuaranteedMessageHeader()
	{}
public:
	unsigned char	buffer[0];
	
};


class FIGenericMessage:public FIMessageHeader
{
private:
	// Keep the constructor private because we don't want 
	// anyone to call it when there is an undefined size 
	// for the class.
	FIGenericMessage():FIMessageHeader()
	{}
public:
	unsigned char	buffer[0];
};



class FIVerifyCluster:public FIMessageHeader
{
protected:
	FIVerifyCluster():FIMessageHeader()
	{}

public:
	unsigned char n_messages;
	MessageTagger message[0];

	void Init()
	{
		FIMessageHeader::Init();
		SetType(FIDP_MSG_VERIFY_CLUSTER);
		n_messages = 0;
	}
};


class FIPlayerIDMessage:public FIGuaranteedMessageHeader
{
public:
	unsigned long playerID[MAXPLAYERS];
	BYTE serverIndex;

	FIPlayerIDMessage():FIGuaranteedMessageHeader()
	{
		SetType(FIDP_MSG_PLAYERID);
	}

	int GetPlayerNumber(unsigned long player_id)
	{
		int i;
		for(i=0;i<MAXPLAYERS;i++)
		{
			if (playerID[i] == player_id)
			{
				return i;
			}
		}
		return -1;
	}

	DWORD GetServerID()
	{
		return playerID[serverIndex];
	}


};



class FIPlayersInGroupMessage:public FIGuaranteedMessageHeader
{
public:
	DWORD groupID;
	DWORD playerID[MAXPLAYERS];

	FIPlayersInGroupMessage():FIGuaranteedMessageHeader()
	{
		SetType(FIDP_MSG_PLAYERS_IN_GROUP);
	}

};


class FIServerIDMessage:public FIGuaranteedMessageHeader
{
public:
	DWORD serverID;

	FIServerIDMessage(DWORD server_id):FIGuaranteedMessageHeader()
	{
		serverID = server_id;
		SetType(FIDP_MSG_SERVERID);
	}
};




#pragma warning (default : 4200)


#pragma pack()






class FIDPMessage:public NetworkMessageContainer, public ListItem
{
public:
	unsigned long	time;
	unsigned long	bufferSize;

	// originalTime is the time at which this message was originally
	// sent.  It differs from "time" which is the time which this message
	// was last resent.
	unsigned long	originalTime;

	// isResend is set to true when this message is sent for the second time.
	BOOL			isResend;

	// numTimesSent is the number of times this message has been sent.
	long			numTimesSent;

	// Constructor and destructor
	FIDPMessage(DPID player_id, DWORD buf_size=MAXMESSAGESIZE);

	virtual ~FIDPMessage();

   
	// Member functions
	//

	void Clear();

	inline DPID SenderID()
	{
		return senderId;
	}

	inline DPID ReceiverID()
	{
		return receiverId;
	}

	inline DWORD MessageSize()
	{
		return messageSize;
	}

	inline const void *GetBuffer()
	{
		return buffer;
	}

	inline DWORD Time()
	{
		return time;
	}

	inline void SetTime(DWORD new_time)
	{
		time = new_time;
		if (!isResend)
			originalTime = time;

	}

	inline void SetSenderID(DPID id)
	{
		senderId = id;
	}

	inline void SetReceiverID(DPID id)
	{
		receiverId = id;
	}

	DWORD SetMessageBuffer(LPVOID data, DWORD nbytes);


	HRESULT	ReceiveMessage();

};

