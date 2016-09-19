//===========================================================================//
// File:	filestrm.tcp                                                     //
// Contents: Test stuff for MMIO class                                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MMIOstream ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
//#############################################################################
//

bool
	FileStream::TestClass()
{
	char buffer[65535];
	int i;
	Time total;
	Time s; 

	Check_Object(FileStreamManager::Instance);
	FileStream Testit;

	memset(buffer,'A',65535);

	SPEW((GROUP_STUFF_TEST, "Starting FileStream test..."));
	s = gos_GetHiResTime();
 	for(i=0; i < 1024; i++)
	{
		Testit.Open("filetest.tst",WriteOnly);
		Testit.WriteBytes(buffer,65535);
		Testit.Close();
		Testit.Open("filetest.tst",ReadOnly);
		Testit.ReadBytes(buffer,65535);
		Testit.Close();
	}
	total = gos_GetHiResTime() - s;
	for(i=0; i < 65535; i++) Verify(buffer[i] == 'A');
	SPEW((GROUP_STUFF_TEST, "Opening, writing, closing, opening, reading, and closing"));
	SPEW((
		GROUP_STUFF_TEST,
		"  64K file 1024 times, averaging out to %f ticks apiece...",
		total / 1024.0f
	));
	SPEW((GROUP_STUFF_TEST, "File data checks out, as well..."));
	SPEW((GROUP_STUFF_TEST, "Leaving FileStream test..."));

	return true;
};
