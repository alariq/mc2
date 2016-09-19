//===========================================================================//
// File:	sfeskt.hh                                                        //
// Contents: Interface definition for safe socket class                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"socket.hpp"

namespace Stuff {
	
	class SafeIterator;
	
	typedef int IteratorMemo;

	enum
	{
		PlugAdded = 0,
		PlugRemoved,
		NextSafeSocketMemo
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeSocket ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SafeSocket:
		public Socket
	{
		friend class SafeIterator;

	public:
		~SafeSocket();
		void
			TestInstance();

	protected:
		explicit SafeSocket(Node *node);

		void
			SendIteratorMemo(
				IteratorMemo memo,
				void *content
			);

	private:
		SafeIterator *iteratorHead;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SafeIterator:
		public SocketIterator
	{
		friend class SafeSocket;

	public:
		~SafeIterator();
		void
			TestInstance() const;

	protected:
		explicit SafeIterator(SafeSocket *safeSocket);

	private:
		virtual void
			ReceiveMemo(
				IteratorMemo memo,
				void *content
			);

		SafeIterator *nextIterator;
		SafeIterator *prevIterator;
	};

}
