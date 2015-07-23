#ifndef __UIMENU_H__
#define __UIMENU_H__

#pragma once
#include <map>

namespace DuiLib 
{

#define WM_MENUCLICK WM_USER + 121  //�������հ�ť��������Ϣ

	struct ContextMenuParam
	{
		// 1: remove all
		// 2: remove the sub menu
		WPARAM wParam;
		HWND hWnd;
	};

	enum MenuAlignment
	{
		eMenuAlignment_Left = 1 << 1,
		eMenuAlignment_Top = 1 << 2,
		eMenuAlignment_Right = 1 << 3,
		eMenuAlignment_Bottom = 1 << 4,
	};

	enum MenuItemDefaultInfo
	{
		ITEM_DEFAULT_HEIGHT = 30,		//ÿһ��item��Ĭ�ϸ߶ȣ�ֻ����״����ʱ�Զ��壩
		ITEM_DEFAULT_WIDTH = 150,		//���ڵ�Ĭ�Ͽ��

		ITEM_DEFAULT_ICON_WIDTH = 26,	//Ĭ��ͼ����ռ���
		ITEM_DEFAULT_ICON_SIZE = 16,	//Ĭ��ͼ��Ĵ�С

		ITEM_DEFAULT_EXPLAND_ICON_WIDTH = 20,	//Ĭ���¼��˵���չͼ����ռ���
		ITEM_DEFAULT_EXPLAND_ICON_SIZE = 9,		//Ĭ���¼��˵���չͼ��Ĵ�С

		DEFAULT_LINE_LEFT_INSET = ITEM_DEFAULT_ICON_WIDTH + 3,	//Ĭ�Ϸָ��ߵ���߾�
		DEFAULT_LINE_RIGHT_INSET = 7,	//Ĭ�Ϸָ��ߵ��ұ߾�
		DEFAULT_LINE_HEIGHT = 6,		//Ĭ�Ϸָ�����ռ�߶�
		DEFAULT_LINE_COLOR = 0xFFBCBFC4	//Ĭ�Ϸָ�����ɫ

	};

	///////////////////////////////////////////////
	class ReceiverBase;
	class ObserverBase
	{
	public:
		virtual void AddReceiver(ReceiverBase* receiver) = 0;
		virtual void RemoveReceiver(ReceiverBase* receiver) = 0;
		virtual BOOL RBroadcast(ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////
	class ReceiverBase
	{
	public:
		virtual void AddObserver(ObserverBase* observer) = 0;
		virtual void RemoveObserver() = 0;
		virtual BOOL Receive(ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////

template <typename ReturnT, typename ParamT>
class Receiver;
class Observer : public ObserverBase
{
	friend class Iterator;
public:
	Observer()
	{
		m_pMainWndPaintManager = NULL;
		m_pMenuCheckInfo = NULL;
	}

	virtual ~Observer()	{}

	virtual void AddReceiver(ReceiverBase* receiver)
	{
		if (receiver == NULL)
			return;

		receivers_.push_back(receiver);
		receiver->AddObserver(this);
	}

	virtual void RemoveReceiver(ReceiverBase* receiver)
	{
		if (receiver == NULL)
			return;

		ReceiversVector::iterator it = receivers_.begin();
		for (; it != receivers_.end(); ++it)
		{
			if (*it == receiver)
			{
				receivers_.erase(it);
				break;
			}
		}
	}

	virtual BOOL RBroadcast(ContextMenuParam param)
	{
		ReceiversVector::reverse_iterator it = receivers_.rbegin();
		for (; it != receivers_.rend(); ++it)
		{
			(*it)->Receive(param);
		}

		return BOOL();
	}

	class Iterator
	{
		Observer & _tbl;
		DWORD index;
		ReceiverBase* ptr;
	public:
		Iterator( Observer & table )
			: _tbl( table ), index(0), ptr(NULL)
		{}

		Iterator( const Iterator & v )
			: _tbl( v._tbl ), index(v.index), ptr(v.ptr)
		{}

		ReceiverBase* next()
		{
			if ( index >= _tbl.receivers_.size() )
				return NULL;

			for ( ; index < _tbl.receivers_.size(); )
			{
				ptr = _tbl.receivers_[ index++ ];
				if ( ptr )
					return ptr;
			}
			return NULL;
		}
	};

	virtual void SetManger(CPaintManagerUI* pManager)
	{
		if (pManager != NULL)
			m_pMainWndPaintManager = pManager;
	}

	virtual CPaintManagerUI* GetManager() const
	{
		return m_pMainWndPaintManager;
	}

	virtual void SetMenuCheckInfo(map<CDuiString,bool>* pInfo)
	{
		if (pInfo != NULL)
			m_pMenuCheckInfo = pInfo;
		else
			m_pMenuCheckInfo = NULL;
	}

	virtual map<CDuiString,bool>* GetMenuCheckInfo() const
	{
		return m_pMenuCheckInfo;
	}

protected:
	typedef std::vector<ReceiverBase*> ReceiversVector;
	ReceiversVector receivers_;

	CPaintManagerUI* m_pMainWndPaintManager;
	map<CDuiString,bool>* m_pMenuCheckInfo;
};

////////////////////////////////////////////////////
template <typename ReturnT, typename ParamT>
class Receiver : public ReceiverBase
{
public:
	Receiver(){}

	virtual ~Receiver()	{}

	virtual void AddObserver(ObserverBase* observer)
	{
		observers_.push_back(observer);
	}

	virtual void RemoveObserver()
	{
		ObserversVector::iterator it = observers_.begin();
		for (; it != observers_.end(); ++it)
		{
			(*it)->RemoveReceiver(this);
		}
	}

	virtual BOOL Receive(ContextMenuParam param)
	{
		return BOOL();
	}

protected:
	typedef std::vector<ObserverBase*> ObserversVector;
	ObserversVector observers_;
};

/////////////////////////////////////////////////////////////////////////////////////
//

typedef class Observer ContextMenuObserver;
typedef class Receiver<BOOL, ContextMenuParam> ContextMenuReceiver;

extern ContextMenuObserver s_context_menu_observer;

class CListUI;
class UILIB_API CMenuUI : public CListUI
{
public:
	CMenuUI();
	~CMenuUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	virtual void DoEvent(TEventUI& event);

    virtual bool Add(CControlUI* pControl);
    virtual bool AddAt(CControlUI* pControl, int iIndex);

    virtual int GetItemIndex(CControlUI* pControl) const;
    virtual bool SetItemIndex(CControlUI* pControl, int iIndex);
    virtual bool Remove(CControlUI* pControl);

	SIZE EstimateSize(SIZE szAvailable);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CMenuElementUI;
class UILIB_API CMenuWnd : public CWindowWnd, public ContextMenuReceiver, public IDialogBuilderCallback
{
public:
	CMenuWnd();
	~CMenuWnd();

	/*
	 *	@pOwner һ���˵���Ҫָ��������������ǲ˵��ڲ�ʹ�õ�
	 *	@xml	�˵��Ĳ����ļ�
	 *	@point	�˵������Ͻ�����
	 *	@pMainPaintManager	�˵��ĸ����������ָ��
	 *	@xml	����˵��ĵ�ѡ�͸�ѡ��Ϣ�ṹָ��
	 *	@dwAlignment		�˵��ĳ���λ�ã�Ĭ�ϳ������������²ࡣ
	 */
	void Init(CMenuElementUI* pOwner, STRINGorID xml, POINT point,
		CPaintManagerUI* pMainPaintManager, map<CDuiString,bool>* pMenuCheckInfo = NULL,
		DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top);
    LPCTSTR GetWindowClassName() const;
    void OnFinalMessage(HWND hWnd);
	void Notify(TNotifyUI& msg);
	CControlUI* CreateControl(LPCTSTR pstrClassName);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL Receive(ContextMenuParam param);

	// ��ȡ���˵��ؼ������ڶ�̬����Ӳ˵�
	CMenuUI* GetMenuUI();

	// ���µ����˵��Ĵ�С
	void ResizeMenu();

	// ���µ����Ӳ˵��Ĵ�С
	void ResizeSubMenu();

public:
	POINT m_BasedPoint;
	STRINGorID m_xml;
	CDuiString m_sType;
    CPaintManagerUI m_pm;
    CMenuElementUI* m_pOwner;
    CMenuUI* m_pLayout;
	DWORD		m_dwAlignment;	//�˵����뷽ʽ
};

class CListContainerElementUI;
class UILIB_API CMenuElementUI : public CListContainerElementUI
{
	friend CMenuWnd;
public:
    CMenuElementUI();
	~CMenuElementUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);
    void DoPaint(HDC hDC, const RECT& rcPaint);
	void DrawItemText(HDC hDC, const RECT& rcItem);
	SIZE EstimateSize(SIZE szAvailable);
	void DoEvent(TEventUI& event);
	CMenuWnd* GetMenuWnd();
	void CreateMenuWnd();

	void SetLineType();
	void SetLineColor(DWORD color);
	DWORD GetLineColor() const;
	void SetLinePadding(RECT rcInset);
	RECT GetLinePadding() const;
	void SetIcon(LPCTSTR strIcon);
	void SetIconSize(LONG cx, LONG cy);
	void DrawItemIcon(HDC hDC, const RECT& rcItem);
	void SetChecked(bool bCheck = true);
	bool GetChecked() const;
	void SetCheckItem(bool bCheckItem = false);
	bool GetCheckItem() const;

	void SetShowExplandIcon(bool bShow);
	void DrawItemExpland(HDC hDC, const RECT& rcItem);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

protected:
	CMenuWnd* m_pWindow;

	bool	m_bDrawLine;	//���ָ���
	DWORD		m_dwLineColor;  //�ָ�����ɫ
	RECT		m_rcLinePadding;	//�ָ��ߵ����ұ߾�

	SIZE		m_szIconSize; 	//��ͼ��
	CDuiString	m_strIcon;
	bool		m_bCheckItem;
	bool		m_bShowExplandIcon;
};

} // namespace DuiLib

#endif // __UIMENU_H__
