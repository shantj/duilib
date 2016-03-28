#ifndef __UIDATETIME_H__
#define __UIDATETIME_H__

#pragma once

namespace DuiLib
{
	class CDateTimeWnd;

	/// 时间选择控件
	class UILIB_API CDateTimeUI : public CLabelUI
	{
		friend class CDateTimeWnd;
	public:
		CDateTimeUI();
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		HWND GetNativeWindow() const;

		SYSTEMTIME& GetTime();
		void SetTime(SYSTEMTIME* pst);

		void GetRange(SYSTEMTIME& pMinTime, SYSTEMTIME& pMaxTime);
		void SetRange(SYSTEMTIME* pMinTime, SYSTEMTIME* pMaxTime);

		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;

		void UpdateText();

		void DoEvent(TEventUI& event);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		UINT GetTimeStyle();

		CDuiString GetFormatString();

	protected:
		SYSTEMTIME m_sysTime;
		int        m_nDTUpdateFlag;
		bool       m_bReadOnly;
		UINT m_uStyle;
		CDuiString m_sFormat;

		CDateTimeWnd* m_pWindow;

		bool m_bSetRange;
		SYSTEMTIME m_pMinTime;
		SYSTEMTIME m_pMaxTime;
	};
}
#endif // __UIDATETIME_H__