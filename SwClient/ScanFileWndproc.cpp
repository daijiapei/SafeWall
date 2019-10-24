
#include "SwClient.h"

LRESULT CALLBACK ScanFileWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps= {0};

	switch(message)
	{
	case WM_INITDIALOG:
		{
			//³õÊ¼»¯µÇÂ¼´°¿Ú
			//int cx = GetSystemMetrics(SM_CXSCREEN);
			//int cy = GetSystemMetrics(SM_CYSCREEN);

			//RECT rect;
			//GetWindowRect(hwnd,&rect);

			//int x = (cx - rect.right - rect.left) / 2;
			//int y = (cy - rect.bottom - rect.top) / 2;

			//MoveWindow(hwnd, x,y, rect.right - rect.left, rect.bottom - rect.top, FALSE);
		}break;
	case WM_PAINT:
		{
			HDC hdc = BeginPaint (hwnd, &ps);

			EndPaint (hwnd, &ps);
			return FALSE;
		}break;
	case WM_COMMAND:
		{
			//return OnCommand (hwnd, message, wParam, lParam);
		}break;
	case WM_CLOSE:
		{
			EndDialog(hwnd, wParam);
			return 0;
		}break;
	}
	return FALSE;
}
