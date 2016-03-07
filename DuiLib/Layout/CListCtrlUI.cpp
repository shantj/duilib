#include "StdAfx.h"	
#include "CListCtrlUI.h"

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

DuiLib::CListCtrlUI::CListCtrlUI(void) : m_iCurSel(-1), m_bScrollSelect(false)
{
	m_ListInfo.nColumns = 0;
	m_ListInfo.nFont = -1;
	m_ListInfo.uTextStyle = DT_VCENTER;
	m_ListInfo.dwTextColor = 0xFF000000;
	m_ListInfo.dwBkColor = 0;
	m_ListInfo.bAlternateBk = false;
	m_ListInfo.dwSelectedTextColor = 0xFF000000;
	m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
	m_ListInfo.dwHotTextColor = 0xFF000000;
	m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
	m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
	m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
	m_ListInfo.dwLineColor = 0;
	m_ListInfo.bShowHtml = false;
	m_ListInfo.bMultiExpandable = false;
	::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
	::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));

	m_bStartRect = false;
	m_iFirstSelect = -1;
	m_iSelectCount = 0;
}

LPCTSTR CListCtrlUI::GetClass() const
{
	return _T("ListViewUI");
}

LPVOID CListCtrlUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, DUI_CTR_LISTCTRL) == 0 ) return static_cast<CListCtrlUI*>(this);
	if( _tcscmp(pstrName, _T("IListOwner")) == 0 ) return static_cast<IListOwnerUI*>(this);
	return CContainerUI::GetInterface(pstrName);
}

CListHeaderUI* CListCtrlUI::GetHeader() const
{
	return NULL;
}

TListInfoUI* CListCtrlUI::GetListInfo()
{
	return &m_ListInfo;
}

int CListCtrlUI::GetCurSel() const
{
	return m_iCurSel;
}

bool CListCtrlUI::SelectItem(int iIndex, bool bTakeFocus /*= false*/)
{
	BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000);
	// We should first unselect the currently selected item

	return SelectItem(iIndex, bTakeFocus, bCtrl);
}

bool CListCtrlUI::SelectItem(int iIndex, bool bTakeFocus, bool ctrl)
{
	if(!ctrl)
	{
		for(int i=0;i<GetCount();i++)
		{
			if(i!=iIndex)
			{
				CControlUI* pControl = CContainerUI::GetItemAt(i);
				if( pControl != NULL) 
				{
					IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
					if( pListItem == NULL ) 
					{
						continue;
					}
					if (pListItem->IsSelected())
					{
						pListItem->Select(false, false);
					}
				}
			}
		}

		m_iCurSel = -1;
		m_iFirstSelect = -1;
		m_iSelectCount = 0;
	}

	int iOldSel = m_iCurSel;
	if( iIndex < 0 ) return false;

	CControlUI* pControl = CContainerUI::GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	/*if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;*/

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;

	if (pListItem->IsSelected() == false)
	{
		m_iSelectCount--;
		if (m_iSelectCount == 0)
		{
			m_iCurSel = m_iFirstSelect = -1;
		}
		else if (iIndex == m_iCurSel)
		{
			//找到第一个选项
			for (int i=0; i<GetCount(); i++)
			{
				CControlUI* pControl = CContainerUI::GetItemAt(i);
				if( pControl != NULL) {
					IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
					if( pListItem == NULL ) continue;;
					if (pListItem->IsSelected())
					{
						m_iCurSel = m_iFirstSelect = i;
					}
				}
			}
		}
		if (m_pManager)
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMUNSELECT, m_iCurSel, iOldSel);
		}
		return true;
	}

	if( iIndex == m_iCurSel )
	{
		return true;
	}

	m_iCurSel = iIndex;
	m_iFirstSelect = iIndex;
	m_iSelectCount++;

	EnsureVisible(m_iCurSel);
	if( bTakeFocus ) pControl->SetFocus();
	if( m_pManager != NULL ) 
	{
		m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, m_iCurSel, iOldSel);
	}

	return true;
}

void CListCtrlUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pParent != NULL ) m_pParent->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_SETFOCUS ) 
	{
		m_bFocused = true;
		return;
	}
	if( event.Type == UIEVENT_KILLFOCUS ) 
	{
		m_bFocused = false;
		return;
	}
	/*if (event.Type == UIEVENT_BUTTONDOWN)
	{
		SelectItem(-1);
		if( IsEnabled() ){
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
		}
		return;
	}*/
	if ( event.Type == UIEVENT_RBUTTONDOWN )
	{
		SelectItem(-1);
		if( IsEnabled() ){
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMRCLICK);
		}
		return;
	}

	/*********鼠标滑动进行多选**********/
	if (event.Type == UIEVENT_BUTTONDOWN)
	{
		SelectItem(-1);
		if( IsEnabled() ){
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
		}

		m_bStartRect = true;
		m_startPoint = event.ptMouse; //记录开始点
		return;
	}
	if (event.Type == UIEVENT_MOUSEMOVE)
	{
		if(m_bStartRect)
		{
			HWND hwnd = m_pManager->GetPaintWindow();
			RECT rcClient;
			POINT point = event.ptMouse;
			RECT rect = {0};
			rcClient = this->GetPos();

			if(((point.x >= m_startPoint.x-20 && point.x <= m_startPoint.x+20 ) 
				&& (point.y >= m_startPoint.y-20 && point.y <= m_startPoint.y+20)))
				return;
			if ((point.x <= rcClient.left || point.x >= rcClient.right || point.y <= rcClient.top || point.y >= rcClient.bottom))
			{
				return;
			}

			::InvalidateRect(hwnd, &rcClient, FALSE);
			UpdateWindow(hwnd);

			if(point.x<m_startPoint.x && point.y < m_startPoint.y)
			{
				RECT rectTemp = {point.x,point.y,m_startPoint.x,m_startPoint.y};
				rect  = rectTemp;
			}
			else if(point.x>m_startPoint.x && point.y < m_startPoint.y)
			{
				RECT rectTemp = {m_startPoint.x,point.y,point.x,m_startPoint.y};
				rect  = rectTemp;
			}
			else if(point.x<m_startPoint.x && point.y > m_startPoint.y)
			{
				RECT rectTemp = {point.x,m_startPoint.y,m_startPoint.x,point.y};
				rect  = rectTemp;
			}
			else 
			{
				RECT rectTemp = {m_startPoint.x,m_startPoint.y,point.x,point.y};
				rect  = rectTemp;
			}

			UpdateSelectionForRect(rect);
			CRenderEngine::DrawImage(m_pManager->GetPaintDC(), m_pManager, rect, m_rcPaint, GetSelectedFrameImage());
		}
		return;
	}
	if (event.Type == UIEVENT_BUTTONUP)
	{
		m_bStartRect = false;

		HWND hwnd = m_pManager->GetPaintWindow();
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
		return;
	}
	/*********************************/

	switch( event.Type ) {
	case UIEVENT_KEYDOWN:
		switch( event.chKey ) {
	case VK_UP:
		SelectItem(FindSelectable(m_iCurSel - 1, false), true);
		return;
	case VK_DOWN:
		SelectItem(FindSelectable(m_iCurSel + 1, true), true);
		return;
	case VK_PRIOR:
		PageUp();
		return;
	case VK_NEXT:
		PageDown();
		return;
	case VK_HOME:
		SelectItem(FindSelectable(0, false), true);
		return;
	case VK_END:
		SelectItem(FindSelectable(GetCount() - 1, true), true);
		return;
	case VK_RETURN:
		if( m_iCurSel != -1 ) GetItemAt(m_iCurSel)->Activate();
		return;
		}
		break;
	case UIEVENT_SCROLLWHEEL:
		{
			switch( LOWORD(event.wParam) ) {
	case SB_LINEUP:
		if( m_bScrollSelect ) SelectItem(FindSelectable(m_iCurSel - 1, false), true);
		else LineUp();
		return;
	case SB_LINEDOWN:
		if( m_bScrollSelect ) SelectItem(FindSelectable(m_iCurSel + 1, true), true);
		else LineDown();
		return;
			}
		}
		break;
	}
	CControlUI::DoEvent(event);
}

void CListCtrlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("itemsize")) == 0 ) {
		SIZE szItem = { 0 };
		LPTSTR pstr = NULL;
		szItem.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		szItem.cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr);     
		SetItemSize(szItem);
	}
	else if( _tcscmp(pstrName, _T("columns")) == 0 ) SetColumns(_ttoi(pstrValue));
// 	else if( _tcscmp(pstrName, _T("header")) == 0 ) GetHeader()->SetVisible(_tcscmp(pstrValue, _T("hidden")) != 0);
// 	else if( _tcscmp(pstrName, _T("headerbkimage")) == 0 ) GetHeader()->SetBkImage(pstrValue);
	else if( _tcscmp(pstrName, _T("scrollselect")) == 0 ) SetScrollSelect(_tcscmp(pstrValue, _T("true")) == 0);
//	else if( _tcscmp(pstrName, _T("multiexpanding")) == 0 ) SetMultiExpanding(_tcscmp(pstrValue, _T("true")) == 0);
	else if( _tcscmp(pstrName, _T("itemfont")) == 0 ) m_ListInfo.nFont = _ttoi(pstrValue);
	else if( _tcscmp(pstrName, _T("itemalign")) == 0 ) {
		if( _tcsstr(pstrValue, _T("left")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
			m_ListInfo.uTextStyle |= DT_LEFT;
		}
		if( _tcsstr(pstrValue, _T("center")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
			m_ListInfo.uTextStyle |= DT_CENTER;
		}
		if( _tcsstr(pstrValue, _T("right")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
			m_ListInfo.uTextStyle |= DT_RIGHT;
		}
	}
	else if( _tcscmp(pstrName, _T("itemendellipsis")) == 0 ) {
		if( _tcscmp(pstrValue, _T("true")) == 0 ) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
		else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
	}    
 	if( _tcscmp(pstrName, _T("itemtextpadding")) == 0 ) {
 		RECT rcTextPadding = { 0 };
 		LPTSTR pstr = NULL;
 		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
 		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
 		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
 		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
 		SetItemTextPadding(rcTextPadding);
 	}
 	else if( _tcscmp(pstrName, _T("itemtextcolor")) == 0 ) {
 		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
 		LPTSTR pstr = NULL;
 		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
 		SetItemTextColor(clrColor);
 	}
 	else if( _tcscmp(pstrName, _T("itembkcolor")) == 0 ) {
 		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
 		LPTSTR pstr = NULL;
 		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
 		SetItemBkColor(clrColor);
 	}
	else if( _tcscmp(pstrName, _T("itembkimage")) == 0 ) SetItemBkImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemaltbk")) == 0 ) SetAlternateBk(_tcscmp(pstrValue, _T("true")) == 0);
	else if( _tcscmp(pstrName, _T("itemselectedtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetSelectedItemTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemselectedbkcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetSelectedItemBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemselectedimage")) == 0 ) SetSelectedItemImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemhottextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetHotItemTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemhotbkcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetHotItemBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemhotimage")) == 0 ) SetHotItemImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemdisabledtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledItemTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemdisabledbkcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledItemBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemdisabledimage")) == 0 ) SetDisabledItemImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemlinecolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetItemLineColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemshowhtml")) == 0 ) SetItemShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
	else if ( _tcscmp(pstrName, _T("selectframeimage")) == 0 )  SetSelectedFrameImage(pstrValue);
	else CContainerUI::SetAttribute(pstrName, pstrValue);
}

bool CListCtrlUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
	// We also need to recognize header sub-items
	int iOrginIndex = CContainerUI::GetItemIndex(pControl);
	if( iOrginIndex == -1 ) return false;
	if( iOrginIndex == iIndex ) return true;

	IListItemUI* pSelectedListItem = NULL;
	if( m_iCurSel >= 0 ) pSelectedListItem = 
		static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
	IListItemUI* pFirstSelectItem = NULL;
	if (m_iFirstSelect >= 0) pFirstSelectItem = 
		static_cast<IListItemUI*>(GetItemAt(m_iFirstSelect)->GetInterface(_T("ListItem")));
	if( !CContainerUI::SetItemIndex(pControl, iIndex) ) return false;
	int iMinIndex = min(iOrginIndex, iIndex);
	int iMaxIndex = max(iOrginIndex, iIndex);
	for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
		CControlUI* p = CContainerUI::GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) {
			pListItem->SetIndex(i);
		}
	}
	if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
	if( m_iFirstSelect >= 0 && pFirstSelectItem != NULL ) m_iFirstSelect = pFirstSelectItem->GetIndex();
	return true;
}

int CListCtrlUI::GetCount() const
{
	return CContainerUI::GetCount();
}

bool CListCtrlUI::Add(CControlUI* pControl)
{
	if (pControl == NULL)
	{
		return false;
	}

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem != NULL ) {
		pListItem->SetOwner(this);
		pListItem->SetIndex(GetCount());
	}
	return CContainerUI::Add(pControl);
}

bool CListCtrlUI::AddAt(CControlUI* pControl, int iIndex)
{
	if (!CContainerUI::AddAt(pControl, iIndex)) return false;

	// The list items should know about us
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem != NULL ) {
		pListItem->SetOwner(this);
		pListItem->SetIndex(iIndex);
	}

	for(int i = iIndex + 1; i < GetCount(); ++i) {
		CControlUI* p = GetItemAt(i);
		pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) {
			pListItem->SetIndex(i);
		}
	}
	if( m_iCurSel >= iIndex ) m_iCurSel += 1;
	if (m_iFirstSelect >= iIndex) m_iFirstSelect += 1;
	return true;
}

bool CListCtrlUI::Remove(CControlUI* pControl)
{
	int iIndex = CContainerUI::GetItemIndex(pControl);
	if (iIndex == -1) return false;

	if (!CContainerUI::RemoveAt(iIndex)) return false;

	for(int i = iIndex; i < CContainerUI::GetCount(); ++i) {
		CControlUI* p = CContainerUI::GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) {
			pListItem->SetIndex(i);
		}
	}

	if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
		int iSel = m_iCurSel;
		m_iCurSel = -1;
		CControlUI* p = CContainerUI::GetItemAt(FindSelectable(iSel, false));
		if (p != NULL)
		{
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) {
				pListItem->Select();
			}
		}
	}
	else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	return true;
}

bool CListCtrlUI::RemoveAt(int iIndex)
{
	if (!CContainerUI::RemoveAt(iIndex)) return false;

	for(int i = iIndex; i < CContainerUI::GetCount(); ++i) {
		CControlUI* p = CContainerUI::GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) pListItem->SetIndex(i);
	}

	if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
		int iSel = m_iCurSel;
		m_iCurSel = -1;
		CControlUI* p = CContainerUI::GetItemAt(FindSelectable(iSel, false));
		if (p != NULL)
		{
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) {
				pListItem->Select();
			}
		}
	}
	else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	return true;
}

void CListCtrlUI::RemoveAll()
{
	m_iCurSel = -1;
	m_iFirstSelect = -1;
	m_iSelectCount = 0;
	CContainerUI::RemoveAll();
}

void CListCtrlUI::EnsureVisible(int iIndex)
{
	if( m_iCurSel < 0 ) return;
	RECT rcItem = GetItemAt(iIndex)->GetPos();
	RECT rcList = GetPos();
	RECT rcListInset = GetInset();

	rcList.left += rcListInset.left;
	rcList.top += rcListInset.top;
	rcList.right -= rcListInset.right;
	rcList.bottom -= rcListInset.bottom;

	CScrollBarUI* pHorizontalScrollBar = GetHorizontalScrollBar();
	if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

	int iPos = GetScrollPos().cy;
	if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
	int dx = 0;
	if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
	if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
	Scroll(0, dx);
}

void CListCtrlUI::Scroll(int dx, int dy)
{
	if( dx == 0 && dy == 0 ) return;
	SIZE sz = GetScrollPos();
	SetScrollPos(CDuiSize(sz.cx + dx, sz.cy + dy));
}

bool CListCtrlUI::GetScrollSelect()
{
	return m_bScrollSelect;
}

void CListCtrlUI::SetScrollSelect(bool bScrollSelect)
{
	m_bScrollSelect = bScrollSelect;
}

void CListCtrlUI::SetItemTextPadding(RECT rc)
{
	m_ListInfo.rcTextPadding = rc;
	NeedUpdate();
}

void CListCtrlUI::SetItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwTextColor = dwTextColor;
	Invalidate();
}

void CListCtrlUI::SetItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwBkColor = dwBkColor;
	Invalidate();
}

void CListCtrlUI::SetItemBkImage(LPCTSTR pStrImage)
{
	if( m_ListInfo.diBk.sDrawString == pStrImage && m_ListInfo.diBk.pImageInfo != NULL ) return;
	m_ListInfo.diBk.Clear();
	m_ListInfo.diBk.sDrawString = pStrImage;
	Invalidate();
}

bool CListCtrlUI::IsAlternateBk() const
{
	return m_ListInfo.bAlternateBk;
}

void CListCtrlUI::SetAlternateBk(bool bAlternateBk)
{
	m_ListInfo.bAlternateBk = bAlternateBk;
	Invalidate();
}

void CListCtrlUI::SetSelectedItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwSelectedTextColor = dwTextColor;
	Invalidate();
}

void CListCtrlUI::SetSelectedItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwSelectedBkColor = dwBkColor;
	Invalidate();
}

void CListCtrlUI::SetSelectedItemImage(LPCTSTR pStrImage)
{
	if( m_ListInfo.diSelected.sDrawString == pStrImage && m_ListInfo.diSelected.pImageInfo != NULL ) return;
	m_ListInfo.diSelected.Clear();
	m_ListInfo.diSelected.sDrawString = pStrImage;
	Invalidate();
}

void CListCtrlUI::SetHotItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwHotTextColor = dwTextColor;
	Invalidate();
}

void CListCtrlUI::SetHotItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwHotBkColor = dwBkColor;
	Invalidate();
}

void CListCtrlUI::SetHotItemImage(LPCTSTR pStrImage)
{
	if( m_ListInfo.diHot.sDrawString == pStrImage && m_ListInfo.diHot.pImageInfo != NULL ) return;
	m_ListInfo.diHot.Clear();
	m_ListInfo.diHot.sDrawString = pStrImage;
	Invalidate();
}

void CListCtrlUI::SetDisabledItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwDisabledTextColor = dwTextColor;
	Invalidate();
}

void CListCtrlUI::SetDisabledItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwDisabledBkColor = dwBkColor;
	Invalidate();
}

void CListCtrlUI::SetDisabledItemImage(LPCTSTR pStrImage)
{
	if( m_ListInfo.diDisabled.sDrawString == pStrImage && m_ListInfo.diDisabled.pImageInfo != NULL ) return;
	m_ListInfo.diDisabled.Clear();
	m_ListInfo.diDisabled.sDrawString = pStrImage;
	Invalidate();
}

void CListCtrlUI::SetItemLineColor(DWORD dwLineColor)
{
	m_ListInfo.dwLineColor = dwLineColor;
	Invalidate();
}

bool CListCtrlUI::IsItemShowHtml()
{
	return m_ListInfo.bShowHtml;
}

void CListCtrlUI::SetItemShowHtml(bool bShowHtml /*= true*/)
{
	if( m_ListInfo.bShowHtml == bShowHtml ) return;

	m_ListInfo.bShowHtml = bShowHtml;
	NeedUpdate();
}

DWORD CListCtrlUI::GetItemBkColor() const
{
	return m_ListInfo.dwBkColor;
}

void CListCtrlUI::SetSelectedFrameImage(LPCTSTR pStrImage)
{
	if( m_ListInfo.diSelectedFrame.sDrawString == pStrImage && m_ListInfo.diSelectedFrame.pImageInfo != NULL ) return;
	m_ListInfo.diSelectedFrame.Clear();
	m_ListInfo.diSelectedFrame.sDrawString = pStrImage;
	Invalidate();
}

DuiLib::TDrawInfo CListCtrlUI::GetSelectedFrameImage()
{
	return m_ListInfo.diSelectedFrame;
}

bool CListCtrlUI::SelectRange(int iIndex, bool bTakeFocus)
{
	if (m_iCurSel == -1)
	{
		m_iCurSel = 0;
		m_iFirstSelect = 0;
	}

	int i = 0;
	int iFirst = m_iFirstSelect;
	int iLast = iIndex;
	int iCount = GetCount();

	if(iFirst == iLast) return true;
	CControlUI* pControl = CContainerUI::GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	/*if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;*/

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;
	if( !pListItem->Select(true, false) ) {
		m_iCurSel = -1;
		return false;
	}
	m_iSelectCount++;
	//m_iCurSel = iIndex;
	EnsureVisible(iIndex);
	if( bTakeFocus ) pControl->SetFocus();
	if( m_pManager != NULL ) {
		m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECTMUTIL, iIndex, m_iCurSel);
	}
	//locate (and select) either first or last
	// (so order is arbitrary)
	while(i<iCount){
		if(i==iFirst || i == iLast){
			i++;
			break;
		}

		CControlUI* pControl = CContainerUI::GetItemAt(i);
		if( pControl != NULL) {
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL ) continue;
			if (pListItem->IsSelected())
			{
				pListItem->Select(false,false);
				m_iSelectCount--;
			}
		}
		i++;
	}

	// select rest of range
	while(i<iCount){
		CControlUI* pControl = CContainerUI::GetItemAt(i);
		if( pControl != NULL) {
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL ) continue;
			if (pListItem->IsSelected() == false)
			{
				pListItem->Select(true,false);
				m_iSelectCount++;
			}
		}
		if(i==iFirst || i == iLast){
			i++;
			break;
		}
		i++;
	}

	// unselect rest of range
	while(i<iCount){
		CControlUI* pControl = CContainerUI::GetItemAt(i);
		if( pControl != NULL) {
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL ) continue;
			if (pListItem->IsSelected())
			{
				pListItem->Select(false,false);
				m_iSelectCount--;
			}
		}
		i++;
	}

	return true;
}

bool CListCtrlUI::SelectMulti(int iIndex, bool bSelect/*=true*/)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl != NULL) {
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL ) return false;
		if (pListItem->IsSelected() != bSelect)
		{
			pListItem->SelectMulti(bSelect);
			if (bSelect)
			{
				m_iSelectCount++;
				if (m_iCurSel == -1)
				{
					m_iFirstSelect = m_iCurSel = iIndex;
				}
			}
			else
			{
				m_iSelectCount--;
				if (m_iCurSel == iIndex)
				{
					if (m_iSelectCount == 0)
					{
						m_iFirstSelect = m_iCurSel = -1;
					}
					else
					{
						//找到第一个选项
						for (int i=0; i<GetCount(); i++)
						{
							CControlUI* pControl = GetItemAt(i);
							if( pControl != NULL) {
								IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
								if( pListItem == NULL ) continue;;
								if (pListItem->IsSelected())
								{
									m_iCurSel = m_iFirstSelect = i;
								}
							}
						}
					}
				}
			}
		}
	}

	return true;
}

int CListCtrlUI::GetSelectCount()
{
	return m_iSelectCount;
}

void CListCtrlUI::UpdateSelectionForRect(RECT rect)
{
	RECT rcTemp = { 0 };
	for (int iIndex = 0; iIndex<GetCount();iIndex++)
	{
		CControlUI * pControl = GetItemAt(iIndex);
		if( pControl == NULL) { continue;}
		CListCtrlItemElementUI* pListElement = static_cast<CListCtrlItemElementUI*>(pControl);
		RECT rectItem = pListElement->GetPos();

		if( ::IntersectRect(&rcTemp, &rect, &pListElement->GetPos()) )
		{//item in rect
			if (pListElement->IsSelected() == false)
			{
				pListElement->Select(true,false);
				SelectItem(iIndex, false, true);
			}
		}else
		{//item not in rect
			if (pListElement->IsSelected())
			{
				pListElement->Select(false,false);
				SelectItem(iIndex, false, true);
			}
		}
	}
}

CListCtrlItemElementUI::CListCtrlItemElementUI() :
m_iIndex(-1),
m_bSelected(false)
{

}

LPCTSTR CListCtrlItemElementUI::GetClass() const
{
	return _T("ListViewItem");
}

LPVOID CListCtrlItemElementUI::GetInterface(LPCTSTR pstrName)
{
 	if( _tcscmp(pstrName, DUI_CTR_LISTITEM) == 0 ) return static_cast<IListItemUI*>(this);
 	if( _tcscmp(pstrName, DUI_CTR_LISTCONTAINERELEMENT) == 0 ) return static_cast<CListContainerElementUI*>(this);
	if( _tcscmp(pstrName, DUI_CTR_LISTCTRLITEM) == 0) return static_cast<CListCtrlItemElementUI*>(this);
	return CContainerUI::GetInterface(pstrName);
}

int CListCtrlItemElementUI::GetIndex() const
{
	return m_iIndex;
}

void CListCtrlItemElementUI::SetIndex(int iIndex)
{
	m_iIndex = iIndex;
}

IListOwnerUI* CListCtrlItemElementUI::GetOwner()
{
	return m_pOwner;
}

void CListCtrlItemElementUI::SetOwner(CControlUI* pOwner)
{
	m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(_T("IListOwner")));
}


void CListCtrlItemElementUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_DBLCLICK )
	{
		if( IsEnabled() ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMDBCLICK);
			Activate();
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_KEYDOWN && IsEnabled() )
	{
		if( event.chKey == VK_RETURN ) {
			Activate();
			Invalidate();
			return;
		}
	}
	if( event.Type == UIEVENT_BUTTONDOWN )
	{
		if( IsEnabled() ){
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
			Select();
			Invalidate();
		}
		return;
	}
	if ( event.Type == UIEVENT_RBUTTONDOWN )
	{
		if( IsEnabled() ){
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMRCLICK);
			Select(true, true, true);
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP ) 
	{
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if( IsEnabled() ) {
			m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}

	// An important twist: The list-item will send the event not to its immediate
	// parent but to the "attached" list. A list may actually embed several components
	// in its path to the item, but key-presses etc. needs to go to the actual list.
	if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CListCtrlItemElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
	DrawItemBk(hDC, m_rcItem);
	CContainerUI::DoPaint(hDC, rcPaint);
}

void CListCtrlItemElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
	return;
}

void CListCtrlItemElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
{
	ASSERT(m_pOwner);
	if( m_pOwner == NULL ) return;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	if( pInfo == NULL ) return;
	DWORD iBackColor = 0;
	if( !pInfo->bAlternateBk || m_iIndex % 2 == 0 ) iBackColor = pInfo->dwBkColor;

	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		iBackColor = pInfo->dwHotBkColor;
	}
	if( IsSelected() ) {
		iBackColor = pInfo->dwSelectedBkColor;
	}
	if( !IsEnabled() ) {
		iBackColor = pInfo->dwDisabledBkColor;
	}
	if ( iBackColor != 0 ) {
		CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(iBackColor));
	}

	if( !IsEnabled() ) {
		if( DrawImage(hDC, pInfo->diDisabled) ) return;
	}
	if( IsSelected() ) {
		if( DrawImage(hDC, pInfo->diSelected) ) return;
	}
	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( DrawImage(hDC, pInfo->diHot) ) return;
	}
	if( !DrawImage(hDC, m_diBk) ) {
		if( !pInfo->bAlternateBk || m_iIndex % 2 == 0 ) {
			if( DrawImage(hDC, pInfo->diBk) ) return;
		}
	}

	if ( pInfo->dwLineColor != 0 ) {
		RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
	}
}

bool CListCtrlItemElementUI::IsSelected() const
{
	 return m_bSelected;
}

bool CListCtrlItemElementUI::Select(bool bSelect /*= true*/, bool bCallBack/*=true*/, bool bRclick)
{
	BOOL bShift = (GetKeyState(VK_SHIFT) & 0x8000);
	BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000);
	if( !IsEnabled() ) return false;

	bool bOldSelected = m_bSelected;

	if(bCtrl && !bRclick)
	{
		m_bSelected = !m_bSelected;
	}else
	{
		m_bSelected = bSelect;
	}

	if(bSelect && bCallBack && m_pOwner != NULL ) 
	{
		if (!bRclick || (bRclick && !bOldSelected))
		{
			if(bShift)
			{
				m_pOwner->SelectRange(m_iIndex);
			}else
			{
				m_pOwner->SelectItem(m_iIndex);
			}
		}
	}
	Invalidate();

	return true;
}

bool CListCtrlItemElementUI::SelectMulti(bool bSelect/*=true*/)
{
	m_bSelected = bSelect;
	Invalidate();

	return true;
}

void CListCtrlItemElementUI::SetPos(RECT rc, bool bNeedInvalidate /* = true */)
{
	CContainerUI::SetPos(rc, bNeedInvalidate);  
	if( m_pOwner == NULL ) return;

	CListCtrlUI* pListUI = static_cast<CListCtrlUI*>(m_pOwner);
	if (pListUI == NULL)	return;

	CListHeaderUI* listHeader = pListUI->GetHeader();
	if (listHeader == NULL) return;

	TListInfoUI* pInfo = m_pOwner->GetListInfo();  
	int nCount = m_items.GetSize();  
	for (int i = 0; i < nCount; i++)  
	{  
		CControlUI *pHorizontalLayout = static_cast<CControlUI*>(m_items[i]);  

		CListHeaderItemUI *pHeaderItem = static_cast<CListHeaderItemUI*>(listHeader->GetItemAt(i));  
		if (pHorizontalLayout != NULL && pHeaderItem != NULL)  
		{  
			RECT rtHeader = pHeaderItem->GetPos();  
			RECT rt = pHorizontalLayout->GetPos();  
			rt.left = rtHeader.left;  
			rt.right = rtHeader.right;  
			pHorizontalLayout->SetPos(rt);  
		}  
	}  
}

} // namespace DuiLib
