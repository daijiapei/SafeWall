// SafeWallExt.h : CSafeWallExt 的声明

#pragma once
#include "resource.h"       // 主符号



#include "SwShellExt_i.h"
#include "shlobj.h"
//此文件在添加->类->简单的ATL对象中新增

//1.  HKEY_CLASSES_ROOT\Folder\Shellex\ContextMenuHandlers 项中加入新的项。
//	并在默认的键值中加入拓展dll的GUID .
//这样改DLL文件正式成为shell组件, 并且是针对folder（文件夹）生效
//如果是添加到*中，针对所有文件生效
//
//2.  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved 
//项中加入以GUID名字的键，使非管理员用户可以调用该接口

#define IOMSG_ENCRYPT_FILE_MENU 0x11
#define IOMSG_DECRYPT_FILE_MENU 0X12

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif

using namespace ATL;


// CSafeWallExt

class ATL_NO_VTABLE CSafeWallExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSafeWallExt, &CLSID_SafeWallExt>,
	public IDispatchImpl<ISafeWallExt, &IID_ISafeWallExt, &LIBID_SwShellExtLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit, //shell扩展
    public IContextMenu    //上下文菜单
{
public:
	CSafeWallExt()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SAFEWALLEXT)


BEGIN_COM_MAP(CSafeWallExt)
	COM_INTERFACE_ENTRY(ISafeWallExt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)//Sell对象
	COM_INTERFACE_ENTRY(IContextMenu)//菜单对象
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

//下面为自定义添加的内容
public:
	//初始化
    HRESULT STDMETHODCALLTYPE Initialize( 
        /* [in] */ LPCITEMIDLIST pidlFolder,
        /* [in] */ IDataObject *pdtobj,
        /* [in] */ HKEY hkeyProgID);
	
	//想菜单中添加内容，详见msdn。
	//这个函数是最为关键的函数之一。（其实关键函数有2个，下面那个就是另一个之一）
	//要增加几个菜单，在什么位置，叫什么名字，带不带图标，点击后命令id等等都是这里完成的。
    STDMETHOD(QueryContextMenu)(THIS_
        HMENU hmenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags);

	//关键函数的另一个之一，定义菜单的消息响应。
	//请msdn下查下CMINVOKECOMMANDINFOEX里的 lpVerb参数的意思，
	//你会发现他的高位标示了是字符串还是命令id偏移量。
	//我们对字符串不关心，我们关心的是命令id. 这个函数是要实现具体点击菜单后所要实现的功能。
	//所以判断是点了 XXXX(102),还是YYYY(103)菜单呢。
    STDMETHOD(InvokeCommand)(THIS_
        LPCMINVOKECOMMANDINFO lpici);


	//获得右键菜单命令信息，包括帮助信息
    STDMETHOD(GetCommandString)(THIS_
        UINT_PTR    idCmd,
        UINT        uType,
        UINT      * pwReserved,
        LPSTR       pszName,
        UINT        cchMax);

private:

    WCHAR   m_pszFileName[MAX_PATH];
    HBITMAP m_hBitmap;
    CHAR    m_pszVerb[32];
    WCHAR   m_pwszVerb[32];

};

OBJECT_ENTRY_AUTO(__uuidof(SafeWallExt), CSafeWallExt)
