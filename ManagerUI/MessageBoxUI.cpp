

#include "manager.h"

class CMessageBoxUI : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	LPCTSTR GetWindowClassName() const {return _T("MessageBoxUI");};
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG;};
	void OnFinalMessage(HWND )
	{
		m_pm.RemovePreMessageFilter(this);
		delete this;
	}

	void SetCaption(TCHAR * lpCaption)
	{
		CLabelUI* captionUI = static_cast<CLabelUI*>(m_pm.FindControl(_T("caption")));
		captionUI->SetText(lpCaption);
	}

	void SetNotice(TCHAR * lpText)
	{
		CLabelUI* noticeUI = static_cast<CLabelUI*>(m_pm.FindControl(_T("notice")));
		noticeUI->SetText(lpText);
		int len , row;
#ifdef _UNICODE
		len = wcslen(lpText);
#else
		len = strlen(lpText);
#endif
		row = len / 35 + 1;

		//计算显示长度：
		m_width =  200 + ((row > 1) ? (35 * 16) : (len % 35) * 16);
		m_height = 140 + 20 * row;
		
		MoveWindow(m_hWnd,0 ,0, m_width,m_height,FALSE);
	}

	void Notify(TNotifyUI& msg)
	{
		if( msg.sType == _T("click") ) {
			if( msg.pSender->GetName() == _T("closebtn") ) 
			{ 
				PostQuitMessage(0); return; 
			}
			else if( msg.pSender->GetName() == _T("btnSure") ) 
			{
				Close(TRUE);
				return; 
			}
			else if( msg.pSender->GetName() == _T("btnCancel") ) 
			{
				Close(FALSE);
				return; 
			}
		}
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		m_pm.AddPreMessageFilter(this);
		CDialogBuilder builder;
		//CDialogBuilderCallbackEx cb;
		CControlUI* pRoot = builder.Create(_T("sysres\\MessageBoxUI.xml"), (UINT)0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		//Init();
		return 0;
	}

	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
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

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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


protected:
	CPaintManagerUI m_pm;
	
private:
	int m_width;
	int m_height;
};


UINT DuiMessageBox(HWND hwnd, TCHAR * lpText, TCHAR * lpCaption, UINT uType)
{
	CMessageBoxUI* pDialog= new CMessageBoxUI();
	if( pDialog == NULL ) { return -1;}
	pDialog->Create(hwnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
	pDialog->SetCaption(lpCaption);
	pDialog->SetNotice(lpText);
	pDialog->CenterWindow();
		
	return pDialog->ShowModal();
}