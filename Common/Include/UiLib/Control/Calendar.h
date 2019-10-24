#ifndef Calendar_h__
#define Calendar_h__

#include <map>
#pragma once

namespace UiLib
{
	typedef struct tag_CalendarStyle
	{
		CDuiString	nCalenderBorderColor;		//日历边框颜色
		CDuiString	nCalendarBkColor;			//日历背景颜色
		CDuiString	nMainTitleBkColor;			//日历主标题背景颜色
		CDuiString	nSubTitleBkColor;			//日历副标题背景颜色
		CDuiString	nWeekendColorA;				//周末的日期隔行背景颜色
		CDuiString	nWeekendColorB;				//周末的日期隔行背景颜色
		CDuiString	nToDayColor;				//日期当前天的背景颜色
		CDuiString	nDayHotColor;				//日期获得焦点时背景颜色
		CDuiString	nDayPushedColor;			//日期被按下时背景颜色
		CDuiString	nDaySelectColor;			//日期被选中时背景颜色
		CDuiString	nDayDisabledColor;			//日期被禁用时的背景色
		CDuiString	nNoCurMonthDayColor;		//非本月日期的背景颜色
		CDuiString	nWeekIntervalColorA;		//周隔行颜色A
		CDuiString	nWeekIntervalColorB;		//周隔行颜色B
	}TCalendarStyle;
	
	typedef struct tag_SubTitleString
	{
		CDuiString	nSundayStr;
		CDuiString	nMondayStr;
		CDuiString	nTuesdayStr;
		CDuiString	nWednesdayStr;
		CDuiString	nThursdayStr;
		CDuiString	nFridayStr;
		CDuiString	nSaturdayStr;
	}TSubTitleString;

	class CCalendar : public CHorizontalLayoutUI
	{
	public:
		CCalendar(void);
		~CCalendar(void);

	public:
		bool AddSubTitleString(LPCTSTR _Name,TSubTitleString& _SubTitleString);
		bool AddSubTitleString(LPCTSTR _Name,LPCTSTR _Sunday,LPCTSTR _Monday,LPCTSTR _Tuesday,LPCTSTR _Wednesday,LPCTSTR _Thursday,LPCTSTR _Friday,LPCTSTR _Saturday);
		TSubTitleString& GetSubTitleString(LPCTSTR _Name = NULL);
		bool RemoveAtSubTitleString(LPCTSTR _Name);
	public:
		TCalendarStyle	m_DefaultStyle;

	private:
		CHorizontalLayoutUI*	pMainTitleHoriz;
		CHorizontalLayoutUI*	pSubTitleHoriz;
		CVerticalLayoutUI*		pDayPanelVert;

		struct tm*				pCurDateTime;

		TSubTitleString			mSubTitleString;
		CStdStringPtrMap		mSubTitleStringArray;
	};
}



#endif // Calendar_h__
