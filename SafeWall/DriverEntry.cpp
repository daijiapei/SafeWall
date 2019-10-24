

#include "SafeWall.h"

EXTERN_C ULONG g_process_name_offset = 0;
EXTERN_C BOOLEAN g_safewall_start = FALSE;

EXTERN_C ULONG gOsMajorVersion = 0;
EXTERN_C ULONG gOsMinorVersion = 0;

EXTERN_C PDRIVER_OBJECT gMyDriverObject = NULL;        //保存由I/O管理器生成并传入的驱动对象
EXTERN_C PDEVICE_OBJECT gMyControlDeviceObject = NULL; //保存由本过滤驱动生成的控制设备对象
EXTERN_C FAST_MUTEX gFastMutexAttachLock = {0};        //定义一个快速互斥结构变量(对象),挂载卷时用到
EXTERN_C LPSAFEWALL_OBJECT gStandardSafeWallObj  = NULL;      //已经已经格式化的salewall对象


PFILE_OBJECT pTestID = NULL;

#pragma INITCODE
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	//步骤：
	//1.取得系统版本
	//2.设置分发函数
	//以下步骤在AddDriver函数中
	//1.创建设备，并设置设备的快速IO分发函数
	//2.设置文件系统变动通知函数，并且把已挂载的文件系统附加到设备
	//3.把未格式化的CD和磁盘也附加到设备(因为未格式化的设备，未挂载到任何文件系统，
	//  所以不会收到通知，要额外处理)
	NTSTATUS status = STATUS_SUCCESS;
	KdPrint(("Enter DriverEntry\n"));

	gMyDriverObject = pDriverObject;

	RTL_OSVERSIONINFOW versionInfo = {0};
    versionInfo.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOW );
	status = RtlGetVersion(&versionInfo);//取得当前系统版本
    ASSERT( NT_SUCCESS( status ) );
	gOsMajorVersion = versionInfo.dwMajorVersion;
	gOsMinorVersion = versionInfo.dwMinorVersion;

	InitMySafeWallObject(&gStandardSafeWallObj);
	InitializeMyFileListHead();//初始化文件链表
	InitializeMyProcessListHead();//初始化进程链表

	ExInitializeFastMutex( &gFastMutexAttachLock );//初始化"FastMutex(快速互斥)"对象,以后多线程只能互斥访问它

	//取得进程名称偏移位置
	ULONG offset;
	PEPROCESS curproc;
	curproc = PsGetCurrentProcess();
	ULONG offset_end = 3 * 4 * 1024;

	for(offset = 0; offset < offset_end; offset++)
	{
		if(!strncmp("System",(PCHAR)curproc + offset, strlen("System")))
		{
			g_process_name_offset = offset;
			KdPrint(("g_process_name_offset = %d",g_process_name_offset));
			break;
		}
	}

	pDriverObject->DriverExtension->AddDevice = AddDevice;
	pDriverObject->DriverUnload = DriverUnload;

	for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)                                                                
    {
        pDriverObject->MajorFunction[i] = DispatchRoutine;                                                                
    }
	//注册具体派遣函数
    //pDriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE] = DispatchRoutine;//命名管道
    pDriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] = DispatchRoutine;//油槽
	//pDriverObject->MajorFunction[IRP_MJ_READ] = DriverReadDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_WRITE] = DriverWriteDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = DriverCleanUpDispatchRoutine;
    //pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCloseDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = FileSystemDeviceControl;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverControlDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = DriverQueryInformationDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = DriverSetInformationDispatchRoutin;
	//pDriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] = DriverDirectoryControlDispatchRoutine;
	KdPrint(("DriverEntry end\n"));

	return status;
}


#pragma PAGEDCODE
NTSTATUS AddDevice(IN PDRIVER_OBJECT pDriverObject,IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	
	NTSTATUS status;
	UNICODE_STRING devNameString;                        //定义名字串结构变量
	KdPrint(("Enter AddDevice\n"));

	//创建控制设备名称
    RtlInitUnicodeString( &devNameString, L"\\FileSystem\\Filters\\SafeWallSysFilter" );//用来创建文件系统控制设备对象

	//创建控制设备对象
    status = IoCreateDevice( pDriverObject,
                                0,                                      //没有 设备扩展 
                                &devNameString,                            //设备名:   FileSystem\\Filters\\SFilter
                                FILE_DEVICE_DISK_FILE_SYSTEM,           //设备类型: 磁盘文件系统
                                FILE_DEVICE_SECURE_OPEN,                //设备特征: 对发送到CDO的打开请求进行安全检查
                                FALSE,                                  //生成一个在用户模式下使用的设备
                                &gMyControlDeviceObject );         //接收生成的"控制设备对象"

#if WINVER < 0x0501
	if (status == STATUS_OBJECT_PATH_NOT_FOUND)                         //判断是否 未找到路径
    {
        RtlInitUnicodeString( &devNameString, L"\\FileSystem\\SafeWallSysFilter" );  //重新创建 控制设备名称 
        status = IoCreateDevice( pDriverObject, 0,
                                    &devNameString,                           
                                    FILE_DEVICE_DISK_FILE_SYSTEM,
                                    FILE_DEVICE_SECURE_OPEN,
                                    FALSE,
                                    &gMyControlDeviceObject );        //接收生成的 控制设备对象 
    }
#endif

	if (!NT_SUCCESS( status )) //判断IoCreateDevice调用是否成功
    {
		KdPrint(( "文件系统 \"%wZ\" 控制设备创建失败 \n", &devNameString));
		return status;                //错误返回(创建失败)
    }
	KdPrint(( "文件系统 \"%wZ\" 控制设备创建成功 \n", &devNameString));



	//创建连接符号，用户态中用CreateFile打开该路径
	UNICODE_STRING symLinkName;
#ifdef _WIN64
	status = IoRegisterDeviceInterface( PhysicalDeviceObject, &MYDEVICE, NULL ,&symLinkName);
	if(!NT_SUCCESS(status))
	{
		KdPrint(( "设备连接符号 \"%wZ\" 注册失败 \n", &symLinkName));
		IoDeleteDevice(gMyControlDeviceObject);
		return status;
	}
	status=IoSetDeviceInterfaceState(&symLinkName, TRUE);
	if(!NT_SUCCESS(status))
	{
		KdPrint(( "设备连接符号 \"%wZ\" 启用失败 \n", &symLinkName));
		IoDeleteDevice(gMyControlDeviceObject);
		return status;
	}
	KdPrint(( "设备连接符号 \"%wZ\" 启用成功 \n", &symLinkName));
	RtlFreeUnicodeString(&symLinkName);
#else  /* _WIN32 */
	
	RtlInitUnicodeString(&symLinkName, SAFEWALL_DEVICE_SYMNAME);
	status = IoCreateSymbolicLink(&symLinkName, &devNameString);
	if(!NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&symLinkName);
		status = IoCreateSymbolicLink(&symLinkName, &devNameString);
		if( !NT_SUCCESS(status))
		{
			KdPrint(( "设备连接符号 \"%wZ\" 创建失败 \n", &symLinkName));
			IoDeleteDevice(gMyControlDeviceObject );   //删除上面创建的CDO
			return status;
		}
	}
	KdPrint(( "设备连接符号 \"%wZ\" 创建成功 \n", &symLinkName));
#endif


	//下面将分配快速IO分发函数
	KdPrint(( "开始分配快速IO分发函数 \n"));
	PFAST_IO_DISPATCH fastIoDispatch = (PFAST_IO_DISPATCH)ExAllocatePool( NonPagedPool,                          //从非分页池中分配
                                                sizeof( FAST_IO_DISPATCH ));        //要分配的字节数
    
    if (!fastIoDispatch)                //内存分配失败
    {
		IoDeleteDevice(gMyControlDeviceObject );   //删除上面创建的CDO
        return STATUS_INSUFFICIENT_RESOURCES;            //返回一个错误status码(资源不足)
    }
    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );                        
    pDriverObject->FastIoDispatch = fastIoDispatch;                                  //将FastIo分派表保存到驱动对象的FastIoDispatch域
    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );              //设置FastIo分派表的长度域

    fastIoDispatch->FastIoCheckIfPossible = FastIoCheckIfPossible;                //设置FastIo分派函数,共21个
    fastIoDispatch->FastIoRead = FastIoRead;
    fastIoDispatch->FastIoWrite = FastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = FastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = FastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = FastIoLock;
    fastIoDispatch->FastIoUnlockSingle = FastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = FastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = FastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = FastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = FastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = FastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = FastIoMdlRead;
    fastIoDispatch->MdlReadComplete = FastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = FastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = FastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = FastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = FastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = FastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = FastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = FastIoQueryOpen;
	KdPrint(( "快速IO分发函数分配完成 \n"));

	KdPrint(( "------------------------------注册fsFilter回调函数------------------------------- \n"));
	//--------------------------------注册fsFilter回调函数-------------------------------
    FS_FILTER_CALLBACKS fsFilterCallbacks;
    fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
	fsFilterCallbacks.PreAcquireForSectionSynchronization = PreFsFilterPassThrough;
	fsFilterCallbacks.PostAcquireForSectionSynchronization = PostFsFilterPassThrough;
	fsFilterCallbacks.PreReleaseForSectionSynchronization = PreFsFilterPassThrough;
	fsFilterCallbacks.PostReleaseForSectionSynchronization = PostFsFilterPassThrough;
	fsFilterCallbacks.PreAcquireForCcFlush = PreFsFilterPassThrough;
	fsFilterCallbacks.PostAcquireForCcFlush = PostFsFilterPassThrough;
	fsFilterCallbacks.PreReleaseForCcFlush = PreFsFilterPassThrough;
	fsFilterCallbacks.PostReleaseForCcFlush = PostFsFilterPassThrough;
	fsFilterCallbacks.PreAcquireForModifiedPageWriter = PreFsFilterPassThrough;
	fsFilterCallbacks.PostAcquireForModifiedPageWriter = PostFsFilterPassThrough;
	fsFilterCallbacks.PreReleaseForModifiedPageWriter = PreFsFilterPassThrough;
	fsFilterCallbacks.PostReleaseForModifiedPageWriter = PostFsFilterPassThrough;
	
	
	/*以下择自wdk开发文档
	在将操作请求传递给低级筛选器驱动程序和底层文件系统之前调用筛选器通知回调例程。
	在回调例程，过滤驱动程序应执行任何必要的处理并立即返回status_success。如果过滤器
	驱动程序的回调函数返回一个状态值比其他status_success，这使得操作请求失败。重复失
	败的某些要求，如锁定请求，可以阻止系统的进步。因此，过滤器驱动程序应该失败时，这
	样的请求只有在绝对必要的。当这些请求失败时，筛选器驱动程序应尽可能全面准确地返回
	错误状态值。
	注意筛选器驱动程序的通知回调例程不能释放文件系统资源的请求。如果一个过滤驱动程序
	返回一个状态值比其他任何以下通知回调例程status_success，状态值被忽略。*/

	//在过滤操作完成后，调用上层驱动过滤前，调用filtercallback例程
	status = FsRtlRegisterFileSystemFilterCallbacks( pDriverObject, &fsFilterCallbacks );
	if (!NT_SUCCESS( status ))
    {
        pDriverObject->FastIoDispatch = NULL;
		ExFreePool( fastIoDispatch );
		IoDeleteDevice(gMyControlDeviceObject );   //删除上面创建的CDO
		return status;
    }
	KdPrint(( "------------------------------fsFilter回调函数注册完成------------------------------- \n"));
	//注册文件系统变动函数，当有新的文件系统加入时，会调用对应的函数
	//XP以上的操作系统,当加入新的驱动时，即时文件系统已加载，也会调用一次变动函数
	//而XP以上的则不会，如server2000

	status = IoRegisterFsRegistrationChange( pDriverObject, FileSystemChangeNotification );
	if (!NT_SUCCESS( status ))
	{
		KdPrint(( "文件系统变动函数注册失败！" ));

		pDriverObject->FastIoDispatch = NULL;                    //注销指向fastIo函数组的指针为NULL
		ExFreePool( fastIoDispatch);     //释放分配给fastIo函数组的内存
		IoDeleteDevice(gMyControlDeviceObject );   //删除上面创建的CDO
		return status;                                                                                        //错误返回
	}

	KdPrint(( "文件系统变动函数注册成功！" ));

	do{
                
		PDEVICE_OBJECT rawDeviceObject;
		PFILE_OBJECT fileObject;
		RtlInitUnicodeString( &devNameString, L"\\Device\\RawDisk" ); //RawDisk: 未格式化的磁盘

		/*
		IoGetDeviceObjectPointer函数的功能是:
		它从下层的设备对象名称来获得下层设备指针。该函数造成了对下层设备对象以及下层设备对象所对应的文件对象的引用。
		如果本层驱动在卸载之前对下层的设备对象的引用还没有消除，则下层驱动的卸载会被停止。因此必须要消除对下层设备对象的引用。
		但是程序一般不会直接对下层设备对象的引用减少。因此只要减少对文件对象的引用就可以减少文件对象和设备对象两个对象的引用。
		事实上，IoGetDeviceObjectPointer返回的并不是下层设备对象的指针，而是该设备堆栈中顶层的设备对象的指针。

		IoGetDeviceObjectPointer函数的调用必须在 IRQL=PASSIVE_LEVEL的级别上运行。
		*/

		status = IoGetDeviceObjectPointer( &devNameString, FILE_READ_ATTRIBUTES, &fileObject, &rawDeviceObject );
		if (NT_SUCCESS( status ))
		{
			FileSystemChangeNotification( rawDeviceObject, TRUE );
			ObDereferenceObject( fileObject ); //这里减少对文件对象的引用
		}

		RtlInitUnicodeString( &devNameString, L"\\Device\\RawCdRom" );
		status = IoGetDeviceObjectPointer( &devNameString, FILE_READ_ATTRIBUTES, &fileObject, &rawDeviceObject );
		if (NT_SUCCESS( status ))
		{
			FileSystemChangeNotification( rawDeviceObject, TRUE );
			ObDereferenceObject( fileObject );//这里减少对文件对象的引用
		}
	}while(FALSE);

	//加入DO_BUFFERED_IO 用来接收控制码
	gMyControlDeviceObject->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	//清除初始化标志，标志驱动可以开始接受消息了
	ClearFlag( gMyControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

	KdPrint(("Leave AddDevice\n"));
	return STATUS_SUCCESS;
}


#pragma PAGEDCODE
VOID  DriverUnload( IN PDRIVER_OBJECT DriverObject )
{
    PDEVICE_EXTENSION devExt;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    LARGE_INTEGER interval;
	KdPrint(("Enter DriverUnload\n"));
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];
    ASSERT(DriverObject == gMyDriverObject);

    while(TRUE)
    {
		/*使用IoEnumerateDeviceObjectList与 DriverObject->DeviceObject->nextDeviceObject 的
		  差异在于，IoEnumerateDeviceObjectList会增加对象的引用，避免设备在卸载时，
		  设备指针变成无效
		*/
		status = IoEnumerateDeviceObjectList( DriverObject, devList, sizeof(devList), &numDevices);
        if (numDevices <= 0)
        {
                break;
        }

		//当设备数量大于DEVOBJ_LIST_SIZE时，一次处理不完，所以用死循环，分批处理
        numDevices = min( numDevices, DEVOBJ_LIST_SIZE );
        for (ULONG i=0; i < numDevices; i++)
        {
			devExt = (PDEVICE_EXTENSION)devList[i]->DeviceExtension;
            if (NULL != devExt)
            {
                    IoDetachDevice( devExt->AttachedToDeviceObject );
            }
        }

		//解绑后，可能有些驱动消息还没处理完成，所以休息5秒，再对驱动进行删除
        interval.QuadPart = (5 * DELAY_ONE_SECOND);		//delay 5 seconds
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
        for (ULONG i=0; i < numDevices; i++)
        {
            if (NULL != devList[i]->DeviceExtension)
            {
                CleanupMountedDevice( devList[i] );
            }
            else
			{
                ASSERT(devList[i] == gMyControlDeviceObject);
                gMyControlDeviceObject = NULL;
            }

            IoDeleteDevice( devList[i] );
			ObDereferenceObject( devList[i] );
		}
	}

    fastIoDispatch = DriverObject->FastIoDispatch;
    DriverObject->FastIoDispatch = NULL;
    ExFreePool( fastIoDispatch );
	ExFreePool( gStandardSafeWallObj);
	KdPrint(("Leave DriverUnload\n"));
}

#pragma PAGEDCODE
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	//完成TRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("默认处理：我的驱动产生了一条消息\n"));
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }

    IoSkipCurrentIrpStackLocation( pIrp );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
	return status;

}