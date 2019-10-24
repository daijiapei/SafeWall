#ifndef UIDateTime_h__
#define UIDateTime_h__

#pragma once

namespace UiLib
{
	class CDateTimeWnd;

	class CDateTimeUI : public CCalendarUI
	{
		friend class CDateTimeWnd;
	public:
		CDateTimeUI(void);
		~CDateTimeUI(void);

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName); 

	protected:
		CDateTimeWnd* m_pWindow;
	};
}


#endif // UIDateTime_h__
