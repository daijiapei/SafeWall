
#include "SafeWall.h"

#pragma PAGEDCODE
NTSTATUS  DriverCloseDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
    NTSTATUS status;
	UNICODE_STRING ProcName;
	UNICODE_STRING FilePath;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;

    PAGED_CODE();

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject))
    {
		KdPrint(("我的驱动产生了一条关闭文件的消息\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;
	
	/*if(pFileObject == NULL)
	{
		KdPrint(("pFileObject是空的"));
	}
	else
	{
		FCB * pFcb = (FCB*)pFileObject->FsContext;
		PSAFEWALL_FILE_LIST pFileList = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext);
		if(pFileList != NULL && (pFileList->flags & SAFEWALL_FLAG_FILEGROUP))
		{
			KdPrint(("pFileList我的驱动产生了一条关闭文件的消息。UncleanCount:%d。OpenCount:%d。NonCachedUncleanCount:%d", \
				pFcb->UncleanCount,pFcb->OpenCount,pFcb->NonCachedUncleanCount));
		}
	}*/

	/*RtlAppendUnicodeToString(&tarFileName, L"config.txt");
	GetFileNameForPath(&pFileObject->FileName , &srcFileName);
	if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		GetCurrentProcessName(&ProcName);
		KdPrint(("先手——%02x一个文件：%wZ ；进程 %wZ；  Options：0x%08x",stack->MajorFunction, \
			&pFileObject->FileName,&ProcName, stack->Parameters.Create.Options ));
	}*/

	/*RtlAppendUnicodeToString(&tarFileName, L"test.txt");
	GetFileNameForPath(&pFileObject->FileName , &srcFileName);
	if(RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return status;
	}*/

    //初始化事件对象，设置完成例程。
    KEVENT waitEvent;
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	//当在IoCallDriver有后续操作时，需要使用IoCopyCurrentIrpStackLocationToNext
    IoCopyCurrentIrpStackLocationToNext(pIrp);

	//设置完成例程
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
    if (STATUS_PENDING == status)//若状态是 挂起
    {
        NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
    }

	//if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	//{
	//	GetCurrentProcessName(&ProcName);
	//	KdPrint(("倒手——%02x一个文件：%wZ ；进程 %wZ；  Options：0x%08x",stack->MajorFunction, \
	//		&pFileObject->FileName,&ProcName, stack->Parameters.Create.Options ));
	//}

    status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return status;

}
