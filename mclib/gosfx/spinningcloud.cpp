#include"gosfxheaders.hpp"

//==========================================================================//
// File:	 gosFX_SpinningCloud.cpp										//
// Contents: Base gosFX::SpinningCloud Component							//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
//############################################################################
//########################  gosFX::SpinningCloud__Specification  #############################
//############################################################################

//------------------------------------------------------------------------------
//
gosFX::SpinningCloud__Specification::SpinningCloud__Specification(
	Stuff::RegisteredClass::ClassID class_id,
	Stuff::MemoryStream *stream,
	int gfx_version
):
	ParticleCloud__Specification(class_id, stream, gfx_version)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);

	//
	//-----------------
	// Load the fcurves
	//-----------------
	//
	m_pSpin.Load(stream, gfx_version);
	m_pScale.Load(stream, gfx_version);

	//
	//-----------------------
	// Load the rotation flag
	//-----------------------
	//
	*stream >> m_randomStartingRotation;
	*stream >> m_alignZUsingX >> m_alignZUsingY;
	if (gfx_version >= 10)
		*stream >> m_alignYUsingVelocity;
	else
		m_alignYUsingVelocity = false;
}

//------------------------------------------------------------------------------
//
gosFX::SpinningCloud__Specification::SpinningCloud__Specification(
	Stuff::RegisteredClass::ClassID class_id
):
	ParticleCloud__Specification(class_id)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
	m_randomStartingRotation = false;
	m_alignZUsingX = false;
	m_alignZUsingY = false;
	m_alignYUsingVelocity = false;
}

//------------------------------------------------------------------------------
//
void
	gosFX::SpinningCloud__Specification::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);
	ParticleCloud__Specification::Save(stream);

	m_pSpin.Save(stream);
	m_pScale.Save(stream);

	*stream << m_randomStartingRotation << m_alignZUsingX << m_alignZUsingY;
	*stream << m_alignYUsingVelocity;
}

//------------------------------------------------------------------------------
//
void 
	gosFX::SpinningCloud__Specification::BuildDefaults()
{

	Check_Object(this);
	ParticleCloud__Specification::BuildDefaults();

	m_randomStartingRotation = false;
	m_alignZUsingX = false;
	m_alignZUsingY = false;
	m_alignYUsingVelocity = false;

	m_pSpin.m_ageCurve.SetCurve(0.0f);
	m_pSpin.m_seeded = false;
	m_pSpin.m_seedCurve.SetCurve(1.0f);

	m_pScale.m_ageCurve.SetCurve(1.0f);
	m_pScale.m_seeded = false;
	m_pScale.m_seedCurve.SetCurve(1.0f);

}


//------------------------------------------------------------------------------
//
bool 
	gosFX::SpinningCloud__Specification::IsDataValid(bool fix_data)
{

	Check_Object(this);
	Stuff::Scalar min,max;
	m_pScale.ExpensiveComputeRange(&min,&max);
	if(min<0.0f) 
	if(fix_data)
	{
	m_pScale.m_ageCurve.SetCurve(1.0f);
	m_pScale.m_seeded = false;
	m_pScale.m_seedCurve.SetCurve(1.0f);
	PAUSE(("Warning: Curve \"pScale\" in Effect \"%s\" Is Out of Range and has been Reset",(char *)m_name));

	}
	else
		return false;
	return ParticleCloud__Specification::IsDataValid(fix_data);
}

//------------------------------------------------------------------------------
//
void
	gosFX::SpinningCloud__Specification::Copy(SpinningCloud__Specification *spec)
{
	Check_Object(this);
	Check_Object(spec);

	ParticleCloud__Specification::Copy(spec);

	gos_PushCurrentHeap(Heap);
	m_pSpin = spec->m_pSpin;
	m_pScale = spec->m_pScale;

	m_randomStartingRotation = spec->m_randomStartingRotation;
	m_alignZUsingX = spec->m_alignZUsingX;
	m_alignZUsingY = spec->m_alignZUsingY;
	m_alignYUsingVelocity = spec->m_alignYUsingVelocity;

	gos_PopCurrentHeap();
}

//############################################################################
//##############################  gosFX::SpinningCloud  ################################
//############################################################################

gosFX::SpinningCloud::ClassData*
	gosFX::SpinningCloud::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::SpinningCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == Heap);
	DefaultData =
		new ClassData(
			SpinningCloudClassID,
			"gosFX::SpinningCloud",
			ParticleCloud::DefaultData,
			NULL,
			NULL
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::SpinningCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//------------------------------------------------------------------------------
//
gosFX::SpinningCloud::SpinningCloud(
	ClassData *class_data,
	Specification *spec,
	unsigned flags
):
	ParticleCloud(class_data, spec, flags)
{
	Verify(gos_GetCurrentHeap() == Heap);
}

//------------------------------------------------------------------------------
//
bool
	gosFX::SpinningCloud::Execute(ExecuteInfo *info)
{
	Check_Object(this);
	Check_Object(info);

	//
	//----------------------------------------
	// If we aren't supposed to execute, don't
	//----------------------------------------
	//
	if (!IsExecuted())
		return false;

	//
	//------------------------------------------------------------------
	// Animate the particles.  If it is time for us to die, return false
	//------------------------------------------------------------------
	//
	if (!ParticleCloud::Execute(info))
		return false;

	//
	//-----------------------------------------------------------------------
	// If there are active particles to animate, get the current center point
	// of the bounds
	//-----------------------------------------------------------------------
	//
	if (m_activeParticleCount > 0)
	{
		Stuff::ExtentBox box(Stuff::Point3D::Identity, Stuff::Point3D::Identity);
		unsigned i=0;

		//
		//-------------------------------------------------------------------
		// If there is no bounds yet, we need to create our extent box around
		// the first legal point we find
		//-------------------------------------------------------------------
		//
		while (i<m_activeParticleCount)
		{
			Particle *particle = GetParticle(i++);
			Check_Object(particle);

			//
			//-----------------------------------------------------------
			// We have found our first particle, so put the box around it
			//-----------------------------------------------------------
			//
			if (particle->m_age < 1.0f)
			{
				box.maxX =
					particle->m_localTranslation.x
					 + particle->m_radius*particle->m_scale;
				box.minX =
					particle->m_localTranslation.x
					 - particle->m_radius*particle->m_scale;
				box.maxY =
					particle->m_localTranslation.y
					 + particle->m_radius*particle->m_scale;
				box.minY =
					particle->m_localTranslation.y
					 - particle->m_radius*particle->m_scale;
				box.maxZ =
					particle->m_localTranslation.z
					 + particle->m_radius*particle->m_scale;
				box.minZ =
					particle->m_localTranslation.z
					 - particle->m_radius*particle->m_scale;
				break;
			}
		}

		//
		//-----------------------------
		// Look for the other particles
		//-----------------------------
		//
		while (i<m_activeParticleCount)
		{
			Particle *particle = GetParticle(i++);
			Check_Object(particle);
			if (particle->m_age < 1.0f)
			{
				Stuff::ExtentBox local_box;
				local_box.minX =
					particle->m_localTranslation.x
					 - particle->m_radius*particle->m_scale;
				local_box.maxX =
					particle->m_localTranslation.x
					 + particle->m_radius*particle->m_scale;
				local_box.minY =
					particle->m_localTranslation.y
					 - particle->m_radius*particle->m_scale;
				local_box.maxY =
					particle->m_localTranslation.y
					 + particle->m_radius*particle->m_scale;
				local_box.minZ =
					particle->m_localTranslation.z
					 - particle->m_radius*particle->m_scale;
				local_box.maxZ =
					particle->m_localTranslation.z
					 + particle->m_radius*particle->m_scale;
				box.Union(box, local_box);
			}
		}

		//
		//------------------------------------
		// Now, build a info->m_bounds around this box
		//------------------------------------
		//
		Verify(box.maxX >= box.minX);
		Verify(box.maxY >= box.minY);
		Verify(box.maxZ >= box.minZ);
		Stuff::OBB local_bounds = Stuff::OBB::Identity;
		local_bounds.axisExtents.x = 0.5f * (box.maxX - box.minX);
		local_bounds.axisExtents.y = 0.5f * (box.maxY - box.minY);
		local_bounds.axisExtents.z = 0.5f * (box.maxZ - box.minZ);
		local_bounds.localToParent(3,0) = box.minX + local_bounds.axisExtents.x;
		local_bounds.localToParent(3,1) = box.minY + local_bounds.axisExtents.y;
		local_bounds.localToParent(3,2) = box.minZ + local_bounds.axisExtents.z;
		local_bounds.sphereRadius = local_bounds.axisExtents.GetLength();
		if (local_bounds.sphereRadius < Stuff::SMALL)
			local_bounds.sphereRadius = 0.01f;
		Stuff::OBB parent_bounds;
		parent_bounds.Multiply(local_bounds, m_localToParent);
		info->m_bounds->Union(*info->m_bounds, parent_bounds);
	}

	//
	//----------------------------------------------
	// Tell our caller that we get to keep executing
	//----------------------------------------------
	//
	return true;
}

//------------------------------------------------------------------------------
//
void
	gosFX::SpinningCloud::CreateNewParticle(
		unsigned index,
		Stuff::Point3D *translation
	)
{
	//
	//---------------------------
	// Let our parent do creation
	//---------------------------
	//
	ParticleCloud::CreateNewParticle(index, translation);

	//
	//--------------------------------------
	// Figure out where the particle is born
	//--------------------------------------
	//
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Particle *particle = GetParticle(index);
	Check_Object(particle);
	Stuff::Scalar seed = particle->m_seed;
	Stuff::Scalar age = m_age;
	particle->m_localTranslation = *translation;

	//
	//---------------------------------
	// Figure out the particle rotation
	//---------------------------------
	//
	particle->m_angularVelocity.x = spec->m_pSpin.ComputeValue(age, seed);
	particle->m_angularVelocity.y = spec->m_pSpin.ComputeValue(age, seed);
	particle->m_angularVelocity.z = spec->m_pSpin.ComputeValue(age, seed);

	//
	//-----------------------------------------------------
	// If we are aligning with velocity, deal with that now
	//-----------------------------------------------------
	//
	if (spec->m_alignYUsingVelocity)
	{
		Stuff::LinearMatrix4D basis(true);
		basis.AlignLocalAxisToWorldVector(
			particle->m_localLinearVelocity,
			Stuff::Y_Axis,
			Stuff::X_Axis,
			Stuff::Z_Axis
		);
		particle->m_localRotation = basis;
	}
	else if (spec->m_randomStartingRotation)
	{
		Stuff::EulerAngles
			rotation(
				Stuff::Two_Pi*Stuff::Random::GetFraction(),
				Stuff::Pi*Stuff::Random::GetFraction(),
				Stuff::Two_Pi*Stuff::Random::GetFraction()
			);
		particle->m_localRotation = rotation;
	}
	else
		particle->m_localRotation = Stuff::UnitQuaternion::Identity;
}

//------------------------------------------------------------------------------
//
bool
	gosFX::SpinningCloud::AnimateParticle(
		unsigned index,
		const Stuff::LinearMatrix4D *world_to_new_local,
		Stuff::Time till
	)
{
	Check_Object(this);

	//
	//-----------------------------------------------------------------------
	// If this cloud is unparented, we need to transform the point from local
	// space into world space and set the internal position/velocity pointers
	// to these temporary values
	//-----------------------------------------------------------------------
	//
	Particle *particle = GetParticle(index);
	Check_Object(particle);
	Stuff::Scalar age = particle->m_age;
	if (age >= 1.0f)
		return false;
	Stuff::Point3D *translation = &particle->m_localTranslation;
	Stuff::UnitQuaternion *rotation = &particle->m_localRotation;
	Stuff::Vector3D *velocity = &particle->m_localLinearVelocity;
	int sim_mode = GetSimulationMode();
	if (sim_mode == DynamicWorldSpaceSimulationMode)
	{
		particle->m_worldTranslation.Multiply(*translation, m_localToWorld);
		Stuff::LinearMatrix4D local_rot(*rotation);
		Stuff::LinearMatrix4D world_rot;
		world_rot.Multiply(local_rot, m_localToWorld);
		particle->m_worldRotation = world_rot;
		particle->m_worldLinearVelocity.Multiply(*velocity, m_localToWorld);
		translation = &particle->m_worldTranslation;
		rotation = &particle->m_worldRotation;
		velocity = &particle->m_worldLinearVelocity;
	}
	Check_Object(translation);
	Check_Object(rotation);
	Check_Object(velocity);

	//
	//------------------------------------------------------------------
	// First, calculate the drag on the particle.  Drag can never assist
	// velocity
	//------------------------------------------------------------------
	//
	Stuff::Scalar seed = particle->m_seed;
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Stuff::Scalar drag = -spec->m_pDrag.ComputeValue(age, seed);
	Max_Clamp(drag, 0.0f);
	Stuff::Vector3D ether;
	ether.x = spec->m_pEtherVelocityX.ComputeValue(age, seed);
	ether.y = spec->m_pEtherVelocityY.ComputeValue(age, seed);
	ether.z = spec->m_pEtherVelocityZ.ComputeValue(age, seed);
	Stuff::Vector3D accel(Stuff::Vector3D::Identity);

	//
	//-------------------------------------------------------------------
	// Deal with pseudo-world simulation.  In this mode, we interpret the
	// forces as if they are already in worldspace, and we transform them
	// back to local space
	//-------------------------------------------------------------------
	//
	if (sim_mode == StaticWorldSpaceSimulationMode)
	{
		Stuff::LinearMatrix4D world_to_effect;
		world_to_effect.Invert(m_localToWorld);
		Stuff::Vector3D local_ether;
		local_ether.MultiplyByInverse(ether, world_to_effect);
		Stuff::Vector3D rel_vel;
		rel_vel.Subtract(*velocity, local_ether);
		accel.Multiply(rel_vel, drag);

		//
		//-----------------------------------------
		// Now, add in acceleration of the particle
		//-----------------------------------------
		//
		Stuff::Vector3D world_accel;
		world_accel.x = spec->m_pAccelerationX.ComputeValue(age, seed);
		world_accel.y = spec->m_pAccelerationY.ComputeValue(age, seed);
		world_accel.z = spec->m_pAccelerationZ.ComputeValue(age, seed);
		Stuff::Vector3D local_accel;
		local_accel.Multiply(world_accel, world_to_effect);
		accel += local_accel;
	}

	//
	//----------------------------------------------------------------------
	// Otherwise, just add the forces in the same space the particles are in
	//----------------------------------------------------------------------
	//
	else
	{
		Stuff::Vector3D rel_vel;
		rel_vel.Subtract(*velocity, ether);
		accel.Multiply(rel_vel, drag);

		//
		//-----------------------------------------
		// Now, add in acceleration of the particle
		//-----------------------------------------
		//
		accel.x += spec->m_pAccelerationX.ComputeValue(age, seed);
		accel.y += spec->m_pAccelerationY.ComputeValue(age, seed);
		accel.z += spec->m_pAccelerationZ.ComputeValue(age, seed);
	}

	//
	//-------------------------------------------------
	// Compute the particle's new velocity and position
	//-------------------------------------------------
	//
	Stuff::Scalar time_slice =
		static_cast<Stuff::Scalar>(till - m_lastRan);
	velocity->AddScaled(*velocity, accel, time_slice);
	translation->AddScaled(*translation, *velocity, time_slice);

	//
	//-----------------------
	// Deal with the rotation
	//-----------------------
	//
	if (!spec->m_alignYUsingVelocity)
	{
		Stuff::Vector3D omega(particle->m_angularVelocity);
		omega *= time_slice;
		Stuff::UnitQuaternion omega_q;
		omega_q = omega;
		rotation->Multiply(omega_q, Stuff::UnitQuaternion(*rotation));
		rotation->Normalize();
	}

	//
	//---------------------------------------------------------------------
	// If we are unparented, we need to transform the velocity and position
	// data back into the NEW local space
	//---------------------------------------------------------------------
	//
	if (sim_mode == DynamicWorldSpaceSimulationMode)
	{
		Check_Object(world_to_new_local);
		particle->m_localLinearVelocity.Multiply(*velocity, *world_to_new_local);
		particle->m_localTranslation.Multiply(*translation, *world_to_new_local);
		Stuff::LinearMatrix4D world_rot(*rotation);
		Stuff::LinearMatrix4D local_rot;
		local_rot.Multiply(world_rot, *world_to_new_local);
		particle->m_localRotation = local_rot;
	}

	//
	//------------------------------------------------------------
	// If we are aligning Y using velocity, do the calculation now
	//------------------------------------------------------------
	//
	if (spec->m_alignYUsingVelocity)
	{
		Stuff::LinearMatrix4D basis(true);
		basis.AlignLocalAxisToWorldVector(
			particle->m_localLinearVelocity,
			Stuff::Y_Axis,
			Stuff::X_Axis,
			Stuff::Z_Axis
		);
		particle->m_localRotation = basis;
	}

	//
	//------------------
	// Animate the scale
	//------------------
	//
	particle->m_scale = spec->m_pScale.ComputeValue(age, seed);
	return true;
}

//------------------------------------------------------------------------------
//
void
	gosFX::SpinningCloud::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}
