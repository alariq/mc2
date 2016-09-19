//===========================================================================//
// File:	filestrmmgr.hpp                                                  //
// Contents: Implementation Details of resource management                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"filestream.hpp"

namespace Stuff {

	//##########################################################################
	//#######################    FileDependencies    ###########################
	//##########################################################################

	class FileDependencies:
		public Stuff::Plug
	{
	public:
		FileDependencies();
		FileDependencies(const FileDependencies &dependencies):
			Plug(DefaultData)
				{
					Check_Object(this); Check_Object(&dependencies);
					*this = dependencies;
				}
		~FileDependencies();

		FileDependencies&
			operator=(const FileDependencies &dependencies);

		void
			AddDependency(FileStream *stream);
		void
			AddDependencies(const FileDependencies* dependencies);
		void
			AddDependencies(MemoryStream *dependencies);

		DynamicMemoryStream
			m_fileNameStream;
	};

	//##########################################################################
	//#######################    FileStreamManager    ##########################
	//##########################################################################

	class FileStreamManager
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor, destructor, and testing
	//
	public:
		FileStreamManager();
		~FileStreamManager();

		void
			TestInstance()
				{}
		static void
			TestClass();

		static FileStreamManager
			*Instance;
			
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Utility functions
	//
	public:
		void
			CleanUpAfterCompares();

		bool
			CompareModificationDate(
				const MString &file_name,
				__int64 time_stamp
			);
		void
			PurgeFileCompareCache();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Private data
	//
	protected:
		typedef Stuff::PlugOf<__int64>
			FileStatPlug;
		Stuff::TreeOf<FileStatPlug*, Stuff::MString>
			compareCache;
	};

}
