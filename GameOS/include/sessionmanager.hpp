#pragma once
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// SessionManager.h: interface for the SessionManager class.
//
#include"networkingglobals.hpp"
#include"flinkedlist.hpp"
#include"session.hpp"
#include"netplayer.hpp"
#include"networkmessages.hpp"

typedef void(*LPVOIDCALLBACK)();

#include"network.hpp"




//
//============================================================
//
// Contents:  Low level networking
//
// Owner   :  AndyG
//
// All Rights reserved worldwide (c) Microsoft
// This unpublished sourcecode is PROPRIETARY and CONFIDENTIAL               
//============================================================
//
#include"session.hpp"



//
// Current session
//
extern FIDPSession *currentSession;



//
// Current connection
//
extern int currentConnectionType;
extern bool Connected;




//
// Modem/Serial has dialed
//
extern bool Connected;




//
// Modem variables
//
extern char modemNames[10][64];
extern DWORD NumModems;
extern bool modemChecked;
extern bool hasModem;
bool __stdcall FindModems();
bool __stdcall gos_ConnectModem( char *phone_number, char *modem_name );


//
// COM port variables
//
bool __stdcall gos_ConnectComPort(DWORD com_port);


//
// IPX variables
//
extern bool hasIPX;
bool __stdcall gos_ConnectIPX();



//
// TCPIP variables
//
extern bool hasTCP;
bool __stdcall gos_ConnectTCP( char *ip_address, WORD port );















/////  AddressInfo is used in connections initialized entirely by
// user defined data, such as a TCP connection created by calling
// the CreateCompoundAddress function and using it as the connection.
typedef struct _DPAddressInfo
{
	DWORD	type;
	DWORD	size;
	LPVOID	data;
} DPAddressInfo;






//////////////////////////////////////////////////////////////////////
// SessionManager class.  The SessionManager deals with all DirectPlay
// specific functionality.  It allows the user to call a few simple functions
// to get a network game going and send/receive messages.  
//
//	----------   Using the SessionManager: ---------------
//  The user must create one instance (and only one) of the SessionManager,
// passing it the Application's GUID.
//
//  Once the SessionManager is created, the user must choose a connection type
// and call Connect() with the appropriate connection ID.  If using connections
// from the GetConnections() method, the user can simply call Connect().  
// Otherwise, one of the ConnectThruXXX functions must be called.
//
//	Once connected, the user can either Host or Join a session.  To host a session,
// simply create a temporary session object and pass it into the HostSession() function.
// To join a session, the user must get a GUID from the list of sessions returned by
// the GetSessions() function, then call JoinSession() with that GUID.
//
//  Now, a player has been created for the user, and the session is setup to send
// and receive messages.  Messages are constantly being retrieved from the 
// ReceiveThread, so the user can process them anytime he wishes by calling
// ProcessSystemMessages() and ProcessApplicationMessages() respectively.  The user
// must additionally call UpdateGuaranteedMessages() at a high frequency in order
// to re-send lost messages.
//
//	Sending Messages:
// To send a message, the user must create an application specific message
// descended from either FIMessageHeader or FIGuaranteedMessageHeader.  The
// constructor should call SetType() and set the message type to a constant
// added to the enumerated message type list.  Once the message is created and
// all parameters are set, it can be sent by calling the appropriate send function. 
// 
// 
// call
//
//////////////////////////////////////////////////////////////////////

class SessionManager
{
	public:

	FLinkedList<FIDPSession>	listOfSessions;
	FLinkedList<FIDPPlayer>		listOfPlayers;
	FLinkedList<FIDPMessage>	listOfEmptyMessages;
	FLinkedList<FIDPMessage>	listOfSystemMessages;
	FLinkedList<FIDPMessage>	listOfApplicationMessages;
	FLinkedList<FIDPMessage>	guaranteedMessagesToProcess;
	FLinkedList<FIDPMessage>	guaranteedMessagesToSend;
	FLinkedList<FIDPMessage>	serverMessageHoldList;


	// The needIDList is used when a player is added to the session,
	// but we don't know what playerNumber to assign it.  When a 
	// playerID message comes through, we can go through the needIDList
	// and give these players their numbers. 
	FLinkedList<FIDPPlayer> needIDList;

	// rtPlayerListIterator is used to iterate the player list from the
	// receiveThread.
	FListIterator<FIDPPlayer> *rtPlayerListIterator;
	
	// this list is filled in with players who have dropped out.
	DPID cancelledPlayerID[MAXPLAYERS];
	
	FIServerIDMessage *serverIDMessage;

	HANDLE hPlayerEvent;
	HANDLE hKillReceiveEvent;
	HANDLE playerThread;
	DPID thisPlayerID;
	DPID serverID;
	FIDPPlayer *thisPlayer;
	DWORD threadID;

	BOOL launchedFromLobby; // TRUE if this app was launched through a DPlay lobby.
	BOOL lobbyChecked; // TRUE if we have initialized launchedFromLobby.
	BOOL inReceiveThread; // TRUE while in ReceiveThread, FALSE while in another thread.
	BOOL DisabledCallerID;
	BOOL CallerIDChanged[6];
	
	
	long enterGameTime;


	// nextSessionID is the ID of the next session added.  When enumerating all
	// sessions from scratch, this number is set back to zero.
	int nextSessionID;

	// enableAutoDialValue is the value of the EnableAutodial key in the
	// registry.  
	DWORD enableAutodialValue;

	HANDLE ReceiveMutex;
	HANDLE EmptyMessageListMutex;
	CRITICAL_SECTION ProcessingMessageList;
	CRITICAL_SECTION AddingMessageList;

	// bSessionHost is set to TRUE when this application hosts
	// a session.
	BOOL bSessionHost;

	// canSendGuaranteedMessages is set to true when this player
	// gets a list of player ids from the server.
	BOOL canSendGuaranteedMessages;

	


	int NetworkProtocolFlags;
	
	unsigned char messageBuffer[1024];


	DPID newPlayerNumbers[MAXPLAYERS];


	// nextFileSendID is a number used as a file handle for files
	// being send across the network.
	int nextFileSendID;

#ifdef _ARMOR
	unsigned long lastBandwidthRecordingTime;
	unsigned long bytesSent;
	unsigned long lastRecordedBytesSent;
	unsigned long packetNumber;
	unsigned long lastPacketNumber;
	unsigned long packetsInBandwidthCheck;
#endif


	FIVerifyCluster *verifyClusterBuffer;
	FIVerifyCluster *verifyMessageCluster[MAXPLAYERS];
	DPID receiveThreadPlayerIDs[MAXPLAYERS];
	
	int serverOrder[MAXPLAYERS];
	DWORD pingUpdateTime;
	DWORD pingUpdateFrequency;


	HRESULT	ResetDirectPlayInterface();

	HRESULT CreatePlayer(char *player_name);


	inline HRESULT SetSessionDescriptor(FIDPSession *ss)
	{
		return wSetSessionDesc(dplay4, ss -> GetSessionDescriptor(),0);
	}


	void SendVerifyMessage(int player_number, FIDPMessage *msg, int send_count);


	void EnumeratePlayers(FIDPSession *session);

	void AddPlayerOrGroup(
						DWORD dwPlayerType,
						DPID dpId,
						DPID dpIdParent,
						LPDPNAME dpnName,
						DWORD dwFlags);
	
	// DeletePlayerOrGroup is called when a player or group leaves the
	// game -- after calling AppCallback.  It actually deletes this player
	// or group from the linked list.
	void DeletePlayerOrGroup(
						DWORD dwPlayerType,
						DPID dpId);


	// PlayerOrGroupLeaving is called when a player or group leaves the
	// game -- before calling AppCallback.
	void PlayerOrGroupLeaving(	DWORD dwPlayerType,
								DPID dpId);
	
	void HandleApplicationMessage(FIDPMessage *msg);

	void UpdatePlayerGuaranteedMessages
			(FIDPPlayer *player, DWORD time);

	// ProcessGuaranteedMessages is called by ProcessApplicationMessages
	// once all messages have been initially handled.  Guaranteed messages
	// are processed in 2 steps.  The first puts them in the proper hold 
	// buffers.  The second gets messages ready to be processed from the
	// hold buffers.
	void ProcessGuaranteedMessages();

	// The following 2 functions are called to add and remove messages from
	// the empty message queue.  They use mutexes so the 2 threads don't
	// both access the queue at the same time.
	void AddMessageToEmptyQueue(FIDPMessage *msg);

	FIDPMessage *GetMessageFromEmptyQueue();




public:
		// Returns the session in the listOfSessions with a matching guid
		// or NULL if there is no match.
		FIDPSession *FindMatchingSession(LPGUID pguid);
private:


	// Functions called by the Callbacks.  These functions are
	// essentially the callbacks for enumerating connections,
	// sessions, players, and groups.  However, since the callback
	// itself cannot be a member function of a class, the callback
	// bounces the message into the class using the lpContext member.
	BOOL FAR PASCAL AddConnection(
						LPCGUID     lpguidSP,
						LPVOID		lpConnection,
						DWORD		dwConnectionSize,
						LPCDPNAME   lpName,
						DWORD 		dwFlags,
						LPVOID 		lpContext);

	BOOL FAR PASCAL AddSession(
						LPCDPSESSIONDESC2 lpThisSD,
						LPDWORD lpdwTimeOut,
						DWORD dwFlags,
						LPVOID lpContext
						);

	BOOL FAR PASCAL NewPlayerEnumeration(DPID dp_id,
										DWORD dwPlayerType,
										LPCDPNAME lpName,
										DWORD dwFlags);

	// RTProcessApplicationMessage is called by the receive thread when
	// a new application message arrives.
	void RTProcessApplicationMessage(FIDPMessage *message_info);

	// RTHandleNewGuaranteedMessage is called to handle an incoming guaranteed
	// message in the receive thread.
	void RTHandleNewGuaranteedMessage(FIDPMessage *message_info, FIDPPlayer *sending_player);

	// returns a DPID from a player number
	DPID RTGetIDFromPlayerNumber(int pn);
	
	FIDPPlayer *RTGetPlayer(DPID id);

	void ClearSessionList();
	

	void HandlePreSystemMessage(FIDPMessage *msg);
	void HandlePostSystemMessage(FIDPMessage *msg);

	void SetupMessageSendCounts(
						FIGuaranteedMessageHeader *message,
						FLinkedList<FIDPPlayer> *player_list);

	// SendPreIDGuaranteedMessages is called once when the new player
	// gets a list of player numbers and the server ID.
	void SendPreIDGuaranteedMessages();

	// GivePlayerAnID assigns the first available playerNumber to 
	// this player.  If there is no gap in assigned numbers, this
	// player gets the next one in line.
	void GivePlayerAnID(FIDPPlayer *player);


	public:
		
		SessionManager(GUID app_guid);
		
		virtual ~SessionManager();

		virtual void destroy (void);


		friend BOOL FAR PASCAL DPSessionMgrSessionsCallback(
						LPCDPSESSIONDESC2	lpSessionDesc,
						LPDWORD				lpdwTimeOut,
						DWORD				dwFlags,
						LPVOID				lpContext);

		friend BOOL FAR PASCAL EnumPlayersCallback (DPID dpId,
										DWORD dwPlayerType,
										LPCDPNAME lpName,
										DWORD dwFlags,
										LPVOID lpContext);

		friend BOOL FAR PASCAL EnumGroupsCallback (DPID dpId,
									  DWORD dwPlayerType,
									  LPCDPNAME lpName,
									  DWORD dwFlags,
									  LPVOID lpContext);

		friend DWORD WINAPI
			SessionManagerReceiveThread(LPVOID lpThreadParameter);


		// ReceiveThread is called by the SessionManagerReceiveThread
		// callback to handle thread events.
		int ReceiveThread();



		// InitializeConnection is called by one of the following functions
		// to allow the user to bypass the crappy windows dialogs.
		long InitializeConnection(DPCOMPOUNDADDRESSELEMENT *compound_address, 
							int n_items);

		long Dial();
		void CancelDialing();


		// Call this function at program start up to find out if
		// we were launched from a lobby.
		BOOL WasLaunchedFromLobby();

		// If we were launched from a lobby, SetupLobbyConnection creates
		// a protocol, session, and player to start the game.
		DWORD SetupLobbyConnection(LPVOIDCALLBACK create_callback,LPVOIDCALLBACK destroy_callback);


		// returns the name of the modem at index or NULL if index
		// is greater than the # of modems.
		char *GetModemName(long index);

		// Host or join a session
		//
		// This must be done before attempting to Send or Receive
		// any messages
		// A return value of DP_OK means it worked
		//	A return value of -1 means the guid passed to JoinSession is
		// invalid.  Any other return value is a directplay error code.
		HRESULT HostSession(FIDPSession& new_session, char *player_name);

		HRESULT JoinSession(LPGUID p_guid, char *player_name);

		// LockSession can only be done by the host.  If successful, it
		// returns 1, else it returns 0
		int LockSession();

		// LeaveSession kills the current player and removes him from the session.
		int LeaveSession();


		void CreateGroup 
			(	LPDPID id, 
				char* name, 
				LPVOID data = NULL, 
				DWORD size = 0, 
				DWORD flags = 0);

		void SetGroupData 
			(	DPID id, 
				LPVOID data, 
				DWORD size, 
				DWORD flags = 0);

		// If player_id is 0 or not filled in, the local player is
		// added.  If the group doesn't exist, the player isn't added.
		// returns 1 if successful, 0 if not.
		int AddPlayerToGroup(DPID group_id, DPID player_id=0);

		int RemovePlayerWithID(DPID player_id);

		int RemovePlayerFromGame(FIDPPlayer *p);

		// Returns 1 if successful, 0 if not.
		int RemovePlayerFromGroup(DPID group_id, DPID player_id=0);

		void GetPlayerListForGroup(
					DPID id, 
					FLinkedList<FIDPPlayer> *player_list);



		// 3. Send a message
		//
		// Messages can be broadcasted to all players,
		// sent to a group, or
		// sent to individual players.

		void SendMessageToGroupGuaranteed(
						DPID group_id, 
						FIGuaranteedMessageHeader *message, 
						DWORD size);
		
		void SendMessageToPlayerGuaranteed(
						DPID player_id, 
						FIGuaranteedMessageHeader *message, 
						DWORD size,
						BOOL set_send_count = TRUE);
		
		void SendMessageToServerGuaranteed(
						FIGuaranteedMessageHeader *message, 
						DWORD size);
						
		void BroadcastMessage(FIMessageHeader *message, DWORD size);
		
		void SendMessageToServer(
						FIMessageHeader *message, 
						DWORD size);
		
		HRESULT SendMessage(
						DPID group_or_player_id,
						FIMessageHeader *message, 
						DWORD size);

		HRESULT SendGOSMessage(NetworkMessageContainer& message_info);

		void SendMessageFromInfo(FIDPMessage *message);

		int SendVerifies();


		// Accessor functions to update lists of network specifics
		FLinkedList<FIDPSession> *GetSessions(bool refresh=true);
		FLinkedList<FIDPPlayer> *GetPlayers(FIDPSession *session=NULL);
		

		inline FIDPSession *GetCurrentSession()
		{
			return currentSession;
		}

		inline FIDPPlayer *GetLocalPlayer()
		{
			return thisPlayer;
		}

		DPID GetServerID()
		{
			return serverID;
		}


		// FindSession returns the session with the corresponding id or NULL
		// if no match is found.
		FIDPSession *FindSession(int session_id);

		// IsLocked returns the status of the session with the given session_id.
		// If the session_id is -1, it returns the status of the current session.
		BOOL IsLocked(int session_id = -1);

		FIDPPlayer *GetPlayerNumber(long i);

		FIDPPlayer *GetPlayer(unsigned long id);

		inline BOOL IsLocalHost()
		{
			return bSessionHost;
		}


		// ProcessMessages should be called to get all new messages and
		// send guaranteed and file messages.
		void ProcessMessages();

		//ProcessSystemMessages processes all of the system messages in the
		//receive buffer.  If the application needs to deal with them
		//individually, the sys_callback function is called for each one.
		int ProcessSystemMessages();

		int ProcessApplicationMessages();

		// UpdateGuaranteedMessages should be sent by the application???
		void UpdateGuaranteedMessages();

		
		
};


extern SessionManager *globalSessionManager;

void InitNetworking();

