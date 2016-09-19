//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once


class GosEventIdMgr
{
	struct EventInfo
	{
		GosLogRef::EventType	m_type;
		char *					m_pName;
		char *					m_pFileName;
		int						m_nLineNo;
		DWORD					m_id;
	};


	static EventInfo			*pEventInfo;
	static unsigned long		NextEntry;
	static int					ListSize;
	static int					ListSpace;

	public:
	static void					Resize();
	static DWORD				IdToEntry( DWORD id ) { return id & 0x000FFFFF; }
	static DWORD				EntryToId( DWORD entry, int type ) { return entry|(type<<20); }
	static DWORD				AssignId( GosLogRef::EventType type, char *name, char *filename, int lineno )
								{
									if( !ListSpace )
										Resize();
									EventInfo *pInfo = pEventInfo + NextEntry;
									ListSpace--;
									pInfo->m_type = type;
									pInfo->m_pName = name;
									pInfo->m_pFileName = filename;
									pInfo->m_nLineNo = lineno;
									pInfo->m_id = EntryToId( NextEntry, (int)type );
									NextEntry++;
									return pInfo->m_id;
								}
	static char *				EventName( DWORD id ) { return pEventInfo[IdToEntry(id)].m_pName; }
	static void					Cleanup();
};
