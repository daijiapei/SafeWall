
#include "SwServer.h"

LRESULT CALLBACK FrameWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

    static SOCKET server;
	SOCKET client;
	int len =0;
	char  sendBuf[1024] = {0};
	char  recvBuf[1024] = {0};
	SOCKADDR_IN addrClient = {0};

	HMENU hMenu;

	switch (message)
	{
	case WM_CREATE:
		{
			return OnCreate(hwnd, message, wParam, lParam);
		}break;
	case WM_SOCKET_EVENT:
		{
			//server = (SOCKET)wParam; // 发生网络事件的套接字   
            //long event = WSAGETSELECTEVENT(lParam); // 事件   
            int error = WSAGETSELECTERROR(lParam); // 错误码  

			switch(WSAGETSELECTEVENT(lParam))
			{
			case FD_ACCEPT:
				{
					//client = accept(server,(SOCKADDR*)&addrClient,&len);
					client = accept(server,NULL,NULL);

					//MessageBox(hwnd,"我在检测accept是否通过……","提示：",NULL);

					if(client != INVALID_SOCKET)
					{
						/*WSAAsyncSelect(client,hwnd,WM_NOTHING,NULL);
						ioctlsocket(client,FIONBIO,NULL);*/
						/*struct linger {
						  u_short l_onoff;
						  u_short l_linger;
						};*/
						linger m_sLinger;
						m_sLinger.l_onoff = 1;
						//在调用closesocket（）时还有数据未发送完，允许等待
						//若m_sLinger.l_onoff=0；则调用closesocket（）后强制关闭
						m_sLinger.l_linger = 5; //设置等待时间为5秒

						DWORD TimeOut = TIMEOUT;
						setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,(const char*)&TimeOut,sizeof(TimeOut));
						setsockopt(client, SOL_SOCKET, SO_SNDTIMEO,(const char*)&TimeOut,sizeof(TimeOut));
					}
					else
					{
						//MessageBox(hwnd,_T("accept发生错误！"),_T("提示："),NULL);
					}
				}
				break;
			}
			return 0;
		}break;
	case WM_COMMAND:
		{
			return OnCommand (hwnd, message, wParam, lParam);
		}break;
	case WM_PAINT:
		{
			hdc = BeginPaint (hwnd, &ps);

			EndPaint (hwnd, &ps);
			return 0;
		}
	case WM_DESTROY:
		{
			PostQuitMessage (0);
			return 0;
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}


