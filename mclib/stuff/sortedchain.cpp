//===========================================================================//
// File:	vchain.cc                                                        //
// Contents: Implementation details of SortedChain class                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SortedChainLink ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//###########################################################################
// SortedChainLink
//###########################################################################
//
SortedChainLink::SortedChainLink(
	SortedChain *vchain,
	Plug *plug
):
	Link(vchain, plug)
{
	next = NULL;
	prev = NULL;
}

//
//###########################################################################
// ~SortedChainLink
//###########################################################################
//
SortedChainLink::~SortedChainLink()
{
	Check_Object(this);
	SortedChain *vchain = Cast_Object(SortedChain*, socket);

	//
	//-------------------------------------------
	// Notify iterators that deletion is occuring
	//-------------------------------------------
	//
	vchain->SendIteratorMemo(PlugRemoved, this);

	//
	//---------------------------
	// Remove from existing links
	//---------------------------
	//
	if (prev != NULL)
	{
		Check_Object(prev);
		prev->next = next;
	}
	else
	{
		Check_Object(vchain);
		vchain->head = next;
	}
	if (next != NULL)
	{
		Check_Object(next);
		next->prev = prev;
	}
	else
	{
   	Check_Object(vchain);
		vchain->tail = prev;
	}
	prev = next = NULL;

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
	if (vchain->GetReleaseNode() != NULL)
	{
		Check_Object(vchain->GetReleaseNode());
		vchain->GetReleaseNode()->ReleaseLinkHandler(vchain, plug);
	}
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	SortedChainLink::TestInstance()
{
	Link::TestInstance();
	
	if (next != NULL)
	{
		Check_Signature(next);
	}
	if (prev != NULL)
	{
		Check_Signature(prev);
	}
}

//
//###########################################################################
// SetupSortedChainLinks
//###########################################################################
//
void
	SortedChainLink::SetupSortedChainLinks(
		SortedChainLink *next,
		SortedChainLink *prev
	)
{
	Check_Object(this);
	this->next = next;
	this->prev = prev;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SortedChain ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//###########################################################################
// SortedChain
//###########################################################################
//
SortedChain::SortedChain(
	Node *node,
	bool has_unique_entries
):
	SortedSocket(node, has_unique_entries)
{
	head = NULL;
	tail = NULL;
}

//
//###########################################################################
// ~SortedChain
//###########################################################################
//
SortedChain::~SortedChain()
{
	Check_Object(this);
	SetReleaseNode(NULL);
	SortedChainLink *link = head;
	while (link)
	{
		Check_Object(link);
		SortedChainLink *next = link->next;
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
	SortedChain::TestInstance()
{
	SortedSocket::TestInstance();
	
	if (head != NULL)
	{
		Check_Object(head);
	}
	if (tail != NULL)
	{
		Check_Object(tail);
	}
}

//
//###########################################################################
// MakeSortedChainLink
//###########################################################################
//
SortedChainLink
   *SortedChain::MakeSortedChainLink(
      Plug*,
      const void*
   )
{
	Check_Object(this);
	STOP(("SortedChain::MakeSortedChainLink - Should never reach here"));
   return NULL;
}

//
//###########################################################################
// CompareSortedChainLinks
//###########################################################################
//
int
   SortedChain::CompareSortedChainLinks(
      SortedChainLink*,
      SortedChainLink*
   )
{
	Check_Object(this);
	STOP(("SortedChain::CompareSortedChainLinks - Should never reach here"));
   return 0;
}

//
//###########################################################################
// CompareValueToSortedChainLink
//###########################################################################
//
int
   SortedChain::CompareValueToSortedChainLink(
      const void*,
      SortedChainLink*
   )
{
	Check_Object(this);
	STOP(("SortedChain::CompareValueToSortedChainLink - Should never reach here"));
   return 0;
}

//
//###########################################################################
// AddImplementation
//###########################################################################
//
void
	SortedChain::AddImplementation(
		Plug *plug
	)
{
	Check_Object(this);
	STOP(("Must use AddValue call"));
	// AddValueImplementation(plug, NULL);	
}

//
//###########################################################################
// AddValueImplementation
//###########################################################################
//
void
	SortedChain::AddValueImplementation(
		Plug *plug,
		const void *value
	)
{
	Check_Object(this);
	SortedChainLink *link;
	
	//
	//-------------------------------------------------------------
	// Verify that value has not been added
	//-------------------------------------------------------------
	//
	Verify(HasUniqueEntries() ? (SearchForValue(value) == NULL) : (bool)true);

	//
	//-------------------------------------------------------------
	// Make new vchain link
	//-------------------------------------------------------------
	//
	link = MakeSortedChainLink(plug, value);
	Register_Object(link);

	//
	//-------------------------------------------------------------
	// Find insertion point for the new link
	//-------------------------------------------------------------
	//
	if (head == NULL)
	{
		link->SetupSortedChainLinks(NULL, NULL);
		head = link;
      tail = link;
	}
	else
	{
		SortedChainLink *greater_link;

		Check_Object(head);
      Check_Object(tail);

		for (
			greater_link = head;
			greater_link != NULL;
			greater_link = greater_link->next
		)
		{
			Check_Object(greater_link);
			if (CompareValueToSortedChainLink(value, greater_link) < 0)
				break;
		}

		if (greater_link == NULL)
		{
			//
			// Add after tail
			//
			link->SetupSortedChainLinks(NULL, tail);
			tail->next = link;
			tail = link;
		}
		else if (greater_link == head)
		{
			//
			// Add before head
			//
			link->SetupSortedChainLinks(head, NULL);
			head->prev = link;
			head = link;
		}
		else
		{
			//
			// Add before greater_link
			//
			link->SetupSortedChainLinks(greater_link, greater_link->prev);
			greater_link->prev->next = link;
			greater_link->prev = link;
		}
	}

	SendIteratorMemo(PlugAdded, link);
}

//
//###########################################################################
// FindImplementation
//###########################################################################
//
Plug*
	SortedChain::FindImplementation(
		const void *value
	)
{
	Check_Object(this);
	SortedChainLink *link;

	if ((link = SearchForValue(value)) != NULL)
	{
		Check_Object(link);
		return link->GetPlug();
	}
	return NULL;
}

//
//#############################################################################
// IsEmpty
//#############################################################################
//
bool
	SortedChain::IsEmpty()
{
	Check_Object(this);
	return (head == NULL);
}

//
//###########################################################################
// SearchForValue
//###########################################################################
//
SortedChainLink*
	SortedChain::SearchForValue(
		const void *value
	)
{
	Check_Object(this);
	SortedChainLink *link;
	int ret;
	
	for (link = head; link != NULL; link = link->next)
	{
		Check_Object(link);
		if ((ret = CompareValueToSortedChainLink(value, link)) == 0)
			break;
		if (ret < 0)
      	return(NULL);
	}
	return link;
}

//
//###########################################################################
// SortedChainIterator
//###########################################################################
//
SortedChainIterator::SortedChainIterator(SortedChain *vchain):
	SortedIterator(vchain)
{
	Check_Object(vchain);
	currentLink = vchain->head;
}

//
//###########################################################################
// SortedChainIterator
//###########################################################################
//
SortedChainIterator::SortedChainIterator(const SortedChainIterator *iterator):
	SortedIterator(Cast_Object(SortedChain*, iterator->socket))
{
	Check_Object(iterator);
	currentLink = iterator->currentLink;
}

Iterator*
	SortedChainIterator::MakeClone()
{
	Check_Object(this);
	return new SortedChainIterator(*this);
}

//
//###########################################################################
//###########################################################################
//
SortedChainIterator::~SortedChainIterator()
{
	Check_Object(this);
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	SortedChainIterator::TestInstance() const
{
	SortedIterator::TestInstance();
	
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
	SortedChainIterator::First()
{
	Check_Object(this);
	currentLink = Cast_Object(SortedChain*, socket)->head;
}

//
//###########################################################################
// Last
//###########################################################################
//
void
	SortedChainIterator::Last()
{
	Check_Object(this);
	currentLink = Cast_Object(SortedChain*, socket)->tail;
}

//
//###########################################################################
// Next
//###########################################################################
//
void
	SortedChainIterator::Next()
{
	Check_Object(this);
	Check_Object(currentLink);
	currentLink = currentLink->next;
}

//
//###########################################################################
// Previous
//###########################################################################
//
void
	SortedChainIterator::Previous()
{
	Check_Object(this);
	Check_Object(currentLink);
	currentLink = currentLink->prev;
}

//
//###########################################################################
// ReadAndNextImplementation
//###########################################################################
//
void
	*SortedChainIterator::ReadAndNextImplementation()
{
	Check_Object(this);
	if (currentLink != NULL)
	{
		Plug *plug;

		Check_Object(currentLink);
		plug = currentLink->GetPlug();
		currentLink = currentLink->next;
		return plug;
	}
	return NULL;
}

//
//###########################################################################
// ReadAndPreviousImplementation
//###########################################################################
//
void
	*SortedChainIterator::ReadAndPreviousImplementation()
{
	Check_Object(this);
	if (currentLink != NULL) 
	{
		Plug *plug;
		
		Check_Object(currentLink);
		plug = currentLink->plug;
		currentLink = currentLink->prev;
		return plug;
	}
	return NULL;
}

//
//###########################################################################
// GetCurrentImplementation
//###########################################################################
//
void
	*SortedChainIterator::GetCurrentImplementation()
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
	SortedChainIterator::GetSize()
{
	Check_Object(this);
	SortedChainLink *link;
	CollectionSize count;

	count = 0;
	for (
		link = Cast_Object(SortedChain*, socket)->head;
		link != NULL;
		link = link->next
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
void
	*SortedChainIterator::GetNthImplementation(
		CollectionSize index
	)
{
	Check_Object(this);
	SortedChainLink *link;
	CollectionSize count;
	
	count = 0;
	for (
		link = Cast_Object(SortedChain*, socket)->head;
		link != NULL;
		link = link->next
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
	SortedChainIterator::Remove()
{
	Check_Object(this);
	Check_Object(currentLink);
	Unregister_Object(currentLink);
	delete currentLink;
}

//
//###########################################################################
// FindImplementation
//###########################################################################
//
Plug*
	SortedChainIterator::FindImplementation(
		const void *value
	)
{
	Check_Object(this);
	SortedChainLink *link;
	
	if ((link = Cast_Object(SortedChain*, socket)->SearchForValue(value)) != NULL)
	{
		Check_Object(link);
		return (currentLink = link)->GetPlug();
	}
	return NULL;
}

//
//###########################################################################
// ReceiveMemo
//###########################################################################
//
void
	SortedChainIterator::ReceiveMemo(
		IteratorMemo memo,
		void *content
	)
{
	Check_Object(this);
	if (memo == PlugRemoved) 
	{
		if (content == currentLink)
		{
			Next();
		}
	}
}
