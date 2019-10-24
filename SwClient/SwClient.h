

#ifndef _SWCLIENT_H
#define _SWCLIENT_H

#define _WIN32_DCOM
#define _WIN32_WINNT 0x0501
#define _WIN32_IE    0x0501

#define  EXPORT         _declspec(dllexport) 

#include <winsock2.h>
#include <windows.h>
#include <winioctl.h>
#include "resource.h"
//#pragma comment (lib, "wsock32.lib")
#pragma comment(lib,"ws2_32.lib")

#include <initguid.h>     
#include <comdef.h>

#include "..\\include\\sevdef.h"
#define  CONFILE   "config.ini"

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

#define  WM_NOTHING         (WM_USER+1)
#define  WM_SOCKET_EVENT    (WM_USER+2)
#define  WMU_NOTIFY         (WM_USER+3)

#define  TIMEOUT            9999   //毫秒,9.99秒
#define  SOCKET_BUFFER_MAX_SIZE      (6*1024)
#define  SOCKET_BUF_SIZE    SOCKET_BUFFER_MAX_SIZE

#define APPMUTEX_GUID  L"{689A0EC9-3FD3-4249-B93C-01917026E39F}"
#define MBTEST(hwnd) MessageBox((hwnd),TEXT("测试中……"),TEXT("提示"),NULL)

#define SW_START    0x803
#define SW_STOP     0x804

static WCHAR szAppName[] = L"数据围墙" ;
extern HANDLE hMutex ;

#ifdef __cplusplus
extern "C" {
#endif

LRESULT OnCreate (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT OnCommand (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT OnSysCommand (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK DialogWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LinkConfigWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChangePwdWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SystemConfigWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ScanFileWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//通知系统服务
BOOL SelfPrivilegeUp(void);
BOOL NotifyService(_In_ DWORD dwControl, _Inout_ LPMYSERVCONTEXT pControlParams);

#ifdef __cplusplus
}
#endif


#endif 