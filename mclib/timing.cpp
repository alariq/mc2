//******************************************************************************************
//	timing.cpp - This file contains the declarations for the timing variables
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------------------------
// Include Files
#ifndef TIMING_H
#include"timing.h"
#endif

#include<windows.h>
#include<winbase.h>

#include<math.h>

//----------------------------------------------------------------------------------
long	turn = 0;
float	frameLength = 0.05f;
float	scenarioTime = 0.0;
DWORD	LastTimeGetTime = 0;
bool dynamicFrameTiming = TRUE;

const char *monthName[12] = 
{
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

//----------------------------------------------------------------------------------
DWORD MCTiming_GetTimeZoneInforation(void *timeData)
{
	// Get Time Zone information for this machine to calculate
	// Astronomy correctly.
	DWORD daylightSavingsInfo = GetTimeZoneInformation((TIME_ZONE_INFORMATION *)timeData);
	return daylightSavingsInfo;
}

//----------------------------------------------------------------------------------
DWORD MCTiming_GetTimeZoneInformationSize (void)
{
	return sizeof(TIME_ZONE_INFORMATION);
}

//----------------------------------------------------------------------------------
void MC_SYSTEMTIME::copyFromSystemTime (void *systemTime)
{
	SYSTEMTIME *sysTime = (SYSTEMTIME *)systemTime;
	
	dwYear		   =	sysTime->wYear;
	dwMonth		   =	sysTime->wMonth;
	dwDayOfWeek	   =	sysTime->wDayOfWeek;
	dwDay		   =	sysTime->wDay;
	dwHour		   =	sysTime->wHour;
	dwMinute	   =	sysTime->wMinute;
	dwSecond	   =	sysTime->wSecond;
	dwMilliseconds =	sysTime->wMilliseconds;
}

//----------------------------------------------------------------------------------
void MCTiming_GetUTCSystemTimeFromInformation(DWORD daylightInfo, void *timeData, MC_SYSTEMTIME *systemTime)
{
	TIME_ZONE_INFORMATION *tzInfo = (TIME_ZONE_INFORMATION *)timeData;

	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);

	long bias = tzInfo->Bias;
	if (daylightInfo == TIME_ZONE_ID_STANDARD)
	{
		bias += tzInfo->StandardBias;
	}
	else if (daylightInfo == TIME_ZONE_ID_DAYLIGHT)
	{
		bias += tzInfo->DaylightBias;
	}
	else		//Assume Standard
	{
		bias += tzInfo->StandardBias;
	}

	systemTime->copyFromSystemTime(&(sysTime));
}

//----------------------------------------------------------------------------------
