//===========================================================================//
// File:	node.hh                                                          //
// Contents: Interface specification for base node class                     //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"socket.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Node ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	typedef Plug__ClassData Node__ClassData;

	class Node:
		public Plug
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		void
			TestInstance();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor/Destructor
	//
	public:
		~Node();

	protected:
		explicit Node(ClassData *class_data);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
	public:
		typedef Node__ClassData ClassData;

		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// plug detachment
	//
	public:
		virtual void
			ReleaseLinkHandler(Socket*, Plug*);
	};

}
