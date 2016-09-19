//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

OBB
	OBB::Identity(LinearMatrix4D::Identity, Vector3D::Identity, 0.0f);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	OBB::TestInstance() const
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
#if !defined(Spew)
	void
		Spew(
			const char* group,
			const OBB &box
		)
	{
		Check_Object(&box);
		SPEW((group, ""));
		SPEW((group, "  Transform = +"));
		Spew(group, box.localToParent);
		SPEW((group, ""));
		SPEW((
			group,
			"  Extents = <%4f,%4f,%4f>, Radius = %4f+",
			box.axisExtents.x,
			box.axisExtents.y,
			box.axisExtents.z,
			box.sphereRadius
		));
	}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
OBB&
	OBB::Multiply(
		const OBB &obb,
		const LinearMatrix4D &matrix
	)
{
	Check_Pointer(this);
	Check_Object(&obb);
	Check_Object(&matrix);

	localToParent.Multiply(obb.localToParent, matrix);
	axisExtents = obb.axisExtents;
	sphereRadius = obb.sphereRadius;
	return *this;
}

#if 0	// moved it into hpp-file
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
OBB&
	OBB::MultiplySphereOnly(
		const OBB &obb,
		const LinearMatrix4D &matrix
	)
{
	Check_Pointer(this);
	Check_Object(&obb);
	Check_Object(&matrix);

#if USE_ASSEMBLER_CODE

	Scalar *f = localToParent.entries;

	_asm {
		mov         edx, matrix
		push        esi
		mov         esi, obb.localToParent
		
		mov         eax, f

		fld         dword ptr [edx+4]		//	m[1][0]
		fmul        dword ptr [esi+01ch]	//	obb.localToParent(3,1)
		
		fld         dword ptr [edx+8]		//	m[2][0]
		fmul        dword ptr [esi+02Ch]	//	obb.localToParent(3,2)

		fxch		st(1)
		fadd        dword ptr [edx+0Ch]		//	m[3][0]
		
		fld         dword ptr [edx]			//	m[0][0]
		fmul        dword ptr [esi+0Ch]		//	obb.localToParent(3,0)

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+14h]		//	m[1][1]
		fmul        dword ptr [esi+01ch]	//	obb.localToParent(3,1)

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+18h]		//	m[2][1]
		fmul        dword ptr [esi+02ch]	//	obb.localToParent(3,2)
		
		fxch		st(1)
		fstp        dword ptr [eax+0ch]		//	localToParent(3,0)

		fadd        dword ptr [edx+1Ch]		//	m[3][1]

		fld         dword ptr [edx+10h]		//	m[0][1]
		fmul        dword ptr [esi+0ch]		//	obb.localToParent(3,0)

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+24h]		//	m[1][2]
		fmul        dword ptr [esi+01ch]	//	obb.localToParent(3,1)

		fxch		st(2)
		faddp       st(1),st

		fld         dword ptr [edx+28h]		//	m[2][2]
		fmul        dword ptr [esi+02ch]	//	obb.localToParent(3,2)
		
		fxch		st(1)
		fstp        dword ptr [eax+01ch]	//	localToParent(3,1)

		fadd        dword ptr [edx+2Ch]		//	m[3][2]
		
		fld         dword ptr [edx+20h]		//	m[0][2]
		fmul        dword ptr [esi+0ch]		//	obb.localToParent(3,0)

		fxch		st(2)
		faddp       st(1),st

		pop         esi

		faddp       st(1),st

		fstp        dword ptr [eax+02ch]	//	localToParent(3,2)
	}
#else
	localToParent(3,0) =
		obb.localToParent(3,0)*matrix(0,0)
		 + obb.localToParent(3,1)*matrix(1,0)
		 + obb.localToParent(3,2)*matrix(2,0)
		 + matrix(3,0);
	localToParent(3,1) =
		obb.localToParent(3,0)*matrix(0,1)
		 + obb.localToParent(3,1)*matrix(1,1)
		 + obb.localToParent(3,2)*matrix(2,1)
		 + matrix(3,1);
	localToParent(3,2) =
		obb.localToParent(3,0)*matrix(0,2)
		 + obb.localToParent(3,1)*matrix(1,2)
		 + obb.localToParent(3,2)*matrix(2,2)
		 + matrix(3,2);
#endif

	sphereRadius = obb.sphereRadius;
	return *this;
}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
OBB::SeparatingAxis
	OBB::FindSeparatingAxis(const OBB& box) const
{
	Check_Object(this);
	Check_Object(&box);
	Verify(axisExtents.x >= 0.0f);
	Verify(axisExtents.y >= 0.0f);
	Verify(axisExtents.z >= 0.0f);
	Verify(box.axisExtents.x >= 0.0f);
	Verify(box.axisExtents.y >= 0.0f);
	Verify(box.axisExtents.z >= 0.0f);

	//
	//------------------------------------------
	// Get the distance between the centerpoints
	//------------------------------------------
	//
	Check_Object(&localToParent);
	Check_Object(&box.localToParent);
	Point3D
		distance(
			localToParent(3,0) - box.localToParent(3,0),
			localToParent(3,1) - box.localToParent(3,1),
			localToParent(3,2) - box.localToParent(3,2)
		);

	//
	//-----------------------------------------------------------------------
	// First check L = Ax.  The next 6 tests are basically face tests to try
	// and find a separating plane.  Both boxes are currently in parent space
	//-----------------------------------------------------------------------
	//
	Scalar ab[3][3];
	ab[0][0] =
		Fabs(
			localToParent(0,0) * box.localToParent(0,0)
			 + localToParent(0,1) * box.localToParent(0,1)
			 + localToParent(0,2) * box.localToParent(0,2)
		);
	ab[0][1] =
		Fabs(
			localToParent(0,0) * box.localToParent(1,0)
			 + localToParent(0,1) * box.localToParent(1,1)
			 + localToParent(0,2) * box.localToParent(1,2)
		);
	ab[0][2] =
		Fabs(
			localToParent(0,0) * box.localToParent(2,0)
			 + localToParent(0,1) * box.localToParent(2,1)
			 + localToParent(0,2) * box.localToParent(2,2)
		);

	Scalar radius =
		axisExtents.x
		 + box.axisExtents.x * ab[0][0]
		 + box.axisExtents.y * ab[0][1]
		 + box.axisExtents.z * ab[0][2];
	Scalar projection =
		localToParent(0,0) * distance.x
		 + localToParent(0,1) * distance.y
		 + localToParent(0,2) * distance.z;
	if (projection > radius || projection < -radius)
	{
		return A0;
	}

	//
	//-------------
	// Check L = Ay
	//-------------
	//
	ab[1][0] =
		Fabs(
			localToParent(1,0) * box.localToParent(0,0)
			 + localToParent(1,1) * box.localToParent(0,1)
			 + localToParent(1,2) * box.localToParent(0,2)
		);
	ab[1][1] =
		Fabs(
			localToParent(1,0) * box.localToParent(1,0)
			 + localToParent(1,1) * box.localToParent(1,1)
			 + localToParent(1,2) * box.localToParent(1,2)
		);
	ab[1][2] =
		Fabs(
			localToParent(1,0) * box.localToParent(2,0)
			 + localToParent(1,1) * box.localToParent(2,1)
			 + localToParent(1,2) * box.localToParent(2,2)
		);

	radius =
		axisExtents.y
		 + box.axisExtents.x * ab[1][0]
		 + box.axisExtents.y * ab[1][1]
		 + box.axisExtents.z * ab[1][2];
	projection =
		localToParent(1,0) * distance.x
		 + localToParent(1,1) * distance.y
		 + localToParent(1,2) * distance.z;
	if (projection > radius || projection < -radius)
	{
		return A1;
	}

	//
	//-------------
	// Check L = Az
	//-------------
	//
	ab[2][0] =
		Fabs(
			localToParent(2,0) * box.localToParent(0,0)
			 + localToParent(2,1) * box.localToParent(0,1)
			 + localToParent(2,2) * box.localToParent(0,2)
		);
	ab[2][1] =
		Fabs(
			localToParent(2,0) * box.localToParent(1,0)
			 + localToParent(2,1) * box.localToParent(1,1)
			 + localToParent(2,2) * box.localToParent(1,2)
		);
	ab[2][2] =
		Fabs(
			localToParent(2,0) * box.localToParent(2,0)
			 + localToParent(2,1) * box.localToParent(2,1)
			 + localToParent(2,2) * box.localToParent(2,2)
		);

	radius =
		axisExtents.z
		 + box.axisExtents.x * ab[2][0]
		 + box.axisExtents.y * ab[2][1]
		 + box.axisExtents.z * ab[2][2];
	projection =
		localToParent(2,0) * distance.x
		 + localToParent(2,1) * distance.y
		 + localToParent(2,2) * distance.z;
	if (projection > radius || projection < -radius)
	{
		return A2;
	}

	//
	//-------------
	// Check L = Bx
	//-------------
	//
	radius =
		box.axisExtents.x 
		 + axisExtents.x * ab[0][0]
		 + axisExtents.y * ab[1][0]
		 + axisExtents.z * ab[2][0];
	projection =
		box.localToParent(0,0) * distance.x
		 + box.localToParent(0,1) * distance.y
		 + box.localToParent(0,2) * distance.z;
	if (projection > radius || projection < -radius)
	{
		return B0;
	}

	//
	//-------------
	// Check L = By
	//-------------
	//
	radius =
		box.axisExtents.y
		 + axisExtents.x * ab[0][1]
		 + axisExtents.y * ab[1][1]
		 + axisExtents.z * ab[2][1];
	projection =
		box.localToParent(1,0) * distance.x
		 + box.localToParent(1,1) * distance.y
		 + box.localToParent(1,2) * distance.z;
	if (projection > radius || projection < -radius)
	{
		return B1;
	}

	//
	//-------------
	// Check L = Bz
	//-------------
	//
	radius =
		box.axisExtents.z
		 + axisExtents.x * ab[0][2]
		 + axisExtents.y * ab[1][2]
		 + axisExtents.z * ab[2][2];
	projection =
		box.localToParent(2,0) * distance.x
		 + box.localToParent(2,1) * distance.y
		 + box.localToParent(2,2) * distance.z;
	if (projection > radius || projection < -radius)
	{
		return B2;
	}

	//
	//-------------------------------------------------------------------------
	// We have finished with the separation tests based upon the box faces.  We
	// now need to test with the edges, so make a matrix that converts local b
	// to local a, and figure a the new distance vector relative to a
	//-------------------------------------------------------------------------
	//
	LinearMatrix4D parent_to_a;
	parent_to_a.Invert(localToParent);
	LinearMatrix4D b_to_a;
	b_to_a.Multiply(box.localToParent, parent_to_a);
	distance = b_to_a;

	//
	//-------------------------------------------------------------------------
	// L = Ax crossed with Bx.  Since B has now been expressed in A, Ax can now
	// be simplified to <1,0,0>, making the cross-product much simpler
	//-------------------------------------------------------------------------
	//
	Scalar minor[3][3];
	minor[1][0] = Fabs(b_to_a(0,1)*b_to_a(2,2) - b_to_a(2,1)*b_to_a(0,2));
	minor[2][0] = Fabs(b_to_a(0,1)*b_to_a(1,2) - b_to_a(1,1)*b_to_a(0,2));

	Scalar ba[3][3];
	ba[0][1] = Fabs(b_to_a(0,1));
	ba[0][2] = Fabs(b_to_a(0,2));

	radius =
		axisExtents.y*ba[0][2]
		 + axisExtents.z*ba[0][1]
		 + box.axisExtents.y*minor[2][0]
		 + box.axisExtents.z*minor[1][0];
	projection = distance.z*b_to_a(0,1) - distance.y*b_to_a(0,2);
	if (projection > radius || projection < -radius)
	{
		return A0xB0;
	}

	//
	//-----------------------
	// L = Ax crossed with By
	//-----------------------
	//
	minor[0][0] = Fabs(b_to_a(1,1)*b_to_a(2,2) - b_to_a(2,1)*b_to_a(1,2));

	ba[1][1] = Fabs(b_to_a(1,1));
	ba[1][2] = Fabs(b_to_a(1,2));

	radius =
		axisExtents.y*ba[1][2]
		 + axisExtents.z*ba[1][1]
		 + box.axisExtents.x*minor[2][0]
		 + box.axisExtents.z*minor[0][0];
	projection = distance.z*b_to_a(1,1) - distance.y*b_to_a(1,2);
	if (projection > radius || projection < -radius)
	{
		return A0xB1;
	}

	//
	//-----------------------
	// L = Ax crossed with Bz
	//-----------------------
	//
	ba[2][1] = Fabs(b_to_a(2,1));
	ba[2][2] = Fabs(b_to_a(2,2));

	radius =
		axisExtents.y*ba[2][2]
		 + axisExtents.z*ba[2][1]
		 + box.axisExtents.x*minor[1][0]
		 + box.axisExtents.y*minor[0][0];
	projection = distance.z*b_to_a(2,1) - distance.y*b_to_a(2,2);
	if (projection > radius || projection < -radius)
	{
		return A0xB2;
	}

	//
	//-----------------------
	// L = Ay crossed with Bx
	//-----------------------
	//
	minor[2][1] = Fabs(b_to_a(0,2)*b_to_a(1,0) - b_to_a(0,0)*b_to_a(1,2));
	minor[1][1] = Fabs(b_to_a(0,0)*b_to_a(2,2) - b_to_a(2,0)*b_to_a(1,2));

	ba[0][0] = Fabs(b_to_a(0,0));

	radius =
		axisExtents.x*ba[0][2]
		 + axisExtents.z*ba[0][0]
		 + box.axisExtents.y*minor[2][1]
		 + box.axisExtents.z*minor[1][1];
	projection = distance.x*b_to_a(0,2) - distance.z*b_to_a(0,0);
	if (projection > radius || projection < -radius)
	{
		return A1xB0;
	}

	//
	//-----------------------
	// L = Ay crossed with By
	//-----------------------
	//
	minor[0][1] = Fabs(b_to_a(1,2)*b_to_a(2,0) - b_to_a(1,0)*b_to_a(2,2));

	ba[1][0] = Fabs(b_to_a(1,0));

	radius =
		axisExtents.x*ba[1][2]
		 + axisExtents.z*ba[1][0]
		 + box.axisExtents.x*minor[2][1]
		 + box.axisExtents.z*minor[0][1];
	projection = distance.x*b_to_a(1,2) - distance.z*b_to_a(1,0);
	if (projection > radius || projection < -radius)
	{
		return A1xB1;
	}

	//
	//-----------------------
	// L = Ay crossed with Bz
	//-----------------------
	//
	ba[2][0] = Fabs(b_to_a(2,0));

	radius =
		axisExtents.x*ba[2][2]
		 + axisExtents.z*ba[2][0]
		 + box.axisExtents.x*minor[1][1]
		 + box.axisExtents.y*minor[0][1];
	projection = distance.x*b_to_a(2,2) - distance.z*b_to_a(2,0);
	if (projection > radius || projection < -radius)
	{
		return A1xB2;
	}

	//
	//-----------------------
	// L = Az crossed with Bx
	//-----------------------
	//
	minor[2][2] = Fabs(b_to_a(1,1)*b_to_a(0,0) - b_to_a(0,1)*b_to_a(1,0));
	minor[1][2] = Fabs(b_to_a(2,1)*b_to_a(0,0) - b_to_a(2,0)*b_to_a(0,1));

	radius =
		axisExtents.x*ba[0][1]
		 + axisExtents.y*ba[0][0]
		 + box.axisExtents.y*minor[2][2]
		 + box.axisExtents.z*minor[1][2];
	projection = distance.y*b_to_a(0,0) - distance.x*b_to_a(0,1);
	if (projection > radius || projection < -radius)
	{
		return A2xB0;
	}

	//
	//-----------------------
	// L = Az crossed with By
	//-----------------------
	//
	minor[0][2] = Fabs(b_to_a(1,0)*b_to_a(2,1) - b_to_a(2,0)*b_to_a(1,1));

	radius =
		axisExtents.x*ba[1][1]
		 + axisExtents.y*ba[1][0]
		 + box.axisExtents.x*minor[2][2]
		 + box.axisExtents.z*minor[0][2];
	projection = distance.y*b_to_a(1,0) - distance.x*b_to_a(1,1);
	if (projection > radius || projection < -radius)
	{
		return A2xB1;
	}

	//
	//-----------------------
	// L = Az crossed with Bz
	//-----------------------
	//
	radius =
		axisExtents.x*ba[2][1]
		 + axisExtents.y*ba[2][0]
		 + box.axisExtents.x*minor[1][2]
		 + box.axisExtents.y*minor[0][2];
	projection = distance.y*b_to_a(2,0) - distance.x*b_to_a(2,1);
	if (projection > radius || projection < -radius)
	{
		return A2xB2;
	}

	return NoSeparation;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	OBB::Union(
		const OBB &first,
		const OBB &second
	)
{
	Check_Pointer(this);
	Check_Object(&first);
	Check_Object(&second);

	//
	//-------------------------------------------------
	// Learn about the spheres surrounding the two OBBs
	//-------------------------------------------------
	//
	Point3D c1(first.localToParent);
	Point3D c2(second.localToParent);
	Vector3D diff;
	diff.Subtract(c2, c1);
	Scalar len = diff.GetLength();

	//
	//---------------------------------------------------
	// See if the first sphere is contained in the second
	//---------------------------------------------------
	//
	if (len+first.sphereRadius <= second.sphereRadius || !first.sphereRadius)
	{
		if (this != &second)
			*this = second;
		return;
	}

	//
	//---------------------------------------------------
	// See if the second sphere is contained in the first
	//---------------------------------------------------
	//
	if (len+second.sphereRadius <= first.sphereRadius || !second.sphereRadius)
	{
		if (this != &first)
			*this = first;
		return;
	}

	//
	//-------------------------------------------------------
	// The new sphere will lie somewhere between the old ones
	//-------------------------------------------------------
	//
	localToParent = LinearMatrix4D::Identity;
	axisExtents = Vector3D::Identity;
	c1.Lerp(c1, c2, (len + second.sphereRadius - first.sphereRadius) / (2.0f*len));
	localToParent.BuildTranslation(c1);
	sphereRadius = 0.5f * (len + first.sphereRadius + second.sphereRadius);
}
