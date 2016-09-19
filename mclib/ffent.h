/*==========================================================================
 *
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//---------------------------------------------------------------------------//
 *  File:       ffent.h
 *  Content:	FastFile entry definition (internal)
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTBILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 ***************************************************************************/

#pragma pack(1)
typedef struct {
    long	offset;
    char	name[250];
} FILEENTRY, *LPFILEENTRY;
#pragma pack()
