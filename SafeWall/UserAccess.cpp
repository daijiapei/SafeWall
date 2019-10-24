//
//#include "SafeWall.h"
//
//ACCESS_PACKAGE  ProcessAccess;
//ACCESS_PACKAGE  UserAccess;
//ACCESS_PACKAGE  FileAccess;
//
//BOOLEAN StriContainsW(wchar_t *longStr, int size, wchar_t *shortStr);
//BOOLEAN StriContainsA(char *longStr, int size, char *shortStr);
////用户私有权限
//#pragma PAGEDCODE
//void InitializeUserAccess()
//{
//	KeInitializeSpinLock(&UserAccess.locker);
//	UserAccess.package = NULL;
//}
//
//
//#pragma PAGEDCODE
//void ReleaseUserAccess()
//{
//	//关机才会调用该函数，所以干脆什么也不做
//}
//
//#pragma PAGEDCODE
//void SetUserAccess(void * buffer, int lenght)
//{
//}
//
////全局进程权限
//#pragma PAGEDCODE
//void InitializeProcessAccess()
//{
//	KeInitializeSpinLock(&ProcessAccess.locker);
//	ProcessAccess.package = NULL;
//}
//
//#pragma PAGEDCODE
//void ReleaseProcessAccess()
//{
//	//关机才会调用该函数，所以干脆什么也不做
//}
//
//
//#pragma PAGEDCODE
//BOOLEAN SetProcessAccess(void * buffer, int lenght)
//{
//	BOOLEAN bRet = FALSE;
//	KLOCK_QUEUE_HANDLE handle;
//	PROCESS_ACCESS * stu;
//	int size;
//	KeAcquireInStackQueuedSpinLock(&ProcessAccess.locker, &handle);
//
//	if(ProcessAccess.package)
//	{
//		ExFreePool(ProcessAccess.package);
//		ProcessAccess.package = NULL;
//	}
//
//	ProcessAccess.package = ExAllocatePool(NonPagedPool , lenght);
//	if(ProcessAccess.package)
//	{
//		memcpy(ProcessAccess.package, buffer,lenght);
//		stu = (PROCESS_ACCESS*)ProcessAccess.package;
//
//		while(stu->ProcessName + stu->lenght != (char*)ProcessAccess.package + lenght)
//		{
//			stu->next = (PROCESS_ACCESS*)stu->ProcessName + stu->lenght;
//			KdPrint(("ProcessName=%s",stu->ProcessName));
//
//			size = stu->lenght;
//			stu->ProcessName[size] = '\0';
//			while(size--)
//			{
//				if(stu->ProcessName[size] == '\t'|| stu->ProcessName[size] == '\r'
//					|| stu->ProcessName[size] == '\n' || stu->ProcessName[size] == ';'
//					|| stu->ProcessName[size] == ' ')
//				{
//					stu->ProcessName[size] = '\0';
//				}
//			}
//
//			stu = stu->next;
//		}
//		stu->next = NULL;
//		bRet = TRUE;
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return bRet;
//}
//
//
////全局文件权限
//
//
//#pragma PAGEDCODE
//void InitializeFileAcces()
//{
//	KeInitializeSpinLock(&FileAccess.locker);
//	FileAccess.package = NULL;
//}
//
//#pragma PAGEDCODE
//void ReleaseFileAccess()
//{
//	//关机才会调用该函数，所以干脆什么也不做
//}
//
////设置文件权限
//#pragma PAGEDCODE
//BOOLEAN SetFileAccess(char * buffer, int lenght)
//{
//	BOOLEAN bRet = FALSE;
//	KLOCK_QUEUE_HANDLE handle;
//	FILE_ACCESS * access;
//	int size;
//
//	KeAcquireInStackQueuedSpinLock(&FileAccess.locker, &handle);
//	if(FileAccess.package)
//	{
//		ExFreePool(FileAccess.package);
//		FileAccess.package = NULL;
//	}
//	FileAccess.package = ExAllocatePool(NonPagedPool , lenght+sizeof(FILE_ACCESS));
//	if(FileAccess.package)
//	{
//		access = (FILE_ACCESS *)FileAccess.package;
//		size = access->size = lenght/ sizeof(wchar_t);
//		memcpy(access->suffix,buffer, lenght);
//
//		//将最后的字符和特殊符号置零
//		access->suffix[size] = L'\0';
//		while(size--)
//		{
//			if(access->suffix[size] == L'\t'|| access->suffix[size] == L'\r'
//				|| access->suffix[size] == L'\n' || access->suffix[size] == L';'
//				|| access->suffix[size] == L' ')
//			{
//				access->suffix[size] = L'\0';
//			}
//		}
//
//		bRet = TRUE;
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return bRet;
//}
//
////判断文件后缀是否属于加密范围
//#pragma PAGEDCODE
//BOOLEAN FileSuffixIsInvolve(PUNICODE_STRING File)
//{
//	wchar_t suffix[32] = {0};//最高支持31位后缀
//	wchar_t wc;
//	int len = File->Length / 2;
//	int i =0, j = 0, count = 0;
//	FILE_ACCESS * access ;
//	KLOCK_QUEUE_HANDLE handle;
//	BOOLEAN bRet = FALSE;
//	
//	while(--len)
//	{
//		if(File->Buffer[len] == L'.')
//		{
//			break;
//		}
//
//		if(File->Buffer[len] == L'\\')
//		{
//			//该文件没有后缀
//			return FALSE;
//		}
//		suffix[count++] = File->Buffer[len];
//	}
//
//	//将文件名字符还原
//	for(i =0, j=count-1; i < j; i++,j--)
//	{
//		wc = suffix[i];
//		suffix[i] = suffix[j];
//		suffix[j] = wc;
//	}
//	KeAcquireInStackQueuedSpinLock(&FileAccess.locker, &handle);
//	access = (FILE_ACCESS*)FileAccess.package;
//	bRet = StriContainsW(access->suffix, access->size, suffix);
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return bRet;
//}
//
////longStr中是否包含shortStr
////#pragma PAGEDCODE
////BOOLEAN StriContainsW(wchar_t *longStr, int size, wchar_t *shortStr)
////{
////	int i =0;
////	BOOLEAN fir = TRUE;
////	wchar_t *p = NULL;
////	do
////	{
////		p = longStr + i;
////
////		if(fir == TRUE && *p == *shortStr && 0 == wcsicmp(p, shortStr))
////		{
////			return TRUE;
////		}
////		
////		fir = (longStr[i] == L'\0') ? TRUE : FALSE;
////
////	}while(size > (++i));
////	return FALSE;
////}
//
////#pragma PAGEDCODE
////BOOLEAN StriContainsA(char *longStr, int size, char *shortStr)
////{
////	int i =0;
////	BOOLEAN fir = TRUE;
////	char *p = NULL;
////	do
////	{
////		p = longStr + i;
////
////		if(fir == TRUE && *p == *shortStr && 0 == stricmp(p, shortStr))
////		{
////			return TRUE;
////		}
////		
////		fir = (longStr[i] == '\0') ? TRUE : FALSE;
////
////	}while(size > (++i));
////
////	return FALSE;
////}