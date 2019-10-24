
#include "SwServer.h"


LRESULT OnCreate (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//´°¿Ú¾ÓÖÐ
	RECT rect;
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	MoveWindow(hwnd, cx /5 ,cy /5 ,cx - cx /2.5 ,cy - cy / 2.5, FALSE);

	HICON hIcon = LoadIcon((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_APP));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	return 0;
}