

#include "swssvc.h"
#include "..\\include\\assist.h"

DWORD worker(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO)
{
	Json::Reader reader;
	Json::Value msg;
	Json::Value context;
	DWORD dwRet;

	reader.parse(pPerIO->buffer,msg);
	HeapFree(GetProcessHeap(),NULL, pPerIO->buffer);
	pPerIO->buffer = NULL;
	pPerIO->size = NULL;
	context = msg["context"];

	switch(msg["command"].asInt())
	{
		//以下为客户端消息
	case SAFEWALL_CLIENT_LOGON://登录
		{
			mylog(szLogFile,"SAFEWALL_CLIENT_LOGON");
			dwRet = OnClientLogon(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_CLIENT_CANCEL_LOGON:
		{
			mylog(szLogFile,"SAFEWALL_CLIENT_CANCEL_LOGON");
			dwRet = OnClientCancelLogon(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_CLIENT_CHANGE_PASSWORD:
		{
			mylog(szLogFile,"SAFEWALL_CLIENT_CHANGE_PASSWORD");
			dwRet = OnClientChangePwd(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_CLIENT_LOAD_ACCESS:
		{
			mylog(szLogFile,"SAFEWALL_CLIENT_LOAD_ACCESS");
			dwRet = OnClientLoadAccess(pPerHandle, pPerIO, context);
		}break;
		//以下为IC端消息
	case SAFEWALL_IC_LOGON:
		{
			mylog(szLogFile,"SAFEWALL_IC_LOGON");
			dwRet = OnIcLogon(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_IC_CANCEL_LOGON:
		{
			mylog(szLogFile,"SAFEWALL_IC_CANCEL_LOGON");
			dwRet = OnIcCancelLogon(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_IC_GET_ACCESS:
		{
			mylog(szLogFile,"SAFEWALL_IC_GET_ACCESS");
			dwRet = OnIcGetAccess(pPerHandle, pPerIO, context);
		}break;
		//以下为管理端消息
	case SAFEWALL_SERVER_LOGON:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_LOGON");
			dwRet = OnServerLogon(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_CANCEL_LOGON:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_CANCEL_LOGON");
			dwRet = OnServerCancelLogon(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_STOP_SERVICE:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_STOP_SERVICE");
			dwRet = OnServerStopService(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_GET_ACCESS:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_GET_ACCESS");
			dwRet = OnServerGetAccess(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_GET_FILE_ACCESS:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_GET_ACCESS");
			dwRet = OnServerGetFileAccessInfo(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_SET_FILE_ACCESS:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_SET_FILE_ACCESS");
			dwRet = OnServerSetFileAccessInfo(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_GET_PROCESS_ACCESS:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_SET_FILE_ACCESS");
			dwRet = OnServerSetFileAccessInfo(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_SET_PROCESS_ACCESS:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_SET_FILE_ACCESS");
			dwRet = OnServerSetFileAccessInfo(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_GET_USERLIST:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_GET_USERLIST");
			dwRet = OnServerGetUserList(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_ADD_USER:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_ADD_USER");
			dwRet = OnServerAddUser(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_GET_USERINFO:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_GET_USERINFO");
			dwRet = OnServerGetUserInfo(pPerHandle, pPerIO, context);
		}break;
	case SAFEWALL_SERVER_SET_USERINFO:
		{
			mylog(szLogFile,"SAFEWALL_SERVER_SET_USERINFO");
			dwRet = OnServerSetUserInfo(pPerHandle, pPerIO, context);
		}break;
	default:
		{
		}break;
	}

	return dwRet;
	
}