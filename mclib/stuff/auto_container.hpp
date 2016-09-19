//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#ifndef AUTO_CONTAINER_HPP
#define AUTO_CONTAINER_HPP

#pragma warning (disable:4786)							// this is necessary to avoid "truncated to 255 characters in debug info" message
#include"auto_ptr.hpp"
#include"noncopyable.hpp"



namespace Stuff
{
	template <class pointed_to, class container_type>
	class Auto_Container
		: public Noncopyable
	{
	  public:
		virtual ~Auto_Container()
		{
			clear();
		}

		typedef pointed_to* value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* iterator;
		typedef const value_type* const_iterator;
		typedef size_t size_type;

		iterator begin()						{ return (m_Container.begin()); }
		const_iterator begin() const			{ return (m_Container.begin()); }

		iterator end()							{ return (m_Container.end());	}
		const_iterator end() const				{ return (m_Container.end());	}

		size_type size() const					{ return (m_Container.size());	}

		bool empty() const						{ return (m_Container.empty());	}

		size_type max_size() const				{ return (m_Container.max_size());	}

		void swap(container_type& other_container)	{ m_Container.swap(other_container); }

		reference at(size_type pos)				{ return (m_Container.at(pos));	}
		const_reference at(size_type pos) const	{ return (m_Container.at(pos));	}

		reference operator[](size_type pos)		{ return (m_Container[pos]);	}
		const_reference operator[](size_type pos) const	{ return (m_Container[pos]);	}

		void clear()							{ erase(begin(),end());	}

		reference back()						{ return (m_Container.back());	}
		const_reference back() const			{ return (m_Container.back());	}

		reference front()						{ return (m_Container.front());	}
		const_reference front() const			{ return (m_Container.front());	}

		iterator erase(iterator it)
		{
			iterator rv = m_Container.erase(it);
			delete (*rv);
			return (rv);
		}

		iterator erase(iterator first, iterator last)
		{
			{for (iterator i = first;
				  i != last;
				  ++i)
			{
				delete (*i);
			}}

			return (m_Container.erase(first,last));
		}

		Auto_Ptr<pointed_to> remove(iterator it)
		{
			Auto_Ptr<pointed_to> rv(*it);
			m_Container.erase(it);
			return (rv);
		}

		Auto_Ptr<pointed_to> pop_back()
		{
			Auto_Ptr<pointed_to> rv(m_Container.back());
			m_Container.pop_back();
			return (rv);
		}

		void push_back(Auto_Ptr<pointed_to>& x)
		{
			m_Container.push_back(x.ReleaseAndNull());
		}

		iterator insert(iterator it, Auto_Ptr<pointed_to>& x)
		{
			return (m_Container.insert(it,x.ReleaseAndNull()));
		}

		const container_type& GetContainer() const
		{
			return (m_Container);
		}

		void MoveTo(Auto_Container<pointed_to,container_type>& dest)
		{
			while (empty() == false)
			{
				value_type value = m_Container.back();
				dest.m_Container.push_back(value);

				m_Container.pop_back();
			}
		}

	  protected:
		container_type m_Container;
	};
};



#endif // AUTO_CONTAINER_HPP
