//===========================================================================//
// File:		debug3on.hpp                                                 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#undef Verify
#undef Warn
#undef Check_Pointer
#undef Check_Signature 
#undef Check
#undef Cast_Pointer
#undef Cast_Object
#undef Mem_Copy
#undef Str_Copy
#undef Str_Cat
#undef Spew

#undef USE_TIME_ANALYSIS			// trace time statistics
#undef USE_TRACE_LOG					// logging functions
#undef USE_EVENT_STATISTICS      // event statistics

#if !defined(NO_ANALYSIS)
	#define USE_TIME_ANALYSIS
	#define USE_TRACE_LOG
	#define USE_EVENT_STATISTICS
#endif

#define Verify(c)\
	do {if (Stuff::ArmorLevel>0 && !(c)) PAUSE(("Failed " #c));} while(0)

#define Warn(c)\
	do {\
		if (Stuff::ArmorLevel>0 && (c)) SPEW((0, #c));\
	} while(0)

#define Check_Pointer(p) Verify((p) && reinterpret_cast<size_t>(p)!=Stuff::SNAN_NEGATIVE_LONG)

template <class T> T
	Cast_Pointer_Function(T p)
{
	if (ArmorLevel>0)
		Check_Pointer(p);
	return p;
}

#define Cast_Pointer(type, ptr) Stuff::Cast_Pointer_Function(reinterpret_cast<type>(ptr))

#define Mem_Copy(destination, source, length, available)\
	do {\
		Check_Pointer(destination);\
		Check_Pointer(source);\
		Verify((length) <= (available));\
		Verify(\
			abs(\
				reinterpret_cast<char*>(destination)\
				 - reinterpret_cast<const char*>(source)\
			) >= length\
		);\
		memcpy(destination, source, length);\
	} while (0)

#define Str_Copy(destination, source, available)\
	do {\
		Check_Pointer(destination);\
		Check_Pointer(source);\
		Verify((strlen(source) + 1) <= (available));\
		Verify(abs(destination - source) >= (strlen(source) + 1));\
		strcpy(destination, source);\
	} while (0)

#define Str_Cat(destination, source, available)\
	do {\
		Check_Pointer(destination);\
		Check_Pointer(source);\
		Verify((strlen(destination) + strlen(source) + 1) <= (available));\
		strcat(destination, source);\
	} while (0)

#define Check_Signature(p) Stuff::Is_Signature_Bad(p)

template <class T> void
	Check_Object_Function(T *p)
{
	switch (ArmorLevel)
	{
	case 1:
		Check_Pointer(p);
		break;

	case 2:
		Check_Signature(p);
		break;

	case 3:
	case 4:
		Check_Signature(p);
		p->TestInstance();
		break;
	}
}

#define Check_Object(p) Stuff::Check_Object_Function(p)

//
// Cast_Object will only work for polymorphic objects,
// non-polymorphic objects use Cast_Pointer
//

template <class T> T
	Cast_Object_Function(T p)
{
	switch (ArmorLevel)
	{
	case 1:
		Check_Pointer(p);
		break;

	case 2:
		Check_Signature(p);
		break;

	case 3:
		Check_Signature(p);
		p->TestInstance();
		break;

	case 4:
		Check_Signature(p);
		p->TestInstance();
		Verify(dynamic_cast<T>(p) != NULL);
		break;		
	}
	return p;
}

#define Cast_Object(type, ptr) Stuff::Cast_Object_Function(static_cast<type>(ptr))
