#include"gosfxheaders.hpp"
#include<mlr/mlrpointcloud.hpp>

//==========================================================================//
// File:	 gosFX_PointCloud.cpp											//
// Contents: Base gosFX::PointCloud Component								//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
//############################################################################
//########################  gosFX::PointCloud__Specification  #############################
//############################################################################

//------------------------------------------------------------------------------
//
gosFX::PointCloud__Specification::PointCloud__Specification(
	Stuff::MemoryStream *stream,
	int gfx_version
):
	ParticleCloud__Specification(gosFX::PointCloudClassID, stream, gfx_version)
{
	Check_Pointer(this);
	Verify(m_class == PointCloudClassID);
	Verify(gos_GetCurrentHeap() == Heap);
	m_totalParticleSize = gosFX::PointCloud::ParticleSize;
	m_particleClassSize = sizeof(gosFX::PointCloud::Particle);
}

//------------------------------------------------------------------------------
//
gosFX::PointCloud__Specification::PointCloud__Specification():
	ParticleCloud__Specification(gosFX::PointCloudClassID)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
	m_totalParticleSize = gosFX::PointCloud::ParticleSize;
	m_particleClassSize = sizeof(gosFX::PointCloud::Particle);
}

//------------------------------------------------------------------------------
//
gosFX::PointCloud__Specification*
	gosFX::PointCloud__Specification::Make(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	PointCloud__Specification *spec =
		new gosFX::PointCloud__Specification(stream, gfx_version);
	gos_PopCurrentHeap();

	return spec;
}

//############################################################################
//##############################  gosFX::PointCloud  ################################
//############################################################################

gosFX::PointCloud::ClassData*
	gosFX::PointCloud::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::PointCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == Heap);
	DefaultData =
		new ClassData(
			PointCloudClassID,
			"gosFX::PointCloud",
			ParticleCloud::DefaultData,
			(Effect::Factory)&Make,
			(Specification::Factory)&Specification::Make
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//------------------------------------------------------------------------------
//
gosFX::PointCloud::PointCloud(
	Specification *spec,
	unsigned flags
):
	ParticleCloud(DefaultData, spec, flags)
{
	Check_Object(spec);
	Verify(gos_GetCurrentHeap() == Heap);

	gos_PushCurrentHeap(MidLevelRenderer::Heap);
	m_cloudImplementation =
		new MidLevelRenderer::MLRPointCloud(spec->m_maxParticleCount);
	Register_Object(m_cloudImplementation);
	gos_PopCurrentHeap();

	unsigned index = spec->m_maxParticleCount*sizeof(Particle);
	m_P_localTranslation = Cast_Pointer(Stuff::Point3D*, &m_data[index]);
	index += spec->m_maxParticleCount*sizeof(Stuff::Point3D);
	m_P_color = Cast_Pointer(Stuff::RGBAColor*, &m_data[index]);

	m_cloudImplementation->SetData(
		Cast_Pointer(const int *, &m_activeParticleCount),
		m_P_localTranslation,
		m_P_color
	);
}

//------------------------------------------------------------------------------
//
gosFX::PointCloud::~PointCloud()
{
	Unregister_Object(m_cloudImplementation);
	delete m_cloudImplementation;
}

//------------------------------------------------------------------------------
//
gosFX::PointCloud*
	gosFX::PointCloud::Make(
		Specification *spec,
		unsigned flags
	)
{
	Check_Object(spec);

	gos_PushCurrentHeap(Heap);
	PointCloud *cloud = new gosFX::PointCloud(spec, flags);
	gos_PopCurrentHeap();

	return cloud;
}

//------------------------------------------------------------------------------
//
bool
	gosFX::PointCloud::Execute(ExecuteInfo *info)
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
		Stuff::Point3D *vertex = &m_P_localTranslation[0];
		unsigned i=0;

		//
		//-------------------------------------------------------------------
		// If there is no bounds yet, we need to create our extent box around
		// the first legal point we find
		//-------------------------------------------------------------------
		//
		while (i<m_activeParticleCount)
		{
			Particle *particle = GetParticle(i);
			Check_Object(particle);
			if (particle->m_age < 1.0f)
			{
				Check_Object(vertex);
				box.maxX = vertex->x;
				box.minX = vertex->x;
				box.maxY = vertex->y;
				box.minY = vertex->y;
				box.maxZ = vertex->z;
				box.minZ = vertex->z;
				++vertex;
				++i;
				break;
			}
			++vertex;
			++i;
		}

		//
		//-----------------------------
		// Look for the other particles
		//-----------------------------
		//
		while (i<m_activeParticleCount)
		{
			Particle *particle = GetParticle(i);
			Check_Object(particle);
			if (particle->m_age < 1.0f)
			{
				Check_Object(vertex);
				if (vertex->x > box.maxX)
					box.maxX = vertex->x;
				else if (vertex->x < box.minX)
					box.minX = vertex->x;

				if (vertex->y > box.maxY)
					box.maxY = vertex->y;
				else if (vertex->y < box.minY)
					box.minY = vertex->y;

				if (vertex->z > box.maxZ)
					box.maxZ = vertex->z;
				else if (vertex->z < box.minZ)
					box.minZ = vertex->z;
			}
			++vertex;
			++i;
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
	gosFX::PointCloud::CreateNewParticle(
		unsigned index,
		Stuff::Point3D *translation
	)
{
	Check_Object(this);
	Check_Pointer(translation);

	//
	//-----------------------------------------------------------------------
	// Let our parent do creation, then turn on the particle in the cloud and
	// set its position
	//-----------------------------------------------------------------------
	//
	ParticleCloud::CreateNewParticle(index, translation);
	m_cloudImplementation->TurnOn(index);
	Verify(m_cloudImplementation->IsOn(index));
	m_P_localTranslation[index] = *translation;
}

//------------------------------------------------------------------------------
//
bool
	gosFX::PointCloud::AnimateParticle(
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
	Set_Statistic(Point_Count, Point_Count+1);
	Stuff::Vector3D *velocity = &particle->m_localLinearVelocity;
	Stuff::Point3D *translation = &m_P_localTranslation[index];
	int sim_mode = GetSimulationMode();
	if (sim_mode == DynamicWorldSpaceSimulationMode)
	{
		Check_Object(translation);
		Check_Object(velocity);
		particle->m_worldLinearVelocity.Multiply(*velocity, m_localToWorld);
		particle->m_worldTranslation.Multiply(*translation, m_localToWorld);
		translation = &particle->m_worldTranslation;
		velocity = &particle->m_worldLinearVelocity;
	}
	Check_Object(translation);
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
	Stuff::Scalar drag = -spec->m_pDrag.ComputeValue(age, seed);
	Max_Clamp(drag, 0.0f);
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
	//---------------------------------------------------------------------
	// If we are unparented, we need to transform the velocity and position
	// data back into the NEW local space
	//---------------------------------------------------------------------
	//
	if (sim_mode == DynamicWorldSpaceSimulationMode)
	{
		Check_Object(world_to_new_local);
		particle->m_localLinearVelocity.Multiply(
			particle->m_worldLinearVelocity,
			*world_to_new_local
		);
		m_P_localTranslation[index].Multiply(
			particle->m_worldTranslation,
			*world_to_new_local
		);
	}

	//
	//------------------
	// Animate the color
	//------------------
	//
	Check_Pointer(m_P_color);
	m_P_color[index].red = spec->m_pRed.ComputeValue(age, seed);
	m_P_color[index].green = spec->m_pGreen.ComputeValue(age, seed);
	m_P_color[index].blue = spec->m_pBlue.ComputeValue(age, seed);
	m_P_color[index].alpha = spec->m_pAlpha.ComputeValue(age, seed);
	return true;
}

//------------------------------------------------------------------------------
//
void gosFX::PointCloud::DestroyParticle(unsigned index)
{
	Check_Object(this);

	m_cloudImplementation->TurnOff(index);
	Verify(!m_cloudImplementation->IsOn(index));
	ParticleCloud::DestroyParticle(index);
}

//------------------------------------------------------------------------------
//
void gosFX::PointCloud::Draw(DrawInfo *info)
{
	Check_Object(this);
	Check_Object(info);

	if (m_activeParticleCount)
	{
		MidLevelRenderer::DrawEffectInformation dInfo;
		dInfo.effect = m_cloudImplementation;
		Specification *spec = GetSpecification();
		Check_Object(spec);
		dInfo.state.Combine(info->m_state, spec->m_state);
		dInfo.clippingFlags = info->m_clippingFlags;
		Stuff::LinearMatrix4D local_to_world;
		local_to_world.Multiply(m_localToParent, *info->m_parentToWorld);
		dInfo.effectToWorld = &local_to_world;
	
	 	info->m_clipper->DrawEffect(&dInfo);
	}
	ParticleCloud::Draw(info);
}

//------------------------------------------------------------------------------
//
void
	gosFX::PointCloud::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}
