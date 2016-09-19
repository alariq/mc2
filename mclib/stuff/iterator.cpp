//===========================================================================//
// File:	iterator.cc                                                      //
// Contents: Implementation details of base iterator                         //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//


#include"stuffheaders.hpp"

//
//###########################################################################
// First
//###########################################################################
//
void
	Iterator::First()
{
	STOP(("Iterator::First - Should never reach here"));
}

//
//###########################################################################
// Last
//###########################################################################
//
void
	Iterator::Last()
{
	STOP(("Iterator::Last - Should never reach here"));
}

//
//###########################################################################
// Next
//###########################################################################
//
void
	Iterator::Next()
{
	STOP(("Iterator::Next - Should never reach here"));
}

//
//###########################################################################
// Previous
//###########################################################################
//
void
	Iterator::Previous()
{
	STOP(("Iterator::Previous - Should never reach here"));
}

//
//###########################################################################
// GetSize
//###########################################################################
//
CollectionSize
	Iterator::GetSize()
{
	CollectionSize i = 0;
	void *item;

	First();
	while ((item = GetCurrentImplementation()) != NULL)
	{
		i++;
      Next();
	}
   return i;
}

//
//###########################################################################
// ReadAndNextImplementation
//###########################################################################
//
void
	*Iterator::ReadAndNextImplementation()
{
	void *item;

	if ((item = GetCurrentImplementation()) != NULL)
	{
		Next();
	}
	return item;
}

//
//###########################################################################
// ReadAndPreviousImplementation
//###########################################################################
//
void
	*Iterator::ReadAndPreviousImplementation()
{
	void *item;

	if ((item = GetCurrentImplementation()) != NULL)
	{
		Previous();
	}
	return item;
}

//
//###########################################################################
// GetCurrentImplementation
//###########################################################################
//
void*
	Iterator::GetCurrentImplementation()
{
	STOP(("Iterator::GetCurrentImplementation - Should never reach here"));
   return NULL;
}

//
//###########################################################################
// GetNthImplementation
//###########################################################################
//
void
	*Iterator::GetNthImplementation(CollectionSize index)
{
	CollectionSize i = 0;
	void *item;

	First();
	while ((item = GetCurrentImplementation()) != NULL)
	{
		if (i == index)
			return item;
      Next();
		i++;
	}
	return NULL;
}

