#include "gameos.hpp"
#include "toolos.hpp"
#include "memorymanager.hpp" // gos_Heap
#include <stdio.h>
#include <time.h>
#include <stdlib.h> // rand
#include <sys/statvfs.h> // statvfs
#include <stdlib.h> // getenv

// fstat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "strres.h"

////////////////////////////////////////////////////////////////////////////////
void __stdcall AddDebuggerMenuItem(char const*, bool (__stdcall *)(), void (__stdcall *)(), bool (__stdcall *)(), DWORD (__stdcall *)(char const*, DWORD))
{
    // TODO: maybe use dconsole for this
}

void AddStatistic( const char* Name, const char* TypeName, gosType Type, void* Value, DWORD Flags )
{
    printf("STATISTICS: [%s : %s]\n", Name, TypeName);
}
void StatisticFormat(char const* s)
{
    printf("STATISTICS: [%s]\n", s);
}
////////////////////////////////////////////////////////////////////////////////

void* DecodeJPG( const char* FileName, BYTE* Data, DWORD DataSize, DWORD* TextureWidth, DWORD* TextureHeight, bool TextureLoad, void *pDestSurf )
{
    return NULL;
}

gosEnvironment Environment;

////////////////////////////////////////////////////////////////////////////////
void EnterFullScreenMode(void)
{
    Environment.fullScreen = true;
}

void EnterWindowMode(void)
{
    Environment.fullScreen = false;
}

static bool g_gos_exit_game_os = false;

void ExitGameOS()
{
    g_gos_exit_game_os = true;
}

bool gosExitGameOS() {
    return g_gos_exit_game_os;
}

float frameRate = 30.0f; // apparently tiny geometry needs this

__int64 __stdcall GetCycles()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    int64_t v = ts.tv_sec * 1e+9;
    v += ts.tv_nsec;
    return v;
}

double g_prev_elapsed_time_value = -1;
double __stdcall gos_GetElapsedTime( int RealTime )
{
    (void)RealTime;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    double time_sec = ts.tv_sec;
    if(g_prev_elapsed_time_value < 0.0) {
        g_prev_elapsed_time_value = time_sec;
        return time_sec;
    }
    
    if(time_sec - g_prev_elapsed_time_value > Environment.MaxTimeDelta) {
        g_prev_elapsed_time_value += Environment.MaxTimeDelta;
        return g_prev_elapsed_time_value;
    }

    g_prev_elapsed_time_value = time_sec;
    return time_sec;
}

////////////////////////////////////////////////////////////////////////////////
DWORD __stdcall gos_GetMachineInformation( MachineInfo mi, int Param1/*=0*/, int Param2/*=0*/, int Param3/*=0*/, int Param4/*=0*/)
{
    // TODO:
    if(mi == gos_Info_GetDeviceLocalMemory)
        return 1024*1024*1024;
    if(mi == gos_Info_GetDeviceAGPMemory)
        return 512*1024*1024; 
    if (mi == gos_Info_CanMultitextureDetail)
        return true;
    if(mi == gos_Info_NumberDevices)
        return 1;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

static const int g_hepsStackSize = 128;
static gos_Heap* g_heapsStack[g_hepsStackSize];
static int g_heapStackPointer = -1;

////////////////////////////////////////////////////////////////////////////////
HGOSHEAP __stdcall gos_CreateMemoryHeap(char const* HeapName, DWORD MaximumSize/* = 0*/, HGOSHEAP parentHeap/* = ParentClientHeap*/)
{
    gos_Heap* pheap = new gos_Heap();
    memset(pheap, 0, sizeof(gos_Heap));
    pheap->pParent = parentHeap;
    pheap->Magic = (DWORD)(reinterpret_cast<size_t>(HeapName) & 0xffffffff);
    if(HeapName) {
        strncpy(pheap->Name, HeapName, sizeof(pheap->Name)-1);
        pheap->Name[sizeof(pheap->Name)-1] = '\0';
    }

#ifdef LAB_ONLY
    pheap->BytesAllocated = 0;
    pheap->MaximumSize = MaximumSize;
#endif

    return pheap;
}
void __stdcall gos_DestroyMemoryHeap(HGOSHEAP Heap, bool shouldBeEmpty/* = true*/)
{
    // shouldBeEmpty ?? should Instances be zero or what?
    delete Heap;
    Heap = nullptr;
}
void __stdcall gos_PushCurrentHeap(HGOSHEAP Heap)
{
    gosASSERT(g_heapStackPointer < g_hepsStackSize);
    g_heapsStack[++g_heapStackPointer] = Heap;
}
void __stdcall gos_PopCurrentHeap()
{
    gosASSERT(g_heapStackPointer >= 0 && g_heapStackPointer < g_hepsStackSize);
    g_heapsStack[g_heapStackPointer--] = nullptr;
}

HGOSHEAP __stdcall gos_GetCurrentHeap()
{
    if(g_heapStackPointer == -1)
        return NULL;
    return g_heapsStack[g_heapStackPointer];
}

void __stdcall gos_WalkMemoryHeap(HGOSHEAP pHeap, bool vociferous/* = false*/)
{

}

////////////////////////////////////////////////////////////////////////////////
void* operator new(size_t sz) {
    return gos_Malloc(sz, NULL);
}
void operator delete(void* ptr) noexcept {
    gos_Free(ptr);
}

void* __cdecl operator new[](size_t size, HGOSHEAP Heap)
{
    return malloc(size);
}

void* __stdcall gos_Malloc(size_t bytes, HGOSHEAP Heap/* = 0*/)
{
    // FIXME: TODO: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // FIXME: TODO: each malloc should record memory allocation for current heap on a stack
    // FIXME: TODO: so that when Heap destroy() is called all memory will be freed

    /*
    gosHeap* heap = NULL;

    if(Heap) {
        heap = (gosHeap*)Heap;
    } else { 
        heap = gos_GetCurrentHeap();
    }

    if(heap)
        heap->BytesAllocated += bytes;

    if(bytes > 1024*1024)
    {
        PAUSE((""));
    }
    */
    return malloc(bytes);
}
void __stdcall gos_Free(void* ptr)
{
    /*
    gosHeap* heap = (gosHeap*)gos_GetCurrentHeap();
    if(Heap) {
        gosHeap* heap = (gosHeap*)Heap;
        heap->BytesAllocated -= // ??? ;
    }
    */
    free(ptr);
}
////////////////////////////////////////////////////////////////////////////////

void __stdcall gos_srand(unsigned int seed)
{
    return srand(seed);
}

int __stdcall gos_rand()
{
    return rand() % (1<<15);
}
////////////////////////////////////////////////////////////////////////////////
void __stdcall gos_EnableIME(bool enabledDisabled)
{
}
void _stdcall gos_ToggleIME(bool activeInactive)
{
}
void _stdcall gos_GetIMEAppearance(gosIME_Appearance* pia)
{
}
void _stdcall gos_SetIMEAppearance(const gosIME_Appearance* pia)
{
}
void _stdcall gos_PositionIME(DWORD x, DWORD y)
{
}
void _stdcall gos_SetIMELevel(DWORD dwImeLevel)
{
}
////////////////////////////////////////////////////////////////////////////////
float __stdcall gosJoystick_GetAxis( DWORD index, GOSJoystickAxis axis )
{
    (void)index; (void)axis;
    gosASSERT(0 && "Not implemented");
    return 0.0f;
}

DWORD __stdcall gosJoystick_CountJoysticks( bool ReDetect )
{
    (void)ReDetect;
    //gosASSERT(0 && "Not implemented");
    return 0;
}

void __stdcall gosJoystick_SetPolling( DWORD index, bool yesNo, float howOften/*=0.03333f*/ )
{
    (void)index, (void)yesNo, (void)howOften;
    gosASSERT(0 && "Not implemented");
}

void __stdcall gosJoystick_GetInfo( DWORD index, gosJoystick_Info* gji )
{
    gosASSERT(gji);
	gji->lpstrName = NULL;
	gji->bAxisValid = 0;
	gji->nAxes = 0;			
	gji->nButtons = 0;	
	gji->nSliders = 0;
	gji->nPOVs = 0;				
	gji->bIsPolled = 0;		
	gji->bIsForceFeedback = 0;
}

////////////////////////////////////////////////////////////////////////////////
char* __stdcall gos_GetFormattedDate( bool Verbose, WORD Year/*=-1*/, WORD Month/*=-1*/, WORD Day/*=-1*/ )
{
    time_t tim = time(NULL);
    struct tm* gm_time_data = gmtime(&tim);
    if(Year < 0) {
        Year = 1900 + gm_time_data -> tm_year;
    }
    if(Month < 0) {
        Month = 1 + gm_time_data -> tm_mon;
    }
    if(Day < 0) {
        Day = gm_time_data -> tm_mday;
    }

    if(Year > 9999) {
        Year = 0;
    }

    if(Month > 99) {
        Month = 0;
    }

    if(Day > 99) {
        Day = 0;
    }

    if(!Verbose)
    {
        char* date = new char[16];
        sprintf(date, "%02d/%02d/%04d", Day, Month, Year);
        return date;
    }

    gm_time_data->tm_year = Year;
    gm_time_data->tm_mon = Month;
    gm_time_data->tm_mday = Day;

    time_t tt = mktime(gm_time_data);
    char* str_time = ctime(&tt);
    
    char* date = new char[strlen(str_time)+1];
    strcpy(date, str_time);
    return date;

}

////////////////////////////////////////////////////////////////////////////////
char* __stdcall gos_GetFormattedTime( WORD Hour/*=-1*/, WORD Minute/*=-1*/, WORD Second/*=-1*/ )
{
    time_t tim = time(NULL);
    struct tm* gm_time_data = gmtime(&tim);
    if(Hour < 0) {
        Hour = gm_time_data -> tm_hour;
    }
    if(Minute < 0) {
        Minute = gm_time_data -> tm_min;
    }
    if(Second < 0) {
        Second = gm_time_data -> tm_sec;
    }

    if(Hour > 99) {
       Hour = 0;
    }

    if(Minute > 99) {
        Minute = 0;
    }

    if(Second > 99) {
        Second = 0;
    }

    char* date = new char[16];
    sprintf(date, "%02d:%02d:%02d", Hour, Minute, Second);
    return date;
}

////////////////////////////////////////////////////////////////////////////////

void __stdcall gos_TerminateApplication()
{
    g_gos_exit_game_os = true;
}

void __stdcall gos_AbortTermination()
{
    g_gos_exit_game_os = false;
}
////////////////////////////////////////////////////////////////////////////////
__int64 __stdcall gos_GetDriveFreeSpace( char* Path )
{
    struct statvfs s;
    if(-1 == statvfs(Path, &s)) {
        int last_err = errno;
        SPEW(("statvfs: %s\n", strerror(last_err)));
        return 0;
    }

    return s.f_bsize * s.f_bfree;
}

DWORD __stdcall gos_EnableSetting( gosSetting Setting, DWORD Value )
{
    // stub now implement later
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// registry read/write
static const char* g_user_reg_file = ".registry";
static const char* g_user_base_config_dir = ".config";
static const char* g_user_config_dir = ".mechcommander2";

// TODO: use inifile.h functionality for this
void __stdcall gos_LoadDataFromRegistry( const char* keyName, void* pData, DWORD* szData, bool HKLM/*=false*/)
{
    // this code can be useful to get registry file location
    
    const char* home_dir = getenv("HOME");
    if(home_dir == NULL)
    {
        SPEW(("gos_LoadDataFromRegistry", "home directory is not set\n"));
        *szData = 0;
        return;
    }

    const int cfg_dir_size = 1024;
    char cfg_dir[cfg_dir_size] = {0};

    const char* config_dir = getenv("XDG_CONFIG_HOME");
    if(config_dir == NULL) {
        sprintf(cfg_dir, "%s/%s", home_dir, g_user_base_config_dir);
    } else {
        strncpy(cfg_dir, config_dir, 1023);
        cfg_dir[cfg_dir_size-1] = '\0';
    }

    struct stat st;
    if(-1 == stat(cfg_dir, &st)) {
        int last_err = errno;
        SPEW(("stat: %s\n", strerror(last_err)));
        *szData = 0;
        return;
    }
    if(!S_ISDIR(st.st_mode)) {
        SPEW(("REG", "gos_LoadDataFromRegistry: %s is not directory\n", cfg_dir));
        *szData = 0;
        return;
    }

    // 1 + 1 =  for \0 and /
    char* mc2_conf_dir = new char[strlen(cfg_dir) + strlen(g_user_config_dir) + 1 + 1];
    sprintf(mc2_conf_dir, "%s/%s", cfg_dir, g_user_config_dir);

    // if directory does not exist create it
    int permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;      
    if(-1 == stat(mc2_conf_dir, &st)) {
        if(-1 == mkdir(mc2_conf_dir, permissions)) {
            int last_err = errno;
            SPEW(("stat: %s\n", strerror(last_err)));
            *szData = 0;
            delete mc2_conf_dir;
            return;
        }
    }

    char* reg_file = new char[strlen(mc2_conf_dir) + 1 + strlen(g_user_reg_file) + 1];
    sprintf(reg_file, "%s/%s", mc2_conf_dir, g_user_reg_file);

    // check if reg file exists
    if(-1 == stat(reg_file, &st)) {
        *szData = 0;
        return;
    } else {
        *szData = 0;
        // open file and read value
    }
}

void __stdcall gos_SaveDataToRegistry( const char* keyName,  void* pData,  DWORD szData )
{
    // TODO:
}

void __stdcall gos_SaveStringToRegistry( const char* keyName,  char* pData,  DWORD szData )
{
    // TODO;
}


////////////////////////////////////////////////////////////////////////////////

bool mc2IsInDisplayBackBuffer = false;
bool mc2IsInMouseTimer = false;

////////////////////////////////////////////////////////////////////////////////

HGOSHEAP ParentClientHeap = NULL;
float ProcessorSpeed = 10595.42; // sebi: put something cool
