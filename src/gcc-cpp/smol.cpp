#define _WIN32_WINNT 0x0501

typedef enum _WTS_VIRTUAL_CLASS {
  WTSVirtualClientData,
  WTSVirtualFileHandle
} WTS_VIRTUAL_CLASS;

#include <windows.h>
#include <winbase.h>
#include <wtsapi32.h>
#pragma comment(lib, "WtsApi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

#include "smol.h"

//
//----------------------------------------
//
int WINAPI WinMain( HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow )
{
  HANDLE  hMFile;
  HANDLE  hWnd = INVALID_HANDLE_VALUE;
  int     Result;

  // set process priority to HIGH
  SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

  // create map file which will contain shared data - window handle
  hMFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(MapData), szMapFileName );

  Result = GetLastError();

  if ( !hMFile )
    return Result; // exit if failed

  // maps a view of the map file
  pMD = (MapData*) MapViewOfFile( hMFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MapData) );

  if ( !pMD )
  {
    Result = GetLastError();
    // close map file and exit if failed
    CloseHandle( hMFile );
    return Result;
  }

  if ( Result != ERROR_ALREADY_EXISTS )
  {
    pMD->hWnd  = INVALID_HANDLE_VALUE; // init shared data if success
  }

  // If the map does not contain correct window handle, ...
  if ( pMD->hWnd == INVALID_HANDLE_VALUE )
  {
    // Continue

    HWND        hwnd;         // This is the handle of our window
    WNDCLASSEX  wincl;        // Data structure for the windowclass

    // The Window structure
    wincl.cbSize        = sizeof(WNDCLASSEX);
    wincl.hInstance     = hThisInstance;
    wincl.lpszClassName = szWndClassName;
    wincl.lpfnWndProc   = WindowProcedure;      /* This function is called by windows */
    wincl.style         = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;

    // Use default icon and mouse-pointer
    wincl.hIcon        = NULL; //(HICON)   LoadImage(NULL, MAKE_INT_RESOURCE(IDI_APPLICATION), IMAGE_ICON,   0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wincl.hIconSm      = NULL; //(HICON)   LoadImage(NULL, MAKE_INT_RESOURCE(IDI_APPLICATION), IMAGE_ICON,   0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wincl.hCursor      = NULL; //(HCURSOR) LoadImage(NULL, MAKE_INT_RESOURCE(IDC_ARROW),       IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wincl.lpszMenuName = NULL; // No menu
    wincl.cbClsExtra   = 0;    // No extra bytes after the window class
    wincl.cbWndExtra   = 0;    // structure or the window instance

    // Use Windows's default colour as the background of the window
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    // Register the window class, and if it fails quit the program
    if ( RegisterClassEx( &wincl ) )
    {
      // The class is registered, let's create the program
      hwnd = CreateWindowEx(
        WS_EX_LEFT,          // Extended possibilites for variation
        szWndClassName,      // Classname
        szProgramTitle,      // Title Text
        WS_OVERLAPPEDWINDOW, // default window
        CW_USEDEFAULT,       // Windows decides the position
        CW_USEDEFAULT,       // where the window ends up on the screen
        CW_USEDEFAULT,       // The programs width
        CW_USEDEFAULT,       // and height in pixels
        NULL,                // The window is a child-window to desktop
        NULL,                // No menu
        hThisInstance,       // Program Instance handler
        NULL                 // No Window Creation data
      );

      // if window was successfully created, ...
      if ( hwnd )
      {
        MSG messages;     // Here messages to the application are saved

        hWnd      = hwnd;
        pMD->hWnd = hwnd;

        // Hide window
        ShowWindow( hwnd, SW_HIDE );

        // Run the message loop. It will run until GetMessage() returns 0
        while ( GetMessage( &messages, NULL, 0, 0 ) )
        {
          // Translate virtual-key messages into character messages
          TranslateMessage( &messages );
          // Send message to WindowProcedure
          DispatchMessage( &messages );
        }

        // The program return-value is 0 - The value that PostQuitMessage() gave
        Result = messages.wParam;

        hWnd = INVALID_HANDLE_VALUE;
      }
      else
        Result = GetLastError();
    }
  }
  else
  {
    // If the map file already exists and contains correct window handle, ...
    hWnd   = pMD->hWnd;
    Result = 0;
  }

  // Unmap mapped view of a file
  UnmapViewOfFile( pMD );

  // Close open object handle
  CloseHandle( hMFile );

  if ( hWnd != INVALID_HANDLE_VALUE )
  {
    // if program is already running - close it
    SendMessage( (HWND)hWnd, WM_CLOSE, 0, 0 );
  }

  return Result;
};

/* This function is called by the Windows function DispatchMessage() */
LRESULT CALLBACK WindowProcedure( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg ) /* handle the messages */
  {
    case WM_CREATE:
    {
      // set startup timer
      SetTimer( hwnd, IDT_START_TIMER, 1000, &StartTimerProcedure );
      break;
    }
    case WM_DESTROY:
    {
      // unregister session change notifications receiving
      WTSUnRegisterSessionNotification( hwnd );
      // send a WM_QUIT to the message queue
      PostQuitMessage( 0 );
      break;
    }
    case WM_WTSSESSION_CHANGE:
    {
      if ( WTSGetActiveConsoleSessionId() != hSessID )
        break;

      if ( wParam == WTS_SESSION_LOCK )
      {
        // check if session is to be locked
        SetTimer( hwnd, IDT_LOCK_TIMER, 500, &WaitTimerProcedure );
      }
      else if ( wParam == WTS_SESSION_UNLOCK )
      {
        // disable session lock check timer
        KillTimer( hwnd, IDT_LOCK_TIMER );

        // manually wake up monitor to be sure it is on
        SendMessage( HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_ON );
      }

      break;
    }
    default:
      // for messages that we don't deal with
      return DefWindowProc( hwnd, uMsg, wParam, lParam );
  }

  return 0;
};

/* Timer procedure for checking session lock state */
VOID CALLBACK WaitTimerProcedure( HWND hwnd, UINT uMsg, UINT_PTR idEvend, DWORD dwTime )
{
  // try to open current input desktop
  HDESK hDesktop = OpenDesktop( (LPSTR)(szDesktopName), 0, false, DESKTOP_SWITCHDESKTOP );

  if ( hDesktop )
  {
    if ( !SwitchDesktop( hDesktop ) )
    {
      // disable session lock check timer
      KillTimer( hwnd, IDT_LOCK_TIMER );
      // most probably workstation is locked now, so try to power off monitor
      SendMessage( HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_OFF );
    }
    CloseDesktop( hDesktop );
  }

  return;
}

/* Timer procedure for correct start */
VOID CALLBACK StartTimerProcedure( HWND hwnd, UINT uMsg, UINT_PTR idEvend, DWORD dwTime )
{
  static int nRetries = EVENT_WAIT_RETRIES;

  if ( WTSRegisterSessionNotification( hwnd, NOTIFY_FOR_ALL_SESSIONS ) )
  {
    // disable start timer
    KillTimer( hwnd, IDT_START_TIMER );
    ProcessIdToSessionId( GetCurrentProcessId(), &hSessID );
  }
  else if ( !--nRetries )
  {
    PostQuitMessage( nRetries );
  }

  return;
}
