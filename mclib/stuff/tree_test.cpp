//===========================================================================//
// File:	tree.tst                                                         //
// Contents: test routines for Table class                                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

#define TEST_COUNT 50

class TreeTestPlug:
	public Plug
{
public:
	int value;

	TreeTestPlug(int value);
	~TreeTestPlug();
};

class TreeTestNode:
	public Node
{
public:
	TreeOf<TreeTestPlug*, int> tree1;
	TreeOf<TreeTestPlug*, int> tree2;

	TreeTestNode();
	~TreeTestNode();

   bool TestOrder();
	bool RunProfile();
	bool RunTest();
};

TreeTestPlug::TreeTestPlug(int value):
	Plug(DefaultData)
{
	this->value = value;
}

TreeTestPlug::~TreeTestPlug()
{
	this->value = -1;
}

TreeTestNode::TreeTestNode():
	Node(DefaultData),
	tree1(this, true),
	tree2(this, true)
{
}

TreeTestNode::~TreeTestNode()
{
}

//
//###########################################################################
// ProfileClass
//###########################################################################
//

void
	Tree::ProfileClass()
{
	TreeTestNode	testNode;
	#if defined(_ARMOR)
		Time 		startTicks = gos_GetHiResTime();
	#endif

	Test_Message("Tree::ProfileClass \n");

	testNode.RunProfile();

	SPEW((
		GROUP_STUFF_TEST,
		"Tree::ProfileClass elapsed = %d",
		gos_GetHiResTime() - startTicks
	));
}

//
//###########################################################################
// TestClass
//###########################################################################
//

void
	Tree::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Tree test..."));

	TreeTestNode	testNode;

	testNode.RunTest();
}

//
//###########################################################################
// TestOrder
//###########################################################################
//
bool
	TreeTestNode::TestOrder()
{
	TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
	TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);
	TreeTestPlug *testPlug1, *testPlug2;
	int i, size;

	size = iterator1.GetSize();
	for (i = 1; i < size; i++)
	{
		testPlug1 = iterator1.GetNth(i-1);
		testPlug2 = iterator2.GetNth(i);

		Test_Assumption(testPlug1->value < testPlug2->value);
	}
	return true;
}

//
//###########################################################################
// RunProfile
//###########################################################################
//
bool
	TreeTestNode::RunProfile()
{
	TreeTestPlug	*testPlug1, *testPlug2;
	int			 	values[TEST_COUNT];
	int				i, j;
	Time 				startTicks;

	/*
	 * Generate unique values, shuffle them
	 */
	for (i = 0; i < TEST_COUNT; i++) {
		values[i] = i;
	}
	for (i = 0; i < TEST_COUNT; i++) {
		int   tmp;
                
		j = i + Random::GetLessThan(TEST_COUNT - i);
		tmp = values[j];
		values[j] = values[i];
		values[i] = tmp;
	}

	//
	//--------------------------------------------------------------------
	// Run timing tests
	//--------------------------------------------------------------------
	//

	/*
	 * Create plugs and add to both sockets
	 */
	startTicks = gos_GetHiResTime();
	for (i = 0; i < TEST_COUNT; i++) 
	{
		testPlug1 = new TreeTestPlug(values[i]);
		Register_Object( testPlug1 );
		tree1.AddValue(testPlug1, values[i]);
		tree2.AddValue(testPlug1, values[i]);
	}
	SPEW((
		GROUP_STUFF_TEST,
		"TreeTestNode::RunTest Create = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Iterate over both sockets
	 */
	startTicks = gos_GetHiResTime();
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );
		
		i = 0;
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			Test_Assumption( testPlug1->value == i );
			i++;
		}
		Test_Assumption( i == TEST_COUNT );
		
		i = 0;
		while ((testPlug1 = iterator2.ReadAndNext()) != NULL)
		{
			Test_Assumption( testPlug1->value == i );
			i++;
		}
		Test_Assumption( i == TEST_COUNT );
	}
	SPEW((
		GROUP_STUFF_TEST,
		"TreeTestNode::RunTest Iterate = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Find 
	 */
	startTicks = gos_GetHiResTime();
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		for (i = 0; i < TEST_COUNT; i++) 
		{
			testPlug1 = iterator1.Find(i);
			testPlug2 = iterator2.Find(i);

			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );

			Test_Assumption( testPlug1 == testPlug2 );
		}
	}
	SPEW((
		GROUP_STUFF_TEST,
		"TreeTestNode::RunTest Find = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Destroy from tree1, verify with tree2
	 */
	startTicks = gos_GetHiResTime();
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		i = 0;
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			Test_Assumption( testPlug1->value == i );
			i++;
			
			Unregister_Object(testPlug1);
			delete(testPlug1);
		}
		Test_Assumption( i == TEST_COUNT );
		
		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );
	}
	SPEW((
		GROUP_STUFF_TEST,
		"TreeTestNode::RunTest Destroy = %f",
		gos_GetHiResTime() - startTicks
	));
	return true;
}

//
//###########################################################################
// RunTest
//###########################################################################
//
bool
	TreeTestNode::RunTest()
{
	TreeTestPlug	*testPlug1, *testPlug2;
	int		 		values[TEST_COUNT];
	int				i, j;
//	Time 				startTicks;

	/*
	 * Generate unique values, shuffle them
	 */
	for (i = 0; i < TEST_COUNT; i++) {
		values[i] = i;
	}
	for (i = 0; i < TEST_COUNT; i++) {
		int   tmp;
                
		j = i + Random::GetLessThan(TEST_COUNT - i);
		tmp = values[j];
		values[j] = values[i];
		values[i] = tmp;
	}

	//
	//--------------------------------------------------------------------
	// Stress tests
	//--------------------------------------------------------------------
	//

	/*
	 * Create plugs and add to both sockets
	 */
	for (i = 0; i < TEST_COUNT; i++)
	{
		testPlug1 = new TreeTestPlug(values[i]);
		Register_Object(testPlug1);
		tree1.AddValue(testPlug1, values[i]);
		tree2.AddValue(testPlug1, values[i]);
	}
	TestOrder();

	/*
	 * Find
	 */
	{
		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = tree1.Find(i);
			testPlug2 = tree2.Find(i);

			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );

			Test_Assumption( testPlug1 == testPlug2 );
		}
	}

	/*
	 * Test_Assumption first and last
	 */
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		iterator1.First();
		iterator2.First();

		testPlug1 = iterator1.GetCurrent();
		testPlug2 = iterator2.GetCurrent();

		Test_Assumption( testPlug1 == testPlug2 );
		Test_Assumption( testPlug1 == iterator1.GetNth(0) );
		Test_Assumption( testPlug1 == iterator2.GetNth(0) );
	}

	/*
	 * Test_Assumption next and prev
	 */
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		i = 0;
		while ((testPlug1 = iterator1.GetCurrent()) != NULL)
		{
			testPlug2 = iterator2.GetCurrent();

			Test_Assumption( testPlug1 == testPlug2 );

			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );

			iterator1.Next();
			iterator2.Next();

			i++;
		}
		Test_Assumption( i == TEST_COUNT );
	}

	/*
	 * Test_Assumption read next and read prev
	 */
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		i = 0;
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			testPlug2 = iterator2.ReadAndNext();

			Test_Assumption( testPlug1 == testPlug2 );

			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );

			i++;
		}
		Test_Assumption( i == TEST_COUNT );
	}

	/*
	 * Test_Assumption nth
	 */
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = iterator1.GetNth(i);
			testPlug2 = iterator2.GetNth(i);

			Test_Assumption( testPlug1 == testPlug2 );

			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );
		}
	}

	/*
	 * Test_Assumption Remove
	 */
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		i = 0;
		while ((testPlug1 = iterator1.GetCurrent()) != NULL)
		{
			Test_Assumption( testPlug1->value == i );

			iterator1.Remove();

			testPlug2 = iterator2.GetNth(0);

			Test_Assumption( testPlug2->value == i );
			Test_Assumption( testPlug1 == testPlug2 );

			Unregister_Object(testPlug2);
			delete testPlug2;

			i++;
			TestOrder();
		}
		Test_Assumption( i == TEST_COUNT );
		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );
	}

	/*
	 * Test_Assumption random deletion
	 */
	/*
	 * Add plugs to both sockets
	 */
	{
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = new TreeTestPlug(values[i]);
			Register_Object( testPlug1 );
			tree1.AddValue(testPlug1, values[i]);
			tree2.AddValue(testPlug1, values[i]);

			TestOrder();
		}
		TestOrder();
	}

	/*
	 * Perform random deletion
	 */
	{
		int size, index;
		TreeIteratorOf<TreeTestPlug*, int> iterator1(&tree1);
		TreeIteratorOf<TreeTestPlug*, int> iterator2(&tree2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		i = 0;
		while((size = iterator1.GetSize()) != 0)
		{
			index = Random::GetLessThan(size);
			testPlug1 = iterator1.GetNth(index);
			iterator1.Remove();

			testPlug2 = iterator2.GetNth(index);
			Test_Assumption( testPlug1 == testPlug2 );

			Unregister_Object( testPlug2 );
			delete(testPlug2);

			i++;
			TestOrder();
		}
		Test_Assumption( i == TEST_COUNT );
		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );
	}
	return true;
}


