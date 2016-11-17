#include"gosfxheaders.hpp"

//==========================================================================//
// File:	 gosFX_Effect.cpp												//
// Contents: Base gosFX::Effect Component									//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//############################################################################
//########################  EffectSpecification  #############################
//############################################################################

//------------------------------------------------------------------------------
//
gosFX::Event::Event(const Event& event):
	Plug(DefaultData)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);

	m_time = event.m_time;
	m_flags = event.m_flags;
	m_effectID = event.m_effectID;
}

//------------------------------------------------------------------------------
//
gosFX::Event::Event(
	Stuff::MemoryStream *stream,
	int gfx_version
):
	Plug(DefaultData)
{
	Check_Pointer(this);
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	*stream >> m_time >> m_flags >> m_effectID >> m_localToParent;
}

//------------------------------------------------------------------------------
//
gosFX::Event*
	gosFX::Event::Make(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	Event *event = new Event(stream, gfx_version);
	gos_PopCurrentHeap();

	return event;
}

//------------------------------------------------------------------------------
//
void
	gosFX::Event::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	*stream << m_time << m_flags << m_effectID << m_localToParent;
}

//############################################################################
//########################  EffectSpecification  #############################
//############################################################################

//------------------------------------------------------------------------------
//
gosFX::Effect__Specification::Effect__Specification(
	Stuff::RegisteredClass::ClassID class_id,
	Stuff::MemoryStream *stream,
	int gfx_version
):
	m_events(NULL)
{
	Check_Pointer(this);
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	if (gfx_version < 9)
	{
		STOP(("This version of gosFX is no longer supported"));
		return;
	}

	//
	//--------------
	// Read the name
	//--------------
	//
	m_class = class_id;
	*stream >> m_name;

	//
	//--------------------------------------------------------------------
	// Read the events.  If we are using an array, no events will be saved
	//--------------------------------------------------------------------
	//
	unsigned event_count;
	*stream >> event_count;
	while (event_count-- > 0)
	{
		Event *event = Event::Make(stream, gfx_version);
		Register_Object(event);
		m_events.Add(event);
	}

	//
	//--------------------------------------------------------
	// Load the curves, variances, and MLRState if appropriate
	//--------------------------------------------------------
	//
	m_lifeSpan.Load(stream, gfx_version);
	m_minimumChildSeed.Load(stream, gfx_version);
	m_maximumChildSeed.Load(stream, gfx_version);
	m_state.Load(stream, MidLevelRenderer::ReadMLRVersion(stream));
	if (gfx_version < 14)
	{
		m_state.SetRenderPermissionMask(
			m_state.GetRenderPermissionMask() | MidLevelRenderer::MLRState::TextureMask
		);
	}
	if (
		m_state.GetAlphaMode() != MidLevelRenderer::MLRState::OneZeroMode
		&& m_state.GetPriority() < MidLevelRenderer::MLRState::AlphaPriority
	)
		m_state.SetPriority(MidLevelRenderer::MLRState::AlphaPriority);
}

//------------------------------------------------------------------------------
//
gosFX::Effect__Specification::Effect__Specification(
	Stuff::RegisteredClass::ClassID class_id
):
	m_events(NULL)
{
	Check_Pointer(this);
	Verify(gos_GetCurrentHeap() == Heap);
	m_class = class_id;
}

//------------------------------------------------------------------------------
//
gosFX::Effect__Specification::~Effect__Specification()
{
	Check_Pointer(this);

	Stuff::ChainIteratorOf<Event*> events(&m_events);
	events.DeletePlugs();
}

//------------------------------------------------------------------------------
//
gosFX::Effect__Specification*
	gosFX::Effect__Specification::Make(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);

	gos_PushCurrentHeap(Heap);
	Effect__Specification *spec =
		new gosFX::Effect__Specification(EffectClassID, stream, gfx_version);
	gos_PopCurrentHeap();

	return spec;
}

//------------------------------------------------------------------------------
//

void 
	gosFX::Effect__Specification::BuildDefaults()
{

	Check_Object(this);
	
	m_lifeSpan.SetCurve(1.0f);
	m_minimumChildSeed.SetCurve(0.0f);
	m_maximumChildSeed.SetCurve(1.0f);
}


//------------------------------------------------------------------------------
//

bool gosFX::Effect__Specification::IsDataValid(bool fix_data)
{
    Check_Object(this);
    Stuff::Scalar minv,maxv;
    m_lifeSpan.ExpensiveComputeRange(&minv,&maxv);
    if(minv<0.0f)
    {
        if(fix_data)
        {
            m_lifeSpan.SetCurve(1.0f);
            PAUSE(("Warning: Curve \"lifespan\" in Effect \"%s\" Is Out of Range and has been Reset",(char *)m_name));

        }
        else 
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
gosFX::Effect__Specification*
	gosFX::Effect__Specification::Create(
		Stuff::MemoryStream *stream,
		int gfx_version
	)
{
	Check_Object(stream);
	Stuff::RegisteredClass::ClassID class_id;
	*stream >> class_id;
	gosFX::Effect::ClassData* class_data =
		Cast_Pointer(
			gosFX::Effect::ClassData*,
			Stuff::RegisteredClass::FindClassData(class_id)
		);
	Check_Object(class_data);
	Check_Pointer(class_data->specificationFactory);
	gosFX::Effect__Specification *spec =
		(*class_data->specificationFactory)(stream, gfx_version);
	Register_Object(spec);
	return spec;
}

//------------------------------------------------------------------------------
//
void
	gosFX::Effect__Specification::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	*stream << m_class << m_name;
	Stuff::ChainIteratorOf<Event*> events(&m_events);
	unsigned count = events.GetSize();
	*stream << count;
	Event *event;
	while ((event = events.ReadAndNext()) != NULL)
	{
		Check_Object(event);
		event->Save(stream);
	}
	m_lifeSpan.Save(stream);
	m_minimumChildSeed.Save(stream);
	m_maximumChildSeed.Save(stream);
	MidLevelRenderer::WriteMLRVersion(stream);
	m_state.Save(stream);
}

//------------------------------------------------------------------------------
//
void
	gosFX::Effect__Specification::Copy(Effect__Specification *spec)
{
	Check_Object(this);
	Check_Object(spec);
	gos_PushCurrentHeap(Heap);

	Verify(spec->m_class == m_class);
	m_name = spec->m_name;

	//
	//----------------------------------------------
	// Copy the events after delete our current ones
	//----------------------------------------------
	//
	Stuff::ChainIteratorOf<Event*> old_events(&m_events);
	old_events.DeletePlugs();
	Stuff::ChainIteratorOf<Event*> new_events(&spec->m_events);
	Event* event;
	while ((event = new_events.ReadAndNext()) != NULL)
	{
		Check_Object(event);
		Event *new_event = new Event(*event);
		Register_Object(new_event);
		AdoptEvent(new_event);
	}

	//
	//---------------------------------------------------
	// Now copy the curves, variance modes, and MLR state
	//---------------------------------------------------
	//
	m_lifeSpan = spec->m_lifeSpan;
	m_minimumChildSeed = spec->m_minimumChildSeed;
	m_maximumChildSeed = spec->m_maximumChildSeed;
	m_state = spec->m_state;

	gos_PopCurrentHeap();
}

//------------------------------------------------------------------------------
//
void
	gosFX::Effect__Specification::AdoptEvent(Event *event)
{
	Check_Object(this);
	Check_Object(event);
	Verify(event->m_time >= 0.0f && event->m_time <= 1.0f);
	Verify(gos_GetCurrentHeap() == Heap);

	//
	//-----------------------------------------------------------
	// The event must be inserted into the chain in order of time
	//-----------------------------------------------------------
	//
	Stuff::ChainIteratorOf<Event*> events(&m_events);
	Event *insert = NULL;
	while ((insert = events.GetCurrent()) != NULL)
	{
		Check_Object(insert);
		if (insert->m_time > event->m_time)
		{
			events.Insert(event);
			return;
		}
		events.Next();
	}
	m_events.Add(event);
}

//############################################################################
//##############################   gosFX::Effect  ###################################
//############################################################################

gosFX::Effect::ClassData*
	gosFX::Effect::DefaultData = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::Effect::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == Heap);
	DefaultData =
		new ClassData(
			EffectClassID,
			"gosFX::Effect",
			Node::DefaultData,
			&Make,
			&Specification::Make
		);
	Register_Object(DefaultData);
}

//------------------------------------------------------------------------------
//
void
	gosFX::Effect::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
gosFX::Effect::Effect(
	ClassData *class_data,
	Specification *spec,
	unsigned flags
):
	Node(class_data),
	m_children(NULL),
	m_event(&spec->m_events)
{
	Check_Pointer(this);
	Check_Object(spec);
	Verify(gos_GetCurrentHeap() == Heap);
	m_specification = spec;
	m_age = 0.0f;
	m_flags = flags;
	m_lastRan = -1.0f;
	m_localToParent = Stuff::LinearMatrix4D::Identity;
}


//------------------------------------------------------------------------------
//
gosFX::Effect::~Effect()
{
	Check_Object(this);
	Stuff::ChainIteratorOf<gosFX::Effect*> children(&m_children);
	children.DeletePlugs();
}

//------------------------------------------------------------------------------
//
gosFX::Effect*
	gosFX::Effect::Make(
		Specification *spec,
		unsigned flags
	)
{
	Check_Object(spec);

	gos_PushCurrentHeap(Heap);
	Effect *effect = new gosFX::Effect(DefaultData, spec, flags);
	gos_PopCurrentHeap();

	return effect;
}

//------------------------------------------------------------------------------
//
void
	gosFX::Effect::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//------------------------------------------------------------------------------
//
void
	gosFX::Effect::Start(ExecuteInfo *info)
{
	Check_Object(this);
	Check_Pointer(info);
	gos_PushCurrentHeap(Heap);

	//
	//---------------------------------------------------------------------
	// Don't override m_lastran if we are issuing a Start command while the
	// effect is already running
	//---------------------------------------------------------------------
	//
	if (!IsExecuted() || m_lastRan == -1.0)
		m_lastRan = info->m_time;
	SetExecuteOn();

	//
	//-------------------------------------------
	// If no seed was provided, pick one randomly
	//-------------------------------------------
	//
	m_seed = (info->m_seed == -1.0f) ? Stuff::Random::GetFraction() : info->m_seed;
	Verify(m_seed >= 0.0f && m_seed <= 1.0f);

	//
	//--------------------------------------------------------------------
	// Figure out how long the emitter will live and its initial age based
	// upon the effect seed
	//--------------------------------------------------------------------
	//
	Check_Object(m_specification);
	if (info->m_age == -1.0f)
	{
		Stuff::Scalar lifetime =
			m_specification->m_lifeSpan.ComputeValue(m_seed, 0.0f);
		Min_Clamp(lifetime, 0.033333f);
		m_ageRate = 1.0f / lifetime;
		m_age = 0;
	}
	else
	{
		m_age = info->m_age;
		m_ageRate = info->m_ageRate;
		Verify(m_age >= 0.0f && m_age <= 1.0f);
	}

	//
	//--------------------
	// Set up the matrices
	//--------------------
	//
	Check_Object(info->m_parentToWorld);
	m_localToWorld.Multiply(m_localToParent, *info->m_parentToWorld);

	//
	//-------------------------
	// Set up the event pointer
	//-------------------------
	//
	m_event.First();
	gos_PopCurrentHeap();
}

//------------------------------------------------------------------------------
//
bool gosFX::Effect::Execute(ExecuteInfo *info)
{
	Check_Object(this);
	Check_Pointer(info);
	Verify(IsExecuted());
	gos_PushCurrentHeap(Heap);

	//
	//-----------------------------------------------------
	// If a new seed is provided, override the current seed
	//-----------------------------------------------------
	//
	if (info->m_seed != -1.0f)
	{
		Verify(info->m_seed>=0.0f && info->m_seed<1.0f);
		m_seed = info->m_seed;
	}

	//
	//--------------------------------------------
	// Figure out the new age and clear the bounds
	//--------------------------------------------
	//
	Stuff::Scalar age =
		m_age + static_cast<Stuff::Scalar>(info->m_time - m_lastRan) * m_ageRate;
	Verify(age >= 0.0f && age >= m_age);
	*info->m_bounds = Stuff::OBB::Identity;

	//
	//--------------------------------
	// Update the effectToWorld matrix
	//--------------------------------
	//
	Check_Object(info->m_parentToWorld);
	m_localToWorld.Multiply(m_localToParent, *info->m_parentToWorld);

	//
	//--------------------------------------------------
	// Check to see if the top event needs to be handled
	//--------------------------------------------------
	//
	Check_Object(m_specification);
	Event *event;
	while ((event = m_event.GetCurrent()) != NULL)
	{
		Check_Object(event);
		if (event->m_time > m_age)
			break;

		//
		//-------------------------------------------------------------
		// This event needs to go, so spawn and bump the effect pointer
		//-------------------------------------------------------------
		//
		unsigned flags = ExecuteFlag;
		if ((event->m_flags&SimulationModeMask) == ParentSimulationMode)
		{
			Verify((m_flags&SimulationModeMask) != ParentSimulationMode);
			flags |= m_flags&SimulationModeMask;
		}
		else
			flags |= event->m_flags&SimulationModeMask;

		Effect* effect =
			EffectLibrary::Instance->MakeEffect(
				event->m_effectID,
				flags
			);
		Register_Object(effect);
		m_children.Add(effect);
		m_event.Next();

		//
		//---------------------------------------------
		// Now set the info for starting the new effect
		//---------------------------------------------
		//
		effect->m_localToParent = event->m_localToParent;
		Stuff::Scalar min_seed =
			m_specification->m_minimumChildSeed.ComputeValue(m_age, m_seed);
		Stuff::Scalar seed_range =
			m_specification->m_maximumChildSeed.ComputeValue(m_age, m_seed) - min_seed;
		Stuff::Scalar seed =
			Stuff::Random::GetFraction()*seed_range + min_seed;
		Clamp(seed, 0.0f, 1.0f);
		ExecuteInfo
			local_info(
				info->m_time,
				&m_localToWorld,
				NULL,
				seed
			);
		effect->Start(&local_info);
	}

	//
	//------------------------------------------------------------
	// Execute all the children.  If any of them finish, kill them
	//------------------------------------------------------------
	//
	Stuff::ChainIteratorOf<gosFX::Effect*> children(&m_children);
	gosFX::Effect *child;
	Stuff::OBB child_obb = Stuff::OBB::Identity;
	ExecuteInfo
		child_info(
			info->m_time,
			&m_localToWorld,
			&child_obb
		);
	child_info.m_bounds = &child_obb;
	while ((child = children.ReadAndNext()) != NULL)
	{
		Check_Object(child);
		if (!child->Execute(&child_info))
		{
			Unregister_Object(child);
			delete child;
		}

		//
		//--------------------------------------------------------------
		// Merge the bounding sphere of the child into the bounds of the
		// parent
		//--------------------------------------------------------------
		//
		Stuff::OBB parent_bounds;
		parent_bounds.Multiply(child_obb, m_localToParent);
		info->m_bounds->Union(*info->m_bounds, parent_bounds);
	}

	Check_Object(info->m_bounds);

	//
	//----------------------------------------------------------------------
	// Set the new time, then if we have run the course of the effect, start
	// over if we loop, otherwise wait for our children to finish before
	// killing ourselves
	//----------------------------------------------------------------------
	//
	m_lastRan = info->m_time;
	m_age = age;
	if (m_age >= 1.0f)
	{
		if (IsLooped())
			Start(info);
		else if (HasFinished())
			Kill();
	}

	//
	//----------------------------------
	// Tell our parent if we need to die
	//----------------------------------
	//
	gos_PopCurrentHeap();
	return IsExecuted();
}

//------------------------------------------------------------------------------
//
void gosFX::Effect::Stop()
{
	Check_Object(this);

	//
	//------------------------------------
	// Stop the children then stop ourself
	//------------------------------------
	//
	m_age = 1.0f;
	m_event.Last();
	m_event.Next();
}

//------------------------------------------------------------------------------
//
void gosFX::Effect::Kill()
{
	Check_Object(this);

	//
	//------------------------------------
	// Kill the children then kill ourself
	//------------------------------------
	//
	Stuff::ChainIteratorOf<gosFX::Effect*> children(&m_children);
	gosFX::Effect* child;
	while ((child = children.ReadAndNext()) != NULL)
	{
		Check_Object(child);
		child->Kill();
	}
	SetExecuteOff();
}

//------------------------------------------------------------------------------
//
void gosFX::Effect::Draw(DrawInfo *info)
{
	Check_Object(this);
	Check_Pointer(info);

	//
	//-------------------------------------
	// Make sure all the children get drawn
	//-------------------------------------
	//
	DrawInfo new_info;
	Check_Object(m_specification);
	new_info.m_state.Combine(info->m_state, m_specification->m_state);
	Stuff::LinearMatrix4D local_to_world;
	local_to_world.Multiply(m_localToParent, *info->m_parentToWorld);
	new_info.m_parentToWorld = &local_to_world;
	new_info.m_clipper = info->m_clipper;
	new_info.m_clippingFlags = info->m_clippingFlags;
	Stuff::ChainIteratorOf<gosFX::Effect*> children(&m_children);
	gosFX::Effect *child;
	while ((child = children.ReadAndNext()) != NULL)
	{
		Check_Object(child);
		child->Draw(&new_info);
	}
}

//------------------------------------------------------------------------------
//
bool gosFX::Effect::HasFinished()
{
	Check_Object(this);

	//
	//-------------------------------------------------------------------------
	// An effect is not finished if it is executing and its life hasn't expired
	//-------------------------------------------------------------------------
	//
	if (IsExecuted() && m_age < 1.0f)
		return false;

	//
	//-----------------------------------------------
	// It is also not finished if it has any children
	//-----------------------------------------------
	//
	Stuff::ChainIteratorOf<gosFX::Effect*> children(&m_children);
	return children.GetCurrent() == NULL;
}

//#############################################################################
//##########################    gosFX::Effect__ClassData    ##########################
//#############################################################################

void
	gosFX::Effect__ClassData::TestInstance()
{
	Verify(IsDerivedFrom(gosFX::Effect::DefaultData));
}
