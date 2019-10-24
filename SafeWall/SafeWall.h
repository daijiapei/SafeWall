

#ifndef SAFEWALL_HEAD
#define SAFEWALL_HEAD

#define INITGUID

#ifndef MY_UNCODE
#undef MY_UNCODE 
#endif

#include <ntifs.h>
#include <ntdddisk.h>

#include "..\\include\\devdef.h"

#include "fat.h"
#include "nodetype.h"
#include "FatStruc.h"

typedef struct _MY_LIST_HEADER{
	LIST_ENTRY header;
	KSPIN_LOCK locker;
	ULONG icount;
} MY_LIST_HERDER, *PMY_LIST_HEADER;

typedef struct _SAFEWALL_FILE_LIST{
	LIST_ENTRY list;
	PFSRTL_COMMON_FCB_HEADER pFcb;
	DWORD flags;
	UNICODE_STRING FilePath;
	WCHAR wstr[MY_MAX_PATH];
	GUID  secretKey;
}SAFEWALL_FILE_LIST, *PSAFEWALL_FILE_LIST;

typedef struct _SAFEWALL_PROCESS_LIST{
	LIST_ENTRY list;
	PEPROCESS  proc;
	DWORD      flags;
}SAFEWALL_PROCESS_LIST, *PSAFEWALL_PROCESS_LIST;


typedef struct _ACCESS_PACKAGE{
	KSPIN_LOCK locker;
	PVOID  package;
}ACCESS_PACKAGE, * PACCESS_PACKAGE;

EXTERN_C BOOLEAN g_safewall_start ;             //用户已登录
EXTERN_C ULONG g_process_name_offset;             //进程名在PROCESS结构中的偏移量
EXTERN_C ULONG gOsMajorVersion;                   //操作系统主版本号
EXTERN_C ULONG gOsMinorVersion;                   //操作系统次版本号
EXTERN_C PDRIVER_OBJECT gMyDriverObject;          //保存由/O管理器生成并传入的驱动对象
EXTERN_C PDEVICE_OBJECT gMyControlDeviceObject;   //保存由本过滤驱动生成的控制设备对象
EXTERN_C FAST_MUTEX gFastMutexAttachLock;         //定义一个快速互斥结构变量(对象)，挂载卷时用到
EXTERN_C DWORD FileFlags;
EXTERN_C PFILE_OBJECT pTestID;

typedef struct _DEVICE_EXTENSION{ 
	PDEVICE_OBJECT AttachedToDeviceObject;             //绑定的文件系统设备
        PDEVICE_OBJECT StorageStackDeviceObject;       //与文件系统设备相关的真实设备(磁盘) 
        UNICODE_STRING DeviceName;                     //如果绑定了一个卷,这是物理磁盘卷名；否则,为绑定的控制设备名。
        WCHAR DeviceNameBuffer[MY_MAX_PATH];
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _GET_NAME_CONTROL {
        PCHAR allocatedBuffer;                                                                                                                
        CHAR smallBuffer[MY_MAX_PATH];                                                                                                                
} GET_NAME_CONTROL, *PGET_NAME_CONTROL;

#define PAGE_BUFFER_SIZE 4096  //默认缓冲4KB比较合理
#define FILEPAGE_BUFFER_SIZE PAGE_BUFFER_SIZE
//#define FILEPAGE_BUFFER_SIZE (1024*256)
//加密对象版本 68字节
//将加密版本放在前面，先匹配加密对象版本，
//再代入对应的加密对象
typedef union _SAFEWALL_CUID_{
	__int64  QuadPart[2];
	unsigned long data[4];
	char byte[16];
	GUID guid;
	LARGE_INTEGER LargeInt[2];
}SWID ,FAR * PSWID;
#define KEYBYTESOFFSET  128 //key的位可偏移量是128位

#define SAFEWALL_VERSION_SIZE 128 //定性为128
#define SAFEWALL_COMPANYNAME_SIZE ((SAFEWALL_VERSION_SIZE-sizeof(DWORD)-sizeof(SWID)-sizeof(INT))/sizeof(WCHAR))
typedef struct _SAFEWALL_VERSION{
	DWORD vObj;   //对象版本  4位
	WCHAR myCompanyName[SAFEWALL_COMPANYNAME_SIZE];  //SAFEWAL所属公司名称
	SWID myId;    //SAFEWALL的GUID;  16位
	INT objSize;  //版本对象大小，如果为0表示未设置  4位
}SAFEWALL_VERSION,FAR *LPSAFEWALL_VERSION;

// vObj == 1.0
//{02EC6753-6EBE-415C-A07D-CD71B584A361}  1.0  GUID码


#define SAFEWALL_OBJECT_SIZE  1024
//加密对象信息,结构体固定为1024字节
typedef struct {
	SAFEWALL_VERSION swv; //加密对象版本     
	DWORD  AlgorithmVersion; //加密算法版本 
	WCHAR  CompanyName[64]; //加密公司名称  
	SWID   CompanyId; //加密公司GUID    
	WCHAR  FileGroupName[64]; //加密组名称      -
	SWID   FileGroupId;  //加密组ID 
	SWID   privateKey;   //动态私密匙
	LARGE_INTEGER EncryptTime;  //加密事件
	WCHAR   UserId[64];   //用户ID
	DWORD  OriginType;   //加密原因
	char   cbExt[1];      //填补长度空缺
#define  OT_INFECTED      1  //感染加密
#define  OT_MANUAL        2  //手动加密
#define  ManagementName   CompanyName
#define  ManagementId     CompanyId
} FAR *LPSAFEWALL_OBJECT;
//struct _SAFEWALL_OBJECT 对象不能够用栈

extern LPSAFEWALL_OBJECT gStandardSafeWallObj;         //定义一个快速互斥结构变量(对象)


// 宏定义: 测试是不是我的"控制设备对象"
#define IS_MY_CONTROL_DEVICE_OBJECT(_devObj)  (((_devObj) == gMyControlDeviceObject) ? (ASSERT(((_devObj)->DriverObject == gMyDriverObject) && ((_devObj)->DeviceExtension == NULL)), TRUE) : FALSE)

// 宏定义: 测试是不是用来绑定文件系统的设备
#define IS_MY_DEVICE_OBJECT(_devObj)  (((_devObj) != NULL) && ((_devObj)->DriverObject == gMyDriverObject) && ((_devObj)->DeviceExtension != NULL))

// 宏定义: 测试FAST_IO_DISPATCH中的处理函数合法
#define VALID_FAST_IO_DISPATCH_HANDLER(_FastIoDispatchPtr, _FieldName)  (((_FastIoDispatchPtr) != NULL) && (((_FastIoDispatchPtr)->SizeOfFastIoDispatch) >= (FIELD_OFFSET(FAST_IO_DISPATCH, _FieldName) + sizeof(void *))) && ((_FastIoDispatchPtr)->_FieldName != NULL))

// 宏定义: 测试是不是所需要的"设备类型"
#define IS_DESIRED_DEVICE_TYPE(_type)  (((_type) == FILE_DEVICE_DISK_FILE_SYSTEM) || ((_type) == FILE_DEVICE_CD_ROM_FILE_SYSTEM) || ((_type) == FILE_DEVICE_NETWORK_FILE_SYSTEM))

// 宏定义: 
//#define SF_LOG_PRINT( _dbgLevel, _string )  (FlagOn(SfDebug,(_dbgLevel)) ? DbgPrint _string : ((void)0))

//只要在结构体中包含了这个对象，表示属于我们的加密软件
//SAFEWALL对象的GUID

DEFINE_GUID(MYSAFEWALLGUID, 0xD896FA24,0x3F2E,0x415A,0xB2,0x17,0xB9,0xDF,0x9E,0xE4,0x9D,0x4B);

//SAFEWALL对象默认的加密KEY
DEFINE_GUID(SAFEWALLAUTOKEY, 0xea57bd55, 0xc321, 0x4926, 0xb3, 0x1c, 0xbd, 0x19, 0x9b, 0xc7, 0x76, 0x72);

//客户公司的GUID 用来测试
DEFINE_GUID(COMPANYGUID, 0x5F6296EF,0xFE10,0x43C5,0x97,0xA4,0x62,0xCD,0xD4,0x8C,0xDD,0x07);

// {271445DB-596D-47B9-BCAD-241F6A7F6155}
// 过滤设备ID
DEFINE_GUID(MYDEVICE, 0x271445db, 0x596d, 0x47b9, 0xbc, 0xad, 0x24, 0x1f, 0x6a, 0x7f, 0x61, 0x55);


#ifdef __cplusplus
extern "C" {
#endif

__inline int InlineIsEqualSWID(const SWID * swid1, const SWID * swid2);
BOOLEAN InitMySafeWallObject(LPSAFEWALL_OBJECT * lpSafeWall);
DWORD InitSafeWallObject(LPSAFEWALL_OBJECT lpSafeWall);
void FileSystemCacheClear(PFILE_OBJECT pFileObject);
//取得信息
ULONG GetCurrentProcessName(PUNICODE_STRING name);
ULONG GetFileObjectFullPath( PFILE_OBJECT file, PUNICODE_STRING path );

//清除挂载设备
VOID  CleanupMountedDevice( IN PDEVICE_OBJECT DeviceObject );

//分发函数
VOID  DriverUnload( IN PDRIVER_OBJECT pDeviceObject );
NTSTATUS AddDevice(IN PDRIVER_OBJECT pDriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp);
NTSTATUS FileSystemDeviceControl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );
NTSTATUS DriverCloseDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp );
NTSTATUS DriverCreateDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp );
NTSTATUS DriverReadDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp);
NTSTATUS DriverWriteDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp);
NTSTATUS DriverControlDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp);
NTSTATUS DriverCleanUpDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );
NTSTATUS DriverQueryInformationDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp);
NTSTATUS DriverSetInformationDispatchRoutin(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp);
NTSTATUS DriverDirectoryControlDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );
//文件系统变动通知
VOID  FileSystemChangeNotification( IN PDEVICE_OBJECT DeviceObject, IN BOOLEAN FsActive );
//========================== FSFilter 回调函数 ===========================
NTSTATUS  PreFsFilterPassThrough( IN PFS_FILTER_CALLBACK_DATA Data, OUT PVOID *CompletionContext );
VOID  PostFsFilterPassThrough ( IN PFS_FILTER_CALLBACK_DATA Data, IN NTSTATUS OperationStatus, IN PVOID CompletionContext );

//========================== 设置FastIo分派函数,共21个===========================
BOOLEAN  FastIoQueryOpen( IN PIRP Irp, OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoMdlWriteCompleteCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoMdlReadCompleteCompressed( IN PFILE_OBJECT FileObject, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoWriteCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, IN PVOID Buffer, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo, IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoReadCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PVOID Buffer, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo, IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoMdlWriteComplete( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoPrepareMdlWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoMdlReadComplete( IN PFILE_OBJECT FileObject, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoMdlRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoQueryNetworkOpenInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_NETWORK_OPEN_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
VOID  FastIoDetachDevice( IN PDEVICE_OBJECT SourceDevice, IN PDEVICE_OBJECT TargetDevice );
BOOLEAN  FastIoDeviceControl( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength, OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength, IN ULONG IoControlCode, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoUnlockAllByKey( IN PFILE_OBJECT FileObject, PVOID ProcessId, ULONG Key, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoUnlockAll( IN PFILE_OBJECT FileObject, PEPROCESS ProcessId, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoUnlockSingle( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoLock( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key, BOOLEAN FailImmediately, BOOLEAN ExclusiveLock, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoQueryStandardInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_STANDARD_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoQueryBasicInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_BASIC_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN PVOID Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  FastIoCheckIfPossible( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN BOOLEAN CheckForReadOperation, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject);
BOOLEAN  FastIoRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, OUT PVOID Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );

NTSTATUS  AttachToFileSystemDevice( IN PDEVICE_OBJECT DeviceObject, IN PUNICODE_STRING DeviceName );
VOID  DetachFromFileSystemDevice( IN PDEVICE_OBJECT DeviceObject );

VOID  GetObjectName( IN PVOID Object, IN OUT PUNICODE_STRING Name );
VOID  GetBaseDeviceObjectName( IN PDEVICE_OBJECT DeviceObject, IN OUT PUNICODE_STRING Name );
PUNICODE_STRING  GetFileName( IN PFILE_OBJECT FileObject, IN NTSTATUS CreateStatus, IN OUT PGET_NAME_CONTROL NameControl );
VOID GetFileNameCleanup( IN OUT PGET_NAME_CONTROL NameControl );

NTSTATUS  IsShadowCopyVolume( IN PDEVICE_OBJECT StorageStackDeviceObject, OUT PBOOLEAN IsShadowCopy );
BOOLEAN  IsAttachedToDevice( PDEVICE_OBJECT DeviceObject, PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL );
NTSTATUS  EnumerateFileSystemVolumes( IN PDEVICE_OBJECT FSDeviceObject, IN PUNICODE_STRING Name ) ;
NTSTATUS  AttachToMountedDevice( IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_OBJECT SFilterDeviceObject );

//完成函数
NTSTATUS  AutoCompletionRoutine( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context );
NTSTATUS  DeviceControlLoadFileSystemComplete ( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
NTSTATUS  FileSystemControlMountVolumeComplete( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PDEVICE_OBJECT NewDeviceObject );

NTSTATUS ExGuidCreate(OUT GUID *Guid);
char * Encode(IN DWORD version,IN OUT char *buffer, IN ULONG lenght, 
			  IN LARGE_INTEGER *keyOffset, IN SWID * privateKey,IN SWID * publicKey);//加密
char * Decode(IN DWORD version,IN OUT char *buffer, IN ULONG lenght, 
			  IN LARGE_INTEGER *keyOffset, IN SWID * privateKey,IN SWID * publicKey);//解密
NTSTATUS SafeWallEncryptFile(PUNICODE_STRING srcFilePath, PDEVICE_OBJECT DeviceObject);//加密文件
NTSTATUS SafeWallDecryptFile(PUNICODE_STRING srcFilePath, PDEVICE_OBJECT DeviceObject);//解密文件
int GetFileNameForPath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING FileName);//从路径提取文件名
int GetFileDirectoryForPath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING FileDirectoryForPath);//从路径提取路径
NTSTATUS SetSafeWallToFileHead(IN LPSAFEWALL_OBJECT safewall,IN HANDLE FileHandle,IN PFILE_OBJECT FileObject,IN PDEVICE_OBJECT DeviceObject);//写入文件头
BOOLEAN GetSafeWallByFileHandle(IN HANDLE FileHandle,IN PFILE_OBJECT FileObject, IN PDEVICE_OBJECT DeviceObject, OUT LPSAFEWALL_OBJECT SafeWall);
BOOLEAN GetSafeWallByFilePath(IN PUNICODE_STRING FilePath, IN PDEVICE_OBJECT DeviceObject,OUT LPSAFEWALL_OBJECT SafeWall);
DWORD CheckSafeWallFlags(LPSAFEWALL_OBJECT SafeWall);//检查safewall加密标志
//对象表链表
VOID InitializeMyFileListHead();
VOID InitializeMyProcessListHead();
SAFEWALL_FILE_LIST * SelectFileListNode(PFSRTL_COMMON_FCB_HEADER pFcb);
SAFEWALL_FILE_LIST * InsertSingleFileListNode(PFSRTL_COMMON_FCB_HEADER pFcb, BOOLEAN * IsHas);
BOOLEAN DeleteFileListNode(PFSRTL_COMMON_FCB_HEADER pFcb);

#ifdef __cplusplus
}
#endif

//#define WINVER  0x0501


typedef struct _MY_FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[MAX_PATH];
} MY_FILE_NAME_INFORMATION, *PMY_FILE_NAME_INFORMATION;

typedef struct _MY_FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[MAX_PATH];
} MY_FILE_RENAME_INFORMATION, *PMY_FILE_RENAME_INFORMATION;

#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")





#endif