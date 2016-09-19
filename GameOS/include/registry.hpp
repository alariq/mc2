#pragma once
//==========================================================================//
// File:	 Registry.cpp													//
// Contents: Registry routines												//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"string.hpp"

void RegistryManagerInstall();
void RegistryManagerUninstall();
char* ReadRegistry( char* KeyName, char* ValueName, bool LocalMachine );
char* ReadRegistryHKCU( char* KeyName, char* ValueName, bool LocalMachine );


extern DWORD UpdatedExe;					// Has the exe been updated since last run?


