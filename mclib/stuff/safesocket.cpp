//===========================================================================//
// File:	sfeskt.cc                                                        //
// Contents: Implementation details for safe socket class                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//###########################################################################
// SafeSocket
//###########################################################################
//	
SafeSocket::SafeSocket(Node *node):
	Socket(node)
{
	iteratorHead = NULL;
}

//
//###########################################################################
// ~SafeSocket
//###########################################################################
//	
SafeSocket::~SafeSocket()
{
	Check_Object(this);
	Verify(iteratorHead == NULL);
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	SafeSocket::TestInstance()
{
	Socket::TestInstance();
	if (iteratorHead != NULL)
	{
		Check_Signature(iteratorHead);
	}
}

//
//###########################################################################
// SendIteratorMemo
//###########################################################################
//
void
	SafeSocket::SendIteratorMemo(
		IteratorMemo memo,
		void *content
	)
{
	Check_Object(this);
	SafeIterator *iterator;
	
	for (
		iterator = iteratorHead;
		iterator != NULL;
		iterator = iterator->nextIterator
	) {
		Check_Object(iterator);
		iterator->ReceiveMemo(memo, content);
	}
}

//
//###########################################################################
// SafeIterator
//###########################################################################
//
SafeIterator::SafeIterator(SafeSocket *safeSocket):
	SocketIterator(safeSocket)
{
	//
	// Link iterator into sockets set of iterators
	//
   Check_Object(safeSocket);
	if ((nextIterator = safeSocket->iteratorHead) != NULL)
	{
		Check_Object(nextIterator);
		nextIterator->prevIterator = this;
	}
	prevIterator = NULL;
	safeSocket->iteratorHead = this;
}

//
//###########################################################################
// ~SafeIterator
//###########################################################################
//
SafeIterator::~SafeIterator()
{
	Check_Object(this);
	SafeSocket *safeSocket = Cast_Object(SafeSocket*, socket);

	//
	// Remove iterator from sockets set of iterators
	//
	Check_Object(safeSocket);
	if (safeSocket->iteratorHead == this)
	{
		safeSocket->iteratorHead = nextIterator;
	}
	if (prevIterator != NULL) 
	{
		Check_Object(prevIterator);
		prevIterator->nextIterator = nextIterator;
	}
	if (nextIterator != NULL) 
	{
		Check_Object(nextIterator);
		nextIterator->prevIterator = prevIterator;
	}
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	SafeIterator::TestInstance() const
{
	SocketIterator::TestInstance();

	if (prevIterator != NULL) 
	{
		Check_Signature(prevIterator);
	}
	if (nextIterator != NULL) 
	{
		Check_Signature(nextIterator);
	}
}

//
//###########################################################################
// ReceiveMemo
//###########################################################################
//
void
	SafeIterator::ReceiveMemo(
      IteratorMemo,
      void*
   )
{
	Check_Object(this);
	STOP(("SafeIterator::ReceiveMemo - Should never reach here"));
}

