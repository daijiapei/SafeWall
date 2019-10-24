

// HELLO.cpp : Defines the entry point for the application.
//

#include "SwServer.h"

LRESULT CALLBACK LogonDialogWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	HRESULT hr =  CoInitialize(NULL);
 	// TODO: Place code here.
	if(myWSAStartup() == FALSE) return 0;
	HWND hwnd ;
	MSG msg ;

	WNDCLASS wndclass ;

	wndclass.style = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc = FrameWndProc ;
	wndclass.cbClsExtra = 0 ;
	wndclass.cbWndExtra = 0 ;
	wndclass.hInstance = hInstance ;
	wndclass.hIcon = LoadIcon (NULL, MAKEINTRESOURCE(IDI_APP)) ;
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	wndclass.lpszMenuName = NULL ;
	wndclass.lpszClassName = szAppName ;

	if (!RegisterClass (&wndclass))
	{
		MessageBox (NULL, TEXT("This program requires Wndows NT !"),
			szAppName, MB_ICONERROR | MB_OK) ;
		return 0 ;
	}

	SOCKET server = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_LOGON), NULL,(DLGPROC)LogonDialogWndProc,NULL);
	if(NULL == server)
	{
		return 0;
	}

	//到这里,已经登录成功了
	hwnd = CreateWindow (szAppName, 
		szAppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,            //windows菜单的句柄
		hInstance,
		(LPVOID)server);

	ShowWindow (hwnd, nCmdShow) ;
	UpdateWindow (hwnd) ;

	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
	}

	closesocket(server);
	myWSACleanup();
	CoUninitialize();
	return msg.wParam;
}



