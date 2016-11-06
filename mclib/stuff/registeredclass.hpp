//===========================================================================//
// File: vdata.hh                                                            //
// Contents: Interface specifications for virtual data                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"

namespace Stuff {

	class MString;
	class RegisteredClass__ClassData;
	class MemoryStream;

	//##########################################################################
	//#######################    RegisteredClass    ############################
	//##########################################################################

	class RegisteredClass
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
		friend class RegisteredClass__ClassData;

	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// ClassID enumeration
	//
	public:
		typedef int ClassID;
		typedef RegisteredClass__ClassData ClassData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Construction, destruction, saving
	//
	public:
		virtual
			~RegisteredClass()
				{}

	protected:
		explicit RegisteredClass(ClassData *class_data);

	private:
		RegisteredClass(const RegisteredClass &);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
	public:
		ClassData*
			GetClassData() const
				{Check_Object(this); return classData;}
		bool
			IsDerivedFrom(ClassData* parent) const;
		bool
			IsDerivedFrom(ClassID class_id) const;
		bool
			IsDerivedFrom(const char* parent) const;

		static ClassData*
			FindClassData(const char* name);
		static ClassData*
			FindClassData(ClassID class_id)
				{
					Verify(static_cast<unsigned>(class_id) < ClassIDCount);
					return ClassDataArray[class_id];
				}
		static ClassData
			*DefaultData;

	protected:
		ClassData
			*classData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Accessors
	//
	public:
		ClassID
			GetClassID() const;
		const char*
			GetClassString() const;
		static ClassID
			AllocateTemporaryClassID()
				{
					Verify(FirstTemporaryClassID < ClassIDCount);
					return FirstTemporaryClassID++;
				}

	private:
		static ClassData*
			ClassDataArray[ClassIDCount];
		static ClassID
			FirstTemporaryClassID;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Test support
	//
	public:
		void
			TestInstance() const;
	};

	//##########################################################################
	//#####################    Receiver::ClassData    ##########################
	//##########################################################################

	class RegisteredClass__ClassData
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
		friend class RegisteredClass;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Construction, destruction, testing
	//
	public:
		RegisteredClass__ClassData(
			RegisteredClass::ClassID class_id,
			const char *class_name,
			RegisteredClass__ClassData *parent = NULL
		);

		~RegisteredClass__ClassData();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Inheritance stuff
	//
	public:
		bool
			IsDerivedFrom(RegisteredClass__ClassData* parent);

		RegisteredClass::ClassID
			GetClassID()
				{Check_Object(this); return classID;}
		const char*
			GetClassName()
				{Check_Object(this); return className;}
		RegisteredClass__ClassData*
			GetParentClass()
				{Check_Object(this); return parentClass;}

	protected:
		void
			DeriveClass(RegisteredClass__ClassData* child);

		RegisteredClass__ClassData*
			FindClassData(const char* name);

		RegisteredClass::ClassID
			classID;
		const char
			*className;

		RegisteredClass__ClassData
			*firstChildClass,
			*nextSiblingClass,
			*parentClass;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Test support
	//
	public:
		void
      	TestInstance();
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	inline bool
		RegisteredClass::IsDerivedFrom(RegisteredClass__ClassData* parent) const
	{
		Check_Object(this);
		Check_Object(classData);
		return classData->IsDerivedFrom(parent);
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	inline bool
		RegisteredClass::IsDerivedFrom(ClassID class_id) const
	{
		Check_Object(this);
		Check_Object(classData);
		return classData->IsDerivedFrom(FindClassData(class_id));
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	inline bool
		RegisteredClass::IsDerivedFrom(const char* parent) const
	{
		Check_Object(this);
		Check_Object(classData);
		return classData->IsDerivedFrom(FindClassData(parent));
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	inline RegisteredClass::ClassID
		RegisteredClass::GetClassID() const
	{
		Check_Object(this);
		RegisteredClass__ClassData *data = GetClassData();
		Check_Object(data);
		return data->GetClassID();
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	inline RegisteredClass::ClassData*
		RegisteredClass::FindClassData(const char* name)
	{
		return DefaultData->FindClassData(name);
	}

}
