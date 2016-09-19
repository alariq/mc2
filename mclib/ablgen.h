//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								GENERAL.H
//
//***************************************************************************

#ifndef ABLGEN_H
#define	ABLGEN_H

//***************************************************************************

//---------------------------------------------------------------------
// Set this, once we have the actual executor modules up and running...
#define	USE_COMPOUND_STATEMENTS		0
#define	ANALYZE_ON					0
#define	ORDERS_ON					1

#define	CHAR_FORMFEED				'\f'
#define	CHAR_EOF					'\x7f'

#define MAX_INCLUDE_DEPTH			6
#define	MAXLEN_FILENAME				256
#define	MAXLEN_SOURCELINE			2048
#define	MAXLEN_PRINTLINE			80
#define	MAXLEN_TOKENSTRING			MAXLEN_SOURCELINE
#define	MAX_LINES_PER_PAGE			50
#define	MAXLEN_STRING_CONSTANT		1024
#define	MAX_NESTING_LEVEL			3

#define	LEN_DATESTRING				26

#define	MAXSIZE_CODE_BUFFER			20480

#define	MAXSIZE_STACK				10240
#define	STACK_FRAME_HEADER_SIZE		4

#define	SYSTEM_HEAP_SIZE			1024 * 1024		// General system memory...

// Masher Commands available...
#define	COMMAND_HELP_SHORT			0
#define	COMMAND_WAIT_LONG			1
#define	COMMAND_WAIT_SHORT			2
#define	COMMAND_RANDOM_LONG			3
#define	COMMAND_RANDOM_SHORT		4
#define	COMMAND_NUM_RUNS_LONG		5
#define	COMMAND_NUM_RUNS_SHORT		6

#define	ABL_NO_ERR					0

#define	MAX_ORDERS					65535

//***************************************************************************

typedef char*	Address;

//***************************************************************************

#endif

