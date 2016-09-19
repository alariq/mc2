//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#ifndef Auto_Ptr_HPP
#define Auto_Ptr_HPP

#include"stuff.hpp"


namespace Stuff
{

// Auto_Ptr<>: similar to the STL auto_ptr<> class, but better! :)

// An Auto_Ptr<> is a pointer that "owns" the object pointed to.
// When the Auto_Ptr<> is deleted, it will also delete the object
// if it still owns the pointer.  Auto_Ptr<> also handles the transfer
// of ownership between Auto_Ptrs via the == operators.

// This version of Auto_Ptr<> is slightly more memory-efficient as it
// stores the ownership bits in the unused bits of the pointer data member.
// It also allows you to specify whether or not to use array deletion;
// this also is stored in the extra bits of the pointer.


	template <class T>
	class Auto_Ptr
	{
	  public:
		Auto_Ptr()
			: m_ptr(0)
		{
			Verify(sizeof(T*) == sizeof(unsigned int));
		}

		explicit Auto_Ptr(T* ptr, bool delete_as_array = DELETE_NORMAL)
		{
			Verify(sizeof(T *) == sizeof(unsigned int));
			Set(ptr,true,delete_as_array);
		}

		Auto_Ptr(Auto_Ptr const& src)
		{
			Verify(sizeof(T*) == sizeof(unsigned int));
			m_ptr = src.m_ptr;
			src.SetAsOwner(false);
		}

		~Auto_Ptr()
		{
			Delete();
		}

		enum deletion_type
		{
			DELETE_AS_ARRAY = true,
			DELETE_NORMAL = false
		};

		Auto_Ptr<T> Assimilate(T* ptr, bool delete_as_array = DELETE_NORMAL)
		{
			Auto_Ptr<T> temp(*this);
			Set(ptr,true,delete_as_array);
			return (temp);
		}

		Auto_Ptr<T> Assimilate(Auto_Ptr<T>& src)
		{
			Auto_Ptr<T> temp;
			if (&src != this)
			{
				temp = *this;
				m_ptr = src.m_ptr;
				src.SetAsOwner(false);
			}

			return (temp);
		}

		T* GetPointer() const
		{
			return ((T*)(m_bits & POINTER_MASK));
		}

		T* Release() const
		{
			SetAsOwner(false);
			return (GetPointer());
		}

		T* ReleaseAndNull()
		{
			T* rv = GetPointer();
			m_ptr = 0;
			return (rv);
		}

		Auto_Ptr<T>& operator=(Auto_Ptr<T> const& src)
		{
			if (&src != this)
			{
				Delete();
				m_ptr = src.m_ptr;
				src.SetAsOwner(false);
			}

			return (*this);
		}
		
		operator bool() const
		{
			return (GetPointer() != 0);
		}

		T& operator*() const
		{
			return (*GetPointer());
		}

		T* operator->() const
		{
			return (GetPointer());
		}

		void Swap(Auto_Ptr<T>& src)
		{
			if (this != &src)
			{
				T* temp_ptr = src.m_ptr;
				src.m_ptr = m_ptr;
				m_ptr = temp_ptr;
			}
		}

		void Delete()
		{
			if (IsOwner())
			{
				if (IsArray())
				{
					m_bits &= POINTER_MASK;
					delete [] m_ptr;
				}
				else
				{
					m_bits &= POINTER_MASK;
					delete m_ptr;
				}
			}

			m_ptr = 0;
		}

	  private:
		bool IsOwner() const
		{
			return ((m_bits & OWNER_MASK) == OWNER_MASK);
		}

		bool IsArray() const
		{
			return ((m_bits & ARRAY_MASK) == ARRAY_MASK);
		}

		void SetAsOwner(bool fOwn) const
		{
			if (fOwn == true)
			{
				m_bits |= OWNER_MASK;
			}
			else
			{
				m_bits &= ~OWNER_MASK;
			}
		}

		void SetAsArray(bool fArray)
		{
			if (fArray == true)
			{
				m_bits |= ARRAY_MASK;
			}
			else
			{
				m_bits &= ~ARRAY_MASK;
			}
		}

		void Set(T* ptr, bool fIsOwner, bool fIsArray)
		{
			m_ptr = ptr;
			Verify((m_bits & POINTER_MASK) == m_bits);

			if (fIsOwner == true)
			{
				m_bits |= OWNER_MASK;
			}

			if (fIsArray == true)
			{
				m_bits |= ARRAY_MASK;
			}
		}

		union
		{
			T *m_ptr;
			mutable int unsigned m_bits;
		};

		enum
		{
			OWNER_MASK = 1,
			ARRAY_MASK = 2,
			POINTER_MASK = ~3
		};
	};

	template<class T>
	inline bool operator==(const Auto_Ptr<T>& a,
						   const Auto_Ptr<T>& b)
	{
		return (a.GetPointer() == b.GetPointer());
	}

	template<class T>
	inline bool operator==(const T* a,
						   const Auto_Ptr<T>& b)
	{
		return (a == b.GetPointer());
	}

	template<class T>
	inline bool operator==(const Auto_Ptr<T>& a,
						   const T* b)
	{
		return (a.GetPointer() == b);
	}

}; // namespace Stuff


#endif // Auto_Ptr_HPP
