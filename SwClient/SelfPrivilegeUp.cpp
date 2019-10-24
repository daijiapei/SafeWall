
#include "SwClient.h"

//提高进程权限
BOOL SelfPrivilegeUp(void)
{
	BOOL   ret;
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	ret = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	if(!ret)
	{
		return FALSE;
	}

	ret = LookupPrivilegeValue(NULL, SE_SYSTEM_ENVIRONMENT_NAME, &luid);
	if(!ret)
	{
		CloseHandle(hToken);
		return FALSE;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	ret = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if(!ret)
	{
		CloseHandle(hToken);
		return FALSE;
	}
	CloseHandle(hToken);
	return TRUE;
}
