#include "stdafx.h"
#include "UIDateTime.h"

namespace DuiLib
{
	//CDateTimeUI::m_nDTUpdateFlag
#define DT_NONE   0
#define DT_UPDATE 1
#define DT_DELETE 2
#define DT_KEEP   3

	class CDateTimeWnd : public CWindowWnd
	{
	public:
		CDateTimeWnd();

		void Init(CDateTimeUI* pOwner);
		RECT CalPos();

		LPCTSTR GetWindowClassName() const;
		LPCTSTR GetSuperClassName() const;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	protected:
		CDateTimeUI* m_pOwner;
		HBRUSH m_hBkBrush;
		bool m_bInit;
	};

	CDateTimeWnd::CDateTimeWnd() : m_pOwner(NULL), m_hBkBrush(NULL), m_bInit(false)
	{
	}

	void CDateTimeWnd::Init(CDateTimeUI* pOwner)
	{
		m_pOwner = pOwner;
		m_pOwner->m_nDTUpdateFlag = DT_NONE;

		if (m_hWnd == NULL)
		{
			RECT rcPos = CalPos();
			UINT uStyle = m_pOwner->GetTimeStyle();
			Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
			SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);
		}

		/*if (m_pOwner->GetText().IsEmpty())
		{
			if (m_pOwner->m_bSetRange)
			{
				GetLocalTime(&m_pOwner->m_sysTime);
				COleDateTime mintime, systime;
				mintime.SetDateTime(m_pOwner->m_pMinTime.wYear, m_pOwner->m_pMinTime.wMonth, m_pOwner->m_pMinTime.wDay, 
					m_pOwner->m_pMinTime.wHour, m_pOwner->m_pMinTime.wMinute, m_pOwner->m_pMinTime.wSecond);
				systime.SetDateTime(m_pOwner->m_sysTime.wYear, m_pOwner->m_sysTime.wMonth, m_pOwner->m_sysTime.wDay, 
					m_pOwner->m_sysTime.wHour, m_pOwner->m_sysTime.wMinute, m_pOwner->m_sysTime.wSecond);
				if (systime < mintime)
				{
					m_pOwner->m_sysTime = m_pOwner->m_pMinTime;
				}
			}
		}*/

		::SendMessage(m_hWnd, DTM_SETFORMAT, 0, (LPARAM)m_pOwner->GetFormatString().GetData());
		::SendMessage(m_hWnd, DTM_SETSYSTEMTIME, 0, (LPARAM)&m_pOwner->m_sysTime);

		//设置时间范围
		if (m_pOwner->m_bSetRange)
		{
			SYSTEMTIME sysTimes[2];
			sysTimes[0] = m_pOwner->m_pMinTime;
			sysTimes[1] = m_pOwner->m_pMaxTime;
			::SendMessage(m_hWnd, DTM_SETRANGE, GDTR_MIN|GDTR_MAX, (LPARAM)sysTimes);
			//::SendMessage(m_hWnd, MCS_NOTODAY, 0, 1);
		}
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		::SetFocus(m_hWnd);

		m_bInit = true;    
	}

	RECT CDateTimeWnd::CalPos()
	{
		CDuiRect rcPos = m_pOwner->GetPos();

		CControlUI* pParent = m_pOwner;
		RECT rcParent;
		while( pParent = pParent->GetParent() ) {
			if( !pParent->IsVisible() ) {
				rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
				break;
			}
			rcParent = pParent->GetClientPos();
			if( !::IntersectRect(&rcPos, &rcPos, &rcParent) ) {
				rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
				break;
			}
		}

		return rcPos;
	}

	LPCTSTR CDateTimeWnd::GetWindowClassName() const
	{
		return _T("DateTimeWnd");
	}

	LPCTSTR CDateTimeWnd::GetSuperClassName() const
	{
		return DATETIMEPICK_CLASS;
	}

	void CDateTimeWnd::OnFinalMessage(HWND hWnd)
	{
		// Clear reference and die
		if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
		m_pOwner->GetManager()->RemoveNativeWindow(hWnd);
		m_pOwner->m_pWindow = NULL;
		delete this;
	}

	LRESULT CDateTimeWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if( uMsg == WM_CREATE ) {
			m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
			bHandled = FALSE;
		}
		else if( uMsg == WM_KILLFOCUS )
		{
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		}
		else if (uMsg == WM_KEYUP && (wParam == VK_DELETE || wParam == VK_BACK))
		{
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			m_pOwner->m_nDTUpdateFlag = DT_DELETE;
			m_pOwner->UpdateText();
			PostMessage(WM_CLOSE);
			return lRes;
		}
		else if (uMsg == WM_KEYUP && wParam == VK_ESCAPE)
		{
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			m_pOwner->m_nDTUpdateFlag = DT_KEEP;
			PostMessage(WM_CLOSE);
			return lRes;
		}
		//	else if( uMsg == OCM_COMMAND ) {
		// 		if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
		// 		else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
		// 			RECT rcClient;
		// 			::GetClientRect(m_hWnd, &rcClient);
		// 			::InvalidateRect(m_hWnd, &rcClient, FALSE);
		// 		}
		//	}
		//	else if( uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_RETURN ) {
		// 		m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_RETURN);
		//	}
		// 		else if( uMsg == OCM__BASE + WM_CTLCOLOREDIT  || uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) {
		// 			if( m_pOwner->GetNativeEditBkColor() == 0xFFFFFFFF ) return NULL;
		// 			::SetBkMode((HDC)wParam, TRANSPARENT);
		// 			DWORD dwTextColor = m_pOwner->GetTextColor();
		// 			::SetTextColor((HDC)wParam, RGB(GetBValue(dwTextColor),GetGValue(dwTextColor),GetRValue(dwTextColor)));
		// 			if( m_hBkBrush == NULL ) {
		// 				DWORD clrColor = m_pOwner->GetNativeEditBkColor();
		// 				m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
		// 			}
		// 			return (LRESULT)m_hBkBrush;
		// 		}
		else if( uMsg == WM_PAINT) {
			if (m_pOwner->GetManager()->IsLayered()) {
				m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
			}
			bHandled = FALSE;
		}
		else bHandled = FALSE;
		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CDateTimeWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		if (m_pOwner->m_nDTUpdateFlag == DT_NONE)
		{
			::SendMessage(m_hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&m_pOwner->m_sysTime);
			m_pOwner->m_nDTUpdateFlag = DT_UPDATE;
			m_pOwner->UpdateText();
		}
		if ((HWND)wParam != m_pOwner->GetManager()->GetPaintWindow()) {
			::SendMessage(m_pOwner->GetManager()->GetPaintWindow(), WM_KILLFOCUS, wParam, lParam);
		}
		SendMessage(WM_CLOSE);
		return lRes;
	}

	// LRESULT CDateTimeWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	// {
	// 	if( !m_bInit ) return 0;
	// 	if( m_pOwner == NULL ) return 0;
	// 	// Copy text back
	// 	int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
	// 	LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
	// 	ASSERT(pstr);
	// 	if( pstr == NULL ) return 0;
	// 	::GetWindowText(m_hWnd, pstr, cchLen);
	// 	m_pOwner->m_sText = pstr;
	// 	m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_TEXTCHANGED);
	// 	return 0;
	// }

	//////////////////////////////////////////////////////////////////////////
	//
	CDateTimeUI::CDateTimeUI()
	{
		::GetLocalTime(&m_sysTime);
		m_bReadOnly = false;
		m_pWindow = NULL;
		//m_nDTUpdateFlag=DT_UPDATE;
		m_uStyle = WS_CHILD | DTS_SHORTDATEFORMAT;
		m_sFormat = _T("yyyy-MM-dd");
		m_bSetRange = false;
//		UpdateText();		// add by:daviyang35 初始化界面时显示时间
		//m_nDTUpdateFlag = DT_NONE;
	}

	LPCTSTR CDateTimeUI::GetClass() const
	{
		return _T("DateTimeUI");
	}

	LPVOID CDateTimeUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_DATETIME) == 0 ) return static_cast<CDateTimeUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CDateTimeUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	HWND CDateTimeUI::GetNativeWindow() const
	{
		if (m_pWindow) return m_pWindow->GetHWND();
		return NULL;
	}

	SYSTEMTIME& CDateTimeUI::GetTime()
	{
		return m_sysTime;
	}

	void CDateTimeUI::SetTime(SYSTEMTIME* pst)
	{
		m_sysTime = *pst;
		m_nDTUpdateFlag=DT_UPDATE;
		UpdateText();
		m_nDTUpdateFlag=DT_NONE;
		//Invalidate();
	}

	void CDateTimeUI::GetRange(SYSTEMTIME& pMinTime, SYSTEMTIME& pMaxTime)
	{
		pMinTime = m_pMinTime;
		pMaxTime = m_pMaxTime;
	}

	void CDateTimeUI::SetRange(SYSTEMTIME* pMinTime, SYSTEMTIME* pMaxTime)
	{
		m_pMinTime = *pMinTime;
		m_pMaxTime = *pMaxTime;
		m_bSetRange = true;
	}

	void CDateTimeUI::SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		Invalidate();
	}

	bool CDateTimeUI::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	void CDateTimeUI::UpdateText()
	{
		/*if (m_nDTUpdateFlag == DT_DELETE)
			SetText(_T(""));
		else */if (m_nDTUpdateFlag == DT_UPDATE)
		{
			CDuiString sText;
			if ((m_uStyle & DTS_TIMEFORMAT) == DTS_TIMEFORMAT)
			{
				sText.SmallFormat(_T("%02d:%02d"),
					m_sysTime.wHour, m_sysTime.wMinute);
			}
			else
			{
				sText.SmallFormat(_T("%4d-%02d-%02d"),
					m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay);
			}
			SetText(sText);
		}
	}

	void CDateTimeUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			return;
		}
		if( event.Type == UIEVENT_WINDOWSIZE )
		{
			if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if( m_pWindow != NULL ) return;
		}
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
		{
			if( m_pWindow ) return;
			m_pWindow = new CDateTimeWnd();
			ASSERT(m_pWindow);
			m_pWindow->Init(this);
			m_pWindow->ShowWindow();
		}
		if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
		{
			if( IsEnabled() ) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL )
				{
					m_pWindow = new CDateTimeWnd();
					ASSERT(m_pWindow);
				}
				if( m_pWindow != NULL )
				{
					m_pWindow->Init(this);
					m_pWindow->ShowWindow();
				}
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			return;
		}

		CLabelUI::DoEvent(event);
	}

	void CDateTimeUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("style")) == 0 )
		{
			if( _tcsstr(pstrValue, _T("time")) != NULL ) {
				m_uStyle &= ~DTS_SHORTDATEFORMAT;
				m_uStyle |= DTS_TIMEFORMAT;
				m_sFormat = _T("HH:mm");
			}
			else if ( _tcsstr(pstrValue, _T("date")) != NULL) {
				m_uStyle &= ~DTS_TIMEFORMAT;
				m_uStyle |= DTS_SHORTDATEFORMAT;
				m_sFormat = _T("yyy-MM-dd");
			}

			//初始化界面时显示时间
			m_nDTUpdateFlag=DT_UPDATE;
			UpdateText();
			m_nDTUpdateFlag=DT_NONE;
		}
		/*if( _tcscmp(pstrName, _T("format")) == 0 )
		{
			m_sFormat = pstrValue;
		}*/
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	UINT CDateTimeUI::GetTimeStyle()
	{
		return m_uStyle;
	}

	DuiLib::CDuiString CDateTimeUI::GetFormatString()
	{
		return m_sFormat;
	}

}
