#pragma once
//==========================================================================//
// File:	 ErrorHandler.hpp												//
// Contents: error handling routines										//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"string.hpp"

//JJ Fix-A-Roo
typedef IDirect3DVertexBuffer7 * (WINAPI *GETDXFUNC)(IDirect3DVertexBuffer7 *, IDirect3DDevice7*);
extern GETDXFUNC g_pGetDXVB;

//
const int MAX_MESSAGE_SIZE=65536;	// Maximum total error message size
const int MAX_BUG_NOTES=1024;		// Maximum characters entered as bug notes
const int MAX_LINE_LENGTH=256;		// Maximum characters per line in a file (shown as error line)
const int MAX_SUBJECT_LENGTH=128;	// Maximum length of email subject
const int MAX_RAID_DESC=2048;		// Maximum length of the Raid description (may need to be longer to accomodate RTF info)

extern DWORD DisableErrors;
extern volatile WORD FPUControl;	// Current FPU control word
extern WORD FPUDefault;				// Default FPU control word
extern DWORD ShowFrame;
extern void InitProcessorSpeed();
extern char* GetProcessor();
extern float L2SpeedR, L2SpeedW, L2SpeedRW, MainSpeedR, MainSpeedW, MainSpeedRW, VidMemR, VidMemW, VidMemRW, AGPMemR, AGPMemW, AGPMemRW;
extern char SpeedBuffer[16];			// ASCII MHz
extern float ProcessorSpeed;			// Processor speed
void CheckLogFile();
extern char LogBuffer[2048];
extern HANDLE LoggingThreadID;
extern bool ThreadDone;
extern void ShowFTOL();
extern void EnableSpewToFile();
extern void DisableSpewToFile();
extern bool InDebugger;						// During debugger rendering
extern char CacheInformation[128];
//
// Log file data types
//
const int Log_Type = 4;			// Number of bits for log file 'type' data
enum { Log_Invalid, Log_JoystickCount, Log_JoystickAxis, Log_JoystickButton, Log_JoystickInfo, Log_JoystickEffectStatus, Log_Keypress, Log_GetKey, Log_Endmarker, Log_DataBlock, Log_NetInfo, Log_Media }; 



extern DWORD SpewWrites;
extern char DebuggerName[MAX_PATH+1];
extern bool LaunchDebugger;
extern int ScreenImageSize;
char* GetFullErrorMessage( HWND hwnd );
void WriteBitStream( DWORD Value, DWORD Bits, int Type );
char* GetBugNotes( HWND hwnd );
char* SendMail( HWND Window, char* Subject, char* Text, char* Bitmap, char* LogFile );
DWORD ReadBitStream( DWORD Bits, int Type );
void InitLogging();
void DeleteLogging();
void UpDateLogFile();
void Spew( char* string );
char* GetTime();
char* GetExeTime();
char* VersionNumber( char* Buffer, DWORD High, DWORD Low );
BOOL IsDebuggerAvailable();
void TriggerDebugger();
char* gosGetUserName();
void LogRun( char* Message );
void InitRunLog();
void GetInstalledAudioVideoCodecs( FixedLengthString& Buffer );
void ReadLogData( BYTE* pData, DWORD Length );
void WriteLogData( BYTE* pData, DWORD Length );



typedef struct _pFTOL
{
	_pFTOL* pNext;
	DWORD	Heap[6];

} pFTOL;


extern pFTOL* ListOfFTOL;



//
// Routines dealing with floating point precision
//
void CheckPrecision();



//
// Playback/Record mode?
//
extern DWORD LogMode;								// 0=Logging, 1=Playback
void ShowLogFileControls();




//
// Machine Details
//
void ScanCards( FixedLengthString& Buffer );

//
// Functions in imagehlp.cpp
//
void _stdcall InitExceptionHandler( char* CommandLine );
void DestroyImageHlp();
char* GetSymbolFromAddress( char* Buffer, int Address );
char* GetLocationFromAddress( IMAGEHLP_LINE* pline, char* Buffer, int Address );
void InitStackWalk( STACKFRAME* sf, CONTEXT* Context );
int WalkStack( STACKFRAME* sf );
extern DWORD LastOffset;
extern STACKFRAME ImageHlp_sf;
extern IMAGEHLP_LINE ImageHlp_pline;
char* GetExceptionCode( EXCEPTION_RECORD* er );
void GetEnvironmentSettings( char* CommandLine );

//
// Context information
//
extern EXCEPTION_RECORD SavedExceptRec;
extern CONTEXT SavedContext;



extern bool MathExceptions;
extern bool MathSinglePrecision;
extern float OneOverProcessorSpeed;


//
// Used to setup the ErrorDialogProc
//
extern int ErrorFlags;
extern char* ErrorTitle;
extern char* ErrorMessage;
extern char* ErrorMessageTitle;
long _stdcall ProcessException( EXCEPTION_POINTERS* ep );
char* ErrorNumberToMessage( int hResult );
extern volatile int ProcessingError;				// Renentrancy test flag for assert error routine
extern char* ErrorExceptionText;
extern char* Hex8Number( int Number );
extern void GetProcessorDetails( STACKFRAME* sf, FixedLengthString& Buffer );
extern void GetMachineDetails( FixedLengthString& Buffer );
extern char* GetLineFromFile( char* tempLine, char* FileName, int LineNumber );
extern BYTE* GotScreenImage;					// Pointer to buffer containing screen image (always 24 bit bmp)
extern BYTE* GrabScreenImage();
extern void GetDirectXDetails( FixedLengthString& Buffer );
extern void GetGameDetails( FixedLengthString& Buffer, DWORD ErrorFlags );
extern int AllowDebugButton;
extern void DoDetailedDialog();
extern void DoSimpleDialog();
extern void DoColorDialog();
extern int ErrorFlags,ErrorReturn;
bool WriteLogFile( char* FileName );



#define IDC_CONTINUE					3
#define IDC_IGNORE						4
#define IDI_ICON						104
#define IDD_ERROR1						105
#define IDC_EMAIL						1000
#define IDC_ERRORTEXT					1001
#define IDC_SAVE						1002
#define IDC_DEBUG						1003
#define IDC_EXIT						1004
#define IDC_CAPTURESCREEN				1005
#define IDC_LOGFILES					1006
#define IDC_NOTES						1007
#define IDC_RAID						1008
#define IDC_REPLAY						1009
#define IDC_RAIDDESC					1010



typedef struct _IgnoreAddress
{
	_IgnoreAddress*	pNext;
	DWORD			Address;
} IgnoreAddress;


extern IgnoreAddress*	pIgnore;



#pragma pack(push,1)
typedef struct _LogStruct
{
//
// Store time at top of loop
//
	double		CurrentTime;					// Current time at top of the loop
	double		NonPausedCurrentTime;			// Time that does not pause
//	
// Store mouse information at top of loop
//
	float		MouseX;
	float		MouseY;
	int			pXDelta;
	int			pYDelta;
	int			pWheelDelta;
	BYTE		pButtonsPressed;
	WORD		Length;							// Length of bitstream (or 0)
} LogStructure;
#pragma pack(pop)


typedef struct
{
	DWORD			Magic;							// Magic number for valid log file
	DWORD			Version;						// Version number of log file data
	char			GameName[32];					// Name of application being logged
	char			AppPath[256];					// Path of application being logged
	char			CommandLine[128];				// Command line passed to App
	char			UserName[32];					// Username who created log
	char			LogDate[64];					// Date/Time log created
	char			ExeDate[64];					// Date/Time exe file used to create log
	DWORD			Frames;							// Number of frames of data
	DWORD			Length;							// Size of logging information
	long			StartSeed;						// Random Number seed
	LogStructure*	First;							// Pointer to first frame
	LogStructure*	Current;						// Pointer to current frame
	LogStructure*	Last;							// Pointer to last frame

} LogHeader;


extern LogHeader LogInfo;
extern DWORD LogLoops;
extern DWORD FrameNumber;							// When recording=current frame, playback=last frame
extern DWORD Debounce;

extern void EndLogging();
extern void LoadLogFile();
extern void CheckLogInSync();

//
// Size of blocks of memory allocated for logs
//
#define LOGBLOCKSIZE	(1024*1024*4)				// 4 Meg of log data maximum (About 10 minutes at 60f/s)
#define LOGVERSION		11							// Current version number of log file




extern bool	AllowFail;
extern volatile bool	SpewSilence;						// Set to disable all spews
extern HANDLE hSpewOutput;
void __stdcall ExitGameOS();
void InitializeSpew();
void TerminateSpew();
void DebugColor( BYTE red, BYTE green, BYTE blue );
void InitProcessorSpeed();
void DestroyExceptions();


	

