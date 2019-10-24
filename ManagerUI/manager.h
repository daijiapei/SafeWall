
#ifndef __SW_MANAGER
#define __SW_MANAGER

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0501
#endif

#include "csocket.h"
#include <windows.h>
#include <objbase.h>

#include "json\\json.h"
#pragma comment(lib, "json_vc71_libmt.lib") 

#include "..\DuiLib\UIlib.h"

using namespace DuiLib;

#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "..\\Lib\\DuiLib_ud.lib")
#   else
#       pragma comment(lib, "..\\Lib\\DuiLib_d.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "..\\Lib\\DuiLib_u.lib")
#   else
#       pragma comment(lib, "..\\Lib\\DuiLib.lib")
#   endif
#endif

//#include <GdiPlus.h>
//#pragma comment( lib, "GdiPlus.lib" )
//using namespace Gdiplus;

class CAddUserFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	
	HWND hManagerWnd;
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND );
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

protected:
	CPaintManagerUI m_pm;
};

class CAddProcessItemFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	HWND hManagerWnd;
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND );
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
protected:
	CPaintManagerUI m_pm;
};

class CAddListenFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	HWND hManagerWnd;
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND );
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
protected:
	CPaintManagerUI m_pm;
};

class CAddConnectFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	HWND hManagerWnd;
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND );
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
protected:
	CPaintManagerUI m_pm;
};

class CUserAccessFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	HWND hManagerWnd;
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND );
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
protected:
	CPaintManagerUI m_pm;
};

class CChangePwdFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND );
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
protected:
	CPaintManagerUI m_pm;
};

class CLoginFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	LPCTSTR GetWindowClassName() const;
	
	void OnFinalMessage(HWND );
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

private:
	UINT GetClassStyle() const;
	BOOL OnLogin();

protected:
	CPaintManagerUI m_pm;
};

class CDuiFrameWnd : public CWindowWnd, public INotifyUI
{
public:
	CDuiFrameWnd();
	virtual LPCTSTR GetWindowClassName() const;
	virtual UINT GetClassStyle() const ;
	virtual void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);


	void OnFinalMessage(HWND );

private:
	void OnPrepare();
	void OnClose();

protected:
	CPaintManagerUI m_PaintManager;
private:
	CButtonUI* m_pCloseBtn;
};

#include "..\\include\\sevdef.h"
extern CMyTcpSocket * server;
extern Json::Value root;

#define DuiMessageBox  MessageBox

#define  WM_NOTHING         (WM_USER+1)
#define  WM_SOCKET_EVENT  (WM_USER+2)
#define  WMU_NOTIFY         (WM_USER+3)

#ifdef __cplusplus
extern "C" {
#endif

//UINT DuiMessageBox(HWND hwnd, TCHAR * lpText, TCHAR * lpCaption, UINT uType);

#ifdef __cplusplus
}
#endif

#endif
