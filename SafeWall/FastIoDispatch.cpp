
#include "SafeWall.h"

#pragma PAGEDCODE
BOOLEAN  FastIoCheckIfPossible( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN BOOLEAN CheckForReadOperation, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
	//无需额外处理
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
		nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
		ASSERT(nextDeviceObject);
		fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
		if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoCheckIfPossible ))
        {
            return (fastIoDispatch->FastIoCheckIfPossible)(
            FileObject,
            FileOffset,
            Length,
            Wait,
            LockKey,
            CheckForReadOperation,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, OUT PVOID Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
	PSAFEWALL_FILE_LIST pFileList;
	DWORD FileFlags;
    PAGED_CODE();

	pFileList = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext);
	FileFlags = pFileList ? pFileList->flags : NULL ;

    if (DeviceObject->DeviceExtension) 
	{
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead ))
        {
            return (fastIoDispatch->FastIoRead)(
            FileObject,
            FileOffset,
            Length,
            Wait,
            LockKey,
            Buffer,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN PVOID Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{

    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

	RtlAppendUnicodeToString(&tarFileName, L"test.txt");
	GetFileNameForPath(&FileObject->FileName , &srcFileName);
	if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		buffer = (char*)Buffer;
		KdPrint(("FastIoWrite buffer:%s",buffer));
	}
#endif
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
		nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
		ASSERT(nextDeviceObject);
		fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

		if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite ))
		{
			return (fastIoDispatch->FastIoWrite)(
			FileObject,
			FileOffset,
			Length,
			Wait,
			LockKey,
			Buffer,
			IoStatus,
			nextDeviceObject );
		}
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoQueryBasicInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_BASIC_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
		nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
		ASSERT(nextDeviceObject);
		fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
		if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo ))
		{
			return (fastIoDispatch->FastIoQueryBasicInfo)(
			FileObject,
			Wait,
			Buffer,
			IoStatus,
			nextDeviceObject );
		}
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoQueryStandardInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_STANDARD_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
	BOOLEAN bRet;
    PAGED_CODE();
#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

	RtlAppendUnicodeToString(&tarFileName, L"test.txt");
	GetFileNameForPath(&FileObject->FileName , &srcFileName);
	
#endif


    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
		nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
		ASSERT(nextDeviceObject);
		fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
		if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryStandardInfo ))
        {
            bRet = (fastIoDispatch->FastIoQueryStandardInfo)(
            FileObject,
            Wait,
            Buffer,
            IoStatus,
            nextDeviceObject );

			//if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
			//{
			//	buffer = (char*)Buffer;
			//	PFILE_STANDARD_INFORMATION pStandardInformation = (PFILE_STANDARD_INFORMATION)Buffer;
			//	/*KdPrint(("FastIoQueryStandardInfo FileName:%wZ .EndOfFile=%d",\
			//		&FileObject->FileName,pStandardInformation->EndOfFile.LowPart));*/
			//	
			//	pStandardInformation->AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
			//	pStandardInformation->EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
			//	/*KdPrint(("FastIoQueryStandardInfo FileName:%wZ .EndOfFile=%d",\
			//		&FileObject->FileName,pStandardInformation->EndOfFile.LowPart));*/
			//	
			//}

			
			//return bRet;
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoLock( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key, BOOLEAN FailImmediately, BOOLEAN ExclusiveLock, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
	//无需额外处理
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
		nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
		ASSERT(nextDeviceObject);
		fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
		if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock ))
        {
            return (fastIoDispatch->FastIoLock)(
            FileObject,
            FileOffset,
            Length,
            ProcessId,
            Key,
            FailImmediately,
            ExclusiveLock,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoUnlockSingle( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
	//无需额外处理
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
		nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
		ASSERT(nextDeviceObject);
		fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
		if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle ))
        {
            return (fastIoDispatch->FastIoUnlockSingle)(
            FileObject,
            FileOffset,
            Length,
            ProcessId,
            Key,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoUnlockAll( IN PFILE_OBJECT FileObject, PEPROCESS ProcessId, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
	//无需额外处理
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        if (nextDeviceObject)
        {
            fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
            if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll ))
            {
                return (fastIoDispatch->FastIoUnlockAll)(
                FileObject,
                ProcessId,
                IoStatus,
                nextDeviceObject );
            }
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoUnlockAllByKey( IN PFILE_OBJECT FileObject, PVOID ProcessId, ULONG Key, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
	//无需额外处理
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey ))
        {
            return (fastIoDispatch->FastIoUnlockAllByKey)(
            FileObject,
            ProcessId,
            Key,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoDeviceControl( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength, OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength, IN ULONG IoControlCode, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{

    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
	//如果消息是属于我们自己的控制码，会发送到控制例程中
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl ))
        {
            return (fastIoDispatch->FastIoDeviceControl)(
            FileObject,
            Wait,
            InputBuffer,
            InputBufferLength,
            OutputBuffer,
            OutputBufferLength,
            IoControlCode,
            IoStatus,
            nextDeviceObject );
        }
    }

    return FALSE;
}


#pragma PAGEDCODE
VOID  FastIoDetachDevice( IN PDEVICE_OBJECT SourceDevice, IN PDEVICE_OBJECT TargetDevice )
{
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();
	//不用额外处理
    ASSERT(IS_MY_DEVICE_OBJECT( SourceDevice ));
    devExt = (PDEVICE_EXTENSION)SourceDevice->DeviceExtension;
    //SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
    //              ("SFilter!SfFastIoDetachDevice:                Detaching from volume      %p \"%wZ\"\n",
    //               TargetDevice, &devExt->DeviceName) );
    CleanupMountedDevice( SourceDevice );
    IoDetachDevice( TargetDevice );
    IoDeleteDevice( SourceDevice );
}

#pragma PAGEDCODE
BOOLEAN  FastIoQueryNetworkOpenInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_NETWORK_OPEN_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo ))
        {
            return (fastIoDispatch->FastIoQueryNetworkOpenInfo)(
            FileObject,
            Wait,
            Buffer,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoMdlRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

	RtlAppendUnicodeToString(&tarFileName, L"test.txt");
	GetFileNameForPath(&FileObject->FileName , &srcFileName);
	if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		buffer = (char *)MmGetSystemAddressForMdlSafe(*MdlChain,NormalPagePriority);
		KdPrint(("FastIoMdlRead buffer:%s",buffer));
	}
#endif
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead ))
        {
            return (fastIoDispatch->MdlRead)(
            FileObject,
            FileOffset,
            Length,
            LockKey,
            MdlChain,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoMdlReadComplete( IN PFILE_OBJECT FileObject, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

#endif
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete ))
        {
			return (fastIoDispatch->MdlReadComplete)( FileObject, MdlChain, nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoPrepareMdlWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

#ifdef DBG
	//准备写入不用作额外处理
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

#endif

    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite ))
        {
            return (fastIoDispatch->PrepareMdlWrite)(
            FileObject,
            FileOffset,
            Length,
            LockKey,
            MdlChain,
            IoStatus,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoMdlWriteComplete( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

#endif
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete ))
        {
            return (fastIoDispatch->MdlWriteComplete)(
            FileObject,
            FileOffset,
            MdlChain,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoReadCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PVOID Buffer, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo, IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

#endif
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed ))
        {
            return (fastIoDispatch->FastIoReadCompressed)(
            FileObject,
            FileOffset,
            Length,
            LockKey,
            Buffer,
            MdlChain,
            IoStatus,
            CompressedDataInfo,
            CompressedDataInfoLength,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoWriteCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, IN PVOID Buffer, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo, IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

#endif
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed ))
        {
            return (fastIoDispatch->FastIoWriteCompressed)(
            FileObject,
            FileOffset,
            Length,
            LockKey,
            Buffer,
            MdlChain,
            IoStatus,
            CompressedDataInfo,
            CompressedDataInfoLength,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoMdlReadCompleteCompressed( IN PFILE_OBJECT FileObject, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

#ifdef DBG
	//读取完成不用作额外处理
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

#endif
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadCompleteCompressed ))
        {
            return (fastIoDispatch->MdlReadCompleteCompressed)(
            FileObject,
            MdlChain,
            nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoMdlWriteCompleteCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

#ifdef DBG
	//写入完成不用做额外处理
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	char * buffer;

	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

#endif
    if (DeviceObject->DeviceExtension) 
	{
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteCompleteCompressed )) 
		{
            return (fastIoDispatch->MdlWriteCompleteCompressed)(
			FileObject,
			FileOffset,
			MdlChain,
			nextDeviceObject );
        }
    }
    return FALSE;
}

#pragma PAGEDCODE
BOOLEAN  FastIoQueryOpen( IN PIRP Irp, OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation, IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN result;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);
        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen ))
        {
            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
            irpSp->DeviceObject = nextDeviceObject;
            result = (fastIoDispatch->FastIoQueryOpen)(
            Irp,
            NetworkInformation,
            nextDeviceObject );

			irpSp->DeviceObject = DeviceObject;
			return result;
        }
    }
    return FALSE;
}



//========================== FSFilter 回调函数 ===========================
#pragma PAGEDCODE
NTSTATUS  PreFsFilterPassThrough( IN PFS_FILTER_CALLBACK_DATA Data, OUT PVOID *CompletionContext )
{
	//默认处理
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( CompletionContext );

    ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );

    return STATUS_SUCCESS;
}

#pragma PAGEDCODE
VOID  PostFsFilterPassThrough ( IN PFS_FILTER_CALLBACK_DATA Data, IN NTSTATUS OperationStatus, IN PVOID CompletionContext )
{
	//默认处理
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( OperationStatus );
    UNREFERENCED_PARAMETER( CompletionContext );

    ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );
}