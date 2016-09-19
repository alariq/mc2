//---------------------------------------------------------------------------
//
//	DObjNum.h -- File contains the Basic Game Object Type definitions
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DOBJNUM_H
#define DOBJNUM_H
//---------------------------------------------------------------------------
// Include Files
	
//---------------------------------------------------------------------------
// Macro Definitions
// These are very important.  They are the basis that both the data files and
// game code use to determine who is what kind of thing.  This file must be
// kept current to both data and code.
#define INVALID_OBJECT			-1

#define BASETHORMECH			0x1
#define BASELOKIMECH			0x2
#define BASEAWESOMEMECH			0x3
#define RED_LASER				0x4
#define GREEN_LASER				0x5
//#define EXPLOSION				0x6

#define BETULA					0x7
#define PINE					0x8
#define PALM					0x9

#define REAL_FARM_BUILDING		0xA

#define WHITE_SMOKE_GEN			0xB
#define BLACK_SMOKE_GEN			0xC

#define BULLET_GEN				0xD
#define MISSILE_GEN				0xE

#define BASEULLERMECH			0xF
#define BASEHUNCHBACKMECH		0x10
#define BASECOUGARMECH			0x11
#define BASECATAPULTMECH		0x12

#define BARNA_A1				0x13
#define BARNB_A1				0x14
#define BARNC_A1				0x15
#define BARND_A1				0x16
#define COMM_A1					0x17
#define FRMHSA_A1				0x18
#define FRMHSB_A1				0x19
#define FRMHSD_A1				0x1A
#define GRNSTR_A1				0x1B
#define HBGEODESIC_A1			0x1C
#define HPMPB_A1				0x1D
#define HVINDYB_A1				0x1E
#define INDYPLANT_A1			0x1F
#define LTINDYSIMPLE_A1			0x20
#define PWRPLT_A1				0x21
#define SILOB_A1				0x22
#define STGBOX_A1				0x23
#define STRA_A1					0x24
#define TOMSILO_A1				0x25
#define TRNLDSILOS_A1			0x26

#define LASERHIT				0x27
#define AC20HIT					0x28
#define MECHDESTRUCT			0x29
#define BUILDBOOM				0x2A

#define HBSTRIKER				0x2B

#define BASECOMMANDOMECH		0x2C
#define BASECOMMANDO2CMECH		0x2D
#define BASEFIRESTARTERMECH		0x2E
#define BASEHOLLANDERMECH		0x2F
#define BASERAVENMECH			0x30
#define BASECENTURIONMECH		0x31
#define BASEJAGERMECHMECH		0x32
#define BASEATLASMECH			0x33
#define BASEMADCATMECH			0x34
#define BASEMASAKARIMECH		0x35
#define BASEVULTUREMECH			0x36
#define BASEPUMAMECH			0x37

#define THORLARM				0x38
#define THORRARM				0x39
#define THORDEST				0x3A

#define WAYPNT_ICON				0x3B
#define MINE_ICON				0x3C
#define MOVE_ICON				0x3D
#define ARTLRY_ICON				0x3E
#define CRITICAL_HIT			0x3F

#define BASEFIRESTARTER2MECH	0x40
#define BASECENTURTION2MECH		0x41

#define DAMAGE_SMOKE			0x42
#define PPC_FX					0x43

#define BARNA_A2				0x44
#define BARNB_A2				0x45
#define BARNC_A2				0x46
#define BARND_A2				0x47
#define BARND_A3				0x48
#define BARND_A4				0x49
#define COMM_A2					0x4A
#define COMM_A3					0x4B
#define COMM_A4					0x4C
#define FRMHSA_A2				0x4D
#define FRMHSB_A2				0x4E
#define GRNSTR_A2				0x4F
#define HBBARRACK_A1			0x50
#define HBBARRACK_A2			0x51
#define HBBARRACK_A3			0x52
#define HBBARRACK_A4			0x53
#define HBGEODESIC_A2			0x54
#define HBMPERM1_A1				0x55
#define HBMPERM1_A2				0x56
#define HBMPERM1_A3				0x57
#define HBMPERM1_A4				0x58
#define HBMPERM2_A1				0x59
#define HBMPERM2_A2				0x5A
#define HBMPERM2_A3				0x5B
#define HBMPERM2_A4				0x5C
#define HBTRKBNK_A1				0x5D
#define HBTRKBNK_A2				0x5E
#define HBTRKBNK_A3				0x5F
#define HBTRKBNK_A4				0x60
#define HBTRKBUNKER_A1			0x61
#define HBTRKBUNKER_A2			0x62
#define HBTRKBUNKER_A3			0x63
#define HBTRKBUNKER_A4			0x64
#define HBWAREHSEA_A1			0x65
#define HBWAREHSEA_A2			0x66
#define HBWAREHSEA_A3			0x67
#define HBWAREHSEA_A4			0x68
#define HPMPA_A2				0x69
#define HPMPA_A1				0x6A
#define HPMPB_A2				0x6B
#define HPMPB_A3				0x6C
#define HPMPB_A4				0x6D
#define HVINDYA_A1				0x6E
#define HVINDYA_A2				0x6F
#define HVINDYA_A3				0x70
#define HVINDYA_A4				0x71
//#define HVINDYB_A1				0x72
#define HVINDYB_A2				0x73
#define HVINDYB_A3				0x74
#define HVINDYB_A4				0x75
#define HVINDYC_A1				0x76
#define HVINDYC_A2				0x77
#define HVINDYC_A3				0x78
#define HVINDYC_A4				0x79
#define HVINDYD_A1				0x7A
#define HVINDYD_A2				0x7B
#define INDYPLANT_A2			0x7C
#define LTINDYSIMPLE_A2			0x7D
#define PWRPLT_A2				0x7E
#define PWRPLT_A3				0x7F
#define PWRPLT_A4				0x80
#define SILOB_A2				0x81
#define STGBOX_A2				0x82
#define STGBOX_A3				0x83
#define STGBOX_A4				0x84
#define STRA_A2					0x85
#define TRNLDSILOS_A2			0x86
#define TRNLDSILOS_A3			0x87
#define TRNLDSILOS_A4			0x88
#define INDYHOUSE_A1			0x89
#define INDYHOUSE_A2			0x8A
#define TOMSILO_A2				0x8B
#define WTRTWR_A1				0x8C
#define WTRTWR_A2				0x8D
#define WTRCANS_A1				0x8E
#define WTRCANS_A2				0x8F
#define URBANA_A1				0x90
#define URBANA_A2				0x91
#define URBANA_A3				0x92
#define URBANA_A4				0x93
#define URBANB_A1				0x94
#define URBANB_A2				0x95
#define URBANB_A3				0x96
#define URBANB_A4				0x97
#define URBANE_A1				0x98
#define URBANE_A2				0x99
#define URBANE_A3				0x9A
#define URBANE_A4				0x9B
#define FRMHSD_A2				0x9C

#define ATLARM					0x9d
#define ATRARM					0x9e
#define CMLARM					0x9f
#define CMRARM					0xa0
#define CNLARM					0xa1
#define CNRARM					0xa2
#define FSLARM					0xa3
#define FSRARM					0xa4
#define KLARM					0xa5
#define KRARM					0xa6
#define MCLARM					0xa7
#define MCRARM					0xa8
#define PLARM					0xa9
#define PRARM					0xaa
#define AWLARM					0xab
#define AWRARM					0xac
#define CGLARM					0xad
#define CGRARM					0xae
#define HBLARM					0xaf
#define HBRARM					0xb0
#define JMLARM					0xb1
#define JMRARM					0xb2
#define LKLARM					0xb3
#define LKRARM					0xb4
#define ULLARM					0xb5
#define ULRARM					0xb6

#define	BIG_ARTLRY				0xf8	// 248
#define	SMALL_ARTLRY			0xf9	// 249
#define	SENSOR_ARTLRY			0xfa	// 250

#define BIG_STRIKE				0x1fb	// 507
#define SMALL_STRIKE			0x1fc	// 508
#define SENSOR_STRIKE			0x1fd	// 509

#define CAMERA_CALL				0x204	// 516
#define CAMERA_DRONE			0x205	// 517

//---------------------------------------------------------------------------
#endif
