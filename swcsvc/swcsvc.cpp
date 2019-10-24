
#include "swcsvc.h"
#include "..\\include\\assist.h"


BOOL running = TRUE; //是否保持运行
Json::Value root;
SERVICE_STATUS m_ServiceStatus; 
SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
CRITICAL_SECTION gSection;
char szLogFile[MAX_PATH];
char szConfigFile[MAX_PATH];


int main(int argc, char* argv[])
{
	//_wsetlocale(LC_ALL, L"chs");
    if(argc>1) 
    { 
        if( strcmp(argv[1],"-i")==0) 
        { 
			if(InstallService(szClientServiceName)) 
			printf("\n 服务安装成功\n"); 
			else 
			printf("\n 服务安装失败\n"); 
        } 
        else if(strcmp(argv[1],"-d")==0) 
        { 
            if(UnInstallService(szClientServiceName)) 
            printf("\n 服务卸载成功\n"); 
            else 
            printf("\n 服务卸载失败\n"); 
        }
        else 
        { 
            printf("\n 未知的命令\n  重新运行输入-i将安装服务\n重新运行输入-d将卸载服务\n"); 
			//printf("your input: count=%d; str1= %s; str2= %s;\n",argc,argv[0],argv[1]); 
        } 

	}
    else
    {
        SERVICE_TABLE_ENTRY DispatchTable[]={{szClientServiceName,ServiceMain},{NULL,NULL}};   
        StartServiceCtrlDispatcher(DispatchTable);  
    }
    
    return 0; 
}

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv) 
{ 
	//     DWORD status;  
	//     DWORD specificError;  
	m_ServiceStatus.dwServiceType          = SERVICE_WIN32;  
	m_ServiceStatus.dwCurrentState         = SERVICE_START_PENDING;  
	m_ServiceStatus.dwControlsAccepted     = SERVICE_ACCEPT_STOP;  
	m_ServiceStatus.dwWin32ExitCode        = 0;  
	m_ServiceStatus.dwServiceSpecificExitCode = 0;  
	m_ServiceStatus.dwCheckPoint           = 0;  
	m_ServiceStatus.dwWaitHint             = 0;  

	m_ServiceStatusHandle = RegisterServiceCtrlHandler(szClientServiceName,ServiceCtrlHandler);   
	if (m_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)  
	{  
		return;  
	}
	m_ServiceStatus.dwCurrentState         = SERVICE_RUNNING;  
	m_ServiceStatus.dwCheckPoint           = 0;  
	m_ServiceStatus.dwWaitHint             = 0;   
	if (SetServiceStatus (m_ServiceStatusHandle,&m_ServiceStatus))  
	{
		running = TRUE;
		ServiceProc(argc, argv);//服务运行的主过程，主要修改这个函数就可以了
	}

    return;  
}

VOID WINAPI ServiceProc(DWORD argc, LPTSTR *argv) 
{
	HKEY hSubKey;
	DWORD cbData;

	myWSAStartup();
	
	InitializeCriticalSection(&gSection);//临界区

	memset(szConfigFile,0, sizeof(szConfigFile));
	memset(szLogFile,0, sizeof(szLogFile));
	if(ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SOFTWARE\\SafeWall\\Client",
		0,KEY_READ, &hSubKey))
	{
		return;
	}

	cbData = sizeof(szConfigFile);
	RegQueryValueExA(hSubKey,"config",NULL, NULL ,(BYTE*)szConfigFile ,&cbData);
	cbData = sizeof(szLogFile);
	RegQueryValueExA(hSubKey,"log",NULL, NULL ,(BYTE*)szLogFile ,&cbData);
	RegCloseKey(hSubKey);
	
	_beginthreadex(NULL, 0 , KeepActivityThread, (LPVOID)NULL, 0,0);

	return;
}

VOID OnStop()
{
	OnCancelLogon();
	m_ServiceStatus.dwWin32ExitCode = 0;  
	m_ServiceStatus.dwCurrentState    = SERVICE_STOPPED;     
	m_ServiceStatus.dwCheckPoint      = 0;  
	m_ServiceStatus.dwWaitHint        = 0;     
	SetServiceStatus (m_ServiceStatusHandle,&m_ServiceStatus);
	running=FALSE;
	myWSACleanup();
	DeleteCriticalSection(&gSection);
}

VOID WINAPI ServiceCtrlHandler(DWORD Opcode) 
{
	LPMYSERVCONTEXT lpContext=NULL;
	HANDLE hMap;
	switch(Opcode)  
	{   
	case SERVICE_CONTROL_PAUSE:
		{
			m_ServiceStatus.dwCurrentState = SERVICE_PAUSED;  
		}break;
	case SERVICE_CONTROL_CONTINUE:
		{
			m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;  
		}break; 
	case MY_SERVICE_CONTROL_LOGON:
		{
			mylog(szLogFile,"MY_SERVICE_CONTROL_LOGON");
			OpenMapFile(&hMap,&lpContext, szClientMapName);
			lpContext->error = OnLogon(lpContext);
			ReleaseMapFile(hMap,lpContext);
		}break;
	case MY_SERVICE_CONTROL_GET_USERID:
		{
			//mylog(szLogFile,"MY_SERVICE_CONTROL_GET_USERID");
			OpenMapFile(&hMap,&lpContext, szClientMapName);
			lpContext->error = OnGetUserID(lpContext);
			ReleaseMapFile(hMap,lpContext);
		}break;
	case MY_SERVICE_CONTROL_CANCEL_LOGON:
		{
			mylog(szLogFile,"MY_SERVICE_CONTROL_CANCEL_LOGON");
			OpenMapFile(&hMap,&lpContext, szClientMapName);
			lpContext->error = OnCancelLogon();
			ReleaseMapFile(hMap,lpContext);
		}break;
	case MY_SERVICE_CONTROL_CHANGE_PASSWORD:
		{
			mylog(szLogFile,"MY_SERVICE_CONTROL_CHANGE_PASSWORD");
			OpenMapFile(&hMap,&lpContext, szClientMapName);
			lpContext->error = OnChangePassword(lpContext);
			ReleaseMapFile(hMap,lpContext);
		}break;
	case MY_SERVICE_CONTROL_GET_ACCESS:
		{
			mylog(szLogFile,"MY_SERVICE_CONTROL_GET_ACCESS");
			OpenMapFile(&hMap,&lpContext, szClientMapName);
			lpContext->error = OnGetAccess();
			ReleaseMapFile(hMap,lpContext);
		}break;
	case SERVICE_CONTROL_STOP:
		{
			mylog(szLogFile,"SERVICE_CONTROL_STOP");
			OnStop();
		}break;
	case SERVICE_CONTROL_INTERROGATE:  
		break;  
		  
	}       
	return;  
} 


BOOL InstallService(WCHAR * ServiceName) 
{ 
	BOOL bRet;
	WCHAR szPath[MAX_PATH];
	SC_HANDLE schSCManager,schService;
	HKEY hOrderSubKey;
	LPCWCHAR lpszBinaryPathName;
	WCHAR SubKeyPath[MAX_PATH];
	DWORD cbData, cbType;
	BYTE * buffer;
	int buflen = (16 * 1024);

	GetModuleFileName( NULL, szPath, MAX_PATH );
	//wprintf(L"File Path:%s",szPath);
	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);   

	if (schSCManager == NULL)
	{
		return false; 
	}
    
	lpszBinaryPathName=szPath; 

	schService = CreateService((struct SC_HANDLE__ *)schSCManager,ServiceName,ServiceName, // service name to display  
		SERVICE_ALL_ACCESS,          // desired access  
		SERVICE_WIN32_OWN_PROCESS, // service type  
		SERVICE_AUTO_START,        // start type  
		SERVICE_ERROR_NORMAL,        // error control type  
		lpszBinaryPathName,          // service's binary  
		NULL,                        // no load ordering group  
		NULL,                        // no tag identifier  
		NULL,                        // no dependencies  
		NULL,                        // LocalSystem account  
		NULL);                       // no password  

	if (schService == NULL)  
	{
		return false; 
	}

	SERVICE_DESCRIPTION   sdBuf; 
	sdBuf.lpDescription   =   szClientDescriptionName ; 
	ChangeServiceConfig2( schService, SERVICE_CONFIG_DESCRIPTION, &sdBuf); 
	bRet = StartService(schService, NULL, NULL);
	CloseServiceHandle((struct SC_HANDLE__ *)schService);

	//if(!bRet) return bRet;
	////到这里就是服务注册成功了，接下来修改服务的启动顺序
	//buffer = (BYTE*) malloc(buflen);
	//cbType = REG_MULTI_SZ;
	//cbData = buflen;
	//LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Control\\ServiceGroupOrder",
	//	0,KEY_READ| KEY_WRITE, &hOrderSubKey);
	//RegQueryValueEx(hOrderSubKey,L"List2",NULL, NULL ,buffer ,&cbData);
	////RegGetValue(hOrderSubKey,NULL,L"List", RRF_RT_REG_MULTI_SZ,NULL,buffer, &cbData);
	//
	//int len = InsertOnceName((WCHAR*)buffer,cbData/sizeof(WCHAR), buflen/sizeof(WCHAR), L"SafeWall");
	//if(1)
	//{
	//	RegSetValueEx(hOrderSubKey,L"List2", NULL, REG_MULTI_SZ, buffer, len*sizeof(WCHAR));
	//}
	//RegCloseKey(hOrderSubKey);
	/*wsprintf(SubKeyPath,L"SYSTEM\\CurrentControlSet\\Control\\ServiceGroupOrder\\%s", ServiceName);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, SubKeyPath,0, KEY_WRITE, &hSubKey);
	RegSetValueEx(hSubKey, L"Group",NULL, REG_SZ, L"FSFilter Encryption",sizeof(L"FSFilter Encryption"));
	RegSetValue(hSubKey,L"Tag",REG_DWORD, L"0x00000001", sizeof(L"0x00000001"));*/

	return true; 
} 

BOOL UnInstallService(WCHAR * ServiceName) 
{
	BOOL bRet = FALSE;
    HANDLE schSCManager; 
    SC_HANDLE hService; 

	do
	{
		schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if (schSCManager == NULL)	return bRet;

		hService=OpenService((struct SC_HANDLE__ *)schSCManager,ServiceName,SERVICE_ALL_ACCESS);
		if (hService == NULL)	break;

		if(ControlService(hService,SERVICE_CONTROL_STOP, &m_ServiceStatus) == FALSE) break;
		if(DeleteService(hService)== FALSE) break;
		bRet = TRUE;
	}while(FALSE);
	CloseServiceHandle(hService);
	return bRet;

}

