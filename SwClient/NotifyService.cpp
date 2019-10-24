
#include "SwClient.h"

BOOL NotifyService(_In_ DWORD dwControl, _Inout_ LPMYSERVCONTEXT pControlParams)
{
	HANDLE schSCManager; 
    SC_HANDLE hService; 
	BOOL ret = FALSE;
	SERVICE_STATUS status;
	char msg[256] = {0};
	HWND hwnd = (HWND)pControlParams->fromHwnd;
	//HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS ,FALSE, szClientMutexName);

	//if(hMutex == NULL)
	//{
	//	pControlParams->error = GetLastError();
	//	sprintf_s(msg, "OpenMutexA错误： %d", pControlParams->error);
	//	MessageBoxA(hwnd, msg,msg, NULL);
	//	return FALSE;//没有互斥量说明服务未启动，返回失败
	//}
	//WaitForSingleObject(hMutex, INFINITE); 
	//到这里，独占读写共享内存

	/*
	do
	{
		HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, szClientMapName);
		if(!hMap)
		{
			pControlParams->error = GetLastError();
			sprintf_s(msg, "OpenFileMappingA错误： %d", pControlParams->error);
			MessageBoxA(hwnd, msg,msg, NULL);
			break;
		}

		LPVOID pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		memcpy(pBuffer, pControlParams, sizeof(MYSERVCONTEXT));
		schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

		if (!schSCManager)
		{
			pControlParams->error = GetLastError();
			sprintf_s(msg, "OpenSCManager错误： %d", pControlParams->error);
			MessageBoxA(hwnd, msg,msg, NULL);
			break;
		}

		hService=OpenService((struct SC_HANDLE__ *)schSCManager,
				szClientServiceName,SERVICE_USER_DEFINED_CONTROL);
		if(!hService)
		{
			pControlParams->error = GetLastError();
				sprintf_s(msg, "OpenService错误： %d", pControlParams->error);
				MessageBoxA(hwnd, msg,msg, NULL);
		}

		if(ControlService(hService,dwControl,&status))
		{
			//服务处理成功又把内存复制回来
			memcpy( pControlParams, pBuffer, sizeof(MYSERVCONTEXT));
			ret = pControlParams->error > 0 ? FALSE : TRUE;
		}
		else
		{
			pControlParams->error = GetLastError();
			sprintf_s(msg, "ControlService错误： %d", pControlParams->error);
			MessageBoxA(hwnd, msg,msg, NULL);
		}
		CloseServiceHandle((struct SC_HANDLE__ *)hService);
		CloseServiceHandle((struct SC_HANDLE__ *)schSCManager);
		UnmapViewOfFile(pBuffer);
		CloseHandle(hMap);
	}while(FALSE); */

	HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, szClientMapName);
	if(hMap)
	{
		LPVOID pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		memcpy(pBuffer, pControlParams, sizeof(MYSERVCONTEXT));
		
		schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if (schSCManager)
		{
			hService=OpenService((struct SC_HANDLE__ *)schSCManager,
				szClientServiceName,SERVICE_USER_DEFINED_CONTROL);
			if(hService)
			{
				if(ControlService(hService,dwControl,&status))
				{
					//服务处理成功又把内存复制回来
					memcpy( pControlParams, pBuffer, sizeof(MYSERVCONTEXT));
					ret = pControlParams->error >= 0 ? TRUE : FALSE;
				}
				else
				{
					pControlParams->error = GetLastError();
					sprintf_s(msg, "ControlService错误： %d", pControlParams->error);
					MessageBoxA(hwnd, msg,msg, NULL);
				}
				
				CloseServiceHandle((struct SC_HANDLE__ *)hService);
			}
			else
			{
				pControlParams->error = GetLastError();
				sprintf_s(msg, "OpenService错误： %d", pControlParams->error);
				MessageBoxA(hwnd, msg,msg, NULL);
			}
			CloseServiceHandle((struct SC_HANDLE__ *)schSCManager); 
		}
		else
		{
			pControlParams->error = GetLastError();
			sprintf_s(msg, "OpenSCManager错误： %d", pControlParams->error);
			MessageBoxA(hwnd, msg,msg, NULL);
		}

		UnmapViewOfFile(pBuffer);
		CloseHandle(hMap);
	}
	else
	{
		pControlParams->error = GetLastError();
		sprintf_s(msg, "OpenFileMappingA错误： %d", pControlParams->error);
		MessageBoxA(hwnd, msg,msg, NULL);
	}
	
//end:
//	ReleaseMutex(hMutex);
//	CloseHandle(hMutex);
	return ret;
}