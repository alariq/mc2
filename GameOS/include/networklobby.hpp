#pragma once
//===========================================================================//
// File:	 NetworkLobby.hpp												 //
// Contents: API for creating/joining multiplayer games						 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

// NetworkLobby.hpp should be included by the library that is performing all of the
// initial setup of games and players.  It need not be included by the actual application.

#include"networking.hpp"


typedef void(__stdcall *LPSTRINGCALLBACK)(const char *string, DWORD value);






// Functionality: Calls the callback for each session available using the current 
//					protocol.  The string is NULL when all sessions have been enumerated.  The
//					value returned is the session id.  It should be used when attempting to 
//					join a game.
//				  
// Return value: If successful, return value is GOS_OK.  
//
GOSERRORCODE __stdcall gos_EnumerateSessions(LPSTRINGCALLBACK callback);


// Functionality: Enumerates the players in the given session by calling the callback 
//					for each player in the session.  The callback string is NULL when 
//					all players have been enumerated.  The value given in the callback
//					is the GOSNETWORKID of the player.  It can be used when sending messages
//				    to the player.
// Return value: If successful, return value is GOS_OK.  If no session matching <session_name>
//					is found, returns GOS_ERR_SESSIONNOTFOUND.
//
GOSERRORCODE __stdcall gos_EnumeratePlayers(DWORD session_id, LPSTRINGCALLBACK callback);


// Functionality: Creates a new game using the given game name, player name and maximum players.
//				  
// Return value: If successful, return value is GOS_OK.  
//
GOSERRORCODE __stdcall gos_CreateGame(char *game_name, char *player_name, int max_players);


// Functionality: Joins the game with the given session ID.  The session ids are returned in the
//					callback listing all sessions.
//				  
// Return value: If successful, return value is GOS_OK.  If no session matching <session_id>
//					is found, returns GOS_ERR_SESSIONNOTFOUND.
//
GOSERRORCODE __stdcall gos_JoinGame(DWORD session_id, char *player_name);


// Functionality: Calls the callback once for each modem installed on this
//					machine, passing the name of the modem.
//				  
// Return value: If successful, return value is GOS_OK.  
//
GOSERRORCODE __stdcall gos_GetModemNames(LPSTRINGCALLBACK callback);


// Functionality: If connected to the modem protocol, gos_Dial attempts to dial the number
//				  given in the connection parameters.  It returns GOS_OK when connected.  
//
// Return value:  It returns GOS_DIALING while still attempting to connect.
//				  The user must continue calling Dial until it no longer returns GOS_DIALING.
GOSERRORCODE __stdcall gos_Dial();


// Functionality: If connected to a network protocol, gos_Disconnect resets the
//					connection.  This is useful to cancel dialing.
//
GOSERRORCODE __stdcall gos_Disconnect();
