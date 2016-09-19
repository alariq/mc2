//===========================================================================//
// File:	rotation.cc                                                      //
// Contents: Implementation details for rotation classes                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ EulerAngles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const EulerAngles
	EulerAngles::Identity(0.0f,0.0f,0.0f);



bool UseFastLerp = true;
bool UseFastNormalize = true;



static bool __stdcall Check_UseFastLerp() {return UseFastLerp == true;}
static bool __stdcall Check_UseFastNormalize() {return UseFastNormalize == true;}


static void __stdcall Activate_UseFastLerp() {UseFastLerp = !UseFastLerp;}
static void __stdcall Activate_UseFastNormalize() {UseFastNormalize = !UseFastNormalize;}



//
//#############################################################################
//#############################################################################
//
EulerAngles&
	EulerAngles::operator=(const YawPitchRoll &angles)
{
	Check_Pointer(this);
	Check_Object(&angles);

	LinearMatrix4D m;
	m.BuildRotation(angles);
	*this = m;

	return *this;
}

//
//#############################################################################
//#############################################################################
//
EulerAngles&
	EulerAngles::operator=(const UnitQuaternion &quaternion)
{
	Check_Pointer(this);
	Check_Object(&quaternion);

	LinearMatrix4D m;
	m.BuildRotation(quaternion);
	return *this = m;
}

//
//#############################################################################
//#############################################################################
//
EulerAngles&
	EulerAngles::operator=(const LinearMatrix4D &matrix)
{
	Check_Pointer(this);
	Check_Object(&matrix);

	Verify(
		Vector3D::Forward.z == 1.0f && Vector3D::Right.x == -1.0f && Vector3D::Up.y == 1.0f
		 || Vector3D::Forward.z == -1.0f && Vector3D::Right.x == 1.0f && Vector3D::Up.y == 1.0f
	);

	SinCosPair
		p,y,r;

	//
	//-------------------------------------------------
	// First deal with the singularity of 90 degree yaw
	//-------------------------------------------------
	//
	y.sine = -matrix(0,2);
	if (Close_Enough(y.sine,1.0f,0.0001f))
	{
		p.sine = matrix(1,0);
		p.cosine = matrix(2,0);
		pitch = p;
		yaw = Pi_Over_2;
		roll = 0.0f;
		return *this;
	}

	//
	//-----------------------------
	// Now deal with -90 degree yaw
	//-----------------------------
	//
	else if (Close_Enough(y.sine,-1.0f,0.0001f))
	{
		p.sine = -matrix(1,0);
		p.cosine = -matrix(2,0);
		pitch = p;
		yaw = -Pi_Over_2;
		roll = 0.0f;
		return *this;
	}

	//
	//-------------------------------------------------------------------------
	// Otherwise, assume that pitch must be constrained between +/- 90 degrees.
	// This is particularly complex in this case, because pitch is the primary
	// axis.  So, we will set the yaw cosine to the appropriate sign that
	// results in the cosine of pitch being positive.
	//-------------------------------------------------------------------------
	//
	else
	{
		y.cosine = Sqrt(1.0f - y.sine*y.sine);

		Scalar one_y_cosine = 1.0f/ y.cosine;

		p.cosine = matrix(2,2) * one_y_cosine;

		if (p.cosine < 0.0f)
		{
			p.cosine = -p.cosine;
			y.cosine = -y.cosine;
			one_y_cosine = -one_y_cosine;
		}
		p.sine = matrix(1,2) * one_y_cosine;
		r.sine = matrix(0,1) * one_y_cosine;
		r.cosine = matrix(0,0) * one_y_cosine;
		#if defined(_ARMOR)
			Scalar temp = p.sine*y.sine*r.cosine - p.cosine*r.sine;
			Verify(Close_Enough(temp, matrix(1,0), 5e-3f));
		#endif
	}

	pitch = p;
	yaw = y;
	roll = r;
	return *this;
}

//
//#############################################################################
//#############################################################################
//
bool
	Stuff::Small_Enough(
		const EulerAngles& angles,
		Scalar e
	)
{
	Check_Object(&angles);

	return
		Small_Enough(angles.pitch,e)
		 && Small_Enough(angles.yaw,e)
		 && Small_Enough(angles.roll,e);
}

//
//#############################################################################
//#############################################################################
//
bool
	Stuff::Close_Enough(
		const EulerAngles& a1,
		const EulerAngles& a2,
		Scalar e
	)
{
	Check_Object(&a1);
	Check_Object(&a2);

	return
		Close_Enough(a1.pitch,a2.pitch,e)
		 && Close_Enough(a1.yaw,a2.yaw,e)
		 && Close_Enough(a1.roll,a2.roll,e);
}

//
//#############################################################################
//#############################################################################
//
EulerAngles&
	EulerAngles::Lerp(
		const EulerAngles &a1,
		const EulerAngles &a2,
		Scalar t
	)
{
	Check_Pointer(this);
	Check_Object(&a1);
	Check_Object(&a2);

	pitch = Stuff::Lerp(a1.pitch,a2.pitch,t);
	yaw = Stuff::Lerp(a1.yaw,a2.yaw,t);
	roll = Stuff::Lerp(a1.roll,a2.roll,t);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
EulerAngles&
	EulerAngles::Normalize()
{
	Check_Pointer(this);

	pitch.Normalize();
	yaw.Normalize();
	roll.Normalize();
	return *this;
}

//
//#############################################################################
//#############################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const EulerAngles &angle
		)
	{
		Check_Object(&angle);
		SPEW((group, "<+"));
		Spew(group, angle.pitch);
		SPEW((group, ",+"));
		Spew(group, angle.yaw);
		SPEW((group, ",+"));
		Spew(group, angle.roll);
		SPEW((group, ">+"));
	}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ YawPitchRoll ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const YawPitchRoll
	YawPitchRoll::Identity(0.0f, 0.0f, 0.0f);

//
//#############################################################################
//#############################################################################
//
YawPitchRoll&
	YawPitchRoll::operator=(const EulerAngles &angles)
{
	Check_Pointer(this);
	Check_Object(&angles);

	LinearMatrix4D m;
	m.BuildRotation(angles);
	*this = m;

	return *this;
}

//
//#############################################################################
//#############################################################################
//
YawPitchRoll&
	YawPitchRoll::operator=(const UnitQuaternion &quaternion)
{
	Check_Pointer(this);
	Check_Object(&quaternion);

	LinearMatrix4D m;
	m.BuildRotation(quaternion);
	return *this = m;
}

//
//#############################################################################
//#############################################################################
//
YawPitchRoll&
	YawPitchRoll::operator=(const LinearMatrix4D &matrix)
{
	Check_Pointer(this);
	Check_Object(&matrix);

	Verify(
		Vector3D::Forward.z == 1.0f && Vector3D::Right.x == -1.0f && Vector3D::Up.y == 1.0f
		 || Vector3D::Forward.z == -1.0f && Vector3D::Right.x == 1.0f && Vector3D::Up.y == 1.0f
	);

	SinCosPair
		p,y,r;

	//
	//---------------------------------------------------
	// First deal with the singularity of 90 degree pitch
	//---------------------------------------------------
	//
	p.sine = -matrix(2,1);
	if (Close_Enough(p.sine,1.0f,0.0001f))
	{
		y.sine = matrix(1,0);
		y.cosine = matrix(0,0);
		yaw = y;
		pitch = Pi_Over_2;
		roll = 0.0f;
		return *this;
	}

	//
	//-------------------------------
	// Now deal with -90 degree pitch
	//-------------------------------
	//
	else if (Close_Enough(p.sine,-1.0f,0.0001f))
	{
		y.sine = matrix(0,2);
		y.cosine = matrix(0,0);
		yaw = y;
		pitch = -Pi_Over_2;
		roll = 0.0f;
		return *this;
	}

	//
	//------------------------------------------------------------------------
	// Otherwise, assume that pitch must be constrained between +/- 90 degrees
	//------------------------------------------------------------------------
	//
	else {
		p.cosine = Sqrt(1.0f - p.sine*p.sine);
		y.sine = matrix(2,0) / p.cosine;
		y.cosine = matrix(2,2) / p.cosine;
		r.sine = matrix(0,1) / p.cosine;
		r.cosine = matrix(1,1) / p.cosine;
		Verify(
			Close_Enough(
				y.cosine*r.cosine + p.sine*y.sine*r.sine,
				matrix(0,0),
				1e-4f
			)
		);
	}

	pitch = p;
	yaw = y;
	roll = r;
	return *this;
}

//
//#############################################################################
//#############################################################################
//
bool
	Stuff::Small_Enough(
		const YawPitchRoll& angles,
		Scalar e
	)
{
	Check_Object(&angles);

	return
		Small_Enough(angles.pitch,e)
		 && Small_Enough(angles.yaw,e)
		 && Small_Enough(angles.roll,e);
}

//
//#############################################################################
//#############################################################################
//
bool
	Stuff::Close_Enough(
		const YawPitchRoll& a1,
		const YawPitchRoll& a2,
		Scalar e
	)
{
	Check_Object(&a1);
	Check_Object(&a2);

	return
		Close_Enough(a1.pitch,a2.pitch,e)
		 && Close_Enough(a1.yaw,a2.yaw,e)
		 && Close_Enough(a1.roll,a2.roll,e);
}

//
//#############################################################################
//#############################################################################
//
YawPitchRoll&
	YawPitchRoll::Lerp(
		const YawPitchRoll &a1,
		const YawPitchRoll &a2,
		Scalar t
	)
{
	Check_Pointer(this);
	Check_Object(&a1);
	Check_Object(&a2);

	yaw = Stuff::Lerp(a1.yaw,a2.yaw,t);
	pitch = Stuff::Lerp(a1.pitch,a2.pitch,t);
	roll = Stuff::Lerp(a1.roll,a2.roll,t);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
YawPitchRoll&
	YawPitchRoll::Normalize()
{
	Check_Pointer(this);

	yaw.Normalize();
	pitch.Normalize();
	roll.Normalize();
	return *this;
}

//
//#############################################################################
//#############################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const YawPitchRoll &angle
		)
	{
		Check_Object(&angle);

		SPEW((group, "<+"));
		Spew(group, angle.yaw);
		SPEW((group, ",+"));
		Spew(group, angle.pitch);
		SPEW((group, ",+"));
		Spew(group, angle.roll);
		SPEW((group, ">+"));
	}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ UnitQuaternion ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const UnitQuaternion
	UnitQuaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);
DEFINE_TIMER(UnitQuaternion, SlerpTime);
DWORD
	UnitQuaternion::SlerpCount;

//
//#############################################################################
//#############################################################################
//

const int QuaternionLerpTableSize=static_cast<int>(1024);
const int SinTableSize=static_cast<int>(1024);


const float MinCosom = static_cast<float>(-1.0f);
const float MaxCosom = static_cast<float>(1.0f);
const float CosomRangeOverOne = static_cast<float>(1.0f/(MaxCosom-MinCosom));
const float CosBiggestNumber = (float)static_cast<unsigned int>(0xffffffff>>(32-10));


const float MinSin = static_cast<float>(-1.3);
const float MaxSin = static_cast<float>(1.3);
const float SinRangeOverOne = static_cast<float>(1.0f/(MaxSin-MinSin));
const float SinIncrement = static_cast<float>((MaxSin - MinSin) / SinTableSize);
const float SinBiggestNumber = (float)static_cast<unsigned int>(0xffffffff>>(32-10));


float Omega_Table[QuaternionLerpTableSize];
float SinomOverOne_Table[QuaternionLerpTableSize];
float Sin_Table[SinTableSize];

bool  quaternionFastLerpTableBuilt = false;
float tableIncrementStepOverOne;

//
//#############################################################################
//#############################################################################
//

void
	UnitQuaternion::InitializeClass()
{

	Verify(!quaternionFastLerpTableBuilt);
	Verify(QuaternionLerpTableSize > 0);

	Initialize_Timer(SlerpTime, "Slerp Time");
	AddStatistic( "Quat Slerp Count",	"slerps",	gos_DWORD, &SlerpCount, Stat_AutoReset);

	AddDebuggerMenuItem("Libraries\\Animation\\Use Fast Lerp", Check_UseFastLerp, Activate_UseFastLerp, NULL );
	AddDebuggerMenuItem("Libraries\\Animation\\Use Fast Normalize", Check_UseFastNormalize, Activate_UseFastNormalize, NULL );
	

	float increment_step = (MaxCosom - MinCosom) / QuaternionLerpTableSize;
	tableIncrementStepOverOne = 1.0f / increment_step;
	float cosom = MinCosom;

	


	for (int i = 0; i < QuaternionLerpTableSize; ++i)
	{
		Verify(cosom >= MinCosom);
		Verify(cosom <= MaxCosom);

		Omega_Table[i] = Arccos(cosom);
		SinomOverOne_Table[i] = 1.0f/Sin(Omega_Table[i]);

		cosom += increment_step;
	}


	float sin_seed = MinSin;
	
	for (int i = 0; i < SinTableSize; ++i)
	{
		Verify(sin_seed >= MinSin);
		Verify(sin_seed <= MaxSin);

		Sin_Table[i] = Sin(sin_seed);

		sin_seed += SinIncrement;
	}

	quaternionFastLerpTableBuilt = true;
}

//
//#############################################################################
//#############################################################################
//

void
	UnitQuaternion::TerminateClass()
{
	quaternionFastLerpTableBuilt = false;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::operator=(const EulerAngles &angles)
{
	Check_Pointer(this);
	Check_Object(&angles);

	LinearMatrix4D m;
	m.BuildRotation(angles);
	Check_Object(&m);
	*this = m;
	return *this;
}
//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::operator=(const YawPitchRoll &angles)
{
	LinearMatrix4D	lin_matrix;
	lin_matrix.BuildRotation(angles);
	*this = lin_matrix;
   return *this;
}
//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::operator=(const LinearMatrix4D &matrix)
{
	Check_Pointer(this);
	Check_Object(&matrix);

	//
	//------------------------------------------------------------------------
	// Compute the w component.  If it is close enough to zero, then we have a
	// 180 degree pivot, so figure out the correct axis to rotate around
	//------------------------------------------------------------------------
	//
	w = (1.0f + matrix(0,0) + matrix(1,1) + matrix(2,2)) * 0.25f;
	if (Small_Enough(w,1e-2f))
	{
		Verify(w >= -SMALL);
		if (w<0.0f)
		{
			w = 0.0f;
		}

		//
		//----------------------------------------------------------------
		// Figure out the length of each component of the axis of rotation
		//----------------------------------------------------------------
		//
		Scalar temp = (1.0f + matrix(0,0)) * 0.5f - w;
		Min_Clamp(temp, 0.0f);
		x = Sqrt(temp);
		temp = (1.0f + matrix(1,1)) * 0.5f - w;
		Min_Clamp(temp, 0.0f);
		y = Sqrt(temp);
		temp = (1.0f + matrix(2,2)) * 0.5f - w;
		Min_Clamp(temp, 0.0f);
		z = Sqrt(temp);
		w = Sqrt(w);

		//
		//-------------------------------------------
		// Now figure out the signs of the components
		//-------------------------------------------
		//
		if (matrix(0,1) < matrix(1,0))
		{
			z = -z;
		}
		if (matrix(2,0) < matrix(0,2))
		{
			y = -y;
		}
		if (matrix(1,2) < matrix(2,1))
		{
			x = -x;
		}
	}

	//
	//----------------------------------------------------------
	// Otherwise, determine x, y, and z directly from the matrix
	//----------------------------------------------------------
	//
	else
	{
		Verify(w>0.0f);
		w = Sqrt(w);
		x = (matrix(1,2) - matrix(2,1)) * 0.25f / w;
		y = (matrix(2,0) - matrix(0,2)) * 0.25f / w;
		z = (matrix(0,1) - matrix(1,0)) * 0.25f / w;
	}

	Normalize();
	return *this;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::operator=(const Vector3D &v)
{
	Check_Pointer(this);
	Check_Object(&v);

	//
	//---------------------------------------------------------------
	// See if there is any rotation to apply to the source quaternion
	//---------------------------------------------------------------
	//
	Scalar rotation = v.GetLength();
	if (Small_Enough(rotation))
	{
		return *this = Identity;
	}

	//
	//---------------------------------------------------------------------
	// Build a quaternion from the delta vector, treating the length as the
	// amount of rotation and the direction of the vector as the axis of
	// rotation
	//---------------------------------------------------------------------
	//
	SinCosPair half_angle;
	half_angle = 0.5f * Radian::Normalize(rotation);
	rotation = half_angle.sine / rotation;
	x = v.x * rotation;
	y = v.y * rotation;
	z = v.z * rotation;
	w = half_angle.cosine;
	Check_Object(this);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
bool
	Stuff::Close_Enough(
		const UnitQuaternion& a1,
		const UnitQuaternion& a2,
		Scalar e
	)
{
	Check_Object(&a1);
	Check_Object(&a2);

	Vector4D v(a1.x-a2.x, a1.y-a2.y, a1.z-a2.z, a1.w-a2.w);
	return Small_Enough(v, e);
}

//
//#############################################################################
//#############################################################################
//
Scalar
	UnitQuaternion::GetAngle()
{
	Check_Object(this);

	Scalar sine_of_half = Sqrt(x*x + y*y + z*z);
	if (Small_Enough(sine_of_half))
	{
		return 0.0f;
	}

	SinCosPair half_angle(sine_of_half, w);
	Radian angle;
	angle = half_angle;

	return angle * 2.0f;
}

//
//#############################################################################
//#############################################################################
//
void
	UnitQuaternion::GetAxis(UnitVector3D *axis)
{
	Check_Object(this);
	Check_Pointer(axis);

	Scalar len = Sqrt(x*x + y*y + z*z);
	if (Small_Enough(len))
	{
		axis->x = 1.0f;
		axis->y = 0.0f;
		axis->z = 0.0f;
	}
	else
	{
		axis->x = x / len;
		axis->y = y / len;
		axis->z = z / len;
	}

	Check_Object(axis);
	return;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Multiply(const UnitQuaternion &q2, const UnitQuaternion &q1)
{
	Check_Pointer(this);
	Check_Object(&q1);
	Check_Object(&q2);
	Verify(this != &q1 && this != &q2);

	x = q1.w*q2.x + q2.w*q1.x + q1.y*q2.z - q1.z*q2.y;
	y = q1.w*q2.y + q2.w*q1.y + q1.z*q2.x - q1.x*q2.z;
	z = q1.w*q2.z + q2.w*q1.z + q1.x*q2.y - q1.y*q2.x;
	w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;

	Check_Object(this);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Multiply(const UnitQuaternion &q, const LinearMatrix4D &m)
{
	Check_Pointer(this);
	Check_Object(&q);
	Check_Object(&m);

	LinearMatrix4D t1;
	t1.BuildRotation(q);
	LinearMatrix4D t2;
	t2.Multiply(t1,m);
	*this = t2;

	Check_Object(this);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Multiply(
		const UnitQuaternion &q,
		Scalar t
	)
{
	Check_Pointer(this);
	Check_Object(&q);

	//
	//---------------------------------------------------------
	// Figure out the half the angle of rotation and scale that
	//---------------------------------------------------------
	//
	Scalar sine_of_half = Sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
	if (Small_Enough(sine_of_half))
	{
		*this = Identity;
		return *this;
	}

	SinCosPair half_angle(sine_of_half, q.w);
	Radian angle;
	angle = half_angle;
	angle *= t;
	half_angle = angle;

	//
	//-----------------------------------------------------------------
	// Build the scaled quaternion out of the components of the old one
	//-----------------------------------------------------------------
	//
	w = half_angle.cosine;
	sine_of_half = half_angle.sine / sine_of_half;
	x = q.x * sine_of_half;
	y = q.y * sine_of_half;
	z = q.z * sine_of_half;

	Check_Object(this);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::MultiplyScaled(
		const UnitQuaternion &q1,
		const UnitQuaternion &q2,
		Scalar t
	)
{
	Check_Pointer(this);
	Verify(this != &q1);
	Check_Object(&q1);
	Check_Object(&q2);
	Verify(t>=0.0f);

	UnitQuaternion scaled_quat;
	scaled_quat.Multiply(q2, t);
	Multiply(q1, scaled_quat);

	Check_Object(this);
   return *this;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Normalize()
{
	

	Scalar t = x*x + y*y + z*z;
	if (t <= 1.0f)
	{
		t = Sqrt(1.0f - t);
		if (w<0.0f)
		{
			x = -x;
			y = -y;
			z = -z;
		}
		w = t;
		TestInstance();
	}
	else
	{
		t = Sqrt(t);
		t = 1.0f/t;
		x *= t;
		y *= t;
		z *= t;
		w = 0.0f;
		TestInstance();
	}
	return *this;
}

//
//#############################################################################
//#############################################################################
//

UnitQuaternion&
	UnitQuaternion::FastNormalize()
{
	if (!UseFastNormalize)
		return Normalize();

	Scalar t = x*x + y*y + z*z;
	if (t <= 1.0f)
	{
		t = SqrtApproximate(1.0f - t);
		if (w<0.0f)
		{
			x = -x;
			y = -y;
			z = -z;
		}
		w = t;
		TestInstance();
	}
	else
	{
		t = SqrtApproximate(t);
		t = 1.0f/t;
		x *= t;
		y *= t;
		z *= t;
		w = 0.0f;
		TestInstance();
	}
	return *this;
}


//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Subtract(
		const UnitQuaternion &end,
		const UnitQuaternion &start
	)
{
	Check_Pointer(this);
	Check_Object(&start);
	Check_Object(&end);

	UnitQuaternion inverse(start);
	inverse.w = -inverse.w;
	Multiply(inverse, end);
	return Normalize();
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Subtract(
		const UnitVector3D &end,
		const UnitVector3D &start
	)
{
	Check_Pointer(this);
	Check_Object(&start);
	Check_Object(&end);

	Vector3D
		axis;
	SinCosPair
		delta;
	delta.cosine = start*end;

	//
	//----------------------------------------------------------------------
	// See if the vectors point in the same direction.  If so, return a null
	// rotation
	//----------------------------------------------------------------------
	//
	if (Close_Enough(delta.cosine, 1.0f))
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 1.0f;
	}

	//
	//-------------------------------------------------------------------------
	// See if the vectors directly oppose each other.  If so, pick the smallest
	// axis coordinate and generate a vector along it.  Project this onto the
	// base vector and subtract it out, leaving a perpendicular projection.
	// Extend that out to unit length, then set the angle to PI
	//-------------------------------------------------------------------------
	//
	else if (Close_Enough(delta.cosine, -1.0f))
	{
		//
		//---------------------------
		// Pick out the smallest axis
		//---------------------------
		//
		int
			smallest=0;
		Scalar
			value=2.0f;
		for (int i=X_Axis; i<=Z_Axis; ++i)
		{
			if (Abs(start[i]) < value)
			{
				smallest = i;
				value = Abs(start[i]);
			}
		}

		//
		//----------------------------------------
		// Set up a vector along the selected axis
		//----------------------------------------
		//
		axis.x = 0.0f;
		axis.y = 0.0f;
		axis.z = 0.0f;
		axis[smallest] = 1.0f;

		//
		//-------------------------------------------------------------------
		// If the value on that axis wasn't zero, subtract out the projection
		//-------------------------------------------------------------------
		//
		if (!Small_Enough(value))
		{
			Vector3D t;
			t.Multiply(start, start*axis);
			axis.Subtract(axis, t);
			axis.Normalize(axis);
		}

		//
		//----------------------
		// Convert to quaternion
		//----------------------
		//
		x = axis.x;
		y = axis.y;
		z = axis.z;
		w = 0.0f;
	}

	//
	//--------------------------------------------------
	// Otherwise, generate the cross product and unitize
	//--------------------------------------------------
	//
	else
	{
		axis.Cross(start, end);
		delta.sine = axis.GetLength();
		axis /= delta.sine;

		//
		//---------------------------------------------------------------
		// Now compute sine and cosine of half the angle and generate the
		// quaternion
		//---------------------------------------------------------------
		//
		delta.sine = Sqrt((1.0f - delta.cosine)*0.5f);
		x = axis.x * delta.sine;
		y = axis.y * delta.sine;
		z = axis.z * delta.sine;
		w = Sqrt((1.0f + delta.cosine)*0.5f);
	}
	return *this;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Subtract(
		const Vector3D &end,
		const Vector3D &start
	)
{
	Check_Pointer(this);
	Check_Object(&start);
	Check_Object(&end);

	UnitVector3D
		s,e;

	s = start;
	e = end;
	return Subtract(e, s);
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Lerp(
		const EulerAngles& v1,
		const EulerAngles& v2,
		Scalar t
	)
	{
		UnitQuaternion q1;
		q1 = v1;
		UnitQuaternion q2;
		q2 = v2;
		return Lerp(q1,q2,t);
	}

//#############################################################################
//#############################################################################
//
#define SLERP_THRESHOLD (float)0.00001f


UnitQuaternion &UnitQuaternion::Lerp(const UnitQuaternion& p, const UnitQuaternion& q, Scalar t)
{

	Start_Timer(SlerpTime);

	Set_Statistic(SlerpCount, SlerpCount+1);

	Scalar omega,cosom,sinom,sclp,sclq;
	//UnitQuaternion qt;


	//UnitQuaternion q = q_temp;
	//UnitQuaternion p = p_temp;

	cosom = p.x*q.x + p.y*q.y + p.z*q.z + p.w*q.w;


	if ( (1.0f + cosom) > 0.01f)
	{
		// usual case 
	
		
		if ( (1.0f - cosom) > 0.00001f ) 
		{ 
			//usual case 
			omega = Arccos(cosom);
			sinom = Sin(omega);
			
			//SPEW(("jerryeds","omega:%f sinom:%f", omega, sinom));

			sclp = Sin((1.0f - t)*omega) / sinom;
			sclq = Sin(t*omega) / sinom;
		
			//SPEW(("jerryeds", "* %f %f", sclp, sclq));
		}
		else 
		{ 
			// ends very close -- just lerp
			sclp = 1.0f - t;
			sclq = t;

			//SPEW(("jerryeds", "# %f %f", sclp, sclq));
		}


		x = sclp*p.x + sclq*q.x;
		y = sclp*p.y + sclq*q.y;
		z = sclp*p.z + sclq*q.z;
		w = sclp*p.w + sclq*q.w;

		//SPEW(("jerryeds", "r:<%f,%f,%f,%f>",x,y,z,w));
	}
	else 
	{
		//SPEW(("jerryeds","SPECIAL CASE"));
		/* p and q nearly opposite on sphere-- this is a 360 degree
		   rotation, but the axis of rotation is undefined, so
		   slerp really is undefined too.  So this apparently picks 
		   an arbitrary plane of rotation. However, I think this 
		   code is incorrect.
		   */

		//really we want the shortest distance.  They are almost on top of each other.

		UnitQuaternion r;
		r.Subtract(q, p);

		Vector3D scaled_rotation;
		scaled_rotation = r;
		scaled_rotation *= t;
		UnitQuaternion scaled_quat;
		scaled_quat = scaled_rotation;

		Multiply(scaled_quat, p);

	}

	Stop_Timer(SlerpTime);

	return *this;
}

//
//#############################################################################
//#############################################################################
//

UnitQuaternion&
	UnitQuaternion::FastLerp(
		const UnitQuaternion& p, 
		const UnitQuaternion& q, 
		Scalar t
	)
{


	if (!UseFastLerp)
		return Lerp(p,q,t);


	Start_Timer(SlerpTime);

	Set_Statistic(SlerpCount, SlerpCount+1);

	Verify(quaternionFastLerpTableBuilt);

	Scalar cosom,sclp,sclq;
	
	cosom = p.x*q.x + p.y*q.y + p.z*q.z + p.w*q.w;


	if ( (1.0f + cosom) > 0.01f)
	{
		// usual case 
	

		
		if ( (1.0f - cosom) > 0.00001f ) 
		{ 
			//usual case
			
			

			//table_entry = (int)Scaled_Float_To_Bits(cosom, MinCosom, MaxCosom, 10);


			float tabled_float =  cosom - MinCosom;
			int cos_table_entry = Truncate_Float_To_Word(((tabled_float*CosomRangeOverOne) * CosBiggestNumber));
			
			Verify(cos_table_entry >= 0);
			Verify(cos_table_entry <= QuaternionLerpTableSize);


#if 0
			sclp = Sin((1.0f - t)*Omega_Table[cos_table_entry]) * SinomOverOne_Table[cos_table_entry];
			sclq = Sin(t*Omega_Table[cos_table_entry]) * SinomOverOne_Table[cos_table_entry];
	
#else
		
			float difference, percent, lerped_sin;


			tabled_float =  ((1.0f - t)*Omega_Table[cos_table_entry]) - MinSin;
			int sclp_table_entry = Truncate_Float_To_Word(((tabled_float*SinRangeOverOne) * SinBiggestNumber));


			if (!(sclp_table_entry < SinTableSize))
			{
				Max_Clamp(sclp_table_entry, SinTableSize-1);
			}


			Verify(sclp_table_entry >= 0 && sclp_table_entry < SinTableSize);
			difference = tabled_float - (SinIncrement * sclp_table_entry);
			percent = difference / SinIncrement;
			int lerp_to_entry = sclp_table_entry + 1;
			Max_Clamp(lerp_to_entry, SinTableSize-1);
			lerped_sin = Stuff::Lerp(Sin_Table[sclp_table_entry], Sin_Table[lerp_to_entry], percent);
			sclp = lerped_sin * SinomOverOne_Table[cos_table_entry];



			tabled_float =  (t*Omega_Table[cos_table_entry]) - MinSin;
			int sclq_table_entry = Truncate_Float_To_Word(((tabled_float*SinRangeOverOne) * SinBiggestNumber));
			Verify(sclq_table_entry >= 0 && sclq_table_entry < SinTableSize);
			difference = tabled_float - (SinIncrement * sclq_table_entry);
			percent = difference / SinIncrement;
			lerp_to_entry = sclq_table_entry + 1;
			Max_Clamp(lerp_to_entry, SinTableSize-1);
			lerped_sin = Stuff::Lerp(Sin_Table[sclq_table_entry], Sin_Table[lerp_to_entry], percent);
			sclq = lerped_sin * SinomOverOne_Table[cos_table_entry];
#endif
			
		}
		else 
		{ 
			// ends very close -- just lerp
			sclp = 1.0f - t;
			sclq = t;

			
		}


		x = sclp*p.x + sclq*q.x;
		y = sclp*p.y + sclq*q.y;
		z = sclp*p.z + sclq*q.z;
		w = sclp*p.w + sclq*q.w;

		
	}
	else 
	{



		//SPEW(("jerryeds","SPECIAL CASE"));
		/* p and q nearly opposite on sphere-- this is a 360 degree
		   rotation, but the axis of rotation is undefined, so
		   slerp really is undefined too.  So this apparently picks 
		   an arbitrary plane of rotation. However, I think this 
		   code is incorrect.
		   */

		//really we want the shortest distance.  They are almost on top of each other.
	
		UnitQuaternion r;
		r.Subtract(q, p);

		Vector3D scaled_rotation;
		scaled_rotation = r;
		scaled_rotation *= t;
		UnitQuaternion scaled_quat;
		scaled_quat = scaled_rotation;

		Multiply(scaled_quat, p);

		Normalize();

	}


	Stop_Timer(SlerpTime);

	return *this;

}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion
	UnitQuaternion::Squad(
		const UnitQuaternion& p, // start quaternion
		const UnitQuaternion& a, // start tangent quaternion
		const UnitQuaternion& b, // end tangent quaternion
		const UnitQuaternion& q, // end quaternion
		Scalar t
	)
{
	Scalar k = 2.0f * (1.0f - t)*t;
    return(Lerp(Lerp(p,q,t),Lerp(a,b,t),k));
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion
	UnitQuaternion::SquadRev(
		Scalar angle,			// angle of rotation 
		const Point3D& axis,	// the axis of rotation 
		const UnitQuaternion& p,	// start quaternion 
		const UnitQuaternion& a, 	// start tangent quaternion 
		const UnitQuaternion& b, 	// end tangent quaternion 
		const UnitQuaternion& q,	// end quaternion 
		Scalar t 				// parameter, in range [0.0,1.0] 
	)
{
	Scalar s,v;
	Scalar omega = angle*0.5f;
	Scalar nrevs = 0.0f;
	UnitQuaternion r,pp,qq;


	if (omega<Pi-0.0001f) 
	{ 
		r = Squad(p,a,b,q,t); 

		return(r); 
	}

	while (omega > (Pi-0.0001f)) 
	{  
		omega -= Pi; nrevs += (float)1.0; 
	}

	if (omega<0.0f) 
	{
		omega = (float)0.0;
	}

	s = t*angle/Pi;		  /* 2t(omega+Npi)/pi */
	
	if (s < (float)1.0) 
	{
		pp.Orthog(p,axis);
		r = Squad(p,a,pp,pp,s); 	/* in first 90 degrees */
	}
	else 
	{
		if ( ( v = s + 1.0f - 2.0f*(nrevs+(omega/Pi)) ) <=  0.0f)  
		{
			/* middle part, on great circle(p,q) */
			while (s >= 2.0f) 
			{
				s -= 2.0f;
			}
			pp.Orthog(p,axis);
			r = Lerp(p,pp,s);
		}
		else 
		{ 	  /* in last 90 degrees */ 
			qq.Orthog(q,axis);
			qq.Negate();
			r= Squad(qq,qq,b,q,v);
		}
	}

	return(r);
}

//
//#############################################################################
//#############################################################################
//

UnitQuaternion& 
	UnitQuaternion::MakeClosest(const UnitQuaternion& qto)
{
	Scalar dot =  x*qto.x + y*qto.y + z*qto.z+ w*qto.w;
	if (dot<0.0f) 
	{
		x = -x; y = -y; z = -z; w = -w;
	}
	return *this;
}

//
//#############################################################################
//#############################################################################
//
Scalar
	UnitQuaternion::Dot(
		const UnitQuaternion& p, 
		const UnitQuaternion& q
	)
{
	return (q.x*p.x + q.y*p.y + q.z*p.z + q.w*p.w);
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion&
	UnitQuaternion::Inverse(const UnitQuaternion& q)
{
	Scalar l,norminv;
	
	l =	(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	if (l==0.0f)
	{
		l = 1.0f;
	}
	norminv = 1.0f/l;
	x = -q.x * norminv;
	y = -q.y * norminv;
	z = -q.z * norminv;
	w = q.w * norminv;
	return *this;
}

//
//#############################################################################
//#############################################################################
//

// Ratio of two quaternions: This creates a result quaternion r = p/q, such
// that q*r = p.  (order of multiplication is important)

UnitQuaternion&
	UnitQuaternion::Divide(
		const UnitQuaternion& p, 
		const UnitQuaternion& q
	)
{
	UnitQuaternion i;
	i.Inverse(q);
	Multiply(i, p);
	
	return *this;
}

//
//#############################################################################
//#############################################################################
//

UnitQuaternion&
	UnitQuaternion::LnDif(
		const UnitQuaternion& p, 
		const UnitQuaternion& q
	)
{
	UnitQuaternion r;
	r.Divide(q,p);
	return(LogN(r));
}

//
//#############################################################################
//#############################################################################
//

//  natural logarithm of UNIT quaternion 

UnitQuaternion&
	UnitQuaternion::LogN(
		const UnitQuaternion& q
	)	
{
	Scalar theta,scale;
	scale = Sqrt(q.x*q.x + q.y*q.y + q.z*q.z );
	theta = Arctan(scale,q.w);
	
	if (scale > 0.0f)
	{
		scale = theta/scale;
	}

	x = scale*q.x;
	y = scale*q.y;
	z = scale*q.z;
	w = 0.0f;
	return *this;
}
//
//#############################################################################
//#############################################################################
//


UnitQuaternion& 
	UnitQuaternion::Exp(const UnitQuaternion& q) 
{
	Scalar theta,scale;
	theta = Sqrt(q.x*q.x + q.y*q.y + q.z*q.z );
	scale = 1.0f;
	if (theta >0.0001f) 
	{
		scale = Sin(theta)/theta;
	}
	x = scale*q.x;
	y = scale*q.y;
	z = scale*q.z;
	w = Cos(theta);
	return *this;
}

//
//#############################################################################
//#############################################################################
//
UnitQuaternion
	UnitQuaternion::CompA(
		const UnitQuaternion& qprev,
		const UnitQuaternion& q,
		const UnitQuaternion& qnext
	)
{
	UnitQuaternion qm,qp,r;

	qm.LnDif(q,qprev);
	qp.LnDif(q,qnext);

	r.x= (qm.x+qp.x) * -0.25f;
	r.y= (qm.y+qp.y) * -0.25f;
	r.z= (qm.z+qp.z) * -0.25f;
	r.w= (qm.w+qp.w) * -0.25f;

	r.Exp(r);
	Multiply(q,r);
	
	return *this;
}

//
//#############################################################################
//#############################################################################
//

UnitQuaternion&
	UnitQuaternion::Orthog(
		const UnitQuaternion& p, 
		const Point3D& axis
	)
{
	Multiply(p, UnitQuaternion(axis.x,axis.y,axis.z,0.0f));
	return *this;
}



//
//#############################################################################
//#############################################################################
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const UnitQuaternion &quat
		)
	{
		Check_Object(&quat);
		SPEW((group, "<%f, %f, %f, %f>+", quat.x, quat.y, quat.z, quat.w));
	}
#endif

//
//#############################################################################
//#############################################################################
//
void
	UnitQuaternion::TestInstance() const
{

	Scalar diff = x*x + y*y + z*z + w*w - 1.0f;
	if (!Small_Enough(diff))
	{
		UnitQuaternion q2 = *this;
		q2.Normalize();
		diff = q2.x*q2.x + q2.y*q2.y + q2.z*q2.z + q2.w*q2.w - 1.0f;
		if (Small_Enough(diff))
			STOP(("UnitQuaternion needs normalizing"));
	}
	Verify(Small_Enough(diff));

}
