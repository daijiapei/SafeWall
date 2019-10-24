
#include "SafeWall.h"

#define IS_WINDOWS2000() ((gOsMajorVersion == 5) && (gOsMinorVersion == 0))
#define IS_WINDOWSXP() ((gOsMajorVersion == 5) && (gOsMinorVersion == 1))

NTSTATUS  IsShadowCopyVolume( IN PDEVICE_OBJECT StorageStackDeviceObject, OUT PBOOLEAN IsShadowCopy )
{

/*******************************************************************************************************************************************
SfIsShadowCopyVolume( )函数的主要功能是：涉及到 卷影拷贝服务的。

卷影拷贝服务(Volume Shadow Copy Service，VSS)是一种备份和恢复的技术。它是一种基于时间点来备份文件拷贝的技术。
通过使用卷影拷贝服务，我们可以在特定卷上建立数据拷贝时间点，并在将来的某一时刻把数据恢复到任何一个你曾创建的时间点的状态。
最常见的请求一般是恢复人为原因造成的数据丢失。用户不经意地存储了有错误信息的文件，或者是不小心删除文件，或是其他的数据意外。
备份VSS快照产生干净的数据镜像以及恢复时间点拷贝的能力。我们既可以恢复整个快照，也可以取所需，或者还可以使用VSS备份工具来恢复单独的文件和文件夹。

VSS不是对应用程序进行备份的，VSS可以对逻辑卷(硬盘分区)进行快照。
VSS是Windows下的快照技术，由Requestor, Write,和Provider组成；主要由驱动程序Volsnap.sys实现，被加到卷管理驱动和文件系统驱动之间，同时集成了COM技术。
因此，它不再是单单在卷级上的block进行处理，而是和各种系统应用相关联，比如SQL，EXCHANGE，AD等等。从而使得在不关机，也不停止应用的情况下，做快照。
VSS被广泛的应用到Windows的备份处理中。

VSS 解决问题的方法是，通过提供以下三个重要实体之间的通讯来保证备份的高度真实和恢复过程的简便。
(1)请求程序：它是用来请求时间点数据副本或卷影副本的应用程序，比如备份或存储管理应用程序。
(2)写入程序：它们负责数据通知和数据保护。写入程序是VSS区别于其它卷影副本或快照解决方案的地方。在VSS的卷影复制过程中会涉及一些应用程序。
(3)提供程序：它用于暴露基于硬件或软件的卷影副本的机制。许多存储硬件供应商都会为它们的存储阵列编写提供程序。

VSS服务唯一的缺点是：
我们需要为每一个卷影留出更多的磁盘空间，我们必须在某处存储这些拷贝。因为VSS使用指针数据，这些拷贝占用的空间要比想像的小得多，我们可以有效地存储这些拷贝。

有关VSS的更多说明，可以去浏览Microsoft的下述网站

http://technet.microsoft.com/en-us/library/ee923636.aspx

*********************************************************************************************************************************************/
    PAGED_CODE();

    *IsShadowCopy = FALSE;

#if WINVER >= 0x0501
    if (IS_WINDOWS2000())
        {
#endif
                UNREFERENCED_PARAMETER( StorageStackDeviceObject );
                return STATUS_SUCCESS;
#if WINVER >= 0x0501
        }
        
        if (IS_WINDOWSXP())
        {
                UNICODE_STRING volSnapDriverName;
                WCHAR buffer[MAX_DEVNAME_LENGTH];
                PUNICODE_STRING storageDriverName;
                ULONG returnedLength;
                NTSTATUS status;
                
                if (FILE_DEVICE_DISK != StorageStackDeviceObject->DeviceType)
                {
                        return STATUS_SUCCESS;
                }
                
                storageDriverName = (PUNICODE_STRING) buffer;
                RtlInitEmptyUnicodeString( storageDriverName, (PWCHAR)Add2Ptr( storageDriverName, sizeof( UNICODE_STRING ) ), sizeof( buffer ) - sizeof( UNICODE_STRING ) );
                status = ObQueryNameString( StorageStackDeviceObject, (POBJECT_NAME_INFORMATION)storageDriverName, storageDriverName->MaximumLength, &returnedLength );
                if (!NT_SUCCESS( status ))
                {
                        return status;
                }
                
                RtlInitUnicodeString( &volSnapDriverName, L"\\Driver\\VolSnap" );
				//比较字符串，不区分大小写
                if (RtlEqualUnicodeString( storageDriverName, &volSnapDriverName, TRUE ))
                {
                        *IsShadowCopy = TRUE;
                }
                else
                {
                        NOTHING;
                }
                
                return STATUS_SUCCESS;
        }
        else
        {
                PIRP irp;
                KEVENT event;
                IO_STATUS_BLOCK iosb;
                NTSTATUS status;
                if (FILE_DEVICE_VIRTUAL_DISK != StorageStackDeviceObject->DeviceType)
                {
                        return STATUS_SUCCESS;
                }
                
                KeInitializeEvent( &event, NotificationEvent, FALSE );

           /*
                *Microsoft WDK官方文档对 IOCTL_DISK_IS_WRITABLE是这样解释的：
                *Determines whether a disk is writable.
                *The Status field can be set to STATUS_SUCCESS, or possibly to STATUS_INSUFFICIENT_RESOURCES, STATUS_IO_DEVICE_ERROR, or STATUS_MEDIA_WRITE_PROTECTED.
                *
                *IOCTL_DISK_IS_WRITABLE是没有输入也没有输出的。
                */
                irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_IS_WRITABLE,
                                             StorageStackDeviceObject,
                                             NULL,
                                             0,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &iosb );
                if (irp == NULL)
                {
                        return STATUS_INSUFFICIENT_RESOURCES;
                }
                
                status = IoCallDriver( StorageStackDeviceObject, irp );
                if (status == STATUS_PENDING)
                {
                        (VOID)KeWaitForSingleObject( &event,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         NULL );
                        status = iosb.Status;
                }

                if (STATUS_MEDIA_WRITE_PROTECTED == status)
                {
                        *IsShadowCopy = TRUE;
                        status = STATUS_SUCCESS;
                }

                return status;
        }
#endif
}