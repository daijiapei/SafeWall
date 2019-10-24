#ifndef _DYNAMIC
#define UILIB_API
#else
#if defined(UILIB_EXPORTS)
#if defined(_MSC_VER)
#define UILIB_API __declspec(dllexport)
#else
#define UILIB_API 
#endif
#else
#if defined(_MSC_VER)
#define UILIB_API __declspec(dllimport)
#else
#define UILIB_API 
#endif
#endif
#endif

#define UILIB_COMDAT __declspec(selectany) 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stddef.h>
#include <richedit.h>
#include <tchar.h>
#include <assert.h>
#include <crtdbg.h>
#include <malloc.h>

#pragma comment(lib,"oledlg.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"Riched20.lib")
#pragma comment(lib,"gdiplus.lib")

#include "Utils/Utils.h"
#include "Utils/Internal.h"
#include "Utils/UIDelegate.h"
#include "Core/UIBase.h"
#include "Core/UIManager.h"
#include "Core/UIDxAnimation.h"

#include "Core/UIDefine.h"
#include "Core/UIControl.h"
#include "Core/UIContainer.h"

#include "Layout/UIVerticalLayout.h"
#include "Layout/UIHorizontalLayout.h"
#include "Layout/UITileLayout.h"
#include "Layout/UITabLayout.h"
#include "Layout/UIChildLayout.h"

#include "Control/UIList.h"
#include "Control/UICombo.h"
#include "Control/UIScrollBar.h"
#include "Control/UITreeView.h"

#include "Control/UILabel.h"
#include "Control/UIText.h"
#include "Control/UIEdit.h"
#include "Control/UIGifAnim.h"

#include <algorithm>
#include "Control/UIAnimation.h"
#include "Control/UIFadeButton.h"
#include "Control/UIButton.h"
#include "Control/UIOption.h"
#include "Control/UICheckBox.h"

#include "Control/UIProgress.h"
#include "Control/UISlider.h"

#include "Control/UIComboBox.h"
#include "Control/UIRichEdit.h"
#include "Control/UICalendar.h"

#include "Control/UIActiveX.h"
#include "Control/UIWebBrowser.h"

#include "Core/UIMarkup.h"
#include "Core/UIDlgBuilder.h"
#include "Core/UIRender.h"

