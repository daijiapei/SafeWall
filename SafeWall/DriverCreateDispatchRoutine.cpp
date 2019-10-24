
#include "SafeWall.h"
#include "myfs.h"

#pragma PAGEDCODE
NTSTATUS  DriverCreateDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
    NTSTATUS status;
	KEVENT waitEvent;
	DWORD FileFlags;
	DWORD ProcFlags;

	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	DWORD dwDesiredAccess ;        //访问模式：读写删除
	DWORD dwCreationDisposition ;  //如何创建：打开或创建文件
	DWORD dwCreateOptions ;        //打开模式：目录，非目录，同步，异步
	DWORD dwShareMode ;            //共享模式：（不共享、读、写、删除）
	DWORD dwFileAttributes ;       //文件属性：隐藏、只读
	LPSAFEWALL_OBJECT lpSafeWall;
	PFSRTL_COMMON_FCB_HEADER pFcb;
	PSAFEWALL_FILE_LIST pFileList;

	FILE_OBJECT FileObject;
	BOOLEAN IsHas;
    //PAGED_CODE();

    if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("我的驱动产生了一条打开文件的消息\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

    //ASSERT(IS_MY_DEVICE_OBJECT( pDeviceObject ));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;
	PDEVICE_OBJECT storageStackDeviceObject = stack->Parameters.MountVolume.Vpb->RealDevice;//磁盘设备对象

	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

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

	//在Ceatea消息中，有些参数传到下层会有可能被修改，所以要等到
	//下层调用返回时，得到的信息才是最真实的信息。
	//为什么要用事件等待，而不是在完成例程中调用下面的方法？
	//因为完成例程的运行级别太高，容易引发错误，所以很多时候，
	//都是传递一个事件给完成函数，等完成函数成功后，再继续执行操作
    //ASSERT(KeReadStateEvent(&waitEvent) || !NT_SUCCESS(pIrp->IoStatus.Status));

	if(!NT_SUCCESS(pIrp->IoStatus.Status) || (stack->Parameters.Create.Options & FILE_DIRECTORY_FILE))
	{
		//如果操作失败或者目标包含路径标志，直接返回
		status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		//KdPrint(("倒手失败或filename非文件，status=0x%08x，dwCreateOptions=0x%08x ",status,dwCreateOptions));
		return status;
	}
	
	dwDesiredAccess = stack->Parameters.Create.SecurityContext->DesiredAccess; //访问模式：读写删除
	dwCreationDisposition = (stack->Parameters.Create.Options>>24);            //如何创建：打开或创建文件
	dwCreateOptions = (stack->Parameters.Create.Options & 0x00ffffff);         //打开模式：目录，非目录，同步，异步
	dwShareMode = stack->Parameters.Create.ShareAccess;                        //共享模式：（不共享、读、写、删除）
	dwFileAttributes = stack->Parameters.Create.FileAttributes;                //文件属性：隐藏、只读
	//到这里就是成功了，执行我们的过滤操作
	/*
		把原生的FILEOBJECT发送到下层读取文件，会发生一些奇怪的事情，比如在用户界面不能打开
		读取过的文件，把FILEOBJECT复制一个出来后发送到下层就解决了这个文件，估计原因是因为
		对文件读写时，改写了FILEOBJECT的某些内容
		其中FileObject.CurrentByteOffset.QuadPart是改变之一
	*/
	
	if((FILE_OPEN & dwCreationDisposition) && (dwCreateOptions & FILE_NON_DIRECTORY_FILE))//如果是打开一个文件
	{
		FileObject = *pFileObject;
		/*lpSafeWall = (LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool, SAFEWALL_OBJECT_SIZE);
		FileFlags = GetFileSafeWallFlags(&FileObject,
			((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject,
			lpSafeWall);*/
		RtlAppendUnicodeToString(&tarFileName, L"safewall.txt");
		GetFileNameForPath(&FileObject.FileName , &srcFileName);
		FileFlags = NULL;
		if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
		{
			FileFlags = SAFEWALL_FLAG_FILEGROUP | SAFEWALL_FLAG_OBJECT | SAFEWALL_FLAG_MANAGEMENT;
		}
		
		if(FileFlags & SAFEWALL_FLAG_FILEGROUP)
		{
			//KdPrint(("%wZ是加密文件", &FileObject.FileName));
			pFcb = (PFSRTL_COMMON_FCB_HEADER)FileObject.FsContext;
			pFileList = InsertSingleFileListNode((PFSRTL_COMMON_FCB_HEADER)FileObject.FsContext,&IsHas);
			if(FALSE == IsHas)
			{
				//如果本来没有的，把其他东西加上去
				pFileList->flags = FileFlags;
				/*pFcb->FileSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
				pFcb->ValidDataLength.QuadPart -= SAFEWALL_OBJECT_SIZE;*/
			}
			PSECTION_OBJECT_POINTERS SectionObjectPointer = pFileObject->SectionObjectPointer;

			KdPrint(("dwDesiredAccess=0x%08x",dwDesiredAccess));
			KdPrint(("dwCreationDisposition=0x%08x",dwCreationDisposition));
			KdPrint(("dwCreateOptions=0x%08x",dwCreateOptions));
			KdPrint(("dwShareMode=0x%08x",dwShareMode));
			KdPrint(("dwFileAttributes=0x%08x",dwFileAttributes));
		}
		//ExFreePool(lpSafeWall);
	}

    status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return status;

}


#pragma PAGEDCODE
ULONG GetFileObjectFullPath( PFILE_OBJECT FileObject, PUNICODE_STRING FilePath )
{
	NTSTATUS status;
	POBJECT_NAME_INFORMATION  obj_name_info = NULL;
	WCHAR buffer[64] = { 0 };
	void *objptr;
	ULONG length = 0;
	BOOLEAN need_split = FALSE;

	ASSERT( FileObject != NULL );
	if(FileObject == NULL)
		return 0;
	if(FileObject->FileName.Buffer == NULL)
		return 0;

	obj_name_info = (POBJECT_NAME_INFORMATION)buffer;
	do {

		// 获取FileName前面的部分（设备路径或者根目录路径）
		if(FileObject->RelatedFileObject != NULL)
			objptr = (void *)FileObject->RelatedFileObject;
		else
			objptr= (void *)FileObject->DeviceObject;
		status = ObQueryNameString(objptr,obj_name_info,64*sizeof(WCHAR),&length);
		if(status == STATUS_INFO_LENGTH_MISMATCH)
		{
			obj_name_info = (POBJECT_NAME_INFORMATION)ExAllocatePool(NonPagedPool,length);
			if(obj_name_info == NULL)
				return STATUS_INSUFFICIENT_RESOURCES;
			RtlZeroMemory(obj_name_info,length);
			status = ObQueryNameString(objptr,obj_name_info,length,&length);            
		}
		// 失败了就直接跳出即可
		if(!NT_SUCCESS(status))
			break;

		// 判断二者之间是否需要多一个斜杠。这需要两个条件:
		// FileName第一个字符不是斜杠。obj_name_info最后一个
		// 字符不是斜杠。
		if( FileObject->FileName.Length > 2 &&
			FileObject->FileName.Buffer[ 0 ] != L'\\' &&
			obj_name_info->Name.Buffer[ obj_name_info->Name.Length / sizeof(WCHAR) - 1 ] != L'\\' )
			need_split = TRUE;

		// 获总体名字的长度。如果长度不足，也直接返回。
		length = obj_name_info->Name.Length + FileObject->FileName.Length;
		if(need_split)
			length += sizeof(WCHAR);
		if(FilePath->MaximumLength < length)
			break;

		// 先把设备名拷贝进去。
		RtlCopyUnicodeString(FilePath,&obj_name_info->Name);
		if(need_split)
			// 追加一个斜杠
			RtlAppendUnicodeToString(FilePath,L"\\");

		// 然后追加FileName
		RtlAppendUnicodeStringToString(FilePath,&FileObject->FileName);
	} while(0);

	// 如果分配过空间就释放掉。
	if((void *)obj_name_info != (void *)buffer)
		ExFreePool(obj_name_info);
	return length;
}