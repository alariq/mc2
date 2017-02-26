//========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//========================================================================//

#pragma once

#include"stuff.hpp"
#include<toolos.hpp>

namespace Stuff {

	#if defined(TRACE_ENABLED) 

		//#######################################################################
		//###################    TraceSample    #################################
		//#######################################################################
	
		#pragma pack( push, trace_pack, 4 )
		struct TraceSample
		{
			int
				sampleLength;
			Time
				sampleTime;
			BYTE
				traceNumber,
				sampleType;

			enum Type {
				GoingUp = 0,
				GoingDown,
				Marker,
				IntegerSnapshot,
				ScalarSnapshot,
				SuspendSampling,
				ResumeSampling,
				QueueingEvent,
				DispatchingEvent
			};
			void
				TestInstance() {}
		};

		template <class T> struct SnapshotOf:
			public TraceSample
		{
			T snapShot;
		};

		#pragma pack( pop, trace_pack )

		//#######################################################################
		//#########################    Trace    #################################
		//#######################################################################

		class 
#ifdef PLATFORM_WINDOWS
            __declspec(novtable) 
#endif
            Trace:
			public Plug
		{
			friend class TraceManager;

		public:
			enum Type {
				BitType,
				IntegerType,
				ScalarType,
				EventType
			};

		protected:
			static BYTE NextTraceID;

			const char
				*traceName;
			Time
				lastActivity;
			BYTE
				traceNumber,
				traceType;

			Trace(
				const char* name,
				Type type
			);

			MemoryStream*
				GetTraceLog();
			void
				IncrementSampleCount();

			virtual void
				DumpTraceStatus()=0;
			virtual void
				ResetTrace()=0;

			#if defined(USE_TIME_ANALYSIS)
				virtual void
					StartTiming()=0;
				virtual Scalar
					CalculateUsage(
						Time when,
						Time sample_time
					)=0;
				virtual void
					PrintUsage(Scalar usage);
			#endif
		};

		//#######################################################################
		//########################    BitTrace    ###############################
		//#######################################################################

		class BitTrace:
			public Trace
		{
		protected:
			static BYTE
				NextActiveLine;

			int
				traceUp;
			Time
				lastUpTime,
				totalUpTime;
			BYTE
				activeLine;
			DWORD
				bitFlag;
			static int
				NextBit;

			void
				DumpTraceStatus();
			void
				ResetTrace();
			#if defined(USE_TIME_ANALYSIS)
				void
					StartTiming();
				Scalar
					CalculateUsage(
						Time when,
						Time sample_time
					);
				void
					PrintUsage(Scalar usage);
			#endif

		public:
			BitTrace(const char* name);

			void
				Set();
			void
				Clear();
			bool
				IsTraceOn()
					{Check_Object(this); return traceUp>0;}

			Time
				GetLastUpTime()
					{Check_Object(this); return lastUpTime;}
			Time
				GetTotalUpTime()
					{Check_Object(this); return totalUpTime;}

			void
				TestInstance();
		};

		//#######################################################################
		//########################    TraceOf    ################################
		//#######################################################################

		template <class T> class TraceOf:
			public Trace
		{
		protected:
			Time
				weightedSum;
			T
				currentValue;

			TraceSample::Type
				sampleType;

			void
				DumpTraceStatus();
			void
				ResetTrace();
			#if defined(USE_TIME_ANALYSIS)
				void
					StartTiming();
				Scalar
					CalculateUsage(
						Time when,
						Time sample_time
					);
				void
					PrintUsage(Scalar usage);
			#endif

		public:
			TraceOf(
				const char* name,
				const T& initial_value,
				Type trace_type,
				TraceSample::Type sample_type
			);

			void
				TakeSnapshot(const T& value);
		};

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		template <class T> TraceOf<T>::TraceOf(
				const char* name,
				const T& initial_value,
				Type trace_type,
				TraceSample::Type sample_type
			):
				Trace(name, trace_type)
			{
				currentValue = initial_value;
				sampleType = sample_type;
				TraceOf<T>::ResetTrace();
			}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		template <class T> void
			TraceOf<T>::DumpTraceStatus()
		{
			Check_Object(this);
			Spew(GROUP_STUFF_TRACE, currentValue);
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		template <class T> void
			TraceOf<T>::ResetTrace()
		{
			#if defined(USE_TIME_ANALYSIS)
				weightedSum = 0.0;
			#endif
		}

		#if defined(USE_TIME_ANALYSIS)

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//
			template <class T> void
				TraceOf<T>::StartTiming()
			{
				Check_Object(this);
				weightedSum = 0.0;
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//
			template <class T> Scalar
				TraceOf<T>::CalculateUsage(
					Time when,
					Time sample_time
				)
			{
				Time last_part = when - lastActivity;
				weightedSum += last_part * currentValue;
				Scalar result = static_cast<Scalar>(weightedSum / sample_time);
				weightedSum = 0.0;
				return result;
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//
			template <class T> void
				TraceOf<T>::PrintUsage(Scalar usage)
			{
				Check_Object(this);
				Spew(GROUP_STUFF_TRACE, usage);
			}

		#endif

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		template <class T> void
			TraceOf<T>::TakeSnapshot(const T& value)
		{
			Check_Object(this);

			#if defined(USE_TIME_ANALYSIS) || defined(USE_TRACE_LOG)
				Time now = gos_GetHiResTime();
			#endif

			#if defined(USE_TIME_ANALYSIS)
				Time last_part = now - lastActivity;
				weightedSum += last_part * currentValue;
				lastActivity = now;
			#endif

			currentValue = value;

			#if defined(USE_TRACE_LOG)
				IncrementSampleCount();
				MemoryStream *log = GetTraceLog();
				if (log)
				{
					Check_Object(log);
					SnapshotOf<T> *sample =
						Cast_Pointer(SnapshotOf<T>*, log->GetPointer());
					sample->sampleLength = sizeof(*sample);
					sample->sampleType = (BYTE)sampleType;
					sample->traceNumber = traceNumber;
					sample->sampleTime = now;
					sample->snapShot = currentValue;
					log->AdvancePointer(sample->sampleLength);
				}
			#endif
		}

		//#######################################################################
		//#####################    TraceManager    ##############################
		//#######################################################################

		class TraceManager
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
		{
		public:
			static void
				InitializeClass();
			static void
				TerminateClass();

			friend class Trace;
			friend class BitTrace;

		protected:
			ChainOf<Trace*> traceChain;
			Time sampleStart;
			int
				actualSampleCount,
				ignoredSampleCount;
			MemoryStream
				*allocatedTraceLog,
				*activeTraceLog;
			BYTE traceCount;
			DWORD
				activeBits;

			void
				Add(Trace *trace);

		public:
			TraceManager();
			~TraceManager();

			static TraceManager
				*Instance;

			BYTE
				GetTraceCount()
					{Check_Object(this); return traceCount;}
			void
				DumpTracesStatus();
			void
				ResetTraces();

			DWORD
				GetBitTraceStatus()
					{Check_Object(this); return activeBits;}
			const char*
				GetNameOfTrace(int bit_no);

			#if defined(USE_TIME_ANALYSIS)
				void
					StartTimingAnalysis();
				int
					SnapshotTimingAnalysis(bool print=false);
			#endif

			#if defined(USE_TRACE_LOG)
				void
					CreateTraceLog(
						size_t max_trace_samples,
                  bool start_sampling
					);
				void
					SaveTraceLog(const char* filename);
				void
					MarkTraceLog();
				void
					SuspendTraceLogging();
				void
					ResumeTraceLogging();
			#endif

			#if defined(USE_ACTIVE_PROFILE)
				virtual void
					SetLineImplementation(BYTE line);
				virtual void
					ClearLineImplementation(BYTE line);
				virtual bool
					IsLineValidImplementation(BYTE line);
			#endif

			void
				TestInstance()
					{}
		};

		inline MemoryStream*
			Trace::GetTraceLog()
		{
			Check_Object(this); Check_Object(TraceManager::Instance);
			return TraceManager::Instance->activeTraceLog;
		}

		inline void
			Trace::IncrementSampleCount()
		{
			Check_Object(this); Check_Object(TraceManager::Instance);
			if (!TraceManager::Instance->activeTraceLog)
			{
				++TraceManager::Instance->ignoredSampleCount;
			}
			else
			{
				++TraceManager::Instance->actualSampleCount;
			}
		}

	#endif

}
