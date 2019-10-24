#ifndef ListCommonDefine_h__
#define ListCommonDefine_h__

#include <math.h>
#include "../Utils/IWindowBase.h"

namespace UiLib
{
	typedef std::basic_string<TCHAR> tString;

	struct NodeData
	{
		int level_;
		bool folder_;
		bool child_visible_;
		bool has_child_;
		tString text_;
		tString value;
		CListContainerElementUI* list_elment_;
	};

	double CalculateDelay(double state);

	class Node
	{
	public:
		Node();
		explicit Node(NodeData t);
		Node(NodeData t, Node* parent);
		~Node();
		NodeData& data();
		int num_children() const;
		Node* child(int i);
		Node* parent();
		bool folder() const;
		bool has_children() const;
		void add_child(Node* child);
		void remove_child(Node* child);
		Node* get_last_child();

	private:
		void set_parent(Node* parent);

	private:
		typedef std::vector <Node*>	Children;

		Children	children_;
		Node*		parent_;

		NodeData    data_;
	};

	struct ItemData
	{
		bool folder;
		bool empty;
		tString mName;
		tString mIcon;
	};

	class CTreeListWnd : public CListUI,public INotifyUI
	{
	public:
		enum {SCROLL_TIMERID = 10};

	public:
		CTreeListWnd(CPaintManagerUI& paint_manager);
		~CTreeListWnd(void);

	public:
		static bool OnLogoButtonEvent(void* event);

	public:
		virtual bool Add(CControlUI* pControl);
		virtual bool AddAt(CControlUI* pControl, int iIndex);
		virtual bool Remove(CControlUI* pControl);
		virtual bool RemoveAt(int iIndex);
		virtual void RemoveAll();
		virtual void DoEvent(TEventUI& event);
		virtual Node* GetRoot();
		virtual Node* AddNode(const ItemData& item, Node* parent = NULL){return NULL;};
		virtual bool RemoveNode(Node* node);
		virtual void SetChildVisible(Node* node, bool visible);
		virtual bool CanExpand(Node* node) const;
		virtual bool SelectItem(int iIndex, bool bTakeFocus = false);
		virtual bool ItemHot(int iIndex);
		virtual bool ItemNon( int iIndex );
		virtual void OnSelect(CControlUI* pControl,bool isSelect){};

		virtual void Notify(TNotifyUI& msg);
		virtual void IsChildVisible(CControlUI* control,bool visible);

	public:
		Node*	root_node_;
		LONG	delay_deltaY_;
		DWORD	delay_number_;
		DWORD	delay_left_;
		CDuiRect	text_padding_;
		int level_text_start_pos_;
		CPaintManagerUI& paint_manager_;

		CDialogBuilder m_dlgBuilder;


		int ListItemNormalHeight;
		int ListItemSelectedHeight;
		tString kOperatorPannelControlName;
		tString kIconPanelControlName;

		tString mItemText;
		tString mExpandIcon;
		tString mCollapseIcon;
	};
}
#endif // ListCommonDefine_h__
