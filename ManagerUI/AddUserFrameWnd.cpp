
#include "manager.h"

LPCTSTR CAddUserFrameWnd::GetWindowClassName() const {return _T("UIAddUserFrame");};

UINT CAddUserFrameWnd::GetClassStyle() const { return UI_CLASSSTYLE_DIALOG;};

void CAddUserFrameWnd::OnFinalMessage(HWND )
{
	m_pm.RemovePreMessageFilter(this);
	delete this;
}

void CAddUserFrameWnd::Notify(TNotifyUI& msg)
{
    if( msg.sType == _T("click") ) {
        if( msg.pSender->GetName() == _T("closebtn") ) 
		{ 
			Close(TRUE);
		}
        else if( msg.pSender->GetName() == _T("btnLogin") ) 
		{
			Close(TRUE);
			return; 
		}
		else if( msg.pSender->GetName() == _T("btnExit") ) 
		{
			PostQuitMessage(0); return; 
			return; 
		}

    }
}

LRESULT CAddUserFrameWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    m_pm.Init(m_hWnd);
    m_pm.AddPreMessageFilter(this);
    CDialogBuilder builder;
    //CDialogBuilderCallbackEx cb;
    CControlUI* pRoot = builder.Create(_T("sysres\\AddUser.xml"), (UINT)0, NULL, &m_pm);
    ASSERT(pRoot && "Failed to parse XML");
    m_pm.AttachDialog(pRoot);
    m_pm.AddNotifier(this);

    //Init();
    return 0;
}

LRESULT CAddUserFrameWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
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

LRESULT CAddUserFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
