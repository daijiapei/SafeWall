
#include "SafeWall.h"
#include "myfs.h"

EXTERN_C DWORD FileFlags = FILE_SHOWICO|FILE_HIDE;

#pragma PAGEDCODE
NTSTATUS DriverControlDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	//KdPrint(("Enter DriverControlDispatchRoutine\n"));

	if (!IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		//KdPrint(("一条来自其他驱动的控制消息\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }
	//如果控制码来自自己的驱动，那么由自己处理

	//得到当前堆栈
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//得到输入缓冲区大小
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	//得到输出缓冲区大小
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	//得到IOCTL码
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
	//KdPrint(("Default：%X\n",code));
	ULONG info = 0;
	//KdPrint(("pDeviceObject = 0x%08x", pDeviceObject));
	//文件名
	UNICODE_STRING srcFilePath = {0};
	WCHAR dosFilePath[MY_MAX_PATH] = {0};
	RtlInitEmptyUnicodeString(&srcFilePath, dosFilePath, sizeof(dosFilePath));

	WCHAR * InputBuffer = (WCHAR *)pIrp->AssociatedIrp.SystemBuffer;

	
	switch(code)
	{
	case IOCTL_ENCRYPT:
		{
			
			InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
			RtlAppendUnicodeToString(&srcFilePath, InputBuffer);
			KdPrint(("加密：%wZ\n",&srcFilePath));
			
			SafeWallEncryptFile(&srcFilePath, NULL);
		}break;
	case IOCTL_DECRYPT:
		{
			InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
			RtlAppendUnicodeToString(&srcFilePath, InputBuffer);
			KdPrint(("解密：%wZ\n",&srcFilePath));

			SafeWallDecryptFile(&srcFilePath, NULL);
		}break;
	case IOCTL_START:
		{
			g_safewall_start = TRUE;
			KdPrint(("启动数据围墙"));
		}break;
	case IOCTL_STOP:
		{
			g_safewall_start = FALSE;
			KdPrint(("暂停数据围墙"));
		}break;
	case IOCTL_SET_USER_ACCESS:
		{
		}break;
	case IOCTL_SET_PROCESS_ACCESS:
		{
		}break;
	case IOCTL_SET_FILE_ACCESS:
		{
		}break;
	case IOCTL_SHOWICO:
		{
			if((int) *InputBuffer)
			{
				FileFlags |= FILE_SHOWICO;
			}
			else
			{
				FileFlags &= (~FILE_SHOWICO);
			}
			
		}break;
	case SAFEWALL_FILEHIDE:
		{
			if((int) *InputBuffer)
			{
				FileFlags |= FILE_HIDE;
			}
			else
			{
				FileFlags &= (~FILE_HIDE);
			}
		}break;
	case IOCTL_QUERY_FILEATTRIBUTES://咨询文件是否属于加密文件
		{
			DWORD * OutputBuffer = (DWORD*)pIrp->AssociatedIrp.SystemBuffer;
			DWORD Flags = NULL;
			//if(g_safewall_start && (FileFlags & FILE_SHOWICO))
			if(TRUE)
			{
				InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
				RtlAppendUnicodeToString(&srcFilePath, InputBuffer);
				LPSAFEWALL_OBJECT SafeWall = 
					(LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool , SAFEWALL_OBJECT_SIZE);
				if(SafeWall != NULL)
				{
					if(GetSafeWallByFilePath(&srcFilePath,NULL,SafeWall))
					{
						Flags = CheckSafeWallFlags(SafeWall);
					}
					ExFreePool(SafeWall);
				}
				
				KdPrint(("长度%d, 请求：%wZ\n",cbin,&srcFilePath));
				if(Flags & SAFEWALL_FLAG_OBJECT)//如果包含加密对象，则显示
				{
					* OutputBuffer = 1;
				}
				else
				{
					* OutputBuffer = 0;
				}
			}
			else
			{
				* OutputBuffer = 0;
			}
			info = sizeof(DWORD);
		}break;
	case IOCTL_TEST:
		{
			
			//InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
			KdPrint(("收到一条测试的消息\n"));
		}break;
	default:
		{
			KdPrint(("Default：%08x\n",code));
			status = STATUS_INVALID_VARIANT;
		}break;
	}

	end:
	//设置IRP的完成状态
	pIrp->IoStatus.Status = status;
	//设置IRP请求操作的字节数
	pIrp->IoStatus.Information = info;
	//结束IRP请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	//KdPrint(("Leave DriverControlDispatchRoutine\n"));
	return status;
}