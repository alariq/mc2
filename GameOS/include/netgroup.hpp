//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// NetGroup.hpp: interface for the FIDPGroup class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(NETGROUP_HPP)
#define NETGROUP_HPP

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef FLINKED_LIST_HPP
#include"flinkedlist.hpp"
#endif

#ifndef NETWORKMESSAGES_HPP
#include"networkmessages.hpp"
#endif


class ListFriendlyDPID:public ListItem
{
public:
	DPID ID;

	ListFriendlyDPID(DPID id)
	{
		ID = id;
	}

	ListFriendlyDPID& operator=(DPID id)
	{
		ID = id;
		return *this;
	}


};

class FIDPGroup:public ListItem
{
protected:
	DPID				groupID;
	DPID				parentGroupID;
	char				shortName[64];
	char				longName[256];
	unsigned long		groupFlags;
	
	LPVOID		groupData;
	DWORD		dataSize;	
	FLinkedList<ListFriendlyDPID> playerIDList;



public:
	
	
	// Constructor and destructor
	FIDPGroup();
	FIDPGroup(DPID id,
		DPID parent_id,
		LPCDPNAME name,
		DWORD flags);

	virtual ~FIDPGroup();


	inline FLinkedList<ListFriendlyDPID> *GetPlayerList()
	{
		return &playerIDList;
	}

	BOOL AddPlayer(DPID& id);

	BOOL RemovePlayer(DPID& id);

	inline void  SetShortName(char *name)
	{
		if (name != NULL)
		{
			strncpy(shortName,name,64);		
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

	const char *GetShortName()
	{
		return shortName;
	}

	const char *GetLongName()
	{
		return longName;
	}


	void SetGroupData(LPVOID data, DWORD size);

	inline LPVOID GetGroupData()
	{
		return groupData;
	}

	inline DPID ID()
	{
		return groupID;
	}

};

#endif // !defined(FIDPGROUP_H)
