//===========================================================================//
// File:	node.cc                                                          //
// Contents: Implementation details of Node class                            //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

Node::ClassData*
	Node::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Node::InitializeClass()
{
	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			NodeClassID,
			"Stuff::Node",
			Plug::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Node::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Node ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Node::Node(ClassData *class_data):
	Plug(class_data)
{
}

Node::~Node()
{
	Check_Object(this);
}

void
	Node::ReleaseLinkHandler(Socket*, Plug*)
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	Node::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
}
