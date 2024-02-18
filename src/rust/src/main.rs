#![cfg(windows)]
#![windows_subsystem = "windows"] // no console

use windows::{
  core::*,//{Result, PCWSTR, w},
  Win32::Foundation::*, //{CloseHandle, GetLastError, LPARAM, WPARAM, HWND, HANDLE, LRESULT, ERROR_SUCCESS, ERROR_ALREADY_EXISTS, INVALID_HANDLE_VALUE, WIN32_ERROR},
  Win32::Graphics::Gdi::*, //{HBRUSH, COLOR_BACKGROUND},
  Win32::System::LibraryLoader::*, //GetModuleHandleW,
  Win32::System::Memory::*, //{CreateFileMappingW, MapViewOfFile, UnmapViewOfFile, FILE_MAP_ALL_ACCESS, PAGE_READWRITE},
  Win32::System::RemoteDesktop::*, //{ProcessIdToSessionId, WTSRegisterSessionNotification, WTSUnRegisterSessionNotification, NOTIFY_FOR_ALL_SESSIONS},
  Win32::System::Threading::*, //{GetCurrentProcess, GetCurrentProcessId, SetPriorityClass, HIGH_PRIORITY_CLASS},
  Win32::UI::WindowsAndMessaging::*, //{CreateWindowExW, RegisterClassExW, ShowWindow, DispatchMessageW, GetMessageW, TranslateMessage, DefWindowProcW, SendMessageW, SetWindowLongW, SetTimer,  GetWindowLongW, KillTimer, PostQuitMessage, GWL_USERDATA, WNDCLASSEXW, SC_MONITORPOWER, WM_SYSCOMMAND, MSG, WM_CLOSE, WM_CREATE, WM_DESTROY, WTS_SESSION_LOCK, WTS_SESSION_UNLOCK, WM_WTSSESSION_CHANGE, HMENU, HICON, HCURSOR, SW_HIDE, CW_USEDEFAULT, CS_BYTEALIGNWINDOW, CS_VREDRAW, CS_HREDRAW, WS_OVERLAPPEDWINDOW, WS_EX_LEFT},
};
use std::mem::size_of;

//const MONITOR_LOW       : isize =  1;
const MONITOR_OFF       : isize =  2;
const MONITOR_ON        : isize = -1;

const WND_CLASS_NAME: PCWSTR = w!("clSMOL");
const PROGRAM_TITLE : PCWSTR = w!("Switch-off Monitor On Lock");
const MAP_FILENAME  : PCWSTR = w!("Local\\SMOL_MAP_FILE");
const HWND_BROADCAST: HWND   = HWND(0xFFFF);

struct MapData {
  window_handle: HWND,
}

struct Config {
  map_file_handle : HANDLE,
  map_data_pointer: *const MapData,
  win32_error     : WIN32_ERROR,
}

impl Config {
  fn with_last_error(mut self) -> Self {
    self.win32_error = unsafe { GetLastError() };
    self
  }
}

fn create_window() -> HWND {
  unsafe {
    let instance_handle = GetModuleHandleW(PCWSTR::null());
    
    if ( instance_handle.is_err() ) {
      return HWND::default()
    };

    let instance_handle = instance_handle.unwrap();

    // The Window class structure
    let wincl = WNDCLASSEXW {
      cbSize        : size_of::<WNDCLASSEXW>() as u32,
      hInstance     : instance_handle,
      lpszClassName : WND_CLASS_NAME,
      lpfnWndProc   : Some(window_procedure),      // This function is called by windows
      style         : CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW,

      // Use default icon and mouse-pointer
      hIcon         : HICON(0),       // No icon
      hIconSm       : HICON(0),       // No icon 
      hCursor       : HCURSOR(0),     // No special cursor
      lpszMenuName  : PCWSTR::null(), // No menu
      cbClsExtra    : 0,              // No extra bytes after the window class
      cbWndExtra    : 0,              // structure or the window instance

      // Use Windows's default colour as the background of the window
      hbrBackground : HBRUSH(COLOR_BACKGROUND.0 as isize),
    };

    // Register the window class, and if it fails quit the program
    if ( RegisterClassExW( &wincl ) != 0 ) {
      // The class is registered, let's create the window
      CreateWindowExW(
        WS_EX_LEFT,          // Extended possibilites for variation
        WND_CLASS_NAME,      // Classname
        PROGRAM_TITLE,       // Title Text
        WS_OVERLAPPEDWINDOW, // default window
        CW_USEDEFAULT,       // Windows decides the position
        CW_USEDEFAULT,       // where the window ends up on the screen
        CW_USEDEFAULT,       // The programs width
        CW_USEDEFAULT,       // and height in pixels
        HWND(0),             // The window is a child-window to desktop
        HMENU(0),            // No menu
        instance_handle,     // Program Instance handler
        None                 // No Window Creation data
      )
    }
    else {
      HWND::default()
    }
  }
}

fn init() -> Config {
  let mut config = Config {
    map_file_handle : HANDLE::default(),
    map_data_pointer: std::ptr::null(),
    win32_error     : WIN32_ERROR::default(),
  };

  unsafe {
    // set process priority to HIGH
    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

    // create map file which will contain shared data - window handle
    let map_file_handle =
      CreateFileMappingW( INVALID_HANDLE_VALUE, None, PAGE_READWRITE, 0, size_of::<MapData>() as u32, MAP_FILENAME );

    config.win32_error = GetLastError();
    
    if ( map_file_handle.is_err() ) {
      return config;
    }

    config.map_file_handle = map_file_handle.unwrap();

    let map_data_ptr = MapViewOfFile( config.map_file_handle, FILE_MAP_ALL_ACCESS, 0, 0, size_of::<MapData>() );
    let map_data_ptr: *mut MapData = map_data_ptr as *const _ as *mut _;

    if ( map_data_ptr.is_null() ) {
      return config.with_last_error();
    }
    
    config.map_data_pointer = map_data_ptr;

    // If the map does not contain correct window handle, ...
    if ( config.win32_error != ERROR_ALREADY_EXISTS )
    {
      let window_handle = create_window();
      (*map_data_ptr).window_handle = window_handle;

      if ( window_handle == HWND::default() ) {
        return config.with_last_error();
      }

      ShowWindow( window_handle, SW_HIDE );
    }
  }

  config
}

fn deinit(cfg: &Config) -> Result<()> {
  unsafe {
    let mut window_handle = HWND::default();

    // Unmap mapped view of a file
    if ( !cfg.map_data_pointer.is_null() ) {
      window_handle = (*cfg.map_data_pointer).window_handle;
      UnmapViewOfFile( cfg.map_data_pointer as *const _);
    }

    // Close open object handle
    if ( !cfg.map_file_handle.is_invalid() ) {
      CloseHandle( cfg.map_file_handle );
    }

    if ( window_handle != HWND::default() ) {
      // if program is already running - close it
      SendMessageW( window_handle, WM_CLOSE, WPARAM(0), LPARAM(0) );
    }

    if ( cfg.win32_error == ERROR_SUCCESS ) {
      Ok(())
    }
    else {
      Err( cfg.win32_error.into() )
    }
  }
}

fn process_messages(cfg: &mut Config) {
  unsafe {
    // Run the message loop. It will run until GetMessage() returns 0
    let mut message: MSG = MSG::default(); // Here messages to the application are saved

//    while ( GetMessageW( &mut message, (*cfg.map_data_pointer).window_handle, 0, 0 ).as_bool() )
    while ( GetMessageW( &mut message, HWND(0), 0, 0 ).as_bool() )
    {
      // Translate virtual-key messages into character messages
      TranslateMessage(&message);
      // Send message to WindowProcedure
      DispatchMessageW(&message);
    }

    // The program return-value is 0 - The value that PostQuitMessage() gave
    cfg.win32_error = WIN32_ERROR(message.wParam.0 as u32);
  }
}

fn main() -> Result<()> {
  let mut config = init();

  if ( config.win32_error == ERROR_SUCCESS ) {
    process_messages(&mut config);
  }
  
  deinit(&config)
}

/* This function is called by the Windows function DispatchMessage() */
unsafe extern "system" fn window_procedure( hwnd: HWND, msg: u32, w_param: WPARAM, l_param: LPARAM ) -> LRESULT {
  match ( msg ) /* handle the messages */
  {
    WM_CREATE =>
    {
      // set startup timer
      if ( !WTSRegisterSessionNotification( hwnd, NOTIFY_FOR_ALL_SESSIONS ).as_bool() ) {
        PostQuitMessage(0);
      }
    },
    WM_DESTROY =>
    {
      // unregister session change notifications receiving
      WTSUnRegisterSessionNotification( hwnd );
      // send a WM_QUIT to the message queue
      PostQuitMessage( 0 );
    },
    WM_WTSSESSION_CHANGE => 
    {
      if ( w_param == WPARAM(WTS_SESSION_LOCK as usize) )
      {
        SendMessageW( HWND_BROADCAST, WM_SYSCOMMAND, WPARAM(SC_MONITORPOWER as usize), LPARAM(MONITOR_OFF) );
      }
      else if ( w_param == WPARAM(WTS_SESSION_UNLOCK as usize) )
      {
        // manually wake up monitor to be sure it is on
        SendMessageW( HWND_BROADCAST, WM_SYSCOMMAND, WPARAM(SC_MONITORPOWER as usize), LPARAM(MONITOR_ON) );
      }
    }
    _ =>
      // for messages that we don't deal with
      return DefWindowProcW( hwnd, msg, w_param, l_param ),
  }

  return LRESULT(0);
}
