//===========================================================================//
// File:	srtskt.cc                                                        //
// Contents: Implementation of sorted socket class                           //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

SortedSocket::SortedSocket(
	Node *node,
	bool has_unique_entries
):
	SafeSocket(node)
{
	hasUniqueEntries = has_unique_entries;
}

SortedSocket::~SortedSocket()
{
	Check_Object(this);
}

void
	SortedSocket::AddValueImplementation(
		Plug*,
		const void*
	)
{
	Check_Object(this);
	STOP(("SortedSocket::AddValueImplementation - Should never reach here"));
}

Plug*
	SortedSocket::FindImplementation(const void*)
{
	Check_Object(this);
	STOP(("SortedSocket::FindImplementation - Should never reach here"));
	return NULL;
}

SortedIterator::SortedIterator(SortedSocket *sortedSocket):
	SafeIterator(sortedSocket)
{
}

SortedIterator::~SortedIterator()
{
	Check_Object(this);
}

Plug*
	SortedIterator::FindImplementation(const void*)
{
	Check_Object(this);
	STOP(("SortedIterator::FindImplementation - Should never reach here"));
	return NULL;
}

void*
	SortedIterator::GetValueImplementation()
{
	Check_Object(this);
	STOP(("SortedIterator::GetValueImplementation - Should never reach here"));
	return NULL;
}

