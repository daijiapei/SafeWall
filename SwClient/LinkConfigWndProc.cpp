
#include "SwClient.h"
#include "..\\include\\inirw.h"

LRESULT CALLBACK LinkConfigWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps= {0};
	char ipaddr[256] = {0};
	char port[32] = {0};

	switch(message)
	{
	case WM_INITDIALOG:
		{
			//初始化登录窗口

			INIOBJECT iniobj = CreateIniObject(CONFILE);
			if(iniobj)
			{
				iniGetString(iniobj,"safewall server","ipaddr",ipaddr,sizeof(ipaddr),"");
				iniGetString(iniobj,"safewall server","port",port,sizeof(port),"");
			}

			ReleaseIniObject(iniobj);
			SetDlgItemTextA(hwnd, IDC_IPADDR, ipaddr);
			SetDlgItemTextA(hwnd, IDC_PORT, port);

		}break;
	case WM_PAINT:
		{
			HDC hdc = BeginPaint (hwnd, &ps);

			EndPaint (hwnd, &ps);
			return FALSE;
		}break;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					int len = 0;
					len = GetDlgItemTextA(hwnd, IDC_IPADDR, ipaddr, sizeof(ipaddr));
					ipaddr[len] = NULL;
					len = GetDlgItemTextA(hwnd, IDC_PORT, port, sizeof(port));
					port[len] = NULL;
					
					INIOBJECT iniobj = CreateIniObject(CONFILE);
					iniSetString(iniobj,"safewall server","ipaddr",ipaddr);
					iniSetString(iniobj,"safewall server","port",port);
					ReleaseIniObject(iniobj);
					MessageBox(hwnd,L"保存成功",L"提示",NULL);
					EndDialog(hwnd, 0);
				}break;
			case IDCANCEL:
				{
					//DestroyWindow(hwnd);
					EndDialog(hwnd, 0);
				}break;
			case IDB_TEST:
				{
					MessageBox(hwnd,TEXT("测试连接"),TEXT("测试"),NULL);
				}break;
			}
			return 0;
		}break;
	case WM_CLOSE:
		{
			EndDialog(hwnd, wParam);
			return 0;
		}break;
	}
	return FALSE;
}
