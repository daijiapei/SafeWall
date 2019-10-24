
#include "manager.h"

LPCTSTR CLoginFrameWnd::GetWindowClassName() const {return _T("UILoginFrame");};

UINT CLoginFrameWnd::GetClassStyle() const { return UI_CLASSSTYLE_DIALOG;};

void CLoginFrameWnd::OnFinalMessage(HWND )
{
	m_pm.RemovePreMessageFilter(this);
	delete this;
}

void CLoginFrameWnd::Notify(TNotifyUI& msg)
{
    if( msg.sType == _T("click") ) {
        if( msg.pSender->GetName() == _T("closebtn") ) 
		{ 
			PostQuitMessage(0); 
			return; 
		}
        else if( msg.pSender->GetName() == _T("btnLogin") ) 
		{
			Close(OnLogin());
			//Close(TRUE);
			return; 
		}
		else if( msg.pSender->GetName() == _T("btnExit") ) 
		{
			PostQuitMessage(0);
			return; 
		}
    }
}

BOOL CLoginFrameWnd::OnLogin()
{
	Json::Value json;
	Json::Value json2;
	Json::Value context;
	Json::Value reply;
	Json::Reader reader;
	BOOL bRet = FALSE;
	CEditUI* edtPassword = static_cast<CEditUI*>(m_pm.FindControl(_T("edtPassword")));
	char * buffer;
	context["password"] = edtPassword->GetText().GetData();
	json["command"] = SAFEWALL_SERVER_LOGON;
	json["context"] = context;
	std::string sentbuf = json.toStyledString();
	do
	{
		if(FALSE == server->connect("127.0.0.1",1808))
		{
			DuiMessageBox(m_hWnd, _T("连接服务器失败"), _T("提示"), NULL);
		}
		
		if(SOCKET_ERROR == server->send(sentbuf.c_str(),sentbuf.length()))
		{
			break;
		}
		
		if(SOCKET_ERROR == server->recv(&buffer))
		{
			break;
		}

		//对接收到的信息进行解析
		if(false == reader.parse(buffer,reply))
		{
			break;
		}

		if(0 != stricmp(reply["state"].asCString(),"success"))
		{
			//登录失败
			DuiMessageBox(m_hWnd, _T("登录失败，请检查你的密码"), _T("提示"), NULL);
			break;
		}
		free(buffer);
		//到这里就是成功了，接下来获取策略
		json2["command"]=SAFEWALL_SERVER_GET_ACCESS;
		std::string sentbuf2 = json2.toStyledString();

		if(SOCKET_ERROR == server->send(sentbuf2.c_str(),sentbuf2.length()))
		{
			break;
		}
		
		if(SOCKET_ERROR == server->recv(&buffer))
		{
			break;
		}

		//对接收到的信息进行解析
		if(false == reader.parse(buffer,root))
		{
			break;
		}
		free(buffer);
		bRet = TRUE;
	}while(FALSE);
	
	return bRet;
}

LRESULT CLoginFrameWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    m_pm.Init(m_hWnd);
    m_pm.AddPreMessageFilter(this);
    CDialogBuilder builder;
    //CDialogBuilderCallbackEx cb;
    CControlUI* pRoot = builder.Create(_T("sysres\\login.xml"), (UINT)0, NULL, &m_pm);
    ASSERT(pRoot && "Failed to parse XML");
    m_pm.AttachDialog(pRoot);
    m_pm.AddNotifier(this);

    //Init();
    return 0;
}

LRESULT CLoginFrameWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if( uMsg == WM_KEYDOWN ) {
        if( wParam == VK_RETURN ) {
			//MessageBox(m_hWnd,"MessageHandler","VK_RETURN",NULL);
            CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("edtPassword")));
            if( pEdit->GetText().IsEmpty() ) pEdit->SetFocus();
            else {
                Close(OnLogin());
            }
            return true;
        }
        else if( wParam == VK_ESCAPE ) {
            PostQuitMessage(0);
            return true;
        }

    }
    return false;
}

LRESULT CLoginFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch(uMsg)
	{
	case WM_CREATE:
		{
			lRes = OnCreate(uMsg,wParam,lParam,bHandled);
		}break;
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
			RECT rcCaption = m_pm.GetCaptionRect();

			if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
				&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) 
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
				//如果是按钮（一般是关闭之类的按钮）肯定不处理
				if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 )
					return HTCAPTION;
			}

			return HTCLIENT;
		}break;
	default:
		bHandled = FALSE;
	}
	if(bHandled) return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
