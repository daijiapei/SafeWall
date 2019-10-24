

#include "SafeWall.h"

#pragma PAGEDCODE
NTSTATUS  DriverCleanUpDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
    NTSTATUS status;
	DWORD FileFlags = 0;
	DWORD ProcFlags = 0;
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * oldBuffer = NULL, * newBuffer = NULL;
	PMDL oldMdl = NULL, newMdl = NULL;
    PAGED_CODE();

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("我的驱动产生了一条清除文件的消息\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }
    
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);

	PFILE_OBJECT pFileObject = stack->FileObject;
	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

	//dwCreationDisposition = (stack->Parameters.Create.Options>>24);   
	//if((FILE_CREATE & dwCreationDisposition) && !(IRP_PAGING_IO & pIrp->Flags))
	
	RtlAppendUnicodeToString(&tarFileName, L"safe.txt");
	GetFileNameForPath(&pFileObject->FileName , &srcFileName);
	ULONG lenght;
	if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		GetCurrentProcessName(&ProcName);
		ProcFlags = TRUE;
		lenght =  stack->Parameters.Write.Length;IRP_CLOSE_OPERATION;
		if(NULL != pIrp->MdlAddress)
		{
			//将用户地址转换成内核地址
			oldMdl = pIrp->MdlAddress;
			oldBuffer = (char*)MmGetSystemAddressForMdlSafe(oldMdl,NormalPagePriority);
			//newMdl = MdlMemoryAlloc(lenght);//用户地址
			/*if(NULL == newMdl)
				newBuffer = NULL;*/
			//else//如果用户地址申请成功，再将它转换成内核地址
				//newBuffer = (char*)MmGetSystemAddressForMdlSafe(newMdl,NormalPagePriority);
		}
		else
		{
			oldBuffer = (char*)pIrp->UserBuffer;
			//newBuffer = (char*)ExAllocatePool(NonPagedPool,lenght);
		}
		
		//memcpy(newBuffer, oldBuffer, lenght);

		//for(int i=0; lenght > i; i++)
		//{
		//	newBuffer[i] = oldBuffer[i]+1;
		//}

		//if(pIrp->MdlAddress != NULL)
		//{
		//	pIrp->MdlAddress = newMdl;
		//}
		//else
		//{
		//	pIrp->UserBuffer = newBuffer;
		//}
		//KdPrint(("进入 Driver CleanUp 测试\n"));
		//KdPrint(("WriteOldBuffer =%s\n",oldBuffer));
		//KdPrint(("%wZ进程, flag=0x%08x",&ProcName,pIrp->Flags));
		//KdPrint(("AllocationSize:%d, FileSize:%d, ValidDataLength:%d",\
		//	pFcb->AllocationSize.LowPart,pFcb->FileSize.LowPart,pFcb->ValidDataLength.LowPart));
		//KdPrint(("Length = %d,ByteOffset=%lld\n",stack->Parameters.Write.Length,stack->Parameters.Write.ByteOffset.QuadPart));
	}
	//if(pFileObject == NULL)
	//{
	//	KdPrint(("pFileObject是空的"));
	//}
	//else
	//{
	//	FCB * pFcb = (FCB*)pFileObject->FsContext;
	//	PSAFEWALL_FILE_LIST pFileList = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext);
	//	if(pFileList != NULL && (pFileList->flags & SAFEWALL_FLAG_FILEGROUP))
	//	{

	//		KdPrint(("pFileList我的驱动产生了一条清除文件的消息。UncleanCount:%d。OpenCount:%d。NonCachedUncleanCount:%d", \
	//			pFcb->UncleanCount,pFcb->OpenCount,pFcb->NonCachedUncleanCount));
	//	}
	//}
	

    //初始化事件对象，设置完成例程。
    KEVENT waitEvent;
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	//当在IoCallDriver有后续操作时，需要使用IoCopyCurrentIrpStackLocationToNext
    IoCopyCurrentIrpStackLocationToNext( pIrp );

	//设置完成例程
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
    if (STATUS_PENDING == status)//若状态是 挂起
    {
        NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
    }

    status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return status;

}
