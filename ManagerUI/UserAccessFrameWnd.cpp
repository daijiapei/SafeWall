
#include "manager.h"

LPCTSTR CUserAccessFrameWnd::GetWindowClassName() const {return _T("UIUserAccessFrameWnd");};

UINT CUserAccessFrameWnd::GetClassStyle() const { return UI_CLASSSTYLE_DIALOG;};

void CUserAccessFrameWnd::OnFinalMessage(HWND )
{
	m_pm.RemovePreMessageFilter(this);
	delete this;
}

void CUserAccessFrameWnd::Notify(TNotifyUI& msg)
{
	CDuiString    strName     = msg.pSender->GetName();
	if(msg.sType == _T("selectchanged"))
    {
        CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("UserTabFrame")));
 
        if(strName == _T("OptUser"))
            pControl->SelectItem(0);
        else if(strName == _T("OptDevice"))
            pControl->SelectItem(1);
        else if(strName == _T("OptListen"))
            pControl->SelectItem(2);
		else if(strName == _T("OptConnect"))
            pControl->SelectItem(3);
    }
    else if( msg.sType == _T("click") ) {
        if( strName == _T("closebtn") ) 
		{ 
			Close(TRUE);
			return;
		}
        else if( strName == _T("btnSaveAcc") ) 
		{
			return; 
		}
		else if( strName == _T("btnCancelAcc") ) 
		{
			return; 
		}
		else if( strName == _T("btnSaveDev") ) 
		{
			return; 
		}
		else if( strName == _T("btnCancelDev") ) 
		{
			return; 
		}
		else if( strName == _T("btnAddLsen") ) 
		{
			CAddListenFrameWnd* pDialog= new CAddListenFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
			return; 
		}
		else if( strName == _T("btnChangeLsen") ) 
		{
			CAddListenFrameWnd* pDialog= new CAddListenFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
			return; 
		}
		else if( strName == _T("btnDelLsen") ) 
		{
			return; 
		}
		else if( strName == _T("btnAddConn") ) 
		{
			CAddConnectFrameWnd* pDialog= new CAddConnectFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
			return; 
		}
		else if( strName == _T("btnChangeConn") ) 
		{
			CAddConnectFrameWnd* pDialog= new CAddConnectFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
			return; 
		}
		else if( strName == _T("btnDelConn") ) 
		{
			return; 
		}
    }
}

LRESULT CUserAccessFrameWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    m_pm.Init(m_hWnd);
    m_pm.AddPreMessageFilter(this);
    CDialogBuilder builder;

    CControlUI* pRoot = builder.Create(_T("sysres\\UserAccess.xml"), (UINT)0, NULL, &m_pm);
    ASSERT(pRoot && "Failed to parse XML");
    m_pm.AttachDialog(pRoot);
    m_pm.AddNotifier(this);

    //Init();
    return 0;
}

LRESULT CUserAccessFrameWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if( uMsg == WM_KEYDOWN ) {
        if( wParam == VK_RETURN ) {
            CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
            if( pEdit->GetText().IsEmpty() ) pEdit->SetFocus();
            else {
                pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("pwdedit")));
                if( pEdit->GetText().IsEmpty() ) pEdit->SetFocus();
                else Close();
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

LRESULT CUserAccessFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
				&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
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
