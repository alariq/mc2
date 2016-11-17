//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MATHFUNC_H
#define MATHFUNC_H

#ifndef STUFF_STUFF_HPP
#include<stuff/stuff.hpp>
#endif

// DESCRIBES A COORDINATE SYSTEM, OR FRAME OR REFERENCE
class frameOfRef
{
public:
	Stuff::Vector3D i;
	Stuff::Vector3D j;
	Stuff::Vector3D k;

	frameOfRef(void);

	frameOfRef (frameOfRef &copy)
	{
		*this = copy;
	}

	frameOfRef & set_i (Stuff::Vector3D &new_i)
	{
		i = new_i;
		return *this;
	}

	frameOfRef & set_j (Stuff::Vector3D &new_j)
	{
		j = new_j;
		return *this;
	}

	frameOfRef & set_k (Stuff::Vector3D &new_k)
	{
		k = new_k;
		return *this;
	}

	frameOfRef & operator =(frameOfRef &fr);

	frameOfRef & orthonormalize ()
	{
		return orthonormalize_on_yaxis();
	}

	frameOfRef & orthonormalize_on_xaxis ();

	frameOfRef & orthonormalize_on_yaxis ();

	frameOfRef & orthonormalize_on_zaxis ();

	frameOfRef & reset_to_world_frame ();

	frameOfRef & rotate_about_i (float &angle);
	frameOfRef & rotate_about_j (float &angle);
	frameOfRef & rotate_about_k (float &angle);

	frameOfRef & rotate_about_i_orthonormal (float &angle);
	frameOfRef & rotate_about_j_orthonormal (float &angle);
	frameOfRef & rotate_about_k_orthonormal (float &angle);

	frameOfRef & rotate(float &i_angle,
						float &j_angle,
						float &k_angle);

	frameOfRef & rotate(Stuff::Vector3D &rotation_vector);

	frameOfRef & trans_to_frame (frameOfRef &new_frame);
	frameOfRef & trans_from_frame (frameOfRef &old_frame);
	
	void trans_to_frame (Stuff::Vector3D &vector)
	{
		float vx = vector.x;
		float vy = vector.y;
		float vz = vector.z;
		
		vector.x = i.x*vx + i.y*vy + i.z*vz;
		vector.y = j.x*vx + j.y*vy + j.z*vz;
		vector.z = k.x*vx + k.y*vy + k.z*vz;
	}	
};

void Rotate (Stuff::Vector2DOf<float> &vec, float angle);

void Rotate (Stuff::Vector3D &vec, float angle);

void RotateLight (Stuff::Vector3D &vec, float angle);

void OppRotate (Stuff::Vector3D &vec, float angle);

float distance_from (Stuff::Vector3D &v1, Stuff::Vector3D &v2);

float my_acos (float val);

float angle_from (Stuff::Vector2DOf<float>&v1, Stuff::Vector2DOf<float> &v2);

float angle_from (Stuff::Vector3D &v1, Stuff::Vector3D &v2);

float world_angle_between (Stuff::Vector3D &v1, Stuff::Vector3D &v2);

Stuff::Vector3D relativePositionToPoint (Stuff::Vector3D point, float angle, float distance, unsigned long flags);

long RandomNumber (long range);

long SignedRandomNumber (long range);

bool RollDice (long percent);

inline long float2long (float val)
{
	//_ftol TRUNCS not rounds.  Processor wants to round.  Surely there is some flag to not have this happen?
	// There is but BOY is it slow.  We will try Andy's Magical formula instead.
	// Doesn't work either.  Major bug in Intel's FPU.
	// Will simply call long here now to insure working ok and address later.
	
	long result = long(val);
	return result;
	
#if 0
	float point5 = 0.49f;
	if (val < 0.0f)
	{
		__asm
		{
			fld		val
			fadd	point5
			fistp	result
		}
	}
	else
	{
		__asm
		{
			fld		val
			fsub	point5
			fistp	result
		}
	}

	return result;

	if (val > 0.0f)
		val -= 0.49999999f;
	else
		val += 0.49999999f;

	val += 3 << 22;
	return ((*(long*)&val)&0x007fffff) - 0x00400000;
#endif
}

//---------------------------------------------------------------------------
inline float mc2_atan2 (float f1, float f2)
{
	//Return atan of f1/f2;
	float result = 1.570796f;
	//f2 is always assumed positive here!!!
	if (f2 > Stuff::SMALL)
	{
#ifndef LINUX_BUILD
		__asm
		{
			fld f1
			fld f2
			fpatan
			fabs
			fstp result
		}
#else
        result = atan2(f1, f2);
#endif
	}
	
	return result;
}

// sebi: implemented in math.h
#if 0
//---------------------------------------------------------------------------
inline float fmax (float f1, float f2)
{
	if (f1 > f2)
		return f1;
	else
		return f2;
}

//---------------------------------------------------------------------------
inline float fmin (float f1, float f2)
{
	if (f1 < f2)
		return f1;
	else
		return f2;
}
#endif

#ifndef max
template<typename T> T max(const T& a, const T& b) { return a > b ? a : b; }
#endif
#ifndef min
template<typename T> T min(const T& a, const T& b) { return a < b ? a : b; }
#endif

//---------------------------------------------------------------------------
inline float sign (float f1)
{
	if (f1 < 0.0f)
		return -1.0f;
		
	return 0.0f;
}
#endif
