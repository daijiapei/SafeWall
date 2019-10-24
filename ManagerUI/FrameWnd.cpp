
#include "manager.h"
#include "resource.h"

CMyTcpSocket * server;
Json::Value root;

CDuiFrameWnd::CDuiFrameWnd() {};

LPCTSTR CDuiFrameWnd::GetWindowClassName() const {   return _T("ManagerFrame");  };
UINT CDuiFrameWnd::GetClassStyle() const { return CS_DBLCLKS; };
void CDuiFrameWnd::OnFinalMessage(HWND /*hWnd*/) { delete this; };


void CDuiFrameWnd::OnPrepare()
{
	CLoginFrameWnd* pLoginFrame = new CLoginFrameWnd();
    if( pLoginFrame == NULL ) { Close(); return; }
    pLoginFrame->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
	ShowWindow(false);
    pLoginFrame->CenterWindow();
		
	server = new CMyTcpSocket();
    if(pLoginFrame->ShowModal())
	{
		ShowWindow(true);
	}
	else
	{
		delete server;
	}
}

void CDuiFrameWnd::OnClose()
{
	return ;
}

void CDuiFrameWnd::Notify(TNotifyUI& msg)
{
	CDuiString    strName     = msg.pSender->GetName();
	if( msg.sType == _T("windowinit") ) 
	{
		OnPrepare();
	}
    else if(msg.sType == _T("selectchanged"))
    {
        
        CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("TabFrame")));
 
        if(strName == _T("OptControlKey"))
            pControl->SelectItem(0);
        else if(strName == _T("OptFileAccess"))
            pControl->SelectItem(1);
        else if(strName == _T("OptProcessAccess"))
            pControl->SelectItem(2);
		else if(strName == _T("OptUserSetting"))
            pControl->SelectItem(3);
		else if(strName == _T("OptOnlineUser"))
            pControl->SelectItem(4);
		else if(strName == _T("OptLookLog"))
            pControl->SelectItem(5);
    }
	else if(msg.sType == _T("click"))
	{
		if(strName == _T("closebtn"))
		{
			//系统按钮-关闭
			OnClose();
			PostQuitMessage(0);
			return;
		}
		else if(strName == _T("btnChangePwd"))
		{
			//修改集控密码
			CChangePwdFrameWnd* pDialog= new CChangePwdFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnfaSave"))
		{
			//保存文件后缀策略
		}
		else if(strName == _T("btnfaCancel"))
		{
			//取消修改文件后缀策略
		}
		else if(strName == _T("btnPaAdd"))
		{
			//新增进程策略
			CAddProcessItemFrameWnd* pDialog= new CAddProcessItemFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnPaSave"))
		{
			//保存进程策略
		}
		else if(strName == _T("btnPaCancel"))
		{
			//取消修改进程策略
		}
		else if(strName == _T("btnAddUser"))
		{
			//添加用户
			CAddUserFrameWnd* pDialog= new CAddUserFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnChangeUser"))
		{
			//修改用户
			CUserAccessFrameWnd* pDialog= new CUserAccessFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnDelUser"))
		{
			//删除用户
		}
	}
	else if(msg.sType == _T("itemselect"))
	{
	}
};

LRESULT CDuiFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;

	switch(uMsg)
	{
	case WM_CREATE:
		{
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~(WS_CAPTION);
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			m_PaintManager.Init(m_hWnd);

			CDialogBuilder builder;
			CControlUI* pRoot = builder.Create(_T("sysres\\manager.xml"), (UINT)0, NULL, &m_PaintManager);   // duilib.xml需要放到exe目录下
			ASSERT(pRoot && "Failed to parse XML");

			m_PaintManager.AttachDialog(pRoot);
			m_PaintManager.AddNotifier(this);   // 添加控件等消息响应，这样消息就会传达到duilib的消息循环，我们可以在Notify函数里做消息处理

			m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));//添加系统按钮
			return lRes;
		}break;
		// 以下3个消息WM_NCACTIVATE、WM_NCCALCSIZE、WM_NCPAINT用于屏蔽系统标题栏
	case WM_NCACTIVATE:
		{
			if( !::IsIconic(m_hWnd) ) 
			{
				return (wParam == 0) ? TRUE : FALSE;
			}
		}break;
	case WM_NCCALCSIZE:
		{
			return 0;
		}break;
	case WM_NCPAINT:
		{
			return 0;
		}break;
	case WM_NCHITTEST:
		{
			//通过标题栏拖动窗口
			POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
			::ScreenToClient(*this, &pt);

			RECT rcClient;
			::GetClientRect(*this, &rcClient);

			RECT rcCaption = m_PaintManager.GetCaptionRect();
			if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
				&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
					CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(pt));
					//如果是按钮（一般是关闭之类的按钮）肯定不处理
					if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 )
						return HTCAPTION;
			}

			return HTCLIENT;
		}break;
	default:
		break;
	}


    if( m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes) ) 
    {
        return lRes;
    }

    return __super::HandleMessage(uMsg, wParam, lParam);
}


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    CPaintManagerUI::SetInstance(hInstance);
	HRESULT Hr = ::CoInitialize(NULL);
	CMyTcpSocket::WSAStartup();
    CDuiFrameWnd * pFrame = new CDuiFrameWnd();

	pFrame->Create(NULL, _T("数据围墙管理器"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
    pFrame->CenterWindow();
	pFrame->ShowModal();
	pFrame->SetIcon(IDI_ICON1);
	//ShowWindow(*pFrame, SW_SHOWNORMAL);

 //   CPaintManagerUI::MessageLoop();
	CMyTcpSocket::WSACleanup();
	::CoUninitialize();
    return 0;
}