//===========================================================================//
// File:	socket.hh                                                        //
// Contents: Interface specifications for base socket and its iterator       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"plug.hpp"

namespace Stuff {

	class Node;

	//##########################################################################
	//###########################    Socket    #################################
	//##########################################################################

	//
	//--------------------------------------------------------------------------
	// NOTE: All unsafe, untyped public methods are named XXXPlug or
	// XXXIterator.  The safe, typed public methods are named XXX and are
	// declared in the template sub-classes.
	//--------------------------------------------------------------------------
	//

	class Socket
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Destructor, testing
	//
	public:
		virtual
			~Socket();
		void
			TestInstance()
				{}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Socket methods
	//
	public:
		//
		//-----------------------------------------------------------------------
		// AddPlug - Add a plug to this socket, untyped access.
		//-----------------------------------------------------------------------
		//
		void
			AddPlug(Plug *plug)
				{AddImplementation(plug);}

		//
		//-----------------------------------------------------------------------
		// RemovePlug - Remove a plug from this socket, untyped access.
		//-----------------------------------------------------------------------
		//
		virtual void
			RemovePlug(Plug *plug);

		//
		//-----------------------------------------------------------------------
		// IsPlugMember - Determine if the plug is a member of this socket.
		//-----------------------------------------------------------------------
		//
		virtual bool
			IsPlugMember(Plug *plug);

		//
		//-----------------------------------------------------------------------
		// IsEmpty - Returns true if the socket contains no plugs.
		//-----------------------------------------------------------------------
		//
		virtual bool
			IsEmpty();

 		//
		//-----------------------------------------------------------------------
		// Accessors for the release node, this is provided for socket 
		//		utilities and are not normally used by a client.
		//-----------------------------------------------------------------------
		//
		Node*
			GetReleaseNode()
				{return socketsNode;}
		void
			SetReleaseNode(Node *release_node)
				{socketsNode = release_node;}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Protected methods
	//
	protected:
		explicit Socket(Node *node);

		virtual void
			AddImplementation(Plug *plug);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Protected data
	//
	protected:
		Node
      	*socketsNode;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Socket inlines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	inline void
		Socket::RemovePlug(Plug *plug)
	{
		Check_Object(plug);
		plug->RemoveSocket(this);
	}
	
	inline bool
		Socket::IsPlugMember(Plug *plug)
	{
		Check_Object(plug);
		return plug->IsSocketMember(this);
	}	

	//##########################################################################
	//########################    SocketIterator    ############################
	//##########################################################################

	//
	//--------------------------------------------------------------------------
	// NOTE: All unsafe, untyped public methods are named XXXPlug or
	// XXXIterator.  The safe, typed public methods are named XXX and are
	// declared in the template sub-classes.
	//--------------------------------------------------------------------------
	//

	class SocketIterator:
		public Iterator
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Destructor and testing
	//
	public:
		~SocketIterator()
			{}
		void
			TestInstance() const
				{}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Iterator methods (see Iterator for full listing)
	//
	public:
		Plug*
			ReadAndNextPlug()
				{return static_cast<Plug*>(ReadAndNextImplementation());}
		Plug*
			ReadAndPreviousPlug()
				{return static_cast<Plug*>(ReadAndPreviousImplementation());}
		Plug*
			GetCurrentPlug()
				{return static_cast<Plug*>(GetCurrentImplementation());}
		Plug*
			GetNthPlug(CollectionSize index)
				{return static_cast<Plug*>(GetNthImplementation(index));}

		//
		//-----------------------------------------------------------------------
		// InsertPlug - Inserts plug at current location, untyped access.
		//-----------------------------------------------------------------------
		//
		void
			InsertPlug(Plug *plug)
				{InsertImplementation(plug);}

		//
		//-----------------------------------------------------------------------
		// Remove - Removes the link at the current location, does not remove 
		//		the plug.
		//-----------------------------------------------------------------------
		//
		virtual void
			Remove();

		//
		//-----------------------------------------------------------------------
		// DeletePlugs	- For each plug in the socket, the routine unregisters it 
		//		and then deletes it.
		//-----------------------------------------------------------------------
		//
		void
			DeletePlugs(bool defeat_release_node = true);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Protected interface
	//
	protected:
		explicit SocketIterator(Socket *socket)
			{this->socket = socket;}

		virtual void
			InsertImplementation(Plug*);

		Socket
	      	*socket;
	};

}
