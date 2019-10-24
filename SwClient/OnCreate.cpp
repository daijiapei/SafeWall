
#include "SwClient.h"
#include "..\\include\\inirw.h"

DWORD WINAPI CheckStateThread(LPVOID lpParam);

LRESULT OnCreate (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	NOTIFYICONDATA nid;
	RECT rect;
	//创建系统托盘
	HICON hAppIcon = LoadIcon ((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE), MAKEINTRESOURCE(IDI_APP));
	hMenu = LoadMenu((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDR_TRAY_MENU) );
	hMenu = GetSubMenu(hMenu,0);
	//SetProp(hwnd,TEXT("HMENU"),(HANDLE)hMenu);
	SetWindowLong(hwnd, DWL_USER,(LONG)hMenu);

	nid.cbSize = sizeof (NOTIFYICONDATA);
	nid.hWnd   = hwnd;
	nid.uID  = IDI_APP;
	nid.uFlags  = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
	nid.hIcon   = hAppIcon;
	nid.uCallbackMessage = WMU_NOTIFY;
	wcscpy_s(nid.szTip, szAppName);
	wcscpy_s(nid.szInfo, szAppName);
	wcscpy_s(nid.szInfoTitle, TEXT(":)"));
	nid.uTimeout = 10000; 
	nid.dwInfoFlags = NIIF_NONE;
	Shell_NotifyIcon (NIM_ADD, &nid);

	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);

	//ini配置
	int len = 0;
	char userid[64] ={0};
	char password[64]={0};
	int savepwd = 0;
	int autorun = 0;

	INIOBJECT iniobj = CreateIniObject(CONFILE);
	iniGetString(iniobj,"logon", "userid", userid, sizeof(userid),"");
	iniGetString(iniobj,"logon", "password", password, sizeof(password),"");
	savepwd = iniGetInt(iniobj, "logon", "savepwd", savepwd);
	autorun = iniGetInt(iniobj, "logon", "autorun", autorun);
	ReleaseIniObject(iniobj);

	SetDlgItemTextA(hwnd, IDC_USERID, userid);
	if(savepwd)
	{
		CheckDlgButton(hwnd, IDC_SAVEPWD, BST_CHECKED);
		SetDlgItemTextA(hwnd, IDC_PASSWORD, password);
	}
	if(autorun)
	{
		CheckDlgButton(hwnd, IDC_AUTORUN, BST_CHECKED);
	}

	//窗口居中
	GetWindowRect(hwnd,&rect);
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	int x = (cx - rect.right - rect.left) / 2;
	int y = (cy - rect.bottom - rect.top) / 2;
	MoveWindow(hwnd, x,y, rect.right - rect.left, rect.bottom - rect.top, FALSE);
	CloseHandle(CreateThread(NULL, 0 , CheckStateThread, (LPVOID)hwnd, 0,0));
	return 0;
}

DWORD WINAPI CheckStateThread(LPVOID lpParam)
{
	HWND hwnd = (HWND) lpParam;
	while(TRUE)
	{
		Sleep(60 * 1000);
		if(IsWindowVisible(hwnd) == TRUE)
		{
			//窗口是显示的
			SendMessage(hwnd,WM_SHOWWINDOW, TRUE, NULL);
		}
	}
	return 0;
}