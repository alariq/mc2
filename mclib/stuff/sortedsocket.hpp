//===========================================================================//
// File:	srtskt.hh                                                        //
// Contents: Interface definition for sorted socket class                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"safesocket.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ SortedSocket ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SortedSocket:
		public SafeSocket
	{
	public:
		~SortedSocket();

		void
			AddValuePlug(
				Plug *plug,
				const void *value
			)
         	{AddValueImplementation(plug, value);}

		Plug*
			FindPlug(const void *value)
				{return FindImplementation(value);}

	protected:
		explicit SortedSocket(
			bool has_unique_entries = true
		);
		SortedSocket(
			Node *node,
			bool has_unique_entries = true
		);

		bool
			HasUniqueEntries()
				{return hasUniqueEntries;}

		virtual void
			AddValueImplementation(
				Plug *plug,
				const void *value
			);

		virtual Plug*
			FindImplementation(const void *value);

	private:
		bool
      	hasUniqueEntries;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ SortedIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SortedIterator:
		public SafeIterator
	{
	public:
		~SortedIterator();

		virtual Plug*
			FindImplementation(const void *value);

		virtual void*
			GetValueImplementation();

	protected:
		explicit SortedIterator(SortedSocket *sortedSocket);
	};

}
