//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRCLIPTRICK_HPP

#if !defined(STUFF_VECTOR4D_HPP)
#include<stuff/vector4d.hpp>
#endif

//	defined in MLRPrimitiveBase.cpp
extern int clipTrick[6][2];

inline void
	DoClipTrick(Vector4D& v, int ct)
	{
		v[clipTrick[ct][0]] = clipTrick[ct][1] ? v.w-SMALL : SMALL;
	}

inline void
	DoCleanClipTrick(Vector4D& v, int ct)
	{
		v[clipTrick[ct][0]] = clipTrick[ct][1] ? v.w : 0.0f;
	}
