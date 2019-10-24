
#include "SafeWall.h"
#include "fat.h"

#pragma PAGEDCODE
NTSTATUS DriverReadDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp)
{
	NTSTATUS status;
	KEVENT waitEvent;
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
	PSAFEWALL_FILE_LIST pFileList;
	PSECTION_OBJECT_POINTERS SectionObjectPointer;
	char * buffer;

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("我的驱动产生了一条读取文件的消息\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;

	pFileList = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext);
	FileFlags = pFileList ? pFileList->flags : NULL ;

	if(!(FileFlags & SAFEWALL_FLAG_OBJECT) ||
		!(pIrp->Flags & (IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO | IRP_NOCACHE)))
	{
		//不包含加密对象，或者非硬盘读写，直接发下层
		IoSkipCurrentIrpStackLocation( pIrp );
		status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
		return status;
	}
	//到这里就是我们要处理的机密对象了
	
	PFSRTL_COMMON_FCB_HEADER pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

	GetCurrentProcessName(&ProcName);
	stack->Parameters.Read.ByteOffset.QuadPart += SAFEWALL_OBJECT_SIZE;
	pFileObject->CurrentByteOffset.QuadPart += SAFEWALL_OBJECT_SIZE;
	/*pFcb->FileSize.QuadPart += SAFEWALL_OBJECT_SIZE;
	pFcb->ValidDataLength.QuadPart += SAFEWALL_OBJECT_SIZE;*/
	KdPrint(("开始处理读取文件消息"));
	KdPrint(("flags=0x%08x .ProcName=%wZ.Read-pFileObject->CurrentByteOffset:%lld, ",\
		pIrp->Flags,&ProcName,pFileObject->CurrentByteOffset.QuadPart));
	KdPrint(("AllocationSize:%d, FileSize:%d, ValidDataLength:%d",\
		pFcb->AllocationSize.LowPart,pFcb->FileSize.LowPart,pFcb->ValidDataLength.LowPart));
	SectionObjectPointer = pFileObject->SectionObjectPointer;
	KdPrint(("DataSectionObject 0x%08x", SectionObjectPointer->DataSectionObject));
	KdPrint(("ImageSectionObject 0x%08x", SectionObjectPointer->ImageSectionObject));
	KdPrint(("SharedCacheMap 0x%08x", SectionObjectPointer->SharedCacheMap));
    //初始化事件对象，设置完成例程。
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	//当在IoCallDriver有后续操作时，需要使用IoCopyCurrentIrpStackLocationToNext
    IoCopyCurrentIrpStackLocationToNext( pIrp );
	//设置完成例程
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
    if (STATUS_PENDING == status)//若状态是挂起
    {
        KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
    }

	//先还原，再判断是否发生错误
	stack->Parameters.Read.ByteOffset.QuadPart -= SAFEWALL_OBJECT_SIZE;
	pFileObject->CurrentByteOffset.QuadPart -= SAFEWALL_OBJECT_SIZE;
	//pFcb->ValidDataLength.QuadPart -= SAFEWALL_OBJECT_SIZE;
	if(!NT_SUCCESS(pIrp->IoStatus.Status))
	{
		status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return status;
	}
	SectionObjectPointer = pFileObject->SectionObjectPointer;

	//到这里就是读取成功了，进行解密
	KdPrint(("DataSectionObject 0x%08x", SectionObjectPointer->DataSectionObject));
	KdPrint(("ImageSectionObject 0x%08x", SectionObjectPointer->ImageSectionObject));
	KdPrint(("SharedCacheMap 0x%08x", SectionObjectPointer->SharedCacheMap));

	//KdPrint(("lenght %d , %d. offset %lld", Segment->Length.HighPart,Segment->Length.LowPart,Segment->Image.FileOffset));
	//KdPrint(("Segment = %s", Segment->Image.VirtualAddress));
	//KdPrint(("Segment = %s", Segment->Image.VirtualAddress));

	KdPrint(("%wZ 包含了文件组标志,fileflags=0x%08x", &pFileObject->FileName,pFileObject->Flags));
	KdPrint(("Read-pFileObject->CurrentByteOffset:%lld",pFileObject->CurrentByteOffset.QuadPart));
	KdPrint(("Information%lld",pIrp->IoStatus.Information));
	KdPrint(("AllocationSize:%d, FileSize:%d, ValidDataLength:%d",\
		pFcb->AllocationSize.LowPart,pFcb->FileSize.LowPart,pFcb->ValidDataLength.LowPart));
	//取得BUFFER并解密
	/*pFcb->FileSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
	pFcb->ValidDataLength.QuadPart -= SAFEWALL_OBJECT_SIZE;*/
	ASSERT(pIrp->MdlAddress != NULL || pIrp->UserBuffer != NULL);
	if(pIrp->MdlAddress != NULL)
	{
		buffer = (char *)MmGetSystemAddressForMdlSafe(pIrp->MdlAddress,NormalPagePriority);
	}
	else
	{
		buffer = (char *)pIrp->UserBuffer;
	}
		
	for(ULONG_PTR i = 0; pIrp->IoStatus.Information > i; i++)
	{
		buffer[i] = ~buffer[i];
	}

	CcIsFileCached(pFileObject);
	KdPrint(("结束处理读取文件消息"));
    status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	//KdPrint(("离开 DriverReadDispatchRoutine\n"));
    return status;
}
