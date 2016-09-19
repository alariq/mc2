#pragma once
//===========================================================================//
// File:	 LinkedList.hpp													 //
// Contents: Linked list routines											 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

	class gosLink
	{
		public:
			gosLink * Next;
			gosLink * Prev;
			void * linkData;
		public:
			gosLink()
			{
				linkData = 0;
				Next = 0;
				Prev = 0;
			}
			gosLink(void * ptr)
			{
				linkData = ptr;
				Next = 0;
				Prev = 0;
			}
    		virtual ~gosLink()
			{
				Next = 0;
				Prev = 0;
			}
			gosLink * GetNext()
			{
				return (gosLink *) linkData;
			}
	};


	template <class T> class LinkedList
	{
		public:
			gosLink * m_Head;
			int m_Size;
			
		public:
			LinkedList()
			{
				m_Head = 0;
				m_Size = 0;
			};
			LinkedList(T  ptr)
			{			 
				gosLink * newlink = (gosLink *) malloc(sizeof(gosLink));
				m_Head = newlink;
				m_Head->Next = 0;
				m_Size = 1;
			};
			~LinkedList()
			{
				while (m_Head != 0) Del((T ) m_Head->linkData);
			};
			void Add(T ptr)
			{
				gosLink * newlink = (gosLink *) malloc(sizeof(gosLink));
				newlink->linkData = ptr;
				newlink->Next = 0;
				newlink->Prev = 0;
				if (m_Head == 0)
				{
					m_Head = newlink;
					newlink->Next = 0;
					newlink->Prev = 0;
				}
				else
				{
					gosLink * tmp = m_Head;
					while(tmp->Next != 0) tmp = tmp->Next;
					tmp->Next = newlink;
					newlink->Next = 0;
					newlink->Prev = tmp;
				}
				m_Size += 1;
			}
			void Del(T  ptr)
			{
				if( !m_Head )						//YIK - Why does it need this! (Delete All surfaces used to crash)
					return;
				gosLink * tmp = m_Head;
				if (tmp->linkData == (void *) ptr)
				{
					m_Head = tmp->Next;
					tmp->Prev = 0;
					m_Size -= 1;
				memset(tmp,0,sizeof(tmp));
				free(tmp);
				}
				else
				{
					gosLink * target;
					while(tmp->Next != 0)
					{
						if (tmp->Next->linkData == (void *) ptr)
						{
							target = tmp->Next;
							tmp->Next = target->Next;
							if (target->Next)
								target->Next->Prev = tmp;
							m_Size -= 1;
							free(target);
							memset(tmp,0,sizeof(tmp));
							return;
						}
						else
						{
							tmp = (gosLink *) tmp->Next;
						}
					}
				}

			}

			int Size()
			{
				return m_Size;
			}
			T Get(int index)
			{
				gosLink * tmp = m_Head;
				if (tmp == 0)
				{
					return 0;
				}
				while(index && tmp->Next)
				{
					tmp = tmp->Next;
					index--;
				}
				if (index != 0) 
				{
					return 0;
				}
				else
				{
					return (T) tmp->linkData;
				}
			}
	 };

	template <class T> class LinkedListIterator
	{
		public:
			gosLink * 
				m_Iterator;
			LinkedList<T> * 
				m_List;
		public:
			LinkedListIterator(LinkedList<T> * list)
			{
				m_Iterator = list->m_Head;
				m_List = list;
			}
			~LinkedListIterator()
			{
			}
			T  Head()
			{
				m_Iterator = m_List->m_Head;
				if (m_List->m_Head == 0) return 0;
				return (T) ((gosLink *)m_List->m_Head)->linkData;
			}
			T  Tail()
			{
				m_Iterator = m_List->m_Head;
				if (!m_Iterator) return 0;
				while (m_Iterator->Next != 0)
				{
					m_Iterator = m_Iterator->Next;
				}
				return (T) ((gosLink *)m_Iterator)->linkData;
			}
			T  ReadAndNext()
			{
				gosLink * tmp = m_Iterator;
				if (tmp == 0) return (T )0;
				m_Iterator = m_Iterator->Next;
				return (T ) ((gosLink *)tmp)->linkData;
			}
			T  ReadAndPrev()
			{
				gosLink * tmp = m_Iterator;
				if (tmp == 0) return (T )0;
				m_Iterator = m_Iterator->Prev;
				return (T ) ((gosLink *)tmp)->linkData;
			}
			T  Next()
			{
				m_Iterator = m_Iterator->Next;
				if (m_Iterator == 0) return 0;
				return (T ) m_Iterator->linkData;
			}
	};

