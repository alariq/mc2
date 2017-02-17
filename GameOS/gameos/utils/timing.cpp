#include "timing.h"

#ifdef PLATFORM_WINDOWS
#include<windows.h>
#else
#include<time.h>
#endif

namespace timing {
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
}
