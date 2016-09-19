//===========================================================================//
// File:	slot.cc                                                          //
// Contents: Implementation details for the slot class                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

MemoryBlock*
	SlotLink::AllocatedMemory = NULL;

//
//#############################################################################
//#############################################################################
//
void
	SlotLink::InitializeClass(
		size_t block_count,
		size_t block_delta
	)
{
	Verify(!AllocatedMemory);
	AllocatedMemory =
		new MemoryBlock(
			sizeof(SlotLink),
			block_count,
			block_delta,
			"Stuff::SlotLink",
			ConnectionEngineHeap
		);
}

//
//#############################################################################
//#############################################################################
//
void
	SlotLink::TerminateClass()
{
	delete AllocatedMemory;
	AllocatedMemory = NULL;
}

//
//###########################################################################
// SlotLink
//###########################################################################
//
SlotLink::SlotLink(
	Slot *slot,
	Plug *plug
):
	Link(slot, plug)
{
}

//
//###########################################################################
// ~SlotLink
//###########################################################################
//
SlotLink::~SlotLink()
{
	Check_Object(this);
	Slot *slot = Cast_Object(Slot*, socket);
	
	//
	//-------------------------------------------------
	// Make sure the link is not referenced by the slot
	//-------------------------------------------------
	//
	Check_Object(slot);
	Verify(slot->slotLink == this);
	slot->slotLink = NULL;

	//
	//------------------------------------------
	// Remove this link from any plug references
	//------------------------------------------
	//
	ReleaseFromPlug();

	//
	//-------------------------------------------------------------
	// Tell the node to release this link.  Note that this link
	// is not referenced by the plug or the slot at this point in
	// time.
	//-------------------------------------------------------------
	//
	if (slot->GetReleaseNode() != NULL)
	{
		Check_Object(slot->GetReleaseNode());
		slot->GetReleaseNode()->ReleaseLinkHandler(slot, plug);
	}
}

//
//###########################################################################
// Slot
//###########################################################################
//
Slot::Slot(
	Node *node
):
	Socket(node)
{
	slotLink = NULL;
}

//
//###########################################################################
// ~Slot
//###########################################################################
//
Slot::~Slot()
{
	Check_Object(this);
	SetReleaseNode(NULL);
	if (slotLink != NULL)
	{
		Unregister_Object(slotLink);
		delete slotLink;
	}
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	Slot::TestInstance()
{
	Socket::TestInstance();
	if (slotLink != NULL)
	{
		Check_Object(slotLink);
	}
}

//
//###########################################################################
// Remove
//###########################################################################
//
void
	Slot::Remove()
{
	Check_Object(this);
	if (slotLink != NULL)
	{
		Unregister_Object(slotLink);
		delete slotLink;
		slotLink = NULL;
	}
}

//
//###########################################################################
// AddImplementation
//###########################################################################
//
void
	Slot::AddImplementation(
		Plug *plug
	)
{
	Check_Object(this);
	Verify(slotLink == NULL);
	slotLink = new SlotLink(this, plug);
	Register_Object(slotLink);
}

//
//###########################################################################
// GetCurrentPlug
//###########################################################################
//
Plug*
	Slot::GetCurrentPlug()
{
	Check_Object(this);
	if (slotLink != NULL)
	{
		Check_Object(slotLink);
		return slotLink->GetPlug();
	}
	return NULL;
}

