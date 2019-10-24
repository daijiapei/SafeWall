

#include "SwClient.h"

LRESULT OnSysCommand (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case SC_CLOSE:
		{
			ShowWindow(hwnd,FALSE);
			return TRUE;
		}break;
	}
	
	return FALSE;
}

