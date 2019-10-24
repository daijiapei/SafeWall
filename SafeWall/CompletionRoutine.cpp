
#include "SafeWall.h"

//对完成函数的默认处理方法，就是设置事件已经完成
#pragma PAGEDCODE
NTSTATUS  AutoCompletionRoutine( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context )
{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    ASSERT(Context != NULL);

    KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma PAGEDCODE
NTSTATUS  DeviceControlLoadFileSystemComplete ( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS status;

    PAGED_CODE();

    devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    if (!NT_SUCCESS( Irp->IoStatus.Status ) && (Irp->IoStatus.Status != STATUS_IMAGE_ALREADY_LOADED))
    {
		KdPrint(("正式将设备绑定到文件系统\n"));
        IoAttachDeviceToDeviceStackSafe( DeviceObject, devExt->AttachedToDeviceObject, &devExt->AttachedToDeviceObject );
        ASSERT(devExt->AttachedToDeviceObject != NULL);
    }
    else
    {
        CleanupMountedDevice( DeviceObject );
        IoDeleteDevice( DeviceObject );
    }

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

#pragma PAGEDCODE
NTSTATUS  FileSystemControlMountVolumeComplete( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PDEVICE_OBJECT NewDeviceObject )
{
        /*
        参数说明:

        DeviceObject:        它是绑定到文件系统控制设备对象的设备栈上，它是一个过滤设备对象。
        Irp:                 它是发送给文件系统CDO的挂载请求。它是一个新卷的挂载请求。
		NewDeviceObject:     它是新创建的过滤设备对象，用于绑定到文件系统的卷设备对象的设备栈上。

        */

    PVPB vpb;
    PDEVICE_EXTENSION newDevExt;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT attachedDeviceObject;
    NTSTATUS status;

    PAGED_CODE();

    newDevExt = (PDEVICE_EXTENSION)NewDeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

   /*
    * 获取我们保存的VPB，这个时候就可以通过该设备对象得到VPB
    * VPB->DeviceObject是  文件系统创建的卷设备对象
    * VPB->RealDevice是    磁盘驱动创建的物理设备对象
    */
    vpb = newDevExt->StorageStackDeviceObject->Vpb;
    if (vpb != irpSp->Parameters.MountVolume.Vpb)
    {
		KdPrint(("irpSp->Parameters.MountVolume.Vpb 有所改变\n"));
    }

    if (NT_SUCCESS( Irp->IoStatus.Status ))
    {
		//请求快速互斥体
        ExAcquireFastMutex( &gFastMutexAttachLock );
        if (!IsAttachedToDevice( vpb->DeviceObject, &attachedDeviceObject ))
        {          
			/*
			* SfAttachToMountedDevice的意义：将我们创建的过滤设备对象NewDeviceObject绑定到文件系统创建的VPB->DeviceObject的设备对象栈上。                                        
            */
            status = AttachToMountedDevice( vpb->DeviceObject, NewDeviceObject );
            if (!NT_SUCCESS( status ))
            {
                CleanupMountedDevice( NewDeviceObject );
                IoDeleteDevice( NewDeviceObject );
            }
            ASSERT( NULL == attachedDeviceObject );
        }
        else
        {
            CleanupMountedDevice( NewDeviceObject );
            IoDeleteDevice( NewDeviceObject );
            ObDereferenceObject( attachedDeviceObject );
        }
		//释放快速互斥体
        ExReleaseFastMutex( &gFastMutexAttachLock );
    }
    else
    {
        CleanupMountedDevice( NewDeviceObject );
        IoDeleteDevice( NewDeviceObject );
    }
    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}
