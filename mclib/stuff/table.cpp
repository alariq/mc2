//===========================================================================//
// File:	table.cc                                                         //
// Contents: Implementation details of Table class                           //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

#define VERIFY_INDEX(x) 		Verify(0 <= (x) && (x) < numItems)

//
//###########################################################################
// TABLEENTRY
//###########################################################################
//

TableEntry::TableEntry(
	Table *table,
	Plug *plug
):
	Link(table, plug)
{
	Check_Object(this);
}

TableEntry::~TableEntry()
{
	Check_Object(this);
	Table *table = Cast_Object(Table*, socket);
	IteratorPosition index;

	//
	//--------------------------------------
	// Find the link in the table and remove
	//--------------------------------------
	//
	index = table->SearchForTableEntry(this);
	table->RemoveNthTableEntry(index);

	//
	//--------------------------------------------
	// Notify iterators that link is being deleted
	//--------------------------------------------
	//
	table->SendIteratorMemo(PlugRemoved, &index);

	//
	//------------------------------------------
	// Remove this link from any plug references
	//------------------------------------------
	//
	ReleaseFromPlug();

	//
	//-------------------------------------------------------------
	// Tell the node to release this link.  Note that this link
	// is not referenced by the plug or the chain at this point in
	// time.
	//-------------------------------------------------------------
	//
	if (table->GetReleaseNode() != NULL)
	{
		Check_Object(table->GetReleaseNode());
		table->GetReleaseNode()->ReleaseLinkHandler(table, plug);
	}
}

//
//###########################################################################
// TABLE
//###########################################################################
//

//
//###########################################################################
// Table
//###########################################################################
//
Table::Table(
	Node *node,
	bool has_unique_entries
):
	SortedSocket(node, has_unique_entries)
{
	array = NULL;
	maxItems = numItems = 0;
}

//
//###########################################################################
// ~Table
//###########################################################################
//
Table::~Table()
{
	Check_Object(this);
	SetReleaseNode(NULL);
	int i = numItems;
	while (i > 0) 
	{
		--i;
		Unregister_Object(array[i]);
		delete array[i];
	}
	Verify(numItems == 0);
	Verify(array == NULL);
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	Table::TestInstance()
{
	SortedSocket::TestInstance();
	
	if (numItems > 0 || array != NULL) 
	{
		Verify(numItems > 0);
		Verify(numItems <= maxItems);
		Check_Pointer(array);
	}
}

//
//###########################################################################
// AddImplementation
//###########################################################################
//
void
	Table::AddImplementation(Plug *plug)
{
	Check_Object(this);
	AddValueImplementation(plug, NULL);
}

//
//###########################################################################
// AddValueImplementation
//###########################################################################
//
void
	Table::AddValueImplementation(
		Plug *plug,
		const void *value
	)
{
	Check_Object(this);
	TableEntry *link;

	/*
	 * Verify that value has not been added
	 */
	Verify(HasUniqueEntries() ? (SearchForValue(value) == TableNullIndex) : true);

	/*
	 * Make new table entry
	 */
	link = MakeTableEntry(plug, value);
	Register_Object(link);

	/*
	 * Add, sort and send memo to iterators to update references
	 */
	AddTableEntry(link);
	SortTableEntries();
	SendIteratorMemo(PlugAdded, link);
}

//
//###########################################################################
// FindImplementation
//###########################################################################
//
Plug*
	Table::FindImplementation(
		const void *value
	)
{
	Check_Object(this);
	IteratorPosition index;
	
	if ((index = SearchForValue(value)) != TableNullIndex) 
	{
		Check_Pointer(array);
		VERIFY_INDEX(index);
		Check_Object(array[index]);
		
		return array[index]->GetPlug();
	}
	return NULL;
}

#if 0
//
//###########################################################################
// FindCloseImplementation
//###########################################################################
//
Plug*
	Table::FindCloseImplementation(
		const void *value
	)
{
	Check_Object(this);
	IteratorPosition index;
	
	index = SearchForCloseValue(value);
	Verify(index != TableNullIndex);

	Check_Pointer(array);
	VERIFY_INDEX(index);
	Check_Object(array[index]);
		
	return array[index]->GetPlug();
}
#endif

//
//#############################################################################
// IsEmpty
//#############################################################################
//
bool
	Table::IsEmpty()
{
	Check_Object(this);
	return (numItems == (CollectionSize)0);
}

//
//###########################################################################
// MakeTableEntry
//###########################################################################
//
TableEntry*
	Table::MakeTableEntry(
      Plug*,
      const void*
   )
{
	Check_Object(this);
	STOP(("Table::MakeTableEntry - Should never reach here"));
   return NULL;
}

//
//###########################################################################
// CompareTableEntries
//###########################################################################
//
int
	Table::CompareTableEntries(
      TableEntry*,
      TableEntry*
   )
{
	Check_Object(this);
	STOP(("Table::CompareTableEntries - Should never reach here"));
   return 0;
}

//
//###########################################################################
// CompareValueToTableEntry
//###########################################################################
//
int
	Table::CompareValueToTableEntry(
      const void*,
      TableEntry*
   )
{
	Check_Object(this);
	STOP(("Table::CompareValueToTableEntry - Should never reach here"));
   return 0;
}

//
//###########################################################################
// AddTableEntry
//###########################################################################
//
void
	Table::AddTableEntry(
		TableEntry *link
	)
{
	Check_Object(link);

	CollectionSize new_num_items = numItems+1;
	if (array == NULL)
	{
		Verify(new_num_items == 1);
		array = new TableEntry*[new_num_items];
		Register_Pointer(array);
		maxItems = 1;
	}
	else if (new_num_items > maxItems)
	{
		TableEntry **new_array = new TableEntry*[new_num_items];
		Register_Pointer(new_array);

		Mem_Copy(
			new_array,
			array,
			numItems * sizeof(TableEntry*),
			new_num_items * sizeof(TableEntry*)
		);
		Unregister_Pointer(array);
		delete[] array;
		array = new_array;
		maxItems = new_num_items;
	}
	Check_Pointer(array);
	array[numItems] = link;
	numItems = new_num_items;
}

//
//###########################################################################
// SortTableEntries
//###########################################################################
//
void
	Table::SortTableEntries()
{
	Check_Object(this);
	size_t i, j;
	TableEntry *temp;
	
	for (i = 1; i < numItems; i++) 
	{
		Check_Pointer(array);
		Verify(i < numItems);

		temp = array[i];
		j = i;
		
		Verify(j-1 < numItems);

		while (CompareTableEntries(array[j-1], temp) > 0)
		{
			Verify(j < numItems);
			Verify(j-1 < numItems);

			array[j] = array[j-1];
			j--;
			if (j < 1)
				break;
		}
		
		Verify(j < numItems);
		array[j] = temp;
	}
}

//
//###########################################################################
// SearchForValue
//###########################################################################
//
IteratorPosition
	Table::SearchForValue(
		const void *value
	)
{
	Check_Object(this);
	size_t n = numItems;
	size_t i = 0, j;
	IteratorPosition k;
	
	while (i < n) 
	{
		j = (i + n - 1) >> 1;
		
		Check_Pointer(array);
		Verify(j < numItems);
		
		if ((k = CompareValueToTableEntry(value, array[j])) == 0)
			return j;
		
		if (k < 0)
			n = j;
		else
			i = j + 1;
	}
	return TableNullIndex;
}

#if 0
//
//###########################################################################
// SearchForCloseValue
//###########################################################################
//
IteratorPosition
	Table::SearchForCloseValue(
		const void *value
	)
{
	Check_Object(this);
	size_t n = numItems;
	size_t i = 0, j;
	IteratorPosition k;
	
	while (i < n) 
	{
		j = (i + n - 1) >> 1;
		
		Check_Pointer(array);
		Verify(j < numItems);
		
		if ((k = CompareValueToTableEntry(value, array[j])) == 0)
			break;
		
		if (k < 0)
			n = j;
		else
			i = j + 1;
	}
	return j;
}
#endif

//
//###########################################################################
// SearchForTableEntry
//###########################################################################
//
IteratorPosition
	Table::SearchForTableEntry(
		TableEntry *link
	)
{
	Check_Object(this);
	CollectionSize i;
	
	for (i = 0; i < numItems; i++) 
	{
		Check_Pointer(array);
		VERIFY_INDEX(i);
		
		if (array[i] == link)
			return i;
	}
	return TableNullIndex;
}

//
//###########################################################################
// RemoveNthTableEntry
//###########################################################################
//
void
	Table::RemoveNthTableEntry(
		CollectionSize index
	)
{
	Check_Object(this);
	char *itemPtr, *lastItem;
	size_t width;

	Check_Pointer(array);
	VERIFY_INDEX(index);

	/*
	 * Find the location of the item
	 */
	itemPtr = Cast_Pointer(char*, &array[index]);
	
	/*
	 * Remove the item from the array
	 */	
	width = sizeof(void*);
	lastItem = Cast_Pointer(char*, array) + (numItems - 1) * width;

	if (itemPtr < lastItem) 
	{
		memmove(itemPtr, itemPtr + width, (size_t)(lastItem - itemPtr));
	}

	/*
	 * Resize the array
	 */
	CollectionSize new_num_items = numItems-1;
	if (new_num_items == 0)
	{
		Unregister_Pointer(array);
		delete[] array;
		array = NULL;
	}
	numItems = new_num_items;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~ TableIterator inlines ~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if defined(_ARMOR)
	TableEntry*
		TableIterator::NthEntry(
			CollectionSize index
		)
	{
		Check_Object(this);
		Check_Pointer(array);
		Verify(0 <= index && index < numItems);
		Check_Object(array[index]);

		return array[index];
	}
#endif

//
//###########################################################################
// TableIterator
//###########################################################################
//
TableIterator::TableIterator(Table *table):
	SortedIterator(table)
{
	array = table->array;
	numItems = table->numItems;

	if (array != NULL)
		currentPosition = 0;
	else
		currentPosition = TableNullIndex;
}

//
//###########################################################################
//###########################################################################
//
TableIterator::~TableIterator()
{
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	TableIterator::TestInstance()
{
	SortedIterator::TestInstance();
	
	if (array != NULL) 
	{
		Check_Pointer(array);
		Verify(0 < numItems);
		Verify(currentPosition < numItems);
	}
}

//
//###########################################################################
// First
//###########################################################################
//
void
	TableIterator::First()
{
	if (array != NULL)
		currentPosition = 0;
}

//
//###########################################################################
// Last
//###########################################################################
//
void
	TableIterator::Last()
{
	if (array != NULL)
		currentPosition = numItems - 1;
}

//
//###########################################################################
// Next
//###########################################################################
//
void
	TableIterator::Next()
{
	IncrementPosition();
}

//
//###########################################################################
// Previous
//###########################################################################
//
void
	TableIterator::Previous()
{
	DecrementPosition();
}

//
//###########################################################################
// ReadAndNextImplementation
//###########################################################################
//
void
	*TableIterator::ReadAndNextImplementation()
{
	if (currentPosition != TableNullIndex) 
	{
		Plug *plug;
		
		plug = NthEntry(currentPosition)->GetPlug();
		IncrementPosition();
		return plug;
	}
	return NULL;
}

//
//###########################################################################
// ReadAndPreviousImplementation
//###########################################################################
//
void
	*TableIterator::ReadAndPreviousImplementation()
{
	if (currentPosition != TableNullIndex) 
	{
		Plug *plug;

		plug = NthEntry(currentPosition)->GetPlug();
		DecrementPosition();
		return plug;
	}
	return NULL;
}

//
//###########################################################################
// GetCurrentImplementation
//###########################################################################
//
void
	*TableIterator::GetCurrentImplementation()
{
	if (currentPosition != TableNullIndex) 
	{
		return NthEntry(currentPosition)->GetPlug();
	}
	return NULL;
}

//
//###########################################################################
// GetSize
//###########################################################################
//
CollectionSize
	TableIterator::GetSize()
{
	return numItems;
}

//
//###########################################################################
// GetNthImplementation
//###########################################################################
//
void
	*TableIterator::GetNthImplementation(
		CollectionSize index
	)
{
	if (index < numItems) 
	{
		return NthEntry(currentPosition = index)->GetPlug();
	}
	return NULL;
}

//
//###########################################################################
// Remove
//###########################################################################
//
void
	TableIterator::Remove()
{
	if (currentPosition != TableNullIndex)
	{
		Unregister_Object(NthEntry(currentPosition));
		delete NthEntry(currentPosition);
	}
}

//
//###########################################################################
// FindImplementation
//###########################################################################
//
Plug
	*TableIterator::FindImplementation(
		const void *value
	)
{
	IteratorPosition index;
	Table *table = Cast_Object(Table*, socket);
	
	if ((index = table->SearchForValue(value)) != TableNullIndex)
	{
		if (!table->HasUniqueEntries())
		{
			while (
				index-1 >= 0 &&
				table->CompareTableEntries(NthEntry(index-1), NthEntry(index)) == 0
			)
			{
				index--;
			}
		}
	 	return (NthEntry(currentPosition = index)->GetPlug());
	}
	currentPosition = TableNullIndex;
	return NULL;
}

//
//###########################################################################
// ReceiveMemo
//###########################################################################
//
void
	TableIterator::ReceiveMemo(
		IteratorMemo memo,
		void *content
	)
{
	switch (memo)
	{
	case PlugAdded:
      {
			Table *table = Cast_Object(Table*, socket);

         Check_Object(table);
         array = table->array;
         numItems = table->numItems;

      	//
         // If a plug is added before or at the current position then
         // the current position should be incremented one forward,
         // otherwise, no action is necessary
         //
         TableEntry *link;
         IteratorPosition index;

         link = Cast_Object(TableEntry*, content);
         index = table->SearchForTableEntry(link);
         VERIFY_INDEX(index);
         if (index <= currentPosition)
         {
         	currentPosition++;
         }
			Verify(TableNullIndex <= currentPosition && currentPosition < numItems);
      }
      break;

	case PlugRemoved:
		{
			Table *table = Cast_Object(Table*, socket);

         Check_Object(table);
         array = table->array;
         numItems = table->numItems;

         //
         // If a plug is removed before the current position then decrement
         // the current position, else if the current position is at the end
         // of the table then decrement the counter
         //
			IteratorPosition index;

			index = *Cast_Pointer(IteratorPosition*,content);
			if (index < currentPosition)
			{
				currentPosition--;
			}
			else if (numItems <= currentPosition)
			{
				currentPosition--;
			}
			Verify(TableNullIndex <= currentPosition && currentPosition < numItems);
		}
      break;
	}
}
