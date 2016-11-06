#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include"windows.h"
#include"string.h"
#include"wchar.h"

static int gGetLastError = 0;

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

BOOL CopyFile(  LPCTSTR lpExistingFileName,
                LPCTSTR lpNewFileName,
                BOOL    bFailIfExists)
{
    int src = open(lpExistingFileName, O_RDONLY);
    if(src == -1) {
        return FALSE;
    }

    /* rw-rw-rw- */
    int permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;      

    int dst_flags = O_CREAT | O_WRONLY | O_TRUNC;
    if(bFailIfExists)
        dst_flags |= O_EXCL;

    int dst = open(lpNewFileName, dst_flags, permissions);
    if(dst == -1) {
        return FALSE;
    }

    const int BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    int num_read = 0;

    bool error = false;
    while ((num_read = read(src, buf, BUF_SIZE)) > 0) {
        if(write(dst, buf, num_read) != num_read) {
            error = true;
            break;
        }
    }


    if (num_read != -1 && !error)
    {
        struct stat file_info = {0};
        fstat(src, &file_info);

        struct utimbuf ut;
        ut.actime = file_info.st_atim.tv_sec;
        ut.modtime = file_info.st_mtim.tv_sec;
        utime(lpExistingFileName, &ut);
    }

    close(src);
    close(dst);

    if (num_read == -1 || error)
        return FALSE;

    return TRUE;
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

WINBASEAPI PVOID WINAPI VirtualAlloc(PVOID lpAddress, DWORD dwSize, DWORD flAllocationType, DWORD flProtect)
{
    if(flAllocationType == MEM_RESERVE)
    {       
        // to RESERVE memory in Linux, use mmap with a private, anonymous, non-accessible mapping.
        // The following line reserves 1gb of ram starting at 0x10000000.
        void* result = mmap(0, dwSize, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if(result == MAP_FAILED)
            gGetLastError = errno;

        return result == MAP_FAILED ? 0 : result;
    }
    else if(flAllocationType == MEM_COMMIT)
    {
        void* memptr;
        if(lpAddress == 0) {
            memptr = mmap(0, dwSize, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
            if(memptr == MAP_FAILED)
                gGetLastError = errno;

            if(memptr == MAP_FAILED)
               return 0;

            lpAddress = memptr;
        }

        assert(lpAddress!=0);

        // to COMMIT memory in Linux, use mprotect on the range of memory you'd like to commit, and
        // grant the memory READ and/or WRITE access.
        // The following line commits 1mb of the buffer.  It will return -1 on out of memory errors.
        //
        int flags = 0;

        if(flProtect == PAGE_READONLY)
            flags = PROT_READ;
        else if(flProtect == PAGE_READWRITE)
            flags = PROT_READ | PROT_WRITE;

        int result = mprotect((void*)lpAddress, dwSize, flags);
        if(result == -1)
        {
            gGetLastError = errno;
            return NULL;
        }
        return lpAddress;
    }
    else
    {
        assert(0 && "VirtualAlloc: unsupported allocation type");
    }
}


WINBASEAPI BOOL WINAPI VirtualFree(PVOID lpAddress, DWORD dwSize, DWORD dwFreeType)
{
    assert(dwFreeType == MEM_RELEASE);
    int rv = munmap(lpAddress, dwSize);

    return rv==-1 ? FALSE : TRUE;
}

WINBASEAPI DWORD WINAPI GetLastError()
{
    return gGetLastError;
}

HANDLE WINAPI FindFirstFileA( LPCTSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    // scandir + fnmatch
    assert(0 && "Not implemented");
    return 0;
}

BOOL WINAPI FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
    assert(0 && "Not implemented");
    return FALSE;
}

BOOL WINAPI FindClose(HANDLE hFindFile)
{
    assert(0 && "Not implemented");
    return FALSE;
}





