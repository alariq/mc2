//===========================================================================//
// File:	socket.cc                                                        //
// Contents: Implementation details for Socket and its iterator              //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Socket ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
// Socket
//#############################################################################
//
Socket::Socket(Node *node)
{
	socketsNode = node;
}

//
//#############################################################################
// ~Socket
//#############################################################################
//
Socket::~Socket()
{
	Check_Object(this);
}

//
//#############################################################################
// AddImplementation
//#############################################################################
//
void
   Socket::AddImplementation(Plug*)
{
	Check_Object(this);
	STOP(("Socket::AddImplementation - virtual method with no override"));
}

//
//#############################################################################
// IsEmpty
//#############################################################################
//
bool
   Socket::IsEmpty()
{
	Check_Object(this);
	STOP(("Socket::IsEmpty - virtual method with no override"));
	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SocketIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
// Remove
//#############################################################################
//
void
	SocketIterator::Remove()
{
	Check_Object(this);
	STOP(("SocketIterator::Remove - Should never reach here"));
}

//
//#############################################################################
// DeletePlugs
//#############################################################################
//
void
	SocketIterator::DeletePlugs(bool defeat_release_node)
{
	Check_Object(this);
	Plug *plug;
	Node *save_release_node = NULL;

	if (defeat_release_node)
	{
   	Check_Object(socket);
		save_release_node = socket->GetReleaseNode();
		socket->SetReleaseNode(NULL);
	}

	First();
	while ((plug = ReadAndNextPlug()) != NULL)
	{
		Unregister_Object(plug);
		delete plug;
	}

	if (defeat_release_node)
	{
   	Check_Object(socket);
		socket->SetReleaseNode(save_release_node);
	}
}

//
//#############################################################################
// InsertImplementation
//#############################################################################
//
void
	SocketIterator::InsertImplementation(Plug*)
{
	//
	// Should never reach here
	//
	Check_Object(this);
	STOP(("SocketIterator::InsertImplementation - Should never reach here"));
}
