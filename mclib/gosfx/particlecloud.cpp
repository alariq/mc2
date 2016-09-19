#include"gosfxheaders.hpp"

//==========================================================================//
// File:	 gosFX_ParticleCloud.cpp										//
// Contents: Base gosFX::ParticleCloud Component							//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
//############################################################################
//########################  gosFX::ParticleCloud__Specification  #############################
//############################################################################

//------------------------------------------------------------------------------
//
gosFX::ParticleCloud__Specification::ParticleCloud__Specification(
	Stuff::RegisteredClass::ClassID class_id,
	Stuff::MemoryStream *stream,
	int gfx_version
):
	Effect__Specification(class_id, stream, gfx_version)
{
	Check_Pointer(this);
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	//
	//-------------------
	// Load in the curves
	//-------------------
	//
	m_startingPopulation.Load(stream, gfx_version);
	m_particlesPerSecond.Load(stream, gfx_version);
	m_emitterSizeX.Load(stream, gfx_version);
	m_emitterSizeY.Load(stream, gfx_version);
	m_emitterSizeZ.Load(stream, gfx_version);
	m_minimumDeviation.Load(stream, gfx_version);
	m_maximumDeviation.Load(stream, gfx_version);
	m_startingSpeed.Load(stream, gfx_version);
	m_pLifeSpan.Load(stream, gfx_version);
	m_pEtherVelocityX.Load(stream, gfx_version);
	m_pEtherVelocityY.Load(stream, gfx_version);
	m_pEtherVelocityZ.Load(stream, gfx_version);
	m_pAccelerationX.Load(stream, gfx_version);
	m_pAccelerationY.Load(stream, gfx_version);
	m_pAccelerationZ.Load(stream, gfx_version);
	m_pDrag.Load(stream, gfx_version);
	m_pRed.Load(stream, gfx_version);
	m_pGreen.Load(stream, gfx_version);
	m_pBlue.Load(stream, gfx_version);
	m_pAlpha.Load(stream, gfx_version);

	*stream >> m_maxParticleCount;
}

//------------------------------------------------------------------------------
//
gosFX::ParticleCloud__Specification::ParticleCloud__Specification(
	Stuff::RegisteredClass::ClassID class_id
):
	Effect__Specification(class_id)
{
	Verify(gos_GetCurrentHeap() == Heap);
	m_maxParticleCount = 0;
	Check_Pointer(this);
}

//------------------------------------------------------------------------------
//
void
	gosFX::ParticleCloud__Specification::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);
	Effect__Specification::Save(stream);

	//
	//----------------
	// Save our curves
	//----------------
	//
	m_startingPopulation.Save(stream);
	m_particlesPerSecond.Save(stream);
	m_emitterSizeX.Save(stream);
	m_emitterSizeY.Save(stream);
	m_emitterSizeZ.Save(stream);
	m_minimumDeviation.Save(stream);
	m_maximumDeviation.Save(stream);
	m_startingSpeed.Save(stream);
	m_pLifeSpan.Save(stream);
	m_pEtherVelocityX.Save(stream);
	m_pEtherVelocityY.Save(stream);
	m_pEtherVelocityZ.Save(stream);
	m_pAccelerationX.Save(stream);
	m_pAccelerationY.Save(stream);
	m_pAccelerationZ.Save(stream);
	m_pDrag.Save(stream);
	m_pRed.Save(stream);
	m_pGreen.Save(stream);
	m_pBlue.Save(stream);
	m_pAlpha.Save(stream);

	*stream << m_maxParticleCount;
}

//------------------------------------------------------------------------------
//
void 
	gosFX::ParticleCloud__Specification::BuildDefaults()
{

	Check_Object(this);
	Effect__Specification::BuildDefaults();

	m_startingPopulation.SetCurve(0.0f);
	m_particlesPerSecond.SetCurve(5.0f);

	m_emitterSizeX.SetCurve(0.0f);
	m_emitterSizeY.SetCurve(0.0f);
	m_emitterSizeZ.SetCurve(0.0f);

	m_minimumDeviation.SetCurve(0.0f);
	m_maximumDeviation.SetCurve(0.0f);

	m_startingSpeed.m_ageCurve.SetCurve(1.0f);
	m_startingSpeed.m_seeded = false;
	m_startingSpeed.m_seedCurve.SetCurve(1.0f);

	m_pLifeSpan.m_ageCurve.SetCurve(1.0f);
	m_pLifeSpan.m_seeded = false;
	m_pLifeSpan.m_seedCurve.SetCurve(1.0f);

	m_pAccelerationX.m_ageCurve.SetCurve(0.0f);
	m_pAccelerationX.m_seeded = false;
	m_pAccelerationX.m_seedCurve.SetCurve(1.0f);

	m_pAccelerationY.m_ageCurve.SetCurve(0.0f);
	m_pAccelerationY.m_seeded = false;
	m_pAccelerationY.m_seedCurve.SetCurve(1.0f);

	m_pAccelerationZ.m_ageCurve.SetCurve(0.0f);
	m_pAccelerationZ.m_seeded = false;
	m_pAccelerationZ.m_seedCurve.SetCurve(1.0f);

	m_pEtherVelocityX.m_ageCurve.SetCurve(0.0f);
	m_pEtherVelocityX.m_seeded = false;
	m_pEtherVelocityX.m_seedCurve.SetCurve(1.0f);

	m_pEtherVelocityY.m_ageCurve.SetCurve(0.0f);
	m_pEtherVelocityY.m_seeded = false;
	m_pEtherVelocityY.m_seedCurve.SetCurve(1.0f);

	m_pEtherVelocityZ.m_ageCurve.SetCurve(0.0f);
	m_pEtherVelocityZ.m_seeded = false;
	m_pEtherVelocityZ.m_seedCurve.SetCurve(1.0f);

	m_pDrag.m_ageCurve.SetCurve(0.0f);
	m_pDrag.m_seeded = false;
	m_pDrag.m_seedCurve.SetCurve(1.0f);

	m_pRed.m_ageCurve.SetCurve(1.0f);
	m_pRed.m_seeded = false;
	m_pRed.m_seedCurve.SetCurve(1.0f);

	m_pGreen.m_ageCurve.SetCurve(1.0f);
	m_pGreen.m_seeded = false;
	m_pGreen.m_seedCurve.SetCurve(1.0f);

	m_pBlue.m_ageCurve.SetCurve(1.0f);
	m_pBlue.m_seeded = false;
	m_pBlue.m_seedCurve.SetCurve(1.0f);

	m_pAlpha.m_ageCurve.SetCurve(1.0f);
	m_pAlpha.m_seeded = false;
	m_pAlpha.m_seedCurve.SetCurve(1.0f);

}


//------------------------------------------------------------------------------
//
bool 
	gosFX::ParticleCloud__Specification::IsDataValid(bool fix_data)
{

	Check_Object(this);
	Stuff::Scalar min,max;
	  m_pLifeSpan.ExpensiveComputeRange(&min,&max);
	  if(min<0.0f) 
	  {
		if(fix_data)
		{
		m_pLifeSpan.m_ageCurve.SetCurve(1.0f);
		m_pLifeSpan.m_seeded = false;
		m_pLifeSpan.m_seedCurve.SetCurve(1.0f);
		PAUSE(("Warning: Curve \"pLifeSpan\" in Effect \"%s\" Is Out of Range and has been Reset",(char *)m_name));

		}
		else
		return false;
	  }
	 
	  m_startingPopulation.ExpensiveComputeRange(&min,&max);
	  if(min<0.0f) 
			if(fix_data)
			{
				m_startingPopulation.SetCurve(0.0f);
				PAUSE(("Warning: Curve \"startingPopulation\" in Effect \"%s\" Is Out of Range and has been Reset",(char *)m_name));

			}
			else
				return false;
	
	if(m_maxParticleCount<1) 
		if(fix_data)
		{
			m_maxParticleCount=1;
			PAUSE(("Warning: Value \"maxParticleCount\" in Effect \"%s\" Is Out of Range and has been Reset",(char *)m_name));

		}
		else
			return false;
	
	return	Effect__Specification::IsDataValid(fix_data);
}

//------------------------------------------------------------------------------
//
void
	gosFX::ParticleCloud__Specification::Copy(ParticleCloud__Specification *spec)
{
	Check_Object(this);
	Check_Object(spec);
	Verify(gos_GetCurrentHeap() == Heap);

	Effect__Specification::Copy(spec);

	//
	//----------------
	// Copy the curves
	//----------------
	//
	gos_PushCurrentHeap(Heap);
	m_startingPopulation = spec->m_startingPopulation;
	m_particlesPerSecond = spec->m_particlesPerSecond;
	m_emitterSizeX = spec->m_emitterSizeX;
	m_emitterSizeY = spec->m_emitterSizeY;
	m_emitterSizeZ = spec->m_emitterSizeZ;
	m_minimumDeviation = spec->m_minimumDeviation;
	m_maximumDeviation = spec->m_maximumDeviation;
	m_startingSpeed = spec->m_startingSpeed;
	m_pLifeSpan = spec->m_pLifeSpan;
	m_pEtherVelocityX = spec->m_pEtherVelocityX;
	m_pEtherVelocityY = spec->m_pEtherVelocityY;
	m_pEtherVelocityZ = spec->m_pEtherVelocityZ;
	m_pAccelerationX = spec->m_pAccelerationX;
	m_pAccelerationY = spec->m_pAccelerationY;
	m_pAccelerationZ = spec->m_pAccelerationZ;
	m_pDrag = spec->m_pDrag;
	m_pRed = spec->m_pRed;
	m_pGreen = spec->m_pGreen;
	m_pBlue = spec->m_pBlue;
	m_pAlpha = spec->m_pAlpha;
	m_maxParticleCount = spec->m_maxParticleCount;
	gos_PopCurrentHeap();
}

//############################################################################
//############################  gosFX::ParticleCloud  ###############################
//############################################################################

gosFX::ParticleCloud::ClassData*
	gosFX::ParticleCloud::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::ParticleCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == Heap);
	DefaultData =
		new ClassData(
			ParticleCloudClassID,
			"gosFX::ParticleCloud",
			Effect::DefaultData,
			NULL,
			NULL
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::ParticleCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//------------------------------------------------------------------------------
//
gosFX::ParticleCloud::ParticleCloud(
	ClassData *class_data,
	Specification *spec,
	unsigned flags
):
	Effect(class_data, spec, flags)
{
	Check_Pointer(this);
	Check_Object(spec);
	Verify(gos_GetCurrentHeap() == Heap);

	//
	//------------------------------------------------
	// Set up the data pointers into the channel block
	//------------------------------------------------
	//
	m_data.SetLength(spec->m_maxParticleCount*spec->m_totalParticleSize);

	//
	//-------------------------------
	// Set up an empty particle cloud
	//-------------------------------
	//
	m_activeParticleCount = 0;
	m_birthAccumulator = 0.0f;
}

//------------------------------------------------------------------------------
//
void
	gosFX::ParticleCloud::Start(ExecuteInfo *info)
{
	Check_Object(this);
	Check_Pointer(info);

	//
	//--------------------------------------------------------------------------
	// Let effect initialize, then figure out how many particles we want to make
	//--------------------------------------------------------------------------
	//
	Effect::Start(info);
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Stuff::Scalar newbies =
		spec->m_startingPopulation.ComputeValue(m_age, m_seed);
	Min_Clamp(newbies, 0.0f);
	m_birthAccumulator += newbies;
}

//------------------------------------------------------------------------------
//
bool gosFX::ParticleCloud::Execute(ExecuteInfo *info)
{
	Check_Object(this);
	Check_Object(info);
	Verify(IsExecuted());

	//
	//--------------------------------------------------------------------
	// If we were given a new matrix, see if we have a parent.  If so,
	// concatenate the two and figure out its inverse.  If no parent, then
	// just invert the new matrix, otherwise just use the existing one
	//--------------------------------------------------------------------
	//
	Stuff::LinearMatrix4D new_world_to_local;
	Stuff::LinearMatrix4D *matrix = NULL;
	int sim_mode = GetSimulationMode();
	if (sim_mode == DynamicWorldSpaceSimulationMode)
	{
		Stuff::LinearMatrix4D local_to_world;
		local_to_world.Multiply(m_localToParent, *info->m_parentToWorld);
		new_world_to_local.Invert(local_to_world);
		matrix = &new_world_to_local;
	}

	//
	//--------------------------------------------------------
	// Figure out the birth rate and request the new particles
	//--------------------------------------------------------
	//
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Stuff::Scalar dT =
		static_cast<Stuff::Scalar>(info->m_time - m_lastRan);
	Verify(dT >= 0.0f);
	Stuff::Scalar prev_age = m_age;
	m_age += dT * m_ageRate;
	if (m_age >= 1.0f)
		m_birthAccumulator = 0.0f;
	else
	{
		Stuff::Scalar new_life =
			spec->m_particlesPerSecond.ComputeValue(m_age, m_seed);
		Min_Clamp(new_life, 0.0f);
		m_birthAccumulator += dT * new_life;
	}

	//
	//-----------------------------------
	// Deal with all the active particles
	//-----------------------------------
	//
	int i;
	int last_real = -1;
	for (i = 0; i < m_activeParticleCount; i++)
	{
		//
		//--------------------------------------------------------------------
		// If the particle is active, age it and if it is not yet time to die,
		// go to the next particle, otherwise kill it
		//--------------------------------------------------------------------
		//
		Particle *particle = GetParticle(i);
		Check_Object(particle);
		if (particle->m_age < 1.0f)
		{
			particle->m_age += dT*particle->m_ageRate;
			if (AnimateParticle(i, matrix, info->m_time))
			{
				last_real = i;
				continue;
			}
			DestroyParticle(i);
		}

		//
		//--------------------------------------------------------------------
		// If there are new particles to be born, go ahead and create them now
		//--------------------------------------------------------------------
		//
		if (m_birthAccumulator >= 1.0f)
		{
			Stuff::Point3D translation;
			CreateNewParticle(i, &translation);
			if (AnimateParticle(i, matrix, info->m_time))
				last_real = i;
			else
				DestroyParticle(i);
			m_birthAccumulator -= 1.0f;
		}
	}
	m_activeParticleCount = last_real + 1;

	//
	//----------------------------------------------------------------------
	// If there are still new particles to be born, then we must try to grow
	// the active particle count
	//----------------------------------------------------------------------
	//
	while (
		m_birthAccumulator >= 1.0f
		 && m_activeParticleCount < spec->m_maxParticleCount
	)
	{
		i = m_activeParticleCount++;
		Stuff::Point3D translation;
		CreateNewParticle(i, &translation);
		if (!AnimateParticle(i, matrix, info->m_time))
		{
			DestroyParticle(i);
			--m_activeParticleCount;
		}
		m_birthAccumulator -= 1.0f;
	}

	//
	//---------------------------------------------------------
	// Only allow fractional births to carry over to next frame
	//---------------------------------------------------------
	//
	m_birthAccumulator -= static_cast<Stuff::Scalar>(floor(m_birthAccumulator));

	//
	//----------------------------
	// Now let effect do its thing
	//----------------------------
	//
	m_age = prev_age;
	return Effect::Execute(info);
}

//------------------------------------------------------------------------------
//
bool gosFX::ParticleCloud::HasFinished()
{
	Check_Object(this);
	return Effect::HasFinished() && (m_activeParticleCount == 0);
}

//------------------------------------------------------------------------------
//
void gosFX::ParticleCloud::Kill()
{
	Check_Object(this);

	//
	//-------------------------------------------------------------
	// Destroy all the particles and set up an empty particle cloud
	//-------------------------------------------------------------
	//
	for(int i=0; i < m_activeParticleCount; i++)
		DestroyParticle(i);
	m_activeParticleCount = 0;
	m_birthAccumulator = 0.0f;

	//
	//----------------------------------------
	// Now let the base effect handle stopping
	//----------------------------------------
	//
	Effect::Kill();
}

//------------------------------------------------------------------------------
//
void
	gosFX::ParticleCloud::CreateNewParticle(
		unsigned index,
		Stuff::Point3D *translation
	)
{
	Check_Object(this);

	//
	//----------------------------------------------------
	// Figure out the age and age rate of the new particle
	//----------------------------------------------------
	//
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Particle *particle = GetParticle(index);
	Check_Object(particle);
	particle->m_age = 0.0f;
	Stuff::Scalar min_seed =
		spec->m_minimumChildSeed.ComputeValue(m_age, m_seed);
	Stuff::Scalar seed_range =
		spec->m_maximumChildSeed.ComputeValue(m_age, m_seed) - min_seed;
	Stuff::Scalar seed =
		Stuff::Random::GetFraction()*seed_range + min_seed;
	Clamp(seed, 0.0f, 1.0f);
	particle->m_seed = seed;
	Stuff::Scalar lifetime =
		spec->m_pLifeSpan.ComputeValue(m_age, seed);
	Min_Clamp(lifetime, 0.0333333f);
	particle->m_ageRate = 1.0f / lifetime;

	//
	//--------------------------------
	// Figure out the initial position
	//--------------------------------
	//
	Stuff::YawPitchRange
		initial_p(
			Stuff::Random::GetFraction() * Stuff::Two_Pi,
			Stuff::Random::GetFraction() * Stuff::Pi - Stuff::Pi_Over_2,
			Stuff::Random::GetFraction()
		);
	Stuff::Vector3D position(initial_p);
	translation->x =
		position.x * spec->m_emitterSizeX.ComputeValue(m_age, seed);
	translation->y =
		position.y * spec->m_emitterSizeY.ComputeValue(m_age, seed);
	translation->z =
		position.z * spec->m_emitterSizeZ.ComputeValue(m_age, seed);

	//
	//--------------------------------
	// Figure out the initial velocity
	//--------------------------------
	//
	Stuff::Scalar pitch_min =
		spec->m_minimumDeviation.ComputeValue(m_age, seed);
	Stuff::Scalar pitch_range =
		spec->m_maximumDeviation.ComputeValue(m_age, seed) - pitch_min;
	if (pitch_range < 0.0f)
		pitch_range = 0.0f;
	pitch_min +=
		pitch_range * Stuff::Random::GetFraction() - Stuff::Pi_Over_2;
	Stuff::YawPitchRange
		initial_v(
			Stuff::Random::GetFraction() * Stuff::Two_Pi,
			pitch_min,
			spec->m_startingSpeed.ComputeValue(m_age, seed)
		);
	particle->m_localLinearVelocity = initial_v;
}

//------------------------------------------------------------------------------
//
void gosFX::ParticleCloud::DestroyParticle(unsigned index)
{
	Particle *particle = GetParticle(index);
	Check_Object(particle);
	particle->m_age = 1.0f;
}

//------------------------------------------------------------------------------
//
void
	gosFX::ParticleCloud::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}
