#ifndef ELIST_H
#define ELIST_H

#pragma warning( disable : 4211 )

#include<memory.h>
#include"heap.h"
//--------------------------------------------------------------------------------------
//
// Mech Commander 2
//
// standard linked list
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//*************************************************************************************************

#define ELIST_TPL_DEF	template<class T, class T_ARG>
#define ELIST_TPL_ARG	T, T_ARG

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
EList:
This template class is a doubly linked list. The list is implemented using a fixed memory pool,
which ensures proximity of all the elements, in order to increase the cacheability of the list.

Two template parameters are required to declare a list. The first is the type of the object,
that is intended to be stored in the list. The second is the type by which we intend to pass the
object. 

The user needs to pass the initial element count and the amount of elements to grow the
memory pool by, if required to accomodate all elements, to the constructor.
Optionally a name can be supplied, however this will only be used during debug builds.

Example:
	EList<KSomeClass, const KSomeClass&>	m_List(10, 5, "SomeList");
	EList<int, const int>					m_List(100, 10);


Note:
	The second template argument is const to provide for const safety.
	It is syntacticly legal to use a non-const version, however, it will not provide
	const safty. Unfortunately there is no way to prevent this.

Classes used with the list need to define a copy constructor, an assignment operator, and 
an equality operator in order to work properly (of course only if the default versions are
not sufficient).

**************************************************************************************************/
// sebi: one cannot simply redefine placement new ( c++ standard)
//#ifndef __PLACEMENT_NEW_INLINE
//#define __PLACEMENT_NEW_INLINE
//	inline void *__cdecl operator new(size_t, void *_P)
//	{return (_P); } // placement new
//#endif


ELIST_TPL_DEF class EList
{
protected:

	//
	//	The node is used to chain the elements of the list together, it contains pointers
	//	to the elements preceeding or following it, in addition to the element itself.
	//	If no preceeding or following element are present the value is NULL
	//
	struct ENode
	{
		ENode(T_ARG New_Element) : m_Data(New_Element) {};
		ENode*	m_pNext;
		ENode*	m_pPrev;
		T		m_Data;

	private:
		ENode& operator=(const ENode& rNode);
	};

	enum
	{
		MAX_NAME_LENGTH = 64
	};


public:

	/**************************************************************************************************
	CLASS DESCRIPTION
	EList::EConstIterator
	This class is used to iterate through the List.  You can read objects with this iterator but you
	can't change them.  Use class !!(EIterator) for changing objects. 
	**************************************************************************************************/
	class EConstIterator
	{
	public:
		// Note: Code needs to be implemented in class decleration, due
		// to MSVC bug with nested template classes
		
		inline EConstIterator()							{ m_pCur_Node = NULL; }
		inline EConstIterator(const EConstIterator& rIter)	{ m_pCur_Node = rIter.m_pCur_Node; }
		inline EConstIterator& operator=(const EConstIterator& rIter)	{ m_pCur_Node = rIter.m_pCur_Node; return(*this); }

		//
		//	NOTE:	Postfix operators require "int" to be passed as parameter,
		//			else compiler error C2807 will occur when used
		//
		inline EConstIterator& operator++(int)	// postfix increment
		{
			#ifdef	DEBUG
			if(!(IsValid()))
			{
				gosASSERT("EList::EIterator: Attempt to increment invalid iterator\n");
			}
			#endif

			m_pCur_Node = m_pCur_Node->m_pNext;
			return(*this);
		}

		inline EConstIterator& operator--(int)	// postfix decrement
		{
			#ifdef	KTL_DEBUG
			if(!(IsValid()))
			{
				Assert("EList::EIterator: Attempt to decrement invalid iterator\n");
			}
			#endif

			m_pCur_Node = m_pCur_Node->m_pPrev;
			return(*this);
		}

		inline EConstIterator& operator+=(unsigned long Increment)
		{
			while(Increment)
			{
				#ifdef	KTL_DEBUG
				if(!(IsValid()))
				{
					Assert("EList::EIterator: Attempt to increment invalid iterator\n");
				}
				#endif

				m_pCur_Node = m_pCur_Node->m_pNext;
				Increment--;
			}
			return(*this);
		}

		inline EConstIterator& operator-=(unsigned long Decrement)
		{
			while(Decrement)
			{
				#ifdef	KTL_DEBUG
				if(!(IsValid()))
				{
					Assert("EList::EIterator: Attempt to increment invalid iterator\n");
				}
				#endif

				m_pCur_Node = m_pCur_Node->m_pPrev;
				Decrement--;
			}
			return(*this);
		}

		inline bool operator==(const EConstIterator& rIter) const
		{
			return(m_pCur_Node == rIter.m_pCur_Node);
		}

		inline bool operator!=(const EConstIterator& rIter) const
		{
			return(m_pCur_Node != rIter.m_pCur_Node);
		}

		inline T_ARG operator*() const
		{
			return Item();
		}
		
		inline T_ARG Item() const		{ return(m_pCur_Node->m_Data); }
		inline bool IsValid() const	{ return(m_pCur_Node ? true : false); }
		inline bool IsDone() const		{ return(m_pCur_Node ? false : true); }

		friend class EList<ELIST_TPL_ARG>;
	protected:

		ENode*	m_pCur_Node;
#ifdef KTL_DEBUG
		char	m_Name[64];
#endif
	};	// END CLASS EConstIterator
	

	/**************************************************************************************************
	CLASS DESCRIPTION
	EList::EIterator
	This class is used for non constant iteration through a list.
	**************************************************************************************************/
	class EIterator : public EConstIterator
	{
	public:
		inline EIterator() : EConstIterator() {}
		inline EIterator(const EIterator& rIter) : EConstIterator(rIter) {}

		inline T& Item()				{ return(EConstIterator::m_pCur_Node->m_Data); }

		inline T& operator*()
		{
			return Item();
		}

	};	// END CLASS EIterator


	//===== ENUMERATIONS & CONSTANTS =====
    //sebi
	//typename static	const	EIterator	INVALID_ITERATOR;
	static	const	EIterator	INVALID_ITERATOR;

	//===== CREATORS =====

	inline EList();
	EList(const EList<ELIST_TPL_ARG>& rList);
	~EList();

	//===== OPERATORS =====

	EList<ELIST_TPL_ARG>& operator=(const EList<ELIST_TPL_ARG>& rList);
	inline	T&		operator[](unsigned long Pos);	
	inline	T&		operator[](const typename EList<ELIST_TPL_ARG>::EIterator& rIter);	
	inline	T_ARG	operator[](unsigned long Pos) const;
	inline	T_ARG	operator[](const typename EList<ELIST_TPL_ARG>::EIterator& rIter) const;

	inline	bool	operator==(const EList<ELIST_TPL_ARG>& rList) const;
	inline	bool	operator!=(const EList<ELIST_TPL_ARG>& rList) const;

	//===== MANIPULATORS =====

	bool	Clear();

	inline	bool	Prepend(T_ARG New_Element);	// Add an element to the start of the list
	inline	bool	Append(T_ARG New_Element);	// Add an element to the end of the list
	inline	bool	Insert(T_ARG New_Element, const typename EList<ELIST_TPL_ARG>::EIterator& rIter);	// Insert an element bofore the specified position
	inline	bool	Insert(T_ARG New_Element, unsigned long Pos);		// Insert an element bofore the specified position

	bool	Prepend(const EList<ELIST_TPL_ARG>& rList);	// Prepend a list to this one
	bool	Append(const EList<ELIST_TPL_ARG>& rList);	// Append a list to this one
	bool	Insert(const EList<ELIST_TPL_ARG>& rList, const typename EList<ELIST_TPL_ARG>::EIterator& rIter);	// Insert a list at the specified position
	bool	Insert(const EList<ELIST_TPL_ARG>& rList, unsigned long Pos);	// Insert a list at the specified position

	inline	bool	DeleteHead();					// Remove the element at the beginning of the list
	inline	bool	DeleteTail();					// Remove the element at the end of the list
	inline	bool	Delete(const typename EList<ELIST_TPL_ARG>::EIterator& rIter);	// Remove the element at the specified position
	inline	bool	Delete(unsigned long Pos);				// Remove the element at the specified position
	inline	bool	Delete(const typename EList<ELIST_TPL_ARG>::EIterator& rStart_Iter, const typename EList<ELIST_TPL_ARG>::EIterator& rEnd_Iter);	// Remove the element at the specified position
	inline	bool	Delete(unsigned long Start_Pos, unsigned long End_Pos);	// Remove the element at the specified position


	inline	bool	Replace(T_ARG Element, const typename EList<ELIST_TPL_ARG>::EIterator& rIter);	// Replace an element at the specified position
	inline	bool	Replace(T_ARG Element, unsigned long Pos);		// Replace an element at the specified position

	inline	typename EList<ELIST_TPL_ARG>::EIterator Iterator(unsigned long Pos);	
	inline	const typename EList<ELIST_TPL_ARG>::EConstIterator Iterator(unsigned long Pos) const;
	inline	typename EList<ELIST_TPL_ARG>::EIterator Begin();
	inline	const typename EList<ELIST_TPL_ARG>::EConstIterator Begin() const;
	inline	typename EList<ELIST_TPL_ARG>::EIterator End();
	inline	const typename EList<ELIST_TPL_ARG>::EConstIterator End() const;
  
	inline	T&	Get(const typename EList<const ELIST_TPL_ARG>::EIterator& rIter);	// Retrieve the element at the specified position
	inline	T&	Get(unsigned long Pos);			// Retrieve the element at the specified position
	inline	T&	GetHead();				// Retrieve the element at the start of the list
	inline	T&	GetTail();				// Retrieve the element at the end of the list

	//===== ACCESSORS =====

	inline	T_ARG	Get(const typename EList<ELIST_TPL_ARG>::EConstIterator& rIter) const;	// Retrieve the element at the specified position
	inline	T_ARG	Get(unsigned long Pos) const;	// Retrieve the element at the specified position
	inline	T_ARG	GetHead() const;		// Retrieve the element at the start of the list
	inline	T_ARG	GetTail() const;		// Retrieve the element at the end of the list

	inline	unsigned long	Count() const;			// Get the number of elements in the list
	inline	bool	IsEmpty() const;		// Check if the list is empty
	inline	bool	Exists(unsigned long Pos) const;// Check if an element at that position exists
	inline	typename EList<ELIST_TPL_ARG>::EConstIterator	Find(T_ARG Item, unsigned long Start_Index = 0) const;
	inline	typename EList<ELIST_TPL_ARG>::EIterator	Find(T_ARG Item, unsigned long Start_Index = 0);
	inline	typename EList<ELIST_TPL_ARG>::EConstIterator	Find(T_ARG Item, const typename EList<ELIST_TPL_ARG>::EConstIterator& rStart_Iterator) const;
	inline	typename EList<ELIST_TPL_ARG>::EIterator	Find(T_ARG Item, const typename EList<ELIST_TPL_ARG>::EIterator& rStart_Iterator);

	inline	unsigned long GrowSize() const;			// Get the growsize of list

private:

	//===== DATA =====

	unsigned long				m_Count;			// number of elements currently in list

	ENode*				m_pHead;			// pointer to first node in list
	ENode*				m_pTail;			// pointer to last node in list

	//===== HELPER FUNCTIONS =====

	inline	bool	AddFirstElement(T_ARG New_Element);
	inline	typename EList<ELIST_TPL_ARG>::ENode*	CreateElement(T_ARG New_Element);
	inline	void	KillElement(ENode* pElement);
	void	DestroyList();
	bool	CopyData(const EList<ELIST_TPL_ARG>& rSrc);


}; // END CLASS EList


//*************************************************************************************************
// Constants
//*************************************************************************************************

ELIST_TPL_DEF const typename EList<ELIST_TPL_ARG>::EIterator	EList<ELIST_TPL_ARG>::INVALID_ITERATOR = EList<ELIST_TPL_ARG>::EIterator();


//*************************************************************************************************
// Inline Functions
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// CREATORS
//-------------------------------------------------------------------------------------------------
/**************************************************************************************************
FUNCTION DESCRIPTION:
	EList
		Standard constructor used to instantiate a list
INPUT PARAMETERS:
	Size:	Initial size of the memory pool in number of elements
	Grow:	The size in elements to grow the pool when required
	pName:	The name of the list (optional)
***************************************************************************************************/
ELIST_TPL_DEF inline EList<ELIST_TPL_ARG>::EList()
: m_Count(0), m_pHead(NULL), m_pTail(NULL)
{
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	EList
		Copy constructor of the list
INPUT PARAMETERS:
	rList:
		The list to be made a copy of
***************************************************************************************************/
ELIST_TPL_DEF EList<ELIST_TPL_ARG>::EList(const EList<ELIST_TPL_ARG>& rList)

{
	CopyData(rList);
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	~EList
		List Destructor, deletes all the elements in the list and then frees the memory pool
***************************************************************************************************/
ELIST_TPL_DEF EList<ELIST_TPL_ARG>::~EList()
{
	DestroyList();
}
	
//-------------------------------------------------------------------------------------------------
// OPERATORS
//-------------------------------------------------------------------------------------------------
/**************************************************************************************************
FUNCTION DESCRIPTION:
	operator=
		Assignment operator, destroys the current elements and creates copies of all the
		elements in the source list and adds them to this list. 
INPUT PARAMETERS:
	rList:	The list to be assigned to this one
RETURN VALUE:
	A reference to this list
***************************************************************************************************/
ELIST_TPL_DEF EList<ELIST_TPL_ARG>& EList<ELIST_TPL_ARG>::operator=(const EList<ELIST_TPL_ARG>& rList)
{
	//
	//	Get rid of any list entries that might be used
	//
	DestroyList();

	CopyData(rList);
	return(*this);
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	operator[]
		Retrieve a reference to an element in the list.
		Const versions of these functions are also supplied
INPUT PARAMETERS:
	rIter:	An iterator that points at the element for which we want a reference
	Pos:	Position of the element in the list for which we want a reference
RETURN VALUE:
		A reference to the desired element
***************************************************************************************************/
ELIST_TPL_DEF inline T& EList<ELIST_TPL_ARG>::operator[](const typename EList<ELIST_TPL_ARG>::EIterator& rIter)
{
	gosASSERT(rIter.IsValid() && m_Count);	// Make sure we are using a valid iterator and have something in the list
	return(rIter.m_pCur_Node->m_Data);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline T& EList<ELIST_TPL_ARG>::operator[](unsigned long Pos)
{
	gosASSERT(Pos < m_Count);		// Need to stay within range of number of elements
	return  operator[](Iterator((unsigned long)Pos));
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline T_ARG EList<ELIST_TPL_ARG>::operator[](const typename EList<ELIST_TPL_ARG>::EIterator& rIter) const
{
	gosASSERT(rIter.IsValid() && m_Count);	// Make sure we are using a valid iterator and have something in the list
	return(rIter.m_pCur_Node->m_Data);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline T_ARG EList<ELIST_TPL_ARG>::operator[](unsigned long Pos) const
{
	gosASSERT(Pos < m_Count);		// Need to stay within range of number of elements
	return(Get(Iterator(Pos)));
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	operator==
		Equality operator, tests if this and another list are equal
INPUT PARAMETERS:
	rList:	Reference to the list we want to compare this one with
RETURN VALUE:
		TRUE if equal, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::operator==(const EList<ELIST_TPL_ARG>& rList) const
{
	//
	//	If the count of elements is different in each list,
	//	they're not equal
	//
	if(m_Count != rList.m_Count)
	{
		return(false);
	}

	//
	//	In case both lists are empty, they're equal, however, we don't
	//	want to iterate if there is nothing to iterate
	//
	if(m_Count == 0 && rList.m_Count == 0)
	{
		return(true);
	}

	EConstIterator	List_Iter = rList.Begin();
	EConstIterator	This_Iter = Begin();

	//
	//	Iterate through both lists and check if they're equal
	//
	for(unsigned long i = 0; i < m_Count; i++)
	{
		if(List_Iter.Item() != This_Iter.Item())
		{
			return(false);
		}
	}

	return(true);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::operator!=(const EList<ELIST_TPL_ARG>& rList) const
{
	return(!(rList == *this));
}


//-------------------------------------------------------------------------------------------------
// MANIPULATORS
//-------------------------------------------------------------------------------------------------
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Iterator
		Get an iterator that points at the element corresponding with the Position passed
		
INPUT PARAMETERS:
	Pos:	The position of the element we want an iterator for
RETURN VALUE:
		An iterator that points at the element at the position we passed
***************************************************************************************************/
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::EIterator EList<ELIST_TPL_ARG>::Iterator(unsigned long Pos)
{
	gosASSERT(Pos < m_Count && m_Count);	// Need to stay within range of number of elements

	if(!m_Count)
	{
		return(INVALID_ITERATOR);
	}

	//
	//	Iterate through the list until we get to the element we desire
	//
	ENode	*pNode = m_pHead;
	for(unsigned long i = 0; i < Pos; i++)
	{
		pNode = pNode->m_pNext;
	}
	EIterator	Iter;
	Iter.m_pCur_Node = pNode;
	return(Iter);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline const typename EList<ELIST_TPL_ARG>::EConstIterator EList<ELIST_TPL_ARG>::Iterator(unsigned long Pos) const
{
	gosASSERT(Pos < m_Count && m_Count);	// Need to stay within range of number of elements

	if(!m_Count)
	{
		return(INVALID_ITERATOR);
	}

	//
	//	Iterate through the list until we get to the element we desire
	//
	ENode	*pNode = m_pHead;
	for(unsigned long i = 0; i < Pos; i++)
	{
		pNode = pNode->m_pNext;
	}
	EIterator	Iter;
	Iter.m_pCur_Node = pNode;
	return(Iter);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Begin
		Get an iterator that points at the head element of the list.
		Also available as const version
RETURN VALUE:
	An iterator that points at the head element		
***************************************************************************************************/
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::EIterator EList<ELIST_TPL_ARG>::Begin()
{
	if(!m_Count)
	{
		return(INVALID_ITERATOR);
	}

	EIterator	Iter;
	Iter.m_pCur_Node = m_pHead;
	return(Iter);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline const typename EList<ELIST_TPL_ARG>::EConstIterator EList<ELIST_TPL_ARG>::Begin() const
{
	if(!m_Count)
	{
		return(INVALID_ITERATOR);
	}

	EIterator	Iter;
	Iter.m_pCur_Node = m_pHead;
	return(Iter);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	End
		Get an iterator that points at the tail element of the list
		Also available as const version
RETURN VALUE:
	An iterator that points at the tail element		
***************************************************************************************************/
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::EIterator EList<ELIST_TPL_ARG>::End()
{
	gosASSERT(m_Count);		// Don't try to get an iterator if list is empty

	if(!m_Count)
	{
		return(INVALID_ITERATOR);
	}

	EIterator	Iter;
	Iter.m_pCur_Node = m_pTail;
	return(Iter);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline const typename EList<ELIST_TPL_ARG>::EConstIterator EList<ELIST_TPL_ARG>::End() const
{
	gosASSERT(m_Count);		// Don't try to get an iterator if list is empty

	if(!m_Count)
	{
		return(INVALID_ITERATOR);
	}

	EIterator	Iter;
	Iter.m_pCur_Node = m_pTail;
	return(Iter);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Clear
		Deletes all elements from the list
RETURN VALUE:
		TRUE if success
***************************************************************************************************/
ELIST_TPL_DEF bool EList<ELIST_TPL_ARG>::Clear()
{
	DestroyList();
	return(true);	// <<HACK>> BR - need to do checking
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Prepend
		Prepend an element to the list, this element will be the new head element
INPUT PARAMETERS:
	New_Element:	The element we want to prepend to the list
RETURN VALUE:
		TRUE if success
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Prepend(T_ARG New_Element)
{
	//
	//	If the list is empty, let's create the first element
	//
	if(IsEmpty())
	{
		return(AddFirstElement(New_Element));
	}

	//
	//	Otherwise create a new element and prepend to to the list
	//
	ENode* pNode = CreateElement(New_Element);
	if(pNode)
	{
		m_pHead->m_pPrev = pNode;
		pNode->m_pNext = m_pHead;
		pNode->m_pPrev = NULL;
		m_pHead = pNode;
		m_Count++;
		return(true);
	}
	return(false);	// <<HACK>> BR
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Append
		Append an element to the list, this element will be the new tail element
INPUT PARAMETERS:
	New_Element:	The element we want to append to the list
RETURN VALUE:
		TRUE if success
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Append(T_ARG New_Element)
{
	//
	//	If the list is empty, let's create the first element
	//
	if(IsEmpty())
	{
		return(AddFirstElement(New_Element));
	}

	//
	//	Otherwise create a new element and append to to the list
	//
	ENode* pNode = CreateElement(New_Element);
	if(pNode)
	{
		m_pTail->m_pNext = pNode;
		pNode->m_pPrev = m_pTail;
		pNode->m_pNext = NULL;
		m_pTail = pNode;
		m_Count++;
		return(true);
	}
	return(false);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Insert
		Insert an element into the list at the given position
INPUT PARAMETERS:
	New_Element:	The element to be added
	Pos:			The position at which to insert the element
	rIter:			An iterator the points to the location where we want to insert the element
RETURN VALUE:
		TRUE if success
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Insert(T_ARG New_Element, const typename EList<ELIST_TPL_ARG>::EIterator& rIter)
{
	//
	//	Make sure we have a valid iterator before we continue
	//
	gosASSERT(rIter.m_pCur_Node);

	//
	//	Attempt to create a new element
	//
	ENode* pNode = CreateElement(New_Element);
	if(!pNode)
	{
		return(false);
	}

	//
	//	Connect the newly created node
	//
	pNode->m_pNext = rIter.m_pCur_Node;
	pNode->m_pPrev = rIter.m_pCur_Node->m_pPrev;


	//
	//	Make sure to connect the previous node only if we are not the first in the list
	//
	if(pNode->m_pPrev)
	{
		pNode->m_pPrev->m_pNext = pNode;
	}
	else
	{
		// if we are, set the haed pointer
		m_pHead = pNode;
	}

	//
	//	Last thing to do is to connect the next node to oursef
	//
	pNode->m_pNext->m_pPrev = pNode;
	m_Count++;
	return(true);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Insert(T_ARG  New_Element, unsigned long Pos)
{
	gosASSERT(m_Count > Pos && m_Count);

	//
	//	We can't INSERT after the end of the list, so fail here
	//
	if(m_Count <= Pos)
	{
		return(false);
	}

	//
	//	In case list is empty, simply prepend the element, Prepend() handles this case
	//
	if(IsEmpty())
	{
		return(Prepend(New_Element));
	}

	return(Insert(New_Element, Iterator(Pos)));
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	DeleteHead
		Delete the head element from the list. This will call the destructor of the element
		to assure that any memory allocated by the element can be freed.
RETURN VALUE:
		TRUE is success, FALSE if list is empty
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::DeleteHead()
{
	//
	//	can't delete anything if there is nothing...
	//
	gosASSERT(m_Count);
	if(!m_Count)
	{
		return(false);
	}


	//
	//	In case it's the only element in the list
	//
	if(m_Count == 1)
	{
		KillElement(m_pHead);
		m_pHead = m_pTail = NULL;		// make sure haed and tail pointers are set to NULL
										// if the list is empty
		m_Count = 0;
		return(true);
	}
	else
	{
		ENode* pElement_To_Kill = m_pHead;		// Get the element we want to kill
		m_pHead = m_pHead->m_pNext;				// Second element becomes head of the list 
		m_pHead->m_pPrev = NULL;
		KillElement(pElement_To_Kill);

		m_Count--;
		return(true);
	}
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	DeleteTail
		Delete the tail element from the list. This will call the destructor of the element
		to assure that any memory allocated by the element can be freed.
RETURN VALUE:
		TRUE is success, FALSE if list is empty
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::DeleteTail()
{
	//
	//	Make sure we're not trying to delete from an empty list
	//
	gosASSERT(m_Count);
	if(!m_Count)
	{
		return(false);
	}

	if(m_Count == 1)
	{
		KillElement(m_pHead);
		m_pHead = m_pTail = NULL;		// make sure haed and tail pointers are set to NULL
										// if the list is empty
		m_Count = 0;
		return(true);
	}
	else
	{
		ENode* pElement_To_Kill = m_pTail;
		m_pTail = m_pTail->m_pPrev;
		m_pTail->m_pNext = NULL;
		KillElement(pElement_To_Kill);

		m_Count--;
		return(true);
	}
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Delete
		Delete an element from the list
INPUT PARAMETERS:
	rIter:	An iterator that points at the element we want to delete
RETURN VALUE:
		TRUE if success
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Delete(const typename EList<ELIST_TPL_ARG>::EIterator& rIter)
{
	//
	//	Verify that we have a valid iterator
	//
	gosASSERT(rIter.IsValid());

	if(m_Count == 1)
	{
		KillElement(m_pHead);
		m_pHead = m_pTail = NULL;		
		m_Count = 0;
		return(true);
	}
	else
	{
		ENode* pElement_To_Kill = rIter.m_pCur_Node;
		if(m_pTail == pElement_To_Kill)
		{
			m_pTail = pElement_To_Kill->m_pPrev;
			m_pTail->m_pNext = NULL;
		}
		else if(m_pHead == pElement_To_Kill)
		{
			m_pHead = pElement_To_Kill->m_pNext;
			m_pHead->m_pPrev = NULL;
		}
		else
		{
			pElement_To_Kill->m_pPrev->m_pNext = pElement_To_Kill->m_pNext;
			pElement_To_Kill->m_pNext->m_pPrev = pElement_To_Kill->m_pPrev;
		}

		KillElement(pElement_To_Kill);
		m_Count--;
		return(true);
	}
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Delete
		Delete an element from the list
INPUT PARAMETERS:
	Pos:	Position of the element we want to delete
RETURN VALUE:
		TRUE if success
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Delete(unsigned long Pos)
{
	gosASSERT(m_Count > Pos && m_Count);

	//
	//	We can't DELETE after the end of the list or if empty, so fail here
	//
	if(m_Count <= Pos || (!m_Count))
	{
		return(false);
	}

	return(Delete(Iterator(Pos)));
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Delete
		Delete a range of elements in the list
INPUT PARAMETERS:
	Start_Pos:	First element in the list to delete
	End_Pos:	Last element in the list to delete
	rStart_Iter:First element in the list to delete
	rEnd_Iter:	Last element in the list to delete
RETURN VALUE:
		TRUE if successfull
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Delete(const typename EList<ELIST_TPL_ARG>::EIterator& rStart_Iter, const typename EList<ELIST_TPL_ARG>::EIterator& rEnd_Iter)
{
	gosASSERT(rStart_Iter.IsValid() && rEnd_Iter.IsValid());
	gosASSERT((rStart_Iter != rEnd_Iter));

	if( (!(rStart_Iter.IsValid())) || (!(rEnd_Iter.IsValid())) )
	{
		return(false);
	}

	EIterator Iter;
	EIterator Next_Iter = rStart_Iter;

	while(Next_Iter != rEnd_Iter)
	{
		Iter = Next_Iter;

		Next_Iter++;
		Delete(Iter);
	}
	return(true);
}

//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Delete(unsigned long Start_Pos, unsigned long End_Pos)
{
	gosASSERT(Start_Pos < End_Pos);
	return(Delete(Iterator(Start_Pos), Iterator(End_Pos)));
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Get
		Gets a reference to the element that is at the position we passed to the function
INPUT PARAMETERS:
	Pos:	The position of the element we want to get
	rIter:	The iterator of the element we want to get
RETURN VALUE:
		Reference to the element at the position we passed
***************************************************************************************************/
ELIST_TPL_DEF inline T& EList<ELIST_TPL_ARG>::Get(const typename EList<const ELIST_TPL_ARG>::EIterator& rIter)
{
	gosASSERT(m_Count && rIter.IsValid());
	return(rIter.m_pCur_Node->m_Data);
}
	
//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline T& EList<ELIST_TPL_ARG>::Get(unsigned long Pos)
{
	gosASSERT(m_Count > Pos && m_Count);
	return(Get(Iterator(Pos)));
}
/**************************************************************************************************
FUNCTION DESCRIPTION:
	GetHead
		Get a reference to the element that is at the head of the list
RETURN VALUE:
		Reference to the element at the head of the list
***************************************************************************************************/
ELIST_TPL_DEF inline T& EList<ELIST_TPL_ARG>::GetHead()
{
	gosASSERT(m_Count);
	return(m_pHead->m_Data);
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	GetTail
		Get a reference to the element that is at the tail of the list
RETURN VALUE:
		Reference to the element at the tail of the list
***************************************************************************************************/
ELIST_TPL_DEF inline T& EList<ELIST_TPL_ARG>::GetTail()
{
	gosASSERT(m_Count);
	return(m_pTail->m_Data);
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Replace
		Replace an element in the list with a new one
INPUT PARAMETERS:
	Element:	The new element we want to replace the old one with
	rIter:		An iterator to the element we want to replace
RETURN VALUE:
		TRUE if success, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Replace(T_ARG Element, const typename EList<ELIST_TPL_ARG>::EIterator& rIter)
{
	gosASSERT(rIter.IsValid());

	if(!(rIter.IsValid()))
	{
		return(false);
	}
	rIter.m_pCur_Node->m_Data = Element;
	return(true);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Replace
		Replace an element in the list with a new one
INPUT PARAMETERS:
	Element:	The new element we want to replace the old one with
	Pos:		An index to the element we want to replace
RETURN VALUE:
		TRUE if success, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Replace(T_ARG Element, unsigned long Pos)
{
	return(Replace(Element, Iterator(Pos)));
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Append
		Appends the given list to this one
INPUT PARAMETERS:
	rList:	A reference to the list that is to be prepended
RETURN VALUE:
		TRUE if success, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF bool EList<ELIST_TPL_ARG>::Append(const EList<ELIST_TPL_ARG>& rList)
{
	gosASSERT(&rList != this);
	//
	//	Iterate through the source list and add any element to this one
	//
	const ENode*	pNode = rList.m_pHead;
	for(unsigned long i = 0; i < rList.m_Count; i++)
	{
		Append(pNode->m_Data);		// <<TODO>> BR - This needs to be optimized
		pNode = pNode->m_pNext;
	}
	return(true);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Prepend
		Prepends the given list to this one
INPUT PARAMETERS:
	rList:	A reference to the list that is to be prepended
RETURN VALUE:
		TRUE if success, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF bool EList<ELIST_TPL_ARG>::Prepend(const EList<ELIST_TPL_ARG>& rList)
{
	gosASSERT(&rList != this);
	//
	//	Iterate through the source list and add any element to this one
	//
	const ENode*	pNode = rList.m_pTail;
	for(unsigned long i = 0; i < rList.m_Count; i++)
	{
		Prepend(pNode->m_Data);		// <<TODO>> BR - This needs to be optimized
		pNode = pNode->m_pPrev;
	}
	return(true);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Insert
		Insert all elements from the list given into this list
INPUT PARAMETERS:
	rList:	A reference to the list to be inserted
	rIter:	An iterator into the list at which to start the insert
RETURN VALUE:
		TRUE if success, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF bool EList<ELIST_TPL_ARG>::Insert(const EList<ELIST_TPL_ARG>& rList, const typename EList<ELIST_TPL_ARG>::EIterator& rIter)
{
	gosASSERT(&rList != this);
	gosASSERT(rIter.IsValid());
	EConstIterator	Src_Iter = rList.Begin();

	for(unsigned long i = 0; i < rList.m_Count; i++)
	{
		Insert(Src_Iter.Item(), rIter);
		Src_Iter++;
	}
	return(true);

}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Insert
		Insert all elements from the list given into this list
INPUT PARAMETERS:
	rList:	A reference to the list to be inserted
	Pos:	An index to the position at which to start the insert
RETURN VALUE:
		TRUE if success, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF bool EList<ELIST_TPL_ARG>::Insert(const EList<ELIST_TPL_ARG>& rList, unsigned long Pos)
{
	gosASSERT( &rList != this);
	return(Insert(rList, Iterator(Pos)));
}


//-------------------------------------------------------------------------------------------------
// ACCESSORS
//-------------------------------------------------------------------------------------------------
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Get
		Get an element from the list using an iterator to determine it's location
INPUT PARAMETERS:
	rIter:	The iterator of the element we want to get
RETURN VALUE:
		The element referenced by the iterator
***************************************************************************************************/
ELIST_TPL_DEF inline T_ARG EList<ELIST_TPL_ARG>::Get(const typename EList<ELIST_TPL_ARG>::EConstIterator& rIter) const
{
	gosASSERT(m_Count && rIter.IsValid());
	return(rIter.m_pCur_Node->m_Data);
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Get
		Get the element that is at the position we passed to the function
INPUT PARAMETERS:
	Pos:	The position of the element we want to get
RETURN VALUE:
		The element at the position we passed
***************************************************************************************************/
ELIST_TPL_DEF inline T_ARG EList<ELIST_TPL_ARG>::Get(unsigned long Pos) const
{
	gosASSERT(m_Count > Pos && m_Count);
	return(Get(Iterator(Pos)));
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	GetHead
		Get the element that is at the head of the list
RETURN VALUE:
		The element at the head of the list
***************************************************************************************************/
ELIST_TPL_DEF inline T_ARG EList<ELIST_TPL_ARG>::GetHead() const
{
	gosASSERT(m_Count);
	return(m_pHead->m_Data);
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	GetTail
		Get the element that is at the tail of the list
RETURN VALUE:
		The element at the tail of the list
***************************************************************************************************/
ELIST_TPL_DEF inline T_ARG EList<ELIST_TPL_ARG>::GetTail() const
{
	gosASSERT(m_Count);
	return(m_pTail->m_Data);
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	Find
		Search through the list for the item corresponding to "Item"
INPUT PARAMETERS:
	Item:				The item to search for
	Start_Index:		The element at which to start the search
RETURN VALUE:
		The iterator that corresponds with the element in the list we searched for,
		or INVALID_ITERATOR if the element could not be found
***************************************************************************************************/
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::EConstIterator EList<ELIST_TPL_ARG>::Find(T_ARG Item, unsigned long Start_Index) const
{
	gosASSERT(Start_Index < m_Count && (!(IsEmpty())));
	return(Find(Item, Iterator(Start_Index)));
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Find
		Search through the list for the item corresponding to "Item"
INPUT PARAMETERS:
	Item:				The item to search for
	Start_Index:		The element at which to start the search
RETURN VALUE:
		The iterator that corresponds with the element in the list we searched for,
		or INVALID_ITERATOR if the element could not be found
***************************************************************************************************/
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::EIterator EList<ELIST_TPL_ARG>::Find(T_ARG Item, unsigned long Start_Index)
{
	gosASSERT(Start_Index < m_Count && (!(IsEmpty())));
	return(Find(Item, Iterator(Start_Index)));
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Find
		Search through the list for the item corresponding to "Item"
INPUT PARAMETERS:
	Item:				The item to search for
	rStart_Iterator:	The element at which to start the search
RETURN VALUE:
		The iterator that corresponds with the element in the list we searched for,
		or INVALID_ITERATOR if the element could not be found		
***************************************************************************************************/
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::EConstIterator EList<ELIST_TPL_ARG>::Find(T_ARG Item, const typename EList<ELIST_TPL_ARG>::EConstIterator& rStart_Iterator) const
{
	gosASSERT(rStart_Iterator.IsValid());

	EConstIterator	Iter = rStart_Iterator;

	while(!(Iter.IsDone()))
	{
		if(Iter.Item() == Item)
		{
			return(Iter);
		}
		Iter++;
	}
	return(INVALID_ITERATOR);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Find
		Search through the list for the item corresponding to "Item"
INPUT PARAMETERS:
	Item:				The item to search for
	rStart_Iterator:	The element at which to start the search
RETURN VALUE:
		The iterator that corresponds with the element in the list we searched for,
		or INVALID_ITERATOR if the element could not be found		
***************************************************************************************************/
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::EIterator EList<ELIST_TPL_ARG>::Find(T_ARG Item, const typename EList<ELIST_TPL_ARG>::EIterator& rStart_Iterator)
{
	gosASSERT(rStart_Iterator.IsValid());

	EIterator	Iter = rStart_Iterator;

	while(!(Iter.IsDone()))
	{
		if(Iter.Item() == Item)
		{
			return(Iter);
		}
		Iter++;
	}
	return(INVALID_ITERATOR);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	Count
		Retrieve the number of elements currently in the list
RETURN VALUE:
		The number of elements in the list
***************************************************************************************************/
ELIST_TPL_DEF inline unsigned long EList<ELIST_TPL_ARG>::Count() const
{
	return(m_Count);
}
	

/**************************************************************************************************
FUNCTION DESCRIPTION:
	IsEmpty
		Test the list for emptyness
RETURN VALUE:
		TRUE if list is empty, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::IsEmpty() const
{
	return(m_Count ? false : true);
}

/**************************************************************************************************
FUNCTION DESCRIPTION:
	GrowSize
		Retrieve the current Growsize of the list
RETURN VALUE:
		Current Growsize
***************************************************************************************************/
// sebi no m_ListPool, was compiling on windows because this function was never used
/*
ELIST_TPL_DEF inline unsigned long EList<ELIST_TPL_ARG>::GrowSize() const
{
	return(m_List_Pool.PageSize());
}
*/


/**************************************************************************************************
FUNCTION DESCRIPTION:
	Exists
		Determine whether an entry exists in the list
INPUT PARAMETERS:
	Pos:	The position of the element we want to check for existance
RETURN VALUE:
	TRUE if exist, FALSE if not
***************************************************************************************************/
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::Exists(unsigned long Pos) const
{
	if(Pos < m_Count)
	{
		return(true);
	}
	return(false);
}


//-------------------------------------------------------------------------------------------------
// HELPER FUNCTIONS
//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline bool EList<ELIST_TPL_ARG>::AddFirstElement(T_ARG New_Element)
{
	ENode* pElement = CreateElement(New_Element);
	if(!pElement)
	{
		return(false);
	}

	m_pHead = m_pTail = pElement;				// Setup List head and tail pointers
	pElement->m_pNext = pElement->m_pPrev = NULL;
	m_Count = 1;

	return(true);
}

//-------------------------------------------------------------------------------------------------
// Note: m_pNext and m_pPrev are not initialized
ELIST_TPL_DEF inline typename EList<ELIST_TPL_ARG>::ENode* EList<ELIST_TPL_ARG>::CreateElement(T_ARG New_Element)
{
	ENode*	pNode = (ENode*)systemHeap->Malloc( sizeof( ENode ) );	// Allocate memory for the node and data
	if(!pNode)
	{
		return(NULL);						// return NULL if it fails
	}

	pNode = new(pNode) ENode(New_Element);	// and construct the data object explicitly
	gosASSERT(pNode);
	return(pNode);							// return the pointer to the Node of the element we created
}
//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline void EList<ELIST_TPL_ARG>::KillElement(ENode* pElement)
{
	gosASSERT(pElement);
	pElement->m_Data.~T();			// Destruct the data component of the element
	systemHeap->Free(pElement);					// Now free the element		
}
	
//-------------------------------------------------------------------------------------------------
ELIST_TPL_DEF inline void EList<ELIST_TPL_ARG>::DestroyList()
{
	//
	//	simply return if list is empty
	//
	if(IsEmpty())
	{
		return;
	}

	//
	//	Otherwise remove the head element until none are left
	//
	while(m_Count)
	{
		DeleteHead();
	}
}
	
/**************************************************************************************************
FUNCTION DESCRIPTION:
	CopyData
		Copies data from the source list into this one
		NOTE:	The function assumes the list to be empty at when it is called
INPUT PARAMETERS:
	rSrc:	Reference to the source list
RETURN VALUE:
		TRUE is success
***************************************************************************************************/
ELIST_TPL_DEF bool EList<ELIST_TPL_ARG>::CopyData(const EList<ELIST_TPL_ARG>& rSrc)
{
	//
	//	Iterate through the source list and add any element to this one
	//

	m_Count = rSrc.m_Count;
	if(!m_Count)
	{
		m_pHead = m_pTail = NULL;
		return(true);
	}

	ENode*	pSrc_Node = rSrc.m_pHead;
	ENode*	pNode = CreateElement(pSrc_Node->m_Data);
	gosASSERT(pNode && pSrc_Node);		// Should never be NULL
	m_pHead = m_pTail = pNode;
	m_pHead->m_pPrev = NULL;
	
	for(unsigned long i = 1; i<rSrc.m_Count; i++)
	{
		pSrc_Node = pSrc_Node->m_pNext;
		gosASSERT(pSrc_Node);			// Should never be NULL

		m_pTail = CreateElement(pSrc_Node->m_Data);
		gosASSERT(m_pTail);

		m_pTail->m_pPrev = pNode;
		pNode->m_pNext = m_pTail;
		pNode = m_pTail;
	}
	m_pTail->m_pNext = NULL;


	return(true);
}
	
//*************************************************************************************************
#endif  // end of file ( ELIST.h )
