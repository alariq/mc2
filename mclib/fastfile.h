//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//----------------------------------------------------------------------------
// Global Fast File Header. 
//

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef FFILE_H
#include"ffile.h"
#endif

//-----------------------------------------------------------------------------------
// MechCommander's File classes understand how to deal with the FastFile, once open.
// In other words, the global class needs only to register newly opened fastfiles,
// close them all on exit and provide a find function which returns a pointer to the
// fastfile containing the file we care about.  If no file is found, returns NULL.
extern bool FastFileInit (const char *fname);
extern void FastFileFini (void);
extern FastFile *FastFileFind (const char *fname, long &fastFileHandle);
extern DWORD elfHash (const char *name);
//-----------------------------------------------------------------------------------


