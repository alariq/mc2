//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#if !defined(OWNTARCE_HPP)
	#define OWNTRACE_HPP

	#define Own_Trace 0

	class OwnTrace
	{
	public:
		OwnTrace(const char *);

		void Start();
		void Stop();

		void Print();

	private:
		MString name;
		int on, count;
		double sum;

		double start, end;
	};

	inline OwnTrace::OwnTrace(const char *n)
	{
		name = n;

		sum = 0;
		on = 0;
		count = 0;
	}

	inline void OwnTrace::Start()
	{
#if Own_Trace
		if(on==0)
		{
			count++;
			on = 1;
			start = System_Clock->Now();
		}
#endif
	}

	inline void OwnTrace::Stop()
	{
#if Own_Trace
		if(on==1)
		{
			on = 0;
			end = System_Clock->Now();

			sum += end - start;
		}
#endif
	}

	inline void OwnTrace::Print()
	{
#if Own_Trace
		DEBUG_STREAM << name << " - count: " << count << " sum: " << sum;
		if (count>0)
		{
			DEBUG_STREAM << " per call: " << sum/(double)(count) << '\n';
		}
		else
		{
			DEBUG_STREAM << '\n';
		}
#endif
	}

#endif