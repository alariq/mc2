//******************************************************************************************
//
//	dradio.h -- File contains the Radio Definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DRADIO_H
#define DRADIO_H

class Radio;
typedef Radio *RadioPtr;

#define	MSG_TOTAL_MSGS 0

enum RadioMessageType {
	RADIO_ACK = MSG_TOTAL_MSGS,			//3 Types
	RADIO_CALLED_SHOT,
	RADIO_CANNOT_CAPTURE,
	RADIO_CAPTURED_BUILDING,
	RADIO_CAPTURED_VEHICLE,
	RADIO_REFIT_INCOMPLETE,
	RADIO_REFIT_DONE,
	RADIO_OBJECT_DEAD,
	RADIO_BUILDING_DEAD,
	RADIO_VEHICLE_DEAD,					//2 Types
	RADIO_MECH_DEAD,					//2 Types
	RADIO_MECH_SALVAGE,					//2 Types
	RADIO_SENSOR_CONTACT,				//2 Types
	RADIO_HITTING_MINES,				
	RADIO_UNDER_ATTACK,					//2 Types
	RADIO_UNDER_AIRSTRIKE,
	RADIO_MOVE_BLOCKED,
	RADIO_ILLEGAL_ORDER,
	RADIO_EJECTING,
	RADIO_DEATH,
	RADIO_CRIPPLED,
	RADIO_PILOT_HURT,
	RADIO_WEAPON_DOWN,
	RADIO_WEAPONS_OUT,
	RADIO_AMMO_OUT,
	RADIO_TAUNT,	 					//3 Types
	RADIO_GUARD,
	RADIO_TARGET_ACK,					//2 Types
	RADIO_CALLSIGN,						//2 TYpes
	RADIO_AIRSTRIKE,
	RADIO_ARMORHOLE,					//2 Types
	RADIO_MESSAGE_COUNT
};

enum RadioNoise
{
	SHORT_STATIC,
	LONG_STATIC,
	CLICK
};

//******************************************************************************************
#endif

