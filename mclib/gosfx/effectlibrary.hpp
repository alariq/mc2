//==========================================================================//
// File:	 EffectLibrary.hpp												//
// Contents: Base Effect Component											//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"gosfx.hpp"
#include"effect.hpp"

namespace gosFX
{
	class EffectLibrary
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialization
	//
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	public:
		EffectLibrary();
		~EffectLibrary();

		void
			Load(Stuff::MemoryStream* stream);
		void
			Save(Stuff::MemoryStream* stream);

		enum MergeMode {
			OnlyAddNewEffects,
			ReplaceMatchingEffects,
			ReplaceNamedEffects
		};

		void
			Merge(
				EffectLibrary &source,
				MergeMode merge_mode=(MergeMode)OnlyAddNewEffects
			);

		static EffectLibrary*
			Instance;

	protected:
		EffectLibrary(EffectLibrary &source)
			{STOP(("Shouldn't be called"));}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Effect management
	//
	public:
		Stuff::DynamicArrayOf<Effect::Specification*>
			m_effects;

		Effect::Specification*
			Find(const char* name);
		Effect*
			MakeEffect(
				unsigned index,
				unsigned flags
			);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const
				{}
	};
}
