#ifndef PLATFORM_WINDOWS

#include "platform_windef.h"
#include "platform_mmsystem.h"


#include <time.h>

DWORD timeGetTime()
{
    // we do not care about overflow here
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    DWORD milliseconds = ts.tv_sec * 1e+3;
    milliseconds += ts.tv_nsec / 1e+6;
    return milliseconds;
}

#endif // PLATFORM_WINDOWS