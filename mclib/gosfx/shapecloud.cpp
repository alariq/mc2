#include"gosfxheaders.hpp"

//==========================================================================//
// File:	 gosFX_ShapeCloud.cpp											//
// Contents: Base gosFX::ShapeCloud Component								//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
//############################################################################
//########################  gosFX::ShapeCloud__Specification  #############################
//############################################################################

//------------------------------------------------------------------------------
//
gosFX::ShapeCloud__Specification::ShapeCloud__Specification(
	Stuff::MemoryStream *stream,
	int gfx_version
):
	SpinningCloud__Specification(gosFX::ShapeCloudClassID, stream, gfx_version)
{
	Check_Pointer(this);
	Verify(m_class == ShapeCloudClassID);
	Verify(gos_GetCurrentHeap() == Heap);
	m_particleClassSize = sizeof(gosFX::ShapeCloud::Particle);
	m_totalParticleSize = sizeof(gosFX::ShapeCloud::Particle);

	//
	//---------------
	// Load the shape
	//---------------
	//
	m_shape =
		MidLevelRenderer::MLRShape::Make(
			stream,
			MidLevelRenderer::ReadMLRVersion(stream)
		);
	Register_Object(m_shape);
	*stream >> m_radius;

	//
	//---------------------------------
	// Make sure everything is in order
	//---------------------------------
	//
}

//------------------------------------------------------------------------------
//
gosFX::ShapeCloud__Specification::ShapeCloud__Specification(
	MidLevelRenderer::MLRShape *shape
):
	SpinningCloud__Specification(gosFX::ShapeCloudClassID)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
	m_totalParticleSize = m_particleClassSize = sizeof(gosFX::ShapeCloud::Particle);
	m_shape = NULL;
	SetShape(shape);
}

//------------------------------------------------------------------------------
//
gosFX::ShapeCloud__Specification::~ShapeCloud__Specification()
{
	Check_Object(this);
	if (m_shape)
	{
		Check_Object(m_shape);
		m_shape->DetachReference();
	}
}

//------------------------------------------------------------------------------
//
gosFX::ShapeCloud__Specification*
	gosFX::ShapeCloud__Specification::Make(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	ShapeCloud__Specification *spec =
		new gosFX::ShapeCloud__Specification(stream, gfx_version);
	gos_PopCurrentHeap();

	return spec;
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShapeCloud__Specification::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);
	SpinningCloud__Specification::Save(stream);
	MidLevelRenderer::WriteMLRVersion(stream);
	m_shape->Save(stream);
	*stream << m_radius;
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShapeCloud__Specification::Copy(ShapeCloud__Specification *spec)
{
	Check_Object(this);
	Check_Object(spec);

	SpinningCloud__Specification::Copy(spec);

	gos_PushCurrentHeap(Heap);
	m_radius = spec->m_radius;
	m_shape = spec->m_shape;
	gos_PopCurrentHeap();

	Check_Object(m_shape);
	m_shape->AttachReference();
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShapeCloud__Specification::SetShape(MidLevelRenderer::MLRShape *shape)
{
	Check_Object(this);

	//
	//------------------------------------
	// Detach the old shape if it is there
	//------------------------------------
	//
	if (m_shape)
	{
		Check_Object(m_shape);
		m_shape->DetachReference();
	}

	//
	//------------------------------------
	// Attach the new shape if it is there
	//------------------------------------
	//
	if (shape)
	{
		Check_Object(shape);
		m_shape = shape;
		m_shape->AttachReference();

		//
		//-----------------------------------------------------------------
		// Get the radius of the bounding sphere.  This will be the largest
		// distance any point is from the origin
		//-----------------------------------------------------------------
		//
		m_radius = 0.0f;
		int count = m_shape->GetNum();
		for (int i=0; i<count; ++i)
		{
			MidLevelRenderer::MLRPrimitiveBase *primitive = m_shape->Find(i);
			Check_Object(primitive);
			Stuff::Point3D *points;
			int vertex_count;
			primitive->GetCoordData(&points, &vertex_count);
			for (int v=0; v<vertex_count; ++v)
			{
				Stuff::Scalar len = points[v].GetLengthSquared();
				if (len > m_radius)
					m_radius = len;
			}
		}
		m_radius = Stuff::Sqrt(m_radius);
	}
}

//############################################################################
//##############################  gosFX::ShapeCloud  ################################
//############################################################################

gosFX::ShapeCloud::ClassData*
	gosFX::ShapeCloud::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::ShapeCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == Heap);
	DefaultData =
		new ClassData(
			ShapeCloudClassID,
			"gosFX::ShapeCloud",
			SpinningCloud::DefaultData,
			(Effect::Factory)&Make,
			(Specification::Factory)&Specification::Make
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShapeCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//------------------------------------------------------------------------------
//
gosFX::ShapeCloud::ShapeCloud(
	Specification *spec,
	unsigned flags
):
	SpinningCloud(DefaultData, spec, flags)
{
	Verify(gos_GetCurrentHeap() == Heap);
}

//------------------------------------------------------------------------------
//
gosFX::ShapeCloud*
	gosFX::ShapeCloud::Make(
		Specification *spec,
		unsigned flags
	)
{
	Check_Object(spec);

	gos_PushCurrentHeap(Heap);
	ShapeCloud *cloud = new gosFX::ShapeCloud(spec, flags);
	gos_PopCurrentHeap();

	return cloud;
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShapeCloud::CreateNewParticle(
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
	Specification *spec = GetSpecification();
	Check_Object(spec);
	Particle *particle = GetParticle(index);
	Check_Object(particle);
	particle->m_radius = spec->m_radius;
}

//------------------------------------------------------------------------------
//
bool
	gosFX::ShapeCloud::AnimateParticle(
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
	Set_Statistic(Shape_Count, Shape_Count+1);
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
	particle->m_color.red = spec->m_pRed.ComputeValue(age, seed);
	particle->m_color.green = spec->m_pGreen.ComputeValue(age, seed);
	particle->m_color.blue = spec->m_pBlue.ComputeValue(age, seed);
	particle->m_color.alpha = spec->m_pAlpha.ComputeValue(age, seed);
	return true;
}

//------------------------------------------------------------------------------
//
void gosFX::ShapeCloud::Draw(DrawInfo *info)
{
	Check_Object(this);
	Check_Object(info);
	Check_Object(info->m_parentToWorld);

	//
	//----------------------------
	// Set up the common draw info
	//----------------------------
	//
	if (m_activeParticleCount)
	{
		MidLevelRenderer::DrawScalableShapeInformation dinfo;
		MidLevelRenderer::MLRShape *shape = GetSpecification()->m_shape;
		dinfo.clippingFlags.SetClippingState(0x3f);
		dinfo.worldToShape = NULL;
		Specification *spec = GetSpecification();
		Check_Object(spec);
		dinfo.state.Combine(info->m_state, spec->m_state);
		dinfo.activeLights = NULL;
		dinfo.nrOfActiveLights = 0;
		dinfo.shape = shape;
		Stuff::LinearMatrix4D local_to_world;
		local_to_world.Multiply(m_localToParent, *info->m_parentToWorld);

		//
		//--------------------------------------------------------------
		// Check the orientation mode.  The first case is XY orientation
		//--------------------------------------------------------------
		//
		unsigned i;
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
				for (unsigned i = 0; i < m_activeParticleCount; i++)
				{
					Particle *particle = GetParticle(i);
					Check_Object(particle);

					//
					//-----------------------------------------------------------------
					// If the particle is still alive, concatenate into world space and
					// issue the draw command
					//-----------------------------------------------------------------
					//
					if (particle->m_age < 1.0f)
					{
						Stuff::Vector3D direction_in_cloud;
						direction_in_cloud.Subtract(
							camera_in_cloud,
							particle->m_localTranslation
						);
						Stuff::LinearMatrix4D shape_to_cloud;
						shape_to_cloud.BuildRotation(particle->m_localRotation);
						shape_to_cloud.AlignLocalAxisToWorldVector(
							direction_in_cloud,
							Stuff::Z_Axis,
							Stuff::Y_Axis,
							Stuff::X_Axis
						);
						shape_to_cloud.BuildTranslation(particle->m_localTranslation);
						Stuff::LinearMatrix4D shape_to_world;
						shape_to_world.Multiply(
							shape_to_cloud,
							local_to_world
						);
						dinfo.shapeToWorld = &shape_to_world;
						Stuff::Vector3D
							scale(
								particle->m_scale,
								particle->m_scale,
								particle->m_scale
							);
						dinfo.scaling = &scale;
						dinfo.paintMe = &particle->m_color;
		 				info->m_clipper->DrawScalableShape(&dinfo);
					}
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
				for (unsigned i = 0; i < m_activeParticleCount; i++)
				{
					Particle *particle = GetParticle(i);
					Check_Object(particle);

					//
					//-----------------------------------------------------------------
					// If the particle is still alive, concatenate into world space and
					// issue the draw command
					//-----------------------------------------------------------------
					//
					if (particle->m_age < 1.0f)
					{
						Stuff::Vector3D direction_in_cloud;
						direction_in_cloud.Subtract(
							camera_in_cloud,
							particle->m_localTranslation
						);
						Stuff::LinearMatrix4D shape_to_cloud;
						shape_to_cloud.BuildRotation(particle->m_localRotation);
						shape_to_cloud.AlignLocalAxisToWorldVector(
							direction_in_cloud,
							Stuff::Z_Axis,
							Stuff::X_Axis,
							-1
						);
						shape_to_cloud.BuildTranslation(particle->m_localTranslation);
						Stuff::LinearMatrix4D shape_to_world;
						shape_to_world.Multiply(
							shape_to_cloud,
							local_to_world
						);
						dinfo.shapeToWorld = &shape_to_world;
						Stuff::Vector3D
							scale(
								particle->m_scale,
								particle->m_scale,
								particle->m_scale
							);
						dinfo.scaling = &scale;
						dinfo.paintMe = &particle->m_color;
		 				info->m_clipper->DrawScalableShape(&dinfo);
					}
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
			for (unsigned i = 0; i < m_activeParticleCount; i++)
			{
				Particle *particle = GetParticle(i);
				Check_Object(particle);

				//
				//-----------------------------------------------------------------
				// If the particle is still alive, concatenate into world space and
				// issue the draw command
				//-----------------------------------------------------------------
				//
				if (particle->m_age < 1.0f)
				{
					Stuff::Vector3D direction_in_cloud;
					direction_in_cloud.Subtract(
						camera_in_cloud,
						particle->m_localTranslation
					);
					Stuff::LinearMatrix4D shape_to_cloud;
					shape_to_cloud.BuildRotation(particle->m_localRotation);
					shape_to_cloud.AlignLocalAxisToWorldVector(
						direction_in_cloud,
						Stuff::Z_Axis,
						Stuff::Y_Axis,
						-1
					);
					shape_to_cloud.BuildTranslation(particle->m_localTranslation);
					Stuff::LinearMatrix4D shape_to_world;
					shape_to_world.Multiply(
						shape_to_cloud,
						local_to_world
					);
					dinfo.shapeToWorld = &shape_to_world;
					Stuff::Vector3D
						scale(
							particle->m_scale,
							particle->m_scale,
							particle->m_scale
						);
					dinfo.scaling = &scale;
					dinfo.paintMe = &particle->m_color;
		 			info->m_clipper->DrawScalableShape(&dinfo);
				}
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
			for (i=0; i < m_activeParticleCount; i++)
			{
				Particle *particle = GetParticle(i);
				Check_Object(particle);

				//
				//-----------------------------------------------------------------
				// If the particle is still alive, concatenate into world space and
				// issue the draw command
				//-----------------------------------------------------------------
				//
				if (particle->m_age < 1.0f)
				{
					Stuff::LinearMatrix4D shape_to_cloud;
					shape_to_cloud.BuildTranslation(particle->m_localTranslation);
					shape_to_cloud.BuildRotation(particle->m_localRotation);
					Stuff::LinearMatrix4D shape_to_world;
					shape_to_world.Multiply(
						shape_to_cloud,
						local_to_world
					);
					dinfo.shapeToWorld = &shape_to_world;
					Stuff::Vector3D
						scale(
							particle->m_scale,
							particle->m_scale,
							particle->m_scale
						);
					dinfo.scaling = &scale;
					dinfo.paintMe = &particle->m_color;
		 			info->m_clipper->DrawScalableShape(&dinfo);
				}
			}
		}
	}

	//
	//----------------------------
	// Let our parent do its thing
	//----------------------------
	//
	SpinningCloud::Draw(info);
}

//------------------------------------------------------------------------------
//
void
	gosFX::ShapeCloud::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}
