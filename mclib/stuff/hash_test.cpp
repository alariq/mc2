//===========================================================================//
// File:	hash.tst                                                         //
// Contents: 										                         //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

#define TEST_COUNT 50

class HashTestPlug:
	public Plug
{
public:
	int value;

	HashTestPlug(int value);
	~HashTestPlug();
};

class HashTestNode:
	public Node
{
public:
	HashOf<HashTestPlug*, int> hash1;
	HashOf<HashTestPlug*, int> hash2;

	HashTestNode();
	~HashTestNode();

	bool RunProfile();
	bool RunTest();
};

HashTestPlug::HashTestPlug(int value):
	Plug(DefaultData)
{
	this->value = value;
}

HashTestPlug::~HashTestPlug()
{
}

HashTestNode::HashTestNode():
	Node(DefaultData),
	hash1(TEST_COUNT/2, this, true),
	hash2(TEST_COUNT/2, this, true)
{
}

HashTestNode::~HashTestNode()
{
}

//
//###########################################################################
// ProfileClass
//###########################################################################
//

bool
	Hash::ProfileClass()
{
	HashTestNode testNode;
	#if defined(_ARMOR)
		Time startTicks = gos_GetHiResTime();
	#endif

	Test_Message("Hash::ProfileClass \n");

	testNode.RunProfile();

	SPEW((
		GROUP_STUFF_TEST,
		"Hash::ProfileClass elapsed = %f",
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
	Hash::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting Hash test..."));

	HashTestNode testNode;

	testNode.RunTest();
   return true;
}

bool
	HashTestNode::RunProfile()
{
	HashTestPlug	*testPlug1, *testPlug2;
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
		testPlug1 = new HashTestPlug(values[i]);
		Register_Object( testPlug1 );
		hash1.AddValue(testPlug1, values[i]);
		hash2.AddValue(testPlug1, values[i]);
	}
	SPEW((
		GROUP_STUFF_TEST,
		"HashTestNode::RunTest Create = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Iterate over both sockets
	 */
	startTicks = gos_GetHiResTime();
	{
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);

		i = 0;
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			i++;
		}
		Test_Assumption( i == TEST_COUNT );

		i = 0;
		while ((testPlug1 = iterator2.ReadAndNext()) != NULL)
		{
			i++;
		}
		Test_Assumption( i == TEST_COUNT );
	}
	SPEW((
		GROUP_STUFF_TEST,
		"HashTestNode::RunTest Iterate = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Find
	 */
	startTicks = gos_GetHiResTime();
	{
		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = hash1.Find(i);
			testPlug2 = hash2.Find(i);

			Test_Assumption( testPlug1 == testPlug2 );
		}
	}
	SPEW((
		GROUP_STUFF_TEST,
		"HashTestNode::RunTest Find = %f",
		gos_GetHiResTime() - startTicks
	));

	/*
	 * Destroy from hash1, verify with hash2
	 */
	startTicks = gos_GetHiResTime();
	{
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);

		i = 0;
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			i++;
			
			Unregister_Object(testPlug1);
			delete(testPlug1);
		}
		Test_Assumption( i == TEST_COUNT );
      Test_Assumption( iterator2.GetCurrent() == NULL );
	}
	SPEW((
		GROUP_STUFF_TEST,
		"HashTestNode::RunTest Destroy = %f",
		gos_GetHiResTime() - startTicks
	));
	return true;
}

bool
	HashTestNode::RunTest()
{
	HashTestPlug	*testPlug1, *testPlug2;
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
		testPlug1 = new HashTestPlug(values[i]);
		Register_Object( testPlug1 );
		hash1.AddValue(testPlug1, values[i]);
		hash2.AddValue(testPlug1, values[i]);
	}

	/*
	 * Find
	 */
	{
		for (i = 0; i < TEST_COUNT; i++) 
		{
			testPlug1 = hash1.Find(i);
			testPlug2 = hash2.Find(i);
			
			Test_Assumption( testPlug1->value == i );
			Test_Assumption( testPlug2->value == i );

			Test_Assumption( testPlug1 == testPlug2 );
		}
	}

	/*
	 * Test_Assumption first and last
	 */
	{
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);

		iterator1.First();
		iterator2.First();
		
		testPlug1 = iterator1.GetCurrent();
		testPlug2 = iterator2.GetCurrent();

		Test_Assumption( testPlug1 == testPlug2 );
	}

	/*
	 * Test_Assumption next and prev
	 */
	{
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);
	
		i = 0;
		while ((testPlug1 = iterator1.GetCurrent()) != NULL)
		{
			testPlug2 = iterator2.GetCurrent();
			
			Test_Assumption( testPlug1 == testPlug2 );

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
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);

		i = 0;
		while ((testPlug1 = iterator1.ReadAndNext()) != NULL)
		{
			testPlug2 = iterator2.ReadAndNext();

			Test_Assumption( testPlug1 == testPlug2 );

			i++;
		}
		Test_Assumption( i == TEST_COUNT );
	}

	/*
	 * Test_Assumption Remove
	 */
	{
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);

		i = 0;
		while ((testPlug1 = iterator1.GetCurrent()) != NULL)
		{
			testPlug2 = iterator2.GetCurrent();

			Test_Assumption( testPlug1 == testPlug2 );

			iterator1.Remove();

			Unregister_Object(testPlug1);
			delete(testPlug1);

			i++;
		}
		Test_Assumption( iterator2.GetCurrent() == NULL );
		Test_Assumption( i == TEST_COUNT );
	}

	/*
	 * Test_Assumption random deletion
	 */
	/*
	 * Add plugs to both sockets
	 */
	{
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);

		Test_Assumption( iterator1.GetCurrent() == NULL );
		Test_Assumption( iterator2.GetCurrent() == NULL );

		for (i = 0; i < TEST_COUNT; i++)
		{
			testPlug1 = new HashTestPlug(values[i]);
			Register_Object( testPlug1 );
			hash1.AddValue(testPlug1, values[i]);
			hash2.AddValue(testPlug1, values[i]);
		}
	}

	/*
	 * Perform random deletion
	 */
	{
		int size, index;
		HashIteratorOf<HashTestPlug*, int> iterator1(&hash1);
		HashIteratorOf<HashTestPlug*, int> iterator2(&hash2);

		i = 0;
		iterator1.First();
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
		Test_Assumption( iterator1.GetCurrent() == NULL );
		Test_Assumption( iterator2.GetCurrent() == NULL );
	}
	return true;
}

