//***************************************************************************
//
//	err.cpp -- Error Handling routines
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//--------------
// Include Files
#include<stdio.h>
#include<gameos.hpp>

//***************************************************************************
// Error Handling routines
//***************************************************************************

void Fatal (long errCode, const char* errMessage) {

	char msg[512];
	sprintf(msg, " [FATAL %d] %s ", errCode, errMessage);
	STOP((msg));
}

//---------------------------------------------------------------------------

void Assert (bool test, long errCode, const char* errMessage) {

	if (!test) {
		char msg[512];
		sprintf(msg, " [ASSERT %d] %s ", errCode, errMessage);
		STOP((msg));
	}
}

//***************************************************************************
