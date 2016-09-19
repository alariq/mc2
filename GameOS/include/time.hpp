#pragma once
//===========================================================================//
// File:	Time.cpp                                                         //
// Contents: Time Function													 //												 
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//


void TimeManagerInstall();
void StartTimeManager();
void TimeManagerUpdate();
void TimeManagerUninstall();
void StartPauseTime();
void EndPauseTime();

extern DWORD WatchDogFrame;
extern DWORD CurrentFrameNumber;
extern __int64 StartCount;
extern __int64 frequency;
extern __int64 StartOfRenderTime;
