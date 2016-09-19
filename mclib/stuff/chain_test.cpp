//===========================================================================//
// File:	chain.tst                                                        //
// Contents: Test_Assumption function for chain class                        //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

#define TEST_COUNT 50

class ChainTestPlug:
	public Plug
{
public:
	long value;

	ChainTestPlug(long value);
	~ChainTestPlug();
};

class ChainTestNode:
	public Node
{
public:
	int receivedCommand;

	ChainOf<ChainTestPlug*> chain1;
	ChainOf<ChainTestPlug*> chain2;

	ChainTestNode();
	~ChainTestNode();

	bool RunProfile();
	bool RunTest();
};

ChainTestPlug::ChainTestPlug(long value):
	Plug(DefaultData)
{
	this->value = value;
}

ChainTestPlug::~ChainTestPlug()
{
}

ChainTestNode::ChainTestNode():
	Node(DefaultData),
	chain1(NULL),
	chain2(this)
{
	receivedCommand = 0;
}

ChainTestNode::~ChainTestNode()
{
}

//
//###########################################################################
// ProfileClass
//###########################################################################
//
bool
	Chain::ProfileClass()
{
	ChainTestNode testNode;
	#if defined(_ARMOR)
		Time startTicks = gos_GetHiResTime();
	#endif

	Test_Message("Chain::ProfileClass\n");

	testNode.RunProfile();

	SPEW((
		GROUP_STUFF_TEST,
		"Chain::ProfileClass elapsed = %f",
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
	Chain::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Chain test..."));

	ChainTestNode	testNode;

	testNode.RunTest();

	return true;
}

//
//###########################################################################
// RunProfile
//###########################################################################
//
bool
	ChainTestNode::RunProfile()
{
	ChainTestPlug		*testPlug1;
	CollectionSize 	i;
	Time 			startTicks;

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
		testPlug1 = new ChainTestPlug(i);
		Register_Object( testPlug1 );
		chain1.Add(testPlug1);
		chain2.Add(testPlug1);
	}
	SPEW((
		GROUP_STUFF_TEST,
		"ChainTestNode::RunTest Create = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Iterate over both sockets
	 */
	startTicks = gos_GetHiResTime();
	{
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

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
		"ChainTestNode::RunTest Iterate = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Destroy from chain1, verify with chain2
	 */
	startTicks = gos_GetHiResTime();
	{
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		#if defined(_ARMOR)
			i = 0;
		#endif
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			Test_Assumption( testPlug1->value == i++ );

			Unregister_Object( testPlug1 );
			delete(testPlug1);
		}
		Test_Assumption( i == TEST_COUNT );
		
		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );
	}
	SPEW((
		GROUP_STUFF_TEST,
		"ChainTestNode::RunTest Destroy = %f",
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
	ChainTestNode::RunTest()
{
	ChainTestPlug		*testPlug1, *testPlug2;
	CollectionSize 	i;
//	Time 					startTicks;

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
		testPlug1 = new ChainTestPlug(i);
		Register_Object( testPlug1 );
		chain1.Add(testPlug1);
		chain2.Add(testPlug1);
	}

	/*
	 * Test_Assumption first and last
	 */
	{
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

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
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);
	
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
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

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
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

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
	 * Test_Assumption plug iterator
	 */
	{
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainTestNode *testNode;
			
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			PlugIteratorOf<ChainTestNode*> iterator2(testPlug1);

			while((testNode = iterator2.ReadAndNext()) != NULL)
			{
				Test_Assumption( testNode == this );
			}
		}
	}
	{
		ChainTestNode testNode1;
		ChainTestNode testNode2;
		ChainOf<ChainTestPlug*> chain1(&testNode1);
		ChainOf<ChainTestPlug*> chain2(&testNode2);
		ChainOf<ChainTestPlug*> chain3(NULL);
		ChainTestPlug testPlug(0);

		chain1.Add(&testPlug);
		chain2.Add(&testPlug);
		chain3.Add(&testPlug);

		PlugIteratorOf<ChainTestNode*> iterator(&testPlug);

		Test_Assumption(iterator.ReadAndNext() == &testNode2);
		Test_Assumption(iterator.ReadAndNext() == &testNode1);
	}

	/*
	 * Test_Assumption Remove
	 */
	{
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );
		Test_Assumption( iterator2.GetSize() == TEST_COUNT );

		i = 0;
		while ((testPlug1 = iterator1.GetCurrent()) != NULL)
		{
			Test_Assumption( testPlug1->value == i );

         if (i % 2 == 0)
         {
				iterator1.Remove();
         }
         else
         {
            iterator1.Next();

				chain1.Remove(testPlug1);
         }

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
	 * Test_Assumption insert
	 */
	{
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);

		Test_Assumption( iterator1.GetSize() == 0 );

		for (i = 0; i < TEST_COUNT; i += 2)
		{
			testPlug1 = new ChainTestPlug(i);
			Register_Object(testPlug1);
			chain1.Add(testPlug1);
		}

		for (i = 1; i < TEST_COUNT; i += 2)
		{
			testPlug1 = new ChainTestPlug(i);
			Register_Object(testPlug1);

			iterator1.First();
			while ((testPlug2 = iterator1.GetCurrent()) != NULL)
			{
				if (i < testPlug2->value)
				{
					iterator1.Insert(testPlug1);
					break;
				}
         	iterator1.Next();
			}
			if (testPlug2 == NULL)
			{
				chain1.Add(testPlug1);
			}
		}

		Test_Assumption( iterator1.GetSize() == TEST_COUNT );

		for (i = 1; i < TEST_COUNT; i++)
		{
			testPlug1 = iterator1.GetNth(i);
			testPlug2 = iterator1.GetNth(i-1);

			Test_Assumption(testPlug2->value < testPlug1->value);
		}
	}
	{
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);

		while ((testPlug1 = iterator1.GetCurrent()) != NULL)
		{
			Unregister_Object(testPlug1);
			delete(testPlug1);
			iterator1.First();
		}
	}

	/*
	 * Test_Assumption random deletion
	 */
	{
		/*
		 * Add plugs to both sockets
		 */
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

		Test_Assumption( iterator1.GetSize() == 0 );
		Test_Assumption( iterator2.GetSize() == 0 );

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = new ChainTestPlug(i);
			Register_Object( testPlug1 );
			chain1.Add(testPlug1);
			chain2.Add(testPlug1);
		}
	}
	{
		int size, index;
		ChainIteratorOf<ChainTestPlug*> iterator1(&chain1);
		ChainIteratorOf<ChainTestPlug*> iterator2(&chain2);

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

