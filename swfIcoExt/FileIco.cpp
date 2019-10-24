// FileIco.cpp : CFileIco 的实现


#include "stdafx.h"
#include "FileIco.h"
#include "..\\include\\assist.h"

//用IsMemberOf来判断是否是你需要叠加图标的对象
//用GetPriority来指定你叠加图标的优先级
//用GetOverlayInfo来指定图标路径和图标索引号

//IsMemberOf返回OK执行GetOverlayInfo（查询用哪个图标和图标路径）
STDMETHODIMP CFileIco::IsMemberOf(THIS_ LPCWSTR pwszPath, DWORD dwAttrib)
{
	WCHAR buffer =  NULL;
	WCHAR VolumePathName[MY_MAX_PATH] = {0};
	HRESULT hr = S_FALSE;
	DWORD Bytes = 0 ;
	DWORD ShowIco = 0;

	if(hDevice == INVALID_HANDLE_VALUE || NULL == hDevice)
	{
		/*wsprintf(VolumePathName,L"hDevice=0x%08x, Error=%d",hDevice,GetLastError());
		MessageBox(NULL,VolumePathName,VolumePathName,NULL);*/
		return hr;
	}

	if(PathIsDirectory(pwszPath) == FILE_ATTRIBUTE_DIRECTORY) return hr; //文件夹不显示图标
	/*FileNameToVolumePathName(pwszPath, VolumePathName, sizeof(VolumePathName));
	MessageBox(NULL,VolumePathName, pwszPath, NULL);*/

	//将盘符转换成磁盘路径， 然后向驱动发送请求，咨询该文件是否加密文件
	if(FileNameToVolumePathName(pwszPath, VolumePathName, sizeof(VolumePathName)) &&
		DeviceIoControl(hDevice,IOCTL_QUERY_FILEATTRIBUTES, VolumePathName,
		sizeof(VolumePathName), &ShowIco, sizeof(ShowIco), &Bytes, NULL) && ShowIco)
	{
		hr = S_OK;
	}

	return hr;
}

//GetOverlayInfo返回OK执行GetPriority

STDMETHODIMP CFileIco::GetOverlayInfo(THIS_ LPWSTR pwszIconFile, int cchMax, int * pIndex, DWORD * pdwFlags)
{
	HKEY hSubKey; 
	//将Ico路径读入pwszIconFile变量中
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SOFTWARE\\SafeWall\\Client",
		0,KEY_READ| KEY_WRITE, &hSubKey);
	RegQueryValueEx(hSubKey,L"FileIco",NULL, NULL ,(LPBYTE)pwszIconFile ,(LPDWORD)&cchMax);
	RegCloseKey(hSubKey);
	//MessageBox(NULL,pwszIconFile,pwszIconFile,NULL);
	*pIndex=0; 
	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
	return S_OK;
}

STDMETHODIMP CFileIco::GetPriority(THIS_ int * pIPriority)
{
	*pIPriority = 0;//优先级标志，一般是0，
	return S_OK;
}
