// FileIco.h : CFileIco 的声明

#pragma once
#include "resource.h"       // 主符号
#include "swfIcoExt_i.h"
#include "shlobj.h"
#include "..\\include\\devdef.h"
#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif

using namespace ATL;


// CFileIco

class ATL_NO_VTABLE CFileIco :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFileIco, &CLSID_FileIco>,
	public IDispatchImpl<IFileIco, &IID_IFileIco, &LIBID_swfIcoExtLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellIconOverlayIdentifier
{
public:
	HANDLE hDevice;
	CFileIco()
	{
		hDevice = CreateFile(SAFEWALL_DEVICE_DOSNAME,
			GENERIC_READ|GENERIC_WRITE , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	~CFileIco()
	{
		CloseHandle(hDevice);
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FILEICO)


BEGIN_COM_MAP(CFileIco)
	COM_INTERFACE_ENTRY(IFileIco)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellIconOverlayIdentifier)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	//初始化
    HRESULT STDMETHODCALLTYPE Initialize( 
        /* [in] */ LPCITEMIDLIST pidlFolder,
        /* [in] */ IDataObject *pdtobj,
        /* [in] */ HKEY hkeyProgID);

	HRESULT STDMETHODCALLTYPE GetOverlayInfo(
		  OUT PWSTR pwszIconFile,
				int   cchMax,
		  OUT int   *pIndex,
		  OUT DWORD *pdwFlags);

	HRESULT STDMETHODCALLTYPE GetPriority(
		  OUT int *pPriority);

	HRESULT STDMETHODCALLTYPE IsMemberOf(
		  IN PCWSTR pwszPath,
			   DWORD  dwAttrib);

};

OBJECT_ENTRY_AUTO(__uuidof(FileIco), CFileIco)
