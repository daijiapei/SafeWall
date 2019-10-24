
#include "SafeWall.h"

#pragma PAGEDCODE
NTSTATUS  DriverDirectoryControlDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
    NTSTATUS status;
	KEVENT waitEvent;
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];

    PAGED_CODE();

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject))
    {
		KdPrint(("我的驱动产生了一条路径控制的消息\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;//这里很多时候是无效的

	if(IRP_MN_QUERY_DIRECTORY != stack->MinorFunction)
	{
		IoSkipCurrentIrpStackLocation(pIrp);
		status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
		return status;
	}

	KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
	status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
	if (STATUS_PENDING == status)//若状态是 挂起
	{
		KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
	}

	if (!NT_SUCCESS(status) || stack->Parameters.QueryFile.Length == 0) {
        status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return status;
    }

	//KdPrint(("路径控制 FileName: %wZ\\%wZ. FileInformationClass:%d,Length=%d, FileIndex=%d, flags=0x%08x",\
	//	&pFileObject->FileName,stack->Parameters.QueryDirectory.FileName,\
	//	stack->Parameters.QueryDirectory.FileInformationClass,\
	//	stack->Parameters.QueryDirectory.Length,stack->Parameters.QueryDirectory.FileIndex,\
	//	pIrp->Flags));
	
	char * buffer = (char*)pIrp->UserBuffer;
	PULONG BufferSize = &stack->Parameters.QueryDirectory.Length;

	switch(stack->Parameters.QueryDirectory.FileInformationClass)
    {
	case FileBothDirectoryInformation:
		{
			FILE_BOTH_DIR_INFORMATION* pBothDir = (FILE_BOTH_DIR_INFORMATION *)buffer;
			
			ULONG offset = 0;
			//RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
			RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));
			RtlAppendUnicodeToString(&tarFileName, L"test.txt");
			do
			{
				offset = pBothDir->NextEntryOffset;
				
				/*KdPrint(("Both路径控制 FileName: %wZ\\%ws.FileNameLength:%d. FileInformationClass:%d,Length=%d, FileIndex=%d, flags=0x%08x",\
						&pFileObject->FileName,pBothDir->FileName,pBothDir->FileNameLength,\
						stack->Parameters.QueryDirectory.FileInformationClass,\
						stack->Parameters.QueryDirectory.Length,stack->Parameters.QueryDirectory.FileIndex,\
						pIrp->Flags));*/

				srcFileName.MaximumLength = srcFileName.Length = pBothDir->FileNameLength;
				srcFileName.Buffer = pBothDir->FileName;
				if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
				{
					KdPrint(("filename:%wZ Both路径控制 AllocationSize:%d. EndOfFile:%d",&srcFileName,\
						pBothDir->AllocationSize.LowPart,pBothDir->EndOfFile.LowPart));

					//pBothDir->AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
					pBothDir->EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
				}
				pBothDir = (PFILE_BOTH_DIR_INFORMATION)((ULONG)pBothDir + offset);
			}while(offset);
		}break;
	case FileDirectoryInformation:
		{
			FILE_DIRECTORY_INFORMATION * pDirectory = (FILE_DIRECTORY_INFORMATION *) buffer;
			//pDirectory->AllocationSize.QuadPart  -= SAFEWALL_OBJECT_SIZE;
			//pDirectory->EndOfFile.QuadPart  -= SAFEWALL_OBJECT_SIZE;
		}break;
	case FileFullDirectoryInformation:
		{
			FILE_FULL_DIR_INFORMATION * pFullDir = (FILE_FULL_DIR_INFORMATION*) buffer;
			//pFullDir->AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
			//pFullDir->EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
		}break;
	case FileIdBothDirectoryInformation:
		{
			FILE_ID_BOTH_DIR_INFORMATION * pIdBothDir = (FILE_ID_BOTH_DIR_INFORMATION*)buffer;
			//pIdBothDir->AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
			//pIdBothDir->EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
		}break;
	case FileIdFullDirectoryInformation:
		{
			FILE_ID_FULL_DIR_INFORMATION * pIdFullDir = (FILE_ID_FULL_DIR_INFORMATION*)buffer;
			//pIdFullDir->AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
			//pIdFullDir->EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
		}break;
	case FileNamesInformation:
		{
			FILE_NAMES_INFORMATION * pNames = (FILE_NAMES_INFORMATION*)buffer;
		}break;
	case FileObjectIdInformation:
		{
			FILE_OBJECTID_INFORMATION * pObjectid = (FILE_OBJECTID_INFORMATION*)buffer;
		}break;
	case FileQuotaInformation:
		{
			//IRP_MJ_QUERY_QUOTA
		}break;
	case FileReparsePointInformation:
		{
			FILE_REPARSE_POINT_INFORMATION * pReparsePoint = (FILE_REPARSE_POINT_INFORMATION *) buffer;
		}break;
	default:
		break;
	}

    status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return status;

}
