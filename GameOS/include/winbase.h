#ifndef _WINBASE_H
#define _WINBASE_H

#ifdef __cplusplus
extern "C" {
#endif

#define WINBASEAPI    

#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2

typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME,*LPSYSTEMTIME;

typedef struct _TIME_ZONE_INFORMATION {
	LONG Bias;
	WCHAR StandardName[32];
	SYSTEMTIME StandardDate;
	LONG StandardBias;
	WCHAR DaylightName[32];
	SYSTEMTIME DaylightDate;
	LONG DaylightBias;
} TIME_ZONE_INFORMATION,*LPTIME_ZONE_INFORMATION;

DWORD GetTimeZoneInformation(LPTIME_ZONE_INFORMATION);
VOID WINAPI GetSystemTime(LPSYSTEMTIME);

#define RtlMoveMemory memmove
#define RtlCopyMemory memcpy
#define RtlFillMemory(d,l,f) memset((d), (f), (l))
#define RtlZeroMemory(d,l) RtlFillMemory((d),(l),0)
#define MoveMemory RtlMoveMemory
#define CopyMemory RtlCopyMemory
#define FillMemory RtlFillMemory
#define ZeroMemory RtlZeroMemory

BOOL CreateDirectory(LPCSTR,LPSECURITY_ATTRIBUTES);
BOOL SetFileAttributes(LPCSTR,DWORD);
BOOL DeleteFile(LPCSTR);

int lstrlen(LPCSTR);
int lstrlenA(LPCSTR);

#define lstrcmpi lstrcmpiA

int lstrcmpiA(LPCSTR,LPCSTR);
int lstrcmpiW(LPCWSTR,LPCWSTR);


WINBASEAPI PVOID WINAPI VirtualAlloc(PVOID,DWORD,DWORD,DWORD);
WINBASEAPI BOOL WINAPI VirtualFree(PVOID,DWORD,DWORD);

WINBASEAPI DWORD WINAPI GetLastError(void);

#ifdef __cplusplus
}
#endif

#endif /* _WINBASE_H */
