

#ifndef _SWSSVC_H
#define _SWSSVC_H

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <winioctl.h>
#include <stdlib.h>
#include <string>
#include <iostream>  
#include "json\json.h"
//#pragma comment (lib, "wsock32.lib")
#pragma comment(lib, "json_vc71_libmt.lib")
#pragma comment(lib,"ws2_32.lib")

#include <initguid.h>     
#include <comdef.h>

#include "..\\include\\sevdef.h"



#define MY_SERVICE_CONTROL_REFRESH_POLICY  (SERVICE_USER_DEFINED_CONTROL + 1) //刷新
#define MY_SERVICE_CONTROL_SYSTEM_LOGIN    (SERVICE_USER_DEFINED_CONTROL + 2) //管理员登录
//#define MY_SERVICE_CONTROL_READ_POLICY     (SERVICE_USER_DEFINED_CONTROL + 3) //读取策略
//#define MY_SERVICE_CONTROL_WRITE_POLICY    (SERVICE_USER_DEFINED_CONTROL + 4) //写入策略
//#define MY_SERVICE_CONTROL_CLOSE           (SERVICE_USER_DEFINED_CONTROL + 5) //关闭服务
//

typedef struct _USERLIST_HEADER{
	LIST_ENTRY header;
	CRITICAL_SECTION section;
	ULONG icount;
} USERLIST_HERDER, *PUSERLIST_HEADER;

typedef struct _SYSTEM_LOGIN_CONTEXT{
	char   userName[64];
	char   password[64];
	wchar_t   filePath[256];
	wchar_t   error[256];
}SYSTEM_LOGIN_CONTEXT, *LPSYSTEM_LOGIN_CONTEXT;

typedef struct _PER_HANDLE_DATA{
	LIST_ENTRY list;
	SOCKET socket;
	struct sockaddr_in addr;
	DWORD dwUser;
	char userid[64];
	char MAC[6];
	char PCName[64];
	time_t  loginTime;
	time_t  lastTime;
}PER_HANDLE_DATA, *PPER_HANDLE_DATA;

typedef struct _PER_IO_DATA{
	OVERLAPPED ol;
	long  size;
	int  offset;
	char * buffer;
	DWORD dwOperationType;
}PER_IO_DATA, *PPER_IO_DATA;

typedef struct _ONLINELIST{
	LIST_ENTRY list;
	SOCKET socket;
	SOCKADDR_IN si;
	char userName[64];
	char MAC[6];
	char PCName[64];
	DWORD dwUser;
	LONG  loginTime;
	PVOID pNext;
}ONLINELIST, *PONLINELIST;

typedef struct _FILE_EVENT{
	char FilePath[MAX_PATH];
	CRITICAL_SECTION section;
	Json::Value * root;
}FILEEVENT, *PFILEEVENT;

EXTERN_C BOOL running;
//extern Json::Value root;
EXTERN_C USERLIST_HERDER gUserList;
EXTERN_C FILEEVENT PolicyFile;
//extern FILEEVENT AccessFile;
EXTERN_C FILEEVENT UserFile;
EXTERN_C FILEEVENT IcUserFile;
EXTERN_C char szLogFile[MAX_PATH];

#ifdef __cplusplus
extern "C" {
#endif

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv); 
VOID WINAPI ServiceCtrlHandler(DWORD Opcode) ;
//void WINAPI ServiceCtrlHandler(DWORD Opcode); 
BOOL InstallService(WCHAR * ServiceName); //安装
BOOL UnInstallService(WCHAR * ServiceName); //卸载

//服务运行过程，主要修改这个函数
void WINAPI ServiceProc(DWORD argc, LPTSTR *argv) ;
UINT WINAPI ServerThread(LPVOID lpParam);  //IO线程

void SendProc(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans);
void RecvProc(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans);


//#if DBG
//#define MYLOG(_x_)   
//#else 
//void mylog(const char * text);
//#define MYLOG   mylog
//#endif // DBG

BOOL ReadPolicy(FILEEVENT * file);
BOOL SavePolicy(FILEEVENT * file);

//CS数据分析交互
DWORD worker(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO);
//客户端
DWORD OnClientLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnClientCancelLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnClientChangePwd(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnClientLoadAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
//IC端
DWORD OnIcLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnIcCancelLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnIcGetAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
//集控管理端
DWORD OnServerLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerCancelLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerStopService(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerGetAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerSetAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerGetFileAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerGetProcessAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerSetFileAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerSetProcessAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerGetUserList(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerAddUser(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerGetUserInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);
DWORD OnServerSetUserInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context);

void InitUserList(PUSERLIST_HEADER UserList);
void ReleaseUserList(PUSERLIST_HEADER UserList);
BOOL InserUserListNode(PUSERLIST_HEADER UserList, PPER_HANDLE_DATA pPerHandle);
BOOL DeleteUserListNode(PUSERLIST_HEADER UserList, PPER_HANDLE_DATA pPerHandle);
#ifdef __cplusplus
}
#endif


#endif