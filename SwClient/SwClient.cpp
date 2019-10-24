

#include "SwClient.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HRESULT hr =  CoInitialize(NULL);
	//SelfPrivilegeUp();
	HANDLE hMutext = CreateMutexA(NULL, FALSE, szClientMutexName);
	HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,0,
			sizeof(MYSERVCONTEXT), szClientMapName);

	DialogBox(hInstance,MAKEINTRESOURCE(IDD_LOGON), NULL,(DLGPROC)DialogWndProc );

	CloseHandle(hMap);
	CloseHandle(hMutext);
	CoUninitialize();
	return 0;

}

LRESULT CALLBACK DialogWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps= {0};
	NOTIFYICONDATA nid;
	HMENU hMenu;
	MYSERVCONTEXT servContext = {0};
	servContext.fromHwnd = (long*)hwnd;
	switch(message)
	{
	case WM_INITDIALOG:
		{
			return OnCreate (hwnd, message, wParam, lParam);	
		}break;
	case WM_SHOWWINDOW:
		{
			char title[256] = {0};
			if(wParam)
			{
				if(NotifyService(MY_SERVICE_CONTROL_GET_USERID, &servContext))
				{
					if(servContext.userid[0])
					{
						strcpy_s(title, servContext.userid);
						strcat_s(title, "-ÒÑµÇÂ¼");
					}
				}

				if(title[0])
				{
					SetWindowTextA(hwnd, title);
				}
				else
				{
					SetWindowTextW(hwnd, szAppName);
				}
			}
		}break;
	case WM_PAINT:
		{
			HDC hdc = BeginPaint (hwnd, &ps);

			EndPaint (hwnd, &ps);
			return FALSE;
		}break;
	case WM_SYSCOMMAND:
		{
			//return OnSysCommand (hwnd, message, wParam, lParam);
		}break;
	case WM_COMMAND:
		{
			return OnCommand (hwnd, message, wParam, lParam);
		}break;
	case WM_DRAWITEM:
		{
			return 0;
		}break;
	case WMU_NOTIFY: //ÍÐÅÌÏûÏ¢
		{
			POINT point;
			switch(lParam)
			{
			case WM_RBUTTONDOWN:
				{
					//hMenu = (HMENU)GetProp(hwnd,TEXT("HMENU"));
					hMenu = (HMENU)GetWindowLong(hwnd, DWL_USER);
					GetCursorPos(&point) ;
					SetForegroundWindow(hwnd) ;
					TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd , NULL);
				}break;
			case WM_LBUTTONDOWN:
				{
					//ShowWindow(hwnd, SW_SHOW);
					ShowWindow(hwnd,!IsWindowVisible(hwnd));
				}break;
			}
			return 0;
		}break;
	case WM_CLOSE:
		{
			nid.uID  = IDI_APP;
			nid.hWnd = hwnd;
			Shell_NotifyIcon (NIM_DELETE, &nid);
			EndDialog(hwnd, wParam);
			return 0;
		}break;
	}
	return FALSE;
}
