#ifndef SMOL_H_INCLUDED
#define SMOL_H_INCLUDED

#define MONITOR_ON          -1
#define MONITOR_LOW         1
#define MONITOR_OFF         2
#define IDT_LOCK_TIMER      1
#define IDT_START_TIMER     2
#define EVENT_WAIT_RETRIES  300

extern "C" {
WINBASEAPI DWORD WINAPI WTSGetActiveConsoleSessionId(VOID);
}

struct MapData
{
  HANDLE    hWnd;
};

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM,   LPARAM);
VOID    CALLBACK StartTimerProcedure  (HWND, UINT, UINT_PTR, DWORD);
VOID    CALLBACK WaitTimerProcedure  (HWND, UINT, UINT_PTR, DWORD);

/*  Make the class name into a global variable  */
const TCHAR szWndClassName[] = TEXT("clSMOL");
const TCHAR szProgramTitle[] = TEXT("Switch-off Monitor On Lock");
const TCHAR szMapFileName[]  = TEXT("Local\\SMOL_MAP_FILE");
const TCHAR szDesktopName[]  = TEXT("Default");

MapData *pMD    = NULL;
DWORD   hSessID = 0;

#endif // SMOL_H_INCLUDED
