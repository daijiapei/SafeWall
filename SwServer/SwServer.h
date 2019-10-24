

#ifndef _SWCLIENT_H
#define _SWCLIENT_H

#define _WIN32_DCOM
#define _WIN32_WINNT 0x0501
#define _WIN32_IE    0x0501


#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define  EXPORT         _declspec(dllexport) 

#include "..\\include\\mysock.h"
#include <windows.h>
#include <winioctl.h>
#include "resource.h"
#include "json\json.h"
#pragma comment(lib, "json_vc71_libmt.lib") 


#include <initguid.h>     
#include "..\\include\\sevdef.h"


#define  WM_NOTHING         (WM_USER+1)
#define  WM_SOCKET_EVENT    (WM_USER+2)
#define  WMU_NOTIFY         (WM_USER+3)

#define  TIMEOUT            9999   //毫秒,9.99秒
#define  SOCKET_BUFFER_MAX_SIZE      (6*1024)
#define  SOCKET_BUF_SIZE    SOCKET_BUFFER_MAX_SIZE

#define MBTEST(hwnd) MessageBox((hwnd),_T("测试中……"),_T("提示"),NULL)

#define SW_START    0x803
#define SW_STOP     0x804

static WCHAR szAppName[] = TEXT ("集控管理端") ;

#ifdef __cplusplus
extern "C" {
#endif

LRESULT CALLBACK DialogWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FrameWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT OnCreate (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT OnCommand (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#ifdef __cplusplus
}
#endif


#endif 