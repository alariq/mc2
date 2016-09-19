//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

class CZonePing 
{
// CZonePing
public:
	CZonePing();
	~CZonePing();

    void * operator new(size_t size);
    void * operator new[](size_t size);
    void operator delete( void* ptr );
    void operator delete[] ( void* ptr );


// IZonePing
public:
    typedef void PingCallbackFunc( DWORD inet, DWORD latency, void * context );


    int __stdcall StartupServer();
    virtual HRESULT __stdcall StartupClient( DWORD ping_interval_sec );
    virtual HRESULT __stdcall Shutdown();
    virtual HRESULT __stdcall Add( DWORD inet );
    virtual HRESULT __stdcall Ping( DWORD inet );
    virtual HRESULT __stdcall Remove( DWORD inet );
    virtual HRESULT __stdcall RemoveAll();
    virtual HRESULT __stdcall Lookup( DWORD inet, DWORD *pLatency );
    virtual HRESULT __stdcall RegisterCallback( DWORD inet, PingCallbackFunc * pfn, void * context);


private:
    class ZonePing
    {
    public: 
        ZonePing(DWORD inet = 0);

	    enum PINGSTATE { UNKNOWN, PINGER, PINGEE };

        void * operator new(size_t size);
        void * operator new[](size_t size);
        void operator delete( void* ptr );
        void operator delete[] ( void* ptr );

        DWORD     m_inet;
        DWORD     m_latency;
        DWORD     m_samples;
        DWORD     m_tick;

        // mdm - for use with callbacks
        PingCallbackFunc * m_pCallback;
        void *    m_context;

        PINGSTATE m_state;
        ZonePing* m_pNext;

    };


protected:
    ZonePing * FindNextItem( ZonePing * pPing, bool * bWrapped );
    inline DWORD GetListIndex(DWORD inet)
	{
		return ( inet & 0x000000FF ) % m_PingIntervalSec;
	}

    void ScanForMissed( void );

    inline DWORD GetTickDelta( DWORD now, DWORD then )
    {
        if ( now >= then )
        {
            return now - then;
        }
        else
        {
            return INFINITE - then + now;
        }
    }

    inline DWORD Get13BitTickDelta( DWORD now, DWORD then )
    {
        if ( now >= then )
        {
            return now - then;
        }
        else
        {
            return 0x1FFF - then + now;
        }
    }

    BOOL    CreateSocket();

    ZonePing* m_PingArray;
    DWORD     m_PingEntries;
    ZonePing * m_pCurrentItem;

    DWORD*    m_inetArray;
    DWORD     m_inetAlloc;

    DWORD  m_PingIntervalSec;
    DWORD  m_CurInterval;

    SOCKET m_Socket;
    BOOL   m_bWellKnownPort;
    HANDLE m_hWellKnownPortEvent;

    HANDLE m_hStopEvent;
    CRITICAL_SECTION m_pCS[1];

    HANDLE m_hPingerThread;
    HANDLE m_hPingeeThread;

    static DWORD WINAPI PingerThreadProc( LPVOID p );
    static DWORD WINAPI PingeeThreadProc( LPVOID p );

    void PingerThread();
    void PingeeThread();

    HANDLE m_hStartupMutex;
    long   m_refCountStartup;

    DWORD  m_inetLocal[4];  // allow up to 4 ip address
};
