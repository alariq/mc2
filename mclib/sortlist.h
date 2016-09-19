//***************************************************************************
//
//	Sortlist.h -- Sort List defines
//
//	MechCommander II
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef SORTLIST_H
#define SORTLIST_H

//***************************************************************************

//--------------
// Include Files
#ifndef DSTD_H
#include"dstd.h"
#endif

//--------------------------------
// Structure and Class Definitions

typedef struct _SortListNode {
	float			value;			// sort value
	unsigned long	id;				// item 
} SortListNode;

class SortList {

	protected:

		SortListNode*		list;
		long				numItems;

	public:

		void init (void) {
			list = NULL;
			numItems = 0;
		}

		SortList (void) {
			init();
		}

		long init (long numItems);

		void setId (long index, long id) {
			if ((index >= 0) && (index < numItems))
				list[index].id = id;
		}

		void setValue (long index, float value) {
			if ((index >= 0) && (index < numItems))
				list[index].value = value;
		}

		long getId (long index) {
			return(list[index].id);
		}

		float getValue (long index) {
			return(list[index].value);
		}

		void clear (bool setToMin = true);

		long getNumItems (void) {
			return(numItems);
		}

		void sort (bool descendingOrder = true);

		void destroy (void);

		~SortList (void) {
			destroy();
		}
};

typedef SortList* SortListPtr;

//***************************************************************************

#endif
