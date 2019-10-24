#ifndef __UI_FRAME_LAYOUT_H__
#define __UI_FRAME_LAYOUT_H__

#pragma once

namespace UiLib {
	class UILIB_API CFrameLayoutUI : public CContainerUI
	{
	public:
		CFrameLayoutUI();
		virtual ~CFrameLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetPos(RECT rc);
	};

} // namespace UiLib

#endif // __UI_FRAME_LAYOUT_H__
