//===========================================================================//
// File:	random.cc                                                        //
// Contents: Implementation details for the random number generator          //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

int
	Random::Numbers[250];
int
	Random::Index = -1;

Random*
	Random::Instance = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Random::InitializeClass()
{
	Verify(!Random::Instance);
	Verify(Index == -1);
	Random::Instance = new Random;
	Register_Pointer(Random::Instance);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Random::TerminateClass()
{
	Unregister_Pointer(Random::Instance);
	delete Random::Instance;
	Random::Instance = NULL;
	Index = -1;
}

//
//###########################################################################
//###########################################################################
//
void Random::Init()
{
	//
	//------------------------------
	// Load the random number buffer
	//------------------------------
	//
	int i;
	Index=0;
	for (i=0; i<250; i++)
		Numbers[i] = gos_rand();

	//
	//--------------------------------------------------------------------
	// In order to preserve a good random number mix for the XOR function,
	// mask and set the bits of 15 words in a descending manner as in
	// 01xx..., 001xx..., 0001xx..., ...
	//--------------------------------------------------------------------
	//
	int mask = GetRandMax() >> 1;
	int msb = mask + 1;
	int rand_size;
	for (rand_size=0; !(msb&(1<<rand_size)); ++rand_size);
	i = 14;
	while (rand_size--)
	{
		Verify(i<ELEMENTS(Numbers));
		Numbers[i] &= mask;
		Numbers[i] |= msb;
		mask >>= 1;
		msb >>= 1;
		i += 11;
	}
}

//
//###########################################################################
//###########################################################################
//
int
	Random::GetRandomInt()
{
	int
		indent,
		result;

	//
	//------------------------------------------------------------------
	// The random number generated will be the result of an XOR with the
	// element 103 positions further (wrapping around) in the table
	//------------------------------------------------------------------
	//
	indent = (Index>=147)?Index-147:Index+103;
	result = Numbers[Index]^Numbers[indent];

	//
	//------------------------------------------------------------------------
	// Replace the current random number with the new one generated, increment
	// the buffer index pointer, and return the number
	//------------------------------------------------------------------------
	//
	Numbers[Index] = result;
	if (++Index == ELEMENTS(Numbers))
		Index=0;
	return result;
}

//
//###########################################################################
//###########################################################################
//
Scalar
	Random::GetFraction()
{
	Scalar
		result;

	result = static_cast<Scalar>(GetRandomInt());
    // sebi
	result /= static_cast<Scalar>(GetRandMax() + 1);
	return result;
}

//
//###########################################################################
//###########################################################################
//
int
	Random::GetLessThan(int range)
{
	int
		result,
		max;
	max = GetRandMax() - ((GetRandMax() + 1)%range);
	do
	{
		result = GetRandomInt();
	} while (result>max);
	return result%range;
}

//
//###########################################################################
//###########################################################################
//
Die::Die(int n)
{
	dieSides = (n>1)?n:2;
	highestRandom = Random::GetRandMax() - ((Random::GetRandMax()+1)%dieSides);
}

//
//###########################################################################
//###########################################################################
//
Die::operator int() {
	int
		result;

	//
	//------------------------------------------------------------------------
	// In order to not skew the probabilities to the low numbers, make sure
	// that the random # used is not greater than the limit determined for the
	// number of sides.
	//------------------------------------------------------------------------
	//
	do
	{
		result = Random::GetInt();
	} while (result>highestRandom);

	//
	//-------------------------------------------------------------------
	// Once the base random number is determined, do modulus division and
	// increment by 1 to map into the die range.
	//-------------------------------------------------------------------
	//
	return result%dieSides + 1;
}
