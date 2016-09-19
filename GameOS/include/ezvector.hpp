//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

class EZVector
{
public:
	union
	{
		float m[3];
		struct
		{
			float x;
			float y;
			float z;
		};
	};

public:
	EZVector();
	EZVector(float, float, float);
	EZVector(EZVector * v);
	~EZVector();
	void Normalize(EZVector * );
	void Add(EZVector *);
	static float Dot(EZVector *, EZVector *);
	static void Cross(EZVector *, EZVector *, EZVector *);
};