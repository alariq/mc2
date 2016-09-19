#pragma once
//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// Session.h: interface for the Session class.
//
//////////////////////////////////////////////////////////////////////
#include<dplay.h>
#include"flinkedlist.hpp"

#define MAXNAMELEN 64

class FIDPSession: public ListItem
{
	protected:
		DPSESSIONDESC2 SessionDescriptor;
		char Name[MAXNAMELEN];
		char Password[MAXNAMELEN];

	public:
		int ID;
			
	public:
		FIDPSession(int id=0);
		FIDPSession(DPSESSIONDESC2 sdesc, int id);
		FIDPSession(FIDPSession& session);

		virtual ~FIDPSession();

		void Initialize(DPSESSIONDESC2 sdesc);


		DPSESSIONDESC2 *GetSessionDescriptor()
		{
			return &SessionDescriptor;
		}

		inline void SetNumPlayers(int nplayers)
		{
			SessionDescriptor.dwCurrentPlayers = nplayers;
		}

		inline void SetMaxPlayers(int maxplayers)
		{
			SessionDescriptor.dwMaxPlayers = maxplayers;
		}

		void SetName(char *name);
		
		void SetPassword(char *password);

		inline int GetNumPlayers()
		{
			return SessionDescriptor.dwCurrentPlayers;
		}

		inline int GetMaxPlayers()
		{
			return SessionDescriptor.dwMaxPlayers;
		}

		inline const char *GetName()
		{
			return SessionDescriptor.lpszSessionNameA;
		}
		
		inline const char *GetPassword()
		{
			return SessionDescriptor.lpszPasswordA;
		}

		inline LPGUID GetGuidInstance()
		{
			return &SessionDescriptor.guidInstance;
		}

		
		inline BOOL IsPasswordRequired()
		{
			return SessionDescriptor.dwFlags&DPSESSION_PASSWORDREQUIRED;
		}

		inline BOOL IsLocked()
		{
			return SessionDescriptor.dwFlags&DPSESSION_NEWPLAYERSDISABLED;
		}

		inline void Lock()
		{
			if (!IsLocked())
			{
				SessionDescriptor.dwFlags |= DPSESSION_NEWPLAYERSDISABLED;				
				PropagateSettings();
			}
		}

		void PropagateSettings();
		

};









