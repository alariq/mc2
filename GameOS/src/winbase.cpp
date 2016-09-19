#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include"windows.h"
#include"string.h"
#include"wchar.h"

DWORD GetTimeZoneInformation(LPTIME_ZONE_INFORMATION tzi)
{

    const wchar_t* tzn = L"EST";
    wcscpy(tzi -> StandardName, tzn);
    tzi -> StandardBias = 0;
    GetSystemTime(&tzi -> StandardDate);

    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    tzi -> Bias = tz.tz_minuteswest; // sebi !NB check this

    return TIME_ZONE_ID_STANDARD;

}

VOID WINAPI GetSystemTime(LPSYSTEMTIME t)
{
    time_t tim = time(NULL);
    struct tm* gm_time_data = gmtime(&tim);

    t -> wYear = 1900 + gm_time_data -> tm_year;
    t -> wMonth = 1 + gm_time_data -> tm_mon;
    t -> wDayOfWeek = gm_time_data -> tm_wday;
    t -> wDay = gm_time_data -> tm_mday;
    t -> wHour = gm_time_data -> tm_hour;
    t -> wMinute = gm_time_data -> tm_min;
    t -> wSecond = gm_time_data -> tm_sec;
    t -> wMilliseconds = 0;
}

BOOL CreateDirectory(LPCSTR dir, LPSECURITY_ATTRIBUTES attr)
{
    return 0 == mkdir(dir, S_IRWXU);
}

BOOL SetFileAttributes(LPCSTR str, DWORD attr_bits)
{
    return TRUE; // sebi: does this means anything for Linux ?
}

BOOL DeleteFile(LPCSTR file)
{
    return 0 == unlink(file);
}

int lstrlen(LPCSTR str)
{
    return strlen(str);
}

int lstrlenA(LPCSTR str)
{
    return lstrlen(str);
}

int lstrcmpiA(LPCSTR str1, LPCSTR str2)
{
    return strcasecmp(str1, str2);
}
int lstrcmpiW(LPCWSTR str1, LPCWSTR str2)
{
    return wcscasecmp(str1, str2);
}
