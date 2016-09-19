//******************************************************************************************
//	color.cpp - This file contains the code for the Color Tables
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef COLOR_H
#include"color.h"
#endif

#ifndef PATHS_H
#include"paths.h"
#endif

#ifndef CIDENT_H
#include"cident.h"
#endif

#ifndef INIFILE_H
#include"inifile.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#include<gameos.hpp>
//----------------------------------------------------------------------------------
DWORD **colorRGBLookup = NULL;
long numColorRGBTables = 0;

#define MAX_COLOR_ENTRIES		56
//----------------------------------------------------------------------------------
void initColorTables (void)
{
	FullPathFileName colorPath;
	colorPath.init(cameraPath,"colors",".fit");

	FitIniFile colorFile;
	long result = colorFile.open(colorPath);
	gosASSERT(result == NO_ERR);

	result = colorFile.seekBlock("Main");
	gosASSERT(result == NO_ERR);

	result = colorFile.readIdLong("NumTables",numColorRGBTables);
	gosASSERT(result == NO_ERR);

	colorRGBLookup = (DWORD **)systemHeap->Malloc(sizeof(DWORD *) * numColorRGBTables);
	gosASSERT(colorRGBLookup != NULL);

	memset(colorRGBLookup,0,sizeof(DWORD *) * numColorRGBTables);

	for (long i=0;i<numColorRGBTables;i++)
	{
		char tableBlock[256];
		sprintf(tableBlock,"Table%d",i);

		result = colorFile.seekBlock(tableBlock);
		gosASSERT(result == NO_ERR);

		colorRGBLookup[i] = (DWORD *)systemHeap->Malloc(sizeof(DWORD) * MAX_COLOR_ENTRIES);
		gosASSERT(colorRGBLookup[i] != NULL);

		DWORD *table = colorRGBLookup[i];

		for (long j=0;j<MAX_COLOR_ENTRIES;j++)
		{
			char colorBlock[256];
			sprintf(colorBlock,"Color%d",j);

            // sebi
            long unsigned int tmp;
			result = colorFile.readIdULong(colorBlock, tmp);
            table[j] = (DWORD)tmp;
			gosASSERT(result == NO_ERR);
		}
	}

	colorFile.close();
}	

//----------------------------------------------------------------------------------
void destroyColorTables (void)
{
	for (long i=0;i<numColorRGBTables;i++)
	{
		systemHeap->Free(colorRGBLookup[i]);
		colorRGBLookup[i] = NULL;
	}

	if ( colorRGBLookup )
		systemHeap->Free(colorRGBLookup);
	colorRGBLookup = NULL;
}	

//----------------------------------------------------------------------------------
DWORD *getColorTable (long tableNum)
{
	if ((tableNum >= 0) && (tableNum < numColorRGBTables))
	{
		return(colorRGBLookup[tableNum]);
	}

	return(NULL);
}	

//----------------------------------------------------------------------------------
