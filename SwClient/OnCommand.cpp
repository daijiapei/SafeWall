

#include "SwClient.h"
#include "..\\include\\inirw.h"

LRESULT OnCommand (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char userid[64] ={0};
	char password[64]={0};
	char title[64] = {0};
	MYSERVCONTEXT servContext = {0};
	servContext.fromHwnd = (long*)hwnd;
	UINT savepwd = BST_UNCHECKED;
	UINT autorun = BST_UNCHECKED;

	switch(LOWORD(wParam))
	{
	case IDOK:
		{
			int len = 0;

			if(NotifyService(MY_SERVICE_CONTROL_GET_USERID,&servContext))
			{
				if(servContext.userid[0] != '\0')
				{
					MessageBox(hwnd,L"用户已登录！重新登录请注销！",L"提示",NULL);
					return 0;
				}
			}
			len = GetDlgItemTextA(hwnd, IDC_USERID, userid, sizeof(userid));
			len = GetDlgItemTextA(hwnd, IDC_PASSWORD, password, sizeof(password));
			savepwd = IsDlgButtonChecked(hwnd, IDC_SAVEPWD);
			autorun = IsDlgButtonChecked(hwnd, IDC_AUTORUN);

			INIOBJECT iniobj = CreateIniObject(CONFILE);
			iniSetString(iniobj,"logon","userid",userid);
			if(BST_CHECKED  == savepwd )
			{
				iniSetInt(iniobj,"logon","savepwd",1,10);
				iniSetString(iniobj,"logon","password",password);
			}
			else 
			{
				iniSetInt(iniobj,"logon","savepwd",0,10);
				iniSetString(iniobj,"logon","password","");
			}

			if(BST_CHECKED  == autorun )
			{
				iniSetInt(iniobj,"logon","autorun",1,10);
			}
			else 
			{
				iniSetInt(iniobj,"logon","autorun",0,10);
			}
			ReleaseIniObject(iniobj);

			strcpy_s(servContext.userid, userid);
			strcpy_s(servContext.password, password);
			if(NotifyService(MY_SERVICE_CONTROL_LOGON,&servContext))
			{
				strcpy_s(title, servContext.userid);
				strcat_s(title, "-已登录");
				SetWindowTextA(hwnd, title);
				MessageBox(hwnd,L"登录成功",L"提示",NULL);
			}
			else
			{
				MessageBox(hwnd,L"登录失败",L"提示",NULL);
			}
		}break;
	case IDCANCEL:
		{
			ShowWindow(hwnd,FALSE);
		}break;
	case ID_MAIN: //主界面
		{
			ShowWindow(hwnd,TRUE);
		}break;
	case ID_CANCEL_LOGON:  //注销登录
		{
			NotifyService(MY_SERVICE_CONTROL_CANCEL_LOGON,&servContext);
			SetWindowTextW(hwnd, szAppName);
			MessageBox(hwnd,L"已注销",L"提示",NULL);
		}break;
	case ID_LINK_CONFIG: //连接配置
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_LINK_CONFIG), hwnd,(DLGPROC)LinkConfigWndProc, NULL );
		}break;
	case ID_SYSTEM_CONFIG:  //系统配置
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_SYSTEM_CONFIG), hwnd,(DLGPROC)SystemConfigWndProc, NULL );
			MessageBox(hwnd,L"系统配置",L"提示",NULL);
		}break;
	case ID_REFRESH_STRATEGY:  //刷新策略
		{
			if(NotifyService(MY_SERVICE_CONTROL_GET_ACCESS,&servContext))
			{
				MessageBox(hwnd,L"策略刷新成功",L"提示",NULL);
			}
			else
			{
				MessageBox(hwnd,L"策略刷新失败",L"提示",NULL);
			}
		}break;
	case ID_CHANGE_PWD:
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_CHANGE_PWD), hwnd,(DLGPROC)ChangePwdWndProc,NULL );
		}break;
	case ID_SCANFILE:  //扫描加密文档
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_SCANFILE), hwnd,(DLGPROC)ScanFileWndProc,NULL );
		}break;
	default: //未知
		break;
	}

	return 0;
}