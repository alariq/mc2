//***************************************************************************
//
//	PQueue.h -- Prototype for priority queues
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef PQUEUE_H
#define PQUEUE_H

//***************************************************************************

//--------------
// Include Files

//--------------------------------
// Structure and Class Definitions

typedef struct _PQNode {
	long			key;			// sort value
	unsigned long	id;				// hash value for this map position
	long			row;			// HB-specific
	long			col;			// HB-specific
} PQNode;

class PriorityQueue {

	protected:

		PQNode*		pqList;
		long		maxItems;
		long		numItems;
		long		keyMin;

		void downHeap (long curIndex);

		void upHeap (long curIndex);

	public:

		void init (void) {
			pqList = NULL;
			numItems = 0;
		}

		PriorityQueue (void) {
			init();
		}

		long init (long maxItems, long keyMinValue = -2000000);

		long insert (PQNode& item);

		void remove (PQNode& item);

		void change (long itemIndex, long newValue);

		long find (long id);
		
		void clear (void) {
			numItems = 0;
		}

		long getNumItems (void) { return(numItems); }
		
		bool isEmpty (void) {
			return(numItems == 0);
		}
		
		PQNode* getItem (long itemIndex) {
			return(&pqList[itemIndex]);
		}

		void destroy (void);

		~PriorityQueue (void) {
			destroy();
		}
};

typedef PriorityQueue* PriorityQueuePtr;

//***************************************************************************

#endif
