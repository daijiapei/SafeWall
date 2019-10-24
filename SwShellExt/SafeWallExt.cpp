// SafeWallExt.cpp : CSafeWallExt 的实现

#include "stdafx.h"
#include "SafeWallExt.h"
#include "..\\include\\devdef.h"
#include "..\\include\\assist.h"

// CSafeWallExt


#pragma warning(disable:4996)
// CSysMenuShellExt

BOOL SendDeviceControl(int code,char * buffer, int buflen);


HRESULT CSafeWallExt::Initialize ( 
        /* [in] */ LPCITEMIDLIST pidlFolder, //经测试发现是空的
        /* [in] */ IDataObject *pdtobj,
        /* [in] */ HKEY hkeyProgID)
    {
        HRESULT hr;
        UINT    nFileCount;
        UINT    nLen;

        FORMATETC fmt = 
        {
            CF_HDROP,
            NULL,
            DVASPECT_CONTENT,
            -1,
            TYMED_HGLOBAL
        };

        STGMEDIUM sm = 
        {
            TYMED_HGLOBAL
        };

        if (FAILED(pdtobj->GetData(&fmt, &sm)))
        {
            return E_INVALIDARG;
        }

        nFileCount = DragQueryFile((HDROP)sm.hGlobal, 0xFFFFFFFF, NULL, 0);

        if (nFileCount >= 1)
        {
            nLen = DragQueryFile((HDROP)sm.hGlobal, 0, m_pszFileName, sizeof(m_pszFileName));

            if (nLen >0 &&  MAX_PATH > nLen)
            {
                m_pszFileName[nLen] = _T('\0');
            }
            else
            {
                hr = E_INVALIDARG;
            }    
        }
        else
        {            
            hr = E_INVALIDARG;
        }

        ReleaseStgMedium(&sm);

        return hr;
    }

HRESULT CSafeWallExt::QueryContextMenu(THIS_
        HMENU hmenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags)
    {
		// 如果标志包含 CMF_DEFAULTONLY 我们不作任何事情. 
		//_T("数据围墙  ");
		if ( uFlags & CMF_VERBSONLY ) 
		{ 
			return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 ); 
		}
		//MessageBox(NULL,m_pszFileName,m_pszFileName,NULL);
		
		MENUITEMINFO mii;
		WCHAR szMenuName[MAX_PATH] = {0};
		HMENU subMenu = CreatePopupMenu();
		int index = 0;
		/*ZeroMemory(&mii, sizeof(mii));
		mii.wID = idCmdFirst+1;
		if(GetMenuItemInfo(hmenu, mii.wID, FALSE, &mii))
		{
			MessageBox(NULL, szMenuName,szMenuName, NULL);
		}
		else
		{
			MessageBox(NULL, L"失败",L"失败", NULL);
		}
		ZeroMemory(&mii, sizeof(mii));
		HMENU subMenu = CreatePopupMenu();
		mii.cbSize=sizeof(mii);
		mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
		mii.wID = idCmdFirst+1;
		mii.hSubMenu = subMenu;
		mii.dwTypeData = _T("数据围墙  ");;*/

		/*1*/
        InsertMenu(hmenu,indexMenu,MF_SEPARATOR|MF_BYPOSITION,0,NULL);//分割线
		//InsertMenuItem (hmenu, mii.wID, FALSE, &mii );
		/*2*/
		InsertMenu (hmenu, indexMenu+1, MF_BYPOSITION|MF_POPUP,(UINT_PTR) subMenu , _T("数据围墙  "));

		InsertMenu(subMenu, 0, MF_STRING | MF_BYPOSITION, idCmdFirst + IOMSG_ENCRYPT_FILE_MENU, _T("     加密"));
		InsertMenu(subMenu, 1, MF_STRING | MF_BYPOSITION, idCmdFirst + IOMSG_DECRYPT_FILE_MENU, _T("     解密"));

		/*InsertMenu(hmenu, indexMenu+1, MF_STRING | MF_BYPOSITION, idCmdFirst + 102, _T("加密"));
		InsertMenu(hmenu, indexMenu+2, MF_STRING | MF_BYPOSITION, idCmdFirst + 103, _T("解密"));*/
		/*3*/
		InsertMenu(hmenu,indexMenu+2,MF_SEPARATOR|MF_BYPOSITION,0,NULL);//分割线
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IOMSG_DECRYPT_FILE_MENU + 3));
    }

HRESULT CSafeWallExt::InvokeCommand(THIS_
        LPCMINVOKECOMMANDINFO lpici)
    {
        if(HIWORD(((CMINVOKECOMMANDINFOEX *) lpici)->lpVerbW))
		{
			return S_OK;
		}
		else
		{
			WCHAR buffer[MAX_PATH] = {0};
	
			FileNameToVolumePathName(m_pszFileName, buffer, sizeof(buffer) / sizeof(WCHAR));
			switch(LOWORD(lpici->lpVerb))
			{
			case IOMSG_ENCRYPT_FILE_MENU:
				{
					//发送加密指令
					SendDeviceControl(SAFEWALL_ENCRYPT_FILE,(char*) buffer , sizeof(buffer));
					//MessageBox(NULL,buffer,szVolumePathNames,MB_OK);
				}break;
			case IOMSG_DECRYPT_FILE_MENU:
				{
					//发送解密指令
					SendDeviceControl(SAFEWALL_DECRYPT_FILE,(char*) buffer , sizeof(buffer));
					//MessageBox(NULL,buffer,szVolumePathNames,MB_OK);
				}break;
			default:
				break;
			}
		}
		return S_OK;

    }

//哪个菜单被选择
HRESULT CSafeWallExt::GetCommandString(THIS_
        UINT_PTR    idCmd,
        UINT        uType,
        UINT      * pwReserved,
        LPSTR       pszName,
        UINT        cchMax)
    {
        if ( (idCmd != 102) && (idCmd != 103))
		{
			return E_INVALIDARG;
		}
		lstrcpynA(pszName, "数据围墙文件透明加密", cchMax);
		return S_OK;
    }



BOOL SendDeviceControl(int code,char * buffer, int buflen)
{
	BOOL ret = TRUE;

	HANDLE hDevice = CreateFile(SAFEWALL_DEVICE_DOSNAME,
		GENERIC_READ|GENERIC_WRITE , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hDevice == INVALID_HANDLE_VALUE || NULL == hDevice)
	{
		DWORD error = GetLastError();
		WCHAR text[MAX_PATH] = {0};
		swprintf(text, L"设备打开失败，错误代码：%d ,hDevice = 0x%08x" , error, hDevice);
		MessageBox(NULL,text,_T("数据围墙"),MB_OK);
		return FALSE;
	}

	DWORD Bytes = 0 ;
	DWORD IOCTL = CTL_CODE(FILE_DEVICE_UNKNOWN, code,  METHOD_BUFFERED, FILE_ANY_ACCESS);
	DeviceIoControl(hDevice,IOCTL, buffer, buflen, NULL, NULL, &Bytes, NULL);

	if(ret == FALSE)
	{
		MessageBox(NULL,_T("驱动消息发送失败"),_T("数据围墙"),MB_OK);
	}
	CloseHandle(hDevice);
	return ret;
}
