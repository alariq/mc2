//---------------------------------------------------------------------------
//
// LList.h -- Prototype for linked lists
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef LLIST_H
#define LLIST_H
//---------------------------------------------------------------------------
// Include Files
#ifndef DSTD_H
#include"dstd.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions
#ifndef NULL
#define NULL 0L
#endif

//---------------------------------------------------------------------------
// Structure and Class Definitions

struct Link
{
	Link *Next;

	Link (Link *InitNext = NULL)
	{
		Next = InitNext;
	}

	Link *GetNext (void)
	{
		return Next;
	}

	virtual ~Link (void) {}
};

//---------------------------------------------------------------------------
class LinkedList
{
  protected:

	Link *Head;
	Link *Tail;

  public:

	LinkedList (void)
	{
		Head = Tail = NULL;
	}

	virtual ~LinkedList ()
	{
		Kill();
	}

	void AddToHead (Link *Node);
	void AddToTail (Link *Node);

	void Remove (Link *Node, Link *Last=NULL);          // The second param is if you know the previous pointer, you can optimize
	void Destroy (Link *Node, Link *Last = NULL);       // The 'Last' parameter is optional in all of the methods that have it.

	void InsertAfter (Link *PlaceTo, Link *NodeToPut);  // If sent, it avoids a search through the list.
	void InsertBefore (Link *PlaceTo, Link *NodeToPut);

	void Kill();

	void MoveAfter (Link *PlaceTo, Link *NodeToPut, Link *Last = NULL);

	bool Traverse (Link *&Current);

	bool IsEmpty (void)
	{
		return Head == NULL;
	}

	Link *GetHead (void)
	{
		return Head;
	}

	Link *GetTail (void)
	{
		return Tail;
	}

	unsigned long Count (void);
};

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//---------------------------------------------------------------------------
