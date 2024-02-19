;------;
; SMOL ;
;------;

.486
.model	flat, stdcall
option	casemap: none

include	smol.inc

align 4

.code
start:
  ; set process priority to HIGH
  invoke  GetCurrentProcess
  invoke  SetPriorityClass, eax, HIGH_PRIORITY_CLASS

  ; create map file which will contain shared data - window handle
  xor     eax, eax
  invoke  CreateFileMapping, INVALID_HANDLE_VALUE, eax, PAGE_READWRITE, eax, sizeof(MAPDATA), ADDR szMapFileName
  push    eax

  invoke  GetLastError
  pop     ebx

  ; if file mapping failed - exit with last error return code
  or      ebx, ebx
  jz      l_exit

  mov     hMapFile, ebx
  push    eax

  ; maps a view of the map file
  invoke  MapViewOfFile, hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MAPDATA)
  mov     pMapData, eax
  push    eax

  invoke  GetLastError
  pop     ebx
  pop     ecx
  push    eax

  ; if view of file was not mapped - close map file and exit with last error return code
  or      ebx, ebx
  jz      l_close_hanlde

  pop     eax
  assume  ebx: PTR MAPDATA

  ; if it is not the first instance, step over
  xor     ecx, ERROR_ALREADY_EXISTS
  jz      l_already_exists

  ; it is a first instance of the program, so
  ; init shared data (set window handle to zero)
  xor     ecx, ecx
  mov     [ebx].hWnd, ecx

l_already_exists:
  ; get window handle from shared data
  mov     eax, [ebx].hWnd

  assume  ebx: nothing

  ; store handle to a temp variable
  mov     hMainWnd, eax

  ; if handle is not zero - will close the first instance and exit
  or      eax, eax
  jnz     l_unmap

  ; it is the first instance, so proceed...
  invoke  GetModuleHandle, NULL
  mov     ebx, eax
  push    ebx

  invoke  GetCommandLine
  pop     ebx

  invoke  WinMain, ebx, NULL, eax, SW_HIDE

  ; set temp variable of window handle to zero
  xor     ebx, ebx
  mov     hMainWnd, ebx

l_unmap:
  ; store last result
  push    eax
  invoke  UnmapViewOfFile, pMapData

l_close_hanlde:
  invoke  CloseHandle, hMapFile
  ; restore last result
  pop     eax

  ; if temp variable of window handle is zero - exit
  mov     ebx, hMainWnd
  or      ebx, ebx
  jz      l_exit

  ; close previous program instance
  xor     eax, eax
  invoke  SendMessage, ebx, WM_CLOSE, eax, eax
  xor     eax, eax

l_exit:
  invoke  ExitProcess, eax
;
;----------------------------------------
;
WinMain Proc  hInst: HINSTANCE, hPrevInst: HINSTANCE, CmdLine: DWORD, CmdShow: DWORD
  LOCAL wc  : WNDCLASSEX
  LOCAL msg : MSG

  ;----------------------------;
  ; Fill main window structure ;
  ;----------------------------;
  xor     eax, eax
  mov     wc.cbSize, SIZEOF WNDCLASSEX
  mov     wc.style, eax;CS_HREDRAW or CS_VREDRAW or CS_BYTEALIGNWINDOW
  mov     wc.lpfnWndProc, OFFSET WndProc
  mov     wc.cbClsExtra, eax
  mov     wc.cbWndExtra, eax
  push    hInst
  pop     wc.hInstance
  mov     wc.hbrBackground, eax;COLOR_BTNFACE + 1
  mov     wc.lpszMenuName, eax
  mov     wc.lpszClassName, OFFSET szWndClassName
  mov     wc.hIcon, eax
  mov     wc.hIconSm, eax
  mov     wc.hCursor, eax
  invoke  RegisterClassEx, ADDR wc

  ;-----------------------------;
  ; Create and hide main window ;
  ;-----------------------------;
  xor     eax, eax
  xor     ebx, CW_USEDEFAULT
  invoke  CreateWindowEx,         \
  			    WS_EX_LEFT,           \
  			    ADDR szWndClassName,  \
  			    ADDR szProgramTitle,  \
  			    WS_OVERLAPPEDWINDOW,  \
  			    ebx,                  \
  			    ebx,                  \
  			    ebx,                  \
  			    ebx,                  \
            eax,                  \
            eax,                  \
            hInst,                \
            eax

  mov     hMainWnd, eax
  push    eax

  invoke  GetLastError
  pop     ebx

  or      eax, eax
  jnz     l_wmp_exit

  mov     eax, pMapData

  assume  eax: PTR MAPDATA
  mov     [eax].hWnd, ebx 
  assume  eax: nothing

  invoke	ShowWindow, ebx, SW_HIDE

  ;--------------------------;
  ; Main window message loop ;
  ;--------------------------;
  .while TRUE
    invoke  GetMessage, ADDR msg, NULL, 0, 0
    .break  .if (!eax)
    invoke  TranslateMessage, ADDR msg
    invoke  DispatchMessage, ADDR msg
  .endw

  mov     eax, msg.wParam

l_wmp_exit:
  ret
WinMain	endp
;
;----------------------------------------
;
WndProc proc  hwnd: DWORD, uMsg: DWORD, wParam: DWORD, lParam: DWORD

  .if ( uMsg == WM_DESTROY )
    ; unregister session change notifications receiving
    invoke  WTSUnRegisterSessionNotification, hwnd
    ; send a WM_QUIT to the message queue
    invoke  PostQuitMessage, NULL

    ;-------------;

  .elseif ( uMsg == WM_CREATE )
    invoke  WTSRegisterSessionNotification, hwnd, NOTIFY_FOR_ALL_SESSIONS
    or      eax, eax
    jnz     l_wpp_exit
    invoke  PostQuitMessage, eax
    
  .elseif ( uMsg == WM_WTSSESSION_CHANGE )
    .if ( wParam == WTS_SESSION_LOCK )
      ; most probably workstation is locked now, so try to suspend monitor
      invoke  SendMessage, HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_OFF  
    .elseif ( wParam == WTS_SESSION_UNLOCK )
      ; manually wake up monitor to be sure it is on
      invoke  SendMessage, HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, MONITOR_ON
    .endif
  .else
    invoke	DefWindowProc, hwnd, uMsg, wParam, lParam	
    ret
  .endif

l_wpp_exit:
  xor     eax, eax
  ret
WndProc endp

end start
;---------;
; The End ;
;---------;
