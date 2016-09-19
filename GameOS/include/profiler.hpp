#pragma once
//===========================================================================//
// File:	 Profiler.hpp													 //
// Contents: Profiler API's													 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

extern DWORD* TraceBuffer;
extern gos_VERTEX* LineBuffer;





extern BYTE	ProfileFlags[32];			// Remember graphs activated 
extern DWORD DebuggerTextures;
extern float PercentHistory[512];		// Used to remember cycles per frame
extern bool NewPerformanceRegister;

//
// Structure used to hold statistics
//
typedef struct _Stat
{
	_Stat*			pNext;				// Pointer to Next
	DWORD			Flags;				// flags
	char*			TypeName;			// Pointer to type name
	gosType			Type;				// Type of variable
	DWORD			Count;				// Count for average
	void*			pVariable;			// Pointer to the variable in question
		
	float			MaxPercentage;		// Max percentage
	float			MinPercentage;		// Min percentage
	float			TotalPercentage;	// Total percentage
	float			MaxValue;			// Highest graph value (if flag set)
	float			Minimum;			// Lowest
	float			Maximum;			// Highest
	float			Total;				// Total (for average)

	float*			History;			// Last 512 values (for graphing)
	bool			BlockOwner;			// Do I own the whole statistic history block?

	char			Name[1];			// Name of statistic

} Stat;


const int StatsInBlock=32;				// Number of stats in a single block

__inline float GetHistory( Stat* pStat, int GraphHead )
{
	return pStat->History[GraphHead*StatsInBlock];
}
__inline void SetHistory( Stat* pStat, int GraphHead, float Value )
{
	pStat->History[GraphHead*StatsInBlock]=Value;
}


extern Stat*	pStatistics;		// Pointer to chain of statistics
extern DWORD	GraphHead;			// Pointer to current element in statistics History
extern DWORD	NumberStatistics;	// Entries in statistic list
extern Stat* GraphsActive[20];		// Number of frame graphs active
extern DWORD NumberGraphsActive;



void UpdateGraphs();
void InitStatistics();
void DestroyStatistics();
void UpdateStatistics();
void ResetStatistics();

#ifdef LAB_ONLY
void SaveStatistics();
void RestoreStatistics();
#endif



