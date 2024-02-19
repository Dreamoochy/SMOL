; include section
include     windows.inc
include     kernel32.inc
include     user32.inc
include     wtsapi32.inc

includelib  kernel32.lib
includelib  user32.lib
includelib  wtsapi32.lib

; function prototypes
WinMain \
  proto :DWORD, \ ; HINSTANCE hInstance     - Handle to the current instance of the application.
        :DWORD, \ ; HINSTANCE hPrevInstance - Handle to the previous instance of the application. This parameter is always NULL.
        :DWORD, \ ; LPSTR lpCmdLine         - Pointer to a null-terminated string specifying the command line for the application.
        :DWORD    ; int nCmdShow            - Specifies how the window is to be shown.

WndProc \
  proto :DWORD, \ ; HWND hwnd     - Handle to the window.
        :DWORD, \ ; UINT uMsg     - Specifies the message.
        :DWORD, \ ; WPARAM wParam - Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
        :DWORD    ; LPARAM lParam - Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.

MonitorTimerProc \
  proto :DWORD, \ ; HWND hwnd        - Handle to the window associated with the timer.
        :DWORD, \ ; UINT uMsg        - WM_TIMER message.
        :DWORD, \ ; UINT_PTR idEvent - timer identifier.
        :DWORD    ; DWORD dwTime     - Specifies the number of milliseconds that have elapsed since the system was started.

StartupTimerProc \
  proto :DWORD, \ ; HWND hwnd        - Handle to the window associated with the timer.
        :DWORD, \ ; UINT uMsg        - WM_TIMER message.
        :DWORD, \ ; UINT_PTR idEvent - timer identifier.
        :DWORD    ; DWORD dwTime     - Specifies the number of milliseconds that have elapsed since the system was started.

MAPDATA	STRUCT
	hWnd	DWORD ?
MAPDATA	ENDS

MONITOR_ON              equ -1
MONITOR_LOW             equ 1
MONITOR_OFF             equ 2
NOTIFY_FOR_THIS_SESSION equ 0
NOTIFY_FOR_ALL_SESSIONS equ 1

.data
szWndClassName  db "clSMOL", 0
szProgramTitle  db "Switch-off Monitor On session Lock", 0
szMapFileName   db "Local\SMOL_MAP_FILE", 0
hMainWnd        DWORD 0
 
.data?
hMapFile  DWORD ?
pMapData  DWORD ?
    
