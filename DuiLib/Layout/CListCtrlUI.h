#ifndef __UILISTVIEW_H__
#define __UILISTVIEW_H__

#pragma once
#include "..\Control\UIList.h"

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//

class UILIB_API CListCtrlUI : public CTileLayoutUI, public IListOwnerUI
{
public:
	CListCtrlUI(void);

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	CListHeaderUI* GetHeader() const;
	TListInfoUI* GetListInfo();
	int GetCurSel() const;
	bool SelectItem(int iIndex, bool bTakeFocus = false, bool bTriggerEvent=true);
	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	bool SetItemIndex(CControlUI* pControl, int iIndex);
	int GetCount() const;
	bool Add(CControlUI* pControl);
	bool AddAt(CControlUI* pControl, int iIndex);
	bool Remove(CControlUI* pControl);
	bool RemoveAt(int iIndex);
	void RemoveAll();

	void EnsureVisible(int iIndex);
	void Scroll(int dx, int dy);
	bool GetScrollSelect();
	void SetScrollSelect(bool bScrollSelect);

// 	void SetItemFont(int index);
// 	void SetItemTextStyle(UINT uStyle);
 	void SetItemTextPadding(RECT rc);
 	void SetItemTextColor(DWORD dwTextColor);
 	void SetItemBkColor(DWORD dwBkColor);
 	void SetItemBkImage(LPCTSTR pStrImage);
 	bool IsAlternateBk() const;
 	void SetAlternateBk(bool bAlternateBk);
 	void SetSelectedItemTextColor(DWORD dwTextColor);
 	void SetSelectedItemBkColor(DWORD dwBkColor);
 	void SetSelectedItemImage(LPCTSTR pStrImage); 
 	void SetHotItemTextColor(DWORD dwTextColor);
 	void SetHotItemBkColor(DWORD dwBkColor);
 	void SetHotItemImage(LPCTSTR pStrImage);
 	void SetDisabledItemTextColor(DWORD dwTextColor);
 	void SetDisabledItemBkColor(DWORD dwBkColor);
 	void SetDisabledItemImage(LPCTSTR pStrImage);
 	void SetItemLineColor(DWORD dwLineColor);
 	bool IsItemShowHtml();
 	void SetItemShowHtml(bool bShowHtml = true);
// 	RECT GetItemTextPadding() const;
// 	DWORD GetItemTextColor() const;
 	DWORD GetItemBkColor() const;
// 	LPCTSTR GetItemBkImage() const;
// 	DWORD GetSelectedItemTextColor() const;
// 	DWORD GetSelectedItemBkColor() const;
 	//LPCTSTR GetSelectedItemImage() const;
// 	DWORD GetHotItemTextColor() const;
// 	DWORD GetHotItemBkColor() const;
// 	LPCTSTR GetHotItemImage() const;
// 	DWORD GetDisabledItemTextColor() const;
// 	DWORD GetDisabledItemBkColor() const;
// 	LPCTSTR GetDisabledItemImage() const;
// 	DWORD GetItemLineColor() const;
	void SetSelectedFrameImage(LPCTSTR pStrImage);
	TDrawInfo GetSelectedFrameImage();

	bool SelectRange(int iIndex, bool bTakeFocus = false, bool bTriggerEvent=true);
	bool SelectMulti(int iIndex, bool bSelect=true);
	int GetSelectCount();

private:
	void UpdateSelectionForRect(RECT rect);
	bool SelectItem(int iIndex, bool bTakeFocus, bool ctrl, bool bTriggerEvent=true);

protected:
	int m_iCurSel;
	TListInfoUI m_ListInfo;
	bool m_bScrollSelect;

	bool m_bStartRect;
	POINT m_startPoint;

	int m_iFirstSelect;
	int m_iSelectCount;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListCtrlItemElementUI : public CListContainerElementUI
{
public:
	CListCtrlItemElementUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	int GetIndex() const;
	void SetIndex(int iIndex);

	IListOwnerUI* GetOwner();
	void SetOwner(CControlUI* pOwner);

	void DoEvent(TEventUI& event);
	void DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl);

	void DrawItemText(HDC hDC, const RECT& rcItem);    
	void DrawItemBk(HDC hDC, const RECT& rcItem);

	bool IsSelected() const;
	bool Select(bool bSelect = true, bool bTriggerEvent = true, bool bCallBack=true, bool bRclick = false);
	bool SelectMulti(bool bSelect=true);

	void SetPos(RECT rc, bool bNeedInvalidate /* = true */);

protected:
	int m_iIndex;
	bool m_bSelected;
};

}// namespace DuiLib

#endif