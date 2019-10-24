
#include "SafeWall.h"

//文件系统分发函数
NTSTATUS  FileSystemControlMountVolume( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
NTSTATUS  DeviceControlLoadFileSystem( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
#pragma PAGEDCODE
NTSTATUS  FileSystemDeviceControl( IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp )
{
    /*
    参数说明:
    DeviceObject:    我们创建的设备对象。它是被绑定到文件系统控制设备对象栈上。
    */

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( pIrp );
    //PAGED_CODE();
    //ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
    //ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    switch (irpSp->MinorFunction) 
	{
	case IRP_MN_MOUNT_VOLUME://一个卷被挂载
		{
			//绑定一个卷
			return FileSystemControlMountVolume( DeviceObject, pIrp );
		}break;
	case IRP_MN_LOAD_FILE_SYSTEM: //加载文件系统
		{
			return DeviceControlLoadFileSystem( DeviceObject, pIrp );
		}break;
	case IRP_MN_USER_FS_REQUEST:
        {
            switch (irpSp->Parameters.FileSystemControl.FsControlCode) 
			{
				//解挂载一个卷，什么也不做，只是打印出来，然后传递给下一层
				//据说这样做是可以的，只是会引起少量的内存泄漏。
				//但是接挂载事件并不经常发生，所以可以暂时忽略
                case FSCTL_DISMOUNT_VOLUME:
                {
                    PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
                    break;
                }
            }
            break;
        }
    }        

	//传递给下层
    IoSkipCurrentIrpStackLocation( pIrp );
    return IoCallDriver( ((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
}


//绑定挂载卷
#pragma PAGEDCODE
NTSTATUS  FileSystemControlMountVolume( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    /*
    参数说明:

    DeviceObject:    它是我们创建的设备对象。它被绑定到文件系统CDO的设备栈上。
    Irp:             它是发送给文件系统CDO的挂载请求。它是一个新卷的挂载请求。
        
    */
        
    PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

    PDEVICE_OBJECT storageStackDeviceObject;

    // newDeviceObject是将要绑定到文件系统的卷设备对象上。或者说，这个newDeviceObject要被绑定到新挂载卷的设备卷上。
    PDEVICE_OBJECT newDeviceObject;

    PDEVICE_EXTENSION newDevExt;
    NTSTATUS status;
    BOOLEAN isShadowCopyVolume;
    

    /*PAGED_CODE();
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType));*/

    /*
    在把IRP发送到文件系统之前，当挂载请求到来时，Vpb->RealDevice保存的是：将要被挂载的磁盘设备对象。
    storageStackDeviceObject事先保存了VPB的值，这是因为：当IRP下发给底层驱动后，可能会改变。
    */
    storageStackDeviceObject = irpSp->Parameters.MountVolume.Vpb->RealDevice;

    status = IsShadowCopyVolume ( storageStackDeviceObject, &isShadowCopyVolume );

    if (NT_SUCCESS(status) && isShadowCopyVolume) 
    {
		UNICODE_STRING shadowDeviceName;
		WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

		RtlInitEmptyUnicodeString( &shadowDeviceName, shadowNameBuffer, sizeof(shadowNameBuffer) );
		GetObjectName( storageStackDeviceObject, &shadowDeviceName );

		//如果不打算绑定卷影则跳到下一层驱动
		IoSkipCurrentIrpStackLocation( Irp );
		return IoCallDriver( devExt->AttachedToDeviceObject, Irp );
	}

    status = IoCreateDevice( gMyDriverObject, sizeof( DEVICE_EXTENSION ), NULL, 
		DeviceObject->DeviceType,  0, FALSE, &newDeviceObject );

    /*如果不把IRP发送到文件系统中，那么文件系统就不会收到这个卷的挂载请求。*/
    if (!NT_SUCCESS( status ))
    {
        KdPrint(( "创建绑定挂载卷设备失败, status=%08x\n", status ));

		Irp->IoStatus.Information = 0;
		Irp->IoStatus.Status = status;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );

		return status;
	}

    //填写设备扩展，这样目的是：可以让完成函数更容易到storageStackDeviceObject
    newDevExt = (PDEVICE_EXTENSION)newDeviceObject->DeviceExtension;
	newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
	RtlInitEmptyUnicodeString( &newDevExt->DeviceName, newDevExt->DeviceNameBuffer, sizeof(newDevExt->DeviceNameBuffer) );
	GetObjectName( storageStackDeviceObject, &newDevExt->DeviceName );

    //在这里设置了事件对象，把它用在完成例程中。这样做的目的是：通知当前例程，文件系统已经完成了当前卷的挂载。
    KEVENT waitEvent;
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	IoCopyCurrentIrpStackLocationToNext ( Irp );
	IoSetCompletionRoutine( Irp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
	status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
	if (STATUS_PENDING == status) 
	{
		//等待更底层的驱动完成，然后就会调用完成例程。
		status = KeWaitForSingleObject( &waitEvent, Executive, KernelMode, FALSE, NULL );
		//ASSERT( STATUS_SUCCESS == status );
	}
	//ASSERT(KeReadStateEvent(&waitEvent) ||!NT_SUCCESS(Irp->IoStatus.Status));

	KdPrint(("挂载卷消息传递完成，接下来将对卷进行绑定\n"));
    //执行到了这里，说明卷的挂载已经完成，要开始绑定卷了。等到完成函数设置了事件之后，再来绑定卷。
	status = FileSystemControlMountVolumeComplete( DeviceObject, Irp, newDeviceObject );

    return status;
}

#pragma PAGEDCODE
NTSTATUS  DeviceControlLoadFileSystem( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
	//加载文件系统
    PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

    KEVENT waitEvent;
        
    KeInitializeEvent( &waitEvent,NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( Irp );
        
    IoSetCompletionRoutine( Irp,AutoCompletionRoutine, &waitEvent, TRUE,TRUE, TRUE );

    status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

    if (STATUS_PENDING == status) 
    {
		status = KeWaitForSingleObject( &waitEvent, Executive, KernelMode, FALSE, NULL );
        //ASSERT( STATUS_SUCCESS == status );
    }

	//ASSERT(KeReadStateEvent(&waitEvent) || !NT_SUCCESS(Irp->IoStatus.Status));

	KdPrint(("文件系统加载成功，下面开始将设备绑定到文件系统\n"));
    status = DeviceControlLoadFileSystemComplete( DeviceObject, Irp );
    
    return status;
}