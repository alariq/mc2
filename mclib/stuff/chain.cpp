//===========================================================================//
// File:	chain.cc                                                         //
// Contents: Implementation details of chains and their iterators            //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ChainLink ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MemoryBlock*
	ChainLink::AllocatedMemory = NULL;

//
//#############################################################################
//#############################################################################
//
void
	ChainLink::InitializeClass(
		size_t block_count,
		size_t block_delta
	)
{
	Verify(!AllocatedMemory);
	AllocatedMemory =
		new MemoryBlock(
			sizeof(ChainLink),
			block_count,
			block_delta,
			"Stuff::ChainLink",
			ConnectionEngineHeap
		);
}

//
//#############################################################################
//#############################################################################
//
void
	ChainLink::TerminateClass()
{
	delete AllocatedMemory;
	AllocatedMemory = NULL;
}

//
//#############################################################################
// ChainLink
//#############################################################################
//
ChainLink::ChainLink(
	Chain *chain,
	Plug *plug,
	ChainLink *next_link,
	ChainLink *prev_link
):
	Link(chain, plug)
{
	//
	//-------------------------
	// Link into existing chain
	//-------------------------
	//
	if ((nextChainLink = next_link) != NULL)
	{
		Check_Object(nextChainLink);
		nextChainLink->prevChainLink = this;
	}
	if ((prevChainLink = prev_link) != NULL)
	{
		Check_Object(prevChainLink);
		prevChainLink->nextChainLink = this;
	}
}

//
//#############################################################################
// ~ChainLink
//#############################################################################
//
ChainLink::~ChainLink()
{
	Check_Object(this);
	Chain *chain = Cast_Object(Chain*, socket);
	
	//
	//--------------------------------------
	// Remove this link from the linked list
	//--------------------------------------
	//
	if (prevChainLink != NULL)
	{
		Check_Object(prevChainLink);
		prevChainLink->nextChainLink = nextChainLink;
	}
	else
	{
		Check_Object(chain);
		chain->head = nextChainLink;
	}
	if (nextChainLink != NULL)
	{
		Check_Object(nextChainLink);
		nextChainLink->prevChainLink = prevChainLink;
	}
	else
	{
		Check_Object(chain);
		chain->tail = prevChainLink;
	}
	prevChainLink = nextChainLink = NULL;

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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Chain ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
// Chain
//#############################################################################
//
Chain::Chain(Node *node):
	Socket(node)
{
	head = NULL;
	tail = NULL;
}

//
//#############################################################################
// ~Chain
//#############################################################################
//
Chain::~Chain()
{
	Check_Object(this);
	SetReleaseNode(NULL);
	ChainLink *link = head;
	while (link)
	{
		Check_Object(link);
		ChainLink *next = link->nextChainLink;
		Unregister_Object(link);
		delete link;
		link = next;
	}
}

//
//#############################################################################
// AddImplementation
//#############################################################################
//
void
	Chain::AddImplementation(Plug *plug)
{
	Check_Object(this);

	tail = new ChainLink(this, plug, NULL, tail);
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
	Chain::IsEmpty()
{
	Check_Object(this);
	return (head == NULL);
}

//
//#############################################################################
// InsertChainLink
//#############################################################################
//
ChainLink*
	Chain::InsertChainLink(
		Plug *plug,
		ChainLink *current_link
	)
{
	Check_Object(this);
	Check_Object(plug);
	Check_Object(current_link);

	ChainLink *new_link =
		new ChainLink(
			this,
			plug,
			current_link,
			current_link->prevChainLink
		);
	Register_Object(new_link);

	Check_Object(head);
	if (head == current_link)
	{
		head = new_link;
	}
	return new_link;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ChainIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//###########################################################################
// First
//###########################################################################
//
void
	ChainIterator::First()
{
	Check_Object(this);
	currentLink = Cast_Object(Chain*, socket)->head;
}

//
//###########################################################################
// Last
//###########################################################################
//
void
	ChainIterator::Last()
{
	Check_Object(this);
	currentLink = Cast_Object(Chain*, socket)->tail;
}

//
//###########################################################################
// Next
//###########################################################################
//
void
	ChainIterator::Next()
{
	Check_Object(this);
	Check_Object(currentLink);
	currentLink = currentLink->nextChainLink;
}

//
//###########################################################################
// Previous
//###########################################################################
//
void
	ChainIterator::Previous()
{
	Check_Object(this);
	Check_Object( currentLink );
	currentLink = currentLink->prevChainLink;
}

//
//###########################################################################
// ReadAndPreviousImplementation
//###########################################################################
//
void*
	ChainIterator::ReadAndPreviousImplementation()
{
	Check_Object(this);
	if (currentLink != NULL)
	{
		Plug *plug;

		Check_Object(currentLink);
		plug = currentLink->plug;
		currentLink = currentLink->prevChainLink;
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
	ChainIterator::GetCurrentImplementation()
{
	Check_Object(this);
	if (currentLink != NULL)
	{
		Check_Object(currentLink);
		return currentLink->plug;
	}
	return NULL;
}

//
//###########################################################################
// GetSize
//###########################################################################
//
CollectionSize
	ChainIterator::GetSize()
{
	Check_Object(this);

	ChainLink *link;
	CollectionSize count;

	count = 0;

	for (
		link = Cast_Object(Chain*, socket)->head;
		link != NULL;
		link = link->nextChainLink
	) {
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
	ChainIterator::GetNthImplementation(CollectionSize index)
{
	Check_Object(this);
	ChainLink *link;
	CollectionSize count;

	count = 0;
	for (
		link = Cast_Object(Chain*, socket)->head;
		link != NULL;
		link = link->nextChainLink
	) {
		Check_Object(link);
		if (count == index) {
			currentLink = link;
			return currentLink->plug;
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
	ChainIterator::Remove()
{
	Check_Object(this);
	ChainLink *oldLink = currentLink;

	Check_Object(currentLink);
	currentLink = currentLink->nextChainLink;

	Unregister_Object(oldLink);
	delete oldLink ;
}

//
//###########################################################################
// InsertImplementation
//###########################################################################
//
void
	ChainIterator::InsertImplementation(Plug *plug)
{
	Check_Object(this);
	currentLink =
		Cast_Object(Chain*, socket)->InsertChainLink(plug, currentLink);
	Check_Object(currentLink);
}
