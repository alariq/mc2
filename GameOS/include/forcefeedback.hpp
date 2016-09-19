#pragma once
//===========================================================================//
// File:	 ForceFeedback.hpp												 //
// Contents: ForceFeedback routines											 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#define GROUP_FORCEFEEDBACK "Group_ForceFeedback"

struct FFEffectData
{
	GUID diGUID;
	DIEFFECT diEffectData;
	LPDIRECTINPUTEFFECT diEffect;
	UCHAR diTypeSpecific[64];
	DIENVELOPE diEnvelope;
	DWORD rgdwAxes[12];
	LONG rglDirection[12];
	FFEffectData * pNextEffect;
};

struct gosForceEffect
{
public:
	FFEffectData * ffEffectData;
	gosForce m_eType;
	gosForceEffect * pNext;
	DWORD dwStick;
	double longestDuration;
	double startTime;
	bool bStatus;
	bool fromFile;
public:
	gosForceEffect(gosJoystick_ForceEffect * fe, DWORD joy);
	gosForceEffect(const char * fe, DWORD joy);
	void ReCreate();
	~gosForceEffect();
	void Play(DWORD times);
	void Stop();
	void Update(gosJoystick_ForceEffect * fe);
};