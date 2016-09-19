//---------------------------------------------------------------------------
//	LList.cpp -- Prototype for linked list class
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files

#ifndef LLIST_H
#include"llist.h"
#endif

//---------------------------------------------------------------------------
// Class LinkedList
void LinkedList::AddToHead (Link *Node)
{
	if (Node == NULL)
	{
		return;
	}

	Node -> Next = Head;
	Head = Node;

	if (Tail == NULL)
	{
		Tail = Head;
	}
}

//---------------------------------------------------------------------------
void LinkedList::AddToTail (Link *Node)
{
	if (Node == NULL)
	{
		return;
	}

	if (Tail != NULL)
	{
		Tail -> Next = Node;
	}
	else
	{
		Head = Node;
	}

	Tail = Node;
}

//---------------------------------------------------------------------------
void LinkedList::Destroy (Link *Node, Link *Last)
{
	Remove (Node, Last);
	delete Node;
}

//---------------------------------------------------------------------------
void LinkedList::InsertAfter (Link* PlaceTo, Link *NodeToPut)
{
	if (PlaceTo == NULL || NodeToPut == NULL)
	{
		return;
	}

	if (PlaceTo != Tail)
	{
		NodeToPut -> Next = PlaceTo -> Next;
		PlaceTo -> Next = NodeToPut;
		return;
	}
	else
	{
		Tail = NodeToPut;
		NodeToPut -> Next = NULL;
		PlaceTo -> Next = NodeToPut;
	}
}

//---------------------------------------------------------------------------
inline void LinkedList::MoveAfter (Link *PlaceTo, Link *NodeToPut, Link *Last)
{
	Remove (NodeToPut, Last);
	InsertAfter (PlaceTo, NodeToPut);
}

//---------------------------------------------------------------------------
void LinkedList::Remove (Link *Node, Link *Last)
{
	Link *Search;

	if (Last == NULL)
	{
		Search = Head;
		while (Search != NULL && Search != Node)
		{
			Last = Search;
			Search = Search -> Next;
		}
		if (Search == NULL)
			return;     // the node wasn't in the list
	}

	if (Last == NULL)
	{
		Head = Head -> Next;
		if (!Head)
			Tail = NULL;
		return;
	}

	if (Tail != Node)
	{
		Last -> Next = Node -> Next;
		if (Head == Node)
		{
			Head = Node -> Next;
		}
	}
	else   // then Tail == Node
	{
		Tail = Last;
		Last -> Next = NULL;
	}
}

//---------------------------------------------------------------------------
void LinkedList::Kill (void)
{
	Link *Temp;

	while (Head != NULL)
	{
		Temp = Head -> Next;
		delete Head;
		Head = Temp;
	}
	Head = Tail = NULL;
}

//---------------------------------------------------------------------------
bool LinkedList::Traverse (Link *&Current)
{
	if (Current == NULL)
	{
		Current = Head;
	}
	else
	{
		Current = Current -> Next;
	}

	return (Current != NULL);
}

//---------------------------------------------------------------------------
unsigned long LinkedList::Count (void)
{
	unsigned long Counter = 0;

	Link *Current = Head;
	while (Current != NULL)
	{
		Counter ++;
		Current = Current -> Next;
	}
	return Counter;
}

//---------------------------------------------------------------------------


