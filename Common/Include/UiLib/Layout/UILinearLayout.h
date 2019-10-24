#ifndef __UI_LINEAR_LAYOUT_H__
#define __UI_LINEAR_LAYOUT_H__

#pragma once

namespace UiLib {
	class UILIB_API CLinearLayoutUI : public CContainerUI
	{
	public:
		CLinearLayoutUI();
		virtual ~CLinearLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetPos(RECT rc);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	protected:
		RECT doGravityLayout(const CLayoutGravity &stGravity, const SIZE &stItem, bool isVertical, RECT &stRect);

	private:
		CLayoutGravity m_stGravity;
		DWORD m_bVertical;
	};

} // namespace UiLib

#endif // __UI_LINEAR_LAYOUT_H__
