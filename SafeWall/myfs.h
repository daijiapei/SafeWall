

#ifndef MY_FILE_SYSTEM_H
#define MY_FILE_SYSTEM_H

#include "ntifs.h"
#include "ntdddisk.h"



#ifdef __cplusplus
extern "C" {
#endif


int GetVolumeFileNameByFullFilePath(IN PUNICODE_STRING FilePath,OUT PUNICODE_STRING VolumeFileName);

NTSTATUS devCreateFile(
	_Out_ PHANDLE FileHandle,
	_Out_ PFILE_OBJECT *FileObject,
    _In_ ACCESS_MASK DesiredAccess,
	_In_ OBJECT_ATTRIBUTES *ObjectAttributes,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_opt_ PLARGE_INTEGER AllocationSize,
    _In_ ULONG FileAttributes,
    _In_ ULONG ShareAccess,
    _In_ ULONG CreateDisposition,
    _In_ ULONG CreateOptions,
	_In_ PDEVICE_OBJECT pDeviceObject);

NTSTATUS devReadFile(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset,
	_In_ PDEVICE_OBJECT pDeviceObject
    );

NTSTATUS devWriteFile(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_reads_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length,
	_In_opt_ PLARGE_INTEGER ByteOffset,
	_In_ PDEVICE_OBJECT pDeviceObject
    );

NTSTATUS devCloseFile(_In_ HANDLE FileHandle, _In_ PDEVICE_OBJECT pDeviceObject);

VOID CallNextCloseFile(_In_ HANDLE FileHandle, _In_ PFILE_OBJECT FileObject);

NTSTATUS devQueryInformationFile(
	_In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_Out_writes_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass,
	_In_ PDEVICE_OBJECT pDeviceObject);

NTSTATUS devSetInformationFile(
    _In_ HANDLE FileHandle,
	_In_ PFILE_OBJECT FileObject,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass,
    _In_ PDEVICE_OBJECT pDeviceObject);



#ifdef __cplusplus
}
#endif

#endif