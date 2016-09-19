//===========================================================================//
// File:	vchain.tst                                                       //
// Contents: test routines for SortedChain class                             //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

#define TEST_COUNT 50

class SortedChainTestPlug:
	public Plug
{
public:
	int value;

	SortedChainTestPlug(int value);
	~SortedChainTestPlug();
};

class SortedChainTestNode:
	public Node
{
public:
	SortedChainOf<SortedChainTestPlug*, int> vchain1;
	SortedChainOf<SortedChainTestPlug*, int> vchain2;

	SortedChainTestNode();
	~SortedChainTestNode();

	bool RunProfile();
	bool RunTest();
};

SortedChainTestPlug::SortedChainTestPlug(int value):
	Plug(DefaultData)
{
	this->value = value;
}

SortedChainTestPlug::~SortedChainTestPlug()
{
}

SortedChainTestNode::SortedChainTestNode():
	Node(DefaultData),
	vchain1(this, true), vchain2(this, true)
{
}

SortedChainTestNode::~SortedChainTestNode()
{
}

//
//###########################################################################
// ProfileClass
//###########################################################################
//

bool
	SortedChain::ProfileClass()
{
	SortedChainTestNode testNode;
	#if defined(_ARMOR)
		Time startTicks = gos_GetHiResTime();
	#endif

	Test_Message("SortedChain::ProfileClass");

	testNode.RunProfile();

	SPEW((
		GROUP_STUFF_TEST,
		"SortedChain::ProfileClass elapsed = %f",
		gos_GetHiResTime() - startTicks
	));
	return true;
}

//
//###########################################################################
// TestClass
//###########################################################################
//

bool
	SortedChain::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting SortedChain test..."));

	SortedChainTestNode	testNode;

	testNode.RunTest();
	return true;
}

bool
	SortedChainTestNode::RunProfile()
{
	SortedChainTestPlug	*testPlug1, *testPlug2;
	int		 		values[TEST_COUNT];
	int				i, j;
	Time 		startTicks;

	/*
	 * Generate unique values, shuffle them
	 */
	for (i = 0; i < TEST_COUNT; i++) {
		values[i] = i;
	}
	for (i = 0; i < TEST_COUNT; i++) {
		int  tmp;
                
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
		testPlug1 = new SortedChainTestPlug(values[i]);
		Register_Object(testPlug1);
		vchain1.AddValue(testPlug1, values[i]);
		vchain2.AddValue(testPlug1, values[i]);
	}
	SPEW((
		GROUP_STUFF_TEST,
		"SortedChainTestNode::RunTest Create = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Iterate over both sockets
	 */
	startTicks = gos_GetHiResTime();
	{
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);

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
		"SortedChainTestNode::RunTest Iterate = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Find 
	 */
	startTicks = gos_GetHiResTime();
	{
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);

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
		"SortedChainTestNode::RunTest Find = %d",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Destroy from vchain1, verify with vchain2
	 */
	startTicks = gos_GetHiResTime();
	{
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		i = 0;
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			Test_Assumption( testPlug1->value == i );
			i++;

			Unregister_Object( testPlug1 );
			delete(testPlug1);
		}
		Test_Assumption( i == TEST_COUNT );

		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );
	}
	SPEW((
		GROUP_STUFF_TEST,
		"SortedChainTestNode::RunTest Destroy = %f",
		gos_GetHiResTime() - startTicks
	));
	return true;
}

bool
	SortedChainTestNode::RunTest()
{
	SortedChainTestPlug	*testPlug1, *testPlug2;
	int	 			values[TEST_COUNT];
	int				i, j;
//	Time 				startTicks;

	/*
	 * Generate unique values, shuffle them
	 */
	for (i = 0; i < TEST_COUNT; i++) {
		values[i] = i;
	}
	for (i = 0; i < TEST_COUNT; i++) {
		int  tmp;
                
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
		testPlug1 = new SortedChainTestPlug(values[i]);
		Register_Object( testPlug1 );
		vchain1.AddValue(testPlug1, values[i]);
		vchain2.AddValue(testPlug1, values[i]);
	}

	/*
	 * Find
	 */
	{
		for (i = 0; i < TEST_COUNT; i++) 
		{
			testPlug1 = vchain1.Find(i);
			testPlug2 = vchain2.Find(i);
			
			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );

			Test_Assumption( testPlug1 == testPlug2 );
		}
	}

	/*
	 * Test_Assumption first and last
	 */
	{
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		iterator1.First();
		iterator2.First();
		
		testPlug1 = iterator1.GetCurrent();
		testPlug2 = iterator2.GetCurrent();

		Test_Assumption( testPlug1 == testPlug2 );
		Test_Assumption( testPlug1 == iterator1.GetNth(0) );
		Test_Assumption( testPlug1 == iterator2.GetNth(0) );
		
		iterator1.Last();
		iterator2.Last();
		
		testPlug1 = iterator1.GetCurrent();
		testPlug2 = iterator2.GetCurrent();
		
		Test_Assumption( testPlug1 == testPlug2 );
		Test_Assumption( testPlug1 == iterator1.GetNth(TEST_COUNT - 1) );
		Test_Assumption( testPlug1 == iterator2.GetNth(TEST_COUNT - 1) );
	}

	/*
	 * Test_Assumption next and prev
	 */
	{
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);
	
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

		iterator1.Last();
		iterator2.Last();

		i = TEST_COUNT-1;
		while ((testPlug1 = iterator1.GetCurrent()) != NULL)
		{
			testPlug2 = iterator2.GetCurrent();

			Test_Assumption( testPlug1 == testPlug2 );

			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );

			iterator1.Previous();
			iterator2.Previous();

			i--;
		}
		Test_Assumption( i == -1 );
	}

	/*
	 * Test_Assumption read next and read prev
	 */
	{
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);
	
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

		iterator1.Last();
		iterator2.Last();

		i = TEST_COUNT-1;
		while ((testPlug1 = iterator1.ReadAndPrevious()) != NULL)
		{
			testPlug2 = iterator2.ReadAndPrevious();

			Test_Assumption( testPlug1 == testPlug2 );

			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );
			
			i--;
		}
		Test_Assumption( i == -1 );
	}
	
	/*
	 * Test_Assumption nth
	 */
	{
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);
	
		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );
		
		for (
			i = 0;
			i < TEST_COUNT;
			i++
		)
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
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);

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

			Unregister_Object( testPlug2 );
			delete(testPlug2);

			i++;
		}
		Test_Assumption( i == TEST_COUNT );
		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );
	}

	/*
	 * Test_Assumption random deletion
	 */
	{
		/*
		 * Add plugs to both sockets
		 */
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);

		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = new SortedChainTestPlug(values[i]);
			Register_Object( testPlug1 );
			vchain1.AddValue(testPlug1, values[i]);
			vchain2.AddValue(testPlug1, values[i]);
		}
	}

	{
		/*
		 * Perform random deletion
		 */
		int size, index;
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator1(&vchain1);
		SortedChainIteratorOf<SortedChainTestPlug*, int> iterator2(&vchain2);

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
		}
		Test_Assumption( i == TEST_COUNT );
		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );
	}
	return true;
}
