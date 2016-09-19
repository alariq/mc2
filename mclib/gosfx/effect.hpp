//==========================================================================//
// File:	 gosFX_Effect.hpp												//
// Contents: Base Effect Component											//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"gosfx.hpp"
#include<mlr/mlr.hpp>

namespace MidLevelRenderer
{
	class MLRState;
	class MLRClipper;
	class MLRClippingState;
}

namespace gosFX
{
	class Effect__ClassData;

	//############################################################################
	//####################################  Event  ###############################
	//############################################################################

	class Event:
		public Stuff::Plug
	{
	public:
		Event():
			Plug(DefaultData)
				{}
		Event(const Event& event);

		static Event*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		void
			Save(Stuff::MemoryStream *stream);

		Stuff::Scalar
			m_time;
		unsigned
			m_flags,
			m_effectID;
		Stuff::LinearMatrix4D
			m_localToParent;

	protected:
		Event(
			Stuff::MemoryStream *stream,
			int gfx_version
		);
	};

	//############################################################################
	//########################  Effect__Specification  #############################
	//############################################################################

	class Effect__Specification
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		Effect__Specification(
			Stuff::RegisteredClass::ClassID class_id,
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		Effect__Specification(Stuff::RegisteredClass::ClassID class_id=gosFX::EffectClassID);
		virtual ~Effect__Specification();

		static Effect__Specification*
			Create(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		typedef Effect__Specification*
			(*Factory)(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		static Effect__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		virtual void
			Save(Stuff::MemoryStream *stream);

		virtual void 
			BuildDefaults();
	
		virtual bool 
			IsDataValid(bool fix_data=false);

		Stuff::RegisteredClass::ClassID
			GetClassID()
				{Check_Object(this); return m_class;}

		virtual void
			Copy(Effect__Specification *spec);

		Stuff::MString
			m_name;
		unsigned
			m_effectID;

	protected:
		Stuff::RegisteredClass::ClassID
			m_class;

	//----------------------------------------------------------------------
	// Events
	//
	public:
		Stuff::ChainOf<Event*>
			m_events;

		void
			AdoptEvent(Event *event);

	//-------------------------------------------------------------------------
	// FCurves
	//
	public:
		ConstantCurve
			m_lifeSpan;
		SplineCurve
			m_minimumChildSeed,
			m_maximumChildSeed;

	//----------------------------------------------------------------------
	// States
	//
	public:
		MidLevelRenderer::MLRState
			m_state;

	//----------------------------------------------------------------------
	// Testing
	//
	public:
		void
			TestInstance() const
				{}
	};

	//############################################################################
	//###############################  Effect  ###################################
	//############################################################################

	class Effect:
		public Stuff::Node
	{
		friend class EffectCloud;

	//----------------------------------------------------------------------------
	// Types
	//
	public:
		struct ExecuteInfo
		{
			Stuff::Time
				m_time;
			Stuff::Scalar
				m_seed,					// 0 <= m_seed <= 1
				m_age,
				m_ageRate;
			const Stuff::LinearMatrix4D
				*m_parentToWorld;
			Stuff::OBB
				*m_bounds;

			ExecuteInfo(
				Stuff::Time time,
				const Stuff::LinearMatrix4D *parent_to_world,
				Stuff::OBB *bounds,
				Stuff::Scalar seed = -1.0f
			)
				{
					m_time = time; m_seed = seed;
					m_parentToWorld = parent_to_world;
					m_bounds = bounds; m_age = -1.0f; m_ageRate = -1.0f;
				}

			void TestInstance() const
				{}

		private:
			ExecuteInfo(
				Stuff::Scalar time,
				const Stuff::LinearMatrix4D *parent_to_world,
				Stuff::OBB *bounds,
				Stuff::Scalar seed = -1.0f
			);
		};

		struct DrawInfo
		{
			const Stuff::LinearMatrix4D *m_parentToWorld;
			MidLevelRenderer::MLRClippingState m_clippingFlags;
			MidLevelRenderer::MLRState m_state;
			MidLevelRenderer::MLRClipper *m_clipper;

			void TestInstance() const
				{}
		};

		typedef Effect__Specification Specification;
		typedef Effect__ClassData ClassData;

	//----------------------------------------------------------------------------
	// Initialization
	//
	public:
		static void InitializeClass();
		static void	TerminateClass();
		static ClassData
			*DefaultData;

	//----------------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		Effect(
			ClassData *class_data,
			Specification *spec,
			unsigned flags
		);

	public:
		~Effect();

		typedef Effect*
			(*Factory)(
				Specification *spec,
				unsigned flags
			);

		static Effect*
			Make(
				Specification *spec,
				unsigned flags
			);

		Specification*
			GetSpecification()
				{Check_Object(this); return m_specification;}

	protected:
		Specification
			*m_specification;

	//----------------------------------------------------------------------------
	// Events
	//
	protected:
		Stuff::ChainOf<Effect *>
			m_children;
		Stuff::ChainIteratorOf<Event*>
			m_event;

	//----------------------------------------------------------------------------
	// Testing
	//
	public:
		void
			TestInstance() const;

	//----------------------------------------------------------------------------
	// API
	//
	public:
		virtual void Start(ExecuteInfo *info);
		void Stop();
		virtual bool Execute(ExecuteInfo *info);
		virtual void Kill();
		virtual void Draw(DrawInfo *info);
		virtual bool HasFinished();

		enum {
			ExecuteFlag = 1,
			LoopFlag = 2,
			LocalSpaceSimulationMode = 0,
			DynamicWorldSpaceSimulationMode = 4,
			StaticWorldSpaceSimulationMode = 8,
			ParentSimulationMode = 12,
			SimulationModeMask = 12
		};

		static Stuff::Vector3D
			s_ether;
		static Stuff::Vector3D
			s_gravity;

	public:
		void
			SetExecuteOn()
				{Check_Object(this); m_flags |= ExecuteFlag;}
		void
			SetExecuteOff()
				{Check_Object(this); m_flags &= ~ExecuteFlag;}
		bool
			IsExecuted()
				{Check_Object(this); return (m_flags&ExecuteFlag) != 0;}

		void
			SetLoopOn()
				{Check_Object(this); m_flags |= LoopFlag;}
		void
			SetLoopOff()
				{Check_Object(this); m_flags &= ~LoopFlag;}
		bool
			IsLooped()
				{Check_Object(this); return (m_flags&LoopFlag) != 0;}

		void
			UseLocalSpaceSimulation()
				{Check_Object(this); m_flags &= ~SimulationModeMask;}
		void
			UseStaticWorldSpaceSimulation()
				{
					Check_Object(this); m_flags &= ~SimulationModeMask;
					m_flags |= StaticWorldSpaceSimulationMode;
				}
		void
			UseDynamicWorldSpaceSimulation()
				{
					Check_Object(this); m_flags &= ~SimulationModeMask;
					m_flags |= DynamicWorldSpaceSimulationMode;
				}
		int
			GetSimulationMode()
				{Check_Object(this); return m_flags & SimulationModeMask;}

		int
			GetSimulationFlags()
				{Check_Object(this); return m_flags;}

		Stuff::LinearMatrix4D
			m_localToWorld;

	protected:
		Stuff::LinearMatrix4D
			m_localToParent;
		Stuff::Time
			m_lastRan;
		Stuff::Scalar
			m_age,
			m_ageRate,
			m_seed;
		unsigned
			m_flags;
	};


	//##########################################################################
	//########################    Effect__ClassData    #########################
	//##########################################################################

	class Effect__ClassData:
		public Stuff::Plug::ClassData
	{
	//----------------------------------------------------------------------------
	//
	public:
		Effect__ClassData(
			Stuff::RegisteredClass::ClassID class_id,
			const char* class_name,
			Stuff::Plug::ClassData *parent_class,
			Effect::Factory effect_factory,
			Effect::Specification::Factory spec_factory
		):
			RegisteredClass__ClassData(class_id, class_name, parent_class),
			effectFactory(effect_factory),
			specificationFactory(spec_factory)
				{}

		Effect::Factory 
			effectFactory;
		Effect::Specification::Factory
			specificationFactory;

	//----------------------------------------------------------------------------
	//
	public:
		void TestInstance();
	};

}
