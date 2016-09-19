//===========================================================================//
// File:	schain.cc                                                        //
// Contents: Implementation details of safe chains                           //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//
//###########################################################################
// SCHAINLINK
//###########################################################################
//

MemoryBlock*
	SafeChainLink::AllocatedMemory = NULL;

//
//###########################################################################
//###########################################################################
//
void
	SafeChainLink::InitializeClass(
		size_t block_count,
		size_t block_delta
	)
{
	Verify(!AllocatedMemory);
	AllocatedMemory =
		new MemoryBlock(
			sizeof(SafeChainLink),
			block_count,
			block_delta,
			"Stuff::SafeChainLink",
			ConnectionEngineHeap
		);
}

//
//###########################################################################
//###########################################################################
//
void
	SafeChainLink::TerminateClass()
{
	delete AllocatedMemory;
	AllocatedMemory = NULL;
}

//
//###########################################################################
// SafeChainLink
//###########################################################################
//
SafeChainLink::SafeChainLink(
	SafeChain *chain,
	Plug *plug,
	SafeChainLink *nextSafeChainLink,
	SafeChainLink *prevSafeChainLink
):
	Link(chain, plug)
{
	//
	//-------------------------
	// Link into existing chain
	//-------------------------
	//
	if ((this->nextSafeChainLink = nextSafeChainLink) != NULL)
	{
		Check_Object(nextSafeChainLink);
		nextSafeChainLink->prevSafeChainLink = this;
	}
	if ((this->prevSafeChainLink = prevSafeChainLink) != NULL)
	{
		Check_Object(prevSafeChainLink);
		prevSafeChainLink->nextSafeChainLink = this;
	}
}

//
//###########################################################################
// ~SafeChainLink
//###########################################################################
//
SafeChainLink::~SafeChainLink()
{
	Check_Object(this);
	SafeChain *chain = Cast_Object(SafeChain*, socket);

	//
	//---------------------------------------------
	// Notify iterators that the link is going away
	//---------------------------------------------
	//
	chain->SendIteratorMemo(PlugRemoved, this);

	//
	//---------------------------
	// Remove from existing links
	//---------------------------
	//
	if (prevSafeChainLink != NULL)
	{
		Check_Object(prevSafeChainLink);
		prevSafeChainLink->nextSafeChainLink = nextSafeChainLink;
	}
	else
	{
		Check_Object(chain);
		chain->head = nextSafeChainLink;
	}
	if (nextSafeChainLink != NULL)
	{
		Check_Object(nextSafeChainLink);
		nextSafeChainLink->prevSafeChainLink = prevSafeChainLink;
	}
	else
	{
   	Check_Object(chain);
		chain->tail = prevSafeChainLink;
	}
	prevSafeChainLink = nextSafeChainLink = NULL;

	//
	//------------------------------------------
	// Remove this link from any plug references
	//------------------------------------------
	//
	ReleaseFromPlug();

	//
	//-------------------------------------------------------------
	// Tell the node to release this link.  Note that this link
	// is not referenced by the plug or the chain at this point in
	// time.
	//-------------------------------------------------------------
	//
	if (chain->GetReleaseNode() != NULL)
	{
		Check_Object(chain->GetReleaseNode());
		chain->GetReleaseNode()->ReleaseLinkHandler(chain, plug);
	}
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	SafeChainLink::TestInstance()
{
	Link::TestInstance();
	
	if (prevSafeChainLink != NULL) 
	{
		Check_Signature(prevSafeChainLink);
	}
	if (nextSafeChainLink != NULL) 
	{
		Check_Signature(nextSafeChainLink);
	}
}

//
//###########################################################################
// SCHAIN
//###########################################################################
//

//
//###########################################################################
// SafeChain
//###########################################################################
//
SafeChain::SafeChain(
	Node *node
) :
SafeSocket(
	node
)
{
	head = NULL;
	tail = NULL;
}

//
//###########################################################################
// ~SafeChain
//###########################################################################
//
SafeChain::~SafeChain()
{
	Check_Object(this);
	SetReleaseNode(NULL);
	SafeChainLink *link = head;
	while (link)
	{
		Check_Object(link);
		SafeChainLink *next = link->nextSafeChainLink;
		Unregister_Object(link);
		delete link;
		link = next;
	}
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	SafeChain::TestInstance()
{
	SafeSocket::TestInstance();
	
	if (head != NULL)
	{
		Check_Object(head);
	}
}

//
//###########################################################################
// AddImplementation
//###########################################################################
//
void
	SafeChain::AddImplementation(
		Plug *plug
	)
{
	Check_Object(this);
	tail = new SafeChainLink(this, plug, NULL, tail);
	Register_Object(tail);
	if (head == NULL)
	{
		head = tail;
	}
}

//
//#############################################################################
// IsEmpty
//#############################################################################
//
bool
	SafeChain::IsEmpty()
{
	Check_Object(this);
	return (head == NULL);
}

//
//###########################################################################
// InsertPlug
//###########################################################################
//
SafeChainLink*
	SafeChain::InsertSafeChainLink(
		Plug *plug,
		SafeChainLink *link
	)
{
	Check_Object(this);
	Check_Object(link);
	Check_Object(plug);

	SafeChainLink *new_link =
		new SafeChainLink(
			this,
			plug,
			link,
			link->prevSafeChainLink
		);
	Register_Object(new_link);

	Check_Object(head);
	if (head == link)
	{
		head = new_link;
	}
	return new_link;
}

//
//###########################################################################
// SafeChainIterator
//###########################################################################
//
SafeChainIterator::SafeChainIterator(
	SafeChain *chain,
	bool move_next_on_remove
):
	SafeIterator(chain)
{
	Check_Object(chain);
	currentLink = chain->head;
	moveNextOnRemove = move_next_on_remove;
}

SafeChainIterator::SafeChainIterator(const SafeChainIterator &iterator):
	SafeIterator(Cast_Object(SafeChain*, iterator.socket))
{
	Check_Object(&iterator);
	currentLink = iterator.currentLink;
	moveNextOnRemove = iterator.moveNextOnRemove;
}

SafeChainIterator::~SafeChainIterator()
{
	Check_Object(this);
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	SafeChainIterator::TestInstance() const
{
	SafeIterator::TestInstance();
	
	if (currentLink != NULL) 
	{
		Check_Object(currentLink);
	}
}

//
//###########################################################################
// First
//###########################################################################
//
void
	SafeChainIterator::First()
{
	Check_Object(this);
	currentLink = Cast_Object(SafeChain*, socket)->head;
}

//
//###########################################################################
// Last
//###########################################################################
//
void
	SafeChainIterator::Last()
{
	Check_Object(this);
	currentLink = Cast_Object(SafeChain*, socket)->tail;
}

//
//###########################################################################
// Next
//###########################################################################
//
void
	SafeChainIterator::Next()
{
	Check_Object(this);
	Check_Object(currentLink);
	currentLink = currentLink->nextSafeChainLink;
}

//
//###########################################################################
// Previous
//###########################################################################
//
void
	SafeChainIterator::Previous()
{
	Check_Object(this);
	Check_Object(currentLink);
	currentLink = currentLink->prevSafeChainLink;
}

//
//###########################################################################
// ReadAndNextImplementation
//###########################################################################
//
void*
	SafeChainIterator::ReadAndNextImplementation()
{
	Check_Object(this);
	if (currentLink != NULL) 
	{
		Plug *plug;
		
		Check_Object(currentLink);
		plug = currentLink->GetPlug();
		currentLink = currentLink->nextSafeChainLink;
		return plug;
	}
	return NULL;
}

//
//###########################################################################
// ReadAndPreviousImplementation
//###########################################################################
//
void*
	SafeChainIterator::ReadAndPreviousImplementation()
{
	Check_Object(this);
	if (currentLink != NULL) 
	{
		Plug *plug;
		
		Check_Object(currentLink);
		plug = currentLink->plug;
		currentLink = currentLink->prevSafeChainLink;
		return plug;
	}
	return NULL;
}

//
//###########################################################################
// GetCurrentImplementation
//###########################################################################
//
void*
	SafeChainIterator::GetCurrentImplementation()
{
	Check_Object(this);
	if (currentLink != NULL) 
	{
		Check_Object(currentLink);
		return currentLink->GetPlug();
	}
	return NULL;
}

//
//###########################################################################
// GetSize
//###########################################################################
//
CollectionSize
	SafeChainIterator::GetSize()
{
	Check_Object(this);
	SafeChainLink *link;
	CollectionSize count;
	
	count = 0;
	for (
		link = Cast_Object(SafeChain*, socket)->head;
		link != NULL;
		link = link->nextSafeChainLink
	) 
	{
		Check_Object(link);
		count++;
	}
	return count;
}

//
//###########################################################################
// GetNthImplementation
//###########################################################################
//
void*
	SafeChainIterator::GetNthImplementation(
		CollectionSize index
	)
{
	Check_Object(this);
	SafeChainLink *link;
	CollectionSize count;
	
	count = 0;
	for (
		link = Cast_Object(SafeChain*, socket)->head;
		link != NULL;
		link = link->nextSafeChainLink
	) 
	{
		Check_Object(link);
		if (count == index) 
		{
			currentLink = link;
			return currentLink->GetPlug();
		}
		count++;
	}
	return NULL;
}

//
//###########################################################################
// Remove
//###########################################################################
//
void
	SafeChainIterator::Remove()
{
	Check_Object(this);
	Check_Object(currentLink);
	Unregister_Object(currentLink);
	delete currentLink;
}

//
//#############################################################################
// InsertImplementation
//#############################################################################
//
void
	SafeChainIterator::InsertImplementation(Plug *plug)
{
	Check_Object(this);
	currentLink =
		Cast_Object(SafeChain*, socket)->InsertSafeChainLink(plug, currentLink);
	Check_Object(currentLink);
}

//
//###########################################################################
// ReceiveMemo
//###########################################################################
//
void
	SafeChainIterator::ReceiveMemo(
		IteratorMemo memo,
		void *content
	)
{
	Check_Object(this);
	if (memo == PlugRemoved) 
	{
		if (content == currentLink)
		{
			if (moveNextOnRemove)
			{
				Next();
			}
			else
			{
				Previous();
			}
		}
	}
}
