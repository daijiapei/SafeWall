

#ifndef _SWCSVC_H
#define _SWCSVC_H

#define _CRT_SECURE_NO_DEPRECATE 1

#define _CRT_NONSTDC_NO_DEPRECATE 1

#include <process.h>
#include "..\\include\\mysock.h"
#include <windows.h>

#include <stdlib.h>
#include <string>
#include <iostream>  
#include "json\json.h"
#pragma comment(lib, "json_vc71_libmt.lib") 

#include <initguid.h>     
#include <comdef.h>

#include "..\\include\\sevdef.h"


extern BOOL running;
extern Json::Value root;
extern CRITICAL_SECTION gSection;
extern char szLogFile[MAX_PATH];
extern char szConfigFile[MAX_PATH];


#ifdef __cplusplus
extern "C" {
#endif

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv); 
//DWORD WINAPI ServiceCtrlHandlerEx(DWORD Opcode,DWORD dwEventType, LPVOID lpEventData,  LPVOID lpContext) ;
void WINAPI ServiceCtrlHandler(DWORD Opcode); 
BOOL InstallService(WCHAR * ServiceName); //安装
BOOL UnInstallService(WCHAR * ServiceName); //卸载

//服务运行过程，主要修改这个函数
void WINAPI ServiceProc(DWORD argc, LPTSTR *argv) ;
UINT WINAPI ServerThread(LPVOID lpParam);  //IO线程

UINT WINAPI KeepActivityThread(LPVOID lpParam);
DWORD OnLogon(LPMYSERVCONTEXT lpContext);
DWORD OnGetUserID(LPMYSERVCONTEXT lpContext);
DWORD OnCancelLogon(void);
DWORD OnChangePassword(LPMYSERVCONTEXT lpContext);
DWORD OnGetAccess(void);

void ReleaseMapFile(IN HANDLE hMap,IN LPMYSERVCONTEXT lpContext);
BOOL OpenMapFile(OUT HANDLE * hMap,OUT LPMYSERVCONTEXT * lpContext,IN char * lpcMapName);
#ifdef __cplusplus
}
#endif


#endif