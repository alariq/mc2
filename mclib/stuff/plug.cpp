//===========================================================================//
// File:	plug.cc                                                          //
// Contents: Implementation details for plugs and their iterators            //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//#############################################################################
//###############################    Plug    ##################################
//#############################################################################

Plug::ClassData*
	Plug::DefaultData = NULL;

HGOSHEAP
	Stuff::ConnectionEngineHeap = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Plug::InitializeClass()
{
	Verify(!ConnectionEngineHeap);
	ConnectionEngineHeap = gos_CreateMemoryHeap("ConnectionEngine");
	Check_Pointer(ConnectionEngineHeap);

	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			PlugClassID,
			"Stuff::Plug",
			RegisteredClass::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Plug::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;

	Check_Pointer(ConnectionEngineHeap);
	gos_DestroyMemoryHeap(ConnectionEngineHeap);
	ConnectionEngineHeap = NULL;
}

//
//#############################################################################
// Plug
//#############################################################################
//
Plug::Plug(ClassData *class_data):
	RegisteredClass(class_data)
{
	linkHead = NULL;
}

//
//###########################################################################
// ~Plug
//###########################################################################
//
Plug::~Plug()
{
	Check_Object(this);
	Link *link = linkHead;
	while (link)
	{
		Check_Object(link);
		Link *next = link->nextLink;
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
	Plug::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
	if (linkHead != NULL)
	{
		Check_Object(linkHead);
	}
}

//
//#############################################################################
// RemoveSocket
//#############################################################################
//
void
	Plug::RemoveSocket(Socket *socket)
{
	Check_Object(this);
	Check_Object(socket);

	Link *link;

	for (link = linkHead; link != NULL; link = link->nextLink)
	{
		Check_Object(link);
      if (link->GetSocket() == socket)
      {
         Unregister_Object(link);
         delete link;
         return;
      }
	}
}

//
//#############################################################################
// IsSocketMember
//#############################################################################
//
bool
	Plug::IsSocketMember(Socket *socket)
{
	Check_Object(this);
	Check_Object(socket);

	Link *link;

	for (link = linkHead; link != NULL; link = link->nextLink)
	{
		Check_Object(link);
      if (link->GetSocket() == socket)
      {
         return true;
      }
	}
	return false;
}

//
//#############################################################################
// GetSocketCount
//#############################################################################
//
CollectionSize
	Plug::GetSocketCount()
{
	Check_Object(this);

	CollectionSize socket_count = 0;
	Link *link;

	for (link = linkHead; link != NULL; link = link->nextLink)
	{
		Check_Object(link);
		socket_count++;
	}
	return socket_count;
}

//#############################################################################
//###########################    PlugIterator    ##############################
//#############################################################################

//
//#############################################################################
// PlugIterator
//#############################################################################
//
PlugIterator::PlugIterator(
	Plug *plug,
   RegisteredClass::ClassID class_to_iterate
)
{
	Check_Object(plug);
	this->plug = plug;
   classToIterate = class_to_iterate;
	currentLink = plug->linkHead;
	NextNode();
}

PlugIterator::PlugIterator(const PlugIterator &iterator)
{
	Check_Object(&iterator);
	plug = iterator.plug;
   classToIterate = iterator.classToIterate;
	currentLink = iterator.currentLink;
	NextNode();
}

//
//#############################################################################
// PlugIterator
//#############################################################################
//
PlugIterator::~PlugIterator()
{
	Check_Object(this);
}

//
//#############################################################################
// TestInstance
//#############################################################################
//
void
	PlugIterator::TestInstance() const
{
	if (currentLink != NULL)
	{
		Check_Object(currentLink);
	}
}

//
//#############################################################################
// First
//#############################################################################
//
void
	PlugIterator::First()
{
	Check_Object(this);
	Check_Object(plug);
	currentLink = plug->linkHead;
   NextNode();
}

//
//#############################################################################
// Last
//#############################################################################
//
void
	PlugIterator::Last()
{
	Check_Object(this);
	if (currentLink == NULL)
	{
		Check_Object(plug);
		if ((currentLink = plug->linkHead) == NULL)
			return;
	}

	Check_Object(currentLink);
	while(currentLink->nextLink != NULL)
	{
		currentLink = currentLink->nextLink;
		Check_Object(currentLink);
	}
   PreviousNode();
}

//
//#############################################################################
// Next
//#############################################################################
//
void
	PlugIterator::Next()
{
	Check_Object(this);
	Check_Object(currentLink);
	currentLink = currentLink->nextLink;
   NextNode();
}

//
//#############################################################################
// Previous
//#############################################################################
//
void
	PlugIterator::Previous()
{
	Check_Object(this);
	Check_Object(currentLink);
	currentLink = currentLink->prevLink;
   PreviousNode();
}

//
//#############################################################################
// ReadAndNextImplementation
//#############################################################################
//
void*
	PlugIterator::ReadAndNextImplementation()
{
	Check_Object(this);
	if (currentLink != NULL)
	{
		Node *node;

		Check_Object(currentLink);
		Check_Object(currentLink->socket);
		node = currentLink->socket->GetReleaseNode();
		currentLink = currentLink->nextLink;
		NextNode();
		return node;
	}
	return NULL;
}

//
//#############################################################################
// ReadAndPreviousImplementation
//#############################################################################
//
void*
	PlugIterator::ReadAndPreviousImplementation()
{
	Check_Object(this);
	if (currentLink != NULL)
	{
		Node *node;

		Check_Object(currentLink);
		Check_Object(currentLink->socket);
		node = currentLink->socket->GetReleaseNode();
		currentLink = currentLink->prevLink;
      PreviousNode();
		return node;
	}
	return NULL;
}

//
//#############################################################################
// GetCurrentImplementation
//#############################################################################
//
void*
	PlugIterator::GetCurrentImplementation()
{
	Check_Object(this);
	if (currentLink != NULL)
	{
		Check_Object(currentLink);
		Check_Object(currentLink->socket);
		return currentLink->socket->GetReleaseNode();
	}
	return NULL;
}

//
//#############################################################################
// GetSize
//#############################################################################
//
CollectionSize
	PlugIterator::GetSize()
{
	Check_Object(this);
	CollectionSize i = 0;

	First();
	while (GetCurrentImplementation() != NULL)
	{
		i++;
      Next();
	}
	return i;
}

//
//#############################################################################
// GetNthImplementation
//#############################################################################
//
void*
	PlugIterator::GetNthImplementation(
		CollectionSize index
	)
{
	Check_Object(this);
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

//
//#############################################################################
// NextNode
//#############################################################################
//
void
	PlugIterator::NextNode()
{
	Check_Object(this);
   while (currentLink != NULL)
   {
		Node *node;

   	Check_Object(currentLink);
      Check_Object(currentLink->socket);
		if ((node = currentLink->socket->GetReleaseNode()) != NULL)
      {
      	Check_Object(node);
         if (
				classToIterate == NullClassID ||
				classToIterate == node->GetClassID()
         )
         {
            return;
         }
      }
      currentLink = currentLink->nextLink;
   }
   return;
}

//
//###########################################################################
// PreviousNode
//###########################################################################
//
void
	PlugIterator::PreviousNode()
{
	Check_Object(this);
   while (currentLink != NULL)
   {
		Node *node;

   	Check_Object(currentLink);
      Check_Object(currentLink->socket);
		if ((node = currentLink->socket->GetReleaseNode()) != NULL)
      {
      	Check_Object(node);
         if (
				classToIterate == NullClassID ||
				classToIterate == node->GetClassID()
         )
         {
            return;
         }
      }
      currentLink = currentLink->prevLink;
   }
   return;
}

//
//###########################################################################
// Remove
//###########################################################################
//
void
	PlugIterator::Remove()
{
	Check_Object(this);
	Link *old_link;

	Check_Object(currentLink);

	old_link = currentLink;
	currentLink = currentLink->nextLink;

	Unregister_Object(old_link);
	delete old_link;
}
