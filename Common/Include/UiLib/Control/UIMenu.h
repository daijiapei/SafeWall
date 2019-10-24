#ifndef UIMenu_h__
#define UIMenu_h__

#pragma once

namespace UiLib
{
	class CMenuUI;
	class COptionUI;
	class CButtonUI;
	class CHorizontalLayoutUI;

	class CMenuItemUI : public CListContainerElementUI
	{
	public:
		CMenuItemUI(CMenuUI* _pMenuUI = NULL);
		~CMenuItemUI(void);

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

	public:
		CHorizontalLayoutUI*	GetItemHorzLayout(){return pHorz;};
		COptionUI*				GetPrefixBtn(){return pPrefixBtn;};
		CButtonUI*				GetTextBtn(){return pTextBtn;};
		CButtonUI*				GetSubMenuBtn(){return pSubMenuBtn;};

	private:
		CHorizontalLayoutUI*	pHorz;
		COptionUI*				pPrefixBtn;
		CButtonUI*				pTextBtn;
		CButtonUI*				pSubMenuBtn;
		CMenuUI*				pMenuUI;
	};

	class CMenuUI : public CListUI
	{
	public:
		CMenuUI(void);
		~CMenuUI(void);

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

	private:
		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);

		int GetItemIndex(CControlUI* pControl) const;
		bool SetItemIndex(CControlUI* pControl, int iIndex);

	public:
		bool Add(CMenuItemUI* _pMenuItem);
		bool AddAt(CMenuItemUI* _pMenuItem,int iIndex);
		bool Remove(CMenuItemUI* _pMenuItem);

		CMenuItemUI* GetItemAt(int iIndex) const;
		int GetItemIndex(CMenuItemUI* _pMenuItem) const;
		bool SetItemIndex(CMenuItemUI* _pMenuItem, int iIndex);

	public:
		void CalMenuHeight();
	};
}

#endif // UIMenu_h__
