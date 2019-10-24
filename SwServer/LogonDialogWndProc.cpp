
#include "SwServer.h"

LRESULT CALLBACK LogonDialogWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps= {0};
	HMENU hMenu;
	
	char password[64] = {0};
	char * buffer = NULL;
	int lenght;

	Json::Value root; 
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;

	switch(message)
	{
	case WM_INITDIALOG:
		{
			int cx = GetSystemMetrics(SM_CXSCREEN);
			int cy = GetSystemMetrics(SM_CYSCREEN);

			RECT rect;
			GetWindowRect(hwnd,&rect);

			int x = (cx - rect.right - rect.left) / 2;
			int y = (cy - rect.bottom - rect.top) / 2;

			MoveWindow(hwnd, x,y, rect.right - rect.left, rect.bottom - rect.top, FALSE);

			HICON hIcon = LoadIcon((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_APP));
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}break;
	case WM_PAINT:
		{
			HDC hdc = BeginPaint (hwnd, &ps);
			
			EndPaint (hwnd, &ps);
			return FALSE;
		}break;
	case WM_COMMAND:
		{
			SOCKET server;
			SOCKADDR_IN servAddr;
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					int port = GetDlgItemInt(hwnd, IDC_PORT,NULL, TRUE);
					GetDlgItemTextA(hwnd,IDC_PASSWORD, password, sizeof(password));

					server = myTcpSocket();

					if(myConnect(server,"127.0.0.1", port) == FALSE)
					{
						MessageBox(hwnd,L"服务器失败，请检查你的连接！",L"提示", NULL);
						myCloseSocket(server);
						return FALSE;
					}

					context["password"] = password;
					root["command"] = SAFEWALL_SERVER_LOGON;
					root["context"] = context;
					std::string sentbuf = root.toStyledString();

					mySend(server,(char*)sentbuf.c_str(),sentbuf.length(), NULL);
					lenght = myRecv(server,&buffer,NULL);
					
					if(SOCKET_ERROR == lenght)
					{
						MessageBox(hwnd, L"与服务器通信失败！", L"提示", NULL);
						myCloseSocket(server);
						return FALSE;
					}

					reader.parse(buffer,reply);
					free(buffer);
					if(0 == stricmp(reply["state"].asCString(),"success"))
					{
						EndDialog(hwnd, server);
						return TRUE;
					}
					else if(0 == stricmp(reply["state"].asCString(),"failed"))
					{
						MessageBox(hwnd, L"密码错误！", L"提示", NULL);
						myCloseSocket(server);
					}
					else
					{
						MessageBox(hwnd, L"通信错误！", L"提示", NULL);
						myCloseSocket(server);
						EndDialog(hwnd, NULL);
					}
					
					return FALSE;
				}break;
			case IDCANCEL:
				{
					EndDialog(hwnd, NULL);
				}break;
			}
			//return OnCommand (hwnd, message, wParam, lParam);
		}break;
	case WM_CLOSE:
		{
			EndDialog(hwnd, NULL);
			return 0;
		}break;
	}
	return FALSE;
}
