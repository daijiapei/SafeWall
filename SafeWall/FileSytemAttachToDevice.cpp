
#include "SafeWall.h"

#pragma PAGEDCODE
VOID  FileSystemChangeNotification( IN PDEVICE_OBJECT DeviceObject, IN BOOLEAN FsActive )
{

/*
   SfFsNotification函数：
   它创建一个设备对象，并将它附加到指定的文件系统控制设备对象(File System CDO)的对象栈上。这就允许这个设备对象过滤所有发送给文件系统的请求。
   这样，我们就能够获得一个挂载卷的请求，就可以附加到这个新的卷设备对象的设备对象栈上。
  
   当SfFsNotification函数调用完毕以后，我们的过滤驱动设备对象就能够接收到发送到文件系统CDO的请求，即接收到IRP_MJ_FILE_SYSTEM_CONTROL，或者说，
   文件系统控制设备已经被绑定，可以动态监控卷的挂载了。那么以后的工作就是要完成对卷的监控绑定了。
  
   
   参数说明:
  
   DeviceObject:   它指向文件系统的控制设备对象(CDO)。即 被激活或则撤消的File System CDO
   FsActive:       值为TRUE，表示文件系统的激活。值为FALSE，表示文件系统的卸载。
  
  */
	KdPrint(("Enter FileSystemChangeNotification\n"));
    UNICODE_STRING name;                                        //定义结构变量
    WCHAR nameBuffer[MAX_DEVNAME_LENGTH];        //定义宽字符缓冲区,长度64

    PAGED_CODE();

    RtlInitEmptyUnicodeString( &name, nameBuffer, sizeof(nameBuffer) );                //初始化name(成员Buffer->nameBuffer,Length=0,MaximumLength=64)
    GetObjectName( DeviceObject, &name );       //取得设备名称，或者可以理解为取得盘符                                                                                         

    if (FsActive)
    {
		KdPrint(("准备将文件系统 %wZ 附加到过滤设备\n",&name));
        AttachToFileSystemDevice( DeviceObject, &name );        //用于完成对文件系统控制设备的绑定
    }
    else
    {
		KdPrint(("解绑文件系统过滤设备: %wZ\n",&name));
        DetachFromFileSystemDevice( DeviceObject );
    }

	KdPrint(("Leave FileSystemChangeNotification\n"));
}

#pragma PAGEDCODE
NTSTATUS  AttachToFileSystemDevice( IN PDEVICE_OBJECT DeviceObject, IN PUNICODE_STRING DeviceName )
{
    /*
        SfAttachToFileSystemDevice函数用来完成对文件系统控制设备的绑定。

        参数说明:
        DeviceObject:   它指向文件系统的控制设备对象(CDO)。即 被激活或则撤消的File System CDO

    */

    PDEVICE_OBJECT newDeviceObject;                  //新设备对象
    PDEVICE_EXTENSION devExt;                //文件系统过滤驱动定义的设备扩展
    NTSTATUS status;                                 //状态码
    UNICODE_STRING fsrecName;                                                                                                                
    UNICODE_STRING fsName;                           //文件系统名
    WCHAR tempNameBuffer[MAX_DEVNAME_LENGTH];		 //临时缓冲区(存放名字串)

    PAGED_CODE();

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType))        //测试给定设备是不是所需要关心的设备
    {
		KdPrint(("我们对  %wZ  文件系统并不关心，返回\n",DeviceName ));
        return STATUS_SUCCESS;
    }
    
#if DBG

	switch(DeviceObject->DeviceType)
	{
	case FILE_DEVICE_DISK_FILE_SYSTEM:
		{
			KdPrint(("%wZ 是磁盘文件系统\n",DeviceName ));
		}break;
	case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
		{
			KdPrint(("%wZ 是光驱文件系统\n",DeviceName ));
		}break;
	case FILE_DEVICE_NETWORK_FILE_SYSTEM:
		{
			KdPrint(("%wZ 是网络文件系统\n",DeviceName ));
		}break;
	default:
		{
			KdPrint(("%wZ 是未知设备\n",DeviceName ));
		}
	}

#endif

    /*
    * Windows的标准文件系统识别器基本上都是由驱动 \FileSystem\Fs_Rec 生成的。所以直接判断驱动的名字可以解决一部分问题。
    * 也不一定非得要把文件系统识别器生成在驱动\FileSystem\Fs_Rec下面。只是说，一般情况下是在\FileSystem\Fs_Rec下面。
    */
    RtlInitEmptyUnicodeString( &fsName, tempNameBuffer, sizeof(tempNameBuffer) );

	//不绑定文件系统识别器
    RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );
    GetObjectName( DeviceObject->DriverObject, &fsName );
	
    if (RtlCompareUnicodeString( &fsName, &fsrecName, TRUE ) == 0)
    {        
        //通过驱动的名字来分辨出部分Windows的标准文件系统识别器。如果是，那么返回成功，也就是放弃绑定了。
        //如果，有错过的文件系统识别器没有被判断到，文件系统控制请求的过滤驱动中有对应的处理。
		KdPrint(("跳过文件识别器\n"));
        return STATUS_SUCCESS;
    }

    //是我们关心的文件系统，且不是微软的文件系统识别器的设备，创建一个设备绑定这个设备对象。
	status = IoCreateDevice( gMyDriverObject,sizeof(DEVICE_EXTENSION ),
                    NULL,DeviceObject->DeviceType, 0, FALSE, &newDeviceObject );

	if (!NT_SUCCESS( status ))
    {
		KdPrint(("文件系统过滤设备创建失败 status=%08x\n",status));
        return status;
    }

	if ( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO ))
    {
        SetFlag( newDeviceObject->Flags, DO_BUFFERED_IO );                                                                
    }

	if ( FlagOn( DeviceObject->Flags, DO_DIRECT_IO ))
    {
        SetFlag( newDeviceObject->Flags, DO_DIRECT_IO );                                                                        
    }

	if ( FlagOn( DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN ) )
    {
        SetFlag( newDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN );                
	}

    devExt = (PDEVICE_EXTENSION)newDeviceObject->DeviceExtension;
 
    /*
    调用SfAttachDeviceToDeviceStack函数将过滤设备对象绑定到File System CDO的设备栈上面。这样，我们的newDeviceObject就可以接收到发送到
    File System CDO的IRP_MJ_FILE_SYSTEM_CONTROL请求了。 以后，程序就可以去绑定卷了。
    使用SfAttachDeviceToDeviceStack函数来进行绑定。参数1绑定到参数2，绑定函数返回的设备存储在参数3中。
    */

    status = IoAttachDeviceToDeviceStackSafe( newDeviceObject, DeviceObject,  &devExt->AttachedToDeviceObject );
    if (!NT_SUCCESS( status ))
    {
		KdPrint(("文件系统绑定失败！status= %08x", status));
        goto ErrorCleanupDevice;
    }
	KdPrint(("文件系统绑定成功！newDeviceObject= 0x%08x，下层AttachedToDeviceObject = 0x%08x", newDeviceObject,devExt->AttachedToDeviceObject));

    RtlInitEmptyUnicodeString( &devExt->DeviceName, devExt->DeviceNameBuffer, sizeof(devExt->DeviceNameBuffer) );
	RtlCopyUnicodeString( &devExt->DeviceName, DeviceName );        //Save Name
	//把DO_DEVICE_INITIALIZING（初始化中）标志清除，才能接收到启动发来的消息
	ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING ); 
	KdPrint(("文件系统 %wZ 已绑定成功，开始接受消息",&devExt->DeviceName ));
    /* 
    函数SpyEnumerateFileSystemVolumes枚举给定的文件系统下的当前存在的所有挂载了的设备，并且绑定他们。
	这样做的目的，是因为过滤驱动可能随时被加载，但是加载过滤驱动的时候，文件系统已经挂载了卷设备。
    既是：让过滤驱动加后载，随时都能绑定已经存在或刚刚挂载上来的文件系统卷设备。
    */
    status = EnumerateFileSystemVolumes( DeviceObject, &fsName );
    if (!NT_SUCCESS( status ))
    {
        IoDetachDevice( devExt->AttachedToDeviceObject );
        goto ErrorCleanupDevice;
    }

    return STATUS_SUCCESS;

    ErrorCleanupDevice:
        CleanupMountedDevice( newDeviceObject );
        IoDeleteDevice( newDeviceObject );

    return status;
}

#pragma PAGEDCODE
VOID  DetachFromFileSystemDevice( IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT ourAttachedDevice;
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();

    ourAttachedDevice = DeviceObject->AttachedDevice;
    while (NULL != ourAttachedDevice)
    {
        if (IS_MY_DEVICE_OBJECT( ourAttachedDevice ))
        {
            devExt = (PDEVICE_EXTENSION) ourAttachedDevice->DeviceExtension;

            CleanupMountedDevice( ourAttachedDevice );
			IoDetachDevice( DeviceObject );
			IoDeleteDevice( ourAttachedDevice );
			return;
		}
        
		DeviceObject = ourAttachedDevice;
		//ourAttachedDevice = ourAttachedDevice->AttachedDevice;
		ourAttachedDevice = DeviceObject->AttachedDevice;
	}
}

//判断一个设备是否已经进行了绑定
#pragma PAGEDCODE
BOOLEAN  IsAttachedToDevice( PDEVICE_OBJECT DeviceObject, PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL )
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    PAGED_CODE();

    currentDevObj = IoGetAttachedDeviceReference( DeviceObject );
        
    do {   
        if (IS_MY_DEVICE_OBJECT( currentDevObj ))
        {
            if (ARGUMENT_PRESENT(AttachedDeviceObject))
            {
                 *AttachedDeviceObject = currentDevObj;
            }
            else
            {
                 ObDereferenceObject( currentDevObj );
            }
            return TRUE;
        }

        nextDevObj = IoGetLowerDeviceObject( currentDevObj );
		ObDereferenceObject( currentDevObj );
		currentDevObj = nextDevObj;
	} while (NULL != currentDevObj);
        
	if (ARGUMENT_PRESENT(AttachedDeviceObject))
	{
		*AttachedDeviceObject = NULL;
	}
        
    return FALSE;
}

//解挂载
#pragma PAGEDCODE
VOID  CleanupMountedDevice( IN PDEVICE_OBJECT DeviceObject )
{        
	//卸载设备，其实什么也不做也可以的，系统会自动帮我们完成大部分的卸载操作，
	//不过会引起少量的内存泄漏，但是往往设备的操作并不频繁发生，而且卸载后我们会要求重启
    UNREFERENCED_PARAMETER( DeviceObject );
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
}

//对设备进行绑定
#pragma PAGEDCODE
NTSTATUS  AttachToMountedDevice( IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_OBJECT SFilterDeviceObject )
{
    /*
    SfAttachToMountedDevice函数的功能: 完成绑定一个文件系统卷设备的操作。
        
    参数说明:
    SFilterDeviceObject:  它是我们使用IoCreateDevice函数来创建的设备对象。

    */

    PDEVICE_EXTENSION newDevExt = (PDEVICE_EXTENSION)SFilterDeviceObject->DeviceExtension;
    NTSTATUS status;
	ULONG i;

    PAGED_CODE();
    ASSERT(IS_MY_DEVICE_OBJECT( SFilterDeviceObject ));
    ASSERT(!IsAttachedToDevice ( DeviceObject, NULL ));

    if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO ))
    {
        SetFlag( SFilterDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO ))
    {
        SetFlag( SFilterDeviceObject->Flags, DO_DIRECT_IO );
    }

    for (i=0; i < 8; i++)
    {
        LARGE_INTEGER interval;

        //调用SfAttachDeviceToDeviceStack函数进行  卷的绑定
        status = IoAttachDeviceToDeviceStackSafe( SFilterDeviceObject, DeviceObject, &newDevExt->AttachedToDeviceObject );
        if (NT_SUCCESS(status))
        {
			
            ClearFlag( SFilterDeviceObject->Flags, DO_DEVICE_INITIALIZING );
			KdPrint(("卷 %wZ设备绑定成功，DeviceObject= 0x%08x 开始接收消息\n",&newDevExt->DeviceName,SFilterDeviceObject));
            return STATUS_SUCCESS;
        }
		//如果失败，等待一下，然后尝试重连
        interval.QuadPart = (500 * DELAY_ONE_MILLISECOND);      //delay 1/2 second  0.5秒
		KeDelayExecutionThread( KernelMode, FALSE, &interval );
	}

    return status;
}

//
#pragma PAGEDCODE
NTSTATUS  EnumerateFileSystemVolumes( IN PDEVICE_OBJECT FSDeviceObject, IN PUNICODE_STRING Name ) 
{
    /*
    参数说明:
	FSDeviceObject:    它指向文件系统的控制设备对象(CDO)。即 被激活或则撤消的File System CDO
    Name:              它是文件系统的名字，比如NTFS就是一个文件系统
    */

    PDEVICE_OBJECT newDeviceObject;
    PDEVICE_EXTENSION newDevExt;
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT storageStackDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    BOOLEAN isShadowCopyVolume;

    PAGED_CODE();
        
    /*
    * IoEnumerateDeviceObjectList函数枚举这个驱动下的设备对象列表。这个函数将被调用2次。
    * 第1次调用： 获取设备列表中的设备对象的数量。
    * 第2次调用:  根据第1次的结果numDevices值来开辟设备对象的存放空间，从而得到设备链devList。
    */
    status = IoEnumerateDeviceObjectList(FSDeviceObject->DriverObject,
                NULL,0, &numDevices);
        
    if (!NT_SUCCESS( status ))
    {
        ASSERT(STATUS_BUFFER_TOO_SMALL == status);
        numDevices += 8;  //为已知的设备开辟内存空间进行存储。额外增加8字节。

        devList = (PDEVICE_OBJECT *)ExAllocatePool( NonPagedPool, (numDevices * sizeof(PDEVICE_OBJECT)) );
        if (NULL == devList)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
                
        status = IoEnumerateDeviceObjectList(FSDeviceObject->DriverObject,
                        devList,(numDevices * sizeof(PDEVICE_OBJECT)), &numDevices);
                
        if (!NT_SUCCESS( status ))
        {
            ExFreePool( devList );
            return status;
        }
                
        //依次遍历各个设备对象
        for (i=0; i < numDevices; i++)
        {
            storageStackDeviceObject = NULL;
            __try 
			{        
                //如果设备对象是文件系统CDO，或者是不符合类型的，或者是已经绑定的
                if ((devList[i] == FSDeviceObject) || (devList[i]->DeviceType != FSDeviceObject->DeviceType) || IsAttachedToDevice( devList[i], NULL ))
                {
                        __leave;//离开try块
                }
                                
				//获取设备驱动堆栈或文件系统最底层的设备对象的名字
                GetBaseDeviceObjectName( devList[i], Name );
                if (Name->Length > 0) //如果有名字，离开
                {
                        __leave;
                }

                /*
                调用IoGetDiskDeviceObject函数来获取一个与文件系统设备对象有关的磁盘设备对象。只绑定已经拥有一个磁盘设备对象的文件系统设备对象。
                */
				status = IoGetDiskDeviceObject( devList[i], &storageStackDeviceObject );

				if (!NT_SUCCESS( status ))
                {
                        __leave;
                }
                                
                status = IsShadowCopyVolume ( storageStackDeviceObject, &isShadowCopyVolume );

				//不绑定卷影
				if (NT_SUCCESS(status) && isShadowCopyVolume) 
				{
					//UNICODE_STRING shadowDeviceName;
					//WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

					//RtlInitEmptyUnicodeString( &shadowDeviceName, shadowNameBuffer, sizeof(shadowNameBuffer) );
					//GetObjectName( storageStackDeviceObject, &shadowDeviceName );

                    __leave;
                }
                                

                // 是一个磁盘设备对象，创建新的设备对象，准备绑定。
                status = IoCreateDevice( gMyDriverObject,
                            sizeof(DEVICE_EXTENSION ),
                            NULL,
                            devList[i]->DeviceType,
                            0,
                            FALSE,
                            &newDeviceObject );
                if (!NT_SUCCESS( status ))
                {
                        __leave;
                }
                                
                newDevExt = (PDEVICE_EXTENSION) newDeviceObject->DeviceExtension;
				newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
                RtlInitEmptyUnicodeString( &newDevExt->DeviceName, newDevExt->DeviceNameBuffer,sizeof(newDevExt->DeviceNameBuffer) );
                GetObjectName( storageStackDeviceObject, &newDevExt->DeviceName );

                /*
                    在绑定最后的时候，再测试下，该设备是否被绑定过。这里加了一个锁。如果没被绑定，则执行下面的绑定过程，否则，直接返回。
                */
                ExAcquireFastMutex( &gFastMutexAttachLock );
                if (!IsAttachedToDevice( devList[i], NULL ))
                {
                    status = AttachToMountedDevice( devList[i], newDeviceObject );
                    if (!NT_SUCCESS( status ))
                    {
                        CleanupMountedDevice( newDeviceObject );
                        IoDeleteDevice( newDeviceObject );
                    }
                }
                else
                {
                    CleanupMountedDevice( newDeviceObject );
                    IoDeleteDevice( newDeviceObject );
                }
                
                ExReleaseFastMutex( &gFastMutexAttachLock );
            }/*try end*/
		__finally 
			{       
				/*
					减少设备对象的计数，这个计数是由函数IoGetDiskDeviceObject增加的。成功绑定后，就减少该设备对象的计数。
					一旦成功绑定到devList[i]，I/O管理器会确定设备栈的下层设备，会一直存在，一直到这个文件系统栈被卸掉。
				*/
                if (storageStackDeviceObject != NULL)
                {
                    ObDereferenceObject( storageStackDeviceObject );
                }

                //减少设备对象的计数，这个计数是由函数IoEnumerateDeviceObjectList增加的。
                ObDereferenceObject( devList[i] );
			}
		}/* for end*/

        status = STATUS_SUCCESS;
        ExFreePool( devList );
    }
    return status;
}
