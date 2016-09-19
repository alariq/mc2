#include"gosfxheaders.hpp"
#include<mlr/mlrngoncloud.hpp>

//==========================================================================//
// File:	 gosFX_gosFX::PertCloud.cpp										//
// Contents: Base gosFX::PertCloud Component								//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//------------------------------------------------------------------------------
//
gosFX::PertCloud__Specification::PertCloud__Specification(
	Stuff::MemoryStream *stream,
	int gfx_version
):
	SpinningCloud__Specification(gosFX::PertCloudClassID, stream, gfx_version)
{
	Check_Pointer(this);
	Verify(m_class == gosFX::PertCloudClassID);
	Verify(gos_GetCurrentHeap() == Heap);

	m_size.Load(stream, gfx_version);
	m_perturbation.Load(stream, gfx_version);
	m_pCenterRed.Load(stream, gfx_version);
	m_pCenterGreen.Load(stream, gfx_version);
	m_pCenterBlue.Load(stream, gfx_version);
	m_pCenterAlpha.Load(stream, gfx_version);

	m_particleClassSize = sizeof(gosFX::PertCloud::Particle);
	Verify(gfx_version > 5);
	*stream >> m_vertices;
	m_totalParticleSize =
		sizeof(gosFX::PertCloud::Particle)
		 + m_vertices * sizeof(Stuff::Point3D)
		 + 2 * sizeof(Stuff::RGBAColor);
}

//------------------------------------------------------------------------------
//
gosFX::PertCloud__Specification::PertCloud__Specification(unsigned sides):
	SpinningCloud__Specification(gosFX::PertCloudClassID)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
	m_particleClassSize = sizeof(gosFX::PertCloud::Particle);
	m_vertices = sides+2;
	m_totalParticleSize =
		sizeof(gosFX::PertCloud::Particle)
		 + m_vertices * sizeof(Stuff::Point3D)
		 + 2 * sizeof(Stuff::RGBAColor);
}

//------------------------------------------------------------------------------
//
gosFX::PertCloud__Specification*
	gosFX::PertCloud__Specification::Make(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	PertCloud__Specification *spec =
		new gosFX::PertCloud__Specification(stream, gfx_version);
	gos_PopCurrentHeap();

	return spec;
}

//------------------------------------------------------------------------------
//
void
	gosFX::PertCloud__Specification::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);
	SpinningCloud__Specification::Save(stream);

	m_size.Save(stream);
	m_perturbation.Save(stream);
	m_pCenterRed.Save(stream);
	m_pCenterGreen.Save(stream);
	m_pCenterBlue.Save(stream);
	m_pCenterAlpha.Save(stream);

	*stream << m_vertices;
}

//------------------------------------------------------------------------------
//
void 
	gosFX::PertCloud__Specification::BuildDefaults()
{

	Check_Object(this);
	SpinningCloud__Specification::BuildDefaults();

	m_pCenterRed.m_ageCurve.SetCurve(1.0f);
	m_pCenterRed.m_seeded = false;
	m_pCenterRed.m_seedCurve.SetCurve(1.0f);

	m_pCenterGreen.m_ageCurve.SetCurve(1.0f);
	m_pCenterGreen.m_seeded = false;
	m_pCenterGreen.m_seedCurve.SetCurve(1.0f);

	m_pCenterBlue.m_ageCurve.SetCurve(1.0f);
	m_pCenterBlue.m_seeded = false;
	m_pCenterBlue.m_seedCurve.SetCurve(1.0f);

	m_pCenterAlpha.m_ageCurve.SetCurve(1.0f);
	m_pCenterAlpha.m_seeded = false;
	m_pCenterAlpha.m_seedCurve.SetCurve(1.0f);

	m_size.m_ageCurve.SetCurve(1.0f);
	m_size.m_seeded = false;
	m_size.m_seedCurve.SetCurve(1.0f);

	m_perturbation.m_ageCurve.SetCurve(0.25f);
	m_perturbation.m_seeded = false;
	m_perturbation.m_seedCurve.SetCurve(1.0f);

}


//------------------------------------------------------------------------------
//
bool 
	gosFX::PertCloud__Specification::IsDataValid(bool fix_data)
{

	Check_Object(this);
	return	SpinningCloud__Specification::IsDataValid(fix_data);
/*
	m_pCenterRed.m_ageCurve.SetCurve(1.0f);
	m_pCenterRed.m_seeded = false;
	m_pCenterRed.m_seedCurve.SetCurve(1.0f);

	m_pCenterGreen.m_ageCurve.SetCurve(1.0f);
	m_pCenterGreen.m_seeded = false;
	m_pCenterGreen.m_seedCurve.SetCurve(1.0f);

	m_pCenterBlue.m_ageCurve.SetCurve(1.0f);
	m_pCenterBlue.m_seeded = false;
	m_pCenterBlue.m_seedCurve.SetCurve(1.0f);

	m_pCenterAlpha.m_ageCurve.SetCurve(1.0f);
	m_pCenterAlpha.m_seeded = false;
	m_pCenterAlpha.m_seedCurve.SetCurve(1.0f);

	m_size.m_ageCurve.SetCurve(1.0f);
	m_size.m_seeded = false;
	m_size.m_seedCurve.SetCurve(1.0f);

	m_perturbation.m_ageCurve.SetCurve(0.25f);
	m_perturbation.m_seeded = false;
	m_perturbation.m_seedCurve.SetCurve(1.0f);
*/

}

//------------------------------------------------------------------------------
//
void
	gosFX::PertCloud__Specification::Copy(PertCloud__Specification *spec)
{
	Check_Object(this);
	Check_Object(spec);

	SpinningCloud__Specification::Copy(spec);

	gos_PushCurrentHeap(Heap);
	m_size = spec->m_size;
	m_perturbation = spec->m_perturbation;
	m_pCenterRed = spec->m_pCenterRed;
	m_pCenterGreen = spec->m_pCenterGreen;
	m_pCenterBlue = spec->m_pCenterBlue;
	m_pCenterAlpha = spec->m_pCenterAlpha;
	m_vertices = spec->m_vertices;
	gos_PopCurrentHeap();
}

//############################################################################
//##############################  gosFX::PertCloud  ################################
//############################################################################

gosFX::PertCloud::ClassData*
	gosFX::PertCloud::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::PertCloud::InitializeClass()
{
	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			gosFX::PertCloudClassID,
			"gosFX::PertCloud",
			SpinningCloud::DefaultData,
			(Effect::Factory)&Make,
			(Specification::Factory)&Specification::Make
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::PertCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//------------------------------------------------------------------------------
//
gosFX::PertCloud::PertCloud(
	Specification *spec,
	unsigned flags
):
	SpinningCloud(DefaultData, spec, flags)
{
	Check_Object(spec);
	Verify(gos_GetCurrentHeap() == Heap);

	gos_PushCurrentHeap(MidLevelRenderer::Heap);
	m_cloudImplementation =
		new MidLevelRenderer::MLRNGonCloud(
			spec->m_vertices,
			spec->m_maxParticleCount
		);
	Register_Object(m_cloudImplementation);
	gos_PopCurrentHeap();

	unsigned index = spec->m_maxParticleCount*sizeof(Particle);
	m_P_vertices = Cast_Pointer(Stuff::Point3D*, &m_data[index]);
	index +=
		spec->m_vertices * spec->m_maxParticleCount * sizeof(Stuff::Point3D);
	m_P_color = Cast_Pointer(Stuff::RGBAColor*, &m_data[index]);
	
	m_cloudImplementation->SetData(
		Cast_Pointer(const int *, &m_activeParticleCount),
		m_P_vertices,
		m_P_color
	);
}

//------------------------------------------------------------------------------
//
gosFX::PertCloud::~PertCloud()
{
	Unregister_Object(m_cloudImplementation);
	delete m_cloudImplementation;
}

//------------------------------------------------------------------------------
//
gosFX::PertCloud*
	gosFX::PertCloud::Make(
		Specification *spec,
		unsigned flags
	)
{
	Check_Object(spec);

	gos_PushCurrentHeap(Heap);
	PertCloud *cloud = new gosFX::PertCloud(spec, flags);
	gos_PopCurrentHeap();

	return cloud;
}

//------------------------------------------------------------------------------
//
bool
	gosFX::PertCloud::AnimateParticle(
		unsigned index,
		const Stuff::LinearMatrix4D *world_to_new_local,
		Stuff::Time till
	)
{
	Check_Object(this);

	//
	//-----------------------------------------
	// Animate the parent then get our pointers
	//-----------------------------------------
	//
	if (!SpinningCloud::AnimateParticle(index, world_to_new_local, till))
		return false;
	Set_Statistic(Pert_Count, Pert_Count+1);
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Particle *particle = GetParticle(index);
	Check_Object(particle);
	Stuff::Scalar seed = particle->m_seed;
	Stuff::Scalar age = particle->m_age;

	//
	//------------------
	// Animate the color
	//------------------
	//
	Check_Pointer(m_P_color);
	index *= 2;
	m_P_color[index].red = spec->m_pCenterRed.ComputeValue(age, seed);
	m_P_color[index].green = spec->m_pCenterGreen.ComputeValue(age, seed);
	m_P_color[index].blue = spec->m_pCenterBlue.ComputeValue(age, seed);
	m_P_color[index].alpha = spec->m_pCenterAlpha.ComputeValue(age, seed);

	++index;
	m_P_color[index].red = spec->m_pRed.ComputeValue(age, seed);
	m_P_color[index].green = spec->m_pGreen.ComputeValue(age, seed);
	m_P_color[index].blue = spec->m_pBlue.ComputeValue(age, seed);
	m_P_color[index].alpha = spec->m_pAlpha.ComputeValue(age, seed);
	return true;
}

//------------------------------------------------------------------------------
//
void
	gosFX::PertCloud::CreateNewParticle(
		unsigned index,
		Stuff::Point3D *translation
	)
{
	Check_Object(this);

	//
	//-------------------------------------------------------------------
	// Let our parent do creation, then turn on the particle in the cloud
	//-------------------------------------------------------------------
	//
	SpinningCloud::CreateNewParticle(index, translation);
	m_cloudImplementation->TurnOn(index);
	Verify(m_cloudImplementation->IsOn(index));

	//
	//--------------------
	// Set up the particle
	//--------------------
	//
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Particle *particle = GetParticle(index);
	Check_Object(particle);

	//
	//----------------------------------------
	// Now we compute the geometry of the pert
	//----------------------------------------
	//
	Verify(spec->m_vertices > 4);
	Stuff::Scalar angle_between = Stuff::Two_Pi/(spec->m_vertices-2);
	Stuff::Scalar radius = spec->m_size.ComputeValue(m_age, particle->m_seed);
	int even = 1;
	particle->m_vertices[0] = Stuff::Point3D::Identity;
	Stuff::Scalar bound = 0.0f;
    int j=1;
	for (; j<spec->m_vertices-1; j++)
	{
		Stuff::Scalar perturbance =
			even * spec->m_perturbation.ComputeValue(m_age, particle->m_seed);
		Stuff::Scalar temp = perturbance + radius;
		particle->m_vertices[j] =
			Stuff::Point3D(
				Stuff::Sin(j*angle_between)*temp,
				Stuff::Cos(j*angle_between)*temp,
				perturbance
			);
		perturbance = temp*temp + perturbance*perturbance;
		if (perturbance > bound)
			bound = perturbance;
		even = -even;
	}
	particle->m_radius = Stuff::Sqrt(bound);
	particle->m_vertices[j] = particle->m_vertices[1];
}

//------------------------------------------------------------------------------
//
void gosFX::PertCloud::DestroyParticle(unsigned index)
{
	SpinningCloud::DestroyParticle(index);
	m_cloudImplementation->TurnOff(index);
	Verify(!m_cloudImplementation->IsOn(index));
}

//------------------------------------------------------------------------------
//
void gosFX::PertCloud::Draw(DrawInfo *info)
{
	Check_Object(this);
	Check_Object(info);

	//
	//---------------------------------------------------------
	// If we have active particles, set up the draw information
	//---------------------------------------------------------
	//
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

		//
		//--------------------------------------------------------------
		// Check the orientation mode.  The first case is XY orientation
		//--------------------------------------------------------------
		//
		unsigned i;
		unsigned vert=0;
		if (spec->m_alignZUsingX)
		{
			if (spec->m_alignZUsingY)
			{
				//
				//-----------------------------------------
				// Get the camera location into local space
				//-----------------------------------------
				//
				Stuff::Point3D
					camera_in_world(info->m_clipper->GetCameraToWorldMatrix());
				Stuff::Point3D camera_in_cloud;
				camera_in_cloud.MultiplyByInverse(
					camera_in_world,
					local_to_world
				);

				//
				//--------------------------------------
				// Spin through all the active particles
				//--------------------------------------
				//
				for (i = 0; i < m_activeParticleCount; i++)
				{
					Particle *particle = GetParticle(i);
					Check_Object(particle);
					if (particle->m_age < 1.0f)
					{

						//
						//--------------------------------
						// Build the local to cloud matrix
						//--------------------------------
						//
						Stuff::Vector3D direction_in_cloud;
						direction_in_cloud.Subtract(
							camera_in_cloud,
							particle->m_localTranslation
						);
						Stuff::LinearMatrix4D pert_to_cloud;
						pert_to_cloud.BuildRotation(particle->m_localRotation);
						pert_to_cloud.AlignLocalAxisToWorldVector(
							direction_in_cloud,
							Stuff::Z_Axis,
							Stuff::Y_Axis,
							Stuff::X_Axis
						);
						pert_to_cloud.BuildTranslation(particle->m_localTranslation);

						//
						//----------------------------------------------------
						// Figure out the scale, then transform all the points
						//----------------------------------------------------
						//
						Stuff::Scalar scale = particle->m_scale;
						for (unsigned v=0; v<spec->m_vertices; ++v)
						{
							Stuff::Point3D scaled;
							scaled.Multiply(particle->m_vertices[v], scale);
							m_P_vertices[vert++].Multiply(scaled, pert_to_cloud);
						}
					}
					else
						vert += spec->m_vertices;
				}
			}

			//
			//-----------------------
			// Handle X-only rotation
			//-----------------------
			//
			else
			{
				//
				//-----------------------------------------
				// Get the camera location into local space
				//-----------------------------------------
				//
				Stuff::Point3D
					camera_in_world(info->m_clipper->GetCameraToWorldMatrix());
				Stuff::Point3D camera_in_cloud;
				camera_in_cloud.MultiplyByInverse(
					camera_in_world,
					local_to_world
				);

				//
				//--------------------------------------
				// Spin through all the active particles
				//--------------------------------------
				//
				for (i = 0; i < m_activeParticleCount; i++)
				{
					Particle *particle = GetParticle(i);
					Check_Object(particle);
					if (particle->m_age < 1.0f)
					{

						//
						//--------------------------------
						// Build the local to cloud matrix
						//--------------------------------
						//
						Stuff::Vector3D direction_in_cloud;
						direction_in_cloud.Subtract(
							camera_in_cloud,
							particle->m_localTranslation
						);
						Stuff::LinearMatrix4D pert_to_cloud;
						pert_to_cloud.BuildRotation(particle->m_localRotation);
						pert_to_cloud.AlignLocalAxisToWorldVector(
							direction_in_cloud,
							Stuff::Z_Axis,
							Stuff::X_Axis,
							-1
						);
						pert_to_cloud.BuildTranslation(particle->m_localTranslation);

						//
						//----------------------------------------------------
						// Figure out the scale, then transform all the points
						//----------------------------------------------------
						//
						Stuff::Scalar scale = particle->m_scale;
						for (unsigned v=0; v<spec->m_vertices; ++v)
						{
							Stuff::Point3D scaled;
							scaled.Multiply(particle->m_vertices[v], scale);
							m_P_vertices[vert++].Multiply(scaled, pert_to_cloud);
						}
					}
					else
						vert += spec->m_vertices;
				}
			}
		}

		//
		//-------------------------------------------------------
		// Each matrix needs to be aligned to the camera around Y
		//-------------------------------------------------------
		//
		else if (spec->m_alignZUsingY)
		{
			//
			//-----------------------------------------
			// Get the camera location into local space
			//-----------------------------------------
			//
			Stuff::Point3D
				camera_in_world(info->m_clipper->GetCameraToWorldMatrix());
			Stuff::Point3D camera_in_cloud;
			camera_in_cloud.MultiplyByInverse(
				camera_in_world,
				local_to_world
			);

			//
			//--------------------------------------
			// Spin through all the active particles
			//--------------------------------------
			//
			for (i = 0; i < m_activeParticleCount; i++)
			{
				Particle *particle = GetParticle(i);
				Check_Object(particle);
				if (particle->m_age < 1.0f)
				{

					//
					//--------------------------------
					// Build the local to cloud matrix
					//--------------------------------
					//
					Stuff::Vector3D direction_in_cloud;
					direction_in_cloud.Subtract(
						camera_in_cloud,
						particle->m_localTranslation
					);
					Stuff::LinearMatrix4D pert_to_cloud;
					pert_to_cloud.BuildRotation(particle->m_localRotation);
					pert_to_cloud.AlignLocalAxisToWorldVector(
						direction_in_cloud,
						Stuff::Z_Axis,
						Stuff::Y_Axis,
						-1
					);
					pert_to_cloud.BuildTranslation(particle->m_localTranslation);

					//
					//----------------------------------------------------
					// Figure out the scale, then transform all the points
					//----------------------------------------------------
					//
					Stuff::Scalar scale = particle->m_scale;
					for (unsigned v=0; v<spec->m_vertices; ++v)
					{
						Stuff::Point3D scaled;
						scaled.Multiply(particle->m_vertices[v], scale);
						m_P_vertices[vert++].Multiply(scaled, pert_to_cloud);
					}
				}
				else
					vert += spec->m_vertices;
			}
		}

		//
		//---------------------------------------------------------------
		// No alignment is necessary, so just multiply out all the active
		// particles
		//---------------------------------------------------------------
		//
		else
		{
			for (i = 0; i < m_activeParticleCount; i++)
			{
				Particle *particle = GetParticle(i);
				Check_Object(particle);
				if (particle->m_age < 1.0f)
				{

					//
					//--------------------------------
					// Build the local to cloud matrix
					//--------------------------------
					//
					Stuff::LinearMatrix4D pert_to_cloud;
					pert_to_cloud.BuildRotation(particle->m_localRotation);
					pert_to_cloud.BuildTranslation(particle->m_localTranslation);

					//
					//----------------------------------------------------
					// Figure out the scale, then transform all the points
					//----------------------------------------------------
					//
					Stuff::Scalar scale = particle->m_scale;
					for (unsigned v=0; v<spec->m_vertices; ++v)
					{
						Stuff::Point3D scaled;
						scaled.Multiply(particle->m_vertices[v], scale);
						m_P_vertices[vert++].Multiply(scaled, pert_to_cloud);
					}
				}
				else
					vert += spec->m_vertices;
			}
		}

		//
		//---------------------
		// Now just do the draw
		//---------------------
		//
	 	info->m_clipper->DrawEffect(&dInfo);
	}

	SpinningCloud::Draw(info);
}

//------------------------------------------------------------------------------
//
void
	gosFX::PertCloud::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}
