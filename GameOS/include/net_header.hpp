#pragma once
//
//============================================================
//
// Contents:  Network header file
//
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#include"network.hpp"
#include"directx.hpp"

//
// Packet overhead of DirectPlay
//
#define PACKET_OVERHEAD 42


#define OUTBOUND_WINDOW


//
//
// Used to store network packets
//
//
typedef struct _PacketBuffer
{
	_PacketBuffer*	pNext;
	DWORD			Size;			// Size of buffer
	DWORD			PacketSize;
	DWORD			FromID;
	DWORD			ToID;
	double			TimeStamp;
	bool			Encrypted;
	BYTE			Data[0];		// Data follows

} PacketBuffer;

//
// List of names, used for modems, serial ports, players, games etc...
//
typedef struct _ListOfNames
{
	_ListOfNames*	pNext;				// Pointer to next in chain
	DWORD			Data;				// Data that is list dependent
	DWORD			PacketsReceived;	// NUmber of packets received from a player
	double			LastTime;			// Time last packet received
	char			Name[1];			// Name

} ListOfNames;


//
// List of games (and players in those games)
//
typedef struct _ListOfGames
{
	_ListOfGames*	pNext;				// Pointer to next in chain
	DPSESSIONDESC2	SessionDescriptor;	// Session description
	ListOfNames*	pPlayers;			// Pointer to list of player names
	bool			bIPX;				// whether the game is hosted with IPX or not (TCP/IP)
	char			Name[1];			// Name

} ListOfGames;

//
// List of messages waiting for the application to get
//
typedef struct _Messages
{
	_Messages*	pNext;
	NetPacket	Data;

} Messages;


//
// All memory in networking must be allocated on this heap
//
extern HGOSHEAP Heap_Network;


//
// Interfaces
//
#define MAX_DP_OBJECTS	8
extern IDirectPlayLobby3*		dplobby3;	// Lobby interface
extern IDirectPlay4*			dplay4;	// Direct Play interface


//
// Current joined or opened session description
//
extern DPSESSIONDESC2 CurrentSession;

//
// Last time that games and players were enumerated
//
extern double LastEnumTime;
extern bool WaitingForEnum;

//
// Protocols
//
extern bool hasTCP;
extern bool hasIPX;							// True when protcol is valid
extern bool hasModem;
extern bool hasSerial;
extern bool hasZoneMatch;

//
// True when connection exists
//
extern int Connected;

//
// Result from last enumeration on a thread
//
extern HRESULT EnumResult;

//
// True when a game created or joined
//
extern bool InNetworkGame;

//
// True when created the game
//
extern bool IAmTheServer;

//
// Current players ID
//
extern DWORD MyDirectPlayID;

//
// String containing my address (when TCPIP)
//
extern char* MyIPAddress;

//
// Valid serial ports
//
extern ListOfNames* SerialPortNames;

//
// Valid modem names (maximum of 8)
//
extern ListOfNames* ModemNames;

//
// Points to a list of the current game names found on the network
//
extern ListOfGames*	GameNames;

extern char** ListOfPassworded;

//
// Points to a list of the current game names found on the network being enumerated
//
extern ListOfGames* EnumNames;

//
// List of messages on the recieve queue
//
extern Messages* pMessages;

//
// List of players in the current game
//
extern ListOfNames* CurrentPlayers;

//
// Buffer used while receiving messages
//
extern BYTE* MessageBuffer;
extern DWORD MessageBufferSize;

//
// Points to a list of messages that the Application will pull message from
//
extern Messages*	pMessages;
//
// Points to the last message read by the Application (will be freed)
//
extern Messages*	pLastMessage;




//
// Linked list of sent/recieved packets
//
typedef struct _PacketLogging
{
	_PacketLogging* pNext;
	double			TimeStamp;							// When packet was added to list
	DWORD			FrameNumber;						// Frame number added
	char			Type[11];							// Type of packet
	char			Player[11];							// From or To player name
	DWORD			FromID;								// ID of FROM player
	DWORD			ToID;								// ID of TO player
	DWORD			Size;								// Size of packet
	BYTE			Data[8];							// 1st 8 bytes

} PacketLogging;
//
// Pointer to latest packet
//
extern PacketLogging*	pPacketLog;
//
// Exact time network stats were grabbed
//
extern double NetworkDebugTime;
//
// Pointer to log at start of frame
//
extern PacketLogging*	pDebugPacketLog;





#pragma pack(push,1)
//
// Structure sent with all network packets
//
#ifdef OUTBOUND_WINDOW
typedef struct _PacketHeader
{
	BYTE	Type;										// User defined type (224-> system messages)
	WORD	ThisPacketNumber;
	WORD	LastPacketReceived;

} PacketHeader;
#else
typedef struct _PacketHeader
{
	BYTE	Type;										// User defined type (224-> system messages)

} PacketHeader;
#endif



#pragma pack(pop)


//
// Routine's
//
void CheckProtocols();
void ReceivePackets();
void GetCurrentPlayers();
void AddGOSMessage( Messages* pMessage );
BOOL PASCAL EnumSessionsCallback( LPCDPSESSIONDESC2 lpSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags, void* lpContext );
BOOL PASCAL EnumJoinSessionCallback( LPCDPSESSIONDESC2 lpSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags, void* lpContext );
BOOL PASCAL EnumPlayersCallback( DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext );
BOOL FAR PASCAL ModemCallback( REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, void* lpContext );
BOOL FAR PASCAL TCPIPCallback( REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, void* lpContext );
void WaitTillQueueEmpty();
void AddPlayerToGame( ListOfNames** pListOfPlayers, char* Name, DPID dpId );
void RemovePlayerFromGame( ListOfNames** pListOfPlayers, char* Name, DPID dpId );
char* GetName10( DWORD Id );
void UpdateNetworkDebugInfo();
char* DecodeIPAddress( DPLCONNECTION* pConnection );
WORD DecodePORTAddress( DPLCONNECTION* pConnection );


//
// Receive packet thread variables
//
extern unsigned int __stdcall NetworkThread(void*);
extern unsigned int NetworkThreadID;
extern HANDLE HandleNetworkThread;
extern CRITICAL_SECTION NetworkCriticalSection;
extern HANDLE NetworkEvent;
extern HANDLE KillNetworkEvent;
extern unsigned int __stdcall NetworkThread(void*);

//
// Enumeration thread variables
//
extern unsigned int EnumThreadID;
extern HANDLE HandleEnumThread;
extern CRITICAL_SECTION EnumCriticalSection;
extern HANDLE EnumEvent;
extern HANDLE KillEnumEvent;
extern unsigned int __stdcall EnumThread(void*);

//
// Used to hold list of packets to pass data between threads
//
extern PacketBuffer* pEmptyList;		// List of spare blocks
extern PacketBuffer* pPackets;			// List of received packets
extern PacketBuffer* pLastPacket;		// End of list of received packets



//
// Functions used initialize game list drivers
//

int InitLanGames();
int InitGUNGames();
void GUNDestroyNetworking();
void CheckForInternet();


// InternalJoinGame() needs this for joining GUN Games.
void PushGameList( void );
bool GUNPrepareDPlay( const char * GameName );

HRESULT QuickEnum( bool async ); // quickly begin dplay enumeration of sessions.



#ifdef OUTBOUND_WINDOW
typedef struct tagPACKETQUEUE
{ 
	NetPacket *				pPacket;
	struct tagPACKETQUEUE *	pNext;

} PACKETQUEUE;


//========================================================================
// OutboundWindow
//
// The OutboundWindow class is an object that manages how many non
// guaranteed packets are allowed to be outstanding and not accounted for.
// This prevents us from filling the modem send queue qith messages.
//
// What we do is we stamp each outgoing message with a count, and the
// remote machine informs us of the last packet it received with every
// packet it sends to us. This way, we can know how many outstanding non
// guaranteed packets are outstanding at any time. If there are more than
// some predetermined number (the "window"), then any attempt to send a
// packet fails.
//
// A client will have a single OutboundWindow object, associated with the
// server. A server will have one for each client.
//
// If we haven't sent the last packet we've received after a certain
// amount of time, we manually send this in a packet that contains ONLY
// this.
//========================================================================
class OutboundWindow
{
public:
	static const int m_WindowSize;
	static const double m_ResendTime;

	// we only have one constructor: you MUST supply the DPID when you create one!!!
	OutboundWindow(DPID);

	static bool __stdcall Synchronize(void);	// call every frame
	static bool __stdcall CleanAll(void);
	static OutboundWindow *Find(DPID dpid);
	bool __stdcall Add(void);
	bool __stdcall Remove(void);
	void * operator new(size_t size)
	{
		return gos_Malloc(size, Heap_Network);
	}
	void operator delete( void* ptr )
	{
		gos_Free(ptr);
	}
	void operator delete[] ( void* ptr )
	{
		gos_Free(ptr);
	}


public:
	double					m_TimeLastPacketSent;
	WORD					m_NextPacketNumberToSend;
	WORD					m_LastPacketWeReceived;
	WORD					m_LastPacketTheyReceived;
	PACKETQUEUE *			m_PacketQueue;
	DPID					m_dpid;			// the dpid of the associated remote machine
	OutboundWindow *		m_pNext;

	static OutboundWindow *	m_pHead;

	//PacketLogging* pThis=(PacketLogging*)gos_Malloc( sizeof(PacketLogging), Heap_Network );

};

#endif OUTBOUND_WINDOW
