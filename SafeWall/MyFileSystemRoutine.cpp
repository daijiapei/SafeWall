
#include "myfs.h"

ACCESS_MASK IopQueryOperationAccess[] =
{
    0,
    0,
    0,
    0,
    FILE_READ_ATTRIBUTES,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    FILE_READ_EA,
    0,
    0,
    FILE_READ_ATTRIBUTES,
    0,
    0,
    0,
    0,
    FILE_READ_ATTRIBUTES,
    FILE_READ_ATTRIBUTES,
    FILE_READ_ATTRIBUTES,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    FILE_READ_ATTRIBUTES,
    FILE_READ_ATTRIBUTES,
    0,
    0,
    0,
    0,
    0,
    0xFFFFFFFF
};

ACCESS_MASK IopSetOperationAccess[] =
{
    0,
    0,
    0,
    0,
    FILE_WRITE_ATTRIBUTES,
    0,
    0,
    0,
    0,
    0,
    DELETE,
    0,
    0,
    DELETE,
    0,
    FILE_WRITE_EA,
    0,
    0,
    0,
    FILE_WRITE_DATA,
    FILE_WRITE_DATA,
    0,
    0,
    FILE_WRITE_ATTRIBUTES,
    0,
    FILE_WRITE_ATTRIBUTES,
    0,
    0,
    0,
    0,
    0,
    FILE_WRITE_DATA,
    0,
    0,
    0,
    0,
    FILE_WRITE_DATA,
    0,
    0,
    FILE_WRITE_DATA,
    DELETE,
    0xFFFFFFFF
};

#pragma PAGEDCODE
NTSTATUS  MyCompletionRoutine( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context )
{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

	*Irp->UserIosb = Irp->IoStatus;
	if(Irp->UserEvent)
	{
		KeSetEvent((PKEVENT)Irp->UserEvent, IO_NO_INCREMENT, FALSE);
	}
	IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma PAGEDCODE
NTSTATUS devQueryInformationFile(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_Out_writes_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass,
	_In_ PDEVICE_OBJECT pDeviceObject)
{
	NTSTATUS status;
	PIRP pIrp;
	KEVENT hEvent;
	IO_STATUS_BLOCK ioStatusBlock;
	PIO_STACK_LOCATION stack;

	//status = ObReferenceObjectByHandle(FileHandle,IopQueryOperationAccess[FileInformationClass],
	//	*IoFileObjectType,KernelMode,(PVOID*)&FileObject,NULL);
	//if (!NT_SUCCESS(status)) return status;

	//因为打算让这个请求同步完成，所以初始化一个事件
	//用来等待请求完成
	KeInitializeEvent(&hEvent, SynchronizationEvent, FALSE);

	//分配IRP
	pIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
	if(NULL == pIrp)
	{
		ObDereferenceObject(FileObject);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//填写IRP的主体
	pIrp->AssociatedIrp.SystemBuffer = FileInformation;
	pIrp->UserEvent = &hEvent;
	pIrp->UserIosb = &ioStatusBlock;
	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	pIrp->Tail.Overlay.OriginalFileObject = FileObject;
	pIrp->RequestorMode = KernelMode;
	pIrp->Flags = 0;

	//设置irpsp
	stack = IoGetNextIrpStackLocation(pIrp);
	stack->MajorFunction = IRP_MJ_QUERY_INFORMATION;
	stack->DeviceObject = pDeviceObject;
	stack->FileObject = FileObject;
	stack->Parameters.QueryFile.Length = Length;
	stack->Parameters.QueryFile.FileInformationClass = FileInformationClass;

	//设置结束例程
	IoSetCompletionRoutine(pIrp, MyCompletionRoutine, 0, TRUE, TRUE, TRUE);

	//发送请求并就等待结束
	status = IoCallDriver(pDeviceObject, pIrp);
	if (STATUS_PENDING == status)//若状态是 挂起
	{
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, TRUE, 0);
	}
	//ObDereferenceObject(FileObject);
	return pIrp->IoStatus.Status;
}

#pragma PAGEDCODE
NTSTATUS devSetInformationFile(
    _In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass,
    _In_ PDEVICE_OBJECT pDeviceObject)
{
	NTSTATUS status;
	PIRP pIrp;
	KEVENT hEvent;
	IO_STATUS_BLOCK ioStatusBlock;
	PIO_STACK_LOCATION stack;

	/*status = ObReferenceObjectByHandle(FileHandle,IopSetOperationAccess[FileInformationClass],
		*IoFileObjectType,KernelMode,(PVOID*)&FileObject,NULL);
	if (!NT_SUCCESS(status)) return status;*/

	//因为打算让这个请求同步完成，所以初始化一个事件
	//用来等待请求完成

	KeInitializeEvent(&hEvent, SynchronizationEvent, FALSE);

	//分配IRP
	pIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
	if(NULL == pIrp)
	{
		ObDereferenceObject(FileObject);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//填写IRP的主体
	pIrp->AssociatedIrp.SystemBuffer = FileInformation;
	pIrp->UserEvent = &hEvent;
	pIrp->UserIosb = &ioStatusBlock;
	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	pIrp->Tail.Overlay.OriginalFileObject = FileObject;
	pIrp->RequestorMode = KernelMode;
	pIrp->Flags = 0;

	//设置irpsp
	stack = IoGetNextIrpStackLocation(pIrp);
	stack->MajorFunction = IRP_MJ_SET_INFORMATION;
	stack->DeviceObject = pDeviceObject;
	stack->FileObject = FileObject;
	stack->Parameters.SetFile.FileObject = NULL;
	stack->Parameters.SetFile.Length = Length;
	stack->Parameters.SetFile.FileInformationClass = FileInformationClass;

	//设置结束例程
	IoSetCompletionRoutine(pIrp, MyCompletionRoutine, 0, TRUE, TRUE, TRUE);

	//发送请求并就等待结束
	status = IoCallDriver(pDeviceObject, pIrp);
	if (STATUS_PENDING == status)//若状态是 挂起
	{
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, TRUE, 0);
	}
	//ObDereferenceObject(FileObject);
	return pIrp->IoStatus.Status;
}

#pragma PAGEDCODE
NTSTATUS MyFileSetInformation(
	DEVICE_OBJECT *dev, FILE_OBJECT * file,
	FILE_INFORMATION_CLASS FileInformationClass, FILE_OBJECT * setFile,
	PVOID buffer, ULONG lenght)
{
	PIRP irp;
	KEVENT event;
	IO_STATUS_BLOCK ioStatusBlock;
	PIO_STACK_LOCATION ioStackLocation;
	//因为打算让这个请求同步完成，所以初始化一个事件
	//用来等待请求完成
	KeInitializeEvent(&event, SynchronizationEvent, FALSE);

	//分配IRP
	irp = IoAllocateIrp(dev->StackSize, FALSE);
	if(NULL == irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//填写IRP的主体
	irp->AssociatedIrp.SystemBuffer = buffer;
	irp->UserEvent = &event;
	irp->UserIosb = &ioStatusBlock;
	irp->Tail.Overlay.Thread = PsGetCurrentThread();
	irp->Tail.Overlay.OriginalFileObject = file;
	irp->RequestorMode = KernelMode;
	irp->Flags = 0;

	//设置irpsp
	ioStackLocation = IoGetNextIrpStackLocation(irp);
	ioStackLocation->MajorFunction = IRP_MJ_SET_INFORMATION;
	ioStackLocation->DeviceObject = dev;
	ioStackLocation->FileObject = file;
	ioStackLocation->Parameters.SetFile.FileObject = setFile;
	ioStackLocation->Parameters.SetFile.FileInformationClass = FileInformationClass;


	//设置结束例程
	IoSetCompletionRoutine(irp, MyCompletionRoutine, 0, TRUE, TRUE, TRUE);

	//发送请求并就等待结束

	(void)IoCallDriver(dev, irp);
	KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, 0);
	return ioStatusBlock.Status;
}


#pragma PAGEDCODE //未完成
int GetSuffixByFileName(PUNICODE_STRING FileName, PUNICODE_STRING SuffixString)
{
	BOOLEAN has = FALSE;
	int offset =0 ;
	WCHAR suffix[64] = {0};
	PWCHAR pw = suffix;
	int count = FileName->Length / sizeof(WCHAR);

	for( offset = count; offset > 0 && count - offset > 32; --offset)
	{
		if(L'.' == FileName->Buffer[offset])
		{
			has = TRUE;
			break;
		}
		if(L'\\' == FileName->Buffer[offset] ) break;
		*pw++ = FileName->Buffer[offset];
	}

	if(FALSE == has)//没找到后缀
	{
		SuffixString->Buffer[0] = L'\0';
		SuffixString->Length = 0;
	}
	else
	{
	}
	return SuffixString->Length;
}


#pragma PAGEDCODE
int GetVolumeFileNameByFullFilePath(IN PUNICODE_STRING FilePath,OUT PUNICODE_STRING VolumeFileName)
{
	int offset =0;
	int count = FilePath->Length / sizeof(WCHAR);
	PWCHAR pw = VolumeFileName->Buffer;
	while(count >offset && L':' != FilePath->Buffer[offset++]);
	if((count - offset) * sizeof(WCHAR) > VolumeFileName->MaximumLength ) 
	{
		return (count - offset) * sizeof(WCHAR);
	}
	VolumeFileName->Length = (count - offset) * sizeof(WCHAR);
	while(count >offset && (*pw++ = FilePath->Buffer[offset++]));

	KdPrint(("Name = %wZ, len = %d", VolumeFileName, VolumeFileName->Length));
	return VolumeFileName->Length;
}

//从路径获取盘符
#pragma PAGEDCODE
int GetHardSymbolicByFullFilePath(IN PUNICODE_STRING FilePath,OUT PUNICODE_STRING HardSymbolic)
{
	BOOLEAN has = FALSE;
	int i;
	int count = FilePath->Length / sizeof(WCHAR);
	for(i=0; count > i && HardSymbolic->MaximumLength > i * sizeof(WCHAR); i++)
	{
		HardSymbolic->Buffer[i] = FilePath->Buffer[i];
		if(L':' == FilePath->Buffer[i])
		{
			has = TRUE;
			break;
		}
	}

	HardSymbolic->Length = (i+1) * sizeof(WCHAR);
	if (NULL == has)
	{
		HardSymbolic->Buffer = L'\0';
		HardSymbolic->Length = 0;

	}
	return HardSymbolic->Length;
}

#pragma PAGEDCODE
NTSTATUS devCreateFile(
	_Out_ PHANDLE FileHandle,
	_Out_ PFILE_OBJECT * FileObject,
    _In_ ACCESS_MASK DesiredAccess,
	_In_ OBJECT_ATTRIBUTES *ObjectAttributes,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_opt_ PLARGE_INTEGER AllocationSize,
    _In_ ULONG FileAttributes,
    _In_ ULONG ShareAccess,
    _In_ ULONG CreateDisposition,
    _In_ ULONG CreateOptions,
	_In_ PDEVICE_OBJECT pDeviceObject)
{
	NTSTATUS status;
	//FullPath是一卷设备名称命名的全路径
	//pDeviceObject 一般是源头设备所附加的设备
	status = IoCreateFileSpecifyDeviceObjectHint(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		NULL,
		0,
		CreateFileTypeNone,
		NULL,
		IO_IGNORE_SHARE_ACCESS_CHECK,
		pDeviceObject);


	if(!NT_SUCCESS(status))
    {
		return status;
    }
    // 从句柄得到一个fileobject便于后面的操作。记得一定要解除
    // 引用。
    status = ObReferenceObjectByHandle(*FileHandle,0, *IoFileObjectType,
        KernelMode,(PVOID*)FileObject,NULL);

    // 如果失败了就关闭，假设没打开文件。但是这个实际上是不
    // 应该出现的。
    if(!NT_SUCCESS(status))
    {
        ASSERT(FALSE);
        ZwClose(FileHandle);
    }
    return status;
}

#pragma PAGEDCODE
NTSTATUS devReadFile2(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset,
	_In_ PDEVICE_OBJECT pDeviceObject
    )
{
	NTSTATUS status;
	PIRP pIrp;
	KEVENT hEvent;
	PIO_STACK_LOCATION stack;

	//因为打算让这个请求同步完成，所以初始化一个事件
	//用来等待请求完成
	KeInitializeEvent(&hEvent, SynchronizationEvent, FALSE);

	//分配IRP
	pIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
	if(NULL == pIrp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pIrp->AssociatedIrp.SystemBuffer = NULL;
	pIrp->MdlAddress = NULL;
	pIrp->UserBuffer = Buffer;
	pIrp->UserEvent = &hEvent;
	pIrp->UserIosb = IoStatusBlock;
	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	pIrp->Tail.Overlay.OriginalFileObject = FileObject;
	pIrp->RequestorMode = KernelMode;
	pIrp->Flags = IRP_DEFER_IO_COMPLETION | IRP_READ_OPERATION |IRP_NOCACHE ;

	//设置irpsp
	stack = IoGetNextIrpStackLocation(pIrp);
	stack->MajorFunction = IRP_MJ_READ;
	stack->MinorFunction = IRP_MN_NORMAL;
	stack->DeviceObject = pDeviceObject;
	stack->FileObject = FileObject;
	stack->Parameters.Read.Key = NULL;
	stack->Parameters.Read.ByteOffset = *ByteOffset;
	stack->Parameters.Read.Length = Length;


	//设置结束例程
	IoSetCompletionRoutine(pIrp, MyCompletionRoutine, 0, TRUE, TRUE, TRUE);
	
	//发送请求并就等待结束
	status = IoCallDriver(pDeviceObject, pIrp);
	if (STATUS_PENDING == status)//若状态是 挂起
	{
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, TRUE, 0);
	}

	return pIrp->IoStatus.Status;
}

NTSTATUS devReadFile(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset,
	_In_ PDEVICE_OBJECT pDeviceObject
    )
{
	NTSTATUS status;
	PIRP pIrp;
	KEVENT hEvent;
	PIO_STACK_LOCATION stack;

	PFILE_OBJECT FileObject2;
	//因为打算让这个请求同步完成，所以初始化一个事件
	//用来等待请求完成
	status = ObReferenceObjectByHandle(FileHandle,FILE_READ_DATA,*IoFileObjectType,
										KernelMode,(PVOID*)&FileObject2,NULL);
	if (!NT_SUCCESS(status)) return status;
	KeInitializeEvent(&hEvent, NotificationEvent, FALSE);

	//分配IRP
	pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, pDeviceObject, Buffer,Length, ByteOffset, &hEvent,IoStatusBlock);
	if(NULL == pIrp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//设置irpsp
	stack = IoGetNextIrpStackLocation(pIrp);
	stack->FileObject = FileObject;
	
	//发送请求并就等待结束
	status = IoCallDriver(pDeviceObject, pIrp);
	if (STATUS_PENDING == status)//若状态是 挂起
	{
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, FALSE, 0);
	}
	ObDereferenceObject(FileObject2);
	return pIrp->IoStatus.Status;
}

#pragma PAGEDCODE
NTSTATUS devWriteFile(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_reads_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length,
	_In_opt_ PLARGE_INTEGER ByteOffset,
	_In_ PDEVICE_OBJECT pDeviceObject
	)
{
	NTSTATUS status;
	PIRP pIrp;
	KEVENT hEvent;
	PIO_STACK_LOCATION stack;
	PFILE_OBJECT FileObject2;
	//因为打算让这个请求同步完成，所以初始化一个事件
	//用来等待请求完成
	status = ObReferenceObjectByHandle(FileHandle,FILE_WRITE_DATA,*IoFileObjectType,
										KernelMode,(PVOID*)&FileObject2,NULL);
	if (!NT_SUCCESS(status)) return status;
	KeInitializeEvent(&hEvent, NotificationEvent, FALSE);

	//分配IRP
	pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE, pDeviceObject, Buffer,Length, ByteOffset, &hEvent,IoStatusBlock);
	if(NULL == pIrp)
	{
		//ObDereferenceObject(FileObject);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//设置stack
	stack = IoGetNextIrpStackLocation(pIrp);
	stack->FileObject = FileObject2;

	//发送请求并就等待结束
	status = IoCallDriver(pDeviceObject, pIrp);
	if (STATUS_PENDING == status)//若状态是 挂起
	{
		KdPrint(("等待事件结束\n"));
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, FALSE, 0);
	}
	ObDereferenceObject(FileObject2);
	return pIrp->IoStatus.Status;//return STATUS_WORKING_SET_QUOTA
}

#pragma PAGEDCODE
NTSTATUS devWriteFile2(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_reads_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length,
	_In_opt_ PLARGE_INTEGER ByteOffset,
	_In_ PDEVICE_OBJECT pDeviceObject
	)
{
	NTSTATUS status;
	PIRP pIrp;
	KEVENT hEvent;
	PIO_STACK_LOCATION stack;
	//因为打算让这个请求同步完成，所以初始化一个事件
	//用来等待请求完成
	/*status = ObReferenceObjectByHandle(FileHandle,FILE_WRITE_DATA,*IoFileObjectType,
										KernelMode,(PVOID*)&FileObject,NULL);
	if (!NT_SUCCESS(status)) return status;*/
	KeInitializeEvent(&hEvent, SynchronizationEvent, FALSE);

	//分配IRP
	pIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
	if(NULL == pIrp)
	{
		//ObDereferenceObject(FileObject);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//填写IRP的主体

	/*if (FileObject->Flags & FO_WRITE_THROUGH) pIrp->Flags |= SL_WRITE_THROUGH;
	if (FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) pIrp->Flags |= IRP_NOCACHE;*/

	//读写缓冲区不要用Systembuffer,用UserBuffer(非缓冲区方式)，
	//这样可以防止影响文件缓冲
	pIrp->AssociatedIrp.SystemBuffer = NULL;
	pIrp->MdlAddress = NULL;
	pIrp->UserBuffer = Buffer;
	pIrp->UserEvent = &hEvent;
	pIrp->UserIosb = IoStatusBlock;
	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	pIrp->Tail.Overlay.OriginalFileObject = FileObject;
	pIrp->RequestorMode = KernelMode;
	pIrp->Flags = IRP_DEFER_IO_COMPLETION | IRP_WRITE_OPERATION |IRP_NOCACHE;

	//设置stack
	stack = IoGetNextIrpStackLocation(pIrp);
	stack->MajorFunction = IRP_MJ_WRITE;
	stack->MinorFunction = IRP_MN_NORMAL;
	stack->DeviceObject = pDeviceObject;
	stack->FileObject = FileObject;
	//stack->Flags = FileObject->Flags & FO_WRITE_THROUGH ? SL_WRITE_THROUGH : 0;
	stack->Parameters.Write.Key = NULL;
	stack->Parameters.Write.ByteOffset = *ByteOffset;
	stack->Parameters.Write.Length = Length;

	//KdPrint(("%d\n",Length));
	//设置结束例程
	IoSetCompletionRoutine(pIrp, MyCompletionRoutine, 0, TRUE, TRUE, TRUE);

	//发送请求并就等待结束
	status = IoCallDriver(pDeviceObject, pIrp);
	if (STATUS_PENDING == status)//若状态是 挂起
	{
		KdPrint(("等待事件结束\n"));
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, TRUE, 0);
	}
	//ObDereferenceObject(FileObject);
	return pIrp->IoStatus.Status;//return STATUS_WORKING_SET_QUOTA
}

#pragma PAGEDCODE
VOID CallNextCloseFile(_In_ HANDLE FileHandle, _In_ PFILE_OBJECT FileObject)
{
	/*注意，CallNextCloseFile不会向下层发送消息，他是直接从最底层关闭对象
	如果一台电脑安装了两个透明加密软件，则下层的驱动将接受不到关闭消息，
	有可能引起冲突，但是一般我们认为用户不会安装两个加密.
	如果希望下层驱动能够捕获关闭消息，请自行构建IRP*/
	if(FileObject) ObDereferenceObject(FileObject);
	ObCloseHandle(FileHandle,KernelMode);
}

#pragma PAGEDCODE
NTSTATUS devCloseFile(IN HANDLE FileHandle, IN PDEVICE_OBJECT pDeviceObject)  
{  
	NTSTATUS status;
	PIRP pIrp;
	KEVENT hEvent;
	PIO_STACK_LOCATION stack;
	PFILE_OBJECT FileObject;
	IO_STATUS_BLOCK IoStatusBlock;

	status = ObReferenceObjectByHandle(FileHandle,FILE_READ_DATA|FILE_WRITE_DATA,
		*IoFileObjectType,KernelMode,(PVOID*)&FileObject,NULL);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	KeInitializeEvent(&hEvent, SynchronizationEvent, FALSE);

	//分配IRP
	pIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
	if(NULL == pIrp)
	{
		ObDereferenceObject(FileObject);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pIrp->UserEvent = &hEvent;  
	pIrp->UserIosb = &IoStatusBlock;  
	pIrp->RequestorMode = KernelMode;  
	pIrp->Flags = IRP_CLOSE_OPERATION|IRP_SYNCHRONOUS_API;  
	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();  
	pIrp->Tail.Overlay.OriginalFileObject = FileObject;  

	stack = IoGetNextIrpStackLocation(pIrp);  
	stack->MajorFunction = IRP_MJ_CLEANUP;  
	stack->FileObject = FileObject;  
   
	status = IoCallDriver(pDeviceObject, pIrp);  
	if (status == STATUS_PENDING)
	{
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, FALSE, NULL);  
	}

	status = IoStatusBlock.Status;  
	if(!NT_SUCCESS(status))  
	{
		IoFreeIrp(pIrp);
		ObDereferenceObject(FileObject);
		return status;  
	}  

	KeClearEvent(&hEvent);
	IoReuseIrp(pIrp , STATUS_SUCCESS);
   
	pIrp->UserEvent = &hEvent;  
	pIrp->UserIosb = &IoStatusBlock;  
	pIrp->Tail.Overlay.OriginalFileObject = FileObject;  
	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();  
	pIrp->AssociatedIrp.SystemBuffer = (PVOID)NULL;  
	pIrp->Flags = IRP_CLOSE_OPERATION|IRP_SYNCHRONOUS_API;  
   
	stack = IoGetNextIrpStackLocation(pIrp);  
	stack->MajorFunction = IRP_MJ_CLOSE;  
	stack->FileObject = FileObject;  
   
	if (FileObject->Vpb && !(FileObject->Flags & FO_DIRECT_DEVICE_OPEN))  
	{  
		InterlockedDecrement((volatile LONG *) &FileObject->Vpb->ReferenceCount);
		FileObject->Flags |= FO_FILE_OPEN_CANCELLED;  
	}

	status = IoCallDriver(pDeviceObject, pIrp);
	if (status == STATUS_PENDING)  
		KeWaitForSingleObject(&hEvent, Executive, KernelMode, FALSE, NULL);  

	IoFreeIrp(pIrp);
	ObDereferenceObject(FileObject);

	status = IoStatusBlock.Status;  
	return status;
}