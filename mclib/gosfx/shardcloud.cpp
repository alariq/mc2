#include"gosfxheaders.hpp"
#include<mlr/mlrtrianglecloud.hpp>

//==========================================================================//
// File:	 gosFX_ShardCloud.cpp											//
// Contents: Base gosFX::ShardCloud Component								//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//------------------------------------------------------------------------------
//
gosFX::ShardCloud__Specification::ShardCloud__Specification(
	Stuff::MemoryStream *stream,
	int gfx_version
):
	SpinningCloud__Specification(gosFX::ShardCloudClassID, stream, gfx_version)
{
	Check_Pointer(this);
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);
	Verify(m_class == ShardCloudClassID);

	m_size.Load(stream, gfx_version);
	m_angularity.Load(stream, gfx_version);

	m_totalParticleSize = gosFX::ShardCloud::ParticleSize;
	m_particleClassSize = sizeof(gosFX::ShardCloud::Particle);
}

//------------------------------------------------------------------------------
//
gosFX::ShardCloud__Specification::ShardCloud__Specification():
	SpinningCloud__Specification(gosFX::ShardCloudClassID)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
	m_totalParticleSize = gosFX::ShardCloud::ParticleSize;
	m_particleClassSize = sizeof(gosFX::ShardCloud::Particle);
}

//------------------------------------------------------------------------------
//
gosFX::ShardCloud__Specification*
	gosFX::ShardCloud__Specification::Make(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	ShardCloud__Specification *spec =
		new gosFX::ShardCloud__Specification(stream, gfx_version);
	gos_PopCurrentHeap();

	return spec;
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShardCloud__Specification::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);
	SpinningCloud__Specification::Save(stream);

	m_size.Save(stream);
	m_angularity.Save(stream);
}

//------------------------------------------------------------------------------
//
void 
	gosFX::ShardCloud__Specification::BuildDefaults()
{

	Check_Object(this);
	SpinningCloud__Specification::BuildDefaults();

	m_size.m_ageCurve.SetCurve(1.0f);
	m_size.m_seeded = false;
	m_size.m_seedCurve.SetCurve(1.0f);

	m_angularity.m_ageCurve.SetCurve(1.0f);
	m_angularity.m_seeded = false;
	m_angularity.m_seedCurve.SetCurve(1.0f);

}

//------------------------------------------------------------------------------
//
bool 
	gosFX::ShardCloud__Specification::IsDataValid(bool fix_data)
{

	Check_Object(this);
return		SpinningCloud__Specification::IsDataValid(fix_data);
/*
	m_size.m_ageCurve.SetCurve(1.0f);
	m_size.m_seeded = false;
	m_size.m_seedCurve.SetCurve(1.0f);

	m_angularity.m_ageCurve.SetCurve(1.0f);
	m_angularity.m_seeded = false;
	m_angularity.m_seedCurve.SetCurve(1.0f);
*/
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShardCloud__Specification::Copy(ShardCloud__Specification *spec)
{
	Check_Object(this);
	Check_Object(spec);

	SpinningCloud__Specification::Copy(spec);

	gos_PushCurrentHeap(Heap);
	m_size = spec->m_size;
	m_angularity = spec->m_angularity;
	gos_PopCurrentHeap();
}

//############################################################################
//##############################  gosFX::ShardCloud  ################################
//############################################################################

gosFX::ShardCloud::ClassData*
	gosFX::ShardCloud::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::ShardCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == Heap);
	DefaultData =
		new ClassData(
			ShardCloudClassID,
			"gosFX::ShardCloud",
			SpinningCloud::DefaultData,
			(Effect::Factory)&Make,
			(Specification::Factory)&Specification::Make
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShardCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//------------------------------------------------------------------------------
//
gosFX::ShardCloud::ShardCloud(
	Specification *spec,
	unsigned flags
):
	SpinningCloud(DefaultData, spec, flags)
{
	Check_Object(spec);
	Verify(gos_GetCurrentHeap() == Heap);

	gos_PushCurrentHeap(MidLevelRenderer::Heap);
	m_cloudImplementation =
		new MidLevelRenderer::MLRTriangleCloud(spec->m_maxParticleCount);
	Register_Object(m_cloudImplementation);
	gos_PopCurrentHeap();

	unsigned index = spec->m_maxParticleCount*sizeof(Particle);
	m_P_vertices = Cast_Pointer(Stuff::Point3D*, &m_data[index]);
	index += 3*spec->m_maxParticleCount*sizeof(Stuff::Point3D);
	m_P_color = Cast_Pointer(Stuff::RGBAColor*, &m_data[index]);

	m_cloudImplementation->SetData(
		Cast_Pointer(const int *, &m_activeParticleCount),
		m_P_vertices,
		m_P_color
	);
}

//------------------------------------------------------------------------------
//
gosFX::ShardCloud::~ShardCloud()
{
	Unregister_Object(m_cloudImplementation);
	delete m_cloudImplementation;
}

//------------------------------------------------------------------------------
//
gosFX::ShardCloud*
	gosFX::ShardCloud::Make(
		Specification *spec,
		unsigned flags
	)
{
	Check_Object(spec);

	gos_PushCurrentHeap(Heap);
	ShardCloud *cloud = new gosFX::ShardCloud(spec, flags);
	gos_PopCurrentHeap();

	return cloud;
}

//------------------------------------------------------------------------------
//
bool
	gosFX::ShardCloud::AnimateParticle(
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
	Set_Statistic(Shard_Count, Shard_Count+1);
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
	index *= 3;
	m_P_color[index].red = spec->m_pRed.ComputeValue(age, seed);
	m_P_color[index].green = spec->m_pGreen.ComputeValue(age, seed);
	m_P_color[index].blue = spec->m_pBlue.ComputeValue(age, seed);
	m_P_color[index].alpha = spec->m_pAlpha.ComputeValue(age, seed);
	m_P_color[index+2] = m_P_color[index+1] = m_P_color[index];
	return true;
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShardCloud::CreateNewParticle(
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
	//-----------------------------
	// Figure out the particle size
	//-----------------------------
	//
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Particle *particle = GetParticle(index);
	Check_Object(particle);
	particle->m_radius = spec->m_size.ComputeValue(m_age, particle->m_seed);
	particle->m_angle =
		Stuff::Sin(spec->m_angularity.ComputeValue(m_age, particle->m_seed));
}

//------------------------------------------------------------------------------
//
void gosFX::ShardCloud::DestroyParticle(unsigned index)
{
	Check_Object(this);

	m_cloudImplementation->TurnOff(index);
	Verify(!m_cloudImplementation->IsOn(index));
	SpinningCloud::DestroyParticle(index);
}

//------------------------------------------------------------------------------
//
void gosFX::ShardCloud::Draw(DrawInfo *info)
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
						Stuff::LinearMatrix4D shard_to_cloud;
						shard_to_cloud.BuildRotation(particle->m_localRotation);
						shard_to_cloud.AlignLocalAxisToWorldVector(
							direction_in_cloud,
							Stuff::Z_Axis,
							Stuff::Y_Axis,
							Stuff::X_Axis
						);
						shard_to_cloud.BuildTranslation(particle->m_localTranslation);

						//
						//--------------------------------------------------
						// Figure out the scale, then build the three points
						//--------------------------------------------------
						//
						Stuff::Scalar scale = particle->m_scale;
						m_P_vertices[vert++].Multiply(
							Stuff::Point3D(
								0.0f,
								-0.5f*scale*particle->m_radius,
								0.0f
							),
							shard_to_cloud
						);
						m_P_vertices[vert++].Multiply(
							Stuff::Point3D(
								scale*particle->m_angle*particle->m_radius,
								scale*particle->m_radius*0.5f,
								0.0f
							),
							shard_to_cloud
						);
						m_P_vertices[vert++].Multiply(
							Stuff::Point3D(
								-scale*particle->m_angle*particle->m_radius,
								scale*particle->m_radius*0.5f,
								0.0f
							),
							shard_to_cloud
						);
					}
					else
						vert += 3;
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
						Stuff::LinearMatrix4D shard_to_cloud;
						shard_to_cloud.BuildRotation(particle->m_localRotation);
						shard_to_cloud.AlignLocalAxisToWorldVector(
							direction_in_cloud,
							Stuff::Z_Axis,
							Stuff::X_Axis,
							-1
						);
						shard_to_cloud.BuildTranslation(particle->m_localTranslation);

						//
						//--------------------------------------------------
						// Figure out the scale, then build the three points
						//--------------------------------------------------
						//
						Stuff::Scalar scale = particle->m_scale;
						m_P_vertices[vert++].Multiply(
							Stuff::Point3D(
								0.0f,
								-0.5f*scale*particle->m_radius,
								0.0f
							),
							shard_to_cloud
						);
						m_P_vertices[vert++].Multiply(
							Stuff::Point3D(
								scale*particle->m_angle*particle->m_radius,
								scale*particle->m_radius*0.5f,
								0.0f
							),
							shard_to_cloud
						);
						m_P_vertices[vert++].Multiply(
							Stuff::Point3D(
								-scale*particle->m_angle*particle->m_radius,
								scale*particle->m_radius*0.5f,
								0.0f
							),
							shard_to_cloud
						);
					}
					else
						vert += 3;
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
					Stuff::LinearMatrix4D shard_to_cloud;
					shard_to_cloud.BuildRotation(particle->m_localRotation);
					shard_to_cloud.AlignLocalAxisToWorldVector(
						direction_in_cloud,
						Stuff::Z_Axis,
						Stuff::Y_Axis,
						-1
					);
					shard_to_cloud.BuildTranslation(particle->m_localTranslation);

					//
					//--------------------------------------------------
					// Figure out the scale, then build the three points
					//--------------------------------------------------
					//
					Stuff::Scalar scale = particle->m_scale;
					m_P_vertices[vert++].Multiply(
						Stuff::Point3D(
							0.0f,
							-0.5f*scale*particle->m_radius,
							0.0f
						),
						shard_to_cloud
					);
					m_P_vertices[vert++].Multiply(
						Stuff::Point3D(
							scale*particle->m_angle*particle->m_radius,
							scale*particle->m_radius*0.5f,
							0.0f
						),
						shard_to_cloud
					);
					m_P_vertices[vert++].Multiply(
						Stuff::Point3D(
							-scale*particle->m_angle*particle->m_radius,
							scale*particle->m_radius*0.5f,
							0.0f
						),
						shard_to_cloud
					);
				}
				else
					vert += 3;
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
					Stuff::LinearMatrix4D shard_to_cloud;
					shard_to_cloud.BuildRotation(particle->m_localRotation);
					shard_to_cloud.BuildTranslation(particle->m_localTranslation);

					//
					//--------------------------------------------------
					// Figure out the scale, then build the three points
					//--------------------------------------------------
					//
					Stuff::Scalar scale = particle->m_scale;
					m_P_vertices[vert++].Multiply(
						Stuff::Point3D(
							0.0f,
							-0.5f*scale*particle->m_radius,
							0.0f
						),
						shard_to_cloud
					);
					m_P_vertices[vert++].Multiply(
						Stuff::Point3D(
							scale*particle->m_angle*particle->m_radius,
							scale*particle->m_radius*0.5f,
							0.0f
						),
						shard_to_cloud
					);
					m_P_vertices[vert++].Multiply(
						Stuff::Point3D(
							-scale*particle->m_angle*particle->m_radius,
							scale*particle->m_radius*0.5f,
							0.0f
						),
						shard_to_cloud
					);
				}
				else
					vert += 3;
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
	gosFX::ShardCloud::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}
