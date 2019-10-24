#ifndef __UIEDIT_H__
#define __UIEDIT_H__

#pragma once

namespace UiLib
{
	class CEditWnd;

	class UILIB_API CEditUI : public CLabelUI
	{
		friend class CEditWnd;
	private:
		class UILIB_API CTimer
		{
		public:
			CTimer();
			~CTimer();
		public:
			void Start();
			void Stop();
			void SetDelay(UINT _Delay = 1000);
			UINT GetDelay();
			void SetCurTickCount();
			bool CheckTickDaYuDelay();
			bool IsRun();
		public:
			static void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
		private:
			bool	mIsRun;
			UINT	m_Delay;
			UINT	m_TimerID;
			UINT	m_TimerAccuracy;
		public:
			CEditUI* pEditUI;
			DWORD m_CurTickCount;
		};
	public:
		CEditUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(bool bEnable = true);
		void SetText(LPCTSTR pstrText);
		void SetMaxChar(UINT uMax);
		UINT GetMaxChar();
		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;
		void SetPasswordMode(bool bPasswordMode);
		bool IsPasswordMode() const;
		void SetPasswordChar(TCHAR cPasswordChar);
		TCHAR GetPasswordChar() const;
		void SetNumberOnly(bool bNumberOnly);
		bool IsNumberOnly() const;
		int GetWindowStyls() const;

		LPCTSTR GetNormalImage();
		void SetNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetHotImage();
		void SetHotImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusedImage();
		void SetFocusedImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage();
		void SetDisabledImage(LPCTSTR pStrImage);
		void SetNativeEditBkColor(DWORD dwBkColor);
		DWORD GetNativeEditBkColor() const;

		void SetSel(long nStartChar, long nEndChar);
		void SetSelAll();
		void SetReplaceSel(LPCTSTR lpszReplace);

		bool MatchRegular(bool isShowMsg = true);
		void SetRegularCheck(LPCTSTR pRegularCheckStr);
		LPCTSTR GetRegularCheck();
		void SetRegularTip(LPCTSTR pRegularTipStr);
		LPCTSTR GetRegularTip();
		void SetMatchCase(bool bMatchCase = false);
		bool GetMatchCase();

		void SetTipValue(LPCTSTR pStrTipValue);
		void SetTipValueColor(LPCTSTR pStrColor);
		DWORD GetTipValueColor();
		CDuiString GetTipValue();
		LPCTSTR GetSrcTipValue();

		void SetPos(RECT rc);
		void SetVisible(bool bVisible = true);
		void SetInternVisible(bool bVisible = true);
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintStatusImage(HDC hDC);
		void PaintText(HDC hDC);

		void SetEnableTimer(bool bEnableTime);
		bool GetEnableTimer();
		void SetTimerDelay(UINT nDelay);
		CEditUI::CTimer* GetTimerObj();
	protected:
		HWND m_Hwnd;
		CEditWnd*	m_pWindow;
		CEditUI::CTimer		m_Timer;
		bool		m_bEnableTime;
		CDuiString	m_sCheckVal;

		UINT m_uMaxChar;
		bool m_bReadOnly;
		bool m_bPasswordMode;
		bool m_bMatchCase;
		TCHAR m_cPasswordChar;
		UINT m_uButtonState;
		CDuiString m_RegularCheckStr;
		CDuiString m_RegularTipStr;
		CDuiString m_RegluarSrcText;
		CDuiString m_sNormalImage;
		CDuiString m_sHotImage;
		CDuiString m_sFocusedImage;
		CDuiString m_sDisabledImage;
		CDuiString m_sTipValue;
		CDuiString m_sSrcTipValue;
		DWORD m_sTipValueColor;
		DWORD m_dwEditbkColor;
		int m_iWindowStyls;
	};
}
#endif // __UIEDIT_H__