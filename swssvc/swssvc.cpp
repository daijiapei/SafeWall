
#include "swssvc.h"
//#include <locale.h>
BOOL running = FALSE; //是否保持运行
//Json::Value root;
USERLIST_HERDER gUserList = {0};
char szLogFile[MAX_PATH] = {0};
FILEEVENT PolicyFile = {0};
FILEEVENT UserFile = {0};
FILEEVENT IcUserFile = {0};
SERVICE_STATUS m_ServiceStatus = {0}; 
SERVICE_STATUS_HANDLE m_ServiceStatusHandle = {0};

int main(int argc, char* argv[])
{
	//_wsetlocale(LC_ALL, L"chs");
    if(argc>1) 
    { 
        if( strcmp(argv[1],"-i")==0) 
        { 
			if(InstallService(szServerServiceName)) 
			printf("\n 服务安装成功\n"); 
			else 
			printf("\n 服务安装失败\n"); 
        } 
        else if(strcmp(argv[1],"-d")==0) 
        { 
            if(UnInstallService(szServerServiceName)) 
            printf("\n 服务卸载成功\n"); 
            else 
            printf("\n 服务卸载失败\n"); 
        }
		else if(strcmp(argv[1], "-r")==0)
		{

		}
        else 
        { 
            printf("\n 未知的命令\n  重新运行输入-i将安装服务\n重新运行输入-d将卸载服务\n"); 
			//printf("your input: count=%d; str1= %s; str2= %s;\n",argc,argv[0],argv[1]); 
        }
	}
    else
    {
        SERVICE_TABLE_ENTRY DispatchTable[]={{szServerServiceName,ServiceMain},{NULL,NULL}};   
        StartServiceCtrlDispatcher(DispatchTable);  
    } 
    
    return 0; 
}

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv) 
{ 
	//     DWORD status;  
	//     DWORD specificError;  
	HKEY hSubKey;
	DWORD cbData;
	char folder[MAX_PATH];
	m_ServiceStatus.dwServiceType          = SERVICE_WIN32;  
	m_ServiceStatus.dwCurrentState         = SERVICE_START_PENDING;  
	m_ServiceStatus.dwControlsAccepted     = SERVICE_ACCEPT_STOP;  
	m_ServiceStatus.dwWin32ExitCode        = 0;  
	m_ServiceStatus.dwServiceSpecificExitCode = 0;  
	m_ServiceStatus.dwCheckPoint           = 0;
	m_ServiceStatus.dwWaitHint             = 0;

	m_ServiceStatusHandle = RegisterServiceCtrlHandler(szServerServiceName,ServiceCtrlHandler);   
	if (m_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)  
	{  
		return;  
	}      
	m_ServiceStatus.dwCurrentState         = SERVICE_RUNNING;  
	m_ServiceStatus.dwCheckPoint           = 0;  
	m_ServiceStatus.dwWaitHint             = 0;   
	if (SetServiceStatus (m_ServiceStatusHandle,&m_ServiceStatus))  
	{
		cbData = sizeof(folder);
		memset(folder,0, sizeof(folder));
		if(ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SOFTWARE\\SafeWall\\Server",
			0,KEY_READ, &hSubKey))
		{
			return;
		}

		if(ERROR_SUCCESS != RegQueryValueExA(hSubKey,"Folder",NULL, NULL ,(BYTE*)folder ,&cbData))
		{
			RegCloseKey(hSubKey);
			return;
		}
		
		RegCloseKey(hSubKey);
		
		if(cbData!=0 && folder[cbData -2] != '\\')
		{
			folder[cbData-1] = '\\';
			folder[cbData] = '\0';
		}

		sprintf(szLogFile,"%s%s",folder, "server.log");
		sprintf(PolicyFile.FilePath,"%s%s",folder, "file\\safewall.pli");
		sprintf(UserFile.FilePath,"%s%s",folder, "file\\user.pli");
		sprintf(IcUserFile.FilePath,"%s%s",folder, "file\\icuser.pli");

		InitializeCriticalSection(&PolicyFile.section);//临界区
		InitializeCriticalSection(&UserFile.section);//临界区
		InitializeCriticalSection(&IcUserFile.section);//临界区

		PolicyFile.root = new Json::Value();
		UserFile.root = new Json::Value();
		IcUserFile.root = new Json::Value();

		ReadPolicy(&PolicyFile);
		ReadPolicy(&UserFile);
		ReadPolicy(&IcUserFile);

		InitUserList(&gUserList);
		ServiceProc(argc, argv);//服务运行的主过程，主要修改这个函数就可以了
	}

    return;
}

VOID WINAPI ServiceCtrlHandler(DWORD Opcode) 
{ 
	switch(Opcode)  
	{  
		case SERVICE_CONTROL_PAUSE:  
			m_ServiceStatus.dwCurrentState = SERVICE_PAUSED;  
			break;  

		case SERVICE_CONTROL_CONTINUE:  
			m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;  
			break;  
		case MY_SERVICE_CONTROL_REFRESH_POLICY:
			break;
			//到这里就表示失败了
			//mylog("策略文件打开失败");
		case SERVICE_CONTROL_STOP:  
			m_ServiceStatus.dwWin32ExitCode = 0;  
			m_ServiceStatus.dwCurrentState    = SERVICE_STOPPED;     
			m_ServiceStatus.dwCheckPoint      = 0;  
			m_ServiceStatus.dwWaitHint        = 0;     
			SetServiceStatus (m_ServiceStatusHandle,&m_ServiceStatus); 
			running=FALSE;

			PolicyFile.FilePath[0] = '\0';
			UserFile.FilePath[0] = '\0';
			IcUserFile.FilePath[0] = '\0';
			DeleteCriticalSection(&PolicyFile.section);
			DeleteCriticalSection(&UserFile.section);
			DeleteCriticalSection(&IcUserFile.section);

			delete PolicyFile.root;
			delete UserFile.root;
			delete IcUserFile.root;

			ReleaseUserList(&gUserList);

			//mylog("服务暂停");
			break; 

		case SERVICE_CONTROL_INTERROGATE:  
			break;  
		  
	}       
	return;  
} 

BOOL InstallService(WCHAR * ServiceName) 
{ 
	WCHAR szPath[MAX_PATH];
	SC_HANDLE schSCManager,schService; 

	GetModuleFileName( NULL, szPath, MAX_PATH );
	//wprintf(L"File Path:%s",szPath);
	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);   

	if (schSCManager == NULL)
	{
		return false; 
	}
    
	LPCWCHAR lpszBinaryPathName=szPath; 

	schService = CreateService((struct SC_HANDLE__ *)schSCManager,ServiceName,ServiceName,             // service name to display  
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
	sdBuf.lpDescription   =   szServerDescriptionName ; 
	ChangeServiceConfig2( schService, SERVICE_CONFIG_DESCRIPTION, &sdBuf); 
	StartService(schService, NULL, NULL);
	CloseServiceHandle((struct SC_HANDLE__ *)schService);  
	return true; 
} 

BOOL UnInstallService(WCHAR * ServiceName) 
{ 
    HANDLE schSCManager; 
    SC_HANDLE hService; 
    schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS); 

    if (schSCManager == NULL)  
        return FALSE;  
    hService=OpenService((struct SC_HANDLE__ *)schSCManager,ServiceName,SERVICE_ALL_ACCESS); 
    if (hService == NULL)  
        return FALSE; 
	if(ControlService(hService,SERVICE_CONTROL_STOP, &m_ServiceStatus) == FALSE)
		return FALSE;
    if(DeleteService(hService)== FALSE) 
        return FALSE; 
    if(CloseServiceHandle(hService)== FALSE) 
        return FALSE; 
    else 
        return TRUE; 
}