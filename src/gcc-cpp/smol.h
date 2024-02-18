#ifndef __SMOL_H_INCLUDED__
#define __SMOL_H_INCLUDED__

#define MONITOR_ON      -1
#define MONITOR_LOW     1
#define MONITOR_OFF     2

struct MapData
{
  HANDLE    hWnd;
};

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
const TCHAR szWndClassName[] = TEXT("clSMOL");
const TCHAR szProgramTitle[] = TEXT("Switch-off Monitor On Lock");
const TCHAR szMapFileName[]  = TEXT("Local\\SMOL_MAP_FILE");

MapData *pMD = NULL;

#endif // __SMOL_H_INCLUDED__
