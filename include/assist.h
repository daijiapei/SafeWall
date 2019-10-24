

#ifndef __ASSIST_HPP
#define __ASSIST_HPP


#include <stdio.h>
#include <Windows.h>
#include "shlobj.h"
#include <winternl.h>


typedef NTSTATUS (CALLBACK *PFN_NTQUERYINFORMATIONPROCESS)(
   HANDLE ProcessHandle, 
   PROCESSINFOCLASS ProcessInformationClass,
   PVOID ProcessInformation,
   ULONG ProcessInformationLength,
   PULONG ReturnLength OPTIONAL
   );


#ifdef __cplusplus
extern "C" {
#endif

BOOL mylog(IN char *file,IN const char * text);
BOOL FileNameToVolumePathName(IN LPCWSTR FileName,OUT LPWSTR VolumePathName,IN DWORD cchMax);
BOOL GetShortcutTarget(IN LPWSTR lpwLnkFile,OUT LPWSTR lpwTarFile);
DWORD WINAPI FollowParentProcess(LPVOID hMainWnd);
int InsertOnceName(WCHAR * tagString,int tagstrLenght,int tagAllLenght, WCHAR * pName);

int bytes_to_format_chars(byte * src, int srclen, char * buf, int buflen);
int format_chars_to_bytes(char * src, byte * buf, int buflen);

#ifdef __cplusplus
}
#endif

#define HoldParentFinish(hMainWnd) CloseHandle(CreateThread(NULL,0,FollowParentProcess,(HWND)(hMainWnd), 0, NULL))

#endif