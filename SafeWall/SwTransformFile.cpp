
#include "SafeWall.h"
#include "stdio.h"
#include "myfs.h"

void Dehead(LPSAFEWALL_OBJECT pobj);
void Enhead(LPSAFEWALL_OBJECT pobj);
int CreateSafeWallFilePath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING newFilePath);
NTSTATUS GetMyDeviceObjectByFilePath(IN PUNICODE_STRING FilePath, OUT PDEVICE_OBJECT *DeviceObject);
NTSTATUS SafeWallReplaceFile(HANDLE srcFile, PFILE_OBJECT srcFileObject, POBJECT_ATTRIBUTES srcFileAttributes,
							 HANDLE newFile, PFILE_OBJECT newFileObject ,POBJECT_ATTRIBUTES newFileAttributes, PDEVICE_OBJECT pDeviceObject);

//加密文件
#pragma PAGEDCODE
NTSTATUS SafeWallEncryptFile(PUNICODE_STRING srcFilePath ,PDEVICE_OBJECT DeviceObject)//加密文件
{
	//要注意DeviceObject一般是源头设备所附加的设备，加密时要往下级设备发消息
	DWORD hFlags = NULL;
	HANDLE srcFile;
	PFILE_OBJECT newFileObject, srcFileObject;
	LPSAFEWALL_OBJECT safewall;
	IO_STATUS_BLOCK srcIoStatus = {0};
	LARGE_INTEGER srcOffset = {0};
	OBJECT_ATTRIBUTES srcFileAttributes;
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = DeviceObject;
	PAGED_CODE();
	//文件必须是全路径
	if(NULL == pDeviceObject || pDeviceObject == gMyControlDeviceObject)
	{
		status = GetMyDeviceObjectByFilePath(srcFilePath, &pDeviceObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("获取文件对应的磁盘设备失败"));
			return status; 
		}
		pDeviceObject = ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject;
	}
	//无法独占打开
	InitializeObjectAttributes(&srcFileAttributes, srcFilePath,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
	
	status = devCreateFile(&srcFile, &srcFileObject, FILE_GENERIC_READ ,&srcFileAttributes, 
		&srcIoStatus, NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT , 
		pDeviceObject);

	/*status = ZwCreateFile(&srcFile,FILE_GENERIC_READ, &srcFileAttributes,
		&srcIoStatus, NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT,NULL,NULL);*/
	
	if(!NT_SUCCESS(status))
	{
		KdPrint(("打开文件 %wZ 失败",srcFilePath));
		return status;
	}
	KdPrint(("打开文件 %wZ 成功",srcFilePath));
	
	//创建safewall对象，并获取文件信息
	safewall = (LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool , SAFEWALL_OBJECT_SIZE);
	if(GetSafeWallByFileHandle(srcFile,srcFileObject, pDeviceObject, safewall))
	{
		hFlags = CheckSafeWallFlags(safewall);
	}
	
	
	KdPrint(("hFlags = 0x%08x",hFlags));
	if(hFlags & SAFEWALL_FLAG_FILEGROUP)//如果拥有文件组标志，表示已经被加密了
	{
		CallNextCloseFile(srcFile,srcFileObject);
		//devCloseFile(srcFile,pDeviceObject);
		KdPrint(("%wZ 文件已经加密",srcFilePath));
		goto end;
	}
	else
	{
		HANDLE newFile;
		UNICODE_STRING newFilePath = {0}; //文件全路径
		UNICODE_STRING FileName = {0};//文件名
		WCHAR dosFilePath[MY_MAX_PATH] = {0}; //文件全路径缓冲
		FILE_STANDARD_INFORMATION FileStandardInfo = {0};//文件信息
		IO_STATUS_BLOCK newStatus={0};
		OBJECT_ATTRIBUTES newFileAttributes;
		LARGE_INTEGER newOffset = {0};
		char * buffer = NULL;
		
		RtlInitEmptyUnicodeString(&newFilePath, dosFilePath, sizeof(dosFilePath));
		CreateSafeWallFilePath(srcFilePath, &newFilePath);

		//申请缓冲区
		buffer = (char *)ExAllocatePool(NonPagedPool, FILEPAGE_BUFFER_SIZE);
		
		status = devQueryInformationFile(srcFile,srcFileObject, &srcIoStatus,&FileStandardInfo,
			sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation,pDeviceObject);
		FileStandardInfo.EndOfFile.QuadPart += SAFEWALL_OBJECT_SIZE;//新文件长度

		KdPrint(("新文件： %wZ\t, 长度:%d",&newFilePath,FileStandardInfo.EndOfFile.LowPart));
		InitializeObjectAttributes(&newFileAttributes, &newFilePath,
			OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
		
		//无法独占打开
		status = devCreateFile(&newFile,&newFileObject, FILE_GENERIC_WRITE,&newFileAttributes, 
			&srcIoStatus, &FileStandardInfo.EndOfFile,FILE_ATTRIBUTE_NORMAL, NULL, FILE_CREATE, 
			FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT, 
			pDeviceObject);

		/*status = ZwCreateFile(&newFile, FILE_GENERIC_WRITE,&newFileAttributes,
			&newStatus,&FileStandardInfo.EndOfFile ,FILE_ATTRIBUTE_NORMAL,NULL,FILE_CREATE, 
			FILE_NON_DIRECTORY_FILE| FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);*/

		if(!NT_SUCCESS(status))
		{
			KdPrint(("创建新文件失败 status = 0x%08x",status));
			//CallNextCloseFile(srcFile, srcFileObject);
			CallNextCloseFile(srcFile,srcFileObject);
			ExFreePool(buffer);//释放缓冲区
			goto end;
		}
		KdPrint(("创建新文件成功"));

		//写入文件头
		InitSafeWallObject(safewall);
		status = SetSafeWallToFileHead(safewall, newFile,newFileObject, pDeviceObject);

		if(!NT_SUCCESS(status))
		{
			KdPrint(("文件头写入时发生错误 status = 0x%08x",status));
			CallNextCloseFile(srcFile,srcFileObject);
			CallNextCloseFile(newFile,newFileObject);
			/*devCloseFile(srcFile,pDeviceObject);
			devCloseFile(newFile,pDeviceObject);*/
			ZwDeleteFile(&newFileAttributes);
			ExFreePool(buffer);//释放缓冲区
			goto end;
		}
		newOffset.QuadPart = SAFEWALL_OBJECT_SIZE;
		srcOffset.QuadPart = 0;
		
		//读取原文件，加密并复制到临时文件
		while(TRUE)
		{
			status = devReadFile(srcFile,srcFileObject, &srcIoStatus, buffer,
				PAGE_BUFFER_SIZE, &srcOffset, pDeviceObject);

			/*status = ZwReadFile(srcFile,NULL,NULL,NULL, &srcIoStatus, buffer,
				FILEPAGE_BUFFER_SIZE, &srcOffset, NULL);*/
			if(!NT_SUCCESS(status))
			{
				if(status == STATUS_END_OF_FILE)
				{
					KdPrint(("文件读完：%d",srcOffset.LowPart ));
					status = STATUS_SUCCESS;
				}
				break;
			}
			//KdPrint(("读取：%d, %s",srcOffset.LowPart,buffer ));
			//加密缓冲区
			Encode(safewall->AlgorithmVersion, buffer ,srcIoStatus.Information, &srcOffset, 
				&safewall->privateKey, &safewall->CompanyId);
			srcOffset.QuadPart += srcIoStatus.Information;
			//将缓冲写入新文件
			KdPrint(("srcIoStatus.Information = %u,srcOffset=%d,newOffset=%d",
				srcIoStatus.Information,srcOffset.LowPart,newOffset.LowPart));
			status = devWriteFile(newFile, newFileObject, &newStatus, buffer, srcIoStatus.Information,
				&newOffset, pDeviceObject);
			/*status = ZwWriteFile(newFile,NULL, NULL,NULL, &newStatus, buffer,  srcIoStatus.Information,
				&newOffset, NULL);*/
			if(!NT_SUCCESS(status))
			{
				//error:STATUS_WORKING_SET_QUOTA
				KdPrint(("文件写入时发生错误,取消操作 status = 0x%08x",status));
				CallNextCloseFile(srcFile, srcFileObject);
				CallNextCloseFile(newFile, newFileObject);
				/*devCloseFile(srcFile,pDeviceObject);
				devCloseFile(newFile,pDeviceObject);*/
				//ZwDeleteFile(&newFileAttributes);
				ExFreePool(buffer);//释放缓冲区

				status = STATUS_DATA_ERROR;
				goto end;
			}
			//KdPrint(("写入位置：%d,写入长度:%d",newOffset.LowPart,newStatus.Information));
			newOffset.QuadPart += newStatus.Information;
		}
		ExFreePool(buffer);//释放缓冲区
		
		/*status = SafeWallReplaceFile(srcFile, &srcFileAttributes, newFile,
			&newFileAttributes, pDeviceObject);*/

		UNICODE_STRING newFileName = {0};//文件名
		IO_STATUS_BLOCK ioStatus={0};
		MY_FILE_RENAME_INFORMATION FileRenameInfo ={0}; //改名结构体

		RtlInitEmptyUnicodeString(&newFileName, FileRenameInfo.FileName, sizeof(FileRenameInfo.FileName));
		GetFileNameForPath(srcFileAttributes.ObjectName, &newFileName);
		FileRenameInfo.FileNameLength = newFileName.Length;
		FileRenameInfo.ReplaceIfExists = 0 ;
		FileRenameInfo.RootDirectory = NULL;

		CallNextCloseFile(srcFile,srcFileObject);
		status = ZwDeleteFile(&srcFileAttributes);//删除源文件
		if(NT_SUCCESS(status))
		{
			KdPrint(("文件删除成功，替换新文件"));

			status = devSetInformationFile(newFile, newFileObject,  &ioStatus,&FileRenameInfo,
				sizeof(MY_FILE_RENAME_INFORMATION), FileRenameInformation,pDeviceObject);
		
			CallNextCloseFile(newFile, newFileObject);
			if(!NT_SUCCESS(status))
			{
				KdPrint(("重命名或设置文件信息失败：status = 0x%08x", status));	
			}
		}
		else
		{
			KdPrint(("源文件删除失败，取消操作。status = 0x%08x",status));
			CallNextCloseFile(newFile, newFileObject);
			ZwDeleteFile(&newFileAttributes);
		}
	}
	end:
	ExFreePool(safewall);
	return status;
}

//解密文件
#pragma PAGEDCODE
NTSTATUS SafeWallDecryptFile(PUNICODE_STRING srcFilePath, PDEVICE_OBJECT DeviceObject)//解密文件
{
	////要注意DeviceObject一般是源头设备所附加的设备，加密时要往下级设备发消息
	DWORD hFlags=NULL;
	HANDLE srcFile;
	PFILE_OBJECT srcFileObject, newFileObject;
	LPSAFEWALL_OBJECT safewall;
	IO_STATUS_BLOCK srcIoStatus = {0};
	LARGE_INTEGER srcOffset = {0};
	OBJECT_ATTRIBUTES srcFileAttributes;
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = DeviceObject;
	PAGED_CODE();
	//文件必须是全路径
	if(NULL == pDeviceObject || pDeviceObject == gMyControlDeviceObject)
	{
		status = GetMyDeviceObjectByFilePath(srcFilePath, &pDeviceObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("获取文件对应的磁盘设备失败"));
			return status;
		}
		pDeviceObject = ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject;
	}
	InitializeObjectAttributes(&srcFileAttributes, srcFilePath,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

	status = devCreateFile(&srcFile, &srcFileObject, FILE_GENERIC_READ ,&srcFileAttributes, 
		&srcIoStatus, NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT , 
		pDeviceObject);

	if(!NT_SUCCESS(status))
	{
		KdPrint(("打开文件 %wZ 失败",srcFilePath));
		return status;
	}
	KdPrint(("打开文件 %wZ 成功",srcFilePath));

	
	//创建safewall对象，并获取文件信息
	safewall = (LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool , SAFEWALL_OBJECT_SIZE);
	if(GetSafeWallByFileHandle(srcFile, srcFileObject,pDeviceObject, safewall))
	{
		hFlags = CheckSafeWallFlags(safewall);
	}
	KdPrint(("hFlags = 0x%08x",hFlags));
	if(!(hFlags & SAFEWALL_FLAG_FILEGROUP))//文件并没有被加密
	{
		CallNextCloseFile(srcFile,srcFileObject);
		KdPrint(("%wZ 文件未加密",srcFilePath));
		goto end;
	}
	else
	{
		//文件已经被加密了，需要解密
		HANDLE newFile;
		UNICODE_STRING newFilePath = {0}; //文件全路径
		UNICODE_STRING FileName = {0};//文件名
		WCHAR dosFilePath[MY_MAX_PATH] = {0}; //文件全路径缓冲
		FILE_STANDARD_INFORMATION FileStandardInfo = {0};//文件信息
		IO_STATUS_BLOCK newStatus={0};
		OBJECT_ATTRIBUTES newFileAttributes;
		LARGE_INTEGER newOffset = {0};
		char * buffer = NULL;
		
		RtlInitEmptyUnicodeString(&newFilePath, dosFilePath, sizeof(dosFilePath));
		CreateSafeWallFilePath(srcFilePath, &newFilePath);

		//申请缓冲区
		buffer = (char *)ExAllocatePool(NonPagedPool, FILEPAGE_BUFFER_SIZE);

		status = devQueryInformationFile(srcFile,srcFileObject ,&srcIoStatus,&FileStandardInfo,
			sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation,pDeviceObject);
		FileStandardInfo.EndOfFile.QuadPart += SAFEWALL_OBJECT_SIZE;//新文件长度

		KdPrint(("新文件： %wZ\n",&newFilePath));
		InitializeObjectAttributes(&newFileAttributes, &newFilePath,
			OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

		status = devCreateFile(&newFile, &newFileObject, FILE_GENERIC_WRITE,&newFileAttributes, 
			&srcIoStatus, &FileStandardInfo.EndOfFile,FILE_ATTRIBUTE_NORMAL, NULL, FILE_CREATE, 
			FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT, 
			pDeviceObject);

		if(!NT_SUCCESS(status))
		{
			KdPrint(("创建新文件失败"));
			CallNextCloseFile(srcFile, srcFileObject);
			ExFreePool(buffer);//释放缓冲区
			goto end;
		}

		//解密跳过文件头，因为safewall结构体已经提取出来了
		srcOffset.QuadPart = SAFEWALL_OBJECT_SIZE;
		newOffset.QuadPart = 0;
		KdPrint(("跳过文件头成功,算法：%d",safewall->AlgorithmVersion));
		while(TRUE)
		{
			status = devReadFile(srcFile,srcFileObject, &srcIoStatus, buffer,
				PAGE_BUFFER_SIZE, &srcOffset, pDeviceObject);
			srcOffset.QuadPart += srcIoStatus.Information;
			if(!NT_SUCCESS(status))
			{
				if(status == STATUS_END_OF_FILE)
					status = STATUS_SUCCESS;
				break;
			}

			Decode(safewall->AlgorithmVersion, buffer,srcIoStatus.Information, &newOffset,
				&safewall->privateKey, &safewall->CompanyId);
			
			//将解密缓冲写入新文件
			status = devWriteFile(newFile, newFileObject, &newStatus, buffer, srcIoStatus.Information,
				&newOffset, pDeviceObject);

			if(!NT_SUCCESS(status))
			{
				KdPrint(("文件写入时发生错误 status = 0x%08x",status));
				CallNextCloseFile(srcFile, srcFileObject);
				CallNextCloseFile(newFile, newFileObject);
				ZwDeleteFile(&newFileAttributes);
				ExFreePool(buffer);//释放缓冲区
				status = STATUS_DATA_ERROR;
				goto end;
			}

			newOffset.QuadPart += newStatus.Information;
		}
		ExFreePool(buffer);//释放缓冲区
		
		status = SafeWallReplaceFile(srcFile,srcFileObject, &srcFileAttributes, 
			newFile, newFileObject,&newFileAttributes, pDeviceObject);
	}
	end:
	ExFreePool(safewall);
	return status;

}

//新文件替换旧文件，并将旧文件删除
#pragma PAGEDCODE
NTSTATUS SafeWallReplaceFile(HANDLE srcFile, PFILE_OBJECT srcFileObject, POBJECT_ATTRIBUTES srcFileAttributes,
							 HANDLE newFile, PFILE_OBJECT newFileObject ,POBJECT_ATTRIBUTES newFileAttributes, PDEVICE_OBJECT pDeviceObject)
{
	NTSTATUS status;
	UNICODE_STRING FileName = {0};//文件名
	IO_STATUS_BLOCK ioStatus={0};
	MY_FILE_RENAME_INFORMATION FileRenameInfo ={0}; //改名结构体
	FILE_BASIC_INFORMATION FileBasicInfo = {0};//文件信息

	/*status = devQueryInformationFile(srcFile,&ioStatus,&FileBasicInfo,
			sizeof(FILE_BASIC_INFORMATION), FileBasicInformation,pDeviceObject);
	if(!NT_SUCCESS(status))
	{
		ZwClose(srcFile); ZwClose(newFile); ZwDeleteFile(newFileAttributes); return status;
	}

	status = devSetInformationFile(newFile,  &ioStatus,&FileBasicInfo,
			sizeof(FILE_BASIC_INFORMATION), FileBasicInformation,pDeviceObject);
	if(!NT_SUCCESS(status))
	{
		ZwClose(srcFile); ZwClose(newFile); ZwDeleteFile(newFileAttributes); return status;
	}
	*/
	RtlInitEmptyUnicodeString(&FileName, FileRenameInfo.FileName, sizeof(FileRenameInfo.FileName));
	GetFileNameForPath(srcFileAttributes->ObjectName, &FileName);
	FileRenameInfo.FileNameLength = FileName.Length;
	FileRenameInfo.ReplaceIfExists = 0 ;
	FileRenameInfo.RootDirectory = NULL;

	CallNextCloseFile(srcFile,srcFileObject);
	status = ZwDeleteFile(srcFileAttributes);//删除源文件
	if(NT_SUCCESS(status))
	{
		KdPrint(("文件删除成功，替换新文件"));

		status = devSetInformationFile(newFile,newFileObject, &ioStatus,&FileRenameInfo,
			sizeof(MY_FILE_RENAME_INFORMATION), FileRenameInformation,pDeviceObject);
		
		CallNextCloseFile(newFile, newFileObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("重命名或设置文件信息失败：status = 0x%08x", status));	
		}
	}
	else
	{
		KdPrint(("源文件删除失败，取消操作。status = 0x%08x",status));
		CallNextCloseFile(newFile,newFileObject);
		ZwDeleteFile(newFileAttributes);
	}

	return status;

}

//从文件全路径取得文件夹路径
#pragma PAGEDCODE
int GetFileDirectoryForPath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING FileDirectoryForPath)
{
	int offset;
	int count;
	int i, j;
	WCHAR wc;
	offset = srcFilePath->Length / sizeof(WCHAR) - 1;
	while(offset > 0 && L'\\' != srcFilePath->Buffer[--offset]);//offset定位在 L'\\' 后一位

	if(offset * sizeof(WCHAR) > FileDirectoryForPath->MaximumLength) return offset * sizeof(WCHAR);

	for(count = 0; offset > count; count++)
	{
		FileDirectoryForPath->Buffer[count] = srcFilePath->Buffer[count];
	}
	FileDirectoryForPath->Buffer[count] = L'\\';
	FileDirectoryForPath->Length = (count + 1) * sizeof(WCHAR);
	return FileDirectoryForPath->Length;
}

//从路径中提取文件名
#pragma PAGEDCODE
int GetFileNameForPath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING FileName)
{
	int offset;
	int count;
	int i, j;
	WCHAR wc;
	offset = srcFilePath->Length / sizeof(WCHAR) - 1;

	//取得颠倒文件名
	for(count = 0; offset >= count  && L'\\' != srcFilePath->Buffer[offset -count]; count ++)
	{
		FileName->Buffer[count] = srcFilePath->Buffer[offset -count];
	}

	if(count * sizeof(WCHAR) > FileName->MaximumLength) return count * sizeof(WCHAR) ;

	FileName->Length = count * sizeof(WCHAR);

	//将文件名字符还原
	for(i=0, j=count-1; i < j; i++,j--)
	{
		wc = FileName->Buffer[i];
		FileName->Buffer[i] = FileName->Buffer[j];
		FileName->Buffer[j] = wc;
	}

	//KdPrint(("文件名 : %wZ", FileName));
	return FileName->Length ;
}

//创建临时文件名
#pragma PAGEDCODE
int CreateSafeWallFilePath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING newFilePath)
{
	int offset;
	BOOLEAN begin;
	UUID uuid;
	WCHAR name[32];
	UNICODE_STRING FileName;

	if(newFilePath->MaximumLength < MY_MAX_PATH || 
		newFilePath->MaximumLength < srcFilePath->MaximumLength)
	{
		return MY_MAX_PATH > srcFilePath->MaximumLength ? MY_MAX_PATH : srcFilePath->MaximumLength + sizeof(name);
	}
	ExUuidCreate(&uuid);

	swprintf(name, L"\\%08x.wall", uuid.Data1);

	offset = srcFilePath->Length / sizeof(WCHAR); //安全性
	RtlCopyUnicodeString(newFilePath, srcFilePath);
	while(offset > 0 && L'\\' != srcFilePath->Buffer[--offset]);//offset定位在 L'\\' 后一位
	newFilePath->Length = offset * sizeof(WCHAR);
	newFilePath->Buffer[offset] = L'\0';

	RtlAppendUnicodeToString(newFilePath, name);
	return newFilePath->Length;
}

//将safewall对象加密后写到文件头
#pragma PAGEDCODE
NTSTATUS SetSafeWallToFileHead(IN LPSAFEWALL_OBJECT safewall,IN HANDLE FileHandle,
							   IN PFILE_OBJECT FileObject ,IN PDEVICE_OBJECT DeviceObject)
{
	NTSTATUS status;
	LPSAFEWALL_OBJECT enswo;
	IO_STATUS_BLOCK ioStatus={0};
	LARGE_INTEGER offset = {0};

	enswo = (LPSAFEWALL_OBJECT)ExAllocatePool( NonPagedPool, SAFEWALL_OBJECT_SIZE);
	*enswo = *safewall;
	if(enswo == NULL)
	{
		return -1;
	}
	Enhead(enswo);
	status = devWriteFile(FileHandle ,FileObject, &ioStatus, enswo, SAFEWALL_OBJECT_SIZE,  &offset, DeviceObject);
	ExFreePool(enswo);
	return status;
}

//检查safewall对象中的加密标志
#pragma PAGEDCODE
DWORD CheckSafeWallFlags(LPSAFEWALL_OBJECT SafeWall)
{
	DWORD dwRet = 0;
	SafeWall->swv.myCompanyName[sizeof(SafeWall->swv.myCompanyName)/sizeof(WCHAR) -1] = L'\0';//安全性
	if(!InlineIsEqualSWID(&SafeWall->swv.myId, (SWID*)&MYSAFEWALLGUID) ||
		wcscmp(SafeWall->swv.myCompanyName, MYSAFEWALLCOMPANYNAME)) 
	{
		//这不是我们的加密文件
		//KdPrint(("这不是数据围墙的加密文件\n"));
		return dwRet;
	}
	//到这里，就是属于我们的加密对象了，添加标志
	dwRet = SAFEWALL_FLAG_OBJECT;

	if(InlineIsEqualSWID(&SafeWall->FileGroupId, &gStandardSafeWallObj->CompanyId)) 
	{
		//KdPrint(("这是属于公司的GUID\n"));
		dwRet |= (SAFEWALL_FLAG_MANAGEMENT | SAFEWALL_FLAG_FILEGROUP);
	}
	else if(InlineIsEqualSWID(&SafeWall->FileGroupId, &gStandardSafeWallObj->FileGroupId)) 
	{
		//KdPrint(("这是属于文件组的GUID\n"));
		dwRet |= SAFEWALL_FLAG_FILEGROUP;
	}
	return dwRet;
}

//从文件名中取得safewall对象
#pragma PAGEDCODE
BOOLEAN GetSafeWallByFilePath(IN PUNICODE_STRING FilePath, IN PDEVICE_OBJECT DeviceObject,OUT LPSAFEWALL_OBJECT SafeWall)
{
	IO_STATUS_BLOCK iostatus ={0};
	HANDLE hFile;
	PFILE_OBJECT FileObject;
	LARGE_INTEGER offset = {0};
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT  pDeviceObject = DeviceObject;
	OBJECT_ATTRIBUTES FileAttributes;
	PAGED_CODE();
	//文件必须是全路径
	if(NULL == pDeviceObject || pDeviceObject == gMyControlDeviceObject)
	{
		status = GetMyDeviceObjectByFilePath(FilePath, &pDeviceObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("获取文件对应的磁盘设备失败"));
			return FALSE; 
		}
		pDeviceObject = ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject;
	}

	InitializeObjectAttributes(&FileAttributes, FilePath,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

	status = devCreateFile(&hFile,&FileObject,  FILE_GENERIC_READ,
		&FileAttributes,&iostatus,NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING|FILE_SYNCHRONOUS_IO_NONALERT,
		pDeviceObject);
	
	if(!NT_SUCCESS(status))
	{
		return FALSE;
	}
	
	status = devReadFile(hFile, FileObject, &iostatus, SafeWall, SAFEWALL_OBJECT_SIZE,
		&offset, pDeviceObject);
	CallNextCloseFile(hFile, FileObject);
	
	if(!NT_SUCCESS(status) || SAFEWALL_OBJECT_SIZE > iostatus.Information)
	{
		return FALSE;
	}
	Dehead(SafeWall);
	KdPrint(("privateKey=%08x-%08x-%08x-%08x",SafeWall->privateKey.data[0],
		SafeWall->privateKey.data[1],SafeWall->privateKey.data[2],SafeWall->privateKey.data[3]));
	//检查safewall的属性

	
	return TRUE;
}

//从句柄中取得safewall对象
#pragma PAGEDCODE
BOOLEAN GetSafeWallByFileHandle(IN HANDLE FileHandle, IN PFILE_OBJECT FileObject, IN PDEVICE_OBJECT DeviceObject, OUT LPSAFEWALL_OBJECT SafeWall)
{
	IO_STATUS_BLOCK iostatus;
	LARGE_INTEGER offset = {0};
	LARGE_INTEGER CurrentByteOffset = {0};
	NTSTATUS status = STATUS_SUCCESS;


	status = devReadFile(FileHandle, FileObject,&iostatus, SafeWall, SAFEWALL_OBJECT_SIZE,
		&offset, DeviceObject);
	//失败或者文件比LPSAFEWALL_OBJECT结构体要小，肯定还没加密
	if(!NT_SUCCESS(status) || SAFEWALL_OBJECT_SIZE > iostatus.Information)
	{
		return FALSE;
	}

	//解密
	Dehead(SafeWall);
	KdPrint(("privateKey=%08x-%08x-%08x-%08x",SafeWall->privateKey.data[0],
		SafeWall->privateKey.data[1],SafeWall->privateKey.data[2],SafeWall->privateKey.data[3]));
	//检查safewall的属性
	
	return TRUE;
}


//从路径获取设备指针
#pragma PAGEDCODE
NTSTATUS GetMyDeviceObjectByFilePath(IN PUNICODE_STRING FilePath, OUT PDEVICE_OBJECT *DeviceObject)
{
	NTSTATUS status;
	ULONG numDevices;
    PDEVICE_OBJECT *devList;
	UNICODE_STRING tmpFilePath;
	PUNICODE_STRING pDeviceName = NULL;
	ULONG i=0;

	//IoEnumerateDeviceObjectList能增加对象引用保证设备指针不失效
	status = IoEnumerateDeviceObjectList( gMyDriverObject, NULL, 0, &numDevices);

	ASSERT(STATUS_BUFFER_TOO_SMALL == status);
	devList = (PDEVICE_OBJECT *)ExAllocatePool( NonPagedPool, (numDevices * sizeof(PDEVICE_OBJECT)) );
	if (NULL == devList)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = IoEnumerateDeviceObjectList(gMyDriverObject,
                        devList,(numDevices * sizeof(PDEVICE_OBJECT)), &numDevices);
    
    if (!NT_SUCCESS( status ))
    {
        ExFreePool( devList );
        return status;
    }

	status = -1;
	tmpFilePath.Buffer = FilePath->Buffer;
	tmpFilePath.Length = FilePath->Length;
	tmpFilePath.MaximumLength = FilePath->MaximumLength;
	for ( i=0; numDevices > i ; i++)
    {
		if (IS_MY_DEVICE_OBJECT(devList[i]))
        {
			pDeviceName = &((PDEVICE_EXTENSION)devList[i]->DeviceExtension)->DeviceName;
			if(pDeviceName->Length > FilePath->Length || 0 == pDeviceName->Length) continue;
			//KdPrint(("devNameString = %wZ", pDeviceName));
			tmpFilePath.Length = pDeviceName->Length;
			if(!RtlCompareUnicodeString(&tmpFilePath, pDeviceName, TRUE))
			{
				*DeviceObject = devList[i];
				status = 0;
				break;
			}
        }
    }

	for ( i=0; numDevices > i ; i++)
	{
		ObDereferenceObject( devList[i] );
	}
	ExFreePool(devList);
	//ObDereferenceObject( FileObject ); //这里减少对文件对象的引用
	return status;
}
