//===========================================================================//
// File:	style.hh                                                         //
// Contents: Base information used by all MUNGA source files                 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"

// sebi: armoron,hpp needs it
namespace Stuff {
		inline void
			Is_Signature_Bad(const volatile void* p);
};


namespace Stuff {

	extern int ArmorLevel;

	enum {SNAN_NEGATIVE_INT32=0xffb1ffb1};

	#if !defined(Verify)
		#if !defined(_ARMOR)
#include"armoroff.hpp"
		#else

#include"armoron.hpp"
		#endif
	#endif

	#define Register_Pointer(p) Check_Pointer(p)
	#define Unregister_Pointer(p) Check_Pointer(p)
	#define Register_Object(p) Check_Object(p)
	#define Unregister_Object(p) Check_Object(p)

	#if defined(USE_TIME_ANALYSIS) || defined(USE_ACTIVE_PROFILE)\
	 || defined(USE_TRACE_LOG)
		#define TRACE_ENABLED
	#endif

	#if defined(TRACE_ENABLED) && !defined(LAB_ONLY)
		#undef TRACE_ENABLED
	#endif

	#define DECLARE_TIMER(scope, t)\
		scope __int64 t##FrameTime
	#define DEFINE_TIMER(c, t)\
		__int64 c::t##FrameTime

	#if defined(LAB_ONLY)
		#define Initialize_Timer(t, name)\
			do {\
				AddStatistic(name, "%", gos_timedata, &t##FrameTime, 0);\
			} while (0);
		#define Start_Timer(t)\
			do {\
				t##FrameTime -= GetCycles();\
			} while (0);
		#define Stop_Timer(t)\
			do {\
				t##FrameTime += GetCycles();\
			} while (0);
		#define Set_Statistic(s,v) (s=v)
	#else
		#define Initialize_Timer(t, name)
		#define Start_Timer(t)
		#define Stop_Timer(t)
		#define Set_Statistic(s,v)
	#endif

	void
		Flood_Memory_With_NAN(
			void *where,
			size_t how_much
		);

	#define Test_Assumption(c)\
		while (!(c)) {SPEW((GROUP_STUFF_TEST, #c" failed!")); return false;}

	#define Test_Message(m) SPEW((GROUP_STUFF_TEST, m));

	#if defined(_ARMOR)
		class MemoryRegister;

		class Signature
		{
			friend class MemoryRegister;

		private:
			enum Mark
			{
				Valid=0x7B135795L,
				Destroyed=0x4FED1231L
			} mark;

		protected:
			Signature();
            // because bjects inherit from it
			virtual ~Signature();

		public:
			friend void
				Is_Signature_Bad(const volatile Signature *p);
		};

		inline void
			Is_Signature_Bad(const volatile void* p)
				{Check_Pointer(p);}

	#endif

	//##########~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//# MACROS #
	//##########

	#define ELEMENTS(Array)	(sizeof(Array)/sizeof(*Array))

	#if defined(__sgi__)
		#define _stricmp(s1, s2)			(strcasecmp(s1, s2))
		#define _strnicmp(s1, s2, n)		(strncasecmp(s1, s2, n))
	#endif

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Spews
	//
	#if !defined(Spew)
		inline void
			Spew(
				const char* group,
				int value
			)
				{SPEW((group, "%d+", value));}

		inline void
			Spew(
				const char* group,
				float value
			)
				{SPEW((group, "%f+", value));}

		inline void
			Spew(
				const char* group,
				const char* value
			)
				{SPEW((group, "%s+", value));}
	#endif

	//#############~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//# TEMPLATES #
	//#############

	extern float SMALL;

	#define Abs(value) ((value>0) ? value : -value)
	#define Clamp(v,f,c) if (v<f) v=f; else if (v>c) v=c; else (void)0
	#define Max_Clamp(v,c) if (v>c) v=c; else (void)0
	#define Is_Many_Bits(value) (Lowbit(value)^value)
	#define Low_Bit(value) (value & (-value))
	#define Min_Clamp(v,f) if (v<f) v=f; else (void)0
	#define Max(a,b) ((a>b) ? a : b)
	#define Min(a,b) ((a<b) ? a : b)
	#define Sgn(value) ((value<-SMALL) ? -1 : value>SMALL)

	//############~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//# TYPEDEFS #
	//############

	enum {INT_BITS=32};

}

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void*
	operator new(size_t size, void* where)
		{Check_Pointer(where); return where;}
#endif
