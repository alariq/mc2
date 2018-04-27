//===========================================================================//
// File:	verify.cc                                                        //
// Contents: verification fail routines                                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//#############################################################################
//#############################################################################
//
static char
	Error_Message_Buffer[400];

int
	Stuff::ArmorLevel = 4;

float
	Stuff::SMALL=1e-4f;

//
//#############################################################################
//#############################################################################
//
#if defined(_ARMOR)
	Signature::Signature()
	{
		mark = Valid;
	}

	Signature::~Signature()
	{
		mark = Destroyed;
	}

	void
		Stuff::Is_Signature_Bad(const volatile Signature *p)
	{
        // sebi: size_t instead of int
		if ((p) && reinterpret_cast<size_t>(p)!=Stuff::SNAN_NEGATIVE_INT32)
		{
			Verify(!(reinterpret_cast<size_t>(p) & 3));
			if (p->mark == Signature::Destroyed)
				PAUSE(("Object has been destroyed"));
			else if (p->mark != Signature::Valid)
				PAUSE(("Object has been corrupted"));
		}
		else
			PAUSE(("Bad object pointer: %x", p));
	}

#endif

// according to standard, placement new cannot be overridden
#if 0
void* operator new(size_t size, void* where) noexcept
		{Check_Pointer(where); return where;}
#endif
