
#include "SafeWall.h"

ULONG GetCurrentProcessName(PUNICODE_STRING name)
{
	PEPROCESS curproc;

	ULONG i, need_len;
	ANSI_STRING ansi_name;
	//g_process_name_offset ：进程名称在PROCESS结构体中的偏移量
	if(g_process_name_offset == 0)
		return 0;

	curproc = PsGetCurrentProcess();//取得PROCESS结构体指针
	RtlInitAnsiString (&ansi_name,((PCHAR)curproc + g_process_name_offset));
	need_len = RtlAnsiStringToUnicodeSize(&ansi_name);
	if(need_len > name->MaximumLength)
	{
		return RtlAnsiStringToUnicodeSize(&ansi_name);
	}
	RtlAnsiStringToUnicodeString(name, &ansi_name,FALSE);
	return need_len;
}

ULONG GetCurrentProcessNameA(__out char * name)
{
	PEPROCESS curproc;

	ULONG len;
	if(g_process_name_offset == 0)
		return 0;

	curproc = PsGetCurrentProcess();//取得PROCESS结构体指针
	strcpy(name, (char*)curproc + g_process_name_offset);

	return strlen(name);
}