//---------------------------------------------------------------------------//
// scale.h - This file contains definitions of the scaleFactor for scaled draws
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef SCALE_H
#define SCALE_H
//---------------------------------------------------------------------------
// Include files

long scaleDraw (MemoryPtr shapeTable, unsigned long frameNum, long sx, long sy, bool reverse, MemoryPtr fadeTable = NULL, bool scaleUp = 0);
//---------------------------------------------------------------------------
#endif