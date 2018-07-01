#include "timing.h"

#ifdef PLATFORM_WINDOWS
#include<windows.h>
#include<time.h>
#else
#include<time.h>
#endif

#include <stdint.h>
#include <cassert>

namespace timing {

#ifdef PLATFORM_WINDOWS
	static LARGE_INTEGER Frequency;
#endif
#ifdef _DEBUG
	static bool initialized = false;
#endif

	void init()
	{
#ifdef PLATFORM_WINDOWS
		QueryPerformanceFrequency(&Frequency);
#endif
#ifdef _DEBUG
		initialized = true;
#endif
	}

	void sleep(unsigned int nanosec) {
#ifdef PLATFORM_WINDOWS
		Sleep(nanosec / 1000000);
#else
		timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = nanosec;
		nanosleep(&ts, NULL);
#endif
	}

	uint64_t gettickcount()
	{
#ifdef PLATFORM_WINDOWS
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return t.QuadPart;
#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
		size_t milliseconds = ts.tv_sec * 1e+3;
		milliseconds += ts.tv_nsec / 1e+6;
		return milliseconds;
#endif
	}

	uint64_t ticks2ms(uint64_t ticks)
	{
#ifdef _DEBUG
		assert(initialized);
#endif
#ifdef PLATFORM_WINDOWS
		ticks = (ticks * 1000) / Frequency.QuadPart;
		return ticks;
#else
		return ticks;
#endif
	}

    uint64_t get_wall_time_ms()
    {
        // or clock_gettime(CLOCKREALTIME, ts);
        struct timespec ts;
        timespec_get(&ts, TIME_UTC);
		size_t milliseconds = ts.tv_sec * 1e+3;
		milliseconds += ts.tv_nsec / 1e+6;
        return milliseconds;
    }
}
