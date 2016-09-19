//===========================================================================//
// File:	analysis.cpp                                                     //
// Contents: Utilities for using logic analyzer with parallel port dongle    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

#if defined(TRACE_ENABLED)

	//##########################################################################
	//############################    Trace    #################################
	//##########################################################################

	BYTE
		Trace::NextTraceID = 0;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	Trace::Trace(
		const char* name,
		Type type 
	):
		Plug(DefaultData)
	{
		traceNumber = NextTraceID++;
		traceType = (BYTE)type;
		traceName = name;
		
		lastActivity = 0.0;

		Check_Object(TraceManager::Instance);
		TraceManager::Instance->Add(this);
	}

	#if defined(USE_TIME_ANALYSIS)
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			Trace::PrintUsage(Scalar usage)
		{
			Check_Object(this);

			SPEW((GROUP_STUFF_TRACE, "%f+", usage));
		}
	#endif

	//##########################################################################
	//###########################    BitTrace    ###############################
	//##########################################################################

	BYTE
		BitTrace::NextActiveLine = 0;
	int
		BitTrace::NextBit = 0;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	BitTrace::BitTrace(const char* name):
		Trace(name, BitType)
	{
		activeLine = NextActiveLine++;
		bitFlag = (NextBit < 32) ? 1 << NextBit++ : 0;
		#if defined(USE_ACTIVE_PROFILE)
			DEBUG_STREAM << name << " used trace line "
			 << static_cast<int>(activeLine) << "!\n";
			if (!IsLineValidImplementation(activeLine))
			{
				STOP(("Invalid active trace line!"));
			}
		#endif

		BitTrace::ResetTrace();
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		BitTrace::DumpTraceStatus()
	{
		Check_Object(this);
		SPEW((
			GROUP_STUFF_TRACE,
			"%d = %d+",
			static_cast<int>(activeLine),
			traceUp
		));
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		BitTrace::ResetTrace()
	{
		#if defined(USE_TIME_ANALYSIS)
			traceUp = 0;
			lastUpTime = 0.0;
			totalUpTime = 0.0;
			TraceManager::Instance->activeBits &= ~bitFlag;
		#endif

		#if defined(USE_ACTIVE_PROFILE)
			ClearLineImplementation(activeLine);
		#endif
	}

	#if defined(USE_TIME_ANALYSIS)

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			BitTrace::StartTiming()
		{
			Check_Object(this);
			totalUpTime = 0.0;
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		Scalar
			BitTrace::CalculateUsage(
				Time when,
				Time sample_time
			)
		{
			if (traceUp > 0)
			{
				totalUpTime += when - lastActivity;
			}
			Scalar result = static_cast<Scalar>(totalUpTime / sample_time);
			SPEW((GROUP_STUFF_TRACE, "%4fs, +", totalUpTime));
			return result;
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			BitTrace::PrintUsage(Scalar usage)
		{
			Check_Object(this);

			SPEW((GROUP_STUFF_TRACE, "%4f%% CPU+", (usage*100.0f)));
			#if defined(USE_ACTIVE_PROFILE)
			SPEW((GROUP_STUFF_TRACE, " (active on line %d)", static_cast<int>(activeLine)));
			#endif
		}

	#endif

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		BitTrace::Set()
	{
		Check_Object(this);

		if (!traceUp++)
		{
			#if defined(USE_ACTIVE_PROFILE)
				SetLineImplementation(activeLine);
			#endif

			TraceManager::Instance->activeBits |= bitFlag;

			#if defined(USE_TIME_ANALYSIS) || defined(USE_TRACE_LOG)
				Time now = gos_GetHiResTime();
			#endif

			#if defined(USE_TIME_ANALYSIS)
				lastActivity = now;
			#endif

			#if defined(USE_TRACE_LOG)
				// Check_Object(traceManager);
				IncrementSampleCount();
				MemoryStream *log = GetTraceLog();
				if (log)
				{
					Check_Object(log);
					TraceSample *sample =
						Cast_Pointer(TraceSample*, log->GetPointer());
					sample->sampleLength = sizeof(*sample);
					sample->sampleType = TraceSample::GoingUp;
					sample->traceNumber = traceNumber;
					sample->sampleTime = now;
					log->AdvancePointer(sample->sampleLength);
				}
			#endif
		}
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		BitTrace::Clear()
	{
		Check_Object(this);

		if (--traceUp == 0)
		{
			TraceManager::Instance->activeBits &= ~bitFlag;

			#if defined(USE_TIME_ANALYSIS) || defined(USE_TRACE_LOG)
				Time now = gos_GetHiResTime();
			#endif

			#if defined(USE_TIME_ANALYSIS)
				lastUpTime = now - lastActivity;
#if 0 // HACK	
				Verify(lastUpTime >= 0.0f)
				totalUpTime += lastUpTime;
#else
				if (lastUpTime >= 0.0f)
				{
					totalUpTime += lastUpTime;
				}
#endif
			#endif

			#if defined(USE_TRACE_LOG)
				//Check_Object(traceManager);
				IncrementSampleCount();
				MemoryStream *log = GetTraceLog();
				if (log)
				{
					Check_Object(log);
					TraceSample *sample =
						Cast_Pointer(TraceSample*, log->GetPointer());
					sample->sampleLength = sizeof(*sample);
					sample->sampleType = TraceSample::GoingDown;
					sample->traceNumber = traceNumber;
					sample->sampleTime = now;
					log->AdvancePointer(sample->sampleLength);
				}
			#endif

			#if defined(USE_ACTIVE_PROFILE)
				ClearLineImplementation(activeLine);
			#endif
		}
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		BitTrace::TestInstance()
	{
		Verify(traceUp >= 0);
	}

	//##########################################################################
	//########################    TraceManager    ##############################
	//##########################################################################

	TraceManager
		*TraceManager::Instance = NULL;

	void
		TraceManager::InitializeClass()
	{
		Verify(!TraceManager::Instance);
		TraceManager::Instance = new TraceManager;
		Register_Object(TraceManager::Instance);
	}

	void
		TraceManager::TerminateClass()
	{
		Unregister_Object(TraceManager::Instance);
		delete TraceManager::Instance;
		TraceManager::Instance = NULL;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	TraceManager::TraceManager():
		traceChain(NULL)
	{
		sampleStart = 0.0;
		actualSampleCount = 0;
		ignoredSampleCount = 0;
		traceCount = 0;
		activeTraceLog = NULL;
		allocatedTraceLog = NULL;
		activeBits = 0;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	TraceManager::~TraceManager()
	{
		Check_Object(this);

		if (allocatedTraceLog)
		{
			Check_Object(allocatedTraceLog);
			allocatedTraceLog->Rewind();
			TraceSample *samples =
				Cast_Pointer(TraceSample*, allocatedTraceLog->GetPointer());
			Unregister_Object(allocatedTraceLog);
			delete allocatedTraceLog;
			activeTraceLog = allocatedTraceLog = NULL;

			Unregister_Pointer(samples);
			delete[] samples;
		}
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		TraceManager::Add(Trace *trace)
	{
		Check_Object(this);

		traceCount = (BYTE)(traceCount + 1);
		traceChain.Add(trace);
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		TraceManager::DumpTracesStatus()
	{
		ChainIteratorOf<Trace*> traces(&traceChain);
		Trace *trace;

		while ((trace = traces.ReadAndNext()) != NULL)
		{
			Check_Object(trace);
			SPEW((GROUP_STUFF_TRACE, "%s: +", trace->traceName));
			trace->DumpTraceStatus();
			SPEW((GROUP_STUFF_TRACE, ""));
		}
		SPEW((GROUP_STUFF_TRACE, ""));
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	void
		TraceManager::ResetTraces()
	{
		ChainIteratorOf<Trace*> traces(&traceChain);
		Trace *trace;

		while ((trace = traces.ReadAndNext()) != NULL)
		{
			Check_Object(trace);
			trace->ResetTrace();
		}

		#if defined(USE_TRACE_LOG)
			actualSampleCount = 0;
			ignoredSampleCount = 0;
			if (allocatedTraceLog)
			{
				allocatedTraceLog->Rewind();
			}
		#endif
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	const char*
		TraceManager::GetNameOfTrace(int bit_no)
	{
		//
		// Set up the iterator
		//
		ChainIteratorOf<Trace*> traces(&traceChain);
		Trace *trace;
		while ((trace = traces.ReadAndNext()) != NULL)
		{
			Check_Object(trace);
			if (trace->traceType == Trace::BitType)
			{
				if (!bit_no)
				{
					break;
				}
				--bit_no;
			}
		}
		Check_Object(trace);
		return trace->traceName;
	}

	#if defined(USE_TIME_ANALYSIS)

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			TraceManager::StartTimingAnalysis()
		{
			sampleStart = gos_GetHiResTime();

			ChainIteratorOf<Trace*> traces(&traceChain);
			Trace *trace;

			while ((trace = traces.ReadAndNext()) != NULL)
			{
				Check_Object(trace);
				trace->StartTiming();
				trace->lastActivity = sampleStart;
			}

			#if defined(USE_TRACE_LOG)
				actualSampleCount = 0;
				ignoredSampleCount = 0;
				if (allocatedTraceLog)
				{
					allocatedTraceLog->Rewind();
				}
			#endif
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		int
			TraceManager::SnapshotTimingAnalysis(bool print)
		{
			Time now = gos_GetHiResTime();
			Time time = now - sampleStart;
			if (time < SMALL)
			{
				return false;
			}

			ChainIteratorOf<Trace*> traces(&traceChain);
			Trace *trace;

			if (print)
			{
				SPEW((GROUP_STUFF_TRACE, "TIMING ANALYSIS"));
				SPEW((GROUP_STUFF_TRACE, "Sample length: %4fs", time));
			}
			while ((trace = traces.ReadAndNext()) != NULL)
			{
				Check_Object(trace);

				if (print)
				{
					SPEW((GROUP_STUFF_TRACE, "%s: +", trace->traceName));
					Scalar usage = trace->CalculateUsage(now, time);
					trace->PrintUsage(usage);
					SPEW((GROUP_STUFF_TRACE, ""));
				}

				trace->StartTiming();
				trace->lastActivity = now;
			}
			SPEW((GROUP_STUFF_TRACE, ""));

			sampleStart = now;
			return true;
		}
	#endif

	#if defined(USE_TRACE_LOG)

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			TraceManager::CreateTraceLog(
				size_t max_trace_count,
				bool start_logging
			)
		{
			Check_Object(this);
			Verify(!allocatedTraceLog);
			TraceSample *samples = new TraceSample[max_trace_count];
			Register_Pointer(samples);
			allocatedTraceLog =
				new MemoryStream(samples, max_trace_count*sizeof(TraceSample));
			Register_Object(allocatedTraceLog);
			if (start_logging)
			{
				activeTraceLog = allocatedTraceLog;
			}
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			TraceManager::SaveTraceLog(const char* filename)
		{
			Check_Object(this);
			if (allocatedTraceLog)
			{
				Check_Object(allocatedTraceLog);

				//
				//--------------------------------------------------------
				// Rewind the memory stream and save it out in a disk file
				//--------------------------------------------------------
				//
				FileStream output(filename, FileStream::WriteOnly);
				size_t size = allocatedTraceLog->GetBytesUsed();
				if (size > 0)
				{
					BYTE trace_count = GetTraceCount();

					//
					//----------------------------
					// Write out the record header
					//----------------------------
					//
					output << static_cast<int>('RVNO') << sizeof(int) << 2;

					//
					//---------------------------------------------------------
					// Write out the header section after figuring out its size
					//---------------------------------------------------------
					//
					ChainIteratorOf<Trace*> traces(&traceChain);
					Trace *trace;
					int header_size = sizeof(int);
					while ((trace = traces.ReadAndNext()) != NULL)
					{
						header_size += 2*sizeof(int);
						header_size += (strlen(trace->traceName)+4)&~3;
					}
					output << static_cast<int>('HDRS') << header_size;
					output << static_cast<int>(trace_count);

					traces.First();
					while ((trace = traces.ReadAndNext()) != NULL)
					{
						int str_len = strlen(trace->traceName) + 1;
						header_size = sizeof(int) + ((str_len+4)&~3);
						output << header_size << static_cast<int>(trace->traceType);
						output.WriteBytes(trace->traceName, str_len);
						while (str_len&3)
						{
							output << '\0';
							++str_len;
						}
					}

					output << static_cast<int>('LOGS') << ((size+3)&~3);
					allocatedTraceLog->Rewind();
					output.WriteBytes(allocatedTraceLog->GetPointer(), size);
					while (size&3)
					{
						output << '\0';
						++size;
					}
				}

				//
				//-------------------
				// Release the memory
				//-------------------
				//
				output.Close();

				TraceSample *samples =
					Cast_Pointer(TraceSample*, allocatedTraceLog->GetPointer());
				Unregister_Object(allocatedTraceLog);
				delete allocatedTraceLog;
				activeTraceLog = allocatedTraceLog = NULL;

				Unregister_Pointer(samples);
				delete[] samples;
			}
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			TraceManager::MarkTraceLog()
		{
			Check_Object(this);

			if (activeTraceLog)
			{
				Check_Object(activeTraceLog);

				Time now = gos_GetHiResTime();

				TraceSample *sample =
					Cast_Pointer(TraceSample*, activeTraceLog->GetPointer());
				sample->sampleLength = sizeof(*sample);
				sample->sampleType = TraceSample::Marker;
				sample->sampleTime = now;
				sample->traceNumber = 0;
				activeTraceLog->AdvancePointer(sample->sampleLength);
			}
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			TraceManager::SuspendTraceLogging()
		{
			Check_Object(this);

			if (activeTraceLog)
			{
				Check_Object(activeTraceLog);

				Time now = gos_GetHiResTime();

				TraceSample *sample =
					Cast_Pointer(TraceSample*, activeTraceLog->GetPointer());
				sample->sampleLength = sizeof(*sample);
				sample->sampleType = TraceSample::SuspendSampling;
				sample->sampleTime = now;
				sample->traceNumber = 0;
				activeTraceLog->AdvancePointer(sample->sampleLength);
				activeTraceLog = NULL;
			}
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			TraceManager::ResumeTraceLogging()
		{
			Check_Object(this);

			if (allocatedTraceLog && !activeTraceLog)
			{
				Check_Object(allocatedTraceLog);
				activeTraceLog = allocatedTraceLog;

				Time now = gos_GetHiResTime();

				TraceSample *sample =
					Cast_Pointer(TraceSample*, activeTraceLog->GetPointer());
				sample->sampleLength = sizeof(*sample);
				sample->sampleType = TraceSample::ResumeSampling;
				sample->sampleTime = now;
				sample->traceNumber = 0;
				activeTraceLog->AdvancePointer(sample->sampleLength);
			}
		}

#if 0
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//
		void
			TraceManager::WriteClassBlocks(MemoryStream& stream)
		{
			stream << static_cast<int>('MSGS')
			 << RegisteredClass::DefaultData->WriteClassBlock(stream, false);
			RegisteredClass::DefaultData->WriteClassBlock(stream, true);
		}
#endif

	#endif

	#if defined(USE_ACTIVE_PROFILE)

		void
			TraceManager::SetLineImplementation(BYTE)
		{
		}

		void
			TraceManager::ClearLineImplementation(BYTE)
		{
		}

		bool
			TraceManager::IsLineValidImplementation(BYTE)
		{
			return true;
		}

	#endif

#endif

