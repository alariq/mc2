#include"gosfxheaders.hpp"

//==========================================================================//
// File:	 gosFX_Effect.cpp												//
// Contents: Base gosFX::Effect Component									//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

gosFX::EffectLibrary*
	gosFX::EffectLibrary::Instance = NULL;

//------------------------------------------------------------------------------
//
void
	gosFX::EffectLibrary::InitializeClass()
{
	Verify(!Instance);
}

//------------------------------------------------------------------------------
//
void
	gosFX::EffectLibrary::TerminateClass()
{
	if (Instance)
	{
		Unregister_Object(Instance);
		delete Instance;
		Instance=NULL;
	}
}

//------------------------------------------------------------------------------
//
gosFX::EffectLibrary::EffectLibrary()
{
	Verify(gos_GetCurrentHeap() == Heap);
}

//------------------------------------------------------------------------------
//
gosFX::EffectLibrary::~EffectLibrary()
{
	for (unsigned i=0; i<m_effects.GetLength(); ++i)
	{
		if (m_effects[i])
		{
			Unregister_Object(m_effects[i]);
			delete m_effects[i];
		}
	}
}

//------------------------------------------------------------------------------
//
void
	gosFX::EffectLibrary::Load(Stuff::MemoryStream* stream)
{
	Verify(gos_GetCurrentHeap() == Heap);
	Verify(!m_effects.GetLength());
	int version = ReadGFXVersion(stream);
	unsigned len;
	*stream >> len;
	m_effects.SetLength(len);
	for (unsigned i=0; i<len; ++i)
	{
        gosFX::Effect::Specification* pspec = gosFX::Effect::Specification::Create(stream, version);
		m_effects[i] = pspec;
		Check_Object(m_effects[i]);
		m_effects[i]->m_effectID = i;
	}
}

//------------------------------------------------------------------------------
//
void
	gosFX::EffectLibrary::Save(Stuff::MemoryStream* stream)
{
	WriteGFXVersion(stream);
	*stream << m_effects.GetLength();
	for (unsigned i=0; i<m_effects.GetLength(); ++i)
	{
		Check_Object(m_effects[i]);
		m_effects[i]->Save(stream);
	}
}

//------------------------------------------------------------------------------
//
gosFX::Effect::Specification*
	gosFX::EffectLibrary::Find(const char* name)
{
	for (unsigned i=0; i<m_effects.GetLength(); ++i)
	{
		gosFX::Effect::Specification *spec = m_effects[i];
		if (spec)
		{
			Check_Object(spec);
			if (!_stricmp(spec->m_name, name))
			{
				Verify( spec->m_effectID == i );
				return spec;
			}
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
//
gosFX::Effect*
	gosFX::EffectLibrary::MakeEffect(
		unsigned index,
		unsigned flags
	)
{
	gosFX::Effect::Specification *spec = m_effects[index];
	Check_Object(spec);
	gosFX::Effect::ClassData *data =
		Cast_Pointer(
			gosFX::Effect::ClassData*, 
			Stuff::RegisteredClass::FindClassData(spec->GetClassID())
		);
	Check_Object(data);
	Check_Pointer(data->effectFactory);
	return (*data->effectFactory)(spec, flags);
}
