//=======================================================================
// File: SCHNTST.CPP
//         
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

#define TEST_COUNT 50

class SafeChainTestPlug:
	public Plug
{
public:
	long value;
		
	SafeChainTestPlug(long value);
	~SafeChainTestPlug();
};

class SafeChainTestNode:
	public Node
{
public:
	SafeChainOf<SafeChainTestPlug*>
		chain1,
		chain2;

	SafeChainTestNode();
	~SafeChainTestNode();

	bool RunProfile();
	bool RunTest();
};

SafeChainTestPlug::SafeChainTestPlug(long value):
	Plug(DefaultData)
{
	this->value = value;
}

SafeChainTestPlug::~SafeChainTestPlug()
{
}

SafeChainTestNode::SafeChainTestNode():
	Node(DefaultData), chain1(this), chain2(this)
{
}

SafeChainTestNode::~SafeChainTestNode()
{
}

//
//###########################################################################
// ProfileClass
//###########################################################################
//

void
	SafeChain::ProfileClass()
{
	SafeChainTestNode testNode;
	#if defined(_ARMOR)
		Time startTicks = gos_GetHiResTime();
	#endif

	Test_Message("SafeChain::ProfileClass");

	testNode.RunProfile();

	SPEW((
		GROUP_STUFF_TEST,
		"SafeChain::ProfileClass elapsed = %f",
		gos_GetHiResTime() - startTicks
	));
}

//
//###########################################################################
// TestClass
//###########################################################################
//

void
	SafeChain::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting SafeChain test..."));

	SafeChainTestNode testNode;

	testNode.RunTest();
}

bool
	SafeChainTestNode::RunProfile()
{
	SafeChainTestPlug	*testPlug1;
	int 				i;
	Time 		startTicks;

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
		testPlug1 = new SafeChainTestPlug(i);
		Register_Object( testPlug1 );
		chain1.Add(testPlug1);
		chain2.Add(testPlug1);
	}
	SPEW((
		GROUP_STUFF_TEST,
		"SafeChainTestNode::RunTest Create = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Iterate over both sockets
	 */
	startTicks = gos_GetHiResTime();
	{
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);
	
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
		"SafeChainTestNode::RunTest Iterate = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Destroy from chain1, verify with chain2
	 */
	startTicks = gos_GetHiResTime();
	{
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);

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
		"SafeChainTestNode::RunTest Destroy = %f",
		gos_GetHiResTime() - startTicks
	));
	return true;
}

bool
	SafeChainTestNode::RunTest()
{
	SafeChainTestPlug	*testPlug1, *testPlug2;
	int 			i, j;
//	Time 		startTicks;

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
		testPlug1 = new SafeChainTestPlug(i);
		Register_Object( testPlug1 );
		chain1.Add(testPlug1);
		chain2.Add(testPlug1);
	}

	/*
	 * Test_Assumption first and last
	 */
	{
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);

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
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);
	
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

		i = TEST_COUNT - 1;
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
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);
	
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

		i = TEST_COUNT - 1;
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
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);
	
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
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);
	
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
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);

		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = new SafeChainTestPlug(i);
			Register_Object( testPlug1 );
			chain1.Add(testPlug1);
			chain2.Add(testPlug1);
		}
	}

	{
		/*
		 * Perform random deletion
		 */
		int size, index;
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);
		SafeChainIteratorOf<SafeChainTestPlug*> iterator2(&chain2);

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

	/*
	 * Test_Assumption insertion
	 */
	{
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);

		Test_Assumption(iterator1.GetSize() == 0);

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = new SafeChainTestPlug(i);
			Register_Object(testPlug1);

			if (i == 0)
			{
				chain1.Add(testPlug1);
			}
			else
			{
				iterator1.First();
				iterator1.Insert(testPlug1);
			}
		}

		for (i = 0, j = TEST_COUNT-1; i < TEST_COUNT; i++, j--)
		{
			testPlug1 = iterator1.GetNth(i);
			Test_Assumption(testPlug1->value == j);
		}

      iterator1.DeletePlugs();
	}
	{
		SafeChainIteratorOf<SafeChainTestPlug*> iterator1(&chain1);

		Test_Assumption(iterator1.GetSize() == 0);

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = new SafeChainTestPlug(i);
			Register_Object(testPlug1);

			if (i == 0)
			{
				chain1.Add(testPlug1);
			}
			else
			{
				iterator1.Last();
				iterator1.Insert(testPlug1);
			}
		}

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = iterator1.GetNth(i);
			if (i == TEST_COUNT-1)
			{
				Test_Assumption(testPlug1->value == 0);
			}
			else
			{
				Test_Assumption(testPlug1->value == i+1);
			}
		}

      iterator1.DeletePlugs();
	}
	return true;
}

