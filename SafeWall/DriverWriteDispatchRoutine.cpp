
#include "SafeWall.h"


PMDL MdlMemoryAlloc(ULONG length);
void MdlMemoryFree(PMDL mdl);

#pragma PAGEDCODE
NTSTATUS DriverWriteDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp)
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
	char * oldBuffer = NULL, * newBuffer = NULL;
	PMDL oldMdl = NULL, newMdl = NULL;
	ULONG lenght;
	PSAFEWALL_FILE_LIST pFileList;
	FILE_OBJECT FileObject;
	//KdPrint(("进入 DriverWriteDispatchRoutine\n"));
	
	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("我的驱动产生了一条写入文件的消息\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;
	PFSRTL_COMMON_FCB_HEADER pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;

	pFileList = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext);
	FileFlags = pFileList ? pFileList->flags : NULL;

	if(!(FileFlags & SAFEWALL_FLAG_OBJECT) ||
		!(pIrp->Flags & (IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO | IRP_NOCACHE)))
	{
		//不包含加密对象，或者非硬盘读写，直接发下层
		IoSkipCurrentIrpStackLocation( pIrp );
		status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
		return status;
	}

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

	IRP_PAGING_IO;
	GetCurrentProcessName(&ProcName);
	if((FileFlags & SAFEWALL_FLAG_FILEGROUP) &&
		(pIrp->Flags & (IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO | IRP_NOCACHE)))
	{
		KdPrint(("进程 进入函数 DriverWriteDispatchRoutine\n"));
		KdPrint(("%wZ进程, flag=0x%08x,fileflags=0x%08x",&ProcName,pIrp->Flags,pFileObject->Flags));
		lenght =  stack->Parameters.Write.Length;
		ASSERT(pIrp->MdlAddress != NULL || pIrp->UserBuffer != NULL);
		if(NULL != pIrp->MdlAddress)
		{
			//将用户地址转换成内核地址
			oldMdl = pIrp->MdlAddress;
			oldBuffer = (char*)MmGetSystemAddressForMdlSafe(oldMdl,NormalPagePriority);
			newMdl = MdlMemoryAlloc(lenght);//用户地址
			if(NULL == newMdl)
				newBuffer = NULL;
			else//如果用户地址申请成功，再将它转换成内核地址
				newBuffer = (char*)MmGetSystemAddressForMdlSafe(newMdl,NormalPagePriority);
		}
		else
		{
			oldBuffer = (char*)pIrp->UserBuffer;
			newBuffer = (char*)ExAllocatePool(NonPagedPool,lenght);
		}

		// 如果缓冲区分配失败了，直接退出即可。
		if(NULL == newBuffer)
		{
			pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest( pIrp, IO_NO_INCREMENT );
			return pIrp->IoStatus.Status;
		}

		//加密
		KdPrint(("AllocationSize:%d, FileSize:%d, ValidDataLength:%d",\
			pFcb->AllocationSize.LowPart,pFcb->FileSize.LowPart,pFcb->ValidDataLength.LowPart));
		KdPrint(("Length = %d,ByteOffset=%lld\n",stack->Parameters.Write.Length,stack->Parameters.Write.ByteOffset.QuadPart));
		KdPrint(("WriteBuffer =%s\n",oldBuffer));
		
		stack->Parameters.Write.ByteOffset.QuadPart += SAFEWALL_OBJECT_SIZE;
		pFileObject->CurrentByteOffset.QuadPart += SAFEWALL_OBJECT_SIZE;
		//pFcb->FileSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
		//pFcb->ValidDataLength.QuadPart -= SAFEWALL_OBJECT_SIZE;
		for(int i=0; lenght > i; i++)
		{
			newBuffer[i] = ~oldBuffer[i];
			//oldBuffer[i] = ~oldBuffer[i];
		}

		if(pIrp->MdlAddress != NULL)
		{
			pIrp->MdlAddress = newMdl;
		}
		else
		{
			pIrp->UserBuffer = newBuffer;
		}
	}

	FileObject = *pFileObject;
	RtlAppendUnicodeToString(&tarFileName, L"safe.txt");
	GetFileNameForPath(&FileObject.FileName , &srcFileName);
	ProcFlags = NULL;
	if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		ProcFlags = TRUE;
		lenght =  stack->Parameters.Write.Length;
		if(NULL != pIrp->MdlAddress)
		{
			//将用户地址转换成内核地址
			oldMdl = pIrp->MdlAddress;
			oldBuffer = (char*)MmGetSystemAddressForMdlSafe(oldMdl,NormalPagePriority);
			newMdl = MdlMemoryAlloc(lenght);//用户地址
			if(NULL == newMdl)
				newBuffer = NULL;
			else//如果用户地址申请成功，再将它转换成内核地址
				newBuffer = (char*)MmGetSystemAddressForMdlSafe(newMdl,NormalPagePriority);
		}
		else
		{
			oldBuffer = (char*)pIrp->UserBuffer;
			newBuffer = (char*)ExAllocatePool(NonPagedPool,lenght);
		}
		
		//memcpy(newBuffer, oldBuffer, lenght);

		for(int i=0; lenght > i; i++)
		{
			newBuffer[i] = oldBuffer[i]+1;
		}

		if(pIrp->MdlAddress != NULL)
		{
			pIrp->MdlAddress = newMdl;
		}
		else
		{
			pIrp->UserBuffer = newBuffer;
		}
		KdPrint(("进入写入测试\n"));
		KdPrint(("WriteOldBuffer =%s\n",oldBuffer));
		KdPrint(("%wZ进程, flag=0x%08x, fileflag=%0x%08x",&ProcName,pIrp->Flags,pFileObject->Flags));
		KdPrint(("AllocationSize:%d, FileSize:%d, ValidDataLength:%d",\
			pFcb->AllocationSize.LowPart,pFcb->FileSize.LowPart,pFcb->ValidDataLength.LowPart));
		KdPrint(("Length = %d,ByteOffset=%lld\n",stack->Parameters.Write.Length,stack->Parameters.Write.ByteOffset.QuadPart));
	}


    //初始化事件对象，设置完成例程。
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	//当在IoCallDriver有后续操作时，需要使用IoCopyCurrentIrpStackLocationToNext
    IoCopyCurrentIrpStackLocationToNext( pIrp );

	//设置完成例程
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
    if (STATUS_PENDING == status)//若状态是 挂起
    {
        KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
    }
	
	//FO_FILE_OPEN;
	if(ProcFlags)
	{
		KdPrint(("Length = %d,ByteOffset=%lld\n",stack->Parameters.Write.Length,stack->Parameters.Write.ByteOffset.QuadPart));
		KdPrint(("WriteNewBuffer =%s\n",newBuffer));
		KdPrint(("CurrentByteOffset:%lld\n",pFileObject->CurrentByteOffset.QuadPart));
		KdPrint(("AllocationSize:%d, FileSize:%d, ValidDataLength:%d",\
			pFcb->AllocationSize.LowPart,pFcb->FileSize.LowPart,pFcb->ValidDataLength.LowPart));
		KdPrint(("离开写入测试\n"));
		if(newMdl != NULL)
		{
			pIrp->MdlAddress = oldMdl;
			MdlMemoryFree(newMdl);
		}
		else
		{
			pIrp->UserBuffer = oldBuffer;
			ExFreePool(newBuffer);
		}
	}

	//不管是否发生错误，都必须还原
	if((FileFlags & SAFEWALL_FLAG_FILEGROUP))
	{
		//还原
		//for(int i=0; lenght > i; i++)
		//{
		//	//newBuffer[i] = ~oldBuffer[i];
		//	oldBuffer[i] = ~oldBuffer[i];
		//}
		//不管是否成功，这些都是必须还原的
		pFileObject->CurrentByteOffset.QuadPart -= SAFEWALL_OBJECT_SIZE;
		stack->Parameters.Write.ByteOffset.QuadPart -= SAFEWALL_OBJECT_SIZE;
		//pFcb->FileSize.QuadPart += SAFEWALL_OBJECT_SIZE;
		//pFcb->ValidDataLength.QuadPart += SAFEWALL_OBJECT_SIZE;
		
		KdPrint(("Length = %d,ByteOffset=%lld\n",stack->Parameters.Write.Length,stack->Parameters.Write.ByteOffset.QuadPart));
		KdPrint(("status = 0x%08x .WriteBuffer =%s\n",status,oldBuffer));
		KdPrint(("CurrentByteOffset:%lld\n",pFileObject->CurrentByteOffset.QuadPart));
		KdPrint(("AllocationSize:%d, FileSize:%d, ValidDataLength:%d",\
			pFcb->AllocationSize.LowPart,pFcb->FileSize.LowPart,pFcb->ValidDataLength.LowPart));
		KdPrint(("离开函数 DriverWriteDispatchRoutine\n"));
		if(newMdl != NULL)
		{
			pIrp->MdlAddress = oldMdl;
			MdlMemoryFree(newMdl);
		}
		else
		{
			pIrp->UserBuffer = oldBuffer;
			ExFreePool(newBuffer);
		}
	}

	status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	//KdPrint(("离开 DriverWriteDispatchRoutine\n"));
	return status;
}


#pragma PAGEDCODE
PMDL MdlMemoryAlloc(ULONG lenght)
{
    void *buffer = ExAllocatePool(NonPagedPool,lenght);
    PMDL mdl;
    if(NULL == buffer)
        return NULL;
    mdl = IoAllocateMdl(buffer,lenght,FALSE,FALSE,NULL);
    if(NULL == mdl)
    {
        ExFreePool(buffer);
        return NULL;
    }
    MmBuildMdlForNonPagedPool(mdl);
    mdl->Next = NULL;
    return mdl;
}

#pragma PAGEDCODE
void MdlMemoryFree(PMDL mdl)
{
    void *buffer = MmGetSystemAddressForMdlSafe(mdl,NormalPagePriority);
    IoFreeMdl(mdl);
    ExFreePool(buffer);
}