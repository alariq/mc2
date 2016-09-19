//---------------------------------------------------------------------------
// sounds.h - This file is the list of available sound bites
//
// MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef SOUNDS_H
#define SOUNDS_h
//---------------------------------------------------------------------------
// Macro Definitions

#define BOMB_EXPLODE		0		//Should never be used
#define ER_LASER_FIRE		1		//ER Laser Fire
#define BIGDEATH			2		//Mech Explosion
#define LASERHITS			3		//All other Laser Fire/Hit
#define MECH_COLLIDE		4		//Mech Collisions
#define LRM_HIT				5		//LRM Hit
#define FLAMER_FIRE			6		//Flamer Fire
#define AC2_FIRE			7		//AutoCannon Fire
#define LRM_FIRE			8		//Lrm Fire
#define SRM_HIT				9		//SRM Hit
#define PPC_FIRE			10		//PPC Fire
#define SRM_FIRE			11		//SRM Fire
#define GAUSS_FIRE			12		//Gauss Rifle Fire
#define FOOTFALL_SMALL		13		//Mech Footstep
#define TREEFALL			14		//TreeFall
#define BUTTON5				15		//GUI SFX
#define BUTTON7				16		//GUI SFX
#define BUTTON6				17		//GUI SFX
#define BUZZER1				18		//GUI SFX
#define CRITICAL_HIT_SFX	19		//Mech Critical Hit
#define PING_SFX			20		//Sensor Contact 
#define MGUN_FIRE			21		//Machine Gun Fire
#define OBJECT_FIRE			22		//Burning Fire
#define ER_LASER_HIT		23		//ER Laser Hit
#define GAUSS_HIT			24		//Gauss Rifle Hit
#define AC2_HIT				25		//AutoCannon Hit
#define MGUN_HIT			26		//Machine Gun Hit
#define EJECT_SFX			27		//Eject
#define TREEFALL1			28		//Treefall
#define MECH_FALL_SOUND		29		//Mech Fall Sound
#define ELEMENTAL_HIT		30		//Elemental Shredding a Mech Sound
#define GENERAL_ALARM_SFX	34		//Perimeter alarm general alarm
#define BUILDING_EXPLODE	35
#define	ELECTRIC_HUM		36
#define	GENERATOR_HUM		37
#define	RADAR_HUM			38
#define	HEAVY_ROTATION		39
#define	ORBITAL_GUN			40
#define	DEFENSE_SILO		41
#define	GATE_OPEN			42
#define	TRAIN_XING			43
#define HPG_ANTENNA			44
#define	GATE_CLOSE			42
#define INVALID_GUI			46
#define ZOOM_GUI			47
#define FORCE_GUI			48
#define SENSOR_GUI			49
#define MAPPLACE_GUI		50
#define MAPBUTTONS_GUI		51
#define CLICKOFF_GUI		52
#define CLICKON_GUI			53
#define	FOLDER_GUI			54
#define OUTSIDESELECT_GUI	55
#define PAGEGUI				56
#define SELECT_GUI			57
#define STARTMISSION_GUI	58
#define TAB_GUI				59
#define POWERDOWN_SFX		60
#define POWERUP_SFX			61
#define INCOMING_ARTILLIRY	62
#define INCOMING_AIRSTRIKE	63
#define WINDOW_OPEN			64
#define WINDOW_CLOSE		65
#define START_GROUPSELECT	66
#define END_GROUPSELECT		67
#define ZOOMIN_GUI			68
#define ZOOMOUT_GUI			69
#define WRONG_GUI			70
#define FRP_BUTTON_GUI		71
#define BREAKINGFENCE		72
#define BUILDING_COLLAPSE	73
#define VTOL_ANIMATE		74
#define VTOL_DROP			75
#define JUMPJETS			76
#define THUNDER1			77
#define THUNDER2			78
#define THUNDER3			79
#define THUNDER4			80
#define RAIN1				81
#define RAIN2				82
#define RAIN3				83
#define RAIN4				84
#define SENSOR1				85
#define SENSOR2				86
#define SENSOR3				87
#define SENSOR4				88
#define LOG_WRONGBUTTON		89
#define LOG_VIDEOBUTTONS	90
#define LOG_STARTMISSION	91
#define LOG_SELECT			90	// changing to be the same as video buttons
#define LOG_NEXTBACKBUTTONS 93	
#define LOG_MISSIONSELECT	94
#define LOG_MAINMENUBUTTON	95
#define LOG_HIGHLIGHTBUTTONS 96
#define LOG_EXITGAME		97
#define LOG_CLICKONBUTTON	98
#define MINE_EXPLOSION		99
#define LOG_DIGITALHIGHLIGHT	96
#define REPAIRBAY_FX		101
#define LOG_PILOTMEDAL		102
#define LOG_KILLMARKER		103
#define LOG_PROMOTED		104
#define MECH_EXPLODE		105
#define COPTER_POWERUP		106
#define COPTER_POWERDN		107
#define VEHICLE_POWERUP		108
#define VEHICLE_POWERDN		109
#define LOAD_DOORS_CLOSING	110
#define LOAD_DOORS_OPENING	111
#define LOG_TEAM_CHAT		112
#define LOG_CHAT			113
#define HELICOPTER_DEATH	114
#define LOG_UNREADY			16
#define SALVAGE_CRAFT		116

//---------------------------------------------------------------------------

#define MISSION_START_0			0		//Mission Start Tune
#define MISSION_START_1			1		//Mission Start Tune
#define MISSION_START_2			2		//Mission Start Tune
#define MISSION_START_3			3		//Mission Start Tune
#define MISSION_START_4			4		//Mission Start Tune
#define NUM_MISSION_TUNES		5

#define CONTACT_TUNE_0			5
#define CONTACT_TUNE_1			6
#define CONTACT_TUNE_2			7
#define NUM_CONTACT_TUNES		3

#define WIN_TUNE_0				8
#define LOSE_TUNE_0				9

#define OBJECTIVE_SUCCESS		10
#define OBJECTIVE_FAILURE		11

#define ENEMY_DONE				12
#define FRIENDLY_DONE			13

#define COMBAT_TUNE_0			14		//Combat Tune
#define COMBAT_TUNE_1			15		//Combat Tune
#define COMBAT_TUNE_2			16		//Combat Tune
#define COMBAT_TUNE_3			17		//Combat Tune
#define COMBAT_TUNE_4			18		//Combat Tune
#define COMBAT_TUNE_5			19		//Combat Tune
#define NUM_COMBAT_TUNES		6

#define GAME_FINALE				20		//Played when no other music is playing
#define BACKGROUND_AMBIENT		21		//Played when no other music is playing

#define LOGISTICS_LOOP			22
#define CREDITS_TUNE			23

#define RESULTS_LOOP			28

//---------------------------------------------------------------------------
#define BETTY_DROP_WEIGHT_EXCEEDED	0
#define BETTY_AIR_SUPERIORITY		1
#define BETTY_COMPONENTS_CAPTURED	2
#define BETTY_CANNOT_COMP_OBJ		3
#define BETTY_OBJECTIVE_COMPLETE	4
#define BETTY_NEW_LEADER			5
#define BETTY_RESOURCE_CAPTURED		6
#define BETTY_THIRTY_LEFT			7
#define BETTY_TWO_MIN_LEFT			8
#define BETTY_PERIMETER_BREACH		9
#define BETTY_LOW_DROP_WEIGHT		10
#define BETTY_MISSION_LOST			11
#define BETTY_PRI_OBJ_1_WON			12
#define BETTY_PRI_OBJ_2_WON			13
#define BETTY_PRI_OBJ_3_WON			14
#define BETTY_SEC_OBJ_1_WON			15
#define BETTY_SEC_OBJ_2_WON			16
#define BETTY_SEC_OBJ_3_WON			17
#define BETTY_MISSION_WON			18
#define BETTY_DEPLOY_MSG			19
#define BETTY_REPAIR_COMPLETE		20
#define BETTY_REPAIR_INCOMPLETE		21
#define BETTY_ARTILLERY_AVAILABLE	22
#define BETTY_SENSOR_PROBE_AVAIL	23
#define BETTY_CAMERA_DRONE_AVAIL	24
#define BETTY_NEW_CAMPAIGN			25
#define BETTY_OP_VIDEO				26
#define BETTY_BAD_FORCE_SELECTION	27
#define BETTY_NEW_EQUIPMENT			28
#define BETTY_NEW_COMPONENT			29
#define BETTY_NEW_MECH				30
#define BETTY_NEW_PILOT				31
#define BETTY_NEW_MECHWARRIOR		32
#define BETTY_NEW_VEHICLE			33
#define BETTY_SENS_TWR_CONTACT		34
#define BETTY_SENS_TWR_DESTROYED	35
#define BETTY_PERIMETER_ALARM_DEST	36
#define BETTY_SECOND_OBJ_NO_COMP	37
#define BETTY_POWERUP				38
#define BETTY_RESOURCES				39
#define BETTY_GUARDATTACK			40
#define BETTY_GUARDDESSTROYED		41
#define BETTY_PROMOREG				42
#define BETTY_PROMOVET				43
#define BETTY_PROMOELI				44
#define BETTY_PROMOACE				45
#define BETTY_NEW_MECHS				46
#define BETTY_NEW_PILOTS			47	
#define BETTY_NEW_WEAPONS			48
#define BETTY_BUILDING_RECAPTURED	49
#define BETTY_GENERATOR_DESTROYED	50
#define BETTY_REPAIR_GONE			51
#define BETTY_MINES_GONE			52
#define BETTY_SENSOR_CAPTURED		53


#define MAX_BETTY_SAMPLES			54

//---------------------------------------------------------------------------
#define SUPPORT_AIRSTRIKE			0
#define SUPPORT_MINELAYER			1
#define SUPPORT_PROBE				2
#define SUPPORT_RECOVER				3
#define SUPPORT_REPAIR				4
#define SUPPORT_SCOUT				5
#define SUPPORT_ARTILLERY			6

#define MAX_SUPPORT_SAMPLES			7

//---------------------------------------------------------------------------
#define ACKNOWLEDGE				2
#define CALLSIGN				10
//---------------------------------------------------------------------------
#endif
