#ifndef PLATFORM_WINDOWS

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
#include <dirent.h>
#include <fnmatch.h>
#include <stdint.h>
#include <libgen.h> // dirname
#include <stdlib.h> // free
#include <stdio.h> // free
#include<wchar.h> // wcscpy

#include"platform_windows.h"
#include"platform_str.h"

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
    int rv = mkdir(dir, S_IRWXU);
    if (rv) {
        gGetLastError = errno;
        switch (gGetLastError)
        {
        case EEXIST:
            gGetLastError = ERROR_ALREADY_EXISTS;
            break;
        case ENOENT:
            gGetLastError = ERROR_PATH_NOT_FOUND;
            break;
        }
    }
    return rv==0;
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
    //assert(dwFreeType == MEM_RELEASE);
    if(dwFreeType == MEM_RELEASE) {

        int rv = munmap(lpAddress, dwSize);
        if(rv == -1) {
            gGetLastError = errno;
            return FALSE;
        }
        return TRUE;
    } 
    /*
    else if(deFreeType == MEM_DECOMMIT) {

        // http://blog.nervus.org/managing-virtual-address-spaces-with-mmap/
        // instead of unmapping the address, we're just gonna trick 
        // the TLB to mark this as a new mapped area which, due to 
        // demand paging, will not be committed until used.

        mmap(lpAddress, dwSize, PROT_NONE, MAP_FIXED|MAP_PRIVATE|MAP_ANON, -1, 0);
        msync(lpAddress, dwSize, MS_SYNC|MS_INVALIDATE);
    }*/
    else if(dwFreeType == MEM_DECOMMIT) {

        int result = mprotect((void*)lpAddress, dwSize, PROT_NONE);
        if(result == -1) {
            gGetLastError = errno;
            return FALSE;
        }
        return TRUE;
    }

    assert(0 && "VirtualFree: Unsupported dwFreeType");
    return FALSE;

}

WINBASEAPI DWORD WINAPI GetLastError()
{
    return gGetLastError;
}

static const char* g_current_wildcard = NULL;
static int path_filter(const struct dirent* e) {
    return 0 == fnmatch(g_current_wildcard, e->d_name, FNM_PATHNAME);
}
struct FindFileData {
    struct dirent* entries;
    int last_retrieved_entry;
    int num_entries;
    char* dir_name;
    bool initialized;
};

static void FillFindData(const char* dir_name, const char* entry_name, LPWIN32_FIND_DATAA lpFindFileData) {

        char entry_path[MAX_PATH];
        snprintf(entry_path, MAX_PATH-1, "%s/%s", dir_name, entry_name);

        struct stat s;
        stat(entry_path, &s);

        if(S_ISDIR(s.st_mode))
            lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        if(S_ISREG(s.st_mode))
            lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
        if(S_ISBLK(s.st_mode))
            lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_DEVICE;

        // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
        uint64_t sec_since = 60*60*24*365*1601L; // well approximately

        // all date setting is incorrect
        uint64_t sec = s.st_atim.tv_sec - sec_since;
        uint64_t nsec = sec*1e9 + s.st_atim.tv_sec;
	    lpFindFileData->ftLastAccessTime.dwLowDateTime = (DWORD)nsec&0xffffffff;
	    lpFindFileData->ftLastAccessTime.dwHighDateTime = (DWORD)(nsec>>32)&0xffffffff;

        sec = s.st_ctim.tv_sec - sec_since;
        nsec = sec*1e9 + s.st_ctim.tv_sec;
	    lpFindFileData->ftCreationTime.dwLowDateTime = (DWORD)nsec&0xffffffff;
	    lpFindFileData->ftCreationTime.dwHighDateTime = (DWORD)(nsec>>32)&0xffffffff;

        sec = s.st_ctim.tv_sec - sec_since;
        nsec = sec*1e9 + s.st_ctim.tv_sec;
	    lpFindFileData->ftLastWriteTime.dwLowDateTime = (DWORD)nsec&0xffffffff;
	    lpFindFileData->ftLastWriteTime.dwHighDateTime = (DWORD)(nsec>>32)&0xffffffff;

        
	    lpFindFileData->nFileSizeHigh = s.st_size&0xffffffff;
	    lpFindFileData->nFileSizeLow = (s.st_size>>32)&0xffffffff;;

        int copy_size = strlen(entry_name);
        copy_size = copy_size < MAX_PATH ? copy_size : MAX_PATH-1;
        strncpy(lpFindFileData->cFileName, entry_name, copy_size);
        lpFindFileData->cFileName[copy_size] = '\0';
}

HANDLE WINAPI FindFirstFileA( LPCTSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    struct dirent** entries;
    int n = 0;

    // this will NOT work correctly if directory has wildcard

    char* dn = strdup(lpFileName);
    char* bn = strdup(lpFileName);
    char* dir_name = dirname(dn);
    char* base_name = basename(bn);

    g_current_wildcard = base_name;
    n = scandir(dir_name, &entries, path_filter, alphasort);
    g_current_wildcard = NULL;


    FindFileData* ffd = NULL;

    if(n > 0) {

        ffd = new FindFileData();
        ffd->entries = new dirent[n];
        for(int i=0;i<n;++i) {
            //memcpy(ffd->entries+i, entries[i], sizeof(struct dirent));
            // sebi: for some reason AddressSanitizer and valgrind complain about memcpy :-/
            const int len = sizeof(ffd->entries[i].d_name);
            strncpy(ffd->entries[i].d_name, entries[i]->d_name, len - 1);
            ffd->entries[i].d_name[len - 1] = '\0';
            free(entries[i]);
        }
        free(entries);
        entries = NULL;
        ffd->last_retrieved_entry = 0;
        ffd->num_entries = n;
        ffd->initialized = true;
        ffd->dir_name = strdup(dir_name);

        FillFindData(ffd->dir_name, ffd->entries[0].d_name, lpFindFileData);

    }

    free(dn);
    free(bn);

    return ffd == NULL ? INVALID_HANDLE_VALUE : ffd;
}

BOOL WINAPI FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
    assert(hFindFile!=0);

    FindFileData* ffd = (FindFileData*)hFindFile;
    assert(ffd->initialized);

    assert(ffd->last_retrieved_entry < ffd->num_entries);

    if(ffd->last_retrieved_entry + 1 == ffd->num_entries) {
        gGetLastError = ERROR_NO_MORE_FILES; 
        return FALSE;
    } else {
        const int e_idx = ++ffd->last_retrieved_entry;
        FillFindData(ffd->dir_name, ffd->entries[e_idx].d_name, lpFindFileData);
        return TRUE;
    }
}

BOOL WINAPI FindClose(HANDLE hFindFile)
{
    assert(hFindFile!=0);

    if(hFindFile == INVALID_HANDLE_VALUE)
        return TRUE;

    FindFileData* ffd = (FindFileData*)hFindFile;
    assert(ffd->initialized);

    free(ffd->dir_name);
    delete[] ffd->entries;
    delete ffd;

    return TRUE;
}


#endif // PLATFORM_WINDOWS



