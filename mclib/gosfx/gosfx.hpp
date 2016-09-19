//===========================================================================//
// File:	MLRStuff.hpp                                                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include<stuff/stuff.hpp>

namespace gosFX
{

	//
	//--------------
	// gosFX classes
	//--------------
	//
	enum {
		EffectClassID = Stuff::FirstgosFXClassID,
			ParticleCloudClassID,
				PointCloudClassID,
				SpinningCloudClassID,
					ShardCloudClassID,
					PertCloudClassID,
					CardCloudClassID,
					ShapeCloudClassID,
					EffectCloudClassID,
			SingletonClassID,
				CardClassID,
				ShapeClassID,
			TubeClassID,
			DebrisCloudClassID,
			PointLightClassID,
		FirstFreegosFXClassID,
	};

	enum {CurrentGFXVersion = 17};

	int
		ReadGFXVersion(Stuff::MemoryStream *erf_stream);
	void
		WriteGFXVersion(Stuff::MemoryStream *erf_stream);

	void InitializeClasses();
	void TerminateClasses();

	extern HGOSHEAP Heap;

	extern const Stuff::LinearMatrix4D &Effect_Into_Motion;
	extern const Stuff::LinearMatrix4D &Effect_Against_Motion;

	DECLARE_TIMER(extern, Animation_Time);
	DECLARE_TIMER(extern, Draw_Time);
	extern DWORD Point_Count;
	extern DWORD Shard_Count;
	extern DWORD Pert_Count;
	extern DWORD Card_Count;
	extern DWORD Shape_Count;
	extern DWORD Profile_Count;
}

#include"fcurve.hpp"
