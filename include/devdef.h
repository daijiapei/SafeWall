
#ifndef __DEVDEF_H_
#define __DEVDEF_H_

#define MYSAFEWALLCOMPANYNAME L"数据围墙-中国COPY软件有限公司\0"

//============================== 常用宏定义 ============================
#define MY_MAX_PATH   360

#ifndef MAX_PATH
#define MAX_PATH  260
#endif

#ifndef WORD
typedef unsigned short       WORD;
#endif

#ifndef DWORD
typedef unsigned long       DWORD;
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b) ((short)(((char)(((DWORD_PTR)(a)) & 0xff)) | ((short)((char)(((DWORD_PTR)(b)) & 0xff))) << 8))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((short)(a)) | ((DWORD)((short)(b))) << 16))
#endif

#ifndef Add2Ptr
//语法: PVOID  Add2Ptr(PUCHAR p,PUCHAR i);  含义: 指针偏移
#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))  
#endif

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))
//============================== 常用宏定义 ============================

//============================== 驱动控制码 ============================
#define SAFEWALL_DEVICE_DOSNAME  L"\\\\.\\safewall"
#define SAFEWALL_DEVICE_SYMNAME  L"\\DosDevices\\SafeWall"

#define SAFEWALL_START    0x801
#define SAFEWALL_STOP     0x802
#define SAFEWALL_ENCRYPT_FILE  0x803
#define SAFEWALL_DECRYPT_FILE  0x804
#define SAFEWALL_QUERY_FILEATTRIBUTES     0x805
#define SAFEWALL_SHOWICO 0x806
#define SAFEWALL_FILEHIDE 0x807
#define SAFEWALL_SET_USER_ACCESS  0x821
#define SAFEWALL_SET_PROCESS_ACCESS 0x822
#define SAFEWALL_SET_FILE_ACCESS  0x823

#define SAFEWALL_TEST     0x899

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif

#define IOCTL_START CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_START, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_STOP CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_STOP, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ENCRYPT CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_ENCRYPT_FILE,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DECRYPT CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_DECRYPT_FILE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_QUERY_FILEATTRIBUTES CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_QUERY_FILEATTRIBUTES, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SHOWICO CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_SHOWICO, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FILEHIDE CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_FILEHIDE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_USER_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN,SAFEWALL_SET_USER_ACCESS, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_PROCESS_ACCESS  CTL_CODE(FILE_DEVICE_UNKNOWN,SAFEWALL_SET_PROCESS_ACCESS, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_FILE_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN,SAFEWALL_SET_FILE_ACCESS, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_TEST, METHOD_BUFFERED, FILE_ANY_ACCESS)


//============================== 驱动控制码 ============================



//============================== 加密驱动标签 ============================
//safewall对象版本
#define SAFEWALl_DWORD_VERSION_1     1
#define SAFEWALl_DWORD_VERSION_2     2
#define SAFEWALl_DWORD_VERSION_3     3

//算法版本
#define SAFEWALl_ALGORIT_VERSION_1     0x00000001
#define SAFEWALl_ALGORIT_VERSION_2     0x00000002
#define SAFEWALl_ALGORIT_VERSION_3     0x00000003

//测试SALEWALL_OBJECT 对象属性
//磁盘文件状态
#define SAFEWALL_FLAG_OBJECT      0x00000001   //这是我们的加密对象
#define SAFEWALL_FLAG_FILEGROUP   0x00000002   //这是我们的文件组
#define SAFEWALL_FLAG_MANAGEMENT  0x00000004   //管理ID标志，最高权限的文件组

//缓冲文件状态
#define SAFEWALL_FLAG_INFECTED    0x80000000   //已加密
#define SAFEWALL_FLAG_WAITENCRYPT     0x40000000   //等待被加密

#define MAX_DEVNAME_LENGTH 64                                //定义常量值
#define DEVOBJ_LIST_SIZE 64

#define DELAY_ONE_MICROSECOND   (-10)  
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)


#define FILE_HIDE      0x00000001  //隐藏加密文件
#define FILE_SHOWICO   0x00000002  //显示加密图标

//============================== 加密驱动标签 ============================


//============================== 加密策略 ============================
//全局进程权限结构体
typedef struct _PROCESS_ACCESS_STRUCT{
	char AccessName[32]; //权限名称
	long AccessFlags; //权限标志
	struct _PROCESS_ACCESS_STRUCT * next;//下一个结构体的位置
	int lenght;  //ProcessName的长度
	char ProcessName[1]; //进程名称，以'\0'分隔
}PROCESS_ACCESS, * PPROCESS_ACCESS;

//全局文件权限
typedef struct _FILE_ACCESS_STRUCT{
	int size;//suffix的字符数
	wchar_t suffix[1];
}FILE_ACCESS, *PFILE_ACCESS;

//用户私有权限
typedef struct _USER_ACCESS_STRUCT{
	char userid[64];
	long AccessFlags;
}USER_ACCESS, *PUSER_ACCESS;

//通用标志
#define SAFEWALL_ENABLE   0x00000001 //启用
#define SAFEWALL_DISABLE  0x00000002 //禁用
#define SAFEWALL_NOLYREAD 0x00000004 //只读

//进程属性
#define PROCESS_NO_ACCESS    0x00000001 //禁止访问加密文件
#define PROCESS_ACCESS_NORMAL    0x00000002 //默认隔离。进程未加密就不能访问加密文件
#define PROCESS_ACCESS_INHERIT   0x00000004 //所有子进程继承该进程的属性
#define PROCESS_ACCESS_CLIPBOARD 0x00000008 //访问粘贴板不加密

#define USER_ACCESS_DISABLE_PRINT 0x00000001 //禁止打印
#define USER_ACCESS_DISABLE_SCREENSHOT 0x00000002 //禁止截屏
#define USER_ACCESS_RUN_NETWORK_PROCESS  0x00000004 //允许运行授权的网络程序

//============================== 加密策略 ============================

#endif 